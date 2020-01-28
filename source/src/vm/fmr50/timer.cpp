/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ timer ]
*/

#include "timer.h"
#include "../i8259.h"
#include "../pcm1bit.h"

void TIMER::initialize()
{
	ctrl = 0;
	tmout0 = tmout1 = false;
}

void TIMER::write_io8(uint32 addr, uint32 data)
{
	// $60: interrupt ctrl register
	if(data & 0x80) {
		tmout0 = false;
	}
	ctrl = data;
	update_intr();
	d_pcm->write_signal(SIG_PCM1BIT_ON, data, 4);
}

uint32 TIMER::read_io8(uint32 addr)
{
	// $60: interrupt cause register
	return (tmout0 ? 1 : 0) | (tmout1 ? 2 : 0) | ((ctrl & 7) << 2) | 0xe0;
}

void TIMER::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_TIMER_CH0) {
		if(data & mask) {
			tmout0 = true;
		}
		update_intr();
	}
	else if(id == SIG_TIMER_CH1) {
		tmout1 = ((data & mask) != 0);
		update_intr();
	}
}

void TIMER::update_intr()
{
	d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR0, (tmout0 && (ctrl & 1)) || (tmout1 && (ctrl & 2)) ? 1 : 0, 1);
}

