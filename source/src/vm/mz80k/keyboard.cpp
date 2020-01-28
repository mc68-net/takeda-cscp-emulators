/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

static const int key_map[10][8] = {
#if defined(_MZ80A)
	// MZ-80A
	// 10KEY "00" -> 10KEY "/"
	{0x10, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13},
	{0x5a, 0x00, 0x2e, 0x41, 0x51, 0x57, 0x31, 0x32},
	{0x43, 0x58, 0x53, 0x44, 0x45, 0x52, 0x33, 0x34},
	{0x42, 0x56, 0x46, 0x47, 0x54, 0x59, 0x35, 0x36},
	{0x20, 0x4e, 0x48, 0x4a, 0x55, 0x49, 0x37, 0x38},
	{0x4d, 0xbc, 0x4b, 0x4c, 0x4f, 0x50, 0x39, 0x30},
	{0xbe, 0xbf, 0xbb, 0xba, 0xc0, 0xdb, 0xbd, 0xde},
	{0xe2, 0x00, 0xdd, 0x0d, 0x26, 0x27, 0xdc, 0x24},
	{0x60, 0x6f, 0x61, 0x62, 0x64, 0x65, 0x67, 0x68},
	{0x6e, 0x00, 0x63, 0x00, 0x66, 0x6d, 0x69, 0x6b},
#else
	// MZ-80K , MZ-1200
/*
	[ ^ ] [ F1] [ / ] [ * ] [ - ]
	[ \ ] [ F2] [ 7 ] [ 8 ] [ 9 ]
	[ [ ] [ F3] [ 4 ] [ 5 ] [ 6 ]
	[ ] ] [ F4] [ 1 ] [ 2 ] [ 3 ]
	[ _ ] [ F5] [ F6] [ F7] [ F8]
*/
	{0x31, 0x33, 0x35, 0x37, 0x39, 0xbd, 0x70, 0x6a},
	{0x32, 0x34, 0x36, 0x38, 0x30, 0xde, 0x6f, 0x6d},
	{0x51, 0x45, 0x54, 0x55, 0x4f, 0xc0, 0x71, 0x68},
	{0x57, 0x52, 0x59, 0x49, 0x50, 0xdc, 0x67, 0x69},
	{0x41, 0x44, 0x47, 0x4a, 0x4c, 0xba, 0x72, 0x65},
	{0x53, 0x46, 0x48, 0x4b, 0xbb, 0xdb, 0x64, 0x66},
	{0x5a, 0x43, 0x42, 0x4d, 0xbe, 0x15, 0x73, 0x62},
	{0x58, 0x56, 0x4e, 0xbc, 0xbf, 0xdd, 0x61, 0x63},
	{0x10, 0x2e, 0x00, 0x27, 0x0d, 0x00, 0x74, 0x76},
	{0x24, 0x20, 0x28, 0x13, 0x00, 0xe2, 0x75, 0x77},
#endif
};

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	column = 0;
	kana = false;
	
	// register event
	register_frame_event(this);
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

void KEYBOARD::key_down(int code)
{
#if defined(_MZ80K) || defined(_MZ1200)
	if(code == 0x15) {
		kana = !kana;
		key_stat[0x10] = kana ? 4 : 0;
		key_stat[0x15] = 4;
	}
#endif
}

#define STATE_VERSION	2

void KEYBOARD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(column);
	state_fio->FputBool(kana);
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
	kana = state_fio->FgetBool();
	return true;
}

