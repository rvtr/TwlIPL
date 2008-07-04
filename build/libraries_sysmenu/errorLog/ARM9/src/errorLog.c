/*---------------------------------------------------------------------------*
  Project:  TwlIPL - ErrorLog
  File:     errorLog.c

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

// Fatal error��������"nand:/sys/log/sysmenu.log"�Ƀ��O��f�����߂̃��C�u����
//


#include <twl.h>
#include <nitro/fs.h>
#include <string.h>
#include <sysmenu.h>
#include <sysmenu/errorLog.h>


#define ERRORLOG_DIRECTORYPATH	"nand:/sys/log"
#define ERRORLOG_FILEPATH		"nand:/sys/log/sysmenu.log"
#define ERRORLOG_BAR			"======================"
#define ERRORLOG_WRITE_FORMAT	"#%02lu-%02lu-%02lu[%3s] %02lu:%02lu:%02lu  ErrorCode: %llu\n%s \n"ERRORLOG_BAR"\n\0"
#define ERRORLOG_READ_FORMAT	"#%2lu-%2lu-%2lu[%3s] %2lu:%2lu:%2lu  ErrorCode: %llu\n%*s \n"ERRORLOG_BAR"\n\0"

#define ERRORLOG_SIZE			( 16 * 1024 )	// �t�@�C����16KB�T�C�Y�Œ�
#define ERRORLOG_BUFSIZE		128				// ��Ԓ������O�̃G���[�ł�128���ȓ��Ɏ��܂�
#define ERRORLOG_NUM_ENTRY		( ERRORLOG_SIZE / ERRORLOG_BUFSIZE ) // ���O�ɏ������܂��G���g���̍ő吔



// �����֐�SYSMi_CheckAndCreateDirectory�̃G���[�`�F�b�J
typedef enum CheckStatus {
	CHECK_EXIST = 0,
	CHECK_CREATE = 1,
	CHECK_FAILED = 2
} CheckStatus;
	
// ���ɏ������܂ꂽ�G���[���O��\�����邽�߂̃G���g��
typedef struct ErrorLogEntry{
	// �G���[�̃^�C���X�^���v
	u32 year;
	u32 month;
	u32 day;
	char week[4]; // �j����3�����\��
	u32 hour;
	u32 minute;
	u32 second;
	// �G���[�R�[�h
	u64 errorCode;
} ErrorLogEntry;

// ���O�G���[�̃G���g��������

/*-- function prototype ----------------------*/
CheckStatus ELi_CheckAndCreateDirectory( const char *path );
CheckStatus ELi_CheckAndCreateFile( FSFile *file, const char *path );
int ELi_ReadEntry( FSFile *file, ErrorLogEntry *entry );
BOOL ELi_SetString( char *buf, ErrorLogEntry *entry );
BOOL ELi_WriteLog( FSFile *file ,ErrorLogEntry *entry, int num, u64 err );
void ELi_fillSpace( char *buf, int bufsize );

static char *s_strWeek[7];
static char *s_strError[FATAL_ERROR_MAX];

/*---------------------------------------------------------------------------*
  Name:         EL_WriteErrorLog

  Description:  nand:/sys/log/sysmenu.log�ɃG���[���O���o�͂��܂��B
  

  Arguments:    errorCode:	���������G���[�̃G���[�R�[�h

  Returns:      �������݂ɐ��������Ƃ���TRUE���A���s�����Ƃ���FALSE��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
BOOL EL_WriteErrorLog( u64 errorCode )
{
	FSFile file;
	ErrorLogEntry entry[ERRORLOG_NUM_ENTRY];
	int numEntry = 0;
	
	FS_InitFile( &file );	

	if( errorCode >= FATAL_ERROR_MAX )
	{
		// �C���[�K���ȃG���[�R�[�h
		OS_TPrintf("EL Error: Illigal error code (%d)\n", errorCode);
		return FALSE;
	}
	
	if( !FS_IsAvailable() )
	{
		// FS��Init����ĂȂ�������Init����
		FS_Init( FS_DMA_NOT_USE );
	}
	
	if( ELi_CheckAndCreateDirectory( ERRORLOG_DIRECTORYPATH ) == CHECK_FAILED )
	{
		return FALSE;
	}
	
	switch ( ELi_CheckAndCreateFile( &file, ERRORLOG_FILEPATH ) )
	{
		case CHECK_FAILED:
			return FALSE;
			break;
			
		case CHECK_EXIST:
			// ���Ƀ��O�t�@�C�������݂��Ă�����A�������烍�O��ǂݏo��
			numEntry = ELi_ReadEntry( &file, entry );
			break;
			
		case CHECK_CREATE:
			// �V�K�Ƀt�@�C�������ꂽ�Ȃ牽�����Ȃ��Ă���
			break;
	}
	
	
	if( !ELi_WriteLog( &file, entry, numEntry+1 , errorCode ) )
	{
		return FALSE;
	}
		
	return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         ELi_CheckAndCreateDirectory

  Description:  ���̊֐��͊Y���f�B���N�g�������݂��Ă���Ή������܂���B
				�Y���f�B���N�g�������݂��Ă��Ȃ������ꍇ��
				�f�B���N�g�����쐬���܂��B

  Arguments:    path:		�`�F�b�N���s���f�B���N�g���̃p�X

  Returns:      �f�B���N�g�������݂����ꍇ��CHECK_EXIST���A
  				���݂��Ă��炸�쐬�����ꍇ��CHECK_CREATE���A
  				�f�B���N�g���쐬�Ɏ��s�����ꍇ��CHECK_FAILED��Ԃ��܂��B
 *---------------------------------------------------------------------------*/

CheckStatus ELi_CheckAndCreateDirectory( const char *path )
{
	FSFile dir;

	FS_InitFile( &dir );
	
	if( FS_OpenDirectory( &dir, path, FS_FILEMODE_RW ) )
	{
		// �f�B���N�g�������݂��Ă����炻�̂܂�Close���Ė߂�
		FS_CloseDirectory( &dir );
		return CHECK_EXIST;
	}

	// �f�B���N�g�������݂��Ȃ��̂Ńf�B���N�g�����쐬
	if( ! FS_CreateDirectory( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("EL Error: FS_CreateDirectory() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		// �f�B���N�g���쐬�Ɏ��s
		return CHECK_FAILED;
	}

	// �f�B���N�g���쐬�ɐ���
	return CHECK_CREATE;
}

/*---------------------------------------------------------------------------*
  Name:         ELi_CheckAndCreateFile

  Description:  ���̊֐��͊Y���t�@�C�������݂��Ă���Ή������܂���B
				�Y���t�@�C�������݂��Ă��Ȃ������ꍇ�̓t�@�C�����쐬���܂��B

  Arguments:    file:		FSFile�\���̂ւ̃|�C���^
  				path:		�`�F�b�N���s���t�@�C���̃p�X

  Returns:      �t�@�C�������݂����ꍇ��CHECK_EXIST���A
  				���݂��Ă��炸�쐬�����ꍇ��CHECK_CREATE���A
  				�t�@�C���쐬�Ɏ��s�����ꍇ��CHECK_FAILED��Ԃ��܂��B
 *---------------------------------------------------------------------------*/

CheckStatus ELi_CheckAndCreateFile( FSFile *file, const char *path )
{

	if( FS_OpenFileEx( file, path, FS_FILEMODE_RWL ) )
	{
		return CHECK_EXIST;
	}

	// �t�@�C�������݂��Ȃ��̂ō쐬����	
	if( !FS_CreateFile( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("EL Error: FS_CreateFile() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		// �t�@�C���쐬�Ɏ��s
		return CHECK_FAILED;
	}


	// �t�@�C���쐬�ɐ���
	if( !FS_OpenFileEx( file, path, FS_FILEMODE_RW ) )
	{
		// �쐬�����t�@�C����open�ł��Ȃ������ꍇ
		OS_TPrintf("EL Error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		return CHECK_FAILED;
	}
	
	if( FS_SetFileLength( file, ERRORLOG_SIZE ) != FS_RESULT_SUCCESS )
	{
		// �쐬�����t�@�C���̃T�C�Y��ݒ�ł��Ȃ�����
		OS_TPrintf("EL Error: FS_SetFileLength() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		FS_CloseFile( file );
		return CHECK_FAILED; 
	}

	// �T�C�Y�ύX���I�������A�O�̂��߃t�@�C���T�C�Y�ύX�s��RWL���[�h�ŊJ���Ȃ����Ă���
	FS_CloseFile( file );

	if( !FS_OpenFileEx( file, path, FS_FILEMODE_RWL ) )
	{
		OS_TPrintf("EL Error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		return CHECK_FAILED;
	}

	
	// �t�@�C���̃[�����߂��s��
	{
		char nullbuf[1024];
		int i;

		MI_CpuClear8( nullbuf, 1024);
		
		for(i = 0; i < 16; i++)
		{
			FS_WriteFile( file, nullbuf, 1024);
		}
	}
	
	return CHECK_CREATE;
	
}



/*---------------------------------------------------------------------------*
  Name:         ELi_ReadEntry

  Description:  ���O�t�@�C���ɏ������܂ꂽ�ߋ��̃G���g����ǂݏo��

  Arguments:    file:		���O�t�@�C����FSFile�\����
  				entry:		�G���g�����������ރo�b�t�@�ւ̃|�C���^

  Returns:      �ǂݏo�����G���g���̐�
 *---------------------------------------------------------------------------*/

int ELi_ReadEntry( FSFile *file, ErrorLogEntry *entry )
{
	char buf[ERRORLOG_BUFSIZE+1];
	int numEntry = 0;
	
	buf[ERRORLOG_BUFSIZE] = '\0';
	
	FS_SeekFileToBegin( file );
	FS_ReadFile( file, buf, ERRORLOG_BUFSIZE );

	// �G���g���̓��ɂ͕K��'#'���������܂�Ă���̂ł���Ŕ���	
	while( buf[0] == '#' )
	{
		// ���߂�ꂽ�t�@�C���t�H�[�}�b�g����G���g���ɓǂݍ���
		STD_TSScanf( buf, ERRORLOG_READ_FORMAT, 
					&(entry[numEntry].year) ,
					&(entry[numEntry].month) ,
					&(entry[numEntry].day) ,
					&(entry[numEntry].week) ,
					&(entry[numEntry].hour) ,
					&(entry[numEntry].minute) ,
					&(entry[numEntry].second) ,
					&(entry[numEntry].errorCode)  );

		numEntry++;

		FS_ReadFile( file, buf, ERRORLOG_BUFSIZE );

	}
	
	return numEntry;
}



/*---------------------------------------------------------------------------*
  Name:         ELi_SetString

  Description:  �o�b�t�@�ɏ������ނׂ����O�f�[�^���Z�b�g���܂��B

  Arguments:    buf:	��������Z�b�g����o�b�t�@�ւ̃|�C���^
  				entry:	�G���[���e�̃G���g��

  Returns:      ���������ꍇ��TRUE�A���s�����ꍇ��FALSE���Ԃ�܂��B
 *---------------------------------------------------------------------------*/

BOOL ELi_SetString( char *buf, ErrorLogEntry *entry )
{
	STD_TSNPrintf(buf, ERRORLOG_BUFSIZE, ERRORLOG_WRITE_FORMAT, 
					entry->year, entry->month, entry->day, entry->week,
					entry->hour, entry->minute, entry->second,
					entry->errorCode, s_strError[entry->errorCode] );
	
	// �]����X�y�[�X�Ŗ��߂āA���s�ŏI�[����
	ELi_fillSpace( buf, ERRORLOG_BUFSIZE );
	buf[ ERRORLOG_BUFSIZE-1 ] = '\n';
	
	//OS_TPrintf("set String...\n%s", buf );
				
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ELi_WriteLog

  Description:  �󂯎�����G���g���̒��g�����O�t�@�C���ɏ����o���܂��B

  Arguments:    file:	���O�t�@�C����FSFile�\���̂ւ̃|�C���^
  				entry:	�G���[���e�̃G���g���̔z��|�C���^
  				num:	entry�Ɋ܂܂��G���g���̐�
  				err:	���񔭐������G���[�̃G���[�R�[�h

  Returns:      ���������ꍇ��TRUE�A���s�����ꍇ��FALSE���Ԃ�܂��B
 *---------------------------------------------------------------------------*/

BOOL ELi_WriteLog( FSFile *file ,ErrorLogEntry *entry, int num, u64 err )
{
	RTCDate date;
	RTCTime time;
	RTCResult rtcRes;
	
	int entryIdx = 0;
	char buf[ERRORLOG_BUFSIZE];

	// �t�@�C�����`�F�b�N
	if( num >= ERRORLOG_NUM_ENTRY ) 
	{
		entryIdx++;
	}
	
	// �t�@�C���̓��ɖ߂��ď������݂Ȃ���
	FS_SeekFileToBegin( file );
	
	for( ; entryIdx < num - 1; entryIdx++ )
	{
		ELi_SetString( buf, &entry[entryIdx] );
		
		if( FS_WriteFile( file, buf, (s32)ERRORLOG_BUFSIZE ) == -1 )
		{
			OS_TPrintf("EL Error: FS_WriteFile() failed. entry: %d\n", entryIdx );
			return FALSE;
		}
	}
	
	// �Ō�̈�͎��O��RTC���擾���ď�������
	if( ( rtcRes = RTC_GetDateTime( &date, &time )) != RTC_RESULT_SUCCESS)
	{
		OS_TPrintf("EL Error: RTC getDateTime() Failed!  Status:%d\n", rtcRes);
		return FALSE;
	}
	
	snprintf( buf, ERRORLOG_BUFSIZE, ERRORLOG_WRITE_FORMAT, 
				date.year, date.month, date.day, s_strWeek[ date.week ], 
				time.hour, time.minute, time.second,
				err, s_strError[ err ] );
	
	// �G���g����̃T�C�Y����������128�o�C�g�Œ�ɂ��邽�߂ɗ]�������󔒂Ŗ��߂�
	// \0�ŏI�[����
	ELi_fillSpace( buf, ERRORLOG_BUFSIZE );
	buf[ERRORLOG_BUFSIZE-1] = '\0';
	
	// �Ō�̃G���g�����t�@�C���ɏ����o��
	if( FS_WriteFile( file, buf, (s32)strlen(buf) ) == -1 )
	{
		OS_TPrintf("EL Error: FS_WriteFile() failed.\n" );
		return FALSE;
	}

	// �t�@�C���̗]���0���߂���
	// open mode���T�C�Y�Œ�Ȃ̂Ńt�@�C���I�[���C�ɂ�����������
	MI_CpuClear8( buf, ERRORLOG_BUFSIZE );
	while ( FS_WriteFile( file, buf, (s32) ERRORLOG_BUFSIZE ) == ERRORLOG_BUFSIZE ) {};

	
	if( !FS_CloseFile( file ) )
	{
		OS_TPrintf("EL Error: FS_CloseFile() failed.\n" );
		return FALSE;
	}
	
	
	return TRUE;
}

void ELi_fillSpace( char *buf, int bufsize )
{
	// �G���g���̖����ɃX�y�[�X������
	// ��̃G���g�������傤��128�o�C�g�ɂȂ�悤�ɒ�������킹��
	u32 length = strlen( buf );
	MI_CpuFill8( &buf[length], ' ', bufsize - length );	
}

static char *s_strWeek[] = {
	"SUN",
	"MON",
	"TUE",
	"WED",
	"THU",
	"FRI",
	"SAT"
};

static char *s_strError[] = {
	"FATAL_ERROR_UNDEFINED",
	"FATAL_ERROR_NAND",
	"FATAL_ERROR_HWINFO_NORMAL",
	"FATAL_ERROR_HWINFO_SECURE",
	"FATAL_ERROR_TWLSETTINGS",
	"FATAL_ERROR_SHARED_FONT",
	"FATAL_ERROR_WLANFIRM_AUTH",
	"FATAL_ERROR_WLANFIRM_LOAD",
	"FATAL_ERROR_TITLE_LOAD_FAILED",
	"FATAL_ERROR_TITLE_POINTER_ERROR",
	"FATAL_ERROR_AUTHENTICATE_FAILED",
	"FATAL_ERROR_ENTRY_ADDRESS_ERROR",
	"FATAL_ERROR_TITLE_BOOTTYPE_ERROR",
	"FATAL_ERROR_SIGN_DECRYPTION_FAILED",
	"FATAL_ERROR_SIGN_COMPARE_FAILED",
	"FATAL_ERROR_HEADER_HASH_CALC_FAILED",
	"FATAL_ERROR_TITLEID_COMPARE_FAILED",
	"FATAL_ERROR_VALID_SIGN_FLAG_OFF",
	"FATAL_ERROR_CHECK_TITLE_LAUNCH_RIGHTS_FAILED",
	"FATAL_ERROR_MODULE_HASH_CHECK_FAILED",
	"FATAL_ERROR_MODULE_HASH_CALC_FAILED",
	"FATAL_ERROR_MEDIA_CHECK_FAILED",
	"FATAL_ERROR_DL_MAGICCODE_CHECK_FAILED",
	"FATAL_ERROR_DL_SIGN_DECRYPTION_FAILED",
	"FATAL_ERROR_DL_HASH_CALC_FAILED",
	"FATAL_ERROR_DL_SIGN_COMPARE_FAILED",
	"FATAL_ERROR_WHITELIST_INITDB_FAILED",
	"FATAL_ERROR_WHITELIST_NOTFOUND",
	"FATAL_ERROR_DHT_PHASE1_FAILED",
	"FATAL_ERROR_DHT_PHASE2_FAILED",
	"FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF",
	"FATAL_ERROR_TWL_BOOTTYPE_UNKNOWN",
	"FATAL_ERROR_NTR_BOOTTYPE_UNKNOWN",
	"FATAL_ERROR_PLATFORM_UNKNOWN",
	"FATAL_ERROR_LOAD_UNFINISHED"
};