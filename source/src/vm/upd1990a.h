/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.19-

	[ uPD1990A / uPD4990A ]
*/

#ifndef _UPD1990A_H_
#define _UPD1990A_H_

#define SIG_UPD1990A_CLK	0
#define SIG_UPD1990A_STB	1
#define SIG_UPD1990A_CMD	2
#define SIG_UPD1990A_C0		3
#define SIG_UPD1990A_C1		4
#define SIG_UPD1990A_C2		5
#define SIG_UPD1990A_DIN	6

#include "vm.h"
#include "../emu.h"
#include "device.h"

class UPD1990A : public DEVICE
{
private:
	// output signals
	outputs_t outputs_dout;
	outputs_t outputs_tp;
	
	cur_time_t cur_time;
	uint8 cmd, mode, tpmode;
	uint64 shift_data;
#ifdef HAS_UPD4990A
	uint32 shift_cmd;
#endif
	bool clk, stb, din, tp;
	int register_id;
	
public:
	UPD1990A(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		init_output_signals(&outputs_dout);
		init_output_signals(&outputs_tp);
		
		cmd = mode = 0;
		tpmode = 5;
		clk = stb = din = tp = true;
	}
	~UPD1990A() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int ch) {
		return (uint32)(shift_data & 1);
	}
	void event_callback(int event_id, int err);
	
	// unique functions
	void set_context_dout(DEVICE* device, int id, uint32 mask) {
		register_output_signal(&outputs_dout, device, id, mask);
	}
	void set_context_tp(DEVICE* device, int id, uint32 mask) {
		register_output_signal(&outputs_tp, device, id, mask);
	}
};

#endif

