/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ cmt ]
*/

#include "cmt.h"
#include "../../fileio.h"

void CMT::initialize()
{
	fio = new FILEIO();
	play = rec = false;
}

void CMT::release()
{
	close_datarec();
	delete fio;
}

void CMT::reset()
{
	close_datarec();
	play = rec = false;
}

void CMT::write_signal(int id, uint32 data, uint32 mask)
{
	if(rec) {
		// recv from sio
		buffer[bufcnt++] = data & mask;
		if(bufcnt >= BUFFER_SIZE){
			fio->Fwrite(buffer, bufcnt, 1);
			bufcnt = 0;
		}
	}
}

void CMT::play_datarec(_TCHAR* filename)
{
	close_datarec();
	
	if(fio->Fopen(filename, FILEIO_READ_BINARY)) {
		fio->Fseek(0, FILEIO_SEEK_END);
		int size = (fio->Ftell() + 9) & (BUFFER_SIZE - 1);
		fio->Fseek(0, FILEIO_SEEK_SET);
		_memset(buffer, 0, sizeof(buffer));
		fio->Fread(buffer, sizeof(buffer), 1);
		
		// send data to sio
		// this implement does not care the sio buffer size... :-(
		for(int i = 0; i < size; i++)
			d_sio->write_signal(did_recv, buffer[i], 0xff);
		play = true;
	}
}

void CMT::rec_datarec(_TCHAR* filename)
{
	close_datarec();
	
	if(fio->Fopen(filename, FILEIO_WRITE_BINARY)) {
		bufcnt = 0;
		rec = true;
	}
}

void CMT::close_datarec()
{
	// close file
	if(rec && bufcnt)
		fio->Fwrite(buffer, bufcnt, 1);
	if(play || rec)
		fio->Fclose();
	play = rec = false;
	
	// clear sio buffer
	d_sio->write_signal(did_clear, 0, 0);
}
