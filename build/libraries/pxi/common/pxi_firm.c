/*---------------------------------------------------------------------------*
  Project:  TwlFirm - library - pxi
  File:     pxi_firm.c

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

#define PXI_FIRM_ID_MAX     8
#define PXI_FIRM_STREAM_MAX 16

typedef struct
{
    u32 wp;
    u32 rp;

    u8 id[PXI_FIRM_ID_MAX];

    u32 semaphore;  // for fs_loader.c

    u32 length;
    u32 current;
    u8 data[PXI_FIRM_STREAM_MAX];
}
PxiWork;
static PxiWork work;

static void PxiFirmStreamCallback( PXIFifoTag tag, u32 data, BOOL err )
{
    (void)tag;
    (void)err;
    if ( !work.length ) // stream is starting
    {
        if ( data > PXI_FIRM_STREAM_MAX )
        {
            OS_TPrintf("Receiving stream has too large size (%d > %d).\n", data, PXI_FIRM_STREAM_MAX);
        }
        work.length = data;
        work.current = 0;
    }
    else if ( work.current < work.length )  // stream is cotinuous
    {
        int i;
        u8* ptr = (u8*)&data;
        for ( i = 0; i < 3 && work.current < work.length; i++ )
        {
            work.data[ work.current++ ] = *ptr++;
        }
    }
    else
    {
        OS_TPrintf("Stream buffer was overflow because of multiple usage.\n");
    }
}

static void PxiFirmIDCallback( PXIFifoTag tag, u32 data, BOOL err )
{
    u32 next_wp = ( work.wp + 1 ) % PXI_FIRM_ID_MAX;
    (void)tag;
    (void)err;

    // special ID
    if ( data == FIRM_PXI_ID_LOAD_BUFFER_SEMAPHORE )
    {
        work.semaphore++;
        if ( work.semaphore > HW_FIRM_LOAD_BUFFER_UNIT_NUMS )
        {
            OS_TPanic("PROGRAM ERROR: Semaphore counter was overlow.");
        }
        return;
    }

    if ( next_wp != work.rp )
    {
        work.wp = next_wp;
        work.id[work.wp] = (u8)data;
    }
    else
    {
        OS_TPrintf("ID buffer was overflow (%d is ignored).\n", (u8)data);
    }
}

/*---------------------------------------------------------------------------*
  Name:         PXI_InitFIRM

  Description:  initialize PXI for firm

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_InitFIRM(void)
{
#ifdef SDK_ARM9
    work.semaphore = 0;
#else
    work.semaphore = HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
#endif
    work.rp = work.wp = work.length = 0;

    PXI_Init();
#ifdef SDK_ARM9
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_USER_1, PXI_PROC_ARM7))
    {
    }
#endif
    PXI_SetFifoRecvCallback( PXI_FIFO_TAG_USER_0, PxiFirmStreamCallback );
    PXI_SetFifoRecvCallback( PXI_FIFO_TAG_USER_1, PxiFirmIDCallback );
#ifdef SDK_ARM7
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_USER_1, PXI_PROC_ARM9))
    {
    }
#endif
#ifdef SDK_ARM9
    PXIi_WaitIDByIntf( FIRM_PXI_ID_INIT_ARM7 );
#else
    PXIi_SendIDByIntf( FIRM_PXI_ID_INIT_ARM7 );
#endif

}

/*---------------------------------------------------------------------------*
  Name:         PXI_SendStream

  Description:  Send data stream

  Arguments:    buf         pointer to data buffer
                size        transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_SendStream( const void* buf, int size )
{
    u8* ptr = (u8*)buf;
    while ( 0 > PXI_SendWordByFifo( PXI_FIFO_TAG_USER_0, (u32)size, 0 ) )
    {
    }
    while ( size > 0 )
    {
        u32 data = (u32)(ptr[0] << 0 | ptr[1] << 8 | ptr[2] << 16);
        while ( 0 > PXI_SendWordByFifo( PXI_FIFO_TAG_USER_0, data, 0 ) )
        {
        }
        size -= 3;
        ptr += 3;
    }
}

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvStream

  Description:  Receive data stream

  Arguments:    buf         pointer to data buffer
                size        transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_RecvStream( void* buf, int size )
{
    while ( 1 )
    {
        OSIntrMode enabled = OS_DisableInterrupts();
        if ( work.length && work.current >= work.length )
        {
            if ( size != work.length )
            {
                OS_TPrintf("Stream data size was not expected.");
            }
            else
            {
                MI_CpuCopy8( work.data, buf, (u32)size );
                work.length = work.current = 0;
            }
            OS_RestoreInterrupts( enabled );
            return;
        }
        OS_RestoreInterrupts( enabled );
    }
}

/*---------------------------------------------------------------------------*
  Name:         PXI_NotifyID

  Description:  Send ID

  Arguments:    id          id to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_NotifyID( FIRMPxiID id )
{
    while ( 0 > PXI_SendWordByFifo( PXI_FIFO_TAG_USER_1, id, 0 ) )
    {
    }
#if 0
#ifdef SDK_ARM9
        OS_TPrintf("[ARM9] Notify: %d\n", (u8)id);
#else
        OS_TPrintf("[ARM7] Notify: %d\n", (u8)id);
#endif
#endif
}

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvID

  Description:  Receive ID

  Arguments:    None

  Returns:      Received ID
 *---------------------------------------------------------------------------*/
FIRMPxiID PXI_RecvID( void )
{
    while ( 1 )
    {
        OSIntrMode enabled = OS_DisableInterrupts();
        if ( work.rp != work.wp )
        {
            FIRMPxiID id;
            work.rp = ( work.rp + 1 ) % PXI_FIRM_ID_MAX;
            id = (FIRMPxiID)work.id[work.rp];
            OS_RestoreInterrupts( enabled );
#if 0
#ifdef SDK_ARM9
        OS_TPrintf("[ARM9] Received: %d\n", id);
#else
        OS_TPrintf("[ARM7] Received: %d\n", id);
#endif
#endif
            return id;
        }
        OS_RestoreInterrupts( enabled );
    }
}

/*---------------------------------------------------------------------------*
  Name:         PXI_ReleaseLoadBufferSemaphore

  Description:  Release semaphore for *_LoadBuffer

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_ReleaseLoadBufferSemaphore( void )
{
    PXI_NotifyID( FIRM_PXI_ID_LOAD_BUFFER_SEMAPHORE );
}

/*---------------------------------------------------------------------------*
  Name:         PXI_AcquireLoadBufferSemaphore

  Description:  Acquire semaphore for *_LoadBuffer

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_AcquireLoadBufferSemaphore( void )
{
    while ( 1 )
    {
        OSIntrMode enabled = OS_DisableInterrupts();
        if ( work.semaphore > 0 )
        {
            work.semaphore--;
            OS_RestoreInterrupts( enabled );
            return;
        }
        OS_RestoreInterrupts( enabled );
    }
}

/*---------------------------------------------------------------------------*
  Name:         PXIi_SendIDByIntf

  Description:  Send 4bit id to the other processor

  Arguments:    id          sending id

  Returns:      None
 *---------------------------------------------------------------------------*/
void PXIi_SendIDByIntf( u32 id )
{
    reg_PXI_INTF = (u16)(id << REG_PXI_INTF_SEND_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         PXIi_RecvIDByIntf

  Description:  Receive 4bit id from the other processor

  Arguments:    None

  Returns:      received id
 *---------------------------------------------------------------------------*/
u32 PXIi_RecvIDByIntf( void )
{
    return (u32)((reg_PXI_INTF & REG_PXI_INTF_RECV_MASK) >> REG_PXI_INTF_RECV_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         PXIi_WaitIDByIntf

  Description:  Wait 4bit id from the other processor

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void PXIi_WaitIDByIntf( u32 id )
{
    while (PXIi_RecvIDByIntf() != id)
    {
    }
}

