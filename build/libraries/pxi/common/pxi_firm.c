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

#define PXI_FIRM_ID_MAX     32
#define PXI_FIRM_STREAM_MAX 32

typedef struct
{
    u32 wp;
    u32 rp;
    u8 id[PXI_FIRM_ID_MAX];

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
        if ( work.length >= PXI_FIRM_STREAM_MAX )
        {
            OS_TPrintf("Receiving stream has too large size (%d >= %d).\n", work.length, PXI_FIRM_STREAM_MAX);
        }
        work.length = data;
        work.current = 0;
    }
    else if ( work.current < work.length )  // stream is cotinuous
    {
        int i;
        for ( i = 0; i < 3 && work.current < work.length; i++ )
        {
            work.data[ work.current++ ] = (u8)( (data >> 16) & 0xFF );
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
    PXI_Init();
#ifdef SDK_ARM9
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_USER_0, PXI_PROC_ARM7))
    {
    }
#endif
    work.rp = work.wp = work.length = 0;
    PXI_SetFifoRecvCallback( PXI_FIFO_TAG_USER_0, PxiFirmStreamCallback );
    PXI_SetFifoRecvCallback( PXI_FIFO_TAG_USER_1, PxiFirmIDCallback );
#ifdef SDK_ARM7
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_USER_1, PXI_PROC_ARM9))
    {
    }
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
        u32 data = (u32)(ptr[0] << 16 | ptr[1] << 8 | ptr[2] << 0);
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
#ifdef SDK_ARM9
        OS_TPrintf("[ARM9] Notify: %d\n", (u8)id);
#else
        OS_TPrintf("[ARM7] Notify: %d\n", (u8)id);
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
#ifdef SDK_ARM9
        OS_TPrintf("[ARM9] Received: %d\n", id);
#else
        OS_TPrintf("[ARM7] Received: %d\n", id);
#endif
            return id;
        }
        OS_RestoreInterrupts( enabled );
    }
}
