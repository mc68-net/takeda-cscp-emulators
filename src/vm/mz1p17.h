/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.12.24-

	[ MZ-1P17 ]
*/

#ifndef _MZ1P17_H_
#define _MZ1P17_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define MZ1P17_MODE_MZ1	0
#define MZ1P17_MODE_MZ2	1
#define MZ1P17_MODE_MZ3	2
#define MZ1P17_MODE_X1	3

// for correct super/sub script mode
//#define PIXEL_PER_INCH	720
// for correct 1/120 inch scroll
#define PIXEL_PER_INCH	360
// for correct 1/180 inch dots
//#define PIXEL_PER_INCH	180
#define DOT_PER_INCH	180
#define DOT_SCALE	(PIXEL_PER_INCH / DOT_PER_INCH)

class FIFO;

class MZ1P17 : public DEVICE
{
private:
	outputs_t outputs_busy;
	outputs_t outputs_ack;
	
	int value, busy_id, ack_id, wait_frames;
	bool strobe, res, busy, ack;
	
	bitmap_t bitmap_paper;
	bitmap_t bitmap_line[4];
	font_t font;
	int space_left, space_top;
	
	bool ank[256][16][8];
	bool gaiji[3][94][48][48];		// 0x78-0x7a,0x21-0x7e
	bool htab[1440 * DOT_SCALE];
	struct {
		int y;
		bool active;
	} vtab[14];
	
	FIFO *fifo;
	
	int lf_pitch;
	bool prev_esc_6;
	int margin_left, margin_right;
	int pitch_mode;
	int script_mode;
	bool kanji_mode, kanji_half, hiragana_mode;
	bool bold, underline, reverse, vertical;
	
	bool ank_double_x, ank_double_y;
	bool kanji_double_x, kanji_double_y;
	int kanji_pitch, kanji_half_pitch;
	
	int dest_line_x, dest_paper_y;
	int color_mode;
	bool double_y_printed;
	bool line_printed, paper_printed;
	int paper_index, written_length;
	_TCHAR base_path[_MAX_PATH];
	
	void set_busy(bool value);
	void set_ack(bool value);
	void process_mz1();
	void process_mz2();
	void process_mz3();
	void process_x1();
	void draw_char(uint16_t code);
	void scroll(int value);
	void finish();
	void finish_line();
	void finish_paper();
	
public:
	MZ1P17(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_busy);
		initialize_output_signals(&outputs_ack);
	}
	~MZ1P17() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_frame();
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("MZ-1P17");
	}
	
	// unique functions
	void set_context_busy(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_busy, device, id, mask);
	}
	void set_context_ack(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ack, device, id, mask);
	}
	int mode;
};

#endif

