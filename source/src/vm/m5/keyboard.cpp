/*
	SORD m5 Emulator 'Emu5'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ keyboard ]
*/

#include "keyboard.h"

static const int key_map[7][8] = {
	// back-space (0x08): reset/halt key
	{0x11, 0x09, 0x10, 0x10, 0x00, 0x00, 0x20, 0x0d},
	{0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38},
	{0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49},
	{0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b},
	{0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0xbc},
	{0x39, 0x30, 0xbd, 0xde, 0xbe, 0xbf, 0xe2, 0xdc},
	{0x4f, 0x50, 0xc0, 0xdb, 0x4c, 0xbb, 0xba, 0xdd}
};

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	joy_stat = emu->joy_buffer();
}

uint32 KEYBOARD::read_io8(uint32 addr)
{
	uint32 val = 0;
	
	switch(addr & 0xff) {
	case 0x30:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
		for(int i = 0; i < 8; i++) {
			val |= key_stat[key_map[addr & 0xf][i]] ? (1 << i) : 0;
		}
		return val;
	case 0x31:
		for(int i = 0; i < 8; i++) {
			val |= key_stat[key_map[1][i]] ? (1 << i) : 0;
		}
		val |= (joy_stat[0] & 0x10) ? 0x01 : 0;
		val |= (joy_stat[0] & 0x20) ? 0x02 : 0;
		val |= (joy_stat[1] & 0x10) ? 0x10 : 0;
		val |= (joy_stat[1] & 0x20) ? 0x20 : 0;
		return val;
	case 0x37:
		val |= (joy_stat[0] & 0x08) ? 0x01 : 0;
		val |= (joy_stat[0] & 0x01) ? 0x02 : 0;
		val |= (joy_stat[0] & 0x04) ? 0x04 : 0;
		val |= (joy_stat[0] & 0x02) ? 0x08 : 0;
		val |= (joy_stat[1] & 0x08) ? 0x10 : 0;
		val |= (joy_stat[1] & 0x01) ? 0x20 : 0;
		val |= (joy_stat[1] & 0x04) ? 0x40 : 0;
		val |= (joy_stat[1] & 0x02) ? 0x80 : 0;
		return val;
	}
	return 0xff;
}

