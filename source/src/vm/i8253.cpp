/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.06.01-

	[ i8253/i8254 ]
*/

#include "i8253.h"

void I8253::initialize()
{
	for(int ch = 0; ch < 3; ch++) {
		counter[ch].prev_out = true;
		counter[ch].prev_in = false;
		counter[ch].gate = true;
		counter[ch].count = 0x10000;
		counter[ch].count_reg = 0;
		counter[ch].ctrl_reg = 0x34;
		counter[ch].mode = 3;
		counter[ch].count_latched = false;
		counter[ch].low_read = counter[ch].high_read = false;
		counter[ch].low_write = counter[ch].high_write = false;
		counter[ch].delay = false;
		counter[ch].start = false;
#ifdef HAS_I8254
		// 8254 read-back command
		counter[ch].null_count = true;
		counter[ch].status_latched = false;
#endif
		counter[ch].regist_id = -1;
	}
}

#define COUNT_VALUE(n) ((counter[n].count_reg == 0) ? 0x10000 : (counter[n].mode == 3 && counter[n].count_reg == 1) ? 0x10001 : counter[n].count_reg)

void I8253::write_io8(uint32 addr, uint32 data)
{
	int ch = addr & 3;
	
	switch(addr & 3)
	{
	case 0:
	case 1:
	case 2:
		// write count register
		if(!counter[ch].low_write && !counter[ch].high_write) {
			if(counter[ch].ctrl_reg & 0x10)
				counter[ch].low_write = true;
			if(counter[ch].ctrl_reg & 0x20)
				counter[ch].high_write = true;
		}
		if(counter[ch].low_write) {
			counter[ch].count_reg = data;
			counter[ch].low_write = false;
		}
		else if(counter[ch].high_write) {
			if((counter[ch].ctrl_reg & 0x30) == 0x20)
				counter[ch].count_reg = data << 8;
			else
				counter[ch].count_reg |= data << 8;
			counter[ch].high_write = false;
		}
#ifdef HAS_I8254
		counter[ch].null_count = true;
#endif
		// set signal
		if(counter[ch].mode == 0)
			set_signal(ch, false);
		else
			set_signal(ch, true);
		// start count
		if(counter[ch].mode == 0 || counter[ch].mode == 4) {
			// restart with new count
			stop_count(ch);
			counter[ch].delay = true;
			start_count(ch);
		}
		else if(counter[ch].mode == 2 || counter[ch].mode == 3) {
			// start with new counter after the current count is finished
			if(!counter[ch].start) {
				counter[ch].delay = true;
				start_count(ch);
			}
		}
		break;
		
	case 3: // ctrl reg
		if((data & 0xc0) == 0xc0) {
#ifdef HAS_I8254
			// i8254 read-back command
			for(ch = 0; ch < 3; ch++) {
				uint8 bit = 2 << ch;
				if(!(data & 0x10) && !counter[ch].status_latched) {
					counter[ch].status = counter[ch].ctrl_reg & 0x3f;
					if(counter[ch].prev_out)
						counter[ch].status |= 0x80;
					if(counter[ch].null_count)
						counter[ch].status |= 0x40;
					counter[ch].status_latched = true;
				}
				if(!(data & 0x20) && !counter[ch].count_latched)
					latch_count(ch);
			}
#endif
			break;
		}
		ch = (data >> 6) & 3;
		
		if(data & 0x30) {
			static int modes[8] = {0, 1, 2, 3, 4, 5, 2, 3};
//			int prev = counter[ch].mode;
			counter[ch].mode = modes[(data >> 1) & 7];
			counter[ch].count_latched = false;
			counter[ch].low_read = counter[ch].high_read = false;
			counter[ch].low_write = counter[ch].high_write = false;
			counter[ch].ctrl_reg = data;
			// set signal
			if(counter[ch].mode == 0)
				set_signal(ch, false);
			else
				set_signal(ch, true);
			// stop count
//			if(counter[ch].mode != prev || counter[ch].mode == 0 || counter[ch].mode == 4) {
				stop_count(ch);
				counter[ch].count_reg = 0;
//			}
#ifdef HAS_I8254
			counter[ch].null_count = true;
#endif
		}
		else if(!counter[ch].count_latched)
			latch_count(ch);
		break;
	}
}

uint32 I8253::read_io8(uint32 addr)
{
	int ch = addr & 3;
	
	switch(ch)
	{
	case 0:
	case 1:
	case 2:
#ifdef HAS_I8254
		if(counter[ch].status_latched) {
			counter[ch].status_latched = false;
			return counter[ch].status;
		}
#endif
		// if not latched, through current count
		if(!counter[ch].count_latched) {
			if(!counter[ch].low_read && !counter[ch].high_read)
				latch_count(ch);
			counter[ch].count_latched = false;
		}
		// return latched count
		if(counter[ch].low_read) {
			counter[ch].low_read = false;
			if(!counter[ch].high_read)
				counter[ch].count_latched = false;
			return counter[ch].latch & 0xff;
		}
		else if(counter[ch].high_read) {
			counter[ch].high_read = false;
			counter[ch].count_latched = false;
			return (counter[ch].latch >> 8) & 0xff;
		}
	}
	return 0xff;
}

void I8253::event_callback(int event_id, int err)
{
	int ch = event_id;
	counter[ch].regist_id = -1;
	input_clock(ch, counter[ch].input_clk);
	
	// regist next event
	if(counter[ch].freq && counter[ch].start) {
		counter[ch].input_clk = counter[ch].delay ? 1 : get_next_count(ch);
		counter[ch].period = CPU_CLOCKS / counter[ch].freq * counter[ch].input_clk + err;
		counter[ch].prev_clk = vm->current_clock() + err;
		vm->regist_event_by_clock(this, ch, counter[ch].period, false, &counter[ch].regist_id);
	}
}

void I8253::write_signal(int id, uint32 data, uint32 mask)
{
	bool next = ((data & mask) != 0);
	
	switch(id)
	{
	case SIG_I8253_CLOCK_0:
		if(counter[0].prev_in && !next)
			input_clock(0, 1);
		counter[0].prev_in = next;
		break;
	case SIG_I8253_CLOCK_1:
		if(counter[1].prev_in && !next)
			input_clock(1, 1);
		counter[1].prev_in = next;
		break;
	case SIG_I8253_CLOCK_2:
		if(counter[2].prev_in && !next)
			input_clock(2, 1);
		counter[2].prev_in = next;
		break;
	case SIG_I8253_GATE_0:
		input_gate(0, next);
		break;
	case SIG_I8253_GATE_1:
		input_gate(1, next);
		break;
	case SIG_I8253_GATE_2:
		input_gate(2, next);
		break;
	}
}

void I8253::input_clock(int ch, int clock)
{
	if(!(counter[ch].start && clock))
		return;
	if(counter[ch].delay) {
		clock -= 1;
		counter[ch].delay = false;
		counter[ch].count = COUNT_VALUE(ch);
#ifdef HAS_I8254
		counter[ch].null_count = false;
#endif
	}
	
	// update counter
	counter[ch].count -= clock;
	int32 tmp = COUNT_VALUE(ch);
loop:
	if(counter[ch].mode == 3) {
		int32 half = tmp >> 1;
		set_signal(ch, (counter[ch].count > half) ? true : false);
	}
	else {
		if(counter[ch].count <= 1) {
			if(counter[ch].mode == 2 || counter[ch].mode == 4 || counter[ch].mode == 5)
				set_signal(ch, false);
		}
		if(counter[ch].count <= 0)
			set_signal(ch, true);
	}
	if(counter[ch].count <= 0) {
		if(counter[ch].mode == 0 || counter[ch].mode == 2 || counter[ch].mode == 3) {
			counter[ch].count += tmp;
#ifdef HAS_I8254
			counter[ch].null_count = false;
#endif
			goto loop;
		}
		else {
			counter[ch].start = false;
			counter[ch].count = 0x10000;
		}
	}
}

void I8253::input_gate(int ch, bool signal)
{
	bool prev = counter[ch].gate;
	counter[ch].gate = signal;
	
	if(prev && !signal) {
		// stop count
		if(!(counter[ch].mode == 1 || counter[ch].mode == 5))
			stop_count(ch);
		// set output signal
		if(counter[ch].mode == 2 || counter[ch].mode == 3)
			set_signal(ch, true);
	}
	else if(!prev && signal) {
		// restart count
		stop_count(ch);
		if(!(counter[ch].mode == 0 || counter[ch].mode == 4))
			counter[ch].delay = true;
		start_count(ch);
		// set output signal
		if(counter[ch].mode == 1)
			set_signal(ch, false);
	}
}

void I8253::start_count(int ch)
{
	if(counter[ch].low_write || counter[ch].high_write)
		return;
	if(!counter[ch].gate)
		return;
	counter[ch].start = true;
	
	// regist event
	if(counter[ch].freq) {
		counter[ch].input_clk = counter[ch].delay ? 1 : get_next_count(ch);
		counter[ch].period = CPU_CLOCKS / counter[ch].freq * counter[ch].input_clk;
		counter[ch].prev_clk = vm->current_clock();
		vm->regist_event_by_clock(this, ch, counter[ch].period, false, &counter[ch].regist_id);
	}
}

void I8253::stop_count(int ch)
{
	counter[ch].start = false;
	
	// cancel event
	if(counter[ch].regist_id != -1)
		vm->cancel_event(counter[ch].regist_id);
	counter[ch].regist_id = -1;
}

void I8253::latch_count(int ch)
{
	if(counter[ch].regist_id != -1) {
		// update counter
		int passed = vm->passed_clock(counter[ch].prev_clk);
		uint32 input = counter[ch].freq * passed / CPU_CLOCKS;
		if(counter[ch].input_clk <= input)
			input = counter[ch].input_clk - 1;
		if(input > 0) {
			input_clock(ch, input);
			// cancel and re-regist event
			vm->cancel_event(counter[ch].regist_id);
			counter[ch].input_clk -= input;
			counter[ch].period -= passed;
			counter[ch].prev_clk = vm->current_clock();
			vm->regist_event_by_clock(this, ch, counter[ch].period, false, &counter[ch].regist_id);
		}
	}
	// latch counter
	counter[ch].latch = (uint16)counter[ch].count;
	counter[ch].count_latched = true;
	if((counter[ch].ctrl_reg & 0x30) == 0x10) {
		// lower byte
		counter[ch].low_read = true;
		counter[ch].high_read = false;
	}
	else if((counter[ch].ctrl_reg & 0x30) == 0x20) {
		// upper byte
		counter[ch].low_read = false;
		counter[ch].high_read = true;
	}
	else {
		// lower -> upper
		counter[ch].low_read = counter[ch].low_read = true;
	}
}

void I8253::set_signal(int ch, bool signal)
{
	bool prev = counter[ch].prev_out;
	counter[ch].prev_out = signal;
	
	if(prev && !signal) {
		// H->L
		for(int i = 0; i < dcount[ch]; i++)
			dev[ch][i]->write_signal(did[ch][i], 0, 0xffffffff);
	}
	else if(!prev && signal) {
		// L->H
		for(int i = 0; i < dcount[ch]; i++)
			dev[ch][i]->write_signal(did[ch][i], 1, 0xffffffff);
	}
}

int I8253::get_next_count(int ch)
{
	if(counter[ch].mode == 2 || counter[ch].mode == 4 || counter[ch].mode == 5)
		return (counter[ch].count > 1) ? counter[ch].count - 1 : 1;
	if(counter[ch].mode == 3) {
		int32 half = COUNT_VALUE(ch) >> 1;
		return (counter[ch].count > half) ? counter[ch].count - half : counter[ch].count;
	}
	return counter[ch].count;
}
