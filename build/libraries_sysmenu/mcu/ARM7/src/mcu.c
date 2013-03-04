/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mcu.c

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
#include <sysmenu/mcu.h>

#include <twl/ltdwram_begin.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define MCUTEST_MESSAGE_ARRAY_MAX       4        // �X���b�h�����p���b�Z�[�W�L���[�̃T�C�Y
#define MCUTEST_THREAD_STACK_SIZE       512      // �X���b�h�̃X�^�b�N�T�C�Y

#define MCUTEST_THREAD_PRIORITY          6

// �A���C�����g�������ăR�s�[����
#define MCU_UNPACK_U16(d, s)    \
    (*(d) = (u16)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8)))
#define MCU_UNPACK_U32(d, s)    \
    (*(d) = (u32)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8) | (((u8*)s)[2] << 16) | (((u8*)s)[3] << 24)))

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/
typedef struct SYSMMcuWork
{
    u32  total;
    u32  current;
    SYSMMcuCommand  command;
    u8 data[MCUTEST_PXI_DATA_SIZE_MAX];   // �㑱�f�[�^�i�[�p

    OSMessageQueue msgQ;               // �X���b�h�����p���b�Z�[�W�L���[
    OSMessage msgArray[MCUTEST_MESSAGE_ARRAY_MAX];
    // ���b�Z�[�W���i�[����o�b�t�@
    OSThread thread;                   // MCU�p�X���b�h
    u64     stack[MCUTEST_THREAD_STACK_SIZE / sizeof(u64)];
    // MCU�p�X���b�h�̃X�^�b�N
}
SYSMMcuWork;

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
 *---------------------------------------------------------------------------*/
static BOOL mcuInitialized;
static SYSMMcuWork mcuWork;
static u8 mcu_ver;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void SYSM_McuPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void SYSM_ReturnMcuResult(SYSMMcuCommand command, SYSMMcuPxiResult result);
static void SYSM_ReturnMcuResultEx(SYSMMcuCommand command, SYSMMcuPxiResult result, u8 size, u8* data);
static void SYSM_McuThread(void *arg);
#if 0
static void SYSM_McuHandlerExternalDC(void)
{
OS_TPrintf("MCU_IE_EXTERNAL_DC_TRIGGER\n");
    SYSM_ReturnMcuResult(MCU_TEST_COMMAND_INTERRUPT, (SYSMMcuPxiResult)MCU_IE_EXTERNAL_DC_TRIGGER);
}
#endif
static void SYSM_McuHandlerBatteryLow(void)
{
OS_TPrintf("MCU_IE_BATTERY_LOW_TRIGGER\n");
    SYSM_ReturnMcuResult(MCU_TEST_COMMAND_INTERRUPT, (SYSMMcuPxiResult)MCU_IE_BATTERY_LOW_TRIGGER);
}

static void SYSM_McuHandlerBatteryEmpty(void)
{
OS_TPrintf("MCU_IE_BATTERY_EMPTY_TRIGGER\n");
    SYSM_ReturnMcuResult(MCU_TEST_COMMAND_INTERRUPT, (SYSMMcuPxiResult)MCU_IE_BATTERY_EMPTY_TRIGGER);
}

static void SYSM_McuHandlerPowerSwitch(void)
{
OS_TPrintf("MCU_IE_POWER_SWITCH_PRESSED\n");
    SYSM_ReturnMcuResult(MCU_TEST_COMMAND_INTERRUPT, (SYSMMcuPxiResult)MCU_IE_POWER_SWITCH_PRESSED);
}

static void SYSM_McuHandlerPowerOffRequest(void)
{
OS_TPrintf("MCU_IE_POWER_OFF_REQUEST\n");
    SYSM_ReturnMcuResult(MCU_TEST_COMMAND_INTERRUPT, (SYSMMcuPxiResult)MCU_IE_POWER_OFF_REQUEST);
}

static void SYSM_McuHandlerResetRequest(void)
{
OS_TPrintf("MCU_IE_RESET_REQUEST\n");
    SYSM_ReturnMcuResult(MCU_TEST_COMMAND_INTERRUPT, (SYSMMcuPxiResult)MCU_IE_RESET_REQUEST);
}

#ifdef SDK_SUPPORT_PMIC_2
// �}�C�R���o�[�W�����擾
u8 SYSMi_GetMcuVersion( void )
{
	return (u8)(mcu_ver >> MCU_REG_VER_INFO_VERSION_SHIFT);
}
#endif // SDK_SUPPORT_PMIC_2

// ������
void SYSM_InitMcuPxi( u32 prio )
{
    if (mcuInitialized)
    {
        return;
    }
    mcuInitialized = TRUE;

    OS_InitMessageQueue(&mcuWork.msgQ, mcuWork.msgArray, MCUTEST_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&mcuWork.thread, SYSM_McuThread, 0,
                    (void *)(mcuWork.stack + (MCUTEST_THREAD_STACK_SIZE / sizeof(u64))),
                    MCUTEST_THREAD_STACK_SIZE, prio);
    OS_WakeupThreadDirect(&mcuWork.thread);

    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_MCUTEST, SYSM_McuPxiCallback);

#ifdef SDK_SUPPORT_PMIC_2
    // �}�C�R���o�[�W�����擾
    mcu_ver = (u8)(MCU_ReadRegister( MCU_REG_VER_INFO_ADDR ));
#endif // SDK_SUPPORT_PMIC_2

#if 0
#if 0
    MCU_SetIrqFunction(MCU_IE_EXTERNAL_DC_TRIGGER, McuHandlerExternalDC);
#endif
    MCU_SetIrqFunction(MCU_IE_BATTERY_LOW_TRIGGER, McuHandlerBatteryLow);
    MCU_SetIrqFunction(MCU_IE_BATTERY_EMPTY_TRIGGER, McuHandlerBatteryEmpty);
    MCU_SetIrqFunction(MCU_IE_POWER_SWITCH_PRESSED, McuHandlerPowerSwitch);
    MCU_SetIrqFunction(MCU_IE_POWER_OFF_REQUEST, McuHandlerPowerOffRequest);
    MCU_SetIrqFunction(MCU_IE_RESET_REQUEST, McuHandlerResetRequest);

{
    OSTick tick = OS_GetTick();
    MCU_ReadRegister(0x70);
    OS_TPrintf("Read: %d usec\n", (u32)OS_TicksToMicroSeconds(OS_GetTick()-tick));
    tick = OS_GetTick();
    MCU_WriteRegister(0x70, 0);
    OS_TPrintf("Write: %d usec\n", (u32)OS_TicksToMicroSeconds(OS_GetTick()-tick));
}

    // PMIC�ւ̓d���{�^�����荞�ݍ폜 (�����)
    OS_DisableIrqMaskEx(OS_IE_GPIO33_0);
    // (��)������
    MCU_InitIrq(1);
#endif
}

static void SYSM_McuPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    if (err)
    {
        return;
    }
    if (data & MCUTEST_PXI_START_BIT)   // �擪�f�[�^
    {
        mcuWork.total = (u8)((data & MCUTEST_PXI_DATA_NUMS_MASK) >> MCUTEST_PXI_DATA_NUMS_SHIFT);
        mcuWork.current = 0;
        mcuWork.command = (SYSMMcuCommand)((data & MCUTEST_PXI_COMMAND_MASK) >> MCUTEST_PXI_COMMAND_SHIFT);
        mcuWork.data[mcuWork.current++] = (u8)((data & MCUTEST_PXI_1ST_DATA_MASK) >> MCUTEST_PXI_1ST_DATA_SHIFT);
    }
    else if (mcuWork.command)    // �㑱�f�[�^
    {
        mcuWork.data[mcuWork.current++] = (u8)((data & 0xFF0000) >> 16);
        mcuWork.data[mcuWork.current++] = (u8)((data & 0x00FF00) >> 8);
        mcuWork.data[mcuWork.current++] = (u8)((data & 0x0000FF) >> 0);
    }
    if (mcuWork.current >= mcuWork.total)
    {
        switch (mcuWork.command)
        {
        case MCU_TEST_COMMAND_READ_REGISTER:
        case MCU_TEST_COMMAND_WRITE_REGISTER:
            if (!OS_SendMessage(&mcuWork.msgQ, NULL, OS_MESSAGE_NOBLOCK))
            {
                SYSM_ReturnMcuResult(mcuWork.command, MCU_PXI_RESULT_FATAL_ERROR);
            }
            break;

        default:
            SYSM_ReturnMcuResult(mcuWork.command, MCU_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

static void SYSM_ReturnMcuResult(SYSMMcuCommand command, SYSMMcuPxiResult result)
{
    u32 pxiData = (u32)(MCUTEST_PXI_START_BIT | MCUTEST_PXI_RESULT_BIT |
            ((command << MCUTEST_PXI_COMMAND_SHIFT) & MCUTEST_PXI_COMMAND_MASK) |
            ((1 << MCUTEST_PXI_DATA_NUMS_SHIFT) & MCUTEST_PXI_DATA_NUMS_MASK) |
            ((result << MCUTEST_PXI_1ST_DATA_SHIFT) & MCUTEST_PXI_1ST_DATA_MASK));
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MCUTEST, pxiData, 0))
    {
    }
}

static void SYSM_ReturnMcuResultEx(SYSMMcuCommand command, SYSMMcuPxiResult result, u8 size, u8* data)
{
    u32 pxiData = (u32)(MCUTEST_PXI_START_BIT | MCUTEST_PXI_RESULT_BIT |
            ((command << MCUTEST_PXI_COMMAND_SHIFT) & MCUTEST_PXI_COMMAND_MASK) |
            (((size+1) << MCUTEST_PXI_DATA_NUMS_SHIFT) & MCUTEST_PXI_DATA_NUMS_MASK) |
            ((result << MCUTEST_PXI_1ST_DATA_SHIFT) & MCUTEST_PXI_1ST_DATA_MASK));
    OSIntrMode enabled = OS_DisableInterrupts();
    int i;
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MCUTEST, pxiData, 0))
    {
    }
    for (i = 0; i < size; i+= 3)
    {
        pxiData = (u32)((data[i] << 16) | (data[i+1] << 8) | data[i+2]);
        while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MCUTEST, pxiData, 0))
        {
        }
    }
    OS_RestoreInterrupts(enabled);
}

static void SYSM_McuThread(void *arg)
{
#pragma unused( arg )
    OSMessage msg;
    u8 value;

    while (TRUE)
    {
        (void)OS_ReceiveMessage(&mcuWork.msgQ, &msg, OS_MESSAGE_BLOCK);
        switch (mcuWork.command)
        {
        case MCU_TEST_COMMAND_READ_REGISTER:
            value = MCU_ReadRegister(mcuWork.data[0]);
            SYSM_ReturnMcuResultEx(mcuWork.command, MCU_PXI_RESULT_SUCCESS, 1, &value);
            break;

        case MCU_TEST_COMMAND_WRITE_REGISTER:
            value = (u8)MCU_WriteRegister(mcuWork.data[0], mcuWork.data[1]);
            SYSM_ReturnMcuResult(mcuWork.command, value ? MCU_PXI_RESULT_SUCCESS : MCU_PXI_RESULT_ILLEGAL_STATUS);
            break;

        default:
            SYSM_ReturnMcuResult(mcuWork.command, MCU_PXI_RESULT_INVALID_COMMAND);
        }
        mcuWork.command = MCU_TEST_COMMAND_NULL;
    }
}

#include <twl/ltdwram_end.h>
