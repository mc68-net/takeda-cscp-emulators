/*
	SEGA SC-3000 Emulator 'eSC-3000'

	Author : Takeda.Toshiya
	Date   : 2010.08.17-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_COLUMN	0

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pio;
	
	const uint8_t* key_stat;
	const uint32_t* joy_stat;
	uint8_t column;
	bool break_pressed;
	
	void update_keyboard();
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void event_frame();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif