/*
	CANON X-07 Emulator 'eX-07'
	Skelton for retropc emulator

	Origin : J.Brigaud
	Author : Takeda.Toshiya
	Date   : 2007.12.26 -

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define WRAM_OFS_UDC0	0x200
#define WRAM_OFS_UDC1	0x300
#define WRAM_OFS_KBUF	0x400
#define WRAM_OFS_SPGM	0x500
#define WRAM_OFS_RSVD	0x700
#define FONT_OFS_UDC0	0x400
#define FONT_OFS_UDC1	0x700
#define CMT_BUF_SIZE	0x8000

#define EVENT_BEEP	0
#define EVENT_CMT	1

static const uint8 sub_cmd_len[0x47] = {
	1,	// 00	Unknown
	1,	// 01	TimeCall
	1,	// 02	Stick
	1,	// 03	Strig
	1,	// 04	Strig1
	3,	// 05	RamRead
	4,	// 06	RamWrite
	3,	// 07	ScrollSet
	1,	// 08	ScrollExet
	2,	// 09	LineClear
	9,	// 0a	TimeSet
	1,	// 0b	CalcDay
	9,	// 0c	AlarmSet
	1,	// 0d	BuzzerOff
	1,	// 0e	BuzzerOn
	2,	// 0f	TrfLine
	3,	// 10	TestPoint
	3,	// 11	Pset
	3,	// 12	Preset
	3,	// 13	Peor
	5,	// 14	Line
	4,	// 15	Circle
	0x82,	// 16	UDKWrite
	2,	// 17	UDKRead
	1,	// 18	UDKOn
	1,	// 19	UDKOff
	10,	// 1a	UDCWrite
	2,	// 1b	UDCRead
	1,	// 1c	UDCInt
	0x81,	// 1d	StartPgmWrite
	0x81,	// 1e	SPWriteCont
	1,	// 1f	SPOn
	1,	// 20	SPOff
	1,	// 21	StartPgmRead
	1,	// 22	OnStat
	1,	// 23	OFFReq
	4,	// 24	Locate
	1,	// 25	CursOn
	1,	// 26	CursOff
	3,	// 27	TestKey
	2,	// 28	TestChr
	1,	// 29	InitSec
	2,	// 2a	InitDate
	1,	// 2b	ScrOff
	1,	// 2c	ScrOn
	1,	// 2d	KeyBufferClear
	1,	// 2e	ClsScr
	1,	// 2f	Home
	1,	// 30	OutUdkOn
	1,	// 31	OutUDKOff
	1,	// 32	RepateKeyOn
	1,	// 33	RepateKeyOff
	1,	// 34	UDK=KANA
	0x82,	// 35	UdkContWrite
	1,	// 36	AlarmRead
	1,	// 37	BuzzZero
	1,	// 38	ClickOff
	1,	// 39	ClickOn
	1,	// 3a	LocateClose
	1,	// 3b	KeybOn
	1,	// 3c	KeybOff
	1,	// 3d	ExecStartPgm
	1,	// 3e	UnexecStartPgm
	1,	// 3f	Sleep
	1,	// 40	UDKInit
	9,	// 41	OutUDC
	1,	// 42	ReadCar
	3,	// 43	ScanR
	3,	// 44	ScanL
	1,	// 45	TimeChk
	1	// 46	AlmChk
};

static const char *udk_ini[12] = {
	"tim?TIME$^",
	"cldCLOAD\"",
	"locLOCATE ",
	"lstLIST ",
	"runRUN^",
	"",
	"dat?DATE$^",
	"csvCSAVE\"",
	"prtPRINT ",
	"slpSLEEP",
	"cntCONT^",
	""
};

static const int udk_ofs[12] = {
	0, 42, 84, 126, 168, 210, 256, 298, 340, 382, 424, 466
};

static const int udk_size[12] = {
	42, 42, 42, 42, 42, 46, 42, 42, 42, 42, 42, 46
};

static const int key_tbl[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0b,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x41,0x42,0x43, 0x44,0x45,0x46,0x47, 0x48,0x49,0x4a,0x4b, 0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53, 0x54,0x55,0x56,0x57, 0x58,0x59,0x5a,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x3a,0x3b, 0x2c,0x2d,0x2e,0x2f,
	0x40,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x5b, 0x5c,0x5d,0x5e,0x00,
	0x00,0x00,0x5c,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_s[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0c,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0x00,0x21,0x22,0x23, 0x24,0x25,0x26,0x27, 0x28,0x29,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x61,0x62,0x63, 0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b, 0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x2a,0x2b, 0x3c,0x3d,0x3e,0x3f,
	0x60,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x7b, 0x7c,0x7d,0x7e,0x00,
	0x00,0x00,0x5f,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_k[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0b,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0xdc,0xc7,0xcc,0xb1, 0xb3,0xb4,0xb5,0xd4, 0xd5,0xd6,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0xc1,0xba,0xbf, 0xbc,0xb2,0xca,0xb7, 0xb8,0xc6,0xcf,0xc9, 0xd8,0xd3,0xd0,0xd7,
	0xbe,0xc0,0xbd,0xc4, 0xb6,0xc5,0xcb,0xc3, 0xbb,0xdd,0xc2,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0xb9,0xda, 0xc8,0xce,0xd9,0xd2,
	0xde,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xdf, 0xb0,0xd1,0xcd,0x00,
	0x00,0x00,0xdb,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_ks[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0c,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0xa6,0x00,0x00,0xa7, 0xa9,0xaa,0xab,0xac, 0xad,0xae,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0xa8,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0xaf,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xa4,0x00,0xa1,0xa5,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xa2, 0x00,0xa3,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_g[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0c,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0x8a,0xe9,0x90,0x91, 0x92,0x93,0xec,0xe0, 0xf2,0xf1,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x88,0xed,0xe4, 0xef,0x99,0xfd,0x9d, 0xfe,0xf9,0xe5,0x9b, 0xf4,0xf5,0x89,0x9e,
	0xf7,0x8b,0xf6,0x9f, 0x97,0x94,0x95,0xfb, 0x98,0x96,0xe1,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x81,0x82, 0x9c,0xf0,0x9a,0x80,
	0xe7,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x84, 0x00,0x85,0xfc,0x00,
	0x00,0x00,0x83,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_c[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0b,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

class FIFO;
class FILEIO;

class IO : public DEVICE
{
private:
	DEVICE *d_beep, *d_cpu, *d_mem;
	int did_beep_on, did_beep_freq, did_rsta, did_rstb;
	uint8* ram;
	
	// registers
	uint8 rregs[8], wregs[8];
	
	// t6834
	void update_intr();
	void send_to_sub();
	void recv_from_sub();
	void ack_from_sub();
	void process_sub();
	FIFO* cmd_buf;
	FIFO* rsp_buf;
	uint8 sub_int;
	uint8 wram[0x800];
	uint8 alarm[8];
	
	// keyboard
	FIFO* key_buf;
	bool ctrl, shift, kana, graph, brk;
	uint8 stick, strig, strig1;
	
	// data recorder
	void send_to_cmt();
	void recv_from_cmt();
	FILEIO* cmt_fio;
	uint8 cmt_buf[CMT_BUF_SIZE];
	int cmt_ptr;
	bool cmt_play, cmt_rec, cmt_mode;
	
	// x-720
	bool vblank;
	uint8 font_code;
	
	// video
	void draw_font(int x, int y, uint8 code);
	void draw_udk();
	void draw_line(int sx, int sy, int ex, int ey);
	void draw_circle(int x, int y, int r);
	void line_clear(int y);
	void scroll();
	uint8 font[256 * 8], udc[256 * 8];
	uint8 lcd[32][120];
	bool locate_on, cursor_on, udk_on;
	int locate_x, locate_y;
	int cursor_x, cursor_y, cursor_blink;
	int scroll_min, scroll_max;
	
	// beep
	int regist_id;
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IO() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique functions
	void play_datarec(_TCHAR* filename);
	void rec_datarec(_TCHAR* filename);
	void close_datarec();
	void set_context_beep(DEVICE* device, int id0, int id1) {
		d_beep = device;
		did_beep_on = id0;
		did_beep_freq = id1;
	}
	void set_context_cpu(DEVICE* device, int id0, int id1) {
		d_cpu = device;
		did_rsta = id0;
		did_rstb = id1;
	}
	void set_context_mem(DEVICE* device, uint8* ptr) {
		d_mem = device;
		ram = ptr;
	}
	void draw_screen();
	void key_down(int code);
	void key_up(int code);
};

#endif
