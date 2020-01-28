/*
	EPSON QC-10 Emulator 'eQC-10'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.03.24 -

	[ multifont rom card ]
*/

#include "mfont.h"
#include "../i8259.h"
#include "../../fifo.h"

#define BIT_IBF	0x80
#define BIT_ERR	2
#define BIT_OBF	1

void MFONT::initialize()
{
	memset(mfont, 0xff, sizeof(mfont));
	
	// load multifont rom images
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sMFONT.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(mfont, sizeof(mfont), 1);
		fio->Fclose();
	}
	delete fio;
	
	cmd = new FIFO(4);
	res = new FIFO(38);
	
	status = 0;
}

void MFONT::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xfc:
		cmd->write(data);
		if(cmd->count() == 3) {
			int mode = cmd->read();
			int code = cmd->read();
			code |= cmd->read() << 8;
			
			if(0x200 <= code && code < 0xc00) {
				int ofs = (code - 0x200) * 36;
				res->clear();
				res->write(0x40);
				for(int i = 0; i < 36; i++) {
					res->write(mfont[ofs + i]);
				}
				status = BIT_IBF | BIT_OBF;
				d_pic->write_signal(SIG_I8259_IR7 | SIG_I8259_CHIP1, 1, 1);
			}
			else {
				// error
				status = BIT_ERR;
			}
		}
		break;
	case 0xfd:
		// set irq
		d_pic->write_signal(SIG_I8259_IR7 | SIG_I8259_CHIP1, 1, 1);
		break;
	}
}

uint32 MFONT::read_io8(uint32 addr)
{
	uint32 val;
	
	switch(addr & 0xff) {
	case 0xfc:
		val = res->read();
		if(res->empty()) {
			status = 0;
		}
		return val;
	case 0xfd:
		// reset irq
		d_pic->write_signal(SIG_I8259_IR7 | SIG_I8259_CHIP1, 0, 1);
		return status;
	}
	return 0xff;
}

