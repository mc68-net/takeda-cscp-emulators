/*
	SHINKO SANGYO YS-6464A Emulator 'eYS-6464A'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.12.30 -

	[ display ]
*/

#include "display.h"
#include "../../fileio.h"

void DISPLAY::initialize()
{
	_memset(seg, 0, sizeof(seg));
	pb = pc = 0;
	
	// regist event
	vm->regist_vline_event(this);
}

void DISPLAY::event_vline(int v, int clock)
{
	if(!v) {
		_memset(seg, 0, sizeof(seg));
	}
	switch(pc & 0xf0)
	{
	case 0x80:
	case 0x90:
	case 0xa0:
	case 0xb0:
	case 0xc0:
	case 0xd0:
		for(int i = 0; i < 8; i++) {
			if(~pb & (1 << i))
				seg[(pc >> 4) & 7][i]++;
		}
		break;
	}
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_DISPLAY_PORT_B)
		pb = data;
	else if(id == SIG_DISPLAY_PORT_C)
		pc = data & 0xf0;
}

void DISPLAY::draw_screen()
{
	// draw leds
	scrntype col_h, col_l;
	scrntype col[9];
	
	col_h = RGB_COLOR(255, 0, 0);
	col_l = RGB_COLOR(107, 0, 0);
	col[0] = RGB_COLOR(38, 8, 0);
	
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < 8; j++) {
			col[j + 1] = (seg[i][j] > 8) ? col_h : col_l;
		}
		for(int y = 0; y < 40; y++) {
			scrntype* dest = emu->screen_buffer(leds[i].y + y) + leds[i].x;
			for(int x = 0; x < 28; x++) {
				dest[x] = col[pat_7seg_led[y][x]];
			}
		}
	}
}

