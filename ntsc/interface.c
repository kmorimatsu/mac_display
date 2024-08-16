#include "rp2040_pwm_ntsc_textgraph.h"
#include "text_graph_library.h"
#include <stdlib.h>
#include "pico/stdlib.h"
#include "text_graph_library.h"

/*
	GP0-GP7: 8 bit I/O
	GP8: MOSI, DC
	GP9: MOSI, /WR
	GP10: MOSI, /RD
	GP11: MISO, /BUSY
*/

#define MOSI_DC_PIN 8
#define MOSI_WR_PIN 9
#define MOSI_RD_PIN 10
#define MISO_BUSY_PIN 11

#define INTERFACE_IN_MASK  0b0011111111111
#define INTERFACE_OUT_MASK 0b0100000000000
#define MOSI_WR_RD_MASK	   0b0011000000000
#define IO_8_BIT_MASK	   0b0000011111111
#define MOSI_DC_MASK	   0b0000100000000


// 0x00 - 0x7F: COMMAND_PRINTCHAR
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

static unsigned char g_command;
static unsigned char g_parameters[256];
static unsigned char g_parameter_pos;
static short* g_short_parameters=(short*)&g_parameters[0];
static int* g_int_parameters=(int*)&g_parameters[0];
static int g_redraw_pos;

void set_command(unsigned char data8){
	// Read 8 bit data
	g_command=data8;
	// Busy line will be L
	gpio_put(MISO_BUSY_PIN,0);
	// Reset parameter position
	g_parameter_pos=0;
	// Wait until WR to H
	while(!gpio_get(MOSI_WR_PIN));
	// Do immediate command
	switch(data8){
		case COMMAND_CLS:
			cls();
			break;
		case COMMAND_TEXTREDRAW:
			g_redraw_pos=0;
			break;
		default:
			if (data8<0x80) printchar(data8);
			break;
	}
	// All done
}

void set_data(unsigned char data8){
	// Read 8 bit data
	g_parameters[g_parameter_pos++]=data8;
	// Busy line will be L
	gpio_put(MISO_BUSY_PIN,0);
	// Wait until WR to H
	while(!gpio_get(MOSI_WR_PIN));
	// Do command
	switch(g_command){
		//void printchar(unsigned char n);
		case COMMAND_PRINTCHAR:
			printchar(data8);
			break;
		//void printstr(unsigned char *s);
		case COMMAND_PRINTSTR:
			if (0==data8) {
				// Null character found. Let's print all words
				printstr(g_parameters);
				g_parameter_pos=0;
			} else if (255==g_parameter_pos) {
				// The buffer is full. Let's print 255 characters
				g_parameters[g_parameter_pos]=0;
				printstr(g_parameters);				
				g_parameter_pos=0;
			}
			break;
		case COMMAND_TEXTREDRAW:
			if (g_redraw_pos<ATTROFFSET*2) TVRAM[g_redraw_pos++]=data8;
			break;
		default:
			break;
	}
	switch(g_parameter_pos){
		case 1:
			//void setcursorcolor(unsigned char c);
			if (COMMAND_SETCURSORCOLOR==g_command) setcursorcolor(g_parameters[0]);
			break;
		case 2:
			break;
		case 3:
			//void setcursor(unsigned char x,unsigned char y,unsigned char c);
			if (COMMAND_SETCURSOR==g_command) setcursor(g_parameters[0],g_parameters[1],g_parameters[2]);
			break;
		case 4:
			//void printnum(unsigned int n);
			if (COMMAND_PRINTNUM==g_command) printnum((unsigned int)g_int_parameters[0]);
			//void set_palette(unsigned char c,unsigned char b,unsigned char r,unsigned char g);
			if (COMMAND_SET_PALETTE==g_command) set_palette(g_parameters[0],g_parameters[1],g_parameters[2],g_parameters[3]);
			break;
		case 5:
			//void printnum2(unsigned int n,unsigned char e);
			if (COMMAND_PRINTNUM2==g_command) printnum2((unsigned int)g_int_parameters[0],g_parameters[4]);
			break;
		default:
			break;
	}
	// All done
}

/*
	The main loop follows.
*/
void main_loop(void){
	unsigned int input_data;
	while(true){
		// Not busy now
		gpio_put(MISO_BUSY_PIN,1);
		while(true){
			input_data=gpio_get_all();
			switch(input_data&MOSI_WR_RD_MASK){
				case 1<<MOSI_RD_PIN: // /RD=H, /WR=L
					if (input_data&MOSI_DC_MASK) set_command(input_data&IO_8_BIT_MASK);
					else set_data(input_data&IO_8_BIT_MASK);
					break;
				case 1<<MOSI_WR_PIN: // /RD=L, /WR=H
					break;
				default:
					continue;
			}
			// All job(s) done. Not busy now
			gpio_put(MISO_BUSY_PIN,1);
		}
	}
}

/*
	Initialize the interface
*/
void interface_init(void){
	int i;
	// Input ports (all pull up)
	gpio_init_mask(INTERFACE_IN_MASK);
	gpio_set_dir_in_masked(INTERFACE_IN_MASK);
	for (i=0;i<29;i++) {
		if (INTERFACE_IN_MASK&(1<<i)) gpio_pull_up(i);
	}
	// Output ports (output H in the beginning)
	gpio_init_mask(INTERFACE_OUT_MASK);
	gpio_set_dir_out_masked(INTERFACE_OUT_MASK);
	gpio_put_masked(INTERFACE_OUT_MASK,INTERFACE_OUT_MASK);
	// NOP command in the beginning
	g_command=COMMAND_NOP;
}