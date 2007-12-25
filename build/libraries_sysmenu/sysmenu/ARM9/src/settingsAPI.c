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

// TWL設定データファイルのリード
BOOL SYSM_ReadTWLSettingsFile( void )
{
	BOOL retval;
	// TWL設定データのリード
	retval = TSD_ReadSettings();
	// NTR設定データのリード
	if( !NSD_IsReadSettings() ) {
		(void)NSD_ReadSettings( THW_GetValidLanguageBitmap() );
#ifndef SDK_FINALROM
		(void)SYSMi_VerifyNTRSettings();		// デバッグ用ベリファイ
#endif
	}
	SYSM_SetValidTSD( retval );
	return retval;
}


// TWL設定データファイルへのライト
BOOL SYSM_WriteTWLSettingsFile( void )
{
	BOOL retval;
	// TWL設定データのライト
	retval = TSD_WriteSettings();
	// ライト成功なら、NVRAMのNTR設定データに値を反映
	if( retval ) {
		SYSM_SetValidTSD( TRUE );
		SYSMi_ConvertTWL2NTRSettings();
		(void)NSD_WriteSettings();
#ifndef SDK_FINALROM
		(void)SYSMi_VerifyNTRSettings();		// デバッグ用ベリファイ
#endif
	}
	return retval;
}


// NTR設定とTWL設定をベリファイして、不一致があれば、NTR設定を更新
void SYSM_VerifyAndRecoveryNTRSettings( void )
{
	BOOL isRecovery = FALSE;
	
	// NVRAMからNTR設定データをロードして、TWL設定データとベリファイ
	if( !NSD_ReadSettings( THW_GetValidLanguageBitmap() ) ||
		!SYSMi_VerifyNTRSettings()
		) {
		// ロード or ベリファイ失敗なら、TWL設定データからNTR設定データを生成して、書き込み
		SYSMi_ConvertTWL2NTRSettings();
		NSD_WriteSettings();
	}
}


// NTR設定とTWL設定をベリファイ
BOOL SYSMi_VerifyNTRSettings( void )
{
	BOOL isFailed = FALSE;
	u32 twlValidLangBitmap;
	
	// 値が一致する必要があるもの
	if(	// NTR設定データバージョン
		( NSD_GetVersion()   != NTR_SETTINGS_DATA_VERSION ) ||
		( NSD_GetExVersion() != NTR_SETTINGS_DATA_EX_VERSION ) ||
		// オーナー情報
		( NSD_GetUserColor() != TSD_GetUserColor() ) ||
		!VerifyData( NSD_GetBirthdayPtr(), TSD_GetBirthdayPtr(), sizeof(NTRDate) ) ||
		!VerifyData( NSD_GetNicknamePtr()->buffer, TSD_GetNicknamePtr(), NTR_NICKNAME_LENGTH ) ||
		( NSD_GetNicknamePtr()->length != MY_StrLen( TSD_GetNicknamePtr() ) ) ||
		!VerifyData( NSD_GetCommentPtr()->buffer,  TSD_GetCommentPtr(),  NTR_COMMENT_LENGTH ) ||
		( NSD_GetCommentPtr()->length != MY_StrLen( TSD_GetCommentPtr() ) ) ||
		// アラーム
		!VerifyData( NSD_GetAlarmDataPtr(), TSD_GetAlarmDataPtr(), sizeof(NTRAlarm) ) ||
		// TP情報
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
	
	// オプション
		// "0"であるべきものチェック
	if( (
		NSD_IsGBUseTopLCD() |
		NSD_IsAutoBoot() |
		NSD_IsBacklightOff() |
		NSD_IsInitialSequence() |
		NSD_GetRTCClockAdjust()
		) != 0 ) {
		isFailed = TRUE;
	}
	
		// "1"であるべきものチェック"
	if( ( NSD_IsSetBirthday() &
		  NSD_IsSetUserColor() &
		  NSD_IsSetTP() &
		  NSD_IsSetLanguage() &
		  NSD_IsSetDateTime() &
		  NSD_IsSetNickname()
		  ) == 0 ) {
		isFailed = TRUE;
	}
	
		// 値が一致する必要があるもの
	if( ( NSD_GetRTCLastSetYear() != TSD_GetRTCLastSetYear() ) ||
		( NSD_GetRTCOffset() != TSD_GetRTCOffset() )
		) {
		isFailed = TRUE;
	}
		// SystemMenuのリージョンによって、ちょっと特殊な処理が必要なもの
	twlValidLangBitmap = ( THW_GetValidLanguageBitmap() & NTR_LANG_BITMAP_ALL ) | ( 0x0001 << NTR_LANG_ENGLISH );
	OS_TPrintf( "%08x %08x\n", twlValidLangBitmap, NSD_GetValidLanguageBitmap() );
	if( twlValidLangBitmap != NSD_GetValidLanguageBitmap() ) {
		// 対応言語ビットマップ不一致
		isFailed = TRUE;
	}else if( !( twlValidLangBitmap & ( 0x0001 << NSD_GetLanguage() ) & ( 0x0001 << NSD_GetLanguageEx() ) ) ) {
		// NSD側が対応言語ビットマップ外の値になっている
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
	
		// 値が何でも問題ないもの
	//	  NSD_GetBacklightBrightness();
	
	OS_TPrintf( "TSD & NSD verify %s.\n", isFailed ? "NG" : "OK" );
	
	return !isFailed;
}


// 指定サイズのベリファイ
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


// TWL設定データ -> NTR設定データのコンバート
void SYSMi_ConvertTWL2NTRSettings( void )
{
	SVC_CpuClearFast( 0x0000, GetNSD(),   sizeof(NTRSettingsData) );
	SVC_CpuClearFast( 0x0000, GetNSDEx(), sizeof(NTRSettingsDataEx) );
	
	// NTR設定データバージョン
	NSD_SetVersion  ( NTR_SETTINGS_DATA_VERSION );
	NSD_SetExVersion( NTR_SETTINGS_DATA_EX_VERSION );
	// オーナー情報
	NSD_SetUserColor( TSD_GetUserColor() );
	NSD_SetBirthday( TSD_GetBirthdayPtr() );
	MI_CpuCopy16( TSD_GetNicknamePtr(), NSD_GetNicknamePtr()->buffer, NTR_NICKNAME_BUFFERSIZE );
	NSD_GetNicknamePtr()->length = MY_StrLen( TSD_GetNicknamePtr() );
	MI_CpuCopy16( TSD_GetCommentPtr(), NSD_GetCommentPtr()->buffer, NTR_COMMENT_BUFFERSIZE );
	NSD_GetCommentPtr()->length  = MY_StrLen( TSD_GetCommentPtr() );
	// アラーム
	NSD_SetAlarmData( TSD_GetAlarmDataPtr() );
	// TPキャリブレーション
	NSD_SetTPCalibration( &TSD_GetTPCalibrationPtr()->data );
	
	// オプション
		// "0"であるべきもの
	NSD_SetFlagGBUseTopLCD( FALSE );
	NSD_SetFlagAutoBoot( FALSE );
	NSD_SetFlagBacklightOff( FALSE );
	NSD_SetFlagInitialSequence( FALSE );
	NSD_SetRTCClockAdjust( 0 );
	
		// "1"であるべきもの
	NSD_SetFlagBirthday( TRUE );
	NSD_SetFlagUserColor( TRUE );
	NSD_SetFlagTP( TRUE );
	NSD_SetFlagLanguage( TRUE );
	NSD_SetFlagDateTime( TRUE );
	NSD_SetFlagNickname( TRUE );
	
		// 値が一致する必要があるもの
	NSD_SetRTCLastSetYear( TSD_GetRTCLastSetYear() );
	NSD_SetRTCOffset( TSD_GetRTCOffset() );
	
		// SystemMenuのリージョンによって、ちょっと特殊な処理が必要なもの
	if( TSD_GetLanguage() < NTR_LANG_CODE_MAX_WW ) {
		// TSD側がNTR標準６言語の時、TSD側 == NSD側
		NSD_SetLanguage  ( (NTRLangCode)TSD_GetLanguage() );
		NSD_SetLanguageEx( (NTRLangCode)TSD_GetLanguage() );
	}else if( TSD_GetLanguage() <= TWL_LANG_KOREAN ) {
		// TSD側がNTR標準６言語以外の中国・韓国語の時、NSD側のlanguageは強制ENGLISH（NCDEx側にちゃんとした値が入る）
		NSD_SetLanguage  ( NTR_LANG_ENGLISH );
		NSD_SetLanguageEx( (NTRLangCode)TSD_GetLanguage() );
	}else {
		// それ以外の時は強制ENGLISH
		NSD_SetLanguage  ( NTR_LANG_ENGLISH );
		NSD_SetLanguageEx( NTR_LANG_ENGLISH );
	}
	// 言語ビットマップ
	{
		u16 validLangBitmap = (u16)( ( THW_GetValidLanguageBitmap() & NTR_LANG_BITMAP_ALL ) | ( 0x0001 << NTR_LANG_ENGLISH ) );
		NSD_SetValidLanguageBitmap( validLangBitmap );	// ライト関数内部でもマスクされるが、ここでもしておく。
	}
	
	// 値が何でも問題ないもの
	NSD_SetBacklightBrightness( TSD_GetBacklightBrightness() & 0x03 );
	// [TODO:] TWL側のバックライト輝度レベルが４段階でない時は、変換が必要。
}


// UTF16の文字列長のチェック
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
