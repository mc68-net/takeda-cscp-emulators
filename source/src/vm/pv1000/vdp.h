/*
	CASIO PV-1000 Emulator 'ePV-1000'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.11.16 -

	[ video processor ]
*/

#ifndef _VDP_H_
#define _VDP_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#ifdef _WIN32_WCE
// RGB565
#define RGB_COLOR(r, g, b) (uint16)(((uint16)(r) << 11) | ((uint16)(g) << 6) | (uint16)(b))
#else
// RGB555
#define RGB_COLOR(r, g, b) (uint16)(((uint16)(r) << 10) | ((uint16)(g) << 5) | (uint16)(b))
#endif

static const uint16 palette_pc[8] = {
	RGB_COLOR( 0, 0, 0), RGB_COLOR(31, 0, 0), RGB_COLOR( 0,31, 0), RGB_COLOR(31,31, 0),
	RGB_COLOR( 0, 0,31), RGB_COLOR(31, 0,31), RGB_COLOR( 0,31,31), RGB_COLOR(31,31,31)
};
static const uint8 plane[4] = {0, 1, 2, 4};

class VDP : public DEVICE
{
private:
	DEVICE* dev;
	
	uint8 bg[192][256];
	uint8* vram;
	uint8* pcg;
	uint8* pattern;
	uint8* base;
	
	void draw_pattern(int x8, int y8, uint16 top);
	void draw_pcg(int x8, int y8, uint16 top);
	
public:
	VDP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~VDP() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	void event_vsync(int v, int clock);
	
	// unique function
	void set_context(DEVICE* device) {
		dev = device;
	}
	void set_memory_ptr(uint8* ptr) {
		base = ptr;
		vram = ptr + 0xb800;
		pcg = ptr + 0xbc00;
		pattern = ptr;
	}
	void draw_screen();
};

#endif
