/********************************************************************/
/*      NitroConfigData.h                                           */
/*          NITRO-IPL                                               */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	NITR設定データ定義　ヘッダ


	$Log: NitroConfigData.h,v $
	Revision 1.4.2.2.4.1.4.1  2007/01/22 07:36:16  yosiokat
	NAT-IPL2への対応。
	
	Revision 1.4.2.5  2006/06/26 02:57:54  yosiokat
	・言語コードを韓国語に対応させる。
	・一部の定義データの名称変更。
	
	Revision 1.4.2.4  2006/02/06 11:34:56  yosiokat
	NCD_GetIPL2BM7RomAddr関数の追加。
	
	Revision 1.4.2.3  2006/02/06 08:27:19  yosiokat
	IPL2バージョンの定義をIPLType.hの定義値を使うよう変更。
	
	Revision 1.4.2.2  2005/10/31 08:49:57  yosiokat
	USG_BACKLIGHT_DEFAULTを"2"に変更。
	
	Revision 1.4.2.1  2005/10/25 08:14:41  yosiokat
	USG対応のため、以下の変更を行う。
	・NitroConfigData構造体にbackLightBrightnessを追加。
	・IPL2_TYPE判定用の定数定義にUSGのものを追加。
	
	Revision 1.4  2005/04/01 05:45:04  yosiokat
	最新版に更新。
	
	Revision 1.3  2005/03/31 06:06:54  yosiokat
	NitroConfigDataアクセス関数の全面改定。
	
	Revision 1.31.2.2  2005/03/16 00:36:01  yosiokat
	言語コードの中国語追加に伴うNitroConfigDataExの新設とNitroConfigDataの取扱いの変更。
	
	Revision 1.31.2.1  2005/03/11 11:08:56  yosiokat
	中国語の追加。
	
	Revision 1.31  2005/02/15 02:38:27  yosiokat
	NCD_CorrectDataの削除。
	
	Revision 1.30  2005/02/07 11:12:03  yosiokat
	NCD_CorrectDataの追加。
	
	Revision 1.29  2004/09/25 10:42:54  Yosiokat
	NvOption内のrsv領域にrtcOffset算出用パラメータrtcLastSetYearを保存するよう変更。
	
	Revision 1.28  2004/09/16 07:02:51  Yosiokat
	オーナー情報のニックネーム、好きな色、誕生日をクリアするNCD_ClearOwnerInfoの追加。
	
	Revision 1.27  2004/09/01 09:18:35  Yosiokat
	・NvOptionにdestroyFlashFlagを追加。
	・上記アクセス関数NCD_GetDestroyFlash、NCD_SetDestroyFlashを追加。
	
	Revision 1.26  2004/08/31 09:52:38  Yosiokat
	small fix.
	
	Revision 1.25  2004/08/27 12:36:08  Yosiokat
	IPL2_BUILD_をSDK_SMALL_BUILDに変更。
	
	Revision 1.24  2004/08/25 09:27:41  Yosiokat
	・必要なくなった下記要素をrsvに変更。（フォーマットの互換を保つため、データは詰めない）
	  NvAlarm.second
	  NvAlarm.enableWeek
	  NvOption.detectPullOutCardFlag
	  NvOption.detectPullOutCtrdgFlag
	  NvOption.timezone
	
	Revision 1.23  2004/08/25 05:12:20  Yosiokat
	ブランチタグred_ipl2_2004_08_24_pp2_isdbg_fixとのマージ。
	
	Revision 1.22  2004/08/25 01:28:23  Yosiokat
	・NvOption.backLightOffFlagがビットフィールドになっていなかったのを修正。
	・NvOption.input_birthdayフラグを追加。アクセス関数NCD_GetInputBirthday、NCD_SetInputBirthday
	・NCDStoreを32byte単位のサイズになるよう調整。（キャッシュラインにあわせる）
	
	Revision 1.21  2004/08/23 08:24:23  Yosiokat
	バックライトON,OFFフラグをIPl2_workから再度NitroConfigDataに戻す。
	
	Revision 1.20  2004/08/19 06:06:02  yosiokat
	NitroConfigDataにビットフィールドが使われているため、他コンパイラで問題が出る可能性がある。
	とりあえず、SDK_CWが定義されていなければ、ヘッダを切ってエラーを出させることで、他コンパイラで問題がないかを確認できるようにしておく。
	
	Revision 1.19  2004/08/18 07:33:00  Yosiokat
	・NCD_FAVORITE_COLOR_MAX_NUM追加。
	・NitroConfigData.option.input_favoriteColor追加。
	
	Revision 1.18  2004/08/17 05:44:19  Nakasima
	・topLcdBackLightOffをNitroConfigDataからIPL2_workへ移動。
	・detectPullOutFlagを削除し、detectPullOutCardFlaとdetectPullOutCtrdgFlagを追加。
	
	Revision 1.17  2004/08/16 10:13:09  Yosiokat
	pullCardFlagをdetectPullOutFlagに変更。
	
	Revision 1.16  2004/08/16 10:09:34  Yosiokat
	NvOptionにpullCardFlagを追加。
	
	Revision 1.15  2004/08/13 07:41:30  Yosiokat
	・NvOwnerInfoからsexを削除。
	・NvOptionからbottomLcdBackLightOffを削除。
	・NvOptionにautoBootFlagを追加。
	・NCD_SetSex, NCD_GetSexを削除。
	・NCD_GetRtcClockAdjust、NCD_GetAutoBootFlag、NCD_SetRtcClockAdjust、NCD_SetAutoBootFlagの追加。
	
	Revision 1.14  2004/07/29 04:53:18  Yosiokat
	構造体のパディングを明示的に追加。
	
	Revision 1.13  2004/07/18 10:55:13  Yosiokat
	NITRO設定データのセット関数を用意。
	
	Revision 1.12  2004/07/17 09:00:14  Yosiokat
	・構造体サイズのコメント修正。
	
	Revision 1.11  2004/07/15 12:47:33  Yosiokat
	・オーナー情報の誕生日の「年」を削除。
	
	Revision 1.10  2004/07/15 12:26:43  Yosiokat
	・NITRO設定データのフォーマットを変更。
	・オーナー情報を以下のように変更。
	　a)ニックネーム長を１０に変更。
	　b)コメントデータを追加
	　c)血液型データを削除。
	　d)好きな色データを追加。（中身は未定）
	  e)性別データをビットフィールドに。
	・アラーム情報を追加。
	・タッチパネルキャリブレーション情報の各要素を詳細に定義。
	・その他の情報をオプション情報にまとめる。（言語コードもビットフィールドとしてここに入れる。）
	
	Revision 1.9  2004/07/13 00:20:15  Yosiokat
	・small fix.
	
	Revision 1.8  2004/07/05 02:29:23  Yosiokat
	言語コードをFRENCHに修正。
	
	Revision 1.7  2004/06/28 01:56:17  Yosiokat
	・バックライトON,OFF設定の値を追加。
	・TP、言語設定、RTC、オーナー情報の入力済みフラグを追加。
	・バージョンを"3"に。
	
	Revision 1.6  2004/06/14 04:55:10  yosiokat
	NitroConfigDataのrtcOffsetをintからs64に変更。
	これに伴いNCDフォーマットのバージョンを"2"に変更。
	
	Revision 1.5  2004/06/07 10:51:58  Yosiokat
	オーナー情報のニックネームをSJISからUnicode（UTF16）に変更したため、バージョンを１に上げる。
	
	Revision 1.4  2004/05/21 06:12:17  Yosiokat
	ヘッダのC++対応部分で「};」のセミコロンがエラーになっていたので、削除。
	
	Revision 1.3  2004/05/21 05:37:46  Yosiokat
	TPキャリブレーションをNVRAMに保存するデータにLCDの左上ポイント＋LCDの右下ポイントを含めるよう変更。（サイズが8->12byteへ）
	また、SDKにconfig.hの名前でNitroConfigData.hと同一ファイルが入っていたので、これをインクルードしないよう、Makefileでコンパイル時にIPL2_BUILD_シンボルを与えるよう変更。
	
	Revision 1.2  2004/05/19 08:25:28  yosiokat
	更新ログの追加。
	

*/


#ifndef	NITRO_CONFIG_DATA_H_
#define	NITRO_CONFIG_DATA_H_
#if		defined(SDK_CW)							// NitroConfigDataにビットフィールドを使っているので、コンパイラ依存で不具合が発生する可能性がある。
												// よって、CW以外のコンパイラの場合は、このヘッダを無効にしてエラーを出させるようにして再確認する。
#ifdef __cplusplus
extern "C" {
#endif


#include <nitro.h>

#define USING_COMPONENT							// IPL2上ではないので、このスイッチを有効に。


// define data ------------------------------------
#define NITRO_CONFIG_DATA_VERSION		5		// NITRO設定データフォーマットバージョン
#define NITRO_CONFIG_DATA_EX_VERSION	1		// 拡張NITRO設定データフォーマットバージョン
#define NCD_NICKNAME_LENGTH				10		// ニックネーム長
#define NCD_COMMENT_LENGTH				26		// コメント長
#define NCD_FAVORITE_COLOR_MAX_NUM		16		// 好きな色の最大数

#define NCD_ROM_ADDR_SHIFT				3		// IPL2ヘッダのNitroConfigData格納アドレスのシフト値
#define FONT_ROM_ADDR_SHIFT				2		// IPL2ヘッダのフォントデータ格納アドレスのシフト値
#define NCD_SYS_RSV_SIZE				1024	// システム予約領域サイズ
#define NCD_APP_RSV_SIZE				512		// アプリ　予約領域サイズ
												// NCD_SYS_RSV_ROM_ADDR = ncd_rom_addr - NCD_SYS_RSV_SIZE
												// NCD_APP_RSV_ROM_ADDR = ncd_rom_addr - NCD_SYS_RSV_SIZE - NCD_APP_RSV_SIZE
#define USG_BACKLIGHT_DEFAULT			2		// USGのバックライトデフォルト値


// 言語設定コード
typedef enum NvLangCode{
	LANG_JAPANESE =0,							// 日本語
	LANG_ENGLISH,								// 英語
	LANG_FRENCH,								// フランス語
	LANG_GERMAN,								// ドイツ語
	LANG_ITALIAN,								// イタリア語
	LANG_SPANISH,								// スペイン語
	LANG_CHINESE,								// 中国語
	LANG_HANGUL,								// 韓国語
	LANG_CODE_MAX
}NvLangCode;

#define LANG_CODE_MAX_WW			LANG_CHINESE					// 中国版以前の通常版IPL2でのLANG_CODE_MAX

#define LANG_BITMAP_WW			( 	( 0x0001 << LANG_JAPANESE )  \
								  | ( 0x0001 << LANG_ENGLISH  )  \
								  | ( 0x0001 << LANG_FRENCH   )  \
								  | ( 0x0001 << LANG_GERMAN   )  \
								  | ( 0x0001 << LANG_ITALIAN  )  \
								  | ( 0x0001 << LANG_SPANISH  ) ) 	// 通常版での対応言語ビットマップ

#define LANG_BITMAP_CN			( 	( 0x0001 << LANG_ENGLISH  )  \
								  | ( 0x0001 << LANG_FRENCH   )  \
								  | ( 0x0001 << LANG_GERMAN   )  \
								  | ( 0x0001 << LANG_ITALIAN  )  \
								  | ( 0x0001 << LANG_SPANISH  )  \
								  | ( 0x0001 << LANG_CHINESE  ) ) 	// 中国版での対応言語ビットマップ

#define LANG_BITMAP_KR			( 	( 0x0001 << LANG_JAPANESE )  \
								  | ( 0x0001 << LANG_ENGLISH  )  \
								  | ( 0x0001 << LANG_FRENCH   )  \
								  | ( 0x0001 << LANG_GERMAN   )  \
								  | ( 0x0001 << LANG_SPANISH  )  \
								  | ( 0x0001 << LANG_HANGUL  ) ) 	// 韓国版での対応言語ビットマップ

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
	u16				rsv2 : 1;					// 予約。
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
// IPL2中国版での拡張フォーマット
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


typedef struct NCDTimeStamp {
	u8 minute;
	u8 hour;
	u8 day;
	u8 month;
	u8 year;
}NCDTimeStamp;


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
extern u16				 valid_lang_bitmap;

#define GetNcdWorkAddr()		( (NitroConfigData   *)( HW_NVRAM_USER_INFO ) )
#define GetNcdExWorkAddr()		( &ncdEx )
												// NITRO設定データ領域のアドレス獲得

extern void NCD_ClearOwnerInfo( void );			// ニックネーム・誕生日・好きな色のクリア

#ifdef USING_COMPONENT
extern void NCD_ReadIPL2Header  ( void );		// IPL2ヘッダの読み出し。
extern u8   NCD_GetIPL2Type     ( void );		// IPL2タイプの取得。
extern u16  NCD_GetIPL2TypeEx   ( void );		// 拡張IPL2タイプの取得。
extern u8  *NCD_GetIPL2Version  ( void );		// IPL2バージョンの取得。
extern u32  NCD_GetNCDRomAddr   ( void );		// NCD格納ROMアドレスの取得。
extern u32  NCD_GetSysRsvRomAddr( void );		// システム予約領域ROMアドレスの取得。
extern u32  NCD_GetAppRsvRomAddr( void );		// アプリ　予約領域ROMアドレスの取得。
extern u32  NCD_GetIPL2BM7RomAddr( void );		// ブートメニューARM7コードROMアドレスの取得。
extern u32  NCD_GetIPL2DataRomAddr( void );		// IPL2データROMアドレスの取得。
extern u32  NCD_GetFontBncmpRomAddr( void );	// フォントデータbncmpのROMアドレス取得。（中国版、日本向け試遊台版でのみ有効。）
extern u32  NCD_GetFontBnfrRomAddr( void );		// フォントデータbnfr のROMアドレス取得。（中国版、日本向け試遊台版でのみ有効。）
#endif

//=========================================================
// データ取得
//=========================================================

//-----------------------------------
// オーナー情報全体の取得。
static inline NvOwnerInfo *NCD_GetOwnerInfo(void)
{
	return	&GetNcdWorkAddr()->owner;
}

// 好きな色の取得。
static inline u8 NCD_GetFavoriteColor(void)
{
	return	(u8)GetNcdWorkAddr()->owner.favoriteColor;
}

// 誕生日の取得。
static inline NvDate *NCD_GetBirthday(void)
{
	return	&GetNcdWorkAddr()->owner.birthday;
}

// ニックネームの取得。
static inline NvNickname *NCD_GetNickname(void)
{
	return	&GetNcdWorkAddr()->owner.nickname;
}

// コメントの取得。
static inline NvComment *NCD_GetComment(void)
{
	return	&GetNcdWorkAddr()->owner.comment;
}


//-----------------------------------
// アラーム情報の取得。
static inline NvAlarm *NCD_GetAlarmData(void)
{
	return	&GetNcdWorkAddr()->alarm;
}


//-----------------------------------
// タッチパネルキャリブレーションデータの取得。
static inline NvTpCalibData *NCD_GetTPCalibration(void)
{
	return	&GetNcdWorkAddr()->tp;
}


//-----------------------------------
// オプション情報の取得。

// 言語コードの取得
static inline NvLangCode NCD_GetLanguageOrg(void)
{
	return	(NvLangCode)GetNcdWorkAddr()->option.language;
}

static inline NvLangCode NCD_GetLanguage(void)
{
	return	(NvLangCode)GetNcdExWorkAddr()->language;
}


// RTCオフセット値の取得
static inline s64 NCD_GetRtcOffset(void)
{
	return	GetNcdWorkAddr()->option.rtcOffset;
}

// RTCクロック調整値の取得
static inline u8 NCD_GetRtcClockAdjust(void)
{
	return	GetNcdWorkAddr()->option.rtcClockAdjust;
}

// RTCの前回セットした年の取得
static inline u8 NCD_GetRtcLastSetYear(void)
{
	return	GetNcdWorkAddr()->option.rtcLastSetYear;
}

// 起動シーケンスの自動起動ONか？（0:OFF, 1:ON）
static inline int NCD_GetAutoBootFlag(void)
{
	return	(int)GetNcdWorkAddr()->option.autoBootFlag;
}

// バックライトOFFか？（0:ON, 1:OFF）
static inline int NCD_GetBackLightOffFlag(void)
{
	return	(int)GetNcdWorkAddr()->option.backLightOffFlag;
}

// フラッシュ壊れシーケンス中かどうか？
static inline int NCD_GetDestroyFlash(void)
{
	return	(int)GetNcdWorkAddr()->option.destroyFlashFlag;
}

// 誕生日データがセットされているか？
static inline int NCD_GetInputBirthday(void)
{
	return	(int)GetNcdWorkAddr()->option.input_birthday;
}

// 好きな色データがセットされているか？
static inline int NCD_GetInputFavoriteColor(void)
{
	return	(int)GetNcdWorkAddr()->option.input_favoriteColor;
}

// TPキャリブレーションデータがセットされているか？
static inline int NCD_GetInputTP(void)
{
	return	(int)GetNcdWorkAddr()->option.input_tp;
}

// 言語コードがセットされているか？
static inline int NCD_GetInputLanguage(void)
{
	return	(int)GetNcdWorkAddr()->option.input_language;
}

// RTCデータがセットされているか？
static inline int NCD_GetInputRTC(void)
{
	return	(int)GetNcdWorkAddr()->option.input_rtc;
}

// オーナー情報のニックネームがセットされているか？
static inline int NCD_GetInputNickname(void)
{
	return	(int)GetNcdWorkAddr()->option.input_nickname;
}

//=========================================================
// データセット
//=========================================================
//-----------------------------------
// オーナー情報全体のセット。
static inline void NCD_SetOwnerInfo(NvOwnerInfo *owinfop)
{
	SVC_CpuCopy( owinfop, &GetNcdWorkAddr()->owner, sizeof(NvOwnerInfo), 16);
}

// 好きな色のセット。
static inline void NCD_SetFavoriteColor(u8 favoriteColor)
{
	GetNcdWorkAddr()->owner.favoriteColor = favoriteColor;
}

// 誕生日のセット。
static inline void NCD_SetBirthday(NvDate *birthp)
{
	GetNcdWorkAddr()->owner.birthday.month	= birthp->month;
	GetNcdWorkAddr()->owner.birthday.day	= birthp->day;
}

// ニックネームのセット。
static inline void NCD_SetNickname(NvNickname *namep)
{
	SVC_CpuCopy( namep, &GetNcdWorkAddr()->owner.nickname, sizeof(NvNickname), 16);
}

// コメントのセット。
static inline void NCD_SetComment(NvComment *commentp)
{
	SVC_CpuCopy( commentp, &GetNcdWorkAddr()->owner.comment, sizeof(NvComment), 16);
}


//-----------------------------------
// アラーム情報のセット。
static inline void NCD_SetAlarmData(NvAlarm *alarmp)
{
	SVC_CpuCopy( alarmp, &GetNcdWorkAddr()->alarm, sizeof(NvAlarm), 16);
}


//-----------------------------------
// タッチパネルキャリブレーションデータのセット。
static inline void NCD_SetTPCalibration(NvTpCalibData *tp_calibp)
{
	SVC_CpuCopy( tp_calibp, &GetNcdWorkAddr()->tp, sizeof(NvTpCalibData), 16);
}


//-----------------------------------
// オプション情報のセット。

// 言語コードのセット
#if 0
static inline void NCD_SetLanguage(NvLangCode language)
{
	GetNcdExWorkAddr()->language				= language;
	GetNcdExWorkAddr()->valid_language_bitmap	= LANG_BITMAP_CHINESE;
	
	if( language == LANG_CHINESE ) {
		GetNcdWorkAddr()->option.language = LANG_ENGLISH;
	}else {
		GetNcdWorkAddr()->option.language = language;
	}
}
#endif

// RTCオフセット値のセット
static inline void NCD_SetRtcOffset(s64 rtcOffset)
{
	GetNcdWorkAddr()->option.rtcOffset = rtcOffset;
}

// RTCクロック調整値のセット
static inline void NCD_SetRtcClockAdjust(u8 rtcClockAdjust)
{
	GetNcdWorkAddr()->option.rtcClockAdjust = rtcClockAdjust;
}


// RTCのLastSetYearへのセット
static inline void NCD_SetRtcLastSetYear(u8 rtcLastSetYear)
{
	GetNcdWorkAddr()->option.rtcLastSetYear = rtcLastSetYear;
}


// 起動シーケンスの自動起動ON,OFFフラグをセット。
static inline void NCD_SetAutoBootFlag(BOOL autoBootFlag)
{
	GetNcdWorkAddr()->option.autoBootFlag = (u16)autoBootFlag;
}

// バックライトON,OFFフラグをセット。
static inline void NCD_SetBackLightOffFlag(BOOL backLightOffFlag)
{
	GetNcdWorkAddr()->option.backLightOffFlag = (u16)backLightOffFlag;
}

// フラッシュ壊れシーケンス中かどうかのフラグセット。
static inline void NCD_SetDestroyFlash(BOOL destroy)
{
	GetNcdWorkAddr()->option.destroyFlashFlag = (u16)destroy;
}

// 誕生日データの入力済みフラグセット。
static inline void NCD_SetInputBirthday(BOOL input)
{
	GetNcdWorkAddr()->option.input_birthday = (u16)input;
}

// 好きな色データの入力済みフラグセット。
static inline void NCD_SetInputFavoriteColor(BOOL input)
{
	GetNcdWorkAddr()->option.input_favoriteColor = (u16)input;
}

// TPキャリブレーションデータの入力済みフラグセット。
static inline void NCD_SetInputTP(BOOL input)
{
	GetNcdWorkAddr()->option.input_tp = (u16)input;
}

// 言語コードの入力済みフラグセット。
static inline void NCD_SetInputLanguage(BOOL input)
{
	GetNcdWorkAddr()->option.input_language = (u16)input;
}

// RTCデータの入力済みフラグセット。
static inline void NCD_SetInputRTC(BOOL input)
{
	GetNcdWorkAddr()->option.input_rtc = (u16)input;
}

// オーナー情報のニックネームの入力済みフラグセット。
static inline void NCD_SetInputNickname(BOOL input)
{
	GetNcdWorkAddr()->option.input_nickname = (u16)input;
}



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// NITRO_CONFIG_DATA_H_
