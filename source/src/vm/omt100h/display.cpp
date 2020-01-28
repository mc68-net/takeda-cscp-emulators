/*
	Gijutsu-Hyoron-Sha Babbage-2nd Emulator 'eBabbage-2nd'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.12.26 -

	[ display ]
*/

#include "display.h"
#include "../../fileio.h"

void DISPLAY::initialize()
{
	_memset(seg, 0, sizeof(seg));
	ls373 = 0;
	pio_7seg = pio_8bit = 0;
	
	// regist event
	vm->regist_vline_event(this);
}

void DISPLAY::event_vline(int v, int clock)
{
	if(!v) {
		_memset(seg, 0, sizeof(seg));
	}
	for(int i = 0; i < 6; i++) {
		if(pio_7seg & (1 << i)) {
			for(int j = 0; j < 7; j++) {
				if(ls373 & (1 << j)) {
					seg[i][j]++;
				}
			}
		}
	}
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_DISPLAY_7SEG_LED) {
		if((pio_7seg & 0x80) && !(data & 0x80)) {
			ls373 = pio_7seg & 0x7f;
		}
		pio_7seg = data;
	}
	else if(id == SIG_DISPLAY_8BIT_LED) {
		pio_8bit = data;
	}
}

void DISPLAY::draw_screen()
{
	// draw leds
	scrntype col_h, col_l;
	scrntype col[8];
	
	col_h = RGB_COLOR(255, 0, 0);
	col_l = RGB_COLOR(107, 0, 0);
	col[0] = RGB_COLOR(82, 8, 16);
	
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < 7; j++) {
			col[j + 1] = (seg[i][j] > 8) ? col_h : col_l;
		}
		for(int y = 0; y < 58; y++) {
			scrntype* dest = emu->screen_buffer(leds[i].y + y) + leds[i].x;
			for(int x = 0; x < 34; x++) {
				dest[x] = col[pat_7seg_led[y][x]];
			}
		}
	}
	for(int i = 0; i < 8; i++) {
		col[1] = (pio_8bit & (1 << i)) ? col_h : col_l;
		for(int y = 0; y < 17; y++) {
			scrntype* dest = emu->screen_buffer(leds[i + 6].y + y) + leds[i + 6].x;
			for(int x = 0; x < 17; x++) {
				dest[x] = col[pat_8bit_led[y][x]];
			}
		}
	}
}
