/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

/*
	[ ^ ] [ F1] [ / ] [ * ] [ - ]
	[ \ ] [ F2] [ 7 ] [ 8 ] [ 9 ]
	[ [ ] [ F3] [ 4 ] [ 5 ] [ 6 ]
	[ ] ] [ F4] [ 1 ] [ 2 ] [ 3 ]
	[ _ ] [ F5] [ F6] [ F7] [ F8]
*/

static const int key_map[10][8] = {
	{0x31, 0x33, 0x35, 0x37, 0x39, 0xbd, 0x70, 0x6a},
	{0x32, 0x34, 0x36, 0x38, 0x30, 0xde, 0x6f, 0x6d},
	{0x51, 0x45, 0x54, 0x55, 0x4f, 0xc0, 0x71, 0x68},
	{0x57, 0x52, 0x59, 0x49, 0x50, 0xdc, 0x67, 0x69},
	{0x41, 0x44, 0x47, 0x4a, 0x4c, 0xba, 0x72, 0x65},
	{0x53, 0x46, 0x48, 0x4b, 0xbb, 0xdb, 0x64, 0x66},
	{0x5a, 0x43, 0x42, 0x4d, 0xbe, 0x14, 0x73, 0x62},
	{0x58, 0x56, 0x4e, 0xbc, 0xbf, 0xdd, 0x61, 0x63},
	{0x10, 0x2e, 0x00, 0x27, 0x0d, 0x00, 0x74, 0x76},
	{0x24, 0x20, 0x28, 0x13, 0x00, 0xe2, 0x75, 0x77},
};

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	column = 0;
	
	// regist event
	vm->regist_frame_event(this);
}

void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
	column = data & 0x0f;
	update_key();
}

void KEYBOARD::event_frame()
{
	update_key();
}

void KEYBOARD::update_key()
{
	uint8 stat = 0xff;
	
	if(column < 10) {
		for(int i = 0; i < 8; i++) {
			if(key_stat[key_map[column][i]]) {
				stat &= ~(1 << i);
			}
		}
	}
	d_pio->write_signal(SIG_I8255_PORT_B, stat, 0xff);
}
