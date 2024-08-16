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
/*
	Modified by Katsumi for MachiKania-NTSC parallel interface
*/
//LCDテキスト・グラフィックライブラリ

#include "pico/stdlib.h"
#include "graphlib.h"
#include "LCDdriver.h"
#include "../config.h"

unsigned char TVRAM[ATTROFFSET*2+1] __attribute__ ((aligned (4)));
unsigned char *fontp; //フォント格納アドレス、初期化時はFontData、RAM指定することでPCGを実現
unsigned int bgcolor; // バックグランドカラー
//unsigned char twidth; //テキスト1行文字数
unsigned char *cursor;
unsigned char cursorcolor;
unsigned short palette[256];
int WIDTH_X; // 横方向文字数
int WIDTH_Y; // 縦方向文字数

/*
	The interface with NTSC pi pico follows

	GP10: MOSI, DC
	GP11: MOSI, /WR
	GP13: MOSI, /RD
	GP12: MISO, /BUSY (GP27 for test by MachiKania)
	
	Communication sequence (write to slave)
	
		Master waits until /BUSY to H
		Master stops interruption if needed
		Master shows the data to data lines
		Master set /WR to L
		Slave recognizes /WR signal, then read data and set /BUSY to L
		Master waits until /BUSY to L, then set /WR to H
		Master set data lines to input mode and restart interruption
		Slave waits until /WR to H, then start the job
		Slave set /BUSY to L after finishing the job
	
	Communication sequence (read from slave)
	
		Master waits until /BUSY to H
		Master stops interruption if needed
		Master set /RD to L
		Slave recognizes /RD signal, then set data to data line and set /BUSY to L
		Master waits until /BUSY to L, then read data, set /RD to H
		Master restart interruption
		Slave waits until /RD to H, then set data lines to input mode and set /BUSY to H
	
*/

#define PARALLEL_DC_PIN   10
#define PARALLEL_WR_PIN   11
#define PARALLEL_RD_PIN   13
#define PARALLEL_BUSY_PIN 12
#define PARALLEL_RESET_PIN 14
#define PARALLEL_DATA_MASK 0xff

#define COMMAND_NOP			   0x80
#define COMMAND_SETCURSOR	   0x90
#define COMMAND_SETCURSORCOLOR 0x91
#define COMMAND_PRINTCHAR	   0x92
#define COMMAND_PRINTSTR	   0x93
#define COMMAND_PRINTNUM	   0x94
#define COMMAND_PRINTNUM2	   0x95
#define COMMAND_CLS			   0x96
#define COMMAND_SET_PALETTE    0x97
#define COMMAND_TEXTREDRAW     0x98

void parallel_init(void){
	int i;
	// Init data line
	gpio_init_mask(PARALLEL_DATA_MASK);
	gpio_set_dir_in_masked(PARALLEL_DATA_MASK);
	for(i=0;i<30;i++) {
		if ((1<<i)&PARALLEL_DATA_MASK) gpio_pull_up(i);
	}
	// Init DC pin
	gpio_init(PARALLEL_DC_PIN);
	gpio_put(PARALLEL_DC_PIN, 1);
	gpio_set_dir(PARALLEL_DC_PIN, GPIO_OUT);
	// Init WR pin
	gpio_init(PARALLEL_WR_PIN);
	gpio_put(PARALLEL_WR_PIN, 1);
	gpio_set_dir(PARALLEL_WR_PIN, GPIO_OUT);
	// Init RD pin
	gpio_init(PARALLEL_RD_PIN);
	gpio_put(PARALLEL_RD_PIN, 1);
	gpio_set_dir(PARALLEL_RD_PIN, GPIO_OUT);
	// Init BUSY pin
	gpio_init(PARALLEL_BUSY_PIN);
	gpio_set_dir(PARALLEL_BUSY_PIN, GPIO_IN);
	gpio_pull_up(PARALLEL_BUSY_PIN);
	// Reset
	gpio_init(PARALLEL_RESET_PIN);
	gpio_put(PARALLEL_RESET_PIN, 0);
	gpio_set_dir(PARALLEL_RESET_PIN, GPIO_OUT);
	sleep_ms(10);
	gpio_init(PARALLEL_RESET_PIN);
	gpio_set_dir(PARALLEL_RESET_PIN, GPIO_IN);
	gpio_pull_up(PARALLEL_RESET_PIN);
}

void parallel_send_main(unsigned char dat){
	// Wait until /BUSY will be H
	while(!gpio_get(PARALLEL_BUSY_PIN));
	// Show the data to data line
	gpio_set_dir_out_masked(PARALLEL_DATA_MASK);
	gpio_put_masked(PARALLEL_DATA_MASK,dat);
	// Set /WR to L
	gpio_put(PARALLEL_WR_PIN,0);
	// Wait until /BUSY will be L
	while(gpio_get(PARALLEL_BUSY_PIN));
	// Set /WR to H
	gpio_put(PARALLEL_WR_PIN,1);
	// Set data line to input mode
	gpio_set_dir_in_masked(PARALLEL_DATA_MASK);
}

void parallel_send_command(unsigned char com){
	// Command mode
	gpio_put(PARALLEL_DC_PIN,1);
	// Send command
	parallel_send_main(com);
}

void parallel_send_data(unsigned char dat){
	// Data mode
	gpio_put(PARALLEL_DC_PIN,0);
	// Send data
	parallel_send_main(dat);
}

unsigned char parallel_receive_data(void){
	unsigned char dat;
	// Wait until /BUSY will be H
	while(!gpio_get(PARALLEL_BUSY_PIN));
	// Set /RD to L
	gpio_put(PARALLEL_RD_PIN,0);
	// Wait until /BUSY will be L
	while(gpio_get(PARALLEL_BUSY_PIN));
	// Read data
	dat=gpio_get_all() & PARALLEL_DATA_MASK;
	// Set /RD to H
	gpio_put(PARALLEL_RD_PIN,1);
	return dat;
}

/*
	Main graphlib routines follow
*/

void set_palette(unsigned char n,unsigned char b,unsigned char r,unsigned char g){
//テキスト／グラフィック共用カラーパレット設定
	//palette[n]=((r>>3)<<11)+((g>>2)<<5)+(b>>3);
	parallel_send_command(COMMAND_SET_PALETTE);
	parallel_send_data(n);
	parallel_send_data(b);
	parallel_send_data(r);
	parallel_send_data(g);
}

void g_pset(int x,int y,unsigned char c)
// (x,y)の位置にカラーパレット番号cで点を描画
{

}

void g_putbmpmn(int x,int y,unsigned short m,unsigned short n,const unsigned char bmp[])
// 横m*縦nドットのキャラクターを座標x,yに表示
// unsigned char bmp[m*n]配列に、単純にカラー番号を並べる
// カラー番号が0の部分は透明色として扱う
{

}

// 縦m*横nドットのキャラクター消去
// カラー0で塗りつぶし
void g_clrbmpmn(int x,int y,unsigned short m,unsigned short n)
{

}

void g_gline(int x1,int y1,int x2,int y2,unsigned char c)
// (x1,y1)-(x2,y2)にカラーパレット番号cで線分を描画
{

}

void g_hline(int x1,int x2,int y,unsigned char c)
// (x1,y)-(x2,y)への水平ラインを高速描画
{

}

void g_circle(int x0,int y0,unsigned int r,unsigned char c)
// (x0,y0)を中心に、半径r、カラーパレット番号cの円を描画
{

}
void g_boxfill(int x1,int y1,int x2,int y2,unsigned char c)
// (x1,y1),(x2,y2)を対角線とするカラーパレット番号cで塗られた長方形を描画
{

}
void g_circlefill(int x0,int y0,unsigned int r,unsigned char c)
// (x0,y0)を中心に、半径r、カラーパレット番号cで塗られた円を描画
{

}
void g_putfont(int x,int y,unsigned char c,int bc,unsigned char n)
//8*8ドットのアルファベットフォント表示
//座標(x,y)、カラーパレット番号c
//bc:バックグランドカラー、負数の場合無視
//n:文字番号
{

}

void g_printstr(int x,int y,unsigned char c,int bc,unsigned char *s){
	//座標(x,y)からカラーパレット番号cで文字列sを表示、bc:バックグランドカラー
	//bcが負の場合は無視

}
void g_printnum(int x,int y,unsigned char c,int bc,unsigned int n){
	//座標(x,y)にカラー番号cで数値nを表示、bc:バックグランドカラー

}
void g_printnum2(int x,int y,unsigned char c,int bc,unsigned int n,unsigned char e){
	//座標(x,y)にカラー番号cで数値nを表示、bc:バックグランドカラー、e桁で表示

}
unsigned int g_color(int x,int y){
//座標(x,y)の色情報を返す、画面外は0を返す
//パレット番号ではないことに注意

}

// テキスト画面クリア
void clearscreen(void)
{
	unsigned int *vp;
	int i;
	vp=(unsigned int *)TVRAM;
	for(i=0;i<ATTROFFSET*2/4;i++) *vp++=0;
	cursor=TVRAM;
	parallel_send_command(COMMAND_CLS);
}

// グラフィック画面クリア
void g_clearscreen(void)
{

}

// カーソル位置の文字をテキストVRAMにしたがって液晶に出力
void putcursorchar(void){
	//g_putfont(((cursor-TVRAM)%WIDTH_X)*8,((cursor-TVRAM)/WIDTH_X)*8,*(cursor+ATTROFFSET),bgcolor,*cursor);
	printchar(cursor[0]);
	printchar(0x08);
}

void textredraw(void){
// テキスト画面再描画
// テキストVRAMの内容にしたがって液晶に出力
	int i;
	parallel_send_command(COMMAND_TEXTREDRAW);
	for(i=0;i<WIDTH_X*WIDTH_Y*2;i++) parallel_send_data(TVRAM[i]);
}

void windowscroll(int y1,int y2){
	// scroll up text bitween line y1 and y2
	unsigned char *p1,*p2,*vramend;

	vramend=TVRAM+WIDTH_X*(y2+1);
	p1=TVRAM+WIDTH_X*y1;
	p2=p1+WIDTH_X;
	while(p2<vramend){
		*(p1+ATTROFFSET)=*(p2+ATTROFFSET);
		*p1++=*p2++;
	}
	while(p1<vramend){
		*(p1+ATTROFFSET)=0;
		*p1++=0;
	}
	textredraw();
}
void vramscroll(void){
	unsigned char *p1,*p2,*vramend;

	vramend=TVRAM+WIDTH_X*WIDTH_Y;
	p1=TVRAM;
	p2=p1+WIDTH_X;
	while(p2<vramend){
		*(p1+ATTROFFSET)=*(p2+ATTROFFSET);
		*p1++=*p2++;
	}
	while(p1<vramend){
		*(p1+ATTROFFSET)=0;
		*p1++=0;
	}
	textredraw();
}
void vramscrolldown(void){
	unsigned char *p1,*p2,*vramend;

	vramend=TVRAM+WIDTH_X*WIDTH_Y;
	p1=vramend-1;
	p2=p1-WIDTH_X;
	while(p2>=TVRAM){
		*(p1+ATTROFFSET)=*(p2+ATTROFFSET);
		*p1--=*p2--;
	}
	while(p1>=TVRAM){
		*(p1+ATTROFFSET)=0;
		*p1--=0;
	}
	textredraw();
}
void setcursor(unsigned char x,unsigned char y,unsigned char c){
	//カーソルを座標(x,y)にカラー番号cに設定
	if(x>=WIDTH_X || y>=WIDTH_Y) return;
	cursor=TVRAM+y*WIDTH_X+x;
	cursorcolor=c;
	parallel_send_command(COMMAND_SETCURSOR);
	parallel_send_data(x);
	parallel_send_data(y);
	parallel_send_data(c);	
}
void setcursorcolor(unsigned char c){
	//カーソル位置そのままでカラー番号をcに設定
	cursorcolor=c;
	parallel_send_command(COMMAND_SETCURSOR);
	parallel_send_data(c);	
}
void printchar_main(unsigned char n){
	//カーソル位置にテキストコードnを1文字表示し、カーソルを1文字進める
	//画面最終文字表示してもスクロールせず、次の文字表示時にスクロールする
	if(cursor<TVRAM || cursor>TVRAM+WIDTH_X*WIDTH_Y) return;
	if(cursor==TVRAM+WIDTH_X*WIDTH_Y){
		vramscroll();
		cursor-=WIDTH_X;
	}
	if(n=='\n'){
		//改行
		cursor+=WIDTH_X-((cursor-TVRAM)%WIDTH_X);
	} else if(n==0x08){
		//BS
		if (TVRAM<cursor) cursor--;
	} else{
		*cursor=n;
		*(cursor+ATTROFFSET)=cursorcolor;
		cursor++;
	}
}
void printchar(unsigned char n){
	if (n<0x80) {
		parallel_send_command(n);
	} else {
		parallel_send_command(COMMAND_PRINTCHAR);
		parallel_send_data(n);
	}
	printchar_main(n);
}
void printstr(unsigned char *s){
	//カーソル位置に文字列sを表示
	while(*s){
		printchar(*s++);
	}
}
void printnum(unsigned int n){
	//カーソル位置に符号なし整数nを10進数表示
	unsigned int d,n1;
	n1=n/10;
	d=1;
	while(n1>=d){
		d*=10;
	}
	while(d!=0){
		printchar('0'+n/d);
		n%=d;
		d/=10;
	}
}
void printnum2(unsigned int n,unsigned char e){
	//カーソル位置に符号なし整数nをe桁の10進数表示（前の空き桁部分はスペースで埋める）
	unsigned int d,n1;
	if(e==0) return;
	n1=n/10;
	d=1;
	e--;
	while(e>0 && n1>=d){
		d*=10;
		e--;
	}
	if(e==0 && n1>d) n%=d*10;
	for(;e>0;e--) printchar(' ');
	while(d!=0){
		printchar('0'+n/d);
		n%=d;
		d/=10;
	}
}
void cls(void){
	//画面消去しカーソルを先頭に移動
	clearscreen();
}
void startPCG(unsigned char *p,int a){
// RAMフォント（PCG）の利用開始
// p：RAMフォントの格納アドレス（8*256＝2048バイト）
// a： システムフォントからのコピー指定。0の場合コピーなし、0以外でコピー
	int i;
	if(a){
		for(i=0;i<8*256;i++) *p++=FontData[i];
		fontp=p-8*256;
	}
	else fontp=p;
}
void stopPCG(void){
// RAMフォント（PCG）の利用停止
	fontp=(unsigned char *)FontData;
}
void set_bgcolor(unsigned char b,unsigned char r,unsigned char g)
{
	bgcolor=((r>>3)<<11)+((g>>2)<<5)+(b>>3);
	textredraw();
}
void init_palette(void){
	//カラーパレット初期化
	int i;
	for(i=0;i<8;i++){
		set_palette(i,255*(i&1),255*((i>>1)&1),255*(i>>2));
	}
	for(i=0;i<8;i++){
		set_palette(i+8,128*(i&1),128*((i>>1)&1),128*(i>>2));
	}
	for(i=16;i<256;i++){
		set_palette(i,255,255,255);
	}
}
void init_textgraph(unsigned char align){
	//テキスト・グラフィックNTSCライブラリの使用開始
	parallel_init();
	//パレット設定
	//LCD縦横設定
	fontp=(unsigned char *)FontData;
	bgcolor=0; //バックグランドカラーは黒
	init_palette(); //カラーパレット初期化
	setcursorcolor(7);
	set_lcdalign(align);
}
void set_lcdalign(unsigned char align){
	// 液晶の縦横設定は無効で、幅と高さは固定
	X_RES=336;
	Y_RES=216;
	WIDTH_X=42;
	WIDTH_Y=27;
	clearscreen();
}
