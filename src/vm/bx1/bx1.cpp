/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ virtual machine ]
*/

#include "bx1.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../mc6800.h"
#include "../io.h"
#include "../memory.h"
#include "../beep.h"
#include "../disk.h"
#include "../mc6843.h"
#include "../mc6844.h"
#include "../mc6850.h"
#include "../noise.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "keyboard.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	cpu = new MC6800(this, emu);
	io = new IO(this, emu);
	memory = new MEMORY(this, emu);
	beep = new BEEP(this, emu);
	fdc = new MC6843(this, emu);	// HD46503
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	dma = new MC6844(this, emu);	// HD46504
	acia = new MC6850(this, emu);
	
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
	fdc->set_context_drq(dma, SIG_MC6844_TX_RQ_0, 1);
	dma->set_context_memory(memory);
	dma->set_context_ch0(fdc);


/*
	memory:
*/

	display->set_context_cpu(cpu);
	display->set_context_ram(ram);
	
	// cpu bus
	cpu->set_context_mem(memory);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(ram, 0x00, sizeof(ram));
	memset(basic, 0xff, sizeof(basic));
	memset(boot, 0xff, sizeof(boot));
	
	memory->read_bios(_T("BASIC.ROM"), basic, sizeof(basic));
	memory->read_bios(_T("BOOT.ROM"), boot, sizeof(boot));
	
	memory->set_memory_rw(0x0000, 0x3fff, ram);
	memory->set_memory_r(0x9000, 0xdfff, basic);
	memory->set_memory_mapped_io_rw(0xe000, 0xefff, io);
	memory->set_memory_r(0xf000, 0xffff, boot);
	
	// io bus
	io->set_iomap_range_rw(0xe121, 0xe122, keyboard);
	io->set_iomap_range_rw(0xe140, 0xe156, dma);
	io->set_iomap_range_rw(0xe180, 0xe187, fdc);
	
/*
	ram
	0062		��ʂ̌����H
	0063-0072	��ʁH

dma[3]	15ch����10h�o�C�g

	chain reg��dma[3]��dma[1]�ɐݒ���R�s�[

dma[2]	bfd5h����09h�o�C�g	��������

	c7h c7h c7h c7h e2h d9h d9h d9h d9h

dma[1]	15ch����10h�o�C�g	��������DISPLAY�H

*/


	
	
/*
outp(0xe210, 0x03);
a  = inp(0xe211);
a &= 0x1c;
a |= 0x81 or 0xa1;
outp(0xe210, a);
*/

//	io->set_iovalue_single_r(0xe189, 0);
//	io->set_iovalue_single_r(0xe212, 0);

/*
	e121	�X�C�b�`�H


	e122	�X�C�b�`�H
		bit5 �t���b�s�[�ǂݍ��݁H



UNKNOWN:	00c0d0	IN8	e182 = ff
UNKNOWN:	00b618	IN8	e184 = ff
UNKNOWN:	00b61b	IN8	e180 = ff
UNKNOWN:	00b61e	OUT8	e180,ff
UNKNOWN:	00b667	OUT8	e18c,00

*/

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
	display->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	beep->initialize_sound(rate, 49152 / 0x80, 8000);
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
		beep->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	keyboard->key_down(code);
//	cpu->write_signal(SIG_CPU_IRQ, 1, 1);
}

void VM::key_up(int code)
{
	keyboard->key_up(code);
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

uint32_t VM::is_floppy_disk_accessed()
{
	return fdc->read_signal(0);
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

#define STATE_VERSION	1

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const char *name = typeid(*device).name() + 6; // skip "class "
		int len = (int)strlen(name);
		
		if(!state_fio->StateCheckInt32(len)) {
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
			return false;
		}
		if(!device->process_state(state_fio, loading)) {
			return false;
		}
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}
