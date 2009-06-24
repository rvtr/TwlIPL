/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     pxi.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: #$
  $Rev: $
  $Author: $
 *---------------------------------------------------------------------------*/

#ifndef SYSM_COMMON_PXI_H_
#define SYSM_COMMON_PXI_H_

#include <nitro/pxi.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
//　PXIコマンド
//----------------------------------------------------------------------
                                        //  (PXI_MAX_FIFO_TAG - 1) is used by SEA
#define PXI_FIFO_TAG_MCUTEST                (PXI_MAX_FIFO_TAG - 2)
#define PXI_FIFO_TAG_HOTSW                  (PXI_MAX_FIFO_TAG - 3)
#define PXI_FIFO_TAG_DECRYPTAES             (PXI_MAX_FIFO_TAG - 4)
#define SYSMENU_PXI_FIFO_TAG                (PXI_MAX_FIFO_TAG - 5)

typedef enum SYSMPXICommand {
	SYSM_PXI_COMM_DUMMY = 0,
#ifdef DHT_TEST
    SYSM_PXI_COMM_DS_HASH_TABLE,
#endif

    SYSM_PXI_COMM_NUM
}SYSMPXICommand;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

// PXI初期化
#ifdef SDK_ARM9
void SYSM_InitPXI( void );
#else // SDK_ARM7
void SYSM_InitPXI( u32 mcu_prio );
#endif // SDK_ARM7

#ifdef DHT_TEST
#ifdef SDK_ARM9
void SYSMi_PrepareDatabase(void);
#endif
#endif

// PXIコマンド送信
BOOL SYSMi_TrySendPXICommand( SYSMPXICommand cmd, u16 data );
BOOL SYSMi_SendPXICommand( SYSMPXICommand command, u16 data );

// PXIコマンド受信
void SYSMi_PXIFifoRecvCallback( PXIFifoTag tag, u32 data, BOOL err );

#ifdef __cplusplus
}
#endif

#endif  // SYSM_COMMON_PXI_H_
