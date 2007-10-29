/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     NitroSettings.c

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


#ifndef	NITRO_SETTINGS_H_
#define	NITRO_SETTINGS_H_
#if		defined(SDK_CW)							// NitroConfigDataにビットフィールドを使っているので、コンパイラ依存で不具合が発生する可能性がある。
												// よって、CW以外のコンパイラの場合は、このヘッダを無効にしてエラーを出させるようにして再確認する。

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define NITRO_CONFIG_DATA_VERSION		5		// NITRO設定データフォーマットバージョン
#define NITRO_CONFIG_DATA_EX_VERSION	1		// 拡張NITRO設定データフォーマットバージョン
#define NCD_NICKNAME_LENGTH				10		// ニックネーム長
#define NCD_COMMENT_LENGTH				26		// コメント長
#define NCD_FAVORITE_COLOR_MAX_NUM		16		// 好きな色の最大数

// 言語設定コード
typedef enum NvLangCode{
	LANG_JAPANESE =0,							// 日本語
	LANG_ENGLISH  =1,							// 英語
	LANG_FRENCH   =2,							// フランス語
	LANG_GERMAN   =3,							// ドイツ語
	LANG_ITALIAN  =4,							// イタリア語
	LANG_SPANISH  =5,							// スペイン語
#ifdef IPL2_DEST_CHINA
  	LANG_CHINESE  =6,							// 中国語
#endif // IPL2_DEST_CHINA
#ifdef IPL2_DEST_KOREA
  	LANG_CHINESE  =6,							// 中国語
	LANG_HANGUL   =7,							// 韓国語
#endif // IPL2_DEST_KOREA
	LANG_CODE_MAX
}NvLangCode;

#define LANG_CODE_MAX_WW			( LANG_SPANISH + 1 )
#define LANG_BITMAP_WW			( 	( 0x0001 << LANG_JAPANESE )  \
								  | ( 0x0001 << LANG_ENGLISH  )  \
								  | ( 0x0001 << LANG_FRENCH   )  \
								  | ( 0x0001 << LANG_GERMAN   )  \
								  | ( 0x0001 << LANG_ITALIAN  )  \
								  | ( 0x0001 << LANG_SPANISH  ) ) 	// 通常版での対応言語ビットマップ

#define VALID_LANG_BITMAP			LANG_BITMAP_WW					// 本IPL2の対応言語ビットマップ


// 日付データ
typedef struct NvDate{
	u8				month;						// 月:01～12
	u8				day;						// 日:01～31
}NvDate;			// 2byte

// ニックネーム
typedef struct NvNickname{
	u16				str[NCD_NICKNAME_LENGTH];	// ニックネーム（Unicode(UTF16)で最大10文字、終端コードなし）
	u8				length;						// 文字数
	u8				rsv;
}NvNickname;		// 22byte

// コメント
typedef struct NvComment{
	u16				str[NCD_COMMENT_LENGTH];	//コメント（Unicode(UTF16)で最大26文字、終端コードなし）
	u8				length;						// 文字数
	u8				rsv;
}NvComment;			// 54byte

// オーナー情報
typedef struct NvOwnerInfo{
	u8				favoriteColor : 4;			// 好きな色
	u8				rsv : 4;					// 予約。
	NvDate			birthday;					// 生年月日
	u8				pad;
	NvNickname		nickname;					// ニックネーム
	NvComment		comment;					// コメント
}NvOwnerInfo;		// 80byte

// IPL用目覚まし時計データ
typedef struct NvAlarm{
	u8				hour;						// アラーム時:00～23
	u8				minute;						// アラーム分:00～59
	u8				rsv1;						// 予約
	u8				pad;
	u16				alarmOn : 1;				// アラームON,OFF（0:OFF, 1:ON）
	u16				rsv2 : 15;					// 予約
}NvAlarm;			// 6byte

// タッチパネルキャリブレーションデータ
typedef struct NvTpCalibData{
	u16				raw_x1;						// 第１キャリブレーション点のTP取得値X
	u16				raw_y1;						// 　　　　〃　　　　　　　　TP取得値Y
	u8				dx1;						// 　　　　〃　　　　　　　　LCD座標 X
	u8				dy1;						// 　　　　〃　　　　　　　　LCD座標 Y
	u16				raw_x2;						// 第２キャリブレーション点のTP取得値X
	u16				raw_y2;						// 　　　　〃　　　　　　　　TP取得値Y
	u8				dx2;   						// 　　　　〃　　　　　　　　LCD座標 X
	u8				dy2;   						// 　　　　〃　　　　　　　　LCD座標 Y
}NvTpCalibData;		// 12byte

// オプション情報
typedef struct NvOption{
	u16				language : 3;				// 言語コード（LANG_SPANISHまでの標準言語コードが入る）
	u16				agbLcd : 1;					// AGBモードで起動する時にどちらのLCDで起動するか？（0:TOP,1:BOTTOM）
	u16				backLightBrightness : 2;	// バックライト輝度データ
	u16				autoBootFlag : 1;			// 起動シーケンスで、メニュー停止なしで自動起動するかどうか？(0:OFF, 1:ON)
	u16				backLightOffFlag : 1;		// バックライトON,OFFフラグ（0:ON, 1:OFF）
	u16				rsv2 : 1;					// 予約
	u16				destroyFlashFlag : 1;		// フラッシュ壊れシーケンス中フラグ
	u16				input_birthday : 1;			// 誕生日が入力されたか？
	u16				input_favoriteColor : 1;	// 好きな色が入力されたか？
	u16				input_tp : 1;				// タッチパネルがキャリブレーションされたか？（  〃  )
	u16				input_language : 1;			// 言語入力がされたか？　		(0:未設定, 1:設定済み)
	u16				input_rtc : 1;				// RTC設定がされたか？			(		〃　　　　　 )
	u16				input_nickname : 1;			// ニックネームが入力されたか？	(		〃　　　　　 )
	u8				rtcLastSetYear;				// RTCの前回設定年
	u8				rtcClockAdjust;				// RTCクロック調整値
	s64				rtcOffset;					// RTC設定時のオフセット値（ユーザーがRTC設定を変更する度にその値に応じて増減します。）
}NvOption;			// 12byte

// NITRO各種設定データ
typedef struct NitroConfigData{
	u8				version;					// フラッシュ格納データフォーマットのバージョン
	u8				pad;
	NvOwnerInfo		owner;						// オーナー情報
	NvAlarm			alarm;						// IPL用目覚まし時計データ
	NvTpCalibData	tp;							// タッチパネルキャリブレーションデータ
	NvOption		option;						// オプション
}NitroConfigData;	// 112byte

// NITRO各種設定データのNVRAM保存時フォーマット
typedef struct NCDStore{
	NitroConfigData ncd;						// NITRO各種設定データ
	u16				saveCount;					// 0x00-0x7fをループしてカウントし、カウント値が新しいデータが有効。
	u16				crc16;						// NITRO各種設定データの16bitCRC
	u8				pad[ 128 - sizeof(NitroConfigData) - 4];
}NCDStore;			// 128byte					// ※本来なら、saveCountとcrc16は256byteの最後に付加して、間にパディングを埋める方がいい。


//----------------------------------------------
// IPL2中国・韓国版での拡張フォーマット
//----------------------------------------------
// 拡張NITRO設定データ
typedef struct NitroConfigDataEx{
	u8				version;					// バージョン
	u8				language;					// 言語コード（LANG_CHINESE以降に拡張された値が入る。）
	u16				valid_language_bitmap;		// 本IPL2で有効な言語コードを示したビットマップ
	u8				pad[ 256 - sizeof(NitroConfigData) - 4 - 4 - 2 ];		// 4:saveCount+crc16, 2:NCDEx.version+NCDEx.language, 2:crc16_ex
}NitroConfigDataEx;	// 138bytes

// NITRO各種設定データのNVRAM保存時フォーマット
typedef struct NCDStoreEx{
	NitroConfigData		ncd;					// NITRO各種設定データ
	u16					saveCount;				// 0x00-0x7fをループしてカウントし、カウント値が新しいデータが有効。
	u16					crc16;					// NITRO各種設定データの16bitCRC
	NitroConfigDataEx	ncd_ex;
	u16					crc16_ex;
}NCDStoreEx;		// 256byte					// ※本来なら、saveCountとcrc16は256byteの最後に付加して、間にパディングを埋める方がいい。


//=========================================================
// NVRAMへのリードライト関数
//=========================================================
#ifdef SDK_ARM9
extern int  NVRAMm_ReadNitroConfigData (NitroConfigData *dstp);
extern void NVRAMm_WriteNitroConfigData(NitroConfigData *dstp);
#endif


//=========================================================
// NITRO設定データへのアクセス関数
//=========================================================
extern NitroConfigDataEx ncdEx;

#define GetNCDWork()		( (NitroConfigData *)( HW_NVRAM_USER_INFO ) )
#define GetNCDExWork()		( &ncdEx )
												// NITRO設定データ領域のアドレス獲得

extern void NCD_ClearOwnerInfo( void );			// ニックネーム・誕生日・好きな色のクリア

//=========================================================
// データ取得
//=========================================================

//-----------------------------------
// オーナー情報全体の取得。
static inline NvOwnerInfo *NCD_GetOwnerInfo(void)
{
	return	&GetNCDWork()->owner;
}

// 好きな色の取得。
static inline u8 NCD_GetFavoriteColor(void)
{
	return	(u8)GetNCDWork()->owner.favoriteColor;
}

// 誕生日の取得。
static inline NvDate *NCD_GetBirthday(void)
{
	return	&GetNCDWork()->owner.birthday;
}

// ニックネームの取得。
static inline NvNickname *NCD_GetNickname(void)
{
	return	&GetNCDWork()->owner.nickname;
}

// コメントの取得。
static inline NvComment *NCD_GetComment(void)
{
	return	&GetNCDWork()->owner.comment;
}


//-----------------------------------
// アラーム情報の取得。
static inline NvAlarm *NCD_GetAlarmData(void)
{
	return	&GetNCDWork()->alarm;
}


//-----------------------------------
// タッチパネルキャリブレーションデータの取得。
static inline NvTpCalibData *NCD_GetTPCalibration(void)
{
	return	&GetNCDWork()->tp;
}


//-----------------------------------
// オプション情報の取得。

// 言語コードの取得
static inline NvLangCode NCD_GetLanguage(void)
{
  	return	(NvLangCode)GetNCDExWork()->language;
}

static inline NvLangCode NCD_GetLanguageOrg(void)
{
	return	(NvLangCode)GetNCDWork()->option.language;
}

// RTCオフセット値の取得
static inline s64 NCD_GetRtcOffset(void)
{
	return	GetNCDWork()->option.rtcOffset;
}

// RTCクロック調整値の取得
static inline u8 NCD_GetRtcClockAdjust(void)
{
	return	GetNCDWork()->option.rtcClockAdjust;
}

// RTCの前回セットした年の取得
static inline u8 NCD_GetRtcLastSetYear(void)
{
	return	GetNCDWork()->option.rtcLastSetYear;
}

// 起動シーケンスの自動起動ONか？（0:OFF, 1:ON）
static inline int NCD_GetAutoBootFlag(void)
{
	return	(int)GetNCDWork()->option.autoBootFlag;
}

// バックライト輝度取得（0-3）
static inline int NCD_GetBackLightBrightness(void)
{
	return	(int)GetNCDWork()->option.backLightBrightness;
}

// フラッシュ壊れシーケンス中かどうか？
static inline int NCD_GetDestroyFlash(void)
{
	return	(int)GetNCDWork()->option.destroyFlashFlag;
}

// 誕生日データがセットされているか？
static inline int NCD_GetInputBirthday(void)
{
	return	(int)GetNCDWork()->option.input_birthday;
}

// 好きな色データがセットされているか？
static inline int NCD_GetInputFavoriteColor(void)
{
	return	(int)GetNCDWork()->option.input_favoriteColor;
}

// TPキャリブレーションデータがセットされているか？
static inline int NCD_GetInputTP(void)
{
	return	(int)GetNCDWork()->option.input_tp;
}

// 言語コードがセットされているか？
static inline int NCD_GetInputLanguage(void)
{
	return	(int)GetNCDWork()->option.input_language;
}

// RTCデータがセットされているか？
static inline int NCD_GetInputRTC(void)
{
	return	(int)GetNCDWork()->option.input_rtc;
}

// オーナー情報のニックネームがセットされているか？
static inline int NCD_GetInputNickname(void)
{
	return	(int)GetNCDWork()->option.input_nickname;
}

//=========================================================
// データセット
//=========================================================
//-----------------------------------
// オーナー情報全体のセット。
static inline void NCD_SetOwnerInfo(NvOwnerInfo *owinfop)
{
	SVC_CpuCopy( owinfop, &GetNCDWork()->owner, sizeof(NvOwnerInfo), 16);
}

// 好きな色のセット。
static inline void NCD_SetFavoriteColor(u8 favoriteColor)
{
	GetNCDWork()->owner.favoriteColor = favoriteColor;
}

// 誕生日のセット。
static inline void NCD_SetBirthday(NvDate *birthp)
{
	GetNCDWork()->owner.birthday.month	= birthp->month;
	GetNCDWork()->owner.birthday.day	= birthp->day;
}

// ニックネームのセット。
static inline void NCD_SetNickname(NvNickname *namep)
{
	SVC_CpuCopy( namep, &GetNCDWork()->owner.nickname, sizeof(NvNickname), 16);
}

// コメントのセット。
static inline void NCD_SetComment(NvComment *commentp)
{
	SVC_CpuCopy( commentp, &GetNCDWork()->owner.comment, sizeof(NvComment), 16);
}


//-----------------------------------
// アラーム情報のセット。
static inline void NCD_SetAlarmData(NvAlarm *alarmp)
{
	SVC_CpuCopy( alarmp, &GetNCDWork()->alarm, sizeof(NvAlarm), 16);
}


//-----------------------------------
// タッチパネルキャリブレーションデータのセット。
static inline void NCD_SetTPCalibration(NvTpCalibData *tp_calibp)
{
	SVC_CpuCopy( tp_calibp, &GetNCDWork()->tp, sizeof(NvTpCalibData), 16);
}


//-----------------------------------
// オプション情報のセット。

// 言語コードのセット
static inline void NCD_SetLanguage(NvLangCode language)
{
#ifdef IPL2_DEST_WW
	GetNCDWork()->option.language = language;
#else // IPL2_DEST_WW
	GetNCDExWork()->language				= language;
	GetNCDExWork()->valid_language_bitmap	= VALID_LANG_BITMAP;
	
	if( language >= LANG_CODE_MAX_WW ) {
		GetNCDWork()->option.language = LANG_ENGLISH;
	}else {
		GetNCDWork()->option.language = language;
	}
#endif // IPL2_DEST_WW
}

// RTCオフセット値のセット
static inline void NCD_SetRtcOffset(s64 rtcOffset)
{
	GetNCDWork()->option.rtcOffset = rtcOffset;
}

// RTCクロック調整値のセット
static inline void NCD_SetRtcClockAdjust(u8 rtcClockAdjust)
{
	GetNCDWork()->option.rtcClockAdjust = rtcClockAdjust;
}


// RTCのLastSetYearへのセット
static inline void NCD_SetRtcLastSetYear(u8 rtcLastSetYear)
{
	GetNCDWork()->option.rtcLastSetYear = rtcLastSetYear;
}


// 起動シーケンスの自動起動ON,OFFフラグをセット。
static inline void NCD_SetAutoBootFlag(BOOL autoBootFlag)
{
	GetNCDWork()->option.autoBootFlag = (u16)autoBootFlag;
}

// バックライト輝度情報をセット。
static inline void NCD_SetBackLightBrightness(BOOL backLightBrightness )
{
	GetNCDWork()->option.backLightBrightness = (u16)backLightBrightness;
}

// フラッシュ壊れシーケンス中かどうかのフラグセット。
static inline void NCD_SetDestroyFlash(BOOL destroy)
{
	GetNCDWork()->option.destroyFlashFlag = (u16)destroy;
}

// 誕生日データの入力済みフラグセット。
static inline void NCD_SetInputBirthday(BOOL input)
{
	GetNCDWork()->option.input_birthday = (u16)input;
}

// 好きな色データの入力済みフラグセット。
static inline void NCD_SetInputFavoriteColor(BOOL input)
{
	GetNCDWork()->option.input_favoriteColor = (u16)input;
}

// TPキャリブレーションデータの入力済みフラグセット。
static inline void NCD_SetInputTP(BOOL input)
{
	GetNCDWork()->option.input_tp = (u16)input;
}

// 言語コードの入力済みフラグセット。
static inline void NCD_SetInputLanguage(BOOL input)
{
	GetNCDWork()->option.input_language = (u16)input;
}

// RTCデータの入力済みフラグセット。
static inline void NCD_SetInputRTC(BOOL input)
{
	GetNCDWork()->option.input_rtc = (u16)input;
}

// オーナー情報のニックネームの入力済みフラグセット。
static inline void NCD_SetInputNickname(BOOL input)
{
	GetNCDWork()->option.input_nickname = (u16)input;
}



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// NITRO_SETTINGS_H_
