/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date  : 2009.06.08-

	[ i386/i486/Pentium/MediaGX ]
*/

#ifndef _I386_H_ 
#define _I386_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I386_A20	1

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I386 : public DEVICE
{
private:
	DEVICE *d_mem, *d_io, *d_pic;
#ifdef I386_PSEUDO_BIOS
	DEVICE *d_bios;
#endif
#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
#endif
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	void *opaque;
	
public:
	I386(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef I386_PSEUDO_BIOS
		d_bios = NULL;
#endif
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
	}
	~I386() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int cycles);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void set_intr_line(bool line, bool pending, uint32_t bit);
	void set_extra_clock(int cycles);
	int get_extra_clock();
	uint32_t get_pc();
	uint32_t get_next_pc();
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffffffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffffffff;
	}
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t read_debug_data16(uint32_t addr);
	void write_debug_data32(uint32_t addr, uint32_t data);
	uint32_t read_debug_data32(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	void write_debug_io16(uint32_t addr, uint32_t data);
	uint32_t read_debug_io32(uint32_t addr);
	void write_debug_io32(uint32_t addr, uint32_t data);
	uint32_t read_debug_io16(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	void get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("80386");
	}
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
#ifdef I386_PSEUDO_BIOS
	void set_context_bios(DEVICE* device)
	{
		d_bios = device;
	}
#endif
#ifdef SINGLE_MODE_DMA
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
#endif
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
	void set_address_mask(uint32_t mask);
	uint32_t get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
};

#endif