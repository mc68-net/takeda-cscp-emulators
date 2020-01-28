/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2013.03.14-

	[ memory/crtc ]
*/

#ifndef _MEMORY_80B_H_
#define _MEMORY_80B_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_VRAM_SEL	0
#define SIG_CRTC_WIDTH80	1
#define SIG_CRTC_REVERSE	2

class Z80;

class MEMORY : public DEVICE
{
private:
	Z80 *d_cpu;
	DEVICE *d_pio;
	
	// memory
	uint8* rbank[32];
	uint8* wbank[32];
	bool is_vram[32];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8 ram[0x10000];
	uint8 vram[0x10000];
	uint8 tvram[0x1000];
	uint8 ipl[0x800];
	
	bool ipl_selected;
	uint8 vram_sel, vram_page;
	void update_vram_map();
	
	// crtc
	scrntype palette_pc[8];
	uint8 font[0x800];
	uint8 screen_txt[200][640];
	uint8 screen_gra[200][640];
	uint8 back_color, text_color, vram_mask;
	bool width80, reverse;
	bool hblank;
	void update_palette();
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void special_reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	uint32 fetch_op(uint32 addr, int *wait);
#ifndef _MZ80B
	void write_io8(uint32 addr, uint32 data);
#endif
	void write_signal(int id, uint32 data, uint32 mask);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
#ifndef _MZ80B
	void update_config();
#endif
	
	// unique function
	void set_context_cpu(Z80* device) {
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device) {
		d_pio = device;
	}
	void load_dat_image(_TCHAR* file_path);
	void load_mzt_image(_TCHAR* file_path);
	void draw_screen();
};

#endif