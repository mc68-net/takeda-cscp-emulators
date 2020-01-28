/*
	EPOCH Super Cassette Vision Emulator 'eSCV'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ EPOCH TV-1 ]
*/

#ifndef _VDP_H_
#define _VDP_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

static const uint16 palette_pc[16] = {
#if 1
	RGB_COLOR( 0,11,19), RGB_COLOR( 0, 0, 0), RGB_COLOR( 7,18,31), RGB_COLOR( 0, 0,31),
	RGB_COLOR( 2,26, 0), RGB_COLOR( 8,31, 2), RGB_COLOR(15,28,24), RGB_COLOR( 0,21, 0),
	RGB_COLOR(31, 5,18), RGB_COLOR(31, 6, 2), RGB_COLOR(31, 7,31), RGB_COLOR(29,19,31),
	RGB_COLOR(31,25, 4), RGB_COLOR( 9,15, 2), RGB_COLOR(20,18,20), RGB_COLOR(31,31,31)
#else
	RGB_COLOR( 0,11,19), RGB_COLOR( 0, 0, 0), RGB_COLOR( 0, 7,31), RGB_COLOR( 0, 0,31),
	RGB_COLOR( 0,31, 0), RGB_COLOR( 7,31,11), RGB_COLOR( 0,31,31), RGB_COLOR( 0,31, 0),
	RGB_COLOR(31, 7,19), RGB_COLOR(31,19,19), RGB_COLOR(31, 7,31), RGB_COLOR(31,19,31),
	RGB_COLOR(31,31,11), RGB_COLOR(15,19, 0), RGB_COLOR(23,23,23), RGB_COLOR(31,31,31)
#endif
};

#if 1
// table analyzed by Enri
static uint8 color_pair0[16] = {0x0, 0xf, 0xc, 0xd, 0xa, 0xb, 0x8, 0x9, 0x6, 0x7, 0x4, 0x5, 0x2, 0x3, 0x1, 0x1};
static uint8 color_pair1[16] = {0x0, 0x1, 0x8, 0xb, 0x2, 0x3, 0xa, 0x9, 0x4, 0x5, 0xc, 0xd, 0x6, 0x7, 0xe, 0xf};
#else
static uint8 color_pair0[16] = {0xe, 0xf, 0xc, 0xd, 0xa, 0xb, 0x8, 0x9, 0x6, 0x7, 0x4, 0x5, 0x2, 0x3, 0x0, 0x1};
static uint8 color_pair1[16] = {0x0, 0x1, 0x8, 0x9, 0x2, 0x3, 0xa, 0xb, 0x4, 0x5, 0xc, 0xd, 0x6, 0x7, 0xe, 0xf};
#endif

static const uint8 symbol[32][8] = {
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},{0x00,0x02,0x7a,0x84,0x84,0x7a,0x00,0x00},
	{0x00,0x3c,0x42,0x7c,0x42,0x7c,0x40,0x40},{0x00,0x62,0x94,0x08,0x08,0x08,0x00,0x00},
	{0x00,0xfc,0x50,0x50,0x52,0x8c,0x00,0x00},{0xfe,0x82,0x60,0x10,0x60,0x82,0xfe,0x00},
	{0x38,0x44,0x82,0x82,0x82,0x44,0xc6,0x00},{0x10,0x7c,0xfe,0xfe,0xfe,0x10,0x38,0x00},
	{0x6c,0xfe,0xfe,0xfe,0xfe,0x7c,0x10,0x00},{0x10,0x10,0x38,0xfe,0x38,0x10,0x10,0x00},
	{0x10,0x7c,0x92,0xfe,0x92,0x10,0x38,0x00},{0x38,0x44,0xba,0xa2,0xba,0x44,0x38,0x00},
	{0x80,0x80,0x88,0x44,0x3e,0x04,0x08,0x00},{0x02,0x02,0x22,0x44,0xf8,0x40,0x20,0x00},
	{0x10,0x00,0x00,0xfe,0x00,0x00,0x10,0x00},{0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x00},
	{0x7c,0x82,0x82,0x82,0x82,0x82,0x7c,0x00},{0x7c,0xfe,0xfe,0xfe,0xfe,0xfe,0x7c,0x00},
	{0x7c,0x82,0xba,0xba,0xba,0x82,0x7c,0x00},{0xfe,0x82,0x82,0x82,0x82,0x82,0xfe,0x00},
	{0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55},{0xff,0x80,0x80,0x80,0x80,0x80,0x80,0x80},
	{0xff,0x01,0x01,0x01,0x01,0x01,0x01,0x01},{0x80,0x80,0x80,0x80,0x80,0x80,0x80,0xff},
	{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0xff},{0xfe,0x00,0xfe,0x10,0x10,0x10,0x10,0x00},
	{0x18,0x14,0x12,0x12,0x72,0xf0,0x60,0x00},{0x18,0x14,0x12,0x12,0x72,0x90,0x60,0x00},
	{0x10,0x08,0x04,0xfe,0x04,0x08,0x10,0x00},{0x10,0x20,0x40,0xfe,0x40,0x20,0x10,0x00},
	{0x10,0x38,0x54,0x92,0x10,0x10,0x10,0x00},{0x10,0x10,0x10,0x92,0x54,0x38,0x10,0x00}
};

class VDP : public DEVICE
{
private:
	DEVICE* dev;
	
	uint8* vram0;
	uint8* vram1;
	uint8* font_ptr;
	uint8 text[320][320];
	uint8 sprite[320][320];
	uint8 font[128][8];
	uint8 vdc0, vdc1, vdc2, vdc3;
	
	void draw_text_screen();
	void draw_text(int dx, int dy, uint8 data, uint8 tcol, uint8 bcol);
	void draw_block(int dx, int dy, uint8 data);
	void draw_graph(int dx, int dy, uint8 data, uint8 col);
	
	void draw_sprite_screen();
	void draw_sprite(int dx, int dy, int sx, int ex, int sy, int ey, int no, uint8 col);
	
public:
	VDP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~VDP() {}
	
	// common functions
	void initialize();
	void event_vsync(int v, int clock);
	
	// unique function
	void set_context(DEVICE* device) {
		dev = device;
	}
	void set_font_ptr(uint8* ptr) {
		font_ptr = ptr;
	}
	void set_vram_ptr(uint8* ptr) {
		vram0 = ptr; vram1 = ptr + 0x1000;
	}
	void draw_screen();
};

#endif
