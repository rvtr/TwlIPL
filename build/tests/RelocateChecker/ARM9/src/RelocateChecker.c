/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     RelocateChecker.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-31#$
  $Rev: 91 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "misc.h"
#include "RelocateChecker.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

// extern data------------------------------------------

// function's prototype declaration---------------------

// global variable -------------------------------------
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable -------------------------------------

// const data  -----------------------------------------

//======================================================
// 再配置チェッカー（多分ここからやる事は表示のみ）
//======================================================

// 初期化
void RelocateCheckerInit( void )
{
	u32 *test;
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	//PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"RelocateChecker");
	//PutStringUTF16( 4 * 8, 8 * 8, TXT_COLOR_BLACK, (const u16 *)L"Under Construction...");
	
	test = (u32 *)0x02000000;
	PrintfSJIS(0,0,TXT_COLOR_BLUE,  "%8x %8x %8x",*(test+5),*(test+6),*(test+7));
	PrintfSJIS(0,2*8,TXT_COLOR_BLUE,  "%8x %8x:arm9",*(test+8),*(test+9));
	
	PrintfSJIS(0,4*8,TXT_COLOR_BLUE,  "%8x %8x %8x",*(test+0),*(test+1),*(test+2));
	PrintfSJIS(0,6*8,TXT_COLOR_BLUE,  "%8x %8x:arm7",*(test+3),*(test+4));
	
	PrintfSJIS(0,8*8,TXT_COLOR_BLUE,  "%8x %8x %8x",*(test+10),*(test+11),*(test+12));
	PrintfSJIS(0,10*8,TXT_COLOR_BLUE,  "%8x %8x:arm9ltd",*(test+13),*(test+14));
	
	PrintfSJIS(0,12*8,TXT_COLOR_BLUE,  "%8x %8x %8x",*(test+15),*(test+16),*(test+17));
	PrintfSJIS(0,14*8,TXT_COLOR_BLUE,  "%8x %8x:arm7ltd",*(test+18),*(test+19));
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// メインループ
void RelocateCheckerMain(void)
{
	BOOL tp_cancel = FALSE;
	
	ReadTP();													// タッチパネル入力の取得
	
	// [RETURN]ボタン押下チェック
	if(tpd.disp.touch) {
		tp_cancel = WithinRangeTP(  RETURN_BUTTON_TOP_X * 8,    RETURN_BUTTON_TOP_Y * 8 - 4,
									RETURN_BUTTON_BOTTOM_X * 8, RETURN_BUTTON_BOTTOM_Y * 8 - 4, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
//		SYSM_RebootLauncher();
	}
	
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
}


