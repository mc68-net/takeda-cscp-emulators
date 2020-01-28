/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.11-

	[ virtual machine ]
*/

#include "x1.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../hd46505.h"
#include "../i8255.h"
#include "../io.h"
#include "../mb8877.h"
#include "../mz1p17.h"
//#include "../pcpr201.h"
#include "../prnfile.h"
#include "../ym2151.h"
#include "../ym2203.h"
#include "../z80.h"
#include "../z80ctc.h"
#include "../z80sio.h"
#ifdef _X1TURBO_FEATURE
#include "../z80dma.h"
#endif

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "emm.h"
#include "floppy.h"
#include "iobus.h"
#include "joystick.h"
#include "memory.h"
#include "mouse.h"
#include "psub.h"

#include "../mcs48.h"
#include "../upd1990a.h"
#include "sub.h"
#include "keyboard.h"

#ifdef _X1TWIN
#include "../huc6280.h"
#include "../pcengine/pce.h"
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	pseudo_sub_cpu = !(FILEIO::IsFileExisting(create_local_path(SUB_ROM_FILE_NAME)) && FILEIO::IsFileExisting(create_local_path(KBD_ROM_FILE_NAME)));
	
	sound_device_type = config.sound_device_type;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	crtc = new HD46505(this, emu);
	pio = new I8255(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	psg = new YM2203(this, emu);
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);
	sio = new Z80SIO(this, emu);
	if(sound_device_type >= 1) {
		opm1 = new YM2151(this, emu);
		ctc1 = new Z80CTC(this, emu);
	}
	if(sound_device_type == 2) {
		opm2 = new YM2151(this, emu);
		ctc2 = new Z80CTC(this, emu);
	}
	if(config.printer_device_type == 0) {
		printer = new PRNFILE(this, emu);
	} else if(config.printer_device_type == 1) {
		printer = new MZ1P17(this, emu);
//	} else if(config.printer_device_type == 2) {
//		printer = new PCPR201(this, emu);
	} else {
		printer = dummy;
	}
#ifdef _X1TURBO_FEATURE
	dma = new Z80DMA(this, emu);
#endif
	
	display = new DISPLAY(this, emu);
	emm = new EMM(this, emu);
	floppy = new FLOPPY(this, emu);
	iobus = new IOBUS(this, emu);
	joy = new JOYSTICK(this, emu);
	memory = new MEMORY(this, emu);
	mouse = new MOUSE(this, emu);
	
	if(pseudo_sub_cpu) {
		psub = new PSUB(this, emu);
		cpu_sub = NULL;
		cpu_kbd = NULL;
	} else {
		// sub cpu
		cpu_sub = new MCS48(this, emu);
		pio_sub = new I8255(this, emu);
		rtc_sub = new UPD1990A(this, emu);
		sub = new SUB(this, emu);
		
		// keyboard
		cpu_kbd = new MCS48(this, emu);
		kbd = new KEYBOARD(this, emu);
	}
	
	// set contexts
	event->set_context_cpu(cpu);
	if(!pseudo_sub_cpu) {
		event->set_context_cpu(cpu_sub, 6000000);
		event->set_context_cpu(cpu_kbd, 6000000);
	}
	if(sound_device_type >= 1) {
		event->set_context_sound(opm1);
	}
	if(sound_device_type == 2) {
		event->set_context_sound(opm2);
	}
	event->set_context_sound(psg);
	event->set_context_sound(drec);
	
	drec->set_context_ear(pio, SIG_I8255_PORT_B, 0x02);
	crtc->set_context_vblank(display, SIG_DISPLAY_VBLANK, 1);
	crtc->set_context_vblank(pio, SIG_I8255_PORT_B, 0x80);
	crtc->set_context_vsync(pio, SIG_I8255_PORT_B, 0x04);
	pio->set_context_port_a(printer, SIG_PRINTER_DATA, 0xff, 0);
	pio->set_context_port_c(drec, SIG_DATAREC_MIC, 0x01, 0);
	pio->set_context_port_c(display, SIG_DISPLAY_COLUMN40, 0x40, 0);
	pio->set_context_port_c(iobus, SIG_IOBUS_MODE, 0x60, 0);
	pio->set_context_port_c(printer, SIG_PRINTER_STROBE, 0x80, 0);
#ifdef _X1TURBO_FEATURE
	fdc->set_context_drq(dma, SIG_Z80DMA_READY, 1);
#endif
	ctc->set_context_zc0(ctc, SIG_Z80CTC_TRIG_3, 1);
	ctc->set_context_zc1(sio, SIG_Z80SIO_TX_CLK_CH0, 1);
	ctc->set_context_zc1(sio, SIG_Z80SIO_RX_CLK_CH0, 1);
	ctc->set_context_zc2(sio, SIG_Z80SIO_TX_CLK_CH1, 1);
	ctc->set_context_zc2(sio, SIG_Z80SIO_RX_CLK_CH1, 1);
	ctc->set_constant_clock(1, CPU_CLOCKS >> 1);
	ctc->set_constant_clock(2, CPU_CLOCKS >> 1);
	sio->set_context_rts(1, mouse, SIG_MOUSE_RTS, 1);
//	sio->set_tx_clock(0, 9600 * 16);	// 9600 baud for RS-232C
//	sio->set_rx_clock(0, 9600 * 16);	// clock is from Z-80CTC ch1 (2MHz/13)
//	sio->set_tx_clock(1, 4800 * 16);	// 4800 baud for mouse
//	sio->set_rx_clock(1, 4800 * 16);	// clock is from Z-80CTC ch2 (2MHz/26)
	
	if(sound_device_type >= 1) {
		ctc1->set_context_zc0(ctc1, SIG_Z80CTC_TRIG_3, 1);
//		ctc1->set_constant_clock(1, CPU_CLOCKS >> 1);
//		ctc1->set_constant_clock(2, CPU_CLOCKS >> 1);
	}
	if(sound_device_type == 2) {
		ctc2->set_context_zc0(ctc2, SIG_Z80CTC_TRIG_3, 1);
//		ctc2->set_constant_clock(1, CPU_CLOCKS >> 1);
//		ctc2->set_constant_clock(2, CPU_CLOCKS >> 1);
	}
	if(config.printer_device_type == 0) {
		PRNFILE *prnfile = (PRNFILE *)printer;
		prnfile->set_context_busy(pio, SIG_I8255_PORT_B, 0x08);
	} else if(config.printer_device_type == 1) {
		MZ1P17 *mz1p17 = (MZ1P17 *)printer;
		mz1p17->mode = MZ1P17_MODE_X1;
		mz1p17->set_context_busy(pio, SIG_I8255_PORT_B, 0x08);
//	} else if(config.printer_device_type == 2) {
//		PCPR201 *pcpr201 = (PCPR201 *)printer;
//		pcpr201->set_context_busy(pio, SIG_I8255_PORT_B, 0x08);
	}
#ifdef _X1TURBO_FEATURE
	dma->set_context_memory(memory);
	dma->set_context_io(iobus);
#endif
	
#ifdef _X1TURBO_FEATURE
	display->set_context_cpu(cpu);
#endif
	display->set_context_crtc(crtc);
	display->set_vram_ptr(iobus->get_vram());
	display->set_regs_ptr(crtc->get_regs());
	floppy->set_context_fdc(fdc);
	iobus->set_context_cpu(cpu);
	iobus->set_context_display(display);
	iobus->set_context_io(io);
	joy->set_context_psg(psg);
#ifdef _X1TURBO_FEATURE
	memory->set_context_pio(pio);
#endif
	mouse->set_context_sio(sio);
	
	if(pseudo_sub_cpu) {
		drec->set_context_remote(psub, SIG_PSUB_TAPE_REMOTE, 1);
		drec->set_context_end(psub, SIG_PSUB_TAPE_END, 1);
		psub->set_context_pio(pio);
		psub->set_context_drec(drec);
	} else {
		// sub cpu
		cpu_sub->set_context_mem(new MCS48MEM(this, emu));
		cpu_sub->set_context_io(sub);
#ifdef USE_DEBUGGER
		cpu_sub->set_context_debugger(new DEBUGGER(this, emu));
#endif
		drec->set_context_end(sub, SIG_SUB_TAPE_END, 1);
		drec->set_context_apss(sub, SIG_SUB_TAPE_APSS, 1);
		pio_sub->set_context_port_c(sub, SIG_SUB_PIO_PORT_C, 0x80, 0);
		// pc1:break -> pb0 of 8255(main)
		pio_sub->set_context_port_c(pio, SIG_I8255_PORT_B, 0x02, -1);
		// pc5:ibf -> pb6 of 8255(main)
		pio_sub->set_context_port_c(pio, SIG_I8255_PORT_B, 0x20, 1);
		// pc7:obf -> pb5 of 8255(main)
		pio_sub->set_context_port_c(pio, SIG_I8255_PORT_B, 0x80, -2);
		// pc7:obf -> pb7 of 8255(sub)
		pio_sub->set_context_port_c(pio_sub, SIG_I8255_PORT_B, 0x80, 0);
		
		sub->set_context_pio(pio_sub);
		sub->set_context_rtc(rtc_sub);
		sub->set_context_drec(drec);
		
		// keyboard
		cpu_kbd->set_context_mem(new MCS48MEM(this, emu));
		cpu_kbd->set_context_io(kbd);
#ifdef USE_DEBUGGER
		cpu_kbd->set_context_debugger(new DEBUGGER(this, emu));
#endif
		kbd->set_context_cpu(cpu_sub);
	}
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(iobus);
#if defined(_X1TURBO_FEATURE) && defined(SINGLE_MODE_DMA)
	cpu->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// z80 family daisy chain
	DEVICE* parent_dev = NULL;
	int level = 0;
	
	#define Z80_DAISY_CHAIN(dev) { \
		if(parent_dev == NULL) { \
			cpu->set_context_intr(dev); \
		} else { \
			parent_dev->set_context_child(dev); \
		} \
		dev->set_context_intr(cpu, level++); \
		parent_dev = dev; \
	}
#ifndef _X1TURBO_FEATURE
	Z80_DAISY_CHAIN(sio);	// CZ-8BM2
	Z80_DAISY_CHAIN(ctc);
#endif
	if(sound_device_type >= 1) {
		Z80_DAISY_CHAIN(ctc1);
	}
	if(sound_device_type == 2) {
		Z80_DAISY_CHAIN(ctc2);
	}
#ifdef _X1TURBO_FEATURE
	Z80_DAISY_CHAIN(sio);
	Z80_DAISY_CHAIN(dma);
	Z80_DAISY_CHAIN(ctc);
#endif
	if(pseudo_sub_cpu) {
		Z80_DAISY_CHAIN(psub);
	} else {
		Z80_DAISY_CHAIN(sub);
	}
	
	// i/o bus
	if(sound_device_type >= 1) {
		io->set_iomap_single_w(0x700, opm1);
		io->set_iovalue_single_r(0x700, 0x00);
		io->set_iomap_single_rw(0x701, opm1);
#ifdef _X1TURBOZ
		io->set_flipflop_single_rw(0x704, 0x00);
#else
		io->set_iomap_range_rw(0x704, 0x707, ctc1);
#endif
	}
	if(sound_device_type == 2) {
		io->set_iomap_single_w(0x708, opm2);
		io->set_iovalue_single_r(0x708, 0x00);
		io->set_iomap_single_rw(0x709, opm2);
		io->set_iomap_range_rw(0x70c, 0x70f, ctc2);
	}
#ifdef _X1TURBO_FEATURE
	io->set_iomap_single_rw(0xb00, memory);
#endif
	io->set_iomap_range_rw(0xd00, 0xd03, emm);
	io->set_iomap_range_r(0xe80, 0xe81, display);
	io->set_iomap_range_w(0xe80, 0xe82, display);
	io->set_iomap_range_rw(0xff8, 0xffb, fdc);
	io->set_iomap_single_w(0xffc, floppy);
#ifdef _X1TURBO_FEATURE
	io->set_iomap_range_r(0xffc, 0xfff, floppy);
#endif
	io->set_iomap_range_rw(0x1000, 0x17ff, display);
	for(int i = 0x1800; i <= 0x18ff; i += 0x10) {
		io->set_iomap_range_rw(i, i + 1, crtc);
	}
	if(pseudo_sub_cpu) {
		io->set_iomap_range_rw(0x1900, 0x19ff, psub);
	} else {
		io->set_iomap_range_rw(0x1900, 0x19ff, sub);
	}
	for(int i = 0x1a00; i <= 0x1aff; i += 4) {
		io->set_iomap_range_rw(i, i + 3, pio);
	}
	for(int i = 0x1b00; i <= 0x1bff; i++) {
		io->set_iomap_alias_rw(i, psg, 1);
	}
	for(int i = 0x1c00; i <= 0x1cff; i++) {
		io->set_iomap_alias_w(i, psg, 0);
	}
	io->set_iomap_range_w(0x1d00, 0x1eff, memory);
#ifndef _X1TURBO_FEATURE
	io->set_iomap_range_rw(0x1f98, 0x1f9b, sio);	// CZ-8BM2
	io->set_iomap_range_rw(0x1fa8, 0x1fab, ctc);
#else
	io->set_iomap_range_rw(0x1f80, 0x1f8f, dma);
	io->set_iomap_range_rw(0x1f90, 0x1f93, sio);
	io->set_iomap_range_rw(0x1fa0, 0x1fa3, ctc);
#ifdef _X1TURBOZ
	io->set_iomap_single_rw(0x1fd0, display);
	io->set_iomap_single_rw(0x1fe0, display);
#else
	io->set_iomap_single_w(0x1fd0, display);
	io->set_iomap_single_w(0x1fe0, display);
#endif
	// 0x1ff0: dipswitch
//	io->set_iovalue_single_r(0x1ff0, 0x00);
	update_dipswitch();
#endif
	io->set_iomap_range_rw(0x2000, 0x3fff, display);	// tvram
	
#ifdef _X1TWIN
	// init PC Engine
	pceevent = new EVENT(this, emu);
	pceevent->set_frames_per_sec(PCE_FRAMES_PER_SEC);
	pceevent->set_lines_per_frame(PCE_LINES_PER_FRAME);
	
	pcecpu = new HUC6280(this, emu);
	pcecpu->set_context_event_manager(pceevent);
	pce = new PCE(this, emu);
	pce->set_context_event_manager(pceevent);
	
	pceevent->set_context_cpu(pcecpu, PCE_CPU_CLOCKS);
	pceevent->set_context_sound(pce);
	
	pcecpu->set_context_mem(pce);
	pcecpu->set_context_io(pce);
#ifdef USE_DEBUGGER
	pcecpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pce->set_context_cpu(pcecpu);
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	if(!pseudo_sub_cpu) {
		// load rom images after cpustate is allocated
		cpu_sub->load_rom_image(create_local_path(SUB_ROM_FILE_NAME));
		cpu_kbd->load_rom_image(create_local_path(KBD_ROM_FILE_NAME));
		
		// patch to set the current year
		uint8_t *rom = cpu_sub->get_rom_ptr();
		sub->rom_crc32 = get_crc32(rom, 0x800);	// 2KB
		if(rom[0x23] == 0xb9 && rom[0x24] == 0x35 && rom[0x25] == 0xb1) {
			cur_time_t cur_time;
			get_host_time(&cur_time);
			rom[0x26] = TO_BCD(cur_time.year);
		}
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
#ifdef _X1TURBO_FEATURE
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
#else
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
#endif
//		fdc->set_drive_rpm(i, 300);
//		fdc->set_drive_mfm(i, true);
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
	pio->write_signal(SIG_I8255_PORT_B, 0x00, 0x08);	// busy = low
	psg->set_reg(0x2e, 0);	// set prescaler
}

void VM::special_reset()
{
	// nmi reset
	cpu->write_signal(SIG_CPU_NMI, 1, 1);
}

void VM::run()
{
	event->drive();
#ifdef _X1TWIN
	if(pce->is_cart_inserted()) {
		pceevent->drive();
	}
#endif
}

double VM::get_frame_rate()
{
#ifdef _X1TWIN
	if(pce->is_cart_inserted()) {
		return pceevent->get_frame_rate();
	}
#endif
	return event->get_frame_rate();
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return cpu;
	} else if(index == 1) {
		return cpu_sub;
	} else if(index == 2) {
		return cpu_kbd;
#ifdef _X1TWIN
	} else if(index == 3 && pce->is_cart_inserted()) {
		return pcecpu;
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
	display->draw_screen();
#ifdef _X1TWIN
	if(pce->is_cart_inserted()) {
		pce->draw_screen();
	}
#endif
}

uint32_t VM::get_access_lamp_status()
{
	uint32_t status = fdc->read_signal(0);
	return (status & (1 | 4)) ? 1 : (status & (2 | 8)) ? 2 : 0;
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
#ifdef _X1TWIN
	pceevent->initialize_sound(rate, samples);
#endif
	
	// init sound gen
	if(sound_device_type >= 1) {
		opm1->initialize_sound(rate, 4000000, samples, 0);
	}
	if(sound_device_type == 2) {
		opm2->initialize_sound(rate, 4000000, samples, 0);
	}
	psg->initialize_sound(rate, 2000000, samples, 0, 0);
#ifdef _X1TWIN
	pce->initialize_sound(rate);
#endif
}

uint16_t* VM::create_sound(int* extra_frames)
{
#ifdef _X1TWIN
	if(pce->is_cart_inserted()) {
		uint16_t* buffer = pceevent->create_sound(extra_frames);
		for(int i = 0; i < *extra_frames; i++) {
			event->drive();
		}
		return buffer;
	}
#endif
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
#ifdef _X1TWIN
	if(pce->is_cart_inserted()) {
		return pceevent->get_sound_buffer_ptr();
	}
#endif
	return event->get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		psg->set_volume(1, decibel_l, decibel_r);
	} else if(ch == 1) {
		if(sound_device_type >= 1) {
			opm1->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch == 2) {
		if(sound_device_type >= 2) {
			opm2->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch == 3) {
		drec->set_volume(0, decibel_l, decibel_r);
#if defined(_X1TWIN)
	} else if(ch == 4) {
		pce->set_volume(0, decibel_l, decibel_r);
#endif
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
#ifdef _X1TWIN
	if(!repeat && !pce->is_cart_inserted()) {
#else
	if(!repeat) {
#endif
		if(pseudo_sub_cpu) {
			psub->key_down(code, false);
		} else {
			kbd->key_down(code, false);
		}
	}
}

void VM::key_up(int code)
{
#ifdef _X1TWIN
	if(!pce->is_cart_inserted()) {
#endif
		if(pseudo_sub_cpu) {
			psub->key_up(code);
		} else {
			//kbd->key_up(code);
		}
#ifdef _X1TWIN
	}
#endif
}

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

void VM::play_tape(const _TCHAR* file_path)
{
	bool value = drec->play_tape(file_path);
	if(pseudo_sub_cpu) {
		psub->close_tape();
		psub->play_tape(value);
	} else {
		sub->close_tape();
		sub->play_tape(value);
	}
}

void VM::rec_tape(const _TCHAR* file_path)
{
	bool value = drec->rec_tape(file_path);
	if(pseudo_sub_cpu) {
		psub->close_tape();
		psub->rec_tape(value);
	} else {
		sub->close_tape();
		sub->rec_tape(value);
	}
}

void VM::close_tape()
{
	drec->close_tape();
	if(pseudo_sub_cpu) {
		psub->close_tape();
	} else {
		sub->close_tape();
	}
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

void VM::push_apss_forward()
{
	drec->do_apss(1);
}

void VM::push_apss_rewind()
{
	drec->do_apss(-1);
}

bool VM::is_frame_skippable()
{
#ifdef _X1TWIN
	if(pce->is_cart_inserted()) {
		return pceevent->is_frame_skippable();
	}
#endif
	return event->is_frame_skippable();
}

#ifdef _X1TWIN
void VM::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		pce->open_cart(file_path);
		pce->reset();
		pcecpu->reset();
	}
}

void VM::close_cart(int drv)
{
	if(drv == 0) {
		pce->close_cart();
		pce->reset();
		pcecpu->reset();
	}
}

bool VM::is_cart_inserted(int drv)
{
	if(drv == 0) {
		return pce->is_cart_inserted();
	} else {
		return false;
	}
}
#endif

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
#ifdef _X1TURBO_FEATURE
	update_dipswitch();
#endif
}

#ifdef _X1TURBO_FEATURE
void VM::update_dipswitch()
{
	// bit0		0=High 1=Standard
	// bit2		0=5"2D 1=5"2HD
	io->set_iovalue_single_r(0x1ff0, (config.monitor_type & 1) | ((config.drive_type & 1) << 2));
}
#endif

#define STATE_VERSION	6

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->FputBool(pseudo_sub_cpu);
	state_fio->FputInt32(sound_device_type);
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
	pseudo_sub_cpu = state_fio->FgetBool();
	sound_device_type = state_fio->FgetInt32();
	
#ifdef _X1TURBO_FEATURE
	// post process
	update_dipswitch();
#endif
	return true;
}
