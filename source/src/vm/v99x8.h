// from "v99x8.h" of Zodiac

#ifndef _V99X8_H_
#define _V99X8_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifndef VAR
#	define VAR extern
#endif

//#include "../misc/ut.h"
//#include <md.h>


#define V99X8_NREG 48
#define V99X8_NSTAT 10

enum
{
	V99X8_SCREEN_IGN = -1,
	V99X8_SCREEN_0, V99X8_SCREEN_1, V99X8_SCREEN_2, V99X8_SCREEN_3,
	V99X8_SCREEN_4, V99X8_SCREEN_5, V99X8_SCREEN_6, V99X8_SCREEN_7,
	V99X8_SCREEN_8, V99X8_SCREEN_X, V99X8_SCREEN_A, V99X8_SCREEN_C
};

typedef struct
{
	bool f_tms;
	bool f_interleave; /* sc7/8 �ɂ�������ʂȃ}�b�s���O���[�h */

	int xsize;
	int xshift;
} v99x8_screen_mode_t;

typedef struct
{
	uint8 ctrl[V99X8_NREG], status[V99X8_NSTAT];

	int scr;
	v99x8_screen_mode_t mode;

	uint8 col_fg, col_bg;

	uint8 *tbl_pg, *tbl_pn, *tbl_cl;

	int pages;	/* VRAM memory size */
	bool f_zoom;

	uint8 *vram;

	int scanline, n_scanlines; /* �������� scanline �� scanline ���B
	                               ??? �����Ƃ悢�l�[�~���O�H */


/* private */


} v99x8_t;

VAR v99x8_t v99x8;


extern void v99x8_init(void);
extern void v99x8_ctrl(int n, uint8 m);
extern int v99x8_hsync(void);

extern uint8 v99x8_in_0(void);	/* VRAM read */
extern uint8 v99x8_in_1(void);	/* status in */

extern void v99x8_out_0(uint8 n);	/* VRAM write */
extern void v99x8_out_1(uint8 n);	/* ctrl out */
extern void v99x8_out_2(uint8 n);	/* palette out */
extern void v99x8_out_3(uint8 n);	/* ctrl out */


extern void v99x8_pallete_set(uint8 n, uint8 r, uint8 g, uint8 b);
extern void v99x8_refresh_init(void);
extern void v99x8_refresh_screen(void);
extern void v99x8_refresh_clear(void);
extern void v99x8_refresh_sc0(int y, int h);
extern void v99x8_refresh_sc1(int y, int h);
extern void v99x8_refresh_sc2(int y, int h);
extern void v99x8_refresh_sc3(int y, int h);
extern void v99x8_refresh_sc4(int y, int h);
extern void v99x8_refresh_sc5(int y, int h);
extern void v99x8_refresh_sc6(int y, int h);
extern void v99x8_refresh_sc7(int y, int h);
extern void v99x8_refresh_sc8(int y, int h);
extern void v99x8_refresh_sca(int y, int h);
extern void v99x8_refresh_scc(int y, int h);
extern void v99x8_refresh_scx(int y, int h);

extern uint8 vram_read(int addr);
extern void vram_write(int addr, uint8 n);

/*
#define VRAM_ADDR(addr) (v99x8.f_interleave ? \
                          (addr >> 1) | ((addr & 1) << 16) : \
                          addr)
*/





/*
	Skelton for retropc emulator

	Origin : "tms9918a.h"
	Author : umaiboux
	Date   : 2014.12.XX -

	[ V99x8 ]
*/


#include "vm.h"
#include "../emu.h"
#include "device.h"


class V99X8 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_irq;
	
	uint8 vram[1024*128];
	scrntype screen[SCREEN_WIDTH*SCREEN_HEIGHT];
	bool intstat;
	
	void set_intstat(bool val);

	int hsync(int v/*void*/);
	void z80_intreq(int a);
	int md_video_pitch(void);
	uint8 *md_video_lockline(int x, int y, int w, int h);
	void md_video_update(int n, /*md_video_rect_t*/void *rp);
	void md_video_fill(int x, int y, int w, int h, uint32 c);
	void v99x8_refresh_screen(void);
	void v99x8_refresh_clear(void);
	uint8 *v99x8_refresh_start(int x, int w, int h);
	void v99x8_refresh_sc0(int y, int h);
	void v99x8_refresh_sc1(int y, int h);
	void v99x8_refresh_sc2(int y, int h);
	void v99x8_refresh_sc3(int y, int h);
	void v99x8_refresh_sc4(int y, int h);
	void v99x8_refresh_sc5(int y, int h);
	void v99x8_refresh_sc6(int y, int h);
	void v99x8_refresh_sc7(int y, int h);
	void v99x8_refresh_sc8(int y, int h);
	void v99x8_refresh_sca(int y, int h);
	void v99x8_refresh_scc(int y, int h);
	void v99x8_refresh_scx(int y, int h);
public:
	V99X8(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_irq);
	}
	~V99X8() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_vline(int v, int clock);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void draw_screen();
};

#endif