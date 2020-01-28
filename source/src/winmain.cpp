/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 main ]
*/

#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <commdlg.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <stdio.h>
#include "res/resource.h"
#include "config.h"
#include "emu.h"

#ifdef _WIN32_WCE
#define timeGetTime() GetTickCount()
#endif

// config
extern config_t config;

// emulation core
EMU* emu;

// command bar
#ifdef _WIN32_WCE
HINSTANCE hInst;
HWND hCmdBar;
BOOL commandbar_show, sip_on;
#endif

// uif
void update_menu(HMENU hMenu, int pos);
#ifdef USE_CART
void open_cart(HWND hWnd);
#endif
#ifdef USE_FD1
void open_disk(HWND hWnd, int drv);
#endif
#ifdef USE_DATAREC
void open_datarec(HWND hWnd, BOOL play);
#endif
#ifdef USE_MEDIA
void open_media(HWND hWnd);
#endif
void set_window(HWND hwnd, int mode);
BOOL fullscreen_now = FALSE;

// windows main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

// timing control
int intervals[FRAMES_PER_10SECS];
#ifdef _WIN32_WCE
#define MIN_SKIP_FRAMES 1
#define MAX_SKIP_FRAMES 30
#else
#define MIN_SKIP_FRAMES 0
#define MAX_SKIP_FRAMES 10
#endif
DWORD rec_next_time, rec_accum_time;
int rec_delay[3];

int get_interval()
{
	static int cnt = 0;
	int interval = 0;
	
	for(int i = 0; i < 10; i++) {
		interval += intervals[cnt];
		cnt = (cnt + 1 < FRAMES_PER_10SECS) ? cnt + 1 : 0;
	}
	return interval;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
{
	// load config
	load_config();
	
	// create window
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
#ifdef _WIN32_WCE
	wndclass.lpszMenuName = 0;
#else
	wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
#endif
	wndclass.lpszClassName = _T("CWINDOW");
	RegisterClass(&wndclass);
	
#ifdef _WIN32_WCE
	// show window
	hInst = hInstance;
	HWND hWnd = CreateWindow(_T("CWINDOW"), _T(DEVICE_NAME), WS_VISIBLE,
	                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	
	// show menu bar
	if(hCmdBar)
		CommandBar_Show(hCmdBar, TRUE);
	commandbar_show = TRUE;
#else
	// get window position
	RECT rect = {0, 0, WINDOW_WIDTH1, WINDOW_HEIGHT1};
	AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_VISIBLE, TRUE, 0);
	HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	int width = GetDeviceCaps(hdc, HORZRES);
	int height = GetDeviceCaps(hdc, VERTRES);
	int dest_x = (int)((width - (rect.right - rect.left)) / 2);
	int dest_y = (int)((height - (rect.bottom - rect.top)) / 2);
	dest_x = (dest_x < 0) ? 0 : dest_x;
	dest_y = (dest_y < 0) ? 0 : dest_y;
	
	// show window
	HWND hWnd = CreateWindow(_T("CWINDOW"), _T(DEVICE_NAME), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
	                         dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	
	if(config.window_mode == 1)
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_WINDOW2, 0L);
	else if(config.window_mode == 2)
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_640X480, 0L);
	else if(config.window_mode == 3)
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_320X240, 0L);
	else
		config.window_mode = 0;
	
	// accelerator
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
#endif
	
	// disenable ime
	ImmAssociateContext(hWnd, 0);
	
#ifdef _USE_GAPI
	// initialize gapi
	GXOpenDisplay(hWnd, GX_FULLSCREEN);
#endif
	
	// initialize emulation core
	emu = new EMU(hWnd);
	emu->set_screen_size(WINDOW_WIDTH1, WINDOW_HEIGHT1);
	
	// timing control
	int remain = 1000;
	
	for(int i = 0; i < FRAMES_PER_10SECS; i++) {
		intervals[i] = (int)(1000 / FRAMES_PER_10SECS);
		remain -= intervals[i];
	}
	for(int i = 0; i < remain; i++)
		intervals[(int)(FRAMES_PER_10SECS * i / remain)]++;
	
	// main loop
	int current_interval = get_interval(), next_interval;
	int skip_frames = 0, rec_cnt = 0, fps = 0, total = 0;
	DWORD next_time = timeGetTime();
	DWORD fps_time = next_time + 1000;
	MSG msg;
	
	while(1) {
		// check window message
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if(!GetMessage(&msg, NULL, 0, 0)) {
#ifndef _WIN32_WCE
				ExitProcess(0);	// trick
#endif
				return msg.wParam;
			}
#ifdef _WIN32_WCE
			TranslateMessage(&msg);
			DispatchMessage(&msg);
#else
			if(!TranslateAccelerator(hWnd, hAccel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
#endif
		}
		else if(emu) {
			// get next period
			next_time += emu->now_skip() ? 0 : current_interval;
			rec_next_time += current_interval;
			next_interval = get_interval();
			
			// drive machine
			emu->run();
			total++;
			
			if(emu->now_rec_video()) {
				while(rec_next_time >= rec_accum_time) {
					// rec pictures 10/15/30 frames per 1 second
					emu->draw_screen();
					fps++;
					rec_accum_time += rec_delay[rec_cnt];
					rec_cnt = (rec_cnt == 2) ? 0 : rec_cnt + 1;
				}
				
				DWORD tmp = timeGetTime();
				if(next_time > tmp) {
					skip_frames = 0;
					
					// sleep 1 frame priod if need
					if((int)(next_time - tmp) >= next_interval)
						Sleep(next_interval);
				}
				else if(++skip_frames > MAX_SKIP_FRAMES) {
					skip_frames = 0;
					next_time = tmp;
				}
			}
			else {
				if(next_time > timeGetTime()) {
					if(skip_frames >= MIN_SKIP_FRAMES) {
						// update window if enough time
						emu->draw_screen();
						skip_frames = 0;
						fps++;
					}
					else
						skip_frames++;
					
					// sleep 1 frame priod if need
					if((int)(next_time - timeGetTime()) >= next_interval)
						Sleep(next_interval);
				}
				else if(++skip_frames > MAX_SKIP_FRAMES) {
					// update window at least once per 10 frames
					emu->draw_screen();
					skip_frames = 0;
					fps++;
					next_time = timeGetTime();
				}
			}
			current_interval = next_interval;
			Sleep(0);
			
			// calc frame rate
			if(fps_time <= timeGetTime()) {
				_TCHAR buf[32];
				int ratio = (int)(100 * fps / total + 0.5);
				_stprintf(buf, _T("%s - %d fps (%d %%)"), _T(DEVICE_NAME), fps, ratio);
				SetWindowText(hWnd, buf);
				fps_time += 1000;
				fps = total = 0;
			}
		}
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef _USE_GAPI
	HDC hdc;
	PAINTSTRUCT ps;
#endif
	_TCHAR path[_MAX_PATH];
	int no;
	
	switch(iMsg)
	{
#ifdef _WIN32_WCE
		case WM_CREATE:
			hCmdBar = CommandBar_Create(hInst, hWnd, 1);
			CommandBar_InsertMenubar(hCmdBar, hInst, IDR_MENU1, 0);
			CommandBar_AddAdornments(hCmdBar, 0, 0);
			break;
#endif
		case WM_CLOSE:
			// release emulation core
			if(emu)
				delete emu;
			emu = NULL;
			save_config();
#ifdef _WIN32_WCE
			// destroy command bar
			if(hCmdBar)
				CommandBar_Destroy(hCmdBar);
			hCmdBar = NULL;
#else
			// quit fullscreen mode
			if(fullscreen_now)
				ChangeDisplaySettings(NULL, 0);
			fullscreen_now = FALSE;
#endif
#ifdef _USE_GAPI
			// release gapi
			GXCloseDisplay();
#endif
			DestroyWindow(hWnd);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
#ifdef _USE_GAPI
		case WM_KILLFOCUS:
			GXSuspend();
			break;
		case WM_SETFOCUS:
			GXResume();
			break;
#else
		case WM_PAINT:
			if(emu) {
				hdc = BeginPaint(hWnd, &ps);
				emu->update_screen(hdc);
				EndPaint(hWnd, &ps);
			}
			return 0;
#endif
#ifdef _USE_WAVEOUT
		case MM_WOM_DONE:
			if(emu)
				emu->notify_sound();
			break;
#endif
#ifndef _WIN32_WCE
		case WM_MOVING:
			if(emu)
				emu->mute_sound();
			break;
#endif
#ifdef _WIN32_WCE
//		case WM_LBUTTONDOWN:
//			if(hCmdBar)
//				CommandBar_Show(hCmdBar, (commandbar_show = !commandbar_show));
//			break;
#endif
		case WM_KEYDOWN:
			if(emu)
				emu->key_down(LOBYTE(wParam));
			break;
		case WM_KEYUP:
			if(emu)
				emu->key_up(LOBYTE(wParam));
			break;
		case WM_SYSKEYDOWN:
			if(emu)
				emu->key_down(LOBYTE(wParam));
#ifdef USE_ALT_F10_KEY
			return 0;	// not activate menu when hit ALT/F10
#endif
			break;
		case WM_SYSKEYUP:
			if(emu)
				emu->key_up(LOBYTE(wParam));
#ifdef USE_ALT_F10_KEY
			return 0;	// not activate menu when hit ALT/F10
#endif
			break;
		case WM_INITMENUPOPUP:
			if(emu)
				emu->mute_sound();
			update_menu((HMENU)wParam, LOWORD(lParam));
			break;
#ifdef USE_SOCKET
		case WM_SOCKET0: no = 0; goto socket;
		case WM_SOCKET1: no = 1; goto socket;
		case WM_SOCKET2: no = 2; goto socket;
		case WM_SOCKET3: no = 3;
socket:
			if(!emu)
				break;
			if(WSAGETSELECTERROR(lParam) != 0) {
				emu->disconnect_socket(no);
				emu->socket_disconnected(no);
				break;
			}
			if(emu->get_socket(no) != (int)wParam)
				break;
			switch(WSAGETSELECTEVENT(lParam))
			{
				case FD_CONNECT:
					emu->socket_connected(no);
					break;
				case FD_CLOSE:
					emu->socket_disconnected(no);
					break;
				case FD_WRITE:
					emu->send_data(no);
					break;
				case FD_READ:
					emu->recv_data(no);
					break;
			}
			break;
#endif
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_RESET:
					if(emu)
						emu->reset();
					rec_next_time = rec_accum_time = 0;
					break;
#ifdef USE_IPL_RESET
				case ID_IPL_RESET:
					if(emu)
						emu->ipl_reset();
					rec_next_time = rec_accum_time = 0;
					break;
#endif
#ifdef USE_DIPSWITCH
				case ID_DIPSWITCH1: config.dipswitch ^= 0x01; break;
				case ID_DIPSWITCH2: config.dipswitch ^= 0x02; break;
				case ID_DIPSWITCH3: config.dipswitch ^= 0x04; break;
				case ID_DIPSWITCH4: config.dipswitch ^= 0x08; break;
				case ID_DIPSWITCH5: config.dipswitch ^= 0x10; break;
				case ID_DIPSWITCH6: config.dipswitch ^= 0x20; break;
				case ID_DIPSWITCH7: config.dipswitch ^= 0x40; break;
				case ID_DIPSWITCH8: config.dipswitch ^= 0x80; break;
#endif
				case ID_CPU_POWER0: no = 0; goto cpu_power;
				case ID_CPU_POWER1: no = 1; goto cpu_power;
				case ID_CPU_POWER2: no = 2; goto cpu_power;
				case ID_CPU_POWER3: no = 3; goto cpu_power;
				case ID_CPU_POWER4: no = 4;
cpu_power:
					config.cpu_power = no;
					if(emu)
						emu->update_config();
					break;
#ifdef USE_AUTO_KEY
				case ID_AUTOKEY_START:
					if(emu)
						emu->start_auto_key();
					break;
				case ID_AUTOKEY_STOP:
					if(emu)
						emu->stop_auto_key();
					break;
#endif
				case ID_EXIT:
					SendMessage(hWnd, WM_CLOSE, 0, 0L);
					break;
#ifdef USE_CART
				case ID_OPEN_CART:
					if(emu)
						open_cart(hWnd);
					break;
				case ID_CLOSE_CART:
					if(emu)
						emu->close_cart();
					break;
				case ID_RECENT_CART1: no = 0; goto recent_cart;
				case ID_RECENT_CART2: no = 1; goto recent_cart;
				case ID_RECENT_CART3: no = 2; goto recent_cart;
				case ID_RECENT_CART4: no = 3; goto recent_cart;
				case ID_RECENT_CART5: no = 4; goto recent_cart;
				case ID_RECENT_CART6: no = 5; goto recent_cart;
				case ID_RECENT_CART7: no = 6; goto recent_cart;
				case ID_RECENT_CART8: no = 7;
recent_cart:
					_tcscpy(path, config.recent_cart[no]);
					for(int i = no; i > 0; i--)
						_tcscpy(config.recent_cart[i], config.recent_cart[i - 1]);
					_tcscpy(config.recent_cart[0], path);
					if(emu)
						emu->open_cart(path);
					break;
#endif
#ifdef USE_FD1
				case ID_OPEN_FD1:
					if(emu)
						open_disk(hWnd, 0);
					break;
				case ID_CLOSE_FD1:
					if(emu)
						emu->close_disk(0);
					break;
				case ID_RECENT_FD11: no = 0; goto recent_disk1;
				case ID_RECENT_FD12: no = 1; goto recent_disk1;
				case ID_RECENT_FD13: no = 2; goto recent_disk1;
				case ID_RECENT_FD14: no = 3; goto recent_disk1;
				case ID_RECENT_FD15: no = 4; goto recent_disk1;
				case ID_RECENT_FD16: no = 5; goto recent_disk1;
				case ID_RECENT_FD17: no = 6; goto recent_disk1;
				case ID_RECENT_FD18: no = 7;
recent_disk1:
					_tcscpy(path, config.recent_disk[0][no]);
					for(int i = no; i > 0; i--)
						_tcscpy(config.recent_disk[0][i], config.recent_disk[0][i - 1]);
					_tcscpy(config.recent_disk[0][0], path);
					if(emu)
						emu->open_disk(path, 0);
					break;
#endif
#ifdef USE_FD2
				case ID_OPEN_FD2:
					if(emu)
						open_disk(hWnd, 1);
					break;
				case ID_CLOSE_FD2:
					if(emu)
						emu->close_disk(1);
					break;
				case ID_RECENT_FD21: no = 0; goto recent_disk2;
				case ID_RECENT_FD22: no = 1; goto recent_disk2;
				case ID_RECENT_FD23: no = 2; goto recent_disk2;
				case ID_RECENT_FD24: no = 3; goto recent_disk2;
				case ID_RECENT_FD25: no = 4; goto recent_disk2;
				case ID_RECENT_FD26: no = 5; goto recent_disk2;
				case ID_RECENT_FD27: no = 6; goto recent_disk2;
				case ID_RECENT_FD28: no = 7;
recent_disk2:
					_tcscpy(path, config.recent_disk[1][no]);
					for(int i = no; i > 0; i--)
						_tcscpy(config.recent_disk[1][i], config.recent_disk[1][i - 1]);
					_tcscpy(config.recent_disk[1][0], path);
					if(emu)
						emu->open_disk(path, 1);
					break;
#endif
#ifdef USE_FD3
				case ID_OPEN_FD3:
					if(emu)
						open_disk(hWnd, 2);
					break;
				case ID_CLOSE_FD3:
					if(emu)
						emu->close_disk(2);
					break;
				case ID_RECENT_FD31: no = 0; goto recent_disk3;
				case ID_RECENT_FD32: no = 1; goto recent_disk3;
				case ID_RECENT_FD33: no = 2; goto recent_disk3;
				case ID_RECENT_FD34: no = 3; goto recent_disk3;
				case ID_RECENT_FD35: no = 4; goto recent_disk3;
				case ID_RECENT_FD36: no = 5; goto recent_disk3;
				case ID_RECENT_FD37: no = 6; goto recent_disk3;
				case ID_RECENT_FD38: no = 7;
recent_disk3:
					_tcscpy(path, config.recent_disk[2][no]);
					for(int i = no; i > 0; i--)
						_tcscpy(config.recent_disk[2][i], config.recent_disk[2][i - 1]);
					_tcscpy(config.recent_disk[2][0], path);
					if(emu)
						emu->open_disk(path, 2);
					break;
#endif
#ifdef USE_FD4
				case ID_OPEN_FD4:
					if(emu)
						open_disk(hWnd, 3);
					break;
				case ID_CLOSE_FD4:
					if(emu)
						emu->close_disk(3);
					break;
				case ID_RECENT_FD41: no = 0; goto recent_disk4;
				case ID_RECENT_FD42: no = 1; goto recent_disk4;
				case ID_RECENT_FD43: no = 2; goto recent_disk4;
				case ID_RECENT_FD44: no = 3; goto recent_disk4;
				case ID_RECENT_FD45: no = 4; goto recent_disk4;
				case ID_RECENT_FD46: no = 5; goto recent_disk4;
				case ID_RECENT_FD47: no = 6; goto recent_disk4;
				case ID_RECENT_FD48: no = 7;
recent_disk4:
					_tcscpy(path, config.recent_disk[3][no]);
					for(int i = no; i > 0; i--)
						_tcscpy(config.recent_disk[3][i], config.recent_disk[3][i - 1]);
					_tcscpy(config.recent_disk[3][0], path);
					if(emu)
						emu->open_disk(path, 3);
					break;
#endif
#ifdef USE_DATAREC
				case ID_PLAY_DATAREC:
					if(emu)
						open_datarec(hWnd, TRUE);
					break;
				case ID_REC_DATAREC:
					if(emu)
						open_datarec(hWnd, FALSE);
					break;
				case ID_CLOSE_DATAREC:
					if(emu)
						emu->close_datarec();
					break;
				case ID_RECENT_DATAREC1: no = 0; goto recent_datarec;
				case ID_RECENT_DATAREC2: no = 1; goto recent_datarec;
				case ID_RECENT_DATAREC3: no = 2; goto recent_datarec;
				case ID_RECENT_DATAREC4: no = 3; goto recent_datarec;
				case ID_RECENT_DATAREC5: no = 4; goto recent_datarec;
				case ID_RECENT_DATAREC6: no = 5; goto recent_datarec;
				case ID_RECENT_DATAREC7: no = 6; goto recent_datarec;
				case ID_RECENT_DATAREC8: no = 7;
recent_datarec:
					_tcscpy(path, config.recent_datarec[no]);
					for(int i = no; i > 0; i--)
						_tcscpy(config.recent_datarec[i], config.recent_datarec[i - 1]);
					_tcscpy(config.recent_datarec[0], path);
					if(emu)
						emu->play_datarec(path);
					break;
#endif
#ifdef USE_MEDIA
				case ID_OPEN_MEDIA:
					if(emu)
						open_media(hWnd);
					break;
				case ID_CLOSE_MEDIA:
					if(emu)
						emu->close_media();
					break;
				case ID_RECENT_MEDIA1: no = 0; goto recent_media;
				case ID_RECENT_MEDIA2: no = 1; goto recent_media;
				case ID_RECENT_MEDIA3: no = 2; goto recent_media;
				case ID_RECENT_MEDIA4: no = 3; goto recent_media;
				case ID_RECENT_MEDIA5: no = 4; goto recent_media;
				case ID_RECENT_MEDIA6: no = 5; goto recent_media;
				case ID_RECENT_MEDIA7: no = 6; goto recent_media;
				case ID_RECENT_MEDIA8: no = 7;
recent_media:
					_tcscpy(path, config.recent_media[no]);
					for(int i = no; i > 0; i--)
						_tcscpy(config.recent_media[i], config.recent_media[i - 1]);
					_tcscpy(config.recent_media[0], path);
					if(emu)
						emu->open_media(path);
					break;
#endif
				case ID_SCREEN_REC30: no = 30; goto record_video;
				case ID_SCREEN_REC15: no = 15; goto record_video;
				case ID_SCREEN_REC10: no = 10;
record_video:
					if(emu) {
						emu->start_rec_video(no, true);
						emu->start_rec_sound();
						rec_delay[0] = (no == 30) ? 33 : (no == 15) ? 66 : 100;
						rec_delay[1] = (no == 30) ? 33 : (no == 15) ? 67 : 100;
						rec_delay[2] = (no == 30) ? 34 : (no == 15) ? 67 : 100;
						rec_next_time = rec_accum_time = 0;
					}
					break;
				case ID_SCREEN_STOP:
					if(emu) {
						emu->stop_rec_video();
						emu->stop_rec_sound();
					}
					break;
				case ID_SCREEN_WINDOW1:
					if(emu)
						set_window(hWnd, 0);
					break;
				case ID_SCREEN_WINDOW2:
					if(emu)
						set_window(hWnd, 1);
					break;
				case ID_SCREEN_640X480:
					if(emu && !fullscreen_now)
						set_window(hWnd, 2);
					break;
				case ID_SCREEN_320X240:
					if(emu && !fullscreen_now)
						set_window(hWnd, 3);
					break;
#ifndef _WIN32_WCE
				// accelerator
				case ID_ACCEL_SCREEN:
					if(emu) {
						emu->mute_sound();
						set_window(hWnd, fullscreen_now ? 0 : 2);
					}
					break;
				case ID_ACCEL_MOUSE:
					if(emu)
						emu->toggle_mouse();
					break;
#endif
#ifdef USE_MONITOR_TYPE
				case ID_SCREEN_A400L: no = 0; goto monitor_type;
				case ID_SCREEN_D400L: no = 1; goto monitor_type;
				case ID_SCREEN_A200L: no = 2; goto monitor_type;
				case ID_SCREEN_D200L: no = 3;
monitor_type:
					config.monitor_type = no;
					if(emu)
						emu->update_config();
					break;
#endif
#ifdef USE_SCANLINE
				case ID_SCREEN_SCANLINE:
					config.scan_line = !config.scan_line;
					if(emu)
						emu->update_config();
					break;
#endif
				case ID_SOUND_REC:
					if(emu)
						emu->start_rec_sound();
					break;
				case ID_SOUND_STOP:
					if(emu)
						emu->stop_rec_sound();
					break;
				case ID_SOUND_FREQ0: no = 0; goto sound_frequency;
				case ID_SOUND_FREQ1: no = 1; goto sound_frequency;
				case ID_SOUND_FREQ2: no = 2; goto sound_frequency;
				case ID_SOUND_FREQ3: no = 3; goto sound_frequency;
				case ID_SOUND_FREQ4: no = 4; goto sound_frequency;
				case ID_SOUND_FREQ5: no = 5; goto sound_frequency;
				case ID_SOUND_FREQ6: no = 6;
sound_frequency:
					config.sound_frequency = no;
					if(emu)
						emu->update_config();
					break;
				case ID_SOUND_LATE0: no = 0; goto sound_latency;
				case ID_SOUND_LATE1: no = 1; goto sound_latency;
				case ID_SOUND_LATE2: no = 2; goto sound_latency;
				case ID_SOUND_LATE3: no = 3;
sound_latency:
					config.sound_latency = no;
					if(emu)
						emu->update_config();
					break;
#ifdef USE_CAPTURE
				case ID_CAPTURE_FILTER:
					if(emu)
						emu->show_capture_device_filter();
					break;
				case ID_CAPTURE_PIN:
					if(emu)
						emu->show_capture_device_pin();
					break;
				case ID_CAPTURE_SOURCE:
					if(emu)
						emu->show_capture_device_source();
					break;
				case ID_CAPTURE_DISCONNECT:
					if(emu)
						emu->disconnect_capture_device();
					break;
				case ID_CAPTURE_DEVICE1: no = 0; goto capture_device;
				case ID_CAPTURE_DEVICE2: no = 1; goto capture_device;
				case ID_CAPTURE_DEVICE3: no = 2; goto capture_device;
				case ID_CAPTURE_DEVICE4: no = 3; goto capture_device;
				case ID_CAPTURE_DEVICE5: no = 4; goto capture_device;
				case ID_CAPTURE_DEVICE6: no = 5; goto capture_device;
				case ID_CAPTURE_DEVICE7: no = 6; goto capture_device;
				case ID_CAPTURE_DEVICE8: no = 7;
capture_device:
					if(emu)
						emu->connect_capture_device(no, false);
					break;
#endif
			}
			break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam) ;
}

void update_menu(HMENU hMenu, int pos)
{
#ifdef MENU_POS_CONTROL
	if(pos == MENU_POS_CONTROL) {
		// control menu
#ifdef USE_DIPSWITCH
		CheckMenuItem(hMenu, ID_DIPSWITCH1, !(config.dipswitch & 0x01) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIPSWITCH2, !(config.dipswitch & 0x02) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIPSWITCH3, !(config.dipswitch & 0x04) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIPSWITCH4, !(config.dipswitch & 0x08) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIPSWITCH5, !(config.dipswitch & 0x10) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIPSWITCH6, !(config.dipswitch & 0x20) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIPSWITCH7, !(config.dipswitch & 0x40) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIPSWITCH8, !(config.dipswitch & 0x80) ? MF_CHECKED : MF_UNCHECKED);
#endif
		if(config.cpu_power == 0)
			CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER0, MF_BYCOMMAND);
		else if(config.cpu_power == 1)
			CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER1, MF_BYCOMMAND);
		else if(config.cpu_power == 2)
			CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER2, MF_BYCOMMAND);
		else if(config.cpu_power == 3)
			CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER3, MF_BYCOMMAND);
		else
			CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER4, MF_BYCOMMAND);
#ifdef USE_AUTO_KEY
		// auto key
		bool now_paste = true, now_stop = true;
#ifndef _WIN32_WCE
		if(emu) {
			now_paste = emu->now_auto_key();
			now_stop = !now_paste;
		}
#endif
		EnableMenuItem(hMenu, ID_AUTOKEY_START, now_paste ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_AUTOKEY_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
#endif
	}
#endif
#ifdef MENU_POS_CART
	if(pos == MENU_POS_CART) {
		// cartridge
		UINT uIDs[8] = {
			ID_RECENT_CART1, ID_RECENT_CART2, ID_RECENT_CART3, ID_RECENT_CART4,
			ID_RECENT_CART5, ID_RECENT_CART6, ID_RECENT_CART7, ID_RECENT_CART8
		};
		bool flag = false;
		
		for(int i = 0; i < 8; i++)
			DeleteMenu(hMenu, uIDs[i], MF_BYCOMMAND);
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_cart[i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, uIDs[i], config.recent_cart[i]);
				flag = true;
			}
		}
		if(!flag)
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_CART1, _T("None"));
	}
#endif
#ifdef MENU_POS_FD1
	if(pos == MENU_POS_FD1) {
		// floppy drive #1
		UINT uIDs[8] = {
			ID_RECENT_FD11, ID_RECENT_FD12, ID_RECENT_FD13, ID_RECENT_FD14,
			ID_RECENT_FD15, ID_RECENT_FD16, ID_RECENT_FD17, ID_RECENT_FD18
		};
		bool flag = false;
		
		for(int i = 0; i < 8; i++)
			DeleteMenu(hMenu, uIDs[i], MF_BYCOMMAND);
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_disk[0][i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, uIDs[i], config.recent_disk[0][i]);
				flag = true;
			}
		}
		if(!flag)
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_FD11, _T("None"));
	}
#endif
#ifdef MENU_POS_FD2
	if(pos == MENU_POS_FD2) {
		// floppy drive #2
		UINT uIDs[8] = {
			ID_RECENT_FD21, ID_RECENT_FD22, ID_RECENT_FD23, ID_RECENT_FD24,
			ID_RECENT_FD25, ID_RECENT_FD26, ID_RECENT_FD27, ID_RECENT_FD28
		};
		bool flag = false;
		
		for(int i = 0; i < 8; i++)
			DeleteMenu(hMenu, uIDs[i], MF_BYCOMMAND);
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_disk[1][i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, uIDs[i], config.recent_disk[1][i]);
				flag = true;
			}
		}
		if(!flag)
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_FD21, _T("None"));
	}
#endif
#ifdef MENU_POS_FD3
	if(pos == MENU_POS_FD3) {
		// floppy drive #3
		UINT uIDs[8] = {
			ID_RECENT_FD31, ID_RECENT_FD32, ID_RECENT_FD33, ID_RECENT_FD34,
			ID_RECENT_FD35, ID_RECENT_FD36, ID_RECENT_FD37, ID_RECENT_FD38
		};
		bool flag = false;
		
		for(int i = 0; i < 8; i++)
			DeleteMenu(hMenu, uIDs[i], MF_BYCOMMAND);
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_disk[2][i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, uIDs[i], config.recent_disk[2][i]);
				flag = true;
			}
		}
		if(!flag)
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_FD31, _T("None"));
	}
#endif
#ifdef MENU_POS_FD4
	if(pos == MENU_POS_FD4) {
		// floppy drive #4
		UINT uIDs[8] = {
			ID_RECENT_FD41, ID_RECENT_FD42, ID_RECENT_FD43, ID_RECENT_FD44,
			ID_RECENT_FD45, ID_RECENT_FD46, ID_RECENT_FD47, ID_RECENT_FD48
		};
		bool flag = false;
		
		for(int i = 0; i < 8; i++)
			DeleteMenu(hMenu, uIDs[i], MF_BYCOMMAND);
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_disk[3][i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, uIDs[i], config.recent_disk[3][i]);
				flag = true;
			}
		}
		if(!flag)
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_FD41, _T("None"));
	}
#endif
#ifdef MENU_POS_DATAREC
	if(pos == MENU_POS_DATAREC) {
		// data recorder
		UINT uIDs[8] = {
			ID_RECENT_DATAREC1, ID_RECENT_DATAREC2, ID_RECENT_DATAREC3, ID_RECENT_DATAREC4,
			ID_RECENT_DATAREC5, ID_RECENT_DATAREC6, ID_RECENT_DATAREC7, ID_RECENT_DATAREC8
		};
		bool flag = false;
		
		for(int i = 0; i < 8; i++)
			DeleteMenu(hMenu, uIDs[i], MF_BYCOMMAND);
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_datarec[i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, uIDs[i], config.recent_datarec[i]);
				flag = true;
			}
		}
		if(!flag)
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_DATAREC1, _T("None"));
	}
#endif
#ifdef MENU_POS_MEDIA
	if(pos == MENU_POS_MEDIA) {
		// media
		UINT uIDs[8] = {
			ID_RECENT_MEDIA1, ID_RECENT_MEDIA2, ID_RECENT_MEDIA3, ID_RECENT_MEDIA4,
			ID_RECENT_MEDIA5, ID_RECENT_MEDIA6, ID_RECENT_MEDIA7, ID_RECENT_MEDIA8
		};
		bool flag = false;
		
		for(int i = 0; i < 8; i++)
			DeleteMenu(hMenu, uIDs[i], MF_BYCOMMAND);
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_media[i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, uIDs[i], config.recent_media[i]);
				flag = true;
			}
		}
		if(!flag)
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_MEDIA1, _T("None"));
	}
#endif
#ifdef MENU_POS_SCREEN
	if(pos == MENU_POS_SCREEN) {
		// recording
		bool now_rec = true, now_stop = true;
#ifndef _WIN32_WCE
		if(emu) {
			now_rec = emu->now_rec_video();
			now_stop = !now_rec;
		}
#endif
		EnableMenuItem(hMenu, ID_SCREEN_REC30, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_REC15, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_REC10, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
		
		// screen mode
#ifdef _WIN32_WCE
		EnableMenuItem(hMenu, ID_SCREEN_WINDOW1, MF_GRAYED);
		EnableMenuItem(hMenu, ID_SCREEN_640X480, MF_GRAYED);
#ifdef USE_SCREEN_X2
		EnableMenuItem(hMenu, ID_SCREEN_WINDOW2, MF_GRAYED);
		EnableMenuItem(hMenu, ID_SCREEN_320X240, MF_GRAYED);
#endif
#else
#ifdef USE_SCREEN_X2
		if(config.window_mode == 0)
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, ID_SCREEN_320X240, ID_SCREEN_WINDOW1, MF_BYCOMMAND);
		else if(config.window_mode == 1)
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, ID_SCREEN_320X240, ID_SCREEN_WINDOW2, MF_BYCOMMAND);
		else if(config.window_mode == 2)
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, ID_SCREEN_320X240, ID_SCREEN_640X480, MF_BYCOMMAND);
		else
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, ID_SCREEN_320X240, ID_SCREEN_320X240, MF_BYCOMMAND);
		EnableMenuItem(hMenu, ID_SCREEN_640X480, fullscreen_now ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_320X240, fullscreen_now ? MF_GRAYED : MF_ENABLED);
#else
		if(config.window_mode == 0)
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, ID_SCREEN_640X480, ID_SCREEN_WINDOW1, MF_BYCOMMAND);
		else if(config.window_mode == 2)
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, ID_SCREEN_640X480, ID_SCREEN_640X480, MF_BYCOMMAND);
		EnableMenuItem(hMenu, ID_SCREEN_640X480, fullscreen_now ? MF_GRAYED : MF_ENABLED);
#endif
#endif
		// mz2500 monitor type
#ifdef USE_MONITOR_TYPE
		if(config.monitor_type == 0)
			CheckMenuRadioItem(hMenu, ID_SCREEN_A400L, ID_SCREEN_D200L, ID_SCREEN_A400L, MF_BYCOMMAND);
		else if(config.monitor_type == 1)
			CheckMenuRadioItem(hMenu, ID_SCREEN_A400L, ID_SCREEN_D200L, ID_SCREEN_D400L, MF_BYCOMMAND);
		else if(config.monitor_type == 2)
			CheckMenuRadioItem(hMenu, ID_SCREEN_A400L, ID_SCREEN_D200L, ID_SCREEN_A200L, MF_BYCOMMAND);
		else
			CheckMenuRadioItem(hMenu, ID_SCREEN_A400L, ID_SCREEN_D200L, ID_SCREEN_D200L, MF_BYCOMMAND);
#endif
		// scanline
#ifdef USE_SCANLINE
		CheckMenuItem(hMenu, ID_SCREEN_SCANLINE, config.scan_line ? MF_CHECKED : MF_UNCHECKED);
#endif
	}
#endif
#ifdef MENU_POS_SOUND
	if(pos == MENU_POS_SOUND) {
		// sound menu
		bool now_rec = false, now_stop = false;
		if(emu) {
			now_rec = emu->now_rec_sound();
			now_stop = !now_rec;
		}
		EnableMenuItem(hMenu, ID_SOUND_REC, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SOUND_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
		
		if(config.sound_frequency == 0)
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ6, ID_SOUND_FREQ0, MF_BYCOMMAND);
		else if(config.sound_frequency == 1)
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ6, ID_SOUND_FREQ1, MF_BYCOMMAND);
		else if(config.sound_frequency == 2)
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ6, ID_SOUND_FREQ2, MF_BYCOMMAND);
		else if(config.sound_frequency == 3)
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ6, ID_SOUND_FREQ3, MF_BYCOMMAND);
		else if(config.sound_frequency == 4)
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ6, ID_SOUND_FREQ4, MF_BYCOMMAND);
		else if(config.sound_frequency == 5)
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ6, ID_SOUND_FREQ5, MF_BYCOMMAND);
		else
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ6, ID_SOUND_FREQ6, MF_BYCOMMAND);
		if(config.sound_latency == 0)
			CheckMenuRadioItem(hMenu, ID_SOUND_LATE0, ID_SOUND_LATE3, ID_SOUND_LATE0, MF_BYCOMMAND);
		else if(config.sound_latency == 1)
			CheckMenuRadioItem(hMenu, ID_SOUND_LATE0, ID_SOUND_LATE3, ID_SOUND_LATE1, MF_BYCOMMAND);
		else if(config.sound_latency == 2)
			CheckMenuRadioItem(hMenu, ID_SOUND_LATE0, ID_SOUND_LATE3, ID_SOUND_LATE2, MF_BYCOMMAND);
		else
			CheckMenuRadioItem(hMenu, ID_SOUND_LATE0, ID_SOUND_LATE3, ID_SOUND_LATE3, MF_BYCOMMAND);
	}
#endif
#ifdef MENU_POS_CAPTURE
	if(pos == MENU_POS_CAPTURE) {
		// video capture menu
		UINT uIDs[8] = {
			ID_CAPTURE_DEVICE1, ID_CAPTURE_DEVICE2, ID_CAPTURE_DEVICE3, ID_CAPTURE_DEVICE4,
			ID_CAPTURE_DEVICE5, ID_CAPTURE_DEVICE6, ID_CAPTURE_DEVICE7, ID_CAPTURE_DEVICE8
		};
		int cap_devs = emu->get_capture_devices();
		int connected = emu->get_connected_capture_device();
		
		for(int i = 0; i < 8; i++)
			DeleteMenu(hMenu, uIDs[i], MF_BYCOMMAND);
		for(int i = 0; i < 8; i++) {
			if(cap_devs >= i + 1)
				AppendMenu(hMenu, MF_STRING, uIDs[i], emu->get_capture_device_name(i));
		}
		if(!cap_devs)
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_CAPTURE_DEVICE1, _T("None"));
		if(connected != -1)
			CheckMenuRadioItem(hMenu, ID_CAPTURE_DEVICE1, ID_CAPTURE_DEVICE1, uIDs[connected], MF_BYCOMMAND);
		EnableMenuItem(hMenu, ID_CAPTURE_FILTER, (connected != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_PIN, (connected != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_SOURCE, (connected != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_DISCONNECT, (connected != -1) ? MF_ENABLED : MF_GRAYED);
	}
#endif
}

#ifdef USE_CART
void open_cart(HWND hWnd)
{
	_TCHAR szFile[_MAX_PATH] = _T("");
	OPENFILENAME OpenFileName;
	_memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = hWnd;
	OpenFileName.lpstrFilter = _T("Game Cartridge (*.rom)\0*.rom\0All Files (*.*)\0*.*\0\0");
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = _MAX_PATH;
	OpenFileName.lpstrTitle = _T("Game Cartridge");
	
	if(GetOpenFileName(&OpenFileName)) {
		// update history
		int no = 7;
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_cart[i], OpenFileName.lpstrFile) == 0) {
				no = i;
				break;
			}
		}
		for(int i = no; i > 0; i--)
			_tcscpy(config.recent_cart[i], config.recent_cart[i - 1]);
		_tcscpy(config.recent_cart[0], OpenFileName.lpstrFile);
		
		// open
		emu->open_cart(OpenFileName.lpstrFile);
	}
}
#endif

#ifdef USE_FD1
void open_disk(HWND hWnd, int drv)
{
	_TCHAR szFile[_MAX_PATH] = _T("");
	OPENFILENAME OpenFileName;
	_memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = hWnd;
	OpenFileName.lpstrFilter = _T("D88 Floppy Disk Files (*.d88)\0*.d88\0TeleDisk Floppy Disk Files (*.td0)\0*.td0\0All Files (*.*)\0*.*\0\0");
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = _MAX_PATH;
	OpenFileName.lpstrTitle = _T("Floppy Disk");
	
	if(GetOpenFileName(&OpenFileName)) {
		// update history
		int no = 7;
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_disk[drv][i], OpenFileName.lpstrFile) == 0) {
				no = i;
				break;
			}
		}
		for(int i = no; i > 0; i--)
			_tcscpy(config.recent_disk[drv][i], config.recent_disk[drv][i - 1]);
		_tcscpy(config.recent_disk[drv][0], OpenFileName.lpstrFile);
		
		// open
		emu->open_disk(OpenFileName.lpstrFile, drv);
	}
}
#endif

#ifdef USE_DATAREC
void open_datarec(HWND hWnd, BOOL play)
{
	_TCHAR szFile[_MAX_PATH] = _T("");
	OPENFILENAME OpenFileName;
	_memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = hWnd;
#ifdef DATAREC_BINARY_ONLY
	OpenFileName.lpstrFilter = _T("Cassette Files (*.cas)\0*.cas\0All Files (*.*)\0*.*\0\0");
#else
	OpenFileName.lpstrFilter = _T("Wave Files (*.wav)\0*.wav\0Cassette Files (*.cas)\0*.cas\0All Files (*.*)\0*.*\0\0");
#endif
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = _MAX_PATH;
	OpenFileName.lpstrTitle = _T("Data Recorder Tape");
	
	if(GetOpenFileName(&OpenFileName)) {
		// update history
		int no = 7;
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_datarec[i], OpenFileName.lpstrFile) == 0) {
				no = i;
				break;
			}
		}
		for(int i = no; i > 0; i--)
			_tcscpy(config.recent_datarec[i], config.recent_datarec[i - 1]);
		_tcscpy(config.recent_datarec[0], OpenFileName.lpstrFile);
		
		// open
		if(play)
			emu->play_datarec(OpenFileName.lpstrFile);
		else
			emu->rec_datarec(OpenFileName.lpstrFile);
	}
}
#endif

#ifdef USE_MEDIA
void open_media(HWND hWnd)
{
	_TCHAR szFile[_MAX_PATH] = _T("");
	OPENFILENAME OpenFileName;
	_memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = hWnd;
	OpenFileName.lpstrFilter = _T("Sound Cassette Files (*.m3u)\0*.m3u\0All Files (*.*)\0*.*\0\0");
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = _MAX_PATH;
	OpenFileName.lpstrTitle = _T("Sound Cassette");
	
	if(GetOpenFileName(&OpenFileName)) {
		// update history
		int no = 7;
		for(int i = 0; i < 8; i++) {
			if(_tcscmp(config.recent_media[i], OpenFileName.lpstrFile) == 0) {
				no = i;
				break;
			}
		}
		for(int i = no; i > 0; i--)
			_tcscpy(config.recent_media[i], config.recent_media[i - 1]);
		_tcscpy(config.recent_media[0], OpenFileName.lpstrFile);
		
		// open
		emu->open_media(OpenFileName.lpstrFile);
	}
}
#endif

void set_window(HWND hwnd, int mode)
{
#ifndef _WIN32_WCE
	static LONG style = WS_VISIBLE;
	static int dest_x = 0, dest_y = 0;
	WINDOWPLACEMENT place;
	place.length = sizeof(WINDOWPLACEMENT);
	
	if(mode == 0 || mode == 1) {
		// window
		RECT rect = {0, 0, (mode == 0) ? WINDOW_WIDTH1 : WINDOW_WIDTH2, (mode == 0) ? WINDOW_HEIGHT1 : WINDOW_HEIGHT2};
		AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, TRUE);
		
		if(!fullscreen_now) {
			HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
			int width = GetDeviceCaps(hdc, HORZRES);
			int height = GetDeviceCaps(hdc, VERTRES);
			dest_x = (int)((width - (rect.right - rect.left)) / 2);
			dest_y = (int)((height - (rect.bottom - rect.top)) / 2);
			dest_x = (dest_x < 0) ? 0 : dest_x;
			dest_y = (dest_y < 0) ? 0 : dest_y;
			SetWindowPos(hwnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		}
		else {
			ChangeDisplaySettings(NULL, 0);
			SetWindowLong(hwnd, GWL_STYLE, style);
			SetWindowPos(hwnd, HWND_TOP, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
			fullscreen_now = FALSE;
		}
		config.window_mode = mode;
		
		// set screen size to emu class
		emu->set_screen_size((mode == 0) ? WINDOW_WIDTH1 : WINDOW_WIDTH2, (mode == 0) ? WINDOW_HEIGHT1 : WINDOW_HEIGHT2);
	}
	else if((mode == 2 || mode == 3) && !fullscreen_now) {
		// fullscreen
		DEVMODE dev;
		HDC hdc = GetDC(NULL);
		ZeroMemory(&dev, sizeof(dev));
		dev.dmSize = sizeof(dev);
		dev.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		dev.dmBitsPerPel = GetDeviceCaps(hdc, BITSPIXEL);
		dev.dmPelsWidth = (mode == 2) ? 640 : 320;
		dev.dmPelsHeight = (mode == 2) ? 480 : 240;
		ReleaseDC(NULL, hdc);
		
		if(ChangeDisplaySettings(&dev, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
			GetWindowPlacement(hwnd, &place);
			dest_x = place.rcNormalPosition.left;
			dest_y = place.rcNormalPosition.top;
			ChangeDisplaySettings(&dev, CDS_FULLSCREEN);
			style = GetWindowLong(hwnd, GWL_STYLE);
			SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE);
			SetWindowPos(hwnd, HWND_TOP, 0, 0, (mode == 2) ? 640 : 320, (mode == 2) ? 480 : 240, SWP_SHOWWINDOW);
			SetCursorPos((mode == 2) ? 320 : 160, (mode == 2) ? 200 : 100);
			fullscreen_now = TRUE;
			config.window_mode = mode;
			
			// set screen size to emu class
			emu->set_screen_size((mode == 2) ? 640 : 320, (mode == 2) ? 480 - 18 : 240 - 18);
		}
	}
#endif
}

