/*---------------------------------------------------------------------------*
  Project:  TwlFirm - library - pxi
  File:     pxi_misc.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include  <firm/os.h>
#include  <twl/memorymap.h>
#include  <firm/pxi.h>

static u16 FifoCtrlInit = 0;

/*********** function prototypes ******************/
static inline PXIFifoStatus PXIi_GetFromFifo(u32 *data_buf);
static inline PXIFifoStatus PXIi_SetToFifo(u32 data);


/*---------------------------------------------------------------------------*
  Name:         PXI_InitFifoFIRM

  Description:  initialize FIFO system for firm

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_InitFifoFIRM(void)
{
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    if (!FifoCtrlInit)
    {
        FifoCtrlInit = TRUE;

        reg_PXI_FIFO_CNT =
            (REG_PXI_FIFO_CNT_SEND_CL_MASK |
             REG_PXI_FIFO_CNT_E_MASK | REG_PXI_FIFO_CNT_ERR_MASK);

#ifdef SDK_ARM9
        PXI_SendIDByIntf( FIRM_PXI_ID_INIT_ARM9 );
        PXI_WaitIDByIntf( FIRM_PXI_ID_INIT_ARM7 );
#else // SDK_ARM7
        PXI_SendIDByIntf( FIRM_PXI_ID_INIT_ARM7 );
        PXI_WaitIDByIntf( FIRM_PXI_ID_INIT_ARM9 );
#endif // SDK_ARM7
    }
    (void)OS_RestoreInterrupts(enabled);
}


/*---------------------------------------------------------------------------*
  Name:         PXI_NotifyID

  Description:  Send 4bit id to other processor

  Arguments:    id          notifying id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_NotifyID( u32 id )
{
    PXI_SendIDByFifo( PXI_FIFO_TAG_SYSTEM, id );
}

/*---------------------------------------------------------------------------*
  Name:         PXI_WaitID

  Description:  Wait 4bit id from the other processor

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_WaitID( u32 id )
{
    PXI_WaitIDByFifo( PXI_FIFO_TAG_SYSTEM, id );
}

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvID

  Description:  Receive 4bit id from the other processor

  Arguments:    None

  Returns:      id
 *---------------------------------------------------------------------------*/
u8 PXI_RecvID( void )
{
    u8 id;

    while (PXI_RecvIDByFifo(PXI_FIFO_TAG_SYSTEM, &id) != PXI_FIFO_SUCCESS)
    {
    }

    return id;
}

/*---------------------------------------------------------------------------*
  Name:         PXI_SendIDByIntf

  Description:  Send 4bit id to the other processor

  Arguments:    id          sending id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_SendIDByIntf( u32 id )
{
    reg_PXI_INTF = (u16)(id << REG_PXI_INTF_SEND_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvIDByIntf

  Description:  Receive 4bit id from the other processor

  Arguments:    None

  Returns:      received id
 *---------------------------------------------------------------------------*/
u32  PXI_RecvIDByIntf( void )
{
    return (u32)((reg_PXI_INTF & REG_PXI_INTF_RECV_MASK) >> REG_PXI_INTF_RECV_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         PXI_WaitIDByIntf

  Description:  Wait 4bit id from the other processor

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_WaitIDByIntf( u32 id )
{
    while (PXI_RecvIDByIntf() != id)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         PXI_SendIDByFifo

  Description:  Send 32bit-word to another CPU via FIFO

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
void PXI_SendIDByFifo(PXIFifoTag tag, u32 id)
{
    static PXIFifoMessage fifomsg;

    fifomsg.e.tag = tag;
    fifomsg.e.data = id;

    while ( PXIi_SetToFifo(fifomsg.raw) != PXI_FIFO_SUCCESS )
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvIDByFifo

  Description:  Recv 32bit-word from another CPU via FIFO

  Arguments:

  Returns:      if error occured, returns minus value
 *---------------------------------------------------------------------------*/
PXIFifoStatus PXI_RecvIDByFifo(PXIFifoTag tag, void* buf)
{
    static PXIFifoMessage fifomsg;
    u8* p = buf;

    while ( PXIi_GetFromFifo(&fifomsg.raw) != PXI_FIFO_SUCCESS )
    {
    }

    if (fifomsg.e.tag != tag)
    {
        return PXI_FIFO_FAIL_RECV_ERR;
    }

    *p = (u8)fifomsg.e.data;

    return PXI_FIFO_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         PXI_WaitIDByFifo

  Description:  Wait 32bit-word from another CPU via FIFO

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_WaitIDByFifo(PXIFifoTag tag, u32 id)
{
    u8 buf = (u8)id;

    do
    {
        while (PXI_RecvIDByFifo(tag, &buf) != PXI_FIFO_SUCCESS)
        {
        }
    }
    while ( buf != id );
}

/*---------------------------------------------------------------------------*
  Name:         PXI_SendDataByFifo

  Description:  Send data to another CPU via FIFO

  Arguments:

  Returns:      None
 *---------------------------------------------------------------------------*/
void PXI_SendDataByFifo(PXIFifoTag tag, void* buf, int size)
{
    static PXIFifoMessage fifomsg;
    u32* p = buf;
    int len = size/4;
    int i;

    fifomsg.e.tag = tag;
    fifomsg.e.data = len;

    while ( PXIi_SetToFifo(fifomsg.raw) != PXI_FIFO_SUCCESS )
    {
    }

    for ( i=0; i<len; i++ )
    {
        while ( PXIi_SetToFifo(p[i]) != PXI_FIFO_SUCCESS )
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvDataByFifo

  Description:  Recv data to another CPU via FIFO

  Arguments:

  Returns:      if error occured, returns minus value
 *---------------------------------------------------------------------------*/
PXIFifoStatus PXI_RecvDataByFifo(PXIFifoTag tag, void* buf, int max_size )
{
    static PXIFifoMessage fifomsg;
    u32* p = buf;
    u32 len;
    int i;

    while ( PXIi_GetFromFifo(&fifomsg.raw) != PXI_FIFO_SUCCESS )
    {
    }

    if (fifomsg.e.tag != tag)
    {
        return PXI_FIFO_FAIL_SEND_ERR;
    }

    len = fifomsg.e.data;

    if (len > max_size/4)
    {
        return PXI_FIFO_FAIL_SEND_ERR;
    }

    for ( i=0; i<len; i++ )
    {
        while ( PXIi_GetFromFifo(&p[i]) != PXI_FIFO_SUCCESS )
        {
        }
    }

    return PXI_FIFO_SUCCESS;
}


//======================================================================
//                      Write Send-FIFO reg.
//======================================================================
static inline PXIFifoStatus PXIi_SetToFifo(u32 data)
{
    OSIntrMode enabled;

    if (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_ERR_MASK)
    {
        reg_PXI_FIFO_CNT |= (REG_PXI_FIFO_CNT_E_MASK | REG_PXI_FIFO_CNT_ERR_MASK);
        return PXI_FIFO_FAIL_SEND_ERR;
    }

    enabled = OS_DisableInterrupts();
    if (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_SEND_FULL_MASK)
    {
        (void)OS_RestoreInterrupts(enabled);
        return PXI_FIFO_FAIL_SEND_FULL;
    }

    reg_PXI_SEND_FIFO = data;
    (void)OS_RestoreInterrupts(enabled);
    return PXI_FIFO_SUCCESS;
}


//======================================================================
//                      Read Send-FIFO reg.
//======================================================================
static inline PXIFifoStatus PXIi_GetFromFifo(u32 *data_buf)
{

    OSIntrMode enabled;

    if (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_ERR_MASK)
    {
        reg_PXI_FIFO_CNT |= (REG_PXI_FIFO_CNT_E_MASK | REG_PXI_FIFO_CNT_ERR_MASK);
        return PXI_FIFO_FAIL_RECV_ERR;
    }

    enabled = OS_DisableInterrupts();
    if (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_RECV_EMP_MASK)
    {
        (void)OS_RestoreInterrupts(enabled);
        return PXI_FIFO_FAIL_RECV_EMPTY;
    }

    *data_buf = reg_PXI_RECV_FIFO;
    (void)OS_RestoreInterrupts(enabled);

    return PXI_FIFO_SUCCESS;
}


