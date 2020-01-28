/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.03.08-

	[ YM2151 ]
*/

#ifndef _YM2151_H_
#define _YM2151_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"
#include "fmgen/opm.h"

#ifdef SUPPORT_WIN32_DLL
#define SUPPORT_MAME_FM_DLL
#include "fmdll/fmdll.h"
#endif

#define SIG_YM2151_MUTE		0

class YM2151 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_irq;
	
	FM::OPM* opm;
#ifdef SUPPORT_MAME_FM_DLL
	CFMDLL* fmdll;
	LPVOID* dllchip;
	struct {
		bool written;
		uint8_t data;
	} port_log[0x100];
#endif
	int base_decibel;
	
	int chip_clock;
	uint8_t ch;
	bool irq_prev, mute;
	
	uint32_t clock_prev;
	uint32_t clock_accum;
	uint32_t clock_const;
	int timer_event_id;
	
	uint32_t clock_busy;
	bool busy;
	
	void update_count();
	void update_event();
	void update_interrupt();
	
public:
	YM2151(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		base_decibel = 0;
	}
	~YM2151() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int error);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("YM2151");
	}
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void initialize_sound(int rate, int clock, int samples, int decibel);
	void set_reg(uint32_t addr, uint32_t data); // for patch
};

#endif
