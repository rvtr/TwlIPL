/*---------------------------------------------------------------------------*
  Project:  TwlFirm - -include - PXI
  File:     pxi_firm.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef FIRM_PXI_COMMON_FIRM_H_
#define FIRM_PXI_COMMON_FIRM_H_

#include <firm/pxi.h>
#include <twl/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

// PXI sync ID for bootrom
typedef enum
{
    // from ARM7
    FIRM_PXI_ID_COLDBOOT = 1,   // _start
    FIRM_PXI_ID_WARMBOOT = 2,   // _start

    FIRM_PXI_ID_INIT_ARM7 = 7,  // PXI_InitFifoFIRM

    FIRM_PXI_ID_LOAD_HEADER = 6,    // FATFS_LoadHeader
    FIRM_PXI_ID_LOAD_STATIC = 5,    // FATFS_LoadStatic

    FIRM_PXI_ID_LOAD_BUFFER_SEMAPHORE = 1,    // *_LoadBuffer

    // from ARM9
    FIRM_PXI_ID_INIT_MMEM = 3,  // _start

    FIRM_PXI_ID_INIT_ARM9 = 9,  // PXI_InitFifoFIRM

    FIRM_PXI_ID_AUTH_HEADER = 10,   // MI_LoadHeader
    FIRM_PXI_ID_AUTH_STATIC = 11,   // MI_LoadStatic

    FIRM_PXI_ID_SET_PATH = 4,       // main (nandfirm)
    FIRM_PXI_ID_DONE_HEADER = 5,    // main (nandfirm)
    FIRM_PXI_ID_DONE_STATIC = 6,    // main (nandfirm)

    // from both of ARM9 and ARM7
    FIRM_PXI_ID_NULL = 0,
    FIRM_PXI_ID_ERR = 0xF
}
FIRMPxiID;


/*---------------------------------------------------------------------------*
  Name:         PXI_InitFIRM

  Description:  initialize PXI for firm

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_InitFIRM(void);

/*---------------------------------------------------------------------------*
  Name:         PXI_SendStream

  Description:  Send data stream

  Arguments:    buf         pointer to data buffer
                size        transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_SendStream( const void* buf, int size );

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvStream

  Description:  Receive data stream

  Arguments:    buf         pointer to data buffer
                size        transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_RecvStream( void* buf, int size );

/*---------------------------------------------------------------------------*
  Name:         PXI_NotifyID

  Description:  Send ID

  Arguments:    id          id to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_NotifyID( FIRMPxiID id );

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvID

  Description:  Receive ID

  Arguments:    None

  Returns:      Received ID
 *---------------------------------------------------------------------------*/
FIRMPxiID PXI_RecvID( void );

/*---------------------------------------------------------------------------*
  Name:         PXI_ReleaseLoadBufferSemaphore

  Description:  Release semaphore for *_LoadBuffer

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_ReleaseLoadBufferSemaphore( void );

/*---------------------------------------------------------------------------*
  Name:         PXI_AcquireLoadBufferSemaphore

  Description:  Acquire semaphore for *_LoadBuffer

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_AcquireLoadBufferSemaphore( void );

/*---------------------------------------------------------------------------*
  Name:         PXIi_SendIDByIntf

  Description:  send 4bit id to other processor

  Arguments:    id          notifying id

  Returns:      None
 *---------------------------------------------------------------------------*/
void PXIi_SendIDByIntf( u32 id );

/*---------------------------------------------------------------------------*
  Name:         PXIi_RecvIDByIntf

  Description:  receive 4bit id from the other processor

  Arguments:    None

  Returns:      received id
 *---------------------------------------------------------------------------*/
u32 PXIi_RecvIDByIntf( void );

/*---------------------------------------------------------------------------*
  Name:         PXIi_WaitIDByIntf

  Description:  Wait 4bit id from the other processor

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void PXIi_WaitIDByIntf( u32 id );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FIRM_PXI_COMMON_FIRM_H_ */
