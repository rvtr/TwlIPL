/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     pxi.c

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
#include <twl/mcu.h>
#include <sysmenu.h>
#include "internal_api.h"
#ifdef SDK_ARM7
#include <pm_pmic.h>
#endif // SDK_ARM7


// define data-----------------------------------------------------------------
#define	SYSM_PXI_COMM_STAT_REQ		0
#define SYSM_PXI_COMM_STAT_ACK		1

// PXIコマンド
typedef union SYSMPXIPacket {
	struct {
		u16 data;
		u8  cmd;  // PXI_FIFOMESSAGE_BITSZ_DATA = 26
		u8  stat : 2;
	};
	u32 raw;
}SYSMPXIPacket;


// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static volatile BOOL s_sending[SYSM_PXI_COMM_NUM];
// const data------------------------------------------------------------------

// PXI初期化
void SYSM_InitPXI( void )
{
    static BOOL isInitialized;
    int i;

    if (isInitialized)
    {
        return;
    }
    isInitialized = TRUE;

    for (i=0; i<SYSM_PXI_COMM_NUM; i++)
    {
        s_sending[i] = FALSE;
    }

    //---- setting PXI
    PXI_Init();
#ifdef SDK_ARM9
    while (!PXI_IsCallbackReady(SYSMENU_PXI_FIFO_TAG, PXI_PROC_ARM7))
    {
    }
#endif // SDK_ARM9
    PXI_SetFifoRecvCallback(SYSMENU_PXI_FIFO_TAG, SYSMi_PXIFifoRecvCallback);
}

// PXIコマンド送信
BOOL SYSMi_TrySendPXICommand( SYSMPXICommand cmd, u16 data )
{
	SYSMPXIPacket packet;
	
	OSIntrMode saved = OS_DisableInterrupts();
	if( s_sending[cmd] )
	{
		OS_RestoreInterrupts( saved );
		return FALSE;
	}
	s_sending[cmd] = TRUE;
	OS_RestoreInterrupts( saved );
	
	packet.stat = SYSM_PXI_COMM_STAT_REQ;
	packet.cmd  = cmd;
	packet.data = data;
	
	while( PXI_SendWordByFifo( SYSMENU_PXI_FIFO_TAG, packet.raw, FALSE) != PXI_FIFO_SUCCESS )
	{
		SVC_WaitByLoop(1);
	}
	
	return TRUE;
}

BOOL SYSMi_SendPXICommand( SYSMPXICommand cmd, u16 data )
{
	while( ! SYSMi_TrySendPXICommand( cmd, data ) )
	{
		OS_WaitAnyIrq();
	}
	while( s_sending[cmd] )
	{
		OS_WaitAnyIrq();
	}
	
	return TRUE;
}

void SYSMi_PXIFifoRecvCallback( PXIFifoTag tag, u32 data, BOOL err )
{
#pragma unused( tag, err )
	SYSMPXIPacket packet;
	u8 cmd;
	packet.raw = data;
	cmd = packet.cmd;

	if( packet.stat == SYSM_PXI_COMM_STAT_ACK )
	{
		s_sending[cmd]  = FALSE;
	}
	if( packet.stat == SYSM_PXI_COMM_STAT_REQ )
	{
		packet.stat = SYSM_PXI_COMM_STAT_ACK;
	}

#ifdef SDK_ARM7

	switch( cmd )
	{
		case SYSM_PXI_COMM_BL_BRIGHT:
#ifdef PMIC_FINAL
			MCU_WriteRegister(MCU_REG_BL_ADDR, (u8)packet.data );
#else // PMIC_FINAL
			PMi_SetRegister( REG_PMIC_BL_BRT_B_ADDR, (u8)packet.data );
#endif // PMIC_FINAL
			break;
		default:
#ifndef SDK_FINALROM
            OS_Panic("illegal SYSM pxi command.");
#else
            OS_Panic("");
#endif
			break;
	}

	// PXI応答返信
	PXI_SendWordByFifo( SYSMENU_PXI_FIFO_TAG, packet.raw, FALSE );

#endif // SDK_ARM7

#if 0
	
#ifdef SDK_ARM9
	
	if( packet.stat == SYSM_PXI_COMM_STAT_ACK ) {
		switch( data ) {
		case SYSM_PXI_COMM_DISABLE_HOTSW:
			SYSMi_GetWork()->flags.arm9.isEnableHotSW = 0;
			break;
		default:
			break;
		}
	}else {	// SYSM_PXI_COMM_STAT_REQ
		switch( data ) {
		default:
			break;
		}
	}
#else  // !SDK_ARM9
	
	if( packet.stat == SYSM_PXI_COMM_STAT_REQ ) {
		switch( data ) {
		case SYSM_PXI_COMM_DISABLE_HOTSW:
			if( SYSMi_GetWork()->flags.arm7.isBusyHotSW ) {
				SYSMi_GetWork()->flags.arm7.disableHotSW_REQ = 1;
			}else {
				SYSMi_GetWork()->flags.arm7.isEnableHotSW = 0;
				// PXIで返信
			}
			break;
		default:
			break;
		}
	}else {	// SYSM_PXI_COMM_STAT_ACK
		switch( data ) {
		default:
			break;
		}
	}
#endif // SDK_ARM9
#endif
}

