/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.04 -

	[ cmt ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CMT_PIO_PA	0
#define SIG_CMT_PIO_PC	1
#define SIG_CMT_OUT	2
#define SIG_CMT_REMOTE	3
#define SIG_CMT_END	4
#define SIG_CMT_TOP	5

class DATAREC;

class CMT : public DEVICE
{
private:
	DEVICE* d_pio;
	DATAREC *d_drec;
	
	uint8 pa, pc;
	bool play, rec;
	bool now_play, now_rewind;
#ifndef _MZ80B
	bool now_apss;
	int register_id;
#endif
	
	void fast_forwad();
	void fast_rewind();
	void forward();
	void stop();
	
public:
	CMT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMT() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
#ifndef _MZ80B
	void event_callback(int event_id, int err);
#endif
	
	// unique function
	void set_context_pio(DEVICE* device) {
		d_pio = device;
	}
	void set_context_drec(DATAREC* device) {
		d_drec = device;
	}
	void play_tape(bool value);
	void rec_tape(bool value);
	void close_tape();
};

#endif
