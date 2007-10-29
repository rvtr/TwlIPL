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
#include "DS_Chat.h"
#include "DS_Setting.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

// extern data------------------------------------------

// function's prototype declaration---------------------

static void DS_ChatInit(void);
static int  DS_Chat(void);

// global variable -------------------------------------

// static variable -------------------------------------

// const data  -----------------------------------------

// ピクトチャットのメインループ
int DS_ChatMain(void)
{
	DS_ChatInit();
	
	while(1) {
		OS_WaitIrq( 1, OS_IE_V_BLANK );
		ReadKeyPad();
		
		if( DS_Chat() ) {
			return 0;
		}
		
		if ( PAD_DetectFold() == TRUE ) {								// スリープモードへの遷移
			SYSM_GoSleepMode();
		}
	}
	return 0;
}


//======================================================
// ピクトチャット
//======================================================
// ピクトチャットの初期化
static void DS_ChatInit( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	switch(csrMenu) {
	  case 0:
		PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE, (const u16 *)L"PictoChat");
		break;
	}
	
	PutStringUTF16( 4 * 8, 8 * 8, TXT_COLOR_BLACK, (const u16 *)L"Under Construction...");
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}


// ピクトチャット（空処理）
static int DS_Chat(void)
{
	BOOL tp_cancel = FALSE;
	
	ReadTpData();													// タッチパネル入力の取得
	
	// [RETURN]ボタン押下チェック
	if(tpd.disp.touch) {
		tp_cancel = InRangeTp( RETURN_BUTTON_TOP_X * 8,    RETURN_BUTTON_TOP_Y * 8 - 4,
							   RETURN_BUTTON_BOTTOM_X * 8, RETURN_BUTTON_BOTTOM_Y * 8 - 4, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		return 1;
	}
	return 0;
}

