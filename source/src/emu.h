/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#ifndef _EMU_H_
#define _EMU_H_

// DirectX
#define DIRECTDRAW_VERSION 0x300
#define DIRECTSOUND_VERSION 0x500
#define DIRECT3D_VERSION 0x500

// for debug
//#define _DEBUG_LOG

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include "common.h"
#include "vm/vm.h"

#ifdef _USE_GAPI
#include <gx.h>
#endif
#ifndef _USE_WAVEOUT
#include <dsound.h>
#endif

#ifndef _WIN32_WCE
#ifdef USE_SOCKET
#include <winsock.h>
#endif
#ifdef USE_CAPTURE
#include <dshow.h>
#include <qedit.h>
#endif
#include <vfw.h>
#endif

#ifdef USE_MEDIA
#define MEDIA_MAX 64
#endif
#ifdef USE_SOCKET
#define WM_SOCKET0 (WM_USER + 1)
#define WM_SOCKET1 (WM_USER + 2)
#define WM_SOCKET2 (WM_USER + 3)
#define WM_SOCKET3 (WM_USER + 4)
#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000
#endif
#ifdef USE_CAPTURE
#define CAPTURE_MAX 8
#endif

class FILEIO;

static int autokey_table[256] = {
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x00d,0x000,0x000,0x00d,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x020,0x131,0x132,0x133,0x134,0x135,0x136,0x137,0x138,0x139,0x1ba,0x1bb,0x0bc,0x0bd,0x0be,0x0bf,
	0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,0x038,0x039,0x0ba,0x0bb,0x1bc,0x1bd,0x1be,0x1bf,
	0x0c0,0x441,0x442,0x443,0x444,0x445,0x446,0x447,0x448,0x449,0x44a,0x44b,0x44c,0x44d,0x44e,0x44f,
	0x450,0x451,0x452,0x453,0x454,0x455,0x456,0x457,0x458,0x459,0x45a,0x0db,0x0dc,0x0dd,0x0de,0x1e2,
	0x1c0,0x841,0x842,0x843,0x844,0x845,0x846,0x847,0x848,0x849,0x84a,0x84b,0x84c,0x84d,0x84e,0x84f,
	0x850,0x851,0x852,0x853,0x854,0x855,0x856,0x857,0x858,0x859,0x85a,0x1db,0x1dc,0x1dd,0x1de,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
#if 1
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
#else
	0x000,0x3be,0x3db,0x3dd,0x3bc,0x3bf,0x330,0x333,0x345,0x334,0x335,0x336,0x337,0x338,0x339,0x35a,
	0x2dc,0x233,0x245,0x234,0x235,0x236,0x254,0x247,0x248,0x2ba,0x242,0x258,0x244,0x252,0x250,0x243,
	0x251,0x241,0x25a,0x257,0x253,0x255,0x249,0x231,0x2bc,0x24b,0x246,0x256,0x232,0x2de,0x2bd,0x24a,
	0x24e,0x2dd,0x2bf,0x24d,0x237,0x238,0x239,0x24f,0x24c,0x2be,0x2bb,0x2e2,0x230,0x259,0x2c0,0x2db,
#endif
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

class EMU
{
protected:
	VM* vm;
private:
	// ----------------------------------------
	// debug log
	// ----------------------------------------
#ifdef _DEBUG_LOG
	FILE* debug;
#endif
	void open_debug();
	void close_debug();
	
	// ----------------------------------------
	// window
	// ----------------------------------------
	HWND main_window_handle;
	int window_width, window_height;
	int power_x, power_y;
	int dest_x, dest_y;
	_TCHAR app_path[_MAX_PATH];
	
	// ----------------------------------------
	// input
	// ----------------------------------------
	void initialize_input();
	void release_input();
	void update_input();
	
	uint8 key_status[256];	// windows key code mapping
	uint8 joy_status[2];	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4-b7 = trigger #1-#4
	int mouse_status[3];	// x, y, button (b0 = left, b1 = right)
	bool mouse_enable;
	
#ifndef _WIN32_WCE
	int joy_num;
	uint32 joy_xmin[2], joy_xmax[2];
	uint32 joy_ymin[2], joy_ymax[2];
	JOYCAPS joycaps[2];
#endif
#ifdef USE_AUTO_KEY
	int cb_phase, cb_size, cb_ptr, cb_code;
	bool cb_shift;
	char* clipboard;
#endif
	
	// ----------------------------------------
	// screen
	// ----------------------------------------
	void initialize_screen();
	void release_screen();
	
	HDC hdcDIB;
	HBITMAP hBMP;
	LPBYTE lpBuf;
	LPWORD lpBMP;
	LPBITMAPINFO lpDIB;
#ifdef _USE_GAPI
	LPWORD lpGapi;
#endif
	
	// record video
#ifndef _WIN32_WCE
	PAVIFILE pAVIFile;
	PAVISTREAM pAVIStream;
	PAVISTREAM pAVICompressed;
	AVICOMPRESSOPTIONS opts;
#endif
	bool now_recv;
	int rec_frames, rec_fps;
	
	// ----------------------------------------
	// sound
	// ----------------------------------------
	void initialize_sound(int rate, int samples);
	void release_sound();
	void update_sound();
	
	int sound_rate, sound_samples;
	bool sound_ok, now_mute;
	
#ifdef _USE_WAVEOUT
	// waveout api
	HWAVEOUT hwo;
	WAVEHDR wavehdr[4];
	int play_block;
#else
	// direct sound
	LPDIRECTSOUND lpds;
	LPDIRECTSOUNDBUFFER lpdsb, lpdsp;
	bool first_half;
#endif
	
	// record sound
	typedef struct {
		DWORD dwRIFF;
		DWORD dwFileSize;
		DWORD dwWAVE;
		DWORD dwfmt_;
		DWORD dwFormatSize;
		WORD wFormatTag;
		WORD wChannels;
		DWORD dwSamplesPerSec;
		DWORD dwAvgBytesPerSec;
		WORD wBlockAlign;
		WORD wBitsPerSample;
		DWORD dwdata;
		DWORD dwDataLength;
	} wavheader_t;
	FILEIO* rec;
	int rec_bufs;
	bool now_recs;
	
	// ----------------------------------------
	// media
	// ----------------------------------------
#ifdef USE_MEDIA
#ifdef _WIN32_WCE
	void initialize_media() {}
	void release_media() {}
#else
	void initialize_media();
	void release_media();
	
	_TCHAR media_path[MEDIA_MAX][_MAX_PATH];
	int media_cnt;
#endif
#endif
	
	// ----------------------------------------
	// timer
	// ----------------------------------------
	void update_timer();
	SYSTEMTIME sTime;
	
	// ----------------------------------------
	// socket
	// ----------------------------------------
#ifdef USE_SOCKET
#ifdef _WIN32_WCE
	void initialize_socket() {}
	void release_socket() {}
	void update_socket() {}
#else
	void initialize_socket();
	void release_socket();
	void update_socket();
	
	int soc[SOCKET_MAX];
	bool is_tcp[SOCKET_MAX];
	struct sockaddr_in udpaddr[SOCKET_MAX];
	int socket_delay[SOCKET_MAX];
	char recv_buffer[SOCKET_MAX][SOCKET_BUFFER_MAX];
	int recv_r_ptr[SOCKET_MAX], recv_w_ptr[SOCKET_MAX];
#endif
#endif
	
	// ----------------------------------------
	// capture
	// ----------------------------------------
#ifdef USE_CAPTURE
#ifdef _WIN32_WCE
	void initialize_capture() {}
	void release_capture() {}
#else
	void initialize_capture();
	void release_capture();
	IPin* get_pin(IBaseFilter *pFilter, PIN_DIRECTION PinDir);
	LPSTR MyAtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars);
	
	int capture_devs;
	int capture_connected;
	_TCHAR capture_dev_name[CAPTURE_MAX][_MAX_PATH];
	
	IGraphBuilder *pGraph;
	IBaseFilter *pSrc;
	ICaptureGraphBuilder2 *pBuilder;
	IMediaControl *pMediaControl;
	IBaseFilter *pF;
	ISampleGrabber *pSGrab;
	
	int capture_width;
	int capture_height;
	int capture_src_y;
	int capture_src_height;
	long capture_bufsize;
	HDC hdcCapDIB;
	HBITMAP hCapBMP;
	LPBYTE lpCapBuf;
	LPWORD lpCapBMP;
	LPBITMAPINFO lpCapDIB;
#endif
#endif
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	EMU(HWND hwnd);
	~EMU();
	
	void application_path(_TCHAR* path);
	
	// ----------------------------------------
	// for windows
	// ----------------------------------------
	
	// drive virtual machine
	void run();
	void reset();
#ifdef USE_IPL_RESET
	void ipl_reset();
#endif
	
	// user interface
#ifdef USE_CART
	void open_cart(_TCHAR* filename);
	void close_cart();
#endif
#ifdef USE_FD1
	void open_disk(_TCHAR* filename, int drv);
	void close_disk(int drv);
#endif
#ifdef USE_DATAREC
	void play_datarec(_TCHAR* filename);
	void rec_datarec(_TCHAR* filename);
	void close_datarec();
#endif
#ifdef USE_MEDIA
#ifdef _WIN32_WCE
	void open_media(_TCHAR* filename) {}
	void close_media() {}
#else
	void open_media(_TCHAR* filename);
	void close_media();
#endif
#endif
	bool now_skip();
	
	void start_rec_sound();
	void stop_rec_sound();
	void restart_rec_sound();
	bool now_rec_sound() { return now_recs; }
	
	void start_rec_video(int fps, bool show_dialog);
	void stop_rec_video();
	void restart_rec_video();
	bool now_rec_video() { return now_recv; }
	
	void update_config();
	
	// input device
	void key_down(int code);
	void key_up(int code);
	
	void enable_mouse();
	void disenable_mouse();
	void toggle_mouse();
	
#ifdef USE_AUTO_KEY
	void start_auto_key();
	void stop_auto_key();
	bool now_auto_key() { return cb_phase ? true : false; }
#endif
	
	// screen
	void set_screen_size(int width, int height);
	void draw_screen();
	void update_screen(HDC hdc);
	
	// sound
#ifdef _USE_WAVEOUT
	void notify_sound();
#endif
	void mute_sound();
	
	// socket
#ifdef USE_SOCKET
#ifdef _WIN32_WCE
	int get_socket(int ch) { return -1; }
	void socket_connected(int ch) {}
	void socket_disconnected(int ch) {}
	void send_data(int ch) {}
	void recv_data(int ch) {}
#else
	int get_socket(int ch) { return soc[ch]; }
	void socket_connected(int ch);
	void socket_disconnected(int ch);
	void send_data(int ch);
	void recv_data(int ch);
#endif
#endif
	// capture
#ifdef USE_CAPTURE
#ifdef _WIN32_WCE
	int get_capture_devices() { return 0; }
	_TCHAR* get_capture_device_name(int index) { return NULL; }
	int get_connected_capture_device() { return -1; }
	void connect_capture_device(int index, bool pin) {}
	void disconnect_capture_device() {}
	void show_capture_device_filter() {}
	void show_capture_device_pin() {}
	void show_capture_device_source() {}
#else
	int get_capture_devices() { return capture_devs; }
	_TCHAR* get_capture_device_name(int index) { return capture_dev_name[index]; }
	int get_connected_capture_device() { return capture_connected; }
	void connect_capture_device(int index, bool pin);
	void disconnect_capture_device();
	void show_capture_device_filter();
	void show_capture_device_pin();
	void show_capture_device_source();
#endif
#endif
	
	// ----------------------------------------
	// for virtual machine
	// ----------------------------------------
	
	// get input device status
	uint8* key_buffer() { return key_status; }
	uint8* joy_buffer() { return joy_status; }
	int* mouse_buffer() { return mouse_status; }
	
	// get screen buffer
	uint16* screen_buffer(int y);
	
	// get timer
	void get_timer(int time[]);
	
	// play media
#ifdef USE_MEDIA
#ifdef _WIN32_WCE
	int media_count() { return 0; }
	void play_media(int trk) {}
	void stop_media() {}
#else
	int media_count();
	void play_media(int trk);
	void stop_media();
#endif
#endif
	// socket
#ifdef USE_SOCKET
#ifdef _WIN32_WCE
	bool init_socket_tcp(int ch) { return false; }
	bool init_socket_udp(int ch) { return false; }
	bool connect_socket(int ch, uint32 ipaddr, int port) { return false; }
	void disconnect_socket(int ch) {}
	bool listen_socket(int ch) { return false; }
	void send_data_tcp(int ch) {}
	void send_data_udp(int ch, uint32 ipaddr, int port) {}
#else
	bool init_socket_tcp(int ch);
	bool init_socket_udp(int ch);
	bool connect_socket(int ch, uint32 ipaddr, int port);
	void disconnect_socket(int ch);
	bool listen_socket(int ch);
	void send_data_tcp(int ch);
	void send_data_udp(int ch, uint32 ipaddr, int port);
#endif
#endif
	// capture
#ifdef USE_CAPTURE
#ifdef _WIN32_WCE
	bool get_capture_device_buffer() { return false; }
	void set_capture_device_channel(int ch) {}
#else
	bool get_capture_device_buffer();
	void set_capture_device_channel(int ch);
#endif
#endif
	// debug log
	void out_debug(const _TCHAR* format, ...);
};

#endif