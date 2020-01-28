/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"
#include "../z80pio.h"
#include "../../fileio.h"

#ifdef _MZ2500
#define MAX_COLUMN 14
#else
#define MAX_COLUMN 12
#endif

static const int key_map[14][8] = {
	{0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77},
	{0x78, 0x79, 0x68, 0x69, 0x6c, 0x6e, 0x6b, 0x6d},
	{0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67},
	{0x09, 0x20, 0x0d, 0x26, 0x28, 0x25, 0x27, 0x13},
	{0xbf, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47},
	{0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f},
	{0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57},
	{0x58, 0x59, 0x5a, 0xde, 0xdc, 0xe2, 0xbe, 0xbc},
	{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37},
	{0x38, 0x39, 0xba, 0xbb, 0xbd, 0xc0, 0xdb, 0x00},
	{0xdd, 0x7b, 0x24, 0x2e, 0x08, 0x1b, 0x6a, 0x6f},
	{0x12, 0x14, 0x10, 0x15, 0x11, 0x00, 0x00, 0x00},
	{0x1d, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x19, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	column = 0;
	register_frame_event(this);
}

void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_KEYBOARD_COLUMN) {
		column = data & mask;	// from z80pio port a
		create_keystat();
	}
}

void KEYBOARD::event_frame()
{
	// update key status
	memset(keys, 0xff, sizeof(keys));
	key_stat[0] = 0;
	for(int i = 0; i < MAX_COLUMN; i++) {
		uint8 tmp = 0;
		for(int j = 0; j < 8; j++) {
			tmp |= (key_stat[key_map[i][j]]) ? 0 : (1 << j);
		}
		keys[i] = tmp;
		keys[0x0f] &= tmp;
	}
	create_keystat();
}

void KEYBOARD::create_keystat()
{
	uint8 val = keys[(column & 0x10) ? (column & 0x0f) : 0x0f];
	d_pio_i->write_signal(SIG_I8255_PORT_B, val, 0x80);	// to i8255 port b
	d_pio->write_signal(SIG_Z80PIO_PORT_B, val, 0xff);	// to z80pio port b
}

#define STATE_VERSION	1

void KEYBOARD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(column);
}

bool KEYBOARD::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	column = state_fio->FgetUint8();
	return true;
}

