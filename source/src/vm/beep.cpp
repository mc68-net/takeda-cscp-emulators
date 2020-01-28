/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.22 -

	[ beep ]
*/

#include "beep.h"
#include "../fileio.h"

void BEEP::reset()
{
	signal = true;
	count = 0;
	on = mute = false;
}

void BEEP::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_BEEP_ON) {
		on = ((data & mask) != 0);
	}
	else if(id == SIG_BEEP_MUTE) {
		mute = ((data & mask) != 0);
	}
}

void BEEP::mix(int32* buffer, int cnt)
{
	if(on && !mute) {
		for(int i = 0; i < cnt; i++) {
			if((count -= 1024) < 0) {
				count += diff;
				signal = !signal;
			}
			*buffer++ += signal ? gen_vol : -gen_vol; // L
			*buffer++ += signal ? gen_vol : -gen_vol; // R
		}
	}
}

void BEEP::init(int rate, double frequency, int volume)
{
	gen_rate = rate;
	gen_vol = volume;
	set_frequency(frequency);
}

void BEEP::set_frequency(double frequency)
{
	diff = (int)(1024.0 * gen_rate / frequency / 2.0 + 0.5);
}

#define STATE_VERSION	1

void BEEP::save_state(FILEIO* fio)
{
	fio->FputUint32(STATE_VERSION);
	fio->FputInt32(this_device_id);
	
	fio->FputBool(signal);
	fio->FputInt32(count);
	fio->FputBool(on);
	fio->FputBool(mute);
}

bool BEEP::load_state(FILEIO* fio)
{
	if(fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(fio->FgetInt32() != this_device_id) {
		return false;
	}
	signal = fio->FgetBool();
	count = fio->FgetInt32();
	on = fio->FgetBool();
	mute = fio->FgetBool();
	return true;
}

