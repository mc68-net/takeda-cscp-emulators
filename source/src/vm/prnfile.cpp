/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.12.18-

	[ dummy printer ]
*/

#include "prnfile.h"

#define EVENT_BUSY	0
#define EVENT_ACK	1

void PRNFILE::initialize()
{
	fio = new FILEIO();
	
	value = busy_id = ack_id = wait_frames = -1;
#ifdef PRINTER_STROBE_RISING_EDGE
	strobe = false;
#else
	strobe = true;
#endif
	res = false;
	set_busy(false);
	set_ack(true);
	
	register_frame_event(this);
}

void PRNFILE::release()
{
	close_file();
	delete fio;
}

void PRNFILE::reset()
{
	close_file();
	
	busy_id = ack_id = wait_frames = -1;
	set_busy(false);
	set_ack(true);
}

void PRNFILE::event_frame()
{
	if(wait_frames > 0 && --wait_frames == 0) {
		close_file();
	}
}

void PRNFILE::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_PRINTER_DATA) {
		if(value == -1) {
			value = 0;
		}
		value &= ~mask;
		value |= (data & mask);
	} else if(id == SIG_PRINTER_STROBE) {
		bool new_strobe = ((data & mask) != 0);
#ifdef PRINTER_STROBE_RISING_EDGE
		bool edge = (!strobe && new_strobe);
#else
		bool edge = (strobe && !new_strobe);
#endif
		strobe = new_strobe;
		
		if(edge && value != -1) {
			if(!fio->IsOpened()) {
				open_file();
			}
			fio->Fputc(value);
			
			// busy 1msec
			if(busy_id != -1) {
				cancel_event(this, busy_id);
			}
			register_event(this, EVENT_BUSY, 10000.0, false, &busy_id);
			set_busy(true);
			
			// wait 1sec and finish printing
#ifdef SUPPORT_VARIABLE_TIMING
			wait_frames = (int)(vm->frame_rate() * 1.0 + 0.5);
#else
			wait_frames = (int)(FRAMES_PER_SEC * 1.0 + 0.5);
#endif
		}
	} else if(id == SIG_PRINTER_RESET) {
		bool new_res = ((data & mask) != 0);
		if(res && !new_res) {
			reset();
		}
		res = new_res;
	}
}

uint32 PRNFILE::read_signal(int ch)
{
	if(ch == SIG_PRINTER_BUSY) {
		if(busy) {
			if(busy_id != -1) {
				cancel_event(this, busy_id);
				busy_id = -1;
			}
			set_busy(false);
			return 0xffffffff;
		}
	} else if(ch == SIG_PRINTER_ACK) {
		if(ack) {
			return 0xffffffff;
		}
	}
	return 0;
}

void PRNFILE::event_callback(int event_id, int err)
{
	if(event_id == EVENT_BUSY) {
		busy_id = -1;
		set_busy(false);
	} else if(event_id == EVENT_ACK) {
		ack_id = -1;
		set_ack(true);
	}
}

void PRNFILE::set_busy(bool value)
{
	if(busy && !value) {
		// ack 10usec
		if(ack_id != -1) {
			cancel_event(this, ack_id);
		}
		register_event(this, EVENT_ACK, 10.0, false, &ack_id);
		set_ack(false);
	}
	busy = value;
	write_signals(&outputs_busy, busy ? 0xffffffff : 0);
}

void PRNFILE::set_ack(bool value)
{
	ack = value;
	write_signals(&outputs_ack, ack ? 0xffffffff : 0);
}

void PRNFILE::open_file()
{
	create_date_file_path(file_path, _MAX_PATH, _T("txt"));
	fio->Fopen(file_path, FILEIO_WRITE_BINARY);
}

void PRNFILE::close_file()
{
	if(fio->IsOpened()) {
		// remove if the file size is less than 2 bytes
		bool remove = (fio->Ftell() < 2);
		fio->Fclose();
		if(remove) {
			FILEIO::RemoveFile(file_path);
		}
	}
}

#define STATE_VERSION	2

void PRNFILE::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(value);
	state_fio->FputInt32(busy_id);
	state_fio->FputInt32(ack_id);
	state_fio->FputBool(strobe);
	state_fio->FputBool(res);
	state_fio->FputBool(busy);
	state_fio->FputBool(ack);
}

bool PRNFILE::load_state(FILEIO* state_fio)
{
	close_file();
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	value = state_fio->FgetInt32();
	busy_id = state_fio->FgetInt32();
	ack_id = state_fio->FgetInt32();
	strobe = state_fio->FgetBool();
	res = state_fio->FgetBool();
	busy = state_fio->FgetBool();
	ack = state_fio->FgetBool();
	
	// post process
	wait_frames = -1;
	return true;
}
