/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.02.09 -

	[ 1bit PCM ]
*/

#ifndef _PCM1BIT_H_
#define _PCM1BIT_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_PCM1BIT_SIGNAL	0
#define SIG_PCM1BIT_ON		1
#define SIG_PCM1BIT_MUTE	2

class PCM1BIT : public DEVICE
{
private:
	bool signal, on, mute;
	int changed;
	uint32_t prev_clock;
	int positive_clocks, negative_clocks;
	int max_vol, last_vol_l, last_vol_r;
	int volume_l, volume_r;
	
public:
	PCM1BIT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
	}
	~PCM1BIT() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("1-Bit PCM");
	}
	
	// unique function
	void initialize_sound(int rate, int volume);
};

#endif
