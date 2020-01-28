/*
	EPSON QC-10 Emulator 'eQC-10'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.02.15 -

	[ memory ]
*/

#include "memory.h"
#include "../upd765a.h"
#include "../../fileio.h"
#include "../../config.h"

extern config_t config;

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		wbank[i] = (w) + 0x800 * (i - sb); \
		rbank[i] = (r) + 0x800 * (i - sb); \
	} \
}

void MEMORY::initialize()
{
	// init memory
	_memset(ram, 0, sizeof(ram));
	_memset(cmos, 0, sizeof(cmos));
	_memset(ipl, 0xff, sizeof(ipl));
	_memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sIPL.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sCMOS.BIN"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(cmos, sizeof(cmos), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::release()
{
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sCMOS.BIN"), app_path);
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		fio->Fwrite(cmos, sizeof(cmos), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::reset()
{
	// init memory map
	bank = 0x10;
	psel = csel = 0;
	update_map();
	
	// init beep
	beep_on = beep_cont = beep_pit = false;
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
//	wbank[addr >> 11][addr & 0x7ff] = data;
	wbank[(addr >> 11) & 0x1f][addr & 0x7ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
//	return rbank[addr >> 11][addr & 0x7ff];
	return rbank[(addr >> 11) & 0x1f][addr & 0x7ff];
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff)
	{
	case 0x18: case 0x19: case 0x1a: case 0x1b:
		bank = data;
		// timer gate signal
		d_pit->write_signal(did_gate0, data, 1);
		d_pit->write_signal(did_gate2, data, 0x80);
		// beep on
		beep_cont = ((data & 4) != 0);
		update_beep();
		break;
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		psel = data;
		break;
	case 0x20: case 0x21: case 0x22: case 0x23:
		csel = data;
		break;
	}
	update_map();
}

uint32 MEMORY::read_io8(uint32 addr)
{
	switch(addr & 0xff)
	{
	case 0x18: case 0x19: case 0x1a: case 0x1b:
		return config.dipswitch;
	case 0x30: case 0x31: case 0x32: case 0x33:
		return (bank & 0xf0) | d_fdc->fdc_status();
	}
	return 0xff;
}

/*
	0000-DFFF	: RAM * 4banks
	E000-FFFF	: RAM
	----
	0000-1FFF	: IPL
	8000-87FF	: CMOS

*/

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	// beep on from pit
	beep_pit = ((data & mask) != 0);
	update_beep();
}

void MEMORY::update_map()
{
	// RAM
	if(bank & 0x10) {
		SET_BANK(0x0000, 0xdfff, ram + 0x00000, ram + 0x00000);
	}
	else if(bank & 0x20) {
		SET_BANK(0x0000, 0xdfff, ram + 0x10000, ram + 0x10000);
	}
	else if(bank & 0x40) {
		SET_BANK(0x0000, 0xdfff, ram + 0x20000, ram + 0x20000);
	}
	else if(bank & 0x80) {
		SET_BANK(0x0000, 0xdfff, ram + 0x30000, ram + 0x30000);
	}
	else {
		SET_BANK(0x0000, 0xdfff, wdmy, rdmy);
	}
	SET_BANK(0xe000, 0xffff, ram + 0xe000, ram + 0xe000);
	
	// PROM
	if(!(psel & 1)) {
		SET_BANK(0x0000, 0x1fff, wdmy, ipl);
	}
	if(csel & 1) {
		SET_BANK(0x8000, 0x87ff, cmos, cmos);
	}
}

void MEMORY::update_beep()
{
	if(!beep_on && (beep_cont || beep_pit)) {
		d_beep->write_signal(did_beep, 1, 1);
		beep_on = true;
	}
	else if(beep_on && !(beep_cont || beep_pit)) {
		d_beep->write_signal(did_beep, 0, 1);
		beep_on = false;
	}
}
