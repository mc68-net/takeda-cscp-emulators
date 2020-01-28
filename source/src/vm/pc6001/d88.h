#ifndef D88_H_INCLUDED
#define D88_H_INCLUDED

#include <stdio.h>

#define PRINTD(...) sprintf(__VA_ARGS__);

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#define	PATH_MAX	260

#define FGETBYTE(fp)		((BYTE)fgetc(fp))
#define FGETWORD(fp)		((WORD)(((BYTE)fgetc(fp))|((BYTE)fgetc(fp)<<8)))
#define FGETDWORD(fp)		((DWORD)(((BYTE)fgetc(fp))|((BYTE)fgetc(fp)<<8)|((BYTE)fgetc(fp)<<16)|((BYTE)fgetc(fp)<<24)))
#define FPUTBYTE(data,fp)	fputc((data)&0xff,fp)
#define FPUTWORD(data,fp)	{ fputc((data)&0xff,fp); fputc(((data)>>8)&0xff,fp); }
#define FPUTDWORD(data,fp)	{ fputc((data)&0xff,fp); fputc(((data)>>8)&0xff,fp); fputc(((data)>>16)&0xff,fp); fputc(((data)>>24)&0xff,fp); }

#define INITARRAY(arr,val)	{for(int i=0; i<COUNTOF(arr); i++) arr[i] = val;}
#define	COUNTOF(arr)		(int)(sizeof(arr)/sizeof((arr)[0]))

// Disk BIOS Status
#define BIOS_READY					(0x00)
#define BIOS_WRITE_PROTECT			(0x70)
#define BIOS_ID_CRC_ERROR			(0xa0)
#define BIOS_DATA_CRC_ERROR			(0xb0)
#define BIOS_NO_DATA				(0xc0)
#define BIOS_MISSING_IAM			(0xe0)
#define BIOS_MISSING_DAM			(0xf0)

////////////////////////////////////////////////////////////////
// �N���X��`
////////////////////////////////////////////////////////////////
class cD88 {
private:
	// D88 �Z�N�^���\����
	struct D88SECTOR {
		BYTE c;				// ID �� C (�V�����_No �Жʂ̏ꍇ��=�g���b�NNo)
		BYTE h;				// ID �� H (�w�b�_�A�h���X �Жʂ̏ꍇ��=0)
		BYTE r;				// ID �� R (�g���b�N���̃Z�N�^No)
		BYTE n;				// ID �� N (�Z�N�^�T�C�Y 0:128 1:256 2:512 3:1024)
		WORD sec_nr;		// ���̃g���b�N���ɑ��݂���Z�N�^�̐�
		BYTE density;		// �L�^���x     0x00:�{���x   0x40:�P���x
		BYTE deleted;		// DELETED MARK 0x00:�m�[�}�� 0x10:DELETED
		BYTE status;		// �X�e�[�^�X
		BYTE reserve[5];	// ���U�[�u
		WORD size;			// ���̃Z�N�^���̃f�[�^�T�C�Y
		DWORD data;			// �f�[�^�ւ̃I�t�Z�b�g
		WORD offset;		// ���ɓǍ��ރf�[�^�̃Z�N�^�擪����̃I�t�Z�b�g
		WORD secno;			// �A�N�Z�X���̃Z�N�^No
		
		D88SECTOR() : c(0), h(0), r(0), n(0), sec_nr(0), density(0), deleted(0),
						status(0), size(0), data(0), offset(0), secno(0)
		{
			INITARRAY( reserve, 0 );
		}
	};
	
	// D88 ���\����
	struct D88INFO {
		BYTE name[17];		// �f�B�X�N�̖��O(ASCII + '\0')
		BYTE reserve[9];	// ���U�[�u
		BYTE protect;		// ���C�g�v���e�N�g  0x00:�Ȃ� 0x10:����
		BYTE type;			// �f�B�X�N�̎��    0x00:2D   0x10:2DD  0x20:2HD
		DWORD size;			// �f�B�X�N�̃T�C�Y(�w�b�_��+�S�g���b�N��)
		DWORD table[164];	// �g���b�N���̃I�t�Z�b�g�e�[�u��(Track 0-163)
		D88SECTOR secinfo;	// �Z�N�^���
		FILE *fp;			// FILE �|�C���^
		int trkno;			// �A�N�Z�X���̃g���b�NNo
		
		D88INFO() : protect(0), type(0), size(0), fp(NULL), trkno(0)
		{
			INITARRAY( name, 0 );
			INITARRAY( reserve, 0 );
			INITARRAY( table, 0 );
		}
	};
	
	D88INFO d88;					// D88 ���
	char FileName[PATH_MAX];		// �t�@�C�����o�b�t�@
	
	bool Protected;					// �v���e�N�g�V�[��
	
	void ReadHeader88();			// D88 �w�b�_�Ǎ���
	void ReadSector88();			// D88 �Z�N�^���Ǎ���
	
public:
	cD88();							// �R���X�g���N�^
	~cD88();						// �f�X�g���N�^
	
	bool Init( char * );			// ������
	
	BYTE Get8();					// 1byte �Ǎ���
	bool Put8( BYTE );				// 1byte ������
	bool Seek( int, int = -1 );		// �V�[�N
	bool SearchSector( BYTE, BYTE, BYTE, BYTE );	// �Z�N�^��T��
	bool NextSector();				// ���̃Z�N�^�Ɉړ�����
	
	void GetID( BYTE *, BYTE *, BYTE *, BYTE * );	// ���݂�CHRN�擾
	WORD GetSecSize();				// ���݂̃Z�N�^�T�C�Y�擾
	BYTE GetSecStatus();			// ���݂̃X�e�[�^�X�擾
	
	char *GetFileName();			// �t�@�C�����擾
	char *GetDiskImgName();			// DISK�C���[�W���擾
	bool IsProtect();				// �v���e�N�g�V�[����Ԏ擾
	
	BYTE Track();					// ���݂̃g���b�N�ԍ��擾
	BYTE Sector();					// ���݂̃Z�N�^�ԍ��擾
	WORD SecNum();					// ���݂̃g���b�N���ɑ��݂���Z�N�^���擾

};

#endif	// D88_H_INCLUDED