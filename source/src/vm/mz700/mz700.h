/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-1500 Emulator 'EmuZ-1500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ virtual machine ]
*/

#ifndef _MZ700_H_
#define _MZ700_H_

#ifdef _MZ1500
#define DEVICE_NAME		"SHARP MZ-1500"
#define CONFIG_NAME		"mz1500"
#else
#define DEVICE_NAME		"SHARP MZ-700"
#define CONFIG_NAME		"mz700"
#endif
#define CONFIG_VERSION		0x03

// device informations for virtual machine
#define FRAMES_PER_10SECS	600
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CHARS_PER_LINE		1
//#define CPU_CLOCKS		3579545
#define CPU_CLOCKS		3584160
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define USE_PCM1BIT
#define PCM1BIT_HIGH_QUALITY
//#define LOW_PASS_FILTER
#define CPU_MEMORY_WAIT
#ifdef _MZ1500
// 1byte=32clock/3.25MHz*8=79usec
#define Z80SIO_DELAY_SEND	100
#define Z80SIO_DELAY_RECV	100
#endif

// device informations for win32
#ifdef _MZ1500
#define USE_QUICKDISK
#endif
#define USE_DATAREC
#define USE_DATAREC_BUTTON
#define DATAREC_MZT
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class AND;
class DATAREC;
class I8253;
class I8255;
class IO;
class PCM1BIT;
class Z80;

//class CMOS;
class DISPLAY;
class EMM;
class KEYBOARD;
class MEMORY;
class RAMFILE;

#ifdef _MZ1500
class SN76489AN;
class Z80PIO;
class Z80SIO;
class PSG;
class QUICKDISK;
#endif

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	AND* and_int;
	DATAREC* drec;
	I8253* pit;
	I8255* pio;
	IO* io;
	PCM1BIT* pcm;
	Z80* cpu;
	
//	CMOS* cmos;
	DISPLAY* display;
	EMM* emm;
	KEYBOARD* keyboard;
	MEMORY* memory;
	RAMFILE* ramfile;
	
#ifdef _MZ1500
	AND* and_snd;
	SN76489AN* psg_l;
	SN76489AN* psg_r;
	Z80PIO* pio_int;
	Z80SIO* sio_rs;	// RS-232C
	Z80SIO* sio_qd;	// QD
	
	PSG* psg;
	QUICKDISK* qd;
#endif
	
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
	
	// user interface
#ifdef _MZ1500
	void open_quickdisk(_TCHAR* filename);
	void close_quickdisk();
#endif
	void play_datarec(_TCHAR* filename);
	void rec_datarec(_TCHAR* filename);
	void close_datarec();
	void push_play();
	void push_stop();
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
	void set_pc(uint32 pc);
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
