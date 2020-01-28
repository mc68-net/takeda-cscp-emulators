/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ cmos ]
*/

#ifndef _CMOS_H_
#define _CMOS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class CMOS : public DEVICE
{
private:
	uint8_t cmos[0x2000];
	bool modified;
	
public:
	CMOS(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMOS() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	uint8_t* get_cmos()
	{
		return cmos;
	}
};

#endif
