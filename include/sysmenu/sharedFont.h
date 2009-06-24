/*---------------------------------------------------------------------------*
  Project:  TwlIPL
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

#ifndef	TWL_SHARED_FONT_H_
#define	TWL_SHARED_FONT_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// 共有フォントインデックス
typedef enum SFONT_Index {
	SHARED_FONT_WW_S = 0,
	SHARED_FONT_WW_M = 1,
	SHARED_FONT_WW_L = 2,
	SHARED_FONT_MAX  = 3
}SFONT_Index;


// 共有フォント初期化
BOOL SFONT_Init( void );

// 共有フォント　テーブルサイズ取得
int SFONT_GetInfoTableSize( void );

// 共有フォント　テーブルロード
BOOL SFONT_LoadInfoTable( void *pBuffer );

// 共有フォント　フォントサイズ取得
int SFONT_GetFontSize( SFONT_Index index );

// 共有フォント　フォントネーム取得
const u8 *SFONT_GetFontName( SFONT_Index index );

// 共有フォント　タイムスタンプ取得
u32 SFONT_GetFontTimestamp( void );

// 共有フォント　フォントロード
BOOL SFONT_LoadFont( SFONT_Index index, void *pBuffer );


#ifdef __cplusplus
}
#endif

#endif  // TWL_SHARED_FONT_H_
