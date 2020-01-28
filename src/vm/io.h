/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ i/o bus ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifndef IO_ADDR_MAX
#define IO_ADDR_MAX 0x100
#endif
#define IO_ADDR_MASK (IO_ADDR_MAX - 1)

class IO : public DEVICE
{
private:
	// i/o map
	struct {
		DEVICE* dev;
		uint32_t addr;
		int wait;
		bool is_flipflop;
	} wr_table[IO_ADDR_MAX];
	
	struct {
		DEVICE* dev;
		uint32_t addr;
		int wait;
		bool value_registered;
		uint32_t value;
	} rd_table[IO_ADDR_MAX];
	
	void write_port8(uint32_t addr, uint32_t data, bool is_dma);
	uint32_t read_port8(uint32_t addr, bool is_dma);
	void write_port16(uint32_t addr, uint32_t data, bool is_dma);
	uint32_t read_port16(uint32_t addr, bool is_dma);
	void write_port32(uint32_t addr, uint32_t data, bool is_dma);
	uint32_t read_port32(uint32_t addr, bool is_dma);
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(wr_table, 0, sizeof(wr_table));
		memset(rd_table, 0, sizeof(rd_table));
		
		// vm->dummy must be generated first !
		for(int i = 0; i < IO_ADDR_MAX; i++) {
			wr_table[i].dev = rd_table[i].dev = vm->dummy;
			wr_table[i].addr = rd_table[i].addr = i;
		}
#ifdef _IO_DEBUG_LOG
		cpu_index = 0;
#endif
	}
	~IO() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_io16(uint32_t addr, uint32_t data);
	uint32_t read_io16(uint32_t addr);
	void write_io32(uint32_t addr, uint32_t data);
	uint32_t read_io32(uint32_t addr);
	void write_io8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io8w(uint32_t addr, int* wait);
	void write_io16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io16w(uint32_t addr, int* wait);
	void write_io32w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io32w(uint32_t addr, int* wait);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_dma_io16(uint32_t addr, uint32_t data);
	uint32_t read_dma_io16(uint32_t addr);
	void write_dma_io32(uint32_t addr, uint32_t data);
	uint32_t read_dma_io32(uint32_t addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("Standard I/O Bus");
	}
	
	// unique functions
	void set_iomap_single_r(uint32_t addr, DEVICE* device);
	void set_iomap_single_w(uint32_t addr, DEVICE* device);
	void set_iomap_single_rw(uint32_t addr, DEVICE* device);
	void set_iomap_alias_r(uint32_t addr, DEVICE* device, uint32_t alias);
	void set_iomap_alias_w(uint32_t addr, DEVICE* device, uint32_t alias);
	void set_iomap_alias_rw(uint32_t addr, DEVICE* device, uint32_t alias);
	void set_iomap_range_r(uint32_t s, uint32_t e, DEVICE* device);
	void set_iomap_range_w(uint32_t s, uint32_t e, DEVICE* device);
	void set_iomap_range_rw(uint32_t s, uint32_t e, DEVICE* device);
	
	void set_iovalue_single_r(uint32_t addr, uint32_t value);
	void set_iovalue_range_r(uint32_t s, uint32_t e, uint32_t value);
	void set_flipflop_single_rw(uint32_t addr, uint32_t value);
	void set_flipflop_range_rw(uint32_t s, uint32_t e, uint32_t value);
	
	void set_iowait_single_r(uint32_t addr, int wait);
	void set_iowait_single_w(uint32_t addr, int wait);
	void set_iowait_single_rw(uint32_t addr, int wait);
	void set_iowait_range_r(uint32_t s, uint32_t e, int wait);
	void set_iowait_range_w(uint32_t s, uint32_t e, int wait);
	void set_iowait_range_rw(uint32_t s, uint32_t e, int wait);
	
#ifdef _IO_DEBUG_LOG
	int cpu_index;
#endif
};

#endif
