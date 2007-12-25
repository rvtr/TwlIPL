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
BOOL SYSMi_VerifyNTRSettings( void );
static BOOL VerifyData( const void *pTgt1, const void *pTgt2, u32 size );
void SYSMi_ConvertTWL2NTRSettings( void );
static u8 MY_StrLen( const u16 *pStr );

// global variables-----------------------------------------------------

// static variables-----------------------------------------------------

// const data-----------------------------------------------------------

// function's description-----------------------------------------------

// TWL�ݒ�f�[�^�t�@�C���̃��[�h
BOOL SYSM_ReadTWLSettingsFile( void )
{
	BOOL retval;
	// TWL�ݒ�f�[�^�̃��[�h
	retval = TSD_ReadSettings();
	// NTR�ݒ�f�[�^�̃��[�h
	if( !NSD_IsReadSettings() ) {
		(void)NSD_ReadSettings( THW_GetValidLanguageBitmap() );
#ifndef SDK_FINALROM
		(void)SYSMi_VerifyNTRSettings();		// �f�o�b�O�p�x���t�@�C
#endif
	}
	SYSM_SetValidTSD( retval );
	return retval;
}


// TWL�ݒ�f�[�^�t�@�C���ւ̃��C�g
BOOL SYSM_WriteTWLSettingsFile( void )
{
	BOOL retval;
	// TWL�ݒ�f�[�^�̃��C�g
	retval = TSD_WriteSettings();
	// ���C�g�����Ȃ�ANVRAM��NTR�ݒ�f�[�^�ɒl�𔽉f
	if( retval ) {
		SYSM_SetValidTSD( TRUE );
		SYSMi_ConvertTWL2NTRSettings();
		(void)NSD_WriteSettings();
#ifndef SDK_FINALROM
		(void)SYSMi_VerifyNTRSettings();		// �f�o�b�O�p�x���t�@�C
#endif
	}
	return retval;
}


// NTR�ݒ��TWL�ݒ���x���t�@�C���āA�s��v������΁ANTR�ݒ���X�V
void SYSM_VerifyAndRecoveryNTRSettings( void )
{
	BOOL isRecovery = FALSE;
	
	// NVRAM����NTR�ݒ�f�[�^�����[�h���āATWL�ݒ�f�[�^�ƃx���t�@�C
	if( !NSD_ReadSettings( THW_GetValidLanguageBitmap() ) ||
		!SYSMi_VerifyNTRSettings()
		) {
		// ���[�h or �x���t�@�C���s�Ȃ�ATWL�ݒ�f�[�^����NTR�ݒ�f�[�^�𐶐����āA��������
		SYSMi_ConvertTWL2NTRSettings();
		NSD_WriteSettings();
	}
}


// NTR�ݒ��TWL�ݒ���x���t�@�C
BOOL SYSMi_VerifyNTRSettings( void )
{
	BOOL isFailed = FALSE;
	u32 twlValidLangBitmap;
	
	// �l����v����K�v���������
	if(	// NTR�ݒ�f�[�^�o�[�W����
		( NSD_GetVersion()   != NTR_SETTINGS_DATA_VERSION ) ||
		( NSD_GetExVersion() != NTR_SETTINGS_DATA_EX_VERSION ) ||
		// �I�[�i�[���
		( NSD_GetUserColor() != TSD_GetUserColor() ) ||
		!VerifyData( NSD_GetBirthdayPtr(), TSD_GetBirthdayPtr(), sizeof(NTRDate) ) ||
		!VerifyData( NSD_GetNicknamePtr()->buffer, TSD_GetNicknamePtr(), NTR_NICKNAME_LENGTH ) ||
		( NSD_GetNicknamePtr()->length != MY_StrLen( TSD_GetNicknamePtr() ) ) ||
		!VerifyData( NSD_GetCommentPtr()->buffer,  TSD_GetCommentPtr(),  NTR_COMMENT_LENGTH ) ||
		( NSD_GetCommentPtr()->length != MY_StrLen( TSD_GetCommentPtr() ) ) ||
		// �A���[��
		!VerifyData( NSD_GetAlarmDataPtr(), TSD_GetAlarmDataPtr(), sizeof(NTRAlarm) ) ||
		// TP���
		!VerifyData( NSD_GetTPCalibrationPtr(), TSD_GetTPCalibrationPtr(), sizeof(NTRTPCalibData) )
		) {
		
		OS_TPrintf( "VERSION   : %d\n", ( NSD_GetVersion()   != NTR_SETTINGS_DATA_VERSION ) );
		OS_TPrintf( "VERSION EX: %d\n", ( NSD_GetExVersion() != NTR_SETTINGS_DATA_EX_VERSION ) );
		OS_TPrintf( "UserColor : %d\n", ( NSD_GetUserColor() != TSD_GetUserColor() ) );
		OS_TPrintf( "Birthday  : %d\n", !VerifyData( NSD_GetBirthdayPtr(), TSD_GetBirthdayPtr(), sizeof(NTRDate) ) );
		OS_TPrintf( "Nickname  : %d\n", !VerifyData( NSD_GetNicknamePtr()->buffer, TSD_GetNicknamePtr(), NTR_NICKNAME_LENGTH ) );
		OS_TPrintf( "  length  : %d\n", ( NSD_GetNicknamePtr()->length != MY_StrLen( TSD_GetNicknamePtr() ) ) );
		OS_TPrintf( "Comment   : %d\n", !VerifyData( NSD_GetCommentPtr()->buffer,  TSD_GetCommentPtr(),  NTR_COMMENT_LENGTH ) );
		OS_TPrintf( "  length  : %d\n", ( NSD_GetCommentPtr()->length != MY_StrLen( TSD_GetCommentPtr() ) ) );
		OS_TPrintf( "Alarm     : %d\n", !VerifyData( NSD_GetAlarmDataPtr(), TSD_GetAlarmDataPtr(), sizeof(NTRAlarm) ) );
		OS_TPrintf( "TP        : %d\n", !VerifyData( NSD_GetTPCalibrationPtr(), TSD_GetTPCalibrationPtr(), sizeof(NTRTPCalibData) ) );
		
		isFailed = TRUE;
	}
	
	// �I�v�V����
		// "0"�ł���ׂ����̃`�F�b�N
	if( (
		NSD_IsGBUseTopLCD() |
		NSD_IsAutoBoot() |
		NSD_IsBacklightOff() |
		NSD_IsInitialSequence() |
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
	if( ( NSD_GetRTCLastSetYear() != TSD_GetRTCLastSetYear() ) ||
		( NSD_GetRTCOffset() != TSD_GetRTCOffset() )
		) {
		isFailed = TRUE;
	}
		// SystemMenu�̃��[�W�����ɂ���āA������Ɠ���ȏ������K�v�Ȃ���
	twlValidLangBitmap = ( THW_GetValidLanguageBitmap() & NTR_LANG_BITMAP_ALL ) | ( 0x0001 << NTR_LANG_ENGLISH );
	OS_TPrintf( "%08x %08x\n", twlValidLangBitmap, NSD_GetValidLanguageBitmap() );
	if( twlValidLangBitmap != NSD_GetValidLanguageBitmap() ) {
		// �Ή�����r�b�g�}�b�v�s��v
		isFailed = TRUE;
	}else if( !( twlValidLangBitmap & ( 0x0001 << NSD_GetLanguage() ) & ( 0x0001 << NSD_GetLanguageEx() ) ) ) {
		// NSD�����Ή�����r�b�g�}�b�v�O�̒l�ɂȂ��Ă���
		isFailed = TRUE;
	}else if( TSD_GetLanguage() < NTR_LANG_CODE_MAX_WW ) {
		if( ( NSD_GetLanguage()   >= NTR_LANG_CODE_MAX_WW ) ||
			( NSD_GetLanguageEx() >= NTR_LANG_CODE_MAX_WW ) ) {
			isFailed = TRUE;
		}
	}else if( TSD_GetLanguage() <= NTR_LANG_KOREAN ) {
		if( ( NSD_GetLanguage()   != NTR_LANG_ENGLISH ) ||
			( NSD_GetLanguageEx() >  NTR_LANG_KOREAN ) ) {
			isFailed = TRUE;
		}
	}else {
		if( ( NSD_GetLanguage()   != NTR_LANG_ENGLISH ) ||
			( NSD_GetLanguageEx() != NTR_LANG_ENGLISH ) ) {
			isFailed = TRUE;
		}
	}
	
		// �l�����ł����Ȃ�����
	//	  NSD_GetBacklightBrightness();
	
	OS_TPrintf( "TSD & NSD verify %s.\n", isFailed ? "NG" : "OK" );
	
	return !isFailed;
}


// �w��T�C�Y�̃x���t�@�C
static BOOL VerifyData( const void *pTgt1, const void *pTgt2, u32 size )
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
void SYSMi_ConvertTWL2NTRSettings( void )
{
	SVC_CpuClearFast( 0x0000, GetNSD(),   sizeof(NTRSettingsData) );
	SVC_CpuClearFast( 0x0000, GetNSDEx(), sizeof(NTRSettingsDataEx) );
	
	// NTR�ݒ�f�[�^�o�[�W����
	NSD_SetVersion  ( NTR_SETTINGS_DATA_VERSION );
	NSD_SetExVersion( NTR_SETTINGS_DATA_EX_VERSION );
	// �I�[�i�[���
	NSD_SetUserColor( TSD_GetUserColor() );
	NSD_SetBirthday( TSD_GetBirthdayPtr() );
	MI_CpuCopy16( TSD_GetNicknamePtr(), NSD_GetNicknamePtr()->buffer, NTR_NICKNAME_BUFFERSIZE );
	NSD_GetNicknamePtr()->length = MY_StrLen( TSD_GetNicknamePtr() );
	MI_CpuCopy16( TSD_GetCommentPtr(), NSD_GetCommentPtr()->buffer, NTR_COMMENT_BUFFERSIZE );
	NSD_GetCommentPtr()->length  = MY_StrLen( TSD_GetCommentPtr() );
	// �A���[��
	NSD_SetAlarmData( TSD_GetAlarmDataPtr() );
	// TP�L�����u���[�V����
	NSD_SetTPCalibration( &TSD_GetTPCalibrationPtr()->data );
	
	// �I�v�V����
		// "0"�ł���ׂ�����
	NSD_SetFlagGBUseTopLCD( FALSE );
	NSD_SetFlagAutoBoot( FALSE );
	NSD_SetFlagBacklightOff( FALSE );
	NSD_SetFlagInitialSequence( FALSE );
	NSD_SetRTCClockAdjust( 0 );
	
		// "1"�ł���ׂ�����
	NSD_SetFlagBirthday( TRUE );
	NSD_SetFlagUserColor( TRUE );
	NSD_SetFlagTP( TRUE );
	NSD_SetFlagLanguage( TRUE );
	NSD_SetFlagDateTime( TRUE );
	NSD_SetFlagNickname( TRUE );
	
		// �l����v����K�v���������
	NSD_SetRTCLastSetYear( TSD_GetRTCLastSetYear() );
	NSD_SetRTCOffset( TSD_GetRTCOffset() );
	
		// SystemMenu�̃��[�W�����ɂ���āA������Ɠ���ȏ������K�v�Ȃ���
	if( TSD_GetLanguage() < NTR_LANG_CODE_MAX_WW ) {
		// TSD����NTR�W���U����̎��ATSD�� == NSD��
		NSD_SetLanguage  ( (NTRLangCode)TSD_GetLanguage() );
		NSD_SetLanguageEx( (NTRLangCode)TSD_GetLanguage() );
	}else if( TSD_GetLanguage() <= TWL_LANG_KOREAN ) {
		// TSD����NTR�W���U����ȊO�̒����E�؍���̎��ANSD����language�͋���ENGLISH�iNCDEx���ɂ����Ƃ����l������j
		NSD_SetLanguage  ( NTR_LANG_ENGLISH );
		NSD_SetLanguageEx( (NTRLangCode)TSD_GetLanguage() );
	}else {
		// ����ȊO�̎��͋���ENGLISH
		NSD_SetLanguage  ( NTR_LANG_ENGLISH );
		NSD_SetLanguageEx( NTR_LANG_ENGLISH );
	}
	// ����r�b�g�}�b�v
	{
		u16 validLangBitmap = (u16)( ( THW_GetValidLanguageBitmap() & NTR_LANG_BITMAP_ALL ) | ( 0x0001 << NTR_LANG_ENGLISH ) );
		NSD_SetValidLanguageBitmap( validLangBitmap );	// ���C�g�֐������ł��}�X�N����邪�A�����ł����Ă����B
	}
	
	// �l�����ł����Ȃ�����
	NSD_SetBacklightBrightness( TSD_GetBacklightBrightness() & 0x03 );
	// [TODO:] TWL���̃o�b�N���C�g�P�x���x�����S�i�K�łȂ����́A�ϊ����K�v�B
}


// UTF16�̕����񒷂̃`�F�b�N
static u8 MY_StrLen( const u16 *pStr )
{
	u8 len = 0;
	while( *pStr++ ) {
		++len;
		if( len == 255 ) {
			break;
		}
	}
	return len;
}
