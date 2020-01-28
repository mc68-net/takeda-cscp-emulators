/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "config.h"

config_t config;

#ifndef CONFIG_NAME
#define CONFIG_NAME "conf"
#endif

BOOL MyWritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
	return MyWritePrivateProfileString(lpAppName, lpKeyName, create_string(_T("%d"), Value), lpFileName);
}

BOOL MyWritePrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool Value, LPCTSTR lpFileName)
{
	return MyWritePrivateProfileString(lpAppName, lpKeyName, create_string(_T("%d"), Value ? 1 : 0), lpFileName);
}

bool MyGetPrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool bDefault, LPCTSTR lpFileName)
{
	return (MyGetPrivateProfileInt(lpAppName, lpKeyName, bDefault ? 1 : 0, lpFileName) != 0);
}

void initialize_config()
{
	// initial settings
	memset(&config, 0, sizeof(config_t));
	
	// control
#if defined(USE_BOOT_MODE) && defined(BOOT_MODE_DEFAULT)
	config.boot_mode = BOOT_MODE_DEFAULT;
#endif
#if defined(USE_CPU_TYPE) && defined(CPU_TYPE_DEFAULT)
	config.cpu_type = CPU_TYPE_DEFAULT;
#endif
#if defined(USE_DIPSWITCH) && defined(DIPSWITCH_DEFAULT)
	config.dipswitch = DIPSWITCH_DEFAULT;
#endif
#if defined(USE_DEVICE_TYPE) && defined(DEVICE_TYPE_DEFAULT)
	config.device_type = DEVICE_TYPE_DEFAULT;
#endif
#if defined(USE_DRIVE_TYPE) && defined(DRIVE_TYPE_DEFAULT)
	config.drive_type = DRIVE_TYPE_DEFAULT;
#endif
#if defined(USE_FD1)
	for(int drv = 0; drv < MAX_FD; drv++) {
#if defined(CORRECT_DISK_TIMING_DEFAULT)
		config.correct_disk_timing[drv] = CORRECT_DISK_TIMING_DEFAULT;
#else
		config.correct_disk_timing[drv] = true;
#endif
#if defined(IGNORE_DISK_CRC_DEFAULT)
		config.ignore_disk_crc[drv] = IGNORE_DISK_CRC_DEFAULT;
#endif
	}
#endif
#if defined(USE_TAPE)
	config.wave_shaper = true;
	config.direct_load_mzt = true;
	config.baud_high = true;
#endif
	
	// screen
#ifndef ONE_BOARD_MICRO_COMPUTER
#ifdef _WIN32
	config.use_d3d9 = true;
#endif
	config.fullscreen_stretch_type = 1;	// Stretch (Aspect)
#endif
	
	// sound
#if defined(SOUND_RATE_DEFAULT)
	config.sound_frequency = SOUND_RATE_DEFAULT;
#else
	config.sound_frequency = 6;	// 48KHz
#endif
	config.sound_latency = 1;	// 100msec
#if defined(USE_SOUND_DEVICE_TYPE) && defined(SOUND_DEVICE_TYPE_DEFAULT)
	config.sound_device_type = SOUND_DEVICE_TYPE_DEFAULT;
#endif
	
	// input
#ifdef _WIN32
	config.use_direct_input = true;
	config.disable_dwm = false;
#endif
#ifdef USE_JOYSTICK
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			config.joy_buttons[i][j] = (i << 4) | j;
		}
	}
#endif
	
	// printer
#if defined(USE_PRINTER) && defined(PRINTER_DEVICE_TYPE_DEFAULT)
	config.printer_device_type = PRINTER_DEVICE_TYPE_DEFAULT;
#endif
}

void load_config(const _TCHAR* config_path)
{
	// initial settings
	initialize_config();
	
	// control
#ifdef USE_BOOT_MODE
	config.boot_mode = MyGetPrivateProfileInt(_T("Control"), _T("BootMode"), config.boot_mode, config_path);
#endif
#ifdef USE_CPU_TYPE
	config.cpu_type = MyGetPrivateProfileInt(_T("Control"), _T("CPUType"), config.cpu_type, config_path);
#endif
#ifdef USE_DIPSWITCH
	config.dipswitch = MyGetPrivateProfileInt(_T("Control"), _T("DipSwitch"), config.dipswitch, config_path);
#endif
#ifdef USE_DEVICE_TYPE
	config.device_type = MyGetPrivateProfileInt(_T("Control"), _T("DeviceType"), config.device_type, config_path);
#endif
#ifdef USE_DRIVE_TYPE
	config.drive_type = MyGetPrivateProfileInt(_T("Control"), _T("DriveType"), config.drive_type, config_path);
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		config.correct_disk_timing[drv] = MyGetPrivateProfileBool(_T("Control"), create_string(_T("CorrectDiskTiming%d"), drv + 1), config.correct_disk_timing[drv], config_path);
		config.ignore_disk_crc[drv] = MyGetPrivateProfileBool(_T("Control"), create_string(_T("IgnoreDiskCRC%d"), drv + 1), config.ignore_disk_crc[drv], config_path);
	}
#endif
#ifdef USE_TAPE
	config.tape_sound = MyGetPrivateProfileBool(_T("Control"), _T("TapeSound"), config.tape_sound, config_path);
	config.wave_shaper = MyGetPrivateProfileBool(_T("Control"), _T("WaveShaper"), config.wave_shaper, config_path);
	config.direct_load_mzt = MyGetPrivateProfileBool(_T("Control"), _T("DirectLoadMZT"), config.direct_load_mzt, config_path);
	config.baud_high = MyGetPrivateProfileBool(_T("Control"), _T("BaudHigh"), config.baud_high, config_path);
#endif
	
	// recent files
#ifdef USE_CART1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), _T(""), config.initial_cart_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_CART; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentCartPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_cart_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_FD1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), _T(""), config.initial_floppy_disk_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_FD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentDiskPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_floppy_disk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_QD1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), _T(""), config.initial_quick_disk_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_QD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_quick_disk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_TAPE
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), _T(""), config.initial_tape_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentTapePath1_%d"), i + 1), _T(""), config.recent_tape_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_COMPACT_DISC
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialCompactDiscDir"), _T(""), config.initial_compact_disc_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentCompactDiscPath1_%d"), i + 1), _T(""), config.recent_compact_disc_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_LASER_DISC
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), _T(""), config.initial_laser_disc_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentLaserDiscPath1_%d"), i + 1), _T(""), config.recent_laser_disc_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), _T(""), config.initial_binary_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_BINARY; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentBinaryPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_binary_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
	
	// screen
#ifndef ONE_BOARD_MICRO_COMPUTER
	config.window_mode = MyGetPrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
#ifdef _WIN32
	config.use_d3d9 = MyGetPrivateProfileBool(_T("Screen"), _T("UseD3D9"), config.use_d3d9, config_path);
	config.wait_vsync = MyGetPrivateProfileBool(_T("Screen"), _T("WaitVSync"), config.wait_vsync, config_path);
#endif
	config.window_stretch_type = MyGetPrivateProfileInt(_T("Screen"), _T("WindowStretchType"), config.window_stretch_type, config_path);
	config.fullscreen_stretch_type = MyGetPrivateProfileInt(_T("Screen"), _T("FullScreenStretchType"), config.fullscreen_stretch_type, config_path);
#endif
#ifdef USE_MONITOR_TYPE
	config.monitor_type = MyGetPrivateProfileInt(_T("Screen"), _T("MonitorType"), config.monitor_type, config_path);
#endif
#ifdef USE_CRT_FILTER
	config.crt_filter = MyGetPrivateProfileBool(_T("Screen"), _T("CRTFilter"), config.crt_filter, config_path);
#endif
#ifdef USE_SCANLINE
	config.scan_line = MyGetPrivateProfileBool(_T("Screen"), _T("ScanLine"), config.scan_line, config_path);
#endif
#ifdef USE_SCREEN_ROTATE
	config.rotate_type = MyGetPrivateProfileInt(_T("Screen"), _T("RotateType"), config.rotate_type, config_path);
#endif
#if defined(_USE_QT)
	config.use_opengl_scanline = MyGetPrivateProfileBool(_T("Screen"), _T("UseOpenGLScanLine"), config.use_opengl_scanline, config_path);
	config.opengl_scanline_vert = MyGetPrivateProfileBool(_T("Screen"), _T("OpenGLScanLineVert"), config.opengl_scanline_vert, config_path);;
	config.opengl_scanline_horiz = MyGetPrivateProfileBool(_T("Screen"), _T("OpenGLScanLineHoriz"), config.opengl_scanline_horiz, config_path);;
	config.use_opengl_filters = MyGetPrivateProfileBool(_T("Screen"), _T("UseOpenGLFilters"), config.use_opengl_filters, config_path);
	config.opengl_filter_num = MyGetPrivateProfileInt(_T("Screen"), _T("OpenGLFilterNum"), config.opengl_filter_num, config_path);
#endif
	
	// sound
	config.sound_frequency = MyGetPrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	config.sound_latency = MyGetPrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	config.sound_device_type = MyGetPrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
#ifdef USE_SOUND_VOLUME
	for(int i = 0; i < USE_SOUND_VOLUME; i++) {
		int tmp_l = MyGetPrivateProfileInt(_T("Sound"), create_string(_T("VolumeLeft%d"), i + 1), config.sound_volume_l[i], config_path);
		int tmp_r = MyGetPrivateProfileInt(_T("Sound"), create_string(_T("VolumeRight%d"), i + 1), config.sound_volume_r[i], config_path);
		config.sound_volume_l[i] = max(-40, min(0, tmp_l));
		config.sound_volume_r[i] = max(-40, min(0, tmp_r));
	}
#endif
#ifdef _WIN32
	MyGetPrivateProfileString(_T("Sound"), _T("FMGenDll"), _T("mamefm.dll"), config.fmgen_dll_path, _MAX_PATH, config_path);
#endif
	
	// input
#ifdef _WIN32
	config.use_direct_input = MyGetPrivateProfileBool(_T("Input"), _T("UseDirectInput"), config.use_direct_input, config_path);
	config.disable_dwm = MyGetPrivateProfileBool(_T("Input"), _T("DisableDwm"), config.disable_dwm, config_path);
#endif
#ifdef USE_JOYSTICK
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			config.joy_buttons[i][j] = MyGetPrivateProfileInt(_T("Input"), create_string(_T("JoyButtons%d_%d"), i + 1, j + 1), config.joy_buttons[i][j], config_path);
		}
	}
#endif
	
	// printer
#ifdef USE_PRINTER
	config.printer_device_type = MyGetPrivateProfileInt(_T("Printer"), _T("DeviceType"), config.printer_device_type, config_path);
	MyGetPrivateProfileString(_T("Printer"), _T("PrinterDll"), _T("printer.dll"), config.printer_dll_path, _MAX_PATH, config_path);
#endif
}

void save_config(const _TCHAR* config_path)
{
	// control
#ifdef USE_BOOT_MODE
	MyWritePrivateProfileInt(_T("Control"), _T("BootMode"), config.boot_mode, config_path);
#endif
#ifdef USE_CPU_TYPE
	MyWritePrivateProfileInt(_T("Control"), _T("CPUType"), config.cpu_type, config_path);
#endif
#ifdef USE_DIPSWITCH
	MyWritePrivateProfileInt(_T("Control"), _T("DipSwitch"), config.dipswitch, config_path);
#endif
#ifdef USE_DEVICE_TYPE
	MyWritePrivateProfileInt(_T("Control"), _T("DeviceType"), config.device_type, config_path);
#endif
#ifdef USE_DRIVE_TYPE
	MyWritePrivateProfileInt(_T("Control"), _T("DriveType"), config.drive_type, config_path);
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		MyWritePrivateProfileBool(_T("Control"), create_string(_T("CorrectDiskTiming%d"), drv + 1), config.correct_disk_timing[drv], config_path);
		MyWritePrivateProfileBool(_T("Control"), create_string(_T("IgnoreDiskCRC%d"), drv + 1), config.ignore_disk_crc[drv], config_path);
	}
#endif
#ifdef USE_TAPE
	MyWritePrivateProfileBool(_T("Control"), _T("TapeSound"), config.tape_sound, config_path);
	MyWritePrivateProfileBool(_T("Control"), _T("WaveShaper"), config.wave_shaper, config_path);
	MyWritePrivateProfileBool(_T("Control"), _T("DirectLoadMZT"), config.direct_load_mzt, config_path);
	MyWritePrivateProfileBool(_T("Control"), _T("BaudHigh"), config.baud_high, config_path);
#endif
	
	// recent files
#ifdef USE_CART1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), config.initial_cart_dir, config_path);
	for(int drv = 0; drv < MAX_CART; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentCartPath%d_%d"), drv + 1, i + 1), config.recent_cart_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_FD1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), config.initial_floppy_disk_dir, config_path);
	for(int drv = 0; drv < MAX_FD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentDiskPath%d_%d"), drv + 1, i + 1), config.recent_floppy_disk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_QD1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), config.initial_quick_disk_dir, config_path);
	for(int drv = 0; drv < MAX_QD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1), config.recent_quick_disk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_TAPE
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), config.initial_tape_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentTapePath1_%d"), i + 1), config.recent_tape_path[i], config_path);
	}
#endif
#ifdef USE_COMPACT_DISC
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialCompactDiscDir"), config.initial_compact_disc_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentCompactDiscPath1_%d"), i + 1), config.recent_compact_disc_path[i], config_path);
	}
#endif
#ifdef USE_LASER_DISC
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), config.initial_laser_disc_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentLaserDiscPath1_%d"), i + 1), config.recent_laser_disc_path[i], config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), config.initial_binary_dir, config_path);
	for(int drv = 0; drv < MAX_BINARY; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentBinaryPath%d_%d"), drv + 1, i + 1), config.recent_binary_path[drv][i], config_path);
		}
	}
#endif
	
	// screen
#ifndef ONE_BOARD_MICRO_COMPUTER
	MyWritePrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
#ifdef _WIN32
	MyWritePrivateProfileBool(_T("Screen"), _T("UseD3D9"), config.use_d3d9, config_path);
	MyWritePrivateProfileBool(_T("Screen"), _T("WaitVSync"), config.wait_vsync, config_path);
#endif
	MyWritePrivateProfileInt(_T("Screen"), _T("WindowStretchType"), config.window_stretch_type, config_path);
	MyWritePrivateProfileInt(_T("Screen"), _T("FullScreenStretchType"), config.fullscreen_stretch_type, config_path);
#endif
#ifdef USE_MONITOR_TYPE
	MyWritePrivateProfileInt(_T("Screen"), _T("MonitorType"), config.monitor_type, config_path);
#endif
#ifdef USE_CRT_FILTER
	MyWritePrivateProfileBool(_T("Screen"), _T("CRTFilter"), config.crt_filter, config_path);
#endif
#ifdef USE_SCANLINE
	MyWritePrivateProfileBool(_T("Screen"), _T("ScanLine"), config.scan_line, config_path);
#endif
#ifdef USE_SCREEN_ROTATE
	MyWritePrivateProfileInt(_T("Screen"), _T("RotateType"), config.rotate_type, config_path);
#endif
#if defined(_USE_QT)
	MyWritePrivateProfileBool(_T("Screen"), _T("UseOpenGLScanLine"), config.use_opengl_scanline, config_path);
	MyWritePrivateProfileBool(_T("Screen"), _T("OpenGLScanLineVert"), config.opengl_scanline_vert, config_path);;
	MyWritePrivateProfileBool(_T("Screen"), _T("OpenGLScanLineHoriz"), config.opengl_scanline_horiz, config_path);;
	MyWritePrivateProfileBool(_T("Screen"), _T("UseOpenGLFilters"), config.use_opengl_filters, config_path);
	MyWritePrivateProfileInt(_T("Screen"), _T("OpenGLFilterNum"), config.opengl_filter_num, config_path);
#endif
	
	// sound
	MyWritePrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	MyWritePrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	MyWritePrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
#ifdef USE_SOUND_VOLUME
	for(int i = 0; i < USE_SOUND_VOLUME; i++) {
		MyWritePrivateProfileInt(_T("Sound"), create_string(_T("VolumeLeft%d"), i + 1), config.sound_volume_l[i], config_path);
		MyWritePrivateProfileInt(_T("Sound"), create_string(_T("VolumeRight%d"), i + 1), config.sound_volume_r[i], config_path);
	}
#endif
	
	// input
#ifdef _WIN32
	MyWritePrivateProfileBool(_T("Input"), _T("UseDirectInput"), config.use_direct_input, config_path);
	MyWritePrivateProfileBool(_T("Input"), _T("DisableDwm"), config.disable_dwm, config_path);
#endif
#ifdef USE_JOYSTICK
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			MyWritePrivateProfileInt(_T("Input"), create_string(_T("JoyButtons%d_%d"), i + 1, j + 1), config.joy_buttons[i][j], config_path);
		}
	}
#endif
	
	// printer
#ifdef USE_PRINTER
	MyWritePrivateProfileInt(_T("Printer"), _T("DeviceType"), config.printer_device_type, config_path);
#endif
}

#define STATE_VERSION	4

void save_config_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	state_fio->FputUint32(STATE_VERSION);
	
#ifdef USE_BOOT_MODE
	state_fio->FputInt32(config.boot_mode);
#endif
#ifdef USE_CPU_TYPE
	state_fio->FputInt32(config.cpu_type);
#endif
#ifdef USE_DIPSWITCH
	state_fio->FputUint32(config.dipswitch);
#endif
#ifdef USE_DEVICE_TYPE
	state_fio->FputInt32(config.device_type);
#endif
#ifdef USE_DRIVE_TYPE
	state_fio->FputInt32(config.drive_type);
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		state_fio->FputBool(config.correct_disk_timing[drv]);
		state_fio->FputBool(config.ignore_disk_crc[drv]);
	}
#endif
#ifdef USE_MONITOR_TYPE
	state_fio->FputInt32(config.monitor_type);
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	state_fio->FputInt32(config.sound_device_type);
#endif
#ifdef USE_PRINTER
	state_fio->FputInt32(config.printer_device_type);
#endif
}

bool load_config_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
#ifdef USE_BOOT_MODE
	config.boot_mode = state_fio->FgetInt32();
#endif
#ifdef USE_CPU_TYPE
	config.cpu_type = state_fio->FgetInt32();
#endif
#ifdef USE_DIPSWITCH
	config.dipswitch = state_fio->FgetUint32();
#endif
#ifdef USE_DEVICE_TYPE
	config.device_type = state_fio->FgetInt32();
#endif
#ifdef USE_DRIVE_TYPE
	config.drive_type = state_fio->FgetInt32();
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		config.correct_disk_timing[drv] = state_fio->FgetBool();
		config.ignore_disk_crc[drv] = state_fio->FgetBool();
	}
#endif
#ifdef USE_MONITOR_TYPE
	config.monitor_type = state_fio->FgetInt32();
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	config.sound_device_type = state_fio->FgetInt32();
#endif
#ifdef USE_PRINTER
	config.printer_device_type = state_fio->FgetInt32();
#endif
	return true;
}
