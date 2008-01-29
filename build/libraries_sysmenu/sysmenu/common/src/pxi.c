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
#include <sysmenu.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
#define	SYSM_PXI_COMM_STAT_REQ		0
#define SYSM_PXI_COMM_STAT_ACK		1

// PXIコマンド
typedef union SYSMPXIPacket {
	struct {
		u16 data;
		u16 command : 14;
		u16 stat : 2;
	};
	u32 raw;
}SYSMPXIPacket;


// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static volatile BOOL s_sending = FALSE;
// const data------------------------------------------------------------------

// PXIコマンド送信
BOOL SYSMi_SendPXICommand( SYSMPXICommand command )
{
	SYSMPXIPacket packet;
	
	OSIntrMode saved = OS_DisableInterrupts();
	if( s_sending ) {
		OS_RestoreInterrupts( saved );
		return FALSE;
	}
	s_sending      = TRUE;
	OS_RestoreInterrupts( saved );
	
	packet.stat    = SYSM_PXI_COMM_STAT_REQ;
	packet.command = (u16)command;
	packet.data    = 0;
	
	while( PXI_SendWordByFifo( SYSMENU_PXI_FIFO_TAG, packet.raw, 0 ) < 0 ) {}
	
	while( s_sending ) {
		OS_WaitAnyIrq();
	}
	return TRUE;
}

void SYSMi_PXIFifoRecvCallback( PXIFifoTag tag, u32 data, BOOL err )
{
#pragma unused( tag, err )
	SYSMPXIPacket packet;
	packet.raw = data;
	
#ifdef SDK_ARM9
	
	s_sending  = FALSE;
	
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
}

