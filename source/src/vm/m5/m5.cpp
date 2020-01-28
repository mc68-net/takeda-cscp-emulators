/*
	SORD m5 Emulator 'Emu5'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ virtual machine ]
*/

#include "m5.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../io.h"
#include "../memory.h"
#include "../sn76489an.h"
#include "../tms9918a.h"
#include "../z80.h"
#include "../z80ctc.h"

#include "cmt.h"
#include "keyboard.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	event->initialize();		// must be initialized first
	
	drec = new DATAREC(this, emu);
	io = new IO(this, emu);
	memory = new MEMORY(this, emu);
	psg = new SN76489AN(this, emu);
	vdp = new TMS9918A(this, emu);
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);
	
	cmt = new CMT(this, emu);
	key = new KEYBOARD(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	
	drec->set_context_out(cmt, SIG_CMT_IN, 1);
	vdp->set_context_irq(ctc, SIG_Z80CTC_TRIG_3, 1);
	cmt->set_context_drec(drec);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(ctc);
	
	// z80 family daisy chain
	ctc->set_context_intr(cpu, 0);
	
	// memory bus
	memset(ram, 0, sizeof(ram));
	memset(ext, 0, sizeof(ext));
	memset(ipl, 0xff, sizeof(ipl));
	memset(cart, 0xff, sizeof(cart));
	
	memory->read_bios(_T("IPL.ROM"), ipl, sizeof(ipl));
	
	memory->set_memory_r(0x0000, 0x1fff, ipl);
	memory->set_memory_r(0x2000, 0x6fff, cart);
	memory->set_memory_rw(0x7000, 0x7fff, ram);
	memory->set_memory_rw(0x8000, 0xffff, ext);
	
	// i/o bus
	io->set_iomap_range_rw(0x00, 0x03, ctc);
	io->set_iomap_range_rw(0x10, 0x11, vdp);
	io->set_iomap_single_w(0x20, psg);
	io->set_iomap_range_r(0x30, 0x37, key);
	io->set_iomap_single_w(0x40, cmt);
	io->set_iomap_single_rw(0x50, cmt);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id != event->this_device_id) {
			device->initialize();
		}
	}
}

VM::~VM()
{
	// delete all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->release();
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

void VM::run()
{
	event->drive();
}

// ----------------------------------------------------------------------------
// event manager
// ----------------------------------------------------------------------------

void VM::register_event(DEVICE* dev, int event_id, int usec, bool loop, int* register_id)
{
	event->register_event(dev, event_id, usec, loop, register_id);
}

void VM::register_event_by_clock(DEVICE* dev, int event_id, int clock, bool loop, int* register_id)
{
	event->register_event_by_clock(dev, event_id, clock, loop, register_id);
}

void VM::cancel_event(int register_id)
{
	event->cancel_event(register_id);
}

void VM::register_frame_event(DEVICE* dev)
{
	event->register_frame_event(dev);
}

void VM::register_vline_event(DEVICE* dev)
{
	event->register_vline_event(dev);
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

uint32 VM::get_prv_pc()
{
	return cpu->get_prv_pc();
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	vdp->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	psg->init(rate, 3579545, 8000);
}

uint16* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(_TCHAR* file_path)
{
	memset(cart, 0xff, sizeof(cart));
	memory->read_image(file_path, cart, sizeof(cart));
	reset();
}

void VM::close_cart()
{
	memset(cart, 0xff, sizeof(cart));
	reset();
}

void VM::play_datarec(_TCHAR* file_path)
{
	drec->play_datarec(file_path);
}

void VM::rec_datarec(_TCHAR* file_path)
{
	drec->rec_datarec(file_path);
}

void VM::close_datarec()
{
	drec->close_datarec();
}

bool VM::now_skip()
{
	return drec->skip();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

