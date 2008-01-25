/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_pxi.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "kami_pxi.h"
#include "fifo.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// 詰めてコピーする
#define KAMI_PACK_U16(d, s)                   \
    ((d)[0] = (u8)((*((u16*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u16*)s) >>  8) & 0xFF))

#define KAMI_PACK_U32(d, s)                   \
    ((d)[0] = (u8)((*((u32*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u32*)s) >>  8) & 0xFF),  \
     (d)[2] = (u8)((*((u32*)s) >> 16) & 0xFF),  \
     (d)[3] = (u8)((*((u32*)s) >> 24) & 0xFF))


/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct KamiWork
{
    BOOL lock;

    KamiCommand  command;
    KAMIPxiResult result;
    KAMICallback  callback;
    void*   arg;

    u32 total;
    u32 current;
    u8* data;
}
KamiWork;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static BOOL kamiInitialized;
static KamiWork kamiWork;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL KamiSendPxiCommand(KamiCommand command, u8 size, u8 data);
static void KamiSendPxiData(u8 *pData);
static void KamiPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void KamiDone(KAMIResult result);
static void KamiWaitBusy(void);


void KamiPxiInit( void )
{
    kamiWork.lock = FALSE;

    PXI_Init();
    while ( !PXI_IsCallbackReady(PXI_FIFO_TAG_KAMITEST, PXI_PROC_ARM7 ))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_KAMITEST, KamiPxiCallback);
    if ( 0 > PXI_SendWordByFifo(PXI_FIFO_TAG_KAMITEST, 0, 0))
    {
        return;
    }
}

///////////////////////////////////////////////////////////////////

//typedef void (*KAMICallback)(KAMIResult result, void *arg);
/*
void CDC_ReadCallback(KAMIResult result, void* arg);
void CDC_ReadCallback(KAMIResult result, void* arg)
{

}
*/

/*---------------------------------------------------------------------------*
  Name:         CODEC レジスタリード関数

  Description:  

  Arguments:    None.

  Returns:      
 *---------------------------------------------------------------------------*/

KAMIResult CDC_ReadRegister(u8 page, u8 reg_no, u8* pData)
{
    OSIntrMode enabled;


	// ロック
    enabled = OS_DisableInterrupts();
    if (kamiWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return KAMI_RESULT_BUSY;
    }
    kamiWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    kamiWork.callback = NULL;
    kamiWork.arg = 0;
    kamiWork.data = (u8*)pData;

    if (KamiSendPxiCommand(CODEC_READ_REGISTER, 2, page))
    {
        KamiSendPxiData(&reg_no);
	    KamiWaitBusy();
	    return (KAMIResult)kamiWork.result;
    }
    return KAMI_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CODEC レジスタライト関数

  Description:  

  Arguments:    None.

  Returns:      
 *---------------------------------------------------------------------------*/

KAMIResult CDC_WriteRegister(u8 page, u8 reg_no, u8 value)
{
    OSIntrMode enabled;
    u8  data[2];
	int i;


	// ロック
    enabled = OS_DisableInterrupts();
    if (kamiWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return KAMI_RESULT_BUSY;
    }
    kamiWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    kamiWork.callback = NULL;
    kamiWork.arg = 0;
    kamiWork.data = 0;

	// データ作成
	data[0] = reg_no;
	data[1] = value;

    if (KamiSendPxiCommand(CODEC_WRITE_REGISTER, 3, page))
    {
	    for (i = 0; i < 2; i++) 
		{
	        KamiSendPxiData(&data[i]);
		}
	    KamiWaitBusy();
	    return (KAMIResult)kamiWork.result;
    }
    return KAMI_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         GPIO333 アクセス関数

  Description:  

  Arguments:    None.

  Returns:      
 *---------------------------------------------------------------------------*/

KAMIResult GPIO333_Write(BOOL value)
{
    OSIntrMode enabled;

	// ロック
    enabled = OS_DisableInterrupts();
    if (kamiWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return KAMI_RESULT_BUSY;
    }
    kamiWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    kamiWork.callback = NULL;
    kamiWork.arg = 0;
    kamiWork.data = 0;

    if (KamiSendPxiCommand(GPIO333_WRITE, 1, (u8)value))
    {
	    KamiWaitBusy();
	    return (KAMIResult)kamiWork.result;
    }
    return KAMI_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         フォーマット実行関数

  Description:  

  Arguments:    FormatMode

  Returns:      
 *---------------------------------------------------------------------------*/

KAMIResult ExeFormatAsync(FormatMode format_mode, KAMICallback callback)
{
    OSIntrMode enabled;

	// ロック
    enabled = OS_DisableInterrupts();
    if (kamiWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return KAMI_RESULT_BUSY;
    }
    kamiWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    kamiWork.callback = callback;
    kamiWork.arg = 0;
    kamiWork.data = 0;

    if (KamiSendPxiCommand(EXE_FORMAT, 1, format_mode) == FALSE)
    {
    	return KAMI_RESULT_SEND_ERROR;
    }
    return KAMI_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NANDアクセス関数

  Description:  

  Arguments:    None.

  Returns:      
 *---------------------------------------------------------------------------*/

KAMIResult kamiNandIo(u32 block, void* buffer, u32 count, BOOL is_read)
{
    OSIntrMode enabled;
    u8  data[12];
	int i;

	// ロック
    enabled = OS_DisableInterrupts();
    if (kamiWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return KAMI_RESULT_BUSY;
    }
    kamiWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    kamiWork.callback = NULL;
    kamiWork.arg = 0;
    kamiWork.data = 0;

	// データ作成
	KAMI_PACK_U32(&data[0], &block);
	KAMI_PACK_U32(&data[4], &buffer);
	KAMI_PACK_U32(&data[8], &count);

    if (KamiSendPxiCommand(KAMI_NAND_IO, 12, (u8)is_read))
    {
	    for (i = 0; i < 12; i+=3) 
		{
	        KamiSendPxiData(&data[i]);
		}
	    KamiWaitBusy();
	    return (KAMIResult)kamiWork.result;
    }
    return KAMI_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         Nvramアクセス関数

  Description:  

  Arguments:    None.

  Returns:      
 *---------------------------------------------------------------------------*/
KAMIResult kamiNvramIo(u32 address, void* buffer, u32 size, BOOL is_read)
{
    OSIntrMode enabled;
    u8  data[12];
	int i;

	// ロック
    enabled = OS_DisableInterrupts();
    if (kamiWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return KAMI_RESULT_BUSY;
    }
    kamiWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    kamiWork.callback = NULL;
    kamiWork.arg = 0;
    kamiWork.data = 0;

	// データ作成
	KAMI_PACK_U32(&data[0], &address);
	KAMI_PACK_U32(&data[4], &buffer);
	KAMI_PACK_U32(&data[8], &size);

    if (KamiSendPxiCommand(KAMI_NVRAM_IO, 12, (u8)is_read))
    {
	    for (i = 0; i < 12; i+=3) 
		{
	        KamiSendPxiData(&data[i]);
		}
	    KamiWaitBusy();
	    return (KAMIResult)kamiWork.result;
    }
    return KAMI_RESULT_SEND_ERROR;
}
///////////////////////////////////////////////////////////////////


/////////
static BOOL KamiSendPxiCommand(KamiCommand command, u8 size, u8 data)
{
    u32 pxiData = (u32)(KAMITEST_PXI_START_BIT |
            ((command << KAMITEST_PXI_COMMAND_SHIFT) & KAMITEST_PXI_COMMAND_MASK) |
            ((size << KAMITEST_PXI_DATA_NUMS_SHIFT) & KAMITEST_PXI_DATA_NUMS_MASK) |
            ((data << KAMITEST_PXI_1ST_DATA_SHIFT) & KAMITEST_PXI_1ST_DATA_MASK));
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_KAMITEST, pxiData, 0))
    {
        return FALSE;
    }
    return TRUE;
}

static void KamiSendPxiData(u8 *pData)
{
    u32 pxiData = (u32)((pData[0] << 16) | (pData[1] << 8) | pData[2]);
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_KAMITEST, pxiData, 0))
    {
    }
}

static void KamiPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    if (err)
    {
        KamiDone(KAMI_RESULT_FATAL_ERROR);
        return;
    }
    if (data & KAMITEST_PXI_START_BIT)   // 先頭データ
    {
        if (data & KAMITEST_PXI_RESULT_BIT)
        {
            kamiWork.total = (u8)((data & KAMITEST_PXI_DATA_NUMS_MASK) >> KAMITEST_PXI_DATA_NUMS_SHIFT);
            kamiWork.current = 0;
            kamiWork.command = (KamiCommand)((data & KAMITEST_PXI_COMMAND_MASK) >> KAMITEST_PXI_COMMAND_SHIFT);
            kamiWork.result = (KAMIPxiResult)((data & KAMITEST_PXI_1ST_DATA_MASK) >> KAMITEST_PXI_1ST_DATA_SHIFT);
        }
        else    // 未知のデータ
        {
            KamiDone(KAMI_RESULT_FATAL_ERROR);
            return;
        }
    }
    else    // 後続データ
    {
        if (kamiWork.data == NULL)
        {
            KamiDone(KAMI_RESULT_FATAL_ERROR);
            return;
        }
        if (kamiWork.current < kamiWork.total-1)
        {
            kamiWork.data[kamiWork.current++] = (u8)((data & 0xFF0000) >> 16);
        }
        if (kamiWork.current < kamiWork.total-1)
        {
            kamiWork.data[kamiWork.current++] = (u8)((data & 0x00FF00) >> 8);
        }
        if (kamiWork.current < kamiWork.total-1)
        {
            kamiWork.data[kamiWork.current++] = (u8)((data & 0x0000FF) >> 0);
        }
    }
    if (kamiWork.current == kamiWork.total-1)
    {
        KAMIResult result;
        switch (kamiWork.result)
        {
        case KAMI_PXI_RESULT_SUCCESS:     // alias KAMI_PXI_RESULT_SUCCESS_TRUE
            result = KAMI_RESULT_SUCCESS; // alias KAMI_RESULT_SUCCESS_TRUE
            break;
        case KAMI_PXI_RESULT_SUCCESS_FALSE:
            result = KAMI_RESULT_SUCCESS_FALSE;
            break;
        case KAMI_PXI_RESULT_INVALID_COMMAND:
            result = KAMI_RESULT_INVALID_COMMAND;
            break;
        case KAMI_PXI_RESULT_INVALID_PARAMETER:
            result = KAMI_RESULT_INVALID_PARAMETER;
            break;
        case KAMI_PXI_RESULT_ILLEGAL_STATUS:
            result = KAMI_RESULT_ILLEGAL_STATUS;
            break;
        case KAMI_PXI_RESULT_BUSY:
            result = KAMI_RESULT_BUSY;
            break;
        default:
            result = KAMI_RESULT_FATAL_ERROR;
        }
        KamiDone(result);
        return;
    }
}

extern void PXIi_HandlerRecvFifoNotEmpty(void);
static void KamiWaitBusy(void)
{
    volatile BOOL *p = &kamiWork.lock;

    while (*p)
    {
        if (OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE)
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}

static void KamiDone(KAMIResult result)
{
    KAMICallback callback = kamiWork.callback;
    void* arg = kamiWork.arg;
    kamiWork.callback = NULL;
    kamiWork.arg = NULL;
    if (kamiWork.lock)
    {
        kamiWork.lock = FALSE;
    }
    if (callback)
    {
        callback(result, arg);
    }
}
