/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.28 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_Z80PIO_A	0

static const int key_map[12][8] = {
	{ 0x12, 0x10, 0x14, 0x00, 0x11, 0x15, 0x00, 0x00 },
	{ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 },
	{ 0x68, 0x69, 0x6d, 0x28, 0x26, 0x24, 0x6e, 0x0d },
	{ 0x27, 0x1b, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00 },
	{ 0x7a, 0xe5, 0x78, 0x79, 0x2d, 0x25, 0x00, 0x20 },
	{ 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 },
	{ 0x31, 0x30, 0x34, 0x52, 0x59, 0xe2, 0xde, 0xdc },
	{ 0x32, 0x33, 0x38, 0x54, 0x55, 0x37, 0xc0, 0xdb },
	{ 0xbd, 0x35, 0x36, 0x46, 0x48, 0x39, 0xba, 0xdd },
	{ 0x51, 0x57, 0x45, 0x47, 0x4a, 0x49, 0x4f, 0x50 },
	{ 0x41, 0x53, 0x44, 0x56, 0x4e, 0x4b, 0x4c, 0xbb },
	{ 0x5a, 0x58, 0x43, 0x42, 0x4d, 0xbc, 0xbe, 0xbf }
};

class KEYBOARD : public DEVICE
{
private:
	DEVICE* dev;
	int did;
	
	uint8* key_stat;
	void create_key();
	uint8 sel, keys[12];
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unique functions
	void set_context(DEVICE* device, int id) {
		dev = device; did = id;
	}
};

#endif

