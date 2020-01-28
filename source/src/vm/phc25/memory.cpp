/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ memory ]
*/

#include "memory.h"
#include "../../fileio.h"

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
	// load ipl
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sBASIC.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
#ifdef _MAP1010
	SET_BANK(0x0000, 0x5fff, wdmy, rom );
	SET_BANK(0x6000, 0x77ff, vram, vram);
	SET_BANK(0x7800, 0x7fff, wdmy, rdmy);
	SET_BANK(0x8000, 0xffff, ram,  ram );
#else
	SET_BANK(0x0000, 0x5fff, wdmy, rom );
	SET_BANK(0x6000, 0x77ff, vram, vram);
	SET_BANK(0x7800, 0xbfff, wdmy, rdmy);
	SET_BANK(0xc000, 0xffff, ram,  ram );
#endif
}

void MEMORY::reset()
{
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
#ifdef _MAP1010
	if(0x7800 <= addr && addr < 0x8000) {
		// memory mapped i/o
//		emu->out_debug("UNKNOWN:\t%6x\tWM8\t%4x,%2x\n", vm->get_prv_pc(), addr, data);
		return;
	}
#endif
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
#ifdef _MAP1010
	if(0x7800 <= addr && addr < 0x7860) {
		uint32 data = d_kbd->read_io8(addr);
//		emu->out_debug("%6x\tRM8\t%4x,%2x\n", vm->get_prv_pc(), addr, data);
		return data;
	}
	else if(0x7860 <= addr && addr < 0x8000) {
		// memory mapped i/o
		uint32 data = 0xff;
//		emu->out_debug("UNKNOWN:\t%6x\tRM8\t%4x,%2x\n", vm->get_prv_pc(), addr, data);
		return data;
	}
#endif
	return rbank[addr >> 11][addr & 0x7ff];
}

