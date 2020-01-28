/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.19-

	[ 74LS244 / 74LS245 ]
*/

#ifndef _LS244_H_
#define _LS244_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_LS244_INPUT	0

class LS244 : public DEVICE
{
private:
	// output signals
	outputs_t outputs;
	
	uint8_t din;
	
public:
	LS244(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs);
	}
	~LS244() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("74LS244");
	}
	
	// unique function
	void set_context_output(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&outputs, device, id, mask, shift);
	}
};

#endif
