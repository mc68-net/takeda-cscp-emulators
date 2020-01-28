/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.19-

	[ uPD1990A ]
*/

#include "upd1990a.h"

#define YEAR		time[0]
#define MONTH		time[1]
#define DAY		time[2]
#define DAY_OF_WEEK	time[3]
#define HOUR		time[4]
#define MINUTE		time[5]
#define SECOND		time[6]

void UPD1990A::initialize()
{
	cmd = mode = 0;
	srl = srh = 0;
	clk = stb = din = true;
}

void UPD1990A::write_io8(uint32 addr, uint32 data)
{
	// for NEC PC-98x1 $20
	cmd = data & 7;
	din = ((data & 0x20) != 0);
	write_signal(SIG_UPD1990A_STB, data, 8);
	write_signal(SIG_UPD1990A_CLK, data, 0x10);
}

void UPD1990A::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_UPD1990A_CLK) {
		bool next = ((data & mask) != 0);
		if(!clk && next) {
			int dout = srl & 1;
			if(mode == 1) {
				srl = (srl >> 1) | ((srh & 1) << 19);
				srh = (srh >> 1) | ((din ? 1 : 0) << 19);
			}
			for(int i = 0; i < dcount; i++)
				dev[i]->write_signal(did[i], dout ? 0xffffffff : 0, dmask[i]);
		}
		clk = next;
	}
	else if(id == SIG_UPD1990A_STB) {
		bool next = ((data & mask) != 0);
		if(!stb && next) {
			if((mode = cmd) == 3) {
				int time[8];
				emu->get_timer(time);
				srl = SECOND % 10;
				srl |= (int)(SECOND / 10) << 4;
				srl |= (MINUTE % 10) << 8;
				srl |= (int)(MINUTE / 10) << 12;
				srl |= (HOUR % 10) << 16;
				srh = (int)(HOUR / 10);
				srh |= (DAY % 10) << 4;
				srh |= (int)(DAY / 10) << 8;
				srh |= DAY_OF_WEEK << 12;
				srh |= MONTH << 16;
			}
		}
		stb = next;
	}
	else if(id == SIG_UPD1990A_C0)
		cmd = (cmd & ~1) | (data & mask ? 1 : 0);
	else if(id == SIG_UPD1990A_C1)
		cmd = (cmd & ~2) | (data & mask ? 2 : 0);
	else if(id == SIG_UPD1990A_C2)
		cmd = (cmd & ~4) | (data & mask ? 4 : 0);
	else if(id == SIG_UPD1990A_DIN)
		din = ((data & mask) != 0);
}
