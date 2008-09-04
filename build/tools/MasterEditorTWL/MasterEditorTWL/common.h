#pragma once

// ビルドスイッチ
#define METWL_VER_APPTYPE_USER
//#define METWL_VER_APPTYPE_SYSTEM
//#define METWL_VER_APPTYPE_SECURE
//#define METWL_VER_APPTYPE_LAUNCHER

//#define   METWL_WHETHER_PLATFORM_CHECK	// プラットフォームがTWL対応以外かどうかをチェックする
//#define   METWL_WHETHER_SIGN_DECRYPT		// 署名を外してダイジェストをベリファイする
#define   METWL_WHETHER_MRC					// MRC機能を含めるか

// 定数
#define   METWL_ERRLIST_NORANGE			0			// エラーのアドレス範囲を指定しないことを表す特別な値
#define   METWL_ROMSIZE_MIN				256			// 最小のデバイス容量 256Mbit
#define   METWL_ROMSIZE_MAX				4096		// 最大の..           4Gbit
#define   METWL_MASK_REGION_JAPAN       0x00000001
#define   METWL_MASK_REGION_AMERICA     0x00000002
#define   METWL_MASK_REGION_EUROPE      0x00000004
#define   METWL_MASK_REGION_AUSTRALIA   0x00000008
#define   METWL_MASK_REGION_CHINA       0x00000010
#define   METWL_MASK_REGION_KOREA       0x00000020
#define   METWL_MASK_REGION_ALL         0xffffffff
