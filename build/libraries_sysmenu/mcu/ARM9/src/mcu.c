/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mcu.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-01-29#$
  $Rev: 3905 $
  $Author: yutaka $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <sysmenu/mcu.h>

#define DMA_WILL_STOP   // MCU_GetMaxLinesRoundならdefine、MCU_GET_MAX_LINESならundef

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// 詰めてコピーする
#define MCU_PACK_U16(d, s)                   \
    ((d)[0] = (u8)((*((u16*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u16*)s) >>  8) & 0xFF))

#define MCU_PACK_U32(d, s)                   \
    ((d)[0] = (u8)((*((u32*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u32*)s) >>  8) & 0xFF),  \
     (d)[2] = (u8)((*((u32*)s) >> 16) & 0xFF),  \
     (d)[3] = (u8)((*((u32*)s) >> 24) & 0xFF))


/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct SYSMMcuWork
{
    BOOL lock;

    SYSMMcuCommand  command;
    SYSMMcuPxiResult result;
    SYSMMcuCallback  callback;
    SYSMMcuCallback  handler;
    void*   arg;

    u32 total;
    u32 current;
    u8* data;
}
SYSMMcuWork;

#include <twl/ltdmain_begin.h>

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static BOOL mcuInitialized;
static SYSMMcuWork mcuWork;
static u8 mcu_ver;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL SYSM_SendMcuPxiCommand(SYSMMcuCommand command, u8 size, u8 data);
static void SYSM_SendMcuPxiData(u8 *pData);
static void SYSM_McuPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void SYSM_DoneMcu(SYSMMcuResult result);
static void SYSM_WaitMcuBusy(void);


// 初期化
void SYSM_InitMcuPxi( void )
{
    if (mcuInitialized)
    {
        return;
    }
    mcuInitialized = TRUE;
    mcuWork.lock = TRUE;
    mcuWork.handler = NULL;

    PXI_Init();
    while ( !PXI_IsCallbackReady(PXI_FIFO_TAG_MCUTEST, PXI_PROC_ARM7 ))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_MCUTEST, SYSM_McuPxiCallback);

    mcuWork.lock = TRUE;
    mcuWork.callback = NULL;
    if ( 0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MCUTEST, 0, 0))
    {
        return;
    }
    while (*(vu32*)&mcuWork.lock)
    {
    }

#ifdef SDK_SUPPORT_PMIC_2
    // マイコンバージョン取得
	while ( SYSM_ReadMcuRegisterAsync( MCU_REG_VER_INFO_ADDR, &mcu_ver, NULL, NULL ) != MCU_RESULT_SUCCESS ) {}
#endif // SDK_SUPPORT_PMIC_2
}

#ifdef SDK_SUPPORT_PMIC_2
// マイコンバージョン取得
u8 SYSMi_GetMcuVersion( void )
{
	return (u8)(mcu_ver >> MCU_REG_VER_INFO_VERSION_SHIFT);
}
#endif // SDK_SUPPORT_PMIC_2

SYSMMcuResult SYSM_ReadMcuRegisterAsync( u8 addr, u8 *pValue, SYSMMcuCallback callback, void* arg )
{
    OSIntrMode enabled = OS_DisableInterrupts();
    if (mcuWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MCU_RESULT_BUSY;
    }
    mcuWork.lock = TRUE;
    mcuWork.callback = callback;
    mcuWork.arg = arg;
    mcuWork.data = pValue;
    (void)OS_RestoreInterrupts(enabled);

    if (SYSM_SendMcuPxiCommand(MCU_TEST_COMMAND_READ_REGISTER, 1, (u8)addr))
    {
        return MCU_RESULT_SUCCESS;
    }
    return MCU_RESULT_SEND_ERROR;
}

SYSMMcuResult SYSM_WriteMcuRegisterAsync( u8 addr, u8 value, SYSMMcuCallback callback, void* arg )
{
    OSIntrMode enabled;
    u8 data[3];

    enabled = OS_DisableInterrupts();
    if (mcuWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MCU_RESULT_BUSY;
    }
    mcuWork.lock = TRUE;
    mcuWork.callback = callback;
    mcuWork.arg = arg;
    mcuWork.data = NULL;
    data[0] = value;
    (void)OS_RestoreInterrupts(enabled);

    if (SYSM_SendMcuPxiCommand(MCU_TEST_COMMAND_WRITE_REGISTER, 2, (u8)addr))
    {
        SYSM_SendMcuPxiData(data);
        return MCU_RESULT_SUCCESS;
    }
    return MCU_RESULT_SEND_ERROR;
}

void SYSM_SetMcuInterruptHandler( SYSMMcuCallback handler )
{
    mcuWork.handler = handler;
}

/////////
static BOOL SYSM_SendMcuPxiCommand(SYSMMcuCommand command, u8 size, u8 data)
{
    u32 pxiData = (u32)(MCUTEST_PXI_START_BIT |
            ((command << MCUTEST_PXI_COMMAND_SHIFT) & MCUTEST_PXI_COMMAND_MASK) |
            ((size << MCUTEST_PXI_DATA_NUMS_SHIFT) & MCUTEST_PXI_DATA_NUMS_MASK) |
            ((data << MCUTEST_PXI_1ST_DATA_SHIFT) & MCUTEST_PXI_1ST_DATA_MASK));
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MCUTEST, pxiData, 0))
    {
        return FALSE;
    }
    return TRUE;
}

static void SYSM_SendMcuPxiData(u8 *pData)
{
    u32 pxiData = (u32)((pData[0] << 16) | (pData[1] << 8) | pData[2]);
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MCUTEST, pxiData, 0))
    {
    }
}

static void SYSM_McuPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    if (err)
    {
        SYSM_DoneMcu(MCU_RESULT_FATAL_ERROR);
        return;
    }
    if (data & MCUTEST_PXI_START_BIT)   // 先頭データ
    {
        if (data & MCUTEST_PXI_RESULT_BIT)
        {
            mcuWork.total = (u8)((data & MCUTEST_PXI_DATA_NUMS_MASK) >> MCUTEST_PXI_DATA_NUMS_SHIFT);
            mcuWork.current = 0;
            mcuWork.command = (SYSMMcuCommand)((data & MCUTEST_PXI_COMMAND_MASK) >> MCUTEST_PXI_COMMAND_SHIFT);
            mcuWork.result = (SYSMMcuPxiResult)((data & MCUTEST_PXI_1ST_DATA_MASK) >> MCUTEST_PXI_1ST_DATA_SHIFT);
        }
        else    // 未知のデータ
        {
            SYSM_DoneMcu(MCU_RESULT_FATAL_ERROR);
            return;
        }
    }
    else if (mcuWork.command)   // 後続データ
    {
        if (mcuWork.data == NULL)
        {
            SYSM_DoneMcu(MCU_RESULT_FATAL_ERROR);
            return;
        }
        if (mcuWork.current < mcuWork.total-1)
        {
            mcuWork.data[mcuWork.current++] = (u8)((data & 0xFF0000) >> 16);
        }
        if (mcuWork.current < mcuWork.total-1)
        {
            mcuWork.data[mcuWork.current++] = (u8)((data & 0x00FF00) >> 8);
        }
        if (mcuWork.current < mcuWork.total-1)
        {
            mcuWork.data[mcuWork.current++] = (u8)((data & 0x0000FF) >> 0);
        }
    }

    if (mcuWork.command == MCU_TEST_COMMAND_NULL)
    {
        SYSM_DoneMcu(MCU_RESULT_SUCCESS);
        return;
    }

    if (mcuWork.command == MCU_TEST_COMMAND_INTERRUPT)
    {
        if (mcuWork.handler)
        {
            mcuWork.handler(MCU_RESULT_SUCCESS, (void*)mcuWork.result);
        }
        return;
    }

    if (mcuWork.current == mcuWork.total-1)
    {
        SYSMMcuResult result;
        switch (mcuWork.result)
        {
        case MCU_PXI_RESULT_SUCCESS:     // alias MCU_PXI_RESULT_SUCCESS_TRUE
            result = MCU_RESULT_SUCCESS; // alias MCU_RESULT_SUCCESS_TRUE
            break;
        case MCU_PXI_RESULT_SUCCESS_FALSE:
            result = MCU_RESULT_SUCCESS_FALSE;
            break;
        case MCU_PXI_RESULT_INVALID_COMMAND:
            result = MCU_RESULT_INVALID_COMMAND;
            break;
        case MCU_PXI_RESULT_INVALID_PARAMETER:
            result = MCU_RESULT_ILLEGAL_PARAMETER;
            break;
        case MCU_PXI_RESULT_ILLEGAL_STATUS:
            result = MCU_RESULT_ILLEGAL_STATUS;
            break;
        case MCU_PXI_RESULT_BUSY:
            result = MCU_RESULT_BUSY;
            break;
        default:
            result = MCU_RESULT_FATAL_ERROR;
        }
        SYSM_DoneMcu(result);
        mcuWork.command = MCU_TEST_COMMAND_NULL;
        return;
    }
}

static void SYSM_DoneMcu(SYSMMcuResult result)
{
    SYSMMcuCallback callback = mcuWork.callback;
    void* arg = mcuWork.arg;
    mcuWork.callback = NULL;
    mcuWork.arg = NULL;
    if (mcuWork.lock)
    {
        mcuWork.lock = FALSE;
    }
    if (callback)
    {
        callback(result, arg);
    }
}

extern void PXIi_HandlerRecvFifoNotEmpty(void);
static void SYSM_WaitMcuBusy(void)
{
    volatile BOOL *p = &mcuWork.lock;

    while (*p)
    {
        if (OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE)
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}

#include <twl/ltdmain_end.h>
