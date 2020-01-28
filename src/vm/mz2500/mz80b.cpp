/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.14-

	[ virtual machine ]
*/

#include "mz80b.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../io.h"
#include "../mb8877.h"
#include "../mz1p17.h"
#include "../pcm1bit.h"
#include "../prnfile.h"
#include "../z80.h"
#include "../z80pio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "cmt.h"
#include "floppy.h"
#include "keyboard.h"
#include "memory80b.h"
#include "mz1r12.h"
#include "mz1r13.h"
#include "printer.h"
#include "timer.h"

#ifdef SUPPORT_QUICK_DISK
#include "../z80sio.h"
#include "../mz700/quickdisk.h"
#endif

#ifdef SUPPORT_16BIT_BOARD
#include "../i286.h"
#include "../i8259.h"
#include "mz1m01.h"
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	pit = new I8253(this, emu);
	pio_i = new I8255(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	pcm = new PCM1BIT(this, emu);
	cpu = new Z80(this, emu);
	pio = new Z80PIO(this, emu);
	
	cmt = new CMT(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	mz1r12 = new MZ1R12(this, emu);
	mz1r13 = new MZ1R13(this, emu);
	printer = new PRINTER(this, emu);
	timer = new TIMER(this, emu);
	
#ifdef SUPPORT_QUICK_DISK
	sio = new Z80SIO(this, emu);
	qd = new QUICKDISK(this, emu);
#endif
	
#ifdef SUPPORT_16BIT_BOARD
	pio_to16 = new Z80PIO(this, emu);
	cpu_16 = new I286(this, emu);	// 8088
	pic_16 = new I8259(this, emu);
	mz1m01 = new MZ1M01(this, emu);
#endif
	
	// set contexts
	event->set_context_cpu(cpu);
#ifdef SUPPORT_16BIT_BOARD
	event->set_context_cpu(cpu_16, 5000000);
#endif
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
	
	drec->set_context_ear(cmt, SIG_CMT_OUT, 1);
	drec->set_context_remote(cmt, SIG_CMT_REMOTE, 1);
	drec->set_context_end(cmt, SIG_CMT_END, 1);
	drec->set_context_top(cmt, SIG_CMT_TOP, 1);
	
	pit->set_context_ch0(pit, SIG_I8253_CLOCK_1, 1);
	pit->set_context_ch1(pit, SIG_I8253_CLOCK_2, 1);
	pit->set_constant_clock(0, 31250);
	pio_i->set_context_port_a(cmt, SIG_CMT_PIO_PA, 0xff, 0);
	pio_i->set_context_port_a(memory, SIG_CRTC_REVERSE, 0x10, 0);
	pio_i->set_context_port_c(cmt, SIG_CMT_PIO_PC, 0xff, 0);
	pio_i->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x04, 0);
	pio->set_context_port_a(memory, SIG_MEMORY_VRAM_SEL, 0xc0, 0);
	pio->set_context_port_a(memory, SIG_CRTC_WIDTH80, 0x20, 0);
	pio->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0x1f, 0);
	
	cmt->set_context_pio(pio_i);
	cmt->set_context_drec(drec);
	floppy->set_context_fdc(fdc);
	keyboard->set_context_pio_i(pio_i);
	keyboard->set_context_pio(pio);
	memory->set_context_cpu(cpu);
	memory->set_context_pio(pio_i);
	if(config.printer_device_type == 0) {  
		printer->set_context_prn(new PRNFILE(this, emu));
	} else if(config.printer_device_type == 1) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_MZ1;
		printer->set_context_prn(mz1p17);
	} else if(config.printer_device_type == 2) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_MZ3;
		printer->set_context_prn(mz1p17);
	} else {
		printer->set_context_prn(dummy);
	}
	timer->set_context_pit(pit);
	
#ifdef SUPPORT_QUICK_DISK
	// Z80SIO:RTSA -> QD:WRGA
	sio->set_context_rts(0, qd, QUICKDISK_SIO_RTSA, 1);
	// Z80SIO:DTRB -> QD:MTON
	sio->set_context_dtr(1, qd, QUICKDISK_SIO_DTRB, 1);
	// Z80SIO:SENDA -> QD:RECV
	sio->set_context_sync(0, qd, QUICKDISK_SIO_SYNC, 1);
	sio->set_context_rxdone(0, qd, QUICKDISK_SIO_RXDONE, 1);
	sio->set_context_send(0, qd, QUICKDISK_SIO_DATA);
	sio->set_context_break(0, qd, QUICKDISK_SIO_BREAK, 1);
	// Z80SIO:CTSA <- QD:PROTECT
	// Z80SIO:DCDA <- QD:INSERT
	// Z80SIO:DCDB <- QD:HOE
	qd->set_context_sio(sio);
	
	sio->set_tx_clock(0, 101562.5);
	sio->set_rx_clock(0, 101562.5);
	sio->set_tx_clock(1, 101562.5);
	sio->set_rx_clock(1, 101562.5);
#endif
	
#ifdef SUPPORT_16BIT_BOARD
	pio_to16->set_context_port_a(mz1m01, SIG_MZ1M01_PORT_A, 0xff, 0);
	pio_to16->set_context_port_b(mz1m01, SIG_MZ1M01_PORT_B, 0x80, 0);
	pio_to16->set_context_ready_a(pic_16, SIG_I8259_IR0, 1);
	pio_to16->set_context_ready_b(pic_16, SIG_I8259_IR1, 1);
	pio_to16->set_context_port_b(pic_16, SIG_I8259_IR2, 0x80, 0);
	pio_to16->set_hand_shake(0, true);
	pio_to16->set_hand_shake(1, true);
	pic_16->set_context_cpu(cpu_16);
	cpu_16->set_context_mem(mz1m01);
	cpu_16->set_context_io(mz1m01);
	cpu_16->set_context_intr(pic_16);
#ifdef USE_DEBUGGER
	cpu_16->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	mz1m01->set_context_cpu(cpu_16);
	mz1m01->set_context_pic(pic_16);
	mz1m01->set_context_pio(pio_to16);
#endif
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	
	// z80 family daisy chain
#ifdef SUPPORT_16BIT_BOARD
	// FIXME: Z80PIO on MZ-1M01 is not daisy-chained to other Z80 family !!!
	cpu->set_context_intr(pio_to16);
	pio_to16->set_context_intr(cpu, 0);
	pio_to16->set_context_child(pio);
#else
	cpu->set_context_intr(pio);
#endif
	pio->set_context_intr(cpu, 1);
#ifdef SUPPORT_QUICK_DISK
	pio->set_context_child(sio);
	sio->set_context_intr(cpu, 2);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0xb8, 0xbb, mz1r13);
#ifdef SUPPORT_QUICK_DISK
	io->set_iomap_alias_rw(0xd0, sio, 0);
	io->set_iomap_alias_rw(0xd1, sio, 2);
	io->set_iomap_alias_rw(0xd2, sio, 1);
	io->set_iomap_alias_rw(0xd3, sio, 3);
#endif
#ifdef SUPPORT_16BIT_BOARD
	io->set_iomap_range_rw(0xd4, 0xd7, pio_to16);
#endif
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdd, floppy);
	io->set_iomap_range_rw(0xe0, 0xe3, pio_i);
	io->set_iomap_range_rw(0xe4, 0xe7, pit);
	io->set_iomap_range_rw(0xe8, 0xeb, pio);
	io->set_iomap_range_w(0xf0, 0xf3, timer);
	io->set_iomap_range_w(0xf4, 0xf7, memory);
	io->set_iomap_range_rw(0xf8, 0xfa, mz1r12);
	io->set_iomap_range_rw(0xfe, 0xff, printer);
	
	io->set_iowait_range_rw(0xd8, 0xdf, 1);
	io->set_iowait_range_rw(0xe8, 0xeb, 1);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
//		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
	}
}

VM::~VM()
{
	// delete all devices
	for(DEVICE* device = first_device; device;) {
		DEVICE *next_device = device->next_device;
		device->release();
		delete device;
		device = next_device;
	}
}

DEVICE* VM::get_device(int id)
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id == id) {
			return device;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
}

void VM::special_reset()
{
	// reset all devices
//	for(DEVICE* device = first_device; device; device = device->next_device) {
//		device->special_reset();
//	}
	memory->special_reset();
	cpu->reset();
#ifdef SUPPORT_16BIT_BOARD
	pio_to16->reset();
	cpu_16->reset();
	pic_16->reset();
	mz1m01->reset();
#endif
}

void VM::run()
{
	event->drive();
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return cpu;
#ifdef SUPPORT_16BIT_BOARD
	} else if(index == 1) {
		return cpu_16;
#endif
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	memory->draw_screen();
}

uint32_t VM::get_access_lamp_status()
{
#ifdef SUPPORT_QUICK_DISK
	uint32_t status = fdc->read_signal(0) | qd->read_signal(0);
#else
	uint32_t status = fdc->read_signal(0);
#endif
	return (status & (1 | 4)) ? 1 : (status & (2 | 8)) ? 2 : 0;
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	pcm->initialize_sound(rate, 8000);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return event->get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		drec->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
}

void VM::close_floppy_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::is_floppy_disk_inserted(int drv)
{
	return fdc->is_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	fdc->is_disk_protected(drv, value);
}

bool VM::is_floppy_disk_protected(int drv)
{
	return fdc->is_disk_protected(drv);
}

#ifdef SUPPORT_QUICK_DISK
void VM::open_quick_disk(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		qd->open_disk(file_path);
	}
}

void VM::close_quick_disk(int drv)
{
	if(drv == 0) {
		qd->close_disk();
	}
}

bool VM::is_quick_disk_inserted(int drv)
{
	if(drv == 0) {
		return qd->is_disk_inserted();
	} else {
		return false;
	}
}
#endif

void VM::play_tape(const _TCHAR* file_path)
{
	if(check_file_extension(file_path, _T(".dat"))) {
		memory->load_dat_image(file_path);
		return;
	} else if(check_file_extension(file_path, _T(".mzt")) || check_file_extension(file_path, _T(".mzf"))) {
		if(config.direct_load_mzt && memory->load_mzt_image(file_path)) {
			return;
		}
	} else if(check_file_extension(file_path, _T(".mtw"))) {
		memory->load_mzt_image(file_path);
	}
	bool value = drec->play_tape(file_path);
	cmt->close_tape();
	cmt->play_tape(value);
}

void VM::rec_tape(const _TCHAR* file_path)
{
	bool value = drec->rec_tape(file_path);
	cmt->close_tape();
	cmt->rec_tape(value);
}

void VM::close_tape()
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	cmt->close_tape();
}

bool VM::is_tape_inserted()
{
	return drec->is_tape_inserted();
}

bool VM::is_tape_playing()
{
	return drec->is_tape_playing();
}

bool VM::is_tape_recording()
{
	return drec->is_tape_recording();
}

int VM::get_tape_position()
{
	return drec->get_tape_position();
}

void VM::push_play()
{
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop()
{
	drec->set_remote(false);
}

void VM::push_fast_forward()
{
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind()
{
	drec->set_ff_rew(-1);
	drec->set_remote(true);
}

bool VM::is_frame_skippable()
{
	return event->is_frame_skippable();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

#define STATE_VERSION	2

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
}

bool VM::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(!device->load_state(state_fio)) {
			return false;
		}
	}
	return true;
}

