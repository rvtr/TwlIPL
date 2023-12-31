/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     HashCheckTemplate.c

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

#include <twl.h>
#include <sysmenu.h>
#include "misc.h"
#include "HashCheckTemplate.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

#define UNDEF_CODE			0xe7ffdeff

// extern data------------------------------------------

// function's prototype declaration---------------------

// global variable -------------------------------------
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable -------------------------------------
static BOOL s_switch = TRUE;

//static u32 dummy[1024*1024/2]={1,2,3,};//2MB

// const data  -----------------------------------------

//======================================================
// 再配置チェッカー（多分ここからやる事は表示のみ）
//======================================================

/*
static void draw_sub1(u8 *ram_addr, u8 *header_addr, int y)
{
	int l;
	PrintfSJIS(8,y,TXT_UCOLOR_DARKGREEN,  "ARM9FLX ( VERIFY %s ) :",( (*(u32 *)0x02000180 == UNDEF_CODE) ? "OK" : "NG" ));

	for (l=0; l<20; l++)
	{
		PrintfSJIS(24+(l%10)*19, y+12+12*(l/10), TXT_COLOR_BLACK, "%.2x", *(ram_addr+l));
		if(s_switch)
		{
			PrintfSJIS(24+(l%10)*19, y+12+12*(l/10), (*(ram_addr+l) != *(header_addr+l)) ? TXT_COLOR_RED : TXT_COLOR_BLUE, "%.2x", *(header_addr+l));
		}
	}
}
*/

static void draw_sub2(u8 *ram_addr, u8 *header_addr, int y, const u16 *str)
{
	int l;
	PutStringUTF16(8,y,TXT_UCOLOR_DARKGREEN, str);
	for (l=0; l<20; l++)
	{
		PrintfSJIS(24+(l%10)*19, y+12+12*(l/10), TXT_COLOR_BLACK, "%.2x", *(ram_addr+l));
		if(s_switch )
		{
			PrintfSJIS(24+(l%10)*19, y+12+12*(l/10), (*(ram_addr+l) != *(header_addr+l)) ? TXT_COLOR_RED : TXT_COLOR_BLUE, "%.2x", *(header_addr+l));
		}
	}
}

static void encryObjdraw(void)
{
	int l;
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"decrypted encryObj (first 200 bytes)");
	for (l=0; l<50; l++)
	{
		PrintfSJIS(4+(l%4)*64, 24+12*(l/4), TXT_COLOR_BLACK, "%.8x", *(((u32 *)0x02000180)+l) );
	}
}

static void draw()
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );

	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SelfDigestChecker");
	PutStringUTF16( 3 * 8, 1 * 12, TXT_COLOR_BLACK,  (const u16 *)L"Press A to Check Digest....");
	
	draw_sub2((u8 *)0x02000100, (u8 *)(HW_TWL_ROM_HEADER_BUF + 0x38c), 12*2+4, (const u16 *)L"ARM9FLX :" );
	draw_sub2((u8 *)0x02000120, (u8 *)(HW_TWL_ROM_HEADER_BUF + 0x314), 12*5+8, (const u16 *)L"ARM7FLX :" );
	draw_sub2((u8 *)0x02000140, (u8 *)(HW_TWL_ROM_HEADER_BUF + 0x350), 12*8+12, (const u16 *)L"ARM9LTD :" );
	draw_sub2((u8 *)0x02000160, (u8 *)(HW_TWL_ROM_HEADER_BUF + 0x364), 12*11+16, (const u16 *)L"ARM7LTD :" );
}

// 初期化
void HashCheckTemplateInit( void )
{
	GX_DispOff();
 	GXS_DispOff();

	draw();

	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

static u32 c;
// メインループ
void HashCheckTemplateMain(void)
{
	BOOL tp_cancel = FALSE;
	
	ReadTP();													// タッチパネル入力の取得
	
	// [RETURN]ボタン押下チェック
	if(tpd.disp.touch) {
		tp_cancel = WithinRangeTP(  RETURN_BUTTON_TOP_X * 8,    RETURN_BUTTON_TOP_Y * 8 - 4,
									RETURN_BUTTON_BOTTOM_X * 8, RETURN_BUTTON_BOTTOM_Y * 8 - 4, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A )) {
		s_switch = !s_switch;
		draw();
	}
	
	if( ( pad.trg & PAD_BUTTON_X )) {
		encryObjdraw();
	}
	
	if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
//		SYSM_RebootLauncher();
	}
	
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
}


