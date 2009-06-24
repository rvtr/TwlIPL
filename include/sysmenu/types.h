/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     types.h

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

#ifndef	__SYSM_TYPES_H__
#define	__SYSM_TYPES_H__

#include <twl.h>
#include <twl/os/common/format_rom.h>
#include <../build/libraries/os/common/include/application_jump_private.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

// NAMTitleIDをHiLoに分割してアクセスする場合に使用
typedef struct TitleID_HiLo {
	u8			Lo[ 4 ];
	u32			Hi;
}TitleID_HiLo;


// タイトル情報サブ
typedef struct TitleInfoSub {
	RomExpansionFlags	exFlags;
	char				platform_code;
	u8					parental_control_rating_info[ PARENTAL_CONTROL_INFO_SIZE ];
	u32					card_region_bitmap;
	u8					agree_EULA_version;
}TitleInfoSub;


// タイトル情報
typedef struct TitleProperty {			// この情報は、ランチャー時には認証通ってないけど、起動時には認証通すので大丈夫だろう。
	NAMTitleId			titleID;		// タイトルID（TitleID_Hiで起動メディアは判定できる？）
	LauncherBootFlags	flags;			// ブート時のランチャー動作フラグ
	TWLBannerFile		*pBanner;		// バナーへのポインタ（固定長フォーマットなら偽造されても大丈夫だろう。)
	TitleInfoSub		sub_info;
}TitleProperty;


#ifdef __cplusplus
}
#endif

#endif  // __SYSM_TYPES_H__
