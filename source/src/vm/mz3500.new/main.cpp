/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ main ]
*/

#include "main.h"
#include "../upd765a.h"
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
		}
	} \
}

void MAIN::initialize()
{
	// init memory
	_memset(ipl, 0xff, sizeof(ipl));
	_memset(ram, 0, sizeof(ram));
	_memset(common, 0, sizeof(common));
	_memset(basic, 0xff, sizeof(basic));
	_memset(ext, 0xff, sizeof(ext));
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
	_stprintf(file_path, _T("%sBASIC.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	}
	delete fio;
	
	// interrupt
	intfd = int0 = int1 = int2 = int3 = int4 = false;
	me = false;
	prev_irq = false;
	inp = 0;
}

void MAIN::reset()
{
	// memory mapper
	ms = ma = mo = 0;
	me1 = me2 = true;
	update_bank();
	
	// sub cpu
	srqb = 2;
	sres = 0;
	sack = false;
	srdy = true;
	
	// interrupt
	intfd = int0 = int1 = int2 = int3 = int4 = false;
	me = false;
	prev_irq = false;
	inp = 0;
	
	// mfd
	drq = index = false;
}

void MAIN::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 MAIN::read_data8(uint32 addr)
{
	addr &= 0xffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

void MAIN::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xec:
	case 0xed:
	case 0xee:
	case 0xef:
		int0 = false;
		update_irq();
		break;
	case 0xf8:
	case 0xfa:
		if(data & 0x40) {
			for(int i = 0; i < 3; i++) {
				if(data & (1 << i)) {
					d_fdc->write_signal(SIG_UPD765A_SELECT, i, 3);
					break;
				}
			}
		}
		d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 0x10);
		d_fdc->write_signal(SIG_UPD765A_TC, data, 0x20);
		motor = ((data & 0x10) != 0);
		me = ((data & 0x80) != 0);
		update_irq();
		break;
	case 0xf9:
	case 0xfb;
		d_fdc->write_signal(SIG_UPD765A_DACK, 1, 1);
		break;
	case 0xfc:
		if((srqb & 2) != (data & 2)) {
			d_subcpu->write_signal(SIG_CPU_BUSREQ, data, 2);
			srqb = data & 2;
		}
		e1 = data & 1;
		break;
	case 0xfd:
		if(!(sres & 0x80) && (data & 0x80)) {
			d_subcpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
			srqb = 0;
			d_subcpu->reset();
		}
		sres = data & 0x80;
		ms = data & 3;
		update_bank();
		break;
	case 0xfe:
		mo = data & 7;
		ma = (data >> 4) & 0x0f;
		update_bank();
		break;
	case 0xff:
		me1 = ((data & 1) != 0);
		me2 = ((data & 2) != 0);
//		update_bank();
		break;
	}
}

uint32 MAIN::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0xec:
	case 0xed:
	case 0xee:
	case 0xef:
		int0 = false;
		update_irq();
		break;
	case 0xf8:
	case 0xfa:
		return (drq ? 1 : 0) | (index ? 2 : 0) | (motor ? 4 : 0);
	case 0xfe:
		return 0xe4;
	case 0xff:
		return 0xe0 | (srdy ? 0 : 0x10) | (sack ? 0 : 8) | inp;
	}
	return 0xff;
}

void MAIN::write_signal(uint32 id, uint32 data, uint32 mask)
{
	if(id == SIG_MAIN_SACK) {
		sack = ((data & mask) != 0);
	}
	else if(id == SIG_MAIN_SRDY) {
		srdy = ((data & mask) != 0);
	}
	else if(id == SIG_MAIN_INT0) {
		int0 = ((data & mask) != 0);
		update_irq();
	}
	else if(id == SIG_MAIN_INT1) {
		int1 = ((data & mask) != 0);
		update_irq();
	}
	else if(id == SIG_MAIN_INT2) {
		int2 = ((data & mask) != 0);
		update_irq();
	}
	else if(id == SIG_MAIN_INT3) {
		int3 = ((data & mask) != 0);
		update_irq();
	}
	else if(id == SIG_MAIN_INT4) {
		int4 = ((data & mask) != 0);
		update_irq();
	}
	else if(id == SIG_MAIN_INTFD) {
		intfd = ((data & mask) != 0);
		update_irq();
	}
	else if(id == SIG_MAIN_DRQ) {
		drq = ((data & mask) != 0);
	}
	else if(id == SIG_MAIN_INDEX) {
		index = ((data & mask) != 0);
	}
}

void MAIN::update_irq()
{
	bool next = false;
	
	if(intfd && me) {
		intfd = false;
		inp = 0;
		next = true;
	}
	else if(!e1) {
		// irq disabled
	}
	else if(int0) {
		int0 = false;
		inp = 1;
		next = true;
	}
	else if(int1) {
		int1 = false;
		inp = 2;
		next = true;
	}
	else if(int2) {
		int2 = false;
		inp = 3;
		next = true;
	}
	else if(int3) {
		int3 = false;
		inp = 4;
		next = true;
	}
	else if(int4) {
		int4 = false;
		inp = 5;
		next = true;
	}
	if(prev_irq != next) {
		d_cpu->set_intr_line(next, true, 0);
		prev_irq = next;
	}
}

void MAIN::update_bank()
{
	SET_BANK(0x0000, 0xffff, wdmy, rdmy);
	
	if((ms & 3) == 0) {
		// SD0: INITIALIZE STATE
		SET_BANK(0x0000, 0x0fff, wdmy, ipl + 0x1000);
//		SET_BANK(0x1000, 0x1fff, wdmy, ipl + 0x1000);
		SET_BANK(0x2000, 0x3fff, wdmy, basic + 0x2000);
		SET_BANK(0x4000, 0xbfff, ram + 0x4000, ram + 0x4000);	// note: check me1 and me2
		switch(ma & 0x0f) {
		case 0x00: SET_BANK(0xc000, 0xffff, ram + 0x0c000, ram + 0x0c000); break;
		case 0x01: SET_BANK(0xc000, 0xffff, ram + 0x00000, ram + 0x00000); break;
		case 0x02: SET_BANK(0xc000, 0xffff, ram + 0x10000, ram + 0x10000); break;
		case 0x03: SET_BANK(0xc000, 0xffff, ram + 0x14000, ram + 0x14000); break;
		case 0x04: SET_BANK(0xc000, 0xffff, ram + 0x18000, ram + 0x18000); break;
		case 0x05: SET_BANK(0xc000, 0xffff, ram + 0x1c000, ram + 0x1c000); break;
		case 0x06: SET_BANK(0xc000, 0xffff, ram + 0x20000, ram + 0x20000); break;
		case 0x07: SET_BANK(0xc000, 0xffff, ram + 0x24000, ram + 0x24000); break;
		case 0x08: SET_BANK(0xc000, 0xffff, ram + 0x28000, ram + 0x28000); break;
		case 0x09: SET_BANK(0xc000, 0xffff, ram + 0x2c000, ram + 0x2c000); break;
		case 0x0a: SET_BANK(0xc000, 0xffff, ram + 0x30000, ram + 0x30000); break;
		case 0x0b: SET_BANK(0xc000, 0xffff, ram + 0x34000, ram + 0x34000); break;
		case 0x0c: SET_BANK(0xc000, 0xffff, ram + 0x38000, ram + 0x38000); break;
		case 0x0d: SET_BANK(0xc000, 0xffff, ram + 0x3c000, ram + 0x3c000); break;
		case 0x0f: SET_BANK(0xf800, 0xffff, common, common); break;
		}
	}
	else if((ms & 3) == 1) {
		// SD1: SYSTEM LOADING & CP/M
		SET_BANK(0x0000, 0xf7ff, ram, ram);
		SET_BANK(0xf800, 0xffff, common, common);
	}
	else if((ms & 3) == 2) {
		// SD2: ROM based BASIC
		SET_BANK(0x0000, 0x1fff, wdmy, basic);
		switch(mo & 0x07) {
		case 0x00: SET_BANK(0x2000, 0x3fff, wdmy, basic + 0x2000); break;
		case 0x01: SET_BANK(0x2000, 0x3fff, wdmy, basic + 0x4000); break;
		case 0x02: SET_BANK(0x2000, 0x3fff, wdmy, basic + 0x6000); break;
		case 0x03: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x0000); break;
		case 0x04: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x2000); break;
		case 0x05: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x4000); break;
		case 0x06: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x6000); break;
		}
		SET_BANK(0x4000, 0xbfff, ram + 0x4000, ram + 0x4000);	// note: check me1 and me2
		switch(ma & 0x0f) {
		case 0x00: SET_BANK(0xc000, 0xffff, ram + 0x0c000, ram + 0x0c000); break;
		case 0x01: SET_BANK(0xc000, 0xffff, ram + 0x00000, ram + 0x00000); break;
		case 0x02: SET_BANK(0xc000, 0xffff, ram + 0x10000, ram + 0x10000); break;
		case 0x03: SET_BANK(0xc000, 0xffff, ram + 0x14000, ram + 0x14000); break;
		case 0x04: SET_BANK(0xc000, 0xffff, ram + 0x18000, ram + 0x18000); break;
		case 0x05: SET_BANK(0xc000, 0xffff, ram + 0x1c000, ram + 0x1c000); break;
		case 0x06: SET_BANK(0xc000, 0xffff, ram + 0x20000, ram + 0x20000); break;
		case 0x07: SET_BANK(0xc000, 0xffff, ram + 0x24000, ram + 0x24000); break;
		case 0x08: SET_BANK(0xc000, 0xffff, ram + 0x28000, ram + 0x28000); break;
		case 0x09: SET_BANK(0xc000, 0xffff, ram + 0x2c000, ram + 0x2c000); break;
		case 0x0a: SET_BANK(0xc000, 0xffff, ram + 0x30000, ram + 0x30000); break;
		case 0x0b: SET_BANK(0xc000, 0xffff, ram + 0x34000, ram + 0x34000); break;
		case 0x0c: SET_BANK(0xc000, 0xffff, ram + 0x38000, ram + 0x38000); break;
		case 0x0d: SET_BANK(0xc000, 0xffff, ram + 0x3c000, ram + 0x3c000); break;
		case 0x0f: SET_BANK(0xf800, 0xffff, common, common); break;
		}
	}
	else {
		// SD3: RAM based BASIC
		SET_BANK(0x0000, 0x1fff, ram, ram);
		switch(mo & 0x07) {
		case 0x0: SET_BANK(0x2000, 0x3fff, ram + 0x2000, ram + 0x2000); break;
		case 0x1: SET_BANK(0x2000, 0x3fff, ram + 0xc000, ram + 0xc000); break;
		case 0x2: SET_BANK(0x2000, 0x3fff, ram + 0xe000, ram + 0xe000); break;
		case 0x3: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x0000); break;
		case 0x4: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x2000); break;
		case 0x5: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x4000); break;
		case 0x6: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x6000); break;
		}
		SET_BANK(0x4000, 0xbfff, ram + 0x4000, ram + 0x4000);	// note: check me1 and me2
		switch(ma & 0x0f) {
		case 0x00: SET_BANK(0xc000, 0xffff, ram + 0x0c000, ram + 0x0c000); break;
		case 0x01: SET_BANK(0xc000, 0xffff, ram + 0x00000, ram + 0x00000); break;
		case 0x02: SET_BANK(0xc000, 0xffff, ram + 0x10000, ram + 0x10000); break;
		case 0x03: SET_BANK(0xc000, 0xffff, ram + 0x14000, ram + 0x14000); break;
		case 0x04: SET_BANK(0xc000, 0xffff, ram + 0x18000, ram + 0x18000); break;
		case 0x05: SET_BANK(0xc000, 0xffff, ram + 0x1c000, ram + 0x1c000); break;
		case 0x06: SET_BANK(0xc000, 0xffff, ram + 0x20000, ram + 0x20000); break;
		case 0x07: SET_BANK(0xc000, 0xffff, ram + 0x24000, ram + 0x24000); break;
		case 0x08: SET_BANK(0xc000, 0xffff, ram + 0x28000, ram + 0x28000); break;
		case 0x09: SET_BANK(0xc000, 0xffff, ram + 0x2c000, ram + 0x2c000); break;
		case 0x0a: SET_BANK(0xc000, 0xffff, ram + 0x30000, ram + 0x30000); break;
		case 0x0b: SET_BANK(0xc000, 0xffff, ram + 0x34000, ram + 0x34000); break;
		case 0x0c: SET_BANK(0xc000, 0xffff, ram + 0x38000, ram + 0x38000); break;
		case 0x0d: SET_BANK(0xc000, 0xffff, ram + 0x3c000, ram + 0x3c000); break;
		case 0x0f: SET_BANK(0xf800, 0xffff, common, common); break;
		}
	}
}

