/*
	NEC PC-6001 Emulator 'yaPC-6001'

	Author : tanam
	Date   : 2013.07.15-

	[ system port ]
*/

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>

#include "../i8255.h"
#include "../upd765a.h"

#include "system.h"

#ifdef _WINDOWS
#include <shlwapi.h>
#endif

#ifdef Q_OS_WIN32
#include <shlwapi.h>
#endif

#ifdef ANDROID
#define TEXT QString
void ZeroMemory(void* p, int s) {
	memset(p, 0, s);
}

int fopen_s(FILE **fp, char *path, char *mode) {
	if (NULL != (*fp=fopen(path, mode))) return 0;
	return -1;
}

bool StrCmp(QString a, QString b) {
	return (a==b);
}

QString StrToUpper(QString str) {
	return str.toUpper();
}

QString PathFindExtension(QString name) {
	QFileInfo fi(name);
	return (fi.completeSuffix());
}

void PathRemoveExtension(char *path) {
	char *str=strchr(path, '.');
	if (str != NULL) *str=0;
}

void PathAddExtension(char *path, QString ext) {
	QString tmp=QString(path)+ext;
	memcpy(path, tmp.toStdString().c_str(), tmp.length());
}

#endif

char	DISK_LOG[1024];
char	disk_path1[1024];
char	disk_path2[1024];
char	disk_path3[1024];
DSK60 *dsk;

// �C�x���gID
// --- mini FDD ---
#define EID_INIT1	(1)		// 00h �C�j�V�����C�Y(�h���C�u1)
#define EID_INIT2	(2)		// 00h �C�j�V�����C�Y(�h���C�u2)
#define EID_WRDATEX	(21)	// 01h ���C�g �f�[�^���s
#define EID_RDDATEX	(22)	// 02h ���[�h �f�[�^���s
#define EID_GETPAR	(30)	// �p�����[�^��M

//************* Wait (us) *************
// --- mini FDD ---
// ���̕� �悭������Ȃ̂Œ��Ă��Ɓ[
#define WFDD_INIT			(500000)	// 00h �C�j�V�����C�Y
#define WFDD_WRDAT			(100)		// 01h ���C�g �f�[�^
#define WFDD_RDDAT			(100)		// 02h ���[�h �f�[�^
#define WFDD_SDDAT			(100)		// 03h �Z���h �f�[�^
#define WFDD_COPY			(100)		// 04h �R�s�[
#define WFDD_FORMAT			(100)		// 05h �t�H�[�}�b�g
#define WFDD_SDRES			(100)		// 06h �Z���h ���U���g �X�e�[�^�X
#define WFDD_SDDRV			(100)		// 07h �Z���h �h���C�u �X�e�[�^�X
#define WFDD_TRN			(100)		// 11h �g�����X�~�b�g
#define WFDD_RCV			(100)		// 12h ���V�[�u
#define WFDD_LOAD			(100)		// 14h ���[�h
#define WFDD_SAVE			(100)		// 15h �Z�[�u
#define WFDD_GETPAR			(100)		// �p�����[�^��M
#define WFDD_SEEK			(13000)		// �Ƃ肠����SRT=13

// create d88 from pp31
void p31_d88(FILE *fp_p31, FILE *fp_d88, char *name);

char *StrToUpper(char *s)
{
    char *p;  
 
    for (p = s; *p; p++) *p = toupper(*p);
	return s;
}

void conv(char *name)
{
	if (!name || !name[0]) return;

	FILE *fp_p31,  *fp_d88;
	char path[MAX_PATH], d88name[MAX_PATH];

	if (StrCmp(StrToUpper(PathFindExtension(name)), TEXT(".P31")) != 0) {
///		MessageBox(NULL, TEXT("Drop .P31 file."), TEXT("P31_D88"), MB_OK);
		return;
	}
	if (fopen_s(&fp_p31, (char *)name, "rb")) {
///		MessageBox(NULL, TEXT("Can't open the P31 file."), TEXT("P31_D88"), MB_OK);
		return;
	}

	strcpy(path, name);
	PathRemoveExtension(path);
	strcpy(d88name, path);
	PathAddExtension(path, TEXT(".D88"));

	if (!fopen_s(&fp_d88, (char *)path, "rb")) {
		fclose(fp_d88);
		fclose(fp_p31);
		return;
	}
	if (fopen_s(&fp_d88, (char *)path, "wb")) {
///		MessageBox(NULL, TEXT("Can't create D88 file."), TEXT("P31_D88"), MB_OK);
		fclose(fp_p31);
		return;
	}

	p31_d88(fp_p31, fp_d88, d88name);

///	MessageBox(NULL, TEXT("the D88 image created."), TEXT("P31_D88"), MB_OK);
	fclose(fp_d88);
	fclose(fp_p31);
}

// create d88 file from p31
void p31_d88(FILE *fp_p31, FILE *fp_d88, char *name)
{	
	int trk, sec, i;
	unsigned long dsize;

	// d88 header
	for (i=0; i<16; i++) fputc(0, fp_d88);
	fputc(0, fp_d88);
	for (i=0; i<9; i++) fputc(0, fp_d88);	// reserved
	fputc(0x00, fp_d88);					// write protect no
	fputc(0x00, fp_d88);					// disk type = 2D
	dsize = 32+4*164+(16+256)*16*80;		// disk size	
	fputc(dsize&0xff, fp_d88);
	fputc((dsize>>8)&0xff, fp_d88);
	fputc((dsize>>16)&0xff, fp_d88);
	fputc((dsize>>24)&0xff, fp_d88);

	// pointers to tracks
	// 0 - 79 track
	for (trk=0; trk<80; trk++) {
		unsigned long val = 32+4*164+(16+256)*16*trk;
		fputc(val&0xff, fp_d88);
		fputc((val>>8)&0xff, fp_d88);
		fputc((val>>16)&0xff, fp_d88);
		fputc((val>>24)&0xff, fp_d88);
	}
	// 80 - 163 track
	for (trk=80; trk<164; trk++) {
		fputc(0, fp_d88);
		fputc(0, fp_d88);
		fputc(0, fp_d88);
		fputc(0, fp_d88);
	}
	// track data
	for (trk=0; trk<80; trk++) {
		for (sec=1; sec<17; sec++) {
			// sector header
			fputc(trk>>1, fp_d88);					// C (cylinder)
			fputc(trk&1, fp_d88);					// H (side)
			fputc(sec, fp_d88);						// R (sector)
			fputc(1, fp_d88);						// N (256 bytes/sector)
			fputc(16, fp_d88);						// 16 sectors/track
			fputc(0, fp_d88);
			fputc(0, fp_d88);						// double density
			fputc(0, fp_d88);						// deleted mark
			fputc(0, fp_d88);						// status
			for (i=0; i<5; i++) fputc(0, fp_d88);	// reserved
			fputc(0, fp_d88); fputc(1, fp_d88);	// data size of sector part
			{
				unsigned char buff[256];

				fread(buff, 1, 256, fp_p31);
				fwrite(buff, 1, 256, fp_d88);
				fread(buff, 1, 256, fp_p31);
			}
		}
	}
}

////////////////////////////////////////////////////////////////
// �f�B�X�N ���N���X
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// �R���X�g���N�^
////////////////////////////////////////////////////////////////
DSK6::DSK6() : DrvNum(0)
{
	for( int i=0; i<MAXDRV; i++ ){
		ZeroMemory( FilePath[i], PATH_MAX );
		Dimg[i] = NULL;
		Sys[i]  = false;
	}
}


////////////////////////////////////////////////////////////////
// �f�X�g���N�^
////////////////////////////////////////////////////////////////
DSK6::~DSK6( void )
{
	for( int i=0; i<DrvNum; i++ )
		if( Dimg[i] ) Unmount( i );
}


////////////////////////////////////////////////////////////////
// �C�x���g�R�[���o�b�N�֐�
//
// ����:	id		�C�x���gID
//			clock	�N���b�N
// �Ԓl:	�Ȃ�
////////////////////////////////////////////////////////////////
void DSK6::EventCallback( int id, int clock ){}


////////////////////////////////////////////////////////////////
// �E�F�C�g�J�E���^���Z�b�g
////////////////////////////////////////////////////////////////
void DSK6::ResetWait( void )
{
	waitcnt = 0;
}


////////////////////////////////////////////////////////////////
// �E�F�C�g�J�E���^���Z
////////////////////////////////////////////////////////////////
void DSK6::AddWait( int w )
{
	waitcnt += w;
}


////////////////////////////////////////////////////////////////
// �E�F�C�g�ݒ�
////////////////////////////////////////////////////////////////
bool DSK6::SetWait( int eid )
{
	PRINTD( DISK_LOG, "[DISK][SetWait] %dus ->", waitcnt );
	
	if( waitcnt ) { /// && vm->evsc->Add( this, eid, waitcnt, EV_US ) ){
		waitcnt = 0;
		PRINTD( DISK_LOG, "OK\n" );
		return true;
	}else{
		waitcnt = 0;
		PRINTD( DISK_LOG, "FALSE\n" );
		return false;
	}
}


////////////////////////////////////////////////////////////////
// DISK �}�E���g
////////////////////////////////////////////////////////////////
bool DSK6::Mount( int drvno, char *filename )
{
	PRINTD( DISK_LOG, "[DISK][Mount] Drive : %d\n", drvno );
	
	if( drvno >= DrvNum ) return false;
	
	// �����}�E���g�ς݂ł���΃A���}�E���g����
	if( Dimg[drvno] ) Unmount( drvno );
	
	// �f�B�X�N�C���[�W�I�u�W�F�N�g���m��
	try{
		Dimg[drvno] = new cD88;
		if( !Dimg[drvno]->Init( filename ) ) return false; /// throw Error::DiskMountFailed;
	}
///	catch( std::bad_alloc ){	// new �Ɏ��s�����ꍇ
///		Error::SetError( Error::MemAllocFailed );
///		return false;
///	}
	catch(...){	// ��O����
///	catch( Error::Errno i ){	// ��O����
///		Error::SetError( i );
		
		Unmount( drvno );
		return false;
	}
	
	// �t�@�C���p�X�ۑ�
	strncpy( FilePath[drvno], filename, PATH_MAX );
	
	// �V�X�e���f�B�X�N�`�F�b�N
	Dimg[drvno]->Seek( 0 );
	if( Dimg[drvno]->Get8() == 'S' &&
		Dimg[drvno]->Get8() == 'Y' &&
		Dimg[drvno]->Get8() == 'S' )
			Sys[drvno] = true;
	else
			Sys[drvno] = false;
	Dimg[drvno]->Seek( 0 );	// �O�̂���
	
	return true;
}


////////////////////////////////////////////////////////////////
// DISK �A���}�E���g
////////////////////////////////////////////////////////////////
void DSK6::Unmount( int drvno )
{
	PRINTD( DISK_LOG, "[DISK][Unmount] Drive : %d\n", drvno );
	
	if( drvno >= DrvNum ) return;
	
	if( Dimg[drvno] ){
		// �f�B�X�N�C���[�W�I�u�W�F�N�g���J��
		delete Dimg[drvno];
		Dimg[drvno] = NULL;
		*FilePath[drvno] = '\0';
		Sys[drvno] = false;
	}
}


////////////////////////////////////////////////////////////////
// �h���C�u���擾
////////////////////////////////////////////////////////////////
int DSK6::GetDrives( void )
{
	return DrvNum;
}


////////////////////////////////////////////////////////////////
// �}�E���g�ς�?
////////////////////////////////////////////////////////////////
bool DSK6::IsMount( int drvno )
{
	if( drvno < DrvNum ) return Dimg[drvno] ? true : false;
	else                 return false;
}


////////////////////////////////////////////////////////////////
// �V�X�e���f�B�X�N?
////////////////////////////////////////////////////////////////
bool DSK6::IsSystem( int drvno )
{
	return Sys[drvno];
}


////////////////////////////////////////////////////////////////
// �v���e�N�g?
////////////////////////////////////////////////////////////////
bool DSK6::IsProtect( int drvno )
{
	if( !IsMount( drvno ) ) return false;
	
	return Dimg[drvno]->IsProtect();
}


////////////////////////////////////////////////////////////////
// �t�@�C���p�X�擾
////////////////////////////////////////////////////////////////
const char *DSK6::GetFile( int drvno )
{
	return FilePath[drvno];
}


////////////////////////////////////////////////////////////////
// DISK���擾
////////////////////////////////////////////////////////////////
const char *DSK6::GetName( int drvno )
{
	if( !IsMount( drvno ) ) return "";
	
	return Dimg[drvno]->GetDiskImgName();
}

////////////////////////////////////////////////////////////////
// �~�j�t���b�s�[�f�B�X�N�N���X
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// �R���X�g���N�^
////////////////////////////////////////////////////////////////
DSK60::DSK60() :
 io_D1H(0), io_D2H(0x08)
{
	INITARRAY( RBuf, 0 );
	INITARRAY( WBuf, 0 );
}


////////////////////////////////////////////////////////////////
// �f�X�g���N�^
////////////////////////////////////////////////////////////////
DSK60::~DSK60( void ){}


////////////////////////////////////////////////////////////////
// �C�x���g�R�[���o�b�N�֐�
//
// ����:	id		�C�x���gID
//			clock	�N���b�N
// �Ԓl:	�Ȃ�
////////////////////////////////////////////////////////////////
void DSK60::EventCallback( int id, int clock )
{
	switch( id ){
	case EID_INIT1:		// 00h �C�j�V�����C�Y(�h���C�u1)
		PRINTD( DISK_LOG, "<< [DISK][EventCallback] EID_INIT1 >>\n" );
		if( DrvNum > 1 ){
			mdisk.busy = 2;
			// �E�F�C�g���Z
			DSK6::ResetWait();
			DSK6::AddWait( WFDD_INIT );
			DSK6::SetWait( EID_INIT2 );
			break;
		}
	case EID_INIT2:		// 00h �C�j�V�����C�Y(�h���C�u2)
		PRINTD( DISK_LOG, "<< [DISK][EventCallback] EID_INIT2 >>\n" );
		if( !(io_D2H&0x10) ){
			// DAV�������Ă��Ȃ���Α҂�
			DSK6::ResetWait();
			DSK6::AddWait( WFDD_INIT );
			DSK6::SetWait( EID_INIT2 );
		}else{
			mdisk.busy = 0;
			mdisk.RFD  = 1;
			mdisk.DAC  = 1;
		}
		break;
		
	case EID_WRDATEX:	// 01h ���C�g �f�[�^���s
		PRINTD( DISK_LOG, "<< [DISK][EventCallback] EID_WRDATEX >>\n" );
		mdisk.busy = 0;
		mdisk.RFD  = 1;
		mdisk.DAC  = 1;
		break;
		
	case EID_RDDATEX:	// 02h ���[�h �f�[�^���s
		PRINTD( DISK_LOG, "<< [DISK][EventCallback] EID_RDDATEX >>\n" );
		mdisk.busy = 0;
		mdisk.RFD  = 1;
		mdisk.DAC  = 1;
		break;
		
	case EID_GETPAR:	// �p�����[�^��M
		PRINTD( DISK_LOG, "<< [DISK][EventCallback] EID_GETPAR >>\n" );
		mdisk.RFD = 1;
		mdisk.DAC = 1;
		break;
		
	default:;
	}
}


////////////////////////////////////////////////////////////////
// DISK���� ������
////////////////////////////////////////////////////////////////
bool DSK60::Init( int num )
{
	PRINTD( DISK_LOG, "[DISK][Init]\n" );
	
///	DrvNum = max( min( num, MAXDRV ) , 0 );
	DrvNum = num;
	Reset();
	
	return true;
}


////////////////////////////////////////////////////////////////
// ���Z�b�g
////////////////////////////////////////////////////////////////
void DSK60::Reset( void )
{
	ZeroMemory( &mdisk, sizeof( DISK60 ) );
	mdisk.command = IDLE;	// �󂯎�����R�}���h
	mdisk.retdat  = 0xff;	// port D0H ����Ԃ��l
	
	io_D1H = 0;
	io_D2H = 0x08;
}


////////////////////////////////////////////////////////////////
// �A�N�Z�X��?
////////////////////////////////////////////////////////////////
bool DSK60::InAccess( int drvno )
{
	return ( mdisk.busy == ( drvno + 1 ) ) ? true : false;
}


////////////////////////////////////////////////////////////////
// DISK���j�b�g����̃f�[�^���� (port D0H)
////////////////////////////////////////////////////////////////
BYTE DSK60::FddIn( void )
{
	PRINTD( DISK_LOG, "[DISK][FddIn]  <- " );
	
	if( mdisk.DAV && mdisk.step != 0 ){		// �R�}���h�������Ńf�[�^���L���ȏꍇ
		switch( mdisk.command ){
		case SEND_DATA:				// 03h �Z���h �f�[�^
			PRINTD( DISK_LOG, "SEND_DATA" );
			// �o�b�t�@����ǂ�
			mdisk.retdat = RBuf[mdisk.ridx++];
			if( mdisk.ridx >= mdisk.rsize ){
				mdisk.rsize   = 0;
				mdisk.ridx    = 0;
				mdisk.command = IDLE;
				mdisk.step    = 0;
			}
			break;
			
		case SEND_RESULT_STATUS:	// 06h �Z���h ���U���g �X�e�[�^�X
			PRINTD( DISK_LOG, "SEND_RESULT_STATUS" );
			//	Bit7:I/O����I���t���O
			//	Bit6:�Ǎ��݃o�b�t�@�Ƀf�[�^ �L:1 ��:0
			//	Bit5-1:-
			//	Bit0:�G���[�L:1 ��:0
			mdisk.retdat = mdisk.rsize ? 0x40 : 0;
			break;
			
		case SEND_DRIVE_STATUS:		// 07h �Z���h �h���C�u �X�e�[�^�X
			PRINTD( DISK_LOG, "SEND_DRIVE_STATUS" );
			mdisk.retdat = 0xf0;
			for( int i=DrvNum; i>0; i-- )
				mdisk.retdat |= 1<<(4+i);
			break;
			
		case 253:					// fdh �t�@�C���ꗗ
			PRINTD( DISK_LOG, "LIST_FILE" );
			// �o�b�t�@����ǂ�
			mdisk.retdat = RBuf[mdisk.ridx++];
			if( mdisk.ridx >= mdisk.rsize ){
				mdisk.rsize   = 0;
				mdisk.ridx    = 0;
				mdisk.command = IDLE;
				mdisk.step    = 0;
			}
			break;

		default:
			mdisk.retdat = 0xff;
		}
		PRINTD( DISK_LOG, "%02X\n", mdisk.retdat );
		
		return mdisk.retdat;
	}
	else{			// �f�[�^�������ȏꍇ
		
		PRINTD( DISK_LOG, "FF\n" );
		
		return 0xff;
	}
}


////////////////////////////////////////////////////////////////
// DISK���j�b�g�ւ̃R�}���h�C�f�[�^�o�� (port D1H)
////////////////////////////////////////////////////////////////
void DSK60::FddOut( BYTE dat )
{
	PRINTD( DISK_LOG, "[DISK][FddOut]    -> %02X ", dat );
	
	int eid  = EID_GETPAR;
	
	io_D1H = dat;
	mdisk.RFD = 0;
	
	DSK6::ResetWait();
	DSK6::AddWait( WFDD_GETPAR );
	
	if( mdisk.command == IDLE ){	// �R�}���h�̏ꍇ
		mdisk.command = dat;
		switch( mdisk.command ){
		case INIT:					// 00h �C�j�V�����C�Y
			PRINTD( DISK_LOG, "INIT" );
			eid  = EID_INIT1;
			DSK6::AddWait( WFDD_INIT );
			
			mdisk.busy = 1;
			break;
			
		case WRITE_DATA:			// 01h ���C�g �f�[�^
			PRINTD( DISK_LOG, "WRITE_DATA" );
			mdisk.step  = 1;
			mdisk.wsize = 0;
			break;
			
		case READ_DATA:				// 02h ���[�h �f�[�^
			PRINTD( DISK_LOG, "READ_DATA" );
			mdisk.step  = 1;
			mdisk.rsize = 0;
			mdisk.ridx  = 0;
			break;
			
		case SEND_DATA:				// 03h �Z���h �f�[�^
			PRINTD( DISK_LOG, "SEND_DATA" );
			mdisk.step = 1;
			break;
			
		case COPY:					// 04h �R�s�[
			PRINTD( DISK_LOG, "COPY" );
			break;
			
		case FORMAT:				// 05h �t�H�[�}�b�g
			PRINTD( DISK_LOG, "FORMAT" );
			break;
			
		case SEND_RESULT_STATUS:	// 06h �Z���h ���U���g �X�e�[�^�X
			PRINTD( DISK_LOG, "SEND_RESULT_STATUS" );
			mdisk.step = 1;
			break;
			
		case SEND_DRIVE_STATUS:		// 07h �Z���h �h���C�u �X�e�[�^�X
			PRINTD( DISK_LOG, "SEND_DRIVE_STATUS" );
			mdisk.step = 1;
			break;
			
		case TRANSMIT:				// 11h �g�����X�~�b�g
			PRINTD( DISK_LOG, "TRANSMIT" );
			break;
			
		case RECEIVE:				// 12h ���V�[�u
			PRINTD( DISK_LOG, "RECEIVE" );
			break;
			
		case LOAD:					// 14h ���[�h
			PRINTD( DISK_LOG, "LOAD" );
			break;
			
		case SAVE:					// 15h �Z�[�u
			PRINTD( DISK_LOG, "SAVE" );
			break;

		case 253:					// fdh �t�@�C���ꗗ
			PRINTD( DISK_LOG, "LIST_FILE" );
			mdisk.step = 1;
			break;

		case 254:					// feh �t�@�C���I��
			PRINTD( DISK_LOG, "SELECT_FILE" );
			mdisk.step = 1;
			break;

		default:
			eid = 0;
		}
	}else{					// �f�[�^�̏ꍇ
		switch( mdisk.command ){
		case WRITE_DATA:			// 01h ���C�g �f�[�^
			switch( mdisk.step ){
			case 1:	// 01h:�]���u���b�N��
				mdisk.blk   = dat; /// max( dat, 16 );
				mdisk.size  = mdisk.blk*256;
				mdisk.step++;
				break;
				
			case 2:	// 02h:�h���C�u�ԍ�-1
				mdisk.drv = dat;
				mdisk.step++;
				break;
				
			case 3:	// 03h:�g���b�N�ԍ�
				mdisk.trk = dat;
				mdisk.step++;
				break;
				
			case 4:	// 04h:�Z�N�^�ԍ�
				mdisk.sct = dat;
				mdisk.step++;
				break;
				
			case 5:	// 05h:�f�[�^��������
				eid  = EID_WRDATEX;
				mdisk.busy = mdisk.drv + 1;
				
				WBuf[mdisk.wsize++] = dat;
				if( mdisk.wsize >= mdisk.size ){
					if( Dimg[mdisk.drv] ){
						// �g���b�NNo��2�{(1D->2D)
						DSK6::AddWait( abs( Dimg[mdisk.drv]->Track() - mdisk.trk*2 ) / 2 );
						Dimg[mdisk.drv]->Seek( mdisk.trk*2 );
						Dimg[mdisk.drv]->SearchSector( mdisk.trk, 0, mdisk.sct, 1 );
						// �o�b�t�@���珑����
						for( int i=0; i < mdisk.wsize; i++ )
							Dimg[mdisk.drv]->Put8( WBuf[i] );
///						DSK6::AddWait( mdisk.blk * WAIT_SECTOR(1) );
					}
					mdisk.step = 0;
				}
				break;
			}
			break;
			
		case READ_DATA:				// 02h ���[�h �f�[�^
			switch( mdisk.step ){
			case 1:	// 01h:�]���u���b�N��
				mdisk.blk  = dat; /// max( dat, 16 );
				mdisk.size = mdisk.blk*256;
				mdisk.step++;
				break;
				
			case 2:	// 02h:�h���C�u�ԍ�-1
				mdisk.drv = dat;
				mdisk.step++;
				break;
				
			case 3:	// 03h:�g���b�N�ԍ�
				mdisk.trk = dat;
				mdisk.step++;
				break;
				
			case 4:	// 04h:�Z�N�^�ԍ�
				mdisk.sct = dat;
				
				eid  = EID_RDDATEX;
				mdisk.busy = mdisk.drv + 1;
				
				if( Dimg[mdisk.drv] ){
					// �g���b�NNo��2�{(1D->2D)
					DSK6::AddWait( abs( Dimg[mdisk.drv]->Track() - mdisk.trk*2 ) / 2 );
					Dimg[mdisk.drv]->Seek( mdisk.trk*2 );
					Dimg[mdisk.drv]->SearchSector( mdisk.trk, 0, mdisk.sct, 1 );
					// �o�b�t�@�ɓǍ���
					for( mdisk.rsize = 0; mdisk.rsize < mdisk.size; mdisk.rsize++ )
						RBuf[mdisk.rsize] = Dimg[mdisk.drv]->Get8();
///					DSK6::AddWait( mdisk.blk * WAIT_SECTOR(1) );
					mdisk.ridx = 0;
				}
				mdisk.step = 0;
				break;
			}
			break;

		case 253:					// fdh �t�@�C���ꗗ
			if( mdisk.step==1 ){	// 01h:�h���C�u�ԍ�-1
				char file_path[256];

				mdisk.drv = dat;
				mdisk.blk  = 16;
				mdisk.size = 0;
#ifdef QT_VERSION
				sprintf(file_path, "/sdcard/D%d/", mdisk.drv + 1);
				QDir ffd = QDir(file_path);
				ffd.setFilter( QDir::Files );
				QStringList entries = ffd.entryList();
				for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry) {
					RBuf[mdisk.size]=1;
					strncpy((char *)RBuf+mdisk.size+1, entry->toLocal8Bit().data(), 12);
					if (NULL==strstr((char *)RBuf+mdisk.size+1, "P31")) continue; 
					*(RBuf+mdisk.size+9)='.';
					char *dot=strchr((char *)RBuf+mdisk.size+1, '.');
					*dot=0;
					*(dot+1)=0;
					*(dot+2)=0;
					*(dot+3)=0;
					*(dot+4)=0;
					*(dot+5)=0;
					*(dot+6)=0;
					*(dot+7)=0;
					*(dot+8)=0;
					*(dot+9)=0;
					mdisk.size+=9;
				}
#else
				sprintf(file_path, disk_path1, mdisk.drv + 1);
				WIN32_FIND_DATA ffd;
				HANDLE h = FindFirstFile(file_path, &ffd);
				if ( h != INVALID_HANDLE_VALUE ) {
					do {
						RBuf[mdisk.size]=1;
						strncpy((char *)RBuf+mdisk.size+1, ffd.cFileName, 8);
						*(RBuf+mdisk.size+9)='.';
						char *dot=strchr((char *)RBuf+mdisk.size+1, '.');
						*dot=0;
						*(dot+1)=0;
						*(dot+2)=0;
						*(dot+3)=0;
						*(dot+4)=0;
						*(dot+5)=0;
						*(dot+6)=0;
						*(dot+7)=0;
						*(dot+8)=0;
						*(dot+9)=0;
						mdisk.size+=9;
					} while ( FindNextFile(h, &ffd) );
					FindClose(h);
				}
#endif
				eid  = EID_RDDATEX;
				mdisk.busy = mdisk.drv + 1;
				
				mdisk.rsize = ++mdisk.size;
				mdisk.ridx = 0;
			}
			break;

		case 254:					// feh �t�@�C���I��
			switch( mdisk.step ){
			case 1:	// 01h:�h���C�u�ԍ�-1
				mdisk.drv = dat;
				mdisk.wsize = 0;
				mdisk.step++;
				break;

			case 2:	// 02h:�t�@�C����
				WBuf[mdisk.wsize++] = dat;
				if( mdisk.wsize == 8 ){
					char file_path[256];
					sprintf(file_path, disk_path2, mdisk.drv + 1, WBuf);
					conv(file_path);
					sprintf(file_path, disk_path3, mdisk.drv + 1, WBuf);
					dsk->Mount( mdisk.drv, file_path );
					mdisk.step = 0;
				}
			}
			break;
		}
	}
	
	PRINTD( DISK_LOG, "\n" );
	
	// �E�F�C�g�ݒ�
	if( eid ) {
		EventCallback( eid , 0);
///		DSK6::SetWait( eid );
	}
}


////////////////////////////////////////////////////////////////
// DISK���j�b�g����̐���M������ (port D2H)
////////////////////////////////////////////////////////////////
BYTE DSK60::FddCntIn( void )
{
	PRINTD( DISK_LOG, "[DISK][FddCntIn]  <- %02X %s %s %s\n",
						(io_D2H&0xf0) | 0x08 | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV,
						mdisk.DAC ? "DAC" : "", mdisk.RFD ? "RFD" : "", mdisk.DAV ? "DAV" : "" );
	
	io_D2H = (io_D2H&0xf0) | 0x08 | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
	return io_D2H;
}


////////////////////////////////////////////////////////////////
// DISK���j�b�g�ւ̐���M���o�� (port D3H)
////////////////////////////////////////////////////////////////
void DSK60::FddCntOut( BYTE dat )
{
	PRINTD( DISK_LOG, "[DISK][FddCntOut] -> %02X ", dat );
	
	if( dat&0x80 ){		// �ŏ��bit�`�F�b�N
						// 1�̏ꍇ��8255�̃��[�h�ݒ�Ȃ̂Ŗ���(�K�����[�h0�Ɖ��肷��)
		PRINTD( DISK_LOG, "8255 mode set\n" );
		return;
	}
	
	switch( (dat>>1)&0x07 ){
	case 7:	// bit7 ATN
		PRINTD( DISK_LOG, "ATN:%d", dat&1 );
		if( (dat&1) && !(io_D2H&0x80) ){
			mdisk.RFD = 1;
			mdisk.command = IDLE;
		}
		break;
		
	case 6:	// bit6 DAC
		PRINTD( DISK_LOG, "DAC:%d", dat&1 );
		if( (dat&1) && !(io_D2H&0x40) ) mdisk.DAV = 0;
		break;
		
	case 5:	// bit5 RFD
		PRINTD( DISK_LOG, "RFD:%d", dat&1 );
		if( (dat&1) && !(io_D2H&0x20) ) mdisk.DAV = 1;
		break;
		
	case 4:	// bit4 DAV
		PRINTD( DISK_LOG, "DAV:%d", dat&1 );
		if( !(dat&1) ) mdisk.DAC = 0;
		break;
	}
	
	if( dat&1 ) io_D2H |=   1<<((dat>>1)&0x07);
	else		io_D2H &= ~(1<<((dat>>1)&0x07));
	
	PRINTD( DISK_LOG, "\n" );
}


////////////////////////////////////////////////////////////////
// I/O�A�N�Z�X�֐�
////////////////////////////////////////////////////////////////
void DSK60::OutD1H( int, BYTE data ){ FddOut( data ); }
void DSK60::OutD2H( int, BYTE data ){ io_D2H = (data&0xf0) | (io_D2H&0x0f); }
void DSK60::OutD3H( int, BYTE data ){ FddCntOut( data ); }

BYTE DSK60::InD0H( int ){ return FddIn(); }
BYTE DSK60::InD1H( int ){ return io_D1H; }
BYTE DSK60::InD2H( int ){ return FddCntIn(); }

void SYSTEM::initialize()
{
	dsk = new DSK60();
	dsk->Init(MAXDRV);
	strcpy(disk_path1, emu->bios_path(_T("D%d/*.P31")));
	strcpy(disk_path2, emu->bios_path(_T("D%d/%s.P31")));
	strcpy(disk_path3, emu->bios_path(_T("D%d/%s.D88")));
}

void SYSTEM::write_io8(uint32 addr, uint32 data)
{
	if ((addr & 0xff)==0xD1) {
		dsk->OutD1H(0xD1, data); ///	disk_out(0xD1, data);
	}
	if ((addr & 0xff)==0xD3) {
		dsk->OutD3H(0xD3, data); ///	disk_out(0xD3, data);
	}
	return;
}

uint32 SYSTEM::read_io8(uint32 addr)
{
	byte Value=0xff;
	switch(addr & 0xff) {
	case 0xD1:	
		Value= dsk->InD1H(0xD1); ///		Value= disk_inp(0xD1);	
		break;
	case 0xD2:	
		Value= dsk->InD2H(0xD2); ///		Value= disk_inp(0xD2);	
		break;
	case 0xD0:	
		Value= dsk->InD0H(0xD0); ///		Value= 	disk_inp(0xD0);	
		break;
	}
	if (0x09d2==addr) {
		Value=0x0f;
	}
	return(Value);
}

void SYSTEM::open_disk(int drv, _TCHAR* file_path, int offset)
{
	dsk->Mount( drv, file_path );
}

void SYSTEM::close_disk(int drv)
{
	dsk->Unmount(drv);
}
