/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.14-

	[ virtual machine ]
*/

#ifndef _MZ80B_H_
#define _MZ80B_H_

#if defined(_MZ80B)
#define DEVICE_NAME		"SHARP MZ-80B"
#define CONFIG_NAME		"mz80b"
#elif defined(_MZ2000)
#define DEVICE_NAME		"SHARP MZ-2000"
#define CONFIG_NAME		"mz2000"
#else
#define DEVICE_NAME		"SHARP MZ-2200"
#define CONFIG_NAME		"mz2200"
#endif

#ifndef _MZ80B
#define SUPPORT_QUICK_DISK
#define SUPPORT_16BIT_BOARD
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define HAS_MB8876
#ifdef SUPPORT_QUICK_DISK
#endif
#ifdef SUPPORT_16BIT_BOARD
#define HAS_I88
#define I8259_MAX_CHIPS		1
#endif
#define PRINTER_STROBE_RISING_EDGE

// memory wait
#define Z80_MEMORY_WAIT
#define Z80_IO_WAIT

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#ifdef SUPPORT_QUICK_DISK
#define USE_QD1
#endif
#define USE_TAPE
#define USE_TAPE_BUTTON
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#ifndef _MZ80B
#define USE_MONITOR_TYPE	4
#define USE_CRT_FILTER
#endif
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_SOUND_VOLUME	2
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8253;
class I8255;
class IO;
class MB8877;
class PCM1BIT;
class Z80;
class Z80PIO;

class CMT;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class MZ1R12;
class MZ1R13;
class PRINTER;
class TIMER;

#ifdef SUPPORT_QUICK_DISK
class Z80SIO;
class QUICKDISK;
#endif

#ifdef SUPPORT_16BIT_BOARD
class I286;
class I8259;
class MZ1M01;
#endif

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8253* pit;
	I8255* pio_i;
	IO* io;
	MB8877* fdc;
	PCM1BIT* pcm;
	Z80* cpu;
	Z80PIO* pio;
	
	CMT* cmt;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	MZ1R12* mz1r12;
	MZ1R13* mz1r13;
	PRINTER* printer;
	TIMER* timer;
	
#ifdef SUPPORT_QUICK_DISK
	Z80SIO* sio;
	QUICKDISK* qd;
#endif
	
#ifdef SUPPORT_16BIT_BOARD
	Z80PIO* pio_to16;
	I286* cpu_16;
	I8259* pic_16;
	MZ1M01* mz1m01;
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
	void special_reset();
	void run();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	uint32_t get_access_lamp_status();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
#ifdef SUPPORT_QUICK_DISK
	void open_quick_disk(int drv, const _TCHAR* file_path);
	void close_quick_disk(int drv);
	bool is_quick_disk_inserted(int drv);
#endif
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
	bool is_tape_playing();
	bool is_tape_recording();
	int get_tape_position();
	void push_play();
	void push_stop();
	void push_fast_forward();
	void push_fast_rewind();
	void push_apss_forward() {}
	void push_apss_rewind() {}
	bool is_frame_skippable();
	
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif