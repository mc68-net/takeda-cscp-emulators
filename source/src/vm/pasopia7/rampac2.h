/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ ram pac 2 (32kbytes) ]
*/

#ifndef _RAMPAC2_H_
#define _RAMPAC2_H_

#include "../vm.h"
#include "../../emu.h"
#include "pac2dev.h"

static const uint8 header[16] = {
	0xaa, 0x1f, 0x04, 0x00, 0x04, 0x80, 0x00, 0x01, 0x04, 0x04, 0x01, 0x03, 0x08, 0x00, 0x00, 0x00
};

class RAMPAC2 : public PAC2DEV
{
private:
	_TCHAR path[_MAX_PATH];
	uint8 ram[32*1024];
	uint32 ptr;
	
public:
	RAMPAC2(VM* parent_vm, EMU* parent_emu) : PAC2DEV(parent_vm, parent_emu) {}
	~RAMPAC2() {}
	
	// common functions
	void initialize(int id);
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
};

#endif

