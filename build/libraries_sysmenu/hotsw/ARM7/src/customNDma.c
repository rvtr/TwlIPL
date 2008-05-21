/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     customNDma.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#include 	<twl.h>
#include	<sysmenu.h>
#include	<hotswTypes.h>
#include	<customNDma.h>

// Extern -------------------------------------------------------------------
extern CardThreadData HotSwThreadData;

// union --------------------------------------------------------------------
#ifndef USE_NEW_DMA
typedef union
{
    u32     b32;
    u16     b16;
}
MIiDmaClearSrc;
#endif

// Define data --------------------------------------------------------------
#ifndef USE_NEW_DMA
#define NDMA_WORD_COUNT_1					0x1
#define ASSERT_NDMANO( ndmaNo )				SDK_ASSERTMSG( (ndmaNo) <= MI_NDMA_MAX_NUM, "illegal NDMA No." );
#define ASSERT_DMANO( dmaNo )				SDK_ASSERTMSG( (dmaNo) <= MI_DMA_MAX_NUM, "illegal DMA No." );

#define MIi_DMA_MODE_NOINT					1
#define MIi_DMA_MODE_WAIT					2
#define MIi_DMA_MODE_NOCLEAR				4
#define MIi_DMA_MODE_SRC32      			0x10
#define MIi_DMA_MODE_SRC16      			0x20

#define MIi_DMA_CLEAR_DATA_BUF     			HW_PRV_WRAM_DMA_CLEAR_DATA_BUF
#endif

// Function prototype -------------------------------------------------------
#ifdef USE_NEW_DMA
static void HOTSWi_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size, u32 dcont);
static void InterruptCallbackNDma(void);
#else
static void HOTSWi_DmaCopy32_Card(u32 dmaNo, const void *src, void *dest, u32 size, u32 dcont);
static void HOTSWi_DmaSetParameters(u32 dmaNo, u32 src, u32 dest, u32 ctrl, u32 mode);
#endif


// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         HOTSW_NDmaCopy_Card
  
  Description:  カードから送られてきたデータを指定アドレスにNDMA転送する
 *---------------------------------------------------------------------------*/
#ifdef USE_NEW_DMA
void HOTSW_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size)
{
	HOTSWi_NDmaCopy_Card(ndmaNo, src, dest, size, MI_NDMA_DEST_INC);
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSW_NDmaPipe_Card
  
  Description:  カードから送られてきたデータを指定アドレスにNDMA転送で読み捨てる
 *---------------------------------------------------------------------------*/
#ifdef USE_NEW_DMA
void HOTSW_NDmaPipe_Card(u32 ndmaNo, const void *src, void *dest, u32 size)
{
	HOTSWi_NDmaCopy_Card(ndmaNo, src, dest, size, MI_NDMA_DEST_FIX);
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_NDmaCopy_Card
  
  Description:  NDMA転送の準備

  注：先にこの関数でDMA転送の準備をしてから、カードレジスタのstartフラグを上げてください
 *---------------------------------------------------------------------------*/
#ifdef USE_NEW_DMA
static void HOTSWi_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size, u32 dcont)
{
	u32 contData;
	OSIntrMode enabled = OS_DisableInterrupts();

    //--- Assert
	ASSERT_NDMANO( ndmaNo );
    
	//---- confirm CARD free
	HOTSW_WaitCardCtrl();

	//---- confirm DMA free
	HOTSW_WaitNDmaCtrl(ndmaNo);

    //---- Handler Set
    (void)OS_SetIrqFunction(OS_IE_NDMA2, InterruptCallbackNDma);
    
	//---- set up registers
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_SAD_WOFFSET )  = (u32)src;
	MI_NDMA_REG( ndmaNo, MI_NDMA_REG_DAD_WOFFSET )  = (u32)dest;
	MI_NDMA_REG( ndmaNo, MI_NDMA_REG_BCNT_WOFFSET ) = MI_NDMA_INTERVAL_PS_1;
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_TCNT_WOFFSET ) = (u32)(size/4);
	MI_NDMA_REG( ndmaNo, MI_NDMA_REG_WCNT_WOFFSET ) = NDMA_WORD_COUNT_1;
    
	//---- decide control register
	contData  = MI_NDMA_BWORD_1 | MI_NDMA_ENABLE/* | MI_NDMA_CONTINUOUS_ON*/;
	contData |= MI_NDMA_SRC_FIX | dcont | MI_NDMA_SRC_RELOAD_DISABLE | MI_NDMA_DEST_RELOAD_DISABLE;
#ifndef DEBUG_USED_CARD_SLOT_B_
	contData |= MI_NDMA_TIMING_CARD_A;
#else
	contData |= MI_NDMA_TIMING_CARD_B;
#endif

	//---- set interrupt enable 
	contData |= MI_NDMA_IF_ENABLE;
    
	//---- start
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) = contData;
    
	(void)OS_RestoreInterrupts( enabled );
}
#endif


/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackNDma

  Description:  カードB データ転送終了割り込みハンドラ
 *---------------------------------------------------------------------------*/
#ifdef USE_NEW_DMA
static void InterruptCallbackNDma(void)
{
	// メッセージ送信
    OS_SendMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&HotSwThreadData.hotswDmaMsg[HotSwThreadData.idx_dma], OS_MESSAGE_NOBLOCK);

	// メッセージインデックスをインクリメント
    HotSwThreadData.idx_dma = (HotSwThreadData.idx_dma+1) % HOTSW_DMA_MSG_NUM;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSW_DmaCopy32_Card
  
  Description:  カードから送られてきたデータを指定アドレスにDMA転送する
 *---------------------------------------------------------------------------*/
#ifndef USE_NEW_DMA
void HOTSW_DmaCopy32_Card(u32 dmaNo, const void *src, void *dest, u32 size)
{
	HOTSWi_DmaCopy32_Card(dmaNo, src, dest, size, MI_DMA_DEST_INC);
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSW_DmaPipe32_Card
  
  Description:  カードから送られてきたデータを指定アドレスにDMA転送で読み捨てる
 *---------------------------------------------------------------------------*/
#ifndef USE_NEW_DMA
void HOTSW_DmaPipe32_Card(u32 dmaNo, const void *src, void *dest, u32 size)
{
	HOTSWi_DmaCopy32_Card(dmaNo, src, dest, size, MI_DMA_DEST_FIX);
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSW_DmaCopy32_Card

  Description:  DMA転送の準備

  注：先にこの関数でDMA転送の準備をしてから、カードレジスタのstartフラグを上げてください
 *---------------------------------------------------------------------------*/
#ifndef USE_NEW_DMA
static void HOTSWi_DmaCopy32_Card(u32 dmaNo, const void *src, void *dest, u32 size, u32 dcont)
{
	u32 contData;

    //--- Assert
	ASSERT_DMANO( dmaNo );
    
    if (size == 0)
    {
        return;
    }

	//---- confirm CARD free
	HOTSW_WaitCardCtrl();

	//---- confirm DMA free
	HOTSW_WaitDmaCtrl(dmaNo);

    //---- decide control register
	contData = ( MI_DMA_ENABLE | MI_DMA_IF_ENABLE | MI_DMA_TIMING_CARD | MI_DMA_SRC_FIX | dcont | MI_DMA_CONTINUOUS_ON | MI_DMA_32BIT_BUS | 1 );
    
    //---- parameter set
    HOTSWi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, contData, 0);
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_DmaSetParameters

  Description:  DMAのパラメータセット
 *---------------------------------------------------------------------------*/
#ifndef USE_NEW_DMA
static void HOTSWi_DmaSetParameters(u32 dmaNo, u32 src, u32 dest, u32 ctrl, u32 mode)
{
	OSIntrMode enabled;
    vu32 *p;

	if ( ! (mode & MIi_DMA_MODE_NOINT) )
	{
		enabled = OS_DisableInterrupts();
	}

    //p = (vu32 *)((u32)REG_DMA0SAD_ADDR + dmaNo * 12);
    p = (vu32*)MI_DMA_REGADDR( dmaNo, MI_DMA_REG_SAD_WOFFSET );

	if ( mode & MIi_DMA_MODE_SRC32 )
	{
		MIiDmaClearSrc *srcp = (MIiDmaClearSrc *) ((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
		srcp->b32 = src;
		src = (u32)srcp;
	}
	else if ( mode & MIi_DMA_MODE_SRC16 )
	{
		MIiDmaClearSrc *srcp = (MIiDmaClearSrc *) ((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
		srcp->b16 = (u16)src;
		src = (u32)srcp;
	}

    *p = (vu32)src;
    *(p + 1) = (vu32)dest;
    *(p + 2) = (vu32)ctrl;

	if ( mode & MIi_DMA_MODE_WAIT )
	{
		// ARM7 must wait 2 cycle (load is 3 cycle)
        u32     dummy = reg_MI_DMA0SAD;
	}

	if ( ! (mode & MIi_DMA_MODE_NOINT) )
	{
		(void)OS_RestoreInterrupts(enabled);
	}
}
#endif