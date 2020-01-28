/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2013.12.31-

	[ printer ]
*/

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_PRINTER_OUT	0
#define SIG_PRINTER_STB	1

class PRINTER : public DEVICE
{
private:
	
public:
	PRINTER(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PRINTER() {}
	
	// common function
	void write_signal(int id, uint32 data, uint32 mask);
};

#endif

