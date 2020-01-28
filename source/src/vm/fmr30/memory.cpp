/*
	FUJITSU FMR-30 Emulator 'eFMR-30'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ memory and crtc ]
*/

#include "memory.h"
#include "../../fileio.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) \
			wbank[i] = wdmy; \
		else \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		if((r) == rdmy) \
			rbank[i] = rdmy; \
		else \
			rbank[i] = (r) + 0x1000 * (i - sb); \
	} \
}

void MEMORY::initialize()
{
	// init memory
	_memset(ram, 0, sizeof(ram));
	_memset(vram, 0, sizeof(vram));
	_memset(cvram, 0, sizeof(cvram));
	_memset(kvram, 0, sizeof(kvram));
	_memset(ipl, 0xff, sizeof(ipl));
	_memset(kanji16, 0xff, sizeof(kanji16));
	_memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom image
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sIPL.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	else {
		// load pseudo ipl
		_memcpy(ipl + 0xc000, bios1, sizeof(bios1));
		_memcpy(ipl + 0xfff0, bios2, sizeof(bios2));
		
		// ank8/16
		_stprintf(file_path, _T("%sANK8.ROM"), app_path);
		if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
			fio->Fread(ipl, 0x800, 1);
			fio->Fclose();
		}
		_stprintf(file_path, _T("%sANK16.ROM"), app_path);
		if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
			fio->Fread(ipl + 0x800, 0x1000, 1);
			fio->Fclose();
		}
	}
	_stprintf(file_path, _T("%sKANJI16.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(kanji16, sizeof(kanji16), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory
	SET_BANK(0x000000, 0xffffff, wdmy, rdmy);
	SET_BANK(0x000000, sizeof(ram) - 1, ram, ram);
	SET_BANK(0xff0000, 0xffffff, wdmy, ipl);
	
	// regist event
	vm->regist_frame_event(this);
}

void MEMORY::reset()
{
	// reset crtc
	lcdadr = 0;
	_memset(lcdreg, 0, sizeof(lcdreg));
	dcr1 = dcr2 = 0;
	kj_l = kj_h = kj_ofs = kj_row = 0;
	blinkcnt = 0;
	
	// reset memory
	mcr1 = 2;
	mcr2 = a20 = 0;
	update_bank();
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffffff;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffffff;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_dma8(uint32 addr, uint32 data)
{
	write_data8(addr, data);
}

uint32 MEMORY::read_dma8(uint32 addr)
{
	return read_data8(addr);
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xffff)
	{
	// memory controller
	case 0x1d:
		mcr1 = data;
		update_bank();
		// protect mode ???
//		d_cpu->write_signal(did_a20, data, 0x10);
		break;
	case 0x1e:
		mcr2 = data;
		update_bank();
		break;
	case 0x26:
		a20 = data;
		// protect mode ???
		d_cpu->write_signal(did_a20, data, 0x80);
		break;
	// dma bank
	case 0x120:
	case 0x121:
	case 0x122:
	case 0x123:
		d_dma->write_signal(did_dma + (addr & 3), data, 0xf);
		break;
	// lcd controller
	case 0x300:
		lcdadr = data;
		break;
	case 0x302:
		lcdreg[lcdadr & 31] = data;
		break;
	case 0x308:
		dcr1 = (dcr1 & 0xff00) | data;
		break;
	case 0x309:
		dcr1 = (dcr1 & 0xff) | (data << 8);
		// bit11-10: vram bank
		update_bank();
		break;
	case 0x30a:
		dcr1 = (dcr1 & 0xff00) | data;
		break;
	case 0x30b:
		dcr1 = (dcr1 & 0xff) | (data << 8);
		break;
	// kanji rom
	case 0x30c:
		kj_h = data & 0x7f;
		break;
	case 0x30d:
		kj_l = data & 0x7f;
		kj_row = 0;
		if(kj_h < 0x30)
			kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) | (((kj_l - 0x20) & 0x20) <<  9) | (((kj_l - 0x20) & 0x40) <<  7) | (((kj_h - 0x00) & 0x07) << 10);
		else if(kj_h < 0x70)
			kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) + (((kj_l - 0x20) & 0x60) <<  9) + (((kj_h - 0x00) & 0x0f) << 10) + (((kj_h - 0x30) & 0x70) * 0xc00) + 0x08000;
		else
			kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) | (((kj_l - 0x20) & 0x20) <<  9) | (((kj_l - 0x20) & 0x40) <<  7) | (((kj_h - 0x00) & 0x07) << 10) | 0x38000;
		break;
	case 0x30e:
		kanji16[(kj_ofs | ((kj_row & 0xf) << 1)) & 0x3ffff] = data;
		break;
	case 0x30f:
		kanji16[(kj_ofs | ((kj_row++ & 0xf) << 1) | 1) & 0x3ffff] = data;
		break;
	}
}

uint32 MEMORY::read_io8(uint32 addr)
{
	uint32 val = 0xff;
	
	switch(addr & 0xffff)
	{
	case 0x1d:
		return mcr1;
	case 0x1e:
		return mcr2;
	case 0x26:
		return a20;
	// lcd controller
	case 0x300:
		return lcdadr;
	case 0x302:
		return lcdreg[lcdadr & 31];
	case 0x308:
		return dcr1 & 0xff;
	case 0x309:
		return (dcr1 >> 8) & 0xff;
	case 0x30a:
		return dcr2 & 0xff;
	case 0x30b:
		return (dcr2 >> 8) & 0xff;
	// kanji rom
	case 0x30c:
		return kj_h;
	case 0x30d:
		return kj_l;
	case 0x30e:
		return kanji16[(kj_ofs | ((kj_row & 0xf) << 1)) & 0x3ffff];
	case 0x30f:
		return kanji16[(kj_ofs | ((kj_row++ & 0xf) << 1) | 1) & 0x3ffff];
	}
	return 0xff;
}

void MEMORY::event_frame()
{
	blinkcnt++;
}

void MEMORY::update_bank()
{
	if(!(mcr2 & 1)) {
		// $c0000-$cffff: vram
		SET_BANK(0xc0000, 0xcffff, wdmy, rdmy);
		int bank = 0x8000 * ((dcr1 >> 10) & 3);
		SET_BANK(0xc0000, 0xc7fff, vram + bank, vram + bank);
		SET_BANK(0xc8000, 0xc8fff, cvram, cvram);
		SET_BANK(0xca000, 0xcafff, kvram, kvram);
	}
	else {
		SET_BANK(0xc0000, 0xcffff, ram + 0xc0000, ram + 0xc0000);
	}
	if(!(mcr1 & 1)) {
		// $f000-$ffff: rom
		SET_BANK(0xf0000, 0xfffff, wdmy, ipl);
	}
	else {
		SET_BANK(0xf0000, 0xfffff, ram + 0xf0000, ram + 0xf0000);
	}
}

void MEMORY::draw_screen()
{
	// render screen
	_memset(screen_txt, 0, sizeof(screen_txt));
	_memset(screen_cg, 0, sizeof(screen_cg));
	if(dcr1 & 2) {
		if(dcr1 & 8)
			draw_text40();
		else
			draw_text80();
	}
	if(dcr1 & 1)
		draw_cg();
	
	scrntype cd = RGB_COLOR(48, 56, 16);
	scrntype cb = RGB_COLOR(160, 168, 160);
	for(int y = 0; y < 400; y++) {
		scrntype* dest = emu->screen_buffer(y);
		uint8* txt = screen_txt[y];
		uint8* cg = screen_cg[y];
		
		for(int x = 0; x < 640; x++)
			dest[x] = (txt[x] || cg[x]) ? cd : cb;
	}
	
	// access lamp
	uint32 stat_f = d_fdc->read_signal(0) | d_bios->read_signal(0);
	if(stat_f) {
		scrntype col = (stat_f & 0x10   ) ? RGB_COLOR(0, 0, 255) :
		               (stat_f & (1 | 4)) ? RGB_COLOR(255, 0, 0) :
		               (stat_f & (2 | 8)) ? RGB_COLOR(0, 255, 0) : 0;
		for(int y = 400 - 8; y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			for(int x = 640 - 8; x < 640; x++)
				dest[x] = col;
		}
	}
}

void MEMORY::draw_text40()
{
	uint8 *ank8 = ipl;
	uint8 *ank16 = ipl + 0x800;
	
	int src = 0;//((lcdreg[12] << 9) | (lcdreg[13] << 1)) & 0xfff;
	int caddr = (lcdreg[10] & 0x20) ? -1 : (((lcdreg[14] << 9) | (lcdreg[15] << 1)) & 0xfff);
	int yofs = lcdreg[9] + 1;
	int ymax = 400 / yofs;
	int freq = (dcr1 >> 4) & 3;
	bool blink = !((freq == 3) || (blinkcnt & (32 >> freq)));
	
	for(int y = 0; y < ymax; y++) {
		for(int x = 0; x < 40; x++) {
			bool cursor = (src == caddr);
			int cx = x;
			uint8 code = cvram[src];
			uint8 h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8 attr = cvram[src];
			uint8 l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8 col = attr & 0x27;
			bool blnk = blink && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			
			if(attr & 0x40) {
				// kanji
				int ofs;
				if(h < 0x30)
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10);
				else if(h < 0x70)
					ofs = (((l - 0x00) & 0x1f) <<  5) + (((l - 0x20) & 0x60) <<  9) + (((h - 0x00) & 0x0f) << 10) + (((h - 0x30) & 0x70) * 0xc00) + 0x08000;
				else
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10) | 0x38000;
				
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8 pat0 = kanji16[ofs + (l << 1) + 0];
					uint8 pat1 = kanji16[ofs + (l << 1) + 1];
					pat0 = blnk ? 0 : rev ? ~pat0 : pat0;
					pat1 = blnk ? 0 : rev ? ~pat1 : pat1;
					int yy = y * yofs + l;
					if(yy >= 400)
						break;
					uint8* d = &screen_txt[yy][x << 4];
					
					d[ 0] = d[ 1] = (pat0 & 0x80) ? col : 0;
					d[ 2] = d[ 3] = (pat0 & 0x40) ? col : 0;
					d[ 4] = d[ 5] = (pat0 & 0x20) ? col : 0;
					d[ 6] = d[ 7] = (pat0 & 0x10) ? col : 0;
					d[ 8] = d[ 9] = (pat0 & 0x08) ? col : 0;
					d[10] = d[11] = (pat0 & 0x04) ? col : 0;
					d[12] = d[13] = (pat0 & 0x02) ? col : 0;
					d[14] = d[15] = (pat0 & 0x01) ? col : 0;
					d[16] = d[17] = (pat1 & 0x80) ? col : 0;
					d[18] = d[19] = (pat1 & 0x40) ? col : 0;
					d[20] = d[21] = (pat1 & 0x20) ? col : 0;
					d[22] = d[23] = (pat1 & 0x10) ? col : 0;
					d[24] = d[25] = (pat1 & 0x08) ? col : 0;
					d[26] = d[27] = (pat1 & 0x04) ? col : 0;
					d[28] = d[29] = (pat1 & 0x02) ? col : 0;
					d[30] = d[31] = (pat1 & 0x01) ? col : 0;
				}
				src = (src + 2) & 0xfff;
				x++;
			}
			else {
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8 pat = ank16[(code << 4) + l];
					pat = blnk ? 0 : rev ? ~pat : pat;
					int yy = y * yofs + l;
					if(yy >= 400)
						break;
					uint8* d = &screen_txt[yy][x << 4];
					
					d[ 0] = d[ 1] = (pat & 0x80) ? col : 0;
					d[ 2] = d[ 3] = (pat & 0x40) ? col : 0;
					d[ 4] = d[ 5] = (pat & 0x20) ? col : 0;
					d[ 6] = d[ 7] = (pat & 0x10) ? col : 0;
					d[ 8] = d[ 9] = (pat & 0x08) ? col : 0;
					d[10] = d[11] = (pat & 0x04) ? col : 0;
					d[12] = d[13] = (pat & 0x02) ? col : 0;
					d[14] = d[15] = (pat & 0x01) ? col : 0;
				}
			}
			if(cursor && !blink) {
				int st = lcdreg[10] & 0x1f;
				int ed = lcdreg[11] & 0x1f;
				for(int i = st; i <= ed && i < yofs; i++)
					_memset(&screen_txt[y * yofs + i][cx << 4], 7, 8);
			}
		}
	}
}

void MEMORY::draw_text80()
{
	uint8 *ank8 = ipl;
	uint8 *ank16 = ipl + 0x800;
	
	int src = 0;//((lcdreg[12] << 9) | (lcdreg[13] << 1)) & 0xfff;
	int caddr = (lcdreg[10] & 0x20) ? -1 : (((lcdreg[14] << 9) | (lcdreg[15] << 1)) & 0xfff);
	int yofs = lcdreg[9] + 1;
	int ymax = 400 / yofs;
	int freq = (dcr1 >> 4) & 3;
	bool blink = !((freq == 3) || (blinkcnt & (32 >> freq)));
	
	for(int y = 0; y < ymax; y++) {
		for(int x = 0; x < 80; x++) {
			bool cursor = (src == caddr);
			int cx = x;
			uint8 code = cvram[src];
			uint8 h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8 attr = cvram[src];
			uint8 l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8 col = attr & 0x27;
			bool blnk = blink && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			
			if(attr & 0x40) {
				// kanji
				int ofs;
				if(h < 0x30)
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10);
				else if(h < 0x70)
					ofs = (((l - 0x00) & 0x1f) <<  5) + (((l - 0x20) & 0x60) <<  9) + (((h - 0x00) & 0x0f) << 10) + (((h - 0x30) & 0x70) * 0xc00) + 0x08000;
				else
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10) | 0x38000;
				
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8 pat0 = kanji16[ofs + (l << 1) + 0];
					uint8 pat1 = kanji16[ofs + (l << 1) + 1];
					pat0 = blnk ? 0 : rev ? ~pat0 : pat0;
					pat1 = blnk ? 0 : rev ? ~pat1 : pat1;
					int yy = y * yofs + l;
					if(yy >= 400)
						break;
					uint8* d = &screen_txt[yy][x << 3];
					
					d[ 0] = (pat0 & 0x80) ? col : 0;
					d[ 1] = (pat0 & 0x40) ? col : 0;
					d[ 2] = (pat0 & 0x20) ? col : 0;
					d[ 3] = (pat0 & 0x10) ? col : 0;
					d[ 4] = (pat0 & 0x08) ? col : 0;
					d[ 5] = (pat0 & 0x04) ? col : 0;
					d[ 6] = (pat0 & 0x02) ? col : 0;
					d[ 7] = (pat0 & 0x01) ? col : 0;
					d[ 8] = (pat1 & 0x80) ? col : 0;
					d[ 9] = (pat1 & 0x40) ? col : 0;
					d[10] = (pat1 & 0x20) ? col : 0;
					d[11] = (pat1 & 0x10) ? col : 0;
					d[12] = (pat1 & 0x08) ? col : 0;
					d[13] = (pat1 & 0x04) ? col : 0;
					d[14] = (pat1 & 0x02) ? col : 0;
					d[15] = (pat1 & 0x01) ? col : 0;
				}
				src = (src + 2) & 0xfff;
				x++;
			}
			else {
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8 pat = ank16[(code << 4) + l];
					pat = blnk ? 0 : rev ? ~pat : pat;
					int yy = y * yofs + l;
					if(yy >= 400)
						break;
					uint8* d = &screen_txt[yy][x << 3];
					
					d[0] = (pat & 0x80) ? col : 0;
					d[1] = (pat & 0x40) ? col : 0;
					d[2] = (pat & 0x20) ? col : 0;
					d[3] = (pat & 0x10) ? col : 0;
					d[4] = (pat & 0x08) ? col : 0;
					d[5] = (pat & 0x04) ? col : 0;
					d[6] = (pat & 0x02) ? col : 0;
					d[7] = (pat & 0x01) ? col : 0;
				}
			}
			if(cursor && !blink) {
				int st = lcdreg[10] & 0x1f;
				int ed = lcdreg[11] & 0x1f;
				for(int i = st; i <= ed && i < yofs; i++)
					_memset(&screen_txt[y * yofs + i][cx << 3], 7, 8);
			}
		}
	}
}

void MEMORY::draw_cg()
{
	uint8* plane = vram + ((dcr1 >> 8) & 3) * 0x8000;
	int ptr = 0;
	
	for(int y = 0; y < 400; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8 pat = plane[ptr++];
			uint8* d = &screen_cg[y][x];
			
			d[0] = pat & 0x80;
			d[1] = pat & 0x40;
			d[2] = pat & 0x20;
			d[3] = pat & 0x10;
			d[4] = pat & 0x08;
			d[5] = pat & 0x04;
			d[6] = pat & 0x02;
			d[7] = pat & 0x01;
		}
	}
}
