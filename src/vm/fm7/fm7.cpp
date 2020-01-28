/*
 * FM7 -> VM
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *   Feb 27, 2015 : Initial
 */

#include "fm7.h"
#include "../../emu.h"
#include "../../config.h"
#include "../device.h"
#include "../event.h"
#include "../memory.h"
#include "../prnfile.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "../datarec.h"
#include "../disk.h"

#include "../mc6809.h"
#include "../z80.h"
#include "../mb8877.h"

#include "../pcm1bit.h"
#include "../ym2203.h"
#if defined(_FM77AV_VARIANTS)
#include "mb61vh010.h"
#include "../beep.h"
#endif
#if defined(HAS_DMA)
#include "hd6844.h"
#endif

#if defined(USE_LED_DEVICE)
#include "./dummydevice.h"
#else
#define SIG_DUMMYDEVICE_BIT0 0
#define SIG_DUMMYDEVICE_BIT1 1
#define SIG_DUMMYDEVICE_BIT2 2
#endif

#include "./fm7_mainio.h"
#include "./fm7_mainmem.h"
#include "./fm7_display.h"
#include "./fm7_keyboard.h"
#include "./joystick.h"

#include "./kanjirom.h"

VM::VM(EMU* parent_emu): emu(parent_emu)
{
	
	first_device = last_device = NULL;
#if defined(_FM8)
	psg = NULL;
#else	
# if defined(_FM77AV_VARIANTS)
	opn[0] = opn[1] = opn[2] = NULL;
# else   
	opn[0] = opn[1] = opn[2] = psg = NULL; 
# endif
#endif
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	dummycpu = new DEVICE(this, emu);
	maincpu = new MC6809(this, emu);
	subcpu = new MC6809(this, emu);
#ifdef WITH_Z80
	z80cpu = new Z80(this, emu);
#endif
	// basic devices
	// I/Os
#if defined(HAS_DMA)
	dmac = new HD6844(this, emu);
#endif   
#if defined(_FM8)
	psg = new YM2203(this, emu);
#else	
	opn[0] = new YM2203(this, emu); // OPN
	opn[1] = new YM2203(this, emu); // WHG
	opn[2] = new YM2203(this, emu); // THG
# if !defined(_FM77AV_VARIANTS)
	psg = new YM2203(this, emu);
# endif
#endif
	drec = new DATAREC(this, emu);
	pcm1bit = new PCM1BIT(this, emu);
	fdc = new MB8877(this, emu);
	joystick  = new JOYSTICK(this, emu);
	printer = new PRNFILE(this, emu);
#if defined(_FM77AV_VARIANTS)
	alu = new MB61VH010(this, emu);
	keyboard_beep = new BEEP(this, emu);
#endif	
	keyboard = new KEYBOARD(this, emu);
	display = new DISPLAY(this, emu);	

	mainio  = new FM7_MAINIO(this, emu);
	mainmem = new FM7_MAINMEM(this, emu);
	
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	if((config.dipswitch & FM7_DIPSW_CONNECT_KANJIROM) != 0) {
		kanjiclass1 = new KANJIROM(this, emu, false);
	}
#else
	kanjiclass1 = new KANJIROM(this, emu, false);
#endif	
#ifdef CAPABLE_KANJI_CLASS2
	kanjiclass2 = new KANJIROM(this, emu, true);
#endif
	
	//mainmem = new FM7_MAINMEM(this, emu);


#if defined(USE_LED_DEVICE)
	led_terminate = new DUMMYDEVICE(this, emu);
#else
	led_terminate = new DEVICE(this, emu);
#endif
	this->connect_bus();
	
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

void VM::connect_bus(void)
{
	uint32_t mainclock;
	uint32_t subclock;

	/*
	 * CLASS CONSTRUCTION
	 *
	 * VM 
	 *  |-> MAINCPU -> MAINMEM -> MAINIO -> MAIN DEVICES
	 *  |             |        |      
	 *  | -> SUBCPU  -> SUBMEM  -> SUBIO -> SUB DEVICES
	 *  | -> DISPLAY
	 *  | -> KEYBOARD
	 *
	 *  MAINMEM can access SUBMEM/IO, when SUBCPU is halted.
	 *  MAINMEM and SUBMEM can access DISPLAY and KEYBOARD with exclusive.
	 *  MAINCPU can access MAINMEM.
	 *  SUBCPU  can access SUBMEM.
	 *  DISPLAY : R/W from MAINCPU and SUBCPU.
	 *  KEYBOARD : R/W
	 *
	 */
	event->set_frames_per_sec(FRAMES_PER_SEC);
	event->set_lines_per_frame(LINES_PER_FRAME);
	//event->set_context_cpu(dummycpu, (CPU_CLOCKS * 3) / 8); // MAYBE FIX With eFM77AV40/20.
	// With slow clock (for dummycpu), some softwares happen troubles,
	// Use faster clock for dummycpu. 20160319 K.Ohta
	event->set_context_cpu(dummycpu, SUBCLOCK_NORMAL);

#if defined(_FM8)
	mainclock = MAINCLOCK_SLOW;
	subclock = SUBCLOCK_SLOW;
#else
	if(config.cpu_type == 0) {
		// 2MHz
		subclock = SUBCLOCK_NORMAL;
		mainclock = MAINCLOCK_NORMAL;
	} else {
		// 1.2MHz
		mainclock = MAINCLOCK_SLOW;
		subclock = SUBCLOCK_SLOW;
	}
	//if((config.dipswitch & FM7_DIPSW_CYCLESTEAL) != 0) subclock = subclock / 3;
#endif
	event->set_context_cpu(maincpu, mainclock);
	event->set_context_cpu(subcpu,  subclock);
   
#ifdef WITH_Z80
	event->set_context_cpu(z80cpu,  4000000);
	z80cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
#endif

	event->set_context_sound(pcm1bit);
#if defined(_FM8)
	event->set_context_sound(psg);
	event->set_context_sound(drec);
#else
	event->set_context_sound(opn[0]);
	event->set_context_sound(opn[1]);
	event->set_context_sound(opn[2]);
# if !defined(_FM77AV_VARIANTS)
	event->set_context_sound(psg);
# endif
	event->set_context_sound(drec);
# if defined(_FM77AV_VARIANTS)
	event->set_context_sound(keyboard_beep);
# endif
#endif   
#if !defined(_FM77AV_VARIANTS) && !defined(_FM77L4)
	event->register_frame_event(display);
#endif	
	mainio->set_context_maincpu(maincpu);
	mainio->set_context_subcpu(subcpu);
	
	mainio->set_context_display(display);
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	if((config.dipswitch & FM7_DIPSW_CONNECT_KANJIROM) != 0) {
		mainio->set_context_kanjirom_class1(kanjiclass1);
	}
#else
	mainio->set_context_kanjirom_class1(kanjiclass1);
#endif	
	mainio->set_context_mainmem(mainmem);
	mainio->set_context_keyboard(keyboard);
	mainio->set_context_printer(printer);
	mainio->set_context_printer_reset(printer, SIG_PRINTER_RESET, 0xffffffff);
	mainio->set_context_printer_strobe(printer, SIG_PRINTER_STROBE, 0xffffffff);
	mainio->set_context_printer_select(printer, SIG_PRINTER_SELECT, 0xffffffff);
#if defined(CAPABLE_KANJI_CLASS2)
	mainio->set_context_kanjirom_class2(kanjiclass2);
#endif
	keyboard->set_context_break_line(mainio, FM7_MAINIO_PUSH_BREAK, 0xffffffff);
	keyboard->set_context_int_line(mainio, FM7_MAINIO_KEYBOARDIRQ, 0xffffffff);
	keyboard->set_context_int_line(display, SIG_FM7_SUB_KEY_FIRQ, 0xffffffff);
#if defined(_FM77AV_VARIANTS)
	keyboard->set_context_beep(keyboard_beep);
#endif	
	keyboard->set_context_rxrdy(display, SIG_FM7KEY_RXRDY, 0x01);
	keyboard->set_context_key_ack(display, SIG_FM7KEY_ACK, 0x01);
	keyboard->set_context_ins_led( led_terminate, SIG_DUMMYDEVICE_BIT0, 0xffffffff);
	keyboard->set_context_caps_led(led_terminate, SIG_DUMMYDEVICE_BIT1, 0xffffffff);
	keyboard->set_context_kana_led(led_terminate, SIG_DUMMYDEVICE_BIT2, 0xffffffff);
   
	drec->set_context_ear(mainio, FM7_MAINIO_CMT_RECV, 0xffffffff);
	//drec->set_context_remote(mainio, FM7_MAINIO_CMT_REMOTE, 0xffffffff);
	mainio->set_context_datarec(drec);
	
	mainmem->set_context_mainio(mainio);
	mainmem->set_context_display(display);
	mainmem->set_context_maincpu(maincpu);
#if defined(CAPABLE_DICTROM)
	mainmem->set_context_kanjirom_class1(kanjiclass1);
#endif  
	display->set_context_mainio(mainio);
	display->set_context_subcpu(subcpu);
	display->set_context_keyboard(keyboard);

	mainio->set_context_clock_status(mainmem, FM7_MAINIO_CLOCKMODE, 0xffffffff);
	mainio->set_context_clock_status(display, SIG_DISPLAY_CLOCK, 0xffffffff);
	
	subcpu->set_context_bus_halt(display, SIG_FM7_SUB_HALT, 0xffffffff);
	subcpu->set_context_bus_halt(mainmem, SIG_FM7_SUB_HALT, 0xffffffff);

#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	display->set_context_kanjiclass1(kanjiclass1);
#endif	
#if defined(CAPABLE_KANJI_CLASS2)
	display->set_context_kanjiclass2(kanjiclass2);
#endif   
#if defined(_FM77AV_VARIANTS)
	display->set_context_alu(alu);
	alu->set_context_memory(display);
#endif	
	// Palette, VSYNC, HSYNC, Multi-page, display mode. 
	mainio->set_context_display(display);
	
	//FDC
	fdc->set_context_irq(mainio, FM7_MAINIO_FDC_IRQ, 0x1);
	fdc->set_context_drq(mainio, FM7_MAINIO_FDC_DRQ, 0x1);
	mainio->set_context_fdc(fdc);
	// SOUND
	mainio->set_context_beep(pcm1bit);
#if defined(_FM8)	
	mainio->set_context_psg(psg);
#else
# if !defined(_FM77AV_VARIANTS)
	mainio->set_context_psg(psg);
# endif
	opn[0]->set_context_irq(mainio, FM7_MAINIO_OPN_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[0], 0);
	//joystick->set_context_opn(opn[0]);
	mainio->set_context_joystick(joystick);
	opn[0]->set_context_port_b(joystick, FM7_JOYSTICK_MOUSE_STROBE, 0xff, 0);
	
	opn[1]->set_context_irq(mainio, FM7_MAINIO_WHG_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[1], 1);
	opn[2]->set_context_irq(mainio, FM7_MAINIO_THG_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[2], 2);
#endif   
	subcpu->set_context_bus_halt(display, SIG_FM7_SUB_HALT, 0xffffffff);
	subcpu->set_context_bus_clr(display, SIG_FM7_SUB_USE_CLR, 0x0000000f);
   
	event->register_frame_event(joystick);
#if defined(HAS_DMA)
	dmac->set_context_src(fdc, 0);
	dmac->set_context_dst(mainmem, 0);
	dmac->set_context_int_line(mainio, 0, FM7_MAINIO_DMA_INT, 0xffffffff);
	dmac->set_context_drq_line(maincpu, 1, SIG_CPU_BUSREQ, 0xffffffff);
	mainio->set_context_dmac(dmac);
#endif
	
	// MEMORIES must set before initialize().
	maincpu->set_context_mem(mainmem);
	subcpu->set_context_mem(display);
#ifdef WITH_Z80
	z80cpu->set_context_mem(mainmem);
#endif
#ifdef USE_DEBUGGER
	maincpu->set_context_debugger(new DEBUGGER(this, emu));
	subcpu->set_context_debugger(new DEBUGGER(this, emu));
# ifdef WITH_Z80
	z80cpu->set_context_debugger(new DEBUGGER(this, emu));
# endif
#endif

	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}

	// Disks
	for(int i = 0; i < 2; i++) {
#if defined(_FM77AV20) || defined(_FM77AV20EX) || \
	defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
#else
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
#endif
#if defined(_FM77AV_VARIANTS)
		fdc->set_drive_rpm(i, 360);
#else		
		fdc->set_drive_rpm(i, 360);
#endif		
		fdc->set_drive_mfm(i, true);
	}
#if defined(_FM77) || defined(_FM77L4)
	for(int i = 2; i < 4; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2HD);
		fdc->set_drive_rpm(i, 360);
		fdc->set_drive_mfm(i, true);
	}
#endif
	
}  

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
	//update_dipswitch();
}

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
#if !defined(_FM77AV_VARIANTS) || defined(_FM8)
	psg->set_reg(0x27, 0); // stop timer
	psg->set_reg(0x2e, 0);	// set prescaler
	psg->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
#endif
#if !defined(_FM8)
	for(int i = 0; i < 3; i++) {
		opn[i]->set_reg(0x27, 0); // stop timer
		opn[i]->set_reg(0x2e, 0);	// set prescaler
		opn[i]->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
	}
#endif
}

void VM::special_reset()
{
	// BREAK + RESET
	mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 1, 1);
	mainio->reset();
	mainmem->reset();
	
#if defined(FM77AV_VARIANTS)	
	mainio->write_signal(FM7_MAINIO_HOT_RESET, 1, 1);
#endif	
	display->reset();
	subcpu->reset();
	mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 1, 1);
	maincpu->reset();
	event->register_event(mainio, EVENT_UP_BREAK, 10000.0 * 1000.0, false, NULL);
}

void VM::run()
{
	event->drive();
}

double VM::get_frame_rate()
{
	return event->get_frame_rate();
}

#if defined(USE_LED_DEVICE)
uint32_t VM::get_led_status()
{
	return led_terminate->read_signal(SIG_DUMMYDEVICE_READWRITE);
}
#endif // USE_LED_DEVICE


// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return maincpu;
	} else if(index == 1) {
		return subcpu;
	}
#if defined(_WITH_Z80)
	else if(index == 2) {
		return z80cpu;
	}
#endif
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

uint32_t VM::get_access_lamp_status()
{
	uint32_t status = fdc->read_signal(0xff);
	return (status & (1 | 4)) ? 1 : (status & (2 | 8)) ? 2 : 0;
}

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	// init sound gen
#if defined(_FM8)
	psg->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
#else	
	opn[0]->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
	opn[1]->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
	opn[2]->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
# if !defined(_FM77AV_VARIANTS)   
	psg->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
# endif
# if defined(_FM77AV_VARIANTS)
	keyboard_beep->initialize_sound(rate, 2400.0, 512);
# endif
#endif	
	pcm1bit->initialize_sound(rate, 2000);
	drec->initialize_sound(rate, 0);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	uint16_t* p = event->create_sound(extra_frames);
	return p;
}

int VM::get_sound_buffer_ptr()
{
	int pos = event->get_sound_buffer_ptr();
	return pos; 
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
#if !defined(_FM77AV_VARIANTS)
	if(ch-- == 0) {
		psg->set_volume(0, decibel_l, decibel_r);
		psg->set_volume(1, decibel_l, decibel_r);
	} else
#endif
#if !defined(_FM8)		
	if(ch-- == 0) {
		opn[0]->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[0]->set_volume(1, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[1]->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[1]->set_volume(1, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[2]->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[2]->set_volume(1, decibel_l, decibel_r);
	} else
#endif	
	if(ch-- == 0) {
		pcm1bit->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		drec->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		keyboard->key_down(code);
	}
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
	return event->is_frame_skippable();
}

void VM::update_dipswitch()
{
	// bit0		0=High 1=Standard
	// bit2		0=5"2D 1=5"2HD
  //	io->set_iovalue_single_r(0x1ff0, (config.monitor_type & 1) | ((config.drive_type & 1) << 2));
}

void VM::set_cpu_clock(DEVICE *cpu, uint32_t clocks) {
	event->set_secondary_cpu_clock(cpu, clocks);
}

#define STATE_VERSION	3
void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
}

bool VM::load_state(FILEIO* state_fio)
{
	uint32_t version = state_fio->FgetUint32_BE();
	int i = 1;
	if(version > STATE_VERSION) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(!device->load_state(state_fio)) {
			printf("Load Error: DEVID=%d\n", device->this_device_id);
			return false;
		}
	}
	if(version >= 1) {// V1 
		if(version == 3) return true;
	}
	return false;
}

#ifdef USE_DIG_RESOLUTION
void VM::get_screen_resolution(int *w, int *h)
{
	switch(display->get_screen_mode()) {
	case DISPLAY_MODE_8_200L:
	case DISPLAY_MODE_8_200L_TEXT:
		*w = 640;
		*h = 200;
		break;
	case DISPLAY_MODE_8_400L:
	case DISPLAY_MODE_8_400L_TEXT:
		*w = 640;
		*h = 400;
		break;
	case DISPLAY_MODE_4096:
	case DISPLAY_MODE_256k:
		*w = 320;
		*h = 200;
		break;
	default:
		*w = 640;
		*h = 200;
		break;
	}
}
#endif

bool VM::is_screen_changed()
{
	bool f = true;
#if defined(USE_MINIMUM_RENDERING)
	f = display->screen_update();
	display->reset_screen_update();
#endif	
	return f;
}