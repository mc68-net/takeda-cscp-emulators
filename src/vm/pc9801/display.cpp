/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ display ]
*/

#include "display.h"
#include "../i8259.h"
#include "../upd7220.h"

#if !defined(SUPPORT_HIRESO)
	#define TVRAM_ADDRESS		0xa0000
	#define VRAM_PLANE_SIZE		0x08000
	#define VRAM_PLANE_ADDR_MASK	0x07fff
	#define VRAM_PLANE_ADDR_0	0x08000
	#define VRAM_PLANE_ADDR_1	0x10000
	#define VRAM_PLANE_ADDR_2	0x18000
	#define VRAM_PLANE_ADDR_3	0x00000
#else
	#define TVRAM_ADDRESS		0xe0000
	#define VRAM_PLANE_SIZE		0x20000
	#define VRAM_PLANE_ADDR_MASK	0x1ffff
	#define VRAM_PLANE_ADDR_0	0x00000
	#define VRAM_PLANE_ADDR_1	0x20000
	#define VRAM_PLANE_ADDR_2	0x40000
	#define VRAM_PLANE_ADDR_3	0x60000
#endif

#define SCROLL_PL	0
#define SCROLL_BL	1
#define SCROLL_CL	2
#define SCROLL_SSL	3
#define SCROLL_SUR	4
#define SCROLL_SDR	5

#define MODE1_ATRSEL	0
#define MODE1_GRAPHIC	1
#define MODE1_COLUMN	2
#define MODE1_FONTSEL	3
#define MODE1_200LINE	4
#define MODE1_KAC	5
#define MODE1_MEMSW	6
#define MODE1_DISP	7

#define MODE2_16COLOR	0x00
#define MODE2_EGC	0x02
#define MDOE2_TXTSHIFT	0x20

#define GRCG_PLANE_0	0x01
#define GRCG_PLANE_1	0x02
#define GRCG_PLANE_2	0x04
#define GRCG_PLANE_3	0x08
#define GRCG_RW_MODE	0x40
#define GRCG_CG_MODE	0x80

#define ATTR_ST		0x01
#define ATTR_BL		0x02
#define ATTR_RV		0x04
#define ATTR_UL		0x08
#define ATTR_VL		0x10
#define ATTR_COL	0xe0

static const uint8_t memsw_default[] = {
	0xe1, 0x48, 0xe1, 0x05, 0xe1, 0x04, 0xe1, 0x00,
	0xe1, 0x01, 0xe1, 0x00, 0xe1, 0x00, 0xe1, 0x00,
};

#if defined(SUPPORT_EGC)
static const uint8_t egc_bytemask_u0[64] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
	0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x01,
	0xe0, 0x70, 0x38, 0x1c, 0x0e, 0x07, 0x03, 0x01,
	0xf0, 0x78, 0x3c, 0x1e, 0x0f, 0x07, 0x03, 0x01,
	0xf8, 0x7c, 0x3e, 0x1f, 0x0f, 0x07, 0x03, 0x01,
	0xfc, 0x7e, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01,
	0xfe, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01,
	0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01,
};

static const uint8_t egc_bytemask_u1[8] =  {
	0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
};

static const uint8_t egc_bytemask_d0[64] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80,
	0x07, 0x0e, 0x1c, 0x38, 0x70, 0xe0, 0xc0, 0x80,
	0x0f, 0x1e, 0x3c, 0x78, 0xf0, 0xe0, 0xc0, 0x80,
	0x1f, 0x3e, 0x7c, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
	0x3f, 0x7e, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
	0x7f, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
	0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
};

static const uint8_t egc_bytemask_d1[8] = {
	0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
};

static const uint16_t egc_maskword[16][4] = {
	{0x0000, 0x0000, 0x0000, 0x0000}, {0xffff, 0x0000, 0x0000, 0x0000},
	{0x0000, 0xffff, 0x0000, 0x0000}, {0xffff, 0xffff, 0x0000, 0x0000},
	{0x0000, 0x0000, 0xffff, 0x0000}, {0xffff, 0x0000, 0xffff, 0x0000},
	{0x0000, 0xffff, 0xffff, 0x0000}, {0xffff, 0xffff, 0xffff, 0x0000},
	{0x0000, 0x0000, 0x0000, 0xffff}, {0xffff, 0x0000, 0x0000, 0xffff},
	{0x0000, 0xffff, 0x0000, 0xffff}, {0xffff, 0xffff, 0x0000, 0xffff},
	{0x0000, 0x0000, 0xffff, 0xffff}, {0xffff, 0x0000, 0xffff, 0xffff},
	{0x0000, 0xffff, 0xffff, 0xffff}, {0xffff, 0xffff, 0xffff, 0xffff}
};
#endif

void DISPLAY::initialize()
{
	// load font data
	memset(font, 0xff, sizeof(font));
	
	FILEIO* fio = new FILEIO();
	
#if !defined(SUPPORT_HIRESO)
	uint8_t *p = font + 0x81000;
	uint8_t *q = font + 0x83000;
	for(int i = 0; i < 256; i++) {
		for(int j = 0; j < 4; j++) {
			uint32_t bit = 0;
			if(i & (1 << j)) {
				bit |= 0xf0f0f0f0;
			}
			if(i & (0x10 << j)) {
				bit |= 0x0f0f0f0f;
			}
			*(uint32_t *)p = bit;
			p += 4;
			*(uint16_t *)q = (uint16_t)bit;
			q += 2;
		}
		q += 8;
	}
	for(int i = 0; i < 0x80; i++) {
		q = font + (i << 12);
		memset(q + 0x000, 0, 0x0560 - 0x000);
		memset(q + 0x580, 0, 0x0d60 - 0x580);
		memset(q + 0xd80, 0, 0x1000 - 0xd80);
	}
	if(!fio->Fopen(create_local_path(_T("FONT16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY);
	}
	if(fio->IsOpened()) {
		uint8_t *buf = (uint8_t *)malloc(0x46800);
		fio->Fread(buf, 0x46800, 1);
		fio->Fclose();
		
		// 8x8 font
		uint8_t *dst = font + 0x82000;
		uint8_t *src = buf;
		for(int i = 0; i < 256; i++) {
			memcpy(dst, src, 8);
			dst += 16;
			src += 8;
		}
		// 8x16 font
		memcpy(font + 0x80000, buf + 0x0800, 16 * 128);
		memcpy(font + 0x80800, buf + 0x1000, 16 * 128);
		// kanji font
		kanji_copy(font, buf, 0x01, 0x30);
		kanji_copy(font, buf, 0x30, 0x56);
		kanji_copy(font, buf, 0x58, 0x5d);
		
		free(buf);
	}
#else
	if(fio->Fopen(create_local_path(_T("FONT24.ROM")), FILEIO_READ_BINARY)) {
		uint8_t pattern[72];
		
		for(int code = 0x00; code <= 0xff; code++) {
			fio->Fread(pattern, 48, 1);
			ank_copy(code, pattern);
		}
		for(int first = 0x21; first <= 0x27; first++) {
			for(int second = 0x21; second <= 0x7e; second++) {
				fio->Fread(pattern, 72, 1);
				kanji_copy(first, second, pattern);
			}
		}
		for(int first = 0x30; first <= 0x73; first++) {
			for(int second = 0x21; second <= 0x7e; second++) {
				fio->Fread(pattern, 72, 1);
				kanji_copy(first, second, pattern);
			}
		}
		for(int first = 0x79; first <= 0x7c; first++) {
			for(int second = 0x21; second <= 0x7e; second++) {
				fio->Fread(pattern, 72, 1);
				kanji_copy(first, second, pattern);
			}
		}
		memcpy(font + ANK_FONT_OFS + FONT_SIZE * 0x100, font + ANK_FONT_OFS, FONT_SIZE * 0x100);
		memcpy(font + ANK_FONT_OFS + FONT_SIZE * 0x200, font + ANK_FONT_OFS, FONT_SIZE * 0x100);
		memcpy(font + ANK_FONT_OFS + FONT_SIZE * 0x300, font + ANK_FONT_OFS, FONT_SIZE * 0x100);
	}
#endif
	delete fio;
	
	// init palette
	for(int i = 0; i < 8; i++) {
		palette_chr[i] = RGB_COLOR((i & 2) ? 0xff : 0, (i & 4) ? 0xff : 0, (i & 1) ? 0xff : 0);
	}
	
	memset(palette_gfx8, 0, sizeof(palette_gfx8));
	memset(digipal, 0, sizeof(digipal));
	
#if defined(SUPPORT_16_COLORS)
	memset(palette_gfx16, 0, sizeof(palette_gfx16));
	memset(anapal, 0, sizeof(anapal));
	anapal_sel = 0;
#endif
	
	memset(tvram, 0, sizeof(tvram));
	memset(vram, 0, sizeof(vram));
	
	for(int i = 0; i < 16; i++) {
		tvram[0x3fe0 + (i << 1)] = memsw_default[i];
	}
	if(sound_bios_ok) {
		tvram[0x3fee] = 8;
	}
#ifndef HAS_UPD4990A
	dll_cur_time_t cur_time;
	get_host_time(&cur_time);
	tvram[0x3ffe] = TO_BCD(cur_time.year);
#endif
	
	// set vram pointer to gdc
	d_gdc_chr->set_vram_ptr(tvram, 0x2000);
	d_gdc_chr->set_screen_width(80);
	d_gdc_gfx->set_vram_bus_ptr(this, 0x20000);
	d_gdc_gfx->set_screen_width(SCREEN_WIDTH >> 3);
	
	// register event
	register_frame_event(this);
}

#if !defined(SUPPORT_HIRESO)
void DISPLAY::kanji_copy(uint8_t *dst, uint8_t *src, int from, int to)
{
	for(int i = from; i < to; i++) {
		uint8_t *p = src + 0x1800 + (0x60 * 32 * (i - 1));
		uint8_t *q = dst + 0x20000 + (i << 4);
		for(int j = 0x20; j < 0x80; j++) {
			for(int k = 0; k < 16; k++) {
				*(q + 0x800) = *(p + 16);
				*q++ = *p++;
			}
			p += 16;
			q += 0x1000 - 16;
		}
	}
}
#else
void DISPLAY::ank_copy(int code, uint8_t *pattern)
{
	uint16_t *dest = (uint16_t *)(font + ANK_FONT_OFS + FONT_SIZE * code);
	
	for(int i = 0; i < 24; i++) {
		dest[i] = (pattern[2 * i] << 8) | pattern[2 * i + 1];
	}
}

void DISPLAY::kanji_copy(int first, int second, uint8_t *pattern)
{
	uint16_t *dest_l = (uint16_t *)(font + FONT_SIZE * ((first - 0x20) | (second << 8)));
	uint16_t *dest_r = (uint16_t *)(font + FONT_SIZE * ((first - 0x20) | (second << 8)) + KANJI_2ND_OFS);
	
	for(int i = 0; i < 24; i++) {
		uint32_t p = (pattern[3 * i] << 16) | (pattern[3 * i + 1] << 8) | pattern[3 * i + 2];
		dest_l[i] = p >> 12;
		dest_r[i] = p << 2;
	}
}
#endif

void DISPLAY::reset()
{
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	vram_disp_sel = 0x00;
	vram_draw_sel = 0x00;
#endif
	vram_disp_b = vram + VRAM_PLANE_ADDR_0;
	vram_disp_r = vram + VRAM_PLANE_ADDR_1;
	vram_disp_g = vram + VRAM_PLANE_ADDR_2;
#if defined(SUPPORT_16_COLORS)
	vram_disp_e = vram + VRAM_PLANE_ADDR_3;
#endif
	vram_draw   = vram + 0x00000;
	
	crtv = 2;
	
	scroll[SCROLL_PL ] = 0;
	scroll[SCROLL_BL ] = 0x0f;
	scroll[SCROLL_CL ] = 0x10;
	scroll[SCROLL_SSL] = 0;
	scroll[SCROLL_SUR] = 0;
	scroll[SCROLL_SDR] = 24;
	
	memset(modereg1, 0, sizeof(modereg1));
#if defined(SUPPORT_16_COLORS)
	memset(modereg2, 0, sizeof(modereg2));
#endif
#if defined(SUPPORT_GRCG)
	grcg_mode = grcg_tile_ptr = 0;
#endif
#if defined(SUPPORT_EGC)
	egc_access = 0xfff0;
	egc_fgbg = 0x00ff;
	egc_ope = 0;
	egc_fg = 0;
	egc_mask.w = 0xffff;
	egc_bg = 0;
	egc_sft = 0;
	egc_leng = 0x000f;
	egc_lastvram.q = 0;
	egc_patreg.q = 0;
	egc_fgc.q = 0;
	egc_bgc.q = 0;
	egc_func = 0;
	egc_remain = 0;
	egc_stack = 0;
	egc_inptr = egc_buf;
	egc_outptr = egc_buf;
	egc_mask2.w = 0;
	egc_srcmask.w = 0;
	egc_srcbit = 0;
	egc_dstbit = 0;
	egc_sft8bitl = 0;
	egc_sft8bitr = 0;
	memset(egc_buf, 0, sizeof(egc_buf));
	egc_vram_src.q = 0;
	egc_vram_data.q = 0;
	egc_shift();
	egc_srcmask.w = 0xffff;
#endif
	
	font_code = 0;
	font_line = 0;
//	font_lr = 0;
}

void DISPLAY::event_frame()
{
	if(crtv > 1) {
		// dont raise irq at first frame
		crtv--;
	} else if(crtv == 1) {
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR2, 1, 1);
		crtv = 0;
	}
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x64:
		crtv = 1;
		break;
	case 0x68:
		modereg1[(data >> 1) & 7] = data & 1;
		break;
#if defined(SUPPORT_16_COLORS)
	case 0x6a:
		modereg2[(data >> 1) & 127] = data & 1;
		break;
#endif
	case 0x6c:
//		border = (data >> 3) & 7;
		break;
	case 0x6e:
//		border = (data >> 3) & 7;
#if !defined(_PC9801)
		if(data & 1) {
			d_gdc_chr->set_horiz_freq(24830);
			d_gdc_gfx->set_horiz_freq(24830);
		} else {
			d_gdc_chr->set_horiz_freq(15750);
			d_gdc_gfx->set_horiz_freq(15750);
		}
#endif
		break;
	case 0x70:
	case 0x72:
	case 0x74:
	case 0x76:
	case 0x78:
	case 0x7a:
		scroll[(addr >> 1) & 7] = data;
		break;
#if defined(SUPPORT_GRCG)
#if !defined(SUPPORT_HIRESO)
	case 0x007c:
#else
	case 0x00a4:
#endif
		grcg_mode = data;
		grcg_tile_ptr = 0;
		break;
#if !defined(SUPPORT_HIRESO)
	case 0x007e:
#else
	case 0x00a6:
#endif
		grcg_tile[grcg_tile_ptr] = data;
		grcg_tile_ptr = (grcg_tile_ptr + 1) & 3;
		break;
#endif
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	// vram select
	case 0x00a4:
		if(data & 1) {
			vram_disp_b = vram + 0x28000;
			vram_disp_r = vram + 0x30000;
			vram_disp_g = vram + 0x38000;
#if defined(SUPPORT_16_COLORS)
			vram_disp_e = vram + 0x20000;
#endif
		} else {
			vram_disp_b = vram + 0x08000;
			vram_disp_r = vram + 0x10000;
			vram_disp_g = vram + 0x18000;
#if defined(SUPPORT_16_COLORS)
			vram_disp_e = vram + 0x00000;
#endif
		}
		vram_disp_sel = data;
		break;
	case 0x00a6:
		if(data & 1) {
			vram_draw = vram + 0x20000;
		} else {
			vram_draw = vram + 0x00000;
		}
		vram_draw_sel = data;
		break;
#endif
	// palette
	case 0xa8:
#if defined(SUPPORT_16_COLORS)
		if(modereg2[MODE2_16COLOR]) {
			anapal_sel = data & 0x0f;
			break;
		}
#endif
		digipal[0] = data;
		palette_gfx8[7] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx8[3] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0xaa:
#if defined(SUPPORT_16_COLORS)
		if(modereg2[MODE2_16COLOR]) {
			anapal[anapal_sel][0] = (data & 0x0f) << 4;
			palette_gfx16[anapal_sel] = RGB_COLOR(anapal[anapal_sel][1], anapal[anapal_sel][0], anapal[anapal_sel][2]);
			break;
		}
#endif
		digipal[1] = data;
		palette_gfx8[5] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx8[1] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0xac:
#if defined(SUPPORT_16_COLORS)
		if(modereg2[MODE2_16COLOR]) {
			anapal[anapal_sel][1] = (data & 0x0f) << 4;
			palette_gfx16[anapal_sel] = RGB_COLOR(anapal[anapal_sel][1], anapal[anapal_sel][0], anapal[anapal_sel][2]);
			break;
		}
#endif
		digipal[2] = data;
		palette_gfx8[6] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx8[2] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0xae:
#if defined(SUPPORT_16_COLORS)
		if(modereg2[MODE2_16COLOR]) {
			anapal[anapal_sel][2] = (data & 0x0f) << 4;
			palette_gfx16[anapal_sel] = RGB_COLOR(anapal[anapal_sel][1], anapal[anapal_sel][0], anapal[anapal_sel][2]);
			break;
		}
#endif
		digipal[3] = data;
		palette_gfx8[4] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx8[0] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	// cg window
	case 0xa1:
		font_code = (data << 8) | (font_code & 0xff);
		break;
	case 0xa3:
		font_code = (font_code & 0xff00) | data;
		break;
	case 0xa5:
//		font_line = data & 0x1f;
//		font_lr = ((~data) & 0x20) << 6;
		font_line = data;
		break;
	case 0xa9:
		if((font_code & 0x7e) == 0x56) {
			uint16_t font_lr = ((~font_line) & 0x20) << 6;
			font[((font_code & 0x7f7f) << 4) + font_lr + (font_line & 0x0f)] = data;
		}
		break;
#if defined(SUPPORT_EGC)
	// egc
	case 0x04a0:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_access &= 0xff00;
			egc_access |= data;
		}
		break;
	case 0x04a1:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_access &= 0x00ff;
			egc_access |= data << 8;
		}
		break;
	case 0x04a2:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_fgbg &= 0xff00;
			egc_fgbg |= data;
		}
		break;
	case 0x04a3:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_fgbg &= 0x00ff;
			egc_fgbg |= data << 8;
		}
		break;
	case 0x04a4:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_ope &= 0xff00;
			egc_ope |= data;
		}
		break;
	case 0x04a5:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_ope &= 0x00ff;
			egc_ope |= data << 8;
		}
		break;
	case 0x04a6:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_fg &= 0xff00;
			egc_fg |= data;
			egc_fgc.d[0] = *(uint32_t *)(egc_maskword[data & 0x0f] + 0);
			egc_fgc.d[1] = *(uint32_t *)(egc_maskword[data & 0x0f] + 2);
		}
		break;
	case 0x04a7:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_fg &= 0x00ff;
			egc_fg |= data << 8;
		}
		break;
	case 0x04a8:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			if(!(egc_fgbg & 0x6000)) {
				egc_mask.b[0] = data;
			}
		}
		break;
	case 0x04a9:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			if(!(egc_fgbg & 0x6000)) {
				egc_mask.b[1] = data;
			}
		}
		break;
	case 0x04aa:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_bg &= 0xff00;
			egc_bg |= data;
			egc_bgc.d[0] = *(uint32_t *)(egc_maskword[data & 0x0f] + 0);
			egc_bgc.d[1] = *(uint32_t *)(egc_maskword[data & 0x0f] + 2);
		}
		break;
	case 0x04ab:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_bg &= 0x00ff;
			egc_bg |= data << 8;
		}
		break;
	case 0x04ac:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_sft &= 0xff00;
			egc_sft |= data;
			egc_shift();
			egc_srcmask.w = 0xffff;
		}
		break;
	case 0x04ad:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_sft &= 0x00ff;
			egc_sft |= data << 8;
			egc_shift();
			egc_srcmask.w = 0xffff;
		}
		break;
	case 0x04ae:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_leng &= 0xff00;
			egc_leng |= data;
			egc_shift();
			egc_srcmask.w = 0xffff;
		}
		break;
	case 0x04af:
		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_leng &= 0x00ff;
			egc_leng |= data << 8;
			egc_shift();
			egc_srcmask.w = 0xffff;
		}
		break;
#endif
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	switch(addr) {
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	// vram select
	case 0x00a4:
		return vram_disp_sel;
	case 0x00a6:
		return vram_draw_sel;
#endif
	// palette
	case 0xa8:
		return digipal[0];
	case 0xaa:
		return digipal[1];
	case 0xac:
		return digipal[2];
	case 0xae:
		return digipal[3];
	// cg window
	case 0xa1:
		return (font_code >> 8) & 0xff;
	case 0xa3:
		return (font_code >> 0) & 0xff;
	case 0xa5:
		return font_line;
	case 0xa9:
		if((font_code & 0xff) >= 0x09 && (font_code & 0xff) < 0x0c) {
			uint16_t font_lr = ((~font_line) & 0x20) << 6;
			if(!font_lr) {
				return font[((font_code & 0x7f7f) << 4) + (font_line & 0x0f)];
			}
		} else if(font_code & 0xff00) {
			uint16_t font_lr = ((~font_line) & 0x20) << 6;
			return font[((font_code & 0x7f7f) << 4) + font_lr + (font_line & 0x0f)];
		} else if(!(font_line & 0x10)) {
			return font[0x80000 + (font_code << 4) + (font_line & 0x1f)];
		}
		return 0;
	}
	return 0xff;
}

//		CPU	GDC
//	B	A8000h	08000h
//	R	B0000h	10000h
//	G	B8000h	18000h
//	I	E0000h	00000h

// CPU memory bus

void DISPLAY::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if(TVRAM_ADDRESS <= addr && addr < (TVRAM_ADDRESS + 0x3fe2)) {
		tvram[addr - TVRAM_ADDRESS] = data;
	} else if((TVRAM_ADDRESS + 0x3fe2) <= addr && addr < (TVRAM_ADDRESS + 0x4000)) {
		// memory switch
		if(modereg1[MODE1_MEMSW]) {
			tvram[addr - TVRAM_ADDRESS] = data;
		}
	} else if((TVRAM_ADDRESS + 0x4000) <= addr && addr < (TVRAM_ADDRESS + 0x5000)) {
		if((font_code & 0x7e) == 0x56) {
			/* FIXME: need to fix for hireso */
			uint32_t low = 0x7fff0, high;
			uint8_t code = font_code & 0x7f;
			uint16_t lr = ((~font_line) & 0x20) << 6;
			if(!(font_code & 0xff00)) {
				high = 0x80000 + (font_code << 4);
				if(!modereg1[MODE1_FONTSEL]) {
					high += 0x2000;
				}
			} else {
				high = (font_code & 0x7f7f) << 4;
				if(code >= 0x56 && code < 0x58) {
					high += lr;
				} else if(code >= 0x09 && code < 0x0c) {
					if(lr) {
						high = low;
					}
				} else if((code >= 0x0c && code < 0x10) || (code >= 0x58 && code < 0x60)) {
					high += lr;
				} else {
					low = high;
					high += 0x800;
				}
			}
			if(addr & 1) {
				font[high + ((addr >> 1) & 0x0f)] = data;
			} else {
				font[low  + ((addr >> 1) & 0x0f)] = data;
			}
		}
#if !defined(SUPPORT_HIRESO)
	} else if(0xa8000 <= addr && addr < 0xc0000) {
		write_dma_io8(addr - 0xa0000, data);
#if defined(SUPPORT_16_COLORS)
	} else if(0xe0000 <= addr && addr < 0xe8000) {
		write_dma_io8(addr - 0xe0000, data);
#endif
#else
	} else if(0xc0000 <= addr && addr < 0xe0000) {
		write_dma_io8(addr - 0xc0000, data);
#endif
	}
}

void DISPLAY::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	if(TVRAM_ADDRESS <= addr && addr < (TVRAM_ADDRESS + 0x3fe2)) {
		*(uint16_t *)(&tvram[addr - TVRAM_ADDRESS]) = data;
	} else if((TVRAM_ADDRESS + 0x3fe2) <= addr && addr < (TVRAM_ADDRESS + 0x4000)) {
		// memory switch
		if(modereg1[MODE1_MEMSW]) {
			*(uint16_t *)(&tvram[addr - TVRAM_ADDRESS]) = data;
		}
	} else if((TVRAM_ADDRESS + 0x4000) <= addr && addr < (TVRAM_ADDRESS + 0x5000)) {
		write_memory_mapped_io8(addr + 0, (data >> 0) & 0xff);
		write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
#if !defined(SUPPORT_HIRESO)
	} else if(0xa8000 <= addr && addr < 0xc0000) {
		write_dma_io16(addr - 0xa0000, data);
#if defined(SUPPORT_16_COLORS)
	} else if(0xe0000 <= addr && addr < 0xe8000) {
		write_dma_io16(addr - 0xe0000, data);
#endif
#else
	} else if(0xc0000 <= addr && addr < 0xe0000) {
		write_dma_io16(addr - 0xc0000, data);
#endif
	}
}

uint32_t DISPLAY::read_memory_mapped_io8(uint32_t addr)
{
	if(TVRAM_ADDRESS <= addr && addr < (TVRAM_ADDRESS + 0x2000)) {
		return tvram[addr - TVRAM_ADDRESS];
	} else if((TVRAM_ADDRESS + 0x2000) <= addr && addr < (TVRAM_ADDRESS + 0x4000)) {
		if(addr & 1) {
			return 0xff;
		}
		return tvram[addr - TVRAM_ADDRESS];
	} else if((TVRAM_ADDRESS + 0x4000) <= addr && addr < (TVRAM_ADDRESS + 0x5000)) {
		/* FIXME: need to fix for hireso */
		uint32_t low = 0x7fff0, high;
		uint8_t code = font_code & 0x7f;
		uint16_t lr = ((~font_line) & 0x20) << 6;
		if(!(font_code & 0xff00)) {
			high = 0x80000 + (font_code << 4);
			if(!modereg1[MODE1_FONTSEL]) {
				high += 0x2000;
			}
		} else {
			high = (font_code & 0x7f7f) << 4;
			if(code >= 0x56 && code < 0x58) {
				high += lr;
			} else if(code >= 0x09 && code < 0x0c) {
				if(lr) {
					high = low;
				}
			} else if((code >= 0x0c && code < 0x10) || (code >= 0x58 && code < 0x60)) {
				high += lr;
			} else {
				low = high;
				high += 0x800;
			}
		}
		if(addr & 1) {
			return font[high + ((addr >> 1) & 0x0f)];
		} else {
			return font[low  + ((addr >> 1) & 0x0f)];
		}
#if !defined(SUPPORT_HIRESO)
	} else if(0xa8000 <= addr && addr < 0xc0000) {
		return read_dma_io8(addr - 0xa0000);
#if defined(SUPPORT_16_COLORS)
	} else if(0xe0000 <= addr && addr < 0xe8000) {
		return read_dma_io8(addr - 0xe0000);
#endif
#else
	} else if(0xc0000 <= addr && addr < 0xe0000) {
		return read_dma_io8(addr - 0xc0000);
#endif
	}
	return 0xff;
}

uint32_t DISPLAY::read_memory_mapped_io16(uint32_t addr)
{
	if(TVRAM_ADDRESS <= addr && addr < (TVRAM_ADDRESS + 0x2000)) {
		return *(uint16_t *)(&tvram[addr - TVRAM_ADDRESS]);
	} else if((TVRAM_ADDRESS + 0x2000) <= addr && addr < (TVRAM_ADDRESS + 0x4000)) {
		if(addr & 1) {
			return 0xffff;
		}
		return *(uint16_t *)(&tvram[addr - TVRAM_ADDRESS]);
	} else if((TVRAM_ADDRESS + 0x4000) <= addr && addr < (TVRAM_ADDRESS + 0x5000)) {
		return read_memory_mapped_io8(addr) | (read_memory_mapped_io8(addr + 1) << 8);
#if !defined(SUPPORT_HIRESO)
	} else if(0xa8000 <= addr && addr < 0xc0000) {
		return read_dma_io16(addr - 0xa0000);
#if defined(SUPPORT_16_COLORS)
	} else if(0xe0000 <= addr && addr < 0xe8000) {
		return read_dma_io16(addr - 0xe0000);
#endif
#else
	} else if(0xc0000 <= addr && addr < 0xe0000) {
		return read_dma_io16(addr - 0xc0000);
#endif
	}
	return 0xffff;
}

// Graphic GDC bus

void DISPLAY::write_dma_io8(uint32_t addr, uint32_t data)
{
#if defined(SUPPORT_GRCG)
	if(grcg_mode & GRCG_CG_MODE) {
#if defined(SUPPORT_EGC)
		if(modereg2[MODE2_EGC]) {
			egc_writeb(addr, data);
		} else
#endif
		grcg_writeb(addr, data);
	} else
#endif
	vram_draw[addr & 0x1ffff] = data;
}

void DISPLAY::write_dma_io16(uint32_t addr, uint32_t data)
{
#if defined(SUPPORT_GRCG)
	if(grcg_mode & GRCG_CG_MODE) {
#if defined(SUPPORT_EGC)
		if(modereg2[MODE2_EGC]) {
			egc_writew(addr, data);
		} else
#endif
		grcg_writew(addr, data);
	} else
#endif
	*(uint16_t *)(&vram_draw[addr & 0x1ffff]) = data;
}

uint32_t DISPLAY::read_dma_io8(uint32_t addr)
{
#if defined(SUPPORT_GRCG)
	if(grcg_mode & GRCG_CG_MODE) {
#if defined(SUPPORT_EGC)
		if(modereg2[MODE2_EGC]) {
			return egc_readb(addr);
		}
#endif
		return grcg_readb(addr);
	}
#endif
	return vram_draw[addr & 0x1ffff];
}

uint32_t DISPLAY::read_dma_io16(uint32_t addr)
{
#if defined(SUPPORT_GRCG)
	if(grcg_mode & GRCG_CG_MODE) {
#if defined(SUPPORT_EGC)
		if(modereg2[MODE2_EGC]) {
			return egc_readw(addr);
		}
#endif
		return grcg_readw(addr);
	}
#endif
	return *(uint16_t *)(&vram_draw[addr & 0x1ffff]);
}

// GRCG

#if defined(SUPPORT_GRCG)
void DISPLAY::grcg_writeb(uint32_t addr1, uint32_t data)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	
	if(grcg_mode & GRCG_RW_MODE) {
		// RMW
		if(!(grcg_mode & GRCG_PLANE_0)) {
			vram_draw[addr | VRAM_PLANE_ADDR_0] &= ~data;
			vram_draw[addr | VRAM_PLANE_ADDR_0] |= grcg_tile[0] & data;
		}
		if(!(grcg_mode & GRCG_PLANE_1)) {
			vram_draw[addr | VRAM_PLANE_ADDR_1] &= ~data;
			vram_draw[addr | VRAM_PLANE_ADDR_1] |= grcg_tile[1] & data;
		}
		if(!(grcg_mode & GRCG_PLANE_2)) {
			vram_draw[addr | VRAM_PLANE_ADDR_2] &= ~data;
			vram_draw[addr | VRAM_PLANE_ADDR_2] |= grcg_tile[2] & data;
		}
		if(!(grcg_mode & GRCG_PLANE_3)) {
			vram_draw[addr | VRAM_PLANE_ADDR_3] &= ~data;
			vram_draw[addr | VRAM_PLANE_ADDR_3] |= grcg_tile[3] & data;
		}
	} else {
		// TDW
		if(!(grcg_mode & GRCG_PLANE_0)) {
			vram_draw[addr | VRAM_PLANE_ADDR_0] = grcg_tile[0];
		}
		if(!(grcg_mode & GRCG_PLANE_1)) {
			vram_draw[addr | VRAM_PLANE_ADDR_1] = grcg_tile[1];
		}
		if(!(grcg_mode & GRCG_PLANE_2)) {
			vram_draw[addr | VRAM_PLANE_ADDR_2] = grcg_tile[2];
		}
		if(!(grcg_mode & GRCG_PLANE_3)) {
			vram_draw[addr | VRAM_PLANE_ADDR_3] = grcg_tile[3];
		}
	}
}

void DISPLAY::grcg_writew(uint32_t addr1, uint32_t data)
{
	grcg_writeb(addr1 + 0, (data >> 0) & 0xff);
	grcg_writeb(addr1 + 1, (data >> 8) & 0xff);
}

uint32_t DISPLAY::grcg_readb(uint32_t addr1)
{
	if(grcg_mode & GRCG_RW_MODE) {
		// VRAM
#if !defined(SUPPORT_HIRESO)
		return vram_draw[addr1 & 0x1ffff];
#else
		int plane = (grcg_mode >> 4) & 3;
		return vram_draw[(addr1 & 0x1ffff) | (0x20000 * plane)];
#endif
	} else {
		// TCR
		uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
		uint8_t data = 0;
		
		if(!(grcg_mode & GRCG_PLANE_0)) {
			data |= vram_draw[addr | VRAM_PLANE_ADDR_0] ^ grcg_tile[0];
		}
		if(!(grcg_mode & GRCG_PLANE_1)) {
			data |= vram_draw[addr | VRAM_PLANE_ADDR_1] ^ grcg_tile[1];
		}
		if(!(grcg_mode & GRCG_PLANE_2)) {
			data |= vram_draw[addr | VRAM_PLANE_ADDR_2] ^ grcg_tile[2];
		}
		if(!(grcg_mode & GRCG_PLANE_3)) {
			data |= vram_draw[addr | VRAM_PLANE_ADDR_3] ^ grcg_tile[3];
		}
		return data ^ 0xff;
	}
}

uint32_t DISPLAY::grcg_readw(uint32_t addr1)
{
	return grcg_readb(addr1) | (grcg_readb(addr1 + 1) << 8);
}
#endif

// EGC based on Neko Project 2 and QEMU/9821

#if defined(SUPPORT_EGC)
void DISPLAY::egc_shift()
{
	uint8_t src8, dst8;
	
	egc_remain = (egc_leng & 0xfff) + 1;
	egc_func = (egc_sft >> 12) & 1;
	if(!egc_func) {
		egc_inptr = egc_buf;
		egc_outptr = egc_buf;
	} else {
		egc_inptr = egc_buf + 4096 / 8 + 3;
		egc_outptr = egc_buf + 4096 / 8 + 3;
	}
	egc_srcbit = egc_sft & 0x0f;
	egc_dstbit = (egc_sft >> 4) & 0x0f;
	
	src8 = egc_srcbit & 0x07;
	dst8 = egc_dstbit & 0x07;
	if(src8 < dst8) {
		egc_func += 2;
		egc_sft8bitr = dst8 - src8;
		egc_sft8bitl = 8 - egc_sft8bitr;
	}
	else if(src8 > dst8) {
		egc_func += 4;
		egc_sft8bitl = src8 - dst8;
		egc_sft8bitr = 8 - egc_sft8bitl;
	}
	egc_stack = 0;
}

void DISPLAY::egc_sftb_upn_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
			egc_dstbit = 0;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
			egc_dstbit = 0;
		}
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u1[egc_remain - 1];
			egc_remain = 0;
		}
	}
	egc_vram_src.b[0][ext] = egc_outptr[0];
	egc_vram_src.b[1][ext] = egc_outptr[4];
	egc_vram_src.b[2][ext] = egc_outptr[8];
	egc_vram_src.b[3][ext] = egc_outptr[12];
	egc_outptr++;
}

void DISPLAY::egc_sftb_dnn_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
			egc_dstbit = 0;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
			egc_dstbit = 0;
		}
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d1[egc_remain - 1];
			egc_remain = 0;
		}
	}
	egc_vram_src.b[0][ext] = egc_outptr[0];
	egc_vram_src.b[1][ext] = egc_outptr[4];
	egc_vram_src.b[2][ext] = egc_outptr[8];
	egc_vram_src.b[3][ext] = egc_outptr[12];
	egc_outptr--;
}

void DISPLAY::egc_sftb_upr_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
		}
		egc_dstbit = 0;
		egc_vram_src.b[0][ext] = (egc_outptr[0] >> egc_sft8bitr);
		egc_vram_src.b[1][ext] = (egc_outptr[4] >> egc_sft8bitr);
		egc_vram_src.b[2][ext] = (egc_outptr[8] >> egc_sft8bitr);
		egc_vram_src.b[3][ext] = (egc_outptr[12] >> egc_sft8bitr);
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u1[egc_remain - 1];
			egc_remain = 0;
		}
		egc_vram_src.b[0][ext] = (egc_outptr[0] << egc_sft8bitl) | (egc_outptr[1] >> egc_sft8bitr);
		egc_vram_src.b[1][ext] = (egc_outptr[4] << egc_sft8bitl) | (egc_outptr[5] >> egc_sft8bitr);
		egc_vram_src.b[2][ext] = (egc_outptr[8] << egc_sft8bitl) | (egc_outptr[9] >> egc_sft8bitr);
		egc_vram_src.b[3][ext] = (egc_outptr[12] << egc_sft8bitl) | (egc_outptr[13] >> egc_sft8bitr);
		egc_outptr++;
	}
}

void DISPLAY::egc_sftb_dnr_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
		}
		egc_dstbit = 0;
		egc_vram_src.b[0][ext] = (egc_outptr[0] << egc_sft8bitr);
		egc_vram_src.b[1][ext] = (egc_outptr[4] << egc_sft8bitr);
		egc_vram_src.b[2][ext] = (egc_outptr[8] << egc_sft8bitr);
		egc_vram_src.b[3][ext] = (egc_outptr[12] << egc_sft8bitr);
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d1[egc_remain - 1];
			egc_remain = 0;
		}
		egc_outptr--;
		egc_vram_src.b[0][ext] = (egc_outptr[1] >> egc_sft8bitl) | (egc_outptr[0] << egc_sft8bitr);
		egc_vram_src.b[1][ext] = (egc_outptr[5] >> egc_sft8bitl) | (egc_outptr[4] << egc_sft8bitr);
		egc_vram_src.b[2][ext] = (egc_outptr[9] >> egc_sft8bitl) | (egc_outptr[8] << egc_sft8bitr);
		egc_vram_src.b[3][ext] = (egc_outptr[13] >> egc_sft8bitl) | (egc_outptr[12] << egc_sft8bitr);
	}
}

void DISPLAY::egc_sftb_upl_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
			egc_dstbit = 0;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
			egc_dstbit = 0;
		}
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u1[egc_remain - 1];
			egc_remain = 0;
		}
	}
	egc_vram_src.b[0][ext] = (egc_outptr[0] << egc_sft8bitl) | (egc_outptr[1] >> egc_sft8bitr);
	egc_vram_src.b[1][ext] = (egc_outptr[4] << egc_sft8bitl) | (egc_outptr[5] >> egc_sft8bitr);
	egc_vram_src.b[2][ext] = (egc_outptr[8] << egc_sft8bitl) | (egc_outptr[9] >> egc_sft8bitr);
	egc_vram_src.b[3][ext] = (egc_outptr[12] << egc_sft8bitl) | (egc_outptr[13] >> egc_sft8bitr);
	egc_outptr++;
}

void DISPLAY::egc_sftb_dnl_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
			egc_dstbit = 0;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
			egc_dstbit = 0;
		}
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d1[egc_remain - 1];
			egc_remain = 0;
		}
	}
	egc_outptr--;
	egc_vram_src.b[0][ext] = (egc_outptr[1] >> egc_sft8bitl) | (egc_outptr[0] << egc_sft8bitr);
	egc_vram_src.b[1][ext] = (egc_outptr[5] >> egc_sft8bitl) | (egc_outptr[4] << egc_sft8bitr);
	egc_vram_src.b[2][ext] = (egc_outptr[9] >> egc_sft8bitl) | (egc_outptr[8] << egc_sft8bitr);
	egc_vram_src.b[3][ext] = (egc_outptr[13] >> egc_sft8bitl) | (egc_outptr[12] << egc_sft8bitr);
}

void DISPLAY::egc_sftb_upn0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_upn_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

void DISPLAY::egc_sftw_upn0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_upn_sub(0);
	if(egc_remain) {
		egc_sftb_upn_sub(1);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[1] = 0;
	}
	egc_shift();
}

void DISPLAY::egc_sftb_dnn0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_dnn_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

void DISPLAY::egc_sftw_dnn0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_dnn_sub(1);
	if(egc_remain) {
		egc_sftb_dnn_sub(0);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[0] = 0;
	}
	egc_shift();
}

void DISPLAY::egc_sftb_upr0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_upr_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

void DISPLAY::egc_sftw_upr0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_upr_sub(0);
	if(egc_remain) {
		egc_sftb_upr_sub(1);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[1] = 0;
	}
	egc_shift();
}

void DISPLAY::egc_sftb_dnr0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_dnr_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

void DISPLAY::egc_sftw_dnr0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_dnr_sub(1);
	if(egc_remain) {
		egc_sftb_dnr_sub(0);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[0] = 0;
	}
	egc_shift();
}

void DISPLAY::egc_sftb_upl0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_upl_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

void DISPLAY::egc_sftw_upl0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_upl_sub(0);
	if(egc_remain) {
		egc_sftb_upl_sub(1);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[1] = 0;
	}
	egc_shift();
}

void DISPLAY::egc_sftb_dnl0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_dnl_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

void DISPLAY::egc_sftw_dnl0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_dnl_sub(1);
	if(egc_remain) {
		egc_sftb_dnl_sub(0);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[0] = 0;
	}
	egc_shift();
}

void DISPLAY::egc_sftb(int func, uint32_t ext)
{
	switch(func) {
	case 0: egc_sftb_upn0(ext); break;
	case 1: egc_sftb_dnn0(ext); break;
	case 2: egc_sftb_upr0(ext); break;
	case 3: egc_sftb_dnr0(ext); break;
	case 4: egc_sftb_upl0(ext); break;
	case 5: egc_sftb_dnl0(ext); break;
	}
}

void DISPLAY::egc_sftw(int func)
{
	switch(func) {
	case 0: egc_sftw_upn0(); break;
	case 1: egc_sftw_dnn0(); break;
	case 2: egc_sftw_upr0(); break;
	case 3: egc_sftw_dnr0(); break;
	case 4: egc_sftw_upl0(); break;
	case 5: egc_sftw_dnl0(); break;
	}
}

void DISPLAY::egc_shiftinput_byte(uint32_t ext)
{
	if(egc_stack <= 16) {
		if(egc_srcbit >= 8) {
			egc_srcbit -= 8;
		} else {
			egc_stack += (8 - egc_srcbit);
			egc_srcbit = 0;
		}
		if(!(egc_sft & 0x1000)) {
			egc_inptr++;
		} else {
			egc_inptr--;
		}
	}
	egc_srcmask.b[ext] = 0xff;
	egc_sftb(egc_func, ext);
}

void DISPLAY::egc_shiftinput_incw()
{
	if(egc_stack <= 16) {
		egc_inptr += 2;
		if(egc_srcbit >= 8) {
			egc_outptr++;
		}
		egc_stack += (16 - egc_srcbit);
		egc_srcbit = 0;
	}
	egc_srcmask.w = 0xffff;
	egc_sftw(egc_func);
}

void DISPLAY::egc_shiftinput_decw()
{
	if(egc_stack <= 16) {
		egc_inptr -= 2;
		if(egc_srcbit >= 8) {
			egc_outptr--;
		}
		egc_stack += (16 - egc_srcbit);
		egc_srcbit = 0;
	}
	egc_srcmask.w = 0xffff;
	egc_sftw(egc_func);
}

#define EGC_OPE_SHIFTB(addr, value) \
	do { \
		if(egc_ope & 0x400) { \
			egc_inptr[ 0] = (uint8_t)value; \
			egc_inptr[ 4] = (uint8_t)value; \
			egc_inptr[ 8] = (uint8_t)value; \
			egc_inptr[12] = (uint8_t)value; \
			egc_shiftinput_byte(addr & 1); \
		} \
	} while (0)

#define EGC_OPE_SHIFTW(value) \
	do { \
		if(egc_ope & 0x400) { \
			if(!(egc_sft & 0x1000)) { \
				egc_inptr[ 0] = (uint8_t)value; \
				egc_inptr[ 1] = (uint8_t)(value >> 8); \
				egc_inptr[ 4] = (uint8_t)value; \
				egc_inptr[ 5] = (uint8_t)(value >> 8); \
				egc_inptr[ 8] = (uint8_t)value; \
				egc_inptr[ 9] = (uint8_t)(value >> 8); \
				egc_inptr[12] = (uint8_t)value; \
				egc_inptr[13] = (uint8_t)(value >> 8); \
				egc_shiftinput_incw(); \
			} else { \
				egc_inptr[-1] = (uint8_t)value; \
				egc_inptr[ 0] = (uint8_t)(value >> 8); \
				egc_inptr[ 3] = (uint8_t)value; \
				egc_inptr[ 4] = (uint8_t)(value >> 8); \
				egc_inptr[ 7] = (uint8_t)value; \
				egc_inptr[ 8] = (uint8_t)(value >> 8); \
				egc_inptr[11] = (uint8_t)value; \
				egc_inptr[12] = (uint8_t)(value >> 8); \
				egc_shiftinput_decw(); \
			}  \
		} \
	} while (0)

uint64_t DISPLAY::egc_ope_00(uint8_t ope, uint32_t addr)
{
	return 0;
}

uint64_t DISPLAY::egc_ope_0f(uint8_t ope, uint32_t addr)
{
	egc_vram_data.d[0] = ~egc_vram_src.d[0];
	egc_vram_data.d[1] = ~egc_vram_src.d[1];
	return egc_vram_data.q;
}

uint64_t DISPLAY::egc_ope_c0(uint8_t ope, uint32_t addr)
{
	egcquad_t dst;
	
	dst.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
	dst.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
	dst.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
	dst.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
	egc_vram_data.d[0] = (egc_vram_src.d[0] & dst.d[0]);
	egc_vram_data.d[1] = (egc_vram_src.d[1] & dst.d[1]);
	return egc_vram_data.q;
}

uint64_t DISPLAY::egc_ope_f0(uint8_t ope, uint32_t addr)
{
	return egc_vram_src.q;
}

uint64_t DISPLAY::egc_ope_fc(uint8_t ope, uint32_t addr)
{
	egcquad_t dst;

	dst.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
	dst.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
	dst.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
	dst.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
	egc_vram_data.d[0] = egc_vram_src.d[0];
	egc_vram_data.d[0] |= ((~egc_vram_src.d[0]) & dst.d[0]);
	egc_vram_data.d[1] = egc_vram_src.d[1];
	egc_vram_data.d[1] |= ((~egc_vram_src.d[1]) & dst.d[1]);
	return egc_vram_data.q;
}

uint64_t DISPLAY::egc_ope_ff(uint8_t ope, uint32_t addr)
{
	return ~0;
}

uint64_t DISPLAY::egc_ope_nd(uint8_t ope, uint32_t addr)
{
	egcquad_t pat;

	switch(egc_fgbg & 0x6000) {
	case 0x2000:
		pat.d[0] = egc_bgc.d[0];
		pat.d[1] = egc_bgc.d[1];
		break;
	case 0x4000:
		pat.d[0] = egc_fgc.d[0];
		pat.d[1] = egc_fgc.d[1];
		break;
	default:
		if((egc_ope & 0x0300) == 0x0100) {
			pat.d[0] = egc_vram_src.d[0];
			pat.d[1] = egc_vram_src.d[1];
		} else {
			pat.d[0] = egc_patreg.d[0];
			pat.d[1] = egc_patreg.d[1];
		}
		break;
	}
	egc_vram_data.d[0] = 0;
	egc_vram_data.d[1] = 0;
	if(ope & 0x80) {
		egc_vram_data.d[0] |= (pat.d[0] & egc_vram_src.d[0]);
		egc_vram_data.d[1] |= (pat.d[1] & egc_vram_src.d[1]);
	}
	if(ope & 0x40) {
		egc_vram_data.d[0] |= ((~pat.d[0]) & egc_vram_src.d[0]);
		egc_vram_data.d[1] |= ((~pat.d[1]) & egc_vram_src.d[1]);
	}
	if(ope & 0x08) {
		egc_vram_data.d[0] |= (pat.d[0] & (~egc_vram_src.d[0]));
		egc_vram_data.d[1] |= (pat.d[1] & (~egc_vram_src.d[1]));
	}
	if(ope & 0x04) {
		egc_vram_data.d[0] |= ((~pat.d[0]) & (~egc_vram_src.d[0]));
		egc_vram_data.d[1] |= ((~pat.d[1]) & (~egc_vram_src.d[1]));
	}
	return egc_vram_data.q;
}

uint64_t DISPLAY::egc_ope_np(uint8_t ope, uint32_t addr)
{
	egcquad_t dst;
	
	dst.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
	dst.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
	dst.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
	dst.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
	
	egc_vram_data.d[0] = 0;
	egc_vram_data.d[1] = 0;
	if(ope & 0x80) {
		egc_vram_data.d[0] |= (egc_vram_src.d[0] & dst.d[0]);
		egc_vram_data.d[1] |= (egc_vram_src.d[1] & dst.d[1]);
	}
	if(ope & 0x20) {
		egc_vram_data.d[0] |= (egc_vram_src.d[0] & (~dst.d[0]));
		egc_vram_data.d[1] |= (egc_vram_src.d[1] & (~dst.d[1]));
	}
	if(ope & 0x08) {
		egc_vram_data.d[0] |= ((~egc_vram_src.d[0]) & dst.d[0]);
		egc_vram_data.d[1] |= ((~egc_vram_src.d[1]) & dst.d[1]);
	}
	if(ope & 0x02) {
		egc_vram_data.d[0] |= ((~egc_vram_src.d[0]) & (~dst.d[0]));
		egc_vram_data.d[1] |= ((~egc_vram_src.d[1]) & (~dst.d[1]));
	}
	return egc_vram_data.q;
}

uint64_t DISPLAY::egc_ope_xx(uint8_t ope, uint32_t addr)
{
	egcquad_t pat;
	egcquad_t dst;
	
	switch(egc_fgbg & 0x6000) {
	case 0x2000:
		pat.d[0] = egc_bgc.d[0];
		pat.d[1] = egc_bgc.d[1];
		break;
	case 0x4000:
		pat.d[0] = egc_fgc.d[0];
		pat.d[1] = egc_fgc.d[1];
		break;
	default:
		if((egc_ope & 0x0300) == 0x0100) {
			pat.d[0] = egc_vram_src.d[0];
			pat.d[1] = egc_vram_src.d[1];
		} else {
			pat.d[0] = egc_patreg.d[0];
			pat.d[1] = egc_patreg.d[1];
		}
		break;
	}
	dst.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
	dst.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
	dst.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
	dst.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
	
	egc_vram_data.d[0] = 0;
	egc_vram_data.d[1] = 0;
	if(ope & 0x80) {
		egc_vram_data.d[0] |= (pat.d[0] & egc_vram_src.d[0] & dst.d[0]);
		egc_vram_data.d[1] |= (pat.d[1] & egc_vram_src.d[1] & dst.d[1]);
	}
	if(ope & 0x40) {
		egc_vram_data.d[0] |= ((~pat.d[0]) & egc_vram_src.d[0] & dst.d[0]);
		egc_vram_data.d[1] |= ((~pat.d[1]) & egc_vram_src.d[1] & dst.d[1]);
	}
	if(ope & 0x20) {
		egc_vram_data.d[0] |= (pat.d[0] & egc_vram_src.d[0] & (~dst.d[0]));
		egc_vram_data.d[1] |= (pat.d[1] & egc_vram_src.d[1] & (~dst.d[1]));
	}
	if(ope & 0x10) {
		egc_vram_data.d[0] |= ((~pat.d[0]) & egc_vram_src.d[0] & (~dst.d[0]));
		egc_vram_data.d[1] |= ((~pat.d[1]) & egc_vram_src.d[1] & (~dst.d[1]));
	}
	if(ope & 0x08) {
		egc_vram_data.d[0] |= (pat.d[0] & (~egc_vram_src.d[0]) & dst.d[0]);
		egc_vram_data.d[1] |= (pat.d[1] & (~egc_vram_src.d[1]) & dst.d[1]);
	}
	if(ope & 0x04) {
		egc_vram_data.d[0] |= ((~pat.d[0]) & (~egc_vram_src.d[0]) & dst.d[0]);
		egc_vram_data.d[1] |= ((~pat.d[1]) & (~egc_vram_src.d[1]) & dst.d[1]);
	}
	if(ope & 0x02) {
		egc_vram_data.d[0] |= (pat.d[0] & (~egc_vram_src.d[0]) & (~dst.d[0]));
		egc_vram_data.d[1] |= (pat.d[1] & (~egc_vram_src.d[1]) & (~dst.d[1]));
	}
	if(ope & 0x01) {
		egc_vram_data.d[0] |= ((~pat.d[0]) & (~egc_vram_src.d[0]) & (~dst.d[0]));
		egc_vram_data.d[1] |= ((~pat.d[1]) & (~egc_vram_src.d[1]) & (~dst.d[1]));
	}
	return egc_vram_data.q;
}

uint64_t DISPLAY::egc_opefn(uint32_t func, uint8_t ope, uint32_t addr)
{
	switch(func & 0xff) {
	case 0x00: return egc_ope_00(ope, addr);
	case 0x01: return egc_ope_xx(ope, addr);
	case 0x02: return egc_ope_xx(ope, addr);
	case 0x03: return egc_ope_np(ope, addr);
	case 0x04: return egc_ope_xx(ope, addr);
	case 0x05: return egc_ope_nd(ope, addr);
	case 0x06: return egc_ope_xx(ope, addr);
	case 0x07: return egc_ope_xx(ope, addr);
	case 0x08: return egc_ope_xx(ope, addr);
	case 0x09: return egc_ope_xx(ope, addr);
	case 0x0a: return egc_ope_nd(ope, addr);
	case 0x0b: return egc_ope_xx(ope, addr);
	case 0x0c: return egc_ope_np(ope, addr);
	case 0x0d: return egc_ope_xx(ope, addr);
	case 0x0e: return egc_ope_xx(ope, addr);
	case 0x0f: return egc_ope_0f(ope, addr);
	case 0x10: return egc_ope_xx(ope, addr);
	case 0x11: return egc_ope_xx(ope, addr);
	case 0x12: return egc_ope_xx(ope, addr);
	case 0x13: return egc_ope_xx(ope, addr);
	case 0x14: return egc_ope_xx(ope, addr);
	case 0x15: return egc_ope_xx(ope, addr);
	case 0x16: return egc_ope_xx(ope, addr);
	case 0x17: return egc_ope_xx(ope, addr);
	case 0x18: return egc_ope_xx(ope, addr);
	case 0x19: return egc_ope_xx(ope, addr);
	case 0x1a: return egc_ope_xx(ope, addr);
	case 0x1b: return egc_ope_xx(ope, addr);
	case 0x1c: return egc_ope_xx(ope, addr);
	case 0x1d: return egc_ope_xx(ope, addr);
	case 0x1e: return egc_ope_xx(ope, addr);
	case 0x1f: return egc_ope_xx(ope, addr);
	case 0x20: return egc_ope_xx(ope, addr);
	case 0x21: return egc_ope_xx(ope, addr);
	case 0x22: return egc_ope_xx(ope, addr);
	case 0x23: return egc_ope_xx(ope, addr);
	case 0x24: return egc_ope_xx(ope, addr);
	case 0x25: return egc_ope_xx(ope, addr);
	case 0x26: return egc_ope_xx(ope, addr);
	case 0x27: return egc_ope_xx(ope, addr);
	case 0x28: return egc_ope_xx(ope, addr);
	case 0x29: return egc_ope_xx(ope, addr);
	case 0x2a: return egc_ope_xx(ope, addr);
	case 0x2b: return egc_ope_xx(ope, addr);
	case 0x2c: return egc_ope_xx(ope, addr);
	case 0x2d: return egc_ope_xx(ope, addr);
	case 0x2e: return egc_ope_xx(ope, addr);
	case 0x2f: return egc_ope_xx(ope, addr);
	case 0x30: return egc_ope_np(ope, addr);
	case 0x31: return egc_ope_xx(ope, addr);
	case 0x32: return egc_ope_xx(ope, addr);
	case 0x33: return egc_ope_np(ope, addr);
	case 0x34: return egc_ope_xx(ope, addr);
	case 0x35: return egc_ope_xx(ope, addr);
	case 0x36: return egc_ope_xx(ope, addr);
	case 0x37: return egc_ope_xx(ope, addr);
	case 0x38: return egc_ope_xx(ope, addr);
	case 0x39: return egc_ope_xx(ope, addr);
	case 0x3a: return egc_ope_xx(ope, addr);
	case 0x3b: return egc_ope_xx(ope, addr);
	case 0x3c: return egc_ope_np(ope, addr);
	case 0x3d: return egc_ope_xx(ope, addr);
	case 0x3e: return egc_ope_xx(ope, addr);
	case 0x3f: return egc_ope_np(ope, addr);
	case 0x40: return egc_ope_xx(ope, addr);
	case 0x41: return egc_ope_xx(ope, addr);
	case 0x42: return egc_ope_xx(ope, addr);
	case 0x43: return egc_ope_xx(ope, addr);
	case 0x44: return egc_ope_xx(ope, addr);
	case 0x45: return egc_ope_xx(ope, addr);
	case 0x46: return egc_ope_xx(ope, addr);
	case 0x47: return egc_ope_xx(ope, addr);
	case 0x48: return egc_ope_xx(ope, addr);
	case 0x49: return egc_ope_xx(ope, addr);
	case 0x4a: return egc_ope_xx(ope, addr);
	case 0x4b: return egc_ope_xx(ope, addr);
	case 0x4c: return egc_ope_xx(ope, addr);
	case 0x4d: return egc_ope_xx(ope, addr);
	case 0x4e: return egc_ope_xx(ope, addr);
	case 0x4f: return egc_ope_xx(ope, addr);
	case 0x50: return egc_ope_nd(ope, addr);
	case 0x51: return egc_ope_xx(ope, addr);
	case 0x52: return egc_ope_xx(ope, addr);
	case 0x53: return egc_ope_xx(ope, addr);
	case 0x54: return egc_ope_xx(ope, addr);
	case 0x55: return egc_ope_nd(ope, addr);
	case 0x56: return egc_ope_xx(ope, addr);
	case 0x57: return egc_ope_xx(ope, addr);
	case 0x58: return egc_ope_xx(ope, addr);
	case 0x59: return egc_ope_xx(ope, addr);
	case 0x5a: return egc_ope_nd(ope, addr);
	case 0x5b: return egc_ope_xx(ope, addr);
	case 0x5c: return egc_ope_xx(ope, addr);
	case 0x5d: return egc_ope_xx(ope, addr);
	case 0x5e: return egc_ope_xx(ope, addr);
	case 0x5f: return egc_ope_nd(ope, addr);
	case 0x60: return egc_ope_xx(ope, addr);
	case 0x61: return egc_ope_xx(ope, addr);
	case 0x62: return egc_ope_xx(ope, addr);
	case 0x63: return egc_ope_xx(ope, addr);
	case 0x64: return egc_ope_xx(ope, addr);
	case 0x65: return egc_ope_xx(ope, addr);
	case 0x66: return egc_ope_xx(ope, addr);
	case 0x67: return egc_ope_xx(ope, addr);
	case 0x68: return egc_ope_xx(ope, addr);
	case 0x69: return egc_ope_xx(ope, addr);
	case 0x6a: return egc_ope_xx(ope, addr);
	case 0x6b: return egc_ope_xx(ope, addr);
	case 0x6c: return egc_ope_xx(ope, addr);
	case 0x6d: return egc_ope_xx(ope, addr);
	case 0x6e: return egc_ope_xx(ope, addr);
	case 0x6f: return egc_ope_xx(ope, addr);
	case 0x70: return egc_ope_xx(ope, addr);
	case 0x71: return egc_ope_xx(ope, addr);
	case 0x72: return egc_ope_xx(ope, addr);
	case 0x73: return egc_ope_xx(ope, addr);
	case 0x74: return egc_ope_xx(ope, addr);
	case 0x75: return egc_ope_xx(ope, addr);
	case 0x76: return egc_ope_xx(ope, addr);
	case 0x77: return egc_ope_xx(ope, addr);
	case 0x78: return egc_ope_xx(ope, addr);
	case 0x79: return egc_ope_xx(ope, addr);
	case 0x7a: return egc_ope_xx(ope, addr);
	case 0x7b: return egc_ope_xx(ope, addr);
	case 0x7c: return egc_ope_xx(ope, addr);
	case 0x7d: return egc_ope_xx(ope, addr);
	case 0x7e: return egc_ope_xx(ope, addr);
	case 0x7f: return egc_ope_xx(ope, addr);
	case 0x80: return egc_ope_xx(ope, addr);
	case 0x81: return egc_ope_xx(ope, addr);
	case 0x82: return egc_ope_xx(ope, addr);
	case 0x83: return egc_ope_xx(ope, addr);
	case 0x84: return egc_ope_xx(ope, addr);
	case 0x85: return egc_ope_xx(ope, addr);
	case 0x86: return egc_ope_xx(ope, addr);
	case 0x87: return egc_ope_xx(ope, addr);
	case 0x88: return egc_ope_xx(ope, addr);
	case 0x89: return egc_ope_xx(ope, addr);
	case 0x8a: return egc_ope_xx(ope, addr);
	case 0x8b: return egc_ope_xx(ope, addr);
	case 0x8c: return egc_ope_xx(ope, addr);
	case 0x8d: return egc_ope_xx(ope, addr);
	case 0x8e: return egc_ope_xx(ope, addr);
	case 0x8f: return egc_ope_xx(ope, addr);
	case 0x90: return egc_ope_xx(ope, addr);
	case 0x91: return egc_ope_xx(ope, addr);
	case 0x92: return egc_ope_xx(ope, addr);
	case 0x93: return egc_ope_xx(ope, addr);
	case 0x94: return egc_ope_xx(ope, addr);
	case 0x95: return egc_ope_xx(ope, addr);
	case 0x96: return egc_ope_xx(ope, addr);
	case 0x97: return egc_ope_xx(ope, addr);
	case 0x98: return egc_ope_xx(ope, addr);
	case 0x99: return egc_ope_xx(ope, addr);
	case 0x9a: return egc_ope_xx(ope, addr);
	case 0x9b: return egc_ope_xx(ope, addr);
	case 0x9c: return egc_ope_xx(ope, addr);
	case 0x9d: return egc_ope_xx(ope, addr);
	case 0x9e: return egc_ope_xx(ope, addr);
	case 0x9f: return egc_ope_xx(ope, addr);
	case 0xa0: return egc_ope_nd(ope, addr);
	case 0xa1: return egc_ope_xx(ope, addr);
	case 0xa2: return egc_ope_xx(ope, addr);
	case 0xa3: return egc_ope_xx(ope, addr);
	case 0xa4: return egc_ope_xx(ope, addr);
	case 0xa5: return egc_ope_nd(ope, addr);
	case 0xa6: return egc_ope_xx(ope, addr);
	case 0xa7: return egc_ope_xx(ope, addr);
	case 0xa8: return egc_ope_xx(ope, addr);
	case 0xa9: return egc_ope_xx(ope, addr);
	case 0xaa: return egc_ope_nd(ope, addr);
	case 0xab: return egc_ope_xx(ope, addr);
	case 0xac: return egc_ope_xx(ope, addr);
	case 0xad: return egc_ope_xx(ope, addr);
	case 0xae: return egc_ope_xx(ope, addr);
	case 0xaf: return egc_ope_nd(ope, addr);
	case 0xb0: return egc_ope_xx(ope, addr);
	case 0xb1: return egc_ope_xx(ope, addr);
	case 0xb2: return egc_ope_xx(ope, addr);
	case 0xb3: return egc_ope_xx(ope, addr);
	case 0xb4: return egc_ope_xx(ope, addr);
	case 0xb5: return egc_ope_xx(ope, addr);
	case 0xb6: return egc_ope_xx(ope, addr);
	case 0xb7: return egc_ope_xx(ope, addr);
	case 0xb8: return egc_ope_xx(ope, addr);
	case 0xb9: return egc_ope_xx(ope, addr);
	case 0xba: return egc_ope_xx(ope, addr);
	case 0xbb: return egc_ope_xx(ope, addr);
	case 0xbc: return egc_ope_xx(ope, addr);
	case 0xbd: return egc_ope_xx(ope, addr);
	case 0xbe: return egc_ope_xx(ope, addr);
	case 0xbf: return egc_ope_xx(ope, addr);
	case 0xc0: return egc_ope_c0(ope, addr);
	case 0xc1: return egc_ope_xx(ope, addr);
	case 0xc2: return egc_ope_xx(ope, addr);
	case 0xc3: return egc_ope_np(ope, addr);
	case 0xc4: return egc_ope_xx(ope, addr);
	case 0xc5: return egc_ope_xx(ope, addr);
	case 0xc6: return egc_ope_xx(ope, addr);
	case 0xc7: return egc_ope_xx(ope, addr);
	case 0xc8: return egc_ope_xx(ope, addr);
	case 0xc9: return egc_ope_xx(ope, addr);
	case 0xca: return egc_ope_xx(ope, addr);
	case 0xcb: return egc_ope_xx(ope, addr);
	case 0xcc: return egc_ope_np(ope, addr);
	case 0xcd: return egc_ope_xx(ope, addr);
	case 0xce: return egc_ope_xx(ope, addr);
	case 0xcf: return egc_ope_np(ope, addr);
	case 0xd0: return egc_ope_xx(ope, addr);
	case 0xd1: return egc_ope_xx(ope, addr);
	case 0xd2: return egc_ope_xx(ope, addr);
	case 0xd3: return egc_ope_xx(ope, addr);
	case 0xd4: return egc_ope_xx(ope, addr);
	case 0xd5: return egc_ope_xx(ope, addr);
	case 0xd6: return egc_ope_xx(ope, addr);
	case 0xd7: return egc_ope_xx(ope, addr);
	case 0xd8: return egc_ope_xx(ope, addr);
	case 0xd9: return egc_ope_xx(ope, addr);
	case 0xda: return egc_ope_xx(ope, addr);
	case 0xdb: return egc_ope_xx(ope, addr);
	case 0xdc: return egc_ope_xx(ope, addr);
	case 0xdd: return egc_ope_xx(ope, addr);
	case 0xde: return egc_ope_xx(ope, addr);
	case 0xdf: return egc_ope_xx(ope, addr);
	case 0xe0: return egc_ope_xx(ope, addr);
	case 0xe1: return egc_ope_xx(ope, addr);
	case 0xe2: return egc_ope_xx(ope, addr);
	case 0xe3: return egc_ope_xx(ope, addr);
	case 0xe4: return egc_ope_xx(ope, addr);
	case 0xe5: return egc_ope_xx(ope, addr);
	case 0xe6: return egc_ope_xx(ope, addr);
	case 0xe7: return egc_ope_xx(ope, addr);
	case 0xe8: return egc_ope_xx(ope, addr);
	case 0xe9: return egc_ope_xx(ope, addr);
	case 0xea: return egc_ope_xx(ope, addr);
	case 0xeb: return egc_ope_xx(ope, addr);
	case 0xec: return egc_ope_xx(ope, addr);
	case 0xed: return egc_ope_xx(ope, addr);
	case 0xee: return egc_ope_xx(ope, addr);
	case 0xef: return egc_ope_xx(ope, addr);
	case 0xf0: return egc_ope_f0(ope, addr);
	case 0xf1: return egc_ope_xx(ope, addr);
	case 0xf2: return egc_ope_xx(ope, addr);
	case 0xf3: return egc_ope_np(ope, addr);
	case 0xf4: return egc_ope_xx(ope, addr);
	case 0xf5: return egc_ope_nd(ope, addr);
	case 0xf6: return egc_ope_xx(ope, addr);
	case 0xf7: return egc_ope_xx(ope, addr);
	case 0xf8: return egc_ope_xx(ope, addr);
	case 0xf9: return egc_ope_xx(ope, addr);
	case 0xfa: return egc_ope_nd(ope, addr);
	case 0xfb: return egc_ope_xx(ope, addr);
	case 0xfc: return egc_ope_fc(ope, addr);
	case 0xfd: return egc_ope_xx(ope, addr);
	case 0xfe: return egc_ope_xx(ope, addr);
	case 0xff: return egc_ope_ff(ope, addr);
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
	return 0;
}

uint64_t DISPLAY::egc_opeb(uint32_t addr, uint8_t value)
{
	uint32_t tmp;
	
	egc_mask2.w = egc_mask.w;
	switch(egc_ope & 0x1800) {
	case 0x0800:
		EGC_OPE_SHIFTB(addr, value);
		egc_mask2.w &= egc_srcmask.w;
		tmp = egc_ope & 0xff;
		return egc_opefn(tmp, (uint8_t)tmp, addr & (~1));
	case 0x1000:
		switch(egc_fgbg & 0x6000) {
		case 0x2000:
			return egc_bgc.q;
		case 0x4000:
			return egc_fgc.q;
		default:
			EGC_OPE_SHIFTB(addr, value);
			egc_mask2.w &= egc_srcmask.w;
			return egc_vram_src.q;
		}
		break;
	default:
		tmp = value & 0xff;
		tmp = tmp | (tmp << 8);
		egc_vram_data.w[0] = (uint16_t)tmp;
		egc_vram_data.w[1] = (uint16_t)tmp;
		egc_vram_data.w[2] = (uint16_t)tmp;
		egc_vram_data.w[3] = (uint16_t)tmp;
		return egc_vram_data.q;
	}
}

uint64_t DISPLAY::egc_opew(uint32_t addr, uint16_t value)
{
	uint32_t tmp;

	egc_mask2.w = egc_mask.w;
	switch(egc_ope & 0x1800) {
	case 0x0800:
		EGC_OPE_SHIFTW(value);
		egc_mask2.w &= egc_srcmask.w;
		tmp = egc_ope & 0xff;
		return egc_opefn(tmp, (uint8_t)tmp, addr);
	case 0x1000:
		switch(egc_fgbg & 0x6000) {
		case 0x2000:
			return egc_bgc.q;
		case 0x4000:
			return egc_fgc.q;
		default:
			EGC_OPE_SHIFTW(value);
			egc_mask2.w &= egc_srcmask.w;
			return egc_vram_src.q;
		}
		break;
	default:
#ifdef __BIG_ENDIAN__
		value = ((value >> 8) & 0xff) | ((value & 0xff) << 8);
#endif
		egc_vram_data.w[0] = (uint16_t)value;
		egc_vram_data.w[1] = (uint16_t)value;
		egc_vram_data.w[2] = (uint16_t)value;
		egc_vram_data.w[3] = (uint16_t)value;
		return egc_vram_data.q;
	}
}

uint32_t DISPLAY::egc_readb(uint32_t addr1)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	uint32_t ext = addr1 & 1;
	
	egc_lastvram.b[0][ext] = vram_draw[addr | VRAM_PLANE_ADDR_0];
	egc_lastvram.b[1][ext] = vram_draw[addr | VRAM_PLANE_ADDR_1];
	egc_lastvram.b[2][ext] = vram_draw[addr | VRAM_PLANE_ADDR_2];
	egc_lastvram.b[3][ext] = vram_draw[addr | VRAM_PLANE_ADDR_3];
	
	if(!(egc_ope & 0x400)) {
		egc_inptr[0] = egc_lastvram.b[0][ext];
		egc_inptr[4] = egc_lastvram.b[1][ext];
		egc_inptr[8] = egc_lastvram.b[2][ext];
		egc_inptr[12] = egc_lastvram.b[3][ext];
		egc_shiftinput_byte(ext);
	}
	if((egc_ope & 0x0300) == 0x0100) {
		egc_patreg.b[0][ext] = vram_draw[addr | VRAM_PLANE_ADDR_0];
		egc_patreg.b[1][ext] = vram_draw[addr | VRAM_PLANE_ADDR_1];
		egc_patreg.b[2][ext] = vram_draw[addr | VRAM_PLANE_ADDR_2];
		egc_patreg.b[3][ext] = vram_draw[addr | VRAM_PLANE_ADDR_3];
	}
	if(!(egc_ope & 0x2000)) {
		int pl = (egc_fgbg >> 8) & 3;
		if(!(egc_ope & 0x400)) {
			return egc_vram_src.b[pl][ext];
		} else {
			return vram_draw[addr | (VRAM_PLANE_SIZE * pl)];
		}
	}
	return vram_draw[addr1];
}

uint32_t DISPLAY::egc_readw(uint32_t addr1)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	
	if(!(addr & 1)) {
		egc_lastvram.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
		egc_lastvram.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
		egc_lastvram.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
		egc_lastvram.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
		
		if(!(egc_ope & 0x400)) {
			if(!(egc_sft & 0x1000)) {
				egc_inptr[ 0] = egc_lastvram.b[0][0];
				egc_inptr[ 1] = egc_lastvram.b[0][1];
				egc_inptr[ 4] = egc_lastvram.b[1][0];
				egc_inptr[ 5] = egc_lastvram.b[1][1];
				egc_inptr[ 8] = egc_lastvram.b[2][0];
				egc_inptr[ 9] = egc_lastvram.b[2][1];
				egc_inptr[12] = egc_lastvram.b[3][0];
				egc_inptr[13] = egc_lastvram.b[3][1];
				egc_shiftinput_incw();
			} else {
				egc_inptr[-1] = egc_lastvram.b[0][0];
				egc_inptr[ 0] = egc_lastvram.b[0][1];
				egc_inptr[ 3] = egc_lastvram.b[1][0];
				egc_inptr[ 4] = egc_lastvram.b[1][1];
				egc_inptr[ 7] = egc_lastvram.b[2][0];
				egc_inptr[ 8] = egc_lastvram.b[2][1];
				egc_inptr[11] = egc_lastvram.b[3][0];
				egc_inptr[12] = egc_lastvram.b[3][1];
				egc_shiftinput_decw();
			}
		}
		if((egc_ope & 0x0300) == 0x0100) {
			egc_patreg.d[0] = egc_lastvram.d[0];
			egc_patreg.d[1] = egc_lastvram.d[1];
		}
		if(!(egc_ope & 0x2000)) {
			int pl = (egc_fgbg >> 8) & 3;
			if(!(egc_ope & 0x400)) {
				return egc_vram_src.w[pl];
			} else {
				return *(uint16_t *)(&vram_draw[addr | (VRAM_PLANE_SIZE * pl)]);
			}
		}
		return *(uint16_t *)(&vram_draw[addr1]);
	} else if(!(egc_sft & 0x1000)) {
		uint16_t value = egc_readb(addr1);
		value |= egc_readb(addr1 + 1) << 8;
		return value;
	} else {
		uint16_t value = egc_readb(addr1) << 8;
		value |= egc_readb(addr1 + 1);
		return value;
	}
}

void DISPLAY::egc_writeb(uint32_t addr1, uint8_t value)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	uint32_t ext = addr1 & 1;
	egcquad_t data;
	
	if((egc_ope & 0x0300) == 0x0200) {
		egc_patreg.b[0][ext] = vram_draw[addr | VRAM_PLANE_ADDR_0];
		egc_patreg.b[1][ext] = vram_draw[addr | VRAM_PLANE_ADDR_1];
		egc_patreg.b[2][ext] = vram_draw[addr | VRAM_PLANE_ADDR_2];
		egc_patreg.b[3][ext] = vram_draw[addr | VRAM_PLANE_ADDR_3];
	}
	data.q = egc_opeb(addr, value);
	if(egc_mask2.b[ext]) {
		if(!(egc_access & 1)) {
			vram_draw[addr | VRAM_PLANE_ADDR_0] &= ~egc_mask2.b[ext];
			vram_draw[addr | VRAM_PLANE_ADDR_0] |= data.b[0][ext] & egc_mask2.b[ext];
		}
		if(!(egc_access & 2)) {
			vram_draw[addr | VRAM_PLANE_ADDR_1] &= ~egc_mask2.b[ext];
			vram_draw[addr | VRAM_PLANE_ADDR_1] |= data.b[1][ext] & egc_mask2.b[ext];
		}
		if(!(egc_access & 4)) {
			vram_draw[addr | VRAM_PLANE_ADDR_2] &= ~egc_mask2.b[ext];
			vram_draw[addr | VRAM_PLANE_ADDR_2] |= data.b[2][ext] & egc_mask2.b[ext];
		}
		if(!(egc_access & 8)) {
			vram_draw[addr | VRAM_PLANE_ADDR_3] &= ~egc_mask2.b[ext];
			vram_draw[addr | VRAM_PLANE_ADDR_3] |= data.b[3][ext] & egc_mask2.b[ext];
		}
	}
}

void DISPLAY::egc_writew(uint32_t addr1, uint16_t value)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	egcquad_t data;
	
	if(!(addr & 1)) {
		if((egc_ope & 0x0300) == 0x0200) {
			egc_patreg.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
			egc_patreg.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
			egc_patreg.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
			egc_patreg.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
		}
		data.q = egc_opew(addr, value);
		if(egc_mask2.w) {
			if(!(egc_access & 1)) {
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]) &= ~egc_mask2.w;
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]) |= data.w[0] & egc_mask2.w;
			}
			if(!(egc_access & 2)) {
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]) &= ~egc_mask2.w;
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]) |= data.w[1] & egc_mask2.w;
			}
			if(!(egc_access & 4)) {
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]) &= ~egc_mask2.w;
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]) |= data.w[2] & egc_mask2.w;
			}
			if(!(egc_access & 8)) {
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]) &= ~egc_mask2.w;
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]) |= data.w[3] & egc_mask2.w;
			}
		}
	} else if(!(egc_sft & 0x1000)) {
		egc_writeb(addr1, (uint8_t)value);
		egc_writeb(addr1 + 1, (uint8_t)(value >> 8));
	} else {
		egc_writeb(addr1, (uint8_t)(value >> 8));
		egc_writeb(addr1 + 1, (uint8_t)value);
	}
}
#endif

void DISPLAY::draw_screen()
{
	// render screen
	bool gdc_chr_start = d_gdc_chr->get_start();
	bool gdc_gfx_start = d_gdc_gfx->get_start();
	
	if(modereg1[MODE1_DISP] && (gdc_chr_start || gdc_gfx_start)) {
		if(gdc_chr_start) {
			draw_chr_screen();
		} else {
			memset(screen_chr, 0, sizeof(screen_chr));
		}
		if(gdc_gfx_start) {
			draw_gfx_screen();
		} else {
			memset(screen_gfx, 0, sizeof(screen_gfx));
		}
		for(int y = 0; y < SCREEN_HEIGHT; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_chr = screen_chr[y];
#if defined(SUPPORT_16_COLORS)
			if(!modereg2[MDOE2_TXTSHIFT]) {
				src_chr++;
			}
#endif
			uint8_t *src_gfx = screen_gfx[y];
			
#if defined(SUPPORT_16_COLORS)
			if(!modereg2[MODE2_16COLOR]) {
#endif
				for(int x = 0; x < SCREEN_WIDTH; x++) {
					uint8_t chr = src_chr[x];
					dest[x] = chr ? palette_chr[chr & 7] : palette_gfx8[src_gfx[x] & 7];
				}
#if defined(SUPPORT_16_COLORS)
			} else {
				for(int x = 0; x < SCREEN_WIDTH; x++) {
					uint8_t chr = src_chr[x];
					dest[x] = chr ? palette_chr[chr & 7] : palette_gfx16[src_gfx[x]];
				}
			}
#endif
		}
	} else {
		for(int y = 0; y < SCREEN_HEIGHT; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			memset(dest, 0, SCREEN_WIDTH * sizeof(scrntype_t));
		}
	}
	emu->screen_skip_line(false);
}

void DISPLAY::draw_chr_screen()
{
	// scroll registers
	int pl = scroll[SCROLL_PL] & 31;
	if(pl) {
		pl = 32 - pl;
	}
	int bl = scroll[SCROLL_BL] + pl + 1;
	int cl = scroll[SCROLL_CL];
#if defined(SUPPORT_HIRESO)
	bl <<= 1;
	cl <<= 1;
#endif
	int ssl = scroll[SCROLL_SSL];
	int sur = scroll[SCROLL_SUR] & 31;
	if(sur) {
		sur = 32 - sur;
	}
	int sdr = scroll[SCROLL_SDR] + 1;
	
	// address from gdc
	uint32_t gdc_addr[25][80] = {0};
	
	for(int i = 0, ytop = 0; i < 4; i++) {
		uint32_t ra = ra_chr[i * 4];
		ra |= ra_chr[i * 4 + 1] << 8;
		ra |= ra_chr[i * 4 + 2] << 16;
		ra |= ra_chr[i * 4 + 3] << 24;
		uint32_t sad = (ra << 1) & 0x1fff;
		int len = (ra >> 20) & 0x3ff;
		
		if(!len) len = 25;
		
		for(int y = ytop; y < (ytop + len) && y < 25; y++) {
			for(int x = 0; x < 80; x++) {
				gdc_addr[y][x] = sad;
				sad = (sad + 2) & 0x1fff;
			}
		}
		if((ytop += len) >= 25) break;
	}
	uint32_t *addr = &gdc_addr[0][0];
	uint32_t *addr2 = addr + 160 * (sur + sdr);
	
	uint32_t cursor_addr = d_gdc_chr->cursor_addr(0x1fff);
	int cursor_top = d_gdc_chr->cursor_top();
	int cursor_bottom = d_gdc_chr->cursor_bottom();
#if defined(SUPPORT_HIRESO)
	cursor_top <<= 1;
	cursor_bottom <<= 1;
#endif
	bool attr_blink = d_gdc_chr->attr_blink();
	
	// render
	int ysur = bl * sur;
	int ysdr = bl * (sur + sdr);
	int xofs = modereg1[MODE1_COLUMN] ? (FONT_WIDTH * 2) : FONT_WIDTH;
	int addrofs = modereg1[MODE1_COLUMN] ? 2 : 1;
	
	memset(screen_chr, 0, sizeof(screen_chr));
	
	for(int y = 0, cy = 0, ytop = 0; y < SCREEN_HEIGHT && cy < 25; y += bl, cy++) {
		uint32_t gaiji1st = 0, last = 0, offset;
		int kanji2nd = 0;
		if(y == ysur) {
			ytop = y;
			y -= ssl;
			ysur = SCREEN_HEIGHT;
		}
		if(y >= ysdr) {
			y = ytop = ysdr;
			addr = addr2;
			ysdr = SCREEN_HEIGHT;
		}
		for(int x = 0, cx = 0; x < SCREEN_WIDTH && cx < 80; x += xofs, cx++) {
			uint16_t code = *(uint16_t *)(tvram + (*addr));
			uint8_t attr = tvram[(*addr) | 0x2000];
			uint8_t color = (attr & ATTR_COL) ? (attr >> 5) : 8;
			bool cursor = ((*addr) == cursor_addr);
			addr += addrofs;
			if(kanji2nd) {
				kanji2nd = 0;
				offset = last + KANJI_2ND_OFS;
			} else if(code & 0xff00) {
				uint16_t lo = code & 0x7f;
				uint16_t hi = (code >> 8) & 0x7f;
				offset = FONT_SIZE * (lo | (hi << 8));
				if(lo == 0x56 || lo == 0x57) {
					offset += gaiji1st;
					gaiji1st = gaiji1st ? 0 : KANJI_2ND_OFS;
				} else {
					uint16_t lo = code & 0xff;
					if(lo < 0x09 || lo >= 0x0c) {
						kanji2nd = 1;
					}
					gaiji1st = 0;
				}
			} else {
				offset = ANK_FONT_OFS + FONT_SIZE * (code & 0xff);
#if !defined(SUPPORT_HIRESO)
				if((attr & ATTR_VL) && modereg1[MODE1_ATRSEL]) {
					offset += FONT_SIZE * 0x100;
				}
				if(!modereg1[MODE1_FONTSEL]) {
					offset += FONT_SIZE * 0x200;
				}
#endif
				gaiji1st = 0;
			}
			last = offset;
			
			for(int l = 0; l < cl && l < FONT_HEIGHT; l++) {
				int yy = y + l + pl;
				if(yy >= ytop && yy < SCREEN_HEIGHT) {
					uint8_t *dest = &screen_chr[yy][x];
#if !defined(SUPPORT_HIRESO)
					uint8_t pattern = font[offset + l];
#else
					uint16_t pattern = *(uint16_t *)(&font[offset + l * 2]);
#endif
					if(!(attr & ATTR_ST)) {
						pattern = 0;
					} else if(((attr & ATTR_BL) && attr_blink) || (attr & ATTR_RV)) {
						pattern = ~pattern;
					}
					if((attr & ATTR_UL) && l == (FONT_HEIGHT - 1)) {
#if !defined(SUPPORT_HIRESO)
						pattern = 0xff;
#else
						pattern = 0x3ff;
#endif
					}
					if((attr & ATTR_VL) && !modereg1[MODE1_ATRSEL]) {
#if !defined(SUPPORT_HIRESO)
						pattern |= 0x08;
#else
						pattern |= 0x40;
#endif
					}
					if(cursor && l >= cursor_top && l < cursor_bottom) {
						pattern = ~pattern;
					}
					if(modereg1[MODE1_COLUMN]) {
#if !defined(SUPPORT_HIRESO)
						if(pattern & 0x80) dest[ 0] = dest[ 1] = color;
						if(pattern & 0x40) dest[ 2] = dest[ 3] = color;
						if(pattern & 0x20) dest[ 4] = dest[ 5] = color;
						if(pattern & 0x10) dest[ 6] = dest[ 7] = color;
						if(pattern & 0x08) dest[ 8] = dest[ 9] = color;
						if(pattern & 0x04) dest[10] = dest[11] = color;
						if(pattern & 0x02) dest[12] = dest[13] = color;
						if(pattern & 0x01) dest[14] = dest[15] = color;
#else
						if(pattern & 0x2000) dest[ 0] = dest[ 1] = color;
						if(pattern & 0x1000) dest[ 2] = dest[ 3] = color;
						if(pattern & 0x0800) dest[ 4] = dest[ 5] = color;
						if(pattern & 0x0400) dest[ 6] = dest[ 7] = color;
						if(pattern & 0x0200) dest[ 8] = dest[ 9] = color;
						if(pattern & 0x0100) dest[10] = dest[11] = color;
						if(pattern & 0x0080) dest[12] = dest[13] = color;
						if(pattern & 0x0040) dest[14] = dest[15] = color;
						if(pattern & 0x0020) dest[16] = dest[17] = color;
						if(pattern & 0x0010) dest[18] = dest[19] = color;
						if(pattern & 0x0008) dest[20] = dest[21] = color;
						if(pattern & 0x0004) dest[22] = dest[23] = color;
						if(pattern & 0x0002) dest[24] = dest[25] = color;
						if(pattern & 0x0001) dest[26] = dest[27] = color;
#endif
					} else {
#if !defined(SUPPORT_HIRESO)
						if(pattern & 0x80) dest[0] = color;
						if(pattern & 0x40) dest[1] = color;
						if(pattern & 0x20) dest[2] = color;
						if(pattern & 0x10) dest[3] = color;
						if(pattern & 0x08) dest[4] = color;
						if(pattern & 0x04) dest[5] = color;
						if(pattern & 0x02) dest[6] = color;
						if(pattern & 0x01) dest[7] = color;
#else
						if(pattern & 0x2000) dest[ 0] = color;
						if(pattern & 0x1000) dest[ 1] = color;
						if(pattern & 0x0800) dest[ 2] = color;
						if(pattern & 0x0400) dest[ 3] = color;
						if(pattern & 0x0200) dest[ 4] = color;
						if(pattern & 0x0100) dest[ 5] = color;
						if(pattern & 0x0080) dest[ 6] = color;
						if(pattern & 0x0040) dest[ 7] = color;
						if(pattern & 0x0020) dest[ 8] = color;
						if(pattern & 0x0010) dest[ 9] = color;
						if(pattern & 0x0008) dest[10] = color;
						if(pattern & 0x0004) dest[11] = color;
						if(pattern & 0x0002) dest[12] = color;
						if(pattern & 0x0001) dest[13] = color;
#endif
					}
				}
			}
		}
	}
}

void DISPLAY::draw_gfx_screen()
{
	// address from gdc
	uint32_t gdc_addr[SCREEN_HEIGHT][SCREEN_WIDTH >> 3] = {0};
	
	for(int i = 0, ytop = 0; i < 4; i++) {
		uint32_t ra = ra_gfx[i * 4];
		ra |= ra_gfx[i * 4 + 1] << 8;
		ra |= ra_gfx[i * 4 + 2] << 16;
		ra |= ra_gfx[i * 4 + 3] << 24;
		uint32_t sad = (ra << 1) & VRAM_PLANE_ADDR_MASK;
		int len = (ra >> 20) & 0x3ff;
		
		if(!len) len = SCREEN_HEIGHT; // Madou Monogatari 1-2-3
		
		for(int y = ytop; y < (ytop + len) && y < SCREEN_HEIGHT; y++) {
			for(int x = 0; x < (SCREEN_WIDTH >> 3); x++) {
				gdc_addr[y][x] = sad;
				sad = (sad + 1) & VRAM_PLANE_ADDR_MASK;
			}
		}
		if((ytop += len) >= SCREEN_HEIGHT) break;
	}
	uint32_t *addr = &gdc_addr[0][0];
	uint8_t *dest = &screen_gfx[0][0];
	
	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		for(int x = 0; x < SCREEN_WIDTH; x += 8) {
			uint8_t b = vram_disp_b[(*addr)];
			uint8_t r = vram_disp_r[(*addr)];
			uint8_t g = vram_disp_g[(*addr)];
#if defined(SUPPORT_16_COLORS)
			uint8_t e = vram_disp_e[(*addr)];
#else
			uint8_t e = 0;
#endif
			addr++;
			
			*dest++ = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5) | ((e & 0x80) >> 4);
			*dest++ = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4) | ((e & 0x40) >> 3);
			*dest++ = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3) | ((e & 0x20) >> 2);
			*dest++ = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2) | ((e & 0x10) >> 1);
			*dest++ = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1) | ((e & 0x08)     );
			*dest++ = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04)     ) | ((e & 0x04) << 1);
			*dest++ = ((b & 0x02) >> 1) | ((r & 0x02)     ) | ((g & 0x02) << 1) | ((e & 0x02) << 2);
			*dest++ = ((b & 0x01)     ) | ((r & 0x01) << 1) | ((g & 0x01) << 2) | ((e & 0x01) << 3);
		}
		if((cs_gfx[0] & 0x1f) == 1) {
			// 200 line
			if(modereg1[MODE1_200LINE]) {
				memset(dest, 0, SCREEN_WIDTH);
			} else {
				memcpy(dest, dest - SCREEN_WIDTH, SCREEN_WIDTH);
			}
			dest += SCREEN_WIDTH;
			y++;
		}
	}
}

#define STATE_VERSION	3

void DISPLAY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(tvram, sizeof(tvram), 1);
	state_fio->Fwrite(vram, sizeof(vram), 1);
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	state_fio->FputUint8(vram_disp_sel);
	state_fio->FputUint8(vram_draw_sel);
#endif
	state_fio->Fwrite(palette_gfx8, sizeof(palette_gfx8), 1);
	state_fio->Fwrite(digipal, sizeof(digipal), 1);
#if defined(SUPPORT_16_COLORS)
	state_fio->Fwrite(palette_gfx16, sizeof(palette_gfx16), 1);
	state_fio->Fwrite(anapal, sizeof(anapal), 1);
	state_fio->FputUint8(anapal_sel);
#endif
	state_fio->FputUint8(crtv);
	state_fio->Fwrite(scroll, sizeof(scroll), 1);
	state_fio->Fwrite(modereg1, sizeof(modereg1), 1);
#if defined(SUPPORT_16_COLORS)
	state_fio->Fwrite(modereg2, sizeof(modereg2), 1);
#endif
#if defined(SUPPORT_GRCG)
	state_fio->FputUint8(grcg_mode);
	state_fio->FputUint8(grcg_tile_ptr);
	state_fio->Fwrite(grcg_tile, sizeof(grcg_tile), 1);
#endif
#if defined(SUPPORT_EGC)
	state_fio->FputUint16(egc_access);
	state_fio->FputUint16(egc_fgbg);
	state_fio->FputUint16(egc_ope);
	state_fio->FputUint16(egc_fg);
	state_fio->FputUint16(egc_mask.w);
	state_fio->FputUint16(egc_bg);
	state_fio->FputUint16(egc_sft);
	state_fio->FputUint16(egc_leng);
	state_fio->FputUint64(egc_lastvram.q);
	state_fio->FputUint64(egc_patreg.q);
	state_fio->FputUint64(egc_fgc.q);
	state_fio->FputUint64(egc_bgc.q);
	state_fio->FputInt32(egc_func);
	state_fio->FputUint32(egc_remain);
	state_fio->FputUint32(egc_stack);
	int inptr_ofs = egc_inptr - egc_buf;
	int outptr_ofs = egc_outptr - egc_buf;
	state_fio->FputInt32(inptr_ofs);
	state_fio->FputInt32(outptr_ofs);
	state_fio->FputUint16(egc_mask2.w);
	state_fio->FputUint16(egc_srcmask.w);
	state_fio->FputUint8(egc_srcbit);
	state_fio->FputUint8(egc_dstbit);
	state_fio->FputUint8(egc_sft8bitl);
	state_fio->FputUint8(egc_sft8bitr);
	state_fio->Fwrite(egc_buf, sizeof(egc_buf), 1);
	state_fio->FputUint64(egc_vram_src.q);
	state_fio->FputUint64(egc_vram_data.q);
#endif
	state_fio->FputUint16(font_code);
	state_fio->FputUint8(font_line);
//	state_fio->FputUint16(font_lr);
}

bool DISPLAY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(tvram, sizeof(tvram), 1);
	state_fio->Fread(vram, sizeof(vram), 1);
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	vram_disp_sel = state_fio->FgetUint8();
	vram_draw_sel = state_fio->FgetUint8();
#endif
	state_fio->Fread(palette_gfx8, sizeof(palette_gfx8), 1);
	state_fio->Fread(digipal, sizeof(digipal), 1);
#if defined(SUPPORT_16_COLORS)
	state_fio->Fread(palette_gfx16, sizeof(palette_gfx16), 1);
	state_fio->Fread(anapal, sizeof(anapal), 1);
	anapal_sel = state_fio->FgetUint8();
#endif
	crtv = state_fio->FgetUint8();
	state_fio->Fread(scroll, sizeof(scroll), 1);
	state_fio->Fread(modereg1, sizeof(modereg1), 1);
#if defined(SUPPORT_16_COLORS)
	state_fio->Fread(modereg2, sizeof(modereg2), 1);
#endif
#if defined(SUPPORT_GRCG)
	grcg_mode = state_fio->FgetUint8();
	grcg_tile_ptr = state_fio->FgetUint8();
	state_fio->Fread(grcg_tile, sizeof(grcg_tile), 1);
#endif
#if defined(SUPPORT_EGC)
	egc_access = state_fio->FgetUint16();
	egc_fgbg = state_fio->FgetUint16();
	egc_ope = state_fio->FgetUint16();
	egc_fg = state_fio->FgetUint16();
	egc_mask.w = state_fio->FgetUint16();
	egc_bg = state_fio->FgetUint16();
	egc_sft = state_fio->FgetUint16();
	egc_leng = state_fio->FgetUint16();
	egc_lastvram.q = state_fio->FgetUint64();
	egc_patreg.q = state_fio->FgetUint64();
	egc_fgc.q = state_fio->FgetUint64();
	egc_bgc.q = state_fio->FgetUint64();
	egc_func = state_fio->FgetInt32();
	egc_remain = state_fio->FgetUint32();
	egc_stack = state_fio->FgetUint32();
	int inptr_ofs = state_fio->FgetInt32();
	int outptr_ofs = state_fio->FgetInt32();
	egc_inptr = egc_buf + inptr_ofs;
	egc_outptr = egc_buf + outptr_ofs;
	egc_mask2.w = state_fio->FgetUint16();
	egc_srcmask.w = state_fio->FgetUint16();
	egc_srcbit = state_fio->FgetUint8();
	egc_dstbit = state_fio->FgetUint8();
	egc_sft8bitl = state_fio->FgetUint8();
	egc_sft8bitr = state_fio->FgetUint8();
	state_fio->Fread(egc_buf, sizeof(egc_buf), 1);
	egc_vram_src.q = state_fio->FgetUint64();
	egc_vram_data.q = state_fio->FgetUint64();
#endif
	font_code = state_fio->FgetUint16();
	font_line = state_fio->FgetUint8();
//	font_lr = state_fio->FgetUint16();
	
	// post process
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	if(vram_disp_sel & 1) {
		vram_disp_b = vram + 0x28000;
		vram_disp_r = vram + 0x30000;
		vram_disp_g = vram + 0x38000;
#if defined(SUPPORT_16_COLORS)
		vram_disp_e = vram + 0x20000;
#endif
	} else {
		vram_disp_b = vram + 0x08000;
		vram_disp_r = vram + 0x10000;
		vram_disp_g = vram + 0x18000;
#if defined(SUPPORT_16_COLORS)
		vram_disp_e = vram + 0x00000;
#endif
	}
	if(vram_draw_sel & 1) {
		vram_draw = vram + 0x20000;
	} else {
		vram_draw = vram + 0x00000;
	}
#endif
	return true;
}

