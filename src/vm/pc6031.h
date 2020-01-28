//
// PC-6001/6601 disk I/O
// This file is based on a disk I/O program in C++
// by Mr. Yumitaro and translated into C for Cocoa iP6
// by Koichi NISHIDA 2006
//

/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Origin : tanam
	Date   : 2014.05.21-

	[ PC-6031 ]
*/

#ifndef _PC6031_H_
#define _PC6031_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class DISK;

class PC6031 : public DEVICE
{
private:
	DISK* disk[2];
	
	int cur_trk[2];
	int cur_sct[2];
	int cur_pos[2];
	bool access[2];
	
	int Seek88(int drvno, int trackno, int sectno);
	unsigned char Getc88(int drvno);
	int Putc88(int drvno, unsigned char dat);
	
	typedef struct {
		int ATN;				// attention
		int DAC;				// data accepted
		int RFD;				// ready for data
		int DAV;				// data valid
		int command;			// received command
		int step;				// status for waiting parameter
		int blk;				// block number
		int drv;				// drive number - 1
		int trk;				// track number
		int sct;				// sector number
		int size;				// byte number to process
		unsigned char retdat;	// return from port D0H
	} DISK60;
	
	DISK60 mdisk;
	unsigned char io_D1H;
	unsigned char io_D2H, old_D2H;
	unsigned char io_D3H;
	int DrvNum;
	
	unsigned char FddIn60();
	void FddOut60(unsigned char dat);
	unsigned char FddCntIn60(void);
	void FddCntOut60(unsigned char dat);
	
	void OutD1H_60(unsigned char data);
	void OutD2H_60(unsigned char data);
	void OutD3H_60(unsigned char data);
	
	unsigned char InD0H_60();
	unsigned char InD1H_60();
	unsigned char InD2H_60();
	unsigned char InD3H_60();
	
public:
	PC6031(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PC6031() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	uint32_t read_signal(int ch);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("Pseudo PC-6031");
	}
	
	// unique functions
	DISK* get_disk_handler(int drv)
	{
		return disk[drv];
	}
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	bool disk_ejected(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
};

#endif
