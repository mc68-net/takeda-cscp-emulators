/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ mc6847 ]
*/

#ifndef _MC6847_H_
#define _MC6847_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_MC6847_AG		0
#define SIG_MC6847_AS		1
#define SIG_MC6847_INTEXT	2
#define SIG_MC6847_GM		3
#define SIG_MC6847_CSS		4
#define SIG_MC6847_INV		5

class MC6847 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_vsync;
	outputs_t outputs_hsync;
	
	uint8 extfont[256 * 16];
	uint8 sg4[16 * 12];
	uint8 sg6[64 * 12];
	uint8 screen[192][256];
	uint8 *vram_ptr;
	int vram_size;
	scrntype palette_pc[16];
	
	bool ag, as;
	bool intext;
	uint8 gm;
	bool css, inv;
	
	bool vsync, hsync;
	int tWHS;
	
	void set_vsync(bool val);
	void set_hsync(bool val);
	void draw_cg(int xofs, int yofs);
	void draw_rg(int xofs, int yofs);
	void draw_alpha();
	
public:
	MC6847(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		ag = as = intext = css = inv = false;
		gm = 0;
		init_output_signals(&outputs_vsync);
		init_output_signals(&outputs_hsync);
	}
	~MC6847() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	
	// unique functions
	void set_context_vsync(DEVICE* device, int id, uint32 mask) {
		regist_output_signal(&outputs_vsync, device, id, mask);
	}
	void set_context_hsync(DEVICE* device, int id, uint32 mask) {
		regist_output_signal(&outputs_hsync, device, id, mask);
	}
	void set_vram_ptr(uint8* ptr, int size) {
		vram_ptr = ptr; vram_size = size;
	}
	void draw_screen();
};

#endif

