/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     TWLSettings.h

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


#ifndef	TWL_SETTINGS_H_
#define	TWL_SETTINGS_H_
#if		defined(SDK_CW)							// TWLSettingsDataにビットフィールドを使っているので、コンパイラ依存で不具合が発生する可能性がある。
												// よって、CW以外のコンパイラの場合は、このヘッダを無効にしてエラーを出させるようにして再確認する。

#include <twl.h>
#include <sysmenu/settings/common/countryCode.h>
#include <sysmenu/settings/common/NTRSettings.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define TWL_SETTINGS_FILE_LENGTH				( 16 * 1024 )
#define TWL_SETTINGS_DATA_VERSION				1							// TWL設定データフォーマットバージョン(開始No.:1)
// オーナー情報
#define TWL_NICKNAME_LENGTH						NTR_NICKNAME_LENGTH			// ニックネーム長
#define TWL_NICKNAME_BUFFERSIZE					( ( TWL_NICKNAME_LENGTH + 1 ) * 2 )	// ニックネームバッファサイズ
#define TWL_COMMENT_LENGTH						NTR_COMMENT_LENGTH			// コメント長
#define TWL_COMMENT_BUFFERSIZE					( ( TWL_COMMENT_LENGTH + 1 ) * 2 )	// コメントバッファサイズ
#define TWL_FAVORITE_COLOR_MAX_NUM				NTR_FAVORITE_COLOR_MAX_NUM	// 好きな色の最大数
// バックライト輝度
#define TWL_BACKLIGHT_LEVEL_MAX					22							// TWLバックライト最大輝度レベル
// ペアレンタルコントロール
#define TWL_PARENTAL_CONTROL_RATING_AGE_MAX		31
#define TWL_PARENTAL_CONTROL_PASSWORD_LENGTH	4               			// 暗証番号の桁数
#define TWL_PARENTAL_CONTROL_PASSWORD_DEFAULT	"0000"          			// デフォルト暗証番号
#define TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MIN		6   			// 秘密の質問の回答、UTF-16で最小 MIN 文字
#define TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX		32     			// 秘密の質問の回答、UTF-16で最大 MAX 文字
// インストール・ソフト数
#define TWL_FREE_SOFT_BOX_COUNT_MAX				35   						// NANDアプリの最大空きBox数, これ - freeSoftBoxCountでインストールSoft数


// 言語コード
// 欧州と北米の表示テキストの違いは、リージョンと言語コードを併せて判断
typedef enum TWLLangCode{
	TWL_LANG_JAPANESE = 0,						// 日本語
	TWL_LANG_ENGLISH  = 1,						// 英語
	TWL_LANG_FRENCH   = 2,						// フランス語
	TWL_LANG_GERMAN   = 3,						// ドイツ語
	TWL_LANG_ITALIAN  = 4,						// イタリア語
	TWL_LANG_SPANISH  = 5,						// スペイン語
	TWL_LANG_SIMP_CHINESE = 6,					// 中国語（簡体字）
	TWL_LANG_KOREAN   = 7,						// 韓国語
//	TWL_LANG_DUTCH    = 8,						// オランダ語（Wiiでは存在）
//	TWL_LANG_TRAD_CHINESE = 9,					// 台湾語（繁体字）（Wiiでは存在）
	
	TWL_LANG_CODE_MAX
}TWLLangCode;

// 日付
#define TWLDate						NTRDate

// アラーム
#define TWLAlarm					NTRAlarm


// TPキャリブレーション（NTRとの違いは、予約領域あり）
typedef struct TWLTPCalibData {
	NTRTPCalibData	data;						// TPキャリブレーションデータ
	u8				rsv[ 8 ];
}TWLTPCalibData;


// オーナー情報
typedef struct TWLOwnerInfo{
	u8				userColor : 4;				// 好きな色
	u8				rsv : 4;					// 予約。
	u8				pad;						// パディング
	TWLDate			birthday;					// 生年月日
	u16				nickname[ TWL_NICKNAME_LENGTH + 1 ];	// ニックネーム（終端あり）
	u16				comment[ TWL_COMMENT_LENGTH + 1 ];		// コメント（終端あり）
}TWLOwnerInfo;		// 80byte


// ペアレンタルコントロール
// 審査団体
typedef enum TWLRatingOgn {
	TWL_RATING_OGN_CERO          = 0,   // 日本
	TWL_RATING_OGN_ESRB          = 1,   // アメリカ
	TWL_RATING_OGN_USK           = 2,   // ドイツ
	TWL_RATING_OGN_PEGI_GENERAL  = 3,   // 欧州
	TWL_RATING_OGN_PEGI_PORTUGAL = 4,   // ポルトガル
	TWL_RATING_OGN_PEGI_BBFC     = 5,   // イギリス
	TWL_RATING_OGN_AGCB          = 6,   // オーストラリア
	TWL_RATING_OGN_OFLC          = 7,   // ニュージーランド
	TWL_RATING_OGN_GRB           = 8,   // 韓国
	TWL_RATING_OGN_MAX           = 8
}TWLRatingOgn;

// データ
typedef struct TWLParentalControl {
	TWLRatingOgn	ogn;				// 審査団体
//	u8				flags;				// Wiiでは、PARENTAL_CONTROL_USEフラグのみ --> isSetParentalControl があるので現状必要なし
	u8				ratingAge;			// レーティング（年齢）値
	char			password[ TWL_PARENTAL_CONTROL_PASSWORD_LENGTH + 1 ];   // 暗証番号、終端コードあり
	u8				secretQuestion;     // 秘密の質問文 ID
	u8				rsv_A;
	u16				secretAnswer[ TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX + 1 ];  // UTF16,秘密の質問への回答、終端コードあり
	u16				secretAnswerLength; // 秘密の質問への回答文字数（Wiiでu16,LENGTH_MAX が保持できるからu8でもいいのでは？）
//	u8				rsv_B[ 16 ]; 		// 削除予定（16バイトのレーティング情報を持つのはアプリ側のROMヘッダ）
}TWLParentalControl;


// TWL設定データ（基本、過去ver互換を考慮して、追加しかしない方針で。）
typedef struct TWLSettingsData{
	struct flags {
		u32		initialSequence : 1;				// 初回起動シーケンス中？
		u32		isSetCountry : 1;					// 国コード設定済み？
		u32		isSetLanguage : 1;					// 言語設定済み？
		u32		isSetDateTime : 1;					// 日付・時刻設定済み？
		u32		isSetNickname : 1;					// ニックネーム設定済み？
		u32		isSetUserColor : 1;					// ユーザーカラー設定済み？
		u32		isSetBirthday : 1;					// 誕生日設定済み？
		u32		isSetTP : 1;						// TP設定済み？
		u32		isSetParentalControl : 1;			// パレンタルコントロール設定済み？
//		u32		isSetBrowserRestriction : 1;		// Wiiで存在。フルブラウザを制限するかどうか。TWLでは検討中。
		u32		isAgreeEURA : 1;					// EURA同意済み？
		// WiFi設定は別データなので、ここに設定済みフラグは用意しない。
		u32		isGBUseTopLCD : 1;					// １画面のGBゲーム時に上画面を使う？
		u32		isAvailableWireless : 1;            // 無線モジュールのRFユニットの有効化／無効化
		u32		isAvailableBatteryExtension : 1;    // バッテリエクステンションモードの有効化／無効化
		u32		rsv : 19;
	}flags;
	u8					rsv2[ 3 ];					// 予約
	u8					country;					// 国コード
	u8					language;					// 言語(NTRとの違いは、データサイズ8bit)
	u8					backLightBrightness;		// バックライト輝度(NTRとの違いは、データサイズ8bit)
	u8					freeSoftBoxCount;			// インストール可能なNANDアプリ個数
	u8					rtcLastSetYear;				// RTCの前回設定年
	s64					rtcOffset;					// RTC設定時のオフセット値（ユーザーがRTC設定を変更する度にその値に応じて増減します。）
	TWLOwnerInfo		owner;						// オーナー情報
	TWLAlarm			alarm;						// アラーム
	TWLTPCalibData		tp;							// タッチパネルキャリブレーションデータ
	TWLParentalControl	parental;
}TWLSettingsData;	// xxbyte


#ifdef SDK_ARM9

//=========================================================
// グローバル変数
//=========================================================
extern TWLSettingsData s_settings;
#define GetTSD()		( &s_settings )

//=========================================================
// NANDファイルへのリードライト関数
//=========================================================
	// 内部変数へのリード
extern BOOL TSD_ReadSettings( void );
	// 内部変数の値のライト（先にリードしておく必要がある）
extern BOOL TSD_WriteSettings( void );
	// 直接値を指定してのライト（開発用）
extern BOOL TSD_WriteSettingsDirect( const TWLSettingsData *pSrc );
	// 内部変数の値のクリア
extern void TSD_ClearSettings( void );


//=========================================================
// データ取得（TSD_ReadSettingsで内部ワークに読み出した情報の取得）
//=========================================================

// 国コードの取得。
static inline TWLCountryCode TSD_GetCountry( void )
{
	return	(TWLCountryCode)GetTSD()->country;
}

// 言語コードの取得
static inline TWLLangCode TSD_GetLanguage( void )
{
  	return	(TWLLangCode)GetTSD()->language;
}

// バックライト輝度取得
static inline int TSD_GetBacklightBrightness( void )
{
	return	(int)GetTSD()->backLightBrightness;
}

// フリーソフトBOX数の取得
static inline u8 TSD_GetFreeSoftBoxCount( void )
{
  	return	GetTSD()->freeSoftBoxCount;
}

// RTCの前回セットした年の取得
static inline u8 TSD_GetRTCLastSetYear( void )
{
	return	GetTSD()->rtcLastSetYear;
}

// RTCオフセット値の取得
static inline s64 TSD_GetRTCOffset( void )
{
	return	GetTSD()->rtcOffset;
}

// オーナー情報全体の取得。
static inline void TSD_GetOwnerInfo( TWLOwnerInfo *pDst )
{
	MI_CpuCopy8( &GetTSD()->owner, pDst, sizeof(TWLOwnerInfo) );
}

// オーナー情報全体へのポインタの取得。
static inline const TWLOwnerInfo *TSD_GetOwnerInfoPtr( void )
{
	return	(const TWLOwnerInfo *)&GetTSD()->owner;
}

// 好きな色の取得。
static inline u8 TSD_GetUserColor( void )
{
	return	(u8)GetTSD()->owner.userColor;
}

// 誕生日の取得。
static inline void TSD_GetBirthday( TWLDate *pDst )
{
	MI_CpuCopy8( &GetTSD()->owner.birthday, pDst, sizeof(TWLDate) );
}

// 誕生日へのポインタの取得。
static inline const TWLDate *TSD_GetBirthdayPtr( void )
{
	return	(const TWLDate *)&GetTSD()->owner.birthday;
}

// ニックネームの取得。
static inline void TSD_GetNickname( u16 *pDst )
{
	MI_CpuCopy16( GetTSD()->owner.nickname, pDst, TWL_NICKNAME_BUFFERSIZE );
}

// ニックネームへのポインタの取得。
static inline const u16 *TSD_GetNicknamePtr( void )
{
	return	(const u16 *)&GetTSD()->owner.nickname;
}

// コメントの取得。
static inline void TSD_GetComment( u16 *pDst )
{
	MI_CpuCopy16( GetTSD()->owner.comment, pDst, TWL_COMMENT_BUFFERSIZE );
}

// コメントへのポインタの取得。
static inline const u16 *TSD_GetCommentPtr( void )
{
	return	(const u16 *)&GetTSD()->owner.comment;
}

// アラーム情報の取得。
static inline void TSD_GetAlarmData( TWLAlarm *pAlarm )
{
	MI_CpuCopy8( &GetTSD()->alarm, pAlarm, sizeof(TWLAlarm) );
}

// アラーム情報へのポインタの取得。
static inline const TWLAlarm *TSD_GetAlarmDataPtr( void )
{
	return	(const TWLAlarm *)&GetTSD()->alarm;
}

// タッチパネルキャリブレーションデータの取得。
static inline void TSD_GetTPCalibration( TWLTPCalibData *pDst )
{
	MI_CpuCopy8( &GetTSD()->tp, pDst, sizeof(TWLTPCalibData) );
}

// タッチパネルキャリブレーションデータへのポインタの取得。
static inline const TWLTPCalibData *TSD_GetTPCalibrationPtr( void )
{
	return	(const TWLTPCalibData *)&GetTSD()->tp;
}

// 初回起動シーケンス中？
static inline BOOL TSD_IsInitialSequence( void )
{
	return	(BOOL)GetTSD()->flags.initialSequence;
}

// 国コード入力済み？
static inline BOOL TSD_IsSetCountry( void )
{
	return	(BOOL)GetTSD()->flags.isSetCountry;
}

// 言語コード入力済み？
static inline BOOL TSD_IsSetLanguage( void )
{
	return	(BOOL)GetTSD()->flags.isSetLanguage;
}

// 日付・時刻データ入力済み？
static inline BOOL TSD_IsSetDateTime( void )
{
	return	(BOOL)GetTSD()->flags.isSetDateTime;
}

// ニックネーム入力済み？
static inline BOOL TSD_IsSetNickname( void )
{
	return	(BOOL)GetTSD()->flags.isSetNickname;
}

// ユーザーカラー入力済み？
static inline BOOL TSD_IsSetUserColor( void )
{
	return	(BOOL)GetTSD()->flags.isSetUserColor;
}

// 誕生日入力済み？
static inline BOOL TSD_IsSetBirthday( void )
{
	return	(BOOL)GetTSD()->flags.isSetBirthday;
}

// TPキャリブレーションデータ入力済み？
static inline BOOL TSD_IsSetTP( void )
{
	return	(BOOL)GetTSD()->flags.isSetTP;
}

// パレンタルコントロール入力済み？
static inline BOOL TSD_IsSetParentalControl( void )
{
	return	(BOOL)GetTSD()->flags.isSetParentalControl;
}

// EURAビューア同意済み？
static inline BOOL TSD_IsAgreeEURA( void )
{
	return	(BOOL)GetTSD()->flags.isAgreeEURA;
}

// GBアプリなど１画面ソフトが使うのは上画面？
static inline BOOL TSD_IsGBUseTopLCD( void )
{
	return	(BOOL)GetTSD()->flags.isGBUseTopLCD;
}

// 無線使用可能？
static inline BOOL TSD_IsAvailableWireless( void )
{
	return	(BOOL)GetTSD()->flags.isAvailableWireless;
}

// バッテリエクステンションモード有効？
static inline BOOL TSD_IsAvailableBatteryExtension( void )
{
	return	(BOOL)GetTSD()->flags.isAvailableBatteryExtension;
}


//=========================================================
// データセット（TSD_ReadSettingsで内部ワークに読み出した情報への値セット）
//=========================================================

// 国コードのセット。
static inline void TSD_SetCountry( TWLCountryCode country )
{
	GetTSD()->country = (u8)country;
}

// 言語コードのセット
static inline void TSD_SetLanguage( TWLLangCode language )
{
	GetTSD()->language = language;
}

// バックライト輝度情報をセット。
static inline void TSD_SetBacklightBrightness( u8 backLightBrightness )
{
	GetTSD()->backLightBrightness = backLightBrightness;
}

// フリーソフトBOX数のセット
static inline void TSD_SetFreeSoftBoxCount( u8 count )
{
	GetTSD()->freeSoftBoxCount = count;
}

// RTCのLastSetYearへのセット
static inline void TSD_SetRTCLastSetYear( u8 rtcLastSetYear )
{
	GetTSD()->rtcLastSetYear = rtcLastSetYear;
}

// RTCオフセット値のセット
static inline void TSD_SetRTCOffset( s64 rtcOffset )
{
	GetTSD()->rtcOffset = rtcOffset;
}

// オーナー情報全体のセット。
static inline void TSD_SetOwnerInfo( const TWLOwnerInfo *pOwner )
{
	MI_CpuCopy16( pOwner, &GetTSD()->owner, sizeof(TWLOwnerInfo) );
}

// ユーザーカラーのセット。
static inline void TSD_SetUserColor( u8 userColor )
{
	GetTSD()->owner.userColor = userColor;
}

// 誕生日のセット。
static inline void TSD_SetBirthday( const TWLDate *pSrc )
{
	MI_CpuCopy8( pSrc, &GetTSD()->owner.birthday, sizeof(TWLDate) );
}

// ニックネームのセット。
static inline void TSD_SetNickname( const u16 *pSrc )
{
	MI_CpuCopy16( pSrc, GetTSD()->owner.nickname, TWL_NICKNAME_BUFFERSIZE );
}

// コメントのセット。
static inline void TSD_SetComment( const u16 *pSrc )
{
	MI_CpuCopy16( pSrc, GetTSD()->owner.comment, TWL_COMMENT_BUFFERSIZE );
}

// アラーム情報のセット。
static inline void TSD_SetAlarmData( const TWLAlarm *pAlarm )
{
	MI_CpuCopy16( pAlarm, &GetTSD()->alarm, sizeof(TWLAlarm) );
}

// タッチパネルキャリブレーションデータのセット。
static inline void TSD_SetTPCalibration( const TWLTPCalibData *pTPCalib )
{
	MI_CpuCopy16( pTPCalib, &GetTSD()->tp, sizeof(TWLTPCalibData) );
}

// 初回起動シーケンス中かどうかのフラグセット。
static inline void TSD_SetFlagInitialSequence( BOOL initialSequence )
{
	GetTSD()->flags.initialSequence = (u32)initialSequence;
}

// 国コードの入力済みフラグセット。
static inline void TSD_SetFlagCountry( BOOL set )
{
	GetTSD()->flags.isSetCountry = (u32)set;
}

// 言語コードの入力済みフラグセット。
static inline void TSD_SetFlagLanguage( BOOL set )
{
	GetTSD()->flags.isSetLanguage = (u32)set;
}

// 日付・時刻の入力済みフラグセット。
static inline void TSD_SetFlagDateTime( BOOL set )
{
	GetTSD()->flags.isSetDateTime = (u32)set;
}

// ニックネームの入力済みフラグセット。
static inline void TSD_SetFlagNickname( BOOL set )
{
	GetTSD()->flags.isSetNickname = (u32)set;
}

// ユーザーカラーの入力済みフラグセット。
static inline void TSD_SetFlagUserColor( BOOL set )
{
	GetTSD()->flags.isSetUserColor = (u32)set;
}

// 誕生日データの入力済みフラグセット。
static inline void TSD_SetFlagBirthday( BOOL set )
{
	GetTSD()->flags.isSetBirthday = (u32)set;
}

// TPキャリブレーションの入力済みフラグセット。
static inline void TSD_SetFlagTP( BOOL set )
{
	GetTSD()->flags.isSetTP = (u32)set;
}

// パレンタルコントロールの入力済みフラグセット。
static inline void TSD_SetFlagParentalControl( BOOL set )
{
	GetTSD()->flags.isSetNickname = (u32)set;
}

// EURAビューア同意済みフラグセット。
static inline void TSD_SetFlagAgreeEURA( BOOL set )
{
	GetTSD()->flags.isSetNickname = (u32)set;
}

// GBアプリなど１画面ソフトが使う画面フラグをセット
static inline void TSD_SetFlagGBUseTopLCD( BOOL set )
{
	GetTSD()->flags.isGBUseTopLCD = (u32)set;
}

// 無線使用可否フラグをセット
static inline void TSD_SetFlagAvailableWireless( BOOL set )
{
	GetTSD()->flags.isAvailableWireless = set;
}

// バッテリエクステンションモード有効／無効フラグをセット
static inline void TSD_SetFlagAvailableBatteryExtension( BOOL set )
{
	GetTSD()->flags.isAvailableBatteryExtension = set;
}


#endif // SDK_ARM9



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// TWL_SETTINGS_H_
