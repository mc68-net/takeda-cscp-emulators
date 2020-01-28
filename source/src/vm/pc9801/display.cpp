/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ display ]
*/

#include "display.h"
#include "../i8259.h"
#include "../upd7220.h"
#include "../../fileio.h"

#define SCROLL_PL	0
#define SCROLL_BL	1
#define SCROLL_CL	2
#define SCROLL_SSL	3
#define SCROLL_SUR	4
#define SCROLL_SDR	5

#define MODE_ATRSEL	0
#define MODE_GRAPHIC	1
#define MODE_COLUMN	2
#define MODE_FONTSEL	3
#define MODE_200LINE	4
#define MODE_KAC	5
#define MODE_MEMSW	6
#define MODE_DISP	7

#define ATTR_ST		0x01
#define ATTR_BL		0x02
#define ATTR_RV		0x04
#define ATTR_UL		0x08
#define ATTR_VL		0x10
#define ATTR_COL	0xe0

static const uint8 memsw_default[] = {
	0xe1, 0x48, 0xe1, 0x05, 0xe1, 0x04, 0xe1, 0x00,
	0xe1, 0x01, 0xe1, 0x00, 0xe1, 0x00, 0xe1, 0x00,
};

void DISPLAY::initialize()
{
	// clear font
	uint8 *p = font + 0x81000;
	uint8 *q = font + 0x82000;
	for(int i = 0; i < 256; i++) {
		q += 8;
		for(int j = 0; j < 4; j++) {
			uint32 bit = 0;
			if(i & (1 << j)) {
				bit |= 0xf0f0f0f0;
			}
			if(i & (0x10 << j)) {
				bit |= 0x0f0f0f0f;
			}
			*(uint32 *)p = bit;
			p += 4;
			*(uint16 *)q = (uint16)bit;
			q += 2;
		}
	}
	for(int i = 0; i < 0x80; i++) {
		q = font + (i << 12);
		_memset(q + 0x000, 0, 0x0560 - 0x000);
		_memset(q + 0x580, 0, 0x0d60 - 0x580);
		_memset(q + 0xd80, 0, 0x1000 - 0xd80);
	}
	
	// load font data
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sFONT.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		uint8 *buf = (uint8 *)malloc(0x46800);
		fio->Fread(buf, 0x46800, 1);
		fio->Fclose();
		
		// 8x8 font
		uint8 *dst = font + 0x82000;
		uint8 *src = buf;
		int cnt = 256;
		while(cnt--) {
			_memcpy(dst, src, 8);
			dst += 16;
			src += 8;
		}
		// 8x16 font
		_memcpy(font + 0x80000, buf + 0x0800, 16 * 128);
		_memcpy(font + 0x80800, buf + 0x1000, 16 * 128);
		// kanji font
		kanji_copy(font, buf, 0x01, 0x30);
		kanji_copy(font, buf, 0x30, 0x56);
		kanji_copy(font, buf, 0x58, 0x5d);
		
		free(buf);
	}
	delete fio;
	
	// init palette
	for(int i = 0; i < 8; i++) {
		palette_chr[i] = RGB_COLOR((i & 2) ? 0xff : 0, (i & 4) ? 0xff : 0, (i & 1) ? 0xff : 0);
		palette_gfx[i] = 0;//RGB_COLOR((i & 2) ? 0xff : 0, (i & 4) ? 0xff : 0, (i & 1) ? 0xff : 0);
	}
	
	digipal[0] = 0;//(3 << 4) | 7;
	digipal[1] = 0;//(1 << 4) | 5;
	digipal[2] = 0;//(2 << 4) | 6;
	digipal[3] = 0;//(0 << 4) | 4;
	
	_memset(tvram, 0, sizeof(tvram));
	_memset(vram, 0, sizeof(vram));
	
	for(int i = 0; i < 16; i++) {
		tvram[0x3fe0 + (i << 1)] = (i == 7 && sound_bios_ok) ? 0x08 : memsw_default[i];
	}
	
	// regist event
	vm->regist_frame_event(this);
}

void DISPLAY::kanji_copy(uint8 *dst, uint8 *src, int from, int to)
{
	for(int i = from; i < to; i++) {
		uint8 *p = src + 0x1800 + (0x60 * 32 * (i - 1));
		uint8 *q = dst + 0x20000 + (i << 4);
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

void DISPLAY::reset()
{
	vram_disp_b = vram + 0x08000;
	vram_disp_r = vram + 0x10000;
	vram_disp_g = vram + 0x18000;
	vram_draw   = vram + 0x08000;
	
	d_gdc_chr->set_vram_ptr(tvram, 0x2000);
	d_gdc_gfx->set_vram_ptr(vram, 0x20000);
	
	crtv = 2;
	
	scroll[SCROLL_PL ] = 0;
	scroll[SCROLL_BL ] = 0x0f;
	scroll[SCROLL_CL ] = 0x10;
	scroll[SCROLL_SSL] = 0;
	scroll[SCROLL_SUR] = 0;
	scroll[SCROLL_SDR] = 24;
	
	_memset(mode_flipflop, 0, sizeof(mode_flipflop));
	
	font_code = 0;
	font_line = 0;
	font_lr = 0;
}

void DISPLAY::event_frame()
{
	if(crtv > 1) {
		// dont raise irq at first frame
		crtv--;
	}
	else if(crtv == 1) {
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR2, 1, 1);
		crtv = 0;
	}
}

void DISPLAY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xffff) {
	case 0x64:
		crtv = 1;
		break;
	case 0x68:
		mode_flipflop[(data >> 1) & 7] = data & 1;
		break;
	case 0x6c:
//		border = (data >> 3) & 7;
		break;
	case 0x6e:
//		border = (data >> 3) & 7;
#ifndef _PC9801
//		horiz_freq = data & 1;
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
#ifndef _PC9801
	// vram select
	case 0xa4:
		if(data & 1) {
			vram_disp_b = vram + 0x28000;
			vram_disp_r = vram + 0x30000;
			vram_disp_g = vram + 0x38000;
		}
		else {
			vram_disp_b = vram + 0x08000;
			vram_disp_r = vram + 0x10000;
			vram_disp_g = vram + 0x18000;
		}
		break;
	case 0xa6:
		if(data & 1) {
			vram_draw = vram + 0x28000;
			d_gdc_gfx->set_vram_ptr(vram + 0x20000, 0x20000);
		}
		else {
			vram_draw = vram + 0x08000;
			d_gdc_gfx->set_vram_ptr(vram, 0x20000);
		}
		break;
#endif
	// palette
	case 0xa8:
		digipal[0] = data;
		palette_gfx[7] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx[3] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0xaa:
		digipal[1] = data;
		palette_gfx[5] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx[1] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0xac:
		digipal[2] = data;
		palette_gfx[6] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx[2] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0xae:
		digipal[3] = data;
		palette_gfx[4] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx[0] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	// cg window
	case 0xa1:
		font_code = (data << 8) | (font_code & 0xff);
		break;
	case 0xa3:
		font_code = (font_code & 0xff00) | data;
		break;
	case 0xa5:
		font_line = data & 0x1f;
		font_lr = ((~data) & 0x20) << 6;
		break;
	case 0xa9:
		if((font_code & 0x7e) == 0x56) {
			font[((font_code & 0x7f7f) << 4) + font_lr + (font_line & 0x0f)] = data;
		}
		break;
	}
}

uint32 DISPLAY::read_io8(uint32 addr)
{
	switch(addr & 0xffff) {
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
	case 0xa9:
		if((font_code & 0xff) >= 0x09 && (font_code & 0xff) < 0x0c) {
			if(!font_lr) {
				return font[((font_code & 0x7f7f) << 4) + (font_line & 0x0f)];
			}
		}
		else if(font_code & 0xff00) {
			return font[((font_code & 0x7f7f) << 4) + font_lr + (font_line & 0x0f)];
		}
		else if(!(font_line & 0x10)) {
			return font[0x80000 + (font_code << 4) + font_line];
		}
		return 0;
	}
	return 0xff;
}

void DISPLAY::write_memory_mapped_io8(uint32 addr, uint32 data)
{
	if(0xa0000 <= addr && addr < 0xa3fe2) {
		tvram[addr - 0xa0000] = data;
	}
	else if(0xa3fe2 <= addr && addr < 0xa4000) {
		// memory switch
		if(mode_flipflop[MODE_MEMSW]) {
			tvram[addr - 0xa0000] = data;
		}
	}
	else if(0xa8000 <= addr && addr < 0xc0000) {
		vram_draw[addr - 0xa8000] = data;
	}
}

uint32 DISPLAY::read_memory_mapped_io8(uint32 addr)
{
	if(0xa0000 <= addr && addr < 0xa2000) {
		return tvram[addr - 0xa0000];
	}
	else if(0xa2000 <= addr && addr < 0xa4000) {
		if(addr & 1) {
			return 0xff;
		}
		return tvram[addr - 0xa0000];
	}
	else if(0xa8000 <= addr && addr < 0xc0000) {
		return vram_draw[addr - 0xa8000];
	}
	return 0xff;
}

void DISPLAY::draw_screen()
{
	// render screen
	if(mode_flipflop[MODE_DISP]) {
		draw_chr_screen();
		draw_gfx_screen();
		
		for(int y = 0; y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_chr = screen_chr[y];
			uint8 *src_gfx = screen_gfx[y];
			
			for(int x = 0; x < 640; x++) {
				uint8 chr = src_chr[x];
				dest[x] = chr ? palette_chr[chr & 7] : palette_gfx[src_gfx[x]];
			}
		}
	}
	else {
		for(int y = 0; y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			_memset(dest, 0, 640 * sizeof(scrntype));
		}
	}
	
	// access lamp
	uint32 stat_f = (d_fdc_2hd->read_signal(0) & 3) | ((d_fdc_2dd->read_signal(0) & 3) << 2);
	if(stat_f) {
		scrntype col = (stat_f & (1 | 4)) ? RGB_COLOR(255, 0, 0) :
		               (stat_f & (2 | 8)) ? RGB_COLOR(0, 255, 0) : 0;
		for(int y = 400 - 8; y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			for(int x = 640 - 8; x < 640; x++) {
				dest[x] = col;
			}
		}
	}
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
	int ssl = scroll[SCROLL_SSL];
	int sur = scroll[SCROLL_SUR] & 31;
	if(sur) {
		sur = 32 - sur;
	}
	int sdr = scroll[SCROLL_SDR] + 1;
	
	// address from gdc
	_memset(gdc_addr, 0, sizeof(gdc_addr));
	for(int i = 0, ytop = 0; i < 4; i++) {
		uint32 ra = ra_chr[i * 4];
		ra |= ra_chr[i * 4 + 1] << 8;
		ra |= ra_chr[i * 4 + 2] << 16;
		ra |= ra_chr[i * 4 + 3] << 24;
		uint32 sad = (ra << 1) & 0x1fff;
		int len = (ra >> 20) & 0x3ff;
		
		for(int y = ytop; y < (ytop + len) && y < 25; y++) {
			for(int x = 0; x < 80; x++) {
				gdc_addr[y][x] = sad;
				sad = (sad + 2) & 0x1fff;
			}
		}
		ytop += len;
	}
	uint32 *addr = &gdc_addr[0][0];
	uint32 *addr2 = addr + 160 * (sur + sdr);
	
	uint32 cursor_addr = d_gdc_chr->cursor_addr(0x1fff);
	int cursor_top = d_gdc_chr->cursor_top();
	int cursor_bottom = d_gdc_chr->cursor_bottom();
	bool attr_blink = d_gdc_chr->attr_blink();
	
	// render
	int ysur = bl * sur;
	int ysdr = bl * (sur + sdr);
	int xofs = mode_flipflop[MODE_COLUMN] ? 16 : 8;
	int addrofs = mode_flipflop[MODE_COLUMN] ? 2 : 1;
	
	_memset(screen_chr, 0, sizeof(screen_chr));
	
	for(int y = 0, ytop = 0; y < 400; y += bl) {
		uint32 gaiji1st = 0, last = 0, offset;
		int kanji2nd = 0;
		if(y == ysur) {
			ytop = y;
			y -= ssl;
			ysur = 400;
		}
		if(y >= ysdr) {
			y = ytop = ysdr;
			addr = addr2;
			ysdr = 400;
		}
		for(int x = 0; x < 640; x += xofs) {
			uint32 ofs = *addr;
			uint16 code = *(uint16 *)(tvram + *addr);
			uint8 attr = tvram[*addr | 0x2000];
			uint8 color = (attr & ATTR_COL) ? (attr >> 5) : 8;
			bool cursor = (*addr == cursor_addr);
			addr += addrofs;
			if(kanji2nd) {
				kanji2nd = 0;
				offset = last + 0x800;
			}
			else if(code & 0xff00) {
				uint16 lo = code & 0x7f;
				uint16 hi = (code >> 8) & 0x7f;
				offset = (lo << 4) | (hi << 12);
				if(lo == 0x56 || lo == 0x57) {
					offset += gaiji1st;
					gaiji1st ^= 0x800;
				}
				else {
					uint16 lo = code & 0xff;
					if(lo < 0x09 || lo >= 0x0c) {
						kanji2nd = 1;
					}
					gaiji1st = 0;
				}
			}
			else {
				uint16 lo = code & 0xff;
				if(mode_flipflop[MODE_FONTSEL]) {
					offset = 0x80000 | (lo << 4);
				}
				else {
					offset = 0x82000 | (lo << 4);
				}
				gaiji1st = 0;
			}
			last = offset;
			
			for(int l = 0; l < cl && l < 16; l++) {
				int yy = y + l + pl;
				if(yy >= ytop && yy < 400) {
					uint8 *dest = &screen_chr[yy][x];
					uint8 pattern = font[offset + l];
					if(!(attr & ATTR_ST)) {
						pattern = 0;
					}
					else if(((attr & ATTR_BL) && attr_blink) || (attr & ATTR_RV)) {
						pattern = ~pattern;
					}
					if((attr & ATTR_UL) && l == 15) {
						pattern = 0xff;
					}
					if(attr & ATTR_VL) {
						pattern |= 0x08;
					}
					if(cursor && l >= cursor_top && l < cursor_bottom) {
						pattern = ~pattern;
					}
					if(mode_flipflop[MODE_COLUMN]) {
						if(pattern & 0x80) dest[ 0] = dest[ 1] = color;
						if(pattern & 0x40) dest[ 2] = dest[ 3] = color;
						if(pattern & 0x20) dest[ 4] = dest[ 5] = color;
						if(pattern & 0x10) dest[ 6] = dest[ 7] = color;
						if(pattern & 0x08) dest[ 8] = dest[ 9] = color;
						if(pattern & 0x04) dest[10] = dest[11] = color;
						if(pattern & 0x02) dest[12] = dest[13] = color;
						if(pattern & 0x01) dest[14] = dest[15] = color;
					}
					else {
						if(pattern & 0x80) dest[0] = color;
						if(pattern & 0x40) dest[1] = color;
						if(pattern & 0x20) dest[2] = color;
						if(pattern & 0x10) dest[3] = color;
						if(pattern & 0x08) dest[4] = color;
						if(pattern & 0x04) dest[5] = color;
						if(pattern & 0x02) dest[6] = color;
						if(pattern & 0x01) dest[7] = color;
					}
				}
			}
		}
	}
}

void DISPLAY::draw_gfx_screen()
{
	// address from gdc
	_memset(gdc_addr, 0, sizeof(gdc_addr));
	for(int i = 0, ytop = 0; i < 4; i++) {
		uint32 ra = ra_gfx[i * 4];
		ra |= ra_gfx[i * 4 + 1] << 8;
		ra |= ra_gfx[i * 4 + 2] << 16;
		ra |= ra_gfx[i * 4 + 3] << 24;
		uint32 sad = (ra << 1) & 0x7fff;
		int len = (ra >> 20) & 0x3ff;
		
		for(int y = ytop; y < (ytop + len) && y < 400; y++) {
			for(int x = 0; x < 80; x++) {
				gdc_addr[y][x] = sad;
				sad = (sad + 1) & 0x7fff;
			}
		}
		ytop += len;
	}
	uint32 *addr = &gdc_addr[0][0];
	uint8 *dest = &screen_gfx[0][0];
	
	for(int y = 0; y < 400; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8 b = vram_disp_b[*addr];
			uint8 r = vram_disp_r[*addr];
			uint8 g = vram_disp_g[*addr];
			addr++;
			
			*dest++ = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
			*dest++ = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
			*dest++ = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
			*dest++ = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
			*dest++ = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
			*dest++ = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0);
			*dest++ = ((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1);
			*dest++ = ((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
		}
		if((cs_gfx[0] & 0x1f) == 1) {
			// 200 line
			if(mode_flipflop[MODE_200LINE]) {
				memset(dest, 0, 640);
			}
			else {
				memcpy(dest, dest - 640, 640);
			}
			dest += 640;
			y++;
		}
	}
}

