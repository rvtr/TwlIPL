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
#define TWL_DEFAULT_REGION				0							// デフォルトのリージョン　いる？？
#define TWL_SETTINGS_DATA_VERSION		1							// TWL設定データフォーマットバージョン(開始No.:1)
#define TWL_NICKNAME_LENGTH				NTR_NICKNAME_LENGTH			// ニックネーム長
#define TWL_NICKNAME_BUFFERSIZE			( ( TWL_NICKNAME_LENGTH + 1 ) * 2 )	// ニックネームバッファサイズ
#define TWL_COMMENT_LENGTH				NTR_COMMENT_LENGTH			// コメント長
#define TWL_COMMENT_BUFFERSIZE			( ( TWL_COMMENT_LENGTH + 1 ) * 2 )	// コメントバッファサイズ
#define TWL_FAVORITE_COLOR_MAX_NUM		NTR_FAVORITE_COLOR_MAX_NUM	// 好きな色の最大数
#define TSD_TEMP_BUFFER_SIZE			( sizeof(TSDStore) * 2 )	// TSD_ReadTWLSettingsで必要なTempBufferサイズ


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

#define TWL_LANG_CODE_MAX_WW		TWL_LANG_SPANISH

// NTR 言語との対応マップ
#if 0
const u8 LangCodeMapFromTWLtoNTR[][ 2 ] = {
	{ TWL_LANG_JAPANESE,			NTR_LANG_JAPANESE	},
	{ TWL_LANG_ENGLISH,				NTR_LANG_ENGLISH	},
	{ TWL_LANG_FRENCH,				NTR_LANG_FRENCH		},
	{ TWL_LANG_GERMAN,				NTR_LANG_GERMAN		},
	{ TWL_LANG_ITALIAN,				NTR_LANG_ITALIAN	},
	{ TWL_LANG_SPANISH,				NTR_LANG_SPANISH	},
	{ TWL_LANG_SIMP_CHINESE,		NTR_LANG_CHINESE	},
	{ TWL_LANG_KOREAN,				NTR_LANG_KOREAN		},
//	{ TWL_LANG_DUTCH,				NTR_LANG_ENGLISH	},
//	{ TWL_LANG_TRAD_CHINESE,		NTR_LANG_CHINESE	}
};
#endif

#define TWL_LANG_BITMAP_JAPAN		( ( 0x0001 << TWL_LANG_JAPANESE ) ) 	// JPN版での対応言語ビットマップ
#define TWL_LANG_BITMAP_AMERICA		( ( 0x0001 << TWL_LANG_ENGLISH ) | \
									  ( 0x0001 << TWL_LANG_FRENCH  ) | \
									  ( 0x0001 << TWL_LANG_SPANISH  ) ) 	// AME版での対応言語ビットマップ
#define TWL_LANG_BITMAP_EUROPE		( ( 0x0001 << TWL_LANG_ENGLISH ) | \
									  ( 0x0001 << TWL_LANG_FRENCH  ) | \
									  ( 0x0001 << TWL_LANG_GERMAN  ) | \
									  ( 0x0001 << TWL_LANG_ITALIAN  ) | \
									  ( 0x0001 << TWL_LANG_SPANISH  ) ) 	// EUR版での対応言語ビットマップ
#define TWL_LANG_BITMAP_AUSTRALIA	( ( 0x0001 << TWL_LANG_ENGLISH  ) ) 	// AUS版での対応言語ビットマップ
#define TWL_LANG_BITMAP_CHINA		( ( 0x0001 << TWL_LANG_SIMP_CHINESE ) ) // CHI版での対応言語ビットマップ
#define TWL_LANG_BITMAP_KOREA		( ( 0x0001 << TWL_LANG_KOREAN ) ) 		// KOR版での対応言語ビットマップ


// リージョンコード（販社別になる見込み）
typedef enum TWLRegionCode {
	TWL_REGION_JAPAN     = 0,   // NCL
	TWL_REGION_AMERICA   = 1,   // NOA
	TWL_REGION_EUROPE    = 2,   // NOE
	TWL_REGION_AUSTRALIA = 3,   // NAL
	TWL_REGION_CHINA     = 4,   // IQue
	TWL_REGION_KOREA     = 5,   // NOK
	TWL_REGION_MAX       = 5
}TWLRegion;


// 日付
#define TWLDate						NTRDate

// アラーム
#define TWLAlarm					NTRAlarm

// TPキャリブレーション（NTRとの違いは、予約領域あり）
typedef struct TWLTPCalibData {
	NTRTPCalibData	data;
	u8				rsv[ 8 ];
}TWLTPCalibData;

// ニックネーム（NTRとの違いは、文字列に終端あり）
typedef struct TWLNickname{
	u16				buffer[ TWL_NICKNAME_LENGTH + 1 ];	// ニックネーム（Unicode(UTF16)で最大10文字、終端コードあり）
	u8				length;							// 文字数
	u8				rsv;
}TWLNickname;		// 24byte

// コメント（NTRとの違いは、文字列に終端あり）
typedef struct TWLComment{
	u16				buffer[ TWL_COMMENT_LENGTH + 1 ];	//コメント（Unicode(UTF16)で最大26文字、終端コードあり）
	u8				length;							// 文字数
	u8				rsv;
}TWLComment;		// 54byte

// オーナー情報
typedef struct TWLOwnerInfo{
	u8				userColor : 4;				// 好きな色
	u8				rsv : 4;					// 予約。
	TWLDate			birthday;					// 生年月日
	u8				pad;
	TWLNickname		nickname;					// ニックネーム
	TWLComment		comment;					// コメント
}TWLOwnerInfo;		// 80byte


// ペアレンタルコントロール
#define TWL_PARENTAL_CONTROL_PASSWORD_LENGTH     4                  // 暗証番号の桁数
#define TWL_PARENTAL_CONTROL_PASSWORD_DEFAULT    "0000"             // デフォルト暗証番号
#define TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MIN 6             // 秘密の質問の回答、UTF-16で最小 MIN 文字
#define TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX 32            // 秘密の質問の回答、UTF-16で最大 MAX 文字

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

typedef struct TWLParentalControl {
	TWLRatingOgn	ogn;				// 審査団体
//	u8				flags;				// Wiiでは、PARENTAL_CONTROL_USEフラグのみ --> isSetParentalControl があるので現状必要なし
	u8				rating;				// レーティング（年齢）値
	char			password[ TWL_PARENTAL_CONTROL_PASSWORD_LENGTH + 1 ];   // 暗証番号、終端コードあり
	u8				secretQuestion;     // 秘密の質問文 ID
	u8				rsv_A;
	u16				secretAnswer[ TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX + 1 ];  // UTF16,秘密の質問への回答、終端コードあり
	u16				secretAnswerLength; // 秘密の質問への回答文字数（Wiiでu16,LENGTH_MAX が保持できるからu8でもいいのでは？）
//	u8				rsv_B[ 16 ]; 		// 削除予定（16バイトのレーティング情報を持つのはアプリ側のROMヘッダ）
}TWLParentalControl;


// インストール・ソフト数
#define TWL_FREE_SOFT_BOX_COUNT_MAX  35     // NANDアプリの最大空きBox数, これ - freeSoftBoxCountでインストールSoft数


// TWL設定データヘッダ
typedef struct TWLSettingsHeader{
	u8					version;
	u8					saveCount;
	u16					dataLength;
	u8					digest[ SVC_SHA1_DIGEST_SIZE ];				// SHA1ダイジェスト　　CRC16で十分かもなあ。。。
}TWLSettingsHeader;


// TWL設定データ（基本、過去ver互換を考慮して、追加しかしない方針で。）
typedef struct TWLSettingsData{
	struct flags {
		u32		initialSequence : 1;
		u32		isSetCountry : 1;
		u32		isSetLanguage : 1;
		u32		isSetDateTime : 1;
		u32		isSetNickname : 1;
		u32		isSetUserColor : 1;
		u32		isSetBirthday : 1;
		u32		isSetTP : 1;
		u32		isSetParentalControl : 1;
//		u32		isSetBrowserRestriction : 1;    // Wiiで存在。フルブラウザを制限するかどうか。TWLでは検討中。
		u32		isAgreeEURA : 1;
		// WiFi設定は別データなので、ここに設定済みフラグは用意しない。
		u32		isGBUseTopLCD : 1;
		u32		isAvailableWireless : 1;            // 無線モジュールのRFユニットの有効化／無効化
		u32		isAvailableBatteryExtension : 1;    // バッテリエクステンションモードの有効化／無効化
		u32		rsv : 19;
	}flags;
	u16					valid_language_bitmap;		// 対応言語ビットマップ（※ここじゃなく、"/sys/HWINFO.dat"内の方が良さそう）
	TWLCountryCode		country;					// 国コード
	u8					region;						// リージョン（※ここじゃなく、"/sys/HWINFO.dat"内の方が良さそう）
	u8					language;					// 言語(NTRとの違いは、データサイズ8bit)
	u8					backLightBrightness;		// バックライト輝度(NTRとの違いは、データサイズ8bit)
	u8					rtcLastSetYear;				// RTCの前回設定年
	s64					rtcOffset;					// RTC設定時のオフセット値（ユーザーがRTC設定を変更する度にその値に応じて増減します。）
	TWLOwnerInfo		owner;						// オーナー情報
	TWLAlarm			alarm;						// アラーム
	TWLTPCalibData		tp;							// タッチパネルキャリブレーションデータ
	TWLParentalControl	parental;
	u8					freeSoftBoxCount;			// インストール可能なNANDアプリ個数
}TWLSettingsData;	// xxbyte


// TWL設定データ保存フォーマット
typedef struct TSDStore {
	TWLSettingsHeader	header;
	TWLSettingsData		tsd;
}TSDStore;


#ifdef SDK_ARM9

//=========================================================
// グローバル変数
//=========================================================
extern TWLSettingsData   *g_pTSD;
#define GetTSD()		( g_pTSD )

//=========================================================
// NANDファイルへのリードライト関数
//=========================================================
extern BOOL TSD_IsReadSettings( void );
extern BOOL TSD_ReadSettings( TSDStore (*pTempBuffer)[2] );	// TSD_TEMP_BUFFER_SIZEのpTempBufferが必要。
extern BOOL TSD_WriteSettings( void );						// 先にNSD_ReadSettingsを実行しておく必要がある。

//=========================================================
// データ取得（TSD_ReadSettingsで内部ワークに読み出した情報の取得）
//=========================================================

// リージョンの取得。
static inline u8 TSD_GetRegion( void )
{
	return	(u8)GetTSD()->region;
}

// オーナー情報全体の取得。
static inline TWLOwnerInfo *TSD_GetOwnerInfo( void )
{
	return	&GetTSD()->owner;
}

// 好きな色の取得。
static inline u8 TSD_GetUserColor( void )
{
	return	(u8)GetTSD()->owner.userColor;
}

// 誕生日の取得。
static inline TWLDate *TSD_GetBirthday( void )
{
	return	&GetTSD()->owner.birthday;
}

// ニックネームの取得。
static inline TWLNickname *TSD_GetNickname( void )
{
	return	&GetTSD()->owner.nickname;
}

// コメントの取得。
static inline TWLComment *TSD_GetComment( void )
{
	return	&GetTSD()->owner.comment;
}

// アラーム情報の取得。
static inline TWLAlarm *TSD_GetAlarmData( void )
{
	return	&GetTSD()->alarm;
}

// タッチパネルキャリブレーションデータの取得。
static inline TWLTPCalibData *TSD_GetTPCalibration( void )
{
	return	&GetTSD()->tp;
}

// 言語コードの取得
static inline TWLLangCode TSD_GetLanguage( void )
{
  	return	(TWLLangCode)GetTSD()->language;
}

// 対応言語ビットマップの取得
static inline u16 TSD_GetLanguageBitmap( void )
{
  	return	GetTSD()->valid_language_bitmap;
}

// RTCオフセット値の取得
static inline s64 TSD_GetRTCOffset( void )
{
	return	GetTSD()->rtcOffset;
}

// RTCの前回セットした年の取得
static inline u8 TSD_GetRTCLastSetYear( void )
{
	return	GetTSD()->rtcLastSetYear;
}

// バックライト輝度取得
static inline int TSD_GetBacklightBrightness( void )
{
	return	(int)GetTSD()->backLightBrightness;
}

// 初回起動シーケンス中？
static inline BOOL TSD_IsInitialSequence( void )
{
	return	(BOOL)GetTSD()->flags.initialSequence;
}

// 誕生日入力済み？
static inline BOOL TSD_IsSetBirthday( void )
{
	return	(BOOL)GetTSD()->flags.isSetBirthday;
}

// ユーザーカラー入力済み？
static inline BOOL TSD_IsSetUserColor( void )
{
	return	(BOOL)GetTSD()->flags.isSetUserColor;
}

// TPキャリブレーションデータ入力済み？
static inline BOOL TSD_IsSetTP( void )
{
	return	(BOOL)GetTSD()->flags.isSetTP;
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


//=========================================================
// データセット（TSD_ReadSettingsで内部ワークに読み出した情報への値セット）
//=========================================================

// リージョンのセット。
static inline void TSD_SetRegion( u8 region )
{
	GetTSD()->region = region;
}

// オーナー情報全体のセット。
static inline void TSD_SetOwnerInfo( TWLOwnerInfo *pOwner )
{
	MI_CpuCopy16( pOwner, &GetTSD()->owner, sizeof(TWLOwnerInfo) );
}

// ユーザーカラーのセット。
static inline void TSD_SetUserColor( u8 userColor )
{
	GetTSD()->owner.userColor = userColor;
}

// 誕生日のセット。
static inline void TSD_SetBirthday( TWLDate *pBirth )
{
	GetTSD()->owner.birthday.month	= pBirth->month;
	GetTSD()->owner.birthday.day	= pBirth->day;
}

// ニックネームのセット。
static inline void TSD_SetNickname( TWLNickname *pNickname )
{
	MI_CpuCopy16( pNickname, &GetTSD()->owner.nickname, sizeof(TWLNickname) );
}

// コメントのセット。
static inline void TSD_SetComment( TWLComment *pComment )
{
	MI_CpuCopy16( pComment, &GetTSD()->owner.comment, sizeof(TWLComment) );
}

// アラーム情報のセット。
static inline void TSD_SetAlarmData( TWLAlarm *pAlarm )
{
	MI_CpuCopy16( pAlarm, &GetTSD()->alarm, sizeof(TWLAlarm) );
}

// タッチパネルキャリブレーションデータのセット。
static inline void TSD_SetTPCalibration( TWLTPCalibData *pTPCalib )
{
	MI_CpuCopy16( pTPCalib, &GetTSD()->tp, sizeof(TWLTPCalibData) );
}

// 言語コードのセット
static inline void TSD_SetLanguage( TWLLangCode language )
{
	GetTSD()->language = language;
}

// 対応言語ビットマップのセット
static inline void TSD_SetLanguageBitmap( u16 valid_language_bitmap )
{
	GetTSD()->valid_language_bitmap = valid_language_bitmap;
}

// RTCオフセット値のセット
static inline void TSD_SetRTCOffset( s64 rtcOffset )
{
	GetTSD()->rtcOffset = rtcOffset;
}

// RTCのLastSetYearへのセット
static inline void TSD_SetRTCLastSetYear( u8 rtcLastSetYear )
{
	GetTSD()->rtcLastSetYear = rtcLastSetYear;
}

// バックライト輝度情報をセット。
static inline void TSD_SetBacklightBrightness( u8 backLightBrightness )
{
	GetTSD()->backLightBrightness = backLightBrightness;
}

// フラッシュ壊れシーケンス中かどうかのフラグセット。
static inline void TSD_SetFlagInitialSequence( BOOL initialSequence )
{
	GetTSD()->flags.initialSequence = (u32)initialSequence;
}

// 誕生日データの入力済みフラグセット。
static inline void TSD_SetFlagBirthday( BOOL set )
{
	GetTSD()->flags.isSetBirthday = (u32)set;
}

// ユーザーカラーの入力済みフラグセット。
static inline void TSD_SetFlagUserColor( BOOL set )
{
	GetTSD()->flags.isSetUserColor = (u32)set;
}

// TPキャリブレーションの入力済みフラグセット。
static inline void TSD_SetFlagTP( BOOL set )
{
	GetTSD()->flags.isSetTP = (u32)set;
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


#endif // SDK_ARM9



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// TWL_SETTINGS_H_
