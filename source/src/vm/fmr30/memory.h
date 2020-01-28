/*
	FUJITSU FMR-30 Emulator 'eFMR-30'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

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
	0xB8,0x00,0x7F,			// mov	ax,7F00
	0x8E,0xD0,			// mov	ss,ax
	0xBC,0x64,0x0F,			// mov	sp,0F64
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
	DEVICE *d_cpu, *d_fdc, *d_bios, *d_dma;
	int did_a20, did_dma;
	
	uint8* rbank[4096];	// 16MB / 4KB
	uint8* wbank[4096];
	uint8 wdmy[0x1000];
	uint8 rdmy[0x1000];
	
	uint8 ram[0x100000];	// RAM 1MB
	uint8 vram[0x20000];	// VRAM 32KB * 4pl
	uint8 cvram[0x1000];
	uint8 kvram[0x1000];
	
	uint8 ipl[0x10000];	// IPL 64KB
	uint8 kanji16[0x40000];	// KANJI(16x16) 256KB
	
	uint8 mcr1, mcr2, a20;
	uint8 lcdadr, lcdreg[32];
	uint16 dcr1, dcr2;
	int kj_h, kj_l, kj_ofs, kj_row;
	
	int blinkcnt;
	uint8 screen_txt[400][648];
	uint8 screen_cg[400][640];
	
	void update_bank();
	void draw_text40();
	void draw_text80();
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
	void set_context_dma(DEVICE* device, int id) {
		d_dma = device; did_dma = id;
	}
	uint8* get_vram() {
		return vram;
	}
	uint8* get_cvram() {
		return cvram;
	}
	uint8* get_kvram() {
		return kvram;
	}
	void draw_screen();
};

#endif
