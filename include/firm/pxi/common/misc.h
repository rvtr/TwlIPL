/*---------------------------------------------------------------------------*
  Project:  TwlFirm - -include - PXI
  File:     misc.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef FIRM_PXI_COMMON_MISC_H_
#define FIRM_PXI_COMMON_MISC_H_

#include <firm/pxi.h>
#include <twl/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

// PXI sync ID for bootrom
typedef enum
{
    // from ARM7
    FIRM_PXI_ID_COLDBOOT = 1,
    FIRM_PXI_ID_WARMBOOT = 2,

    FIRM_PXI_ID_INIT_ARM7 = 7,
    FIRM_PXI_ID_LOAD_HEADER = 6,
    FIRM_PXI_ID_LOAD_ARM9_STATIC = 5,
    FIRM_PXI_ID_LOAD_ARM7_STATIC = 4,
    FIRM_PXI_ID_LOAD_ARM9_LTD_STATIC = 3,
    FIRM_PXI_ID_LOAD_ARM7_LTD_STATIC = 2,
    FIRM_PXI_ID_LOAD_PIRIOD = 1,

    // from ARM9
    FIRM_PXI_ID_INIT_ARM9 = 9,
    FIRM_PXI_ID_AUTH_HEADER = 10,
    FIRM_PXI_ID_AUTH_ARM9_STATIC = 11,
    FIRM_PXI_ID_AUTH_ARM7_STATIC = 12,
    FIRM_PXI_ID_AUTH_ARM9_LTD_STATIC = 13,
    FIRM_PXI_ID_AUTH_ARM7_LTD_STATIC = 14,

    // from both of ARM9 and ARM7
    FIRM_PXI_ID_NULL = 0,
    FIRM_PXI_ID_ERR = 0xF
}
FIRMPxiID;


/*---------------------------------------------------------------------------*
  Name:         PXI_InitFifoFIRM

  Description:  initialize FIFO system for bootrom

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_InitFifoFIRM(void);

/*---------------------------------------------------------------------------*
  Name:         PXI_NotifyID

  Description:  send 4bit id to other processor

  Arguments:    id          notifying id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_NotifyID( u32 id );

/*---------------------------------------------------------------------------*
  Name:         PXI_WaitID

  Description:  Wait 4bit id from the other processor

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_WaitID( u32 id );

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvID

  Description:  Receive 4bit id from the other processor

  Arguments:    None

  Returns:      id
 *---------------------------------------------------------------------------*/
u8 PXI_RecvID( void );

/*---------------------------------------------------------------------------*
  Name:         PXI_SendIDByIntf

  Description:  send 4bit id to other processor

  Arguments:    id          notifying id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_SendIDByIntf( u32 id );

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvIDByIntf

  Description:  receive 4bit id from the other processor

  Arguments:    None

  Returns:      received id
 *---------------------------------------------------------------------------*/
u32  PXI_RecvIDByIntf( void );

/*---------------------------------------------------------------------------*
  Name:         PXI_WaitIDByIntf

  Description:  Wait 4bit id from the other processor

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_WaitIDByIntf( u32 id );

/*---------------------------------------------------------------------------*
  Name:         PXI_SendIDByFifo

  Description:  Send 32bit-word to another CPU via FIFO

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
void PXI_SendIDByFifo(PXIFifoTag tag, u32 id);

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvIDByFifo

  Description:  Recv 32bit-word from another CPU via FIFO

  Arguments:

  Returns:      if error occured, returns minus value
 *---------------------------------------------------------------------------*/
PXIFifoStatus PXI_RecvIDByFifo(PXIFifoTag tag, void* buf);

/*---------------------------------------------------------------------------*
  Name:         PXI_WaitIDByFifo

  Description:  Wait 32bit-word from another CPU via FIFO

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_WaitIDByFifo(PXIFifoTag tag, u32 id);

/*---------------------------------------------------------------------------*
  Name:         PXI_SendDataByFifo

  Description:  Send 32bit-word to another CPU via FIFO

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
void PXI_SendDataByFifo(PXIFifoTag tag, void* buf, int size);

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvDataByFifo

  Description:  Recv 32bit-word to another CPU via FIFO

  Arguments:

  Returns:      if error occured, returns minus value
 *---------------------------------------------------------------------------*/
PXIFifoStatus PXI_RecvDataByFifo(PXIFifoTag tag, void* buf, int max_size);

/*---------------------------------------------------------------------------*
  Name:         PXI_IsSendFifoFull

  Description:  Check if send fifo is full?

  Arguments:    None.

  Returns:      if send fifo is full
 *---------------------------------------------------------------------------*/
static inline BOOL PXI_IsSendFifoFull(void)
{
    return (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_SEND_FULL_MASK) >> REG_PXI_FIFO_CNT_SEND_FULL_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         PXI_IsRecvFifoEmpty

  Description:  Check if receive fifo is empty?

  Arguments:    None.

  Returns:      if receive fifo is empty
 *---------------------------------------------------------------------------*/
static inline BOOL PXI_IsRecvFifoEmpty(void)
{
    return (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_RECV_EMP_MASK) >> REG_PXI_FIFO_CNT_RECV_EMP_SHIFT;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FIRM_PXI_COMMON_MISC_H_ */
