/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ system port ]
*/

#include "sysport.h"

void SYSPORT::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0x3f0)
	{
	case 0x70:
		// port-c
		if(data & 8)
			d_fdc->reset();
		break;
	case 0x260:
		// z80ctc reti
		if(data == 0x4d)
			d_ctc->intr_reti();
		break;
	}
}

uint32 SYSPORT::read_io8(uint32 addr)
{
	switch(addr & 0x3ff)
	{
	case 0x60:
		// port-a
		return 0xff;
	case 0x240:
		// z80ctc vector
		return d_ctc->intr_ack();
	case 0x250:
		// z80sio vector
		return d_sio->intr_ack();
	case 0x270:
		// port-b
		return 0xff;
	}
	return 0xff;
}

