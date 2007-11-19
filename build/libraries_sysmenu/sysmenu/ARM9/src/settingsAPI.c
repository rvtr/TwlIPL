/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     settingsAPI.c

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

// define data----------------------------------------------------------

// function's prototype-------------------------------------------------
static BOOL SYSMi_VerifyNTRSettings( void );
static BOOL VerifyData( void *pTgt1, void *pTgt2, u32 size );
static void SYSMi_ConvertTWL2NTRSettings( void );

// global variables-----------------------------------------------------

// static variables-----------------------------------------------------

// const data-----------------------------------------------------------

static const u16 s_validLangBitmapList[] = {
	NTR_LANG_BITMAP_WW,
	NTR_LANG_BITMAP_CHINA,
	NTR_LANG_BITMAP_KOREA,
};

// function's description-----------------------------------------------

// TWL�ݒ�f�[�^�t�@�C���̃��[�h
BOOL SYSM_ReadTWLSettingsFile( void )
{
	BOOL retval = FALSE;;
	{
		TSDStore (*pTempBuffer)[2] = SYSM_Alloc( TSD_TEMP_BUFFER_SIZE );
		if( pTempBuffer == NULL ) {
			OS_TPrintf( "%s : malloc failed.\n", __FUNCTION__ );
			goto RETURN;
		}
		MI_CpuFill32( pTempBuffer, 0xffffffff, TSD_TEMP_BUFFER_SIZE );
		retval = TSD_ReadSettings( pTempBuffer );
		SYSM_Free( pTempBuffer );
	}
	if( !NSD_IsReadSettings() ) {
		NSDStoreEx (*pTempBuffer)[2] = SYSM_Alloc( NSD_TEMP_BUFFER_SIZE );
		if( pTempBuffer == NULL ) {
			OS_TPrintf( "%s : malloc failed.\n", __FUNCTION__ );
			goto RETURN;
		}
		MI_CpuFill32( pTempBuffer, 0xffffffff, NSD_TEMP_BUFFER_SIZE );
		retval = NSD_ReadSettings( TSD_GetRegion(), pTempBuffer );
		SYSM_Free( pTempBuffer );
#ifndef SDK_FINALROM
		(void)SYSMi_VerifyNTRSettings();
#endif
	}
RETURN:
	SYSM_SetValidTSD( retval );
	return retval;
}


// TWL�ݒ�f�[�^�t�@�C���ւ̃��C�g
BOOL SYSM_WriteTWLSettingsFile( void )
{
	BOOL retval;
	retval = TSD_WriteSettings();
	if( retval ) {									// ���C�g�����Ȃ�ATSD��NSD�ɕϊ����āANVRAM�ɂ���������
		SYSM_SetValidTSD( TRUE );
		SYSMi_ConvertTWL2NTRSettings();
		NSD_WriteSettings( TSD_GetRegion() );
#ifndef SDK_FINALROM
		(void)SYSM_VerifyAndRecoveryNTRSettings();	// ���f�o�b�O�p�@�x���t�@�C���āANG�Ȃ烊�J�o��
#endif
	}
	return retval;
}


// NTR�ݒ��TWL�ݒ���x���t�@�C���āA�s��v������΁ANTR�ݒ���X�V
void SYSM_VerifyAndRecoveryNTRSettings( void )
{
	BOOL isRecovery = FALSE;
	NSDStoreEx (*pTempBuffer)[2] = SYSM_Alloc( NSD_TEMP_BUFFER_SIZE );
	
	// NVRAM����NTR�ݒ�f�[�^�����[�h���āATWL�ݒ�f�[�^�ƃx���t�@�C
	if( pTempBuffer == NULL ) {
		OS_Panic( "%s : malloc error.\n", __FUNCTION__ );
	}
	if( !NSD_ReadSettings( TSD_GetRegion(), pTempBuffer ) ||
		!SYSMi_VerifyNTRSettings()
		) {
		// ���[�h or �x���t�@�C���s�Ȃ�ATWL�ݒ�f�[�^����NTR�ݒ�f�[�^�𐶐����āA��������
		SYSMi_ConvertTWL2NTRSettings();
		NSD_WriteSettings( TSD_GetRegion() );
	}
	SYSM_Free( pTempBuffer );
}


// NTR�ݒ��TWL�ݒ���x���t�@�C
static BOOL SYSMi_VerifyNTRSettings( void )
{
	BOOL isFailed = FALSE;
//	NTRAlarm zeroAlarm;				// TWL�ŃA���[�����Ȃ����ꍇ�́A�[���l�A���[���Ɣ�r������B
//	MI_CpuClear( &zeroAlarm, sizeof(NTRAlarm) );
	
	// �l����v����K�v���������
	if(	// NTR�ݒ�f�[�^�o�[�W����
		( NSD_GetVersion()   != NTR_SETTINGS_DATA_VERSION ) ||
		( NSD_GetExVersion() != NTR_SETTINGS_DATA_EX_VERSION ) ||
		// �I�[�i�[���
		( NSD_GetUserColor() != TSD_GetUserColor() ) ||
		!VerifyData( NSD_GetBirthday(), TSD_GetBirthday(), sizeof(NTRDate) ) ||
		!VerifyData( NSD_GetNickname()->buffer, TSD_GetNickname()->buffer, NTR_NICKNAME_LENGTH ) ||
		( NSD_GetNickname()->length != TSD_GetNickname()->length ) ||
		!VerifyData( NSD_GetComment()->buffer,  TSD_GetComment()->buffer,  NTR_COMMENT_LENGTH ) ||
		( NSD_GetComment()->length != TSD_GetComment()->length ) ||
		// �A���[��
		!VerifyData( NSD_GetAlarmData(), TSD_GetAlarmData(), sizeof(NTRAlarm) ) ||
		// TP���
		!VerifyData( NSD_GetTPCalibration(), &TSD_GetTPCalibration()->data, sizeof(NTRTPCalibData) )
		) {
		
		OS_TPrintf( "VERSION   : %d\n", ( NSD_GetVersion()   != NTR_SETTINGS_DATA_VERSION ) );
		OS_TPrintf( "VERSION EX: %d\n", ( NSD_GetExVersion() != NTR_SETTINGS_DATA_EX_VERSION ) );
		OS_TPrintf( "UserColor : %d\n", ( NSD_GetUserColor() != TSD_GetUserColor() ) );
		OS_TPrintf( "Birthday  : %d\n", !VerifyData( NSD_GetBirthday(), TSD_GetBirthday(), sizeof(NTRDate) ) );
		OS_TPrintf( "Nickname  : %d\n", !VerifyData( NSD_GetNickname()->buffer, TSD_GetNickname()->buffer, NTR_NICKNAME_LENGTH ) );
		OS_TPrintf( "  length  : %d\n", ( NSD_GetNickname()->length != TSD_GetNickname()->length ) );
		OS_TPrintf( "Comment   : %d\n", !VerifyData( NSD_GetComment()->buffer,  TSD_GetComment()->buffer,  NTR_COMMENT_LENGTH ) );
		OS_TPrintf( "  length  : %d\n", ( NSD_GetComment()->length != TSD_GetComment()->length ) );
		OS_TPrintf( "Alarm     : %d\n", !VerifyData( NSD_GetAlarmData(), TSD_GetAlarmData(), sizeof(NTRAlarm) ) );
		OS_TPrintf( "TP        : %d\n", !VerifyData( NSD_GetTPCalibration(), &TSD_GetTPCalibration()->data, sizeof(NTRTPCalibData) ) );
		
		isFailed = TRUE;
	}
	
	// �I�v�V����
		// "0"�ł���ׂ����̃`�F�b�N
	if( (
		NSD_IsGBUseTopLCD() |
		NSD_IsAutoBoot() |
		NSD_IsBacklightOff() |
		NSD_IsInitialSequence() |
		NSD_GetRTCLastSetYear() |
		NSD_GetRTCClockAdjust()
		) != 0 ) {
		isFailed = TRUE;
	}
	
		// "1"�ł���ׂ����̃`�F�b�N"
	if( ( NSD_IsSetBirthday() &
		  NSD_IsSetUserColor() &
		  NSD_IsSetTP() &
		  NSD_IsSetLanguage() &
		  NSD_IsSetDateTime() &
		  NSD_IsSetNickname()
		  ) == 0 ) {
		isFailed = TRUE;
	}
	
		// �l����v����K�v���������
	if( ( NSD_GetRTCOffset() != TSD_GetRTCOffset() ) ||
		( NSD_GetLanguageEx() != TSD_GetLanguage() )
//		|| ( NSD_GetLanguageBitmap() != TSD_GetLanguageBitmap() )
		) {
		isFailed = TRUE;
	}
	
		// SystemMenu�̃��[�W�����ɂ���āA������Ɠ���ȏ������K�v�Ȃ���
	if( TSD_GetLanguage() < TWL_LANG_CODE_MAX_WW ) {
		// TSD�����W���U����̎��ANSD���́A�e���[�W�����̑Ή�����r�b�g�}�b�v�̂����A�W���U����̂��̂�����肦�Ȃ��B
		u16 defaultLangBitmap = (u16)( s_validLangBitmapList[ TSD_GetRegion() & 0x03 ] & NTR_LANG_BITMAP_WW );
		if( ( defaultLangBitmap & ( 0x0001 << NSD_GetLanguage() ) ) == 0 ) {
			isFailed = TRUE;
		}
	}else {
		// TSD�����W���U����ȊO�̎��ANSD����language�͋���ENGLISH�iNCDEx���ɂ����Ƃ����l������j
		if( NSD_GetLanguage() != NTR_LANG_ENGLISH ) {
			isFailed = TRUE;
		}
	}
	
		// �l�����ł����Ȃ�����
	//	  NSD_GetBacklightBrightness();
	
	OS_TPrintf( "TSD & NSD verify %s.\n", isFailed ? "NG" : "OK" );
	
	return !isFailed;
}


// �w��T�C�Y�̃x���t�@�C
static BOOL VerifyData( void *pTgt1, void *pTgt2, u32 size )
{
	u8 *p1 = (u8 *)pTgt1;
	u8 *p2 = (u8 *)pTgt2;
	
	while( size-- ) {
		if( *p1++ != *p2++ ) {
			return FALSE;
		}
	}
	return TRUE;
}


// TWL�ݒ�f�[�^ -> NTR�ݒ�f�[�^�̃R���o�[�g
static void SYSMi_ConvertTWL2NTRSettings( void )
{
	SVC_CpuClearFast( 0x0000, GetNSD(),   sizeof(NTRSettingsData) );
	SVC_CpuClearFast( 0x0000, GetNSDEx(), sizeof(NTRSettingsDataEx) );
	
	// NTR�ݒ�f�[�^�o�[�W����
	NSD_SetVersion  ( NTR_SETTINGS_DATA_VERSION );
	NSD_SetExVersion( NTR_SETTINGS_DATA_EX_VERSION );
	// �I�[�i�[���
	NSD_SetUserColor( TSD_GetUserColor() );
	MI_CpuCopy8 ( TSD_GetBirthday(), NSD_GetBirthday(), sizeof(NTRDate) );
	MI_CpuCopy16( TSD_GetNickname()->buffer, NSD_GetNickname()->buffer, NTR_NICKNAME_BUFFERSIZE );
	NSD_GetNickname()->length = TSD_GetNickname()->length;
	MI_CpuCopy16( TSD_GetComment()->buffer, NSD_GetComment()->buffer, NTR_COMMENT_BUFFERSIZE );
	NSD_GetNickname()->length  = TSD_GetNickname()->length;
	// �A���[��
	MI_CpuCopy16( TSD_GetAlarmData(), NSD_GetAlarmData(), sizeof(NTRAlarm) );
	// TP�L�����u���[�V����
	MI_CpuCopy16( &TSD_GetTPCalibration()->data, NSD_GetTPCalibration(), sizeof(NTRTPCalibData) );
	
	// �I�v�V����
		// "0"�ł���ׂ�����
	NSD_SetFlagGBUseTopLCD( FALSE );
	NSD_SetFlagAutoBoot( FALSE );
	NSD_SetFlagBacklightOff( FALSE );
	NSD_SetFlagInitialSequence( FALSE );
	NSD_SetRTCLastSetYear( 0 );
	NSD_SetRTCClockAdjust( 0 );
	
		// "1"�ł���ׂ�����
	NSD_SetFlagBirthday( TRUE );
	NSD_SetFlagUserColor( TRUE );
	NSD_SetFlagTP( TRUE );
	NSD_SetFlagLanguage( TRUE );
	NSD_SetFlagDateTime( TRUE );
	NSD_SetFlagNickname( TRUE );
	
		// �l����v����K�v���������
	NSD_SetRTCOffset( TSD_GetRTCOffset() );
	NSD_SetLanguageEx( (NTRLangCode)TSD_GetLanguage() );
//	NSD_SetLanguageBitmap( TSD_GetLanguageBitmap() );
	
		// SystemMenu�̃��[�W�����ɂ���āA������Ɠ���ȏ������K�v�Ȃ���
	if( TSD_GetLanguage() < TWL_LANG_CODE_MAX_WW ) {
		// TSD�����W���U����̎��ATSD�� == NSD��
		NSD_SetLanguage( (NTRLangCode)TSD_GetLanguage() );
	}else {
		// TSD�����W���U����ȊO�̎��ANSD����language�͋���ENGLISH�iNCDEx���ɂ����Ƃ����l������j
		NSD_SetLanguage( NTR_LANG_ENGLISH );
	}
	
	// �l�����ł����Ȃ�����
	// �������� TWL���̃o�b�N���C�g�P�x���x�����S�i�K�łȂ����́A�ϊ����K�v�B��������
	NSD_SetBacklightBrightness( TSD_GetBacklightBrightness() );
}

