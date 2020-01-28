/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ virtual machine ]
*/

#ifndef _PHC25_H_
#define _PHC25_H_

#ifdef _MAP1010
#define DEVICE_NAME		"SEIKO MAP-1010"
#define CONFIG_NAME		"map1010"
#else
#define DEVICE_NAME		"SANYO PHC-25"
#define CONFIG_NAME		"phc25"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192

#define MC6847_ATTR_OFS		0x800
#define MC6847_ATTR_INV		0x01
#define MC6847_ATTR_AS		0x02
#define MC6847_ATTR_CSS		0x04
#define HAS_AY_3_8910

// device informations for win32
#define USE_TAPE
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_CAPS
#define USE_SOUND_VOLUME	2
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("CMT"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class IO;
class MC6847;
class NOT;
class YM2203;
class Z80;

class JOYSTICK;
class KEYBOARD;
class MEMORY;
class SYSTEM;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	IO* io;
	MC6847* vdp;
	NOT* not_vsync;
	YM2203* psg;
	Z80* cpu;
	
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MEMORY* memory;
	SYSTEM* system;
	
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