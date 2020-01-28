/*
	SHARP X1twin Emulator 'eX1twin'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ floppy ]
*/

#include "floppy.h"
#include "../mb8877.h"

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	// $ffc
	d_fdc->write_signal(SIG_MB8877_DRIVEREG, data, 0x03);
	d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 0x10);
	// NOTE: motor seems to be on automatically when fdc command is requested,
	// so motor is always on temporary
//	d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x80);
}

