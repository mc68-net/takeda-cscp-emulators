/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

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
	
	cur_time_t cur_time;
	int register_id;
	
	uint16_t rtcmr, rtdsr, rtadr, rtobr, rtibr;
	uint8_t regs[40];
	
	void read_from_cur_time();
	void write_to_cur_time();
	void update_checksum();
	void update_intr();
public:
	RTC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~RTC() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32_t addr, uint32_t data)
	{
		write_io16(addr, data);
	}
	uint32_t read_io8(uint32_t addr)
	{
		return (uint8_t)read_io16(addr);
	}
	void write_io16(uint32_t addr, uint32_t data);
	uint32_t read_io16(uint32_t addr);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif
