/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ ram pack ]
*/

#include "rampack.h"

void RAMPACK::initialize()
{
	memset(ram, 0, sizeof(ram));
	modified = false;
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("RAMPACK%d.BIN"), index), FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

void RAMPACK::release()
{
	if(modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("RAMPACK%d.BIN"), index), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(ram, sizeof(ram), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void RAMPACK::write_io8(uint32_t addr, uint32_t data)
{
	if(addr < 0x4000) {
		if(ram[addr] != data) {
			ram[addr] = data;
			modified = true;
		}
	}
}

uint32_t RAMPACK::read_io8(uint32_t addr)
{
	if(addr < 0x4000) {
		return ram[addr];
	} else if(0xff00 <= addr && addr < 0xff80) {
		return 0x01; // device id
	}
	return 0xff;
}

#define STATE_VERSION	1

void RAMPACK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->FputBool(modified);
}

bool RAMPACK::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	modified = state_fio->FgetBool();
	return true;
}
