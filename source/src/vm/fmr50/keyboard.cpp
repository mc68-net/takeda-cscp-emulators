/*
	Fujitsu FMR-50 Emulator 'eFMR-50'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.05.01 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../fifo.h"

void KEYBOARD::initialize()
{
	key_buf = new FIFO(64);
	vm->regist_frame_event(this);
}

void KEYBOARD::reset()
{
	_memset(table, 0, sizeof(table));
	key_buf->clear();
	kbstat = kbdata = kbint = kbmsk = 0;
}

void KEYBOARD::write_io8(uint32 addr, uint32 data)
{
	switch(addr)
	{
	case 0x600:
		// data
//		kbstat |= 2;
		break;
	case 0x602:
		// command
		break;
	case 0x604:
		kbmsk = data;
		break;
	}
}

uint32 KEYBOARD::read_io8(uint32 addr)
{
	switch(addr)
	{
	case 0x600:
		kbint &= ~1;
		d_pic->write_signal(did_pic, 0, 0);
		kbstat &= ~1;
		return kbdata;
	case 0x602:
		return kbstat;
	case 0x604:
		return kbint;
	}
	return 0;
}

void KEYBOARD::event_frame()
{
	if(!(kbstat & 1) && !key_buf->empty()) {
		kbstat |= 1;
		kbdata = key_buf->read();
	}
	if((kbstat & 1) && (kbmsk & 1) && !(kbint & 1)) {
		kbint |= 1;
		d_pic->write_signal(did_pic, 1, 1);
	}
//	kbstat &= ~2;
}

void KEYBOARD::key_down(int code)
{
	if(!table[code]) {
		table[code] = 1;
		if(code = key_table[code]) {
			// $11:CTRL, $10:SHIFT
			key_buf->write(0xa0 | (table[0x11] ? 8 : 0) | (table[0x10] ? 4 : 0));
			key_buf->write(code);
		}
	}
}

void KEYBOARD::key_up(int code)
{
	if(table[code]) {
		table[code] = 0;
		if(code = key_table[code]) {
			key_buf->write(0xb0);
			key_buf->write(code);
		}
	}
}

