/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.03.08-

	[ YM2151 ]
*/

#include "ym2151.h"

void YM2151::initialize()
{
	opm = new FM::OPM;
	usec = (int)(1000000. / FRAMES_PER_SEC / LINES_PER_FRAME + 0.5);
	vm->regist_vline_event(this);
	irq = mute = false;
}

void YM2151::release()
{
	delete opm;
	free(sound_tmp);
}

void YM2151::reset()
{
	opm->Reset();
}

void YM2151::write_io8(uint32 addr, uint32 data)
{
	if(addr & 1) {
		opm->SetReg(ch, data);
	}
	else {
		ch = data;
	}
}

uint32 YM2151::read_io8(uint32 addr)
{
	if(addr & 1) {
		return opm->ReadStatus();
	}
	return 0xff;
}

void YM2151::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_YM2151_MUTE) {
		mute = ((data & mask) != 0);
	}
}

void YM2151::event_vline(int v, int clock)
{
	bool next = opn->Count(usec);
	if(irq != next) {
		for(int i = 0; i < dcount_irq; i++) {
			d_irq[i]->write_signal(did_irq[i], next ? 0xffffffff : 0, dmask_irq[i]);
		}
		irq = next;
	}
}

void YM2151::mix(int32* buffer, int cnt)
{
	if(mute) {
		return;
	}
	_memset(sound_tmp, 0, cnt * 2 * sizeof(int32));
	opm->Mix(sound_tmp, cnt);
	for(int i = 0, j = 0; i < cnt; i++, j += 2) {
		buffer[i] += sound_tmp[j];
	}
}

void YM2151::init(int rate, int clock, int samples, int vol)
{
	opn->Init(clock, rate, false);
	opn->SetVolume(vol);
	sound_tmp = (int32*)malloc(samples * 2 * sizeof(int32));
}

