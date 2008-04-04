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

// Define data --------------------------------------------------------------
#define NDMA_WORD_COUNT_1					0x1
#define ASSERT_DMANO( ndmaNo )				SDK_ASSERTMSG( (ndmaNo) <= MI_NDMA_MAX_NUM, "illegal NDMA No." );


extern CardThreadData HotSwThreadData;

// Function prototype -------------------------------------------------------
static void HOTSWi_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size, u32 dcont);
static void InterruptCallbackNDma(void);

// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         HOTSW_NDmaCopy_Card
  
  Description:  �J�[�h���瑗���Ă����f�[�^���w��A�h���X��DMA�]������
 *---------------------------------------------------------------------------*/
void HOTSW_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size)
{
	HOTSWi_NDmaCopy_Card(ndmaNo, src, dest, size, MI_NDMA_DEST_INC);
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_NDmaPipe_Card
  
  Description:  �J�[�h���瑗���Ă����f�[�^���w��A�h���X��DMA�]���œǂݎ̂Ă�
 *---------------------------------------------------------------------------*/
void HOTSW_NDmaPipe_Card(u32 ndmaNo, const void *src, void *dest, u32 size)
{
	HOTSWi_NDmaCopy_Card(ndmaNo, src, dest, size, MI_NDMA_DEST_FIX);
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_NDmaCopy_Card
  
  Description:  DMA�]���̏���

  ���F��ɂ��̊֐���DMA�]���̏��������Ă���A�J�[�h���W�X�^��start�t���O���グ�Ă�������
 *---------------------------------------------------------------------------*/
static void HOTSWi_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size, u32 dcont)
{
	u32 contData;
	OSIntrMode enabled = OS_DisableInterrupts();

    //--- Assert
	ASSERT_DMANO( ndmaNo );
    
	//---- confirm CARD free
	HOTSW_WaitCardCtrl();

	//---- confirm DMA free
	HOTSW_WaitDmaCtrl(ndmaNo);

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


/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackNDma

  Description:  �J�[�hB �f�[�^�]���I�����荞�݃n���h��
 *---------------------------------------------------------------------------*/
static void InterruptCallbackNDma(void)
{
	// ���b�Z�[�W���M
    OS_SendMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&HotSwThreadData.hotswDmaMsg[HotSwThreadData.idx_dma], OS_MESSAGE_NOBLOCK);

	// ���b�Z�[�W�C���f�b�N�X���C���N�������g
    HotSwThreadData.idx_dma = (HotSwThreadData.idx_dma+1) % HOTSW_DMA_MSG_NUM;
}
