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


#include <twl.h>
#include <nitro/fs.h>
#include <string.h>
#include <sysmenu.h>
#include <sysmenu/errorLog.h>


#define ERRORLOG_DIRECTORYPATH	"nand:/sys/log"
#define ERRORLOG_FILEPATH		"nand:/sys/log/sysmenu.log"

#define ERRORLOG_HEADER_SIZE		6
#define ERRORLOG_WRITE_FORMAT_RED1	"\n\n#RED %02u-%02u-%02u[%3s] %02u:%02u:%02u\n"
#define ERRORLOG_WRITE_FORMAT_RED2	"title: %04s ErrorCode: %u\n%s"
#define ERRORLOG_WRITE_FORMAT1		"\n\n#FFT %02u-%02u-%02u[%3s] %02u:%02u:%02u\n"
#define ERRORLOG_WRITE_FORMAT2		"title: %04s\n%s"
#define ERRORLOG_READ_FORMAT_RED1	"\n\n#RED %d-%d-%d[%3s] %d:%d:%d\n"
#define ERRORLOG_READ_FORMAT_RED2	"title: %4s ErrorCode: %u\n%*s"
#define ERRORLOG_READ_FORMAT1		"\n\n#FFT %d-%d-%d[%3s] %d:%d:%d\n"
#define ERRORLOG_READ_FORMAT2		"title: %4s\n%s"

#define ERRORLOG_WRITE_FORMAT		ERRORLOG_WRITE_FORMAT1 ERRORLOG_WRITE_FORMAT2
#define ERRORLOG_WRITE_FORMAT_RED	ERRORLOG_WRITE_FORMAT_RED1 ERRORLOG_WRITE_FORMAT_RED2
#define ERRORLOG_READ_FORMAT		ERRORLOG_READ_FORMAT1 ERRORLOG_READ_FORMAT2
#define ERRORLOG_READ_FORMAT_RED	ERRORLOG_READ_FORMAT_RED1 ERRORLOG_READ_FORMAT_RED2

#define ERRORLOG_NUM_ARGS			9

#define ERRORLOG_STR_OFFSET		42



// �����֐�SYSMi_CheckAndCreateDirectory�̃G���[�`�F�b�J
typedef enum CheckStatus {
	CHECK_EXIST = 0,
	CHECK_CREATE = 1,
	CHECK_FAILED = 2
} CheckStatus;
	

/*-- global variables ----------------------*/

ErrorLogWork elWork;
static BOOL isInitialized = FALSE;

/*-- function prototype ----------------------*/
CheckStatus ERRORLOGi_CheckAndCreateDirectory( const char *path );
CheckStatus ERRORLOGi_CheckAndCreateFile( const char *path );
int ERRORLOGi_ReadEntry( void );
BOOL ERRORLOGi_SetString( char *buf, ErrorLogEntry *entry );
BOOL ERRORLOGi_addNewEntryRED( int idx, RTCDate *date, RTCTime *time, int errorCode );
void ERRORLOGi_addNewEntry( int idx, RTCDate *date, RTCTime *time, const char *fmt, va_list arglist );
void ERRORLOGi_WriteLogToBuf( char *buf );
BOOL ERRORLOGi_WriteLogToFile( char *buf );
void ERRORLOGi_fillSpace( char *buf, int bufsize );
BOOL ERRORLOGi_WriteCommon( BOOL isLauncherError, u64 errorCode, const char *fmt, va_list arglist );
u32 ERRORLOGi_getTitleId( void );

static char *s_strWeek[7];
static char *s_strError[FATAL_ERROR_MAX];


/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_Init

  Description:  Errorlog���C�u�����p�̏������֐��ł��B
  				

  Arguments:    Alloc:	�������m�ۗp�̊֐��ł��B	
  				Free:	�������J���p�̊֐��ł��B

  Returns:      ���������TRUE���A���s�����FALSE��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
BOOL ERRORLOG_Init( void* (*AllocFunc) (u32) , void (*FreeFunc) (void*)  )
{
	if( isInitialized )
	{
		return FALSE ;
	}
	
	SDK_POINTER_ASSERT(AllocFunc);
    SDK_POINTER_ASSERT(FreeFunc);

	elWork.Alloc = AllocFunc;
	elWork.Free = FreeFunc;
	
	OS_InitMutex( &elWork.mutex );
	OS_LockMutex( &elWork.mutex );

	// ���O�ǂݏo���p�̃o�b�t�@���m��
	if( elWork.entry == NULL )
	{
		elWork.entry = (ErrorLogEntry*) elWork.Alloc ( sizeof (ErrorLogEntry) * ERRORLOG_NUM_ENTRY );
		MI_CpuClear8( elWork.entry, sizeof (ErrorLogEntry) * ERRORLOG_NUM_ENTRY);
	}
	
	SDK_ASSERT( elWork.entry );
	
	if( !FS_IsAvailable() )
	{
		// FS��Init����ĂȂ�������Init����
		FS_Init( FS_DMA_NOT_USE );
	}

	FS_InitFile( &elWork.file );	
	
	// �t�@�C���̑��݊m�F
	if( ERRORLOGi_CheckAndCreateDirectory( ERRORLOG_DIRECTORYPATH ) == CHECK_FAILED )
	{
		OS_UnlockMutex( &elWork.mutex );
		return FALSE;
	}
	
	// �t�@�C���̑��݂��m�F�A���łɒ��ŃI�[�v�����ăG���g���̓ǂݍ���
	switch ( ERRORLOGi_CheckAndCreateFile( ERRORLOG_FILEPATH ) )
	{
		case CHECK_FAILED:
			OS_UnlockMutex( &elWork.mutex );
			return FALSE;
			break;
			
		case CHECK_EXIST:
			// ���Ƀ��O�t�@�C�������݂��Ă�����A�������烍�O��ǂݏo��
			elWork.numEntry = ERRORLOGi_ReadEntry();
			break;
			
		case CHECK_CREATE:
			// �V�K�Ƀt�@�C�������ꂽ�Ȃ牽�����Ȃ��Ă���
			break;
	}

	FS_CloseFile( &elWork.file );
	OS_UnlockMutex( &elWork.mutex );
	isInitialized = TRUE;

	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_End

  Description:  Errorlog���C�u�����̏I���������s���܂��B
  				�ēxEL���C�u�����𗘗p���邽�߂ɂ�ErrorLog_Init���ĂԕK�v������܂��B
  				
  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
void ERRORLOG_End( void )
{
	int idx;

	for( idx = 0; idx < ERRORLOG_NUM_ENTRY ; idx++ )
	{
		if( elWork.entry[idx].errorStr != NULL )
		{
			elWork.Free( elWork.entry[idx].errorStr );
		}
	}
	
	elWork.Free( elWork.entry );

	isInitialized = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_Printf

  Description:  nand:/sys/log/sysmenu.log�ɃG���[���O���t���[�Ȍ`���ŏ������݂܂��B
  
  Arguments:    :	printf�ɏ���

  Returns:      �������݂ɐ��������Ƃ���TRUE���A���s�����Ƃ���FALSE��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
BOOL ERRORLOG_Printf( const char *fmt, ... )
{
	BOOL result = TRUE;
	va_list arglist;
	
	if( !isInitialized )
	{
		return FALSE;
	}
	
	OS_LockMutex( &elWork.mutex );
	va_start( arglist, fmt );
	result = ERRORLOGi_WriteCommon( FALSE, 0, fmt, arglist );
	va_end( arglist );
	OS_UnlockMutex( &elWork.mutex );

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_Write

  Description:  nand:/sys/log/sysmenu.log�ɃG���[���O���o�͂��܂��B
  
  Arguments:    errorCode:	���������G���[�̃G���[�R�[�h

  Returns:      �������݂ɐ��������Ƃ���TRUE���A���s�����Ƃ���FALSE��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
BOOL ERRORLOG_Write( u64 errorCode )
{
	BOOL res;
	
	if( !isInitialized )
	{
		return FALSE;
	}
	
	OS_LockMutex( &elWork.mutex );
	res = ERRORLOGi_WriteCommon( TRUE, errorCode, NULL, NULL );
	OS_UnlockMutex( &elWork.mutex );
	
	return res;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_WriteCommon

  Description:  
  
  Arguments:	isLauncherError: ERRORLOG_Write����Ă΂ꂽ�ꍇ��TRUE�A
  								 ERRORLOG_Printf����Ă΂ꂽ�ꍇ��FALSE�ƂȂ�܂��B
  				errorCode:	isLauncherError��TRUE�Ȏ��ɏ������܂��G���[�R�[�h
  				fmt, ...:		isLauncherError��FALSE�Ȏ��ɏ������܂�镶����ƈ���

  Returns:      �������݂ɐ��������Ƃ���TRUE���A���s�����Ƃ���FALSE��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
 
BOOL ERRORLOGi_WriteCommon( BOOL isLauncherError, u64 errorCode, const char *fmt, va_list arglist )
{
	int bufBeginPoint = 0; 	// �����O�o�b�t�@�̊J�n�_
	int numEntry = 0;
	int counter = 0;

	RTCDate date;
	RTCTime time;
	RTCResult rtcRes;
	
	char *writeBuf;
	
	writeBuf = (char*) elWork.Alloc( ERRORLOG_SIZE );
	SDK_ASSERT( writeBuf );

	// �V�������O�G���g�����������ނ��߂�RTC
	if( ( rtcRes = RTC_GetDateTime( &date, &time )) != RTC_RESULT_SUCCESS )
	{
		elWork.Free( writeBuf );
		OS_TPrintf("EL Error: RTC getDateTime() Failed!  Status:%d\n", rtcRes);
		return FALSE;
	}

	if( isLauncherError )
	{
		for(counter = 0; counter < FATAL_ERROR_MAX; counter++ )
		{
			if( ( errorCode >> counter ) & 0x1LL )
			{
				// �����̃r�b�g�������Ă�����G���g���ɓ���ăo�b�t�@�J�n�_��i�߂�
				ERRORLOGi_addNewEntryRED( elWork.numEntry % ERRORLOG_NUM_ENTRY , &date, &time, counter );
				elWork.numEntry++;
			}
		}
	}
	else
	{
		ERRORLOGi_addNewEntry( elWork.numEntry % ERRORLOG_NUM_ENTRY, &date, &time, fmt, arglist );
		elWork.numEntry++;
	}
	
	// �܂��G���g�������ƂɃo�b�t�@�ɏ�������
	ERRORLOGi_WriteLogToBuf( writeBuf );
	
	// �ŏI�I�Ƀt�@�C������������
	if( !ERRORLOGi_WriteLogToFile( writeBuf ) )
	{
		elWork.Free( writeBuf );
		return FALSE;
	}
	
	elWork.Free( writeBuf );
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_GetNum

  Description:  �ǂݏo�����G���g�������擾���܂��B

  Arguments:    �Ȃ��B

  Returns:      ���O�t�@�C������ǂݏo�����G���g������Ԃ��܂��B
 *---------------------------------------------------------------------------*/

int ERRORLOG_GetNum()
{
	return elWork.numEntry < ERRORLOG_NUM_ENTRY ? elWork.numEntry : ERRORLOG_NUM_ENTRY;
}

	
/*---------------------------------------------------------------------------*
  Name:         ERRORLOG_Read

  Description:  �w�肵���i���o�̃G���[���O�G���g���ւ̃|�C���^���擾���܂��B

  Arguments:    idx:	�G���g���ԍ��̎w��

  Returns:      idx�Ԗڂ̃G���g���ւ̃|�C���^�ł��B
 *---------------------------------------------------------------------------*/

const ErrorLogEntry* ERRORLOG_Read( int idx )
{
	if( idx >= 0 && idx < ERRORLOG_NUM_ENTRY )
	{
		return &elWork.entry[idx];
	}
	else
	{
		return NULL;
	}
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_addNewEntryRED

  Description:  �G���[�R�[�h��RTC�f�[�^���G���[���O�̃G���g���ɒǉ����܂��B

  Arguments:    int:		�G���g����}������C���f�N�X
  				date:		���t�f�[�^
  				time:		�����f�[�^
  				errorCode:	�G���[�R�[�h

  Returns:      FATAL_ERROR_MAX�𒴂���G���[�R�[�h���n���ꂽ�ꍇ��FALSE���A
  				����ȊO�̂Ƃ���TRUE��Ԃ��܂��B
 *---------------------------------------------------------------------------*/


BOOL ERRORLOGi_addNewEntryRED( int idx, RTCDate *date, RTCTime *time, int errorCode )
{

	if( errorCode >= FATAL_ERROR_MAX )
	{
		// �C���[�K���ȃG���[�R�[�h
		OS_TPrintf("EL Error: Illigal error code (%d)\n", errorCode);
		return FALSE;
	}

	elWork.entry[idx].isLauncherError = TRUE;
	elWork.entry[idx].isBroken = FALSE;
	elWork.entry[idx].year = (int)date->year;
	elWork.entry[idx].month = (int)date->month;
	elWork.entry[idx].day = (int)date->day;
	STD_CopyLStringZeroFill( elWork.entry[idx].week, s_strWeek[ date->week ], 4 );
	elWork.entry[idx].hour = (int)time->hour;
	elWork.entry[idx].minute = (int)time->minute;
	elWork.entry[idx].second = (int)time->second;
	elWork.entry[idx].errorCode = (int)errorCode;
	elWork.entry[idx].titleId = ERRORLOGi_getTitleId();
	
	if( elWork.entry[idx].errorStr != NULL )
	{
		// �㏑�������O��UIG���̃G���[�������ꍇ��str�p�̃o�b�t�@���J�����Ă���
		elWork.Free( elWork.entry[idx].errorStr );
	}
	
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_addNewEntry

  Description:  ���R�ȕ������RTC�f�[�^���G���[���O�G���g���ɒǉ����܂��B

  Arguments:    int:		�G���g����}������C���f�N�X
  				date:		���t�f�[�^
  				time:		�����f�[�^
  				fmt:		�G���[���O�ɏ������܂�镶����̃t�H�[�}�b�g
  				arglist:	fmt�ɑ΂������
  				

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/


void ERRORLOGi_addNewEntry( int idx, RTCDate *date, RTCTime *time, const char *fmt, va_list arglist )
{

	elWork.entry[idx].isLauncherError = FALSE;
	elWork.entry[idx].isBroken = FALSE;
	elWork.entry[idx].year = (int)date->year;
	elWork.entry[idx].month = (int)date->month;
	elWork.entry[idx].day = (int)date->day;
	STD_CopyLStringZeroFill( elWork.entry[idx].week, s_strWeek[ date->week ], 4 );
	elWork.entry[idx].hour = (int)time->hour;
	elWork.entry[idx].minute = (int)time->minute;
	elWork.entry[idx].second = (int)time->second;
	elWork.entry[idx].titleId = ERRORLOGi_getTitleId();
	
	if( elWork.entry[idx].errorStr == NULL )
	{
		// str�p�̗̈悪�������ĂȂ���Ίm�ۂ���
		elWork.entry[idx].errorStr = elWork.Alloc( ERRORLOG_BUFSIZE+1 );
		MI_CpuClear8( elWork.entry[idx].errorStr, ERRORLOG_BUFSIZE+1 );	
	}
	
	STD_TVSNPrintf( elWork.entry[idx].errorStr, ERRORLOG_BUFSIZE, fmt, arglist );
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_CheckAndCreateDirectory

  Description:  ���̊֐��͊Y���f�B���N�g�������݂��Ă���Ή������܂���B
				�Y���f�B���N�g�������݂��Ă��Ȃ������ꍇ��
				�f�B���N�g�����쐬���܂��B

  Arguments:    path:		�`�F�b�N���s���f�B���N�g���̃p�X

  Returns:      �f�B���N�g�������݂����ꍇ��CHECK_EXIST���A
  				���݂��Ă��炸�쐬�����ꍇ��CHECK_CREATE���A
  				�f�B���N�g���쐬�Ɏ��s�����ꍇ��CHECK_FAILED��Ԃ��܂��B
 *---------------------------------------------------------------------------*/

CheckStatus ERRORLOGi_CheckAndCreateDirectory( const char *path )
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
	if( ! FS_CreateDirectoryAuto( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("EL Error: FS_CreateDirectory() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		// �f�B���N�g���쐬�Ɏ��s
		return CHECK_FAILED;
	}

	// �f�B���N�g���쐬�ɐ���
	return CHECK_CREATE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_CheckAndCreateFile

  Description:  ���̊֐��͊Y���t�@�C�������݂��Ă���Ή������܂���B
				�Y���t�@�C�������݂��Ă��Ȃ������ꍇ�̓t�@�C�����쐬���܂��B

  Arguments:    path:		�`�F�b�N���s���t�@�C���̃p�X

  Returns:      �t�@�C�������݂����ꍇ��CHECK_EXIST���A
  				���݂��Ă��炸�쐬�����ꍇ��CHECK_CREATE���A
  				�t�@�C���쐬�Ɏ��s�����ꍇ��CHECK_FAILED��Ԃ��܂��B
 *---------------------------------------------------------------------------*/

CheckStatus ERRORLOGi_CheckAndCreateFile( const char *path )
{

	if( FS_OpenFileEx( &elWork.file, path, FS_FILEMODE_RWL ) )
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
	if( !FS_OpenFileEx( &elWork.file, path, FS_FILEMODE_RW ) )
	{
		// �쐬�����t�@�C����open�ł��Ȃ������ꍇ
		OS_TPrintf("EL Error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		return CHECK_FAILED;
	}
	
	if( FS_SetFileLength( &elWork.file, ERRORLOG_SIZE ) != FS_RESULT_SUCCESS )
	{
		// �쐬�����t�@�C���̃T�C�Y��ݒ�ł��Ȃ�����
		OS_TPrintf("EL Error: FS_SetFileLength() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		FS_CloseFile( &elWork.file );
		return CHECK_FAILED; 
	}

	// �T�C�Y�ύX���I�������A�O�̂��߃t�@�C���T�C�Y�ύX�s��RWL���[�h�ŊJ���Ȃ����Ă���
	FS_CloseFile( &elWork.file );

	if( !FS_OpenFileEx( &elWork.file, path, FS_FILEMODE_RWL ) )
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
			FS_WriteFile( &elWork.file, nullbuf, 1024);
		}
	}
	
	return CHECK_CREATE;
}



/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_ReadEntry

  Description:  ���O�t�@�C���ɏ������܂ꂽ�ߋ��̃G���g����ǂݏo��

  Arguments:    �Ȃ��B

  Returns:      �ǂݏo�����G���g���̐�
 *---------------------------------------------------------------------------*/

int ERRORLOGi_ReadEntry( void )
{
	char buf[ERRORLOG_BUFSIZE+1];
	char titlebuf[5];
	int numEntry = 0;
	int readSize = 0;
	
	buf[ERRORLOG_BUFSIZE] = '\0';
	
	FS_SeekFileToBegin( &elWork.file );
	readSize = FS_ReadFile( &elWork.file, buf, ERRORLOG_BUFSIZE );

	// �G���g���̓��ɂ͕K��'#'���������܂�Ă���̂ł���Ŕ���	
	while( readSize == ERRORLOG_BUFSIZE && numEntry < ERRORLOG_NUM_ENTRY)
	{
		int numArgs = 0;
		
		// ���߂�ꂽ�t�@�C���t�H�[�}�b�g����G���g���ɓǂݍ���
		if( ! STD_StrNCmp( "\n\n#RED", buf, ERRORLOG_HEADER_SIZE ) )
		{
			// �����`�����珑�����܂ꂽ�G���[�̏ꍇ

			
			elWork.entry[numEntry].isLauncherError = TRUE;
			elWork.entry[numEntry].isBroken = FALSE;
			numArgs = STD_TSScanf( buf, ERRORLOG_READ_FORMAT_RED, 
						&(elWork.entry[numEntry].year) ,
						&(elWork.entry[numEntry].month) ,
						&(elWork.entry[numEntry].day) ,
						&(elWork.entry[numEntry].week) ,
						&(elWork.entry[numEntry].hour) ,
						&(elWork.entry[numEntry].minute) ,
						&(elWork.entry[numEntry].second) ,
						titlebuf ,
						&(elWork.entry[numEntry].errorCode)  );
			
			elWork.entry[numEntry].titleId = MI_LoadLE32( titlebuf );
			
		}
		else if( !STD_StrNCmp( "\n\n#FFT", buf, ERRORLOG_HEADER_SIZE ) )
		{
			// �t���[�t�H�[�}�b�g�ŏ������܂ꂽ�G���[�̏ꍇ
			if( elWork.entry[numEntry].errorStr == NULL )
			{
				elWork.entry[numEntry].errorStr = elWork.Alloc( ERRORLOG_BUFSIZE+1 );
			}
			
			elWork.entry[numEntry].isLauncherError = FALSE;
			elWork.entry[numEntry].isBroken = FALSE;
			numArgs = STD_TSScanf( buf, ERRORLOG_READ_FORMAT, 
						&(elWork.entry[numEntry].year) ,
						&(elWork.entry[numEntry].month) ,
						&(elWork.entry[numEntry].day) ,
						&(elWork.entry[numEntry].week) ,
						&(elWork.entry[numEntry].hour) ,
						&(elWork.entry[numEntry].minute) ,
						&(elWork.entry[numEntry].second) ,
						titlebuf ,
						elWork.entry[numEntry].errorStr ); // �Ō�̈�͈������̂��܂����킹�邽��
			
			elWork.entry[numEntry].titleId = MI_LoadLE32( titlebuf );
			STD_CopyLStringZeroFill( elWork.entry[numEntry].errorStr, &buf[ERRORLOG_STR_OFFSET], ERRORLOG_STR_LENGTH + 1);

		}
		
		if ( numArgs != ERRORLOG_NUM_ARGS )
		{
			char cmpBuf[ERRORLOG_BUFSIZE+1];
			MI_CpuClear8( cmpBuf, ERRORLOG_BUFSIZE+1 );
			if( ! MI_CpuComp8( cmpBuf, buf, ERRORLOG_BUFSIZE+1 ) )
			{
				// �S���k�������������炻�̃G���g���͏������܂�Ă��Ȃ�����
				readSize = FS_ReadFile( &elWork.file, buf, ERRORLOG_BUFSIZE );
				continue;
			}
			
			// �G���[���O�����Ă��ĉ�͂ł��Ȃ������ꍇ�̏���
			// �������͌Â����O�ŏ�L�̐ړ������Ȃ����O�̏ꍇ�̏���
			if( elWork.entry[numEntry].errorStr == NULL )
			{
				elWork.entry[numEntry].errorStr = elWork.Alloc( ERRORLOG_BUFSIZE+1 );
			}
			
			elWork.entry[numEntry].isBroken = TRUE;
			STD_CopyLStringZeroFill( elWork.entry[numEntry].errorStr, buf, ERRORLOG_BUFSIZE+1 );
			
		}

		numEntry++;
		readSize = FS_ReadFile( &elWork.file, buf, ERRORLOG_BUFSIZE );
	}
	
	return numEntry;
}



/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_SetString

  Description:  �o�b�t�@�ɏ������ނׂ����O�f�[�^���Z�b�g���܂��B

  Arguments:    buf:	��������Z�b�g����o�b�t�@�ւ̃|�C���^
  				entry:	�G���[���e�̃G���g��

  Returns:      ���������ꍇ��TRUE�A���s�����ꍇ��FALSE���Ԃ�܂��B
 *---------------------------------------------------------------------------*/

BOOL ERRORLOGi_SetString( char *buf, ErrorLogEntry *entry )
{
	char titlebuf[5];
	
	if( entry->isBroken )
	{
		STD_CopyLString( buf, entry->errorStr, ERRORLOG_BUFSIZE );
		buf[ ERRORLOG_BUFSIZE-1 ] = '\n';
		return TRUE;
	}
	
	// ���ĂȂ��ꍇ��titleID���������ޕK�v������̂ŁAu32�𕶎��񉻂���
	STD_CopyLStringZeroFill( titlebuf, (char*)&entry->titleId, 5);
	
	if ( entry->isLauncherError )
	{
		STD_TSNPrintf(buf, ERRORLOG_BUFSIZE, ERRORLOG_WRITE_FORMAT_RED, 
						entry->year, entry->month, entry->day, entry->week,
						entry->hour, entry->minute, entry->second,
						titlebuf, entry->errorCode,
						s_strError[entry->errorCode] ? s_strError[entry->errorCode] : "" );
	}
	else
	{
		STD_TSNPrintf(buf, ERRORLOG_BUFSIZE, ERRORLOG_WRITE_FORMAT,
						entry->year, entry->month, entry->day, entry->week,
						entry->hour, entry->minute, entry->second,
						titlebuf, entry->errorStr );
	}
					    
	// �]����X�y�[�X�Ŗ��߂āA���s�ŏI�[����
	ERRORLOGi_fillSpace( buf, ERRORLOG_BUFSIZE );
	buf[ ERRORLOG_BUFSIZE-1 ] = '\n';
	
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_WriteLogToBuf

  Description:  �󂯎�����G���g���̒��g���o�b�t�@�֏����o���܂��B

  Arguments:    buf:		16KB���o�b�t�@�ւ̃|�C���^

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
void ERRORLOGi_WriteLogToBuf( char *buf )
{
	// �G���g���������o���J�n�_
	int entryIdx = elWork.numEntry <= ERRORLOG_NUM_ENTRY ? 0 : elWork.numEntry % ERRORLOG_NUM_ENTRY ;
	int counter;
	int counterMax = elWork.numEntry <= ERRORLOG_NUM_ENTRY ? elWork.numEntry : ERRORLOG_NUM_ENTRY ;
	
	for( counter = 0; counter < counterMax ; counter++ )
	{
		// buf�Ɉ�G���g���������񉻂��ċl�߂Ă���
		ERRORLOGi_SetString( &buf[ counter * ERRORLOG_BUFSIZE ], &(elWork.entry[ (entryIdx + counter) % ERRORLOG_NUM_ENTRY ]) );
		
		if( counter == counterMax-1 )
		{
			// �Ō�̃G���g���͉��s����ꂸ�Ƀk�������ŏI�[
			buf[ (counter+1) * ERRORLOG_BUFSIZE - 1] = '\0';
		}
	}

	// �o�b�t�@�̂��܂蕔�����[�����߂���
	MI_CpuClear8( &buf[ counter * ERRORLOG_BUFSIZE], (u32) ((ERRORLOG_NUM_ENTRY - counter) * ERRORLOG_BUFSIZE ) );
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_WriteLogToFile

  Description:  �󂯎�����G���g���̒��g�����O�t�@�C���ɏ����o���܂��B

  Arguments:    buf:	�������ݓ��e�̊i�[���ꂽ�o�b�t�@�ւ̃|�C���^

  Returns:      ���������ꍇ��TRUE�A���s�����ꍇ��FALSE���Ԃ�܂��B
 *---------------------------------------------------------------------------*/

BOOL ERRORLOGi_WriteLogToFile( char *buf )
{
	// �t�@�C���̓��ɖ߂��ď������݂Ȃ���
	FS_OpenFileEx( &elWork.file, ERRORLOG_FILEPATH, FS_FILEMODE_W );
	FS_SeekFileToBegin( &elWork.file );
	
	if( FS_WriteFile( &elWork.file, buf, ERRORLOG_SIZE ) != ERRORLOG_SIZE )
	{
		OS_TPrintf("EL Error: FS_WriteFile() failed.\n");
		FS_CloseFile( &elWork.file );
		return FALSE;
	}
	
	if( ! FS_CloseFile( &elWork.file ) )
	{
		OS_TPrintf("EL Error: FS_CloseFile() in WriteLogToFile() failed.");
		return FALSE;
	}
	
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_fillSpace

  Description:  �󂯎�����G���g���̗]��̕������X�y�[�X�Ŗ��߂܂��B

  Arguments:    buf:		�������ݓ��e�̊i�[���ꂽ�o�b�t�@�ւ̃|�C���^
  				bufsize:	�G���g���S�̂̃T�C�Y

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/

void ERRORLOGi_fillSpace( char *buf, int bufsize )
{
	u32 length = strlen( buf );
	MI_CpuFill8( &buf[length], ' ', bufsize - length );	
}

/*---------------------------------------------------------------------------*
  Name:         ERRORLOGi_getTitleId

  Description:  �N�����Ă���A�v����titleId���擾

  Arguments:    

  Returns:      �N�����Ă���A�v����titleID_Lo
 *---------------------------------------------------------------------------*/

u32 ERRORLOGi_getTitleId( void )
{
	return MI_LoadBE32( (void*)(HW_TWL_ROM_HEADER_BUF + 0x230) );

}

FSFile ERRORLOGi_getLogFilePt( void )
{
	return elWork.file;
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

static char *s_strError[ FATAL_ERROR_MAX ] = {
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
	"FATAL_ERROR_LOAD_UNFINISHED",
	"FATAL_ERROR_LOAD_OPENFILE_FAILED",
	"FATAL_ERROR_LOAD_MEMALLOC_FAILED",
	"FATAL_ERROR_LOAD_SEEKFILE_FAILED",
	"FATAL_ERROR_LOAD_READHEADER_FAILED",
	"FATAL_ERROR_LOAD_LOGOCRC_ERROR = 39",
	"FATAL_ERROR_LOAD_READDLSIGN_FAILED",
	"FATAL_ERROR_LOAD_RELOCATEINFO_FAILED",
	"FATAL_ERROR_LOAD_READMODULE_FAILED",
    "FATAL_ERROR_NINTENDO_LOGO_CHECK_FAILED",
    "FATAL_ERROR_SYSMENU_VERSION",
    "FATAL_ERROR_DHT_PHASE1_CALC_FAILED",
    "FATAL_ERROR_LOAD_UNKNOWN_BOOTTYPE",
    "FATAL_ERROR_LOAD_AUTH_HEADER_FAILED",
    "FATAL_ERROR_LOAD_NEVER_STARTED",
};