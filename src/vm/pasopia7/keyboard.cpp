/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../z80pio.h"

static const int key_map[12][8] = {
	{ 0x12, 0x10, 0x14, 0x00, 0x11, 0x15, 0x00, 0x00 },
	{ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 },
	{ 0x68, 0x69, 0x6d, 0x6b, 0x6a, 0x6f, 0x6e, 0x0d },
	{ 0x27, 0x7a, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00 },
	{ 0x24, 0xe5, 0x78, 0x79, 0x25, 0x26, 0x00, 0x20 },
	{ 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 },
	{ 0x31, 0x30, 0x34, 0x52, 0x59, 0xe2, 0xde, 0xdc },
	{ 0x32, 0x33, 0x38, 0x54, 0x55, 0x37, 0xc0, 0xdb },
	{ 0xbd, 0x35, 0x36, 0x46, 0x48, 0x39, 0xba, 0xdd },
	{ 0x51, 0x57, 0x45, 0x47, 0x4a, 0x49, 0x4f, 0x50 },
	{ 0x41, 0x53, 0x44, 0x56, 0x4e, 0x4b, 0x4c, 0xbb },
	{ 0x5a, 0x58, 0x43, 0x42, 0x4d, 0xbc, 0xbe, 0xbf }
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	register_frame_event(this);
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_KEYBOARD_Z80PIO_A) {
		// from Z-80PIO Port A
		sel = data & mask;
		create_key();
	}
}

void KEYBOARD::event_frame()
{
	create_key();
}

void KEYBOARD::create_key()
{
	// update port-b
	uint8_t keys[256];
	memcpy(keys, key_stat, sizeof(keys));
	
	keys[0] = 0;
	// backspace -> del
	if(keys[0x08]) keys[0x2e] = 0x80;
	// down -> up + shift
	if(keys[0x28]) keys[0x26] = keys[0x10] = 0x80;
	// inst -> del + shift
	if(keys[0x2d]) keys[0x2e] = keys[0x10] = 0x80;
	// tab -> esc + shift
	if(keys[0x09]) keys[0x1b] = keys[0x10] = 0x80;
	
	uint8_t val = 0;
	for(int i = 0; i < 3; i++) {
		if(sel & (0x10 << i)) {
			for(int j = 0; j < 4; j++) {
				if(sel & (1 << j)) {
					for(int k = 0; k < 8; k++) {
						val |= keys[key_map[i * 4 + j][k]] ? (1 << k) : 0;
					}
				}
			}
		}
	}
	// to Z-80PIO Port B
	d_pio->write_signal(SIG_Z80PIO_PORT_B, ~val, 0xff);
}

#define STATE_VERSION	1

void KEYBOARD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(sel);
}

bool KEYBOARD::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	sel = state_fio->FgetUint8();
	return true;
}
