/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ pac slot 2 ]
*/

#include "pac2.h"
#include "pac2dev.h"
#include "rampac2.h"
#include "kanjipac2.h"
#include "joypac2.h"

void PAC2::initialize()
{
	// slot 4 : ram pack #5
	// slot 3 : ram pack #6
	// slot 2 : kanji rom
	// slot 1 : joystick
	dummy = new PAC2DEV(vm, emu);
	ram1 = new RAMPAC2(vm, emu);
	ram2 = new RAMPAC2(vm, emu);
	kanji = new KANJIPAC2(vm, emu);
	joy = new JOYPAC2(vm, emu);
	
	ram1->initialize(1);
	ram2->initialize(2);
	kanji->initialize(3);
	joy->initialize(4);
	
	dev[7] = dummy;
	dev[6] = dummy;
	dev[5] = dummy;
	dev[4] = ram1;
	dev[3] = ram2;
	dev[2] = kanji;
	dev[1] = joy;
	dev[0] = dummy;
	
	sel = 0;
}

void PAC2::release()
{
	delete dummy;
	ram1->release();
	delete ram1;
	ram2->release();
	delete ram2;
	delete kanji;
	delete joy;
}

void PAC2::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff)
	{
	case 0x18:
	case 0x19:
	case 0x1a:
		dev[sel]->write_io8(addr, data);
		break;
	case 0x1b:
		if(data & 0x80)
			dev[sel]->write_io8(addr, data);
		else
			sel = data & 0x7;
		break;
	}
}

uint32 PAC2::read_io8(uint32 addr)
{
	return dev[sel]->read_io8(addr);
}

