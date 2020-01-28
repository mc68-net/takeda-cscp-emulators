/*
 * FM-7 Main I/O [sound.cpp]
 *  - PSG, OPN, and Buzzer.
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 19, 2015 : Initial, split from fm7_mainio.cpp.
 *
 */

#include "../pcm1bit.h"
#include "../datarec.h"
#include "../ym2203.h"

#include "fm7_mainio.h"
#include "../../config.h"

void FM7_MAINIO::reset_sound(void)
{
	int i, j, k;

#if defined(_FM8)
	k = 1;
#elif defined(_FM77AV_VARIANTS)
	k = 3;
#else
	k = 4;
#endif
	for(i = 0; i < k; i++) {
		opn_data[i]= 0;
		opn_cmdreg[i] = 0;
		opn_address[i] = 0;
		opn_stat[i] = 0;
		opn_ch3mode[i] = 0x00;
		if(opn[i] != NULL) {
			opn[i]->reset();
			opn[i]->write_data8(0, 0x2e);
			opn[i]->write_data8(1, 0);	// set prescaler
			opn[i]->write_data8(0, 0x27);
			opn[i]->write_data8(1, 0x00);
			for(j = 0; j < 3; j++) {
				opn[i]->set_reg(0x28, j | 0xfe);
			}
//			opn[i]->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
		}
	   
	}
#if defined(_FM8)
	connect_psg = false;
#else	
 #if defined(_FM77AV_VARIANTS)
	opn_psg_77av = true;
 #else
	opn_psg_77av = false;
 #endif
	connect_opn = connect_whg = connect_thg = false;
	if(opn_psg_77av) connect_opn = true;
#endif	

#if defined(_FM8)
	if(config.sound_device_type == 0) {
		connect_psg = false;
	} else {
		connect_psg = true;
	}
#else
	connect_thg = false;
	connect_whg = false;
#if defined(_FM77AV_VARIANTS)	
	connect_opn = true;
#else
	connect_opn = false;
#endif
	switch(config.sound_device_type) {
		case 0:
			break;
		case 1:
	   		connect_opn = true;
	   		break;
		case 2:
	   		connect_whg = true;
	   		break;
		case 3:
	   		connect_whg = true;
	   		connect_opn = true;
	   		break;
		case 4:
	   		connect_thg = true;
	   		break;
		case 5:
	 		connect_thg = true;
	   		connect_opn = true;
	   		break;
		case 6:
	   		connect_thg = true;
	   		connect_whg = true;
	   		break;
		case 7:
	   		connect_thg = true;
	   		connect_whg = true;
	   		connect_opn = true;
	   		break;
	}
#endif	
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, 0x01, 0x01);
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 0x00, 0x01);
	
#if defined(_FM8)
	opn[0]->write_signal(SIG_YM2203_MUTE, !connect_psg ? 0xffffffff : 0x00000000, 0xffffffff);
#else
	opn[0]->write_signal(SIG_YM2203_MUTE, !connect_opn ? 0xffffffff : 0x00000000, 0xffffffff);
	opn[1]->write_signal(SIG_YM2203_MUTE, !connect_whg ? 0xffffffff : 0x00000000, 0xffffffff);
	opn[2]->write_signal(SIG_YM2203_MUTE, !connect_thg ? 0xffffffff : 0x00000000, 0xffffffff);
# if !defined(_FM77AV_VARIANTS)
	opn[3]->write_signal(SIG_YM2203_MUTE, 0x00000000, 0xffffffff);
# endif
#endif	
}


void FM7_MAINIO::set_psg(uint8_t val)
{
#if defined(_FM8)
	if(connect_psg) return set_opn(0, val);
#else	
	if(opn_psg_77av) return set_opn(0, val); // 77AV ETC
	set_opn(3, val);
#endif	
}

uint8_t FM7_MAINIO::get_psg(void)
{
	//uint8_t val = 0xff;
#if defined(_FM8)
	if(connect_psg) return get_opn(0);
	return 0xff;
#else	
	if(opn_psg_77av) {
		return get_opn(0);
	}
	return get_opn(3);
#endif	
}

/*
 * $fd0d : After 77AV, this is OPN.
 */
void FM7_MAINIO::set_psg_cmd(uint8_t cmd)
{
	cmd = cmd & 0x03;
#if defined(_FM8)
	if(connect_psg) set_opn_cmd(0, cmd);
	return;
#else	
	if(opn_psg_77av) {
		set_opn_cmd(0, cmd);
		return;
	}
	set_opn_cmd(3, cmd);
	return;
#endif	
}

// OPN
// Write to FD16, same as 
void FM7_MAINIO::write_opn_reg(int index, uint32_t addr, uint32_t data)
{
#if defined(_FM8)
	if(connect_psg) {
		opn_regs[0][addr] = data;
		opn[0]->write_io8(0, addr & 0x0f);
		opn[0]->write_io8(1, data);
		return;
	}
#else	
	if(index == 3) { // PSG
	  	opn[index]->write_io8(0, addr & 0x0f);
		opn[index]->write_io8(1, data);
		return;
	}
	if((addr >= 0x2d) && (addr < 0x30)) {
		opn[index]->write_io8(0, addr);
		return;
	}
	if(addr == 0x27) {
		opn_ch3mode[index] = data & 0xc0;
	}
	opn[index]->write_io8(0, addr);
	opn[index]->write_io8(1, data);
	opn_regs[index][addr] = data;
	return;
#endif	
}

void FM7_MAINIO::set_opn(int index, uint8_t val)
{
#if defined(_FM8)
	if(!connect_psg) {
		return;
	}
	if(index != 0) return;
#else	
	if((index > 3) || (index < 0)) return;
	if((index == 0) && (!connect_opn)) return;
	if((index == 1) && (!connect_whg)) return;
	if((index == 2) && (!connect_thg)) return;
	if((index == 3) && (opn_psg_77av)) return;
	if(opn[index] == NULL) return;
#endif   
	opn_data[index] = val;
	switch(opn_cmdreg[index]){
		case 0: // High inpedance
		case 1: // Read Data
			break;
		case 2: // Write Data
			write_opn_reg(index, opn_address[index], opn_data[index]);
			break;
		case 3: // Register address
			if(index == 3) {
				opn_address[index] = val & 0x0f;
			} else {
				opn_address[index] = val;
				if((val > 0x2c) && (val < 0x30)) {
					opn_data[index] = 0;
					opn[index]->write_io8(0, val);
					opn[index]->write_io8(1, 0);
				}
			}
			break;
	   
	}
}

uint8_t FM7_MAINIO::get_opn(int index)
{
	uint8_t val = 0xff;
#if defined(_FM8)
	if(!connect_psg) return val;
	if(index != 0) return val;
#else	
	if((index > 2) || (index < 0)) return val;
	if((index == 0) && (!connect_opn)) return val;
	if((index == 1) && (!connect_whg)) return val;
	if((index == 2) && (!connect_thg)) return val;
	if((index == 3) && (opn_psg_77av)) return val;
#endif	   
	if(opn[index] == NULL) return val;
	switch(opn_cmdreg[index]) {
		case 0:
		case 1:
		case 2:
		case 3:
			val = opn_data[index];
			break;
		case 4:
			opn_stat[index] = opn[index]->read_io8(0) & 0x03;
			if(index != 3) val = opn_stat[index];
	   		break;
		case 0x09:
			if(index != 0) return 0xff;
			if(opn_address[0] == 0x0e) {
				return joystick->read_data8(0);
			}
			return 0x00;
			break;
		default:
	 		break;
		}
		return val;
}
  /*
   * $fd16?
   */
void FM7_MAINIO::set_opn_cmd(int index, uint8_t cmd)
{
#if defined(_FM8)
	if(!connect_psg) return;
	if(index != 0) return;
#else	
	if((index >= 4) || (index < 0)) return;
	if((index == 0) && (!connect_opn)) return;
	if((index == 1) && (!connect_whg)) return;
	if((index == 2) && (!connect_thg)) return;
	if((index == 3) && (opn_psg_77av)) return ;
#endif	
	uint32_t mask[16] = { // Parameter is related by XM7. Thanks Ryu.
		0xff, 0x0f, 0xff, 0x0f,
		0xff, 0x0f, 0x1f, 0xff,
		0x1f, 0x1f, 0x1f, 0xff,
		0xff, 0x0f, 0xff, 0xff
	};
	opn_cmdreg[index] = cmd & 0x0f;
	uint8_t val = opn_data[index];
        switch(opn_cmdreg[index]) {
		case 0:
			break;
		case 1:
			opn[index]->write_io8(0, opn_address[index]);
			opn_data[index] = opn[index]->read_io8(1);
			if(opn_address[index] <= 0x0f) {
				opn_data[index] &= mask[opn_address[index]];
			}
			break;
		case 2:
			write_opn_reg(index, opn_address[index], opn_data[index]);
	 		break;
		case 3: // Register address
			if(index == 3) {
				opn_address[index] = val & 0x0f;
			} else {
				opn_address[index] = val;
				if((val > 0x2c) && (val < 0x30)) {
					opn_data[index] = 0;
					opn[index]->write_io8(0, val);
					opn[index]->write_io8(1, 0);
				}
			}
			break;
		case 4:
			opn_stat[index] = opn[index]->read_io8(0) & 0x03;
	   		break;
	 	default:
	   		break;
	}
	return;
}

uint8_t FM7_MAINIO::get_extirq_whg(void)
{
	uint8_t val = 0xff;
#if !defined(_FM8)	
	if(intstat_whg && connect_whg) val &= ~0x08;
#endif	
	return val;
}

uint8_t FM7_MAINIO::get_extirq_thg(void)
{
	uint8_t val = 0xff;
#if !defined(_FM8)	
	if(intstat_thg && connect_thg) val &= ~0x08;
#endif	
	return val;
}

void FM7_MAINIO::opn_note_on(int index)
{
	uint8_t r;
#if !defined(_FM8)
	if((index < 0) || (index >= 3)) return;
	// Not on for CSM mode. From XM7. Thanks, Ryu.
	r = opn_ch3mode[index];
	if ((r & 0xc0) == 0x80) {
		opn[index]->write_io8(0, 0x27);
		opn[index]->write_io8(1, opn_ch3mode[index] & 0xc0);
	}
#endif	
}


void FM7_MAINIO::set_beep(uint32_t data) // fd03
{
	bool flag = ((data & 0xc0) != 0);
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, ~data, 0x01);
	if(flag != beep_flag) {
		if(flag) {
			beep_snd = true;
			pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, 1, 1);
			pcm1bit->write_signal(SIG_PCM1BIT_ON, 1, 1);
		} else {
			beep_snd = false;
			pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, 0, 1);
			pcm1bit->write_signal(SIG_PCM1BIT_ON, 0, 1);
		}
		beep_flag = flag;
	}
	if((data & 0x40) != 0) {
		// BEEP ON, after 205ms, BEEP OFF.  
		set_beep_oneshot();
	}
}

// SIGNAL / FM7_MAINIO_BEEP
void FM7_MAINIO::set_beep_oneshot(void) // SUB:D4xx
{
	beep_snd = true;
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 1, 1);
	if(event_beep_oneshot >= 0) cancel_event(this, event_beep_oneshot);
	register_event(this, EVENT_BEEP_OFF, 205.0 * 1000.0, false, &event_beep_oneshot); // NEXT CYCLE
}

// EVENT_BEEP_OFF
void FM7_MAINIO::event_beep_off(void)
{
	beep_flag = false;
	beep_snd = false;
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 0, 1);
	event_beep_oneshot = -1;
}

// EVENT_BEEP_CYCLE
void FM7_MAINIO::event_beep_cycle(void)
{
	beep_snd = !beep_snd;
	pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, beep_snd ? 1 : 0, 1);
}