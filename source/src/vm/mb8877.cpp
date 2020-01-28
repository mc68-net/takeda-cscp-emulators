/*
	Skelton for retropc emulator

	Origin : XM7
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ MB8877 / MB8876 ]
*/

#include "mb8877.h"
#include "disk.h"

#ifndef MB8877_CORRECT_TIMING
#define MB8877_CORRECT_TIMING
#endif

#define FDC_ST_BUSY		0x01	// busy
#define FDC_ST_INDEX		0x02	// index hole
#define FDC_ST_DRQ		0x02	// data request
#define FDC_ST_TRACK00		0x04	// track0
#define FDC_ST_LOSTDATA		0x04	// data lost
#define FDC_ST_CRCERR		0x08	// crc error
#define FDC_ST_SEEKERR		0x10	// seek error
#define FDC_ST_RECNFND		0x10	// sector not found
#define FDC_ST_HEADENG		0x20	// head engage
#define FDC_ST_RECTYPE		0x20	// record type
#define FDC_ST_WRITEFAULT	0x20	// write fault
#define FDC_ST_WRITEP		0x40	// write protectdc
#define FDC_ST_NOTREADY		0x80	// media not inserted

#define FDC_CMD_TYPE1		1
#define FDC_CMD_RD_SEC		2
#define FDC_CMD_RD_MSEC		3
#define FDC_CMD_WR_SEC		4
#define FDC_CMD_WR_MSEC		5
#define FDC_CMD_RD_ADDR		6
#define FDC_CMD_RD_TRK		7
#define FDC_CMD_WR_TRK		8
#define FDC_CMD_TYPE4		0x80

#define EVENT_SEEK		0
#define EVENT_SEEKEND		1
#define EVENT_SEARCH		2
#define EVENT_TYPE4		3
#define EVENT_DRQ		4
#define EVENT_MULTI1		5
#define EVENT_MULTI2		6
#define EVENT_LOST		7

#define DRIVE_MASK		(MAX_DRIVE - 1)

static const int seek_wait_hi[4] = {3000,  6000, 10000, 16000};
static const int seek_wait_lo[4] = {6000, 12000, 20000, 30000};

#define CANCEL_EVENT(event) { \
	if(register_id[event] != -1) { \
		cancel_event(register_id[event]); \
		register_id[event] = -1; \
	} \
}
#define REGISTER_EVENT(event, usec) { \
	if(register_id[event] != -1) { \
		cancel_event(register_id[event]); \
		register_id[event] = -1; \
	} \
	register_event(this, (event << 8) | cmdtype, usec, false, &register_id[event]); \
}
#define REGISTER_SEEK_EVENT() { \
	if(register_id[EVENT_SEEK] != -1) { \
		cancel_event(register_id[EVENT_SEEK]); \
		register_id[EVENT_SEEK] = -1; \
	} \
	if(disk[drvreg]->drive_type == DRIVE_TYPE_2HD) { \
		register_event(this, (EVENT_SEEK << 8) | cmdtype, seek_wait_hi[cmdreg & 3], false, &register_id[EVENT_SEEK]); \
	} \
	else { \
		register_event(this, (EVENT_SEEK << 8) | cmdtype, seek_wait_lo[cmdreg & 3], false, &register_id[EVENT_SEEK]); \
	} \
	now_seek = after_seek = true; \
}
#define REGISTER_DRQ_EVENT() { \
	double usec = disk[drvreg]->get_usec_per_bytes(1) - passed_usec(prev_drq_clock); \
	if(usec < 4) { \
		usec = 4; \
	} \
	if(register_id[EVENT_DRQ] != -1) { \
		cancel_event(register_id[EVENT_DRQ]); \
		register_id[EVENT_DRQ] = -1; \
	} \
	register_event(this, (EVENT_DRQ << 8) | cmdtype, usec, false, &register_id[EVENT_DRQ]); \
}
#define REGISTER_LOST_EVENT() { \
	if(register_id[EVENT_LOST] != -1) { \
		cancel_event(register_id[EVENT_LOST]); \
		register_id[EVENT_LOST] = -1; \
	} \
	register_event(this, (EVENT_LOST << 8) | cmdtype, disk[drvreg]->get_usec_per_bytes(1), false, &register_id[EVENT_LOST]); \
}

void MB8877::initialize()
{
	// config
	ignore_crc = config.ignore_crc;
	
	// initialize d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK();
	}
	
	// initialize timing
	memset(cur_position, 0, sizeof(cur_position));
	memset(prev_clock, 0, sizeof(prev_clock));
	
	// initialize fdc
	seektrk = 0;
	seekvct = true;
	status = cmdreg = trkreg = secreg = datareg = sidereg = cmdtype = 0;
	drvreg = 0;
}

void MB8877::release()
{
	// release d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		delete disk[i];
	}
}

void MB8877::reset()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc[i].track = 0;
		fdc[i].index = 0;
		fdc[i].access = false;
	}
	for(int i = 0; i < 7; i++) {
		register_id[i] = -1;
	}
	now_search = now_seek = after_seek = false;
}

void MB8877::update_config()
{
	ignore_crc = config.ignore_crc;
}

void MB8877::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 3) {
	case 0:
		// command reg
#ifdef HAS_MB8876
		cmdreg = (~data) & 0xff;
#else
		cmdreg = data;
#endif
		process_cmd();
		break;
	case 1:
		// track reg
#ifdef HAS_MB8876
		trkreg = (~data) & 0xff;
#else
		trkreg = data;
#endif
		if((status & FDC_ST_BUSY) && (fdc[drvreg].index == 0)) {
			// track reg is written after command starts
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				process_cmd();
			}
		}
		break;
	case 2:
		// sector reg
#ifdef HAS_MB8876
		secreg = (~data) & 0xff;
#else
		secreg = data;
#endif
		if((status & FDC_ST_BUSY) && (fdc[drvreg].index == 0)) {
			// sector reg is written after command starts
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				process_cmd();
			}
		}
		break;
	case 3:
		// data reg
#ifdef HAS_MB8876
		datareg = (~data) & 0xff;
#else
		datareg = data;
#endif
		if(motor_on && (status & FDC_ST_DRQ) && !now_search) {
			if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				// write or multisector write
				if(fdc[drvreg].index < disk[drvreg]->sector_size) {
					if(!disk[drvreg]->write_protected) {
						disk[drvreg]->sector[fdc[drvreg].index] = datareg;
						// dm, ddm
						disk[drvreg]->deleted = (cmdreg & 1) ? 0x10 : 0;
					}
					else {
						status |= FDC_ST_WRITEFAULT;
						status &= ~FDC_ST_BUSY;
						cmdtype = 0;
						set_irq(true);
					}
					fdc[drvreg].index++;
				}
				if(fdc[drvreg].index >= disk[drvreg]->sector_size) {
					if(cmdtype == FDC_CMD_WR_SEC) {
						// single sector
						status &= ~FDC_ST_BUSY;
						cmdtype = 0;
						set_irq(true);
					}
					else {
						// multisector
						REGISTER_EVENT(EVENT_MULTI1, 30);
						REGISTER_EVENT(EVENT_MULTI2, 60);
					}
				}
				else if(status & FDC_ST_DRQ) {
					REGISTER_DRQ_EVENT();
				}
				status &= ~FDC_ST_DRQ;
			}
			else if(cmdtype == FDC_CMD_WR_TRK) {
				// read track
				if(fdc[drvreg].index < disk[drvreg]->get_track_size()) {
					if(!disk[drvreg]->write_protected) {
						disk[drvreg]->track[fdc[drvreg].index] = datareg;
					}
					else {
						status |= FDC_ST_WRITEFAULT;
						status &= ~FDC_ST_BUSY;
						status &= ~FDC_ST_DRQ;
						cmdtype = 0;
						set_irq(true);
					}
					fdc[drvreg].index++;
				}
				if(fdc[drvreg].index >= disk[drvreg]->get_track_size()) {
					status &= ~FDC_ST_BUSY;
					cmdtype = 0;
					set_irq(true);
				}
				else if(status & FDC_ST_DRQ) {
					REGISTER_DRQ_EVENT();
				}
				status &= ~FDC_ST_DRQ;
			}
			if(!(status & FDC_ST_DRQ)) {
				CANCEL_EVENT(EVENT_LOST);
				set_drq(false);
				fdc[drvreg].access = true;
			}
		}
		break;
	}
}

uint32 MB8877::read_io8(uint32 addr)
{
	uint32 val;
	
	switch(addr & 3) {
	case 0:
		// status reg
		if(cmdtype == FDC_CMD_TYPE4) {
			// now force interrupt
			if(!disk[drvreg]->inserted || !motor_on) {
				status = FDC_ST_NOTREADY;
			}
			else {
				// MZ-2500 RELICS invites STATUS = 0
				status = 0;
			}
			val = status;
		}
		else if(now_search) {
			// now sector search
			val = FDC_ST_BUSY;
		}
		else {
			// disk not inserted, motor stop
			if(!disk[drvreg]->inserted || !motor_on) {
				status |= FDC_ST_NOTREADY;
			}
			else {
				status &= ~FDC_ST_NOTREADY;
			}
			// write protected
			if(cmdtype == FDC_CMD_TYPE1 || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC || cmdtype == FDC_CMD_WR_TRK) {
				if(disk[drvreg]->inserted && disk[drvreg]->write_protected) {
					status |= FDC_ST_WRITEP;
				}
				else {
					status &= ~FDC_ST_WRITEP;
				}
			}
			else {
				status &= ~FDC_ST_WRITEP;
			}
			// track0, index hole
			if(cmdtype == FDC_CMD_TYPE1) {
				if(fdc[drvreg].track == 0) {
					status |= FDC_ST_TRACK00;
				}
				else {
					status &= ~FDC_ST_TRACK00;
				}
				if(!(status & FDC_ST_NOTREADY)) {
					if(get_cur_position() == 0) {
						status |= FDC_ST_INDEX;
					}
					else {
						status &= ~FDC_ST_INDEX;
					}
				}
			}
			// show busy a moment
			val = status;
			if(cmdtype == FDC_CMD_TYPE1 && !now_seek) {
				status &= ~FDC_ST_BUSY;
			}
		}
		// reset irq/drq
		if(!(status & FDC_ST_DRQ)) {
			set_drq(false);
		}
		set_irq(false);
#ifdef _FDC_DEBUG_LOG
		// request cpu to output debug log
		if(d_cpu) {
			d_cpu->write_signal(SIG_CPU_DEBUG, 1, 1);
		}
		emu->out_debug(_T("FDC\tSTATUS=%2x\n"), val);
#endif
#ifdef HAS_MB8876
		return (~val) & 0xff;
#else
		return val;
#endif
	case 1:
		// track reg
#ifdef HAS_MB8876
		return (~trkreg) & 0xff;
#else
		return trkreg;
#endif
	case 2:
		// sector reg
#ifdef HAS_MB8876
		return (~secreg) & 0xff;
#else
		return secreg;
#endif
	case 3:
		// data reg
		if(motor_on && (status & FDC_ST_DRQ) && !now_search) {
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC) {
				// read or multisector read
				if(fdc[drvreg].index < disk[drvreg]->sector_size) {
					datareg = disk[drvreg]->sector[fdc[drvreg].index];
					fdc[drvreg].index++;
				}
				if(fdc[drvreg].index >= disk[drvreg]->sector_size) {
					if(cmdtype == FDC_CMD_RD_SEC) {
						// single sector
#ifdef _FDC_DEBUG_LOG
						emu->out_debug(_T("FDC\tEND OF SECTOR\n"));
#endif
						status &= ~FDC_ST_BUSY;
						cmdtype = 0;
						set_irq(true);
					}
					else {
						// multisector
#ifdef _FDC_DEBUG_LOG
						emu->out_debug(_T("FDC\tEND OF SECTOR (SEARCH NEXT)\n"));
#endif
						REGISTER_EVENT(EVENT_MULTI1, 30);
						REGISTER_EVENT(EVENT_MULTI2, 60);
					}
				}
				else {
					REGISTER_DRQ_EVENT();
				}
				status &= ~FDC_ST_DRQ;
			}
			else if(cmdtype == FDC_CMD_RD_ADDR) {
				// read address
				if(fdc[drvreg].index < 6) {
					datareg = disk[drvreg]->id[fdc[drvreg].index];
					fdc[drvreg].index++;
				}
				if(fdc[drvreg].index >= 6) {
					status &= ~FDC_ST_BUSY;
					cmdtype = 0;
					set_irq(true);
				}
				else {
					REGISTER_DRQ_EVENT();
				}
				status &= ~FDC_ST_DRQ;
			}
			else if(cmdtype == FDC_CMD_RD_TRK) {
				// read track
				if(fdc[drvreg].index < disk[drvreg]->get_track_size()) {
					datareg = disk[drvreg]->track[fdc[drvreg].index];
					fdc[drvreg].index++;
				}
				if(fdc[drvreg].index >= disk[drvreg]->get_track_size()) {
#ifdef _FDC_DEBUG_LOG
					emu->out_debug(_T("FDC\tEND OF TRACK\n"));
#endif
					status &= ~FDC_ST_BUSY;
					status |= FDC_ST_LOSTDATA;
					cmdtype = 0;
					set_irq(true);
				}
				else {
					REGISTER_DRQ_EVENT();
				}
				status &= ~FDC_ST_DRQ;
			}
			if(!(status & FDC_ST_DRQ)) {
				CANCEL_EVENT(EVENT_LOST);
				set_drq(false);
				fdc[drvreg].access = true;
			}
		}
#ifdef _FDC_DEBUG_LOG
		emu->out_debug(_T("FDC\tDATA=%2x\n"), datareg);
#endif
#ifdef HAS_MB8876
		return (~datareg) & 0xff;
#else
		return datareg;
#endif
	}
	return 0xff;
}

void MB8877::write_dma_io8(uint32 addr, uint32 data)
{
	write_io8(3, data);
}

uint32 MB8877::read_dma_io8(uint32 addr)
{
	return read_io8(3);
}

void MB8877::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MB8877_DRIVEREG) {
		drvreg = data & DRIVE_MASK;
	}
	else if(id == SIG_MB8877_SIDEREG) {
		sidereg = (data & mask) ? 1 : 0;
	}
	else if(id == SIG_MB8877_MOTOR) {
		motor_on = ((data & mask) != 0);
	}
}

uint32 MB8877::read_signal(int ch)
{
	// get access status
	uint32 stat = 0;
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(fdc[i].access) {
			stat |= 1 << i;
		}
		fdc[i].access = false;
	}
	return stat;
}

void MB8877::event_callback(int event_id, int err)
{
	int event = event_id >> 8;
	int cmd = event_id & 0xff;
	register_id[event] = -1;
	
	// cancel event if the command is finished or other command is executed
	if(cmd != cmdtype) {
		if(event == EVENT_SEEK) {
			now_seek = false;
		}
		else if(event == EVENT_SEARCH) {
			now_search = false;
		}
		return;
	}
	
	switch(event) {
	case EVENT_SEEK:
		if(seektrk > fdc[drvreg].track) {
			fdc[drvreg].track++;
		}
		else if(seektrk < fdc[drvreg].track) {
			fdc[drvreg].track--;
		}
		if((cmdreg & 0x10) || ((cmdreg & 0xf0) == 0)) {
			trkreg = fdc[drvreg].track;
		}
		if(seektrk == fdc[drvreg].track) {
			// auto update
			if((cmdreg & 0xf0) == 0) {
				datareg = 0;
			}
			status |= search_track();
			now_seek = false;
			set_irq(true);
		}
		else {
			REGISTER_SEEK_EVENT();
		}
		break;
	case EVENT_SEEKEND:
		if(seektrk == fdc[drvreg].track) {
			// auto update
			if((cmdreg & 0x10) || ((cmdreg & 0xf0) == 0)) {
				trkreg = fdc[drvreg].track;
			}
			if((cmdreg & 0xf0) == 0) {
				datareg = 0;
			}
			status |= search_track();
			now_seek = false;
			CANCEL_EVENT(EVENT_SEEK);
			set_irq(true);
		}
		break;
	case EVENT_SEARCH:
		now_search = false;
		if(!(status & FDC_ST_RECNFND)) {
			status |= FDC_ST_DRQ;
			REGISTER_LOST_EVENT();
			cur_position[drvreg] = next_trans_position[drvreg];
			prev_clock[drvreg] = prev_drq_clock = current_clock();
			set_drq(true);
		}
		break;
	case EVENT_TYPE4:
		cmdtype = FDC_CMD_TYPE4;
		break;
	case EVENT_DRQ:
		if(status & FDC_ST_BUSY) {
			status |= FDC_ST_DRQ;
			REGISTER_LOST_EVENT();
			cur_position[drvreg] = (cur_position[drvreg] + 1) % disk[drvreg]->get_track_size();
			prev_clock[drvreg] = prev_drq_clock = current_clock();
			set_drq(true);
		}
		break;
	case EVENT_MULTI1:
		secreg++;
		break;
	case EVENT_MULTI2:
		if(cmdtype == FDC_CMD_RD_MSEC) {
			cmd_readdata();
		}
		else if(cmdtype == FDC_CMD_WR_MSEC) {
			cmd_writedata();
		}
		break;
	case EVENT_LOST:
		if(status & FDC_ST_BUSY) {
			status |= FDC_ST_LOSTDATA;
			status &= ~FDC_ST_BUSY;
			//status &= ~FDC_ST_DRQ;
			set_irq(true);
			//set_drq(false);
		}
		break;
	}
}

// ----------------------------------------------------------------------------
// command
// ----------------------------------------------------------------------------

void MB8877::process_cmd()
{
#ifdef _FDC_DEBUG_LOG
	static const _TCHAR *cmdstr[0x10] = {
		_T("RESTORE "),	_T("SEEK    "),	_T("STEP    "),	_T("STEP    "),
		_T("STEP IN "),	_T("STEP IN "),	_T("STEP OUT"),	_T("STEP OUT"),
		_T("RD DATA "),	_T("RD DATA "),	_T("RD DATA "),	_T("WR DATA "),
		_T("RD ADDR "),	_T("FORCEINT"),	_T("RD TRACK"),	_T("WR TRACK")
	};
	emu->out_debug(_T("FDC\tCMD=%2xh (%s) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, cmdstr[cmdreg >> 4], datareg, drvreg, trkreg, sidereg, secreg);
#endif
	
	CANCEL_EVENT(EVENT_TYPE4);
	set_irq(false);
	
	switch(cmdreg & 0xf0) {
	// type-1
	case 0x00:
		cmd_restore();
		break;
	case 0x10:
		cmd_seek();
		break;
	case 0x20:
	case 0x30:
		cmd_step();
		break;
	case 0x40:
	case 0x50:
		cmd_stepin();
		break;
	case 0x60:
	case 0x70:
		cmd_stepout();
		break;
	// type-2
	case 0x80:
	case 0x90:
		cmd_readdata();
		break;
	case 0xa0:
	case 0xb0:
		cmd_writedata();
		break;
	// type-3
	case 0xc0:
		cmd_readaddr();
		break;
	case 0xe0:
		cmd_readtrack();
		break;
	case 0xf0:
		cmd_writetrack();
		break;
	// type-4
	case 0xd0:
		cmd_forceint();
		break;
	default:
		break;
	}
}

void MB8877::cmd_restore()
{
	// type-1 restore
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_HEADENG | FDC_ST_BUSY;
	trkreg = 0xff;
	
	seektrk = 0;
	seekvct = true;
	
	REGISTER_SEEK_EVENT();
	REGISTER_EVENT(EVENT_SEEKEND, 300);
}

void MB8877::cmd_seek()
{
	// type-1 seek
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_HEADENG | FDC_ST_BUSY;
	
#if 0
	seektrk = fdc[drvreg].track + datareg - trkreg;
#else
	seektrk = datareg;
#endif
	seektrk = (seektrk > 83) ? 83 : (seektrk < 0) ? 0 : seektrk;
	seekvct = !(datareg > trkreg);
	
	REGISTER_SEEK_EVENT();
	REGISTER_EVENT(EVENT_SEEKEND, 300);
}

void MB8877::cmd_step()
{
	// type-1 step
	if(seekvct) {
		cmd_stepout();
	}
	else {
		cmd_stepin();
	}
}

void MB8877::cmd_stepin()
{
	// type-1 step in
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_HEADENG | FDC_ST_BUSY;
	
	seektrk = (fdc[drvreg].track < 83) ? fdc[drvreg].track + 1 : 83;
	seekvct = false;
	
	REGISTER_SEEK_EVENT();
	REGISTER_EVENT(EVENT_SEEKEND, 300);
}

void MB8877::cmd_stepout()
{
	// type-1 step out
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_HEADENG | FDC_ST_BUSY;
	
	seektrk = (fdc[drvreg].track > 0) ? fdc[drvreg].track - 1 : 0;
	seekvct = true;
	
	REGISTER_SEEK_EVENT();
	REGISTER_EVENT(EVENT_SEEKEND, 300);
}

void MB8877::cmd_readdata()
{
	// type-2 read data
	cmdtype = (cmdreg & 0x10) ? FDC_CMD_RD_MSEC : FDC_CMD_RD_SEC;
	int side = (cmdreg & 2) ? ((cmdreg & 8) ? 1 : 0) : sidereg;
	status = search_sector(fdc[drvreg].track, side, secreg, ((cmdreg & 2) != 0));
	if(!(status & FDC_ST_RECNFND)) {
//		status |= FDC_ST_DRQ | FDC_ST_BUSY;
		status |= FDC_ST_BUSY;
	}
	now_search = true;
	
	double time;
	if(!(status & FDC_ST_RECNFND)) {
		time = get_usec_to_start_trans();
	}
	else {
		time = disk[drvreg]->get_usec_per_bytes(disk[drvreg]->get_track_size());
	}
	REGISTER_EVENT(EVENT_SEARCH, time);
	CANCEL_EVENT(EVENT_LOST);
}

void MB8877::cmd_writedata()
{
	// type-2 write data
	cmdtype = (cmdreg & 0x10) ? FDC_CMD_WR_MSEC : FDC_CMD_WR_SEC;
	int side = (cmdreg & 2) ? ((cmdreg & 8) ? 1 : 0) : sidereg;
	status = search_sector(fdc[drvreg].track, side, secreg, ((cmdreg & 2) != 0));
	status &= ~FDC_ST_RECTYPE;
	if(!(status & FDC_ST_RECNFND)) {
//		status |= FDC_ST_DRQ | FDC_ST_BUSY;
		status |= FDC_ST_BUSY;
	}
	now_search = true;
	
	double time;
	if(!(status & FDC_ST_RECNFND)) {
		time = get_usec_to_start_trans();
	}
	else {
		time = disk[drvreg]->get_usec_per_bytes(disk[drvreg]->get_track_size());
	}
	REGISTER_EVENT(EVENT_SEARCH, time);
	CANCEL_EVENT(EVENT_LOST);
}

void MB8877::cmd_readaddr()
{
	// type-3 read address
	cmdtype = FDC_CMD_RD_ADDR;
	status = search_addr();
	if(!(status & FDC_ST_RECNFND)) {
//		status |= FDC_ST_DRQ | FDC_ST_BUSY;
		status |= FDC_ST_BUSY;
	}
	now_search = true;
	
	double time;
	if(!(status & FDC_ST_RECNFND)) {
		time = get_usec_to_start_trans();
	}
	else {
		time = disk[drvreg]->get_usec_per_bytes(disk[drvreg]->get_track_size());
	}
	REGISTER_EVENT(EVENT_SEARCH, time);
	CANCEL_EVENT(EVENT_LOST);
}

void MB8877::cmd_readtrack()
{
	// type-3 read track
	cmdtype = FDC_CMD_RD_TRK;
//	status = FDC_ST_DRQ | FDC_ST_BUSY;
	status = FDC_ST_BUSY;
	
	disk[drvreg]->make_track(fdc[drvreg].track, sidereg);
	fdc[drvreg].index = 0;
	now_search = true;
	
	int bytes = disk[drvreg]->get_track_size() - get_cur_position();
	if(bytes < 12) {
		bytes += disk[drvreg]->get_track_size();
	}
	double time = disk[drvreg]->get_usec_per_bytes(bytes);
	REGISTER_EVENT(EVENT_SEARCH, time);
	CANCEL_EVENT(EVENT_LOST);
}

void MB8877::cmd_writetrack()
{
	// type-3 write track
	cmdtype = FDC_CMD_WR_TRK;
//	status = FDC_ST_DRQ | FDC_ST_BUSY;
	status = FDC_ST_BUSY;
	
	fdc[drvreg].index = 0;
	now_search = true;
	
	int bytes = disk[drvreg]->get_track_size() - get_cur_position();
	if(bytes < 12) {
		bytes += disk[drvreg]->get_track_size();
	}
	double time = disk[drvreg]->get_usec_per_bytes(bytes);
	REGISTER_EVENT(EVENT_SEARCH, time);
	CANCEL_EVENT(EVENT_LOST);
}

void MB8877::cmd_forceint()
{
	// type-4 force interrupt
#if 0
	if(!disk[drvreg]->inserted || !motor_on) {
		status = FDC_ST_NOTREADY | FDC_ST_HEADENG;
	}
	else {
		status = FDC_ST_HEADENG;
	}
	cmdtype = FDC_CMD_TYPE4;
#else
	if(cmdtype == 0 || cmdtype == 4) {
		status = 0;
		cmdtype = FDC_CMD_TYPE1;
	}
	status &= ~FDC_ST_BUSY;
#endif
	
	// force interrupt if bit0-bit3 is high
	if(cmdreg & 0x0f) {
		set_irq(true);
	}
	now_search = now_seek = false;
	
	CANCEL_EVENT(EVENT_SEEK);
	CANCEL_EVENT(EVENT_SEEKEND);
	CANCEL_EVENT(EVENT_SEARCH);
	CANCEL_EVENT(EVENT_TYPE4);
	CANCEL_EVENT(EVENT_DRQ);
	CANCEL_EVENT(EVENT_MULTI1);
	CANCEL_EVENT(EVENT_MULTI2);
	CANCEL_EVENT(EVENT_LOST);
	REGISTER_EVENT(EVENT_TYPE4, 100);
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------

uint8 MB8877::search_track()
{
	int trk = fdc[drvreg].track;
	
	if(!disk[drvreg]->get_track(trk, sidereg)) {
		return FDC_ST_SEEKERR;
	}
	
	// verify track number
	if(!(cmdreg & 4)) {
		return 0;
	}
	for(int i = 0; i < disk[drvreg]->sector_num; i++) {
		if(disk[drvreg]->verify[i] == trkreg) {
			return 0;
		}
	}
	return FDC_ST_SEEKERR;
}

uint8 MB8877::search_sector(int trk, int side, int sct, bool compare)
{
	// get track
	if(!disk[drvreg]->get_track(trk, side)) {
		set_irq(true);
		return FDC_ST_RECNFND;
	}
	
	// get current position
	int sector_num = disk[drvreg]->sector_num;
	int position = get_cur_position();
	
	if(position > disk[drvreg]->sync_position[sector_num - 1]) {
		position -= disk[drvreg]->get_track_size();
	}
	
	// first scanned sector
	int first_sector = 0;
	for(int i = 0; i < sector_num; i++) {
		if(position < disk[drvreg]->sync_position[i]) {
			first_sector = i;
			break;
		}
	}
	
	// scan sectors
	for(int i = 0; i < sector_num; i++) {
		// get sector
		int index = first_sector + i;
		if(index >= sector_num) {
			index -= sector_num;
		}
		disk[drvreg]->get_sector(trk, side, index);
		
		// check id
		if(disk[drvreg]->id[2] != sct) {
			continue;
		}
		// check density
		if(disk[drvreg]->density) {
			continue;
		}
		
		// sector found
		next_trans_position[drvreg] = disk[drvreg]->data_position[i];
		next_sync_position[drvreg] = disk[drvreg]->sync_position[i];
		fdc[drvreg].index = 0;
		return (disk[drvreg]->deleted ? FDC_ST_RECTYPE : 0) | ((disk[drvreg]->status && !ignore_crc) ? FDC_ST_CRCERR : 0);
	}
	
	// sector not found
	disk[drvreg]->sector_size = 0;
	set_irq(true);
	return FDC_ST_RECNFND;
}

uint8 MB8877::search_addr()
{
	int trk = fdc[drvreg].track;
	
	// get track
	if(!disk[drvreg]->get_track(trk, sidereg)) {
		set_irq(true);
		return FDC_ST_RECNFND;
	}
	
	// get current position
	int sector_num = disk[drvreg]->sector_num;
	int position = get_cur_position();
	
	if(position > disk[drvreg]->sync_position[sector_num - 1]) {
		position -= disk[drvreg]->get_track_size();
	}
	
	// first scanned sector
	int first_sector = 0;
	for(int i = 0; i < sector_num; i++) {
		if(position < disk[drvreg]->sync_position[i]) {
			first_sector = i;
			break;
		}
	}
	
	// get sector
	if(disk[drvreg]->get_sector(trk, sidereg, first_sector)) {
		next_trans_position[drvreg] = disk[drvreg]->id_position[first_sector];
		next_sync_position[drvreg] = disk[drvreg]->sync_position[first_sector];
		fdc[drvreg].index = 0;
		secreg = disk[drvreg]->id[0];
		return (disk[drvreg]->status && !ignore_crc) ? FDC_ST_CRCERR : 0;
	}
	
	// sector not found
	disk[drvreg]->sector_size = 0;
	set_irq(true);
	return FDC_ST_RECNFND;
}

// ----------------------------------------------------------------------------
// timing
// ----------------------------------------------------------------------------

int MB8877::get_cur_position()
{
	return (int)(cur_position[drvreg] + passed_usec(prev_clock[drvreg]) / disk[drvreg]->get_usec_per_bytes(1)) % disk[drvreg]->get_track_size();
}

double MB8877::get_usec_to_start_trans()
{
	// get current position
	int position = get_cur_position();
	int bytes = next_trans_position[drvreg] - position;
	if(next_sync_position[drvreg] < position) {
		bytes += disk[drvreg]->get_track_size();
	}
	double time = disk[drvreg]->get_usec_per_bytes(bytes);
	if(after_seek) {
		// wait 70msec to read/write data just after seek command is done
//		if(time < 70000) {
//			time += disk[drvreg]->get_usec_per_bytes(disk[drvreg]->get_track_size());
//		}
		after_seek = false;
	}
	return time;
}

// ----------------------------------------------------------------------------
// irq / drq
// ----------------------------------------------------------------------------

void MB8877::set_irq(bool val)
{
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
}

void MB8877::set_drq(bool val)
{
	write_signals(&outputs_drq, val ? 0xffffffff : 0);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void MB8877::open_disk(int drv, _TCHAR path[], int offset)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(path, offset);
	}
}

void MB8877::close_disk(int drv)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->close();
		cmdtype = 0;
	}
}

bool MB8877::disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

void MB8877::set_drive_type(int drv, uint8 type)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_type = type;
	}
}

uint8 MB8877::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->drive_type;
	}
	return DRIVE_TYPE_UNK;
}

void MB8877::set_drive_rpm(int drv, int rpm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_rpm = rpm;
	}
}

void MB8877::set_drive_mfm(int drv, bool mfm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_mfm = mfm;
	}
}

uint8 MB8877::fdc_status()
{
	// for each virtual machines
#if defined(_FMR50) || defined(_FMR60)
	return disk[drvreg]->inserted ? 2 : 0;
#else
	return 0;
#endif
}
