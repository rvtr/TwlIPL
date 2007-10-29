/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     font.h

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

#ifndef	__FONT_H_
#define	__FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl.h>

// define data----------------------------------
#define	STR_ENTRY_MAX_NUM				256					// 登録可能な文字列データの最大個数

#define SJIS_CHAR_VRAM_OFFSET			0x4000				// SJIS文字列キャラクタ用VRAMのオフセット値
#define SJIS_CHAR_VRAM_SIZE				(0x8000 + 0x20)		// 　　〃　　　　　　　　　　のサイズ（0x20はヒープのヘッダ）

#define VRAM_M_ARENA_LO					(HW_BG_VRAM    + SJIS_CHAR_VRAM_OFFSET - 0x20)
#define VRAM_M_ARENA_HI					(VRAM_M_ARENA_LO + SJIS_CHAR_VRAM_SIZE)
#define VRAM_S_ARENA_LO					(HW_DB_BG_VRAM + SJIS_CHAR_VRAM_OFFSET - 0x20)
#define VRAM_S_ARENA_HI					(VRAM_S_ARENA_LO + SJIS_CHAR_VRAM_SIZE)
															// VRAMアリーナのLo & Hi
	// SJISコード判定用の値
#define SJIS_HIGHER_CODE1_MIN			0x81
#define SJIS_HIGHER_CODE1_MAX			0x9f
#define SJIS_HIGHER_CODE2_MIN			0xe0
#define SJIS_HIGHER_CODE2_MAX			0xea

// 関数のエラーリターン値
#define DSJIS_ERR_ENTRY_GET_FAILED		0x8000
#define DSJIS_ERR_ENTRY_ALLOC_FAILED	0x8001
#define DSJIS_ERR_CHAR_ALLOC_FAILED		0x8002
#define DSJIS_ERR_STR_MEMORY_OVER		0x8003
#define DSJIS_ERR_STR_LENGTH_TOO_LONG	0x8004

// SetTargetScreenSJISの引数target
typedef enum TargetScreen {
	TOP_SCREEN =0,
	BOTTOM_SCREEN
}TargetScreen;

// フォント種類データ（SelectFontで指定）
typedef enum FontType{										// 全角　　半角
	FONT12,													// 12x12 & 12x 7dot
	FONT_TYPE_MAX
}FontType;


// function's prototype declaration-------------

void InitFont( TargetScreen target );
void SetFont( FontType font );
void SetTargetScreenSJIS( TargetScreen target );
u16  ChangeColorSJIS( u16 handle, u16 new_color );

// 以下の表示関数は、データアドレスからデータハンドルを算出するので、ハンドルを引数で与えなくて良いが、同一アドレスのデータを複数場所に配置することができない。
u16  DrawStringSJIS ( int x, int y, u16 color, const void *str );
u16  DrawHexSJIS    ( int x, int y, u16 color, const void *hexp, u16 figure );
u16  DrawDecimalSJIS( int x, int y, u16 color, const void *hexp, u16 figure, u16 size );

// Ex系は、引数にindexを設けることで、上記関数で制限されている同一アドレスデータの複数場所配置に対応している。
u16	 DrawStringSJISEx ( int x, int y, u16 color, const void *strp, u32 index );
u16  DrawHexSJISEx    ( int x, int y, u16 color, const void *hexp, u16 figure, u32 index );
u16  DrawDecimalSJISEx( int x, int y, u16 color, const void *hexp, u16 figure, u16 size, u32 index );

// 表示文字列クリア関数
void ClearStringSJIS( void *datap );
void ClearStringSJISEx( void *datap, u32 handleIndex );
void ClearStringSJIS_handle( u16 handle );
void ClearAllStringSJIS( void );


#ifdef __cplusplus
}
#endif

#endif		// __FONT_H_

