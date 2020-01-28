/*
	EPSON HC-80 Emulator 'eHC-80'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.03.14 -

	[ memory ]
*/

#include "memory.h"
#include "../../fileio.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 13, eb = (e) >> 13; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} \
		else { \
			wbank[i] = (w) + 0x2000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} \
		else { \
			rbank[i] = (r) + 0x2000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// initialize memory
	_memset(ram, 0, sizeof(ram));
	_memset(sys, 0xff, sizeof(sys));
	_memset(rdmy, 0xff, sizeof(rdmy));
	
	// buttery backuped dram
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	// load rom images
	_stprintf(file_path, _T("%sDRAM.BIN"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sSYS.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(sys, sizeof(sys), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::release()
{
	// buttery backuped dram
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sDRAM.BIN"), app_path);
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::reset()
{
	set_bank(0);
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	wbank[(addr >> 13) & 7][addr & 0x1fff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	return rbank[(addr >> 13) & 7][addr & 0x1fff];
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	set_bank(data);
}

void MEMORY::set_bank(uint32 val)
{
	if(val & 1) {
		SET_BANK(0x0000, 0xffff, ram, ram);
	}
	else {
		SET_BANK(0x0000, 0x7fff, wdmy, sys);
		SET_BANK(0x8000, 0xffff, ram + 0x8000, ram + 0x8000);
	}
}

