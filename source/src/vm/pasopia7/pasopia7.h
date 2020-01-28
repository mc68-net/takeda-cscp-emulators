/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ virtual machine ]
*/

#ifndef _PASOPIA7_H_
#define _PASOPIA7_H_

// device informations for win32
#ifdef _LCD
#define DEVICE_NAME		"TOSHIBA PASOPIA 7 with LCD"
#define CONFIG_NAME		"pasopia7lcd"
#else
#define DEVICE_NAME		"TOSHIBA PASOPIA 7"
#define CONFIG_NAME		"pasopia7"
#endif

#ifdef _LCD
#define WINDOW_WIDTH1		320
#define WINDOW_HEIGHT1		128
#define WINDOW_WIDTH2		320
#define WINDOW_HEIGHT2		128
#else
#define WINDOW_WIDTH1		640
#define WINDOW_HEIGHT1		400
#define WINDOW_WIDTH2		640
#define WINDOW_HEIGHT2		400
#endif

#define USE_DATAREC
#define USE_FD1
#define USE_FD2
//#define USE_FD3
//#define USE_FD4
#define USE_ALT_F10_KEY
#define USE_SCANLINE

// device informations for virtual machine
#ifdef _LCD
#define FRAMES_PER_10SECS	744
#define FRAMES_PER_SEC		74.4
#define LINES_PER_FRAME 	32
#define CHARS_PER_LINE		94
#else
#define FRAMES_PER_10SECS	599
#define FRAMES_PER_SEC		59.9
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		114
#endif
#define CPU_CLOCKS		3993600
#ifdef _LCD
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		128
#else
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#endif
#define MAX_DRIVE		4
//#define UPD765A_DMA_MODE
#define UPD765A_WAIT_SEEK

// irq priority
#define IRQ_Z80PIO	0
//			1
#define IRQ_Z80CTC	2
//			3-5
#define IRQ_EXTERNAL	6

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class DATAREC;
class I8255;
class NOT;
class SN76489AN;
class UPD765A;
class Z80;
class Z80CTC;
class Z80PIC;
class Z80PIO;

class FLOPPY;
class HD46505;
class IO8;
class IOTRAP;
class KEYBOARD;
class MEMORY;
class PAC2;
class TIMER;

class VM
{
	// define friend
	friend IO8;
protected:
	EMU* emu;
	
	// devices
	DEVICE* dummy;
	EVENT* event;
	
	BEEP* beep;
	DATAREC* drec;
	I8255* pio0;
	I8255* pio1;
	I8255* pio2;
	NOT* not;
	SN76489AN* psg0;
	SN76489AN* psg1;
	UPD765A* fdc;
	Z80* cpu;
	Z80CTC* ctc;
	Z80PIC* pic;
	Z80PIO* pio;
	
	FLOPPY* floppy;
	HD46505* crtc;
	IO8* io;
	IOTRAP* iotrap;
	KEYBOARD* key;
	MEMORY* memory;
	PAC2* pac2;
	TIMER* timer;
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	
	VM(EMU* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset();
	void run();
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int samples, bool fill);
	
	// user interface
	void open_disk(_TCHAR* filename, int drv);
	void close_disk(int drv);
	void play_datarec(_TCHAR* filename);
	void rec_datarec(_TCHAR* filename);
	void close_datarec();
	bool now_skip();
	
	void update_config();
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// event callbacks
	void regist_event(DEVICE* device, int event_id, int usec, bool loop, int* regist_id);
	void regist_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* regist_id);
	void cancel_event(int regist_id);
	void regist_frame_event(DEVICE* dev);
	void regist_vsync_event(DEVICE* dev);
	void regist_hsync_event(DEVICE* dev);
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
