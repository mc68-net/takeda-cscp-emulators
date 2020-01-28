/*
	EPSON HC-20 Emulator 'eHC-20'

	Author : Takeda.Toshiya
	Date   : 2011.05.23-

	[ virtual machine ]
*/

#ifndef _HC20_H_
#define _HC20_H_

#define DEVICE_NAME		"EPSON HC-20"
#define CONFIG_NAME		"hc20"

// device informations for virtual machine
#define FRAMES_PER_SEC		72
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		614400
#define SCREEN_WIDTH		120
#define SCREEN_HEIGHT		32
#define MAX_DRIVE		2
#define HAS_HD6301
#define HAS_UPD7201

// device informations for win32
#define WINDOW_MODE_BASE	3
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0x0f
#define USE_FD1
#define USE_FD2
#define USE_TAPE
#define TAPE_BINARY_ONLY
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	12
#define USE_AUTO_KEY_CAPS
#define DONT_KEEEP_KEY_PRESSED
#define USE_NOTIFY_POWER_OFF
#define USE_ACCESS_LAMP
#define USE_SOUND_VOLUME	1
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class HD146818P;
class I8255;
class MC6800;
class TF20;
class UPD765A;
class Z80;
class Z80SIO;

class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	HD146818P* rtc;
	MC6800* cpu;
	
	TF20* tf20;
	I8255* pio_tf20;
	UPD765A* fdc_tf20;
	Z80* cpu_tf20;
	Z80SIO* sio_tf20;
	
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
	void notify_power_off();
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
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
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