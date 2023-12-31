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
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/exi/ARM7/genPort2.h>
#include "kami_pxi.h"
#include "fifo.h"
#include "twl/cdc.h"
#include "formatter.h"
#include "mcu_firm.h"
#include <twl/ltdmain_begin.h>
#include <twl/mcu.h>
#include <twl/camera.h>
#include <twl/camera/ARM7/i2c_sharp.h>
#include <twl/camera/ARM7/i2c_micron.h>

/* sdmc.h はFATFSライブラリ内の非公開ヘッダのため、必要な定義をローカルで持つ。 */
//#include <twl/sdmc.h>

typedef enum {
    SDMC_PORT_CARD    = 0x400,
    SDMC_PORT_NAND    = 0x401
}SDMC_PORT_NO;

#define TRUE  1                 /* Don't change */
#define FALSE 0                 /* Don't change */

typedef unsigned char byte;     /* Don't change */
typedef unsigned short word;    /* Don't change */
typedef unsigned long dword;    /* Don't change */

#define BOOLEAN int

extern BOOL FATFSi_nandRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading);
extern BOOL sdmcFormatNandLog( BOOL verify_flag);
extern void sdmcGetCID( SDMC_PORT_NO port, u32* dest);
extern void SPI_Lock(u32 id);
extern void SPI_Unlock(u32 id);

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define KAMITEST_MESSAGE_ARRAY_MAX       4        // スレッド同期用メッセージキューのサイズ
#define KAMITEST_THREAD_STACK_SIZE       2048     // スレッドのスタックサイズ

#define KAMITEST_THREAD_PRIORITY          6

// アライメント調整してコピーする
#define KAMI_UNPACK_U8(d, s)    \
    (*(d) = (u8)((((u8*)s)[0] << 0)))
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
static u32  kamiSpiLockId;

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
    kamiSpiLockId = (u32)OS_GetLockID();
    if (kamiSpiLockId == OS_LOCK_ID_ERROR)
    {
        OS_Panic("%s: OS_GetLockID failed.\n", __FUNCTION__);
    }

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
		case KAMI_CODEC_IO:
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
        case KAMI_EXE_FORMAT:
            {
				result = ExeFormat((FormatMode)kamiWork.data[0]);	// Quick or Full
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

		case KAMI_MCU_WRITE_FIRM:
			{
				void* buffer;
				KAMI_UNPACK_U32((u32 *)(&buffer), &kamiWork.data[1]);

				if ( MCU_WriteFirm( buffer ) )
				{
		            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS);
				}
				else
				{
		            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS_FALSE);
				}
			}
			break;

		case KAMI_MCU_IO:
			{
				BOOL is_read;
				u32  reg_no;
				u32  write;
				u32  read;

				is_read = (BOOL)kamiWork.data[0];
				KAMI_UNPACK_U32(&reg_no,  &kamiWork.data[1]);
				KAMI_UNPACK_U32(&write,  &kamiWork.data[5]);

				if (is_read)
				{
					read = MCU_ReadRegister( (u8)reg_no );
	            	KamiReturnResultEx(kamiWork.command, KAMI_PXI_RESULT_SUCCESS,  sizeof(u8), (u8*)&read );
				}
				else
				{
					MCU_WriteRegister( (u8)reg_no, (u8)write );
		            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS);
				}
			}
			break;

		case KAMI_CODEC_IO:
			{
				BOOL is_read;
				u8  page;
				u8  reg_no;
				u32  write;
				u32  read;

				is_read = (BOOL)kamiWork.data[0];
				KAMI_UNPACK_U8(&page,    &kamiWork.data[1]);
				KAMI_UNPACK_U8(&reg_no,  &kamiWork.data[2]);
				KAMI_UNPACK_U32(&write,  &kamiWork.data[5]);

				if (is_read)
				{
			        SPI_Lock(kamiSpiLockId);    // CODEC用SPI排他ロック
					read = CDC_ReadSpiRegisterEx( page, reg_no );
			        SPI_Unlock(kamiSpiLockId);  // CODEC用SPI排他ロック
	            	KamiReturnResultEx(kamiWork.command, KAMI_PXI_RESULT_SUCCESS,  sizeof(u8), (u8*)&read );
				}
				else
				{
			        SPI_Lock(kamiSpiLockId);    // CODEC用SPI排他ロック
					CDC_WriteSpiRegisterEx( page, reg_no, (u8)write );
			        SPI_Unlock(kamiSpiLockId);  // CODEC用SPI排他ロック
		            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS);
				}
			}
			break;

		case KAMI_ARM7_IO:
			{
				BOOL is_read;
				u32  addr;
				u32  write;
				u32  read;

				is_read = (BOOL)kamiWork.data[0];
				KAMI_UNPACK_U32(&addr,  &kamiWork.data[1]);
				KAMI_UNPACK_U32(&write, &kamiWork.data[5]);

				if (is_read)
				{
					read = *(u32 *)addr;
	            	KamiReturnResultEx(kamiWork.command, KAMI_PXI_RESULT_SUCCESS,  sizeof(u32), (u8*)&read );
				}
				else
				{
					*(u32 *)addr = write;
					KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS);
				}
			}
			break;

		case KAMI_CDC_GO_DSMODE:
			{
				CDC_Init();	// IIRなどのパラメータ初期化のため
				CDC_GoDsMode();
	            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS);
			}
			break;

		case KAMI_CLEAR_NAND_ERRORLOG:
			{
				if (sdmcFormatNandLog(TRUE))
				{
		            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS);
				}
				else
				{
		            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_SUCCESS_FALSE);
				}
			}
			break;

        case KAMI_GET_CAMERA_MODULE_TYPE:
            {
                CameraModuleTypes types;
                if (CAMERAi_IsSharpModule(CAMERA_SELECT_IN))
                {
                    types.in = CAMERA_MODULE_TYPE_SHARP;
                }
                else if (CAMERAi_IsMicronModule(CAMERA_SELECT_IN))
                {
                    types.in = CAMERA_MODULE_TYPE_MICRON;
                }
                else
                {
                    types.in = CAMERA_MODULE_TYPE_UNKNOWN;
                }
                if (CAMERAi_IsSharpModule(CAMERA_SELECT_OUT))
                {
                    types.out = CAMERA_MODULE_TYPE_SHARP;
                }
                else if (CAMERAi_IsMicronModule(CAMERA_SELECT_OUT))
                {
                    types.out = CAMERA_MODULE_TYPE_MICRON;
                }
                else
                {
                    types.out = CAMERA_MODULE_TYPE_UNKNOWN;
                }
                KamiReturnResultEx(kamiWork.command, KAMI_PXI_RESULT_SUCCESS, sizeof(CameraModuleTypes), (u8*)&types);
            }
            break;

		case KAMI_GET_NAND_CID:
			{
				u8 buffer[16];
				sdmcGetCID( SDMC_PORT_NAND, (u32*)buffer);
	            KamiReturnResultEx(kamiWork.command, KAMI_PXI_RESULT_SUCCESS, sizeof(buffer), (u8*)buffer );
			}
			break;

        default:
            KamiReturnResult(kamiWork.command, KAMI_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

#include <twl/ltdmain_end.h>
