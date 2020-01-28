/*
 * BUBBLE CASETTE for FM-8/7? [bubblecasette.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 22, 2016 : Initial
 *
 */
#include "../vm.h"
#include "../device.h"
#include "./bubblecasette.h"

BUBBLECASETTE::BUBBLECASETTE(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	fio = NULL;
	memset(image_path, 0x00, _MAX_PATH * sizeof(_TCHAR));
	bubble_type = -1;
	memset(&bbl_header, 0x00, sizeof(bbl_header_t));
	memset(bubble_data, 0x00, 0x20000);
	bubble_inserted = false;
	read_access = write_access = false;
	p_emu = parent_emu;
	set_device_name(_T("FM Bubble Casette"));
}

BUBBLECASETTE::~BUBBLECASETTE()
{
}

void BUBBLECASETTE::initialize()
{
	is_wrote = false;
	header_changed = false;
	is_b77 = false;
	write_protect = false;
	cmd_reg = 0;
	media_offset = 0;
	media_offset_new = 0;
	media_size = 0;
	file_length = 0;
}

void BUBBLECASETTE::reset()
{
	data_reg = 0;
	offset_reg = 0;
	if(is_wrote) {
		write_one_page();
		if(is_b77) {
			if(header_changed) write_header();
			header_changed = false;
		}
		is_wrote = false;
	}
	cmd_reg = 0;
	page_address.d = 0;
	page_count.d = 0;
	write_access = read_access = false;
	
	stat_busy = false;
	stat_tdra = false;
	stat_rda = false;
	cmd_error = false;
	stat_error = false;
	
	eject_error = false;
	povr_error = false;
	crc_error = false;
	transfer_error = false;
	bad_loop_over_error = false;
	no_marker_error = false;
	undefined_cmd_error = false;
}

uint32_t BUBBLECASETTE::read_data8(uint32_t address)
{
	uint32_t val = 0xff;
	uint32_t offset;
	uint16_t mask;
	uint16_t page_size_tmp;
	uint32_t media_size_tmp;
	if(bubble_type == BUBBLE_TYPE_32KB) {
		page_size_tmp = 0x20;
		mask = 0x3ff;
		media_size_tmp = 0x8000;
	} else if(bubble_type == BUBBLE_TYPE_128KB) {
		page_size_tmp = 0x40;
		mask = 0x7ff;
		media_size_tmp = 0x20000;
	} else {
		//return val; // Not inserted.
	}
	switch(address & 7) {
	case 0: // Data Resistor
		val = (uint32_t)data_reg;
		offset = (page_address.w.l & mask) * page_size_tmp + offset_reg;
		if(stat_rda) {
			if(offset >= media_size_tmp) {
				povr_error = true;
				stat_error = true;
				cmd_error = true;
				return val;
			}
			write_access = false;
			read_access = true;
			offset_reg++;
			if(offset_reg == page_size_tmp) {
				stat_busy = false;
				stat_rda = false;
				cmd_error = true;
				offset_reg = 0;
				//if(!read_one_page()) {
				//	stat_error = true;
				//	cmd_error = true;
				//	transfer_error = true; // Okay?
				//	// Error handling: End?
				//	page_count.w.l = 0;
				//}
				if(page_count.w.l > 0) page_count.w.l--;
				if((page_count.w.l > 0) && (offset < media_size_tmp)) {
					// Multi read
					stat_busy = true;
					cmd_error = false;
					page_address.w.l = (page_address.w.l + 1) & mask;
					data_reg = bubble_data[page_address.w.l * page_size_tmp];
					//stat_rda = true;
				} else { // End multi read
					offset_reg = 0;
					read_access = false;
					//page_count.w.l = 0; // ??
				}
			} else {
				// Normal read
				data_reg = bubble_data[(page_address.w.l & mask) * page_size_tmp + offset_reg];
				//stat_rda = true;
				cmd_error = false;
				stat_busy = true;
			}
		}
		break;
	case 2: // Read status register
		val = 0x00;
		if(!bubble_inserted) stat_busy = false;
		if((cmd_reg == 1) && (bubble_inserted)){
			if(!stat_rda) stat_rda = true;
		} else 	if((cmd_reg == 2) && (bubble_inserted)){
			if(!stat_tdra) stat_tdra = true;
		}
		val |= (cmd_error)  ? 0x80 : 0;
		val |= (stat_tdra)  ? 0x40 : 0;
		val |= (stat_rda)   ? 0x20 : 0;
		val |= (stat_error) ? 0x02 : 0;
		val |= (stat_busy)  ? 0x01 : 0;
		break;
	case 3: // Read Error register
		val = 0x00;
		val |= (eject_error)         ? 0x80 : 0;
		val |= (povr_error)          ? 0x20 : 0;
		val |= (crc_error)           ? 0x10 : 0;
		val |= (transfer_error)      ? 0x08 : 0;
		val |= (bad_loop_over_error) ? 0x04 : 0;
		val |= (no_marker_error)     ? 0x02 : 0;
		val |= (undefined_cmd_error) ? 0x01 : 0;
		break;
	// 4-7 : Write Only.
	}
	return val;
}

void BUBBLECASETTE::bubble_command(uint8_t cmd)
{
	uint16_t mask;
	uint16_t page_size_tmp;
	uint32_t media_size_tmp;
	if(bubble_type == BUBBLE_TYPE_32KB) {
		page_size_tmp = 0x20;
		mask = 0x3ff;
		media_size_tmp = 0x8000;
	} else if(bubble_type == BUBBLE_TYPE_128KB) {
		page_size_tmp = 0x40;
		mask = 0x7ff;
		media_size_tmp = 0x20000;
	}
	switch(cmd) {
	case 1: // Read
		offset_reg = 0;
		read_access = false;
		if(!bubble_inserted) {
			stat_error = true;
			cmd_error = true;
			no_marker_error = true;
		} else {
			data_reg = bubble_data[(page_address.w.l & mask) * page_size_tmp];
			stat_rda = true;
			read_access = true;
		}
		break;
	case 2: // Write :: Will not check insert?
		stat_busy = true;
		write_access = false;
		if(!bubble_inserted) {
			stat_error = true;
			cmd_error = true;
			no_marker_error = true;
		} else {
			if(write_protect) {
				stat_busy = false;
				cmd_error = true;
			} else {
				offset_reg = 0;
				write_access = true;
				stat_tdra = true;
			}
		}
		break;
	case 4:
	case 15: // Reset
		data_reg = 0;
		if(is_wrote) {
			write_one_page();
			is_wrote = false;
		}
		page_address.d = 0;
		page_count.d = 0;
		write_access = read_access = false;
		break;
	default:
		stat_error = true;
		cmd_error = true;
		undefined_cmd_error = true;
		break;
	}
}
void BUBBLECASETTE::write_data8(uint32_t address, uint32_t data)
{
	uint8_t val;
	uint32_t offset;
	uint16_t mask;
	uint16_t page_size_tmp;
	uint32_t media_size_tmp;
	if(bubble_type == BUBBLE_TYPE_32KB) {
		page_size_tmp = 0x20;
		mask = 0x3ff;
		media_size_tmp = 0x8000;
	} else if(bubble_type == BUBBLE_TYPE_128KB) {
		page_size_tmp = 0x40;
		mask = 0x7ff;
		media_size_tmp = 0x20000;
	} else {
		//return; // Not inserted.
	}
	val = (uint8_t)data;

	switch(address & 7) {
	case 0: // Write Data
		data_reg = val;
		if(stat_tdra) {
			if(offset >= media_size_tmp) {
				povr_error = true;
				stat_error = true;
				cmd_error = true;
				return;
			}
			is_wrote = true;
			offset = page_address.w.l * page_size_tmp + offset_reg;
			bubble_data[(page_address.w.l & mask) * page_size_tmp + offset_reg] = data_reg;
			write_access = true;
			read_access = false;
			offset_reg++;
			if(offset_reg == page_size_tmp) {
				stat_busy = false;
				stat_tdra = false;
				cmd_error = true;
				stat_error = false;
				if(!write_one_page()) {
					stat_error = true;
					cmd_error = true;
					transfer_error = true; // Okay?
					// Error handling: End?
					page_count.w.l = 0;
					return;
				}
				offset_reg = 0;
				if(page_count.w.l > 0) {
					page_count.w.l--;
					//page_address.w.l = (page_address.w.l + 1) & mask;
				}
				if((page_count.w.l > 0) && (offset < media_size_tmp)) {
					stat_busy = true;
					cmd_error = false;
					page_address.w.l = (page_address.w.l + 1) & mask;
				} else {
					// Completed
					write_access = false;
				}
			} else {
				//stat_busy = true;
				//stat_tdra = false; // Move to event_callback()?
			}
		}
		break;
	case 1: // CMD register
		stat_busy = false;
		stat_tdra = false;
		stat_rda = false;
		cmd_error = false;
		stat_error = false;

		eject_error = false;
		povr_error = false;
		crc_error = false;
		transfer_error = false;
		bad_loop_over_error = false;
		no_marker_error = false;
		undefined_cmd_error = false;
		cmd_reg = val;
		bubble_command(val);
		
		break;
	case 2: // Read only
	case 3: // Read only
		break;
	case 4:
		page_address.b.h = val;
		break;
	case 5:
		page_address.b.l = val;
		break;
	case 6:
		page_count.b.h = val;
		break;
	case 7:
		page_count.b.l = val;
		break;
	}
}

uint32_t BUBBLECASETTE::read_signal(int address)
{
	return 0;
}

void BUBBLECASETTE::write_signal(int id, uint32_t data, uint32_t mask)
{
}

void BUBBLECASETTE::open(const _TCHAR* file_path, int bank)
{
	int i;
	int contain_medias = 0;
	is_wrote = false;
	header_changed = false;
	is_b77 = false;
	fio = NULL;
	
	media_offset = 0;
	media_offset_new = 0;
	file_length = 0;
	bubble_inserted = false;

	memset(bubble_data, 0, 0x20000);
	memset(&bbl_header, 0, sizeof(bbl_header_t));
	bubble_type = -1;

	if(fio != NULL) {
		close();
	}
	fio = new FILEIO;
	if(fio == NULL) return;
	memset(image_path, 0x00, _MAX_PATH * sizeof(_TCHAR));
	_tcsncpy(image_path, file_path, _MAX_PATH);
	
	if(fio->IsFileExisting(file_path)) {
		fio->Fopen(file_path, FILEIO_READ_WRITE_BINARY);
		file_length = fio->FileLength();
		if(file_length == 0) return;
		//printf("Size=%d\n", file_length);
		if(file_length == 0x8000) { // 32KB
			bubble_type = BUBBLE_TYPE_32KB;
			media_size = 0x8000;
		} else if(file_length == 0x20000) {
			bubble_type = BUBBLE_TYPE_128KB;
			media_size = 0x20000;
		} else {
			bubble_type = BUBBLE_TYPE_B77;
		}
		
		if(bubble_type != BUBBLE_TYPE_B77) {
			if(bubble_type < 0) return;
			write_protect = false;
			switch(bubble_type) {
			case BUBBLE_TYPE_32KB:
				fio->Fread(bubble_data, 0x8000, 1);
				break;
			case BUBBLE_TYPE_128KB:
				fio->Fread(bubble_data, 0x20000, 1);
				break;
			}
			media_num = 0;
			bubble_inserted = true;
		} else { // b77
			int remain;
			do {
				write_protect = false;
				if(!this->read_header()) break;
				if(contain_medias != bank) {
					fio->Fseek(media_offset_new , FILEIO_SEEK_SET); // Skip
					if(fio->Ftell() >= file_length) return; // Error
				} else { // Image found
					if(bubble_type == BUBBLE_TYPE_32KB) {
						if(bbl_header.offset.d > 0x20) fio->Fseek(media_offset, FILEIO_SEEK_SET);
						remain = file_length - bbl_header.offset.d;
						media_size = 0x8000;
						if(remain >= 0x8000) {
							remain = 0x8000;
						}
						fio->Fread(bubble_data, remain, 1);
						is_b77 = true;
					} else if(bubble_type == BUBBLE_TYPE_128KB) {
						if(bbl_header.offset.d > 0x20) fio->Fseek(media_offset, FILEIO_SEEK_SET);
						remain = file_length - bbl_header.offset.d;
						media_size = 0x20000;
						
						if(remain >= 0x20000) {
							remain = 0x20000;
						}
						fio->Fread(bubble_data, remain, 1);
						is_b77 = true;
					}
					bubble_inserted = true;
					media_num = (uint32_t)bank;
					contain_medias++;
					return;
				}
				contain_medias++;
			} while(contain_medias <= 16);
		}
	}
}

bool BUBBLECASETTE::read_header()
{
	uint32_t f_pos;
	uint8_t tmpval[16];
	if(fio == NULL) return false;
	if(fio->Ftell() >= file_length) return false;
	// You need convert to [UTF8|Local8Bit] when using UI.
	fio->Fread(bbl_header.filename, 0x10, 1);
	if(fio->Fread(&tmpval, 0x10, 1) != 0x10) return false;
	// Offset(little endian)
	bbl_header.offset.b.l  = tmpval[4];
	bbl_header.offset.b.h  = tmpval[5];
	bbl_header.offset.b.h2 = tmpval[6];
	bbl_header.offset.b.h3 = tmpval[7];
	// Casette size(little endian)
	bbl_header.size.b.l  = tmpval[12];
	bbl_header.size.b.h  = tmpval[13];
	bbl_header.size.b.h2 = tmpval[14];
	bbl_header.size.b.h3 = tmpval[15];
	// Misc
	bbl_header.misc[0] = tmpval[0];
	bbl_header.misc[1] = tmpval[1];
	bbl_header.misc[2] = tmpval[2];
	bbl_header.misc[3] = tmpval[3];
	
	bbl_header.misc[4] = tmpval[8];
	bbl_header.misc[5] = tmpval[9];
	bbl_header.misc[6] = tmpval[10];
	bbl_header.misc[7] = tmpval[11];
	
	switch(tmpval[11]) {
	case 0x80:
		bubble_type = BUBBLE_TYPE_32KB;
		break;
	case 0x90:
		bubble_type = BUBBLE_TYPE_128KB;
		break;
	default:
		return false;
		break;
	}
	media_size = bbl_header.size.d;
	media_offset = media_offset_new + bbl_header.offset.d;
	media_offset_new = media_offset_new + media_size;
	return true;
}	

void BUBBLECASETTE::write_header()
{
	uint32_t f_pos;
	uint8_t tmpval[16];
	if(fio == NULL) return;
	if(fio->Ftell() >= file_length) return;
	// You need convert to [UTF8|Local8Bit] when using UI.
	// Offset(little endian)
	tmpval[4]  = bbl_header.offset.b.l;
	tmpval[5]  = bbl_header.offset.b.h;
	tmpval[6]  = bbl_header.offset.b.h2;
	tmpval[7]  = bbl_header.offset.b.h3;
	// Casette size(little endian)
	tmpval[12] = bbl_header.size.b.l;
	tmpval[13] = bbl_header.size.b.h;
	tmpval[14] = bbl_header.size.b.h2;
	tmpval[15] = bbl_header.size.b.h3;
	// Misc
	tmpval[0]  = bbl_header.misc[0];
	tmpval[1]  = bbl_header.misc[1];
	tmpval[2]  = bbl_header.misc[2];
	tmpval[3]  = bbl_header.misc[3];
	
	tmpval[8]  = bbl_header.misc[4];
	tmpval[9]  = bbl_header.misc[5];
	tmpval[10] = bbl_header.misc[6];
	tmpval[11] = bbl_header.misc[7];
	
	switch(bubble_type) {
	case BUBBLE_TYPE_32KB:
		tmpval[11] = 0x80;
		break;
	case BUBBLE_TYPE_128KB:
		tmpval[11] = 0x90;
		break;
	default:
		return;
		break;
	}
	fio->Fwrite(bbl_header.filename, 0x10, 1);
	fio->Fwrite(&tmpval, 0x10, 1);
	return;
}

bool BUBBLECASETTE::read_one_page()
{
	uint32_t f_pos;
	if(fio == NULL) return false;
	if(!fio->IsOpened()) return false;
	if(!bubble_inserted) {
		// Error Handling
		return false;
	}
	f_pos = media_offset;
	{
		uint32_t offset = 0;
		uint32_t page_size = 0;
		int remain = (int)media_size - (int)bbl_header.offset.d;
		if(remain <= 0) return false;
		switch(bubble_type) {
		case BUBBLE_TYPE_32KB:
			offset = (page_address.w.l & 0x03ff) * 0x20;
			page_size = 0x20;
			break;
		case BUBBLE_TYPE_128KB:
			offset = bbl_header.offset.d + (page_address.w.l & 0x07ff) * 0x40;
			page_size = 0x40;
			break;
		default:
			return false;
			break;
		}
		if(remain < (int)(offset + page_size)) return false;
		fio->Fseek(f_pos + offset, FILEIO_SEEK_SET);
		fio->Fread(&bubble_data[offset], page_size, 1);
	}
	return true;
}

bool BUBBLECASETTE::write_one_page()
{
	uint32_t f_pos;
	if(fio == NULL) return false;
	if(!fio->IsOpened()) return false;
	if(!bubble_inserted) {
		// Error Handling
		return false;
	}
	f_pos = media_offset;
	if(is_wrote) {
		uint32_t offset = 0;
		uint32_t page_size = 0;
		int remain = (int)media_size - (int)bbl_header.offset.d;
		if(remain <= 0) return false;
		switch(bubble_type) {
		case BUBBLE_TYPE_32KB:
			offset = (page_address.w.l & 0x03ff) * 0x20;
			page_size = 0x20;
			break;
		case BUBBLE_TYPE_128KB:
			offset = bbl_header.offset.d + (page_address.w.l & 0x07ff) * 0x40;
			page_size = 0x40;
			break;
		default:
			return false;
			break;
		}
		//printf("Write One Page: PAGE=%04x COUNT=%04x:\n ",page_address.w.l, page_count.w.l);
		if(remain < (int)(offset + page_size)) return false;
		fio->Fseek(f_pos + offset, FILEIO_SEEK_SET);
		fio->Fwrite(&bubble_data[offset], page_size, 1);
		is_wrote = false;
	}
	return true;
}

void BUBBLECASETTE::close()
{
	int i;
	if(fio != NULL) {
		if(fio->IsOpened()) {
			if(is_wrote) write_one_page();
			if(is_b77) {
				if(header_changed) write_header();
				header_changed = false;
				is_b77 = false;
			}
			fio->Fclose();
		}
		delete fio;
	}
	fio = NULL;
	memset(image_path, 0x00, _MAX_PATH * sizeof(_TCHAR));
	bubble_type = -1;
	memset(&bbl_header, 0x00, sizeof(bbl_header_t));
	memset(bubble_data, 0x00, 0x20000);
	media_offset = 0;
	media_offset_new = 0;
	is_wrote = false;
	is_b77 = false;
	header_changed = false;
	bubble_inserted = false;
	read_access = write_access = false;
}

void BUBBLECASETTE::event_callback(int event_id, int err)
{
}

#define STATE_VERSION 1
void BUBBLECASETTE::save_state(FILEIO *state_fio)
{
	int i, j;
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log(_T("Save State: BUBBLE: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	// Attributes
	state_fio->FputUint32_BE(file_length);
	state_fio->FputBool(bubble_inserted);
	state_fio->FputInt32_BE(bubble_type);
	state_fio->FputInt32_BE(media_num);
	state_fio->FputUint32_BE(media_offset);
	state_fio->FputUint32_BE(media_offset_new);
	state_fio->FputUint32_BE(media_size);
	// Data reg
	state_fio->FputUint8(data_reg);
	// Command reg
	state_fio->FputUint8(cmd_reg);
	// Status reg
	state_fio->FputBool(stat_busy);
	state_fio->FputBool(stat_tdra);
	state_fio->FputBool(stat_rda);
	state_fio->FputBool(cmd_error);
	state_fio->FputBool(stat_error);
	// Error reg
	state_fio->FputBool(eject_error);
	state_fio->FputBool(povr_error);
	state_fio->FputBool(crc_error);
	state_fio->FputBool(transfer_error);
	state_fio->FputBool(bad_loop_over_error);
	state_fio->FputBool(no_marker_error);
	state_fio->FputBool(undefined_cmd_error);
	// Page address
	state_fio->FputUint32_BE(page_address.d);
	//Page Count
	state_fio->FputUint32_BE(page_count.d);
	// Misc flags
	state_fio->FputBool(is_b77);
	state_fio->FputBool(read_access);
	state_fio->FputBool(write_access);
	state_fio->FputBool(write_protect);
	state_fio->FputUint8(offset_reg);

	
	state_fio->Fwrite(image_path, _MAX_PATH * sizeof(_TCHAR), 1);
	if(fio != NULL) {
		if(fio->IsOpened()) {
			if(is_wrote) write_one_page();
			if(is_b77) {
				if(header_changed) {
					write_header();
					header_changed = false;
				}
			}
		}
	}
}

bool BUBBLECASETTE::load_state(FILEIO *state_fio)
{
	int i, j;
	if(state_fio->FgetUint32_BE() != STATE_VERSION) return false;
	if(state_fio->FgetInt32_BE() != this_device_id) return false;
	this->out_debug_log(_T("Load State: BUBBLE: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	// Attributes
	file_length = state_fio->FgetUint32_BE();
	bubble_inserted = state_fio->FgetBool();
	bubble_type = state_fio->FgetInt32_BE();
	media_num = state_fio->FgetInt32_BE();
	media_offset = state_fio->FgetInt32_BE();
	media_offset_new = state_fio->FgetInt32_BE();
	media_size = state_fio->FgetInt32_BE();
	// Data reg
	data_reg = state_fio->FgetUint8();
	// Command reg
	cmd_reg = state_fio->FgetUint8();
	// Status reg
	stat_busy = state_fio->FgetBool();
	stat_tdra = state_fio->FgetBool();
	stat_rda = state_fio->FgetBool();
	cmd_error = state_fio->FgetBool();
	stat_error = state_fio->FgetBool();
	// Error reg
	eject_error = state_fio->FgetBool();
	povr_error = state_fio->FgetBool();
	crc_error = state_fio->FgetBool();
	transfer_error = state_fio->FgetBool();
	bad_loop_over_error = state_fio->FgetBool();
	no_marker_error = state_fio->FgetBool();
	undefined_cmd_error = state_fio->FgetBool();
	// Page address
	page_address.d = state_fio->FgetUint32_BE();
	//Page Count
	page_count.d = state_fio->FgetUint32_BE();
	// Misc flags
	is_b77 = state_fio->FgetBool();
	read_access = state_fio->FgetBool();
	write_access = state_fio->FgetBool();
	write_protect = state_fio->FgetBool();
	offset_reg = state_fio->FgetUint8();
	is_wrote = false;
	header_changed = false;
	
	if(state_fio->Fread(image_path, _MAX_PATH * sizeof(_TCHAR), 1) != (_MAX_PATH * sizeof(_TCHAR))) return false;
	if(_tcslen(image_path) > 0) {
		this->open(image_path, (int)media_num);
	}
	return true;
}
