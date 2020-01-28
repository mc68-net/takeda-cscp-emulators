/*
	CASIO FP-200 Emulator 'eFP-200'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2013.03.21-

	[ io ]
*/

#include "io.h"
#include "../datarec.h"
#include "../i8080.h"
#include "../../config.h"
#include "../../fileio.h"

// based on elisa font (http://hp.vector.co.jp/authors/VA002310/index.htm
static const uint8 elisa_font[2048] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x08, 0x04, 0x02, 0x7f, 0x02, 0x04, 0x08, 0x00, 0x08, 0x10, 0x20, 0x7f, 0x20, 0x10, 0x08, 0x00,
	0x08, 0x1c, 0x2a, 0x49, 0x08, 0x08, 0x08, 0x00, 0x08, 0x08, 0x08, 0x49, 0x2a, 0x1c, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08,
	0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x7f, 0x24, 0x24, 0xfe, 0x48, 0x48,
	0x08, 0x1c, 0x2a, 0x1c, 0x0a, 0x2a, 0x1c, 0x08, 0x61, 0x92, 0x94, 0x68, 0x16, 0x29, 0x49, 0x86,
	0x30, 0x48, 0x48, 0x32, 0x4a, 0x44, 0x3a, 0x00, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x04, 0x08, 0x08, 0x08, 0x08, 0x04, 0x03, 0xc0, 0x20, 0x10, 0x10, 0x10, 0x10, 0x20, 0xc0,
	0x08, 0x2a, 0x1c, 0x08, 0x1c, 0x2a, 0x08, 0x00, 0x08, 0x08, 0x08, 0x7f, 0x08, 0x08, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x20, 0x40, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00,
	0x3c, 0x42, 0x46, 0x5a, 0x62, 0x42, 0x3c, 0x00, 0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00,
	0x3c, 0x42, 0x02, 0x0c, 0x10, 0x20, 0x7e, 0x00, 0x3c, 0x42, 0x02, 0x3c, 0x02, 0x42, 0x3c, 0x00,
	0x04, 0x0c, 0x14, 0x24, 0x44, 0x7e, 0x04, 0x00, 0x7e, 0x40, 0x40, 0x7c, 0x02, 0x02, 0x7c, 0x00,
	0x3c, 0x42, 0x40, 0x7c, 0x42, 0x42, 0x3c, 0x00, 0x7e, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00,
	0x3c, 0x42, 0x42, 0x3c, 0x42, 0x42, 0x3c, 0x00, 0x3c, 0x42, 0x42, 0x3e, 0x02, 0x42, 0x3c, 0x00,
	0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10,
	0x03, 0x0c, 0x30, 0xc0, 0x30, 0x0c, 0x03, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00,
	0xc0, 0x30, 0x0c, 0x03, 0x0c, 0x30, 0xc0, 0x00, 0x1c, 0x22, 0x22, 0x04, 0x08, 0x08, 0x00, 0x08,
	0x1c, 0x22, 0x4d, 0x55, 0x47, 0x5f, 0x20, 0x1e, 0x18, 0x24, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x00,
	0x7c, 0x42, 0x42, 0x7c, 0x42, 0x42, 0x7c, 0x00, 0x1c, 0x22, 0x40, 0x40, 0x40, 0x22, 0x1c, 0x00,
	0x78, 0x24, 0x22, 0x22, 0x22, 0x24, 0x78, 0x00, 0x7e, 0x40, 0x40, 0x7c, 0x40, 0x40, 0x7e, 0x00,
	0x7e, 0x40, 0x40, 0x7c, 0x40, 0x40, 0x40, 0x00, 0x1c, 0x22, 0x40, 0x47, 0x42, 0x26, 0x1a, 0x00,
	0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00,
	0x0e, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00, 0x42, 0x44, 0x48, 0x50, 0x68, 0x44, 0x42, 0x00,
	0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7e, 0x00, 0x41, 0x63, 0x55, 0x49, 0x41, 0x41, 0x41, 0x00,
	0x42, 0x62, 0x52, 0x4a, 0x46, 0x42, 0x42, 0x00, 0x18, 0x24, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00,
	0x7c, 0x42, 0x42, 0x7c, 0x40, 0x40, 0x40, 0x00, 0x18, 0x24, 0x42, 0x42, 0x4a, 0x24, 0x1a, 0x00,
	0x7c, 0x42, 0x42, 0x7c, 0x48, 0x44, 0x42, 0x00, 0x3c, 0x42, 0x40, 0x3c, 0x02, 0x42, 0x3c, 0x00,
	0x7f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00,
	0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x00, 0x41, 0x49, 0x49, 0x55, 0x55, 0x22, 0x22, 0x00,
	0x41, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41, 0x00, 0x41, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08, 0x00,
	0x7e, 0x02, 0x04, 0x08, 0x10, 0x20, 0x7e, 0x00, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07,
	0x41, 0x22, 0x14, 0x7f, 0x08, 0x7f, 0x08, 0x08, 0xe0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe0,
	0x08, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x02, 0x1e, 0x22, 0x1e, 0x00,
	0x20, 0x20, 0x20, 0x3c, 0x22, 0x22, 0x3c, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x20, 0x22, 0x1c, 0x00,
	0x02, 0x02, 0x02, 0x1e, 0x22, 0x22, 0x1e, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x3e, 0x20, 0x1e, 0x00,
	0x0c, 0x12, 0x10, 0x3c, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x1e, 0x02, 0x3c,
	0x20, 0x20, 0x3c, 0x22, 0x22, 0x22, 0x22, 0x00, 0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x1c, 0x00,
	0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x48, 0x30, 0x20, 0x20, 0x24, 0x28, 0x30, 0x28, 0x24, 0x00,
	0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x3c, 0x2a, 0x2a, 0x2a, 0x2a, 0x00,
	0x00, 0x00, 0x3c, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x1c, 0x00,
	0x00, 0x00, 0x3c, 0x22, 0x22, 0x3c, 0x20, 0x20, 0x00, 0x00, 0x1e, 0x22, 0x22, 0x1e, 0x02, 0x02,
	0x00, 0x00, 0x2e, 0x30, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x1e, 0x20, 0x1c, 0x02, 0x3c, 0x00,
	0x00, 0x10, 0x3c, 0x10, 0x10, 0x10, 0x0c, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x1e, 0x00,
	0x00, 0x00, 0x22, 0x22, 0x14, 0x14, 0x08, 0x00, 0x00, 0x00, 0x22, 0x2a, 0x2a, 0x14, 0x14, 0x00,
	0x00, 0x00, 0x22, 0x14, 0x08, 0x14, 0x22, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x1e, 0x02, 0x3c,
	0x00, 0x00, 0x3e, 0x04, 0x08, 0x10, 0x3e, 0x00, 0x03, 0x04, 0x04, 0x08, 0x04, 0x04, 0x04, 0x03,
	0x08, 0x08, 0x08, 0x00, 0x08, 0x08, 0x08, 0x00, 0xc0, 0x20, 0x20, 0x10, 0x20, 0x20, 0x20, 0xc0,
	0x00, 0x00, 0x30, 0x49, 0x06, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x10, 0x08, 0x08, 0x00, 0x08,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
	0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
	0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0x08, 0x08, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08,
	0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x0f, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0xf8, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x07, 0x08, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0xc0, 0x20, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0xc0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xa0, 0x40,
	0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xf0,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
	0x00, 0x7e, 0x02, 0x7e, 0x02, 0x04, 0x08, 0x30, 0x00, 0x00, 0x00, 0x3e, 0x0a, 0x0c, 0x08, 0x10,
	0x00, 0x00, 0x00, 0x04, 0x08, 0x18, 0x28, 0x08, 0x00, 0x00, 0x00, 0x08, 0x3e, 0x22, 0x02, 0x0c,
	0x00, 0x00, 0x00, 0x00, 0x1c, 0x08, 0x08, 0x3e, 0x00, 0x00, 0x00, 0x04, 0x3e, 0x0c, 0x14, 0x24,
	0x00, 0x00, 0x00, 0x10, 0x3e, 0x12, 0x14, 0x10, 0x00, 0x00, 0x00, 0x00, 0x38, 0x08, 0x08, 0x7c,
	0x00, 0x00, 0x00, 0x3c, 0x04, 0x3c, 0x04, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x52, 0x52, 0x04, 0x18,
	0x00, 0x00, 0x00, 0x40, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x01, 0x0a, 0x0c, 0x08, 0x08, 0x10,
	0x00, 0x02, 0x04, 0x08, 0x18, 0x28, 0x48, 0x08, 0x08, 0x08, 0x7e, 0x42, 0x42, 0x02, 0x04, 0x18,
	0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7f, 0x04, 0x04, 0x7f, 0x04, 0x0c, 0x14, 0x24, 0x44,
	0x10, 0x10, 0x7e, 0x12, 0x12, 0x22, 0x22, 0x44, 0x10, 0x10, 0x7f, 0x08, 0x7f, 0x08, 0x04, 0x04,
	0x20, 0x3e, 0x22, 0x42, 0x02, 0x04, 0x04, 0x18, 0x20, 0x20, 0x3f, 0x24, 0x44, 0x04, 0x08, 0x30,
	0x00, 0x00, 0x7e, 0x02, 0x02, 0x02, 0x02, 0x7e, 0x00, 0x24, 0xff, 0x24, 0x24, 0x04, 0x04, 0x18,
	0x00, 0x40, 0x20, 0x42, 0x22, 0x04, 0x08, 0x70, 0x00, 0x7e, 0x02, 0x04, 0x08, 0x14, 0x22, 0x41,
	0x20, 0x20, 0x7e, 0x22, 0x24, 0x28, 0x20, 0x1e, 0x00, 0x41, 0x41, 0x21, 0x02, 0x02, 0x04, 0x18,
	0x10, 0x1e, 0x12, 0x32, 0x4e, 0x04, 0x08, 0x70, 0x06, 0x38, 0x08, 0x7f, 0x08, 0x08, 0x08, 0x30,
	0x00, 0x52, 0x52, 0x02, 0x04, 0x04, 0x08, 0x30, 0x3e, 0x00, 0x00, 0x7f, 0x08, 0x08, 0x08, 0x30,
	0x20, 0x20, 0x20, 0x38, 0x24, 0x22, 0x20, 0x20, 0x04, 0x04, 0x7f, 0x04, 0x04, 0x04, 0x08, 0x30,
	0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x7e, 0x02, 0x22, 0x14, 0x0c, 0x12, 0x60,
	0x08, 0x08, 0x7f, 0x01, 0x06, 0x1a, 0x69, 0x08, 0x00, 0x02, 0x02, 0x02, 0x04, 0x04, 0x08, 0x30,
	0x00, 0x00, 0x24, 0x24, 0x22, 0x42, 0x81, 0x00, 0x00, 0x40, 0x40, 0x7e, 0x40, 0x40, 0x40, 0x3e,
	0x00, 0x00, 0x7e, 0x02, 0x02, 0x04, 0x08, 0x30, 0x00, 0x00, 0x18, 0x24, 0x42, 0x01, 0x00, 0x00,
	0x08, 0x08, 0x7f, 0x08, 0x08, 0x2a, 0x49, 0x08, 0x00, 0x7e, 0x02, 0x02, 0x24, 0x18, 0x08, 0x04,
	0x00, 0x18, 0x06, 0x18, 0x06, 0x00, 0x30, 0x0e, 0x00, 0x08, 0x08, 0x10, 0x10, 0x24, 0x42, 0x7d,
	0x00, 0x01, 0x01, 0x12, 0x0a, 0x04, 0x0a, 0x30, 0x00, 0x7e, 0x10, 0x10, 0x7f, 0x10, 0x10, 0x1e,
	0x10, 0x10, 0xff, 0x11, 0x12, 0x14, 0x10, 0x10, 0x00, 0x00, 0x3c, 0x04, 0x04, 0x04, 0x04, 0x7e,
	0x00, 0x7e, 0x02, 0x02, 0x7e, 0x02, 0x02, 0x7e, 0x00, 0x3c, 0x00, 0x7e, 0x02, 0x02, 0x04, 0x38,
	0x00, 0x22, 0x22, 0x22, 0x22, 0x02, 0x04, 0x18, 0x00, 0x28, 0x28, 0x28, 0x28, 0x29, 0x2a, 0x4c,
	0x00, 0x20, 0x20, 0x20, 0x22, 0x22, 0x24, 0x38, 0x00, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7e,
	0x00, 0x7e, 0x42, 0x42, 0x02, 0x04, 0x08, 0x30, 0x00, 0x40, 0x20, 0x02, 0x02, 0x04, 0x08, 0x70,
	0xa0, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xa0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x0f, 0x08, 0x08,
	0x08, 0x08, 0xff, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 0xf8, 0x08, 0x08,
	0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
	0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
	0x08, 0x1c, 0x3e, 0x7f, 0x7f, 0x1c, 0x3e, 0x00, 0x36, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x08, 0x00,
	0x08, 0x1c, 0x3e, 0x7f, 0x3e, 0x1c, 0x08, 0x00, 0x1c, 0x1c, 0x6b, 0x7f, 0x6b, 0x08, 0x1c, 0x00,
	0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x00, 0x1c, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1c, 0x00,
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
	0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81, 0x7f, 0x49, 0x49, 0x49, 0x7f, 0x41, 0x41, 0x43,
	0x20, 0x3e, 0x48, 0xbe, 0x28, 0xff, 0x08, 0x08, 0x3f, 0x21, 0x3f, 0x21, 0x3f, 0x21, 0x41, 0x83,
	0x7e, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x7e, 0x04, 0xee, 0xa4, 0xff, 0xa2, 0xbf, 0xe2, 0x0a,
	0x18, 0x04, 0x42, 0xbd, 0x14, 0x14, 0x24, 0x4c, 0x24, 0xc4, 0x4e, 0xed, 0x55, 0xe6, 0xc4, 0x58,
	0x7f, 0x00, 0x7f, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x7f, 0x08, 0x7f, 0x49, 0x49, 0x4b, 0x08,
	0xff, 0x80, 0xa4, 0x94, 0x88, 0x94, 0xe0, 0xff, 0xf8, 0xaf, 0xfa, 0xaa, 0xaa, 0xfa, 0x02, 0x06,
	0x22, 0xf2, 0x2f, 0x72, 0x6a, 0xa2, 0x22, 0x26, 0x08, 0x08, 0x08, 0x08, 0x08, 0x14, 0x22, 0xc1,
	0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0x00, 0x00, 0x10, 0x08, 0x04, 0x04, 0x00, 0x00,
};

static const uint8 key_table[10][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x37, 0x55, 0x4a, 0x4d},
	{0x00, 0x00, 0x00, 0x00, 0x38, 0x49, 0x4b, 0xbf},
	{0x00, 0x00, 0x00, 0x00, 0x39, 0x4f, 0x4c, 0xbe},
	{0x00, 0x00, 0x00, 0x00, 0x30, 0x50, 0xbc, 0xbb},
	{0x15, 0x2e, 0x24, 0x70, 0x31, 0x51, 0x41, 0x5a},
	{0x13, 0xdc, 0x27, 0x71, 0x32, 0x57, 0x53, 0x58},
	{0x20, 0xde, 0x25, 0x72, 0x33, 0x45, 0x44, 0x43},
	{0xc0, 0xbd, 0x28, 0x73, 0x34, 0x52, 0x46, 0x56},
	{0xdb, 0xba, 0x26, 0x74, 0x35, 0x54, 0x47, 0x42},
	{0x0d, 0xdd, 0x14, 0xe2, 0x36, 0x59, 0x48, 0x4e}
};

void IO::initialize()
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	} else {
		memcpy(font, elisa_font, sizeof(font));
	}
	delete fio;
	
	sod = false;
	key_stat = emu->key_buffer();
	
	cmt_play = cmt_rec = false;
	cmt_fio = NULL;
}

void IO::release()
{
	if(cmt_play || cmt_rec) {
		close_datarec();
	}
}

void IO::reset()
{
	mode_basic = (config.boot_mode == 0);
	
	memset(lcd, 0, sizeof(lcd));
	lcd[0].cursor = lcd[1].cursor = -1;
	lcd_status = lcd_addr = 0;
	lcd_text = true;
	
	if(cmt_play || cmt_rec) {
		close_datarec();
	}
	cmt_selected = false;
	
	key_column = 0;
	update_sid();
}

void IO::write_io8(uint32 addr, uint32 data)
{
	if(sod) {
		switch(addr & 0xff) {
		case 0x01:
		case 0x02:
			if(lcd_status == 0xb0) {
				if(data == 0) {
					lcd[ addr & 1].cursor = lcd_addr & 0x3ff;
					lcd[~addr & 1].cursor = -1;
				} else if(data == 0x40) {
					lcd_text = false;
				} else if(data == 0x50) {
					lcd[addr & 1].offset = lcd_addr & 0x3ff;
				} else if(data == 0x60) {
					lcd_text = true;
				}
			} else {
				lcd[addr & 1].ram[lcd_addr & 0x3ff].data = data;
				// 0x38f means the first raster of this character
				lcd[addr & 1].ram[lcd_addr & 0x38f].is_text = lcd_text;
			}
			break;
		case 0x08:
			if((lcd_status = data & 0xf0) == 0xb0) {
				lcd_addr = (lcd_addr & 0xff) | (data << 8);
			}
			break;
		case 0x09:
			lcd_addr = (lcd_addr & 0xff00) | data;
			break;
		case 0x10:
			cmt_selected = ((data & 2) == 0);
			break;
		case 0x21:
			key_column = data & 0x0f;
			update_sid();
			break;
		case 0x40:
			if(cmt_selected && cmt_mode == 1) {
#if 0
				if(cmt_count == 0 && !(data & 1)) {
					cmt_count = 1;
					cmt_data = 0;
				} else if(cmt_count > 0) {
					if(cmt_count <= 8 && (data & 1) != 0) {
						cmt_data |= 1 << (cmt_count - 1);
					}
					if(++cmt_count == 8+1+2) {
						if(cmt_rec) {
							cmt_fio->Fputc(cmt_data);
						}
						cmt_count = 0;
					}
				}
#else
				if(cmt_rec) {
					cmt_fio->Fputc(data & 1);
				}
#endif
			}
			break;
		case 0x43:
			cmt_mode = data & 7;
			cmt_count = 0;
			break;
		}
	} else {
		switch(addr & 0xf0) {
		case 0x10:
			d_rtc->write_io8(addr, data);
			break;
		}
	}
}

uint32 IO::read_io8(uint32 addr)
{
	uint32 value = 0;
	
	if(sod) {
		switch(addr & 0xff) {
		case 0x01:
		case 0x02:
			return lcd[addr & 1].ram[lcd_addr & 0x3ff].data;
		case 0x08:
			return ((lcd_addr >> 8) & 0x0f) | lcd_status;
		case 0x09:
			return lcd_addr & 0xff;
		case 0x10:
			return 0;
		case 0x20:
			if(key_column < 10) {
				for(int i = 0; i < 8; i++) {
					if(key_stat[key_table[key_column][i]]) {
						value |= (1 << i);
					}
				}
			}
			return value;
		case 0x40:
			if(cmt_selected && cmt_mode == 3 && cmt_play) {
#if 0
				if(cmt_count == 0) {
					if((cmt_data = cmt_fio->Fgetc()) == EOF) {
						close_datarec();
						return 0;
					}
				}
				int bit = cmt_data & (1 << cmt_count);
				if(++cmt_count == 8) {
					cmt_count = 0;
				}
				return bit ? 8 : 0;
#else
				if((cmt_data = cmt_fio->Fgetc()) == EOF) {
					close_datarec();
					return 0;
				}
				return (cmt_data & 1) << 3;
#endif
			}
			return 0;
		case 0x41:
			if(cmt_selected && ((cmt_mode == 3 && cmt_play) || (cmt_mode == 1 && cmt_rec))) {
				return 0x10;
			}
			return 0;
		}
	} else {
		switch(addr & 0xf0) {
		case 0x10:
			return d_rtc->read_io8(addr);
		case 0x40:
			return 0; // no floppy drives
		}
	}
	return 0xff;
}

void IO::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_IO_SOD) {
		sod = ((data & mask) == 0);
	}
}

void IO::key_down(int code)
{
	if(code >= 0x10 && code <= 0x13) {
		update_sid();
	} else {
		d_cpu->write_signal(SIG_I8085_RST7, 1, 1);
	}
}

void IO::key_up()
{
	update_sid();
}

void IO::update_sid()
{
	switch(key_column) {
	case 5:
		d_cpu->write_signal(SIG_I8085_SID, mode_basic ? 1 : 0, 1);
		break;
	case 6:
		d_cpu->write_signal(SIG_I8085_SID, key_stat[0x10] ? 0 : 1, 1);	// shift
		break;
	case 7:
		d_cpu->write_signal(SIG_I8085_SID, key_stat[0x13] ? 0 : 1, 1);	// break
		break;
	case 8:
		d_cpu->write_signal(SIG_I8085_SID, key_stat[0x12] ? 0 : 1, 1);	// graph
		break;
	case 9:
		d_cpu->write_signal(SIG_I8085_SID, key_stat[0x11] ? 0 : 1, 1);	// ctrl
		break;
	default:
		d_cpu->write_signal(SIG_I8085_SID, 1, 1);
		break;
	}
}

void IO::play_datarec(_TCHAR* file_path)
{
	if(cmt_play || cmt_rec) {
		close_datarec();
	}
	cmt_fio = new FILEIO();
	if(cmt_fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		cmt_play = true;
	} else {
		delete cmt_fio;
		cmt_fio = NULL;
	}
}

void IO::rec_datarec(_TCHAR* file_path)
{
	if(cmt_play || cmt_rec) {
		close_datarec();
	}
	cmt_fio = new FILEIO();
	if(cmt_fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		cmt_rec = true;
	} else {
		delete cmt_fio;
		cmt_fio = NULL;
	}
}

void IO::close_datarec()
{
	if(cmt_fio != NULL) {
		cmt_fio->Fclose();
		delete cmt_fio;
		cmt_fio = NULL;
	}
	cmt_play = cmt_rec = false;
}

void IO::draw_screen()
{
	// render screen
	for(int y = 0; y < 8; y++) {
		for(int x = 0; x < 20; x++) {
			int addr = (y * 0x80 + (x % 10) + lcd[x < 10].offset) & 0x3ff;
			if(lcd[x < 10].ram[addr].is_text) {
				uint8 code = lcd[x < 10].ram[addr].data;
				for(int l = 0; l < 8; l++) {
					uint8 pat = (lcd[x < 10].cursor == addr) ? 0xff : font[(code << 3) + l];
					addr += 0x10;
					uint8* d = &screen[y * 8 + l][x * 8];
					
					d[0] = pat & 0x80;
					d[1] = pat & 0x40;
					d[2] = pat & 0x20;
					d[3] = pat & 0x10;
					d[4] = pat & 0x08;
					d[5] = pat & 0x04;
					d[6] = pat & 0x02;
					d[7] = pat & 0x01;
				}
			} else {
				for(int l = 0; l < 8; l++) {
					uint8 pat = lcd[x < 10].ram[addr].data;
					addr += 0x10;
					uint8* d = &screen[y * 8 + l][x * 8];
					
					d[0] = pat & 0x01;
					d[1] = pat & 0x02;
					d[2] = pat & 0x04;
					d[3] = pat & 0x08;
					d[4] = pat & 0x10;
					d[5] = pat & 0x20;
					d[6] = pat & 0x40;
					d[7] = pat & 0x80;
				}
			}
		}
	}
	
	// copy to real screen
	scrntype cd = RGB_COLOR(48, 56, 16);
	scrntype cb = RGB_COLOR(160, 168, 160);
	for(int y = 0; y < 64; y++) {
		scrntype* dst = emu->screen_buffer(y);
		uint8* src = screen[y];
		
		for(int x = 0; x < 160; x++) {
			dst[x] = src[x] ? cd : cb;
		}
	}
}

