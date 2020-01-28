/*
 * FM-7 Keyboard [joystick.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jun 16, 2015 : Initial
 *
 */

#ifndef _VM_FM7_JOYSTICK_H_
#define _VM_FM7_JOYSTICK_H_
#include "../device.h"

#include "fm7_common.h"
#include "../../fileio.h"

class JOYSTICK : public DEVICE {
 private:
	bool emulate_mouse[2];
	uint32_t joydata[2];
	const uint32_t *rawdata;
	const int32_t *mouse_state;
	int dx, dy;
	int lx, ly;
	uint32_t mouse_button;
	bool mouse_strobe;
	uint32_t mouse_data;
	int mouse_phase;
	int mouse_timeout_event;
	uint32_t port_a_val;
	uint32_t port_b_val;
	uint8_t lpmask;
	int lpt_type;
 protected:
	VM* p_vm;
	EMU* p_emu;
	//DEVICE *opn;
 private:
	uint32_t update_mouse(uint32_t mask);
	void   update_strobe(bool flag);
	uint32_t mouse_type;
 public:
	JOYSTICK(VM *parent_vm, EMU *parent_emu);
	~JOYSTICK();

	void initialize(void);
	void event_frame(void);
	
	uint32_t read_data8(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	
	void reset(void);
	void update_config();
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("FM7_JOYSTICK");
	}
	
	//void set_context_opn(DEVICE *p) {
	//	opn = p;
	//}
};

#endif