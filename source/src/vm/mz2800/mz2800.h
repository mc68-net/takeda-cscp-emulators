/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ virtual machine ]
*/

#ifndef _MZ2800_H_
#define _MZ2800_H_

#define DEVICE_NAME		"SHARP MZ-2800"
#define CONFIG_NAME		"mz2800"

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		108
#define CPU_CLOCKS		8000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define HAS_I286
#define I8259_MAX_CHIPS		2
#define SINGLE_MODE_DMA
#define HAS_RP5C15
#define IO_ADDR_MAX		0x8000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_ACCESS_LAMP
#define USE_SOUND_VOLUME	3
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("OPN (FM)"), _T("OPN (PSG)"), _T("Beep"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class I8253;
class I8255;
class I8259;
class I286;
class IO;
class MB8877;
class NOT;
class PCM1BIT;
class RP5C01;
//class SASI;
class UPD71071;
class YM2203;
class Z80PIO;
class Z80SIO;

class CRTC;
class FLOPPY;
class JOYSTICK;
class KEYBOARD;
class MEMORY;
class MOUSE;
class PRINTER;
class RESET;
class SERIAL;
class SYSPORT;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8253* pit;
	I8255* pio0;
	I8259* pic;
	I286* cpu;
	IO* io;
	MB8877* fdc;
	NOT* not_busy;
	PCM1BIT* pcm;
	RP5C01* rtc;
//	SASI* sasi;
	UPD71071* dma;
	YM2203* opn;
	Z80PIO* pio1;
	Z80SIO* sio;
	
	CRTC* crtc;
	FLOPPY* floppy;
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MEMORY* memory;
	MOUSE* mouse;
	PRINTER* printer;
	RESET* rst;
	SERIAL* serial;
	SYSPORT* sysport;
	
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
	void cpu_reset();
	void run();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	int access_lamp();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// user interface
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void set_disk_protected(int drv, bool value);
	bool get_disk_protected(int drv);
	bool now_skip();
	
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
