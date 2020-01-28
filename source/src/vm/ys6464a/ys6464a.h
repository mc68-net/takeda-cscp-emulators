/*
	SHINKO SANGYO YS-6464A Emulator 'eYS-6464A'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.12.30 -

	[ virtual machine ]
*/

#ifndef _BABBAGE_2ND_H_
#define _BABBAGE_2ND_H_

#define DEVICE_NAME		"SHINKO SANGYO YS-6464A"
#define CONFIG_NAME		"ys6464a"
#define CONFIG_VERSION		0x01

// device informations for virtual machine
#define FRAMES_PER_10SECS	300
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME 	256
#define CHARS_PER_LINE		1
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		357
//#define USE_PCM1BIT
//#define PCM1BIT_HIGH_QUALITY

// device informations for win32
#define USE_RAM
#define USE_BITMAP
#define USE_BUTTON
#define MAX_BUTTONS		21
#define USE_LED
#define MAX_LEDS		6

#include "../../common.h"

const struct {
	const _TCHAR* caption;
	int x, y;
	int width, height;
	int font_size;
	int code;
} buttons[] = {
	{_T("0"), 344, 288, 42, 42, 20, 0x30},
	{_T("1"), 403, 288, 42, 42, 20, 0x31},
	{_T("2"), 462, 288, 42, 42, 20, 0x32},
	{_T("3"), 521, 288, 42, 42, 20, 0x33},
	{_T("4"), 344, 229, 42, 42, 20, 0x34},
	{_T("5"), 403, 229, 42, 42, 20, 0x35},
	{_T("6"), 462, 229, 42, 42, 20, 0x36},
	{_T("7"), 521, 229, 42, 42, 20, 0x37},
	{_T("8"), 344, 170, 42, 42, 20, 0x38},
	{_T("9"), 403, 170, 42, 42, 20, 0x39},
	{_T("A"), 462, 170, 42, 42, 20, 0x41},
	{_T("B"), 521, 170, 42, 42, 20, 0x42},
	{_T("C"), 344, 111, 42, 42, 20, 0x43},
	{_T("D"), 403, 111, 42, 42, 20, 0x44},
	{_T("E"), 462, 111, 42, 42, 20, 0x45},
	{_T("F"), 521, 111, 42, 42, 20, 0x46},
	{_T("WRITE\nINC"), 581, 288, 42, 42, 10, 0x70},
	{_T("READ\nDEC"),  581, 229, 42, 42, 10, 0x71},
	{_T("READ\nINC"),  581, 170, 42, 42, 10, 0x72},
	{_T("ADR\nRUN"),   581, 111, 42, 42, 10, 0x73},
	{_T("RESET"),      265, 288, 42, 42, 10, 0x00}
};
const struct {
	int x, y;
	int width, height;
} leds[] = {
	{357, 23, 28, 40},
	{392, 23, 28, 40},
	{439, 23, 28, 40},
	{474, 23, 28, 40},
	{547, 23, 28, 40},
	{582, 23, 28, 40},
};

class EMU;
class DEVICE;
class EVENT;

class IO;
class I8255;
//class PCM1BIT;
class Z80;

class DISPLAY;
class KEYBOARD;
class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	IO* io;
	I8255* pio;
//	PCM1BIT* pcm;
	Z80* cpu;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
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
	void run();
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int samples, bool fill);
	
	// user interface
	void load_ram(_TCHAR* filename);
	void save_ram(_TCHAR* filename);
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
