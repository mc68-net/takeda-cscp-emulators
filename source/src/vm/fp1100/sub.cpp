/*
	CASIO FP-1100 Emulator 'eFP-1100'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.06.09-

	[ sub pcb ]
*/

#include "sub.h"
#include "../upd7801.h"
#include "../../config.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 7, eb = (e) >> 7; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} \
		else { \
			wbank[i] = (w) + 0x80 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} \
		else { \
			rbank[i] = (r) + 0x80 * (i - sb); \
		} \
	} \
}

void SUB::initialize()
{
	// init memory
	_memset(sub1, 0xff, sizeof(sub1));
	_memset(sub2, 0xff, sizeof(sub2));
	_memset(sub3, 0xff, sizeof(sub3));
	_memset(rdmy, 0xff, sizeof(rdmy));
	_memset(ram, 0, sizeof(ram));
	_memset(vram_b, 0, sizeof(vram_b));
	_memset(vram_r, 0, sizeof(vram_r));
	_memset(vram_g, 0, sizeof(vram_g));
	
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sSUB1.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(sub1, sizeof(sub1), 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sSUB2.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(sub2, sizeof(sub2), 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sSUB3.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(sub3, sizeof(sub3), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x0fff, wdmy, sub1);
	SET_BANK(0x1000, 0x1fff, wdmy, sub2);
	SET_BANK(0x2000, 0x5fff, vram_b, vram_b);
	SET_BANK(0x6000, 0x9fff, vram_r, vram_r);
	SET_BANK(0xa000, 0xdfff, vram_g, vram_g);
	SET_BANK(0xf000, 0xff7f, wdmy, sub3);	// 0xf400-
	SET_BANK(0xff80, 0xffff, ram, ram);
	
	// create palette
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	
	key_stat = emu->key_buffer();
	vm->regist_frame_event(this);
}

void SUB::reset()
{
	pa = pc = 0;
	key_sel = key_data = 0;
	color = 7;
	hsync = wait = false;
	cblink = 0;
}

void SUB::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	switch(addr & 0xfc00) {
	case 0xe000:
//emu->out_debug("SUB\tCRTC[%d]=%2x\n",addr&1,data);
		d_crtc->write_io8(addr, data);
		break;
	case 0xe400:
//emu->out_debug("SUB\tKEYSEL=%2x\n",data);
		key_sel = data;
		key_update();
		// bit4: buzzer
		d_beep->write_signal(did_beep, data, 0x10);
		break;
	case 0xe800:
//emu->out_debug("SUB->MAIN\tCOMM=%2x\n",data);
		d_main->write_signal(did_comm, data, 0xff);
		break;
	case 0xec00:
		break;
	case 0xf000:
//emu->out_debug("SUB\tCOLOR=%2x\n",addr&1,data);
		color = (data >> 4) & 7;
		break;
	default:
		if(0x2000 <= addr && addr < 0xe000) {
//emu->out_debug("SUB\tVRAM[%4x]=%2x\n",addr,data);
			if(!wait && hsync) {
				d_cpu->write_signal(did_wait, 1, 1);
				wait = true;
			}
			uint32 addr2 = (addr - 2000) & 0x3fff;
			if(color & 1) {
				vram_b[addr2] = ~data;
			}
			if(color & 2) {
				vram_r[addr2] = ~data;
			}
			if(color & 4) {
				vram_g[addr2] = ~data;
			}
			break;
		}
		wbank[addr >> 7][addr & 0x7f] = data;
		break;
	}
}

uint32 SUB::read_data8(uint32 addr)
{
	addr &= 0xffff;
	switch(addr & 0xfc00) {
	case 0xe000:
		return d_crtc->read_io8(addr);
	case 0xe400:
		return 0xfd; // dipswitch
	case 0xe800:
		return comm_data;
	case 0xec00:
	case 0xf000:
		return 0xff;
	default:
		if(0x2000 <= addr && addr < 0xe000) {
			if(!wait && hsync) {
				d_cpu->write_signal(did_wait, 1, 1);
				wait = true;
			}
		}
		return rbank[addr >> 7][addr & 0x7f];
	}
}

void SUB::write_io8(uint32 addr, uint32 data)
{
	switch(addr) {
	case P_A:
//emu->out_debug("SUB\tPA=%2x\n",data);
		pa = data;
		break;
	case P_B:
//emu->out_debug("SUB\tPB=%2x\n",data);
		// printer data
		pb = data;
		break;
	case P_C:
//emu->out_debug("SUB\tPC=%2x\n",data);
		//if((pc & 8) != (data & 8)) {
		if(!(pc & 8) && (data & 8)) {
//emu->out_debug("SUB->MAIN\tINTS=%d\n",(data&8)?1:0);
			d_main->write_signal(did_ints, data, 8);
		}
		pc = data;
		break;
	case P_SO:
		break;
	}
}

uint32 SUB::read_io8(uint32 addr)
{
	switch(addr) {
	case P_A:
		return pa;
	case P_B:
		if(key_sel & 0x20) {
//emu->out_debug("SUB\tKEYDAT=%2x\n",key_data);
			return key_data;
		}
//emu->out_debug("SUB\tKEYDAT=ff\n");
		return 0;//xff;
	case P_C:
		return pc;
	case P_SI:
		return 0xff;
	}
	return 0xff;
}

void SUB::write_signal(int id, uint32 data, uint32 mask)
{
	switch(id) {
	case SIG_SUB_INT2:
		// from main pcb
//emu->out_debug("SUB<-MAIN\tINT2=%d\n", (data&mask)?1:0);
		d_cpu->write_signal(did_int2, data, mask);
		break;
	case SIG_SUB_COMM:
		// from main pcb
		comm_data = data & 0xff;
//emu->out_debug("SUB<-MAIN\tCOMM=%2x\n", comm_data);
		break;
	case SIG_SUB_HSYNC:
		// from crtc
		hsync = ((data & mask) != 0);
		if(wait && !hsync) {
			d_cpu->write_signal(did_wait, 0, 0);
			wait = false;
		}
		break;
	}
}

void SUB::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void SUB::key_down(int code)
{
	key_update();
}

void SUB::key_up(int code)
{
	key_update();
}

// BREAK -> PAUSE
// STOP  -> END
// CLS   -> HOME
// KANA  -> ALT
// PF0   -> F10

static const int key_map[16][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x10, 0x11, 0x12, 0x14, 0x15, 0x00, 0x00, 0x13},
	{0x41, 0x1b, 0x6d, 0x51, 0x5a, 0x6a, 0x00, 0x70},
	{0x53, 0x31, 0x6b, 0x57, 0x58, 0x6f, 0x00, 0x71},
	{0x44, 0x32, 0x63, 0x45, 0x43, 0x2e, 0x6e, 0x72},
	{0x46, 0x33, 0x66, 0x52, 0x56, 0x27, 0x00, 0x73},
	{0x47, 0x34, 0x69, 0x54, 0x42, 0x2d, 0x20, 0x74},
	{0x48, 0x35, 0x68, 0x59, 0x4e, 0x28, 0x60, 0x75},
	{0x4a, 0x36, 0x65, 0x55, 0x4d, 0x26, 0x62, 0x76},
	{0x4b, 0x37, 0x64, 0x49, 0xbc, 0x24, 0x61, 0x77},
	{0x4c, 0x38, 0x67, 0x4f, 0xbe, 0x25, 0xdd, 0x78},
	{0xbb, 0x39, 0x0d, 0x50, 0xbf, 0x00, 0xdb, 0x79},
	{0xba, 0x30, 0xde, 0xc0, 0xe2, 0xdc, 0xbd, 0x23},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

void SUB::key_update()
{
	uint8 prev = key_data;
	key_data = 0;
	
	for(int i = 0; i < 8; i++) {
		if(key_stat[key_map[key_sel & 0xf][i]]) {
			key_data |= (1 << i);
		}
	}
	if((key_data & 0x80) != (prev & 0x80)) {
		d_cpu->write_signal(did_int0, key_data, 0x80);
	}
}

void SUB::draw_screen()
{
	// render screen
	int lmax = (regs[9] & 0x1f);
	int ymax = (regs[6] & 0x7f) * lmax;
	uint16 src = ((regs[12] << 11) | (regs[13] << 3)) & 0x3fff;
	uint16 cursor = ((regs[14] << 11) | (regs[15] << 3)) & 0x3fff;
	
	_memset(screen, 0, sizeof(screen));
	
	if((regs[8] & 0x30) != 0x30) {
		if(pa & 8) {
			// 40 column
			for(int y = 0; y < ymax && y < 400; y += lmax) {
				for(int x = 0; x < 640; x += 16) {
					for(int l = 0; l < lmax; l++) {
						uint16 src2 = src | (l & 7);
						uint8 b = (pa & 1) ? 0 : vram_b[src2];
						uint8 r = (pa & 2) ? 0 : vram_r[src2];
						uint8 g = (pa & 4) ? 0 : vram_g[src2];
						if(lmax > 8) {
							if(l < 8) {
								r = g = b;
							}
							else if(l < 16) {
								g = b = r;
							}
							else {
								b = r = g;
							}
						}
						else if(pa & 0x10) {
							b = r = g = b | r | g;
						}
						uint8* d = &screen[y + l][x];
						
						d[ 0] = d[ 1] = ((b & 0x01) << 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
						d[ 2] = d[ 3] = ((b & 0x02) >> 1) | ((r & 0x02) << 0) | ((g & 0x02) << 1);
						d[ 4] = d[ 5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) << 0);
						d[ 6] = d[ 7] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
						d[ 8] = d[ 9] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
						d[10] = d[11] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
						d[12] = d[13] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
						d[14] = d[15] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
					}
					if(src == cursor && (regs[8] & 0xc0) != 0xc0) {
						uint8 bp = regs[10] & 0x60;
						if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
							for(int l = (regs[10] & 0x1f); l < lmax; l++) {
								_memset(&screen[y + l][x], color, 16);
							}
						}
					}
					src = (src + 8) & 0x3fff;
				}
			}
		}
		else {
			// 80 column
			for(int y = 0; y < ymax && y < 400; y += lmax) {
				for(int x = 0; x < 640; x += 8) {
					for(int l = 0; l < lmax; l++) {
						uint16 src2 = src | (l & 7);
						uint8 b = (pa & 1) ? 0 : vram_b[src2];
						uint8 r = (pa & 2) ? 0 : vram_r[src2];
						uint8 g = (pa & 4) ? 0 : vram_g[src2];
						if(lmax > 8) {
							if(l < 8) {
								r = g = b;
							}
							else if(l < 16) {
								g = b = r;
							}
							else {
								b = r = g;
							}
						}
						else if(pa & 0x10) {
							b = r = g = b | r | g; // mono
						}
						uint8* d = &screen[y + l][x];
						
						d[0] = ((b & 0x01) << 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
						d[1] = ((b & 0x02) >> 1) | ((r & 0x02) << 0) | ((g & 0x02) << 1);
						d[2] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) << 0);
						d[3] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
						d[4] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
						d[5] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
						d[6] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
						d[7] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
					}
					if(src == cursor && (regs[8] & 0xc0) != 0xc0) {
						uint8 bp = regs[10] & 0x60;
						if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
							for(int l = (regs[10] & 0x1f); l < lmax; l++) {
								_memset(&screen[y + l][x], color, 8);
							}
						}
					}
					src = (src + 8) & 0x3fff;
				}
			}
		}
	}
	
	// copy to real screen
	if(ymax > 200) {
		// 400 line
		for(int y = 0; y < 400; y++) {
			scrntype* dest = emu->screen_buffer(y);
			uint8* src = screen[y];
			
			for(int x = 0; x < 640; x++) {
				dest[x] = palette_pc[src[x] & 7];
			}
		}
	}
	else {
		// 200 line
		for(int y = 0; y < 200; y++) {
			scrntype* dest0 = emu->screen_buffer(y * 2 + 0);
			scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
			uint8* src = screen[y];
			
			for(int x = 0; x < 640; x++) {
				dest0[x] = palette_pc[src[x] & 7];
			}
			if(config.scan_line) {
				_memset(dest1, 0, 640 * sizeof(scrntype));
			}
			else {
				_memcpy(dest1, dest0, 640 * sizeof(scrntype));
			}
		}
	}
}

void SUB::play_datarec(_TCHAR* filename)
{
	
}

void SUB::rec_datarec(_TCHAR* filename)
{
	
}

void SUB::close_datarec()
{
	
}
