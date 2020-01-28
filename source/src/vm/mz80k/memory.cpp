/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	[ memory ]
*/

#include "memory.h"
#ifdef _MZ1200
#include "display.h"
#endif
#include "../i8253.h"
#include "../i8255.h"
#include "../../fileio.h"

#define EVENT_TEMPO	0
#define EVENT_BLINK	1
#define EVENT_HBLANK	2

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 10, eb = (e) >> 10; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} \
		else { \
			wbank[i] = (w) + 0x400 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} \
		else { \
			rbank[i] = (r) + 0x400 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	_memset(ram, 0, sizeof(ram));
	_memset(vram, 0, sizeof(vram));
	_memset(ipl, 0xff, sizeof(ipl));
#ifdef _MZ1200
	_memset(ext, 0xff, sizeof(ext));
#endif
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
#ifdef _MZ1200
	_stprintf(file_path, _T("%sEXT.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	// 0000-0FFF	IPL/RAM
	// 1000-BFFF	RAM
	// C000-CFFF	RAM/IPL
	SET_BANK(0x0000, 0x0fff, wdmy, ipl);
	SET_BANK(0x1000, 0xbfff, ram + 0x1000, ram + 0x1000);
	SET_BANK(0xc000, 0xcfff, ram + 0xc000, ram + 0xc000);
	SET_BANK(0xd000, 0xd3ff, vram, vram);
	SET_BANK(0xd400, 0xd7ff, vram, vram);
	SET_BANK(0xd800, 0xdbff, vram, vram);
	SET_BANK(0xdc00, 0xdfff, vram, vram);
#ifdef _MZ1200
	SET_BANK(0xe000, 0xe7ff, wdmy, rdmy);
	SET_BANK(0xe800, 0xffff, wdmy, ext);
#else
	SET_BANK(0xe000, 0xffff, wdmy, rdmy);
#endif
	
	// register event
	vm->register_vline_event(this);
	vm->register_event_by_clock(this, EVENT_TEMPO, CPU_CLOCKS / 64, true, NULL);	// 32hz * 2
	vm->register_event_by_clock(this, EVENT_BLINK, CPU_CLOCKS / 3, true, NULL);	// 1.5hz * 2
}

void MEMORY::reset()
{
#ifdef _MZ1200
	// reset memory swap
	SET_BANK(0x0000, 0x0fff, wdmy, ipl);
	SET_BANK(0xc000, 0xcfff, ram + 0xc000, ram + 0xc000);
#endif
	
	tempo = blink = false;
#ifdef _MZ1200
	hblank = false;
#endif
	
	// motor is always rotating...
	d_pio->write_signal(SIG_I8255_PORT_C, 0xff, 0x10);
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
	
#ifdef _MZ1200
	// hblank
	hblank = true;
	int id;
	vm->register_event_by_clock(this, EVENT_HBLANK, 92, false, &id);
#endif
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
#ifdef _MZ1200
	else if(event_id == EVENT_HBLANK) {
		hblank = false;
	}
#endif
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	if(0xe000 <= addr && addr <= 0xe7ff) {
		// memory mapped i/o
		switch(addr) {
		case 0xe000: case 0xe001: case 0xe002: case 0xe003:
			d_pio->write_io8(addr & 3, data);
			break;
		case 0xe004: case 0xe005: case 0xe006: case 0xe007:
			d_ctc->write_io8(addr & 3, data);
			break;
		case 0xe008:
			// 8253 gate0
			d_ctc->write_signal(SIG_I8253_GATE_0, data, 1);
			break;
#ifdef _MZ1200
		case 0xe00c:
			// memory swap
			SET_BANK(0x0000, 0x0fff, ram + 0xc000, ram + 0xc000);
			SET_BANK(0xc000, 0xcfff, wdmy, ipl);
			break;
		case 0xe010:
			// reset memory swap
			SET_BANK(0x0000, 0x0fff, wdmy, ipl);
			SET_BANK(0xc000, 0xcfff, ram + 0xc000, ram + 0xc000);
			break;
		case 0xe014:
			// normal display
			d_disp->write_signal(SIG_DISPLAY_REVERSE, 0, 0);
			break;
		case 0xe015:
			// reverse display
			d_disp->write_signal(SIG_DISPLAY_REVERSE, 1, 1);
			break;
		default:
			if(0xe200 <= addr && addr <= 0xe2ff) {
				// scroll ???
			}
			break;
#endif
		}
		return;
	}
	wbank[addr >> 10][addr & 0x3ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
	if(0xe000 <= addr && addr <= 0xe7ff) {
		// memory mapped i/o
		switch(addr) {
		case 0xe000: case 0xe001: case 0xe002: case 0xe003:
			return d_pio->read_io8(addr & 3);
		case 0xe004: case 0xe005: case 0xe006: case 0xe007:
			return d_ctc->read_io8(addr & 3);
		case 0xe008:
#ifdef _MZ1200
			return (hblank ? 0x80 : 0) | (tempo ? 1 : 0) | 0x7e;
#else
			return (tempo ? 1 : 0) | 0xfe;
#endif
		}
		return 0xff;
	}
	return rbank[addr >> 10][addr & 0x3ff];
}

