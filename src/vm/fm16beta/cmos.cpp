/*
	FUJITSU FM16beta Emulator 'eFM16beta'

	Author : Takeda.Toshiya
	Date   : 2017.12.30-

	[ cmos ]
*/

#include "cmos.h"

void CMOS::initialize()
{
	// load cmos image
	memset(cmos, 0, sizeof(cmos));
	modified = false;
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("CMOS.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(cmos, sizeof(cmos), 1);
		fio->Fclose();
	}
	delete fio;
}

void CMOS::release()
{
	if(modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(cmos, sizeof(cmos), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void CMOS::write_io8(uint32_t addr, uint32_t data)
{
	if(cmos[addr & 0x7ff] != data) {
		cmos[addr & 0x7ff] = data;
		modified = true;
	}
}

uint32_t CMOS::read_io8(uint32_t addr)
{
	return cmos[addr & 0x7ff];
}

#define STATE_VERSION	1

void CMOS::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(cmos, sizeof(cmos), 1);
	state_fio->FputBool(modified);
}

bool CMOS::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(cmos, sizeof(cmos), 1);
	modified = state_fio->FgetBool();
	return true;
}

