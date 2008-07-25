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
#include <twl/sea.h>
#include <nitro/crypto.h>
#include "misc.h"
#include "viewSystemInfo.h"
#include "address.h"

#define CANVAS0_WIDTH 15
#define CANVAS0_HEIGHT 10
#define CANVAS0_LEFT 10
#define CANVAS0_TOP 10

#define CANVAS1_WIDTH 10
#define CANVAS1_HEIGHT 10
#define CANVAS1_LEFT 130
#define CANVAS1_TOP 10

#define TEXT_HSPACE 1	// ������`�掞�̕����� (�s�N�Z���P��)
#define TEXT_VSPACE 1	// ������`�掞�̍s��   (�s�N�Z���P��)
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
	OS_InitTick();
	OS_TPrintf("Initialize begin\n");

	// ARM7���Ń��������ɃZ�b�g���ꂽSCFG�̃f�[�^���m��
	MI_CpuMove16( DISPINFO_SHARED_SCFG_REG_ADDR, gArm7SCFGReg, DISPINFO_SHARED_SCFG_REG_SIZE );
	// WRAM�ɑޔ�����Ă镪���R�s�[
	MI_CpuMove16( DISPINFO_SHARED_SCFG_WRAM_ADDR, gArm7SCFGShared,  DISPINFO_SHARED_SCFG_WRAM_SIZE );

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
	CRYPTO_SetAllocator( Alloc, Free );

	OS_TPrintf("NAM Initialize...\n");
	NAM_Init( Alloc, Free );
		
	InitBG();
	OS_TPrintf("Initialize Finished\n");
	displayInfoInit();

	OS_TPrintf("begin mainloop\n");
	
	
	// ���C�����[�v
	while( 1 )
	{
		OS_WaitIrq(1, OS_IE_V_BLANK); // V�u�����N���荞�ݑ҂�
		
		ReadKeyPad(); // �L�[���͂̎擾
	
		displayInfoMain(); // �r���[�A�X�V
	}
    
    OS_Terminate();
    
}


void VBlankHandler(void)
{
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );	// V�u�����N�����`�F�b�N�̃Z�b�g
}
