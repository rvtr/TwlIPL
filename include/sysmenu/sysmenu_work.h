/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     sysmenu_work.c

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

#ifndef	__SYSMENU_WORK_H__
#define	__SYSMENU_WORK_H__

#include <twl.h>
#include <twl/rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

// compile switch ---------------------------------
#ifndef SDK_FINALROM

//#define __SYSM_DEBUG

#endif // SDK_FINALROM

//#define __DEBUG_SECURITY_CODE										// PassMeのセキュリティコード確認用スイッチ


// define data ------------------------------------
#define SYSMENU_VER							0x071029					// SystemMenuバージョン

#define PXI_FIFO_TAG_SYSM					PXI_FIFO_TAG_USER_1			// SystemMenu用のFIFOタグ

#define PAD_PRODUCTION_NITRO_SHORTCUT		( PAD_BUTTON_A | PAD_BUTTON_B	\
											| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
#define PAD_PRODUCTION_AGB_SHORTCUT			( PAD_BUTTON_A | PAD_BUTTON_B	\
											| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_L )
																		// 量産工程で使用するNITRO初回起動設定をキャンセルするショートカットキー


	// bootFlagの値
#define BFLG_EXIST_AGB_CARTRIDGE			0x00000001
#define BFLG_EXIST_NITRO_CARD				0x00000002
#define BFLG_ILLEGAL_NITRO_CARD				0x00000004
#define BFLG_ILLEGAL_BMENU					0x00000008
#define BFLG_BOOT_AGB						0x00000010
#define BFLG_BOOT_NITRO						0x00000020
#define BFLG_BOOT_BMENU						0x00000040
#define BFLG_BOOT_PICT_CHAT					0x00000080
#define BFLG_BOOT_WIRELESS_BOOT				0x00000100
#define BFLG_LOAD_CARD_COMPLETED			0x00000200
#define BFLG_LOAD_BMENU_COMPLETED			0x00000400
#define BFLG_LOAD_SYSM_DATA_COMPLETED		0x00000800
#define BFLG_REQ_UNCOMP_BMENU				0x00001000
#define BFLG_REQ_UNCOMP_SYSM_DATA			0x00002000
#define BFLG_ARM7_INIT_COMPLETED			0x00004000
#define BFLG_READ_NCD_COMPLETED				0x00008000
#define BFLG_SHORTCUT_CHECK_COMPLETED		0x00010000
#define BFLG_HOT_START						0x00020000
#define BFLG_BOOT_1SEG						0x00040000
#define BFLG_PERMIT_TO_BOOT					0x08000000
#define BFLG_SYSM_DATA_ENABLE				0x10000000
#define BFLG_CARD_CHECKED					0x20000000
#define BFLG_WM_INITIALIZED					0x40000000
#define BFLG_BOOT_DECIDED					0x80000000

#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2

	// mainp_stateの値
typedef enum MainpState {
	MAINP_STATE_INIT = 1,
	MAINP_STATE_START,
	MAINP_STATE_WAIT_BOOT_DECISION,
	MAINP_STATE_WAIT_NITRO_GAME_LOAD,
	MAINP_STATE_WAIT_READY_CHANGE_AGB,
	MAINP_STATE_WAIT_BMENU_LOAD,
	MAINP_STATE_WAIT_BOOT_REQ,
	MAINP_STATE_WAIT_START_NITRO_GAME_REQ,
	MAINP_STATE_BOOT_SELECTED_TARGET,
	MAINP_STATE_BOOT_AGB_REQ
}MainpState;


	// subp_stateの値
typedef enum SubpState {
	SUBP_STATE_INIT = 1,
	SUBP_STATE_STAY,
	SUBP_STATE_CLEAR_MAIN_MEMORY,
	SUBP_STATE_BOOT_NITRO_GAME_INIT,
	SUBP_STATE_LOAD_NITRO_GAME,
	SUBP_STATE_LOAD_BMENU,
	SUBP_STATE_BOOT_NITRO_GAME,
	SUBP_STATE_WAIT_START_BMENU_REQ,
	SUBP_STATE_START_BMENU,
	SUBP_STATE_BOOT_AGB,
	SUBP_STATE_BOOT_AGB_ACK,
	SUBP_STATE_BOOT_FAILED,
	SUBP_STATE_MB_BOOT,
	SUBP_STATE_TERMINATE_WM
}SubpState;


	// SYSMi_SendMessageToARM7(int msg)でARM9からARM7に通知するメッセージ
	// ARM7からのメッセージも含む
typedef enum SYSMMsg {
	MSG_INVALID = 0,												// 無効データ。
	
	MSG_UNCOMP_SYSM_DATA,											// ARM9にSYSM_dataを圧縮展開するよう要求。
	MSG_UNCOMP_BMENU,												// ARM9にbmenuを圧縮展開するよう要求。
	
	MSG_BOOT_TYPE_NITRO,											// ARM7に「NITROゲーム起動」を通知。
	MSG_BOOT_TYPE_AGB,												// ARM7に「AGB起動」を通知。
#ifndef __DS_CHAT_OFF
	MSG_BOOT_TYPE_PICT_CHAT,										// ARM7に「絵チャット起動」を通知。
#endif
	MSG_BOOT_TYPE_WIRELESS_BOOT,									// ARM7に「無線マルチブート起動」を通知。
	MSG_BOOT_TYPE_BMENU,											// ARM7に「ブートメニュー起動」を通知。
	MSG_START_BMENU,												// ARM7に「ブートメニュー開始」を通知。
	MSG_TERMINATE_WM												// ARM7に「WM終了」を通知。
}SYSMMsg;


//----------------------------------------------------------------------
//　データ型定義
//----------------------------------------------------------------------

#ifdef SDK_ARM7		// ※ARM7では、SDKのrtc/ARM9/api.h定義のこのデータはインクルードされないので、ここで定義。
// 曜日定義
typedef enum RTCWeek
{
	RTC_WEEK_SUNDAY = 0 ,				// 日曜日
	RTC_WEEK_MONDAY ,					// 月曜日
	RTC_WEEK_TUESDAY ,					// 火曜日
	RTC_WEEK_WEDNESDAY ,				// 水曜日
	RTC_WEEK_TURSDAY ,					// 木曜日
	RTC_WEEK_FRIDAY ,					// 金曜日
	RTC_WEEK_SATURDAY ,					// 土曜日
	RTC_WEEK_MAX
	
} RTCWeek;

// 午前・午後定義
typedef enum RTCNoon
{
	RTC_NOON_AM = 0,					// 午前
	RTC_NOON_PM ,						// 午後
	RTC_NOON_MAX
	
} RTCNoon;

// 日付構造体
typedef struct RTCDate
{
	u32			year;					// 年 ( 0 ~ 99 )
	u32			month;					// 月 ( 1 ~ 12 )
	u32			day;					// 日 ( 1 ~ 31 )
	RTCWeek		week;					// 曜日
	
} RTCDate;

// 時刻構造体
typedef struct RTCTime
{
	u32			hour;					// 時 ( 0 ~ 23 )
	u32			minute;					// 分 ( 0 ~ 59 )
	u32			second;					// 秒 ( 0 ~ 59 )
} RTCTime;
#endif	// SDK_ARM7


// スピンロック変数構造体
typedef struct LockVariable{
	OSLockWord			lock;
	vu32				value;
}LockVariable;


// RTC日付時刻構造体
typedef struct RtcDateTime {
	RTCDate				Date;
	RTCTime				Time;
}RtcDateTime;


// SYSM共有ワーク構造体
typedef struct SYSM_work{
	u32					card_arm7_ram_adr;							// NITROカードARM7初期ブートコードのRAMロードアドレス
	int					ncd_invalid;								// NITRO設定データ無効フラグ
	u32					ncd_rom_adr;								// NITRO設定データのROMアドレス
	u32					bm_arm7_ram_adr;							// ブートメニューARM9RAMアドレス
	u32					bm_arm7_comp_adr;							// ブートメニューARM7の圧縮バイナリRAMアドレス
	u16					sysm_data_crc16;
	u16					bm_crc16;
	u8					sysm_type;
	u8					pmic_type;									// デバッガのみで使用。
	u8					clone_boot_mode;
	
	
	u8					rtcStatus;
	u16					cardHeaderCrc16;
	u16					rsv;
	BOOL				isOnDebugger;
	BOOL				enableCardNormalOnly;
	u32					nCardID;									// NORMALカードID（LoadCardHeader() で取得）
	
	volatile MainpState	mainp_state;								// ARM9プログラムステート
	volatile SubpState	subp_state;									// ARM7プログラムステート
	LockVariable		boot_flag;									// ブート状態フラグ（SYSM_GetBootFlag(),SetBootFlag()でアクセスを行います。）
	RtcDateTime			rtc[2];										// RTC時間データ([0]:起動時の値、[1]:ゲームブート直前の値）
//	u32					mb_flag;
//	u32					mb_ggid;
}SYSM_work;


//----------------------------------------------------------------------
//　SYSM共有ワーク領域のアドレス獲得
//----------------------------------------------------------------------

#define GetSYSMWork()				( (SYSM_work *)HW_RED_RESERVED )

//・SYSM共有ワーク領域のアドレスを獲得します。

//----------------------------------------------------------------------
//　bootFlagのリード
//----------------------------------------------------------------------
#define SYSM_GetBootFlag()			( *(vu32 *)&GetSYSMWork()->boot_flag.value )

//・bootFlag値を獲得します。


#ifdef __cplusplus
}
#endif

#endif		// __SYSMENU_WORK_H__

