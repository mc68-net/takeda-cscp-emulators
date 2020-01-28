/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.29 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_DISP		0
#define SIG_MEMORY_VSYNC	1

static uint8 bios1[] = {
	0xFA,				// cli
	0xDB,0xE3,			// fninit
	0xB8,0xA0,0xF7,			// mov	ax,F7A0
	0x8E,0xD0,			// mov	ss,ax
	0xBC,0x7E,0x05,			// mov	sp,057E
	// init i/o
	0xB4,0x80,			// mov	ah,80
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	// boot from fdd
	0xB4,0x81,			// mov	ah,81
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	0x73,0x0B,			// jnb	$+11
	0x74,0xF5,			// jz	$-11
	// boot from scsi-hdd
	0xB4,0x82,			// mov	ah,82
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	0x72,0xEC,			// jb	$-20
	// goto ipl
	0x9A,0x04,0x00,0x00,0xB0,	// call	far B000:0004
	0xEB,0xE7			// jmp $-25
};

static uint8 bios2[] = {
	0xEA,0x00,0x00,0x00,0xFC,	// jmp	FC00:0000
	0x00,0x00,0x00,
	0xcf				// iret
};

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu, *d_crtc, *d_fdc, *d_bios;
	int did_a20;
	
	uint8* rbank[8192];	// 16MB / 2KB
	uint8* wbank[8192];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	
	uint8 ram[0x200000];	// RAM 1+1MB
#ifdef _FMR60
	uint8 vram[0x80000];	// VRAM 512KB
	uint8 cvram[0x2000];
	uint8 avram[0x2000];
#else
	uint8 vram[0x40000];	// VRAM 256KB
	uint8 cvram[0x1000];
	uint8 kvram[0x1000];
	uint8 dummy[0x8000];	// dummy plane
#endif
	
	uint8 ipl[0x4000];	// IPL 16KB
#ifdef _FMR60
	uint8 ank24[0x3000];		// ANK(14x24)
	uint8 kanji24[0x240000];	// KANJI(24x24)
#else
	uint8 ank8[0x800];	// ANK(8x8) 2KB
	uint8 ank16[0x1000];	// ANK(8x16) 4KB
	uint8 kanji16[0x40000];	// KANJI(16x16) 256KB
#endif
	uint8 id[2];		// MACHINE ID
	
	// memory
	uint8 protect, rst;
	uint8 mainmem, rplane, wplane;
	uint32 amask;
	
	// crtc
	uint8* chreg;
	bool disp, vsync;
	int blink;
	
	// video
	uint8 apal[16][3], apalsel, dpal[8];
	uint8 outctrl;
	
#ifdef _FMR50
	// 16bit card
	uint8 pagesel, ankcg;
	uint8 dispctrl;
	uint8 mix;
	uint16 accaddr, dispaddr;
	
	// kanji
	int kj_h, kj_l, kj_ofs, kj_row;
	
	// logical operation
	uint8 cmdreg, imgcol, maskreg, compreg[8], compbit, bankdis, tilereg[3];
	uint16 lofs, lsty, lsx, lsy, lex, ley;
	void point(int x, int y, int col);
	void line();
#endif
	
	uint8 screen_txt[SCREEN_HEIGHT][SCREEN_WIDTH + 14];
	uint8 screen_cg[SCREEN_HEIGHT][SCREEN_WIDTH];
//	uint8 screen_txt[400][648];
//	uint8 screen_cg[400][640];
	scrntype palette_txt[16];
	scrntype palette_cg[16];
	
	void update_bank();
#ifdef _FMR60
	void draw_text();
#else
	void draw_text40();
	void draw_text80();
#endif
	void draw_cg();
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_dma8(uint32 addr, uint32 data);
	uint32 read_dma8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unitque function
	void set_context_cpu(DEVICE* device, int id) {
		d_cpu = device; did_a20 = id;
	}
	void set_context_fdc(DEVICE* device) {
		d_fdc = device;
	}
	void set_context_bios(DEVICE* device) {
		d_bios = device;
	}
	void set_context_crtc(DEVICE* device) {
		d_crtc = device;
	}
	void set_chregs_ptr(uint8* ptr) {
		chreg = ptr;
	}
	uint8* get_vram() {
		return vram;
	}
	uint8* get_cvram() {
		return cvram;
	}
#ifdef _FMR60
	uint8* get_avram() {
		return avram;
	}
#else
	uint8* get_kvram() {
		return kvram;
	}
#endif
	void draw_screen();
};

#endif

