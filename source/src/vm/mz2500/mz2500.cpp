/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ virtual machine ]
*/

#include "mz2500.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8253.h"
#include "../i8255.h"
#include "../io8.h"
#include "../mb8877.h"
#include "../pcm1bit.h"
#include "../rp5c15.h"
#include "../w3100a.h"
#include "../ym2203.h"
#include "../z80.h"
#include "../z80pio.h"
#include "../z80sio.h"

#include "calendar.h"
#include "cassette.h"
#include "crtc.h"
#include "emm.h"
#include "extrom.h"
#include "floppy.h"
#include "interrupt.h"
#include "joystick.h"
#include "kanji.h"
#include "keyboard.h"
#include "memory.h"
#include "mouse.h"
#include "reset.h"
#include "romfile.h"
#include "sasi.h"
#include "timer.h"
#include "voice.h"

#include "../../config.h"

extern config_t config;

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	uint8 opn_b = (config.monitor_type & 2) ? 0x77 : 0x37;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	event->initialize();		// must be initialized first
	
	pit = new I8253(this, emu);
	pio0 = new I8255(this, emu);
	io = new IO8(this, emu);
	fdc = new MB8877(this, emu);
	pcm = new PCM1BIT(this, emu);
	rtc = new RP5C15(this, emu);
	w3100a = new W3100A(this, emu);
	opn = new YM2203(this, emu);
	cpu = new Z80(this, emu);
	pio1 = new Z80PIO(this, emu);
	sio = new Z80SIO(this, emu);
	
	calendar = new CALENDAR(this, emu);
	cassette = new CASSETTE(this, emu);
	crtc = new CRTC(this, emu);
	emm = new EMM(this, emu);
	extrom = new EXTROM(this, emu);
	floppy = new FLOPPY(this, emu);
	interrupt = new INTERRUPT(this, emu);
	joystick = new JOYSTICK(this, emu);
	kanji = new KANJI(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	mouse = new MOUSE(this, emu);
	rst = new RESET(this, emu);
	romfile = new ROMFILE(this, emu);
	sasi = new SASI(this, emu);
	timer = new TIMER(this, emu);
	voice = new VOICE(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(opn);
	event->set_context_sound(pcm);
	
	pit->set_context_ch0(interrupt, SIG_INTERRUPT_I8253);
	pit->set_context_ch0(pit, SIG_I8253_CLOCK_1);
	pit->set_context_ch1(pit, SIG_I8253_CLOCK_2);
#ifdef TIMER_FREQ
	pit->set_constant_clock(0, 31250);
#endif
	pio0->set_context_port_a(cassette, SIG_CASSETTE_CONTROL, 0xff, 0);
	pio0->set_context_port_c(rst, SIG_RESET_CONTROL, 0xff, 0);
	pio0->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x04, 0);
	rtc->set_context_alarm(interrupt, SIG_INTERRUPT_RP5C15, 1);
	rtc->set_context_pulse(opn, SIG_YM2203_PORT_B, 8);
	opn->set_context_port_a(floppy, SIG_FLOPPY_REVERSE, 0x02, 0);
	opn->set_context_port_a(crtc, SIG_CRTC_PALLETE, 0x04, 0);
	opn->set_context_port_a(mouse, SIG_MOUSE_SEL, 0x08, 0);
	pio1->set_context_port_a(crtc, SIG_CRTC_COLUMN_SIZE, 0x20, 0);
	pio1->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0xff, 0);
	sio->set_context_dtr1(mouse, SIG_MOUSE_DTR, 1);
	
	calendar->set_context(rtc);
	cassette->set_context(pio0, SIG_I8255_PORT_B);
	crtc->set_context_cpu(cpu);
	crtc->set_context_mem(memory, SIG_MEMORY_HBLANK, SIG_MEMORY_VBLANK);
	crtc->set_context_vblank(interrupt, SIG_INTERRUPT_CRTC);
	crtc->set_context_pio(pio0, SIG_I8255_PORT_B);
	crtc->set_context_fdc(fdc);
	crtc->set_vram_ptr(memory->get_vram());
	crtc->set_tvram_ptr(memory->get_tvram());
	crtc->set_kanji_ptr(memory->get_kanji());
	crtc->set_pcg_ptr(memory->get_pcg());
	floppy->set_context_cpu(cpu);
	floppy->set_context_fdc(fdc, SIG_MB8877_DRIVEREG, SIG_MB8877_SIDEREG);
	keyboard->set_context_pio0(pio0, SIG_I8255_PORT_B);
	keyboard->set_context_pio1(pio1, SIG_Z80PIO_PORT_B);
	memory->set_context_cpu(cpu);
	memory->set_context_crtc(crtc);
	mouse->set_context(sio, SIG_Z80SIO_RECV_CH1, SIG_Z80SIO_CLEAR_CH1);
	timer->set_context(pit, SIG_I8253_GATE_0, SIG_I8253_GATE_1, SIG_I8253_CLOCK_0);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pio1);
	
	// z80 family daisy chain
	pio1->set_context_intr(cpu, 0);
	pio1->set_context_child(sio);
	sio->set_context_intr(cpu, 1);
	sio->set_context_child(interrupt);
	interrupt->set_context_intr(cpu, 2);
	
	// i/o bus
	io->set_iomap_range_w(0x60, 0x63, w3100a);
	io->set_iomap_range_w(0xa0, 0xa3, sio);
	io->set_iomap_range_w(0xa4, 0xa5, sasi);
	io->set_iomap_single_w(0xa8, romfile);
	io->set_iomap_range_w(0xac, 0xad, emm);
	io->set_iomap_single_w(0xae, crtc);
	io->set_iomap_range_w(0xb0, 0xb3, sio);
	io->set_iomap_range_w(0xb4, 0xb5, memory);
	io->set_iomap_range_w(0xb8, 0xb9, kanji);
	io->set_iomap_range_w(0xbc, 0xbd, crtc);
	io->set_iomap_range_w(0xc6, 0xc7, interrupt);
	io->set_iomap_range_w(0xc8, 0xc9, opn);
	io->set_iomap_single_w(0xca, voice);
	io->set_iomap_single_w(0xcc, calendar);
	io->set_iomap_range_w(0xce, 0xcf, memory);
	io->set_iomap_range_w(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdd, floppy);
	io->set_iomap_range_w(0xe0, 0xe3, pio0);
	io->set_iomap_range_w(0xe4, 0xe7, pit);
	io->set_iomap_range_w(0xe8, 0xeb, pio1);
	io->set_iomap_single_w(0xef, joystick);
	io->set_iomap_range_w(0xf0, 0xf3, timer);
	io->set_iomap_range_w(0xf4, 0xf7, crtc);
	io->set_iomap_range_w(0xf8, 0xf9, extrom);
	
	io->set_iomap_single_r(0x63, w3100a);
	io->set_iomap_range_r(0xa0, 0xa3, sio);
	io->set_iomap_range_r(0xa4, 0xa5, sasi);
	io->set_iomap_single_r(0xa9, romfile);
	io->set_iomap_single_r(0xad, emm);
	io->set_iomap_range_r(0xb0, 0xb3, sio);
	io->set_iomap_range_r(0xb4, 0xb5, memory);
	io->set_iomap_range_r(0xb8, 0xb9, kanji);
	io->set_iomap_range_r(0xbc, 0xbf, crtc);
	io->set_iomap_range_r(0xc8, 0xc9, opn);
	io->set_iomap_single_r(0xca, voice);
	io->set_iomap_single_r(0xcc, calendar);
	io->set_iomap_range_r(0xd8, 0xdb, fdc);
	io->set_iomap_range_r(0xe0, 0xe2, pio0);
	io->set_iomap_range_r(0xe4, 0xe6, pit);
	io->set_iomap_single_r(0xe8, pio1);
	io->set_iomap_single_r(0xea, pio1);
	io->set_iomap_single_r(0xef, joystick);
	io->set_iomap_range_r(0xf4, 0xf7, crtc);
	io->set_iomap_single_r(0xf8, extrom);
	
	io->set_iowait_range_r(0xc8, 0xc9, 1);
	io->set_iowait_single_r(0xcc, 3);
	io->set_iowait_range_r(0xd8, 0xdf, 1);
	io->set_iowait_range_r(0xe8, 0xeb, 1);
	
	io->set_iowait_range_w(0xc8, 0xc9, 1);
	io->set_iowait_single_w(0xcc, 3);
	io->set_iowait_range_w(0xd8, 0xdf, 1);
	io->set_iowait_range_w(0xe8, 0xeb, 1);
	
	// initialize and ipl reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id != event->this_device_id)
			device->initialize();
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id != event->this_device_id)
			device->ipl_reset();
	}
	
	// set initial port status
	opn->write_signal(SIG_YM2203_PORT_B, opn_b, 0xff);
}

VM::~VM()
{
	// delete all devices
	for(DEVICE* device = first_device; device; device = device->next_device)
		device->release();
}

DEVICE* VM::get_device(int id)
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id == id)
			return device;
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device)
		device->reset();
}

void VM::ipl_reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device)
		device->ipl_reset();
}

void VM::run()
{
	event->drive();
}

// ----------------------------------------------------------------------------
// event manager
// ----------------------------------------------------------------------------

void VM::regist_event(DEVICE* dev, int event_id, int usec, bool loop, int* regist_id)
{
	event->regist_event(dev, event_id, usec, loop, regist_id);
}

void VM::regist_event_by_clock(DEVICE* dev, int event_id, int clock, bool loop, int* regist_id)
{
	event->regist_event_by_clock(dev, event_id, clock, loop, regist_id);
}

void VM::cancel_event(int regist_id)
{
	event->cancel_event(regist_id);
}

void VM::regist_frame_event(DEVICE* dev)
{
	event->regist_frame_event(dev);
}

void VM::regist_vsync_event(DEVICE* dev)
{
	event->regist_vsync_event(dev);
}

void VM::regist_hsync_event(DEVICE* dev)
{
	event->regist_hsync_event(dev);
}

uint32 VM::current_clock()
{
	return event->current_clock();
}

uint32 VM::passed_clock(uint32 prev)
{
	uint32 current = event->current_clock();
	return (current > prev) ? current - prev : current + (0xffffffff - prev) + 1;
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	crtc->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	opn->init(rate, 2000000, samples, 0, -8);
	pcm->init(4096);
}

uint16* VM::create_sound(int samples, bool fill)
{
	return event->create_sound(samples, fill);
}

// ----------------------------------------------------------------------------
// socket
// ----------------------------------------------------------------------------

void VM::network_connected(int ch)
{
	w3100a->connected(ch);
}

void VM::network_disconnected(int ch)
{
	w3100a->disconnected(ch);
}

uint8* VM::get_sendbuffer(int ch, int* size)
{
	return w3100a->get_sendbuffer(ch, size);
}

void VM::inc_sendbuffer_ptr(int ch, int size)
{
	w3100a->inc_sendbuffer_ptr(ch, size);
}

uint8* VM::get_recvbuffer0(int ch, int* size0, int* size1)
{
	return w3100a->get_recvbuffer0(ch, size0, size1);
}

uint8* VM::get_recvbuffer1(int ch)
{
	return w3100a->get_recvbuffer1(ch);
}

void VM::inc_recvbuffer_ptr(int ch, int size)
{
	w3100a->inc_recvbuffer_ptr(ch, size);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_disk(_TCHAR* filename, int drv)
{
	fdc->open_disk(filename, drv);
}

void VM::close_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::now_skip()
{
	return false;
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device)
		device->update_config();
}
