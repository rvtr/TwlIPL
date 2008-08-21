/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     main.c

  Copyright **** Nintendo.  All rights reserved.

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
#include <twl/nam.h>
#include <nitro/nvram/nvram.h>
#include <nitro/crypto.h>


#include "misc.h"
#include "fatalErrorMaker.h"

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
	OS_InitTick();
	OS_TPrintf("Initialize begin\n");

	//---- interrupt setting
	OS_EnableIrq();
	OS_EnableInterrupts();

	GX_Init();

	OS_SetIrqFunction( OS_IE_V_BLANK, VBlankHandler );
	OS_EnableIrqMask( OS_IE_V_BLANK );	
	GX_VBlankIntr( TRUE );

	OS_TPrintf("RTC Initilize...\n");
	RTC_Init();

	OS_TPrintf("Allocator Initialize...\n");
	InitAllocator();

		
	InitBG();
	OS_TPrintf("Initialize Finished\n");

	NVRAMi_Init();
	FS_Init( FS_DMA_NOT_USE );
	CRYPTO_SetAllocator( Alloc, Free );
	NAM_Init( Alloc, Free );
	

	fatalMakerInit();
	OS_TPrintf("begin mainloop\n");
	
	
	// メインループ
	while( 1 )
	{
		OS_WaitIrq(1, OS_IE_V_BLANK); // Vブランク割り込み待ち
		
		ReadKeyPad(); // キー入力の取得

		fatalMakerMain(); // ビューア更新
	}
    
    OS_Terminate();
    
}


void VBlankHandler(void)
{
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );	// Vブランク割込チェックのセット
}
