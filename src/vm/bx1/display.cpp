/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ display ]
*/

#include "display.h"

void DISPLAY::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// register event
	register_frame_event(this);
}

void DISPLAY::release()
{
}

void DISPLAY::reset()
{
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	return 0xff;
}

void DISPLAY::event_frame()
{
	d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
}

void DISPLAY::draw_screen()
{
	for(int c = 0; c < 16; c++) {
//		uint8_t d = ram[0x15c + c];
		uint8_t d = ram[0x63 + c];
		
		for(int l = 0; l < 8; l++) {
			scrntype_t* dest = emu->get_screen_buffer(l) + 8 * c;
			uint8_t pattern = font[d * 8 + l];

			dest[0] = (pattern & 0x80) ? RGB_COLOR(255, 255, 255) : 0;
			dest[1] = (pattern & 0x40) ? RGB_COLOR(255, 255, 255) : 0;
			dest[2] = (pattern & 0x20) ? RGB_COLOR(255, 255, 255) : 0;
			dest[3] = (pattern & 0x10) ? RGB_COLOR(255, 255, 255) : 0;
			dest[4] = (pattern & 0x08) ? RGB_COLOR(255, 255, 255) : 0;
			dest[5] = (pattern & 0x04) ? RGB_COLOR(255, 255, 255) : 0;
			dest[6] = (pattern & 0x02) ? RGB_COLOR(255, 255, 255) : 0;
			dest[7] = (pattern & 0x01) ? RGB_COLOR(255, 255, 255) : 0;
		}
	}
}

#define STATE_VERSION	1

bool DISPLAY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return true;
}

