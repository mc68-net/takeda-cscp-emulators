/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2015.09.04-

	[ MZ-80FIO ]
*/

#include "mz80fio.h"
#include "../t3444a.h"

void MZ80FIO::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xf8:
		d_fdc->write_io8(2, data);	// sector
		break;
	case 0xf9:
		d_fdc->write_io8(1, data);	// track
		break;
	case 0xfa:
		d_fdc->write_io8(0, data >> 4);	// command
		break;
	case 0xfb:
		d_fdc->write_io8(3, data);	// data
		break;
	}
}

uint32 MZ80FIO::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0xf8:
		d_fdc->write_signal(SIG_T3444A_DRIVE, addr, 0x0700);
		d_fdc->write_signal(SIG_T3444A_MOTOR, addr, 0x0800);
		d_fdc->write_signal(SIG_T3444A_TND,   addr, 0x1000);
		break;
	case 0xf9:
		return (d_fdc->read_signal(SIG_T3444A_DRDY) ? 4 : 0) |
		       (d_fdc->read_signal(SIG_T3444A_CRDY) ? 2 : 0) |
		       (d_fdc->read_signal(SIG_T3444A_RQM ) ? 1 : 0);
	case 0xfa:
		return (d_fdc->read_io8(0) & 0x0f) << 4;
	case 0xfb:
		return d_fdc->read_io8(3);		// data
	}
	return 0xff;
}

