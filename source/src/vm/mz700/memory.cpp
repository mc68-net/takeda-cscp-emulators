/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-1500 Emulator 'EmuZ-1500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ memory ]
*/

#include "memory.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../../fileio.h"

#define EVENT_TEMPO	0
#define EVENT_BLINK	1
#define EVENT_HBLANK	2

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} \
		else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} \
		else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	_memset(ram, 0, sizeof(ram));
	_memset(vram, 0, sizeof(vram));
	_memset(vram + 0x800, 0x71, 0x400);
	_memset(ipl, 0xff, sizeof(ipl));
#ifdef _MZ1500
	_memset(ext, 0xff, sizeof(ext));
#endif
	_memset(font, 0, sizeof(font));
	_memset(rdmy, 0xff, sizeof(rdmy));
	_memset(emm, 0, sizeof(emm));
	
	// load rom image
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sIPL.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
#ifdef _MZ1500
	_stprintf(file_path, _T("%sEXT.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
#endif
	_stprintf(file_path, _T("%sFONT.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sEMM.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(emm, sizeof(emm), 1);
		fio->Fclose();
	}
	delete fio;
	
	// init memory map
	SET_BANK(0x0000, 0xffff, ram, ram);
	
	// regist event
	vm->regist_vline_event(this);
	int id;
	vm->regist_event_by_clock(this, EVENT_TEMPO, CPU_CLOCKS / 64, true, &id);	// 32hz * 2
	vm->regist_event_by_clock(this, EVENT_BLINK, CPU_CLOCKS / 3, true, &id);	// 1.5hz * 2
}

void MEMORY::reset()
{
	// reset memory map
	mem_bank = 3;
	pcg_bank = 0;
	update_map_low();
	update_map_high();
	
	blink = tempo = false;
	hblank = false;
	// motor is always rotating...
	d_pio->write_signal(SIG_I8255_PORT_C, 0xff, 0x10);
	emm_ptr = 0;
}

void MEMORY::event_vline(int v, int clock)
{
	// vblank
	if(v == 0) {
		d_pio->write_signal(SIG_I8255_PORT_C, 0xff, 0x80);
	}
	else if(v == 200) {
		d_pio->write_signal(SIG_I8255_PORT_C, 0, 0x80);
	}
	
	// hblank
	hblank = true;
	int id;
	vm->regist_event_by_clock(this, EVENT_HBLANK, 165, false, &id);
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_TEMPO) {
		// 32khz
		tempo = !tempo;
	}
	else if(event_id == EVENT_BLINK) {
		// 1.5khz
		d_pio->write_signal(SIG_I8255_PORT_C, (blink = !blink) ? 0xff : 0, 0x40);
	}
	else if(event_id == EVENT_HBLANK) {
		hblank = false;
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
	}
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	if((mem_bank & 2) && !(mem_bank & 4)) {
		if(0xd000 <= addr && addr <= 0xdfff) {
			// vram wait
			if(hblank) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
			}
		}
		else if(0xe000 <= addr && addr <= 0xe7ff) {
			// memory mapped i/o
			switch(addr & 0x0f) {
			case 0: case 1: case 2: case 3:
				d_pio->write_io8(addr & 3, data);
				break;
			case 4: case 5: case 6: case 7:
				d_ctc->write_io8(addr & 3, data);
				break;
			case 8:
				// 8253 gate0
				d_ctc->write_signal(SIG_I8253_GATE_0, data, 1);
				break;
			}
			return;
		}
	}
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	if((mem_bank & 2) && !(mem_bank & 4)) {
		if(0xd000 <= addr && addr <= 0xdfff) {
			// vram wait
			if(hblank) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
			}
		}
		else if(0xe000 <= addr && addr <= 0xe7ff) {
			// memory mapped i/o
			switch(addr & 0x0f) {
			case 0: case 1: case 2: case 3:
				return d_pio->read_io8(addr & 3);
			case 4: case 5: case 6: case 7:
				return d_ctc->read_io8(addr & 3);
			case 8:
				return (hblank ? 0x80 : 0) | (tempo ? 1 : 0) | 0x7e;
			}
			return 0xff;
		}
	}
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_data8w(uint32 addr, uint32 data, int* wait)
{
	*wait = ((mem_bank & 1) && addr < 0x1000) ? 1 : 0;
	write_data8(addr, data);
}

uint32 MEMORY::read_data8w(uint32 addr, int* wait)
{
	*wait = ((mem_bank & 1) && addr < 0x1000) ? 1 : 0;
	return read_data8(addr);
}

void MEMORY::write_data16w(uint32 addr, uint32 data, int* wait)
{
	*wait = ((mem_bank & 1) && addr < 0x1000) ? 2 : 0;
	write_data8(addr, data & 0xff);
	write_data8(addr + 1, data >> 8);
}

uint32 MEMORY::read_data16w(uint32 addr, int* wait)
{
	*wait = ((mem_bank & 1) && addr < 0x1000) ? 2 : 0;
	return read_data8(addr) | (read_data8(addr + 1) << 8);
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x00:
		emm_ptr = (emm_ptr & 0xffff00) | data;
		emm_ptr &= EMM_MASK;
		break;
	case 0x01:
		emm_ptr = (emm_ptr & 0xff00ff) | (data << 8);
		emm_ptr &= EMM_MASK;
		break;
	case 0x02:
		emm_ptr = (emm_ptr & 0x00ffff) | (data << 16);
		emm_ptr &= EMM_MASK;
		break;
	case 0x03:
		emm[emm_ptr++] = data;
		emm_ptr &= EMM_MASK;
		break;
	case 0xe0:
		mem_bank &= ~1;
		update_map_low();
		break;
	case 0xe1:
		mem_bank &= ~2;
		update_map_high();
		break;
	case 0xe2:
		mem_bank |= 1;
		update_map_low();
		break;
	case 0xe3:
		mem_bank |= 2;
		update_map_high();
		break;
	case 0xe4:
		mem_bank |= 3;
		mem_bank &= ~4;
		update_map_low();
		update_map_high();
		break;
#ifdef _MZ1500
	case 0xe5:
		mem_bank |= 4;
		pcg_bank = data;
		update_map_high();
		break;
	case 0xe6:
		mem_bank &= ~4;
		update_map_high();
		break;
	case 0xe9:
		d_psg_l->write_io8(0, data);
		d_psg_r->write_io8(0, data);
		break;
#endif
	}
}

uint32 MEMORY::read_io8(uint32 addr)
{
	uint32 val;
	
	switch(addr & 0xff) {
	case 0x00:
		return emm_ptr & 0xff;
	case 0x01:
		return (emm_ptr >> 8) & 0xff;
	case 0x02:
		return (emm_ptr >> 16) & 0xff;
	case 0x03:
		val = emm[emm_ptr++];
		emm_ptr &= EMM_MASK;
		return val;
#ifndef _MZ1500
	case 0xf7:
		return 0x80;
#else
	case 0xfe:
		return 0xc0;
#endif
	}
	return 0xff;
}

void MEMORY::update_map_low()
{
	if(mem_bank & 1) {
		SET_BANK(0x0000, 0x0fff, wdmy, ipl);
	}
	else {
		SET_BANK(0x0000, 0x0fff, ram, ram);
	}
}

void MEMORY::update_map_high()
{
#ifdef _MZ1500
	if(mem_bank & 4) {
		if(pcg_bank & 3) {
			uint8 *bank = pcg + ((pcg_bank & 3) - 1) * 0x2000;
			SET_BANK(0xd000, 0xefff, bank, bank);
			SET_BANK(0xf000, 0xffff, wdmy, rdmy);
		}
		else {
			SET_BANK(0xd000, 0xdfff, wdmy, font);	// read only
			SET_BANK(0xe000, 0xffff, wdmy, rdmy);
		}
	}
	else
#endif
	if(mem_bank & 2) {
		SET_BANK(0xd000, 0xdfff, vram, vram);
#ifdef _MZ1500
		SET_BANK(0xe000, 0xe7ff, wdmy, rdmy);
		SET_BANK(0xe800, 0xffff, wdmy, ext );
#else
		SET_BANK(0xe000, 0xffff, wdmy, rdmy);
#endif
	}
	else {
		SET_BANK(0xd000, 0xffff, ram + 0xd000, ram + 0xd000);
	}
}

void MEMORY::open_mzt(_TCHAR* filename)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(filename, FILEIO_READ_BINARY)) {
		// init memory (???)
		_memcpy(ram, ipl, sizeof(ipl));
		mem_bank &= ~1;
		update_map_low();
		
		// load mzt file
//		fio->Fseek(0, FILEIO_SEEK_END);
//		int remain = fio->Ftell();
//		fio->Fseek(0, FILEIO_SEEK_SET);
//		bool first_block = true;
		
//		while(remain >= 128) {
			uint8 header[128];
			fio->Fread(header, sizeof(header), 1);
//			remain -= 128;
			
			int size = header[0x12] | (header[0x13] << 8);
			int offs = header[0x14] | (header[0x15] << 8);
			int addr = header[0x16] | (header[0x17] << 8);
			
//			if(first_block) {
//				first_block = false;
				vm->set_pc(addr);
//			}
//			if(remain >= size) {
				fio->Fread(ram + offs, size, 1);
//			}
//			remain -= size;
			
			// patch
			if(header[0x40] == 'P' && header[0x41] == 'A' && header[0x42] == 'T' && header[0x43] == ':') {
				int patch_ofs = 0x44;
				for(; patch_ofs < 0x80; ) {
					uint16 patch_addr = header[patch_ofs] | (header[patch_ofs + 1] << 8);
					patch_ofs += 2;
					if(patch_addr == 0xffff) {
						break;
					}
					int patch_len = header[patch_ofs++];
					for(int i = 0; i < patch_len; i++) {
						ram[patch_addr + i] = header[patch_ofs++];
					}
				}
			}
//		}
	}
}

