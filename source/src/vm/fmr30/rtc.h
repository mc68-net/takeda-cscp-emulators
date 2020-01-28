/*
	FUJITSU FMR-30 Emulator 'eFMR-30'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ rtc ]
*/

#ifndef _RTC_H_
#define _RTC_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class RTC : public DEVICE
{
private:
	DEVICE* d_pic;
	int did_pic;
	
	uint16 rtcmr, rtdsr, rtadr, rtobr, rtibr;
	uint8 regs[40];
	int time[8];
	
	void update_calendar();
	void update_checksum();
	void update_intr();
public:
	RTC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~RTC() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data) {
		write_io16(addr, data);
	}
	uint32 read_io8(uint32 addr) {
		return (uint8)read_io16(addr);
	}
	void write_io16(uint32 addr, uint32 data);
	uint32 read_io16(uint32 addr);
	void event_callback(int event_id, int err);
	
	// unique functions
	void set_context_pic(DEVICE* device, int id) {
		d_pic = device; did_pic = id;
	}
};

#endif

