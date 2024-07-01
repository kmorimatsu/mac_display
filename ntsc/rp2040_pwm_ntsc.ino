/*
MIT License
Copyright (c) 2023 lovyan03
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// フレームワークは earlephilhower版のArduinoCoreを使用。
// ※ mbed公式版では動作しない
/*
  The code in this file is modified by Katsumi for MachiKania project.
*/

// #pragma GCC optimize ("O3")

#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include <pico/stdlib.h>
#include <hardware/dma.h>
#include <hardware/pwm.h>
#include <hardware/clocks.h>

// NTSC信号をPWM出力するピン
#define PIN_OUTPUT 19

// デバッグ用、割込み処理中HIGHになるピン
// #define PIN_DEBUG_BUSY 15

// フレームバッファ (RGB332形式。1ピクセルあたり1Byte)
#define FRAME_WIDTH 384
#define FRAME_HEIGHT 256
uint8_t framebuffer[FRAME_WIDTH * FRAME_HEIGHT];

// NTSC出力 1ラインあたりのサンプル数
#define NUM_LINE_SAMPLES 910  // 227.5 * 4

// NTSC出力 走査線数
#define NUM_LINES 525  // 走査線525本

// DMAピンポンバッファ
uint16_t dma_buffer[2][(NUM_LINE_SAMPLES+3)&~3u];

// RGB332 カラーパレット
uint32_t color_tbl[256];

static int pwm_dma_chan;

static void makeDmaBuffer(uint16_t* buf, size_t line_num)
{
  static uint_fast8_t odd_burst = 0;
  odd_burst = 2 ^ odd_burst;

  size_t y = line_num * 2;
  bool odd_frame = y > NUM_LINES;
  if (odd_frame) {
    y = y - NUM_LINES;
  }

  if (y < 20)
  {
    for (int i = 0; i < 2; ++i)
    {
      int sink_index = y + i;
      if (sink_index < 2) continue;
      bool long_sink = sink_index >= 8 && sink_index <= 13;
      auto b = &buf[i * (NUM_LINE_SAMPLES / 2)];
      int sink_len = 0;
      if (sink_index < 20) {
        sink_len = long_sink ? 390 : 32;
        memset(b, 0, sink_len * 2);
      }
      for (int j = sink_len; j < (NUM_LINE_SAMPLES / 2); ++j)
      {
        b[j] = 2;
      }
    }
  }
  else
  {
    y = (y-20);
    int sink_len = 68;
    int active_start = 148;
    int burst_start = 76;
    memset(buf, 0, sink_len * 2);

    for (int i = burst_start; i < burst_start + 4 * 9; ++i)
    {
      buf[i] = 1 + (((1 + i + odd_burst) & 2));
    }

    if (y < 18) { active_start = NUM_LINE_SAMPLES; }

    for (int i = burst_start + 4*10; i < active_start; ++i)
    {
      buf[i] = 2;
    }

    auto f = &framebuffer[(y>>1) * FRAME_WIDTH];
    auto b = &buf[active_start];
    for (int i = active_start; i < NUM_LINE_SAMPLES - 16; i+=4)
    {
      auto c = (const uint8_t*)(&color_tbl[*f++]);
      b[0] = c[(odd_burst + 0)    ];
      b[1] = c[(odd_burst + 1)    ];
      c = (const uint8_t*)(&color_tbl[*f++]);
      b[2] = c[(odd_burst + 2) & 3];
      b[3] = c[(odd_burst + 3) & 3];
      b += 4;
    }
  }
}

static uint32_t setup_palette_ntsc_inner(uint32_t rgb, uint32_t diff_level, uint32_t base_level, float satuation_base, float chroma_scale)
{
  static constexpr float BASE_RAD = (M_PI * 192) / 180; // 2.932153;
  uint8_t buf[4];

  uint32_t r = rgb >> 16;
  uint32_t g = (rgb >> 8) & 0xFF;
  uint32_t b = rgb & 0xFF;

  float y = r * 0.299f + g * 0.587f + b * 0.114f;
  float i = (b - y) * -0.2680f + (r - y) * 0.7358f;
  float q = (b - y) *  0.4127f + (r - y) * 0.4778f;
  y = y * diff_level / 256 + base_level;

  float phase_offset = atan2f(i, q) + BASE_RAD;
  float saturation = sqrtf(i * i + q * q) * chroma_scale;
  saturation = saturation * satuation_base;
  for (int j = 0; j < 4; j++)
  {
    int tmp = ((int)(128.5f + y + sinf(phase_offset + (float)M_PI / 2 * j) * saturation)) >> 8;
    buf[j] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
  }

  return buf[3] << 24
        | buf[2] << 16
        | buf[1] <<  8
        | buf[0] <<  0
        ;
}

static void setup_palette_ntsc_332(uint32_t* palette, uint_fast16_t white_level, uint_fast16_t black_level, uint_fast8_t chroma_level)
{
  float chroma_scale = chroma_level / 7168.0f;
  float satuation_base = black_level / 2;
  uint32_t diff_level = white_level - black_level;

  for (int rgb332 = 0; rgb332 < 256; ++rgb332)
  {
    int r = (( rgb332 >> 5)         * 0x49) >> 1;
    int g = (((rgb332 >> 2) & 0x07) * 0x49) >> 1;
    int b = (( rgb332       & 0x03) * 0x55);

    palette[rgb332] = setup_palette_ntsc_inner(r<<16|g<<8|b, diff_level, black_level, satuation_base, chroma_scale);
  }
}

static void irq_handler(void) {
  static bool flip = false;
  static size_t scanline = 0;
  dma_channel_set_read_addr(pwm_dma_chan, dma_buffer[flip], true);
  dma_hw->ints0 = 1u << pwm_dma_chan;	

#if defined ( PIN_DEBUG_BUSY )
  digitalWrite(PIN_DEBUG_BUSY, HIGH);
#endif
  flip = !flip;
  makeDmaBuffer(dma_buffer[flip], scanline);
  if (++scanline >= NUM_LINES) {
    scanline = 0;
  }
#if defined ( PIN_DEBUG_BUSY )
  digitalWrite(PIN_DEBUG_BUSY, LOW);
#endif
}

void setup(void)
{
#if defined ( PIN_DEBUG_BUSY )
  pinMode(PIN_DEBUG_BUSY, OUTPUT);
#endif
  setup_palette_ntsc_332(color_tbl, 960*2, 286*2, 128);

  // CPUを157.5MHzで動作させる
  uint32_t freq_khz = 157500;

  // PWM周期を11サイクルとする (157.5 [MHz] / 11 = 14318181 [Hz])
  uint32_t pwm_div = 11;

  // ※ NTSCのカラー信号を1周期4サンプルで出力する。
  // 出力されるカラーバースト信号は  14318181 [Hz] / 4 = 3579545 [Hz] となる。

  set_sys_clock_khz(freq_khz, true);

  gpio_set_function(PIN_OUTPUT, GPIO_FUNC_PWM);
  auto pwm_slice_num = pwm_gpio_to_slice_num(PIN_OUTPUT);

  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, 1);

  pwm_init(pwm_slice_num, &config, true);
  pwm_set_wrap(pwm_slice_num, pwm_div - 1);

  pwm_dma_chan = dma_claim_unused_channel(true);
  auto pwm_dma_chan_config = dma_channel_get_default_config(pwm_dma_chan);
  channel_config_set_transfer_data_size(&pwm_dma_chan_config, DMA_SIZE_16);
  channel_config_set_read_increment(&pwm_dma_chan_config, true);
  channel_config_set_write_increment(&pwm_dma_chan_config, false);
  channel_config_set_dreq(&pwm_dma_chan_config, DREQ_PWM_WRAP0 + pwm_slice_num);

  volatile void* wr_addr = &pwm_hw->slice[pwm_slice_num].cc;
  wr_addr = (volatile void*)(((uintptr_t)wr_addr) + 2);

  makeDmaBuffer(dma_buffer[0], 0);

  dma_channel_configure(
      pwm_dma_chan,
      &pwm_dma_chan_config,
      wr_addr,
      dma_buffer[0],
      NUM_LINE_SAMPLES,
      true
  );
  dma_channel_set_irq0_enabled(pwm_dma_chan, true);
  irq_set_exclusive_handler(DMA_IRQ_0, irq_handler);
  irq_set_enabled(DMA_IRQ_0, true);

}

extern const unsigned char font[256*8];
void loop(void)
{
  unsigned char* text=(unsigned char*)"Hello, NTSC World! Hello, MachiKania World!..      ";
  for (size_t y=0;y<FRAME_HEIGHT;y++){
    if (28>y || 243<y) {
      for (size_t x=0;x<FRAME_WIDTH;x++){
        framebuffer[x+y*FRAME_WIDTH] = 0x00;
      }
    } else {
      for (size_t x=0;x<FRAME_WIDTH;x++){
        if (x<14 || 349<x) framebuffer[x+y*FRAME_WIDTH] = 0x00;
        else if (y<124) {
          // Color test
          framebuffer[x+y*FRAME_WIDTH] = (((x-14)>>3)&1) ^ (((y-28)>>3)&1) ? 0x7f : 0xff;
          if ((font[text[(x-14)>>3]*8+((y-28)&7)]<<((x-14)&7))&0x80) framebuffer[x+y*FRAME_WIDTH] = 0xC0;
        } else {
          // Black and white test
          framebuffer[x+y*FRAME_WIDTH] = 0x00;
          if ((font[text[(x-14)>>3]*8+((y-28)&7)]<<((x-14)&7))&0x80) framebuffer[x+y*FRAME_WIDTH] = 0xff;
        }
      }
    }
  }
  while(true) asm("wfi");
}


/*
	The font bellow is provided personally for Katsumi,
	by Kenken (http://www.ze.em-net.ne.jp/~kenken/) with LGPL 2.1 license.
*/

const unsigned char font[256*8]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x08,0x0C,0xFE,0xFE,0x0C,0x08,0x00,
	0x00,0x20,0x60,0xFE,0xFE,0x60,0x20,0x00,
	0x18,0x3C,0x7E,0x18,0x18,0x18,0x18,0x00,
	0x00,0x18,0x18,0x18,0x18,0x7E,0x3C,0x18,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x30,0x30,0x30,0x30,0x00,0x00,0x30,0x00,
	0x6C,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00,
	0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00,
	0x18,0x7E,0xD8,0x7E,0x1A,0xFE,0x18,0x00,
	0xE0,0xE6,0x0C,0x18,0x30,0x6E,0xCE,0x00,
	0x78,0xCC,0xD8,0x70,0xDE,0xCC,0x76,0x00,
	0x0C,0x18,0x30,0x00,0x00,0x00,0x00,0x00,
	0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00,
	0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00,
	0xD6,0x7C,0x38,0xFE,0x38,0x7C,0xD6,0x00,
	0x00,0x30,0x30,0xFC,0x30,0x30,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,
	0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x38,0x38,0x00,
	0x00,0x06,0x0C,0x18,0x30,0x60,0xC0,0x00,
	0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0x18,0x38,0x78,0x18,0x18,0x18,0x7E,0x00,
	0x7C,0xC6,0x06,0x1C,0x70,0xC0,0xFE,0x00,
	0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00,
	0x0C,0x1C,0x3C,0x6C,0xFE,0x0C,0x0C,0x00,
	0xFE,0xC0,0xF8,0x0C,0x06,0xCC,0x78,0x00,
	0x3C,0x60,0xC0,0xFC,0xC6,0xC6,0x7C,0x00,
	0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x00,
	0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00,
	0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00,
	0x00,0x30,0x00,0x00,0x00,0x30,0x00,0x00,
	0x00,0x30,0x00,0x00,0x00,0x30,0x30,0x60,
	0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00,
	0x00,0x00,0xFE,0x00,0xFE,0x00,0x00,0x00,
	0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00,
	0x7C,0xC6,0x06,0x1C,0x30,0x00,0x30,0x00,
	0x3C,0x66,0xDE,0xF6,0xDC,0x60,0x3E,0x00,
	0x38,0x6C,0xC6,0xFE,0xC6,0xC6,0xC6,0x00,
	0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00,
	0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00,
	0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0x00,
	0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xFE,0x00,
	0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xC0,0x00,
	0x3C,0x66,0xC0,0xCE,0xC6,0x66,0x3C,0x00,
	0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00,
	0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,
	0x1E,0x0C,0x0C,0x0C,0x0C,0xCC,0x78,0x00,
	0xC6,0xCC,0xD8,0xF0,0xD8,0xCC,0xC6,0x00,
	0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFE,0x00,
	0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00,
	0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00,
	0x38,0x6C,0xC6,0xC6,0xC6,0x6C,0x38,0x00,
	0xFC,0xC6,0xC6,0xFC,0xC0,0xC0,0xC0,0x00,
	0x38,0x6C,0xC6,0xC6,0xDE,0x6C,0x3E,0x00,
	0xFC,0xC6,0xC6,0xFC,0xD8,0xCC,0xC6,0x00,
	0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0x00,
	0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00,
	0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0xC6,0xC6,0xC6,0x6C,0x6C,0x38,0x38,0x00,
	0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00,
	0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00,
	0xCC,0xCC,0xCC,0x78,0x30,0x30,0x30,0x00,
	0xFE,0x06,0x0C,0x38,0x60,0xC0,0xFE,0x00,
	0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,
	0xCC,0xCC,0x78,0xFC,0x30,0xFC,0x30,0x00,
	0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,
	0x30,0x78,0xCC,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x7C,0x0C,0x7C,0xCC,0x7E,0x00,
	0xC0,0xC0,0xFC,0xE6,0xC6,0xE6,0xFC,0x00,
	0x00,0x00,0x7C,0xC6,0xC0,0xC6,0x7C,0x00,
	0x06,0x06,0x7E,0xCE,0xC6,0xCE,0x7E,0x00,
	0x00,0x00,0x7C,0xC6,0xFE,0xC0,0x7C,0x00,
	0x1C,0x36,0x30,0xFC,0x30,0x30,0x30,0x00,
	0x00,0x00,0x7E,0xCE,0xCE,0x7E,0x06,0x7C,
	0xC0,0xC0,0xFC,0xE6,0xC6,0xC6,0xC6,0x00,
	0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00,
	0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0xCC,0x78,
	0xC0,0xC0,0xCC,0xD8,0xF0,0xF8,0xCC,0x00,
	0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,
	0x00,0x00,0xFC,0xB6,0xB6,0xB6,0xB6,0x00,
	0x00,0x00,0xFC,0xE6,0xC6,0xC6,0xC6,0x00,
	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0x00,
	0x00,0x00,0xFC,0xE6,0xE6,0xFC,0xC0,0xC0,
	0x00,0x00,0x7E,0xCE,0xCE,0x7E,0x06,0x06,
	0x00,0x00,0xDC,0xE6,0xC0,0xC0,0xC0,0x00,
	0x00,0x00,0x7E,0xC0,0x7C,0x06,0xFC,0x00,
	0x30,0x30,0xFC,0x30,0x30,0x36,0x1C,0x00,
	0x00,0x00,0xC6,0xC6,0xC6,0xCE,0x76,0x00,
	0x00,0x00,0xC6,0xC6,0xC6,0x6C,0x38,0x00,
	0x00,0x00,0x86,0xB6,0xB6,0xB6,0xFC,0x00,
	0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00,
	0x00,0x00,0xC6,0xC6,0xCE,0x7E,0x06,0x7C,
	0x00,0x00,0xFE,0x0C,0x38,0x60,0xFE,0x00,
	0x3C,0x60,0x60,0xC0,0x60,0x60,0x3C,0x00,
	0x30,0x30,0x00,0x00,0x00,0x30,0x30,0x00,
	0xF0,0x18,0x18,0x0C,0x18,0x18,0xF0,0x00,
	0x60,0xB6,0x1C,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
	0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,
	0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
	0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
	0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,
	0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,
	0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,
	0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0x18,0x18,0x18,0x18,0xFF,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0xFF,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0xF8,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x1F,0x18,0x18,0x18,
	0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x00,0x00,0x00,0x00,0x1F,0x18,0x18,0x18,
	0x00,0x00,0x00,0x00,0xF8,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x1F,0x00,0x00,0x00,
	0x18,0x18,0x18,0x18,0xF8,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x07,0x0C,0x18,0x18,
	0x00,0x00,0x00,0x00,0xE0,0x30,0x18,0x18,
	0x18,0x18,0x18,0x0C,0x07,0x00,0x00,0x00,
	0x18,0x18,0x18,0x30,0xE0,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x78,0x68,0x78,0x00,
	0x78,0x60,0x60,0x60,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x18,0x18,0x18,0x78,0x00,
	0x00,0x00,0x00,0x00,0x60,0x30,0x18,0x00,
	0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,
	0xFE,0x06,0x06,0xFE,0x06,0x0C,0x78,0x00,
	0x00,0x00,0xFC,0x0C,0x38,0x30,0x60,0x00,
	0x00,0x00,0x0C,0x18,0x38,0x78,0x18,0x00,
	0x00,0x00,0x30,0xFC,0xCC,0x0C,0x38,0x00,
	0x00,0x00,0x00,0xFC,0x30,0x30,0xFC,0x00,
	0x00,0x00,0x18,0xFC,0x38,0x78,0xD8,0x00,
	0x00,0x00,0x60,0xFC,0x6C,0x68,0x60,0x00,
	0x00,0x00,0x00,0x78,0x18,0x18,0xFC,0x00,
	0x00,0x00,0x7C,0x0C,0x7C,0x0C,0x7C,0x00,
	0x00,0x00,0x00,0xAC,0xAC,0x0C,0x38,0x00,
	0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,
	0xFE,0x06,0x06,0x34,0x38,0x30,0x60,0x00,
	0x06,0x0C,0x18,0x38,0x78,0xD8,0x18,0x00,
	0x18,0xFE,0xC6,0xC6,0x06,0x0C,0x38,0x00,
	0x00,0x7E,0x18,0x18,0x18,0x18,0x7E,0x00,
	0x18,0xFE,0x18,0x38,0x78,0xD8,0x18,0x00,
	0x30,0xFE,0x36,0x36,0x36,0x36,0x6C,0x00,
	0x18,0x7E,0x18,0x7E,0x18,0x18,0x18,0x00,
	0x3E,0x66,0xC6,0x0C,0x18,0x30,0xE0,0x00,
	0x60,0x7E,0xD8,0x18,0x18,0x18,0x30,0x00,
	0x00,0xFE,0x06,0x06,0x06,0x06,0xFE,0x00,
	0x6C,0xFE,0x6C,0x0C,0x0C,0x18,0x30,0x00,
	0x00,0xF0,0x00,0xF6,0x06,0x0C,0xF8,0x00,
	0xFE,0x06,0x0C,0x18,0x38,0x6C,0xC6,0x00,
	0x60,0xFE,0x66,0x6C,0x60,0x60,0x3E,0x00,
	0xC6,0xC6,0x66,0x06,0x0C,0x18,0xF0,0x00,
	0x3E,0x66,0xE6,0x3C,0x18,0x30,0xE0,0x00,
	0x0C,0x78,0x18,0xFE,0x18,0x18,0xF0,0x00,
	0x00,0xD6,0xD6,0xD6,0x0C,0x18,0xF0,0x00,
	0x7C,0x00,0xFE,0x18,0x18,0x30,0x60,0x00,
	0x30,0x30,0x38,0x3C,0x36,0x30,0x30,0x00,
	0x18,0x18,0xFE,0x18,0x18,0x30,0x60,0x00,
	0x00,0x7C,0x00,0x00,0x00,0x00,0xFE,0x00,
	0x00,0x7E,0x06,0x6C,0x18,0x36,0x60,0x00,
	0x18,0x7E,0x0C,0x18,0x3C,0x7E,0x18,0x00,
	0x06,0x06,0x06,0x0C,0x18,0x30,0x60,0x00,
	0x30,0x18,0x0C,0xC6,0xC6,0xC6,0xC6,0x00,
	0xC0,0xC0,0xFE,0xC0,0xC0,0xC0,0x7E,0x00,
	0x00,0xFE,0x06,0x06,0x0C,0x18,0x70,0x00,
	0x00,0x30,0x78,0xCC,0x06,0x06,0x00,0x00,
	0x18,0x18,0xFE,0x18,0xDB,0xDB,0x18,0x00,
	0xFE,0x06,0x06,0x6C,0x38,0x30,0x18,0x00,
	0x00,0x3C,0x00,0x3C,0x00,0x7C,0x06,0x00,
	0x0C,0x18,0x30,0x60,0xCC,0xFC,0x06,0x00,
	0x02,0x36,0x3C,0x18,0x3C,0x6C,0xC0,0x00,
	0x00,0xFE,0x30,0xFE,0x30,0x30,0x3E,0x00,
	0x30,0x30,0xFE,0x36,0x3C,0x30,0x30,0x00,
	0x00,0x78,0x18,0x18,0x18,0x18,0xFE,0x00,
	0xFE,0x06,0x06,0xFE,0x06,0x06,0xFE,0x00,
	0x7C,0x00,0xFE,0x06,0x0C,0x18,0x30,0x00,
	0xC6,0xC6,0xC6,0x06,0x06,0x0C,0x38,0x00,
	0x6C,0x6C,0x6C,0x6E,0x6E,0x6C,0xC8,0x00,
	0x60,0x60,0x60,0x66,0x6C,0x78,0x70,0x00,
	0x00,0xFE,0xC6,0xC6,0xC6,0xC6,0xFE,0x00,
	0x00,0xFE,0xC6,0xC6,0x06,0x0C,0x38,0x00,
	0x00,0xF0,0x06,0x06,0x0C,0x18,0xF0,0x00,
	0x18,0xCC,0x60,0x00,0x00,0x00,0x00,0x00,
	0x70,0xD8,0x70,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,
	0x18,0x18,0x1F,0x18,0x18,0x1F,0x18,0x18,
	0x18,0x18,0xFF,0x18,0x18,0xFF,0x18,0x18,
	0x18,0x18,0xF8,0x18,0x18,0xF8,0x18,0x18,
	0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF,
	0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF,
	0xFF,0x7F,0x3F,0x1F,0x0F,0x07,0x03,0x01,
	0xFF,0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,
	0x10,0x38,0x7C,0xFE,0xFE,0x38,0x7C,0x00,
	0x6C,0xFE,0xFE,0xFE,0x7C,0x38,0x10,0x00,
	0x10,0x38,0x7C,0xFE,0x7C,0x38,0x10,0x00,
	0x38,0x38,0xFE,0xFE,0xD6,0x10,0x7C,0x00,
	0x00,0x3C,0x7E,0x7E,0x7E,0x7E,0x3C,0x00,
	0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0x03,0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,
	0x80,0xC0,0x60,0x30,0x18,0x0C,0x06,0x03,
	0x83,0xC6,0x6C,0x38,0x38,0x6C,0xC6,0x83,
	0xFE,0xB6,0xB6,0xFE,0x86,0x86,0x86,0x00,
	0xC0,0xFE,0xD8,0x7E,0x58,0xFE,0x18,0x00,
	0x7E,0x66,0x7E,0x66,0x7E,0x66,0xC6,0x00,
	0xFE,0xC6,0xC6,0xFE,0xC6,0xC6,0xFE,0x00,
	0x06,0xEF,0xA6,0xFF,0xA2,0xFF,0x0A,0x06,
	0x00,0x38,0x6C,0xC6,0x7C,0x34,0x6C,0x00,
	0xFC,0x6C,0xFE,0x6E,0xF6,0xEC,0x6C,0x78,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};