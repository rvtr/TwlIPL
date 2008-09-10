/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - mi
  File:     exDma.h

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

#ifndef TWL_MI_EXDMA_H_
#define TWL_MI_EXDMA_H_

#include <twl/misc.h>
#include <twl/types.h>
#include <twl/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------

//---- registers
typedef struct
{
    const void * src;
    void * dest;
    u32 totalCount;
    u32 wordCount;
    u32 blockInterval;
    u32 fillData;
    u32 ctrl;
}
t_MIExDmaChanRegs;

typedef volatile t_MIExDmaChanRegs MIExDmaChanRegs;


//================================================================================
//                    DMA control definition
//================================================================================
//---- DMA channel No.
#define MI_EXDMA_CH_MIN              4
#define MI_EXDMA_CH_MAX              7

#define MI_EXDMA_CH_NUM              4


//================================================================================
//       DMA WAIT
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_IsExDmaBusy

  Description:  check whether extended DMA is busy or not

  Arguments:    dmaNo : DMA channel No.

  Returns:      TRUE if extended DMA is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL    MIi_IsExDmaBusy( u32 dmaNo );

/*---------------------------------------------------------------------------*
  Name:         MIi_WaitExDma

  Description:  wait while extended DMA is busy

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_WaitExDma( u32 dmaNo );

/*---------------------------------------------------------------------------*
  Name:         MIi_StopExDma

  Description:  stop extended DMA

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_StopExDma( u32 dmaNo );

/*---------------------------------------------------------------------------*
  Name:         MIi_StopDmaAsync

  Description:  stop extended DMA
                async version

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_StopExDmaAsync( u32 dmaNo );

//================================================================================
//            memory operation using DMA
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaSendAsync

  Description:  send data with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaSendAsync( u32 dmaNo, const void *src, void *dest, u32 size );

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaRecvAsync

  Description:  receive data with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaRecvAsync( u32 dmaNo, const void *src, void *dest, u32 size );

//----------------- internel functions -------------------
/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaSendAsyncCore

  Description:  send data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaSendAsyncCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                u32 blockSize, u32 interval, u32 prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                u32 timing );

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaRecvAsyncCore

  Description:  receive data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaRecvAsyncCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                u32 blockSize, u32 interval, u32 prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                u32 timing );

/*---------------------------------------------------------------------------*
  Name:         MIi_SetExDmaParams

  Description:  set DMA

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_SetExDmaParams( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                u32 blockSize, u32 interval, u32 prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                u32 timing,
                u32 fillData, u32 srcDir, u32 destDir);


#ifdef __cplusplus
} /* extern "C" */

#endif

/* TWL_MI_EXDMA_H_ */
#endif
