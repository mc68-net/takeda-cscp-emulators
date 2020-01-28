/*
	FUJITSU FM16pi Emulator 'eFM16pi'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.12.25-

	[ virtual machine ]
*/

#ifndef _FM16PI_H_
#define _FM16PI_H_

#define DEVICE_NAME		"FUJITSU FM16pi"
#define CONFIG_NAME		"fm16pi"
#define CONFIG_VERSION		0x01

// device informations for virtual machine

// TODO: check refresh rate
#define FRAMES_PER_10SECS	600
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	220

#define CPU_CLOCKS		4915200
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		200
#define MAX_DRIVE		4
#define HAS_I86
#define I8259_MAX_CHIPS		1
#define MEMORY_ADDR_MAX		0x100000
#define MEMORY_BANK_SIZE	0x4000
#define IO_ADDR_MAX		0x10000
//#define EVENT_PRECISE		10

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_POWER_OFF
#define USE_ACCESS_LAMP

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class I8251;
class I8253;
class I8255;
class I8259;
class I86;
class IO;
class MB8877;
class MEMORY;
class MSM5832;
class NOT;
class PCM1BIT;

class SUB;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8251* sio;
	I8253* pit;
	I8255* pio;
	I8259* pic;
	I86* cpu;
	IO* io;
	MB8877* fdc;
	MEMORY* memory;
	MSM5832* rtc;
	NOT* not;
	PCM1BIT* pcm;
	
	SUB* sub;
	
	// memory
	uint8 ram[0x80000];
	uint8 kanji[0x40000];
	uint8 cart[0x40000];
	
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
	void notify_power_off();
	void run();
	
	// draw screen
	void draw_screen();
	int access_lamp();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_disk(_TCHAR* file_path, int drv);
	void close_disk(int drv);
	bool now_skip();
	
	void update_config();
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// event callbacks
	void register_event(DEVICE* device, int event_id, int usec, bool loop, int* register_id);
	void register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id);
	void cancel_event(int register_id);
	void register_frame_event(DEVICE* dev);
	void register_vline_event(DEVICE* dev);
	
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
