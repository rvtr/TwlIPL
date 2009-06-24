/********************************************************************/
/*      dispNCD.c                                                   */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	NITRO設定データ表示

	$Log: dispNCD.c,v $
	Revision 1.7.2.2.8.1  2007/01/22 07:36:16  yosiokat
	NAT-IPL2への対応。
	
	Revision 1.7.2.3  2006/06/26 04:09:02  yosiokat
	DrawNCD_Elementの言語コードの表示判定部を修正。
	
	Revision 1.7.2.2  2006/02/01 06:32:43  yosiokat
	IPL2バージョン情報取得関数の整理。
	
	Revision 1.7.2.1  2005/10/25 08:19:39  yosiokat
	・中国版かどうかの判定を関数IsIPLTypeChineseで行うよう変更。
	・USG版の時に、バックライト輝度情報（０～３）を表示するよう変更。
	
	Revision 1.7  2005/04/14 05:53:47  yosiokat
	NITRO設定データ関係を強制的に中国版にするスイッチFORCE_CHINAを追加。
	
	Revision 1.6  2005/04/02 10:04:08  yosiokat
	ユーザーカラーの色表示に対応。
	
	Revision 1.5  2005/04/02 07:42:54  yosiokat
	ユーザーネーム＆コメントをIPL2フォントを使用して全て表示できるよう変更。
	
	Revision 1.4  2005/04/01 05:47:24  yosiokat
	NitroConfigDataEx対応IPL2の時には、言語コードをEx側も表示するよう変更。
	
	Revision 1.3  2005/03/31 05:59:15  yosiokat
	・言語コードにCHINESE、UNKNOWNの追加。
	・TPデータワークのクリアをInitTPData()に変更。
	
	Revision 1.2  2005/03/09 04:44:39  yosiokat
	機能追加。
	
	Revision 1.1.1.1  2004/08/31 06:20:24  Yosiokat
	no message
	

*/

#include <nitro.h>
#include "main.h"
#include "NitroConfigData.h"
#include "font.h"

// define data------------------------------------------
#define CANCEL_BUTTON_LT_X					2
#define CANCEL_BUTTON_LT_Y					22
#define CANCEL_BUTTON_RB_X					(CANCEL_BUTTON_LT_X+8)
#define CANCEL_BUTTON_RB_Y					(CANCEL_BUTTON_LT_Y+2)

#define IPL2_GAIJI_SIKAKU					0x25a0

/* 本体情報 : 「お気に入りの色」定数 (GXRgb) */
#define	FAVORITE_COLOR_VALUE_GRAY         GX_RGB(12,16,19)
#define	FAVORITE_COLOR_VALUE_BROWN        GX_RGB(23, 9, 0)
#define	FAVORITE_COLOR_VALUE_RED          GX_RGB(31, 0, 3)
#define	FAVORITE_COLOR_VALUE_PINK         GX_RGB(31,17,31)
#define	FAVORITE_COLOR_VALUE_ORANGE       GX_RGB(31,18, 0)
#define	FAVORITE_COLOR_VALUE_YELLOW       GX_RGB(30,28, 0)
#define	FAVORITE_COLOR_VALUE_LIME_GREEN   GX_RGB(21,31, 0)
#define	FAVORITE_COLOR_VALUE_GREEN        GX_RGB( 0,31, 0)
#define	FAVORITE_COLOR_VALUE_DARK_GREEN   GX_RGB( 0,20, 7)
#define	FAVORITE_COLOR_VALUE_SEA_GREEN    GX_RGB( 9,27,17)
#define	FAVORITE_COLOR_VALUE_TURQUOISE    GX_RGB( 6,23,30)
#define	FAVORITE_COLOR_VALUE_BLUE         GX_RGB( 0,11,30)
#define	FAVORITE_COLOR_VALUE_DARK_BLUE    GX_RGB( 0, 0,18)
#define	FAVORITE_COLOR_VALUE_PURPLE       GX_RGB(17, 0,26)
#define	FAVORITE_COLOR_VALUE_VIOLET       GX_RGB(26, 0,29)
#define	FAVORITE_COLOR_VALUE_MAGENTA      GX_RGB(31, 0,18)


// extern data------------------------------------------

// function's prototype declaration---------------------
static void DrawNCD_ElementName( void );
static void DrawNCD_Element( void );
static void DispUserNameAndComment( u16 color );

// global variable -------------------------------------

// static variable -------------------------------------

// const data  -----------------------------------------
static const u8 *str_lcd[]		= { (const u8 *)"TOP",
									(const u8 *)"BOTTOM" };
static const u8 *str_onoff[]	= { (const u8 *)"OFF",
									(const u8 *)"ON" };
static const u8 *str_inputFlags[] = {
									(const u8 *)"NAME",
									(const u8 *)"RTC ",
									(const u8 *)"LANG",
									(const u8 *)"TP  ",
									(const u8 *)"COL ",
									(const u8 *)"BDAY", };
static const u8 *str_language[] = { (const u8 *)"JAPANESE",
									(const u8 *)"ENGLISH",
									(const u8 *)"FRENCH",
									(const u8 *)"GERMAN",
									(const u8 *)"ITALIAN",
									(const u8 *)"SPANISH",
									(const u8 *)"CHINISE",
									(const u8 *)"HANGUL",
									(const u8 *)"UNKNOWN",
};

static const u8 *str_userColor[] = {
	(const u8 *)"GRAY",
	(const u8 *)"BROWN",
	(const u8 *)"RED",
	(const u8 *)"PINK",
	(const u8 *)"ORANGE",
	(const u8 *)"YELLOW",
	(const u8 *)"LIME GREEN ",
	(const u8 *)"GREEN",
	(const u8 *)"DARK GREEN",
	(const u8 *)"SEA GREEN",
	(const u8 *)"TURQUOISE",
	(const u8 *)"BLUE",
	(const u8 *)"DARK BLUE",
	(const u8 *)"PURPLE",
	(const u8 *)"VIOLET",
	(const u8 *)"MAGENTA",
};

static const u16 palette_userColor[] = {
	FAVORITE_COLOR_VALUE_GRAY,
	FAVORITE_COLOR_VALUE_BROWN,
	FAVORITE_COLOR_VALUE_RED,
	FAVORITE_COLOR_VALUE_PINK,
	FAVORITE_COLOR_VALUE_ORANGE,
	FAVORITE_COLOR_VALUE_YELLOW,
	FAVORITE_COLOR_VALUE_LIME_GREEN,
	FAVORITE_COLOR_VALUE_GREEN,
	FAVORITE_COLOR_VALUE_DARK_GREEN,
	FAVORITE_COLOR_VALUE_SEA_GREEN,
	FAVORITE_COLOR_VALUE_TURQUOISE,
	FAVORITE_COLOR_VALUE_BLUE,
	FAVORITE_COLOR_VALUE_DARK_BLUE,
	FAVORITE_COLOR_VALUE_PURPLE,
	FAVORITE_COLOR_VALUE_VIOLET,
	FAVORITE_COLOR_VALUE_MAGENTA,
};

static const u16 str_sikaku[] = L"■■■■";

//======================================================
// NITRO設定データ表示
//======================================================

// NITRO設定データ表示の初期化
void SEQ_DispNCD_init( void )
{
	GXS_SetVisiblePlane( GX_PLANEMASK_NONE );
	GX_SetVisiblePlane ( GX_PLANEMASK_NONE );
	
	SVC_CpuClearFast( 0x0000, bgBakM, sizeof(bgBakM) );
	SVC_CpuClearFast( 0x0000, bgBakS, sizeof(bgBakS) );
	
	ClearAllStringSJIS();
	
//	(void)DrawStringSJIS( 1,  0, YELLOW, (const u8 *)"Disp OwnerInfo [Use SDK 2.00 W]" );
//	(void)DrawStringSJIS( 1,  0, YELLOW, (const u8 *)"Disp OwnerInfo [Use SDK 2.01 WC]" );
	(void)DrawStringSJIS( 1,  0, YELLOW, (const u8 *)"Disp OwnerInfo [Use SDK 5.01 PR1 WCK]" );
	
	DrawNCD_ElementName();
	DrawNCD_Element();
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2  | GX_PLANEMASK_OBJ );
}


// NITRO設定データの表示
int SEQ_DispNCD( void )
{
	BOOL tp_cancel		= FALSE;
	
	return 0;
}


// NITRO設定データの要素名表示
static void DrawNCD_ElementName( void )
{
	{	// TOP LCD
		SetTargetScreenSJIS( BOTTOM_SCREEN );
		(void)DrawStringSJIS( 1,  4, CYAN      , (const u8 *)"[OPTION]" );
		(void)DrawStringSJIS( 2,  6, LIGHTGREEN, (const u8 *)"Language  :" );
		(void)DrawStringSJIS( 1,  8, CYAN      , (const u8 *)"[OWNER]" );
		(void)DrawStringSJIS( 2, 10, LIGHTGREEN, (const u8 *)"fav color:" );
		(void)DrawStringSJIS( 2, 12, LIGHTGREEN, (const u8 *)"birthday :" );
		(void)DrawStringSJIS( 2, 14, LIGHTGREEN, (const u8 *)"nickname :" );
		(void)DrawStringSJIS( 2, 16, LIGHTGREEN, (const u8 *)"comment  :" );
//		SetTargetScreenSJIS( BOTTOM_SCREEN );
	}
}


// NITRO設定データの各要素表示
static void DrawNCD_Element( void )
{
	const u8 *str;
	OSOwnerInfo  info;
	OS_GetOwnerInfo( &info );
	
	{	// TOP LCD
		SetTargetScreenSJIS( BOTTOM_SCREEN );
		if( info.language < LANG_CODE_MAX ) {
			str = str_language[ info.language ];
		}else {
			str = str_language[ LANG_CODE_MAX ];
		}
		(void)DrawStringSJISEx( 12,  6, WHITE, str, 0 );
		
//		SetTargetScreenSJIS( BOTTOM_SCREEN );
	}
	{
		int temp;
		
		// ユーザーカラー
		{
			tFntPosition x = (tFntPosition){ 12 * 8    , FNT_POSX_ORIGIN_LEFT, FNT_H_ALIGN_LEFT };
			tFntPosition y = (tFntPosition){ 11 * 8 + 4, FNT_POSY_ORIGIN_BOTTOM , 0 /*unused*/   };
			temp = info.favoriteColor;
			fnt_DrawString( &font_s, &context_s, x, y, 1, 0, str_sikaku, FNT_USER_COLOR );
			(void)DrawDecimalSJIS( 18, 10, WHITE, &temp, 2, 4 );
			(void)DrawStringSJIS ( 20, 10, WHITE, str_userColor[ temp ] );
			GXS_LoadBGPltt( &palette_userColor[ temp ], FNT_USER_COLOR * 2, 2 );
		}
		(void)DrawStringSJIS ( 14, 12, WHITE, (const u8 *)"/" );
		(void)DrawDecimalSJIS( 12, 12, WHITE, &info.birthday.month, 2, 1 );
		(void)DrawDecimalSJIS( 15, 12, WHITE, &info.birthday.day,   2, 1 );
		
		// ユーザーネーム、コメント
		DispUserNameAndComment( FNT_WHITE );
		
		// ユーザーネーム長、コメント長
		(void)DrawDecimalSJIS  (  0, 14, YELLOW, &info.nickNameLength, 2, 1 );
		(void)DrawDecimalSJIS  (  0, 16, YELLOW, &info.commentLength,  2, 1 );
	}
}


static void DispUserNameAndComment( u16 color )
{
	u16 color2 = ( color == FNT_BLACK ) ? (u16)FNT_BLACK : (u16)FNT_YELLOW;
	u16 buff[ 256 ];
	tFntPosition x = (tFntPosition){ 12 * 8    , FNT_POSX_ORIGIN_LEFT, FNT_H_ALIGN_LEFT };
	tFntPosition y = (tFntPosition){ 15 * 8 + 4, FNT_POSY_ORIGIN_BOTTOM , 0 /*unused*/   };
	OSOwnerInfo  info;
	OS_GetOwnerInfo( &info );
	
	// ユーザーネームの表示
	MI_CpuCopy16 ( info.nickName, buff, NCD_NICKNAME_LENGTH * 2 );
	buff[ NCD_NICKNAME_LENGTH ] = 0;
	fnt_DrawString( &font_s, &context_s, x, y, 1, 0, buff, color2 );
	buff[ info.nickNameLength ] = 0;
	fnt_DrawString( &font_s, &context_s, x, y, 1, 0, buff, color );
	
	
	// コメントの表示
	y.pos = 19 * 8 + 4;
	MI_CpuCopy16( info.comment, buff, (u32)( info.commentLength * 2 ) );
	buff[ info.commentLength ] = 0;
	if ( info.commentLength > 13 ) {	// 2行にまたがる時。
		fnt_DrawString( &font_s, &context_s, x, y, 1, 0, &buff[13], color );
	}
	buff[ 13 ] = 0;
	y.pos -= 2 * 8;
	fnt_DrawString( &font_s, &context_s, x, y, 1, 0, buff, color );
}


