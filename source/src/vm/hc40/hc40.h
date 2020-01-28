/*
	EPSON HC-40 Emulator 'eHC-40'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.02.23 -

	[ virtual machine ]
*/

#ifndef _HC40_H_
#define _HC40_H_

#define DEVICE_NAME		"EPSON HC-40"
#define CONFIG_NAME		"hc40"
#define CONFIG_VERSION		0x01

// device informations for virtual machine
#define FRAMES_PER_10SECS	720
#define FRAMES_PER_SEC		72
#define LINES_PER_FRAME		64
#define CHARS_PER_LINE		1
#define CPU_CLOCKS		3686400
#define SCREEN_WIDTH		240
#define SCREEN_HEIGHT		64
#define MAX_DRIVE		4

// device informations for win32
#define WINDOW_WIDTH		(SCREEN_WIDTH * 2)
#define WINDOW_HEIGHT		(SCREEN_HEIGHT * 2)

#define USE_SPECIAL_RESET
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0x6f
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define USE_DATAREC
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class DATAREC;
class TF20;
class Z80;

class IO;
class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	DATAREC* drec;
	TF20* tf20;
	Z80* cpu;
	
	IO* io;
	MEMORY* memory;
	
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
	uint16* create_sound(int samples, bool fill);
	
	// notify key
	void key_down(int code);
	void key_up(int code);
	
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
