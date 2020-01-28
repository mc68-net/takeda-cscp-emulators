/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.04 -

	[ floppy ]
*/

#include "floppy.h"

void FLOPPY::reset()
{
	reverse = laydock = false;
}

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff)
	{
	case 0xdc:
		// drive reg
		if(cpu->get_prv_pc() == 0x5698 && data == 0x86)
			laydock = true;
		if(laydock)
			data &= 0xfc;
		fdc->write_signal(fdc_id0, data, 0xff);
		break;
	case 0xdd:
		// side reg
		fdc->write_signal(fdc_id1, data, 0xff);
		break;
	}
}

void FLOPPY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_FLOPPY_REVERSE)
		reverse = (data & mask) ? true : false;
}

