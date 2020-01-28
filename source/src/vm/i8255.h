/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.14 -

	[ i8255 ]
*/

#ifndef _I8255_H_
#define _I8255_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8255_PORT_A	0
#define SIG_I8255_PORT_B	1
#define SIG_I8255_PORT_C	2

class I8255 : public DEVICE
{
private:
	DEVICE* dev[3][MAX_OUTPUT];
	int dev_id[3][MAX_OUTPUT], dev_shift[3][MAX_OUTPUT], dev_cnt[3];
	uint32 dev_mask[3][MAX_OUTPUT];
	
	typedef struct {
		uint8 wreg;
		uint8 rreg;
		uint8 rmask;
		bool first;
	} port_t;
	port_t port[3];
	
public:
	I8255(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		dev_cnt[0] = dev_cnt[1] = dev_cnt[2] = 0;
		port[0].wreg = port[1].wreg = port[2].wreg = port[0].rreg = port[1].rreg = port[2].rreg = 0;//0xff;
	}
	~I8255() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32 mask, int shift) {
		int c = dev_cnt[0]++;
		dev[0][c] = device; dev_id[0][c] = id; dev_mask[0][c] = mask; dev_shift[0][c] = shift;
	}
	void set_context_port_b(DEVICE* device, int id, uint32 mask, int shift) {
		int c = dev_cnt[1]++;
		dev[1][c] = device; dev_id[1][c] = id; dev_mask[1][c] = mask; dev_shift[1][c] = shift;
	}
	void set_context_port_c(DEVICE* device, int id, uint32 mask, int shift) {
		int c = dev_cnt[2]++;
		dev[2][c] = device; dev_id[2][c] = id; dev_mask[2][c] = mask; dev_shift[2][c] = shift;
	}
};

#endif

