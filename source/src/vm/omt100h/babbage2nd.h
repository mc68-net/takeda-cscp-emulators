/*
	Gijutsu-Hyoron-Sha Babbage-2nd Emulator 'eBabbage-2nd'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.12.26 -

	[ virtual machine ]
*/

#ifndef _BABBAGE_2ND_H_
#define _BABBAGE_2ND_H_

// device informations for win32
#define DEVICE_NAME		"Gijutsu-Hyoron-Sha Babbage-2nd"
#define CONFIG_NAME		"babbage2nd"
#define CONFIG_VERSION		0x01

#define WINDOW_WIDTH1		640
#define WINDOW_HEIGHT1		483
#define WINDOW_WIDTH2		640
#define WINDOW_HEIGHT2		483

#define USE_RAM
#define NOTIFY_KEY_DOWN

// device informations for virtual machine
#define FRAMES_PER_10SECS	300
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME 	256
#define CHARS_PER_LINE		1
#define CPU_CLOCKS		2500000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		483

#include "../../common.h"

#define USE_BITMAP
#define USE_BUTTON
#define MAX_BUTTONS		21
#define USE_LED
#define MAX_LEDS		14

const struct {
	const _TCHAR* caption;
	int x, y;
	int width, height;
	int code;
} buttons[] = {
	{_T("0"),   353, 419, 49, 49, 0x30},
	{_T("1"),   407, 419, 49, 49, 0x31},
	{_T("2"),   461, 419, 49, 49, 0x32},
	{_T("3"),   515, 419, 49, 49, 0x33},
	{_T("4"),   353, 353, 49, 49, 0x34},
	{_T("5"),   407, 353, 49, 49, 0x35},
	{_T("6"),   461, 353, 49, 49, 0x36},
	{_T("7"),   515, 353, 49, 49, 0x37},
	{_T("8"),   353, 287, 49, 49, 0x38},
	{_T("9"),   407, 287, 49, 49, 0x39},
	{_T("A"),   461, 287, 49, 49, 0x41},
	{_T("B"),   515, 287, 49, 49, 0x42},
	{_T("C"),   353, 221, 49, 49, 0x43},
	{_T("D"),   407, 221, 49, 49, 0x44},
	{_T("E"),   461, 221, 49, 49, 0x45},
	{_T("F"),   515, 221, 49, 49, 0x46},
	{_T("GO"),  575, 221, 49, 49, 0x70},
	{_T("AD"),  575, 287, 49, 49, 0x71},
	{_T("DA"),  575, 353, 49, 49, 0x72},
	{_T("INC"), 575, 419, 49, 49, 0x73},
	{_T("RES"),  36,  18, 49, 49, 0x74}
};
const struct {
	int x, y;
	int width, height;
} leds[] = {
	{587,  37, 34, 58},
	{530,  37, 34, 58},
	{455,  37, 34, 58},
	{398,  37, 34, 58},
	{341,  37, 34, 58},
	{284,  37, 34, 58},
	{600, 133, 17, 17},
	{567, 133, 17, 17},
	{534, 133, 17, 17},
	{501, 133, 17, 17},
	{468, 133, 17, 17},
	{435, 133, 17, 17},
	{402, 133, 17, 17},
	{369, 133, 17, 17},
};

class EMU;
class DEVICE;
class EVENT;

class IO;
class Z80;
class Z80CTC;
class Z80PIO;

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
	Z80* cpu;
	Z80CTC* ctc;
	Z80PIO* pio1;
	Z80PIO* pio2;
	
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
	
	// notify key
	void key_down(int code);
	void key_up(int code);
	
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