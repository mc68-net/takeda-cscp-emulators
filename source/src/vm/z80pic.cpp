/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ Z80 mode2 int ]
*/

#include "z80pic.h"
#include "z80.h"

void Z80PIC::reset()
{
	for(int i = 0; i < MAX_IRQ; i++)
		irq[i].request = irq[i].running = false;
	pri_cnt = 0;
}

void Z80PIC::request_int(int pri, uint32 vector, bool pending)
{
	if(pri_cnt < pri + 1)
		pri_cnt = pri + 1;
	
	// accept the request when the requested interrupt is not running now
	if(!irq[pri].running) {
		irq[pri].request = true;
		irq[pri].vector = vector;
		do_ei();
	}
	// cancel request when not pending
	if(!pending)
		irq[pri].request = false;
}

void Z80PIC::cancel_int(int pri)
{
	irq[pri].request = false;
}

void Z80PIC::do_reti()
{
	// most high priority request is finished
	for(int i = 0; i < pri_cnt; i++) {
		if(irq[i].running) {
			irq[i].running = false;
			// try next interrupt
			do_ei();
			return;
		}
	}
}

void Z80PIC::do_ei()
{
	// check cpu status
	if(!dev->accept_int())
		return;
	
	// try interrupt when cpu can accept interrupt requst
	for(int i = 0; i < pri_cnt; i++) {
		// quit when more high priority request is running
		if(irq[i].running)
			return;
		// do interrupt
		if(irq[i].request) {
			irq[i].running = true;
			irq[i].request = false;
			dev->write_signal(SIG_CPU_DO_INT, (uint32)irq[i].vector, 0xffffffff);
			return;
		}
	}
}


