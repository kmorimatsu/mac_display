/*----------------------------------------------------------------------------

Copyright (C) 2022, KenKen, all right reserved.

This program supplied herewith by KenKen is free software; you can
redistribute it and/or modify it under the terms of the same license written
here and only for non-commercial purpose.

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of FITNESS FOR A PARTICULAR
PURPOSE. The copyright owner and contributors are NOT LIABLE for any damages
caused by using this program.

----------------------------------------------------------------------------*/
const unsigned char FontData[256*8]={
//フォントデータ、キャラクタコード順に8バイトずつ、上位ビットが左
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
	0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00,
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
	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x00,
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
