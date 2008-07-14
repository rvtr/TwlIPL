/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInfo
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include "viewSystemInfo.h"

#define CANVAS0_WIDTH 15
#define CANVAS0_HEIGHT 10
#define CANVAS0_LEFT 10
#define CANVAS0_TOP 10

#define CANVAS1_WIDTH 10
#define CANVAS1_HEIGHT 10
#define CANVAS1_LEFT 130
#define CANVAS1_TOP 10

#define TEXT_HSPACE 1	// 文字列描画時の文字間 (ピクセル単位)
#define TEXT_VSPACE 1	// 文字列描画時の行間   (ピクセル単位)
#define CHARACTER_OFFSET	1

/////////////////////////////

void VBlankHandler( void );

/////////////////////////////


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/



void TwlMain( void )
{
	OS_Init();
	OS_TPrintf("Initialize...");
	
	SYSM_Init(Alloc, Free );	
	SYSM_SetArena();
	RTC_Init();

	//---- interrupt setting
	OS_EnableIrq();
	OS_EnableInterrupts();
	OS_SetIrqFunction( OS_IE_V_BLANK, VBlankHandler );
	OS_EnableIrqMask( OS_IE_V_BLANK );	
	GX_VBlankIntr( TRUE );

	InitAllocator();
	InitBG();
	
	OS_TPrintf("Finished\n");

	displayInfoInit();
	
	FS_Init(FS_DMA_NOT_USE);    

	OS_TPrintf("begin mainloop\n");

	// メインループ
	while( 1 )
	{
		OS_WaitIrq(1, OS_IE_V_BLANK); // Vブランク割り込み待ち
		
		ReadKeyPad(); // キー入力の取得
	
		displayInfoMain(); // ビューア更新
	}
    
}


void VBlankHandler(void)
{
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );	// Vブランク割込チェックのセット
}
