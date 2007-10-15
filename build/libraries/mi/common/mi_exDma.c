/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - mi
  File:     mi_init_mainMemory.c

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

#include <firm/mi.h>

static u16 intervalTable[] =
{
    1, 1, 1, 1,
};

static u32 prescaleTable[] =
{
    MI_NDMA_INTERVAL_PS_1,
    MI_NDMA_INTERVAL_PS_1,
    MI_NDMA_INTERVAL_PS_1,
    MI_NDMA_INTERVAL_PS_1,
};

//================================================================================
//            memory oparation using DMA (sync)
//================================================================================

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaSendAsync

  Description:  send data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaSendAsync( u32 dmaNo, const void *src, void *dest, u32 size )
{
    u32 idx = dmaNo - MI_EXDMA_CH_MIN;

    {
        u32 blockSize = MI_NDMA_BWORD_8;
        u32 interval = intervalTable[idx];
        u32 prescale = prescaleTable[idx];

        MIi_ExDmaSendAsyncCore( dmaNo, src, dest, size, size,
                blockSize, interval, prescale,
                MI_NDMA_CONTINUOUS_OFF, MI_NDMA_SRC_RELOAD_DISABLE, MI_NDMA_DEST_RELOAD_DISABLE,
                MI_NDMA_IMM_MODE_ON );
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaRecvAsync

  Description:  receive data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaRecvAsync( u32 dmaNo, const void *src, void *dest, u32 size )
{
    u32 idx = dmaNo - MI_EXDMA_CH_MIN;

    {
        u32 blockSize = MI_NDMA_BWORD_8;
        u32 interval = intervalTable[idx];
        u32 prescale = prescaleTable[idx];

        MIi_ExDmaRecvAsyncCore( dmaNo, src, dest, size, size,
                blockSize, interval, prescale,
                MI_NDMA_CONTINUOUS_OFF, MI_NDMA_SRC_RELOAD_DISABLE, MI_NDMA_DEST_RELOAD_DISABLE,
                MI_NDMA_IMM_MODE_ON );
    }
}

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
                u32 timing )
{
    MIi_WaitExDma( dmaNo );

    MIi_SetExDmaParams( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing,
                0, MI_NDMA_SRC_INC, MI_NDMA_DEST_FIX );
}

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
                u32 timing )
{
    MIi_WaitExDma( dmaNo );

    MIi_SetExDmaParams( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing,
                0, MI_NDMA_SRC_FIX, MI_NDMA_DEST_INC );
}

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
                u32 fillData, u32 srcDir, u32 destDir )
{
    u32 idx = dmaNo - MI_EXDMA_CH_MIN;

    if ( idx < MI_EXDMA_CH_NUM )
    {
        OSIntrMode enabled = OS_DisableInterrupts();

        MIExDmaChanRegs *reg = &((MIExDmaChanRegs*)REG_NDMA0SAD_ADDR)[idx];

        reg->src = src;
        reg->dest = dest;
        reg->fillData = fillData;
        reg->totalCount = size / 4;
        reg->wordCount = oneShotSize / 4;
        reg->blockInterval = (interval << REG_MI_NDMA0BCNT_ICNT_SHIFT) | prescale;
        reg->ctrl = blockSize
                  | srcDir | destDir
                  | srcRld | destRld
                  | continuous
                  | timing
                  | MI_NDMA_ENABLE | MI_NDMA_IF_ENABLE;

        (void)OS_RestoreInterrupts(enabled);
    }
}

//================================================================================
//       DMA WAIT/STOP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_IsExDmaBusy

  Description:  check whether extended DMA is busy or not

  Arguments:    dmaNo : DMA channel No.

  Returns:      TRUE if extended DMA is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL MIi_IsExDmaBusy( u32 dmaNo )
{
    u32 idx = dmaNo - MI_EXDMA_CH_MIN;

    if ( idx < MI_EXDMA_CH_NUM )
    {
        MIExDmaChanRegs *reg = &((MIExDmaChanRegs*)REG_NDMA0SAD_ADDR)[idx];

        return (BOOL)((reg->ctrl & REG_MI_NDMA0CNT_E_MASK) >> REG_MI_NDMA0CNT_E_SHIFT);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         MIi_WaitExDma

  Description:  wait while extended DMA is busy

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_WaitExDma( u32 dmaNo )
{
    u32 idx = dmaNo - MI_EXDMA_CH_MIN;

    if ( idx < MI_EXDMA_CH_NUM )
    {
        MIExDmaChanRegs *reg = &((MIExDmaChanRegs*)REG_NDMA0SAD_ADDR)[idx];

        while (reg->ctrl & REG_MI_NDMA0CNT_E_MASK)
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_StopDma

  Description:  stop extended DMA

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_StopExDma( u32 dmaNo )
{
    MIi_StopExDmaAsync( dmaNo );
    MIi_WaitExDma( dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_StopDmaAsync

  Description:  stop extended DMA
                async version

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_StopExDmaAsync( u32 dmaNo )
{
    OSIntrMode enabled = OS_DisableInterrupts();

    u32 idx = dmaNo - MI_EXDMA_CH_MIN;

    if ( idx < MI_EXDMA_CH_NUM )
    {
        MIExDmaChanRegs *reg = &((MIExDmaChanRegs*)REG_NDMA0SAD_ADDR)[idx];

        reg->ctrl &= ~MI_NDMA_ENABLE;
    }

    (void)OS_RestoreInterrupts(enabled);
}

