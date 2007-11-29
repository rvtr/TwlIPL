/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     reset_param.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-29#$
  $Rev: 72 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#ifndef _RESET_PARAM_H_
#define _RESET_PARAM_H_

#include <twl.h>
#include <twl/nam.h>
#include <spi.h>


#ifdef __cplusplus
extern "C" {
#endif

// define data-------------------------------------------

// BootFlagsで使用するmedia情報
typedef enum TitleMedia {
	TITLE_MEDIA_NAND = 0,
	TITLE_MEDIA_CARD = 1,
	TITLE_MEDIA_MAX  = 2
}TitleMedia;


// タイトル＆リセットパラメータ　フラグ
typedef struct BootFlags {
	u16			isValid : 1;				// TRUE:valid, FALSE:invalid
	u16			media : 3;					// 0:nand, 1:card, 2-7:rsv;
	u16			isLogoSkip : 1;				// ロゴデモスキップ要求
	u16			isInitialShortcutSkip : 1;	// 初回起動シーケンススキップ要求
	u16			isAppLoadCompleted : 1;		// アプリロード済みを示す
	u16			isAppRelocate : 1;			// アプリ再配置要求
	u16			rsv : 9;
}BootFlags;


// リセットパラメータ　ヘッダ
typedef struct ResetParameterHeader {
	u32			magicCode;				// SYSM_RESET_PARAM_MAGIC_CODEが入る
	u8			type;					// タイプによってBodyを判別する。
	u8			bodyLength;				// bodyの長さ
	u16			crc16;					// bodyのCRC16
}ResetParamHeader;


// リセットパラメータ　ボディ
typedef union ResetParamBody {
	struct {							// ※とりあえず最初はTitlePropertyとフォーマットを合わせておく
		NAMTitleId	bootTitleID;		// リセット後にダイレクト起動するタイトルID
		BootFlags	flags;				// リセット時のランチャー動作フラグ
		u8			rsv[ 4 ];			// 予約
	}v1;
}ResetParamBody;


// リセットパラメータ
typedef struct ResetParam {
	ResetParamHeader	header;
	ResetParamBody		body;
}ResetParam;

// function's prototype------------------------------------

void RP_Reset( u8 type, NAMTitleId id, BootFlags *flag );

#ifdef __cplusplus
}       // extern "C"
#endif

#endif  // _RESET_PARAM_H_
