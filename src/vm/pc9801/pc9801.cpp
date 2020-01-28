/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2010.09.15-

	[ virtual machine ]
*/

#include "pc9801.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#if defined(SUPPORT_OLD_BUZZER)
#include "../beep.h"
#endif
#include "../disk.h"
#include "../i8237.h"
#include "../i8251.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../i286.h"
#include "../io.h"
#include "../ls244.h"
#include "../memory.h"
#include "../not.h"
#if !defined(SUPPORT_OLD_BUZZER)
#include "../pcm1bit.h"
#endif
//#include "../pcpr201.h"
#include "../prnfile.h"
#include "../tms3631.h"
#include "../upd1990a.h"
#include "../upd7220.h"
#include "../upd765a.h"
#include "../ym2203.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "floppy.h"
#include "fmsound.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"

#if defined(SUPPORT_320KB_FDD_IF)
#include "../pc80s31k.h"
#include "../z80.h"
#endif
#if defined(SUPPORT_CMT_IF)
#include "cmt.h"
#endif

#if defined(_PC98DO) || defined(_PC98DOPLUS)
#include "../pc80s31k.h"
#include "../z80.h"
#include "../pc8801/pc88.h"
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// check configs
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	boot_mode = config.boot_mode;
#endif
	int cpu_clocks = CPU_CLOCKS;
#if defined(PIT_CLOCK_8MHZ)
	pit_clock_8mhz = true;
#else
	pit_clock_8mhz = false;
#endif
#if defined(_PC9801E)
	if(config.cpu_type != 0) {
		// 8MHz -> 5MHz
		cpu_clocks = 4992030;
		pit_clock_8mhz = false;
	}
#elif defined(_PC9801VM) || defined(_PC98DO)
	if(config.cpu_type != 0) {
		// 10MHz -> 8MHz
		cpu_clocks = 7987248;
		pit_clock_8mhz = true;
	}
#endif
	int pit_clocks = pit_clock_8mhz ? 1996812 : 2457600;
	
	sound_device_type = config.sound_device_type;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
#if defined(SUPPORT_OLD_BUZZER)
	beep = new BEEP(this, emu);
#else
	beep = new PCM1BIT(this, emu);
#endif
	dma = new I8237(this, emu);
#if defined(SUPPORT_CMT_IF)
	sio_cmt = new I8251(this, emu);		// for cmt
#endif
	sio_rs = new I8251(this, emu);		// for rs232c
	sio_kbd = new I8251(this, emu);		// for keyboard
	pit = new I8253(this, emu);
#if defined(SUPPORT_320KB_FDD_IF)
	pio_fdd = new I8255(this, emu);		// for 320kb fdd i/f
#endif
	pio_mouse = new I8255(this, emu);	// for mouse
	pio_sys = new I8255(this, emu);		// for system port
	pio_prn = new I8255(this, emu);		// for printer
	pic = new I8259(this, emu);
	cpu = new I286(this, emu);
	io = new IO(this, emu);
	dmareg1 = new LS244(this, emu);
	dmareg2 = new LS244(this, emu);
	dmareg3 = new LS244(this, emu);
	dmareg0 = new LS244(this, emu);
	rtcreg = new LS244(this, emu);
	memory = new MEMORY(this, emu);
	not_busy = new NOT(this, emu);
#if defined(HAS_I86) || defined(HAS_V30)
	not_prn = new NOT(this, emu);
#endif
	rtc = new UPD1990A(this, emu);
#if defined(SUPPORT_2HD_FDD_IF)
	fdc_2hd = new UPD765A(this, emu);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	fdc_2dd = new UPD765A(this, emu);
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	fdc = new UPD765A(this, emu);
#endif
	gdc_chr = new UPD7220(this, emu);
	gdc_gfx = new UPD7220(this, emu);
	
	if(sound_device_type == 0 || sound_device_type == 1) {
		opn = new YM2203(this, emu);
#ifdef SUPPORT_PC98_OPNA
		opn->is_ym2608 = true;
#endif
		fmsound = new FMSOUND(this, emu);
		joystick = new JOYSTICK(this, emu);
	} else if(sound_device_type == 2 || sound_device_type == 3) {
		tms3631 = new TMS3631(this, emu);
		pit_14 = new I8253(this, emu);
		pio_14 = new I8255(this, emu);
		maskreg_14 = new LS244(this, emu);
	}
	if(config.printer_device_type == 0) {
		printer = new PRNFILE(this, emu);
//	} else if(config.printer_device_type == 1) {
//		printer = new PCPR201(this, emu);
	} else {
		printer = dummy;
	}
	
#if defined(SUPPORT_CMT_IF)
	cmt = new CMT(this, emu);
#endif
	display = new DISPLAY(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	mouse = new MOUSE(this, emu);
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	pio_sub = new I8255(this, emu);
	pc80s31k = new PC80S31K(this, emu);
	fdc_sub = new UPD765A(this, emu);
	cpu_sub = new Z80(this, emu);
#endif
	
	/* IRQ	0  PIT
		1  KEYBOARD
		2  CRTV
		3  (INT0)
		4  RS-232C
		5  (INT1)
		6  (INT2)
		7  SLAVE PIC
		8  PRINTER
		9  (INT3)
		10 (INT41) FDC (640KB I/F)
		11 (INT42) FDC (1MB I/F)
		12 (INT5) PC-9801-26(K) or PC-9801-14
		13 (INT6) MOUSE
		14 
		15 (RESERVED)
	*/
	
	// set contexts
	event->set_context_cpu(cpu, cpu_clocks);
#if defined(SUPPORT_320KB_FDD_IF)
	event->set_context_cpu(cpu_sub, 4000000);
#endif
	event->set_context_sound(beep);
	if(sound_device_type == 0 || sound_device_type == 1) {
		event->set_context_sound(opn);
	} else if(sound_device_type == 2 || sound_device_type == 3) {
		event->set_context_sound(tms3631);
	}
	
	dma->set_context_memory(memory);
	// dma ch.0: sasi
	// dma ch.1: memory refresh
#if defined(SUPPORT_2HD_FDD_IF)
	dma->set_context_ch2(fdc_2hd);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	dma->set_context_ch3(fdc_2dd);
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	dma->set_context_ch2(fdc);
	dma->set_context_ch3(fdc);
#endif
//	sio_rs->set_context_rxrdy(pic, SIG_I8259_CHIP0 | SIG_I8259_IR4, 1);
	sio_kbd->set_context_rxrdy(pic, SIG_I8259_CHIP0 | SIG_I8259_IR1, 1);
	pit->set_context_ch0(pic, SIG_I8259_CHIP0 | SIG_I8259_IR0, 1);
#if defined(SUPPORT_OLD_BUZZER)
	// pit ch.1: memory refresh
#else
	// pit ch.1: buzzer
	pit->set_context_ch1(beep, SIG_PCM1BIT_SIGNAL, 1);
#endif
	// pit ch.2: rs-232c
	pit->set_constant_clock(0, pit_clocks);
	pit->set_constant_clock(1, pit_clocks);
	pit->set_constant_clock(2, pit_clocks);
	pio_mouse->set_context_port_c(mouse, SIG_MOUSE_PORT_C, 0xf0, 0);
	// sysport port.c bit6: printer strobe
#if defined(SUPPORT_OLD_BUZZER)
	pio_sys->set_context_port_c(beep, SIG_BEEP_MUTE, 8, 0);
#else
	pio_sys->set_context_port_c(beep, SIG_PCM1BIT_MUTE, 8, 0);
#endif
	// sysport port.c bit2: enable txrdy interrupt
	// sysport port.c bit1: enable txempty interrupt
	// sysport port.c bit0: enable rxrdy interrupt
	pio_prn->set_context_port_a(printer, SIG_PRINTER_DATA, 0xff, 0);
	pio_prn->set_context_port_c(printer, SIG_PRINTER_STROBE, 0x80, 0);
	if(config.printer_device_type == 0) {
		PRNFILE *prnfile = (PRNFILE *)printer;
		prnfile->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
//	} else if(config.printer_device_type == 1) {
//		PRNFILE *pcpr201 = (PCPR201 *)printer;
//		pcpr201->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
	}
	not_busy->set_context_out(pio_prn, SIG_I8255_PORT_B, 4);
#if defined(HAS_I86) || defined(HAS_V30)
	pio_prn->set_context_port_c(not_prn, SIG_NOT_INPUT, 8, 0);
	not_prn->set_context_out(pic, SIG_I8259_CHIP1 | SIG_I8259_IR0, 1);
#endif
	dmareg1->set_context_output(dma, SIG_I8237_BANK1, 0x0f, 0);
	dmareg2->set_context_output(dma, SIG_I8237_BANK2, 0x0f, 0);
	dmareg3->set_context_output(dma, SIG_I8237_BANK3, 0x0f, 0);
	dmareg0->set_context_output(dma, SIG_I8237_BANK0, 0x0f, 0);
	rtcreg->set_context_output(rtc, SIG_UPD1990A_CMD, 0x07, 0);
	rtcreg->set_context_output(rtc, SIG_UPD1990A_DIN, 0x20, 0);
	rtcreg->set_context_output(rtc, SIG_UPD1990A_STB, 0x08, 0);
	rtcreg->set_context_output(rtc, SIG_UPD1990A_CLK, 0x10, 0);
	pic->set_context_cpu(cpu);
	rtc->set_context_dout(pio_sys, SIG_I8255_PORT_B, 1);
	
	if(sound_device_type == 0 || sound_device_type == 1) {
		opn->set_context_irq(pic, SIG_I8259_CHIP1 | SIG_I8259_IR4, 1);
		opn->set_context_port_b(joystick, SIG_JOYSTICK_SELECT, 0xc0, 0);
		fmsound->set_context_opn(opn);
		joystick->set_context_opn(opn);
	} else if(sound_device_type == 2 || sound_device_type == 3) {
		pio_14->set_context_port_a(tms3631, SIG_TMS3631_ENVELOP1, 0xff, 0);
		pio_14->set_context_port_b(tms3631, SIG_TMS3631_ENVELOP2, 0xff, 0);
		pio_14->set_context_port_c(tms3631, SIG_TMS3631_DATAREG, 0xff, 0);
		maskreg_14->set_context_output(tms3631, SIG_TMS3631_MASKREG, 0xff, 0);
		pit_14->set_constant_clock(2, 1996800 / 8);
		pit_14->set_context_ch2(pic, SIG_I8259_CHIP1 | SIG_I8259_IR4, 1);
	}
	
	display->set_context_pic(pic);
	display->set_context_gdc_chr(gdc_chr, gdc_chr->get_ra());
	display->set_context_gdc_gfx(gdc_gfx, gdc_gfx->get_ra(), gdc_gfx->get_cs());
	keyboard->set_context_sio(sio_kbd);
	mouse->set_context_pic(pic);
	mouse->set_context_pio(pio_mouse);
	
#if defined(SUPPORT_2HD_FDD_IF)
	fdc_2hd->set_context_irq(floppy, SIG_FLOPPY_2HD_IRQ, 1);
	fdc_2hd->set_context_drq(floppy, SIG_FLOPPY_2HD_DRQ, 1);
	fdc_2hd->raise_irq_when_media_changed = true;
	floppy->set_context_fdc_2hd(fdc_2hd);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	fdc_2dd->set_context_irq(floppy, SIG_FLOPPY_2DD_IRQ, 1);
	fdc_2dd->set_context_drq(floppy, SIG_FLOPPY_2DD_DRQ, 1);
	fdc_2dd->raise_irq_when_media_changed = true;
	floppy->set_context_fdc_2dd(fdc_2dd);
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	fdc->set_context_irq(floppy, SIG_FLOPPY_IRQ, 1);
	fdc->set_context_drq(floppy, SIG_FLOPPY_DRQ, 1);
	fdc->raise_irq_when_media_changed = true;
	floppy->set_context_fdc(fdc);
#endif
	floppy->set_context_dma(dma);
	floppy->set_context_pic(pic);
	
#if defined(SUPPORT_CMT_IF)
	sio_cmt->set_context_out(cmt, SIG_CMT_OUT);
//	sio_cmt->set_context_txrdy(cmt, SIG_CMT_TXRDY, 1);
//	sio_cmt->set_context_rxrdy(cmt, SIG_CMT_RXRDY, 1);
//	sio_cmt->set_context_txe(cmt, SIG_CMT_TXEMP, 1);
	cmt->set_context_sio(sio_cmt);
#endif
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	pc80s31k->set_context_cpu(cpu_sub);
	pc80s31k->set_context_fdc(fdc_sub);
	pc80s31k->set_context_pio(pio_sub);
	pio_fdd->set_context_port_a(pio_sub, SIG_I8255_PORT_A, 0xff, 0);
	pio_fdd->set_context_port_b(pio_sub, SIG_I8255_PORT_A, 0xff, 0);
	pio_fdd->set_context_port_c(pio_sub, SIG_I8255_PORT_C, 0x0f, 4);
	pio_fdd->set_context_port_c(pio_sub, SIG_I8255_PORT_C, 0xf0, -4);
	pio_fdd->clear_ports_by_cmdreg = true;
	pio_sub->set_context_port_a(pio_fdd, SIG_I8255_PORT_B, 0xff, 0);
	pio_sub->set_context_port_b(pio_fdd, SIG_I8255_PORT_A, 0xff, 0);
	pio_sub->set_context_port_c(pio_fdd, SIG_I8255_PORT_C, 0x0f, 4);
	pio_sub->set_context_port_c(pio_fdd, SIG_I8255_PORT_C, 0xf0, -4);
	pio_sub->clear_ports_by_cmdreg = true;
	fdc_sub->set_context_irq(cpu_sub, SIG_CPU_IRQ, 1);
	cpu_sub->set_context_mem(pc80s31k);
	cpu_sub->set_context_io(pc80s31k);
	cpu_sub->set_context_intr(pc80s31k);
#ifdef USE_DEBUGGER
	cpu_sub->set_context_debugger(new DEBUGGER(this, emu));
#endif
#endif
	
	// memory bus
	memset(ram, 0, sizeof(ram));
	memset(ipl, 0xff, sizeof(ipl));
	memset(sound_bios, 0xff, sizeof(sound_bios));
#if defined(_PC9801) || defined(_PC9801E)
	memset(fd_bios_2hd, 0xff, sizeof(fd_bios_2hd));
	memset(fd_bios_2dd, 0xff, sizeof(fd_bios_2dd));
#endif
	
	memory->read_bios(_T("IPL.ROM"), ipl, sizeof(ipl));
	if(sound_device_type == 0) {
		display->sound_bios_ok = (memory->read_bios(_T("SOUND.ROM"), sound_bios, sizeof(sound_bios)) != 0);
	} else if(sound_device_type == 2) {
		display->sound_bios_ok = (memory->read_bios(_T("MUSIC.ROM"), sound_bios, sizeof(sound_bios)) != 0);
	} else {
		display->sound_bios_ok = false;
	}
#if defined(_PC9801) || defined(_PC9801E)
	memory->read_bios(_T("2HDIF.ROM"), fd_bios_2hd, sizeof(fd_bios_2hd));
	memory->read_bios(_T("2DDIF.ROM"), fd_bios_2dd, sizeof(fd_bios_2dd));
#endif
	
	memory->set_memory_rw(0x00000, 0x9ffff, ram);
	// A0000h - A1FFFh: TEXT VRAM
	// A2000h - A3FFFh: ATTRIBUTE
	memory->set_memory_mapped_io_rw(0xa0000, 0xa3fff, display);
	// A8000h - BFFFFh: VRAM
	memory->set_memory_mapped_io_rw(0xa8000, 0xbffff, display);
	if(sound_device_type == 0 || sound_device_type == 2) {
		memory->set_memory_r(0xcc000, 0xcffff, sound_bios);
	}
#if defined(_PC9801) || defined(_PC9801E)
	memory->set_memory_r(0xd6000, 0xd6fff, fd_bios_2dd);
	memory->set_memory_r(0xd7000, 0xd7fff, fd_bios_2hd);
#endif
#if defined(SUPPORT_16_COLORS)
	// E0000h - E7FFFh: VRAM
	memory->set_memory_mapped_io_rw(0xe0000, 0xe7fff, display);
#endif
	memory->set_memory_r(0xe8000, 0xfffff, ipl);
	
	// i/o bus
	io->set_iomap_alias_rw(0x00, pic, 0);
	io->set_iomap_alias_rw(0x02, pic, 1);
	io->set_iomap_alias_rw(0x08, pic, 2);
	io->set_iomap_alias_rw(0x0a, pic, 3);
	
	io->set_iomap_alias_rw(0x01, dma, 0x00);
	io->set_iomap_alias_rw(0x03, dma, 0x01);
	io->set_iomap_alias_rw(0x05, dma, 0x02);
	io->set_iomap_alias_rw(0x07, dma, 0x03);
	io->set_iomap_alias_rw(0x09, dma, 0x04);
	io->set_iomap_alias_rw(0x0b, dma, 0x05);
	io->set_iomap_alias_rw(0x0d, dma, 0x06);
	io->set_iomap_alias_rw(0x0f, dma, 0x07);
	io->set_iomap_alias_rw(0x11, dma, 0x08);
	io->set_iomap_alias_w(0x13, dma, 0x09);
	io->set_iomap_alias_w(0x15, dma, 0x0a);
	io->set_iomap_alias_w(0x17, dma, 0x0b);
	io->set_iomap_alias_w(0x19, dma, 0x0c);
	io->set_iomap_alias_rw(0x1b, dma, 0x0d);
	io->set_iomap_alias_w(0x1d, dma, 0x0e);
	io->set_iomap_alias_w(0x1f, dma, 0x0f);
	io->set_iomap_single_w(0x21, dmareg1);
	io->set_iomap_single_w(0x23, dmareg2);
	io->set_iomap_single_w(0x25, dmareg3);
	io->set_iomap_single_w(0x27, dmareg0);
	
	io->set_iomap_single_w(0x20, rtcreg);
	
	io->set_iomap_alias_rw(0x30, sio_rs, 0);
	io->set_iomap_alias_rw(0x32, sio_rs, 1);
	
	io->set_iomap_alias_rw(0x31, pio_sys, 0);
	io->set_iomap_alias_rw(0x33, pio_sys, 1);
	io->set_iomap_alias_rw(0x35, pio_sys, 2);
	io->set_iomap_alias_w(0x37, pio_sys, 3);
	
	io->set_iomap_alias_rw(0x40, pio_prn, 0);
	io->set_iomap_alias_rw(0x42, pio_prn, 1);
	io->set_iomap_alias_rw(0x44, pio_prn, 2);
	io->set_iomap_alias_w(0x46, pio_prn, 3);
	
	io->set_iomap_alias_rw(0x41, sio_kbd, 0);
	io->set_iomap_alias_rw(0x43, sio_kbd, 1);
	
	// 50h, 52h: NMI Flip Flop
	
#if defined(SUPPORT_320KB_FDD_IF)
	io->set_iomap_alias_rw(0x51, pio_fdd, 0);
	io->set_iomap_alias_rw(0x53, pio_fdd, 1);
	io->set_iomap_alias_rw(0x55, pio_fdd, 2);
	io->set_iomap_alias_w(0x57, pio_fdd, 3);
#endif
	
	io->set_iomap_alias_rw(0x60, gdc_chr, 0);
	io->set_iomap_alias_rw(0x62, gdc_chr, 1);
	
	io->set_iomap_single_w(0x64, display);
	io->set_iomap_single_w(0x68, display);
#if defined(SUPPORT_16_COLORS)
	io->set_iomap_single_w(0x6a, display);
#endif
	io->set_iomap_single_w(0x6c, display);
	io->set_iomap_single_w(0x6e, display);
	
	io->set_iomap_single_w(0x70, display);
	io->set_iomap_single_w(0x72, display);
	io->set_iomap_single_w(0x74, display);
	io->set_iomap_single_w(0x76, display);
	io->set_iomap_single_w(0x78, display);
	io->set_iomap_single_w(0x7a, display);
#if defined(SUPPORT_16_COLORS)
	io->set_iomap_single_w(0x7c, display);
	io->set_iomap_single_w(0x7e, display);
#endif
	
	io->set_iomap_alias_rw(0x71, pit, 0);
	io->set_iomap_alias_rw(0x73, pit, 1);
	io->set_iomap_alias_rw(0x75, pit, 2);
	io->set_iomap_alias_w(0x77, pit, 3);
	
	// 80h, 82h: SASI
	
	io->set_iomap_single_rw(0x90, floppy);
	io->set_iomap_single_rw(0x92, floppy);
	io->set_iomap_single_rw(0x94, floppy);
	
#if defined(SUPPORT_CMT_IF)
	io->set_iomap_alias_rw(0x91, sio_cmt, 0);
	io->set_iomap_alias_rw(0x93, sio_cmt, 1);
	io->set_iomap_single_w(0x95, cmt);
	io->set_iomap_single_w(0x97, cmt);
#endif
	
	io->set_iomap_alias_rw(0xa0, gdc_gfx, 0);
	io->set_iomap_alias_rw(0xa2, gdc_gfx, 1);
	
#if defined(SUPPORT_2ND_VRAM)
	io->set_iomap_single_w(0xa4, display);
	io->set_iomap_single_w(0xa6, display);
#endif
	io->set_iomap_single_rw(0xa8, display);
	io->set_iomap_single_rw(0xaa, display);
	io->set_iomap_single_rw(0xac, display);
	io->set_iomap_single_rw(0xae, display);
	
	io->set_iomap_single_w(0xa1, display);
	io->set_iomap_single_w(0xa3, display);
	io->set_iomap_single_w(0xa5, display);
	io->set_iomap_single_rw(0xa9, display);
	
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	io->set_iomap_single_rw(0xbe, floppy);
#endif
	io->set_iomap_single_rw(0xc8, floppy);
	io->set_iomap_single_rw(0xca, floppy);
	io->set_iomap_single_rw(0xcc, floppy);
	
	if(sound_device_type == 0 || sound_device_type == 1) {
		io->set_iomap_single_rw(0x188, fmsound);
		io->set_iomap_single_rw(0x18a, fmsound);
#ifdef SUPPORT_PC98_OPNA
		io->set_iomap_single_rw(0x18c, fmsound);
		io->set_iomap_single_rw(0x18e, fmsound);
		io->set_iomap_single_rw(0xa460, fmsound);
#endif
	} else if(sound_device_type == 2 || sound_device_type == 3) {
		io->set_iomap_alias_rw(0x88, pio_14, 0);
		io->set_iomap_alias_rw(0x8a, pio_14, 1);
		io->set_iomap_alias_rw(0x8c, pio_14, 2);
		io->set_iomap_alias_w(0x8e, pio_14, 3);
		io->set_iovalue_single_r(0x8e, 0x08);
		io->set_iomap_single_w(0x188, maskreg_14);
		io->set_iomap_single_w(0x18a, maskreg_14);
		io->set_iomap_alias_rw(0x18c, pit_14, 2);
		io->set_iomap_alias_w(0x18e, pit_14, 3);
//		io->set_iovalue_single_r(0x18e, 0x00); // INT0
//		io->set_iovalue_single_r(0x18e, 0x40); // INT41
		io->set_iovalue_single_r(0x18e, 0x80); // INT5
//		io->set_iovalue_single_r(0x18e, 0xc0); // INT6
	}
	
#if !defined(SUPPORT_OLD_BUZZER)
	io->set_iomap_alias_rw(0x3fd9, pit, 0);
	io->set_iomap_alias_rw(0x3fdb, pit, 1);
	io->set_iomap_alias_rw(0x3fdd, pit, 2);
	io->set_iomap_alias_w(0x3fdf, pit, 3);
#endif
	
	io->set_iomap_alias_rw(0x7fd9, pio_mouse, 0);
	io->set_iomap_alias_rw(0x7fdb, pio_mouse, 1);
	io->set_iomap_alias_rw(0x7fdd, pio_mouse, 2);
	io->set_iomap_alias_w(0x7fdf, pio_mouse, 3);
#if !(defined(_PC9801) || defined(_PC9801E))
	io->set_iomap_single_w(0xbfdb, mouse);
#endif
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88event = new EVENT(this, emu);
	pc88event->set_frames_per_sec(60);
	pc88event->set_lines_per_frame(260);
	
	pc88 = new PC88(this, emu);
	pc88->set_context_event_manager(pc88event);
	pc88sio = new I8251(this, emu);
	pc88sio->set_context_event_manager(pc88event);
	pc88pio = new I8255(this, emu);
	pc88pio->set_context_event_manager(pc88event);
	pc88pcm = new PCM1BIT(this, emu);
	pc88pcm->set_context_event_manager(pc88event);
	pc88rtc = new UPD1990A(this, emu);
	pc88rtc->set_context_event_manager(pc88event);
	pc88opn = new YM2203(this, emu);
	pc88opn->set_context_event_manager(pc88event);
#ifdef SUPPORT_PC88_OPNA
	pc88opn->is_ym2608 = true;
#endif
	pc88cpu = new Z80(this, emu);
	pc88cpu->set_context_event_manager(pc88event);
	
	if(config.printer_device_type == 0) {
		pc88prn = new PRNFILE(this, emu);
		pc88prn->set_context_event_manager(pc88event);
//	} else if(config.printer_device_type == 1) {
//		pc88prn = new PCPR201(this, emu);
//		pc88prn->set_context_event_manager(pc88event);
	} else {
		pc88prn = dummy;
	}
	
	pc88sub = new PC80S31K(this, emu);
	pc88sub->set_context_event_manager(pc88event);
	pc88pio_sub = new I8255(this, emu);
	pc88pio_sub->set_context_event_manager(pc88event);
	pc88fdc_sub = new UPD765A(this, emu);
	pc88fdc_sub->set_context_event_manager(pc88event);
	pc88cpu_sub = new Z80(this, emu);
	pc88cpu_sub->set_context_event_manager(pc88event);
	
	pc88event->set_context_cpu(pc88cpu, (config.cpu_type != 0) ? 3993624 : 7987248);
	pc88event->set_context_cpu(pc88cpu_sub, 3993624);
	pc88event->set_context_sound(pc88opn);
	pc88event->set_context_sound(pc88pcm);
	
	pc88->set_context_cpu(pc88cpu);
	pc88->set_context_opn(pc88opn);
	pc88->set_context_pcm(pc88pcm);
	pc88->set_context_pio(pc88pio);
	pc88->set_context_prn(pc88prn);
	pc88->set_context_rtc(pc88rtc);
	pc88->set_context_sio(pc88sio);
	pc88cpu->set_context_mem(pc88);
	pc88cpu->set_context_io(pc88);
	pc88cpu->set_context_intr(pc88);
#ifdef USE_DEBUGGER
	pc88cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pc88opn->set_context_irq(pc88, SIG_PC88_SOUND_IRQ, 1);
	pc88sio->set_context_rxrdy(pc88, SIG_PC88_USART_IRQ, 1);
	pc88sio->set_context_out(pc88, SIG_PC88_USART_OUT);
	
	pc88sub->set_context_cpu(pc88cpu_sub);
	pc88sub->set_context_fdc(pc88fdc_sub);
	pc88sub->set_context_pio(pc88pio_sub);
	pc88pio->set_context_port_a(pc88pio_sub, SIG_I8255_PORT_B, 0xff, 0);
	pc88pio->set_context_port_b(pc88pio_sub, SIG_I8255_PORT_A, 0xff, 0);
	pc88pio->set_context_port_c(pc88pio_sub, SIG_I8255_PORT_C, 0x0f, 4);
	pc88pio->set_context_port_c(pc88pio_sub, SIG_I8255_PORT_C, 0xf0, -4);
	pc88pio->clear_ports_by_cmdreg = true;
	pc88pio_sub->set_context_port_a(pc88pio, SIG_I8255_PORT_B, 0xff, 0);
	pc88pio_sub->set_context_port_b(pc88pio, SIG_I8255_PORT_A, 0xff, 0);
	pc88pio_sub->set_context_port_c(pc88pio, SIG_I8255_PORT_C, 0x0f, 4);
	pc88pio_sub->set_context_port_c(pc88pio, SIG_I8255_PORT_C, 0xf0, -4);
	pc88pio_sub->clear_ports_by_cmdreg = true;
	pc88fdc_sub->set_context_irq(pc88cpu_sub, SIG_CPU_IRQ, 1);
	pc88cpu_sub->set_context_mem(pc88sub);
	pc88cpu_sub->set_context_io(pc88sub);
	pc88cpu_sub->set_context_intr(pc88sub);
#ifdef USE_DEBUGGER
	pc88cpu_sub->set_context_debugger(new DEBUGGER(this, emu));
#endif
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
#if defined(_PC9801) || defined(_PC9801E)
	fdc_2hd->get_disk_handler(0)->drive_num = 0;
	fdc_2hd->get_disk_handler(1)->drive_num = 1;
	fdc_2dd->get_disk_handler(0)->drive_num = 2;
	fdc_2dd->get_disk_handler(1)->drive_num = 3;
	fdc_sub->get_disk_handler(0)->drive_num = 4;
	fdc_sub->get_disk_handler(1)->drive_num = 5;
#elif defined(_PC9801VF) || defined(_PC9801U)
	fdc_2dd->get_disk_handler(0)->drive_num = 0;
	fdc_2dd->get_disk_handler(1)->drive_num = 1;
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	fdc->get_disk_handler(0)->drive_num = 0;
	fdc->get_disk_handler(1)->drive_num = 1;
	pc88fdc_sub->get_disk_handler(0)->drive_num = 2;
	pc88fdc_sub->get_disk_handler(1)->drive_num = 3;
#else
	fdc->get_disk_handler(0)->drive_num = 0;
	fdc->get_disk_handler(1)->drive_num = 1;
#endif
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
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
#endif
	
	// initial device settings
	pio_mouse->write_signal(SIG_I8255_PORT_A, 0xf0, 0xff);	// clear mouse status
	pio_mouse->write_signal(SIG_I8255_PORT_B, 0x40, 0xff);	// cpu high & sw3-6
	uint8_t port_c = 0x08;	// normal mode & sw1-5 & sw1-6
#if defined(HAS_V30) || defined(HAS_V33)
	port_c |= 0x04;		// V30
#endif
	pio_mouse->write_signal(SIG_I8255_PORT_C, 0x40, 0xff);
	
	pio_sys->write_signal(SIG_I8255_PORT_A, 0xe3, 0xff);
	pio_sys->write_signal(SIG_I8255_PORT_B, 0xf8, 0xff);//0xe8??
	
#if defined(_PC9801)
	uint8_t prn_b = 0x00;	// system type = first PC-9801
#elif defined(_PC9801U)
	uint8_t prn_b = 0xc0;	// system type = PC-9801U,PC-98LT,PC-98HA
#else
	uint8_t prn_b = 0x80;	// system type = others
#endif
	if(pit_clock_8mhz) {
		prn_b |= 0x20;		// system clock is 8MHz
	}
	prn_b |= 0x10;		// don't use LCD display
#if !defined(SUPPORT_16_COLORS)
	prn_b |= 0x08;		// standard graphics
#endif
	prn_b |= 0x04;		// printer is not busy
#if defined(HAS_V30) || defined(HAS_V33)
	prn_b |= 0x02;
#endif
#if defined(_PC9801VF) || defined(_PC9801U)
	prn_b |= 0x01;		// PC-9801VF or PC-9801U
#endif
	pio_prn->write_signal(SIG_I8255_PORT_B, prn_b, 0xff);
	
#if defined(SUPPORT_320KB_FDD_IF)
	pio_fdd->write_signal(SIG_I8255_PORT_A, 0xff, 0xff);
	pio_fdd->write_signal(SIG_I8255_PORT_B, 0xff, 0xff);
	pio_fdd->write_signal(SIG_I8255_PORT_C, 0xff, 0xff);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	fdc_2dd->write_signal(SIG_UPD765A_FREADY, 1, 1);	// 2DD FDC RDY is pulluped
#endif
	
	if(sound_device_type == 0 || sound_device_type == 1) {
//		opn->write_signal(SIG_YM2203_PORT_A, 0x3f, 0xff);	// PC-9801-26(K) INT0
//		opn->write_signal(SIG_YM2203_PORT_A, 0xbf, 0xff);	// PC-9801-26(K) INT41
		opn->write_signal(SIG_YM2203_PORT_A, 0xff, 0xff);	// PC-9801-26(K) INT5
//		opn->write_signal(SIG_YM2203_PORT_A, 0x7f, 0xff);	// PC-9801-26(K) INT6
	}
	
#if defined(SUPPORT_OLD_BUZZER)
	beep->write_signal(SIG_BEEP_ON, 1, 1);
	beep->write_signal(SIG_BEEP_MUTE, 1, 1);
#else
	beep->write_signal(SIG_PCM1BIT_ON, 1, 1);
	beep->write_signal(SIG_PCM1BIT_MUTE, 1, 1);
#endif
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88opn->set_reg(0x29, 3); // for Misty Blue
	pc88pio->write_signal(SIG_I8255_PORT_C, 0, 0xff);
	pc88pio_sub->write_signal(SIG_I8255_PORT_C, 0, 0xff);
#endif
}

void VM::run()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		pc88event->drive();
	} else
#endif
	event->drive();
}

double VM::get_frame_rate()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(config.boot_mode != 0) {
		return pc88event->get_frame_rate();
	} else
#endif
	return event->get_frame_rate();
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(index == 0 && boot_mode == 0) {
		return cpu;
	} else if(index == 1 && boot_mode != 0) {
		return pc88cpu;
	} else if(index == 2 && boot_mode != 0) {
		return pc88cpu_sub;
	}
#else
	if(index == 0) {
		return cpu;
#if defined(SUPPORT_320KB_FDD_IF)
	} else if(index == 1) {
		return cpu_sub;
#endif
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
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		pc88->draw_screen();
	} else
#endif
	display->draw_screen();
}

uint32_t VM::get_access_lamp_status()
{
#if defined(_PC9801) || defined(_PC9801E)
	return (fdc_2hd->read_signal(0) & 3) | (fdc_2dd->read_signal(0) & 3) | (fdc_sub->read_signal(0) & 3);
#elif defined(_PC9801VF) || defined(_PC9801U)
	return fdc_2dd->read_signal(0);
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		return pc88fdc_sub->read_signal(0);
	} else {
		return fdc->read_signal(0);
	}
#else
	return fdc->read_signal(0);
#endif
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
#if defined(SUPPORT_OLD_BUZZER)
	beep->initialize_sound(rate, 2400, 8000);
#else
	beep->initialize_sound(rate, 8000);
#endif
	if(sound_device_type == 0 || sound_device_type == 1) {
#ifdef HAS_YM2608
		opn->initialize_sound(rate, 7987248, samples, 0, 0);
#else
		opn->initialize_sound(rate, 3993624, samples, 0, 0);
#endif
	} else if(sound_device_type == 2 || sound_device_type == 3) {
		tms3631->initialize_sound(rate, 8000);
	}
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	// init sound manager
	pc88event->initialize_sound(rate, samples);
	
	// init sound gen
#ifdef HAS_YM2608
	pc88opn->initialize_sound(rate, 7987248, samples, 0, 0);
#else
	pc88opn->initialize_sound(rate, 3993624, samples, 0, 0);
#endif
	pc88pcm->initialize_sound(rate, 8000);
#endif
}

uint16_t* VM::create_sound(int* extra_frames)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		return pc88event->create_sound(extra_frames);
	} else
#endif
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		return pc88event->get_sound_buffer_ptr();
	} else
#endif
	return event->get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch-- == 0) {
		if(sound_device_type == 0 || sound_device_type == 1) {
			opn->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		if(sound_device_type == 0 || sound_device_type == 1) {
			opn->set_volume(1, decibel_l, decibel_r);
		}
#if defined(SUPPORT_PC98_OPNA)
	} else if(ch-- == 0) {
		if(sound_device_type == 0 || sound_device_type == 1) {
			opn->set_volume(2, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		if(sound_device_type == 0 || sound_device_type == 1) {
			opn->set_volume(3, decibel_l, decibel_r);
		}
#endif
	} else if(ch-- == 0) {
		if(sound_device_type == 2 || sound_device_type == 3) {
			tms3631->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		beep->set_volume(0, decibel_l, decibel_r);
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	} else if(ch-- == 0) {
		pc88opn->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		pc88opn->set_volume(1, decibel_l, decibel_r);
#if defined(SUPPORT_PC88_OPNA)
	} else if(ch-- == 0) {
		pc88opn->set_volume(2, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		pc88opn->set_volume(3, decibel_l, decibel_r);
#endif
	} else if(ch-- == 0) {
		pc88pcm->set_volume(0, decibel_l, decibel_r);
#endif
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		pc88->key_down(code, repeat);
	} else
#endif
	keyboard->key_down(code, repeat);
}

void VM::key_up(int code)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
//		pc88->key_up(code);
	} else
#endif
	keyboard->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
#if defined(_PC9801) || defined(_PC9801E)
	if(drv == 0 || drv == 1) {
		fdc_2hd->open_disk(drv, file_path, bank);
	} else if(drv == 2 || drv == 3) {
		fdc_2dd->open_disk(drv - 2, file_path, bank);
	} else if(drv == 4 || drv == 5) {
		fdc_sub->open_disk(drv - 4, file_path, bank);
	}
#elif defined(_PC9801VF) || defined(_PC9801U)
	if(drv == 0 || drv == 1) {
		fdc_2dd->open_disk(drv, file_path, bank);
	}
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	if(drv == 0 || drv == 1) {
		fdc->open_disk(drv, file_path, bank);
	} else if(drv == 2 || drv == 3) {
		pc88fdc_sub->open_disk(drv - 2, file_path, bank);
	}
#else
	if(drv == 0 || drv == 1) {
		fdc->open_disk(drv, file_path, bank);
	}
#endif
}

void VM::close_floppy_disk(int drv)
{
#if defined(_PC9801) || defined(_PC9801E)
	if(drv == 0 || drv == 1) {
		fdc_2hd->close_disk(drv);
	} else if(drv == 2 || drv == 3) {
		fdc_2dd->close_disk(drv - 2);
	} else if(drv == 4 || drv == 5) {
		fdc_sub->close_disk(drv - 4);
	}
#elif defined(_PC9801VF) || defined(_PC9801U)
	if(drv == 0 || drv == 1) {
		fdc_2dd->close_disk(drv);
	}
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	if(drv == 0 || drv == 1) {
		fdc->close_disk(drv);
	} else if(drv == 2 || drv == 3) {
		pc88fdc_sub->close_disk(drv - 2);
	}
#else
	if(drv == 0 || drv == 1) {
		fdc->close_disk(drv);
	}
#endif
}

bool VM::is_floppy_disk_inserted(int drv)
{
#if defined(_PC9801) || defined(_PC9801E)
	if(drv == 0 || drv == 1) {
		return fdc_2hd->is_disk_inserted(drv);
	} else if(drv == 2 || drv == 3) {
		return fdc_2dd->is_disk_inserted(drv - 2);
	} else if(drv == 4 || drv == 5) {
		return fdc_sub->is_disk_inserted(drv - 4);
	}
#elif defined(_PC9801VF) || defined(_PC9801U)
	if(drv == 0 || drv == 1) {
		return fdc_2dd->is_disk_inserted(drv);
	}
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	if(drv == 0 || drv == 1) {
		return fdc->is_disk_inserted(drv);
	} else if(drv == 2 || drv == 3) {
		return pc88fdc_sub->is_disk_inserted(drv - 2);
	}
#else
	if(drv == 0 || drv == 1) {
		return fdc->is_disk_inserted(drv);
	}
#endif
	return false;
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
#if defined(_PC9801) || defined(_PC9801E)
	if(drv == 0 || drv == 1) {
		fdc_2hd->is_disk_protected(drv, value);
	} else if(drv == 2 || drv == 3) {
		fdc_2dd->is_disk_protected(drv - 2, value);
	} else if(drv == 4 || drv == 5) {
		fdc_sub->is_disk_protected(drv - 4, value);
	}
#elif defined(_PC9801VF) || defined(_PC9801U)
	if(drv == 0 || drv == 1) {
		fdc_2dd->is_disk_protected(drv, value);
	}
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	if(drv == 0 || drv == 1) {
		fdc->is_disk_protected(drv, value);
	} else if(drv == 2 || drv == 3) {
		pc88fdc_sub->is_disk_protected(drv - 2, value);
	}
#else
	if(drv == 0 || drv == 1) {
		fdc->is_disk_protected(drv, value);
	}
#endif
}

bool VM::is_floppy_disk_protected(int drv)
{
#if defined(_PC9801) || defined(_PC9801E)
	if(drv == 0 || drv == 1) {
		return fdc_2hd->is_disk_protected(drv);
	} else if(drv == 2 || drv == 3) {
		return fdc_2dd->is_disk_protected(drv - 2);
	} else if(drv == 4 || drv == 5) {
		return fdc_sub->is_disk_protected(drv - 4);
	}
#elif defined(_PC9801VF) || defined(_PC9801U)
	if(drv == 0 || drv == 1) {
		return fdc_2dd->is_disk_protected(drv);
	}
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	if(drv == 0 || drv == 1) {
		return fdc->is_disk_protected(drv);
	} else if(drv == 2 || drv == 3) {
		return pc88fdc_sub->is_disk_protected(drv - 2);
	}
#else
	if(drv == 0 || drv == 1) {
		return fdc->is_disk_protected(drv);
	}
#endif
	return false;
}

#if defined(SUPPORT_CMT_IF) || defined(_PC98DO) || defined(_PC98DOPLUS)
void VM::play_tape(const _TCHAR* file_path)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88->play_tape(file_path);
#else
	cmt->play_tape(file_path);
#endif
}

void VM::rec_tape(const _TCHAR* file_path)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88->rec_tape(file_path);
#else
	cmt->rec_tape(file_path);
#endif
}

void VM::close_tape()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88->close_tape();
#else
	cmt->close_tape();
#endif
}

bool VM::is_tape_inserted()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	return pc88->is_tape_inserted();
#else
	return cmt->is_tape_inserted();
#endif
}
#endif

bool VM::is_frame_skippable()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
//		return pc88event->is_frame_skippable();
		return pc88->is_frame_skippable();
	} else
#endif
	return event->is_frame_skippable();
}

void VM::update_config()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != config.boot_mode) {
		// boot mode is changed !!!
		boot_mode = config.boot_mode;
		reset();
	} else
#endif
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

#define STATE_VERSION	7

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->FputBool(pit_clock_8mhz);
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	state_fio->FputInt32(boot_mode);
#endif
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
	state_fio->Fread(ram, sizeof(ram), 1);
	pit_clock_8mhz = state_fio->FgetBool();
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	boot_mode = state_fio->FgetInt32();
#endif
	sound_device_type = state_fio->FgetInt32();
	return true;
}
