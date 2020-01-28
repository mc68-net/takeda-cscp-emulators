/*
	MITSUBISHI Elec. MULTI8 Emulator 'EmuLTI8'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ memory ]
*/

#include "memory.h"
#include "../../fileio.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		wbank[i] = (w) + 0x1000 * (i - sb); \
		rbank[i] = (r) + 0x1000 * (i - sb); \
	} \
}

void MEMORY::initialize()
{
	// init memory
	_memset(rom, 0xff, sizeof(rom));
	_memset(fdc, 0xff, sizeof(fdc));
	_memset(ram0, 0, sizeof(ram0));
	_memset(ram1, 0, sizeof(ram1));
	_memset(vram, 0, sizeof(vram));
	_memset(rdmy, 0xff, sizeof(rdmy));
	
	// load ipl
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sBASIC.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sFDC.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(fdc, sizeof(fdc), 1);
		fio->Fclose();
		
		// 8255 Port A, bit1 = 0 (fdc rom status)
		dev->write_signal(dev_id, 0, 0x02);
	}
	else
		// 8255 Port A, bit1 = 1 (fdc rom not status)
		dev->write_signal(dev_id, 0xffffffff, 0x02);
	delete fio;
}

void MEMORY::reset()
{
	map1 = 0xf;
	map2 = 0;
	update_map();
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	if((addr & 0xc000) == 0x8000 && (map1 & 0x10)) {
		uint32 ptr = addr & 0x3fff;
		// select vram
		if(!(map1 & 0x1))
			vram[0x0000 | ptr] = data;
		if(!(map1 & 0x2))
			vram[0x4000 | ptr] = data;
		if(!(map1 & 0x4))
			vram[0x8000 | ptr] = data;
		if(!(map1 & 0x8))
			vram[0xc000 | ptr] = data;
		return;
	}
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	if((addr & 0xc000) == 0x8000 && (map1 & 0x10)) {
		uint32 ptr = addr & 0x3fff;
		// select vram
		uint32 val = 0xff;
		if(!(map1 & 0x1))
			val &= vram[0x0000 | ptr];
		if(!(map1 & 0x2))
			val &= vram[0x4000 | ptr];
		if(!(map1 & 0x4))
			val &= vram[0x8000 | ptr];
		if(!(map1 & 0x8))
			val &= vram[0xc000 | ptr];
		return val;
	}
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	map2 = data;
	update_map();
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MEMORY_I8255_C) {
		map1 = data & mask;
		update_map();
	}
}

void MEMORY::update_map()
{
	if(map1 & 0x20) {
		SET_BANK(0x0000, 0x7fff, ram0, ram0);
		SET_BANK(0x8000, 0xffff, ram1, ram1);
	}
	else {
		SET_BANK(0x0000, 0x7fff, wdmy, rom);
		if(map2 & 1) {
			SET_BANK(0x6000, 0x6fff, wdmy, fdc + 0x6000);
		}
		SET_BANK(0x8000, 0xffff, ram1, ram1);
	}
}
