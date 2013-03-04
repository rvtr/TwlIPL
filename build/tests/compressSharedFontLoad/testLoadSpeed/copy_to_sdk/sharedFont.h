/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     sharedFont.h

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

#ifndef	TWL_OS_SHARED_FONT_H_
#define	TWL_OS_SHARED_FONT_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9

// 共有フォントインデックス
typedef enum OSSharedFontIndex {
	OS_SHARED_FONT_WW_L = 0,
	OS_SHARED_FONT_WW_M = 1,
	OS_SHARED_FONT_WW_S = 2,
	OS_SHARED_FONT_MAX  = 3
}OSSharedFontIndex;


// 共有フォント初期化
BOOL OS_InitSharedFont( void );

// 共有フォント　テーブルサイズ取得
int OS_GetSharedFontTableSize( void );

// 共有フォント　テーブルロード
BOOL OS_LoadSharedFontTable( void *pBuffer );

// 共有フォント　フォントサイズ取得
int OS_GetSharedFontSize( OSSharedFontIndex index );

// 共有フォント  圧縮後サイズ取得
int OS_GetSharedFontCompressedSize( OSSharedFontIndex index );

// 共有フォント　フォントネーム取得
const u8 *OS_GetSharedFontName( OSSharedFontIndex index );

// 共有フォント　タイムスタンプ取得
u32 OS_GetSharedFontTimestamp( void );

// 共有フォント　フォントロード
BOOL OS_LoadSharedFont( OSSharedFontIndex index, void *pBuffer );

#endif // SDK_ARM9

#ifdef __cplusplus
}
#endif

#endif  // TWL_OS_SHARED_FONT_H_
