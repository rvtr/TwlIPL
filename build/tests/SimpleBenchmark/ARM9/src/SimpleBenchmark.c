/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_Chat.c

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
#include "SimpleBenchmark.h"

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

// テストプログラムの初期化
void SimpleBenchmarkInit( void )
{
	OS_InitTick();
	
	HOTSW_Init();
	
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTest");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );

	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// テストプログラムのメインループ
void SimpleBenchmarkMain(void)
{
	BOOL tp_cancel = FALSE;
	OSTick ot;
	
	ReadTP();													// タッチパネル入力の取得
	
	// [RETURN]ボタン押下チェック
	if(tpd.disp.touch) {
		tp_cancel = WithinRangeTP(  RETURN_BUTTON_TOP_X * 8,    RETURN_BUTTON_TOP_Y * 8 - 4,
									RETURN_BUTTON_BOTTOM_X * 8, RETURN_BUTTON_BOTTOM_Y * 8 - 4, &tpd.disp );
	}
	
	
	ot = OS_GetTick();
	PutStringUTF16( 1 * 8, 1 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 2 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 3 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 4 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 5 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 6 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 7 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 8 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 9 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 10 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 11 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 12 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 13 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 14 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 15 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 16 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 17 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 18 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 19 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 20 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 21 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	PutStringUTF16( 1 * 8, 22 * 8, TXT_COLOR_BLUE,  (const u16 *)L"SimpleBenchmarkTestSimpleBenchmarkTest");
	ot = OS_GetTick()-ot;
	
	OS_TPrintf("%d\n", OS_TicksToMicroSeconds(ot));
	
	if( ( pad.trg & PAD_BUTTON_A ) ) {
	}
	
	if( ( pad.trg & PAD_BUTTON_X ) ) {
	}
	
	if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
//		SYSM_RebootLauncher();
	}
	
	GetAndDrawRTCData( &g_rtcDraw, FALSE );
}


