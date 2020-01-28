/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ virtual machine ]
*/

#ifndef _MZ700_H_
#define _MZ700_H_

#if defined(_MZ700)
#define DEVICE_NAME		"SHARP MZ-700"
#define CONFIG_NAME		"mz700"
#elif defined(_MZ800)
#define DEVICE_NAME		"SHARP MZ-800"
#define CONFIG_NAME		"mz800"
#elif defined(_MZ1500)
#define DEVICE_NAME		"SHARP MZ-1500"
#define CONFIG_NAME		"mz1500"
#endif

// device informations for virtual machine
#if defined(_MZ800)
#define FRAMES_PER_SEC		50
#define LINES_PER_FRAME		312
//#define CPU_CLOCKS		3546900
// 228*312*50
#define CPU_CLOCKS		3556800
#else
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
//#define CPU_CLOCKS		3579545
// 228*262*60
#define CPU_CLOCKS		3584160
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define IO_ADDR_MAX		0x100
#define Z80_MEMORY_WAIT
#if defined(_MZ800) || defined(_MZ1500)
#define MAX_DRIVE		4
#define HAS_MB8876
#endif
#if defined(_MZ1500)
#define MZ1P17_SW1_4_ON
#endif

// device informations for win32
#if defined(_MZ700)
#define USE_DIPSWITCH
#elif defined(_MZ800)
#define USE_BOOT_MODE		2
#endif
#define USE_TAPE
#define USE_TAPE_BUTTON
#if defined(_MZ800) || defined(_MZ1500)
#define USE_QD1
#define USE_FD1
#define USE_FD2
#endif
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#if defined(_MZ800)
#define USE_MONITOR_TYPE	2
#endif
#define USE_CRT_FILTER
#define USE_SCANLINE
//#if defined(_MZ1500)
#define USE_SCREEN_ROTATE
//#endif
#if defined(_MZ800) || defined(_MZ1500)
#define USE_ACCESS_LAMP
#endif
#if defined(_MZ700)
#define USE_SOUND_VOLUME	2
#elif defined(_MZ800)
#define USE_SOUND_VOLUME	3
#elif defined(_MZ1500)
#define USE_SOUND_VOLUME	4
#endif
#if defined(_MZ1500)
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#endif
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
#if defined(_MZ800)
	_T("PSG"),
#elif defined(_MZ1500)
	_T("PSG #1"), _T("PSG #2"),
#endif
	_T("Beep"), _T("CMT"),
};
#endif

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
class EMM;
class KANJI;
class KEYBOARD;
class MEMORY;
class RAMFILE;

#if defined(_MZ800) || defined(_MZ1500)
class MB8877;
class NOT;
class SN76489AN;
class Z80PIO;
class Z80SIO;
class FLOPPY;
#if defined(_MZ1500)
class PSG;
#endif
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
	EMM* emm;
	KANJI* kanji;
	KEYBOARD* keyboard;
	MEMORY* memory;
	RAMFILE* ramfile;
	
#if defined(_MZ800) || defined(_MZ1500)
	AND* and_snd;
	MB8877* fdc;
#if defined(_MZ800)
	NOT* not_pit;
	SN76489AN* psg;
#elif defined(_MZ1500)
	DEVICE* printer;
	NOT* not_reset;
	NOT* not_strobe;
	SN76489AN* psg_l;
	SN76489AN* psg_r;
#endif
	Z80PIO* pio_int;
	Z80SIO* sio_rs;	// RS-232C
	Z80SIO* sio_qd;	// QD
	
	FLOPPY* floppy;
#if defined(_MZ1500)
	PSG* psg;
#endif
	QUICKDISK* qd;
#endif
	
#if defined(_MZ800)
	int boot_mode;
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
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
#if defined(_MZ800) || defined(_MZ1500)
	uint32_t get_access_lamp_status();
#endif
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// user interface
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
#if defined(_MZ800) || defined(_MZ1500)
	void open_quick_disk(int drv, const _TCHAR* file_path);
	void close_quick_disk(int drv);
	bool is_quick_disk_inserted(int drv);
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
#endif
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