/*
	NEC PC-6001 Emulator 'yaPC-6001'

	Author : tanam
	Date   : 2013.07.15-

	[ system port ]
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"
#include "../../fileio.h"
#include "d88.h"

// �ő�h���C�u�ڑ���
#define	MAXDRV	4

// �R�}���h
// ���ۂɂ� PC-80S31 �̃R�}���h�����Ǒ�̓����H
enum FddCommand
{
	INIT				= 0x00,
	WRITE_DATA			= 0x01,
	READ_DATA			= 0x02,
	SEND_DATA			= 0x03,
	COPY				= 0x04,
	FORMAT				= 0x05,
	SEND_RESULT_STATUS	= 0x06,
	SEND_DRIVE_STATUS	= 0x07,
	TRANSMIT			= 0x11,
	RECEIVE				= 0x12,
	LOAD				= 0x14,
	SAVE				= 0x15,
	
	IDLE				= 0xff,	// �����҂��̏��
	EndofFdcCmd
};

enum FdcPhase {
	IDLEP = 0,
	C_PHASE,
	E_PHASE,
	R_PHASE
};

enum FdcSeek{
	SK_STOP = 0,	// �V�[�N�Ȃ�
	SK_SEEK,		// �V�[�N��
	SK_END			// �V�[�N����
};

struct PD765 {
	BYTE command;		// �R�}���h
	
FdcPhase phase;		// Phase (C/E/R)
int step;			// Phase���̏����菇

	BYTE SRT;			// Step Rate Time
	BYTE HUT;			// Head Unloaded Time
	BYTE HLT;			// Head Load Time
	bool ND;			// Non DMA Mode  true:Non DMA false:DMA
	
	FdcSeek SeekSta[4];	// �V�[�N���
	BYTE NCN[4];		// New Cylinder Number
	BYTE PCN[4];		// Present Cylinder Number
	
	
	BYTE MT;			// Multi-track
	BYTE MF;			// MFM/FM Mode
	BYTE SK;			// Skip
	BYTE HD;			// Head
	BYTE US;			// Unit Select
	
	BYTE C;				// Cylinder Number
	BYTE H;				// Head Address
	BYTE R;				// Record
	BYTE N;				// Number
	BYTE EOT;			// End of Track
	BYTE GPL;			// Gap Length
	BYTE DTL;			// Data length
	
	BYTE D;				// Format Data
	BYTE SC;			// Sector
	
	BYTE st0;			// ST0
	BYTE st1;			// ST1
	BYTE st2;			// ST2
	BYTE st3;			// ST3
	
	BYTE status;		// Status
	bool intr;			// FDC�����ݔ����t���O
	
		PD765() :
		command(0), phase(R_PHASE), step(0),
		SRT(32), HUT(0), HLT(0), ND(false),
		MT(0), MF(0), SK(0), HD(0), US(0), C(0), H(0), R(0), N(0),
		EOT(0), GPL(0), DTL(0),
		st0(0), st1(0), st2(0), st3(0), status(0)
		{
			INITARRAY( SeekSta, SK_STOP );
			INITARRAY( NCN, 0 );
			INITARRAY( PCN, 0 );
		}
};

// �~�j�t���b�s�[�f�B�X�N �e����
struct DISK60 {
	int DAC;			// Data Accepted	:�f�[�^��M��
	int RFD;			// Ready For Data	:�f�[�^��M������
	int DAV;			// Data Valid		:�f�[�^���M��
	
	int command;		// �󂯎�����R�}���h
	int step;			// �p�����[�^���͑҂��X�e�[�^�X
	
	int blk;			// �]���u���b�N��
	int drv;			// �h���C�u�ԍ�-1
	int trk;			// �g���b�N�ԍ�
	int sct;			// �Z�N�^�ԍ�
	
	int rsize;			// �Ǎ��݃o�b�t�@�̃f�[�^��
	int wsize;			// �����݃o�b�t�@�̃f�[�^��
	int ridx;
	
	int size;			// ��������o�C�g��
	
	BYTE retdat;		// port D0H ����Ԃ��l
	
	BYTE busy;			// �h���C�uBUSY 1:�h���C�u1 2:�h���C�u2
	
	DISK60() :
		DAC(0), RFD(0), DAV(0),
		command(IDLE), step(0),
		blk(0), drv(0), trk(0), sct(0),
		size(0),
		retdat(0xff), busy(0) {}
};

////////////////////////////////////////////////////////////////
// �N���X��`
////////////////////////////////////////////////////////////////
class DSK6 { /// : public P6DEVICE, public IDoko {
protected:
	int DrvNum;							// �h���C�u��
	char FilePath[MAXDRV][PATH_MAX];	// �t�@�C���p�X
	cD88 *Dimg[MAXDRV];					// �f�B�X�N�C���[�W�I�u�W�F�N�g�ւ̃|�C���^
	bool Sys[MAXDRV];					// �V�X�e���f�B�X�N�t���O
	int waitcnt;						// �E�F�C�g�J�E���^
	
	void ResetWait();					// �E�F�C�g�J�E���^���Z�b�g
	void AddWait( int );				// �E�F�C�g�J�E���^���Z
	bool SetWait( int );				// �E�F�C�g�ݒ�
	
public:
	DSK6(); /// VM6 *, const P6ID& );			// �R���X�g���N�^
	virtual ~DSK6();					// �f�X�g���N�^
	
	virtual void EventCallback( int, int );	// �C�x���g�R�[���o�b�N�֐�
	
	virtual bool Init( int ) = 0;		// ������
	virtual void Reset() = 0;			// ���Z�b�g
	
	bool Mount( int, char * );			// DISK �}�E���g
	void Unmount( int );				// DISK �A���}�E���g
	
	int GetDrives();					// �h���C�u���擾
	
	bool IsMount( int );				// �}�E���g�ς�?
	bool IsSystem( int );				// �V�X�e���f�B�X�N?
	bool IsProtect( int );				// �v���e�N�g?
	virtual bool InAccess( int ) = 0;	// �A�N�Z�X��?
	
	const char *GetFile( int );			// �t�@�C���p�X�擾
	const char *GetName( int );			// DISK���擾
};

class DSK60 : public DSK6 { /// , public Device {
private:
	DISK60 mdisk;			// �~�j�t���b�s�[�f�B�X�N�e����
	
	BYTE RBuf[4096];		// �Ǎ��݃o�b�t�@
	BYTE WBuf[4096];		// �����݃o�b�t�@
	
	BYTE io_D1H;
	BYTE io_D2H;
	
	BYTE FddIn();			// DISK���j�b�g����̃f�[�^���� 		(port D0H)
	void FddOut( BYTE );	// DISK���j�b�g�ւ̃R�}���h�C�f�[�^�o�� (port D1H)
	BYTE FddCntIn();		// DISK���j�b�g����̐���M������ 		(port D2H)
	void FddCntOut( BYTE );	// DISK���j�b�g�ւ̐���M���o�� 		(port D3H)	
public:
	// I/O�A�N�Z�X�֐�
	void OutD1H( int, BYTE );
	void OutD2H( int, BYTE );
	void OutD3H( int, BYTE );
	BYTE InD0H( int );
	BYTE InD1H( int );
	BYTE InD2H( int );
	DSK60();			// �R���X�g���N�^
	~DSK60();							// �f�X�g���N�^
	
	void EventCallback( int, int );		// �C�x���g�R�[���o�b�N�֐�
	
	bool Init( int );					// ������
	void Reset();						// ���Z�b�g
	bool InAccess( int );				// �A�N�Z�X��?
	
	// �f�o�C�XID
	enum IDOut{ outD1H=0, outD2H, outD3H };
	enum IDIn {  inD0H=0,  inD1H,  inD2H };
};

class SYSTEM : public DEVICE
{
private:
	DEVICE *d_pio, *d_fdc;
public:
	SYSTEM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SYSTEM() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);

	// unique functions
	void set_context_pio(DEVICE* device) {
		d_pio = device;
	}
	void set_context_fdc(DEVICE* device) {
		d_fdc = device;
	}
};

#endif

