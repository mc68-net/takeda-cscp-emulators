/*
	TOMY PyuTa Emulator 'ePyuTa'

	Author : Takeda.Toshiya
	Date   : 2007.07.15 -

	[ virtual machine ]
*/

#include "pyuta.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../sn76489an.h"
#include "../tms9918a.h"
#include "../tms9995.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "memory.h"

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
	psg = new SN76489AN(this, emu);
	vdp = new TMS9918A(this, emu);
	cpu = new TMS9995(this, emu);
	
	memory = new MEMORY(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	event->set_context_sound(drec);
	
	drec->set_context_ear(memory, 0, 1);
	memory->set_context_cmt(drec);
	memory->set_context_cpu(cpu);
	memory->set_context_psg(psg);
	memory->set_context_vdp(vdp);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(memory);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
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
	}
	return NULL;
}
#endif

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

int VM::sound_buffer_ptr()
{
	return event->sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::get_sound_device_info(int ch, _TCHAR *buffer, size_t buffer_len, bool *mono)
{
	if(ch == 0) {
		my_tcscpy_s(buffer, buffer_len, _T("PSG"));
	} else if(ch == 1) {
		my_tcscpy_s(buffer, buffer_len, _T("CMT"));
	} else {
		buffer[0] = _T('\0');
	}
}

void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		psg->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		drec->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->open_cart(file_path);
		reset();
	}
}

void VM::close_cart(int drv)
{
	if(drv == 0) {
		memory->close_cart();
		reset();
	}
}

bool VM::cart_inserted(int drv)
{
	if(drv == 0) {
		return memory->cart_inserted();
	} else {
		return false;
	}
}

void VM::play_tape(const _TCHAR* file_path)
{
	drec->play_tape(file_path);
}

void VM::rec_tape(const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
}

void VM::close_tape()
{
	drec->close_tape();
}

bool VM::tape_inserted()
{
	return drec->tape_inserted();
}

bool VM::tape_playing()
{
	return drec->tape_playing();
}

bool VM::tape_recording()
{
	return drec->tape_recording();
}

int VM::tape_position()
{
	return drec->tape_position();
}

bool VM::now_skip()
{
	return event->now_skip();
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

