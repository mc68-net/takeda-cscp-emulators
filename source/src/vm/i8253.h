/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.14 -

	[ i8253/i8254 ]
*/

#ifndef _I8253_H_
#define _I8253_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8253_CLOCK_0	0
#define SIG_I8253_CLOCK_1	1
#define SIG_I8253_CLOCK_2	2
#define SIG_I8253_GATE_0	3
#define SIG_I8253_GATE_1	4
#define SIG_I8253_GATE_2	5

class I8253 : public DEVICE
{
private:
	typedef struct {
		bool prev_out;
		bool prev_in;
		bool gate;
		int32 count;
		uint16 latch;
		uint16 count_reg;
		uint8 ctrl_reg;
		bool count_latched;
		bool low_read, high_read;
		bool low_write, high_write;
		int mode;
		bool delay;
		bool start;
#ifdef HAS_I8254
		bool null_count;
		bool status_latched;
		uint8 status;
#endif
		// constant clock
		uint32 freq;
		int regist_id;
		uint32 input_clk;
		int period;
		uint32 prev_clk;
	} counter_t;
	counter_t counter[3];
	
	DEVICE* dev[3][MAX_OUTPUT];
	int did[3][MAX_OUTPUT], dcount[3];
	uint32 dmask[3][MAX_OUTPUT];
	
	void input_clock(int ch, int clock);
	void input_gate(int ch, bool signal);
	void start_count(int ch);
	void stop_count(int ch);
	void latch_count(int ch);
	void set_signal(int ch, bool signal);
	int get_next_count(int ch);
	
public:
	I8253(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		dcount[0] = dcount[1] = dcount[2] = 0;
		counter[0].freq = counter[1].freq = counter[2].freq = 0;
	}
	~I8253() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_ch0(DEVICE* device, int id, uint32 mask) {
		int c = dcount[0]++;
		dev[0][c] = device; did[0][c] = id; dmask[0][c] = mask;
	}
	void set_context_ch1(DEVICE* device, int id, uint32 mask) {
		int c = dcount[1]++;
		dev[1][c] = device; did[1][c] = id; dmask[0][c] = mask;
	}
	void set_context_ch2(DEVICE* device, int id, uint32 mask) {
		int c = dcount[2]++;
		dev[2][c] = device; did[2][c] = id; dmask[0][c] = mask;
	}
	void set_constant_clock(int ch, uint32 hz) {
		counter[ch].freq = hz;
	}
};

#endif

