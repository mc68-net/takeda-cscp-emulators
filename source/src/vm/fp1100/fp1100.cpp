/*
	CASIO FP-1100 Emulator 'eFP-1100'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ virtual machine ]
*/

#include "fp1100.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../beep.h"
#include "../hd46505.h"
#include "../upd765a.h"
#include "../upd7801.h"
#include "../z80.h"

#include "main.h"
#include "sub.h"
#include "rampack.h"
#include "rompack.h"
#include "fdcpack.h"

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
	
	beep = new BEEP(this, emu);
	crtc = new HD46505(this, emu);
	fdc = new UPD765A(this, emu);
	subcpu = new UPD7801(this, emu);
	cpu = new Z80(this, emu);
	
	main = new MAIN(this, emu);
	sub = new SUB(this, emu);
	rampack1 = new RAMPACK(this, emu);
	rampack1->index = 1;
	rampack2 = new RAMPACK(this, emu);
	rampack2->index = 2;
	rampack3 = new RAMPACK(this, emu);
	rampack3->index = 3;
	rampack4 = new RAMPACK(this, emu);
	rampack4->index = 4;
	rampack5 = new RAMPACK(this, emu);
	rampack5->index = 5;
	rampack6 = new RAMPACK(this, emu);
	rampack6->index = 6;
	rompack = new ROMPACK(this, emu);
	fdcpack = new FDCPACK(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_cpu(subcpu);
	event->set_context_sound(beep);
	
	crtc->set_context_hsync(sub, SIG_SUB_HSYNC, 1);
	fdc->set_context_irq(fdcpack, SIG_FDCPACK_IRQ, 1);
	fdc->set_context_drq(fdcpack, SIG_FDCPACK_DRQ, 1);
	
	main->set_context_cpu(cpu);
	main->set_context_sub(sub);
	main->set_context_slot(0, rampack1);
	main->set_context_slot(1, rampack2);
	main->set_context_slot(2, rampack3);
	main->set_context_slot(3, rampack4);
	main->set_context_slot(4, rampack5);
	main->set_context_slot(5, rampack6);
	main->set_context_slot(6, rompack);
	main->set_context_slot(7, fdcpack);
	
	sub->set_context_cpu(subcpu);
	sub->set_context_main(main);
	sub->set_context_beep(beep);
	sub->set_context_crtc(crtc, crtc->get_regs());
	
	fdcpack->set_context_fdc(fdc);
	fdcpack->set_context_main(main);
	
	// cpu bus
	cpu->set_context_mem(main);
	cpu->set_context_io(main);
	cpu->set_context_intr(main);
	subcpu->set_context_mem(sub);
	subcpu->set_context_io(sub);
	
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

uint32 VM::get_sub_prv_pc()
{
	return subcpu->get_prv_pc();
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	sub->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	beep->init(rate, 2400, 8000);
}

uint16* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	sub->key_down(code);
}

void VM::key_up(int code)
{
	sub->key_up(code);
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

void VM::play_datarec(_TCHAR* filename)
{
	sub->play_datarec(filename);
}

void VM::rec_datarec(_TCHAR* filename)
{
	sub->rec_datarec(filename);
}

void VM::close_datarec()
{
	sub->close_datarec();
}

bool VM::now_skip()
{
	return false;
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

