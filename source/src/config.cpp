/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "fileio.h"

config_t config;

BOOL MyWritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
	_TCHAR String[32];
	my_stprintf_s(String, 32, _T("%d"), Value);
	return MyWritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}

BOOL MyWritePrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool Value, LPCTSTR lpFileName)
{
	_TCHAR String[32];
	my_stprintf_s(String, 32, _T("%d"), Value ? 1 : 0);
	return MyWritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}

bool MyGetPrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool bDefault, LPCTSTR lpFileName)
{
	return (MyGetPrivateProfileInt(lpAppName, lpKeyName, bDefault ? 1 : 0, lpFileName) != 0);
}

void init_config()
{
	// initial settings
	memset(&config, 0, sizeof(config_t));
	
#ifdef _WIN32
	config.use_direct_input = true;
	config.disable_dwm = false;
#endif
	config.swap_joy_buttons = false;
	
#ifndef ONE_BOARD_MICRO_COMPUTER
#ifdef _WIN32
	config.use_d3d9 = true;
#endif
	config.stretch_type = 1;	// Stretch (Aspect)
#endif
	config.sound_frequency = 6;	// 48KHz
	config.sound_latency = 1;	// 100msec
	
#if defined(USE_TAPE)
	config.wave_shaper = true;
	config.direct_load_mzt = true;
	config.baud_high = true;
#endif
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
#if defined(USE_SOUND_DEVICE_TYPE) && defined(SOUND_DEVICE_TYPE_DEFAULT)
	config.sound_device_type = SOUND_DEVICE_TYPE_DEFAULT;
#endif
}

void load_config(const _TCHAR* config_path)
{
	// initial settings
	init_config();
	
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
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("CorrectDiskTiming%d"), drv + 1);
		config.correct_disk_timing[drv] = MyGetPrivateProfileBool(_T("Control"), name, config.correct_disk_timing[drv], config_path);
		my_stprintf_s(name, 64, _T("IgnoreDiskCRC%d"), drv + 1);
		config.ignore_disk_crc[drv] = MyGetPrivateProfileBool(_T("Control"), name, config.ignore_disk_crc[drv], config_path);
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
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentCartPath%d_%d"), drv + 1, i + 1);
			MyGetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_cart_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_FD1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), _T(""), config.initial_disk_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_FD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentDiskPath%d_%d"), drv + 1, i + 1);
			MyGetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_disk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_QD1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), _T(""), config.initial_quickdisk_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_QD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1);
			MyGetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_quickdisk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_TAPE
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), _T(""), config.initial_tape_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("RecentTapePath1_%d"), i + 1);
		MyGetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_tape_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_LASER_DISC
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), _T(""), config.initial_laser_disc_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("RecentLaserDiscPath1_%d"), i + 1);
		MyGetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_laser_disc_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), _T(""), config.initial_binary_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_BINARY; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentBinaryPath%d_%d"), drv + 1, i + 1);
			MyGetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_binary_path[drv][i], _MAX_PATH, config_path);
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
	config.stretch_type = MyGetPrivateProfileInt(_T("Screen"), _T("StretchType"), config.stretch_type, config_path);
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
	
	// sound
	config.sound_frequency = MyGetPrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	config.sound_latency = MyGetPrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	config.sound_device_type = MyGetPrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
	MyGetPrivateProfileString(_T("Sound"), _T("FMGenDll"), _T("mamefm.dll"), config.fmgen_dll_path, _MAX_PATH, config_path);
	
	// input
#ifdef _WIN32
	config.use_direct_input = MyGetPrivateProfileBool(_T("Input"), _T("UseDirectInput"), config.use_direct_input, config_path);
	config.disable_dwm = MyGetPrivateProfileBool(_T("Input"), _T("DisableDwm"), config.disable_dwm, config_path);
#endif
	config.swap_joy_buttons = MyGetPrivateProfileBool(_T("Input"), _T("SwapJoyButtons"), config.swap_joy_buttons, config_path);
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
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("CorrectDiskTiming%d"), drv + 1);
		MyWritePrivateProfileBool(_T("Control"), name, config.correct_disk_timing[drv], config_path);
		my_stprintf_s(name, 64, _T("IgnoreDiskCRC%d"), drv + 1);
		MyWritePrivateProfileBool(_T("Control"), name, config.ignore_disk_crc[drv], config_path);
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
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentCartPath%d_%d"), drv + 1, i + 1);
			MyWritePrivateProfileString(_T("RecentFiles"), name, config.recent_cart_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_FD1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), config.initial_disk_dir, config_path);
	for(int drv = 0; drv < MAX_FD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentDiskPath%d_%d"), drv + 1, i + 1);
			MyWritePrivateProfileString(_T("RecentFiles"), name, config.recent_disk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_QD1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), config.initial_quickdisk_dir, config_path);
	for(int drv = 0; drv < MAX_QD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1);
			MyWritePrivateProfileString(_T("RecentFiles"), name, config.recent_quickdisk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_TAPE
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), config.initial_tape_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("RecentTapePath1_%d"), i + 1);
		MyWritePrivateProfileString(_T("RecentFiles"), name, config.recent_tape_path[i], config_path);
	}
#endif
#ifdef USE_LASER_DISC
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), config.initial_laser_disc_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("RecentLaserDiscPath1_%d"), i + 1);
		MyWritePrivateProfileString(_T("RecentFiles"), name, config.recent_laser_disc_path[i], config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), config.initial_binary_dir, config_path);
	for(int drv = 0; drv < MAX_BINARY; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentBinaryPath%d_%d"), drv + 1, i + 1);
			MyWritePrivateProfileString(_T("RecentFiles"), name, config.recent_binary_path[drv][i], config_path);
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
	MyWritePrivateProfileInt(_T("Screen"), _T("StretchType"), config.stretch_type, config_path);
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
	
	// sound
	MyWritePrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	MyWritePrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	MyWritePrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
	
	// input
#ifdef _WIN32
	MyWritePrivateProfileBool(_T("Input"), _T("UseDirectInput"), config.use_direct_input, config_path);
	MyWritePrivateProfileBool(_T("Input"), _T("DisableDwm"), config.disable_dwm, config_path);
#endif
	MyWritePrivateProfileBool(_T("Input"), _T("SwapJoyButtons"), config.swap_joy_buttons, config_path);
}

#define STATE_VERSION	3

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
	return true;
}

