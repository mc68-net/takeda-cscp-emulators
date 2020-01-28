/*
	NEC PC-100 Emulator 'ePC-100'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.07.12 -

	[ virtual machine ]
*/

#ifndef _PC100_H_
#define _PC100_H_

#define DEVICE_NAME		"NEC PC-100"
#define CONFIG_NAME		"pc100"
#define CONFIG_VERSION		0x01

// device informations for virtual machine
#define FRAMES_PER_10SECS	554
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	544
#define CPU_CLOCKS		6988800
#define SCREEN_WIDTH		720
#define SCREEN_HEIGHT		512
//720
#define MAX_DRIVE		4
#define HAS_I86
#define I8259_MAX_CHIPS		1
//#define PCM1BIT_HIGH_QUALITY
#define UPD765A_DRQ_DELAY
#define UPD765A_WAIT_SEEK
#define UPD765A_STRICT_ID
#define UPD765A_NO_ST0_AT_FOR_SEEK
#define MEMORY_ADDR_MAX		0x100000
#define MEMORY_BANK_SIZE	0x8000
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_SCREEN_ROTATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class AND;
class BEEP;
class I8251;
class I8255;
class I8259;
class I86;
class IO;
class MEMORY;
class PCM1BIT;
class RTC58321;
class UPD765A;

class CRTC;
class IOCTRL;
class KANJI;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	AND* and;
	BEEP* beep;
	I8251* sio;
	I8255* pio0;
	I8255* pio1;
	I8259* pic;	// includes 2chips
	I86* cpu;
	IO* io;
	MEMORY* memory;
	PCM1BIT* pcm;
	RTC58321* rtc;
	UPD765A* fdc;
	
	CRTC* crtc;
	IOCTRL* ioctrl;
	KANJI* kanji;
	
	// memory
	uint8 ram[0xc0000];	// Main RAM 768KB
	uint8 ipl[0x8000];	// IPL 32KB
	
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
