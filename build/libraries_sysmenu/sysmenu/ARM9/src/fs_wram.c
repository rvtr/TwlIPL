/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - fs
  File:     fs_wram.c

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
#include "fs_wram.h"
#include <sysmenu.h>
/*
    ��{�T�O

    ARM9���X���b�h����ƂȂ��ł͍\�����S���ς�邪�ACARD������Ă���̂ŃX���b�h�O��͂��肩��
    (PXI���񓯊��Ŕ�������_�ɒ���)

    ��Read���̗���
    ARM9 (API)                          ARM7 (TASK)
    WRAM�����ׂ�ARM7���ɂ���
    �J�n�v����                          (�t�@�C���V�[�N)
                                        �t�@�C������WRAM��1�u���b�N�R�s�[
                                        ���f�[�^��������
                                        �t�@�C������WRAM��1�u���b�N�R�s�[
    �Y���u���b�N��ARM9���ɂ���          ���f�[�^��������
    (�R�[���o�b�N�Ăяo��)
    WRAM����dst�ɃR�s�[
    �Y���u���b�N��ARM7���ɂ���
    ������ARM7�ɒʒm��                  �t�@�C������WRAM��1�u���b�N�R�s�[
                                        ���f�[�^��������
    �Y���u���b�N��ARM9���ɂ���          WRAM������Ȃ��Ȃ�����X�g�[��
    (�R�[���o�b�N�Ăяo��)
    WRAM����dst�ɃR�s�[
    �Y���u���b�N��ARM7���ɂ���
    ������ARM7�ɒʒm��                  �t�@�C������WRAM��1�u���b�N�R�s�[
                                        ���f�[�^��������
    �Y���u���b�N��ARM9���ɂ���          �S���R�s�[���I���Ί���
    (�R�[���o�b�N�Ăяo��)
    WRAM����dst�ɃR�s�[
    �Y���u���b�N��ARM7���ɂ���
    ������ARM7�ɒʒm��                  �������邱�Ƃ��Ȃ��̂Ŗ���
    �S���R�s�[���I���Ί���

    ��Write���̗���
    ARM9 (API)                          ARM7 (TASK)
    �J�n�\����                          (�t�@�C���V�[�N)
    �Y���u���b�N��ARM9���ɂ���
    src����WRAM��1�u���b�N�R�s�[
    (�R�[���o�b�N�Ăяo��)
    �Y���u���b�N��ARM7���ɂ���
    �f�[�^����������                    �Y���u���b�N���t�@�C���ɏ�������
                                        ���f�[�^�]������
    �Y���u���b�N��ARM9���ɂ���
    src����WRAM��1�u���b�N�R�s�[
    (�R�[���o�b�N�Ăяo��)
    �Y���u���b�N��ARM7���ɂ���
    �f�[�^����������
    �Y���u���b�N��ARM9���ɂ���
    src����WRAM��1�u���b�N�R�s�[
    (�R�[���o�b�N�Ăяo��)
    �Y���u���b�N��ARM7���ɂ���
    �f�[�^����������
    WRAM������Ȃ��Ȃ�����X�g�[��      �Y���u���b�N���t�@�C���ɏ�������
                                        ���f�[�^�]������
    �Y���u���b�N��ARM9���ɂ���
    src����WRAM��1�u���b�N�R�s�[
    (�R�[���o�b�N�Ăяo��)
    �Y���u���b�N��ARM7���ɂ���
    �f�[�^����������
    �S���R�s�[���I���Ί����҂�        �Y���u���b�N���t�@�C���ɏ�������
                                        ���f�[�^�]������
                                        �Y���u���b�N���t�@�C���ɏ�������
                                        ���f�[�^�]������
                                        �S���R�s�[���I���Ί���
    ����

    ������̕��j
    CARD�A�������t�@�C�����Ή��H �� FS��ARM7���ŏ�������API��p�ӂ��邩����

    AES��FSi_IsValidAddressForARM7�̂悤�Ȃ��̂ŃA�N�Z�X�\�ɂł���΁A
    �R�[���o�b�N���ňꎞ�I��ARM7�ɓn���ĈÍ��������{���ĕԂ��Ă��炤��
    �������Ƃ��\ (�����ĂȂ���AES��dest==src���\����)
*/

/*
    �R�[���o�b�N���Ăяo���P��
    READ:  WRAM�T�C�Y128KB�A�R�[���o�b�N�P��512B�ŏ\�� (�������Ă����Ȃ�)
    WRITE: WRAM�T�C�Y256KB�A�R�[���o�b�N�P��1KB�͗~����(�������Ă����Ȃ�)
*/
#define FS_WRAM_CALLBACK_UNIT       1024

#define FS_WRAM_THREAD_STACK_SIZE   512

// �ȉ��A�Œ�l
#define FS_WRAM_MESSAGE_ARRAY_MAX   8
#define FS_WRAM_SLOT_SIZE           (32*1024)

typedef enum FSWramCommand
{
    FS_WRAM_COMMAND_READ,
    FS_WRAM_COMMAND_WRITE,
    FS_WRAM_COMMAND_READ_CARD
}
FSWramCommand;

typedef struct FSWramCommandParam
{
    void*           addr;
    int             length;
}
FSWramCommandParam;

typedef struct FSWramWork
{
    u64             stack[FS_WRAM_THREAD_STACK_SIZE/sizeof(u64)];
    OSThread        thread;

    OSMessageQueue  msgQ4task;
    OSMessage       msgArray4task[FS_WRAM_MESSAGE_ARRAY_MAX];
    OSMessageQueue  msgQ4api;
    OSMessage       msgArray4api[FS_WRAM_MESSAGE_ARRAY_MAX];

    FSWramCommandParam param[FS_WRAM_MESSAGE_ARRAY_MAX];

    BOOL            busy;

    FSWramCommand   command;
    FSFile          *p_file;
    void            *card_src;

    int             nums;   // WRAM�X���b�g��
    u32             base;   // WRAM�̐擪�A�h���X
}
FSWramWork;

static FSWramWork FSiWramWork;

/*
// override
BOOL FSi_IsValidAddressForARM7(const void *buffer, u32 length);
BOOL FSi_IsValidAddressForARM7(const void *buffer, u32 length)
{
    u32     addr = (u32)buffer;
    u32     dtcm = OS_GetDTCMAddress();
    if ((addr + length > dtcm) && (addr < dtcm + HW_DTCM_SIZE))
    {
        return FALSE;
    }
    if ((addr >= HW_TWL_MAIN_MEM) && (addr + length <= HW_TWL_MAIN_MEM_END))
    {
        return TRUE;
    }
    if ((addr >= HW_EXT_WRAM_ARM7) && (addr + length <= GX_GetSizeOfARM7()))
    {
        return TRUE;
    }
    if (FSiWramWork.busy)
    {
        if ((addr >= FSiWramWork.base) && (addr + length <= FSiWramWork.base + FSiWramWork.nums * FS_WRAM_SLOT_SIZE))
        {
            return TRUE;
        }
    }
    return FALSE;
}
*/

static void FSi_WramThread(void* arg)
{
#pragma unused(arg)
    while (1)
    {
        FSWramCommandParam  *cmd;
        int                 result;
        OS_ReceiveMessage(&FSiWramWork.msgQ4task, (OSMessage*)&cmd, OS_MESSAGE_BLOCK);
        switch (FSiWramWork.command)
        {
        case FS_WRAM_COMMAND_READ:
            result = FS_ReadFile( FSiWramWork.p_file, cmd->addr, cmd->length );
//OS_TPrintf("%s:   (0x%08X)             %d msec\n", __func__, cmd->addr, (int)OS_TicksToMilliSeconds(OS_GetTick()));
            break;
        case FS_WRAM_COMMAND_READ_CARD:
            // �J�[�h���[�h
            if(!FSi_IsValidAddressForARM7(cmd->addr, (u32)cmd->length))
            {
				result = 0;
				break;
			}
            HOTSW_ReadCardData( FSiWramWork.card_src, cmd->addr, (u32)cmd->length);
            FSiWramWork.card_src = (void *)((u32)FSiWramWork.card_src + (u32)cmd->length);
            result = cmd->length;//�b��
            break;
        case FS_WRAM_COMMAND_WRITE:
            result = FS_WriteFile( FSiWramWork.p_file, cmd->addr, cmd->length );
//OS_TPrintf("%s:                (0x%08X) %d msec\n", __func__, cmd->addr, (int)OS_TicksToMilliSeconds(OS_GetTick()));
            break;
        default:
            result = -1;
        }
        if ( cmd->length != result )
        {
            OS_TWarning("%s: Failed to %s file (%d <=> %d).\n", __func__, FSiWramWork.command==FS_WRAM_COMMAND_READ?"read":"write", cmd->length, result);
        }
        OS_SendMessage(&FSiWramWork.msgQ4api, (OSMessage)(result==cmd->length?TRUE:FALSE), OS_MESSAGE_BLOCK);
    }
}

BOOL FS_InitWramTransfer( u32 priority )
{
    static BOOL initialized = FALSE;
    if (initialized)
    {
        return FALSE;
    }
    OS_InitMessageQueue(&FSiWramWork.msgQ4task, FSiWramWork.msgArray4task, FS_WRAM_MESSAGE_ARRAY_MAX);
    OS_InitMessageQueue(&FSiWramWork.msgQ4api, FSiWramWork.msgArray4api, FS_WRAM_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&FSiWramWork.thread, FSi_WramThread, 0,
                    (void*)(FSiWramWork.stack + (FS_WRAM_THREAD_STACK_SIZE/sizeof(u64))),
                    FS_WRAM_THREAD_STACK_SIZE, priority);
    OS_WakeupThreadDirect(&FSiWramWork.thread);
	initialized = TRUE;
    return TRUE;
}

static int FSi_Increment( int p )
{
    return (p + 1) % FSiWramWork.nums;
}

static int FSi_GetStackCount( int call, int ret )
{
    return (call - ret + FSiWramWork.nums) % FSiWramWork.nums;
}

static BOOL FSi_ReadWram(u8* dest, u32 len, MIWramPos wram, s32 slot, FSWramCallback callback, void* arg)
{
    u32 size4api = len;
    u32 size4task = len;
    int call = 0;
    int ret = 0;

    while ( size4api > 0 )
    {
        const u8* src = (u8*)(FSiWramWork.base + ret * FS_WRAM_SLOT_SIZE);
        u32 unit = size4api < FS_WRAM_SLOT_SIZE ? size4api : FS_WRAM_SLOT_SIZE;
        BOOL result;

        // �ł��邾���N������
        for ( ; FSi_GetStackCount(call, ret) < FSiWramWork.nums - 1 && size4task > 0; call = FSi_Increment(call) )
        {
            FSiWramWork.param[call].addr = (void*)(FSiWramWork.base + call * FS_WRAM_SLOT_SIZE);
            FSiWramWork.param[call].length = (int)(size4task < FS_WRAM_SLOT_SIZE ? size4task : FS_WRAM_SLOT_SIZE);
            OS_SendMessage(&FSiWramWork.msgQ4task, (OSMessage)&FSiWramWork.param[call], OS_MESSAGE_BLOCK);
            size4task -= FSiWramWork.param[call].length;
        }

        // task�҂�
        OS_ReceiveMessage(&FSiWramWork.msgQ4api, (OSMessage*)&result, OS_MESSAGE_BLOCK);
        if (!result)
        {
            OS_TPrintf("%s: Failed to read file.\n", __func__);
            return FALSE;
        }
        MI_SwitchWramSlot( wram, slot + ret, MI_WRAM_SIZE_32KB, MI_WRAM_ARM7, MI_WRAM_ARM9 );
//OS_TPrintf("%s: %d,%d (0x%08X->0x%08X) %d msec\n", __func__, call, ret, src, dest, (int)OS_TicksToMilliSeconds(OS_GetTick()));
       if ( callback )
        {
            int done;
            for ( done = 0; done < unit; done += FS_WRAM_CALLBACK_UNIT )
            {
                const u8* s = src + done;
                u8* d = dest + done;
                u32 u = unit - done < FS_WRAM_CALLBACK_UNIT ? unit - done : FS_WRAM_CALLBACK_UNIT;
                callback(s, d, u, wram, slot + ret, arg);
                MI_CpuCopyFast( s, d, u );
            }
        }
        else
        {
            MI_CpuCopyFast( src, dest, unit );
        }
        DC_InvalidateRange( (void*)src, unit );
        //MI_SwitchWramSlot( wram, slot + ret, MI_WRAM_SIZE_32KB, MI_WRAM_ARM9, MI_WRAM_ARM7 );
        size4api -= unit;
        dest += unit;
        ret = FSi_Increment(ret);
    }
    return TRUE;
}

static BOOL FSi_WriteWram(const u8* src, u32 len, MIWramPos wram, s32 slot, FSWramCallback callback, void* arg)
{
    u32 size = len;
    int call = 0;
    int ret = 0;

    while ( size > 0 )
    {
        u8* dest = (u8*)(FSiWramWork.base + call * FS_WRAM_SLOT_SIZE);
        u32 unit = size < FS_WRAM_SLOT_SIZE ? size : FS_WRAM_SLOT_SIZE;
        BOOL result;

        // task�ς݉��
        while ( OS_ReceiveMessage(&FSiWramWork.msgQ4api, (OSMessage*)&result, OS_MESSAGE_NOBLOCK) )
        {
            if (!result)
            {
                OS_TPrintf("%s: Failed to read file.\n", __func__);
                return FALSE;
            }
            ret = FSi_Increment(ret);
        }
        // task����[����������҂�
        if ( FSi_GetStackCount( call, ret ) == FSiWramWork.nums - 1 )
        {
            OS_ReceiveMessage(&FSiWramWork.msgQ4api, (OSMessage*)&result, OS_MESSAGE_BLOCK);
            if (!result)
            {
                OS_TPrintf("%s: Failed to read file.\n", __func__);
                return FALSE;
            }
            ret = FSi_Increment(ret);
        }
        MI_SwitchWramSlot( wram, slot + call, MI_WRAM_SIZE_32KB, MI_WRAM_ARM7, MI_WRAM_ARM9 );
//OS_TPrintf("%s: %d,%d (0x%08X->0x%08X) %d msec\n", __func__, call, ret, src, dest, (int)OS_TicksToMilliSeconds(OS_GetTick()));
        if ( callback )
        {
            int done;
            for ( done = 0; done < unit; done += FS_WRAM_CALLBACK_UNIT )
            {
                const u8* s = src + done;
                u8* d = dest + done;
                u32 u = unit - done < FS_WRAM_CALLBACK_UNIT ? unit - done : FS_WRAM_CALLBACK_UNIT;
                MI_CpuCopyFast( s, d, u );
                callback(d, s, u, wram, slot + ret, arg);
            }
        }
        else
        {
            MI_CpuCopyFast( src, dest, unit );
        }
        DC_FlushRange( dest, unit );
        //MI_SwitchWramSlot( wram, slot + call, MI_WRAM_SIZE_32KB, MI_WRAM_ARM9, MI_WRAM_ARM7 );
        // 1�p�ӂł����̂ŋN������
        FSiWramWork.param[call].addr = (void*)(FSiWramWork.base + call * FS_WRAM_SLOT_SIZE);
        FSiWramWork.param[call].length = (int)unit;
        OS_SendMessage(&FSiWramWork.msgQ4task, (OSMessage)&FSiWramWork.param[call], OS_MESSAGE_BLOCK);
        size -= unit;
        src  += unit;
        dest += unit;
        call = FSi_Increment(call);
    }
    // �c��task���
    while ( FSi_GetStackCount(call, ret) )
    {
        BOOL result;
        OS_ReceiveMessage(&FSiWramWork.msgQ4api, (OSMessage*)&result, OS_MESSAGE_BLOCK);
        if (!result)
        {
            OS_TPrintf("%s: Failed to read file.\n", __func__);
            return FALSE;
        }
        ret = FSi_Increment(ret);
    }
    return TRUE;
}

BOOL HOTSW_ReadCardViaWram( void *src, void *dst, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg )
{
    OSIntrMode enabled = OS_DisableInterrupts();
    BOOL result;
    int l,n;
    SDK_ASSERT( wram != MI_WRAM_A );

    if ( FSiWramWork.busy ) // �]����
    {
        OS_RestoreInterrupts(enabled);
        return FALSE;
    }
    FSiWramWork.busy = TRUE;
    OS_RestoreInterrupts(enabled);


    // WRAM�̊m��
    FSiWramWork.base = MI_AllocWramSlot( wram, slot, size, MI_WRAM_ARM9 );
    if ( FSiWramWork.base == 0 )
    {
        FSiWramWork.busy = FALSE;
        OS_TPrintf("Cannot allocate WRAM %d, %d, %d\n", wram, slot, size);
        return FALSE;
    }
    // cash care
    DC_InvalidateRange((void *)FSiWramWork.base, (u32)(size << 15));


    // �p�����[�^�ݒ�
    FSiWramWork.command = FS_WRAM_COMMAND_READ_CARD;
    FSiWramWork.card_src  = src;
    FSiWramWork.nums    = MI_WRAM_ENUM_TO_SIZE( size ) * 1024 / FS_WRAM_SLOT_SIZE;
    
    // �K�v�ɉ�����7���ɃX�C�b�`�\��WRAM�Ƃ��Ďw��
    n = 0;
    for(l=0;l<FSiWramWork.nums;l++)
    {
		n = n << 1;
		n += (1 << slot);
	}
	if( wram == MI_WRAM_B )
	{
	    FSi_SetSwitchableWramSlots(n,0);
	}else if ( wram == MI_WRAM_C )
	{
	    FSi_SetSwitchableWramSlots(0,n);
	}
    
    // WRAM->ARM9�N��
    result = FSi_ReadWram(dst, (u32)len, wram, slot, callback, arg);

    MI_FreeWramSlot( wram, slot, size, MI_WRAM_ARM9 );
    FSi_SetSwitchableWramSlots(0,0); // �X�C�b�`�\WRAM�̎w������ɖ߂�
    FSiWramWork.busy = FALSE;
    return result;
}

BOOL FS_ReadFileViaWram( FSFile *p_file, void *dst, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg )
{
    OSIntrMode enabled = OS_DisableInterrupts();
    BOOL result;
    int l,n;
    SDK_ASSERT( wram != MI_WRAM_A );

    if ( FSiWramWork.busy ) // �]����
    {
        OS_RestoreInterrupts(enabled);
        return FALSE;
    }
    FSiWramWork.busy = TRUE;
    OS_RestoreInterrupts(enabled);


    // WRAM�̊m��
    FSiWramWork.base = MI_AllocWramSlot( wram, slot, size, MI_WRAM_ARM9 );
    if ( FSiWramWork.base == 0 )
    {
        FSiWramWork.busy = FALSE;
        OS_TPrintf("Cannot allocate WRAM %d, %d, %d\n", wram, slot, size);
        return FALSE;
    }
    // cash care
    DC_InvalidateRange((void *)FSiWramWork.base, (u32)(size << 15));


    // �p�����[�^�ݒ�
    FSiWramWork.command = FS_WRAM_COMMAND_READ;
    FSiWramWork.p_file  = p_file;
    FSiWramWork.nums    = MI_WRAM_ENUM_TO_SIZE( size ) * 1024 / FS_WRAM_SLOT_SIZE;
    
    // �K�v�ɉ�����7���ɃX�C�b�`�\��WRAM�Ƃ��Ďw��
    n = 0;
    for(l=0;l<FSiWramWork.nums;l++)
    {
		n = n << 1;
		n += (1 << slot);
	}
	if( wram == MI_WRAM_B )
	{
	    FSi_SetSwitchableWramSlots(n,0);
	}else if ( wram == MI_WRAM_C )
	{
	    FSi_SetSwitchableWramSlots(0,n);
	}
    
    // WRAM->ARM9�N��
    result = FSi_ReadWram(dst, (u32)len, wram, slot, callback, arg);

    MI_FreeWramSlot( wram, slot, size, MI_WRAM_ARM9 );
    FSi_SetSwitchableWramSlots(0,0); // �X�C�b�`�\WRAM�̎w������ɖ߂�
    FSiWramWork.busy = FALSE;
    return result;
}

BOOL FS_WriteFileViaWram( FSFile *p_file, const void *src, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg )
{
    OSIntrMode enabled = OS_DisableInterrupts();
    BOOL result;
    int l,n;
    SDK_ASSERT( wram != MI_WRAM_A );

    if ( FSiWramWork.busy ) // �]����
    {
        OS_RestoreInterrupts(enabled);
        return FALSE;
    }
    FSiWramWork.busy = TRUE;
    OS_RestoreInterrupts(enabled);


    // WRAM�̊m�� 
    FSiWramWork.base = MI_AllocWramSlot( wram, slot, size, MI_WRAM_ARM7 );
    if ( FSiWramWork.base == 0 )
    {
        FSiWramWork.busy = FALSE;
        OS_TPrintf("Cannot allocate WRAM %d, %d, %d\n", wram, slot, size);
        return FALSE;
    }

    // �p�����[�^�ݒ�
    FSiWramWork.command = FS_WRAM_COMMAND_WRITE;
    FSiWramWork.p_file  = p_file;
    FSiWramWork.nums    = MI_WRAM_ENUM_TO_SIZE( size ) * 1024 / FS_WRAM_SLOT_SIZE;

    // �K�v�ɉ�����7���ɃX�C�b�`�\��WRAM�Ƃ��Ďw��
    n = 0;
    for(l=0;l<FSiWramWork.nums;l++)
    {
		n = n << 1;
		n += (1 << slot);
	}
	if( wram == MI_WRAM_B )
	{
	    FSi_SetSwitchableWramSlots(n,0);
	}else if ( wram == MI_WRAM_C )
	{
	    FSi_SetSwitchableWramSlots(0,n);
	}

    // ARM9->WRAM�N��
    result = FSi_WriteWram(src, (u32)len, wram, slot, callback, arg);

    MI_FreeWramSlot( wram, slot, size, MI_WRAM_ARM7 );
    FSi_SetSwitchableWramSlots(0,0); // �X�C�b�`�\WRAM�̎w������ɖ߂�
    FSiWramWork.busy = FALSE;
    return result;
}
