/*
	Skelton for retropc emulator

	Origin : MESS 0.152
	Author : Takeda.Toshiya
	Date   : 2016.03.17-

	[ uPD7810 ]
*/

#ifndef _UPD7810_H_ 
#define _UPD7810_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_UPD7810_INTF1	0
#define SIG_UPD7810_INTF2	1
#define SIG_UPD7810_INTF0	2
#define SIG_UPD7810_INTFE1	3
#define SIG_UPD7810_NMI		4
#define SIG_UPD7810_RXD		5

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class UPD7810 : public DEVICE
{
private:
	outputs_t outputs_txd;
	outputs_t outputs_rxd;
	outputs_t outputs_to;
	
	DEVICE *d_mem, *d_io;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	void *opaque;
	int icount;
	bool busreq, rxd_status;
	
public:
	UPD7810(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_txd);
		initialize_output_signals(&outputs_rxd);
		initialize_output_signals(&outputs_to);
	}
	~UPD7810() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	uint32_t get_pc();
	uint32_t get_next_pc();
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffff;
	}
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	void get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("uPD7810");
	}
	
	// unique function
	void set_context_txd(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_txd, device, id, mask);
	}
	void set_context_rxd(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rxd, device, id, mask);
	}
	void set_context_to(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_to, device, id, mask);
	}
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif
