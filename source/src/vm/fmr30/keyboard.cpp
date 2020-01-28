/*
	FUJITSU FMR-30 Emulator 'eFMR-30'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8251.h"

/*
	ひらがな/ローマ字	ひらがな
	半角/全角		半角/全角
	変換			変換
	無変換			無変換
	かな/漢字
	カタカナ
	前行			PgUp
	次行			PgDn
	実行			F12
	取消			F11
	COPY
*/

static const int key_table[256] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x10,0x00,0x00,0x00,0x1D,0x00,0x00,
	0x53,0x52,0x00,0x7C,0x55,0x52,0x00,0x00,0x00,0x71,0x00,0x01,0x58,0x57,0x00,0x00,
	0x35,0x6E,0x70,0x00,0x4E,0x4F,0x4D,0x51,0x50,0x00,0x00,0x00,0x00,0x48,0x4B,0x00,
	0x0B,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x1E,0x2E,0x2C,0x20,0x13,0x21,0x22,0x23,0x18,0x24,0x25,0x26,0x30,0x2F,0x19,
	0x1A,0x11,0x14,0x1F,0x15,0x17,0x2D,0x12,0x2B,0x16,0x2A,0x00,0x00,0x00,0x00,0x00,
	0x46,0x42,0x43,0x44,0x3E,0x3F,0x40,0x3A,0x3B,0x3C,0x36,0x38,0x00,0x39,0x47,0x37,
	0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x72,0x73,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x7D,0x6B,0x6C,0x6D,0x57,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x27,0x31,0x0C,0x32,0x33,
	0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x0E,0x29,0x0D,0x00,
	0x00,0x00,0x34,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void KEYBOARD::reset()
{
	_memset(table, 0, sizeof(table));
}

void KEYBOARD::key_down(int code)
{
//	if(!table[code]) {
		table[code] = 1;
		if(code = key_table[code]) {
			// $11:CTRL, $10:SHIFT
			d_sio->write_signal(SIG_I8251_RECV, 0xa0 | (table[0x11] ? 8 : 0) | (table[0x10] ? 4 : 0), 0xff);
			d_sio->write_signal(SIG_I8251_RECV, code, 0xff);
		}
//	}
}

void KEYBOARD::key_up(int code)
{
//	if(table[code]) {
		table[code] = 0;
		if(code = key_table[code]) {
			d_sio->write_signal(SIG_I8251_RECV, 0xb0, 0xff);
			d_sio->write_signal(SIG_I8251_RECV, code, 0xff);
		}
//	}
}

