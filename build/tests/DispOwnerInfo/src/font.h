/********************************************************************/
/*      font.c                                                      */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	フォント処理ヘッダ


	$Log: font.h,v $
	Revision 1.2  2005/03/09 04:44:39  yosiokat
	機能追加。
	
	Revision 1.1.1.1  2004/08/31 06:20:24  Yosiokat
	no message
	



	// **** old logs ****

	Revision 1.7  2004/08/18 07:17:26  Yosiokat
	上下LCDをターゲットにして、別個に初期化できるよう変更。
	
	Revision 1.6  2004/08/17 07:52:03  Yosiokat
	・SetTargetScreenSJISを追加して、上下LCDのどちらにも文字表示が可能になるよう変更。
	
	Revision 1.5  2004/08/07 05:44:43  Yosiokat
	・SJIS文字列表示関数を引数でハンドルを指定しない仕様に変更する。
	・上記変更に対応して、クリア関数も仕様変更。
	
	Revision 1.4  2004/07/13 00:31:48  Yosiokat
	・サブLCD側のVRAMを対象にするよう変更。
	
	Revision 1.3  2004/06/06 02:39:31  Yosiokat
	SJISコード判定用の定数定義をfont.hに移動。
	
	Revision 1.2  2004/05/26 01:16:57  Yosiokat
	文字制御をSJISベースに変更中。
	
	Revision 1.1  2004/05/25 08:59:22  Yosiokat
	文字列をSJISで制御するように変更。
	

*/

#ifndef	__FONT_H_
#define	__FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>

// define data----------------------------------
#define	STR_ENTRY_MAX_NUM				256					// 登録可能な文字列データの最大個数

#define SJIS_CHAR_VRAM_OFFSET			0x100				// SJIS文字列キャラクタ用VRAMのオフセット値
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
u16  DrawStringSJIS ( u16 x, u16 y, u16 color, const void *str );
u16  DrawHexSJIS    ( u16 x, u16 y, u16 color, const void *hexp, u16 figure );
u16  DrawDecimalSJIS( u16 x, u16 y, u16 color, const void *hexp, u16 figure, u16 size );

// Ex系は、引数にindexを設けることで、上記関数で制限されている同一アドレスデータの複数場所配置に対応している。
u16	 DrawStringSJISEx ( u16 x, u16 y, u16 color, const void *strp, int index );
u16  DrawHexSJISEx    ( u16 x, u16 y, u16 color, const void *hexp, u16 figure, int index );
u16  DrawDecimalSJISEx( u16 x, u16 y, u16 color, const void *hexp, u16 figure, u16 size, int index );

// 表示文字列クリア関数
void ClearStringSJIS( void *datap );
void ClearStringSJISEx( void *datap, int handleIndex );
void ClearStringSJIS_handle( u16 handle );
void ClearAllStringSJIS( void );


#ifdef __cplusplus
}
#endif

#endif		// __FONT_H_

