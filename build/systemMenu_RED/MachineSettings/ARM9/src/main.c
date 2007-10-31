/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.c

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
#include "misc.h"
#include "MachineSetting.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static void InitAllocator( NNSFndAllocator* pAllocator );
static void InitAllocSystem( void );
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------
NNSFndAllocator g_allocator;
int (*g_pNowProcess)( void );
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------


// ============================================================================
// function's description
// ============================================================================
void TwlMain(void)
{
	
	// ������----------------------------------
    OS_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
    GX_Init();
	GX_SetPower(GX_POWER_ALL);										// �e���W�b�N �p���[ON
	
	// ���荞�݋���----------------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// �f�o�C�X������-------------------------------
	TP_Init();
	(void)RTC_Init();
	
	// �V�X�e���̏�����------------------
	InitAllocator( &g_allocator );
	CMN_InitFileSystem( &g_allocator );
	
	// NitroConfigData�̃��[�h
	(void)NVRAMm_ReadNitroConfigData( GetNCDWork() );
	SYSM_CaribrateTP();
	
	InitBG();
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	MachineSettingInit();
	// ���C�����[�v----------------------------
	while ( 1 ) {
		OS_WaitIrq( 1, OS_IE_V_BLANK );								// V�u�����N���荞�ݑ҂�
		
		ReadKeyPad();												// �L�[���͂̎擾
		
		(void)g_pNowProcess();
		
		GetAndDrawRTCData( &g_rtcDraw, FALSE );
	}
}


// �A���P�[�^�̏�����
static void InitAllocator( NNSFndAllocator* pAllocator )
{
    u32   arenaLow      = MATH_ROUNDUP  ( (u32)OS_GetMainArenaLo(), 16 );
    u32   arenaHigh     = MATH_ROUNDDOWN( (u32)OS_GetMainArenaHi(), 16 );
    u32   heapSize      = arenaHigh - arenaLow;
    void* heapMemory    = OS_AllocFromMainArenaLo( heapSize, 16 );
    NNSFndHeapHandle    heapHandle;
    SDK_NULL_ASSERT( pAllocator );

    heapHandle = NNS_FndCreateExpHeap( heapMemory, heapSize );
    SDK_ASSERT( heapHandle != NNS_FND_HEAP_INVALID_HANDLE );

    NNS_FndInitAllocatorForExpHeap( pAllocator, heapHandle, 4 );
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// V�u�����N�����`�F�b�N�̃Z�b�g
}

