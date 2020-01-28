/*
	EPSON HC-40 Emulator 'eHC-40'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.02.23 -

	[ i/o ]
*/

#include "io.h"
#include "../../fifo.h"

// interrupt bits
#define BIT_7508	0x01
#define BIT_ART		0x02
#define BIT_ICF		0x04
#define BIT_OVF		0x08
#define BIT_EXT		0x10

// art (8251 subset)
#define BUFFER_SIZE	0x40000
#define RECV_DELAY	100
#define TXRDY		0x01
#define RXRDY		0x02
#define TXE		0x04
#define PE		0x08
#define OE		0x10
#define FE		0x20
#define SYNDET		0x40
#define DSR		0x80

void IO::initialize()
{
	// init external ram disk
	_memset(ext, 0, 0x20000);
	_memset(ext + 0x20000, 0xff, 0x20000);
	extar = 0;
	extcr = 0;
	
	// load external ram disk
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sEXTRAM.BIN"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ext, 0x20000, 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sEXT.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ext + 0x20000, 0x20000, 1);
		fio->Fclose();
	}
	delete fio;
	
	// init sub cpu
	cmd_buf = new FIFO(16);
	rsp_buf = new FIFO(16);
	key_buf = new FIFO(7);
	art_buf = new FIFO(BUFFER_SIZE);
	
	// regist events
	vm->regist_frame_event(this);
	int id;
	vm->regist_event_by_clock(this, EVENT_FRC, 0x60000, true, &id);
	vm->regist_event_by_clock(this, EVENT_ONESEC, CPU_CLOCKS, true, &id);
	
	// set pallete
	pd = RGB_COLOR(48, 56, 16);
	pb = RGB_COLOR(160, 168, 160);
	
	// init 7508
	onesec_intr = alarm_intr = false;
	onesec_intr_enb = alarm_intr_enb = kb_intr_enb = true;
	res_7508 = kb_caps = false;
}

void IO::release()
{
	// save external ram disk
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sEXTRAM.BIN"), app_path);
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ext, 0x20000, 1);
		fio->Fclose();
	}
	delete fio;
}

void IO::reset()
{
	// reset gapnit
	bcr = slbcr = isr = ier = bankr = ioctlr = 0;
	icrc = icrb = 0;
	ear = beep = false;
	res_z80 = true;
	
	// reset art
	artdir = 0xff;
	artsr = TXRDY | TXE;
	txen = rxen = false;
	art_buf->clear();
	regist_id = -1;
}

void IO::sysreset()
{
	// reset 7508
	onesec_intr = alarm_intr = false;
	onesec_intr_enb = alarm_intr_enb = kb_intr_enb = true;
	res_7508 = true;
}

void IO::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_IO_DREC) {
		// signal from data recorder
		if(!slbcr) {
			bool next = ((data & mask) != 0);
			if((bcr == 2 && ear && !next) || (bcr == 4 && !ear && next) || (bcr == 6 && ear != next)) {
				icrb = vm->passed_clock(cur_clock) / 6;
				isr |= BIT_ICF;
				update_intr();
			}
			ear = next;
		}
	}
	else if(id == SIG_IO_ART) {
		// data from art
		art_buf->write(data & mask);
		if(rxen && !art_buf->empty() && regist_id == -1)
			vm->regist_event(this, EVENT_ART, RECV_DELAY, false, &regist_id);
	}
}

void IO::event_frame()
{
	d_beep->write_signal(did_beep, beep ? 1 : 0, 1);
	beep = false;
}

void IO::event_callback(int event_id, int err)
{
	if(event_id == EVENT_FRC) {
		// FRC overflow event
		cur_clock = vm->current_clock();
		isr |= BIT_OVF;
		update_intr();
	}
	else if(event_id == EVENT_ONESEC) {
		// 1sec interval event
		onesec_intr = true;
		if(onesec_intr_enb) {
			isr |= BIT_7508;
			update_intr();
		}
	}
	else if(event_id == EVENT_ART) {
		// recv from art event
		if(rxen && !(artsr & RXRDY)) {
			if(!art_buf->empty())
				artdir = art_buf->read();
			if(art_buf->empty())
				artsr |= PE;
			artsr |= RXRDY;
			// interrupt
			isr |= BIT_ART;
			update_intr();
		}
		// if data is still left in buffer, register event for next data
		if(rxen && !art_buf->empty())
			vm->regist_event(this, EVENT_ART, RECV_DELAY, false, &regist_id);
		else
			regist_id = -1;
	}
}

void IO::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff)
	{
	case 0x00:
		// CTLR1
		bcr = data & 6;
		slbcr = data & 1;
		break;
	case 0x01:
		// CMDR
		if(data & 4) {
			isr &= ~BIT_OVF;
			update_intr();
		}
		//if(data & 2)
		//	rdysio = false;
		//if(data & 1)
		//	rdysio = true;
		break;
	case 0x02:
		// CTLR2
		d_drec->write_signal(did_mic, data, 1);
		d_drec->write_signal(did_rmt, data, 2);
		break;
	case 0x04:
		// IER
		ier = data;
		break;
	case 0x05:
		// BANKR
		d_mem->write_signal(0, data, 0xf0);
		bankr = data;
		// dont care EDU,ECA and CKSW now...
		break;
	case 0x06:
		// SIOR
		send_to_7508(data);
		break;
	case 0x08:
		// VADR
		vadr = data;
		break;
	case 0x09:
		// YOFF
		yoff = data;
		break;
	case 0x0a:
		// FR: dont care, always 72Hz
		break;
	case 0x0b:
		// SPUR: dont care
		break;
	case 0x14:
		// ARTDOR
		if(txen)
			d_art->write_signal(did_art, data, 0xff);
		break;
	case 0x15:
		// ARTMR
		break;
	case 0x16:
		// ARTCR
		if(data & 0x10)
			artsr &= ~(PE | OE | FE);
		txen = ((data & 1) != 0);
		rxen = ((data & 4) != 0);
		if(rxen && !art_buf->empty() && regist_id == -1)
			vm->regist_event(this, EVENT_ART, RECV_DELAY, false, &regist_id);
		break;
	case 0x19:
		// IOCTLR
		if((ioctlr & 0x80) != (data & 0x80))
			beep = true;
		ioctlr = data;
		break;
	case 0x90:
		// EXTAR
		extar = (extar & 0xffff00) | data;
		break;
	case 0x91:
		// EXTAR
		extar = (extar & 0xff00ff) | (data << 8);
		break;
	case 0x92:
		// EXTAR
		extar = (extar & 0x00ffff) | ((data & 7) << 16);
		break;
	case 0x93:
		// EXTOR
		if(extar < 0x20000)
			ext[extar] = data;
		extar = (extar & 0xffff00) | ((extar + 1) & 0xff);
		break;
	case 0x94:
		// EXTCR
		extcr = data;
		break;
	}
}

uint32 IO::read_io8(uint32 addr)
{
	uint32 val = 0xff;
	
	switch(addr & 0xff)
	{
	case 0x00:
		// ICRL.C (latch FRC value)
		icrc = vm->passed_clock(cur_clock) / 6;
		return icrc & 0xff;
	case 0x01:
		// ICRH.C
		return (icrc >> 8) & 0xff;
	case 0x02:
		// ICRL.B
		return icrb & 0xff;
	case 0x03:
		// ICRH.B
		isr &= ~BIT_ICF;
		update_intr();
		return (icrb >> 8) & 0xff;
	case 0x04:
		// ISR
		return isr;
	case 0x05:
		// STR
		return (bankr & 0xf0) | 8 | 4 | (ear ? 1 : 0);	// always rdysio=rdy=true
	case 0x06:
		// SIOR
		return rec_from_7508();
	case 0x14:
		// ARTDIR
		isr &= ~BIT_ART;
		update_intr();
		artsr &= ~RXRDY;
		return artdir;
	case 0x15:
		// ARTSR
		return artsr;
	case 0x16:
		// IOSTR
		return 0x40 | (artsr & RXRDY ? 8 : 0);	// not hand shake mode
	case 0x93:
		// EXTIR
		if(extar < 0x40000)
			val = ext[extar];
		extar = (extar & 0xffff00) | ((extar + 1) & 0xff);
		return val;
	case 0x94:
		// EXTSR
		return extcr & ~0x80;
	}
	return 0xff;
}

uint32 IO::intr_ack()
{
	if(isr & BIT_7508) {
		isr &= ~BIT_7508;
		return 0xf0;
	}
	else if(isr & BIT_ART)
		return 0xf2;
	else if(isr & BIT_ICF)
		return 0xf4;
	else if(isr & BIT_OVF)
		return 0xf6;
	else if(isr & BIT_EXT)
		return 0xf8;
	// unknown
	return 0xff;
}

void IO::update_intr()
{
	// set int signal
	bool next = ((isr & ier & 0x1f) != 0);
	d_cpu->set_intr_line(next, true, 0);
}

// ----------------------------------------------------------------------------
// 7508
// ----------------------------------------------------------------------------

#define YEAR		time[0]
#define MONTH		time[1]
#define DAY		time[2]
#define DAY_OF_WEEK	time[3]
#define HOUR		time[4]
#define MINUTE		time[5]
#define SECOND		time[6]

#define DIGIT(t) ((int)((t) / 10) * 16 + (t) % 10)

void IO::send_to_7508(uint8 val)
{
	int time[8], res;
	
	// process command
	cmd_buf->write(val);
	uint8 cmd = cmd_buf->read_not_remove(0);
	
	switch(cmd)
	{
	case 0x01:
		// power off
		cmd_buf->read();
		break;
	case 0x02:
		// status / key
		cmd_buf->read();
		if((onesec_intr && onesec_intr_enb) || (alarm_intr && alarm_intr_enb) || res_z80 || res_7508) {
			res = 0xc1;
			res |= (onesec_intr && onesec_intr_enb) ? 0x20 : 0;
			res |= (res_z80 ? 0x10 : 0) | (res_7508 ? 8 : 0);
			res |= (alarm_intr && alarm_intr_enb) ? 2 : 0;
			// clear interrupt
			onesec_intr = alarm_intr = res_z80 = res_7508 = false;
		}
		else if(key_buf->count())
			res = key_buf->read();
		else
			res = 0xbf;
		rsp_buf->write(res);
		// request next interrupt
		if(key_buf->count() && kb_intr_enb) {
			isr |= BIT_7508;
			update_intr();
		}
		break;
	case 0x03:
		// kb reset
		cmd_buf->read();
		key_buf->clear();
		kb_rep_spd1 = 42 | 0x80;
		kb_rep_spd2 = 18 | 0x80;
		kb_intr_enb = true;
		break;
	case 0x04:
		// kb repeat timer 1 set
		if(cmd_buf->count() == 2) {
			cmd_buf->read();
			kb_rep_spd1 = cmd_buf->read();
		}
		break;
	case 0x05:
		// kb repeat off
		cmd_buf->read();
		kb_rep_enb = false;
		break;
	case 0x06:
		// kb interrupt off
		cmd_buf->read();
		kb_intr_enb = false;
		break;
	case 0x07:
		// clock read
		cmd_buf->read();
		emu->get_timer(time);
		rsp_buf->write((int)((YEAR % 100) / 10));
		rsp_buf->write(YEAR % 10);
		rsp_buf->write(DIGIT(MONTH));
		rsp_buf->write(DIGIT(DAY));
		rsp_buf->write(DIGIT(HOUR));
		rsp_buf->write(DIGIT(MINUTE));
		rsp_buf->write(DIGIT(SECOND));
		rsp_buf->write(DAY_OF_WEEK);
		break;
	case 0x08:
		// power switch read
		cmd_buf->read();
		rsp_buf->write(1);
		break;
	case 0x09:
		// alarm read
		cmd_buf->read();
		rsp_buf->write(alarm[0]);
		rsp_buf->write(alarm[1]);
		rsp_buf->write(alarm[2]);
		rsp_buf->write(alarm[3]);
		rsp_buf->write(alarm[4]);
		rsp_buf->write(alarm[5]);
		break;
	case 0x0a:
		// dip switch read
		cmd_buf->read();
		res = 0;	// standard keyboard = 0, item keyboard = 0x80
		res |= 0x60;	// serial = 0, cart-printer = 0x20, rs-232c = 0x40, printer = 0x60
//		res |= 0x30;	// serial = 0, cart-printer = 0x10, rs-232c = 0x20, printer = 0x30
		res |= 0xf;	// ascii keyboard
		rsp_buf->write(res);
		break;
	case 0x0b:
		// stop key interrupt disable -> kb interrupt on
		cmd_buf->read();
		kb_intr_enb = true;
		break;
	case 0x0d:
		// 1 sec interrupt off
		cmd_buf->read();
		onesec_intr_enb = false;
		break;
	case 0x0e:
		// kb clear
		cmd_buf->read();
		key_buf->clear();
		break;
	case 0x0f:
		// system reset
		cmd_buf->read();
		res_7508 = true;
		break;
	case 0x14:
		// kb repeat timer 2 set
		if(cmd_buf->count() == 2) {
			cmd_buf->read();
			kb_rep_spd2 = cmd_buf->read();
		}
		break;
	case 0x15:
		// kb repeat on
		cmd_buf->read();
		kb_rep_enb = true;
		break;
	case 0x16:
		// kb interrupt on
		cmd_buf->read();
		kb_intr_enb = true;
		break;
	case 0x17:
		// clock write
		if(cmd_buf->count() == 9) {
			cmd_buf->read();
			cmd_buf->read();	// year 10
			cmd_buf->read();	// year 1
			cmd_buf->read();	// month
			cmd_buf->read();	// day
			cmd_buf->read();	// hour
			cmd_buf->read();	// minute
			cmd_buf->read();	// second
			cmd_buf->read();	// day of week
		}
		break;
	case 0x19:
		// alarm set
		if(cmd_buf->count() == 7) {
			cmd_buf->read();
			alarm[0] = cmd_buf->read();
			alarm[1] = cmd_buf->read();
			alarm[2] = cmd_buf->read();
			alarm[3] = cmd_buf->read();
			alarm[4] = cmd_buf->read();
			alarm[5] = cmd_buf->read();
		}
		break;
	case 0x1b:
		// stop key interrupt enable -> kb interrupt off
		cmd_buf->read();
		kb_intr_enb = false;
		break;
	case 0x1d:
		// 1 sec interrupt on
		cmd_buf->read();
		onesec_intr_enb = true;
		break;
	case 0x24:
		// kb repeat timer 1 read
		cmd_buf->read();
		rsp_buf->write(kb_rep_spd1);
		break;
	case 0x29:
		// alarm off
		cmd_buf->read();
		alarm_intr_enb = false;
		break;
	case 0x34:
		// kb repeat timer 2 read
		cmd_buf->read();
		rsp_buf->write(kb_rep_spd2);
		break;
	case 0x39:
		// alarm on
		cmd_buf->read();
		alarm_intr_enb = true;
		break;
	default:
		// unknown cmd
		cmd_buf->read();
		emu->out_debug(_T("unknown cmd %2x\n"), cmd);
	}
}

uint8 IO::rec_from_7508()
{
	return rsp_buf->read();
}

void IO::key_down(int code)
{
	if(code == 0x14) {
		// toggle caps lock
		kb_caps = !kb_caps;
		update_key(kb_caps ? 0xb4 : 0xa4);
		update_key(kb_caps ? 0xa4 : 0xb4);
	}
	else
		update_key(key_tbl[code & 0xff]);
}

void IO::key_up(int code)
{
	if(code == 0x10)
		update_key(0xa3);	// break shift
	else if(code == 0x11)
		update_key(0xa2);	// break ctrl
}

void IO::update_key(int code)
{
	if(code != 0xff) {
		// add to buffer
		if(code == 0x10) {
			// stop key
			key_buf->clear();
			key_buf->write(code);
		}
		else
			key_buf->write(code);
		
		// key interrupt
		if(kb_intr_enb || (!kb_intr_enb && code == 0x10)) {
			isr |= BIT_7508;
			update_intr();
		}
	}
}

// ----------------------------------------------------------------------------
// video
// ----------------------------------------------------------------------------

void IO::draw_screen()
{
	if(yoff & 0x80) {
		uint8* vram = ram + ((vadr & 0xf8) << 8);
		for(int y = 0; y < 64; y++) {
			scrntype* dest = emu->screen_buffer((y - (yoff & 0x3f)) & 0x3f);
			for(int x = 0; x < 30; x++) {
				uint8 pat = *vram++;
				dest[0] = (pat & 0x80) ? pd : pb;
				dest[1] = (pat & 0x40) ? pd : pb;
				dest[2] = (pat & 0x20) ? pd : pb;
				dest[3] = (pat & 0x10) ? pd : pb;
				dest[4] = (pat & 0x08) ? pd : pb;
				dest[5] = (pat & 0x04) ? pd : pb;
				dest[6] = (pat & 0x02) ? pd : pb;
				dest[7] = (pat & 0x01) ? pd : pb;
				dest += 8;
			}
			vram += 2;
		}
	}
	else {
		for(int y = 0; y < 64; y++) {
			scrntype* dest = emu->screen_buffer(y);
			for(int x = 0; x < 240; x++)
				dest[x] = pb;
		}
	}
}
