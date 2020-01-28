/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ virtual machine ]
*/

#ifndef _MZ5500_H_
#define _MZ5500_H_

#if defined(_MZ5500)
#define DEVICE_NAME		"SHARP MZ-5500"
#define CONFIG_NAME		"mz5500"
#elif defined(_MZ6500)
#define DEVICE_NAME		"SHARP MZ-6500"
#define CONFIG_NAME		"mz6500"
#elif defined(_MZ6550)
#define DEVICE_NAME		"SHARP MZ-6550"
#define CONFIG_NAME		"mz6550"
#endif
#define CONFIG_VERSION		0x02

// device informations for virtual machine
#define FRAMES_PER_10SECS	554
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		108
#if defined(_MZ5500)
#define CPU_CLOCKS		4915200
#elif defined(_MZ6500) || defined(_MZ6550)
#define CPU_CLOCKS		8000000
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#ifdef _MZ6550
#define HAS_I286
#else
#define HAS_I86
#endif
#define I8259_MAX_CHIPS		2
#define UPD765A_DMA_MODE
#define UPD765A_WAIT_SEEK
#define UPD765A_STRICT_ID
#define Z80CTC_CLOCKS		2457600
#define IO_ADDR_MAX		0x400
#define HAS_AY_3_8912

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_SCANLINE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class I8237;
class I8255;
class I8259;
class I86;
class IO;
class LS393;
class RP5C01;
class UPD7220;
class UPD765A;
class YM2203;
class Z80CTC;
class Z80SIO;

class DISPLAY;
class KEYBOARD;
class MEMORY;
class SYSPORT;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8237* dma;
	I8255* pio;
	I8259* pic;	// includes 2chips
	I86* cpu;
	IO* io;
	LS393* div;
	RP5C01* rtc;
	UPD7220* gdc;
	UPD765A* fdc;
	YM2203* psg;
	Z80CTC* ctc0;
#if defined(_MZ6500) || defined(_MZ6550)
	Z80CTC* ctc1;
#endif
	Z80SIO* sio;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
	MEMORY* memory;
	SYSPORT* sysport;
	
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
	void special_reset();
	void run();
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_disk(_TCHAR* filename, int drv);
	void close_disk(int drv);
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
	void regist_vline_event(DEVICE* dev);
	
	// clock
	uint32 current_clock();
	uint32 passed_clock(uint32 prev);
	uint32 get_prv_pc();
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
