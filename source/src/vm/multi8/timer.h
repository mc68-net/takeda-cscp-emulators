/*
	MITSUBISHI Elec. MULTI8 Emulator 'EmuLTI8'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16 -

	[ clock supply ]
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class TIMER : public DEVICE
{
private:
	DEVICE* dev;
	int dev_id0, dev_id1;
	
	int clocks;
	
public:
	TIMER(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~TIMER() {}
	
	// common functions
	void initialize();
	void event_vsync(int v, int clock);
	
	// unique functions
	void set_context(DEVICE* device, int id0, int id1) { dev = device; dev_id0 = id0; dev_id1 = id1; }
};

#endif

