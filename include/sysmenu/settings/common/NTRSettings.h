/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     NTRSettings.c

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


#ifndef	NTR_SETTINGS_H_
#define	NTR_SETTINGS_H_
#if		defined(SDK_CW)							// NTRConfigDataにビットフィールドを使っているので、コンパイラ依存で不具合が発生する可能性がある。
												// よって、CW以外のコンパイラの場合は、このヘッダを無効にしてエラーを出させるようにして再確認する。

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define NTR_SETTINGS_DATA_VERSION		5							// NTR設定データフォーマットバージョン
#define NTR_SETTINGS_DATA_EX_VERSION	1							// 拡張NTR設定データフォーマットバージョン
#define NTR_NICKNAME_LENGTH				10							// ニックネーム長
#define NTR_NICKNAME_BUFFERSIZE			( NTR_NICKNAME_LENGTH * 2 )	// ニックネームバッファサイズ
#define NTR_COMMENT_LENGTH				26							// コメント長
#define NTR_COMMENT_BUFFERSIZE			( NTR_COMMENT_LENGTH * 2 )
#define NTR_USER_COLOR_MAX_NUM			16							// ユーザーカラーの最大数
#define NSD_TEMP_BUFFER_SIZE			( sizeof(NSDStoreEx) * 2 )	// NSD_ReadSettingsで必要なTempBufferサイズ

// 言語設定コード
typedef enum NTRLangCode{
	NTR_LANG_JAPANESE = 0,						// 日本語
	NTR_LANG_ENGLISH  = 1,						// 英語
	NTR_LANG_FRENCH   = 2,						// フランス語
	NTR_LANG_GERMAN   = 3,						// ドイツ語
	NTR_LANG_ITALIAN  = 4,						// イタリア語
	NTR_LANG_SPANISH  = 5,						// スペイン語
	NTR_LANG_CHINESE  = 6,						// 中国語
	NTR_LANG_KOREAN   = 7,						// 韓国語
	NTR_LANG_CODE_MAX
}NTRLangCode;

#define NTR_LANG_CODE_MAX_WW		( NTR_LANG_SPANISH + 1 )

#define NTR_LANG_BITMAP_WW			( ( 0x0001 << NTR_LANG_JAPANESE ) | \
									  ( 0x0001 << NTR_LANG_ENGLISH  ) | \
									  ( 0x0001 << NTR_LANG_FRENCH   ) | \
									  ( 0x0001 << NTR_LANG_GERMAN   ) | \
									  ( 0x0001 << NTR_LANG_ITALIAN  ) | \
									  ( 0x0001 << NTR_LANG_SPANISH  ) ) 	// SystemMenu-WW版での対応言語ビットマップ


#define NTR_LANG_BITMAP_CHINA		( ( 0x0001 << NTR_LANG_CHINESE ) | \
									  ( 0x0001 << NTR_LANG_ENGLISH  ) | \
									  ( 0x0001 << NTR_LANG_FRENCH   ) | \
									  ( 0x0001 << NTR_LANG_GERMAN   ) | \
									  ( 0x0001 << NTR_LANG_ITALIAN  ) | \
									  ( 0x0001 << NTR_LANG_SPANISH  ) ) 	// SystemMenu-CN版での対応言語ビットマップ


#define NTR_LANG_BITMAP_KOREA		( ( 0x0001 << NTR_LANG_KOREAN  ) | \
									  ( 0x0001 << NTR_LANG_JAPANESE ) | \
									  ( 0x0001 << NTR_LANG_ENGLISH  ) | \
									  ( 0x0001 << NTR_LANG_FRENCH   ) | \
									  ( 0x0001 << NTR_LANG_GERMAN   ) | \
									  ( 0x0001 << NTR_LANG_SPANISH  ) ) 	// SystemMenu-KR版での対応言語ビットマップ


// 日付データ
typedef struct NTRDate{
	u8				month;						// 月:01～12
	u8				day;						// 日:01～31
}NTRDate;			// 2byte

// ニックネーム
typedef struct NTRNickname{
	u16				buffer[ NTR_NICKNAME_LENGTH ];	// ニックネーム（Unicode(UTF16)で最大10文字、終端コードなし）
	u8				length;						// 文字数
	u8				rsv;
}NTRNickname;		// 22byte

// コメント
typedef struct NTRComment{
	u16				buffer[ NTR_COMMENT_LENGTH ];	//コメント（Unicode(UTF16)で最大26文字、終端コードなし）
	u8				length;						// 文字数
	u8				rsv;
}NTRComment;		// 54byte

// オーナー情報
typedef struct NTROwnerInfo{
	u8				userColor : 4;			// ユーザーカラー
	u8				rsv : 4;					// 予約。
	NTRDate			birthday;					// 生年月日
	u8				pad;
	NTRNickname		nickname;					// ニックネーム
	NTRComment		comment;					// コメント
}NTROwnerInfo;		// 80byte

// IPL用目覚まし時計データ
typedef struct NTRAlarm{
	u8				hour;						// アラーム時:00～23
	u8				minute;						// アラーム分:00～59
	u8				rsv1;						// 予約
	u8				pad;
	u16				alarmOn : 1;				// アラームON,OFF（0:OFF, 1:ON）
	u16				rsv2 : 15;					// 予約
}NTRAlarm;			// 6byte

// タッチパネルキャリブレーションデータ
typedef struct NTRTPCalibData{
	u16				raw_x1;						// 第１キャリブレーション点のTP取得値X
	u16				raw_y1;						// 　　　　〃　　　　　　　　TP取得値Y
	u8				dx1;						// 　　　　〃　　　　　　　　LCD座標 X
	u8				dy1;						// 　　　　〃　　　　　　　　LCD座標 Y
	u16				raw_x2;						// 第２キャリブレーション点のTP取得値X
	u16				raw_y2;						// 　　　　〃　　　　　　　　TP取得値Y
	u8				dx2;   						// 　　　　〃　　　　　　　　LCD座標 X
	u8				dy2;   						// 　　　　〃　　　　　　　　LCD座標 Y
}NTRTPCalibData;	// 12byte

// オプション情報
typedef struct NTROption{
	u16				language : 3;				// 言語コード（NTR_LANG_SPANISHまでの標準言語コードが入る）
	u16				isGBUseTopLCD : 1;			// AGBモードで起動する時にどちらのLCDで起動するか？（0:TOP,1:BOTTOM）
	u16				backlightBrightness : 2;	// バックライト輝度データ
	u16				isAutoBoot : 1;			// 起動シーケンスで、メニュー停止なしで自動起動するかどうか？(0:OFF, 1:ON)
	u16				isBacklightOff : 1;			// バックライトON,OFFフラグ（0:ON, 1:OFF）
	u16				rsv2 : 1;					// 予約
	u16				initialSequence : 1;		// 初回起動シーケンス中フラグ
	u16				isSetBirthday : 1;			// 誕生日が入力されたか？
	u16				isSetUserColor : 1;		// ユーザーカラーが入力されたか？
	u16				isSetTP : 1;				// タッチパネルがキャリブレーションされたか？（  〃  )
	u16				isSetLanguage : 1;			// 言語入力がされたか？　		(0:未設定, 1:設定済み)
	u16				isSetDateTime : 1;			// 日付・時刻設定がされたか？	(		〃　　　　　 )
	u16				isSetNickname : 1;			// ニックネームが入力されたか？	(		〃　　　　　 )
	u8				rtcLastSetYear;				// RTCの前回設定年
	u8				rtcClockAdjust;				// RTCクロック調整値
	s64				rtcOffset;					// RTC設定時のオフセット値（ユーザーがRTC設定を変更する度にその値に応じて増減します。）
}NTROption;			// 12byte


// NTR各種設定データ
typedef struct NTRSettingsData{
	u8				version;					// フラッシュ格納データフォーマットのバージョン
	u8				pad;
	NTROwnerInfo	owner;						// オーナー情報
	NTRAlarm		alarm;						// IPL用目覚まし時計データ
	NTRTPCalibData	tp;							// タッチパネルキャリブレーションデータ
	NTROption		option;						// オプション
}NTRSettingsData;	// 112byte


// 拡張NTR設定データ
typedef struct NTRSettingsDataEx{
	u8				version;					// バージョン
	u8				language;					// 言語コード（NTR_LANG_CHINESE以降に拡張された値が入る。）
	u16				valid_language_bitmap;		// 本IPL2で有効な言語コードを示したビットマップ
	u8				pad[ 256 - sizeof(NTRSettingsData) - 4 - 4 - 2 ];		// 4:saveCount+crc16, 2:NSDEx.version+NSDEx.language, 2:crc16_ex
}NTRSettingsDataEx;	// 138bytes


// NTR各種設定データのNVRAM保存時フォーマット
typedef struct NSDStore{
	NTRSettingsData nsd;				// NTR各種設定データ
	u16				saveCount;			// 0x00-0x7fをループしてカウントし、カウント値が新しいデータが有効。
	u16				crc16;				// NTR各種設定データの16bitCRC
	u8				pad[ 128 - sizeof(NTRSettingsData) - 4];
}NSDStore;			// 128byte			// ※本来なら、saveCountとcrc16は256byteの最後に付加して、間にパディングを埋める方がいい。


// NTR各種設定データEXのNVRAM保存時フォーマット（上記NCDStoreと互換をとるための無理やり拡張）
typedef struct NSDStoreEx{
	NTRSettingsData	nsd;				// NTR各種設定データ
	u16					saveCount;		// 0x00-0x7fをループしてカウントし、カウント値が新しいデータが有効。
	u16					crc16;			// NTR各種設定データの16bitCRC
	NTRSettingsDataEx	nsd_ex;
	u16					crc16_ex;
}NSDStoreEx;		// 256byte			// ※本来なら、saveCountとcrc16は256byteの最後に付加して、間にパディングを埋める方がいい。


#ifdef SDK_ARM9

//=========================================================
// グローバル変数
//=========================================================
extern NTRSettingsData   *g_pNSD;
extern NTRSettingsDataEx *g_pNSDEx;
#define GetNSD()		( g_pNSD )
#define GetNSDEx()		( g_pNSDEx )

//=========================================================
// NVRAMへのリードライト関数
//=========================================================
extern void NSD_ClearSettings( void );
extern BOOL NSD_IsReadSettings( void );
extern BOOL NSD_ReadSettings( u8 region, NSDStoreEx (*pTempBuffer)[2] );	// NSD_TEMP_BUFFER_SIZEのpTempBufferが必要。
extern BOOL NSD_WriteSettings( u8 region );									// 先にNSD_ReadSettingsを実行しておく必要がある。

//=========================================================
// データ取得（NSD_ReadSettingsで内部ワークに読み出した情報の取得）
//=========================================================

// バージョンの取得。
static inline u8 NSD_GetVersion( void )
{
	return	(u8)GetNSD()->version;
}

// EXバージョンの取得。
static inline u8 NSD_GetExVersion( void )
{
	return	(u8)GetNSDEx()->version;
}

// オーナー情報全体へのポインタの取得。
static inline NTROwnerInfo *NSD_GetOwnerInfoPtr( void )
{
	return	&GetNSD()->owner;
}

// オーナー情報全体の取得。
static inline void NSD_GetOwnerInfo( NTROwnerInfo *pDst )
{
	MI_CpuCopy8( &GetNSD()->owner, pDst, sizeof(NTROwnerInfo) );
}

// ユーザーカラーの取得。
static inline u8 NSD_GetUserColor( void )
{
	return	(u8)GetNSD()->owner.userColor;
}

// 誕生日へのポインタの取得。
static inline NTRDate *NSD_GetBirthdayPtr( void )
{
	return	&GetNSD()->owner.birthday;
}

// 誕生日の取得。
static inline void NSD_GetBirthday( NTRDate *pDst )
{
	MI_CpuCopy8( &GetNSD()->owner.birthday, pDst, sizeof(NTRDate) );
}

// ニックネームへのポインタの取得。
static inline NTRNickname *NSD_GetNicknamePtr( void )
{
	return	&GetNSD()->owner.nickname;
}

// ニックネームの取得。
static inline void NSD_GetNickname( NTRNickname *pDst )
{
	MI_CpuCopy8( &GetNSD()->owner.nickname, pDst, sizeof(NTRNickname) );
}

// コメントへのポインタの取得。
static inline NTRComment *NSD_GetCommentPtr( void )
{
	return	&GetNSD()->owner.comment;
}

// コメントの取得。
static inline void NSD_GetComment( NTRComment *pDst )
{
	MI_CpuCopy8( &GetNSD()->owner.comment, pDst, sizeof(NTRComment) );
}

// アラーム情報へのポインタの取得。
static inline NTRAlarm *NSD_GetAlarmDataPtr( void )
{
	return	&GetNSD()->alarm;
}

// アラーム情報の取得。
static inline void NSD_GetAlarmData( NTRAlarm *pDst )
{
	MI_CpuCopy8( &GetNSD()->alarm, pDst, sizeof(NTRAlarm) );
}

// タッチパネルキャリブレーションデータへのポインタの取得。
static inline NTRTPCalibData *NSD_GetTPCalibrationPtr( void )
{
	return	&GetNSD()->tp;
}

// タッチパネルキャリブレーションデータの取得。
static inline void NSD_GetTPCalibration( NTRTPCalibData *pDst )
{
	MI_CpuCopy8( &GetNSD()->tp, pDst, sizeof(NTRTPCalibData) );
}

// 言語コードの取得
static inline NTRLangCode NSD_GetLanguage( void )
{
  	return	(NTRLangCode)GetNSD()->option.language;
}

static inline NTRLangCode NSD_GetLanguageEx( void )
{
	return	(NTRLangCode)GetNSDEx()->language;
}

// 対応言語ビットマップの取得
static inline u16 NSD_GetLanguageBitmap( void )
{
  	return	GetNSDEx()->valid_language_bitmap;
}

// RTCオフセット値の取得
static inline s64 NSD_GetRTCOffset( void )
{
	return	GetNSD()->option.rtcOffset;
}

// RTCクロック調整値の取得
static inline u8 NSD_GetRTCClockAdjust( void )
{
	return	GetNSD()->option.rtcClockAdjust;
}

// RTCの前回セットした年の取得
static inline u8 NSD_GetRTCLastSetYear( void )
{
	return	GetNSD()->option.rtcLastSetYear;
}

// 起動シーケンスの自動起動ONか？（0:OFF, 1:ON）
static inline int NSD_IsAutoBoot( void )
{
	return	(int)GetNSD()->option.isAutoBoot;
}

// バックライト輝度取得（0-3）
static inline BOOL NSD_IsBacklightOff( void )
{
	return	(BOOL)GetNSD()->option.isBacklightOff;
}

// バックライト輝度取得（0-3）
static inline int NSD_GetBacklightBrightness( void )
{
	return	(int)GetNSD()->option.backlightBrightness;
}

// フラッシュ壊れシーケンス中かどうか？
static inline BOOL NSD_IsInitialSequence( void )
{
	return	(int)GetNSD()->option.initialSequence;
}

// 誕生日データがセットされているか？
static inline BOOL NSD_IsSetBirthday( void )
{
	return	(int)GetNSD()->option.isSetBirthday;
}

// ユーザーカラーデータがセットされているか？
static inline int NSD_IsSetUserColor( void )
{
	return	(int)GetNSD()->option.isSetUserColor;
}

// TPキャリブレーションデータがセットされているか？
static inline int NSD_IsSetTP( void )
{
	return	(int)GetNSD()->option.isSetTP;
}

// 言語コードがセットされているか？
static inline int NSD_IsSetLanguage( void )
{
	return	(int)GetNSD()->option.isSetLanguage;
}

// 日付・時刻がセットされているか？
static inline int NSD_IsSetDateTime( void )
{
	return	(int)GetNSD()->option.isSetDateTime;
}

// オーナー情報のニックネームがセットされているか？
static inline int NSD_IsSetNickname( void )
{
	return	(int)GetNSD()->option.isSetNickname;
}

// GBアプリなど１画面ソフトが使うのは上画面？
static inline BOOL NSD_IsGBUseTopLCD( void )
{
	return	(BOOL)GetNSD()->option.isGBUseTopLCD;
}

//=========================================================
// データセット
//=========================================================

// バージョンのセット。
static inline void NSD_SetVersion( u8 version )
{
	GetNSD()->version = version;
}

// バージョンEXのセット。
static inline void NSD_SetExVersion( u8 version )
{
	GetNSDEx()->version = version;
}

// オーナー情報のクリア
extern void NSD_ClearOwnerInfo( void );

// オーナー情報全体のセット。
static inline void NSD_SetOwnerInfo( const NTROwnerInfo *pOwner )
{
	MI_CpuCopy16( pOwner, &GetNSD()->owner, sizeof(NTROwnerInfo) );
}

// ユーザーカラーのセット。
static inline void NSD_SetUserColor( u8 userColor )
{
	GetNSD()->owner.userColor = userColor;
}

// 誕生日のセット。
static inline void NSD_SetBirthday( const NTRDate *pBirthday )
{
	GetNSD()->owner.birthday.month	= pBirthday->month;
	GetNSD()->owner.birthday.day	= pBirthday->day;
}

// ニックネームのセット。
static inline void NSD_SetNickname( const NTRNickname *pName )
{
	MI_CpuCopy16( pName, &GetNSD()->owner.nickname, sizeof(NTRNickname) );
}

// コメントのセット。
static inline void NSD_SetComment( const NTRComment *pComment )
{
	MI_CpuCopy16( pComment, &GetNSD()->owner.comment, sizeof(NTRComment) );
}

// アラーム情報のセット。
static inline void NSD_SetAlarmData( const NTRAlarm *pAlarm )
{
	MI_CpuCopy16( pAlarm, &GetNSD()->alarm, sizeof(NTRAlarm) );
}

// タッチパネルキャリブレーションデータのセット。
static inline void NSD_SetTPCalibration( const NTRTPCalibData *pTPCalib )
{
	MI_CpuCopy16( pTPCalib, &GetNSD()->tp, sizeof(NTRTPCalibData) );
}

// 言語コードのセット
#if 0
static inline void NSD_SetLanguage( NTRLangCode language )
{
	GetNSDEx()->language = language;
	
	if( language >= NTR_LANG_CODE_MAX_WW ) {
		GetNSD()->option.language = NTR_LANG_ENGLISH;
	}else {
		GetNSD()->option.language = language;
	}
}
#else
static inline void NSD_SetLanguage( NTRLangCode language )
{
	GetNSD()->option.language = language;
}
static inline void NSD_SetLanguageEx( NTRLangCode language )
{
	GetNSDEx()->language = language;
}
#endif

// 対応言語ビットマップのセット
static inline void NSD_SetLanguageBitmap( u16 valid_language_bitmap )
{
	GetNSDEx()->valid_language_bitmap = valid_language_bitmap;
}

// RTCオフセット値のセット
static inline void NSD_SetRTCOffset( s64 rtcOffset )
{
	GetNSD()->option.rtcOffset = rtcOffset;
}

// RTCクロック調整値のセット
static inline void NSD_SetRTCClockAdjust( u8 rtcClockAdjust )
{
	GetNSD()->option.rtcClockAdjust = rtcClockAdjust;
}


// RTCのLastSetYearへのセット
static inline void NSD_SetRTCLastSetYear( u8 rtcLastSetYear )
{
	GetNSD()->option.rtcLastSetYear = rtcLastSetYear;
}


// 起動シーケンスの自動起動ON,OFFフラグをセット。
static inline void NSD_SetFlagAutoBoot( BOOL set )
{
	GetNSD()->option.isAutoBoot = (u16)set;
}

// バックライトON/OFFをセット。
static inline void NSD_SetFlagBacklightOff( BOOL set )
{
	GetNSD()->option.isBacklightOff = (u16)set;
}

// バックライト輝度情報をセット。
static inline void NSD_SetBacklightBrightness( BOOL backlightBrightness )
{
	GetNSD()->option.backlightBrightness = (u16)backlightBrightness;
}

// 初回起動シーケンス中かどうかのフラグセット。
static inline void NSD_SetFlagInitialSequence( BOOL set )
{
	GetNSD()->option.initialSequence = (u16)set;
}

// 誕生日データの入力済みフラグセット。
static inline void NSD_SetFlagBirthday( BOOL set )
{
	GetNSD()->option.isSetBirthday = (u16)set;
}

// ユーザーカラーの入力済みフラグセット。
static inline void NSD_SetFlagUserColor( BOOL set )
{
	GetNSD()->option.isSetUserColor = (u16)set;
}

// TPキャリブレーションデータの入力済みフラグセット。
static inline void NSD_SetFlagTP( BOOL set )
{
	GetNSD()->option.isSetTP = (u16)set;
}

// 言語コードの入力済みフラグセット。
static inline void NSD_SetFlagLanguage( BOOL set )
{
	GetNSD()->option.isSetLanguage = (u16)set;
}

// 日付・時刻入力済みフラグセット。
static inline void NSD_SetFlagDateTime( BOOL set )
{
	GetNSD()->option.isSetDateTime = (u16)set;
}

// オーナー情報のニックネームの入力済みフラグセット。
static inline void NSD_SetFlagNickname( BOOL set )
{
	GetNSD()->option.isSetNickname = (u16)set;
}

// GBアプリなど１画面ソフトが使う画面フラグをセット
static inline void NSD_SetFlagGBUseTopLCD( BOOL set )
{
	GetNSD()->option.isGBUseTopLCD = (u32)set;
}

#endif // SDK_ARM9


#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// NTR_SETTINGS_H_
