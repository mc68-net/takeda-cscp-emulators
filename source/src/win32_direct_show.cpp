/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2014.01.09

	[ win32 laser disc ]
*/

#include "emu.h"
#include "vm/vm.h"

void EMU::initialize_direct_show()
{
	pDShowMS = NULL;
	pDShowMP = NULL;
	pDShowMC = NULL;
	pDShowBA = NULL;
	pDShowBV = NULL;
	pDShowVW = NULL;
	pDShowFS = NULL;
	pDShowCGB = NULL;
	pDShowBF = NULL;
	pDShowCapBF = NULL;
	pDShowSG = NULL;
	pDShowGB = NULL;
	
	hdcDibDShow = NULL;
	hBmpDShow = NULL;
	lpBufDShow = NULL;
	
	direct_show_mute[0] = direct_show_mute[1] = true;
#ifdef USE_LASER_DISC
	now_movie_play = now_movie_pause = false;
#endif
#ifdef USE_VIDEO_CAPTURE
	enum_capture_devs();
	cur_capture_dev_index = -1;
#endif
}

#define SAFE_RELEASE(x) { \
	if(x != NULL) { \
		(x)->Release(); \
		(x) = NULL; \
	} \
}

void EMU::release_direct_show()
{
	if(pDShowMC != NULL) {
		pDShowMC->Stop();
	}
	SAFE_RELEASE(pDShowMS);
	SAFE_RELEASE(pDShowMP);
	SAFE_RELEASE(pDShowMC);
	SAFE_RELEASE(pDShowBA);
	SAFE_RELEASE(pDShowBV);
	SAFE_RELEASE(pDShowVW);
	SAFE_RELEASE(pDShowFS);
	SAFE_RELEASE(pDShowCGB);
	SAFE_RELEASE(pDShowBF);
	SAFE_RELEASE(pDShowCapBF);
	SAFE_RELEASE(pDShowSG);
	SAFE_RELEASE(pDShowGB);
	
	release_direct_show_dib_section();
}

void EMU::create_direct_show_dib_section()
{
	HDC hdc = GetDC(main_window_handle);
	hdcDibDShow = CreateCompatibleDC(hdc);
	lpBufDShow = (LPBYTE)GlobalAlloc(GPTR, sizeof(BITMAPINFO));
	lpDibDShow = (LPBITMAPINFO)lpBufDShow;
	memset(&lpDibDShow->bmiHeader, 0, sizeof(BITMAPINFO));
	lpDibDShow->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpDibDShow->bmiHeader.biWidth = direct_show_width;
	lpDibDShow->bmiHeader.biHeight = direct_show_height;
	lpDibDShow->bmiHeader.biPlanes = 1;
#if defined(_RGB555)
	lpDibDShow->bmiHeader.biBitCount = 16;
	lpDibDShow->bmiHeader.biCompression = BI_RGB;
	lpDibDShow->bmiHeader.biSizeImage = direct_show_width * direct_show_height * 3;
#elif defined(_RGB565)
	lpDibDShow->bmiHeader.biBitCount = 16;
	lpDibDShow->bmiHeader.biCompression = BI_BITFIELDS;
	lpDibDShow->bmiHeader.biSizeImage = 0;
	LPDWORD lpBf = (LPDWORD)lpDibDShow->bmiColors;
	lpBf[0] = 0x1f << 11;
	lpBf[1] = 0x3f << 5;
	lpBf[2] = 0x1f << 0;
	lpDibDShow->bmiHeader.biSizeImage = direct_show_width * direct_show_height * 2;
#elif defined(_RGB888)
	lpDibDShow->bmiHeader.biBitCount = 32;
	lpDibDShow->bmiHeader.biCompression = BI_RGB;
	lpDibDShow->bmiHeader.biSizeImage = direct_show_width * direct_show_height * 4;
#endif
	lpDibDShow->bmiHeader.biXPelsPerMeter = 0;
	lpDibDShow->bmiHeader.biYPelsPerMeter = 0;
	lpDibDShow->bmiHeader.biClrUsed = 0;
	lpDibDShow->bmiHeader.biClrImportant = 0;
	hBmpDShow = CreateDIBSection(hdc, lpDibDShow, DIB_RGB_COLORS, (PVOID*)&lpBmpDShow, NULL, 0);
	hOldBmpDShow = (HBITMAP)SelectObject(hdcDibDShow, hBmpDShow);
	ReleaseDC(main_window_handle, hdc);
}

void EMU::release_direct_show_dib_section()
{
	if(hdcDibDShow != NULL && hOldBmpDShow != NULL) {
		SelectObject(hdcDibDShow, hOldBmpDShow);
	}
	if(hBmpDShow != NULL) {
		DeleteObject(hBmpDShow);
		hBmpDShow = NULL;
	}
	if(lpBufDShow != NULL) {
		GlobalFree(lpBufDShow);
		lpBufDShow = NULL;
	}
	if(hdcDibDShow != NULL) {
		DeleteDC(hdcDibDShow);
		hdcDibDShow = NULL;
	}
}

void EMU::get_direct_show_buffer()
{
	if(pDShowSG != NULL) {
#if defined(_RGB555) || defined(_RGB565)
		long buffer_size = direct_show_width * direct_show_height * 2;
#elif defined(_RGB888)
		long buffer_size = direct_show_width * direct_show_height * 4;
#endif
		pDShowSG->GetCurrentBuffer(&buffer_size, (long *)lpBmpDShow);
		if(screen_width == direct_show_width && screen_height == direct_show_height) {
			BitBlt(hdcDib, 0, 0, screen_width, screen_height, hdcDibDShow, 0, 0, SRCCOPY);
		} else {
			StretchBlt(hdcDib, 0, 0, screen_width, screen_height, hdcDibDShow, 0, 0, direct_show_width, direct_show_height, SRCCOPY);
		}
		if(use_d3d9 && lpd3d9Buffer != NULL && render_to_d3d9Buffer && !now_rec_vid) {
			for(int y = 0; y < screen_height; y++) {
				scrntype* src = lpBmp + screen_width * (screen_height - y - 1);
				scrntype* dst = lpd3d9Buffer + screen_width * y;
				memcpy(dst, src, screen_width * sizeof(scrntype));
			}
		}
	} else {
		if(use_d3d9 && lpd3d9Buffer != NULL && render_to_d3d9Buffer && !now_rec_vid) {
			memset(lpd3d9Buffer, 0, screen_width * screen_height * sizeof(scrntype));
		} else {
			memset(lpBmp, 0, screen_width * screen_height * sizeof(scrntype));
		}
	}
}

void EMU::mute_direct_show_dev(bool l, bool r)
{
	if(pDShowBA != NULL) {
		if(l && r) {
			pDShowBA->put_Volume(-10000L);
		} else {
			pDShowBA->put_Volume(0L);
		}
		if(l && !r) {
			pDShowBA->put_Balance(1000L);
		} else if(!l && r) {
			pDShowBA->put_Balance(-1000L);
		} else {
			pDShowBA->put_Balance(0L);
		}
		direct_show_mute[0] = l;
		direct_show_mute[1] = r;
	}
}

static void initialize_media_type(AM_MEDIA_TYPE *mt)
{
	ZeroMemory(mt, sizeof(AM_MEDIA_TYPE));
	mt->majortype = MEDIATYPE_Video;
#if defined(_RGB555)
	mt->subtype = MEDIASUBTYPE_RGB555;
#elif defined(_RGB565)
	mt->subtype = MEDIASUBTYPE_RGB565;
#elif defined(_RGB888)
	mt->subtype = MEDIASUBTYPE_RGB32;
#endif
	mt->formattype = FORMAT_VideoInfo;
}

#ifdef USE_LASER_DISC
bool EMU::open_movie_file(_TCHAR* file_path)
{
	WCHAR	wFile[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, file_path, -1, wFile, MAX_PATH);
	
	AM_MEDIA_TYPE mt;
	initialize_media_type(&mt);
	
	if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&pDShowGB))) {
		return false;
	}
	if(FAILED(CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&pDShowBF))) {
		return false;
	}
	if(FAILED(pDShowBF->QueryInterface(IID_ISampleGrabber, (void **)&pDShowSG))) {
		return false;
	}
	if(FAILED(pDShowSG->SetMediaType(&mt))) {
		return false;
	}
	if(FAILED(pDShowGB->AddFilter(pDShowBF, L"Sample Grabber"))) {
		return false;
	}
	if(FAILED(pDShowGB->RenderFile(wFile, NULL))) {
		return false;
	}
	if(FAILED(pDShowSG->SetBufferSamples(TRUE))) {
		return false;
	}
	if(FAILED(pDShowGB->QueryInterface(IID_IMediaControl, (void **)&pDShowMC))) {
		return false;
	}
	if(FAILED(pDShowGB->QueryInterface(IID_IMediaSeeking, (void **)&pDShowMS))) {
		return false;
	}
	if(FAILED(pDShowGB->QueryInterface(IID_IMediaPosition, (void **)&pDShowMP))) {
		return false;
	}
	if(FAILED(pDShowGB->QueryInterface(IID_IVideoWindow, (void **)&pDShowVW))) {
		return false;
	}
	if(FAILED(pDShowGB->QueryInterface(IID_IBasicVideo, (void **)&pDShowBV))) {
		return false;
	}
	if(FAILED(pDShowGB->QueryInterface(IID_IBasicAudio, (void **)&pDShowBA))) {
		return false;
	}
	if(FAILED(pDShowVW->put_Owner((OAHWND)main_window_handle))) {
		return false;
	}
	if(FAILED(pDShowVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN))) {
		return false;
	}
	if(FAILED(pDShowVW->SetWindowPosition(0, 0, 0, 0))) {
		return false;
	}
	if(FAILED(pDShowVW->SetWindowForeground(FALSE))) {
		return false;
	}
	if(FAILED(pDShowMS->SetTimeFormat(&TIME_FORMAT_FRAME))) {
		return false;
	}
	if(FAILED(pDShowBV->get_AvgTimePerFrame(&movie_fps))) {
		return false;
	}
	movie_fps = 1 / movie_fps;
	
	// get the movie size
	pDShowSG->GetConnectedMediaType(&mt);
	VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)mt.pbFormat;
	direct_show_width = pVideoHeader->bmiHeader.biWidth;
	direct_show_height = pVideoHeader->bmiHeader.biHeight;
	
	// create DIBSection
	create_direct_show_dib_section();
	
	return true;
}

void EMU::close_movie_file()
{
	release_direct_show();
	now_movie_play = false;
	now_movie_pause = false;
}

void EMU::play_movie()
{
	if(pDShowMC != NULL) {
		pDShowMC->Run();
		mute_direct_show_dev(direct_show_mute[0], direct_show_mute[1]);
		now_movie_play = true;
		now_movie_pause = false;
	}
}

void EMU::stop_movie()
{
	if(pDShowMC != NULL) {
		pDShowMC->Stop();
	}
	now_movie_play = false;
	now_movie_pause = false;
}

void EMU::pause_movie()
{
	if(pDShowMC != NULL) {
		pDShowMC->Pause();
		now_movie_pause = true;
	}
}

void EMU::set_cur_movie_frame(int frame, bool relative)
{
	if(pDShowMS != NULL) {
		LONGLONG now = frame;
		pDShowMS->SetPositions(&now, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	}
}

uint32 EMU::get_cur_movie_frame()
{
	if(pDShowMS != NULL) {
		LONGLONG now, stop;
		if(SUCCEEDED(pDShowMS->GetPositions(&now, &stop))) {
			return (uint32)(now & 0xffffffff);
		}
	}
	return 0;
}
#endif

#ifdef USE_VIDEO_CAPTURE
static LPSTR MyAtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	lpa[0] = '\0';
	WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
	return lpa;
}

#include <malloc.h>

#define MyW2T(lpw) (((_lpw = lpw) == NULL) ? NULL : (_convert = (lstrlenW(_lpw) + 1) * 2, MyAtlW2AHelper((LPSTR)_alloca(_convert), _lpw, _convert)))

static IPin* get_pin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
	IEnumPins *pEnum;
	IPin *pPin;
	bool found = false;
	
	pFilter->EnumPins(&pEnum);
	while(SUCCEEDED(pEnum->Next(1, &pPin, 0))) {
		PIN_DIRECTION PinDirThis;
		pPin->QueryDirection(&PinDirThis);
		if(found = (PinDir == PinDirThis)) {
			break;
		}
		SAFE_RELEASE(pPin);
	}
	SAFE_RELEASE(pEnum);
	return found ? pPin : NULL;
}

void EMU::enum_capture_devs()
{
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	
	num_capture_devs = 0;
	
	// create the system device enum
	if(SUCCEEDED(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum))) {
		// create the video input device enu,
		if(SUCCEEDED(pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0)) && pClassEnum != NULL) {
			ULONG cFetched;
			IMoniker *pMoniker = NULL;
			
			while(num_capture_devs < MAX_CAPTURE_DEVS && SUCCEEDED(pClassEnum->Next(1, &pMoniker, &cFetched)) && pMoniker != NULL) {
				IPropertyBag *pBag = NULL;
				if(SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag))) {
					VARIANT var;
					var.vt = VT_BSTR;
					if(pBag->Read(L"FriendlyName", &var, NULL) == NOERROR) {
						LPCWSTR _lpw = NULL;
						int _convert = 0;
						_tcscpy(capture_dev_name[num_capture_devs++], MyW2T(var.bstrVal));
						SysFreeString(var.bstrVal);
						pMoniker->AddRef();
					}
					SAFE_RELEASE(pBag);
				}
				SAFE_RELEASE(pMoniker);
			}
		}
	}
	SAFE_RELEASE(pClassEnum);
	SAFE_RELEASE(pDevEnum);
}

bool EMU::connect_capture_dev(int index, bool pin)
{
	if(cur_capture_dev_index == index && !pin) {
		return true;
	}
	if(cur_capture_dev_index != -1) {
		release_direct_show();
		cur_capture_dev_index = -1;
	}
	
	AM_MEDIA_TYPE mt;
	initialize_media_type(&mt);
	
	if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&pDShowGB))) {
		return false;
	}
	
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	
	if(SUCCEEDED(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum))) {
		if(SUCCEEDED(pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0)) && pClassEnum != NULL) {
			for(int i = 0; i <= index; i++) {
				IMoniker *pMoniker = NULL;
				ULONG cFetched;
				
				if(SUCCEEDED(pClassEnum->Next(1, &pMoniker, &cFetched)) && pMoniker != NULL) {
					if(i == index) {
						pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pDShowCapBF);
					}
					SAFE_RELEASE(pMoniker);
				} else {
					break;
				}
			}
		}
	}
	SAFE_RELEASE(pClassEnum);
	SAFE_RELEASE(pDevEnum);
	
	if(&pDShowCapBF == NULL) {
		return false;
	}
	if(FAILED(pDShowGB->AddFilter(pDShowCapBF, L"Video Capture"))) {
		return false;
	}
	if(FAILED(CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&pDShowCGB))) {
		return false;
	}
	if(FAILED(pDShowCGB->SetFiltergraph(pDShowGB))) {
		return false;
	}
	
	IAMStreamConfig *pSC = NULL;
	ISpecifyPropertyPages *pSpec = NULL;
	
	if(FAILED(pDShowCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pDShowCapBF, IID_IAMStreamConfig, (void **)&pSC))) {
		if(FAILED(pDShowCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pDShowCapBF, IID_IAMStreamConfig, (void **)&pSC))) {
			return false;
		}
	}
	if(SUCCEEDED(pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec))) {
		CAUUID cauuid;
		pSpec->GetPages(&cauuid);
		if(pin) {
			OleCreatePropertyFrame(NULL, 30, 30, NULL, 1, (IUnknown **)&pSC, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL);
			CoTaskMemFree(cauuid.pElems);
		}
		SAFE_RELEASE(pSpec);
	}
	SAFE_RELEASE(pSC);
	
	if(FAILED(CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&pDShowBF))) {
		return false;
	}
	if(FAILED(pDShowBF->QueryInterface(IID_ISampleGrabber, (void **)&pDShowSG))) {
		return false;
	}
	if(FAILED(pDShowSG->SetMediaType(&mt))) {
		return false;
	}
	if(FAILED(pDShowGB->AddFilter(pDShowBF, L"Sample Grabber"))) {
		return false;
	}
	if(FAILED(pDShowGB->Connect(get_pin(pDShowCapBF, PINDIR_OUTPUT), get_pin(pDShowBF, PINDIR_INPUT)))) {
		return false;
	}
	if(FAILED(pDShowSG->SetBufferSamples(TRUE))) {
		return false;
	}
	if(FAILED(pDShowSG->SetOneShot(FALSE))) {
		return false;
	}
	if(FAILED(pDShowGB->QueryInterface(IID_IMediaControl, (void **)&pDShowMC))) {
		return false;
	}
	if(FAILED(pDShowGB->QueryInterface(IID_IBasicAudio, (void **)&pDShowBA))) {
		//return false;
	}
	if(FAILED(pDShowMC->Run())) {
		return false;
	}
	
	// get the capture size
	pDShowSG->GetConnectedMediaType(&mt);
	VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)mt.pbFormat;
	direct_show_width = pVideoHeader->bmiHeader.biWidth;
	direct_show_height = pVideoHeader->bmiHeader.biHeight;
	
	// create DIBSection
	create_direct_show_dib_section();
	
	cur_capture_dev_index = index;
	return true;
}

void EMU::open_capture_dev(int index, bool pin)
{
	if(!connect_capture_dev(index, pin)) {
		release_direct_show();
	}
}

void EMU::close_capture_dev()
{
	release_direct_show();
	cur_capture_dev_index = -1;
}

void EMU::show_capture_dev_filter()
{
	if(pDShowCapBF != NULL) {
		ISpecifyPropertyPages *pSpec = NULL;
		CAUUID cauuid;
		if(SUCCEEDED(pDShowCapBF->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec))) {
			pSpec->GetPages(&cauuid);
			OleCreatePropertyFrame(NULL, 30, 30, NULL, 1, (IUnknown **)&pDShowCapBF, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL);
			CoTaskMemFree(cauuid.pElems);
			SAFE_RELEASE(pSpec);
		}
	}
}

void EMU::show_capture_dev_pin()
{
	if(cur_capture_dev_index != -1) {
		if(!connect_capture_dev(cur_capture_dev_index, true)) {
			release_direct_show();
		}
	}
}

void EMU::show_capture_dev_source()
{
	if(pDShowCGB != NULL) {
		IAMCrossbar *pCrs = NULL;
		ISpecifyPropertyPages *pSpec = NULL;
		CAUUID cauuid;
		
		if(FAILED(pDShowCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pDShowCapBF, IID_IAMCrossbar, (void **)&pCrs))) {
			if(FAILED(pDShowCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pDShowCapBF, IID_IAMCrossbar, (void **)&pCrs))) {
				return;
			}
		}
		if(SUCCEEDED(pCrs->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec))) {
			pSpec->GetPages(&cauuid);
			OleCreatePropertyFrame(NULL, 30, 30, NULL, 1, (IUnknown **)&pCrs, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL);
			CoTaskMemFree(cauuid.pElems);
			SAFE_RELEASE(pSpec);
			
			AM_MEDIA_TYPE mt;
			pDShowSG->GetConnectedMediaType(&mt);
			VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)mt.pbFormat;
			direct_show_width = pVideoHeader->bmiHeader.biWidth;
			direct_show_height = pVideoHeader->bmiHeader.biHeight;
			
			release_direct_show_dib_section();
			create_direct_show_dib_section();
		}
		SAFE_RELEASE(pCrs);
	}
}

void EMU::set_capture_dev_channel(int ch)
{
	IAMTVTuner *pTuner = NULL;
	
	if(pDShowCGB != NULL) {
		if(SUCCEEDED(pDShowCGB->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pDShowCapBF, IID_IAMTVTuner, (void **)&pTuner))) {
			pTuner->put_Channel(ch, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
			SAFE_RELEASE(pTuner);
		}
	}
}
#endif
