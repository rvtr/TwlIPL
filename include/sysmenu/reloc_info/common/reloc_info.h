/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     reloc_info.h

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

#ifndef	__SYSMENU_RELOC_INFO_H__
#define	__SYSMENU_RELOC_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
//　データ型定義
//----------------------------------------------------------------------

#define RELOCATE_INFO_NUM					4 // ROM再配置情報の数（今のところarm9,arm7それぞれltdとflxで最大4つ）
#define DEST_LIST_NUM						(RELOCATE_INFO_NUM + 1)

// 再配置情報データ構造体
typedef struct Relocate_Info
{
	u32				src;
	u32				dest;
	u32				length;
	u32				post_clear_addr;
	u32				post_clear_length;
	BOOL			rev;
}Relocate_Info;

// ROMセグメント名
typedef enum RomSegmentName {
	ARM9_STATIC = 0,
	ARM7_STATIC = 1,
	ARM9_LTD_STATIC = 2,
	ARM7_LTD_STATIC = 3
}RomSegmentName;

//----------------------------------------------------------------------
//　関数宣言
//----------------------------------------------------------------------

// ロード領域のチェック及び再配置情報の生成
BOOL SYSM_CheckLoadRegionAndSetRelocateInfo( RomSegmentName seg, u32 *dest, u32 length, Relocate_Info *info, BOOL isTwlApp );

#ifdef __cplusplus
}
#endif

#endif // __SYSMENU_RELOC_INFO_H__

