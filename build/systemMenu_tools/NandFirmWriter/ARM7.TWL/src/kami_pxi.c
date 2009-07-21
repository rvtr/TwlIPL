/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_pxi.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-07-30#$
  $Rev: 2031 $
  $Author: kamikawa $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/exi/ARM7/genPort2.h>
#include "kami_pxi.h"
#include "fifo.h"
#include "twl/cdc.h"
#include <twl/ltdmain_begin.h>
#include <twl/mcu.h>
#include <twl/camera.h>
#include <twl/camera/ARM7/i2c_sharp.h>
#include <twl/camera/ARM7/i2c_micron.h>

typedef unsigned char byte;     /* Don't change */
typedef unsigned short word;    /* Don't change */
typedef unsigned long dword;    /* Don't change */
#define BOOLEAN int

extern BOOL FATFSi_nandRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading);
extern BOOL sdmcFormatNandLog( BOOL verify_flag);

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define KAMITEST_MESSAGE_ARRAY_MAX       4        // スレッド同期用メッセージキューのサイズ
#define KAMITEST_THREAD_STACK_SIZE       2048     // スレッドのスタックサイズ

#define KAMITEST_THREAD_PRIORITY          6

// アライメント調整してコピーする
#define KAMI_UNPACK_U16(d, s)    \
    (*(d) = (u16)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8)))
#define KAMI_UNPACK_U32(d, s)    \
    (*(d) = (u32)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8) | (((u8*)s)[2] << 16) | (((u8*)s)[3] << 24)))

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct KamiWork
{
    BOOL result;
    u32  total;
    u32  current;
    KamiCommand  command;
    u8 data[KAMITEST_PXI_DATA_SIZE_MAX];   // 後続データ格納用

    OSMessageQueue msgQ;               // スレッド同期用メッセージキュー
    OSMessage msgArray[KAMITEST_MESSAGE_ARRAY_MAX];
    // メッセージを格納するバッファ
    OSThread thread;                   // KAMI用スレッド
    u64     stack[KAMITEST_THREAD_STACK_SIZE / sizeof(u64)];
    // KAMI用スレッドのスタック
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
static void KamiPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void KamiReturnResult(KamiCommand command, KAMIPxiResult result);
static void KamiReturnResultEx(KamiCommand command, KAMIPxiResult result, u8 size, u8* data);
static void KamiThread(void *arg);

void KamiPxiInit(void)
{
    if (kamiInitialized)
    {
        return;
    }
    kamiInitialized = TRUE;

    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_KAMITEST, KamiPxiCallback);

    OS_InitMessageQueue(&kamiWork.msgQ, kamiWork.msgArray, KAMITEST_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&kamiWork.thread, KamiThread, 0,
                    (void *)(kamiWork.stack + (KAMITEST_THREAD_STACK_SIZE / sizeof(u64))),
                    KAMITEST_THREAD_STACK_SIZE, KAMITEST_THREAD_PRIORITY);
    OS_WakeupThreadDirect(&kamiWork.thread);
}

static void KamiPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    if (err)
    {
        return;
    }
    if (data & KAMITEST_PXI_START_BIT)   // 先頭データ
    {
        kamiWork.total = (u8)((data & KAMITEST_PXI_DATA_NUMS_MASK) >> KAMITEST_PXI_DATA_NUMS_SHIFT);
        kamiWork.current = 0;
        kamiWork.command = (KamiCommand)((data & KAMITEST_PXI_COMMAND_MASK) >> KAMITEST_PXI_COMMAND_SHIFT);
        kamiWork.data[kamiWork.current++] = (u8)((data & KAMITEST_PXI_1ST_DATA_MASK) >> KAMITEST_PXI_1ST_DATA_SHIFT);
    }
    else    // 後続データ
    {
        kamiWork.data[kamiWork.current++] = (u8)((data & 0xFF0000) >> 16);
        kamiWork.data[kamiWork.current++] = (u8)((data & 0x00FF00) >> 8);
        kamiWork.data[kamiWork.current++] = (u8)((data & 0x0000FF) >> 0);
    }
    if (kamiWork.current >= kamiWork.total)
    {
        switch (kamiWork.command)
        {
		case KAMI_EXE_FORMAT:
		case KAMI_NAND_IO:
		case KAMI_MCU_WRITE_FIRM:
		case KAMI_MCU_IO:
		case KAMI_ARM7_IO:
		case KAMI_CDC_GO_DSMODE:
		case KAMI_CLEAR_NAND_ERRORLOG:
		case KAMI_GET_CAMERA_MODULE_TYPE:
		case KAMI_GET_NAND_CID:
            if (!OS_SendMessage(&kamiWork.msgQ, NULL, OS_MESSAGE_NOBLOCK))
            {
                KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_FATAL_ERROR);
            }
            break;

        default:
            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

static void KamiReturnResult(KamiCommand command, KAMIPxiResult result)
{
    u32 pxiData = (u32)(KAMITEST_PXI_START_BIT | KAMITEST_PXI_RESULT_BIT |
            ((command << KAMITEST_PXI_COMMAND_SHIFT) & KAMITEST_PXI_COMMAND_MASK) |
            ((1 << KAMITEST_PXI_DATA_NUMS_SHIFT) & KAMITEST_PXI_DATA_NUMS_MASK) |
            ((result << KAMITEST_PXI_1ST_DATA_SHIFT) & KAMITEST_PXI_1ST_DATA_MASK));
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_KAMITEST, pxiData, 0))
    {
    }
}

static void KamiReturnResultEx(KamiCommand command, KAMIPxiResult result, u8 size, u8* data)
{
    u32 pxiData = (u32)(KAMITEST_PXI_START_BIT | KAMITEST_PXI_RESULT_BIT |
            ((command << KAMITEST_PXI_COMMAND_SHIFT) & KAMITEST_PXI_COMMAND_MASK) |
            (((size+1) << KAMITEST_PXI_DATA_NUMS_SHIFT) & KAMITEST_PXI_DATA_NUMS_MASK) |
            ((result << KAMITEST_PXI_1ST_DATA_SHIFT) & KAMITEST_PXI_1ST_DATA_MASK));
    int i;
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_KAMITEST, pxiData, 0))
    {
    }
    for (i = 0; i < size; i+= 3)
    {
        pxiData = (u32)((data[i] << 16) | (data[i+1] << 8) | data[i+2]);
        while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_KAMITEST, pxiData, 0))
        {
        }
    }
}

static void KamiThread(void *arg)
{
#pragma unused( arg )
    OSMessage msg;
    BOOL result;

    while (TRUE)
    {
        (void)OS_ReceiveMessage(&kamiWork.msgQ, &msg, OS_MESSAGE_BLOCK);
        switch (kamiWork.command)
        {
		case KAMI_NAND_IO:
			{
				BOOL is_read;
				u32  block;
				void* buffer;
				u32  count;

				is_read = (BOOL)kamiWork.data[0];
				KAMI_UNPACK_U32(&block,  &kamiWork.data[1]);
				KAMI_UNPACK_U32((u32 *)(&buffer), &kamiWork.data[5]);
				KAMI_UNPACK_U32(&count,  &kamiWork.data[9]);

				result = FATFSi_nandRtfsIo( 0, block, buffer, (u16)count, is_read );
				if (result)
				{
	                KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS_TRUE);
				}
				else
				{
	                KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS_FALSE);
				}
			}
			break;

        default:
            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

#include <twl/ltdmain_end.h>
