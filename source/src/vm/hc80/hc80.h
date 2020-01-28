/*
	EPSON HC-80 Emulator 'eHC-80'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.03.14 -

	[ virtual machine ]
*/

#ifndef _HC80_H_
#define _HC80_H_

// device informations for win32
#define DEVICE_NAME		"EPSON HC-80"
#define CONFIG_NAME		"hc80"
#define CONFIG_VERSION		0x02

#define WINDOW_WIDTH1		480
#define WINDOW_HEIGHT1		64
#define WINDOW_WIDTH2		960
#define WINDOW_HEIGHT2		128

#define USE_IPL_RESET
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0x6f
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_SCREEN_X2

// device informations for virtual machine
#define FRAMES_PER_10SECS	640
#define FRAMES_PER_SEC		64
#define LINES_PER_FRAME		64
#define CHARS_PER_LINE		1
#define CPU_CLOCKS		2457600
#define SCREEN_WIDTH		480
#define SCREEN_HEIGHT		64
#define MAX_DRIVE		4

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class I8251;
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
	I8251* sio;
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
	void ipl_reset();
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
	
	// clock
	uint32 current_clock();
	uint32 passed_clock(uint32 prev);
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif