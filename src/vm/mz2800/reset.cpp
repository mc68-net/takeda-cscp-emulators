/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ reset ]
*/

#include "reset.h"

void RESET::initialize()
{
	prev = 0xff;
}

void RESET::write_signal(int id, uint32_t data, uint32_t mask)
{
	// from i8255 port c
	if(!(prev & 2) && (data & 2)) {
		vm->cpu_reset();
	}
	if(!(prev & 8) && (data & 8)) {
		vm->reset();
	}
	prev = data & mask;
}

#define STATE_VERSION	1

void RESET::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(prev);
}

bool RESET::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	prev = state_fio->FgetUint8();
	return true;
}
