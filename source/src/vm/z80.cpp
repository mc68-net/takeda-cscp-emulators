/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ Z80 ]
*/

#include "z80.h"

#define AF	regs[0].w.l
#define BC	regs[1].w.l
#define DE 	regs[2].w.l
#define HL	regs[3].w.l
#define IX	regs[4].w.l
#define IY	regs[5].w.l

#define _F	regs[0].b.l
#define _A	regs[0].b.h
#define _C	regs[1].b.l
#define _B	regs[1].b.h
#define _E	regs[2].b.l
#define _D	regs[2].b.h
#define _L	regs[3].b.l
#define _H	regs[3].b.h
#define _XL	regs[4].b.l
#define _XH	regs[4].b.h
#define _YL	regs[5].b.l
#define _YH	regs[5].b.h

#define CF	0x01
#define NF	0x02
#define PF	0x04
#define VF	0x04
#define XF	0x08
#define HF	0x10
#define YF	0x20
#define ZF	0x40
#define SF	0x80

#ifndef CPU_START_ADDR
#define CPU_START_ADDR	0
#endif

static const int cc_op[0x100] = {
	 4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4, 8,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
	 7,10,16, 6, 4, 4, 7, 4, 7,11,16, 6, 4, 4, 7, 4, 7,10,13, 6,11,11,10, 4, 7,11,13, 6, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11, 5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11,
	 5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11, 5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11
};

static const int cc_cb[0x100] = {
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8
};

static const int cc_ed[0x100] = {
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	12,12,15,20, 8, 8, 8, 9,12,12,15,20, 8, 8, 8, 9,12,12,15,20, 8, 8, 8, 9,12,12,15,20, 8, 8, 8, 9,
	12,12,15,20, 8, 8, 8,18,12,12,15,20, 8, 8, 8,18,12,12,15,20, 8, 8, 8, 8,12,12,15,20, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

static const int cc_xy[0x100] = {
	 4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
	 4,14,20,10, 9, 9, 9, 4, 4,15,20,10, 9, 9, 9, 4, 4, 4, 4, 4,23,23,19, 4, 4,15, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 9, 9, 9, 9, 9, 9,19, 9, 9, 9, 9, 9, 9, 9,19, 9,19,19,19,19,19,19, 4,19, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	 4,14, 4,23, 4,15, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,10, 4, 4, 4, 4, 4, 4
};

static const int cc_xycb[0x100] = {
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23
};

static const int cc_ex[0x100] = {
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0,
	 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2
};

static const uint16 DAATable[0x800] = {
	0x0044,0x0100,0x0200,0x0304,0x0400,0x0504,0x0604,0x0700,0x0808,0x090c,0x1010,0x1114,0x1214,0x1310,0x1414,0x1510,
	0x1000,0x1104,0x1204,0x1300,0x1404,0x1500,0x1600,0x1704,0x180c,0x1908,0x2030,0x2134,0x2234,0x2330,0x2434,0x2530,
	0x2020,0x2124,0x2224,0x2320,0x2424,0x2520,0x2620,0x2724,0x282c,0x2928,0x3034,0x3130,0x3230,0x3334,0x3430,0x3534,
	0x3024,0x3120,0x3220,0x3324,0x3420,0x3524,0x3624,0x3720,0x3828,0x392c,0x4010,0x4114,0x4214,0x4310,0x4414,0x4510,
	0x4000,0x4104,0x4204,0x4300,0x4404,0x4500,0x4600,0x4704,0x480c,0x4908,0x5014,0x5110,0x5210,0x5314,0x5410,0x5514,
	0x5004,0x5100,0x5200,0x5304,0x5400,0x5504,0x5604,0x5700,0x5808,0x590c,0x6034,0x6130,0x6230,0x6334,0x6430,0x6534,
	0x6024,0x6120,0x6220,0x6324,0x6420,0x6524,0x6624,0x6720,0x6828,0x692c,0x7030,0x7134,0x7234,0x7330,0x7434,0x7530,
	0x7020,0x7124,0x7224,0x7320,0x7424,0x7520,0x7620,0x7724,0x782c,0x7928,0x8090,0x8194,0x8294,0x8390,0x8494,0x8590,
	0x8080,0x8184,0x8284,0x8380,0x8484,0x8580,0x8680,0x8784,0x888c,0x8988,0x9094,0x9190,0x9290,0x9394,0x9490,0x9594,
	0x9084,0x9180,0x9280,0x9384,0x9480,0x9584,0x9684,0x9780,0x9888,0x998c,0x0055,0x0111,0x0211,0x0315,0x0411,0x0515,
	0x0045,0x0101,0x0201,0x0305,0x0401,0x0505,0x0605,0x0701,0x0809,0x090d,0x1011,0x1115,0x1215,0x1311,0x1415,0x1511,
	0x1001,0x1105,0x1205,0x1301,0x1405,0x1501,0x1601,0x1705,0x180d,0x1909,0x2031,0x2135,0x2235,0x2331,0x2435,0x2531,
	0x2021,0x2125,0x2225,0x2321,0x2425,0x2521,0x2621,0x2725,0x282d,0x2929,0x3035,0x3131,0x3231,0x3335,0x3431,0x3535,
	0x3025,0x3121,0x3221,0x3325,0x3421,0x3525,0x3625,0x3721,0x3829,0x392d,0x4011,0x4115,0x4215,0x4311,0x4415,0x4511,
	0x4001,0x4105,0x4205,0x4301,0x4405,0x4501,0x4601,0x4705,0x480d,0x4909,0x5015,0x5111,0x5211,0x5315,0x5411,0x5515,
	0x5005,0x5101,0x5201,0x5305,0x5401,0x5505,0x5605,0x5701,0x5809,0x590d,0x6035,0x6131,0x6231,0x6335,0x6431,0x6535,
	0x6025,0x6121,0x6221,0x6325,0x6421,0x6525,0x6625,0x6721,0x6829,0x692d,0x7031,0x7135,0x7235,0x7331,0x7435,0x7531,
	0x7021,0x7125,0x7225,0x7321,0x7425,0x7521,0x7621,0x7725,0x782d,0x7929,0x8091,0x8195,0x8295,0x8391,0x8495,0x8591,
	0x8081,0x8185,0x8285,0x8381,0x8485,0x8581,0x8681,0x8785,0x888d,0x8989,0x9095,0x9191,0x9291,0x9395,0x9491,0x9595,
	0x9085,0x9181,0x9281,0x9385,0x9481,0x9585,0x9685,0x9781,0x9889,0x998d,0xa0b5,0xa1b1,0xa2b1,0xa3b5,0xa4b1,0xa5b5,
	0xa0a5,0xa1a1,0xa2a1,0xa3a5,0xa4a1,0xa5a5,0xa6a5,0xa7a1,0xa8a9,0xa9ad,0xb0b1,0xb1b5,0xb2b5,0xb3b1,0xb4b5,0xb5b1,
	0xb0a1,0xb1a5,0xb2a5,0xb3a1,0xb4a5,0xb5a1,0xb6a1,0xb7a5,0xb8ad,0xb9a9,0xc095,0xc191,0xc291,0xc395,0xc491,0xc595,
	0xc085,0xc181,0xc281,0xc385,0xc481,0xc585,0xc685,0xc781,0xc889,0xc98d,0xd091,0xd195,0xd295,0xd391,0xd495,0xd591,
	0xd081,0xd185,0xd285,0xd381,0xd485,0xd581,0xd681,0xd785,0xd88d,0xd989,0xe0b1,0xe1b5,0xe2b5,0xe3b1,0xe4b5,0xe5b1,
	0xe0a1,0xe1a5,0xe2a5,0xe3a1,0xe4a5,0xe5a1,0xe6a1,0xe7a5,0xe8ad,0xe9a9,0xf0b5,0xf1b1,0xf2b1,0xf3b5,0xf4b1,0xf5b5,
	0xf0a5,0xf1a1,0xf2a1,0xf3a5,0xf4a1,0xf5a5,0xf6a5,0xf7a1,0xf8a9,0xf9ad,0x0055,0x0111,0x0211,0x0315,0x0411,0x0515,
	0x0045,0x0101,0x0201,0x0305,0x0401,0x0505,0x0605,0x0701,0x0809,0x090d,0x1011,0x1115,0x1215,0x1311,0x1415,0x1511,
	0x1001,0x1105,0x1205,0x1301,0x1405,0x1501,0x1601,0x1705,0x180d,0x1909,0x2031,0x2135,0x2235,0x2331,0x2435,0x2531,
	0x2021,0x2125,0x2225,0x2321,0x2425,0x2521,0x2621,0x2725,0x282d,0x2929,0x3035,0x3131,0x3231,0x3335,0x3431,0x3535,
	0x3025,0x3121,0x3221,0x3325,0x3421,0x3525,0x3625,0x3721,0x3829,0x392d,0x4011,0x4115,0x4215,0x4311,0x4415,0x4511,
	0x4001,0x4105,0x4205,0x4301,0x4405,0x4501,0x4601,0x4705,0x480d,0x4909,0x5015,0x5111,0x5211,0x5315,0x5411,0x5515,
	0x5005,0x5101,0x5201,0x5305,0x5401,0x5505,0x5605,0x5701,0x5809,0x590d,0x6035,0x6131,0x6231,0x6335,0x6431,0x6535,
	0x0604,0x0700,0x0808,0x090c,0x0a0c,0x0b08,0x0c0c,0x0d08,0x0e08,0x0f0c,0x1010,0x1114,0x1214,0x1310,0x1414,0x1510,
	0x1600,0x1704,0x180c,0x1908,0x1a08,0x1b0c,0x1c08,0x1d0c,0x1e0c,0x1f08,0x2030,0x2134,0x2234,0x2330,0x2434,0x2530,
	0x2620,0x2724,0x282c,0x2928,0x2a28,0x2b2c,0x2c28,0x2d2c,0x2e2c,0x2f28,0x3034,0x3130,0x3230,0x3334,0x3430,0x3534,
	0x3624,0x3720,0x3828,0x392c,0x3a2c,0x3b28,0x3c2c,0x3d28,0x3e28,0x3f2c,0x4010,0x4114,0x4214,0x4310,0x4414,0x4510,
	0x4600,0x4704,0x480c,0x4908,0x4a08,0x4b0c,0x4c08,0x4d0c,0x4e0c,0x4f08,0x5014,0x5110,0x5210,0x5314,0x5410,0x5514,
	0x5604,0x5700,0x5808,0x590c,0x5a0c,0x5b08,0x5c0c,0x5d08,0x5e08,0x5f0c,0x6034,0x6130,0x6230,0x6334,0x6430,0x6534,
	0x6624,0x6720,0x6828,0x692c,0x6a2c,0x6b28,0x6c2c,0x6d28,0x6e28,0x6f2c,0x7030,0x7134,0x7234,0x7330,0x7434,0x7530,
	0x7620,0x7724,0x782c,0x7928,0x7a28,0x7b2c,0x7c28,0x7d2c,0x7e2c,0x7f28,0x8090,0x8194,0x8294,0x8390,0x8494,0x8590,
	0x8680,0x8784,0x888c,0x8988,0x8a88,0x8b8c,0x8c88,0x8d8c,0x8e8c,0x8f88,0x9094,0x9190,0x9290,0x9394,0x9490,0x9594,
	0x9684,0x9780,0x9888,0x998c,0x9a8c,0x9b88,0x9c8c,0x9d88,0x9e88,0x9f8c,0x0055,0x0111,0x0211,0x0315,0x0411,0x0515,
	0x0605,0x0701,0x0809,0x090d,0x0a0d,0x0b09,0x0c0d,0x0d09,0x0e09,0x0f0d,0x1011,0x1115,0x1215,0x1311,0x1415,0x1511,
	0x1601,0x1705,0x180d,0x1909,0x1a09,0x1b0d,0x1c09,0x1d0d,0x1e0d,0x1f09,0x2031,0x2135,0x2235,0x2331,0x2435,0x2531,
	0x2621,0x2725,0x282d,0x2929,0x2a29,0x2b2d,0x2c29,0x2d2d,0x2e2d,0x2f29,0x3035,0x3131,0x3231,0x3335,0x3431,0x3535,
	0x3625,0x3721,0x3829,0x392d,0x3a2d,0x3b29,0x3c2d,0x3d29,0x3e29,0x3f2d,0x4011,0x4115,0x4215,0x4311,0x4415,0x4511,
	0x4601,0x4705,0x480d,0x4909,0x4a09,0x4b0d,0x4c09,0x4d0d,0x4e0d,0x4f09,0x5015,0x5111,0x5211,0x5315,0x5411,0x5515,
	0x5605,0x5701,0x5809,0x590d,0x5a0d,0x5b09,0x5c0d,0x5d09,0x5e09,0x5f0d,0x6035,0x6131,0x6231,0x6335,0x6431,0x6535,
	0x6625,0x6721,0x6829,0x692d,0x6a2d,0x6b29,0x6c2d,0x6d29,0x6e29,0x6f2d,0x7031,0x7135,0x7235,0x7331,0x7435,0x7531,
	0x7621,0x7725,0x782d,0x7929,0x7a29,0x7b2d,0x7c29,0x7d2d,0x7e2d,0x7f29,0x8091,0x8195,0x8295,0x8391,0x8495,0x8591,
	0x8681,0x8785,0x888d,0x8989,0x8a89,0x8b8d,0x8c89,0x8d8d,0x8e8d,0x8f89,0x9095,0x9191,0x9291,0x9395,0x9491,0x9595,
	0x9685,0x9781,0x9889,0x998d,0x9a8d,0x9b89,0x9c8d,0x9d89,0x9e89,0x9f8d,0xa0b5,0xa1b1,0xa2b1,0xa3b5,0xa4b1,0xa5b5,
	0xa6a5,0xa7a1,0xa8a9,0xa9ad,0xaaad,0xaba9,0xacad,0xada9,0xaea9,0xafad,0xb0b1,0xb1b5,0xb2b5,0xb3b1,0xb4b5,0xb5b1,
	0xb6a1,0xb7a5,0xb8ad,0xb9a9,0xbaa9,0xbbad,0xbca9,0xbdad,0xbead,0xbfa9,0xc095,0xc191,0xc291,0xc395,0xc491,0xc595,
	0xc685,0xc781,0xc889,0xc98d,0xca8d,0xcb89,0xcc8d,0xcd89,0xce89,0xcf8d,0xd091,0xd195,0xd295,0xd391,0xd495,0xd591,
	0xd681,0xd785,0xd88d,0xd989,0xda89,0xdb8d,0xdc89,0xdd8d,0xde8d,0xdf89,0xe0b1,0xe1b5,0xe2b5,0xe3b1,0xe4b5,0xe5b1,
	0xe6a1,0xe7a5,0xe8ad,0xe9a9,0xeaa9,0xebad,0xeca9,0xedad,0xeead,0xefa9,0xf0b5,0xf1b1,0xf2b1,0xf3b5,0xf4b1,0xf5b5,
	0xf6a5,0xf7a1,0xf8a9,0xf9ad,0xfaad,0xfba9,0xfcad,0xfda9,0xfea9,0xffad,0x0055,0x0111,0x0211,0x0315,0x0411,0x0515,
	0x0605,0x0701,0x0809,0x090d,0x0a0d,0x0b09,0x0c0d,0x0d09,0x0e09,0x0f0d,0x1011,0x1115,0x1215,0x1311,0x1415,0x1511,
	0x1601,0x1705,0x180d,0x1909,0x1a09,0x1b0d,0x1c09,0x1d0d,0x1e0d,0x1f09,0x2031,0x2135,0x2235,0x2331,0x2435,0x2531,
	0x2621,0x2725,0x282d,0x2929,0x2a29,0x2b2d,0x2c29,0x2d2d,0x2e2d,0x2f29,0x3035,0x3131,0x3231,0x3335,0x3431,0x3535,
	0x3625,0x3721,0x3829,0x392d,0x3a2d,0x3b29,0x3c2d,0x3d29,0x3e29,0x3f2d,0x4011,0x4115,0x4215,0x4311,0x4415,0x4511,
	0x4601,0x4705,0x480d,0x4909,0x4a09,0x4b0d,0x4c09,0x4d0d,0x4e0d,0x4f09,0x5015,0x5111,0x5211,0x5315,0x5411,0x5515,
	0x5605,0x5701,0x5809,0x590d,0x5a0d,0x5b09,0x5c0d,0x5d09,0x5e09,0x5f0d,0x6035,0x6131,0x6231,0x6335,0x6431,0x6535,
	0x0046,0x0102,0x0202,0x0306,0x0402,0x0506,0x0606,0x0702,0x080a,0x090e,0x0402,0x0506,0x0606,0x0702,0x080a,0x090e,
	0x1002,0x1106,0x1206,0x1302,0x1406,0x1502,0x1602,0x1706,0x180e,0x190a,0x1406,0x1502,0x1602,0x1706,0x180e,0x190a,
	0x2022,0x2126,0x2226,0x2322,0x2426,0x2522,0x2622,0x2726,0x282e,0x292a,0x2426,0x2522,0x2622,0x2726,0x282e,0x292a,
	0x3026,0x3122,0x3222,0x3326,0x3422,0x3526,0x3626,0x3722,0x382a,0x392e,0x3422,0x3526,0x3626,0x3722,0x382a,0x392e,
	0x4002,0x4106,0x4206,0x4302,0x4406,0x4502,0x4602,0x4706,0x480e,0x490a,0x4406,0x4502,0x4602,0x4706,0x480e,0x490a,
	0x5006,0x5102,0x5202,0x5306,0x5402,0x5506,0x5606,0x5702,0x580a,0x590e,0x5402,0x5506,0x5606,0x5702,0x580a,0x590e,
	0x6026,0x6122,0x6222,0x6326,0x6422,0x6526,0x6626,0x6722,0x682a,0x692e,0x6422,0x6526,0x6626,0x6722,0x682a,0x692e,
	0x7022,0x7126,0x7226,0x7322,0x7426,0x7522,0x7622,0x7726,0x782e,0x792a,0x7426,0x7522,0x7622,0x7726,0x782e,0x792a,
	0x8082,0x8186,0x8286,0x8382,0x8486,0x8582,0x8682,0x8786,0x888e,0x898a,0x8486,0x8582,0x8682,0x8786,0x888e,0x898a,
	0x9086,0x9182,0x9282,0x9386,0x9482,0x9586,0x9686,0x9782,0x988a,0x998e,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,
	0x4003,0x4107,0x4207,0x4303,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,
	0x5007,0x5103,0x5203,0x5307,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,
	0x6027,0x6123,0x6223,0x6327,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,
	0x7023,0x7127,0x7227,0x7323,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,
	0x8083,0x8187,0x8287,0x8383,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,
	0x9087,0x9183,0x9283,0x9387,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,
	0xa0a7,0xa1a3,0xa2a3,0xa3a7,0xa4a3,0xa5a7,0xa6a7,0xa7a3,0xa8ab,0xa9af,0xa4a3,0xa5a7,0xa6a7,0xa7a3,0xa8ab,0xa9af,
	0xb0a3,0xb1a7,0xb2a7,0xb3a3,0xb4a7,0xb5a3,0xb6a3,0xb7a7,0xb8af,0xb9ab,0xb4a7,0xb5a3,0xb6a3,0xb7a7,0xb8af,0xb9ab,
	0xc087,0xc183,0xc283,0xc387,0xc483,0xc587,0xc687,0xc783,0xc88b,0xc98f,0xc483,0xc587,0xc687,0xc783,0xc88b,0xc98f,
	0xd083,0xd187,0xd287,0xd383,0xd487,0xd583,0xd683,0xd787,0xd88f,0xd98b,0xd487,0xd583,0xd683,0xd787,0xd88f,0xd98b,
	0xe0a3,0xe1a7,0xe2a7,0xe3a3,0xe4a7,0xe5a3,0xe6a3,0xe7a7,0xe8af,0xe9ab,0xe4a7,0xe5a3,0xe6a3,0xe7a7,0xe8af,0xe9ab,
	0xf0a7,0xf1a3,0xf2a3,0xf3a7,0xf4a3,0xf5a7,0xf6a7,0xf7a3,0xf8ab,0xf9af,0xf4a3,0xf5a7,0xf6a7,0xf7a3,0xf8ab,0xf9af,
	0x0047,0x0103,0x0203,0x0307,0x0403,0x0507,0x0607,0x0703,0x080b,0x090f,0x0403,0x0507,0x0607,0x0703,0x080b,0x090f,
	0x1003,0x1107,0x1207,0x1303,0x1407,0x1503,0x1603,0x1707,0x180f,0x190b,0x1407,0x1503,0x1603,0x1707,0x180f,0x190b,
	0x2023,0x2127,0x2227,0x2323,0x2427,0x2523,0x2623,0x2727,0x282f,0x292b,0x2427,0x2523,0x2623,0x2727,0x282f,0x292b,
	0x3027,0x3123,0x3223,0x3327,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,
	0x4003,0x4107,0x4207,0x4303,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,
	0x5007,0x5103,0x5203,0x5307,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,
	0x6027,0x6123,0x6223,0x6327,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,
	0x7023,0x7127,0x7227,0x7323,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,
	0x8083,0x8187,0x8287,0x8383,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,
	0x9087,0x9183,0x9283,0x9387,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,
	0xfabe,0xfbba,0xfcbe,0xfdba,0xfeba,0xffbe,0x0046,0x0102,0x0202,0x0306,0x0402,0x0506,0x0606,0x0702,0x080a,0x090e,
	0x0a1e,0x0b1a,0x0c1e,0x0d1a,0x0e1a,0x0f1e,0x1002,0x1106,0x1206,0x1302,0x1406,0x1502,0x1602,0x1706,0x180e,0x190a,
	0x1a1a,0x1b1e,0x1c1a,0x1d1e,0x1e1e,0x1f1a,0x2022,0x2126,0x2226,0x2322,0x2426,0x2522,0x2622,0x2726,0x282e,0x292a,
	0x2a3a,0x2b3e,0x2c3a,0x2d3e,0x2e3e,0x2f3a,0x3026,0x3122,0x3222,0x3326,0x3422,0x3526,0x3626,0x3722,0x382a,0x392e,
	0x3a3e,0x3b3a,0x3c3e,0x3d3a,0x3e3a,0x3f3e,0x4002,0x4106,0x4206,0x4302,0x4406,0x4502,0x4602,0x4706,0x480e,0x490a,
	0x4a1a,0x4b1e,0x4c1a,0x4d1e,0x4e1e,0x4f1a,0x5006,0x5102,0x5202,0x5306,0x5402,0x5506,0x5606,0x5702,0x580a,0x590e,
	0x5a1e,0x5b1a,0x5c1e,0x5d1a,0x5e1a,0x5f1e,0x6026,0x6122,0x6222,0x6326,0x6422,0x6526,0x6626,0x6722,0x682a,0x692e,
	0x6a3e,0x6b3a,0x6c3e,0x6d3a,0x6e3a,0x6f3e,0x7022,0x7126,0x7226,0x7322,0x7426,0x7522,0x7622,0x7726,0x782e,0x792a,
	0x7a3a,0x7b3e,0x7c3a,0x7d3e,0x7e3e,0x7f3a,0x8082,0x8186,0x8286,0x8382,0x8486,0x8582,0x8682,0x8786,0x888e,0x898a,
	0x8a9a,0x8b9e,0x8c9a,0x8d9e,0x8e9e,0x8f9a,0x9086,0x9182,0x9282,0x9386,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,
	0x3a3f,0x3b3b,0x3c3f,0x3d3b,0x3e3b,0x3f3f,0x4003,0x4107,0x4207,0x4303,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,
	0x4a1b,0x4b1f,0x4c1b,0x4d1f,0x4e1f,0x4f1b,0x5007,0x5103,0x5203,0x5307,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,
	0x5a1f,0x5b1b,0x5c1f,0x5d1b,0x5e1b,0x5f1f,0x6027,0x6123,0x6223,0x6327,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,
	0x6a3f,0x6b3b,0x6c3f,0x6d3b,0x6e3b,0x6f3f,0x7023,0x7127,0x7227,0x7323,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,
	0x7a3b,0x7b3f,0x7c3b,0x7d3f,0x7e3f,0x7f3b,0x8083,0x8187,0x8287,0x8383,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,
	0x8a9b,0x8b9f,0x8c9b,0x8d9f,0x8e9f,0x8f9b,0x9087,0x9183,0x9283,0x9387,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,
	0x9a9f,0x9b9b,0x9c9f,0x9d9b,0x9e9b,0x9f9f,0xa0a7,0xa1a3,0xa2a3,0xa3a7,0xa4a3,0xa5a7,0xa6a7,0xa7a3,0xa8ab,0xa9af,
	0xaabf,0xabbb,0xacbf,0xadbb,0xaebb,0xafbf,0xb0a3,0xb1a7,0xb2a7,0xb3a3,0xb4a7,0xb5a3,0xb6a3,0xb7a7,0xb8af,0xb9ab,
	0xbabb,0xbbbf,0xbcbb,0xbdbf,0xbebf,0xbfbb,0xc087,0xc183,0xc283,0xc387,0xc483,0xc587,0xc687,0xc783,0xc88b,0xc98f,
	0xca9f,0xcb9b,0xcc9f,0xcd9b,0xce9b,0xcf9f,0xd083,0xd187,0xd287,0xd383,0xd487,0xd583,0xd683,0xd787,0xd88f,0xd98b,
	0xda9b,0xdb9f,0xdc9b,0xdd9f,0xde9f,0xdf9b,0xe0a3,0xe1a7,0xe2a7,0xe3a3,0xe4a7,0xe5a3,0xe6a3,0xe7a7,0xe8af,0xe9ab,
	0xeabb,0xebbf,0xecbb,0xedbf,0xeebf,0xefbb,0xf0a7,0xf1a3,0xf2a3,0xf3a7,0xf4a3,0xf5a7,0xf6a7,0xf7a3,0xf8ab,0xf9af,
	0xfabf,0xfbbb,0xfcbf,0xfdbb,0xfebb,0xffbf,0x0047,0x0103,0x0203,0x0307,0x0403,0x0507,0x0607,0x0703,0x080b,0x090f,
	0x0a1f,0x0b1b,0x0c1f,0x0d1b,0x0e1b,0x0f1f,0x1003,0x1107,0x1207,0x1303,0x1407,0x1503,0x1603,0x1707,0x180f,0x190b,
	0x1a1b,0x1b1f,0x1c1b,0x1d1f,0x1e1f,0x1f1b,0x2023,0x2127,0x2227,0x2323,0x2427,0x2523,0x2623,0x2727,0x282f,0x292b,
	0x2a3b,0x2b3f,0x2c3b,0x2d3f,0x2e3f,0x2f3b,0x3027,0x3123,0x3223,0x3327,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,
	0x3a3f,0x3b3b,0x3c3f,0x3d3b,0x3e3b,0x3f3f,0x4003,0x4107,0x4207,0x4303,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,
	0x4a1b,0x4b1f,0x4c1b,0x4d1f,0x4e1f,0x4f1b,0x5007,0x5103,0x5203,0x5307,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,
	0x5a1f,0x5b1b,0x5c1f,0x5d1b,0x5e1b,0x5f1f,0x6027,0x6123,0x6223,0x6327,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,
	0x6a3f,0x6b3b,0x6c3f,0x6d3b,0x6e3b,0x6f3f,0x7023,0x7127,0x7227,0x7323,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,
	0x7a3b,0x7b3f,0x7c3b,0x7d3f,0x7e3f,0x7f3b,0x8083,0x8187,0x8287,0x8383,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,
	0x8a9b,0x8b9f,0x8c9b,0x8d9f,0x8e9f,0x8f9b,0x9087,0x9183,0x9283,0x9387,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f
};
static const uint8 irep_tmp[4][4] = {
	{0,0,1,0},{0,1,0,1},{1,0,1,1},{0,1,1,0}
};
static const uint8 drep_tmp[4][4] = {
	{0,1,0,0},{1,0,0,1},{0,0,1,0},{0,1,0,1}
};
static const uint8 breg_tmp[256] = {
	0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1
};
static const uint8 SZ[256] = {
	0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8
};
static const uint8 SZ_BIT[256] = {
	0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8
};
static const uint8 SZP[256] = {
	0x44,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
	0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
	0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
	0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
	0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
	0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
	0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
	0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
	0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
	0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac
};
static const uint8 SZHV_inc[256] = {
	0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
	0x94,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0x90,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0xb0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0xb0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0x90,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0x90,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
	0xb0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
	0xb0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,
};
static const uint8 SZHV_dec[256] = {
	0x42,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x1a,
	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x1a,
	0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x3a,
	0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x3a,
	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x1a,
	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x1a,
	0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x3a,
	0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x3e,
	0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x9a,
	0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x9a,
	0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xba,
	0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xba,
	0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x9a,
	0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x9a,
	0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xba,
	0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xa2,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xba
};

// opecode definitions

#define EAX() { \
	int res = FETCH8(); \
	EA = IX + ((res < 128) ? res : res - 256); \
}

#define EAY() { \
	int res = FETCH8(); \
	EA = IY + ((res < 128) ? res : res - 256); \
}

#define JP() { \
	PC = RM16(PC); \
}

#define JP_COND(cond) { \
	if(cond) { \
		PC = RM16(PC); \
	} \
	else { \
		PC += 2; \
	} \
}

#define JR() { \
	int res = FETCH8(); \
	PC += (res < 128) ? res : res - 256; \
}

#define JR_COND(cond, opcode) { \
	if(cond) { \
		int res = FETCH8(); \
		PC += (res < 128) ? res : res - 256; \
		count -= cc_ex[opcode]; \
	} \
	else { \
		PC++; \
	} \
}

#define CALL() { \
	EA = FETCH16(); \
	PUSH16(PC); \
	PC = EA; \
}

#define CALL_COND(cond, opcode) { \
	if(cond) { \
		EA = FETCH16(); \
		PUSH16(PC); \
		PC = EA; \
		count -= cc_ex[opcode]; \
	} \
	else { \
		PC += 2; \
	} \
}

#define RET() { \
	PC = POP16(); \
}

#define RET_COND(cond, opcode) { \
	if(cond) { \
		PC = POP16(); \
		count -= cc_ex[opcode]; \
	} \
}

#define DI() { \
	IFF1 = IFF2 = 0; \
}

#define EI() { \
	IFF1 = IFF2 = 1; \
}

#define RST(addr) { \
	PUSH16(PC); \
	PC = addr; \
}

#define RETN() { \
	PC = POP16(); \
	IFF1 = IFF2; \
}

#define RETI() { \
	PC = POP16(); \
	IFF1 = IFF2; \
	NOTIFY_RETI(); \
}

#define EX_AF() { \
	uint16 tmp; \
	tmp = AF; AF = exAF; exAF = tmp; \
}

#define EX_DE_HL() { \
	uint16 tmp; \
	tmp = DE; DE = HL; HL = tmp; \
}

#define EXX() { \
	uint16 tmp; \
	tmp = BC; BC = exBC; exBC = tmp; \
	tmp = DE; DE = exDE; exDE = tmp; \
	tmp = HL; HL = exHL; exHL = tmp; \
}

inline uint16 Z80::EXSP(uint16 reg) {
	uint16 res = RM16(SP);
	WM16(SP, reg);
	return res;
}

inline uint8 Z80::INC(uint8 value) {
	uint8 res = value + 1;
	_F = (_F & CF) | SZHV_inc[res];
	return res;
}

inline uint8 Z80::DEC(uint8 value) {
	uint8 res = value - 1;
	_F = (_F & CF) | SZHV_dec[res];
	return res;
}

#define ADD(value) { \
	uint16 val = value; \
	uint16 res = _A + val; \
	_F = SZ[res & 0xff] | ((res >> 8) & CF) | ((_A ^ res ^ val) & HF) | (((val ^ _A ^ 0x80) & (val ^ res) & 0x80) >> 5); \
	_A = (uint8)res; \
}

#define ADC(value) { \
	uint16 val = value; \
	uint16 res = _A + val + (_F & CF); \
	_F = SZ[res & 0xff] | ((res >> 8) & CF) | ((_A ^ res ^ val) & HF) | (((val ^ _A ^ 0x80) & (val ^ res) & 0x80) >> 5); \
	_A = (uint8)res; \
}

#define SUB(value) { \
	uint16 val = value; \
	uint16 res = _A - val; \
	_F = SZ[res & 0xff] | ((res >> 8) & CF) | NF | ((_A ^ res ^ val) & HF) | (((val ^ _A) & (_A ^ res) & 0x80) >> 5); \
	_A = (uint8)res; \
}

#define SBC(value) { \
	uint16 val = value; \
	uint16 res = _A - val - (_F & CF); \
	_F = SZ[res & 0xff] | ((res >> 8) & CF) | NF | ((_A ^ res ^ val) & HF) | (((val ^ _A) & (_A ^ res) & 0x80) >> 5); \
	_A = (uint8)res; \
}

inline uint16 Z80::ADD16(uint16 dreg, uint16 sreg) {
	uint32 res = dreg + sreg;
	_F = (uint8)((_F & (SF | ZF | VF)) | (((dreg ^ res ^ sreg) >> 8) & HF) | ((res >> 16) & CF) | ((res >> 8) & (YF | XF)));
	return (uint16)res;
}

#define ADC16(reg) { \
	uint32 res = HL + reg + (_F & CF); \
	_F = (uint8)((((HL ^ res ^ reg) >> 8) & HF) | ((res >> 16) & CF) | ((res >> 8) & (SF | YF | XF)) | ((res & 0xffff) ? 0 : ZF) | (((reg ^ HL ^ 0x8000) & (reg ^ res) & 0x8000) >> 13)); \
	HL = (uint16)res; \
}

#define SBC16(reg) { \
	uint32 res = HL - reg - (_F & CF); \
	_F = (uint8)((((HL ^ res ^ reg) >> 8) & HF) | NF | ((res >> 16) & CF) | ((res >> 8) & (SF | YF | XF)) | ((res & 0xffff) ? 0 : ZF) | (((reg ^ HL) & (HL ^ res) &0x8000) >> 13)); \
	HL = (uint16)res; \
}

#define NEG() { \
	uint8 value = _A; \
	_A = 0; \
	SUB(value); \
}

#define DAA() { \
	uint16 idx = _A; \
	if(_F & CF) idx |= 0x100; \
	if(_F & HF) idx |= 0x200; \
	if(_F & NF) idx |= 0x400; \
	AF = DAATable[idx]; \
}

#define AND(value) { \
	_A &= value; \
	_F = SZP[_A] | HF; \
}

#define OR(value) { \
	_A |= value; \
	_F = SZP[_A]; \
}

#define XOR(value) { \
	_A ^= value; \
	_F = SZP[_A]; \
}

#define CP(value) { \
	uint16 val = value; \
	uint16 res = _A - val; \
	_F = (SZ[res & 0xff] & (SF | ZF)) | (val & (YF | XF)) | ((res >> 8) & CF) | NF | ((_A ^ res ^ val) & HF) | ((((val ^ _A) & (_A ^ res)) >> 5) & VF); \
}

#define RLCA() { \
	_A = (_A << 1) | (_A >> 7); \
	_F = (_F & (SF | ZF | PF)) | (_A & (YF | XF | CF)); \
}

#define RRCA() { \
	_F = (_F & (SF | ZF | PF)) | (_A & CF); \
	_A = (_A >> 1) | (_A << 7); \
	_F |= (_A & (YF | XF)); \
}

#define RLA() { \
	uint8 res = (_A << 1) | (_F & CF); \
	uint8 c = (_A & 0x80) ? CF : 0; \
	_F = (_F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
	_A = res; \
}

#define RRA() { \
	uint8 res = (_A >> 1) | (_F << 7); \
	uint8 c = (_A & 0x01) ? CF : 0; \
	_F = (_F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
	_A = res; \
}

#define RRD() { \
	uint8 n = RM8(HL); \
	WM8(HL, (n >> 4) | (_A << 4)); \
	_A = (_A & 0xf0) | (n & 0x0f); \
	_F = (_F & CF) | SZP[_A]; \
}

#define RLD() { \
	uint8 n = RM8(HL); \
	WM8(HL, (n << 4) | (_A & 0x0f)); \
	_A = (_A & 0xf0) | (n >> 4); \
	_F = (_F & CF) | SZP[_A]; \
}

inline uint8 Z80::RLC(uint8 value) {
	uint8 res = value;
	uint8 c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

inline uint8 Z80::RRC(uint8 value) {
	uint8 res = value;
	uint8 c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res << 7)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

inline uint8 Z80::RL(uint8 value) {
	uint8 res = value;
	uint8 c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (_F & CF)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

inline uint8 Z80::RR(uint8 value) {
	uint8 res = value;
	uint8 c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (_F << 7)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

inline uint8 Z80::SLA(uint8 value) {
	uint8 res = value;
	uint8 c = (res & 0x80) ? CF : 0;
	res = (res << 1) & 0xff;
	_F = SZP[res] | c;
	return res;
}

inline uint8 Z80::SRA(uint8 value) {
	uint8 res = value;
	uint8 c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res & 0x80)) & 0xff;
	_F = SZP[res] | c;
	return res;
}

inline uint8 Z80::SLL(uint8 value) {
	uint8 res = value;
	uint8 c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | 0x01) & 0xff;
	_F = SZP[res] | c;
	return res;
}

inline uint8 Z80::SRL(uint8 value) {
	uint8 res = value;
	uint8 c = (res & 0x01) ? CF : 0;
	res = (res >> 1) & 0xff;
	_F = SZP[res] | c;
	return res;
}

#define BIT(bit, reg) { \
	_F = (_F & CF) | HF | SZ_BIT[reg & (1 << bit)]; \
}

#define BIT_XY(bit, reg) { \
	_F = (_F & CF) | HF | (SZ_BIT[reg & (1 << bit)] & ~(YF | XF)) | ((EA >> 8) & (YF | XF)); \
}

inline uint8 Z80::RES(uint8 bit, uint8 value) {
	return value & ~(1 << bit);
}

inline uint8 Z80::SET(uint8 bit, uint8 value) {
	return value | (1 << bit);
}

#define LDI() { \
	uint8 io = RM8(HL); \
	WM8(DE, io); \
	_F &= SF | ZF | CF; \
	if((_A + io) & 0x02) _F |= YF; \
	if((_A + io) & 0x08) _F |= XF; \
	HL++; DE++; BC--; \
	if(BC) _F |= VF; \
}

#define CPI() { \
	uint8 val = RM8(HL); \
	uint8 res = _A - val; \
	HL++; BC--; \
	_F = (_F & CF) | (SZ[res] & ~(YF | XF)) | ((_A ^ val ^ res) & HF) | NF; \
	if(_F & HF) res -= 1; \
	if(res & 0x02) _F |= YF; \
	if(res & 0x08) _F |= XF; \
	if(BC) _F |= VF; \
}

#define INI() { \
	uint8 io = IN8(_C, _B); \
	_B--; \
	WM8(HL, io); \
	HL++; \
	_F = SZ[_B]; \
	if(io & SF) _F |= NF; \
	if((((_C + 1) & 0xff) + io) & 0x100) _F |= HF | CF; \
	if((irep_tmp[_C & 3][io & 3] ^ breg_tmp[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) _F |= PF; \
}

#define OUTI() { \
	uint8 io = RM8(HL); \
	_B--; \
	OUT8(_C, _B, io); \
	HL++; \
	_F = SZ[_B]; \
	if(io & SF) _F |= NF; \
	if((((_C + 1) & 0xff) + io) & 0x100) _F |= HF | CF; \
	if((irep_tmp[_C & 3][io & 3] ^ breg_tmp[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) _F |= PF; \
}

#define LDD() { \
	uint8 io = RM8(HL); \
	WM8(DE, io); \
	_F &= SF | ZF | CF; \
	if((_A + io) & 0x02) _F |= YF; \
	if((_A + io) & 0x08) _F |= XF; \
	HL--; DE--; BC--; \
	if(BC) _F |= VF; \
}

#define CPD() { \
	uint8 val = RM8(HL); \
	uint8 res = _A - val; \
	HL--; BC--; \
	_F = (_F & CF) | (SZ[res] & ~(YF | XF)) | ((_A ^ val ^ res) & HF) | NF; \
	if(_F & HF) res -= 1; \
	if(res & 0x02) _F |= YF; \
	if(res & 0x08) _F |= XF; \
	if(BC) _F |= VF; \
}

#define IND() { \
	uint8 io = IN8(_C, _B); \
	_B--; \
	WM8(HL, io); \
	HL--; \
	_F = SZ[_B]; \
	if(io & SF) _F |= NF; \
	if((((_C - 1) & 0xff) + io) & 0x100) _F |= HF | CF; \
	if((drep_tmp[_C & 3][io & 3] ^ breg_tmp[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) _F |= PF; \
}

#define OUTD() { \
	uint8 io = RM8(HL); \
	_B--; \
	OUT8(_C, _B, io); \
	HL--; \
	_F = SZ[_B]; \
	if(io & SF) _F |= NF; \
	if((((_C - 1) & 0xff) + io) & 0x100) _F |= HF | CF; \
	if((drep_tmp[_C & 3][io & 3] ^ breg_tmp[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) _F |= PF; \
}

#define LDIR() { \
	LDI(); \
	if(BC) { \
		PC -= 2; \
		count -= cc_ex[0xb0]; \
	} \
}

#define CPIR() { \
	CPI(); \
	if(BC && !(_F & ZF)) { \
		PC -= 2; \
		count -= cc_ex[0xb1]; \
	} \
}

#define INIR() { \
	INI(); \
	if(_B) { \
		PC -= 2; \
		count -= cc_ex[0xb2]; \
	} \
}

#define OTIR() { \
	OUTI(); \
	if(_B) { \
		PC -= 2; \
		count -= cc_ex[0xb3]; \
	} \
}

#define LDDR() { \
	LDD(); \
	if(BC) { \
		PC -= 2; \
		count -= cc_ex[0xb8]; \
	} \
}

#define CPDR() { \
	CPD(); \
	if(BC && !(_F & ZF)) { \
		PC -= 2; \
		count -= cc_ex[0xb9]; \
	} \
}

#define INDR() { \
	IND(); \
	if(_B) { \
		PC -= 2; \
		count -= cc_ex[0xba]; \
	} \
}

#define OTDR() { \
	OUTD(); \
	if(_B) { \
		PC -= 2; \
		count -= cc_ex[0xbb]; \
	} \
}

#ifdef HAS_NSC800
#define NSC800_INT(v) { \
	if(halt) { \
		PC++; halt = 0; \
	} \
	PUSH16(PC); PC = (v); count -= 7; IFF1 = IFF2 = 0; \
}
#endif

// main

void Z80::reset()
{
	// reset
	PC = CPU_START_ADDR;
	SP = 0;
	_F = _I = _R = 0;
	IM = IFF1 = IFF2 = ICR = 0;
	halt = false;
	intr_req_bit = intr_pend_bit = 0;
#ifdef _CPU_DEBUG_LOG
	debug_count = 0;
#endif
}

void Z80::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CPU_IRQ) {
		intr_req_bit = (intr_req_bit & ~mask) | (data & mask);
		// always pending (temporary)
		intr_pend_bit = (intr_pend_bit & ~mask) | (data & mask);
	}
	else if(id == SIG_CPU_NMI) {
		intr_req_bit = (data & mask) ? (intr_req_bit | NMI_REQ_BIT) : (intr_req_bit & ~NMI_REQ_BIT);
	}
	else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
		if(busreq) {
			count = first = 0;
		}
		// busack
		write_signals(&outputs_busack, busreq ? 0xffffffff : 0);
	}
#ifdef Z80_M1_CYCLE_WAIT
	else if(id == SIG_Z80_M1_CYCLE_WAIT) {
		m1_cycle_wait = data & mask;
	}
#endif
#ifdef HAS_NSC800
	else if(id == SIG_NSC800_INT) {
		intr_req_bit = (data & mask) ? (intr_req_bit | 1) : (intr_req_bit & ~1);
	}
	else if(id == SIG_NSC800_RSTA) {
		intr_req_bit = (data & mask) ? (intr_req_bit | 8) : (intr_req_bit & ~8);
	}
	else if(id == SIG_NSC800_RSTB) {
		intr_req_bit = (data & mask) ? (intr_req_bit | 4) : (intr_req_bit & ~4);
	}
	else if(id == SIG_NSC800_RSTC) {
		intr_req_bit = (data & mask) ? (intr_req_bit | 2) : (intr_req_bit & ~2);
	}
#endif
#ifdef _CPU_DEBUG_LOG
	else if(id == SIG_CPU_DEBUG) {
		if(debug_count == 0) {
			emu->out_debug(_T("---- Z80 DASM ---------------------------------------------------------------->\n"));
		}
		debug_count = 16;
	}
#endif
}

void Z80::run(int clock)
{
	// return now if BUSREQ
	if(busreq) {
		count = first = 0;
		return;
	}
	
	// run cpu while given clocks
	count += clock;
	first = count;
	
	while(count > 0) {
		uint8 code = FETCHOP();
		OP(code);
		if(code == 0xfb) {
			// ei: run next opecode
			OP(FETCHOP());
		}
		if(intr_req_bit) {
			if(intr_req_bit & NMI_REQ_BIT) {
				// nmi
#ifdef _CPU_DEBUG_LOG
				if(debug_count) {
					emu->out_debug(_T("%4x\tNMI\n"), PC);
				}
#endif
				if(halt) {
					PC++;
					halt = false;
				}
				PUSH16(PC);
				PC = 0x0066;
				count -= 5;
				IFF1 = 0;
				intr_req_bit &= ~NMI_REQ_BIT;
			}
#ifdef HAS_NSC800
			else if((intr_req_bit & 1) && (ICR & 1)) {
				// INTR
				uint8 vector = ACK_INTR();
				NSC800_INT(vector);
				intr_req_bit &= ~1;
			}
			else if((intr_req_bit & 8) && (ICR & 8)) {
				// RSTA
				NSC800_INT(0x3c);
				intr_req_bit &= ~8;
			}
			else if((intr_req_bit & 4) && (ICR & 4)) {
				// RSTB
				NSC800_INT(0x34);
				intr_req_bit &= ~4;
			}
			else if((intr_req_bit & 2) && (ICR & 2)) {
				// RSTC
				NSC800_INT(0x2c);
				intr_req_bit &= ~2;
			}
#else
			else if(IFF1) {
				// interrupt
				if(halt) {
					PC++;
					halt = false;
				}
				uint32 vector = ACK_INTR();
				uint8 v0 = vector;
				uint16 v12 = vector >> 8;
#ifdef _CPU_DEBUG_LOG
				if(debug_count) {
					emu->out_debug(_T("%4x\tIRQ VECTOR=%2x\n"), PC, v0);
				}
#endif
				if(IM == 0) {
					// mode 0 (support CALL/RST only)
					PUSH16(PC);
					switch(v0) {
					case 0xcd:		// CALL
						PC = v12;
						break;
					case 0xc7:		// RST 00H
						PC = 0x0000;
						break;
					case 0xcf:		// RST 08H
						PC = 0x0008;
						break;
					case 0xd7:		// RST 10H
						PC = 0x0010;
						break;
					case 0xdf:		// RST 18H
						PC = 0x0018;
						break;
					case 0xe7:		// RST 20H
						PC = 0x0020;
						break;
					case 0xef:		// RST 28H
						PC = 0x0028;
						break;
					case 0xf7:		// RST 30H
						PC = 0x0030;
						break;
					case 0xff:		// RST 38H
						PC = 0x0038;
						break;
					}
					count -= 7;
				}
				else if(IM == 1) {
					// mode 1
					PUSH16(PC);
					PC = 0x0038;
					count -= 7;
				}
				else {
					// mode 2
					PUSH16(PC);
					PC = RM16((_I << 8) | v0);
					count -= 7;
				}
#ifdef _X1TURBO
				// hack for X1turbo2 demonstration :-(
				if(IM == 2 && RM8(PC) == 0xed && RM8(PC + 1) == 0x4d) {
					IFF1 = 0;
				}
				else
#endif
				IFF1 = IFF2 = 0;
				intr_req_bit = 0;
			}
			else {
				intr_req_bit &= intr_pend_bit;
			}
#endif
		}
#ifdef SINGLE_MODE_DMA
		if(d_dma) {
			d_dma->do_dma();
		}
#endif
	}
	first = count;
}

void Z80::OP(uint8 code)
{
	prvPC = PC - 1;
	count -= cc_op[code];
	
#ifdef _CPU_DEBUG_LOG
	uint16 _AF = AF, _BC = BC, _DE = DE, _HL = HL, _IX = IX, _IY = IY, _SP = SP;
	debug_ops[0] = code;
	debug_ops[1] = RM8(prvPC + 1);
	debug_ops[2] = RM8(prvPC + 2);
	debug_ops[3] = RM8(prvPC + 3);
#endif
	
	switch(code) {
	case 0x00: // NOP
		break;
	case 0x01: // LD BC, w
		BC = FETCH16();
		break;
	case 0x02: // LD (BC), A
		WM8(BC, _A);
		break;
	case 0x03: // INC BC
		BC++;
		break;
	case 0x04: // INC B
		_B = INC(_B);
		break;
	case 0x05: // DEC B
		_B = DEC(_B);
		break;
	case 0x06: // LD B, n
		_B = FETCH8();
		break;
	case 0x07: // RLCA
		RLCA();
		break;
	case 0x08: // EX AF, AF'
		EX_AF();
		break;
	case 0x09: // ADD HL, BC
		HL = ADD16(HL, BC);
		break;
	case 0x0a: // LD A, (BC)
		_A = RM8(BC);
		break;
	case 0x0b: // DEC BC
		BC--;
		break;
	case 0x0c: // INC C
		_C = INC(_C);
		break;
	case 0x0d: // DEC C
		_C = DEC(_C);
		break;
	case 0x0e: // LD C, n
		_C = FETCH8();
		break;
	case 0x0f: // RRCA
		RRCA();
		break;
	case 0x10: // DJNZ o
		_B--;
		JR_COND(_B, 0x10);
		break;
	case 0x11: // LD DE, w
		DE = FETCH16();
		break;
	case 0x12: // LD (DE), A
		WM8(DE, _A);
		break;
	case 0x13: // INC DE
		DE++;
		break;
	case 0x14: // INC D
		_D = INC(_D);
		break;
	case 0x15: // DEC D
		_D = DEC(_D);
		break;
	case 0x16: // LD D, n
		_D = FETCH8();
		break;
	case 0x17: // RLA
		RLA();
		break;
	case 0x18: // JR o
		JR();
		break;
	case 0x19: // ADD HL, DE
		HL = ADD16(HL, DE);
		break;
	case 0x1a: // LD A, (DE)
		_A = RM8(DE);
		break;
	case 0x1b: // DEC DE
		DE--;
		break;
	case 0x1c: // INC E
		_E = INC(_E);
		break;
	case 0x1d: // DEC E
		_E = DEC(_E);
		break;
	case 0x1e: // LD E, n
		_E = FETCH8();
		break;
	case 0x1f: // RRA
		RRA();
		break;
	case 0x20: // JR NZ, o
		JR_COND(!(_F & ZF), 0x20);
		break;
	case 0x21: // LD HL, w
		HL = FETCH16();
		break;
	case 0x22: // LD (w), HL
		EA = FETCH16();
		WM16(EA, HL);
		break;
	case 0x23: // INC HL
		HL++;
		break;
	case 0x24: // INC H
		_H = INC(_H);
		break;
	case 0x25: // DEC H
		_H = DEC(_H);
		break;
	case 0x26: // LD H, n
		_H = FETCH8();
		break;
	case 0x27: // DAA
		DAA();
		break;
	case 0x28: // JR Z, o
		JR_COND(_F & ZF, 0x28);
		break;
	case 0x29: // ADD HL, HL
		HL = ADD16(HL, HL);
		break;
	case 0x2a: // LD HL, (w)
		EA = FETCH16();
		HL = RM16(EA);
		break;
	case 0x2b: // DEC HL
		HL--;
		break;
	case 0x2c: // INC L
		_L = INC(_L);
		break;
	case 0x2d: // DEC L
		_L = DEC(_L);
		break;
	case 0x2e: // LD L, n
		_L = FETCH8();
		break;
	case 0x2f: // CPL
		_A ^= 0xff;
		_F = (_F & (SF | ZF | PF | CF)) | HF | NF | (_A & (YF | XF));
		break;
	case 0x30: // JR NC, o
		JR_COND(!(_F & CF), 0x30);
		break;
	case 0x31: // LD SP, w
		SP = FETCH16();
		break;
	case 0x32: // LD (w), A
		EA = FETCH16();
		WM8(EA, _A);
		break;
	case 0x33: // INC SP
		SP++;
		break;
	case 0x34: // INC (HL)
		WM8(HL, INC(RM8(HL)));
		break;
	case 0x35: // DEC (HL)
		WM8(HL, DEC(RM8(HL)));
		break;
	case 0x36: // LD (HL), n
		WM8(HL, FETCH8());
		break;
	case 0x37: // SCF
		_F = (_F & (SF | ZF | PF)) | CF | (_A & (YF | XF));
		break;
	case 0x38: // JR C, o
		JR_COND(_F & CF, 0x38);
		break;
	case 0x39: // ADD HL, SP
		HL = ADD16(HL, SP);
		break;
	case 0x3a: // LD A, (w)
		EA = FETCH16();
		_A = RM8(EA);
		break;
	case 0x3b: // DEC SP
		SP--;
		break;
	case 0x3c: // INC A
		_A = INC(_A);
		break;
	case 0x3d: // DEC A
		_A = DEC(_A);
		break;
	case 0x3e: // LD A, n
		_A = FETCH8();
		break;
	case 0x3f: // CCF
		_F = ((_F & (SF | ZF | PF | CF)) | ((_F & CF) << 4) | (_A & (YF | XF))) ^ CF;
		break;
	case 0x40: // LD B, B
		break;
	case 0x41: // LD B, C
		_B = _C;
		break;
	case 0x42: // LD B, D
		_B = _D;
		break;
	case 0x43: // LD B, E
		_B = _E;
		break;
	case 0x44: // LD B, H
		_B = _H;
		break;
	case 0x45: // LD B, L
		_B = _L;
		break;
	case 0x46: // LD B, (HL)
		_B = RM8(HL);
		break;
	case 0x47: // LD B, A
		_B = _A;
		break;
	case 0x48: // LD C, B
		_C = _B;
		break;
	case 0x49: // LD C, C
		break;
	case 0x4a: // LD C, D
		_C = _D;
		break;
	case 0x4b: // LD C, E
		_C = _E;
		break;
	case 0x4c: // LD C, H
		_C = _H;
		break;
	case 0x4d: // LD C, L
		_C = _L;
		break;
	case 0x4e: // LD C, (HL)
		_C = RM8(HL);
		break;
	case 0x4f: // LD C, A
		_C = _A;
		break;
	case 0x50: // LD D, B
		_D = _B;
		break;
	case 0x51: // LD D, C
		_D = _C;
		break;
	case 0x52: // LD D, D
		break;
	case 0x53: // LD D, E
		_D = _E;
		break;
	case 0x54: // LD D, H
		_D = _H;
		break;
	case 0x55: // LD D, L
		_D = _L;
		break;
	case 0x56: // LD D, (HL)
		_D = RM8(HL);
		break;
	case 0x57: // LD D, A
		_D = _A;
		break;
	case 0x58: // LD E, B
		_E = _B;
		break;
	case 0x59: // LD E, C
		_E = _C;
		break;
	case 0x5a: // LD E, D
		_E = _D;
		break;
	case 0x5b: // LD E, E
		break;
	case 0x5c: // LD E, H
		_E = _H;
		break;
	case 0x5d: // LD E, L
		_E = _L;
		break;
	case 0x5e: // LD E, (HL)
		_E = RM8(HL);
		break;
	case 0x5f: // LD E, A
		_E = _A;
		break;
	case 0x60: // LD H, B
		_H = _B;
		break;
	case 0x61: // LD H, C
		_H = _C;
		break;
	case 0x62: // LD H, D
		_H = _D;
		break;
	case 0x63: // LD H, E
		_H = _E;
		break;
	case 0x64: // LD H, H
		break;
	case 0x65: // LD H, L
		_H = _L;
		break;
	case 0x66: // LD H, (HL)
		_H = RM8(HL);
		break;
	case 0x67: // LD H, A
		_H = _A;
		break;
	case 0x68: // LD L, B
		_L = _B;
		break;
	case 0x69: // LD L, C
		_L = _C;
		break;
	case 0x6a: // LD L, D
		_L = _D;
		break;
	case 0x6b: // LD L, E
		_L = _E;
		break;
	case 0x6c: // LD L, H
		_L = _H;
		break;
	case 0x6d: // LD L, L
		break;
	case 0x6e: // LD L, (HL)
		_L = RM8(HL);
		break;
	case 0x6f: // LD L, A
		_L = _A;
		break;
	case 0x70: // LD (HL), B
		WM8(HL, _B);
		break;
	case 0x71: // LD (HL), C
		WM8(HL, _C);
		break;
	case 0x72: // LD (HL), D
		WM8(HL, _D);
		break;
	case 0x73: // LD (HL), E
		WM8(HL, _E);
		break;
	case 0x74: // LD (HL), H
		WM8(HL, _H);
		break;
	case 0x75: // LD (HL), L
		WM8(HL, _L);
		break;
	case 0x76: // HALT
		PC--;
		halt = true;
		break;
	case 0x77: // LD (HL), A
		WM8(HL, _A);
		break;
	case 0x78: // LD A, B
		_A = _B;
		break;
	case 0x79: // LD A, C
		_A = _C;
		break;
	case 0x7a: // LD A, D
		_A = _D;
		break;
	case 0x7b: // LD A, E
		_A = _E;
		break;
	case 0x7c: // LD A, H
		_A = _H;
		break;
	case 0x7d: // LD A, L
		_A = _L;
		break;
	case 0x7e: // LD A, (HL)
		_A = RM8(HL);
		break;
	case 0x7f: // LD A, A
		break;
	case 0x80: // ADD A, B
		ADD(_B);
		break;
	case 0x81: // ADD A, C
		ADD(_C);
		break;
	case 0x82: // ADD A, D
		ADD(_D);
		break;
	case 0x83: // ADD A, E
		ADD(_E);
		break;
	case 0x84: // ADD A, H
		ADD(_H);
		break;
	case 0x85: // ADD A, L
		ADD(_L);
		break;
	case 0x86: // ADD A, (HL)
		ADD(RM8(HL));
		break;
	case 0x87: // ADD A, A
		ADD(_A);
		break;
	case 0x88: // ADC A, B
		ADC(_B);
		break;
	case 0x89: // ADC A, C
		ADC(_C);
		break;
	case 0x8a: // ADC A, D
		ADC(_D);
		break;
	case 0x8b: // ADC A, E
		ADC(_E);
		break;
	case 0x8c: // ADC A, H
		ADC(_H);
		break;
	case 0x8d: // ADC A, L
		ADC(_L);
		break;
	case 0x8e: // ADC A, (HL)
		ADC(RM8(HL));
		break;
	case 0x8f: // ADC A, A
		ADC(_A);
		break;
	case 0x90: // SUB B
		SUB(_B);
		break;
	case 0x91: // SUB C
		SUB(_C);
		break;
	case 0x92: // SUB D
		SUB(_D);
		break;
	case 0x93: // SUB E
		SUB(_E);
		break;
	case 0x94: // SUB H
		SUB(_H);
		break;
	case 0x95: // SUB L
		SUB(_L);
		break;
	case 0x96: // SUB (HL)
		SUB(RM8(HL));
		break;
	case 0x97: // SUB A
		SUB(_A);
		break;
	case 0x98: // SBC A, B
		SBC(_B);
		break;
	case 0x99: // SBC A, C
		SBC(_C);
		break;
	case 0x9a: // SBC A, D
		SBC(_D);
		break;
	case 0x9b: // SBC A, E
		SBC(_E);
		break;
	case 0x9c: // SBC A, H
		SBC(_H);
		break;
	case 0x9d: // SBC A, L
		SBC(_L);
		break;
	case 0x9e: // SBC A, (HL)
		SBC(RM8(HL));
		break;
	case 0x9f: // SBC A, A
		SBC(_A);
		break;
	case 0xa0: // AND B
		AND(_B);
		break;
	case 0xa1: // AND C
		AND(_C);
		break;
	case 0xa2: // AND D
		AND(_D);
		break;
	case 0xa3: // AND E
		AND(_E);
		break;
	case 0xa4: // AND H
		AND(_H);
		break;
	case 0xa5: // AND L
		AND(_L);
		break;
	case 0xa6: // AND (HL)
		AND(RM8(HL));
		break;
	case 0xa7: // AND A
		AND(_A);
		break;
	case 0xa8: // XOR B
		XOR(_B);
		break;
	case 0xa9: // XOR C
		XOR(_C);
		break;
	case 0xaa: // XOR D
		XOR(_D);
		break;
	case 0xab: // XOR E
		XOR(_E);
		break;
	case 0xac: // XOR H
		XOR(_H);
		break;
	case 0xad: // XOR L
		XOR(_L);
		break;
	case 0xae: // XOR (HL)
		XOR(RM8(HL));
		break;
	case 0xaf: // XOR A
		XOR(_A);
		break;
	case 0xb0: // OR B
		OR(_B);
		break;
	case 0xb1: // OR C
		OR(_C);
		break;
	case 0xb2: // OR D
		OR(_D);
		break;
	case 0xb3: // OR E
		OR(_E);
		break;
	case 0xb4: // OR H
		OR(_H);
		break;
	case 0xb5: // OR L
		OR(_L);
		break;
	case 0xb6: // OR (HL)
		OR(RM8(HL));
		break;
	case 0xb7: // OR A
		OR(_A);
		break;
	case 0xb8: // CP B
		CP(_B);
		break;
	case 0xb9: // CP C
		CP(_C);
		break;
	case 0xba: // CP D
		CP(_D);
		break;
	case 0xbb: // CP E
		CP(_E);
		break;
	case 0xbc: // CP H
		CP(_H);
		break;
	case 0xbd: // CP L
		CP(_L);
		break;
	case 0xbe: // CP (HL)
		CP(RM8(HL));
		break;
	case 0xbf: // CP A
		CP(_A);
		break;
	case 0xc0: // RET NZ
		RET_COND(!(_F & ZF), 0xc0);
		break;
	case 0xc1: // POP BC
		BC = POP16();
		break;
	case 0xc2: // JP NZ, a
		JP_COND(!(_F & ZF));
		break;
	case 0xc3: // JP a
		JP();
		break;
	case 0xc4: // CALL NZ, a
		CALL_COND(!(_F & ZF), 0xc4);
		break;
	case 0xc5: // PUSH BC
		PUSH16(BC);
		break;
	case 0xc6: // ADD A, n
		ADD(FETCH8());
		break;
	case 0xc7: // RST 0
		RST(0x00);
		break;
	case 0xc8: // RET Z
		RET_COND(_F & ZF, 0xc8);
		break;
	case 0xc9: // RET
		RET();
		break;
	case 0xca: // JP Z, a
		JP_COND(_F & ZF);
		break;
	case 0xcb: // **** CB xx
		OP_CB();
		break;
	case 0xcc: // CALL Z, a
		CALL_COND(_F & ZF, 0xcc);
		break;
	case 0xcd: // CALL a
		CALL();
		break;
	case 0xce: // ADC A, n
		ADC(FETCH8());
		break;
	case 0xcf: // RST 1
		RST(0x08);
		break;
	case 0xd0: // RET NC
		RET_COND(!(_F & CF), 0xd0);
		break;
	case 0xd1: // POP DE
		DE = POP16();
		break;
	case 0xd2: // JP NC, a
		JP_COND(!(_F & CF));
		break;
	case 0xd3: // OUT (n), A
		OUT8(FETCH8(), _A, _A);
		break;
	case 0xd4: // CALL NC, a
		CALL_COND(!(_F & CF), 0xd4);
		break;
	case 0xd5: // PUSH DE
		PUSH16(DE);
		break;
	case 0xd6: // SUB n
		SUB(FETCH8());
		break;
	case 0xd7: // RST 2
		RST(0x10);
		break;
	case 0xd8: // RET C
		RET_COND(_F & CF, 0xd8);
		break;
	case 0xd9: // EXX
		EXX();
		break;
	case 0xda: // JP C, a
		JP_COND(_F & CF);
		break;
	case 0xdb: // IN A, (n)
		_A = IN8(FETCH8(), _A);
		break;
	case 0xdc: // CALL C, a
		CALL_COND(_F & CF, 0xdc);
		break;
	case 0xdd: // **** DD xx
		OP_DD();
		break;
	case 0xde: // SBC A, n
		SBC(FETCH8());
		break;
	case 0xdf: // RST 3
		RST(0x18);
		break;
	case 0xe0: // RET PO
		RET_COND(!(_F & PF), 0xe0);
		break;
	case 0xe1: // POP HL
		HL = POP16();
		break;
	case 0xe2: // JP PO, a
		JP_COND(!(_F & PF));
		break;
	case 0xe3: // EX HL, (SP)
		HL = EXSP(HL);
		break;
	case 0xe4: // CALL PO, a
		CALL_COND(!(_F & PF), 0xe4);
		break;
	case 0xe5: // PUSH HL
		PUSH16(HL);
		break;
	case 0xe6: // AND n
		AND(FETCH8());
		break;
	case 0xe7: // RST 4
		RST(0x20);
		break;
	case 0xe8: // RET PE
		RET_COND(_F & PF, 0xe8);
		break;
	case 0xe9: // JP (HL)
		PC = HL;
		break;
	case 0xea: // JP PE, a
		JP_COND(_F & PF);
		break;
	case 0xeb: // EX DE, HL
		EX_DE_HL();
		break;
	case 0xec: // CALL PE, a
		CALL_COND(_F & PF, 0xec);
		break;
	case 0xed: // **** ED xx
		OP_ED();
		break;
	case 0xee: // XOR n
		XOR(FETCH8());
		break;
	case 0xef: // RST 5
		RST(0x28);
		break;
	case 0xf0: // RET P
		RET_COND(!(_F & SF), 0xf0);
		break;
	case 0xf1: // POP AF
		AF = POP16();
		break;
	case 0xf2: // JP P, a
		JP_COND(!(_F & SF));
		break;
	case 0xf3: // DI
		DI();
		break;
	case 0xf4: // CALL P, a
		CALL_COND(!(_F & SF), 0xf4);
		break;
	case 0xf5: // PUSH AF
		PUSH16(AF);
		break;
	case 0xf6: // OR n
		OR(FETCH8());
		break;
	case 0xf7: // RST 6
		RST(0x30);
		break;
	case 0xf8: // RET M
		RET_COND(_F & SF, 0xf8);
		break;
	case 0xf9: // LD SP, HL
		SP = HL;
		break;
	case 0xfa: // JP M, a
		JP_COND(_F & SF);
		break;
	case 0xfb: // EI
		EI();
		break;
	case 0xfc: // CALL M, a
		CALL_COND(_F & SF, 0xfc);
		break;
	case 0xfd: // **** FD xx
		OP_FD();
		break;
	case 0xfe: // CP n
		CP(FETCH8());
		break;
	case 0xff: // RST 7
		RST(0x38);
		break;
	}
#ifdef _CPU_DEBUG_LOG
	if(debug_count) {
		emu->out_debug(_T("%4x\tAF=%4x BC=%4x DE=%4x HL=%4x IX=%4x IY=%4x SP=%4x [%c%c%c%c%c%c%c%c]\n"),
			prvPC, _AF, _BC, _DE, _HL, _IX, _IY, _SP,
			(_AF & CF) ? 'C' : ' ', (_AF & NF) ? 'N' : ' ', (_AF & PF) ? 'P' : ' ', (_AF & XF) ? 'X' : ' ',
			(_AF & HF) ? 'H' : ' ', (_AF & YF) ? 'Y' : ' ', (_AF & ZF) ? 'Z' : ' ', (_AF & SF) ? 'S' : ' ');
		DASM();
		emu->out_debug(_T("%4x\t%s\n"), prvPC, debug_dasm);
		if(--debug_count == 0) {
			emu->out_debug(_T("<---------------------------------------------------------------- Z80 DASM ----\n"));
		}
	}
#endif
}

void Z80::OP_CB()
{
	uint8 code = FETCHOP();
	count -= cc_cb[code];
	
	switch(code) {
	case 0x00: // RLC B
		_B = RLC(_B);
		break;
	case 0x01: // RLC C
		_C = RLC(_C);
		break;
	case 0x02: // RLC D
		_D = RLC(_D);
		break;
	case 0x03: // RLC E
		_E = RLC(_E);
		break;
	case 0x04: // RLC H
		_H = RLC(_H);
		break;
	case 0x05: // RLC L
		_L = RLC(_L);
		break;
	case 0x06: // RLC (HL)
		WM8(HL, RLC(RM8(HL)));
		break;
	case 0x07: // RLC A
		_A = RLC(_A);
		break;
	case 0x08: // RRC B
		_B = RRC(_B);
		break;
	case 0x09: // RRC C
		_C = RRC(_C);
		break;
	case 0x0a: // RRC D
		_D = RRC(_D);
		break;
	case 0x0b: // RRC E
		_E = RRC(_E);
		break;
	case 0x0c: // RRC H
		_H = RRC(_H);
		break;
	case 0x0d: // RRC L
		_L = RRC(_L);
		break;
	case 0x0e: // RRC (HL)
		WM8(HL, RRC(RM8(HL)));
		break;
	case 0x0f: // RRC A
		_A = RRC(_A);
		break;
	case 0x10: // RL B
		_B = RL(_B);
		break;
	case 0x11: // RL C
		_C = RL(_C);
		break;
	case 0x12: // RL D
		_D = RL(_D);
		break;
	case 0x13: // RL E
		_E = RL(_E);
		break;
	case 0x14: // RL H
		_H = RL(_H);
		break;
	case 0x15: // RL L
		_L = RL(_L);
		break;
	case 0x16: // RL (HL)
		WM8(HL, RL(RM8(HL)));
		break;
	case 0x17: // RL A
		_A = RL(_A);
		break;
	case 0x18: // RR B
		_B = RR(_B);
		break;
	case 0x19: // RR C
		_C = RR(_C);
		break;
	case 0x1a: // RR D
		_D = RR(_D);
		break;
	case 0x1b: // RR E
		_E = RR(_E);
		break;
	case 0x1c: // RR H
		_H = RR(_H);
		break;
	case 0x1d: // RR L
		_L = RR(_L);
		break;
	case 0x1e: // RR (HL)
		WM8(HL, RR(RM8(HL)));
		break;
	case 0x1f: // RR A
		_A = RR(_A);
		break;
	case 0x20: // SLA B
		_B = SLA(_B);
		break;
	case 0x21: // SLA C
		_C = SLA(_C);
		break;
	case 0x22: // SLA D
		_D = SLA(_D);
		break;
	case 0x23: // SLA E
		_E = SLA(_E);
		break;
	case 0x24: // SLA H
		_H = SLA(_H);
		break;
	case 0x25: // SLA L
		_L = SLA(_L);
		break;
	case 0x26: // SLA (HL)
		WM8(HL, SLA(RM8(HL)));
		break;
	case 0x27: // SLA A
		_A = SLA(_A);
		break;
	case 0x28: // SRA B
		_B = SRA(_B);
		break;
	case 0x29: // SRA C
		_C = SRA(_C);
		break;
	case 0x2a: // SRA D
		_D = SRA(_D);
		break;
	case 0x2b: // SRA E
		_E = SRA(_E);
		break;
	case 0x2c: // SRA H
		_H = SRA(_H);
		break;
	case 0x2d: // SRA L
		_L = SRA(_L);
		break;
	case 0x2e: // SRA (HL)
		WM8(HL, SRA(RM8(HL)));
		break;
	case 0x2f: // SRA A
		_A = SRA(_A);
		break;
	case 0x30: // SLL B
		_B = SLL(_B);
		break;
	case 0x31: // SLL C
		_C = SLL(_C);
		break;
	case 0x32: // SLL D
		_D = SLL(_D);
		break;
	case 0x33: // SLL E
		_E = SLL(_E);
		break;
	case 0x34: // SLL H
		_H = SLL(_H);
		break;
	case 0x35: // SLL L
		_L = SLL(_L);
		break;
	case 0x36: // SLL (HL)
		WM8(HL, SLL(RM8(HL)));
		break;
	case 0x37: // SLL A
		_A = SLL(_A);
		break;
	case 0x38: // SRL B
		_B = SRL(_B);
		break;
	case 0x39: // SRL C
		_C = SRL(_C);
		break;
	case 0x3a: // SRL D
		_D = SRL(_D);
		break;
	case 0x3b: // SRL E
		_E = SRL(_E);
		break;
	case 0x3c: // SRL H
		_H = SRL(_H);
		break;
	case 0x3d: // SRL L
		_L = SRL(_L);
		break;
	case 0x3e: // SRL (HL)
		WM8(HL, SRL(RM8(HL)));
		break;
	case 0x3f: // SRL A
		_A = SRL(_A);
		break;
	case 0x40: // BIT 0, B
		BIT(0, _B);
		break;
	case 0x41: // BIT 0, C
		BIT(0, _C);
		break;
	case 0x42: // BIT 0, D
		BIT(0, _D);
		break;
	case 0x43: // BIT 0, E
		BIT(0, _E);
		break;
	case 0x44: // BIT 0, H
		BIT(0, _H);
		break;
	case 0x45: // BIT 0, L
		BIT(0, _L);
		break;
	case 0x46: // BIT 0, (HL)
		BIT(0, RM8(HL));
		break;
	case 0x47: // BIT 0, A
		BIT(0, _A);
		break;
	case 0x48: // BIT 1, B
		BIT(1, _B);
		break;
	case 0x49: // BIT 1, C
		BIT(1, _C);
		break;
	case 0x4a: // BIT 1, D
		BIT(1, _D);
		break;
	case 0x4b: // BIT 1, E
		BIT(1, _E);
		break;
	case 0x4c: // BIT 1, H
		BIT(1, _H);
		break;
	case 0x4d: // BIT 1, L
		BIT(1, _L);
		break;
	case 0x4e: // BIT 1, (HL)
		BIT(1, RM8(HL));
		break;
	case 0x4f: // BIT 1, A
		BIT(1, _A);
		break;
	case 0x50: // BIT 2, B
		BIT(2, _B);
		break;
	case 0x51: // BIT 2, C
		BIT(2, _C);
		break;
	case 0x52: // BIT 2, D
		BIT(2, _D);
		break;
	case 0x53: // BIT 2, E
		BIT(2, _E);
		break;
	case 0x54: // BIT 2, H
		BIT(2, _H);
		break;
	case 0x55: // BIT 2, L
		BIT(2, _L);
		break;
	case 0x56: // BIT 2, (HL)
		BIT(2, RM8(HL));
		break;
	case 0x57: // BIT 2, A
		BIT(2, _A);
		break;
	case 0x58: // BIT 3, B
		BIT(3, _B);
		break;
	case 0x59: // BIT 3, C
		BIT(3, _C);
		break;
	case 0x5a: // BIT 3, D
		BIT(3, _D);
		break;
	case 0x5b: // BIT 3, E
		BIT(3, _E);
		break;
	case 0x5c: // BIT 3, H
		BIT(3, _H);
		break;
	case 0x5d: // BIT 3, L
		BIT(3, _L);
		break;
	case 0x5e: // BIT 3, (HL)
		BIT(3, RM8(HL));
		break;
	case 0x5f: // BIT 3, A
		BIT(3, _A);
		break;
	case 0x60: // BIT 4, B
		BIT(4, _B);
		break;
	case 0x61: // BIT 4, C
		BIT(4, _C);
		break;
	case 0x62: // BIT 4, D
		BIT(4, _D);
		break;
	case 0x63: // BIT 4, E
		BIT(4, _E);
		break;
	case 0x64: // BIT 4, H
		BIT(4, _H);
		break;
	case 0x65: // BIT 4, L
		BIT(4, _L);
		break;
	case 0x66: // BIT 4, (HL)
		BIT(4, RM8(HL));
		break;
	case 0x67: // BIT 4, A
		BIT(4, _A);
		break;
	case 0x68: // BIT 5, B
		BIT(5, _B);
		break;
	case 0x69: // BIT 5, C
		BIT(5, _C);
		break;
	case 0x6a: // BIT 5, D
		BIT(5, _D);
		break;
	case 0x6b: // BIT 5, E
		BIT(5, _E);
		break;
	case 0x6c: // BIT 5, H
		BIT(5, _H);
		break;
	case 0x6d: // BIT 5, L
		BIT(5, _L);
		break;
	case 0x6e: // BIT 5, (HL)
		BIT(5, RM8(HL));
		break;
	case 0x6f: // BIT 5, A
		BIT(5, _A);
		break;
	case 0x70: // BIT 6, B
		BIT(6, _B);
		break;
	case 0x71: // BIT 6, C
		BIT(6, _C);
		break;
	case 0x72: // BIT 6, D
		BIT(6, _D);
		break;
	case 0x73: // BIT 6, E
		BIT(6, _E);
		break;
	case 0x74: // BIT 6, H
		BIT(6, _H);
		break;
	case 0x75: // BIT 6, L
		BIT(6, _L);
		break;
	case 0x76: // BIT 6, (HL)
		BIT(6, RM8(HL));
		break;
	case 0x77: // BIT 6, A
		BIT(6, _A);
		break;
	case 0x78: // BIT 7, B
		BIT(7, _B);
		break;
	case 0x79: // BIT 7, C
		BIT(7, _C);
		break;
	case 0x7a: // BIT 7, D
		BIT(7, _D);
		break;
	case 0x7b: // BIT 7, E
		BIT(7, _E);
		break;
	case 0x7c: // BIT 7, H
		BIT(7, _H);
		break;
	case 0x7d: // BIT 7, L
		BIT(7, _L);
		break;
	case 0x7e: // BIT 7, (HL)
		BIT(7, RM8(HL));
		break;
	case 0x7f: // BIT 7, A
		BIT(7, _A);
		break;
	case 0x80: // RES 0, B
		_B = RES(0, _B);
		break;
	case 0x81: // RES 0, C
		_C = RES(0, _C);
		break;
	case 0x82: // RES 0, D
		_D = RES(0, _D);
		break;
	case 0x83: // RES 0, E
		_E = RES(0, _E);
		break;
	case 0x84: // RES 0, H
		_H = RES(0, _H);
		break;
	case 0x85: // RES 0, L
		_L = RES(0, _L);
		break;
	case 0x86: // RES 0, (HL)
		WM8(HL, RES(0, RM8(HL)));
		break;
	case 0x87: // RES 0, A
		_A = RES(0, _A);
		break;
	case 0x88: // RES 1, B
		_B = RES(1, _B);
		break;
	case 0x89: // RES 1, C
		_C = RES(1, _C);
		break;
	case 0x8a: // RES 1, D
		_D = RES(1, _D);
		break;
	case 0x8b: // RES 1, E
		_E = RES(1, _E);
		break;
	case 0x8c: // RES 1, H
		_H = RES(1, _H);
		break;
	case 0x8d: // RES 1, L
		_L = RES(1, _L);
		break;
	case 0x8e: // RES 1, (HL)
		WM8(HL, RES(1, RM8(HL)));
		break;
	case 0x8f: // RES 1, A
		_A = RES(1, _A);
		break;
	case 0x90: // RES 2, B
		_B = RES(2, _B);
		break;
	case 0x91: // RES 2, C
		_C = RES(2, _C);
		break;
	case 0x92: // RES 2, D
		_D = RES(2, _D);
		break;
	case 0x93: // RES 2, E
		_E = RES(2, _E);
		break;
	case 0x94: // RES 2, H
		_H = RES(2, _H);
		break;
	case 0x95: // RES 2, L
		_L = RES(2, _L);
		break;
	case 0x96: // RES 2, (HL)
		WM8(HL, RES(2, RM8(HL)));
		break;
	case 0x97: // RES 2, A
		_A = RES(2, _A);
		break;
	case 0x98: // RES 3, B
		_B = RES(3, _B);
		break;
	case 0x99: // RES 3, C
		_C = RES(3, _C);
		break;
	case 0x9a: // RES 3, D
		_D = RES(3, _D);
		break;
	case 0x9b: // RES 3, E
		_E = RES(3, _E);
		break;
	case 0x9c: // RES 3, H
		_H = RES(3, _H);
		break;
	case 0x9d: // RES 3, L
		_L = RES(3, _L);
		break;
	case 0x9e: // RES 3, (HL)
		WM8(HL, RES(3, RM8(HL)));
		break;
	case 0x9f: // RES 3, A
		_A = RES(3, _A);
		break;
	case 0xa0: // RES 4, B
		_B = RES(4, _B);
		break;
	case 0xa1: // RES 4, C
		_C = RES(4, _C);
		break;
	case 0xa2: // RES 4, D
		_D = RES(4, _D);
		break;
	case 0xa3: // RES 4, E
		_E = RES(4, _E);
		break;
	case 0xa4: // RES 4, H
		_H = RES(4, _H);
		break;
	case 0xa5: // RES 4, L
		_L = RES(4, _L);
		break;
	case 0xa6: // RES 4, (HL)
		WM8(HL, RES(4, RM8(HL)));
		break;
	case 0xa7: // RES 4, A
		_A = RES(4, _A);
		break;
	case 0xa8: // RES 5, B
		_B = RES(5, _B);
		break;
	case 0xa9: // RES 5, C
		_C = RES(5, _C);
		break;
	case 0xaa: // RES 5, D
		_D = RES(5, _D);
		break;
	case 0xab: // RES 5, E
		_E = RES(5, _E);
		break;
	case 0xac: // RES 5, H
		_H = RES(5, _H);
		break;
	case 0xad: // RES 5, L
		_L = RES(5, _L);
		break;
	case 0xae: // RES 5, (HL)
		WM8(HL, RES(5, RM8(HL)));
		break;
	case 0xaf: // RES 5, A
		_A = RES(5, _A);
		break;
	case 0xb0: // RES 6, B
		_B = RES(6, _B);
		break;
	case 0xb1: // RES 6, C
		_C = RES(6, _C);
		break;
	case 0xb2: // RES 6, D
		_D = RES(6, _D);
		break;
	case 0xb3: // RES 6, E
		_E = RES(6, _E);
		break;
	case 0xb4: // RES 6, H
		_H = RES(6, _H);
		break;
	case 0xb5: // RES 6, L
		_L = RES(6, _L);
		break;
	case 0xb6: // RES 6, (HL)
		WM8(HL, RES(6, RM8(HL)));
		break;
	case 0xb7: // RES 6, A
		_A = RES(6, _A);
		break;
	case 0xb8: // RES 7, B
		_B = RES(7, _B);
		break;
	case 0xb9: // RES 7, C
		_C = RES(7, _C);
		break;
	case 0xba: // RES 7, D
		_D = RES(7, _D);
		break;
	case 0xbb: // RES 7, E
		_E = RES(7, _E);
		break;
	case 0xbc: // RES 7, H
		_H = RES(7, _H);
		break;
	case 0xbd: // RES 7, L
		_L = RES(7, _L);
		break;
	case 0xbe: // RES 7, (HL)
		WM8(HL, RES(7, RM8(HL)));
		break;
	case 0xbf: // RES 7, A
		_A = RES(7, _A);
		break;
	case 0xc0: // SET 0, B
		_B = SET(0, _B);
		break;
	case 0xc1: // SET 0, C
		_C = SET(0, _C);
		break;
	case 0xc2: // SET 0, D
		_D = SET(0, _D);
		break;
	case 0xc3: // SET 0, E
		_E = SET(0, _E);
		break;
	case 0xc4: // SET 0, H
		_H = SET(0, _H);
		break;
	case 0xc5: // SET 0, L
		_L = SET(0, _L);
		break;
	case 0xc6: // SET 0, (HL)
		WM8(HL, SET(0, RM8(HL)));
		break;
	case 0xc7: // SET 0, A
		_A = SET(0, _A);
		break;
	case 0xc8: // SET 1, B
		_B = SET(1, _B);
		break;
	case 0xc9: // SET 1, C
		_C = SET(1, _C);
		break;
	case 0xca: // SET 1, D
		_D = SET(1, _D);
		break;
	case 0xcb: // SET 1, E
		_E = SET(1, _E);
		break;
	case 0xcc: // SET 1, H
		_H = SET(1, _H);
		break;
	case 0xcd: // SET 1, L
		_L = SET(1, _L);
		break;
	case 0xce: // SET 1, (HL)
		WM8(HL, SET(1, RM8(HL)));
		break;
	case 0xcf: // SET 1, A
		_A = SET(1, _A);
		break;
	case 0xd0: // SET 2, B
		_B = SET(2, _B);
		break;
	case 0xd1: // SET 2, C
		_C = SET(2, _C);
		break;
	case 0xd2: // SET 2, D
		_D = SET(2, _D);
		break;
	case 0xd3: // SET 2, E
		_E = SET(2, _E);
		break;
	case 0xd4: // SET 2, H
		_H = SET(2, _H);
		break;
	case 0xd5: // SET 2, L
		_L = SET(2, _L);
		break;
	case 0xd6: // SET 2, (HL)
		WM8(HL, SET(2, RM8(HL)));
		break;
	case 0xd7: // SET 2, A
		_A = SET(2, _A);
		break;
	case 0xd8: // SET 3, B
		_B = SET(3, _B);
		break;
	case 0xd9: // SET 3, C
		_C = SET(3, _C);
		break;
	case 0xda: // SET 3, D
		_D = SET(3, _D);
		break;
	case 0xdb: // SET 3, E
		_E = SET(3, _E);
		break;
	case 0xdc: // SET 3, H
		_H = SET(3, _H);
		break;
	case 0xdd: // SET 3, L
		_L = SET(3, _L);
		break;
	case 0xde: // SET 3, (HL)
		WM8(HL, SET(3, RM8(HL)));
		break;
	case 0xdf: // SET 3, A
		_A = SET(3, _A);
		break;
	case 0xe0: // SET 4, B
		_B = SET(4, _B);
		break;
	case 0xe1: // SET 4, C
		_C = SET(4, _C);
		break;
	case 0xe2: // SET 4, D
		_D = SET(4, _D);
		break;
	case 0xe3: // SET 4, E
		_E = SET(4, _E);
		break;
	case 0xe4: // SET 4, H
		_H = SET(4, _H);
		break;
	case 0xe5: // SET 4, L
		_L = SET(4, _L);
		break;
	case 0xe6: // SET 4, (HL)
		WM8(HL, SET(4, RM8(HL)));
		break;
	case 0xe7: // SET 4, A
		_A = SET(4, _A);
		break;
	case 0xe8: // SET 5, B
		_B = SET(5, _B);
		break;
	case 0xe9: // SET 5, C
		_C = SET(5, _C);
		break;
	case 0xea: // SET 5, D
		_D = SET(5, _D);
		break;
	case 0xeb: // SET 5, E
		_E = SET(5, _E);
		break;
	case 0xec: // SET 5, H
		_H = SET(5, _H);
		break;
	case 0xed: // SET 5, L
		_L = SET(5, _L);
		break;
	case 0xee: // SET 5, (HL)
		WM8(HL, SET(5, RM8(HL)));
		break;
	case 0xef: // SET 5, A
		_A = SET(5, _A);
		break;
	case 0xf0: // SET 6, B
		_B = SET(6, _B);
		break;
	case 0xf1: // SET 6, C
		_C = SET(6, _C);
		break;
	case 0xf2: // SET 6, D
		_D = SET(6, _D);
		break;
	case 0xf3: // SET 6, E
		_E = SET(6, _E);
		break;
	case 0xf4: // SET 6, H
		_H = SET(6, _H);
		break;
	case 0xf5: // SET 6, L
		_L = SET(6, _L);
		break;
	case 0xf6: // SET 6, (HL)
		WM8(HL, SET(6, RM8(HL)));
		break;
	case 0xf7: // SET 6, A
		_A = SET(6, _A);
		break;
	case 0xf8: // SET 7, B
		_B = SET(7, _B);
		break;
	case 0xf9: // SET 7, C
		_C = SET(7, _C);
		break;
	case 0xfa: // SET 7, D
		_D = SET(7, _D);
		break;
	case 0xfb: // SET 7, E
		_E = SET(7, _E);
		break;
	case 0xfc: // SET 7, H
		_H = SET(7, _H);
		break;
	case 0xfd: // SET 7, L
		_L = SET(7, _L);
		break;
	case 0xfe: // SET 7, (HL)
		WM8(HL, SET(7, RM8(HL)));
		break;
	case 0xff: // SET 7, A
		_A = SET(7, _A);
		break;
	}
}

void Z80::OP_DD()
{
	uint8 code = FETCHOP();
	count -= cc_xy[code];
	
	switch(code) {
	case 0x09: // ADD IX, BC
		IX = ADD16(IX, BC);
		break;
	case 0x19: // ADD IX, DE
		IX = ADD16(IX, DE);
		break;
	case 0x21: // LD IX, w
		IX = FETCH16();
		break;
	case 0x22: // LD (w), IX
		EA = FETCH16();
		WM16(EA, IX);
		break;
	case 0x23: // INC IX
		IX++;
		break;
	case 0x24: // INC HX
		_XH = INC(_XH);
		break;
	case 0x25: // DEC HX
		_XH = DEC(_XH);
		break;
	case 0x26: // LD HX, n
		_XH = FETCH8();
		break;
	case 0x29: // ADD IX, IX
		IX = ADD16(IX, IX);
		break;
	case 0x2a: // LD IX, (w)
		EA = FETCH16();
		IX = RM16(EA);
		break;
	case 0x2b: // DEC IX
		IX--;
		break;
	case 0x2c: // INC LX
		_XL = INC(_XL);
		break;
	case 0x2d: // DEC LX
		_XL = DEC(_XL);
		break;
	case 0x2e: // LD LX, n
		_XL = FETCH8();
		break;
	case 0x34: // INC (IX+o)
		EAX();
		WM8(EA, INC(RM8(EA)));
		break;
	case 0x35: // DEC (IX+o)
		EAX();
		WM8(EA, DEC(RM8(EA)));
		break;
	case 0x36: // LD (IX+o), n
		EAX();
		WM8(EA, FETCH8());
		break;
	case 0x39: // ADD IX, SP
		IX = ADD16(IX, SP);
		break;
	case 0x44: // LD B, HX
		_B = _XH;
		break;
	case 0x45: // LD B, LX
		_B = _XL;
		break;
	case 0x46: // LD B, (IX+o)
		EAX();
		_B = RM8(EA);
		break;
	case 0x4c: // LD C, HX
		_C = _XH;
		break;
	case 0x4d: // LD C, LX
		_C = _XL;
		break;
	case 0x4e: // LD C, (IX+o)
		EAX();
		_C = RM8(EA);
		break;
	case 0x54: // LD D, HX
		_D = _XH;
		break;
	case 0x55: // LD D, LX
		_D = _XL;
		break;
	case 0x56: // LD D, (IX+o)
		EAX();
		_D = RM8(EA);
		break;
	case 0x5c: // LD E, HX
		_E = _XH;
		break;
	case 0x5d: // LD E, LX
		_E = _XL;
		break;
	case 0x5e: // LD E, (IX+o)
		EAX();
		_E = RM8(EA);
		break;
	case 0x60: // LD HX, B
		_XH = _B;
		break;
	case 0x61: // LD HX, C
		_XH = _C;
		break;
	case 0x62: // LD HX, D
		_XH = _D;
		break;
	case 0x63: // LD HX, E
		_XH = _E;
		break;
	case 0x64: // LD HX, HX
		break;
	case 0x65: // LD HX, LX
		_XH = _XL;
		break;
	case 0x66: // LD H, (IX+o)
		EAX();
		_H = RM8(EA);
		break;
	case 0x67: // LD HX, A
		_XH = _A;
		break;
	case 0x68: // LD LX, B
		_XL = _B;
		break;
	case 0x69: // LD LX, C
		_XL = _C;
		break;
	case 0x6a: // LD LX, D
		_XL = _D;
		break;
	case 0x6b: // LD LX, E
		_XL = _E;
		break;
	case 0x6c: // LD LX, HX
		_XL = _XH;
		break;
	case 0x6d: // LD LX, LX
		break;
	case 0x6e: // LD L, (IX+o)
		EAX();
		_L = RM8(EA);
		break;
	case 0x6f: // LD LX, A
		_XL = _A;
		break;
	case 0x70: // LD (IX+o), B
		EAX();
		WM8(EA, _B);
		break;
	case 0x71: // LD (IX+o), C
		EAX();
		WM8(EA, _C);
		break;
	case 0x72: // LD (IX+o), D
		EAX();
		WM8(EA, _D);
		break;
	case 0x73: // LD (IX+o), E
		EAX();
		WM8(EA, _E);
		break;
	case 0x74: // LD (IX+o), H
		EAX();
		WM8(EA, _H);
		break;
	case 0x75: // LD (IX+o), L
		EAX();
		WM8(EA, _L);
		break;
	case 0x77: // LD (IX+o), A
		EAX();
		WM8(EA, _A);
		break;
	case 0x7c: // LD A, HX
		_A = _XH;
		break;
	case 0x7d: // LD A, LX
		_A = _XL;
		break;
	case 0x7e: // LD A, (IX+o)
		EAX();
		_A = RM8(EA);
		break;
	case 0x84: // ADD A, HX
		ADD(_XH);
		break;
	case 0x85: // ADD A, LX
		ADD(_XL);
		break;
	case 0x86: // ADD A, (IX+o)
		EAX();
		ADD(RM8(EA));
		break;
	case 0x8c: // ADC A, HX
		ADC(_XH);
		break;
	case 0x8d: // ADC A, LX
		ADC(_XL);
		break;
	case 0x8e: // ADC A, (IX+o)
		EAX();
		ADC(RM8(EA));
		break;
	case 0x94: // SUB HX
		SUB(_XH);
		break;
	case 0x95: // SUB LX
		SUB(_XL);
		break;
	case 0x96: // SUB (IX+o)
		EAX();
		SUB(RM8(EA));
		break;
	case 0x9c: // SBC A, HX
		SBC(_XH);
		break;
	case 0x9d: // SBC A, LX
		SBC(_XL);
		break;
	case 0x9e: // SBC A, (IX+o)
		EAX();
		SBC(RM8(EA));
		break;
	case 0xa4: // AND HX
		AND(_XH);
		break;
	case 0xa5: // AND LX
		AND(_XL);
		break;
	case 0xa6: // AND (IX+o)
		EAX();
		AND(RM8(EA));
		break;
	case 0xac: // XOR HX
		XOR(_XH);
		break;
	case 0xad: // XOR LX
		XOR(_XL);
		break;
	case 0xae: // XOR (IX+o)
		EAX();
		XOR(RM8(EA));
		break;
	case 0xb4: // OR HX
		OR(_XH);
		break;
	case 0xb5: // OR LX
		OR(_XL);
		break;
	case 0xb6: // OR (IX+o)
		EAX();
		OR(RM8(EA));
		break;
	case 0xbc: // CP HX
		CP(_XH);
		break;
	case 0xbd: // CP LX
		CP(_XL);
		break;
	case 0xbe: // CP (IX+o)
		EAX();
		CP(RM8(EA));
		break;
	case 0xcb: // ** DD CB xx
		EAX();
		OP_XY();
		break;
	case 0xe1: // POP IX
		IX = POP16();
		break;
	case 0xe3: // EX (SP), IX
		IX = EXSP(IX);
		break;
	case 0xe5: // PUSH IX
		PUSH16(IX);
		break;
	case 0xe9: // JP (IX)
		PC = IX;
		break;
	case 0xf9: // LD SP, IX
		SP = IX;
		break;
	}
}

void Z80::OP_ED()
{
	uint8 code = FETCHOP();
	count -= cc_ed[code];
	
	switch(code) {
	case 0x40: // IN B, (C)
		_B = IN8(_C, _B);
		_F = (_F & CF) | SZP[_B];
		break;
	case 0x41: // OUT (C), B
		OUT8(_C, _B, _B);
		break;
	case 0x42: // SBC HL, BC
		SBC16(BC);
		break;
	case 0x43: // LD (w), BC
		EA = FETCH16();
		WM16(EA, BC);
		break;
	case 0x44: // NEG
		NEG();
		break;
	case 0x45: // RETN
		RETN();
		break;
	case 0x46: // IM 0
		IM = 0;
		break;
	case 0x47: // LD I, A
		_I = _A;
		break;
	case 0x48: // IN C, (C)
		_C = IN8(_C, _B);
		_F = (_F & CF) | SZP[_C];
		break;
	case 0x49: // OUT (C), C
		OUT8(_C, _B, _C);
		break;
	case 0x4a: // ADC HL, BC
		ADC16(BC);
		break;
	case 0x4b: // LD BC, (w)
		EA = FETCH16();
		BC = RM16(EA);
		break;
	case 0x4c: // NEG
		NEG();
		break;
	case 0x4d: // RETI
		RETI();
		break;
	case 0x4e: // IM 0
		IM = 0;
		break;
	case 0x4f: // LD R, A
		_R = _A;
		break;
	case 0x50: // IN D, (C)
		_D = IN8(_C, _B);
		_F = (_F & CF) | SZP[_D];
		break;
	case 0x51: // OUT (C), D
		OUT8(_C, _B, _D);
		break;
	case 0x52: // SBC HL, DE
		SBC16(DE);
		break;
	case 0x53: // LD (w), DE
		EA = FETCH16();
		WM16(EA, DE);
		break;
	case 0x54: // NEG
		NEG();
		break;
	case 0x55: // RETN
		RETN();
		break;
	case 0x56: // IM 1
		IM = 1;
		break;
	case 0x57: // LD A, I
		_A = _I;
		_F = (_F & CF) | SZ[_A] | (IFF2 << 2);
		break;
	case 0x58: // IN E, (C)
		_E = IN8(_C, _B);
		_F = (_F & CF) | SZP[_E];
		break;
	case 0x59: // OUT (C), E
		OUT8(_C, _B, _E);
		break;
	case 0x5a: // ADC HL, DE
		ADC16(DE);
		break;
	case 0x5b: // LD DE, (w)
		EA = FETCH16();
		DE = RM16(EA);
		break;
	case 0x5c: // NEG
		NEG();
		break;
	case 0x5d: // RETI
		RETI();
		break;
	case 0x5e: // IM 2
		IM = 2;
		break;
	case 0x5f: // LD A, R
		_A = _R;
		_F = (_F & CF) | SZ[_A] | (IFF2 << 2);
		break;
	case 0x60: // IN H, (C)
		_H = IN8(_C, _B);
		_F = (_F & CF) | SZP[_H];
		break;
	case 0x61: // OUT (C), H
		OUT8(_C, _B, _H);
		break;
	case 0x62: // SBC HL, HL
		SBC16(HL);
		break;
	case 0x63: // LD (w), HL
		EA = FETCH16();
		WM16(EA, HL);
		break;
	case 0x64: // NEG
		NEG();
		break;
	case 0x65: // RETN
		RETN();
		break;
	case 0x66: // IM 0
		IM = 0;
		break;
	case 0x67: // RRD (HL)
		RRD();
		break;
	case 0x68: // IN L, (C)
		_L = IN8(_C, _B);
		_F = (_F & CF) | SZP[_L];
		break;
	case 0x69: // OUT (C), L
		OUT8(_C, _B, _L);
		break;
	case 0x6a: // ADC HL, HL
		ADC16(HL);
		break;
	case 0x6b: // LD HL, (w)
		EA = FETCH16();
		HL = RM16(EA);
		break;
	case 0x6c: // NEG
		NEG();
		break;
	case 0x6d: // RETI
		RETI();
		break;
	case 0x6e: // IM 0
		IM = 0;
		break;
	case 0x6f: // RLD (HL)
		RLD();
		break;
	case 0x70: // IN 0, (C)
		_F = (_F & CF) | SZP[IN8(_C, _B)];
		break;
	case 0x71: // OUT (C), 0
		OUT8(_C, _B, 0);
		break;
	case 0x72: // SBC HL, SP
		SBC16(SP);
		break;
	case 0x73: // LD (w), SP
		EA = FETCH16();
		WM16(EA, SP);
		break;
	case 0x74: // NEG
		NEG();
		break;
	case 0x75: // RETN
		RETN();
		break;
	case 0x76: // IM 1
		IM = 1;
		break;
	case 0x78: // IN A, (C)
		_A = IN8(_C, _B);
		_F = (_F & CF) | SZP[_A];
		break;
	case 0x79: // OUT (C), A
		OUT8(_C, _B, _A);
		break;
	case 0x7a: // ADC HL, SP
		ADC16(SP);
		break;
	case 0x7b: // LD SP, (w)
		EA = RM16(PC);
		PC += 2;
		SP = RM16(EA);
		break;
	case 0x7c: // NEG
		NEG();
		break;
	case 0x7d: // RETI
		RETI();
		break;
	case 0x7e: // IM 2
		IM = 2;
		break;
	case 0xa0: // LDI
		LDI();
		break;
	case 0xa1: // CPI
		CPI();
		break;
	case 0xa2: // INI
		INI();
		break;
	case 0xa3: // OUTI
		OUTI();
		break;
	case 0xa8: // LDD
		LDD();
		break;
	case 0xa9: // CPD
		CPD();
		break;
	case 0xaa: // IND
		IND();
		break;
	case 0xab: // OUTD
		OUTD();
		break;
	case 0xb0: // LDIR
		LDIR();
		break;
	case 0xb1: // CPIR
		CPIR();
		break;
	case 0xb2: // INIR
		INIR();
		break;
	case 0xb3: // OTIR
		OTIR();
		break;
	case 0xb8: // LDDR
		LDDR();
		break;
	case 0xb9: // CPDR
		CPDR();
		break;
	case 0xba: // INDR
		INDR();
		break;
	case 0xbb: // OTDR
		OTDR();
		break;
	}
}

void Z80::OP_FD()
{
	uint8 code = FETCHOP();
	count -= cc_xy[code];
	
	switch(code) {
	case 0x09: // ADD IY, BC
		IY = ADD16(IY, BC);
		break;
	case 0x19: // ADD IY, DE
		IY = ADD16(IY, DE);
		break;
	case 0x21: // LD IY, w
		IY = RM16(PC);
		PC += 2;
		break;
	case 0x22: // LD (w), IY
		EA = RM16(PC);
		PC += 2;
		WM16(EA, IY);
		break;
	case 0x23: // INC IY
		IY++;
		break;
	case 0x24: // INC HY
		_YH = INC(_YH);
		break;
	case 0x25: // DEC HY
		_YH = DEC(_YH);
		break;
	case 0x26: // LD HY, n
		_YH = FETCH8();
		break;
	case 0x29: // ADD IY, IY
		IY = ADD16(IY, IY);
		break;
	case 0x2a: // LD IY, (w)
		EA = RM16(PC);
		PC += 2;
		IY = RM16(EA);
		break;
	case 0x2b: // DEC IY
		IY--;
		break;
	case 0x2c: // INC LY
		_YL = INC(_YL);
		break;
	case 0x2d: // DEC LY
		_YL = DEC(_YL);
		break;
	case 0x2e: // LD LY, n
		_YL = FETCH8();
		break;
	case 0x34: // INC (IY+o)
		EAY();
		WM8(EA, INC(RM8(EA)));
		break;
	case 0x35: // DEC (IY+o)
		EAY();
		WM8(EA, DEC(RM8(EA)));
		break;
	case 0x36: // LD (IY+o), n
		EAY();
		WM8(EA, FETCH8());
		break;
	case 0x39: // ADD IY, SP
		IY = ADD16(IY, SP);
		break;
	case 0x44: // LD B, HY
		_B = _YH;
		break;
	case 0x45: // LD B, LY
		_B = _YL;
		break;
	case 0x46: // LD B, (IY+o)
		EAY();
		_B = RM8(EA);
		break;
	case 0x4c: // LD C, HY
		_C = _YH;
		break;
	case 0x4d: // LD C, LY
		_C = _YL;
		break;
	case 0x4e: // LD C, (IY+o)
		EAY();
		_C = RM8(EA);
		break;
	case 0x54: // LD D, HY
		_D = _YH;
		break;
	case 0x55: // LD D, LY
		_D = _YL;
		break;
	case 0x56: // LD D, (IY+o)
		EAY();
		_D = RM8(EA);
		break;
	case 0x5c: // LD E, HY
		_E = _YH;
		break;
	case 0x5d: // LD E, LY
		_E = _YL;
		break;
	case 0x5e: // LD E, (IY+o)
		EAY();
		_E = RM8(EA);
		break;
	case 0x60: // LD HY, B
		_YH = _B;
		break;
	case 0x61: // LD HY, C
		_YH = _C;
		break;
	case 0x62: // LD HY, D
		_YH = _D;
		break;
	case 0x63: // LD HY, E
		_YH = _E;
		break;
	case 0x64: // LD HY, HY
		break;
	case 0x65: // LD HY, LY
		_YH = _YL;
		break;
	case 0x66: // LD H, (IY+o)
		EAY();
		_H = RM8(EA);
		break;
	case 0x67: // LD HY, A
		_YH = _A;
		break;
	case 0x68: // LD LY, B
		_YL = _B;
		break;
	case 0x69: // LD LY, C
		_YL = _C;
		break;
	case 0x6a: // LD LY, D
		_YL = _D;
		break;
	case 0x6b: // LD LY, E
		_YL = _E;
		break;
	case 0x6c: // LD LY, HY
		_YL = _YH;
		break;
	case 0x6d: // LD LY, LY
		break;
	case 0x6e: // LD L, (IY+o)
		EAY();
		_L = RM8(EA);
		break;
	case 0x6f: // LD LY, A
		_YL = _A;
		break;
	case 0x70: // LD (IY+o), B
		EAY();
		WM8(EA, _B);
		break;
	case 0x71: // LD (IY+o), C
		EAY();
		WM8(EA, _C);
		break;
	case 0x72: // LD (IY+o), D
		EAY();
		WM8(EA, _D);
		break;
	case 0x73: // LD (IY+o), E
		EAY();
		WM8(EA, _E);
		break;
	case 0x74: // LD (IY+o), H
		EAY();
		WM8(EA, _H);
		break;
	case 0x75: // LD (IY+o), L
		EAY();
		WM8(EA, _L);
		break;
	case 0x77: // LD (IY+o), A
		EAY();
		WM8(EA, _A);
		break;
	case 0x7c: // LD A, HY
		_A = _YH;
		break;
	case 0x7d: // LD A, LY
		_A = _YL;
		break;
	case 0x7e: // LD A, (IY+o)
		EAY();
		_A = RM8(EA);
		break;
	case 0x84: // ADD A, HY
		ADD(_YH);
		break;
	case 0x85: // ADD A, LY
		ADD(_YL);
		break;
	case 0x86: // ADD A, (IY+o)
		EAY();
		ADD(RM8(EA));
		break;
	case 0x8c: // ADC A, HY
		ADC(_YH);
		break;
	case 0x8d: // ADC A, LY
		ADC(_YL);
		break;
	case 0x8e: // ADC A, (IY+o)
		EAY();
		ADC(RM8(EA));
		break;
	case 0x94: // SUB HY
		SUB(_YH);
		break;
	case 0x95: // SUB LY
		SUB(_YL);
		break;
	case 0x96: // SUB (IY+o)
		EAY();
		SUB(RM8(EA));
		break;
	case 0x9c: // SBC A, HY
		SBC(_YH);
		break;
	case 0x9d: // SBC A, LY
		SBC(_YL);
		break;
	case 0x9e: // SBC A, (IY+o)
		EAY();
		SBC(RM8(EA));
		break;
	case 0xa4: // AND HY
		AND(_YH);
		break;
	case 0xa5: // AND LY
		AND(_YL);
		break;
	case 0xa6: // AND (IY+o)
		EAY();
		AND(RM8(EA));
		break;
	case 0xac: // XOR HY
		XOR(_YH);
		break;
	case 0xad: // XOR LY
		XOR(_YL);
		break;
	case 0xae: // XOR (IY+o)
		EAY();
		XOR(RM8(EA));
		break;
	case 0xb4: // OR HY
		OR(_YH);
		break;
	case 0xb5: // OR LY
		OR(_YL);
		break;
	case 0xb6: // OR (IY+o)
		EAY();
		OR(RM8(EA));
		break;
	case 0xbc: // CP HY
		CP(_YH);
		break;
	case 0xbd: // CP LY
		CP(_YL);
		break;
	case 0xbe: // CP (IY+o)
		EAY();
		CP(RM8(EA));
		break;
	case 0xcb: // ** FD CB xx
		EAY();
		OP_XY();
		break;
	case 0xe1: // POP IY
		IY = POP16();
		break;
	case 0xe3: // EX (SP), IY
		IY = EXSP(IY);
		break;
	case 0xe5: // PUSH IY
		PUSH16(IY);
		break;
	case 0xe9: // JP (IY)
		PC = IY;
		break;
	case 0xf9: // LD SP, IY
		SP = IY;
		break;
	}
}

void Z80::OP_XY()
{
	uint8 code = FETCH8();
	count -= cc_xycb[code];
	
	switch(code) {
	case 0x00: // RLC B=(XY+o)
		_B = RLC(RM8(EA));
		WM8(EA, _B);
		break;
	case 0x01: // RLC C=(XY+o)
		_C = RLC(RM8(EA));
		WM8(EA, _C);
		break;
	case 0x02: // RLC D=(XY+o)
		_D = RLC(RM8(EA));
		WM8(EA, _D);
		break;
	case 0x03: // RLC E=(XY+o)
		_E = RLC(RM8(EA));
		WM8(EA, _E);
		break;
	case 0x04: // RLC H=(XY+o)
		_H = RLC(RM8(EA));
		WM8(EA, _H);
		break;
	case 0x05: // RLC L=(XY+o)
		_L = RLC(RM8(EA));
		WM8(EA, _L);
		break;
	case 0x06: // RLC (XY+o)
		WM8(EA, RLC(RM8(EA)));
		break;
	case 0x07: // RLC A=(XY+o)
		_A = RLC(RM8(EA));
		WM8(EA, _A);
		break;
	case 0x08: // RRC B=(XY+o)
		_B = RRC(RM8(EA));
		WM8(EA, _B);
		break;
	case 0x09: // RRC C=(XY+o)
		_C = RRC(RM8(EA));
		WM8(EA, _C);
		break;
	case 0x0a: // RRC D=(XY+o)
		_D = RRC(RM8(EA));
		WM8(EA, _D);
		break;
	case 0x0b: // RRC E=(XY+o)
		_E = RRC(RM8(EA));
		WM8(EA, _E);
		break;
	case 0x0c: // RRC H=(XY+o)
		_H = RRC(RM8(EA));
		WM8(EA, _H);
		break;
	case 0x0d: // RRC L=(XY+o)
		_L = RRC(RM8(EA));
		WM8(EA, _L);
		break;
	case 0x0e: // RRC (XY+o)
		WM8(EA, RRC(RM8(EA)));
		break;
	case 0x0f: // RRC A=(XY+o)
		_A = RRC(RM8(EA));
		WM8(EA, _A);
		break;
	case 0x10: // RL B=(XY+o)
		_B = RL(RM8(EA));
		WM8(EA, _B);
		break;
	case 0x11: // RL C=(XY+o)
		_C = RL(RM8(EA));
		WM8(EA, _C);
		break;
	case 0x12: // RL D=(XY+o)
		_D = RL(RM8(EA));
		WM8(EA, _D);
		break;
	case 0x13: // RL E=(XY+o)
		_E = RL(RM8(EA));
		WM8(EA, _E);
		break;
	case 0x14: // RL H=(XY+o)
		_H = RL(RM8(EA));
		WM8(EA, _H);
		break;
	case 0x15: // RL L=(XY+o)
		_L = RL(RM8(EA));
		WM8(EA, _L);
		break;
	case 0x16: // RL (XY+o)
		WM8(EA, RL(RM8(EA)));
		break;
	case 0x17: // RL A=(XY+o)
		_A = RL(RM8(EA));
		WM8(EA, _A);
		break;
	case 0x18: // RR B=(XY+o)
		_B = RR(RM8(EA));
		WM8(EA, _B);
		break;
	case 0x19: // RR C=(XY+o)
		_C = RR(RM8(EA));
		WM8(EA, _C);
		break;
	case 0x1a: // RR D=(XY+o)
		_D = RR(RM8(EA));
		WM8(EA, _D);
		break;
	case 0x1b: // RR E=(XY+o)
		_E = RR(RM8(EA));
		WM8(EA, _E);
		break;
	case 0x1c: // RR H=(XY+o)
		_H = RR(RM8(EA));
		WM8(EA, _H);
		break;
	case 0x1d: // RR L=(XY+o)
		_L = RR(RM8(EA));
		WM8(EA, _L);
		break;
	case 0x1e: // RR (XY+o)
		WM8(EA, RR(RM8(EA)));
		break;
	case 0x1f: // RR A=(XY+o)
		_A = RR(RM8(EA));
		WM8(EA, _A);
		break;
	case 0x20: // SLA B=(XY+o)
		_B = SLA(RM8(EA));
		WM8(EA, _B);
		break;
	case 0x21: // SLA C=(XY+o)
		_C = SLA(RM8(EA));
		WM8(EA, _C);
		break;
	case 0x22: // SLA D=(XY+o)
		_D = SLA(RM8(EA));
		WM8(EA, _D);
		break;
	case 0x23: // SLA E=(XY+o)
		_E = SLA(RM8(EA));
		WM8(EA, _E);
		break;
	case 0x24: // SLA H=(XY+o)
		_H = SLA(RM8(EA));
		WM8(EA, _H);
		break;
	case 0x25: // SLA L=(XY+o)
		_L = SLA(RM8(EA));
		WM8(EA, _L);
		break;
	case 0x26: // SLA (XY+o)
		WM8(EA, SLA(RM8(EA)));
		break;
	case 0x27: // SLA A=(XY+o)
		_A = SLA(RM8(EA));
		WM8(EA, _A);
		break;
	case 0x28: // SRA B=(XY+o)
		_B = SRA(RM8(EA));
		WM8(EA, _B);
		break;
	case 0x29: // SRA C=(XY+o)
		_C = SRA(RM8(EA));
		WM8(EA, _C);
		break;
	case 0x2a: // SRA D=(XY+o)
		_D = SRA(RM8(EA));
		WM8(EA, _D);
		break;
	case 0x2b: // SRA E=(XY+o)
		_E = SRA(RM8(EA));
		WM8(EA, _E);
		break;
	case 0x2c: // SRA H=(XY+o)
		_H = SRA(RM8(EA));
		WM8(EA, _H);
		break;
	case 0x2d: // SRA L=(XY+o)
		_L = SRA(RM8(EA));
		WM8(EA, _L);
		break;
	case 0x2e: // SRA (XY+o)
		WM8(EA, SRA(RM8(EA)));
		break;
	case 0x2f: // SRA A=(XY+o)
		_A = SRA(RM8(EA));
		WM8(EA, _A);
		break;
	case 0x30: // SLL B=(XY+o)
		_B = SLL(RM8(EA));
		WM8(EA, _B);
		break;
	case 0x31: // SLL C=(XY+o)
		_C = SLL(RM8(EA));
		WM8(EA, _C);
		break;
	case 0x32: // SLL D=(XY+o)
		_D = SLL(RM8(EA));
		WM8(EA, _D);
		break;
	case 0x33: // SLL E=(XY+o)
		_E = SLL(RM8(EA));
		WM8(EA, _E);
		break;
	case 0x34: // SLL H=(XY+o)
		_H = SLL(RM8(EA));
		WM8(EA, _H);
		break;
	case 0x35: // SLL L=(XY+o)
		_L = SLL(RM8(EA));
		WM8(EA, _L);
		break;
	case 0x36: // SLL (XY+o)
		WM8(EA, SLL(RM8(EA)));
		break;
	case 0x37: // SLL A=(XY+o)
		_A = SLL(RM8(EA));
		WM8(EA, _A);
		break;
	case 0x38: // SRL B=(XY+o)
		_B = SRL(RM8(EA));
		WM8(EA, _B);
		break;
	case 0x39: // SRL C=(XY+o)
		_C = SRL(RM8(EA));
		WM8(EA, _C);
		break;
	case 0x3a: // SRL D=(XY+o)
		_D = SRL(RM8(EA));
		WM8(EA, _D);
		break;
	case 0x3b: // SRL E=(XY+o)
		_E = SRL(RM8(EA));
		WM8(EA, _E);
		break;
	case 0x3c: // SRL H=(XY+o)
		_H = SRL(RM8(EA));
		WM8(EA, _H);
		break;
	case 0x3d: // SRL L=(XY+o)
		_L = SRL(RM8(EA));
		WM8(EA, _L);
		break;
	case 0x3e: // SRL (XY+o)
		WM8(EA, SRL(RM8(EA)));
		break;
	case 0x3f: // SRL A=(XY+o)
		_A = SRL(RM8(EA));
		WM8(EA, _A);
		break;
	case 0x40: // BIT 0, B=(XY+o)
	case 0x41: // BIT 0, C=(XY+o)
	case 0x42: // BIT 0, D=(XY+o)
	case 0x43: // BIT 0, E=(XY+o)
	case 0x44: // BIT 0, H=(XY+o)
	case 0x45: // BIT 0, L=(XY+o)
	case 0x46: // BIT 0, (XY+o)
	case 0x47: // BIT 0, A=(XY+o)
		BIT_XY(0, RM8(EA));
		break;
	case 0x48: // BIT 1, B=(XY+o)
	case 0x49: // BIT 1, C=(XY+o)
	case 0x4a: // BIT 1, D=(XY+o)
	case 0x4b: // BIT 1, E=(XY+o)
	case 0x4c: // BIT 1, H=(XY+o)
	case 0x4d: // BIT 1, L=(XY+o)
	case 0x4e: // BIT 1, (XY+o)
	case 0x4f: // BIT 1, A=(XY+o)
		BIT_XY(1, RM8(EA));
		break;
	case 0x50: // BIT 2, B=(XY+o)
	case 0x51: // BIT 2, C=(XY+o)
	case 0x52: // BIT 2, D=(XY+o)
	case 0x53: // BIT 2, E=(XY+o)
	case 0x54: // BIT 2, H=(XY+o)
	case 0x55: // BIT 2, L=(XY+o)
	case 0x56: // BIT 2, (XY+o)
	case 0x57: // BIT 2, A=(XY+o)
		BIT_XY(2, RM8(EA));
		break;
	case 0x58: // BIT 3, B=(XY+o)
	case 0x59: // BIT 3, C=(XY+o)
	case 0x5a: // BIT 3, D=(XY+o)
	case 0x5b: // BIT 3, E=(XY+o)
	case 0x5c: // BIT 3, H=(XY+o)
	case 0x5d: // BIT 3, L=(XY+o)
	case 0x5e: // BIT 3, (XY+o)
	case 0x5f: // BIT 3, A=(XY+o)
		BIT_XY(3, RM8(EA));
		break;
	case 0x60: // BIT 4, B=(XY+o)
	case 0x61: // BIT 4, C=(XY+o)
	case 0x62: // BIT 4, D=(XY+o)
	case 0x63: // BIT 4, E=(XY+o)
	case 0x64: // BIT 4, H=(XY+o)
	case 0x65: // BIT 4, L=(XY+o)
	case 0x66: // BIT 4, (XY+o)
	case 0x67: // BIT 4, A=(XY+o)
		BIT_XY(4, RM8(EA));
		break;
	case 0x68: // BIT 5, B=(XY+o)
	case 0x69: // BIT 5, C=(XY+o)
	case 0x6a: // BIT 5, D=(XY+o)
	case 0x6b: // BIT 5, E=(XY+o)
	case 0x6c: // BIT 5, H=(XY+o)
	case 0x6d: // BIT 5, L=(XY+o)
	case 0x6e: // BIT 5, (XY+o)
	case 0x6f: // BIT 5, A=(XY+o)
		BIT_XY(5, RM8(EA));
		break;
	case 0x70: // BIT 6, B=(XY+o)
	case 0x71: // BIT 6, C=(XY+o)
	case 0x72: // BIT 6, D=(XY+o)
	case 0x73: // BIT 6, E=(XY+o)
	case 0x74: // BIT 6, H=(XY+o)
	case 0x75: // BIT 6, L=(XY+o)
	case 0x76: // BIT 6, (XY+o)
	case 0x77: // BIT 6, A=(XY+o)
		BIT_XY(6, RM8(EA));
		break;
	case 0x78: // BIT 7, B=(XY+o)
	case 0x79: // BIT 7, C=(XY+o)
	case 0x7a: // BIT 7, D=(XY+o)
	case 0x7b: // BIT 7, E=(XY+o)
	case 0x7c: // BIT 7, H=(XY+o)
	case 0x7d: // BIT 7, L=(XY+o)
	case 0x7e: // BIT 7, (XY+o)
	case 0x7f: // BIT 7, A=(XY+o)
		BIT_XY(7, RM8(EA));
		break;
	case 0x80: // RES 0, B=(XY+o)
		_B = RES(0, RM8(EA));
		WM8(EA, _B);
		break;
	case 0x81: // RES 0, C=(XY+o)
		_C = RES(0, RM8(EA));
		WM8(EA, _C);
		break;
	case 0x82: // RES 0, D=(XY+o)
		_D = RES(0, RM8(EA));
		WM8(EA, _D);
		break;
	case 0x83: // RES 0, E=(XY+o)
		_E = RES(0, RM8(EA));
		WM8(EA, _E);
		break;
	case 0x84: // RES 0, H=(XY+o)
		_H = RES(0, RM8(EA));
		WM8(EA, _H);
		break;
	case 0x85: // RES 0, L=(XY+o)
		_L = RES(0, RM8(EA));
		WM8(EA, _L);
		break;
	case 0x86: // RES 0, (XY+o)
		WM8(EA, RES(0, RM8(EA)));
		break;
	case 0x87: // RES 0, A=(XY+o)
		_A = RES(0, RM8(EA));
		WM8(EA, _A);
		break;
	case 0x88: // RES 1, B=(XY+o)
		_B = RES(1, RM8(EA));
		WM8(EA, _B);
		break;
	case 0x89: // RES 1, C=(XY+o)
		_C = RES(1, RM8(EA));
		WM8(EA, _C);
		break;
	case 0x8a: // RES 1, D=(XY+o)
		_D = RES(1, RM8(EA));
		WM8(EA, _D);
		break;
	case 0x8b: // RES 1, E=(XY+o)
		_E = RES(1, RM8(EA));
		WM8(EA, _E);
		break;
	case 0x8c: // RES 1, H=(XY+o)
		_H = RES(1, RM8(EA));
		WM8(EA, _H);
		break;
	case 0x8d: // RES 1, L=(XY+o)
		_L = RES(1, RM8(EA));
		WM8(EA, _L);
		break;
	case 0x8e: // RES 1, (XY+o)
		WM8(EA, RES(1, RM8(EA)));
		break;
	case 0x8f: // RES 1, A=(XY+o)
		_A = RES(1, RM8(EA));
		WM8(EA, _A);
		break;
	case 0x90: // RES 2, B=(XY+o)
		_B = RES(2, RM8(EA));
		WM8(EA, _B);
		break;
	case 0x91: // RES 2, C=(XY+o)
		_C = RES(2, RM8(EA));
		WM8(EA, _C);
		break;
	case 0x92: // RES 2, D=(XY+o)
		_D = RES(2, RM8(EA));
		WM8(EA, _D);
		break;
	case 0x93: // RES 2, E=(XY+o)
		_E = RES(2, RM8(EA));
		WM8(EA, _E);
		break;
	case 0x94: // RES 2, H=(XY+o)
		_H = RES(2, RM8(EA));
		WM8(EA, _H);
		break;
	case 0x95: // RES 2, L=(XY+o)
		_L = RES(2, RM8(EA));
		WM8(EA, _L);
		break;
	case 0x96: // RES 2, (XY+o)
		WM8(EA, RES(2, RM8(EA)));
		break;
	case 0x97: // RES 2, A=(XY+o)
		_A = RES(2, RM8(EA));
		WM8(EA, _A);
		break;
	case 0x98: // RES 3, B=(XY+o)
		_B = RES(3, RM8(EA));
		WM8(EA, _B);
		break;
	case 0x99: // RES 3, C=(XY+o)
		_C = RES(3, RM8(EA));
		WM8(EA, _C);
		break;
	case 0x9a: // RES 3, D=(XY+o)
		_D = RES(3, RM8(EA));
		WM8(EA, _D);
		break;
	case 0x9b: // RES 3, E=(XY+o)
		_E = RES(3, RM8(EA));
		WM8(EA, _E);
		break;
	case 0x9c: // RES 3, H=(XY+o)
		_H = RES(3, RM8(EA));
		WM8(EA, _H);
		break;
	case 0x9d: // RES 3, L=(XY+o)
		_L = RES(3, RM8(EA));
		WM8(EA, _L);
		break;
	case 0x9e: // RES 3, (XY+o)
		WM8(EA, RES(3, RM8(EA)));
		break;
	case 0x9f: // RES 3, A=(XY+o)
		_A = RES(3, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xa0: // RES 4, B=(XY+o)
		_B = RES(4, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xa1: // RES 4, C=(XY+o)
		_C = RES(4, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xa2: // RES 4, D=(XY+o)
		_D = RES(4, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xa3: // RES 4, E=(XY+o)
		_E = RES(4, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xa4: // RES 4, H=(XY+o)
		_H = RES(4, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xa5: // RES 4, L=(XY+o)
		_L = RES(4, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xa6: // RES 4, (XY+o)
		WM8(EA, RES(4, RM8(EA)));
		break;
	case 0xa7: // RES 4, A=(XY+o)
		_A = RES(4, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xa8: // RES 5, B=(XY+o)
		_B = RES(5, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xa9: // RES 5, C=(XY+o)
		_C = RES(5, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xaa: // RES 5, D=(XY+o)
		_D = RES(5, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xab: // RES 5, E=(XY+o)
		_E = RES(5, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xac: // RES 5, H=(XY+o)
		_H = RES(5, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xad: // RES 5, L=(XY+o)
		_L = RES(5, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xae: // RES 5, (XY+o)
		WM8(EA, RES(5, RM8(EA)));
		break;
	case 0xaf: // RES 5, A=(XY+o)
		_A = RES(5, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xb0: // RES 6, B=(XY+o)
		_B = RES(6, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xb1: // RES 6, C=(XY+o)
		_C = RES(6, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xb2: // RES 6, D=(XY+o)
		_D = RES(6, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xb3: // RES 6, E=(XY+o)
		_E = RES(6, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xb4: // RES 6, H=(XY+o)
		_H = RES(6, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xb5: // RES 6, L=(XY+o)
		_L = RES(6, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xb6: // RES 6, (XY+o)
		WM8(EA, RES(6, RM8(EA)));
		break;
	case 0xb7: // RES 6, A=(XY+o)
		_A = RES(6, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xb8: // RES 7, B=(XY+o)
		_B = RES(7, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xb9: // RES 7, C=(XY+o)
		_C = RES(7, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xba: // RES 7, D=(XY+o)
		_D = RES(7, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xbb: // RES 7, E=(XY+o)
		_E = RES(7, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xbc: // RES 7, H=(XY+o)
		_H = RES(7, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xbd: // RES 7, L=(XY+o)
		_L = RES(7, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xbe: // RES 7, (XY+o)
		WM8(EA, RES(7, RM8(EA)));
		break;
	case 0xbf: // RES 7, A=(XY+o)
		_A = RES(7, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xc0: // SET 0, B=(XY+o)
		_B = SET(0, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xc1: // SET 0, C=(XY+o)
		_C = SET(0, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xc2: // SET 0, D=(XY+o)
		_D = SET(0, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xc3: // SET 0, E=(XY+o)
		_E = SET(0, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xc4: // SET 0, H=(XY+o)
		_H = SET(0, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xc5: // SET 0, L=(XY+o)
		_L = SET(0, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xc6: // SET 0, (XY+o)
		WM8(EA, SET(0, RM8(EA)));
		break;
	case 0xc7: // SET 0, A=(XY+o)
		_A = SET(0, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xc8: // SET 1, B=(XY+o)
		_B = SET(1, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xc9: // SET 1, C=(XY+o)
		_C = SET(1, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xca: // SET 1, D=(XY+o)
		_D = SET(1, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xcb: // SET 1, E=(XY+o)
		_E = SET(1, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xcc: // SET 1, H=(XY+o)
		_H = SET(1, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xcd: // SET 1, L=(XY+o)
		_L = SET(1, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xce: // SET 1, (XY+o)
		WM8(EA, SET(1, RM8(EA)));
		break;
	case 0xcf: // SET 1, A=(XY+o)
		_A = SET(1, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xd0: // SET 2, B=(XY+o)
		_B = SET(2, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xd1: // SET 2, C=(XY+o)
		_C = SET(2, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xd2: // SET 2, D=(XY+o)
		_D = SET(2, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xd3: // SET 2, E=(XY+o)
		_E = SET(2, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xd4: // SET 2, H=(XY+o)
		_H = SET(2, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xd5: // SET 2, L=(XY+o)
		_L = SET(2, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xd6: // SET 2, (XY+o)
		WM8(EA, SET(2, RM8(EA)));
		break;
	case 0xd7: // SET 2, A=(XY+o)
		_A = SET(2, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xd8: // SET 3, B=(XY+o)
		_B = SET(3, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xd9: // SET 3, C=(XY+o)
		_C = SET(3, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xda: // SET 3, D=(XY+o)
		_D = SET(3, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xdb: // SET 3, E=(XY+o)
		_E = SET(3, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xdc: // SET 3, H=(XY+o)
		_H = SET(3, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xdd: // SET 3, L=(XY+o)
		_L = SET(3, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xde: // SET 3, (XY+o)
		WM8(EA, SET(3, RM8(EA)));
		break;
	case 0xdf: // SET 3, A=(XY+o)
		_A = SET(3, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xe0: // SET 4, B=(XY+o)
		_B = SET(4, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xe1: // SET 4, C=(XY+o)
		_C = SET(4, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xe2: // SET 4, D=(XY+o)
		_D = SET(4, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xe3: // SET 4, E=(XY+o)
		_E = SET(4, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xe4: // SET 4, H=(XY+o)
		_H = SET(4, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xe5: // SET 4, L=(XY+o)
		_L = SET(4, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xe6: // SET 4, (XY+o)
		WM8(EA, SET(4, RM8(EA)));
		break;
	case 0xe7: // SET 4, A=(XY+o)
		_A = SET(4, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xe8: // SET 5, B=(XY+o)
		_B = SET(5, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xe9: // SET 5, C=(XY+o)
		_C = SET(5, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xea: // SET 5, D=(XY+o)
		_D = SET(5, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xeb: // SET 5, E=(XY+o)
		_E = SET(5, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xec: // SET 5, H=(XY+o)
		_H = SET(5, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xed: // SET 5, L=(XY+o)
		_L = SET(5, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xee: // SET 5, (XY+o)
		WM8(EA, SET(5, RM8(EA)));
		break;
	case 0xef: // SET 5, A=(XY+o)
		_A = SET(5, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xf0: // SET 6, B=(XY+o)
		_B = SET(6, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xf1: // SET 6, C=(XY+o)
		_C = SET(6, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xf2: // SET 6, D=(XY+o)
		_D = SET(6, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xf3: // SET 6, E=(XY+o)
		_E = SET(6, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xf4: // SET 6, H=(XY+o)
		_H = SET(6, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xf5: // SET 6, L=(XY+o)
		_L = SET(6, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xf6: // SET 6, (XY+o)
		WM8(EA, SET(6, RM8(EA)));
		break;
	case 0xf7: // SET 6, A=(XY+o)
		_A = SET(6, RM8(EA));
		WM8(EA, _A);
		break;
	case 0xf8: // SET 7, B=(XY+o)
		_B = SET(7, RM8(EA));
		WM8(EA, _B);
		break;
	case 0xf9: // SET 7, C=(XY+o)
		_C = SET(7, RM8(EA));
		WM8(EA, _C);
		break;
	case 0xfa: // SET 7, D=(XY+o)
		_D = SET(7, RM8(EA));
		WM8(EA, _D);
		break;
	case 0xfb: // SET 7, E=(XY+o)
		_E = SET(7, RM8(EA));
		WM8(EA, _E);
		break;
	case 0xfc: // SET 7, H=(XY+o)
		_H = SET(7, RM8(EA));
		WM8(EA, _H);
		break;
	case 0xfd: // SET 7, L=(XY+o)
		_L = SET(7, RM8(EA));
		WM8(EA, _L);
		break;
	case 0xfe: // SET 7, (XY+o)
		WM8(EA, SET(7, RM8(EA)));
		break;
	case 0xff: // SET 7, A=(XY+o)
		_A = SET(7, RM8(EA));
		WM8(EA, _A);
		break;
	}
}

#ifdef _CPU_DEBUG_LOG
void Z80::DASM()
{
	memset(debug_dasm, 0, sizeof(debug_dasm));
	debug_ptr = 0;
	uint8 code = DEBUG_FETCHOP();
	
	switch(code) {
	case 0x00: _stprintf(debug_dasm, _T("NOP")); break;
	case 0x01: _stprintf(debug_dasm, _T("LD BC, %4x"), DEBUG_FETCH16()); break;
	case 0x02: _stprintf(debug_dasm, _T("LD (BC), A")); break;
	case 0x03: _stprintf(debug_dasm, _T("INC BC")); break;
	case 0x04: _stprintf(debug_dasm, _T("INC B")); break;
	case 0x05: _stprintf(debug_dasm, _T("DEC B")); break;
	case 0x06: _stprintf(debug_dasm, _T("LD B, %2x"), DEBUG_FETCH8()); break;
	case 0x07: _stprintf(debug_dasm, _T("RLCA")); break;
	case 0x08: _stprintf(debug_dasm, _T("EX AF, AF'")); break;
	case 0x09: _stprintf(debug_dasm, _T("ADD HL, BC")); break;
	case 0x0a: _stprintf(debug_dasm, _T("LD A, (BC)")); break;
	case 0x0b: _stprintf(debug_dasm, _T("DEC BC")); break;
	case 0x0c: _stprintf(debug_dasm, _T("INC C")); break;
	case 0x0d: _stprintf(debug_dasm, _T("DEC C")); break;
	case 0x0e: _stprintf(debug_dasm, _T("LD C, %2x"), DEBUG_FETCH8()); break;
	case 0x0f: _stprintf(debug_dasm, _T("RRCA")); break;
	case 0x10: _stprintf(debug_dasm, _T("DJNZ %4x"), DEBUG_FETCH8_RELPC()); break;
	case 0x11: _stprintf(debug_dasm, _T("LD DE, %4x"), DEBUG_FETCH16()); break;
	case 0x12: _stprintf(debug_dasm, _T("LD (DE), A")); break;
	case 0x13: _stprintf(debug_dasm, _T("INC DE")); break;
	case 0x14: _stprintf(debug_dasm, _T("INC D")); break;
	case 0x15: _stprintf(debug_dasm, _T("DEC D")); break;
	case 0x16: _stprintf(debug_dasm, _T("LD D, %2x"), DEBUG_FETCH8()); break;
	case 0x17: _stprintf(debug_dasm, _T("RLA")); break;
	case 0x18: _stprintf(debug_dasm, _T("JR %4x"), DEBUG_FETCH8_RELPC()); break;
	case 0x19: _stprintf(debug_dasm, _T("ADD HL, DE")); break;
	case 0x1a: _stprintf(debug_dasm, _T("LD A, (DE)")); break;
	case 0x1b: _stprintf(debug_dasm, _T("DEC DE")); break;
	case 0x1c: _stprintf(debug_dasm, _T("INC E")); break;
	case 0x1d: _stprintf(debug_dasm, _T("DEC E")); break;
	case 0x1e: _stprintf(debug_dasm, _T("LD E, %2x"), DEBUG_FETCH8()); break;
	case 0x1f: _stprintf(debug_dasm, _T("RRA")); break;
	case 0x20: _stprintf(debug_dasm, _T("JR NZ, %4x"), DEBUG_FETCH8_RELPC()); break;
	case 0x21: _stprintf(debug_dasm, _T("LD HL, %4x"), DEBUG_FETCH16()); break;
	case 0x22: _stprintf(debug_dasm, _T("LD (%4x), HL"), DEBUG_FETCH16()); break;
	case 0x23: _stprintf(debug_dasm, _T("INC HL")); break;
	case 0x24: _stprintf(debug_dasm, _T("INC H")); break;
	case 0x25: _stprintf(debug_dasm, _T("DEC H")); break;
	case 0x26: _stprintf(debug_dasm, _T("LD H, %2x"), DEBUG_FETCH8()); break;
	case 0x27: _stprintf(debug_dasm, _T("DAA")); break;
	case 0x28: _stprintf(debug_dasm, _T("JR Z, %4x"), DEBUG_FETCH8_RELPC()); break;
	case 0x29: _stprintf(debug_dasm, _T("ADD HL, HL")); break;
	case 0x2a: _stprintf(debug_dasm, _T("LD HL, (%4x)"), DEBUG_FETCH16()); break;
	case 0x2b: _stprintf(debug_dasm, _T("DEC HL")); break;
	case 0x2c: _stprintf(debug_dasm, _T("INC L")); break;
	case 0x2d: _stprintf(debug_dasm, _T("DEC L")); break;
	case 0x2e: _stprintf(debug_dasm, _T("LD L, %2x"), DEBUG_FETCH8()); break;
	case 0x2f: _stprintf(debug_dasm, _T("CPL")); break;
	case 0x30: _stprintf(debug_dasm, _T("JR NC, %4x"), DEBUG_FETCH8_RELPC()); break;
	case 0x31: _stprintf(debug_dasm, _T("LD SP, %4x"), DEBUG_FETCH16()); break;
	case 0x32: _stprintf(debug_dasm, _T("LD (%4x), A"), DEBUG_FETCH16()); break;
	case 0x33: _stprintf(debug_dasm, _T("INC SP")); break;
	case 0x34: _stprintf(debug_dasm, _T("INC (HL)")); break;
	case 0x35: _stprintf(debug_dasm, _T("DEC (HL)")); break;
	case 0x36: _stprintf(debug_dasm, _T("LD (HL), %2x"), DEBUG_FETCH8()); break;
	case 0x37: _stprintf(debug_dasm, _T("SCF")); break;
	case 0x38: _stprintf(debug_dasm, _T("JR C, %4x"), DEBUG_FETCH8_RELPC()); break;
	case 0x39: _stprintf(debug_dasm, _T("ADD HL, SP")); break;
	case 0x3a: _stprintf(debug_dasm, _T("LD A, (%4x)"), DEBUG_FETCH16()); break;
	case 0x3b: _stprintf(debug_dasm, _T("DEC SP")); break;
	case 0x3c: _stprintf(debug_dasm, _T("INC A")); break;
	case 0x3d: _stprintf(debug_dasm, _T("DEC A")); break;
	case 0x3e: _stprintf(debug_dasm, _T("LD A, %2x"), DEBUG_FETCH8()); break;
	case 0x3f: _stprintf(debug_dasm, _T("CCF")); break;
	case 0x40: _stprintf(debug_dasm, _T("LD B, B")); break;
	case 0x41: _stprintf(debug_dasm, _T("LD B, C")); break;
	case 0x42: _stprintf(debug_dasm, _T("LD B, D")); break;
	case 0x43: _stprintf(debug_dasm, _T("LD B, E")); break;
	case 0x44: _stprintf(debug_dasm, _T("LD B, H")); break;
	case 0x45: _stprintf(debug_dasm, _T("LD B, L")); break;
	case 0x46: _stprintf(debug_dasm, _T("LD B, (HL)")); break;
	case 0x47: _stprintf(debug_dasm, _T("LD B, A")); break;
	case 0x48: _stprintf(debug_dasm, _T("LD C, B")); break;
	case 0x49: _stprintf(debug_dasm, _T("LD C, C")); break;
	case 0x4a: _stprintf(debug_dasm, _T("LD C, D")); break;
	case 0x4b: _stprintf(debug_dasm, _T("LD C, E")); break;
	case 0x4c: _stprintf(debug_dasm, _T("LD C, H")); break;
	case 0x4d: _stprintf(debug_dasm, _T("LD C, L")); break;
	case 0x4e: _stprintf(debug_dasm, _T("LD C, (HL)")); break;
	case 0x4f: _stprintf(debug_dasm, _T("LD C, A")); break;
	case 0x50: _stprintf(debug_dasm, _T("LD D, B")); break;
	case 0x51: _stprintf(debug_dasm, _T("LD D, C")); break;
	case 0x52: _stprintf(debug_dasm, _T("LD D, D")); break;
	case 0x53: _stprintf(debug_dasm, _T("LD D, E")); break;
	case 0x54: _stprintf(debug_dasm, _T("LD D, H")); break;
	case 0x55: _stprintf(debug_dasm, _T("LD D, L")); break;
	case 0x56: _stprintf(debug_dasm, _T("LD D, (HL)")); break;
	case 0x57: _stprintf(debug_dasm, _T("LD D, A")); break;
	case 0x58: _stprintf(debug_dasm, _T("LD E, B")); break;
	case 0x59: _stprintf(debug_dasm, _T("LD E, C")); break;
	case 0x5a: _stprintf(debug_dasm, _T("LD E, D")); break;
	case 0x5b: _stprintf(debug_dasm, _T("LD E, E")); break;
	case 0x5c: _stprintf(debug_dasm, _T("LD E, H")); break;
	case 0x5d: _stprintf(debug_dasm, _T("LD E, L")); break;
	case 0x5e: _stprintf(debug_dasm, _T("LD E, (HL)")); break;
	case 0x5f: _stprintf(debug_dasm, _T("LD E, A")); break;
	case 0x60: _stprintf(debug_dasm, _T("LD H, B")); break;
	case 0x61: _stprintf(debug_dasm, _T("LD H, C")); break;
	case 0x62: _stprintf(debug_dasm, _T("LD H, D")); break;
	case 0x63: _stprintf(debug_dasm, _T("LD H, E")); break;
	case 0x64: _stprintf(debug_dasm, _T("LD H, H")); break;
	case 0x65: _stprintf(debug_dasm, _T("LD H, L")); break;
	case 0x66: _stprintf(debug_dasm, _T("LD H, (HL)")); break;
	case 0x67: _stprintf(debug_dasm, _T("LD H, A")); break;
	case 0x68: _stprintf(debug_dasm, _T("LD L, B")); break;
	case 0x69: _stprintf(debug_dasm, _T("LD L, C")); break;
	case 0x6a: _stprintf(debug_dasm, _T("LD L, D")); break;
	case 0x6b: _stprintf(debug_dasm, _T("LD L, E")); break;
	case 0x6c: _stprintf(debug_dasm, _T("LD L, H")); break;
	case 0x6d: _stprintf(debug_dasm, _T("LD L, L")); break;
	case 0x6e: _stprintf(debug_dasm, _T("LD L, (HL)")); break;
	case 0x6f: _stprintf(debug_dasm, _T("LD L, A")); break;
	case 0x70: _stprintf(debug_dasm, _T("LD (HL), B")); break;
	case 0x71: _stprintf(debug_dasm, _T("LD (HL), C")); break;
	case 0x72: _stprintf(debug_dasm, _T("LD (HL), D")); break;
	case 0x73: _stprintf(debug_dasm, _T("LD (HL), E")); break;
	case 0x74: _stprintf(debug_dasm, _T("LD (HL), H")); break;
	case 0x75: _stprintf(debug_dasm, _T("LD (HL), L")); break;
	case 0x76: _stprintf(debug_dasm, _T("HALT")); break;
	case 0x77: _stprintf(debug_dasm, _T("LD (HL), A")); break;
	case 0x78: _stprintf(debug_dasm, _T("LD A, B")); break;
	case 0x79: _stprintf(debug_dasm, _T("LD A, C")); break;
	case 0x7a: _stprintf(debug_dasm, _T("LD A, D")); break;
	case 0x7b: _stprintf(debug_dasm, _T("LD A, E")); break;
	case 0x7c: _stprintf(debug_dasm, _T("LD A, H")); break;
	case 0x7d: _stprintf(debug_dasm, _T("LD A, L")); break;
	case 0x7e: _stprintf(debug_dasm, _T("LD A, (HL)")); break;
	case 0x7f: _stprintf(debug_dasm, _T("LD A, A")); break;
	case 0x80: _stprintf(debug_dasm, _T("ADD A, B")); break;
	case 0x81: _stprintf(debug_dasm, _T("ADD A, C")); break;
	case 0x82: _stprintf(debug_dasm, _T("ADD A, D")); break;
	case 0x83: _stprintf(debug_dasm, _T("ADD A, E")); break;
	case 0x84: _stprintf(debug_dasm, _T("ADD A, H")); break;
	case 0x85: _stprintf(debug_dasm, _T("ADD A, L")); break;
	case 0x86: _stprintf(debug_dasm, _T("ADD A, (HL)")); break;
	case 0x87: _stprintf(debug_dasm, _T("ADD A, A")); break;
	case 0x88: _stprintf(debug_dasm, _T("ADC A, B")); break;
	case 0x89: _stprintf(debug_dasm, _T("ADC A, C")); break;
	case 0x8a: _stprintf(debug_dasm, _T("ADC A, D")); break;
	case 0x8b: _stprintf(debug_dasm, _T("ADC A, E")); break;
	case 0x8c: _stprintf(debug_dasm, _T("ADC A, H")); break;
	case 0x8d: _stprintf(debug_dasm, _T("ADC A, L")); break;
	case 0x8e: _stprintf(debug_dasm, _T("ADC A, (HL)")); break;
	case 0x8f: _stprintf(debug_dasm, _T("ADC A, A")); break;
	case 0x90: _stprintf(debug_dasm, _T("SUB B")); break;
	case 0x91: _stprintf(debug_dasm, _T("SUB C")); break;
	case 0x92: _stprintf(debug_dasm, _T("SUB D")); break;
	case 0x93: _stprintf(debug_dasm, _T("SUB E")); break;
	case 0x94: _stprintf(debug_dasm, _T("SUB H")); break;
	case 0x95: _stprintf(debug_dasm, _T("SUB L")); break;
	case 0x96: _stprintf(debug_dasm, _T("SUB (HL)")); break;
	case 0x97: _stprintf(debug_dasm, _T("SUB A")); break;
	case 0x98: _stprintf(debug_dasm, _T("SBC A, B")); break;
	case 0x99: _stprintf(debug_dasm, _T("SBC A, C")); break;
	case 0x9a: _stprintf(debug_dasm, _T("SBC A, D")); break;
	case 0x9b: _stprintf(debug_dasm, _T("SBC A, E")); break;
	case 0x9c: _stprintf(debug_dasm, _T("SBC A, H")); break;
	case 0x9d: _stprintf(debug_dasm, _T("SBC A, L")); break;
	case 0x9e: _stprintf(debug_dasm, _T("SBC A, (HL)")); break;
	case 0x9f: _stprintf(debug_dasm, _T("SBC A, A")); break;
	case 0xa0: _stprintf(debug_dasm, _T("AND B")); break;
	case 0xa1: _stprintf(debug_dasm, _T("AND C")); break;
	case 0xa2: _stprintf(debug_dasm, _T("AND D")); break;
	case 0xa3: _stprintf(debug_dasm, _T("AND E")); break;
	case 0xa4: _stprintf(debug_dasm, _T("AND H")); break;
	case 0xa5: _stprintf(debug_dasm, _T("AND L")); break;
	case 0xa6: _stprintf(debug_dasm, _T("AND (HL)")); break;
	case 0xa7: _stprintf(debug_dasm, _T("AND A")); break;
	case 0xa8: _stprintf(debug_dasm, _T("XOR B")); break;
	case 0xa9: _stprintf(debug_dasm, _T("XOR C")); break;
	case 0xaa: _stprintf(debug_dasm, _T("XOR D")); break;
	case 0xab: _stprintf(debug_dasm, _T("XOR E")); break;
	case 0xac: _stprintf(debug_dasm, _T("XOR H")); break;
	case 0xad: _stprintf(debug_dasm, _T("XOR L")); break;
	case 0xae: _stprintf(debug_dasm, _T("XOR (HL)")); break;
	case 0xaf: _stprintf(debug_dasm, _T("XOR A")); break;
	case 0xb0: _stprintf(debug_dasm, _T("OR B")); break;
	case 0xb1: _stprintf(debug_dasm, _T("OR C")); break;
	case 0xb2: _stprintf(debug_dasm, _T("OR D")); break;
	case 0xb3: _stprintf(debug_dasm, _T("OR E")); break;
	case 0xb4: _stprintf(debug_dasm, _T("OR H")); break;
	case 0xb5: _stprintf(debug_dasm, _T("OR L")); break;
	case 0xb6: _stprintf(debug_dasm, _T("OR (HL)")); break;
	case 0xb7: _stprintf(debug_dasm, _T("OR A")); break;
	case 0xb8: _stprintf(debug_dasm, _T("CP B")); break;
	case 0xb9: _stprintf(debug_dasm, _T("CP C")); break;
	case 0xba: _stprintf(debug_dasm, _T("CP D")); break;
	case 0xbb: _stprintf(debug_dasm, _T("CP E")); break;
	case 0xbc: _stprintf(debug_dasm, _T("CP H")); break;
	case 0xbd: _stprintf(debug_dasm, _T("CP L")); break;
	case 0xbe: _stprintf(debug_dasm, _T("CP (HL)")); break;
	case 0xbf: _stprintf(debug_dasm, _T("CP A")); break;
	case 0xc0: _stprintf(debug_dasm, _T("RET NZ")); break;
	case 0xc1: _stprintf(debug_dasm, _T("POP BC")); break;
	case 0xc2: _stprintf(debug_dasm, _T("JP NZ, %4x"), DEBUG_FETCH16()); break;
	case 0xc3: _stprintf(debug_dasm, _T("JP %4x"), DEBUG_FETCH16()); break;
	case 0xc4: _stprintf(debug_dasm, _T("CALL NZ, %4x"), DEBUG_FETCH16()); break;
	case 0xc5: _stprintf(debug_dasm, _T("PUSH BC")); break;
	case 0xc6: _stprintf(debug_dasm, _T("ADD A, %2x"), DEBUG_FETCH8()); break;
	case 0xc7: _stprintf(debug_dasm, _T("RST 0")); break;
	case 0xc8: _stprintf(debug_dasm, _T("RET Z")); break;
	case 0xc9: _stprintf(debug_dasm, _T("RET")); break;
	case 0xca: _stprintf(debug_dasm, _T("JP Z, %4x"), DEBUG_FETCH16()); break;
	case 0xcb: DASM_CB(); break;
	case 0xcc: _stprintf(debug_dasm, _T("CALL Z, %4x"), DEBUG_FETCH16()); break;
	case 0xcd: _stprintf(debug_dasm, _T("CALL %4x"), DEBUG_FETCH16()); break;
	case 0xce: _stprintf(debug_dasm, _T("ADC A, %2x"), DEBUG_FETCH8()); break;
	case 0xcf: _stprintf(debug_dasm, _T("RST 1")); break;
	case 0xd0: _stprintf(debug_dasm, _T("RET NC")); break;
	case 0xd1: _stprintf(debug_dasm, _T("POP DE")); break;
	case 0xd2: _stprintf(debug_dasm, _T("JP NC, %4x"), DEBUG_FETCH16()); break;
	case 0xd3: _stprintf(debug_dasm, _T("OUT (%2x), A"), DEBUG_FETCH8()); break;
	case 0xd4: _stprintf(debug_dasm, _T("CALL NC, %4x"), DEBUG_FETCH16()); break;
	case 0xd5: _stprintf(debug_dasm, _T("PUSH DE")); break;
	case 0xd6: _stprintf(debug_dasm, _T("SUB %2x"), DEBUG_FETCH8()); break;
	case 0xd7: _stprintf(debug_dasm, _T("RST 2")); break;
	case 0xd8: _stprintf(debug_dasm, _T("RET C")); break;
	case 0xd9: _stprintf(debug_dasm, _T("EXX")); break;
	case 0xda: _stprintf(debug_dasm, _T("JP C, %4x"), DEBUG_FETCH16()); break;
	case 0xdb: _stprintf(debug_dasm, _T("IN A, (%2x)"), DEBUG_FETCH8()); break;
	case 0xdc: _stprintf(debug_dasm, _T("CALL C, %4x"), DEBUG_FETCH16()); break;
	case 0xdd: DASM_DD(); break;
	case 0xde: _stprintf(debug_dasm, _T("SBC A, %2x"), DEBUG_FETCH8()); break;
	case 0xdf: _stprintf(debug_dasm, _T("RST 3")); break;
	case 0xe0: _stprintf(debug_dasm, _T("RET PO")); break;
	case 0xe1: _stprintf(debug_dasm, _T("POP HL")); break;
	case 0xe2: _stprintf(debug_dasm, _T("JP PO, %4x"), DEBUG_FETCH16()); break;
	case 0xe3: _stprintf(debug_dasm, _T("EX HL, (SP)")); break;
	case 0xe4: _stprintf(debug_dasm, _T("CALL PO, %4x"), DEBUG_FETCH16()); break;
	case 0xe5: _stprintf(debug_dasm, _T("PUSH HL")); break;
	case 0xe6: _stprintf(debug_dasm, _T("AND %2x"), DEBUG_FETCH8()); break;
	case 0xe7: _stprintf(debug_dasm, _T("RST 4")); break;
	case 0xe8: _stprintf(debug_dasm, _T("RET PE")); break;
	case 0xe9: _stprintf(debug_dasm, _T("JP (HL)")); break;
	case 0xea: _stprintf(debug_dasm, _T("JP PE, %4x"), DEBUG_FETCH16()); break;
	case 0xeb: _stprintf(debug_dasm, _T("EX DE, HL")); break;
	case 0xec: _stprintf(debug_dasm, _T("CALL PE, %4x"), DEBUG_FETCH16()); break;
	case 0xed: DASM_ED(); break;
	case 0xee: _stprintf(debug_dasm, _T("XOR %2x"), DEBUG_FETCH8()); break;
	case 0xef: _stprintf(debug_dasm, _T("RST 5")); break;
	case 0xf0: _stprintf(debug_dasm, _T("RET P")); break;
	case 0xf1: _stprintf(debug_dasm, _T("POP AF")); break;
	case 0xf2: _stprintf(debug_dasm, _T("JP P, %4x"), DEBUG_FETCH16()); break;
	case 0xf3: _stprintf(debug_dasm, _T("DI")); break;
	case 0xf4: _stprintf(debug_dasm, _T("CALL P, %4x"), DEBUG_FETCH16()); break;
	case 0xf5: _stprintf(debug_dasm, _T("PUSH AF")); break;
	case 0xf6: _stprintf(debug_dasm, _T("OR %2x"), DEBUG_FETCH8()); break;
	case 0xf7: _stprintf(debug_dasm, _T("RST 6")); break;
	case 0xf8: _stprintf(debug_dasm, _T("RET M")); break;
	case 0xf9: _stprintf(debug_dasm, _T("LD SP, HL")); break;
	case 0xfa: _stprintf(debug_dasm, _T("JP M, %4x"), DEBUG_FETCH16()); break;
	case 0xfb: _stprintf(debug_dasm, _T("EI")); break;
	case 0xfc: _stprintf(debug_dasm, _T("CALL M, %4x"), DEBUG_FETCH16()); break;
	case 0xfd: DASM_FD();
	case 0xfe: _stprintf(debug_dasm, _T("CP %2x"), DEBUG_FETCH8()); break;
	case 0xff: _stprintf(debug_dasm, _T("RST 7")); break;
	}
}

void Z80::DASM_CB()
{
	uint8 code = DEBUG_FETCHOP();
	
	switch(code) {
	case 0x00: _stprintf(debug_dasm, _T("RLC B")); break;
	case 0x01: _stprintf(debug_dasm, _T("RLC C")); break;
	case 0x02: _stprintf(debug_dasm, _T("RLC D")); break;
	case 0x03: _stprintf(debug_dasm, _T("RLC E")); break;
	case 0x04: _stprintf(debug_dasm, _T("RLC H")); break;
	case 0x05: _stprintf(debug_dasm, _T("RLC L")); break;
	case 0x06: _stprintf(debug_dasm, _T("RLC (HL)")); break;
	case 0x07: _stprintf(debug_dasm, _T("RLC A")); break;
	case 0x08: _stprintf(debug_dasm, _T("RRC B")); break;
	case 0x09: _stprintf(debug_dasm, _T("RRC C")); break;
	case 0x0a: _stprintf(debug_dasm, _T("RRC D")); break;
	case 0x0b: _stprintf(debug_dasm, _T("RRC E")); break;
	case 0x0c: _stprintf(debug_dasm, _T("RRC H")); break;
	case 0x0d: _stprintf(debug_dasm, _T("RRC L")); break;
	case 0x0e: _stprintf(debug_dasm, _T("RRC (HL)")); break;
	case 0x0f: _stprintf(debug_dasm, _T("RRC A")); break;
	case 0x10: _stprintf(debug_dasm, _T("RL B")); break;
	case 0x11: _stprintf(debug_dasm, _T("RL C")); break;
	case 0x12: _stprintf(debug_dasm, _T("RL D")); break;
	case 0x13: _stprintf(debug_dasm, _T("RL E")); break;
	case 0x14: _stprintf(debug_dasm, _T("RL H")); break;
	case 0x15: _stprintf(debug_dasm, _T("RL L")); break;
	case 0x16: _stprintf(debug_dasm, _T("RL (HL)")); break;
	case 0x17: _stprintf(debug_dasm, _T("RL A")); break;
	case 0x18: _stprintf(debug_dasm, _T("RR B")); break;
	case 0x19: _stprintf(debug_dasm, _T("RR C")); break;
	case 0x1a: _stprintf(debug_dasm, _T("RR D")); break;
	case 0x1b: _stprintf(debug_dasm, _T("RR E")); break;
	case 0x1c: _stprintf(debug_dasm, _T("RR H")); break;
	case 0x1d: _stprintf(debug_dasm, _T("RR L")); break;
	case 0x1e: _stprintf(debug_dasm, _T("RR (HL)")); break;
	case 0x1f: _stprintf(debug_dasm, _T("RR A")); break;
	case 0x20: _stprintf(debug_dasm, _T("SLA B")); break;
	case 0x21: _stprintf(debug_dasm, _T("SLA C")); break;
	case 0x22: _stprintf(debug_dasm, _T("SLA D")); break;
	case 0x23: _stprintf(debug_dasm, _T("SLA E")); break;
	case 0x24: _stprintf(debug_dasm, _T("SLA H")); break;
	case 0x25: _stprintf(debug_dasm, _T("SLA L")); break;
	case 0x26: _stprintf(debug_dasm, _T("SLA (HL)")); break;
	case 0x27: _stprintf(debug_dasm, _T("SLA A")); break;
	case 0x28: _stprintf(debug_dasm, _T("SRA B")); break;
	case 0x29: _stprintf(debug_dasm, _T("SRA C")); break;
	case 0x2a: _stprintf(debug_dasm, _T("SRA D")); break;
	case 0x2b: _stprintf(debug_dasm, _T("SRA E")); break;
	case 0x2c: _stprintf(debug_dasm, _T("SRA H")); break;
	case 0x2d: _stprintf(debug_dasm, _T("SRA L")); break;
	case 0x2e: _stprintf(debug_dasm, _T("SRA (HL)")); break;
	case 0x2f: _stprintf(debug_dasm, _T("SRA A")); break;
	case 0x30: _stprintf(debug_dasm, _T("SLL B")); break;
	case 0x31: _stprintf(debug_dasm, _T("SLL C")); break;
	case 0x32: _stprintf(debug_dasm, _T("SLL D")); break;
	case 0x33: _stprintf(debug_dasm, _T("SLL E")); break;
	case 0x34: _stprintf(debug_dasm, _T("SLL H")); break;
	case 0x35: _stprintf(debug_dasm, _T("SLL L")); break;
	case 0x36: _stprintf(debug_dasm, _T("SLL (HL)")); break;
	case 0x37: _stprintf(debug_dasm, _T("SLL A")); break;
	case 0x38: _stprintf(debug_dasm, _T("SRL B")); break;
	case 0x39: _stprintf(debug_dasm, _T("SRL C")); break;
	case 0x3a: _stprintf(debug_dasm, _T("SRL D")); break;
	case 0x3b: _stprintf(debug_dasm, _T("SRL E")); break;
	case 0x3c: _stprintf(debug_dasm, _T("SRL H")); break;
	case 0x3d: _stprintf(debug_dasm, _T("SRL L")); break;
	case 0x3e: _stprintf(debug_dasm, _T("SRL (HL)")); break;
	case 0x3f: _stprintf(debug_dasm, _T("SRL A")); break;
	case 0x40: _stprintf(debug_dasm, _T("BIT 0, B")); break;
	case 0x41: _stprintf(debug_dasm, _T("BIT 0, C")); break;
	case 0x42: _stprintf(debug_dasm, _T("BIT 0, D")); break;
	case 0x43: _stprintf(debug_dasm, _T("BIT 0, E")); break;
	case 0x44: _stprintf(debug_dasm, _T("BIT 0, H")); break;
	case 0x45: _stprintf(debug_dasm, _T("BIT 0, L")); break;
	case 0x46: _stprintf(debug_dasm, _T("BIT 0, (HL)")); break;
	case 0x47: _stprintf(debug_dasm, _T("BIT 0, A")); break;
	case 0x48: _stprintf(debug_dasm, _T("BIT 1, B")); break;
	case 0x49: _stprintf(debug_dasm, _T("BIT 1, C")); break;
	case 0x4a: _stprintf(debug_dasm, _T("BIT 1, D")); break;
	case 0x4b: _stprintf(debug_dasm, _T("BIT 1, E")); break;
	case 0x4c: _stprintf(debug_dasm, _T("BIT 1, H")); break;
	case 0x4d: _stprintf(debug_dasm, _T("BIT 1, L")); break;
	case 0x4e: _stprintf(debug_dasm, _T("BIT 1, (HL)")); break;
	case 0x4f: _stprintf(debug_dasm, _T("BIT 1, A")); break;
	case 0x50: _stprintf(debug_dasm, _T("BIT 2, B")); break;
	case 0x51: _stprintf(debug_dasm, _T("BIT 2, C")); break;
	case 0x52: _stprintf(debug_dasm, _T("BIT 2, D")); break;
	case 0x53: _stprintf(debug_dasm, _T("BIT 2, E")); break;
	case 0x54: _stprintf(debug_dasm, _T("BIT 2, H")); break;
	case 0x55: _stprintf(debug_dasm, _T("BIT 2, L")); break;
	case 0x56: _stprintf(debug_dasm, _T("BIT 2, (HL)")); break;
	case 0x57: _stprintf(debug_dasm, _T("BIT 2, A")); break;
	case 0x58: _stprintf(debug_dasm, _T("BIT 3, B")); break;
	case 0x59: _stprintf(debug_dasm, _T("BIT 3, C")); break;
	case 0x5a: _stprintf(debug_dasm, _T("BIT 3, D")); break;
	case 0x5b: _stprintf(debug_dasm, _T("BIT 3, E")); break;
	case 0x5c: _stprintf(debug_dasm, _T("BIT 3, H")); break;
	case 0x5d: _stprintf(debug_dasm, _T("BIT 3, L")); break;
	case 0x5e: _stprintf(debug_dasm, _T("BIT 3, (HL)")); break;
	case 0x5f: _stprintf(debug_dasm, _T("BIT 3, A")); break;
	case 0x60: _stprintf(debug_dasm, _T("BIT 4, B")); break;
	case 0x61: _stprintf(debug_dasm, _T("BIT 4, C")); break;
	case 0x62: _stprintf(debug_dasm, _T("BIT 4, D")); break;
	case 0x63: _stprintf(debug_dasm, _T("BIT 4, E")); break;
	case 0x64: _stprintf(debug_dasm, _T("BIT 4, H")); break;
	case 0x65: _stprintf(debug_dasm, _T("BIT 4, L")); break;
	case 0x66: _stprintf(debug_dasm, _T("BIT 4, (HL)")); break;
	case 0x67: _stprintf(debug_dasm, _T("BIT 4, A")); break;
	case 0x68: _stprintf(debug_dasm, _T("BIT 5, B")); break;
	case 0x69: _stprintf(debug_dasm, _T("BIT 5, C")); break;
	case 0x6a: _stprintf(debug_dasm, _T("BIT 5, D")); break;
	case 0x6b: _stprintf(debug_dasm, _T("BIT 5, E")); break;
	case 0x6c: _stprintf(debug_dasm, _T("BIT 5, H")); break;
	case 0x6d: _stprintf(debug_dasm, _T("BIT 5, L")); break;
	case 0x6e: _stprintf(debug_dasm, _T("BIT 5, (HL)")); break;
	case 0x6f: _stprintf(debug_dasm, _T("BIT 5, A")); break;
	case 0x70: _stprintf(debug_dasm, _T("BIT 6, B")); break;
	case 0x71: _stprintf(debug_dasm, _T("BIT 6, C")); break;
	case 0x72: _stprintf(debug_dasm, _T("BIT 6, D")); break;
	case 0x73: _stprintf(debug_dasm, _T("BIT 6, E")); break;
	case 0x74: _stprintf(debug_dasm, _T("BIT 6, H")); break;
	case 0x75: _stprintf(debug_dasm, _T("BIT 6, L")); break;
	case 0x76: _stprintf(debug_dasm, _T("BIT 6, (HL)")); break;
	case 0x77: _stprintf(debug_dasm, _T("BIT 6, A")); break;
	case 0x78: _stprintf(debug_dasm, _T("BIT 7, B")); break;
	case 0x79: _stprintf(debug_dasm, _T("BIT 7, C")); break;
	case 0x7a: _stprintf(debug_dasm, _T("BIT 7, D")); break;
	case 0x7b: _stprintf(debug_dasm, _T("BIT 7, E")); break;
	case 0x7c: _stprintf(debug_dasm, _T("BIT 7, H")); break;
	case 0x7d: _stprintf(debug_dasm, _T("BIT 7, L")); break;
	case 0x7e: _stprintf(debug_dasm, _T("BIT 7, (HL)")); break;
	case 0x7f: _stprintf(debug_dasm, _T("BIT 7, A")); break;
	case 0x80: _stprintf(debug_dasm, _T("RES 0, B")); break;
	case 0x81: _stprintf(debug_dasm, _T("RES 0, C")); break;
	case 0x82: _stprintf(debug_dasm, _T("RES 0, D")); break;
	case 0x83: _stprintf(debug_dasm, _T("RES 0, E")); break;
	case 0x84: _stprintf(debug_dasm, _T("RES 0, H")); break;
	case 0x85: _stprintf(debug_dasm, _T("RES 0, L")); break;
	case 0x86: _stprintf(debug_dasm, _T("RES 0, (HL)")); break;
	case 0x87: _stprintf(debug_dasm, _T("RES 0, A")); break;
	case 0x88: _stprintf(debug_dasm, _T("RES 1, B")); break;
	case 0x89: _stprintf(debug_dasm, _T("RES 1, C")); break;
	case 0x8a: _stprintf(debug_dasm, _T("RES 1, D")); break;
	case 0x8b: _stprintf(debug_dasm, _T("RES 1, E")); break;
	case 0x8c: _stprintf(debug_dasm, _T("RES 1, H")); break;
	case 0x8d: _stprintf(debug_dasm, _T("RES 1, L")); break;
	case 0x8e: _stprintf(debug_dasm, _T("RES 1, (HL)")); break;
	case 0x8f: _stprintf(debug_dasm, _T("RES 1, A")); break;
	case 0x90: _stprintf(debug_dasm, _T("RES 2, B")); break;
	case 0x91: _stprintf(debug_dasm, _T("RES 2, C")); break;
	case 0x92: _stprintf(debug_dasm, _T("RES 2, D")); break;
	case 0x93: _stprintf(debug_dasm, _T("RES 2, E")); break;
	case 0x94: _stprintf(debug_dasm, _T("RES 2, H")); break;
	case 0x95: _stprintf(debug_dasm, _T("RES 2, L")); break;
	case 0x96: _stprintf(debug_dasm, _T("RES 2, (HL)")); break;
	case 0x97: _stprintf(debug_dasm, _T("RES 2, A")); break;
	case 0x98: _stprintf(debug_dasm, _T("RES 3, B")); break;
	case 0x99: _stprintf(debug_dasm, _T("RES 3, C")); break;
	case 0x9a: _stprintf(debug_dasm, _T("RES 3, D")); break;
	case 0x9b: _stprintf(debug_dasm, _T("RES 3, E")); break;
	case 0x9c: _stprintf(debug_dasm, _T("RES 3, H")); break;
	case 0x9d: _stprintf(debug_dasm, _T("RES 3, L")); break;
	case 0x9e: _stprintf(debug_dasm, _T("RES 3, (HL)")); break;
	case 0x9f: _stprintf(debug_dasm, _T("RES 3, A")); break;
	case 0xa0: _stprintf(debug_dasm, _T("RES 4, B")); break;
	case 0xa1: _stprintf(debug_dasm, _T("RES 4, C")); break;
	case 0xa2: _stprintf(debug_dasm, _T("RES 4, D")); break;
	case 0xa3: _stprintf(debug_dasm, _T("RES 4, E")); break;
	case 0xa4: _stprintf(debug_dasm, _T("RES 4, H")); break;
	case 0xa5: _stprintf(debug_dasm, _T("RES 4, L")); break;
	case 0xa6: _stprintf(debug_dasm, _T("RES 4, (HL)")); break;
	case 0xa7: _stprintf(debug_dasm, _T("RES 4, A")); break;
	case 0xa8: _stprintf(debug_dasm, _T("RES 5, B")); break;
	case 0xa9: _stprintf(debug_dasm, _T("RES 5, C")); break;
	case 0xaa: _stprintf(debug_dasm, _T("RES 5, D")); break;
	case 0xab: _stprintf(debug_dasm, _T("RES 5, E")); break;
	case 0xac: _stprintf(debug_dasm, _T("RES 5, H")); break;
	case 0xad: _stprintf(debug_dasm, _T("RES 5, L")); break;
	case 0xae: _stprintf(debug_dasm, _T("RES 5, (HL)")); break;
	case 0xaf: _stprintf(debug_dasm, _T("RES 5, A")); break;
	case 0xb0: _stprintf(debug_dasm, _T("RES 6, B")); break;
	case 0xb1: _stprintf(debug_dasm, _T("RES 6, C")); break;
	case 0xb2: _stprintf(debug_dasm, _T("RES 6, D")); break;
	case 0xb3: _stprintf(debug_dasm, _T("RES 6, E")); break;
	case 0xb4: _stprintf(debug_dasm, _T("RES 6, H")); break;
	case 0xb5: _stprintf(debug_dasm, _T("RES 6, L")); break;
	case 0xb6: _stprintf(debug_dasm, _T("RES 6, (HL)")); break;
	case 0xb7: _stprintf(debug_dasm, _T("RES 6, A")); break;
	case 0xb8: _stprintf(debug_dasm, _T("RES 7, B")); break;
	case 0xb9: _stprintf(debug_dasm, _T("RES 7, C")); break;
	case 0xba: _stprintf(debug_dasm, _T("RES 7, D")); break;
	case 0xbb: _stprintf(debug_dasm, _T("RES 7, E")); break;
	case 0xbc: _stprintf(debug_dasm, _T("RES 7, H")); break;
	case 0xbd: _stprintf(debug_dasm, _T("RES 7, L")); break;
	case 0xbe: _stprintf(debug_dasm, _T("RES 7, (HL)")); break;
	case 0xbf: _stprintf(debug_dasm, _T("RES 7, A")); break;
	case 0xc0: _stprintf(debug_dasm, _T("SET 0, B")); break;
	case 0xc1: _stprintf(debug_dasm, _T("SET 0, C")); break;
	case 0xc2: _stprintf(debug_dasm, _T("SET 0, D")); break;
	case 0xc3: _stprintf(debug_dasm, _T("SET 0, E")); break;
	case 0xc4: _stprintf(debug_dasm, _T("SET 0, H")); break;
	case 0xc5: _stprintf(debug_dasm, _T("SET 0, L")); break;
	case 0xc6: _stprintf(debug_dasm, _T("SET 0, (HL)")); break;
	case 0xc7: _stprintf(debug_dasm, _T("SET 0, A")); break;
	case 0xc8: _stprintf(debug_dasm, _T("SET 1, B")); break;
	case 0xc9: _stprintf(debug_dasm, _T("SET 1, C")); break;
	case 0xca: _stprintf(debug_dasm, _T("SET 1, D")); break;
	case 0xcb: _stprintf(debug_dasm, _T("SET 1, E")); break;
	case 0xcc: _stprintf(debug_dasm, _T("SET 1, H")); break;
	case 0xcd: _stprintf(debug_dasm, _T("SET 1, L")); break;
	case 0xce: _stprintf(debug_dasm, _T("SET 1, (HL)")); break;
	case 0xcf: _stprintf(debug_dasm, _T("SET 1, A")); break;
	case 0xd0: _stprintf(debug_dasm, _T("SET 2, B")); break;
	case 0xd1: _stprintf(debug_dasm, _T("SET 2, C")); break;
	case 0xd2: _stprintf(debug_dasm, _T("SET 2, D")); break;
	case 0xd3: _stprintf(debug_dasm, _T("SET 2, E")); break;
	case 0xd4: _stprintf(debug_dasm, _T("SET 2, H")); break;
	case 0xd5: _stprintf(debug_dasm, _T("SET 2, L")); break;
	case 0xd6: _stprintf(debug_dasm, _T("SET 2, (HL)")); break;
	case 0xd7: _stprintf(debug_dasm, _T("SET 2, A")); break;
	case 0xd8: _stprintf(debug_dasm, _T("SET 3, B")); break;
	case 0xd9: _stprintf(debug_dasm, _T("SET 3, C")); break;
	case 0xda: _stprintf(debug_dasm, _T("SET 3, D")); break;
	case 0xdb: _stprintf(debug_dasm, _T("SET 3, E")); break;
	case 0xdc: _stprintf(debug_dasm, _T("SET 3, H")); break;
	case 0xdd: _stprintf(debug_dasm, _T("SET 3, L")); break;
	case 0xde: _stprintf(debug_dasm, _T("SET 3, (HL)")); break;
	case 0xdf: _stprintf(debug_dasm, _T("SET 3, A")); break;
	case 0xe0: _stprintf(debug_dasm, _T("SET 4, B")); break;
	case 0xe1: _stprintf(debug_dasm, _T("SET 4, C")); break;
	case 0xe2: _stprintf(debug_dasm, _T("SET 4, D")); break;
	case 0xe3: _stprintf(debug_dasm, _T("SET 4, E")); break;
	case 0xe4: _stprintf(debug_dasm, _T("SET 4, H")); break;
	case 0xe5: _stprintf(debug_dasm, _T("SET 4, L")); break;
	case 0xe6: _stprintf(debug_dasm, _T("SET 4, (HL)")); break;
	case 0xe7: _stprintf(debug_dasm, _T("SET 4, A")); break;
	case 0xe8: _stprintf(debug_dasm, _T("SET 5, B")); break;
	case 0xe9: _stprintf(debug_dasm, _T("SET 5, C")); break;
	case 0xea: _stprintf(debug_dasm, _T("SET 5, D")); break;
	case 0xeb: _stprintf(debug_dasm, _T("SET 5, E")); break;
	case 0xec: _stprintf(debug_dasm, _T("SET 5, H")); break;
	case 0xed: _stprintf(debug_dasm, _T("SET 5, L")); break;
	case 0xee: _stprintf(debug_dasm, _T("SET 5, (HL)")); break;
	case 0xef: _stprintf(debug_dasm, _T("SET 5, A")); break;
	case 0xf0: _stprintf(debug_dasm, _T("SET 6, B")); break;
	case 0xf1: _stprintf(debug_dasm, _T("SET 6, C")); break;
	case 0xf2: _stprintf(debug_dasm, _T("SET 6, D")); break;
	case 0xf3: _stprintf(debug_dasm, _T("SET 6, E")); break;
	case 0xf4: _stprintf(debug_dasm, _T("SET 6, H")); break;
	case 0xf5: _stprintf(debug_dasm, _T("SET 6, L")); break;
	case 0xf6: _stprintf(debug_dasm, _T("SET 6, (HL)")); break;
	case 0xf7: _stprintf(debug_dasm, _T("SET 6, A")); break;
	case 0xf8: _stprintf(debug_dasm, _T("SET 7, B")); break;
	case 0xf9: _stprintf(debug_dasm, _T("SET 7, C")); break;
	case 0xfa: _stprintf(debug_dasm, _T("SET 7, D")); break;
	case 0xfb: _stprintf(debug_dasm, _T("SET 7, E")); break;
	case 0xfc: _stprintf(debug_dasm, _T("SET 7, H")); break;
	case 0xfd: _stprintf(debug_dasm, _T("SET 7, L")); break;
	case 0xfe: _stprintf(debug_dasm, _T("SET 7, (HL)")); break;
	case 0xff: _stprintf(debug_dasm, _T("SET 7, A")); break;
	}
}

void Z80::DASM_DD()
{
	uint8 code = DEBUG_FETCHOP();
	int8 ofs;
	
	switch(code) {
	case 0x09: _stprintf(debug_dasm, _T("ADD IX, BC")); break;
	case 0x19: _stprintf(debug_dasm, _T("ADD IX, DE")); break;
	case 0x21: _stprintf(debug_dasm, _T("LD IX, %4x"), DEBUG_FETCH16()); break;
	case 0x22: _stprintf(debug_dasm, _T("LD (%4x), IX"), DEBUG_FETCH16()); break;
	case 0x23: _stprintf(debug_dasm, _T("INC IX")); break;
	case 0x24: _stprintf(debug_dasm, _T("INC HX")); break;
	case 0x25: _stprintf(debug_dasm, _T("DEC HX")); break;
	case 0x26: _stprintf(debug_dasm, _T("LD HX, %2x"), DEBUG_FETCH8()); break;
	case 0x29: _stprintf(debug_dasm, _T("ADD IX, IX")); break;
	case 0x2a: _stprintf(debug_dasm, _T("LD IX, (%4x)"), DEBUG_FETCH16()); break;
	case 0x2b: _stprintf(debug_dasm, _T("DEC IX")); break;
	case 0x2c: _stprintf(debug_dasm, _T("INC LX")); break;
	case 0x2d: _stprintf(debug_dasm, _T("DEC LX")); break;
	case 0x2e: _stprintf(debug_dasm, _T("LD LX, %2x"), DEBUG_FETCH8()); break;
	case 0x34: _stprintf(debug_dasm, _T("INC (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x35: _stprintf(debug_dasm, _T("DEC (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x36: ofs = DEBUG_FETCH8_REL();_stprintf(debug_dasm, _T("LD (IX+(%d)), %2x"), ofs, DEBUG_FETCH8()); break;
	case 0x39: _stprintf(debug_dasm, _T("ADD IX, SP")); break;
	case 0x44: _stprintf(debug_dasm, _T("LD B, HX")); break;
	case 0x45: _stprintf(debug_dasm, _T("LD B, LX")); break;
	case 0x46: _stprintf(debug_dasm, _T("LD B, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x4c: _stprintf(debug_dasm, _T("LD C, HX")); break;
	case 0x4d: _stprintf(debug_dasm, _T("LD C, LX")); break;
	case 0x4e: _stprintf(debug_dasm, _T("LD C, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x54: _stprintf(debug_dasm, _T("LD D, HX")); break;
	case 0x55: _stprintf(debug_dasm, _T("LD D, LX")); break;
	case 0x56: _stprintf(debug_dasm, _T("LD D, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x5c: _stprintf(debug_dasm, _T("LD E, HX")); break;
	case 0x5d: _stprintf(debug_dasm, _T("LD E, LX")); break;
	case 0x5e: _stprintf(debug_dasm, _T("LD E, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x60: _stprintf(debug_dasm, _T("LD HX, B")); break;
	case 0x61: _stprintf(debug_dasm, _T("LD HX, C")); break;
	case 0x62: _stprintf(debug_dasm, _T("LD HX, D")); break;
	case 0x63: _stprintf(debug_dasm, _T("LD HX, E")); break;
	case 0x64: _stprintf(debug_dasm, _T("LD HX, HX")); break;
	case 0x65: _stprintf(debug_dasm, _T("LD HX, LX")); break;
	case 0x66: _stprintf(debug_dasm, _T("LD H, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x67: _stprintf(debug_dasm, _T("LD HX, A")); break;
	case 0x68: _stprintf(debug_dasm, _T("LD LX, B")); break;
	case 0x69: _stprintf(debug_dasm, _T("LD LX, C")); break;
	case 0x6a: _stprintf(debug_dasm, _T("LD LX, D")); break;
	case 0x6b: _stprintf(debug_dasm, _T("LD LX, E")); break;
	case 0x6c: _stprintf(debug_dasm, _T("LD LX, HX")); break;
	case 0x6d: _stprintf(debug_dasm, _T("LD LX, LX")); break;
	case 0x6e: _stprintf(debug_dasm, _T("LD L, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x6f: _stprintf(debug_dasm, _T("LD LX, A")); break;
	case 0x70: _stprintf(debug_dasm, _T("LD (IX+(%d)), B"), DEBUG_FETCH8_REL()); break;
	case 0x71: _stprintf(debug_dasm, _T("LD (IX+(%d)), C"), DEBUG_FETCH8_REL()); break;
	case 0x72: _stprintf(debug_dasm, _T("LD (IX+(%d)), D"), DEBUG_FETCH8_REL()); break;
	case 0x73: _stprintf(debug_dasm, _T("LD (IX+(%d)), E"), DEBUG_FETCH8_REL()); break;
	case 0x74: _stprintf(debug_dasm, _T("LD (IX+(%d)), H"), DEBUG_FETCH8_REL()); break;
	case 0x75: _stprintf(debug_dasm, _T("LD (IX+(%d)), L"), DEBUG_FETCH8_REL()); break;
	case 0x77: _stprintf(debug_dasm, _T("LD (IX+(%d)), A"), DEBUG_FETCH8_REL()); break;
	case 0x7c: _stprintf(debug_dasm, _T("LD A, HX")); break;
	case 0x7d: _stprintf(debug_dasm, _T("LD A, LX")); break;
	case 0x7e: _stprintf(debug_dasm, _T("LD A, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x84: _stprintf(debug_dasm, _T("ADD A, HX")); break;
	case 0x85: _stprintf(debug_dasm, _T("ADD A, LX")); break;
	case 0x86: _stprintf(debug_dasm, _T("ADD A, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x8c: _stprintf(debug_dasm, _T("ADC A, HX")); break;
	case 0x8d: _stprintf(debug_dasm, _T("ADC A, LX")); break;
	case 0x8e: _stprintf(debug_dasm, _T("ADC A, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x94: _stprintf(debug_dasm, _T("SUB HX")); break;
	case 0x95: _stprintf(debug_dasm, _T("SUB LX")); break;
	case 0x96: _stprintf(debug_dasm, _T("SUB (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x9c: _stprintf(debug_dasm, _T("SBC A, HX")); break;
	case 0x9d: _stprintf(debug_dasm, _T("SBC A, LX")); break;
	case 0x9e: _stprintf(debug_dasm, _T("SBC A, (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xa4: _stprintf(debug_dasm, _T("AND HX")); break;
	case 0xa5: _stprintf(debug_dasm, _T("AND LX")); break;
	case 0xa6: _stprintf(debug_dasm, _T("AND (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xac: _stprintf(debug_dasm, _T("XOR HX")); break;
	case 0xad: _stprintf(debug_dasm, _T("XOR LX")); break;
	case 0xae: _stprintf(debug_dasm, _T("XOR (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xb4: _stprintf(debug_dasm, _T("OR HX")); break;
	case 0xb5: _stprintf(debug_dasm, _T("OR LX")); break;
	case 0xb6: _stprintf(debug_dasm, _T("OR (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xbc: _stprintf(debug_dasm, _T("CP HX")); break;
	case 0xbd: _stprintf(debug_dasm, _T("CP LX")); break;
	case 0xbe: _stprintf(debug_dasm, _T("CP (IX+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xcb: DASM_DDCB(); break;
	case 0xe1: _stprintf(debug_dasm, _T("POP IX")); break;
	case 0xe3: _stprintf(debug_dasm, _T("EX (SP), IX")); break;
	case 0xe5: _stprintf(debug_dasm, _T("PUSH IX")); break;
	case 0xe9: _stprintf(debug_dasm, _T("JP (IX)")); break;
	case 0xf9: _stprintf(debug_dasm, _T("LD SP, IX")); break;
	}
}

void Z80::DASM_ED()
{
	uint8 code = DEBUG_FETCHOP();
	
	switch(code) {
	case 0x40: _stprintf(debug_dasm, _T("IN B, (C)")); break;
	case 0x41: _stprintf(debug_dasm, _T("OUT (C), B")); break;
	case 0x42: _stprintf(debug_dasm, _T("SBC HL, BC")); break;
	case 0x43: _stprintf(debug_dasm, _T("LD (%4x), BC"), DEBUG_FETCH16()); break;
	case 0x44: _stprintf(debug_dasm, _T("NEG")); break;
	case 0x45: _stprintf(debug_dasm, _T("RETN")); break;
	case 0x46: _stprintf(debug_dasm, _T("IM 0")); break;
	case 0x47: _stprintf(debug_dasm, _T("LD I, A")); break;
	case 0x48: _stprintf(debug_dasm, _T("IN C, (C)")); break;
	case 0x49: _stprintf(debug_dasm, _T("OUT (C), C")); break;
	case 0x4a: _stprintf(debug_dasm, _T("ADC HL, BC")); break;
	case 0x4b: _stprintf(debug_dasm, _T("LD BC, (%4x)"), DEBUG_FETCH16()); break;
	case 0x4c: _stprintf(debug_dasm, _T("NEG")); break;
	case 0x4d: _stprintf(debug_dasm, _T("RETI")); break;
	case 0x4e: _stprintf(debug_dasm, _T("IM 0")); break;
	case 0x4f: _stprintf(debug_dasm, _T("LD R, A")); break;
	case 0x50: _stprintf(debug_dasm, _T("IN D, (C)")); break;
	case 0x51: _stprintf(debug_dasm, _T("OUT (C), D")); break;
	case 0x52: _stprintf(debug_dasm, _T("SBC HL, DE")); break;
	case 0x53: _stprintf(debug_dasm, _T("LD (%4x), DE")); break;
	case 0x54: _stprintf(debug_dasm, _T("NEG")); break;
	case 0x55: _stprintf(debug_dasm, _T("RETN")); break;
	case 0x56: _stprintf(debug_dasm, _T("IM 1")); break;
	case 0x57: _stprintf(debug_dasm, _T("LD A, I")); break;
	case 0x58: _stprintf(debug_dasm, _T("IN E, (C)")); break;
	case 0x59: _stprintf(debug_dasm, _T("OUT (C), E")); break;
	case 0x5a: _stprintf(debug_dasm, _T("ADC HL, DE")); break;
	case 0x5b: _stprintf(debug_dasm, _T("LD DE, (%4x)"), DEBUG_FETCH16()); break;
	case 0x5c: _stprintf(debug_dasm, _T("NEG")); break;
	case 0x5d: _stprintf(debug_dasm, _T("RETI")); break;
	case 0x5e: _stprintf(debug_dasm, _T("IM 2")); break;
	case 0x5f: _stprintf(debug_dasm, _T("LD A, R")); break;
	case 0x60: _stprintf(debug_dasm, _T("IN H, (C)")); break;
	case 0x61: _stprintf(debug_dasm, _T("OUT (C), H")); break;
	case 0x62: _stprintf(debug_dasm, _T("SBC HL, HL")); break;
	case 0x63: _stprintf(debug_dasm, _T("LD (%4x), HL"), DEBUG_FETCH16()); break;
	case 0x64: _stprintf(debug_dasm, _T("NEG")); break;
	case 0x65: _stprintf(debug_dasm, _T("RETN")); break;
	case 0x66: _stprintf(debug_dasm, _T("IM 0")); break;
	case 0x67: _stprintf(debug_dasm, _T("RRD (HL)")); break;
	case 0x68: _stprintf(debug_dasm, _T("IN L, (C)")); break;
	case 0x69: _stprintf(debug_dasm, _T("OUT (C), L")); break;
	case 0x6a: _stprintf(debug_dasm, _T("ADC HL, HL")); break;
	case 0x6b: _stprintf(debug_dasm, _T("LD HL, (%4x)"), DEBUG_FETCH16()); break;
	case 0x6c: _stprintf(debug_dasm, _T("NEG")); break;
	case 0x6d: _stprintf(debug_dasm, _T("RETI")); break;
	case 0x6e: _stprintf(debug_dasm, _T("IM 0")); break;
	case 0x6f: _stprintf(debug_dasm, _T("RLD (HL)")); break;
	case 0x70: _stprintf(debug_dasm, _T("IN 0, (C)")); break;
	case 0x71: _stprintf(debug_dasm, _T("OUT (C), 0")); break;
	case 0x72: _stprintf(debug_dasm, _T("SBC HL, SP")); break;
	case 0x73: _stprintf(debug_dasm, _T("LD (%4x), SP"), DEBUG_FETCH16()); break;
	case 0x74: _stprintf(debug_dasm, _T("NEG")); break;
	case 0x75: _stprintf(debug_dasm, _T("RETN")); break;
	case 0x76: _stprintf(debug_dasm, _T("IM 1")); break;
	case 0x78: _stprintf(debug_dasm, _T("IN A, (C)")); break;
	case 0x79: _stprintf(debug_dasm, _T("OUT (C), A")); break;
	case 0x7a: _stprintf(debug_dasm, _T("ADC HL, SP")); break;
	case 0x7b: _stprintf(debug_dasm, _T("LD SP, (%4x)"), DEBUG_FETCH16()); break;
	case 0x7c: _stprintf(debug_dasm, _T("NEG")); break;
	case 0x7d: _stprintf(debug_dasm, _T("RETI")); break;
	case 0x7e: _stprintf(debug_dasm, _T("IM 2")); break;
	case 0xa0: _stprintf(debug_dasm, _T("LDI")); break;
	case 0xa1: _stprintf(debug_dasm, _T("CPI")); break;
	case 0xa2: _stprintf(debug_dasm, _T("INI")); break;
	case 0xa3: _stprintf(debug_dasm, _T("OUTI")); break;
	case 0xa8: _stprintf(debug_dasm, _T("LDD")); break;
	case 0xa9: _stprintf(debug_dasm, _T("CPD")); break;
	case 0xaa: _stprintf(debug_dasm, _T("IND")); break;
	case 0xab: _stprintf(debug_dasm, _T("OUTD")); break;
	case 0xb0: _stprintf(debug_dasm, _T("LDIR")); break;
	case 0xb1: _stprintf(debug_dasm, _T("CPIR")); break;
	case 0xb2: _stprintf(debug_dasm, _T("INIR")); break;
	case 0xb3: _stprintf(debug_dasm, _T("OTIR")); break;
	case 0xb8: _stprintf(debug_dasm, _T("LDDR")); break;
	case 0xb9: _stprintf(debug_dasm, _T("CPDR")); break;
	case 0xba: _stprintf(debug_dasm, _T("INDR")); break;
	case 0xbb: _stprintf(debug_dasm, _T("OTDR")); break;
	}
}

void Z80::DASM_FD()
{
	uint8 code = DEBUG_FETCHOP();
	int8 ofs;
	
	switch(code) {
	case 0x09: _stprintf(debug_dasm, _T("ADD IY, BC")); break;
	case 0x19: _stprintf(debug_dasm, _T("ADD IY, DE")); break;
	case 0x21: _stprintf(debug_dasm, _T("LD IY, %4x"), DEBUG_FETCH16()); break;
	case 0x22: _stprintf(debug_dasm, _T("LD (%4x), IY"), DEBUG_FETCH16()); break;
	case 0x23: _stprintf(debug_dasm, _T("INC IY")); break;
	case 0x24: _stprintf(debug_dasm, _T("INC HY")); break;
	case 0x25: _stprintf(debug_dasm, _T("DEC HY")); break;
	case 0x26: _stprintf(debug_dasm, _T("LD HY, %2x"), DEBUG_FETCH8()); break;
	case 0x29: _stprintf(debug_dasm, _T("ADD IY, IY")); break;
	case 0x2a: _stprintf(debug_dasm, _T("LD IY, (%4x)"), DEBUG_FETCH16()); break;
	case 0x2b: _stprintf(debug_dasm, _T("DEC IY")); break;
	case 0x2c: _stprintf(debug_dasm, _T("INC LY")); break;
	case 0x2d: _stprintf(debug_dasm, _T("DEC LY")); break;
	case 0x2e: _stprintf(debug_dasm, _T("LD LY, %2x"), DEBUG_FETCH8()); break;
	case 0x34: _stprintf(debug_dasm, _T("INC (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x35: _stprintf(debug_dasm, _T("DEC (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x36: ofs = DEBUG_FETCH8_REL();_stprintf(debug_dasm, _T("LD (IY+(%d)), %2x"), ofs, DEBUG_FETCH8()); break;
	case 0x39: _stprintf(debug_dasm, _T("ADD IY, SP")); break;
	case 0x44: _stprintf(debug_dasm, _T("LD B, HY")); break;
	case 0x45: _stprintf(debug_dasm, _T("LD B, LY")); break;
	case 0x46: _stprintf(debug_dasm, _T("LD B, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x4c: _stprintf(debug_dasm, _T("LD C, HY")); break;
	case 0x4d: _stprintf(debug_dasm, _T("LD C, LY")); break;
	case 0x4e: _stprintf(debug_dasm, _T("LD C, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x54: _stprintf(debug_dasm, _T("LD D, HY")); break;
	case 0x55: _stprintf(debug_dasm, _T("LD D, LY")); break;
	case 0x56: _stprintf(debug_dasm, _T("LD D, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x5c: _stprintf(debug_dasm, _T("LD E, HY")); break;
	case 0x5d: _stprintf(debug_dasm, _T("LD E, LY")); break;
	case 0x5e: _stprintf(debug_dasm, _T("LD E, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x60: _stprintf(debug_dasm, _T("LD HY, B")); break;
	case 0x61: _stprintf(debug_dasm, _T("LD HY, C")); break;
	case 0x62: _stprintf(debug_dasm, _T("LD HY, D")); break;
	case 0x63: _stprintf(debug_dasm, _T("LD HY, E")); break;
	case 0x64: _stprintf(debug_dasm, _T("LD HY, HY")); break;
	case 0x65: _stprintf(debug_dasm, _T("LD HY, LY")); break;
	case 0x66: _stprintf(debug_dasm, _T("LD H, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x67: _stprintf(debug_dasm, _T("LD HY, A")); break;
	case 0x68: _stprintf(debug_dasm, _T("LD LY, B")); break;
	case 0x69: _stprintf(debug_dasm, _T("LD LY, C")); break;
	case 0x6a: _stprintf(debug_dasm, _T("LD LY, D")); break;
	case 0x6b: _stprintf(debug_dasm, _T("LD LY, E")); break;
	case 0x6c: _stprintf(debug_dasm, _T("LD LY, HY")); break;
	case 0x6d: _stprintf(debug_dasm, _T("LD LY, LY")); break;
	case 0x6e: _stprintf(debug_dasm, _T("LD L, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x6f: _stprintf(debug_dasm, _T("LD LY, A")); break;
	case 0x70: _stprintf(debug_dasm, _T("LD (IY+(%d)), B"), DEBUG_FETCH8_REL()); break;
	case 0x71: _stprintf(debug_dasm, _T("LD (IY+(%d)), C"), DEBUG_FETCH8_REL()); break;
	case 0x72: _stprintf(debug_dasm, _T("LD (IY+(%d)), D"), DEBUG_FETCH8_REL()); break;
	case 0x73: _stprintf(debug_dasm, _T("LD (IY+(%d)), E"), DEBUG_FETCH8_REL()); break;
	case 0x74: _stprintf(debug_dasm, _T("LD (IY+(%d)), H"), DEBUG_FETCH8_REL()); break;
	case 0x75: _stprintf(debug_dasm, _T("LD (IY+(%d)), L"), DEBUG_FETCH8_REL()); break;
	case 0x77: _stprintf(debug_dasm, _T("LD (IY+(%d)), A"), DEBUG_FETCH8_REL()); break;
	case 0x7c: _stprintf(debug_dasm, _T("LD A, HY")); break;
	case 0x7d: _stprintf(debug_dasm, _T("LD A, LY")); break;
	case 0x7e: _stprintf(debug_dasm, _T("LD A, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x84: _stprintf(debug_dasm, _T("ADD A, HY")); break;
	case 0x85: _stprintf(debug_dasm, _T("ADD A, LY")); break;
	case 0x86: _stprintf(debug_dasm, _T("ADD A, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x8c: _stprintf(debug_dasm, _T("ADC A, HY")); break;
	case 0x8d: _stprintf(debug_dasm, _T("ADC A, LY")); break;
	case 0x8e: _stprintf(debug_dasm, _T("ADC A, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x94: _stprintf(debug_dasm, _T("SUB HY")); break;
	case 0x95: _stprintf(debug_dasm, _T("SUB LY")); break;
	case 0x96: _stprintf(debug_dasm, _T("SUB (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0x9c: _stprintf(debug_dasm, _T("SBC A, HY")); break;
	case 0x9d: _stprintf(debug_dasm, _T("SBC A, LY")); break;
	case 0x9e: _stprintf(debug_dasm, _T("SBC A, (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xa4: _stprintf(debug_dasm, _T("AND HY")); break;
	case 0xa5: _stprintf(debug_dasm, _T("AND LY")); break;
	case 0xa6: _stprintf(debug_dasm, _T("AND (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xac: _stprintf(debug_dasm, _T("XOR HY")); break;
	case 0xad: _stprintf(debug_dasm, _T("XOR LY")); break;
	case 0xae: _stprintf(debug_dasm, _T("XOR (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xb4: _stprintf(debug_dasm, _T("OR HY")); break;
	case 0xb5: _stprintf(debug_dasm, _T("OR LY")); break;
	case 0xb6: _stprintf(debug_dasm, _T("OR (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xbc: _stprintf(debug_dasm, _T("CP HY")); break;
	case 0xbd: _stprintf(debug_dasm, _T("CP LY")); break;
	case 0xbe: _stprintf(debug_dasm, _T("CP (IY+(%d))"), DEBUG_FETCH8_REL()); break;
	case 0xcb: DASM_FDCB(); break;
	case 0xe1: _stprintf(debug_dasm, _T("POP IY")); break;
	case 0xe3: _stprintf(debug_dasm, _T("EX (SP), IY")); break;
	case 0xe5: _stprintf(debug_dasm, _T("PUSH IY")); break;
	case 0xe9: _stprintf(debug_dasm, _T("JP (IY)")); break;
	case 0xf9: _stprintf(debug_dasm, _T("LD SP, IY")); break;
	}
}

void Z80::DASM_DDCB()
{
	int8 ofs = DEBUG_FETCH8_REL();
	uint8 code = DEBUG_FETCH8();
	
	switch(code) {
	case 0x00: _stprintf(debug_dasm, _T("RLC B=(IX+(%d))"), ofs); break;
	case 0x01: _stprintf(debug_dasm, _T("RLC C=(IX+(%d))"), ofs); break;
	case 0x02: _stprintf(debug_dasm, _T("RLC D=(IX+(%d))"), ofs); break;
	case 0x03: _stprintf(debug_dasm, _T("RLC E=(IX+(%d))"), ofs); break;
	case 0x04: _stprintf(debug_dasm, _T("RLC H=(IX+(%d))"), ofs); break;
	case 0x05: _stprintf(debug_dasm, _T("RLC L=(IX+(%d))"), ofs); break;
	case 0x06: _stprintf(debug_dasm, _T("RLC (IX+(%d))"), ofs); break;
	case 0x07: _stprintf(debug_dasm, _T("RLC A=(IX+(%d))"), ofs); break;
	case 0x08: _stprintf(debug_dasm, _T("RRC B=(IX+(%d))"), ofs); break;
	case 0x09: _stprintf(debug_dasm, _T("RRC C=(IX+(%d))"), ofs); break;
	case 0x0a: _stprintf(debug_dasm, _T("RRC D=(IX+(%d))"), ofs); break;
	case 0x0b: _stprintf(debug_dasm, _T("RRC E=(IX+(%d))"), ofs); break;
	case 0x0c: _stprintf(debug_dasm, _T("RRC H=(IX+(%d))"), ofs); break;
	case 0x0d: _stprintf(debug_dasm, _T("RRC L=(IX+(%d))"), ofs); break;
	case 0x0e: _stprintf(debug_dasm, _T("RRC (IX+(%d))"), ofs); break;
	case 0x0f: _stprintf(debug_dasm, _T("RRC A=(IX+(%d))"), ofs); break;
	case 0x10: _stprintf(debug_dasm, _T("RL B=(IX+(%d))"), ofs); break;
	case 0x11: _stprintf(debug_dasm, _T("RL C=(IX+(%d))"), ofs); break;
	case 0x12: _stprintf(debug_dasm, _T("RL D=(IX+(%d))"), ofs); break;
	case 0x13: _stprintf(debug_dasm, _T("RL E=(IX+(%d))"), ofs); break;
	case 0x14: _stprintf(debug_dasm, _T("RL H=(IX+(%d))"), ofs); break;
	case 0x15: _stprintf(debug_dasm, _T("RL L=(IX+(%d))"), ofs); break;
	case 0x16: _stprintf(debug_dasm, _T("RL (IX+(%d))"), ofs); break;
	case 0x17: _stprintf(debug_dasm, _T("RL A=(IX+(%d))"), ofs); break;
	case 0x18: _stprintf(debug_dasm, _T("RR B=(IX+(%d))"), ofs); break;
	case 0x19: _stprintf(debug_dasm, _T("RR C=(IX+(%d))"), ofs); break;
	case 0x1a: _stprintf(debug_dasm, _T("RR D=(IX+(%d))"), ofs); break;
	case 0x1b: _stprintf(debug_dasm, _T("RR E=(IX+(%d))"), ofs); break;
	case 0x1c: _stprintf(debug_dasm, _T("RR H=(IX+(%d))"), ofs); break;
	case 0x1d: _stprintf(debug_dasm, _T("RR L=(IX+(%d))"), ofs); break;
	case 0x1e: _stprintf(debug_dasm, _T("RR (IX+(%d))"), ofs); break;
	case 0x1f: _stprintf(debug_dasm, _T("RR A=(IX+(%d))"), ofs); break;
	case 0x20: _stprintf(debug_dasm, _T("SLA B=(IX+(%d))"), ofs); break;
	case 0x21: _stprintf(debug_dasm, _T("SLA C=(IX+(%d))"), ofs); break;
	case 0x22: _stprintf(debug_dasm, _T("SLA D=(IX+(%d))"), ofs); break;
	case 0x23: _stprintf(debug_dasm, _T("SLA E=(IX+(%d))"), ofs); break;
	case 0x24: _stprintf(debug_dasm, _T("SLA H=(IX+(%d))"), ofs); break;
	case 0x25: _stprintf(debug_dasm, _T("SLA L=(IX+(%d))"), ofs); break;
	case 0x26: _stprintf(debug_dasm, _T("SLA (IX+(%d))"), ofs); break;
	case 0x27: _stprintf(debug_dasm, _T("SLA A=(IX+(%d))"), ofs); break;
	case 0x28: _stprintf(debug_dasm, _T("SRA B=(IX+(%d))"), ofs); break;
	case 0x29: _stprintf(debug_dasm, _T("SRA C=(IX+(%d))"), ofs); break;
	case 0x2a: _stprintf(debug_dasm, _T("SRA D=(IX+(%d))"), ofs); break;
	case 0x2b: _stprintf(debug_dasm, _T("SRA E=(IX+(%d))"), ofs); break;
	case 0x2c: _stprintf(debug_dasm, _T("SRA H=(IX+(%d))"), ofs); break;
	case 0x2d: _stprintf(debug_dasm, _T("SRA L=(IX+(%d))"), ofs); break;
	case 0x2e: _stprintf(debug_dasm, _T("SRA (IX+(%d))"), ofs); break;
	case 0x2f: _stprintf(debug_dasm, _T("SRA A=(IX+(%d))"), ofs); break;
	case 0x30: _stprintf(debug_dasm, _T("SLL B=(IX+(%d))"), ofs); break;
	case 0x31: _stprintf(debug_dasm, _T("SLL C=(IX+(%d))"), ofs); break;
	case 0x32: _stprintf(debug_dasm, _T("SLL D=(IX+(%d))"), ofs); break;
	case 0x33: _stprintf(debug_dasm, _T("SLL E=(IX+(%d))"), ofs); break;
	case 0x34: _stprintf(debug_dasm, _T("SLL H=(IX+(%d))"), ofs); break;
	case 0x35: _stprintf(debug_dasm, _T("SLL L=(IX+(%d))"), ofs); break;
	case 0x36: _stprintf(debug_dasm, _T("SLL (IX+(%d))"), ofs); break;
	case 0x37: _stprintf(debug_dasm, _T("SLL A=(IX+(%d))"), ofs); break;
	case 0x38: _stprintf(debug_dasm, _T("SRL B=(IX+(%d))"), ofs); break;
	case 0x39: _stprintf(debug_dasm, _T("SRL C=(IX+(%d))"), ofs); break;
	case 0x3a: _stprintf(debug_dasm, _T("SRL D=(IX+(%d))"), ofs); break;
	case 0x3b: _stprintf(debug_dasm, _T("SRL E=(IX+(%d))"), ofs); break;
	case 0x3c: _stprintf(debug_dasm, _T("SRL H=(IX+(%d))"), ofs); break;
	case 0x3d: _stprintf(debug_dasm, _T("SRL L=(IX+(%d))"), ofs); break;
	case 0x3e: _stprintf(debug_dasm, _T("SRL (IX+(%d))"), ofs); break;
	case 0x3f: _stprintf(debug_dasm, _T("SRL A=(IX+(%d))"), ofs); break;
	case 0x40: _stprintf(debug_dasm, _T("BIT 0, B=(IX+(%d))"), ofs); break;
	case 0x41: _stprintf(debug_dasm, _T("BIT 0, C=(IX+(%d))"), ofs); break;
	case 0x42: _stprintf(debug_dasm, _T("BIT 0, D=(IX+(%d))"), ofs); break;
	case 0x43: _stprintf(debug_dasm, _T("BIT 0, E=(IX+(%d))"), ofs); break;
	case 0x44: _stprintf(debug_dasm, _T("BIT 0, H=(IX+(%d))"), ofs); break;
	case 0x45: _stprintf(debug_dasm, _T("BIT 0, L=(IX+(%d))"), ofs); break;
	case 0x46: _stprintf(debug_dasm, _T("BIT 0, (IX+(%d))"), ofs); break;
	case 0x47: _stprintf(debug_dasm, _T("BIT 0, A=(IX+(%d))"), ofs); break;
	case 0x48: _stprintf(debug_dasm, _T("BIT 1, B=(IX+(%d))"), ofs); break;
	case 0x49: _stprintf(debug_dasm, _T("BIT 1, C=(IX+(%d))"), ofs); break;
	case 0x4a: _stprintf(debug_dasm, _T("BIT 1, D=(IX+(%d))"), ofs); break;
	case 0x4b: _stprintf(debug_dasm, _T("BIT 1, E=(IX+(%d))"), ofs); break;
	case 0x4c: _stprintf(debug_dasm, _T("BIT 1, H=(IX+(%d))"), ofs); break;
	case 0x4d: _stprintf(debug_dasm, _T("BIT 1, L=(IX+(%d))"), ofs); break;
	case 0x4e: _stprintf(debug_dasm, _T("BIT 1, (IX+(%d))"), ofs); break;
	case 0x4f: _stprintf(debug_dasm, _T("BIT 1, A=(IX+(%d))"), ofs); break;
	case 0x50: _stprintf(debug_dasm, _T("BIT 2, B=(IX+(%d))"), ofs); break;
	case 0x51: _stprintf(debug_dasm, _T("BIT 2, C=(IX+(%d))"), ofs); break;
	case 0x52: _stprintf(debug_dasm, _T("BIT 2, D=(IX+(%d))"), ofs); break;
	case 0x53: _stprintf(debug_dasm, _T("BIT 2, E=(IX+(%d))"), ofs); break;
	case 0x54: _stprintf(debug_dasm, _T("BIT 2, H=(IX+(%d))"), ofs); break;
	case 0x55: _stprintf(debug_dasm, _T("BIT 2, L=(IX+(%d))"), ofs); break;
	case 0x56: _stprintf(debug_dasm, _T("BIT 2, (IX+(%d))"), ofs); break;
	case 0x57: _stprintf(debug_dasm, _T("BIT 2, A=(IX+(%d))"), ofs); break;
	case 0x58: _stprintf(debug_dasm, _T("BIT 3, B=(IX+(%d))"), ofs); break;
	case 0x59: _stprintf(debug_dasm, _T("BIT 3, C=(IX+(%d))"), ofs); break;
	case 0x5a: _stprintf(debug_dasm, _T("BIT 3, D=(IX+(%d))"), ofs); break;
	case 0x5b: _stprintf(debug_dasm, _T("BIT 3, E=(IX+(%d))"), ofs); break;
	case 0x5c: _stprintf(debug_dasm, _T("BIT 3, H=(IX+(%d))"), ofs); break;
	case 0x5d: _stprintf(debug_dasm, _T("BIT 3, L=(IX+(%d))"), ofs); break;
	case 0x5e: _stprintf(debug_dasm, _T("BIT 3, (IX+(%d))"), ofs); break;
	case 0x5f: _stprintf(debug_dasm, _T("BIT 3, A=(IX+(%d))"), ofs); break;
	case 0x60: _stprintf(debug_dasm, _T("BIT 4, B=(IX+(%d))"), ofs); break;
	case 0x61: _stprintf(debug_dasm, _T("BIT 4, C=(IX+(%d))"), ofs); break;
	case 0x62: _stprintf(debug_dasm, _T("BIT 4, D=(IX+(%d))"), ofs); break;
	case 0x63: _stprintf(debug_dasm, _T("BIT 4, E=(IX+(%d))"), ofs); break;
	case 0x64: _stprintf(debug_dasm, _T("BIT 4, H=(IX+(%d))"), ofs); break;
	case 0x65: _stprintf(debug_dasm, _T("BIT 4, L=(IX+(%d))"), ofs); break;
	case 0x66: _stprintf(debug_dasm, _T("BIT 4, (IX+(%d))"), ofs); break;
	case 0x67: _stprintf(debug_dasm, _T("BIT 4, A=(IX+(%d))"), ofs); break;
	case 0x68: _stprintf(debug_dasm, _T("BIT 5, B=(IX+(%d))"), ofs); break;
	case 0x69: _stprintf(debug_dasm, _T("BIT 5, C=(IX+(%d))"), ofs); break;
	case 0x6a: _stprintf(debug_dasm, _T("BIT 5, D=(IX+(%d))"), ofs); break;
	case 0x6b: _stprintf(debug_dasm, _T("BIT 5, E=(IX+(%d))"), ofs); break;
	case 0x6c: _stprintf(debug_dasm, _T("BIT 5, H=(IX+(%d))"), ofs); break;
	case 0x6d: _stprintf(debug_dasm, _T("BIT 5, L=(IX+(%d))"), ofs); break;
	case 0x6e: _stprintf(debug_dasm, _T("BIT 5, (IX+(%d))"), ofs); break;
	case 0x6f: _stprintf(debug_dasm, _T("BIT 5, A=(IX+(%d))"), ofs); break;
	case 0x70: _stprintf(debug_dasm, _T("BIT 6, B=(IX+(%d))"), ofs); break;
	case 0x71: _stprintf(debug_dasm, _T("BIT 6, C=(IX+(%d))"), ofs); break;
	case 0x72: _stprintf(debug_dasm, _T("BIT 6, D=(IX+(%d))"), ofs); break;
	case 0x73: _stprintf(debug_dasm, _T("BIT 6, E=(IX+(%d))"), ofs); break;
	case 0x74: _stprintf(debug_dasm, _T("BIT 6, H=(IX+(%d))"), ofs); break;
	case 0x75: _stprintf(debug_dasm, _T("BIT 6, L=(IX+(%d))"), ofs); break;
	case 0x76: _stprintf(debug_dasm, _T("BIT 6, (IX+(%d))"), ofs); break;
	case 0x77: _stprintf(debug_dasm, _T("BIT 6, A=(IX+(%d))"), ofs); break;
	case 0x78: _stprintf(debug_dasm, _T("BIT 7, B=(IX+(%d))"), ofs); break;
	case 0x79: _stprintf(debug_dasm, _T("BIT 7, C=(IX+(%d))"), ofs); break;
	case 0x7a: _stprintf(debug_dasm, _T("BIT 7, D=(IX+(%d))"), ofs); break;
	case 0x7b: _stprintf(debug_dasm, _T("BIT 7, E=(IX+(%d))"), ofs); break;
	case 0x7c: _stprintf(debug_dasm, _T("BIT 7, H=(IX+(%d))"), ofs); break;
	case 0x7d: _stprintf(debug_dasm, _T("BIT 7, L=(IX+(%d))"), ofs); break;
	case 0x7e: _stprintf(debug_dasm, _T("BIT 7, (IX+(%d))"), ofs); break;
	case 0x7f: _stprintf(debug_dasm, _T("BIT 7, A=(IX+(%d))"), ofs); break;
	case 0x80: _stprintf(debug_dasm, _T("RES 0, B=(IX+(%d))"), ofs); break;
	case 0x81: _stprintf(debug_dasm, _T("RES 0, C=(IX+(%d))"), ofs); break;
	case 0x82: _stprintf(debug_dasm, _T("RES 0, D=(IX+(%d))"), ofs); break;
	case 0x83: _stprintf(debug_dasm, _T("RES 0, E=(IX+(%d))"), ofs); break;
	case 0x84: _stprintf(debug_dasm, _T("RES 0, H=(IX+(%d))"), ofs); break;
	case 0x85: _stprintf(debug_dasm, _T("RES 0, L=(IX+(%d))"), ofs); break;
	case 0x86: _stprintf(debug_dasm, _T("RES 0, (IX+(%d))"), ofs); break;
	case 0x87: _stprintf(debug_dasm, _T("RES 0, A=(IX+(%d))"), ofs); break;
	case 0x88: _stprintf(debug_dasm, _T("RES 1, B=(IX+(%d))"), ofs); break;
	case 0x89: _stprintf(debug_dasm, _T("RES 1, C=(IX+(%d))"), ofs); break;
	case 0x8a: _stprintf(debug_dasm, _T("RES 1, D=(IX+(%d))"), ofs); break;
	case 0x8b: _stprintf(debug_dasm, _T("RES 1, E=(IX+(%d))"), ofs); break;
	case 0x8c: _stprintf(debug_dasm, _T("RES 1, H=(IX+(%d))"), ofs); break;
	case 0x8d: _stprintf(debug_dasm, _T("RES 1, L=(IX+(%d))"), ofs); break;
	case 0x8e: _stprintf(debug_dasm, _T("RES 1, (IX+(%d))"), ofs); break;
	case 0x8f: _stprintf(debug_dasm, _T("RES 1, A=(IX+(%d))"), ofs); break;
	case 0x90: _stprintf(debug_dasm, _T("RES 2, B=(IX+(%d))"), ofs); break;
	case 0x91: _stprintf(debug_dasm, _T("RES 2, C=(IX+(%d))"), ofs); break;
	case 0x92: _stprintf(debug_dasm, _T("RES 2, D=(IX+(%d))"), ofs); break;
	case 0x93: _stprintf(debug_dasm, _T("RES 2, E=(IX+(%d))"), ofs); break;
	case 0x94: _stprintf(debug_dasm, _T("RES 2, H=(IX+(%d))"), ofs); break;
	case 0x95: _stprintf(debug_dasm, _T("RES 2, L=(IX+(%d))"), ofs); break;
	case 0x96: _stprintf(debug_dasm, _T("RES 2, (IX+(%d))"), ofs); break;
	case 0x97: _stprintf(debug_dasm, _T("RES 2, A=(IX+(%d))"), ofs); break;
	case 0x98: _stprintf(debug_dasm, _T("RES 3, B=(IX+(%d))"), ofs); break;
	case 0x99: _stprintf(debug_dasm, _T("RES 3, C=(IX+(%d))"), ofs); break;
	case 0x9a: _stprintf(debug_dasm, _T("RES 3, D=(IX+(%d))"), ofs); break;
	case 0x9b: _stprintf(debug_dasm, _T("RES 3, E=(IX+(%d))"), ofs); break;
	case 0x9c: _stprintf(debug_dasm, _T("RES 3, H=(IX+(%d))"), ofs); break;
	case 0x9d: _stprintf(debug_dasm, _T("RES 3, L=(IX+(%d))"), ofs); break;
	case 0x9e: _stprintf(debug_dasm, _T("RES 3, (IX+(%d))"), ofs); break;
	case 0x9f: _stprintf(debug_dasm, _T("RES 3, A=(IX+(%d))"), ofs); break;
	case 0xa0: _stprintf(debug_dasm, _T("RES 4, B=(IX+(%d))"), ofs); break;
	case 0xa1: _stprintf(debug_dasm, _T("RES 4, C=(IX+(%d))"), ofs); break;
	case 0xa2: _stprintf(debug_dasm, _T("RES 4, D=(IX+(%d))"), ofs); break;
	case 0xa3: _stprintf(debug_dasm, _T("RES 4, E=(IX+(%d))"), ofs); break;
	case 0xa4: _stprintf(debug_dasm, _T("RES 4, H=(IX+(%d))"), ofs); break;
	case 0xa5: _stprintf(debug_dasm, _T("RES 4, L=(IX+(%d))"), ofs); break;
	case 0xa6: _stprintf(debug_dasm, _T("RES 4, (IX+(%d))"), ofs); break;
	case 0xa7: _stprintf(debug_dasm, _T("RES 4, A=(IX+(%d))"), ofs); break;
	case 0xa8: _stprintf(debug_dasm, _T("RES 5, B=(IX+(%d))"), ofs); break;
	case 0xa9: _stprintf(debug_dasm, _T("RES 5, C=(IX+(%d))"), ofs); break;
	case 0xaa: _stprintf(debug_dasm, _T("RES 5, D=(IX+(%d))"), ofs); break;
	case 0xab: _stprintf(debug_dasm, _T("RES 5, E=(IX+(%d))"), ofs); break;
	case 0xac: _stprintf(debug_dasm, _T("RES 5, H=(IX+(%d))"), ofs); break;
	case 0xad: _stprintf(debug_dasm, _T("RES 5, L=(IX+(%d))"), ofs); break;
	case 0xae: _stprintf(debug_dasm, _T("RES 5, (IX+(%d))"), ofs); break;
	case 0xaf: _stprintf(debug_dasm, _T("RES 5, A=(IX+(%d))"), ofs); break;
	case 0xb0: _stprintf(debug_dasm, _T("RES 6, B=(IX+(%d))"), ofs); break;
	case 0xb1: _stprintf(debug_dasm, _T("RES 6, C=(IX+(%d))"), ofs); break;
	case 0xb2: _stprintf(debug_dasm, _T("RES 6, D=(IX+(%d))"), ofs); break;
	case 0xb3: _stprintf(debug_dasm, _T("RES 6, E=(IX+(%d))"), ofs); break;
	case 0xb4: _stprintf(debug_dasm, _T("RES 6, H=(IX+(%d))"), ofs); break;
	case 0xb5: _stprintf(debug_dasm, _T("RES 6, L=(IX+(%d))"), ofs); break;
	case 0xb6: _stprintf(debug_dasm, _T("RES 6, (IX+(%d))"), ofs); break;
	case 0xb7: _stprintf(debug_dasm, _T("RES 6, A=(IX+(%d))"), ofs); break;
	case 0xb8: _stprintf(debug_dasm, _T("RES 7, B=(IX+(%d))"), ofs); break;
	case 0xb9: _stprintf(debug_dasm, _T("RES 7, C=(IX+(%d))"), ofs); break;
	case 0xba: _stprintf(debug_dasm, _T("RES 7, D=(IX+(%d))"), ofs); break;
	case 0xbb: _stprintf(debug_dasm, _T("RES 7, E=(IX+(%d))"), ofs); break;
	case 0xbc: _stprintf(debug_dasm, _T("RES 7, H=(IX+(%d))"), ofs); break;
	case 0xbd: _stprintf(debug_dasm, _T("RES 7, L=(IX+(%d))"), ofs); break;
	case 0xbe: _stprintf(debug_dasm, _T("RES 7, (IX+(%d))"), ofs); break;
	case 0xbf: _stprintf(debug_dasm, _T("RES 7, A=(IX+(%d))"), ofs); break;
	case 0xc0: _stprintf(debug_dasm, _T("SET 0, B=(IX+(%d))"), ofs); break;
	case 0xc1: _stprintf(debug_dasm, _T("SET 0, C=(IX+(%d))"), ofs); break;
	case 0xc2: _stprintf(debug_dasm, _T("SET 0, D=(IX+(%d))"), ofs); break;
	case 0xc3: _stprintf(debug_dasm, _T("SET 0, E=(IX+(%d))"), ofs); break;
	case 0xc4: _stprintf(debug_dasm, _T("SET 0, H=(IX+(%d))"), ofs); break;
	case 0xc5: _stprintf(debug_dasm, _T("SET 0, L=(IX+(%d))"), ofs); break;
	case 0xc6: _stprintf(debug_dasm, _T("SET 0, (IX+(%d))"), ofs); break;
	case 0xc7: _stprintf(debug_dasm, _T("SET 0, A=(IX+(%d))"), ofs); break;
	case 0xc8: _stprintf(debug_dasm, _T("SET 1, B=(IX+(%d))"), ofs); break;
	case 0xc9: _stprintf(debug_dasm, _T("SET 1, C=(IX+(%d))"), ofs); break;
	case 0xca: _stprintf(debug_dasm, _T("SET 1, D=(IX+(%d))"), ofs); break;
	case 0xcb: _stprintf(debug_dasm, _T("SET 1, E=(IX+(%d))"), ofs); break;
	case 0xcc: _stprintf(debug_dasm, _T("SET 1, H=(IX+(%d))"), ofs); break;
	case 0xcd: _stprintf(debug_dasm, _T("SET 1, L=(IX+(%d))"), ofs); break;
	case 0xce: _stprintf(debug_dasm, _T("SET 1, (IX+(%d))"), ofs); break;
	case 0xcf: _stprintf(debug_dasm, _T("SET 1, A=(IX+(%d))"), ofs); break;
	case 0xd0: _stprintf(debug_dasm, _T("SET 2, B=(IX+(%d))"), ofs); break;
	case 0xd1: _stprintf(debug_dasm, _T("SET 2, C=(IX+(%d))"), ofs); break;
	case 0xd2: _stprintf(debug_dasm, _T("SET 2, D=(IX+(%d))"), ofs); break;
	case 0xd3: _stprintf(debug_dasm, _T("SET 2, E=(IX+(%d))"), ofs); break;
	case 0xd4: _stprintf(debug_dasm, _T("SET 2, H=(IX+(%d))"), ofs); break;
	case 0xd5: _stprintf(debug_dasm, _T("SET 2, L=(IX+(%d))"), ofs); break;
	case 0xd6: _stprintf(debug_dasm, _T("SET 2, (IX+(%d))"), ofs); break;
	case 0xd7: _stprintf(debug_dasm, _T("SET 2, A=(IX+(%d))"), ofs); break;
	case 0xd8: _stprintf(debug_dasm, _T("SET 3, B=(IX+(%d))"), ofs); break;
	case 0xd9: _stprintf(debug_dasm, _T("SET 3, C=(IX+(%d))"), ofs); break;
	case 0xda: _stprintf(debug_dasm, _T("SET 3, D=(IX+(%d))"), ofs); break;
	case 0xdb: _stprintf(debug_dasm, _T("SET 3, E=(IX+(%d))"), ofs); break;
	case 0xdc: _stprintf(debug_dasm, _T("SET 3, H=(IX+(%d))"), ofs); break;
	case 0xdd: _stprintf(debug_dasm, _T("SET 3, L=(IX+(%d))"), ofs); break;
	case 0xde: _stprintf(debug_dasm, _T("SET 3, (IX+(%d))"), ofs); break;
	case 0xdf: _stprintf(debug_dasm, _T("SET 3, A=(IX+(%d))"), ofs); break;
	case 0xe0: _stprintf(debug_dasm, _T("SET 4, B=(IX+(%d))"), ofs); break;
	case 0xe1: _stprintf(debug_dasm, _T("SET 4, C=(IX+(%d))"), ofs); break;
	case 0xe2: _stprintf(debug_dasm, _T("SET 4, D=(IX+(%d))"), ofs); break;
	case 0xe3: _stprintf(debug_dasm, _T("SET 4, E=(IX+(%d))"), ofs); break;
	case 0xe4: _stprintf(debug_dasm, _T("SET 4, H=(IX+(%d))"), ofs); break;
	case 0xe5: _stprintf(debug_dasm, _T("SET 4, L=(IX+(%d))"), ofs); break;
	case 0xe6: _stprintf(debug_dasm, _T("SET 4, (IX+(%d))"), ofs); break;
	case 0xe7: _stprintf(debug_dasm, _T("SET 4, A=(IX+(%d))"), ofs); break;
	case 0xe8: _stprintf(debug_dasm, _T("SET 5, B=(IX+(%d))"), ofs); break;
	case 0xe9: _stprintf(debug_dasm, _T("SET 5, C=(IX+(%d))"), ofs); break;
	case 0xea: _stprintf(debug_dasm, _T("SET 5, D=(IX+(%d))"), ofs); break;
	case 0xeb: _stprintf(debug_dasm, _T("SET 5, E=(IX+(%d))"), ofs); break;
	case 0xec: _stprintf(debug_dasm, _T("SET 5, H=(IX+(%d))"), ofs); break;
	case 0xed: _stprintf(debug_dasm, _T("SET 5, L=(IX+(%d))"), ofs); break;
	case 0xee: _stprintf(debug_dasm, _T("SET 5, (IX+(%d))"), ofs); break;
	case 0xef: _stprintf(debug_dasm, _T("SET 5, A=(IX+(%d))"), ofs); break;
	case 0xf0: _stprintf(debug_dasm, _T("SET 6, B=(IX+(%d))"), ofs); break;
	case 0xf1: _stprintf(debug_dasm, _T("SET 6, C=(IX+(%d))"), ofs); break;
	case 0xf2: _stprintf(debug_dasm, _T("SET 6, D=(IX+(%d))"), ofs); break;
	case 0xf3: _stprintf(debug_dasm, _T("SET 6, E=(IX+(%d))"), ofs); break;
	case 0xf4: _stprintf(debug_dasm, _T("SET 6, H=(IX+(%d))"), ofs); break;
	case 0xf5: _stprintf(debug_dasm, _T("SET 6, L=(IX+(%d))"), ofs); break;
	case 0xf6: _stprintf(debug_dasm, _T("SET 6, (IX+(%d))"), ofs); break;
	case 0xf7: _stprintf(debug_dasm, _T("SET 6, A=(IX+(%d))"), ofs); break;
	case 0xf8: _stprintf(debug_dasm, _T("SET 7, B=(IX+(%d))"), ofs); break;
	case 0xf9: _stprintf(debug_dasm, _T("SET 7, C=(IX+(%d))"), ofs); break;
	case 0xfa: _stprintf(debug_dasm, _T("SET 7, D=(IX+(%d))"), ofs); break;
	case 0xfb: _stprintf(debug_dasm, _T("SET 7, E=(IX+(%d))"), ofs); break;
	case 0xfc: _stprintf(debug_dasm, _T("SET 7, H=(IX+(%d))"), ofs); break;
	case 0xfd: _stprintf(debug_dasm, _T("SET 7, L=(IX+(%d))"), ofs); break;
	case 0xfe: _stprintf(debug_dasm, _T("SET 7, (IX+(%d))"), ofs); break;
	case 0xff: _stprintf(debug_dasm, _T("SET 7, A=(IX+(%d))"), ofs); break;
	}
}

void Z80::DASM_FDCB()
{
	int8 ofs = DEBUG_FETCH8_REL();
	uint8 code = DEBUG_FETCH8();
	
	switch(code) {
	case 0x00: _stprintf(debug_dasm, _T("RLC B=(IY+(%d))"), ofs); break;
	case 0x01: _stprintf(debug_dasm, _T("RLC C=(IY+(%d))"), ofs); break;
	case 0x02: _stprintf(debug_dasm, _T("RLC D=(IY+(%d))"), ofs); break;
	case 0x03: _stprintf(debug_dasm, _T("RLC E=(IY+(%d))"), ofs); break;
	case 0x04: _stprintf(debug_dasm, _T("RLC H=(IY+(%d))"), ofs); break;
	case 0x05: _stprintf(debug_dasm, _T("RLC L=(IY+(%d))"), ofs); break;
	case 0x06: _stprintf(debug_dasm, _T("RLC (IY+(%d))"), ofs); break;
	case 0x07: _stprintf(debug_dasm, _T("RLC A=(IY+(%d))"), ofs); break;
	case 0x08: _stprintf(debug_dasm, _T("RRC B=(IY+(%d))"), ofs); break;
	case 0x09: _stprintf(debug_dasm, _T("RRC C=(IY+(%d))"), ofs); break;
	case 0x0a: _stprintf(debug_dasm, _T("RRC D=(IY+(%d))"), ofs); break;
	case 0x0b: _stprintf(debug_dasm, _T("RRC E=(IY+(%d))"), ofs); break;
	case 0x0c: _stprintf(debug_dasm, _T("RRC H=(IY+(%d))"), ofs); break;
	case 0x0d: _stprintf(debug_dasm, _T("RRC L=(IY+(%d))"), ofs); break;
	case 0x0e: _stprintf(debug_dasm, _T("RRC (IY+(%d))"), ofs); break;
	case 0x0f: _stprintf(debug_dasm, _T("RRC A=(IY+(%d))"), ofs); break;
	case 0x10: _stprintf(debug_dasm, _T("RL B=(IY+(%d))"), ofs); break;
	case 0x11: _stprintf(debug_dasm, _T("RL C=(IY+(%d))"), ofs); break;
	case 0x12: _stprintf(debug_dasm, _T("RL D=(IY+(%d))"), ofs); break;
	case 0x13: _stprintf(debug_dasm, _T("RL E=(IY+(%d))"), ofs); break;
	case 0x14: _stprintf(debug_dasm, _T("RL H=(IY+(%d))"), ofs); break;
	case 0x15: _stprintf(debug_dasm, _T("RL L=(IY+(%d))"), ofs); break;
	case 0x16: _stprintf(debug_dasm, _T("RL (IY+(%d))"), ofs); break;
	case 0x17: _stprintf(debug_dasm, _T("RL A=(IY+(%d))"), ofs); break;
	case 0x18: _stprintf(debug_dasm, _T("RR B=(IY+(%d))"), ofs); break;
	case 0x19: _stprintf(debug_dasm, _T("RR C=(IY+(%d))"), ofs); break;
	case 0x1a: _stprintf(debug_dasm, _T("RR D=(IY+(%d))"), ofs); break;
	case 0x1b: _stprintf(debug_dasm, _T("RR E=(IY+(%d))"), ofs); break;
	case 0x1c: _stprintf(debug_dasm, _T("RR H=(IY+(%d))"), ofs); break;
	case 0x1d: _stprintf(debug_dasm, _T("RR L=(IY+(%d))"), ofs); break;
	case 0x1e: _stprintf(debug_dasm, _T("RR (IY+(%d))"), ofs); break;
	case 0x1f: _stprintf(debug_dasm, _T("RR A=(IY+(%d))"), ofs); break;
	case 0x20: _stprintf(debug_dasm, _T("SLA B=(IY+(%d))"), ofs); break;
	case 0x21: _stprintf(debug_dasm, _T("SLA C=(IY+(%d))"), ofs); break;
	case 0x22: _stprintf(debug_dasm, _T("SLA D=(IY+(%d))"), ofs); break;
	case 0x23: _stprintf(debug_dasm, _T("SLA E=(IY+(%d))"), ofs); break;
	case 0x24: _stprintf(debug_dasm, _T("SLA H=(IY+(%d))"), ofs); break;
	case 0x25: _stprintf(debug_dasm, _T("SLA L=(IY+(%d))"), ofs); break;
	case 0x26: _stprintf(debug_dasm, _T("SLA (IY+(%d))"), ofs); break;
	case 0x27: _stprintf(debug_dasm, _T("SLA A=(IY+(%d))"), ofs); break;
	case 0x28: _stprintf(debug_dasm, _T("SRA B=(IY+(%d))"), ofs); break;
	case 0x29: _stprintf(debug_dasm, _T("SRA C=(IY+(%d))"), ofs); break;
	case 0x2a: _stprintf(debug_dasm, _T("SRA D=(IY+(%d))"), ofs); break;
	case 0x2b: _stprintf(debug_dasm, _T("SRA E=(IY+(%d))"), ofs); break;
	case 0x2c: _stprintf(debug_dasm, _T("SRA H=(IY+(%d))"), ofs); break;
	case 0x2d: _stprintf(debug_dasm, _T("SRA L=(IY+(%d))"), ofs); break;
	case 0x2e: _stprintf(debug_dasm, _T("SRA (IY+(%d))"), ofs); break;
	case 0x2f: _stprintf(debug_dasm, _T("SRA A=(IY+(%d))"), ofs); break;
	case 0x30: _stprintf(debug_dasm, _T("SLL B=(IY+(%d))"), ofs); break;
	case 0x31: _stprintf(debug_dasm, _T("SLL C=(IY+(%d))"), ofs); break;
	case 0x32: _stprintf(debug_dasm, _T("SLL D=(IY+(%d))"), ofs); break;
	case 0x33: _stprintf(debug_dasm, _T("SLL E=(IY+(%d))"), ofs); break;
	case 0x34: _stprintf(debug_dasm, _T("SLL H=(IY+(%d))"), ofs); break;
	case 0x35: _stprintf(debug_dasm, _T("SLL L=(IY+(%d))"), ofs); break;
	case 0x36: _stprintf(debug_dasm, _T("SLL (IY+(%d))"), ofs); break;
	case 0x37: _stprintf(debug_dasm, _T("SLL A=(IY+(%d))"), ofs); break;
	case 0x38: _stprintf(debug_dasm, _T("SRL B=(IY+(%d))"), ofs); break;
	case 0x39: _stprintf(debug_dasm, _T("SRL C=(IY+(%d))"), ofs); break;
	case 0x3a: _stprintf(debug_dasm, _T("SRL D=(IY+(%d))"), ofs); break;
	case 0x3b: _stprintf(debug_dasm, _T("SRL E=(IY+(%d))"), ofs); break;
	case 0x3c: _stprintf(debug_dasm, _T("SRL H=(IY+(%d))"), ofs); break;
	case 0x3d: _stprintf(debug_dasm, _T("SRL L=(IY+(%d))"), ofs); break;
	case 0x3e: _stprintf(debug_dasm, _T("SRL (IY+(%d))"), ofs); break;
	case 0x3f: _stprintf(debug_dasm, _T("SRL A=(IY+(%d))"), ofs); break;
	case 0x40: _stprintf(debug_dasm, _T("BIT 0, B=(IY+(%d))"), ofs); break;
	case 0x41: _stprintf(debug_dasm, _T("BIT 0, C=(IY+(%d))"), ofs); break;
	case 0x42: _stprintf(debug_dasm, _T("BIT 0, D=(IY+(%d))"), ofs); break;
	case 0x43: _stprintf(debug_dasm, _T("BIT 0, E=(IY+(%d))"), ofs); break;
	case 0x44: _stprintf(debug_dasm, _T("BIT 0, H=(IY+(%d))"), ofs); break;
	case 0x45: _stprintf(debug_dasm, _T("BIT 0, L=(IY+(%d))"), ofs); break;
	case 0x46: _stprintf(debug_dasm, _T("BIT 0, (IY+(%d))"), ofs); break;
	case 0x47: _stprintf(debug_dasm, _T("BIT 0, A=(IY+(%d))"), ofs); break;
	case 0x48: _stprintf(debug_dasm, _T("BIT 1, B=(IY+(%d))"), ofs); break;
	case 0x49: _stprintf(debug_dasm, _T("BIT 1, C=(IY+(%d))"), ofs); break;
	case 0x4a: _stprintf(debug_dasm, _T("BIT 1, D=(IY+(%d))"), ofs); break;
	case 0x4b: _stprintf(debug_dasm, _T("BIT 1, E=(IY+(%d))"), ofs); break;
	case 0x4c: _stprintf(debug_dasm, _T("BIT 1, H=(IY+(%d))"), ofs); break;
	case 0x4d: _stprintf(debug_dasm, _T("BIT 1, L=(IY+(%d))"), ofs); break;
	case 0x4e: _stprintf(debug_dasm, _T("BIT 1, (IY+(%d))"), ofs); break;
	case 0x4f: _stprintf(debug_dasm, _T("BIT 1, A=(IY+(%d))"), ofs); break;
	case 0x50: _stprintf(debug_dasm, _T("BIT 2, B=(IY+(%d))"), ofs); break;
	case 0x51: _stprintf(debug_dasm, _T("BIT 2, C=(IY+(%d))"), ofs); break;
	case 0x52: _stprintf(debug_dasm, _T("BIT 2, D=(IY+(%d))"), ofs); break;
	case 0x53: _stprintf(debug_dasm, _T("BIT 2, E=(IY+(%d))"), ofs); break;
	case 0x54: _stprintf(debug_dasm, _T("BIT 2, H=(IY+(%d))"), ofs); break;
	case 0x55: _stprintf(debug_dasm, _T("BIT 2, L=(IY+(%d))"), ofs); break;
	case 0x56: _stprintf(debug_dasm, _T("BIT 2, (IY+(%d))"), ofs); break;
	case 0x57: _stprintf(debug_dasm, _T("BIT 2, A=(IY+(%d))"), ofs); break;
	case 0x58: _stprintf(debug_dasm, _T("BIT 3, B=(IY+(%d))"), ofs); break;
	case 0x59: _stprintf(debug_dasm, _T("BIT 3, C=(IY+(%d))"), ofs); break;
	case 0x5a: _stprintf(debug_dasm, _T("BIT 3, D=(IY+(%d))"), ofs); break;
	case 0x5b: _stprintf(debug_dasm, _T("BIT 3, E=(IY+(%d))"), ofs); break;
	case 0x5c: _stprintf(debug_dasm, _T("BIT 3, H=(IY+(%d))"), ofs); break;
	case 0x5d: _stprintf(debug_dasm, _T("BIT 3, L=(IY+(%d))"), ofs); break;
	case 0x5e: _stprintf(debug_dasm, _T("BIT 3, (IY+(%d))"), ofs); break;
	case 0x5f: _stprintf(debug_dasm, _T("BIT 3, A=(IY+(%d))"), ofs); break;
	case 0x60: _stprintf(debug_dasm, _T("BIT 4, B=(IY+(%d))"), ofs); break;
	case 0x61: _stprintf(debug_dasm, _T("BIT 4, C=(IY+(%d))"), ofs); break;
	case 0x62: _stprintf(debug_dasm, _T("BIT 4, D=(IY+(%d))"), ofs); break;
	case 0x63: _stprintf(debug_dasm, _T("BIT 4, E=(IY+(%d))"), ofs); break;
	case 0x64: _stprintf(debug_dasm, _T("BIT 4, H=(IY+(%d))"), ofs); break;
	case 0x65: _stprintf(debug_dasm, _T("BIT 4, L=(IY+(%d))"), ofs); break;
	case 0x66: _stprintf(debug_dasm, _T("BIT 4, (IY+(%d))"), ofs); break;
	case 0x67: _stprintf(debug_dasm, _T("BIT 4, A=(IY+(%d))"), ofs); break;
	case 0x68: _stprintf(debug_dasm, _T("BIT 5, B=(IY+(%d))"), ofs); break;
	case 0x69: _stprintf(debug_dasm, _T("BIT 5, C=(IY+(%d))"), ofs); break;
	case 0x6a: _stprintf(debug_dasm, _T("BIT 5, D=(IY+(%d))"), ofs); break;
	case 0x6b: _stprintf(debug_dasm, _T("BIT 5, E=(IY+(%d))"), ofs); break;
	case 0x6c: _stprintf(debug_dasm, _T("BIT 5, H=(IY+(%d))"), ofs); break;
	case 0x6d: _stprintf(debug_dasm, _T("BIT 5, L=(IY+(%d))"), ofs); break;
	case 0x6e: _stprintf(debug_dasm, _T("BIT 5, (IY+(%d))"), ofs); break;
	case 0x6f: _stprintf(debug_dasm, _T("BIT 5, A=(IY+(%d))"), ofs); break;
	case 0x70: _stprintf(debug_dasm, _T("BIT 6, B=(IY+(%d))"), ofs); break;
	case 0x71: _stprintf(debug_dasm, _T("BIT 6, C=(IY+(%d))"), ofs); break;
	case 0x72: _stprintf(debug_dasm, _T("BIT 6, D=(IY+(%d))"), ofs); break;
	case 0x73: _stprintf(debug_dasm, _T("BIT 6, E=(IY+(%d))"), ofs); break;
	case 0x74: _stprintf(debug_dasm, _T("BIT 6, H=(IY+(%d))"), ofs); break;
	case 0x75: _stprintf(debug_dasm, _T("BIT 6, L=(IY+(%d))"), ofs); break;
	case 0x76: _stprintf(debug_dasm, _T("BIT 6, (IY+(%d))"), ofs); break;
	case 0x77: _stprintf(debug_dasm, _T("BIT 6, A=(IY+(%d))"), ofs); break;
	case 0x78: _stprintf(debug_dasm, _T("BIT 7, B=(IY+(%d))"), ofs); break;
	case 0x79: _stprintf(debug_dasm, _T("BIT 7, C=(IY+(%d))"), ofs); break;
	case 0x7a: _stprintf(debug_dasm, _T("BIT 7, D=(IY+(%d))"), ofs); break;
	case 0x7b: _stprintf(debug_dasm, _T("BIT 7, E=(IY+(%d))"), ofs); break;
	case 0x7c: _stprintf(debug_dasm, _T("BIT 7, H=(IY+(%d))"), ofs); break;
	case 0x7d: _stprintf(debug_dasm, _T("BIT 7, L=(IY+(%d))"), ofs); break;
	case 0x7e: _stprintf(debug_dasm, _T("BIT 7, (IY+(%d))"), ofs); break;
	case 0x7f: _stprintf(debug_dasm, _T("BIT 7, A=(IY+(%d))"), ofs); break;
	case 0x80: _stprintf(debug_dasm, _T("RES 0, B=(IY+(%d))"), ofs); break;
	case 0x81: _stprintf(debug_dasm, _T("RES 0, C=(IY+(%d))"), ofs); break;
	case 0x82: _stprintf(debug_dasm, _T("RES 0, D=(IY+(%d))"), ofs); break;
	case 0x83: _stprintf(debug_dasm, _T("RES 0, E=(IY+(%d))"), ofs); break;
	case 0x84: _stprintf(debug_dasm, _T("RES 0, H=(IY+(%d))"), ofs); break;
	case 0x85: _stprintf(debug_dasm, _T("RES 0, L=(IY+(%d))"), ofs); break;
	case 0x86: _stprintf(debug_dasm, _T("RES 0, (IY+(%d))"), ofs); break;
	case 0x87: _stprintf(debug_dasm, _T("RES 0, A=(IY+(%d))"), ofs); break;
	case 0x88: _stprintf(debug_dasm, _T("RES 1, B=(IY+(%d))"), ofs); break;
	case 0x89: _stprintf(debug_dasm, _T("RES 1, C=(IY+(%d))"), ofs); break;
	case 0x8a: _stprintf(debug_dasm, _T("RES 1, D=(IY+(%d))"), ofs); break;
	case 0x8b: _stprintf(debug_dasm, _T("RES 1, E=(IY+(%d))"), ofs); break;
	case 0x8c: _stprintf(debug_dasm, _T("RES 1, H=(IY+(%d))"), ofs); break;
	case 0x8d: _stprintf(debug_dasm, _T("RES 1, L=(IY+(%d))"), ofs); break;
	case 0x8e: _stprintf(debug_dasm, _T("RES 1, (IY+(%d))"), ofs); break;
	case 0x8f: _stprintf(debug_dasm, _T("RES 1, A=(IY+(%d))"), ofs); break;
	case 0x90: _stprintf(debug_dasm, _T("RES 2, B=(IY+(%d))"), ofs); break;
	case 0x91: _stprintf(debug_dasm, _T("RES 2, C=(IY+(%d))"), ofs); break;
	case 0x92: _stprintf(debug_dasm, _T("RES 2, D=(IY+(%d))"), ofs); break;
	case 0x93: _stprintf(debug_dasm, _T("RES 2, E=(IY+(%d))"), ofs); break;
	case 0x94: _stprintf(debug_dasm, _T("RES 2, H=(IY+(%d))"), ofs); break;
	case 0x95: _stprintf(debug_dasm, _T("RES 2, L=(IY+(%d))"), ofs); break;
	case 0x96: _stprintf(debug_dasm, _T("RES 2, (IY+(%d))"), ofs); break;
	case 0x97: _stprintf(debug_dasm, _T("RES 2, A=(IY+(%d))"), ofs); break;
	case 0x98: _stprintf(debug_dasm, _T("RES 3, B=(IY+(%d))"), ofs); break;
	case 0x99: _stprintf(debug_dasm, _T("RES 3, C=(IY+(%d))"), ofs); break;
	case 0x9a: _stprintf(debug_dasm, _T("RES 3, D=(IY+(%d))"), ofs); break;
	case 0x9b: _stprintf(debug_dasm, _T("RES 3, E=(IY+(%d))"), ofs); break;
	case 0x9c: _stprintf(debug_dasm, _T("RES 3, H=(IY+(%d))"), ofs); break;
	case 0x9d: _stprintf(debug_dasm, _T("RES 3, L=(IY+(%d))"), ofs); break;
	case 0x9e: _stprintf(debug_dasm, _T("RES 3, (IY+(%d))"), ofs); break;
	case 0x9f: _stprintf(debug_dasm, _T("RES 3, A=(IY+(%d))"), ofs); break;
	case 0xa0: _stprintf(debug_dasm, _T("RES 4, B=(IY+(%d))"), ofs); break;
	case 0xa1: _stprintf(debug_dasm, _T("RES 4, C=(IY+(%d))"), ofs); break;
	case 0xa2: _stprintf(debug_dasm, _T("RES 4, D=(IY+(%d))"), ofs); break;
	case 0xa3: _stprintf(debug_dasm, _T("RES 4, E=(IY+(%d))"), ofs); break;
	case 0xa4: _stprintf(debug_dasm, _T("RES 4, H=(IY+(%d))"), ofs); break;
	case 0xa5: _stprintf(debug_dasm, _T("RES 4, L=(IY+(%d))"), ofs); break;
	case 0xa6: _stprintf(debug_dasm, _T("RES 4, (IY+(%d))"), ofs); break;
	case 0xa7: _stprintf(debug_dasm, _T("RES 4, A=(IY+(%d))"), ofs); break;
	case 0xa8: _stprintf(debug_dasm, _T("RES 5, B=(IY+(%d))"), ofs); break;
	case 0xa9: _stprintf(debug_dasm, _T("RES 5, C=(IY+(%d))"), ofs); break;
	case 0xaa: _stprintf(debug_dasm, _T("RES 5, D=(IY+(%d))"), ofs); break;
	case 0xab: _stprintf(debug_dasm, _T("RES 5, E=(IY+(%d))"), ofs); break;
	case 0xac: _stprintf(debug_dasm, _T("RES 5, H=(IY+(%d))"), ofs); break;
	case 0xad: _stprintf(debug_dasm, _T("RES 5, L=(IY+(%d))"), ofs); break;
	case 0xae: _stprintf(debug_dasm, _T("RES 5, (IY+(%d))"), ofs); break;
	case 0xaf: _stprintf(debug_dasm, _T("RES 5, A=(IY+(%d))"), ofs); break;
	case 0xb0: _stprintf(debug_dasm, _T("RES 6, B=(IY+(%d))"), ofs); break;
	case 0xb1: _stprintf(debug_dasm, _T("RES 6, C=(IY+(%d))"), ofs); break;
	case 0xb2: _stprintf(debug_dasm, _T("RES 6, D=(IY+(%d))"), ofs); break;
	case 0xb3: _stprintf(debug_dasm, _T("RES 6, E=(IY+(%d))"), ofs); break;
	case 0xb4: _stprintf(debug_dasm, _T("RES 6, H=(IY+(%d))"), ofs); break;
	case 0xb5: _stprintf(debug_dasm, _T("RES 6, L=(IY+(%d))"), ofs); break;
	case 0xb6: _stprintf(debug_dasm, _T("RES 6, (IY+(%d))"), ofs); break;
	case 0xb7: _stprintf(debug_dasm, _T("RES 6, A=(IY+(%d))"), ofs); break;
	case 0xb8: _stprintf(debug_dasm, _T("RES 7, B=(IY+(%d))"), ofs); break;
	case 0xb9: _stprintf(debug_dasm, _T("RES 7, C=(IY+(%d))"), ofs); break;
	case 0xba: _stprintf(debug_dasm, _T("RES 7, D=(IY+(%d))"), ofs); break;
	case 0xbb: _stprintf(debug_dasm, _T("RES 7, E=(IY+(%d))"), ofs); break;
	case 0xbc: _stprintf(debug_dasm, _T("RES 7, H=(IY+(%d))"), ofs); break;
	case 0xbd: _stprintf(debug_dasm, _T("RES 7, L=(IY+(%d))"), ofs); break;
	case 0xbe: _stprintf(debug_dasm, _T("RES 7, (IY+(%d))"), ofs); break;
	case 0xbf: _stprintf(debug_dasm, _T("RES 7, A=(IY+(%d))"), ofs); break;
	case 0xc0: _stprintf(debug_dasm, _T("SET 0, B=(IY+(%d))"), ofs); break;
	case 0xc1: _stprintf(debug_dasm, _T("SET 0, C=(IY+(%d))"), ofs); break;
	case 0xc2: _stprintf(debug_dasm, _T("SET 0, D=(IY+(%d))"), ofs); break;
	case 0xc3: _stprintf(debug_dasm, _T("SET 0, E=(IY+(%d))"), ofs); break;
	case 0xc4: _stprintf(debug_dasm, _T("SET 0, H=(IY+(%d))"), ofs); break;
	case 0xc5: _stprintf(debug_dasm, _T("SET 0, L=(IY+(%d))"), ofs); break;
	case 0xc6: _stprintf(debug_dasm, _T("SET 0, (IY+(%d))"), ofs); break;
	case 0xc7: _stprintf(debug_dasm, _T("SET 0, A=(IY+(%d))"), ofs); break;
	case 0xc8: _stprintf(debug_dasm, _T("SET 1, B=(IY+(%d))"), ofs); break;
	case 0xc9: _stprintf(debug_dasm, _T("SET 1, C=(IY+(%d))"), ofs); break;
	case 0xca: _stprintf(debug_dasm, _T("SET 1, D=(IY+(%d))"), ofs); break;
	case 0xcb: _stprintf(debug_dasm, _T("SET 1, E=(IY+(%d))"), ofs); break;
	case 0xcc: _stprintf(debug_dasm, _T("SET 1, H=(IY+(%d))"), ofs); break;
	case 0xcd: _stprintf(debug_dasm, _T("SET 1, L=(IY+(%d))"), ofs); break;
	case 0xce: _stprintf(debug_dasm, _T("SET 1, (IY+(%d))"), ofs); break;
	case 0xcf: _stprintf(debug_dasm, _T("SET 1, A=(IY+(%d))"), ofs); break;
	case 0xd0: _stprintf(debug_dasm, _T("SET 2, B=(IY+(%d))"), ofs); break;
	case 0xd1: _stprintf(debug_dasm, _T("SET 2, C=(IY+(%d))"), ofs); break;
	case 0xd2: _stprintf(debug_dasm, _T("SET 2, D=(IY+(%d))"), ofs); break;
	case 0xd3: _stprintf(debug_dasm, _T("SET 2, E=(IY+(%d))"), ofs); break;
	case 0xd4: _stprintf(debug_dasm, _T("SET 2, H=(IY+(%d))"), ofs); break;
	case 0xd5: _stprintf(debug_dasm, _T("SET 2, L=(IY+(%d))"), ofs); break;
	case 0xd6: _stprintf(debug_dasm, _T("SET 2, (IY+(%d))"), ofs); break;
	case 0xd7: _stprintf(debug_dasm, _T("SET 2, A=(IY+(%d))"), ofs); break;
	case 0xd8: _stprintf(debug_dasm, _T("SET 3, B=(IY+(%d))"), ofs); break;
	case 0xd9: _stprintf(debug_dasm, _T("SET 3, C=(IY+(%d))"), ofs); break;
	case 0xda: _stprintf(debug_dasm, _T("SET 3, D=(IY+(%d))"), ofs); break;
	case 0xdb: _stprintf(debug_dasm, _T("SET 3, E=(IY+(%d))"), ofs); break;
	case 0xdc: _stprintf(debug_dasm, _T("SET 3, H=(IY+(%d))"), ofs); break;
	case 0xdd: _stprintf(debug_dasm, _T("SET 3, L=(IY+(%d))"), ofs); break;
	case 0xde: _stprintf(debug_dasm, _T("SET 3, (IY+(%d))"), ofs); break;
	case 0xdf: _stprintf(debug_dasm, _T("SET 3, A=(IY+(%d))"), ofs); break;
	case 0xe0: _stprintf(debug_dasm, _T("SET 4, B=(IY+(%d))"), ofs); break;
	case 0xe1: _stprintf(debug_dasm, _T("SET 4, C=(IY+(%d))"), ofs); break;
	case 0xe2: _stprintf(debug_dasm, _T("SET 4, D=(IY+(%d))"), ofs); break;
	case 0xe3: _stprintf(debug_dasm, _T("SET 4, E=(IY+(%d))"), ofs); break;
	case 0xe4: _stprintf(debug_dasm, _T("SET 4, H=(IY+(%d))"), ofs); break;
	case 0xe5: _stprintf(debug_dasm, _T("SET 4, L=(IY+(%d))"), ofs); break;
	case 0xe6: _stprintf(debug_dasm, _T("SET 4, (IY+(%d))"), ofs); break;
	case 0xe7: _stprintf(debug_dasm, _T("SET 4, A=(IY+(%d))"), ofs); break;
	case 0xe8: _stprintf(debug_dasm, _T("SET 5, B=(IY+(%d))"), ofs); break;
	case 0xe9: _stprintf(debug_dasm, _T("SET 5, C=(IY+(%d))"), ofs); break;
	case 0xea: _stprintf(debug_dasm, _T("SET 5, D=(IY+(%d))"), ofs); break;
	case 0xeb: _stprintf(debug_dasm, _T("SET 5, E=(IY+(%d))"), ofs); break;
	case 0xec: _stprintf(debug_dasm, _T("SET 5, H=(IY+(%d))"), ofs); break;
	case 0xed: _stprintf(debug_dasm, _T("SET 5, L=(IY+(%d))"), ofs); break;
	case 0xee: _stprintf(debug_dasm, _T("SET 5, (IY+(%d))"), ofs); break;
	case 0xef: _stprintf(debug_dasm, _T("SET 5, A=(IY+(%d))"), ofs); break;
	case 0xf0: _stprintf(debug_dasm, _T("SET 6, B=(IY+(%d))"), ofs); break;
	case 0xf1: _stprintf(debug_dasm, _T("SET 6, C=(IY+(%d))"), ofs); break;
	case 0xf2: _stprintf(debug_dasm, _T("SET 6, D=(IY+(%d))"), ofs); break;
	case 0xf3: _stprintf(debug_dasm, _T("SET 6, E=(IY+(%d))"), ofs); break;
	case 0xf4: _stprintf(debug_dasm, _T("SET 6, H=(IY+(%d))"), ofs); break;
	case 0xf5: _stprintf(debug_dasm, _T("SET 6, L=(IY+(%d))"), ofs); break;
	case 0xf6: _stprintf(debug_dasm, _T("SET 6, (IY+(%d))"), ofs); break;
	case 0xf7: _stprintf(debug_dasm, _T("SET 6, A=(IY+(%d))"), ofs); break;
	case 0xf8: _stprintf(debug_dasm, _T("SET 7, B=(IY+(%d))"), ofs); break;
	case 0xf9: _stprintf(debug_dasm, _T("SET 7, C=(IY+(%d))"), ofs); break;
	case 0xfa: _stprintf(debug_dasm, _T("SET 7, D=(IY+(%d))"), ofs); break;
	case 0xfb: _stprintf(debug_dasm, _T("SET 7, E=(IY+(%d))"), ofs); break;
	case 0xfc: _stprintf(debug_dasm, _T("SET 7, H=(IY+(%d))"), ofs); break;
	case 0xfd: _stprintf(debug_dasm, _T("SET 7, L=(IY+(%d))"), ofs); break;
	case 0xfe: _stprintf(debug_dasm, _T("SET 7, (IY+(%d))"), ofs); break;
	case 0xff: _stprintf(debug_dasm, _T("SET 7, A=(IY+(%d))"), ofs); break;
	}
}
#endif
