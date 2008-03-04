/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/
#include 	<twl.h>
#include	<sysmenu.h>
#include	<hotswTypes.h>
#include	<customNDma.h>


// Define data --------------------------------------------------------------
#define	NDMA_BLOCK_WORD_COUNT_1				0x0
#define	NDMA_NO_INTERVAL_NORMAL_SCALE		0x0
#define	NDMA_TOTAL_WORD_COUNT_1				0x1
#define NDMA_WORD_COUNT_1					0x1
#define ASSERT_DMANO( ndmaNo )				SDK_ASSERTMSG( (ndmaNo) <= MI_NDMA_MAX_NUM, "illegal NDMA No." );

// Function prototype -------------------------------------------------------
static void HOTSWi_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size, u32 dcont);


// ===========================================================================
// 	Function Describe
// ===========================================================================

//  custom CARD DMA
void HOTSW_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size)
{
	HOTSWi_NDmaCopy_Card(ndmaNo, src, dest, size, MI_NDMA_DEST_INC);
}

void HOTSW_NDmaPipe_Card(u32 ndmaNo, const void *src, void *dest, u32 size)
{
	HOTSWi_NDmaCopy_Card(ndmaNo, src, dest, size, MI_NDMA_DEST_FIX);
}

static void HOTSWi_NDmaCopy_Card(u32 ndmaNo, const void *src, void *dest, u32 size, u32 dcont)
{
	u32 contData;
	OSIntrMode enabled = OS_DisableInterrupts();

    //--- Assert
	ASSERT_DMANO( ndmaNo );

	//---- confirm CARD free
	while( reg_HOTSW_MCCNT1 & REG_MI_MCCNT1_START_MASK ){}

	//---- confirm DMA free
	while( MI_IsNDmaBusy(ndmaNo) == TRUE ){}

	//---- set up registers
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_SAD_WOFFSET )  = (u32)src;
	MI_NDMA_REG( ndmaNo, MI_NDMA_REG_DAD_WOFFSET )  = (u32)dest;
	MI_NDMA_REG( ndmaNo, MI_NDMA_REG_BCNT_WOFFSET ) = NDMA_NO_INTERVAL_NORMAL_SCALE;
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_TCNT_WOFFSET ) = (u32)(size/4);
	MI_NDMA_REG( ndmaNo, MI_NDMA_REG_WCNT_WOFFSET ) = NDMA_WORD_COUNT_1;

	//---- decide control register
	contData  = NDMA_BLOCK_WORD_COUNT_1 | MI_NDMA_ENABLE;
	contData |= (MI_NDMA_SRC_FIX | dcont | MI_NDMA_DEST_RELOAD_DISABLE);
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
