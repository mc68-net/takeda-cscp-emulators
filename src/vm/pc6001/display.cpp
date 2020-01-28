/*
	NEC PC-6001 Emulator 'yaPC-6001'

	Author : Takeda.Toshiya
	Date   : 2013.08.22-

	[ display ]
*/

#include "display.h"
#include "timer.h"
#include "../mc6847.h"

void DISPLAY::reset()
{
	vram_ptr = ram_ptr + 0xe000;
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	unsigned int VRAMHead[4] = { 0xc000, 0xe000, 0x8000, 0xa000 };
	uint16_t port=(addr & 0x00ff);
	
	switch (port) {
	case 0xB0:
		vram_ptr = (ram_ptr + VRAMHead[(data & 0x06) >> 1]);
		d_timer->set_portB0(data);
		break;
	}
}

void DISPLAY::draw_screen()
{
	d_vdp->write_signal(SIG_MC6847_AG, *vram_ptr, 0x80);
	d_vdp->write_signal(SIG_MC6847_GM, *vram_ptr >> 4, 1);
	d_vdp->write_signal(SIG_MC6847_GM, *vram_ptr >> 2, 2);
	d_vdp->write_signal(SIG_MC6847_GM, *vram_ptr >> 0, 4);
	d_vdp->write_signal(SIG_MC6847_CSS, *vram_ptr, 0x02);
	if(*vram_ptr & 0x80) {
		d_vdp->set_vram_ptr(vram_ptr + 0x200, 0x1800);
	} else {
		d_vdp->set_vram_ptr(vram_ptr, 0x1800);
	}
	d_vdp->draw_screen();
}

#define STATE_VERSION	1

void DISPLAY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32((int)(vram_ptr - ram_ptr));
}

bool DISPLAY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	vram_ptr = ram_ptr + state_fio->FgetInt32();
	return true;
}
