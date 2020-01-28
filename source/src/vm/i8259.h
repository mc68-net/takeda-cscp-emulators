/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2005.06.10-

	[ i8259 ]
*/

#ifndef _I8259_H_
#define _I8259_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8259_IR0	0
#define SIG_I8259_IR1	1
#define SIG_I8259_IR2	2
#define SIG_I8259_IR3	3
#define SIG_I8259_IR4	4
#define SIG_I8259_IR5	5
#define SIG_I8259_IR6	6
#define SIG_I8259_IR7	7
#define SIG_I8259_CHIP0	0
#define SIG_I8259_CHIP1	8
#define SIG_I8259_CHIP2	16
#define SIG_I8259_CHIP3	24

class I8259 : public DEVICE
{
private:
	DEVICE* dev;
	
	typedef struct {
		uint8 imr, isr, irr, prio;
		uint8 icw1, icw2, icw3, icw4;
		uint8 icw2_r, icw3_r, icw4_r;
		uint8 special, input;
	} pic_t;
	pic_t pic[I8259_MAX_CHIPS];
	void do_interrupt();
	
public:
	I8259(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~I8259() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void do_ei() {
		do_interrupt();
	}
	
	// unique functions
	void set_context(DEVICE* device) {
		dev = device;
	}
};

#endif

