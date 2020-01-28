/*
	BANDAI RX-78 Emulator 'eRX-78'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

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
	// load ipl
	_memset(ram, 0, sizeof(ram));
	_memset(ext, 0, sizeof(ext));
	_memset(vram, 0, sizeof(vram));
	_memset(ipl, 0xff, sizeof(ipl));
	_memset(cart, 0xff, sizeof(cart));
	_memset(rdmy, 0xff, sizeof(rdmy));
	
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sIPL.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, wdmy, ipl );
	SET_BANK(0x2000, 0x5fff, wdmy, cart);
	SET_BANK(0x6000, 0xafff, ext,  ext );
	SET_BANK(0xb000, 0xefff, ram,  ram );
	SET_BANK(0xf000, 0xffff, wdmy, rdmy);
	
	for(int i = 0; i < 6; i++)
		vbank[i] = vram + 4416 * i;
	rpage = wpage = 0;
}

void MEMORY::reset()
{
	_memset(ram, 0, sizeof(ram));
	_memset(ext, 0, sizeof(ext));
	_memset(vram, 0, sizeof(vram));
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	// known bug : ram must not be mapped to $ec00-$eebf
	if(addr < 0xeec0) {
		wbank[addr >> 12][addr & 0xfff] = data;
		return;
	}
	if(wpage & 0x01)
		vbank[0][addr - 0xeec0] = data;
	if(wpage & 0x02)
		vbank[1][addr - 0xeec0] = data;
	if(wpage & 0x04)
		vbank[2][addr - 0xeec0] = data;
	if(wpage & 0x08)
		vbank[3][addr - 0xeec0] = data;
	if(wpage & 0x10)
		vbank[4][addr - 0xeec0] = data;
	if(wpage & 0x20)
		vbank[5][addr - 0xeec0] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	// known bug : ram must not be mapped to $ec00-$eebf
	if(addr < 0xeec0)
		return rbank[addr >> 12][addr & 0xfff];
	if(1 <= rpage && rpage <= 6)
		return vbank[rpage - 1][addr - 0xeec0];
	return 0xff;
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff)
	{
	case 0xf1:
		rpage = data;
		break;
	case 0xf2:
		wpage = data;
		break;
	}
}

void MEMORY::open_cart(_TCHAR* filename)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(filename, FILEIO_READ_BINARY)) {
		_memset(cart, 0xff, sizeof(cart));
		fio->Fread(cart, sizeof(cart), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::close_cart()
{
	_memset(cart, 0xff, sizeof(cart));
}
