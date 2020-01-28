/*
	BANDAI RX-78 Emulator 'eRX-78'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ cmt ]
*/

#include "cmt.h"

void CMT::initialize()
{
	// data recorder
	in = out = remote = now_acc = false;
	framecnt = 0;
	
	// regist event to detect the end of access
	vm->regist_frame_event(this);
}

void CMT::write_io8(uint32 addr, uint32 data)
{
	// data recorder
	if(!remote) {
		// motor on
		dev->write_signal(did_rmt, 1, 1);
		remote = true;
	}
	bool signal = (data & 1) ? true : false;
	if(signal != out) {
		dev->write_signal(did_out, signal ? 1 : 0, 1);
		out = signal;
	}
	now_acc = true;
}

uint32 CMT::read_io8(uint32 addr)
{
	if(!remote) {
		// motor on
		dev->write_signal(did_rmt, 1, 1);
		remote = true;
	}
	now_acc = true;
	return in ? 1 : 0;
}

void CMT::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CMT_IN)
		in = (data & mask) ? true : false;
}

void CMT::event_frame()
{
	if(remote) {
		if(now_acc)
			framecnt = 0;
		else if(++framecnt >= FRAMES_PER_SEC) {
			// motor off if not accessed for past 1 sec
			dev->write_signal(did_rmt, 0, 1);
			remote = false;
		}
		now_acc = false;
	}
}