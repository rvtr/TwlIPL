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
#if 0
// TWL����->NTR ����ւ̑Ή��}�b�v
const u8 s_langCodeMapFromTWLtoNTR[ TWL_LANG_CODE_MAX ] = {
	NTR_LANG_JAPANESE		// TWL_LANG_JAPANESE
	NTR_LANG_ENGLISH		// TWL_LANG_ENGLISH
	NTR_LANG_FRENCH			// TWL_LANG_FRENCH
	NTR_LANG_GERMAN			// TWL_LANG_GERMAN
	NTR_LANG_ITALIAN		// TWL_LANG_ITALIAN
	NTR_LANG_SPANISH		// TWL_LANG_SPANISH
	NTR_LANG_CHINESE		// TWL_LANG_SIMP_CHINESE
	NTR_LANG_KOREAN			// TWL_LANG_KOREAN
//	NTR_LANG_ENGLISH		// TWL_LANG_DUTCH
//	NTR_LANG_CHINESE		// TWL_LANG_TRAD_CHINESE
};
#endif

// function's description-----------------------------------------------

// TWL�ݒ�f�[�^�t�@�C���̃��[�h
BOOL SYSM_ReadTWLSettingsFile( void )
{
	BOOL retval;
	// TWL�ݒ�f�[�^�̃��[�h
	retval = TSD_ReadSettings();
	// NTR�ݒ�f�[�^�̃��[�h
	if( !NSD_IsReadSettings() ) {
		NSDStoreEx (*pTempBuffer)[2] = SYSM_Alloc( NSD_TEMP_BUFFER_SIZE );
		if( pTempBuffer == NULL ) {
			OS_TPrintf( "%s : malloc failed.\n", __FUNCTION__ );
			goto RETURN;
		}
		(void)NSD_ReadSettings( THW_GetRegion(), pTempBuffer );
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
	// TWL�ݒ�f�[�^�̃��C�g
	retval = TSD_WriteSettings();
	// ���C�g�����Ȃ�ANVRAM��NTR�ݒ�f�[�^�ɒl�𔽉f
	if( retval ) {
		SYSM_SetValidTSD( TRUE );
		SYSMi_ConvertTWL2NTRSettings();
		NSD_WriteSettings( THW_GetRegion() );
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
	if( !NSD_ReadSettings( THW_GetRegion(), pTempBuffer ) ||
		!SYSMi_VerifyNTRSettings()
		) {
		// ���[�h or �x���t�@�C���s�Ȃ�ATWL�ݒ�f�[�^����NTR�ݒ�f�[�^�𐶐����āA��������
		SYSMi_ConvertTWL2NTRSettings();
		NSD_WriteSettings( THW_GetRegion() );
	}
	SYSM_Free( pTempBuffer );
}


// NTR�ݒ��TWL�ݒ���x���t�@�C
BOOL SYSMi_VerifyNTRSettings( void )
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
	{
		NTRLangCode language = ( TSD_GetLanguage() < NTR_LANG_CODE_MAX_WW ) ?
								NSD_GetLanguage() : NSD_GetLanguageEx();
		// NSD���́A�e���[�W�����̑Ή�����r�b�g�}�b�v�̂��̂�����肦�Ȃ��B
		if( ( THW_GetValidLanguageBitmap() & ( 0x0001 << language ) ) == 0 ) {
			isFailed = TRUE;
		}
		if( TSD_GetLanguage() >= NTR_LANG_CODE_MAX_WW ) {
			// TSD����NTR�W���U����ȊO�̎��ANSD����language�͋���ENGLISH�iNCDEx���ɂ����Ƃ����l������j
			if( NSD_GetLanguage() != NTR_LANG_ENGLISH ) {
				isFailed = TRUE;
			}
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
	if( TSD_GetLanguage() < NTR_LANG_CODE_MAX_WW ) {
		// TSD����NTR�W���U����̎��ATSD�� == NSD��
		NSD_SetLanguage( (NTRLangCode)TSD_GetLanguage() );
	}else {
		// TSD����NTR�W���U����ȊO�̎��ANSD����language�͋���ENGLISH�iNCDEx���ɂ����Ƃ����l������j
		NSD_SetLanguage( NTR_LANG_ENGLISH );
	}
	
	// �l�����ł����Ȃ�����
	// �������� TWL���̃o�b�N���C�g�P�x���x�����S�i�K�łȂ����́A�ϊ����K�v�B��������
	NSD_SetBacklightBrightness( TSD_GetBacklightBrightness() );
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
