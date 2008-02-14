/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     HWInfoWriterLib.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
#include "hwi.h"

// define data------------------------------------------

// extern data------------------------------------------
const LCFGTWLHWNormalInfo *LCFG_THW_GetDefaultNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetDefaultSecureInfo( void );
const LCFGTWLHWNormalInfo *LCFG_THW_GetNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetSecureInfo( void );

// function's prototype declaration---------------------
static void ReadTWLSettings( void );
static HwiInitResult ReadPrivateKey( void );
static void ReadHWInfoFile( void );
static void VerifyHWInfo( void );
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len );

// global variable -------------------------------------

// static variable -------------------------------------
static u8 *s_pPrivKeyBuffer = NULL;
static LCFGReadResult (*s_pReadSecureInfoFunc)( void );
static BOOL s_isReadTSD;
static void *(*spAlloc)( u32 length );
static void  (*spFree)( void *ptr );

// const data  -----------------------------------------

static const u32 s_langBitmapList[ LCFG_TWL_REGION_MAX ] = {
	LCFG_TWL_LANG_BITMAP_JAPAN,
	LCFG_TWL_LANG_BITMAP_AMERICA,
	LCFG_TWL_LANG_BITMAP_EUROPE,
	LCFG_TWL_LANG_BITMAP_AUSTRALIA,
	LCFG_TWL_LANG_BITMAP_CHINA,
	LCFG_TWL_LANG_BITMAP_KOREA,
};

static char *strLanguage[] = {
	(char *)"LANG_JAPANESE",
	(char *)"LANG_ENGLISH",
	(char *)"LANG_FRENCH",
	(char *)"LANG_GERMAN",
	(char *)"LANG_ITALIAN",
	(char *)"LANG_SPANISH",
	(char *)"LANG_CHINESE",
	(char *)"LANG_KOREAN",
};

static const char *strLauncherGameCode[] = {
	"LNCJ",
	"LNCE",
	"LNCP",
	"LNCO",
	"LNCC",
	"LNCK",
};

//======================================================
// HW��񃉃C�^�[
//======================================================

/*---------------------------------------------------------------------------*
  Name:         HWI_Init

  Description:  HW��񃉃C�^�[�̏�����

  Arguments:    

  Returns:      None.
 *---------------------------------------------------------------------------*/
HwiInitResult HWI_Init( void *(*pAlloc)( u32 ), void (*pFree)( void * ) )
{
	HwiInitResult result;

	spAlloc = pAlloc;
	spFree  = pFree;

	ACSign_SetAllocFunc( pAlloc, pFree );
	ReadTWLSettings();
	result = ReadPrivateKey();
	ReadHWInfoFile();
//	VerifyHWInfo();

	return result;
}


// TWL�ݒ�f�[�^�̃��[�h
static void ReadTWLSettings( void )
{
	s_isReadTSD = LCFGi_TSD_ReadSettings();
	if( s_isReadTSD ) {
		OS_TPrintf( "TSD read succeeded.\n" );
	}else {
		OS_TPrintf( "TSD read failed.\n" );
	}
}


// �閧���̃��[�h
HwiInitResult ReadPrivateKey( void )
{
	BOOL result = FALSE;
	u32 keyLength;
	FSFile file;
	OSTick start = OS_GetTick();
	
	FS_InitFile( &file );
	if( !FS_OpenFileEx( &file, "rom:key/private_HWInfo.der", FS_FILEMODE_R ) ) 
	{
		OS_TPrintf( "PrivateKey read failed.\n" );
	}
	else 
	{
		keyLength = FS_GetFileLength( &file );
		if( keyLength > 0 ) {
			s_pPrivKeyBuffer = spAlloc( keyLength );
			if( FS_ReadFile( &file, s_pPrivKeyBuffer, (s32)keyLength ) == keyLength ) {
				OS_TPrintf( "PrivateKey read succeeded.\n" );
				result = TRUE;
			}else {
				OS_TPrintf( "PrivateKey read failed.\n" );
			}
		}
		FS_CloseFile( &file );
	}
	
	if( !result && s_pPrivKeyBuffer ) {
		spFree( s_pPrivKeyBuffer );
		s_pPrivKeyBuffer = NULL;
	}
	OS_TPrintf( "PrivKey read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
#ifdef USE_PRODUCT_KEY
	// ���i�p�閧�����L���Ȃ�A��������̃A�N�Z�X
	s_pReadSecureInfoFunc = LCFGi_THW_ReadSecureInfo;
	if (result) { return HWI_INIT_SUCCESS_SIGNATURE_MODE; }
	else		{ return HWI_INIT_FAILURE; }
#else
	// �����łȂ��Ȃ�A�����Ȃ��̃A�N�Z�X
	s_pReadSecureInfoFunc = LCFGi_THW_ReadSecureInfo_NoCheck;
	if (result) { return HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE; }
	else		{ return HWI_INIT_FAILURE; }
#endif
}

// HW���S�̂̃��[�h
static void ReadHWInfoFile( void )
{
	LCFGReadResult retval;
	OSTick start = OS_GetTick();
	
	retval = LCFGi_THW_ReadNormalInfo();
	if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		OS_TPrintf( "HW Normal Info read succeeded.\n" );
	}else {
		OS_TPrintf( "HW Normal Info read failed.\n" );
	}
	
	OS_TPrintf( "HW Normal Info read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
	start = OS_GetTick();
	retval = s_pReadSecureInfoFunc();
	if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		OS_TPrintf( "HW Secure Info read succeeded.\n" );
	}else {
		OS_TPrintf( "HW Secure Info read failed.\n" );
	}
	OS_TPrintf( "HW Secure Info read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
}

// HWInfo�t�@�C���̃x���t�@�C
static void VerifyHWInfo( void )
{
	if( VerifyData(	(const u8 *)LCFG_THW_GetNormalInfo(), (const u8 *)LCFG_THW_GetDefaultNormalInfo(), sizeof(LCFGTWLHWNormalInfo) ) ) {
		OS_TPrintf( "HW normal Info verify succeeded.\n" );
	}else {
		OS_TPrintf( "HW normal Info verify failed.\n" );
	}
	if( VerifyData(	(const u8 *)LCFG_THW_GetSecureInfo(), (const u8 *)LCFG_THW_GetDefaultSecureInfo(), sizeof(LCFGTWLHWSecureInfo) ) ) {
		OS_TPrintf( "HW secure Info verify succeeded.\n" );
	}else {
		OS_TPrintf( "HW secure Info verify failed.\n" );
	}

}

// ��������̃f�[�^�x���t�@�C
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len )
{
	while( len-- ) {
		if( *pTgt++ != *pOrg++ ) {
			return FALSE;
		}
	}
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_ModifyLanguage

  Description:  ����R�[�h�����[�W�����l�ɍ��킹�ďC������B

  Arguments:    

  Returns:      None.
 *---------------------------------------------------------------------------*/
void HWI_ModifyLanguage( u8 region )
{
	u32 langBitmap = s_langBitmapList[ region ];
	u8  nowLanguage = LCFG_TSD_GetLanguage();
	
	// TSD���ǂݍ��߂Ă��Ȃ��Ȃ�A�����������^�[��
	if( !s_isReadTSD ) {
		return;
	}
	
	if( langBitmap & ( 0x0001 << nowLanguage ) ) {
		OS_TPrintf( "Language no change.\n" );
	}else {
		int i;
		for( i = 0; i < LCFG_TWL_LANG_CODE_MAX; i++ ) {
			if( langBitmap & ( 0x0001 << i ) ) {
				break;
			}
		}
		LCFG_TSD_SetLanguage( (LCFGTWLLangCode)i );
		LCFG_TSD_SetFlagCountry( FALSE );				// �����łɍ��R�[�h���N���A���Ă����B
		LCFG_TSD_SetCountry( LCFG_TWL_COUNTRY_UNDEFINED );
		LCFGi_TSD_WriteSettings();
		OS_TPrintf( "Language Change \"%s\" -> \"%s\"\n",
					strLanguage[ nowLanguage ], strLanguage[ LCFG_TSD_GetLanguage() ] );
	}
}

/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWNormalInfoFile

  Description:  HW�m�[�}��Info�t�@�C���̃��C�g

  Arguments:    

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL HWI_WriteHWNormalInfoFile( void )
{
	BOOL isWrite = TRUE;
	LCFGReadResult result;
	
	result = LCFGi_THW_ReadNormalInfo();
	if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		if( !LCFGi_THW_RecoveryNormalInfo( result ) ) {
			OS_TPrintf( "HW Normal Info Recovery failed.\n" );
			isWrite = FALSE;
		}
	}
	if( isWrite &&
		!LCFGi_THW_WriteNormalInfo() ) {
		OS_TPrintf( "HW Normal Info Write failed.\n" );
	}
	
	return isWrite;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWSecureInfoFile

  Description:  HW�Z�L���AInfo�t�@�C���̃��C�g

  Arguments:    

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL HWI_WriteHWSecureInfoFile( u8 region )
{
	BOOL isWrite = TRUE;
	LCFGReadResult result;
	
	// �t�@�C���̃��[�h
	result = s_pReadSecureInfoFunc();
	
	// ���[�h�Ɏ��s�����烊�J�o��
	if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
		if( !LCFGi_THW_RecoverySecureInfo( result ) ) {
			OS_TPrintf( "HW Secure Info Recovery failed.\n" );
			isWrite = FALSE;
		}
	}
	
	// ���[�W�����̃Z�b�g
	LCFG_THW_SetRegion( region );
	
	// �Ή�����r�b�g�}�b�v�̃Z�b�g
	LCFG_THW_SetValidLanguageBitmap( s_langBitmapList[ region ] );
	
	// [TODO:]�ʎY�H���łȂ��ƃV���A��No.�͗p�ӂł��Ȃ��̂ŁA�����ł�MAC�A�h���X�����ƂɓK���Ȓl���Z�b�g����B
	// �V���A��No.�̃Z�b�g
	{
		u8 buffer[ 12 ] = "SERIAL";		// �K���ȕ������MAC�A�h���X�ƌ�������SHA1�����A��SerialNo�Ƃ���B
		u8 serialNo[ SVC_SHA1_DIGEST_SIZE ];
		int i;
		int len = ( LCFG_THW_GetRegion() == LCFG_TWL_REGION_AMERICA ) ?
					LCFG_TWL_HWINFO_SERIALNO_LEN_AMERICA : LCFG_TWL_HWINFO_SERIALNO_LEN_OTHERS;
		OS_GetMacAddress( buffer + 6 );
		SVC_CalcSHA1( serialNo, buffer, sizeof(buffer) );
		for( i = 3; i < SVC_SHA1_DIGEST_SIZE; i++ ) {
			serialNo[ i ] = (u8)( ( serialNo[ i ] % 10 ) + 0x30 );
		}
		MI_CpuCopy8( "SRN", serialNo, 3 );
		MI_CpuClear8( &serialNo[ len ], sizeof(serialNo) - len );
		OS_TPrintf( "serialNo : %s\n", serialNo );
		LCFG_THW_SetSerialNo( serialNo );
	}
	
	// �����`���[TitleID_Lo�̃Z�b�g
	{
		int i;
		u8 titleID_Lo[4];
		for( i = 0; i < 4; i++ ) titleID_Lo[ i ] = (u8)strLauncherGameCode[ region ][ 4 - i - 1 ];
		LCFG_THW_SetLauncherTitleID_Lo( (const u8 *)titleID_Lo );
	}
	
	// ���C�g
	if( isWrite &&
		!LCFGi_THW_WriteSecureInfo( s_pPrivKeyBuffer ) ) {
		isWrite = FALSE;
		OS_TPrintf( "HW Secure Info Write failed.\n" );
	}
	
	return isWrite;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_DeleteHWNormalInfoFile

  Description:  HW�m�[�}��Info�t�@�C���̏���

  Arguments:    None

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_DeleteHWNormalInfoFile( void )
{
	if (FS_DeleteFile( (char *)LCFG_TWL_HWINFO_NORMAL_PATH ))
	{
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
		return TRUE;
	}
	else
	{
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
		return FALSE;
	}
}

/*---------------------------------------------------------------------------*
  Name:         HWI_DeleteHWSecureInfoFile

  Description:  HW�Z�L���AInfo�t�@�C���̏���

  Arguments:    None

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_DeleteHWSecureInfoFile( void )
{
	if (FS_DeleteFile( (char *)LCFG_TWL_HWINFO_SECURE_PATH ))
	{
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
		return TRUE;
	}
	else
	{
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
		return FALSE;
	}
}

