/*
	FUJITSU FMR-30 Emulator 'eFMR-30'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ virtual machine ]
*/

#ifndef _FMR30_H_
#define _FMR30_H_

// device informations for win32
#define DEVICE_NAME		"FUJITSU FMR-30"
#define CONFIG_NAME		"fmr30"
#define CONFIG_VERSION		0x01

#define WINDOW_WIDTH1		640
#define WINDOW_HEIGHT1		400
#define WINDOW_WIDTH2		640
#define WINDOW_HEIGHT2		400

//#define USE_IPL_RESET
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6

// device informations for virtual machine
#define FRAMES_PER_10SECS	554
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		108
#define CPU_CLOCKS		8000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		2
#define MAX_SCSI		8
//#define HAS_I286
#define HAS_I86
#define I86_BIOS_CALL
#define HAS_I8254
#define I8259_MAX_CHIPS		2
#define IO_ADDR_MAX		0x10000

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class I8237;
class I8251;
class I8253;
class I8259;
class I86;
class IO;
class MB8877;
class SN76489AN;

class BIOS;
class CMOS;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class RTC;
//class SCSI;
class SERIAL;
class SYSTEM;
class TIMER;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8237* dma;
	I8251* sio_kb;
	I8251* sio_sub;
	I8251* sio_ch1;
	I8251* sio_ch2;
	I8253* pit;
	I8259* pic;
	I86* cpu;
	IO* io;
	MB8877* fdc;
	SN76489AN* psg;
	
	BIOS* bios;
	CMOS* cmos;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	RTC* rtc;
//	SCSI* scsi;
	SERIAL* serial;
	SYSTEM* system;
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
	uint32 get_prv_pc();
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
