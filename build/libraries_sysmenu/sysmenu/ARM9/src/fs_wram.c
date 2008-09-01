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
    基本概念

    ARM9側スレッドありとなしでは構造が全く変わるが、CARDもやっているのでスレッド前提はありかと
    (PXIが非同期で発生する点に注意)

    ●Read時の流れ
    ARM9 (API)                          ARM7 (TASK)
    WRAMをすべてARM7側にする
    開始要求→                          (ファイルシーク)
                                        ファイルからWRAMに1ブロックコピー
                                        ←データ準備完了
                                        ファイルからWRAMに1ブロックコピー
    該当ブロックをARM9側にする          ←データ準備完了
    (コールバック呼び出し)
    WRAMからdstにコピー
    該当ブロックをARM7側にする
    完了をARM7に通知→                  ファイルからWRAMに1ブロックコピー
                                        ←データ準備完了
    該当ブロックをARM9側にする          WRAMが足りなくなったらストール
    (コールバック呼び出し)
    WRAMからdstにコピー
    該当ブロックをARM7側にする
    完了をARM7に通知→                  ファイルからWRAMに1ブロックコピー
                                        ←データ準備完了
    該当ブロックをARM9側にする          全部コピーし終われば完了
    (コールバック呼び出し)
    WRAMからdstにコピー
    該当ブロックをARM7側にする
    完了をARM7に通知→                  何もすることがないので無視
    全部コピーし終われば完了

    ●Write時の流れ
    ARM9 (API)                          ARM7 (TASK)
    開始予告→                          (ファイルシーク)
    該当ブロックをARM9側にする
    srcからWRAMに1ブロックコピー
    (コールバック呼び出し)
    該当ブロックをARM7側にする
    データ準備完了→                    該当ブロックをファイルに書き込む
                                        ←データ転送完了
    該当ブロックをARM9側にする
    srcからWRAMに1ブロックコピー
    (コールバック呼び出し)
    該当ブロックをARM7側にする
    データ準備完了→
    該当ブロックをARM9側にする
    srcからWRAMに1ブロックコピー
    (コールバック呼び出し)
    該当ブロックをARM7側にする
    データ準備完了→
    WRAMが足りなくなったらストール      該当ブロックをファイルに書き込む
                                        ←データ転送完了
    該当ブロックをARM9側にする
    srcからWRAMに1ブロックコピー
    (コールバック呼び出し)
    該当ブロックをARM7側にする
    データ準備完了→
    全部コピーし終われば完了待ち        該当ブロックをファイルに書き込む
                                        ←データ転送完了
                                        該当ブロックをファイルに書き込む
                                        ←データ転送完了
                                        全部コピーし終われば完了
    完了

    ●今後の方針
    CARD、メモリファイルも対応？ → FSがARM7側で処理するAPIを用意するか次第

    AESもFSi_IsValidAddressForARM7のようなものでアクセス可能にできれば、
    コールバック内で一時的にARM7に渡して暗号処理を施して返してもらうと
    いうことも可能 (試してないがAESはdest==srcが可能かと)
*/

/*
    コールバックを呼び出す単位
    READ:  WRAMサイズ128KB、コールバック単位512Bで十分 (多すぎても問題ない)
    WRITE: WRAMサイズ256KB、コールバック単位1KBは欲しい(多すぎても問題ない)
*/
#define FS_WRAM_CALLBACK_UNIT       1024

#define FS_WRAM_THREAD_STACK_SIZE   512

// 以下、固定値
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

    int             nums;   // WRAMスロット数
    u32             base;   // WRAMの先頭アドレス
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
            // カードリード
            if(!FSi_IsValidAddressForARM7(cmd->addr, (u32)cmd->length))
            {
				result = 0;
				break;
			}
            HOTSW_ReadCardData( FSiWramWork.card_src, cmd->addr, (u32)cmd->length);
            FSiWramWork.card_src = (void *)((u32)FSiWramWork.card_src + (u32)cmd->length);
            result = cmd->length;//暫定
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

        // できるだけ起動する
        for ( ; FSi_GetStackCount(call, ret) < FSiWramWork.nums - 1 && size4task > 0; call = FSi_Increment(call) )
        {
            FSiWramWork.param[call].addr = (void*)(FSiWramWork.base + call * FS_WRAM_SLOT_SIZE);
            FSiWramWork.param[call].length = (int)(size4task < FS_WRAM_SLOT_SIZE ? size4task : FS_WRAM_SLOT_SIZE);
            OS_SendMessage(&FSiWramWork.msgQ4task, (OSMessage)&FSiWramWork.param[call], OS_MESSAGE_BLOCK);
            size4task -= FSiWramWork.param[call].length;
        }

        // task待ち
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

        // task済み回収
        while ( OS_ReceiveMessage(&FSiWramWork.msgQ4api, (OSMessage*)&result, OS_MESSAGE_NOBLOCK) )
        {
            if (!result)
            {
                OS_TPrintf("%s: Failed to read file.\n", __func__);
                return FALSE;
            }
            ret = FSi_Increment(ret);
        }
        // task回収ゼロだったら待つ
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
        // 1つ用意できたので起動する
        FSiWramWork.param[call].addr = (void*)(FSiWramWork.base + call * FS_WRAM_SLOT_SIZE);
        FSiWramWork.param[call].length = (int)unit;
        OS_SendMessage(&FSiWramWork.msgQ4task, (OSMessage)&FSiWramWork.param[call], OS_MESSAGE_BLOCK);
        size -= unit;
        src  += unit;
        dest += unit;
        call = FSi_Increment(call);
    }
    // 残りtask回収
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

    if ( FSiWramWork.busy ) // 転送中
    {
        OS_RestoreInterrupts(enabled);
        return FALSE;
    }
    FSiWramWork.busy = TRUE;
    OS_RestoreInterrupts(enabled);


    // WRAMの確保
    FSiWramWork.base = MI_AllocWramSlot( wram, slot, size, MI_WRAM_ARM9 );
    if ( FSiWramWork.base == 0 )
    {
        FSiWramWork.busy = FALSE;
        OS_TPrintf("Cannot allocate WRAM %d, %d, %d\n", wram, slot, size);
        return FALSE;
    }
    // cash care
    DC_InvalidateRange((void *)FSiWramWork.base, (u32)(size << 15));


    // パラメータ設定
    FSiWramWork.command = FS_WRAM_COMMAND_READ_CARD;
    FSiWramWork.card_src  = src;
    FSiWramWork.nums    = MI_WRAM_ENUM_TO_SIZE( size ) * 1024 / FS_WRAM_SLOT_SIZE;
    
    // 必要に応じて7側にスイッチ可能なWRAMとして指定
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
    
    // WRAM->ARM9起動
    result = FSi_ReadWram(dst, (u32)len, wram, slot, callback, arg);

    MI_FreeWramSlot( wram, slot, size, MI_WRAM_ARM9 );
    FSi_SetSwitchableWramSlots(0,0); // スイッチ可能WRAMの指定を元に戻す
    FSiWramWork.busy = FALSE;
    return result;
}

BOOL FS_ReadFileViaWram( FSFile *p_file, void *dst, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg )
{
    OSIntrMode enabled = OS_DisableInterrupts();
    BOOL result;
    int l,n;
    SDK_ASSERT( wram != MI_WRAM_A );

    if ( FSiWramWork.busy ) // 転送中
    {
        OS_RestoreInterrupts(enabled);
        return FALSE;
    }
    FSiWramWork.busy = TRUE;
    OS_RestoreInterrupts(enabled);


    // WRAMの確保
    FSiWramWork.base = MI_AllocWramSlot( wram, slot, size, MI_WRAM_ARM9 );
    if ( FSiWramWork.base == 0 )
    {
        FSiWramWork.busy = FALSE;
        OS_TPrintf("Cannot allocate WRAM %d, %d, %d\n", wram, slot, size);
        return FALSE;
    }
    // cash care
    DC_InvalidateRange((void *)FSiWramWork.base, (u32)(size << 15));


    // パラメータ設定
    FSiWramWork.command = FS_WRAM_COMMAND_READ;
    FSiWramWork.p_file  = p_file;
    FSiWramWork.nums    = MI_WRAM_ENUM_TO_SIZE( size ) * 1024 / FS_WRAM_SLOT_SIZE;
    
    // 必要に応じて7側にスイッチ可能なWRAMとして指定
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
    
    // WRAM->ARM9起動
    result = FSi_ReadWram(dst, (u32)len, wram, slot, callback, arg);

    MI_FreeWramSlot( wram, slot, size, MI_WRAM_ARM9 );
    FSi_SetSwitchableWramSlots(0,0); // スイッチ可能WRAMの指定を元に戻す
    FSiWramWork.busy = FALSE;
    return result;
}

BOOL FS_WriteFileViaWram( FSFile *p_file, const void *src, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg )
{
    OSIntrMode enabled = OS_DisableInterrupts();
    BOOL result;
    int l,n;
    SDK_ASSERT( wram != MI_WRAM_A );

    if ( FSiWramWork.busy ) // 転送中
    {
        OS_RestoreInterrupts(enabled);
        return FALSE;
    }
    FSiWramWork.busy = TRUE;
    OS_RestoreInterrupts(enabled);


    // WRAMの確保 
    FSiWramWork.base = MI_AllocWramSlot( wram, slot, size, MI_WRAM_ARM7 );
    if ( FSiWramWork.base == 0 )
    {
        FSiWramWork.busy = FALSE;
        OS_TPrintf("Cannot allocate WRAM %d, %d, %d\n", wram, slot, size);
        return FALSE;
    }

    // パラメータ設定
    FSiWramWork.command = FS_WRAM_COMMAND_WRITE;
    FSiWramWork.p_file  = p_file;
    FSiWramWork.nums    = MI_WRAM_ENUM_TO_SIZE( size ) * 1024 / FS_WRAM_SLOT_SIZE;

    // 必要に応じて7側にスイッチ可能なWRAMとして指定
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

    // ARM9->WRAM起動
    result = FSi_WriteWram(src, (u32)len, wram, slot, callback, arg);

    MI_FreeWramSlot( wram, slot, size, MI_WRAM_ARM7 );
    FSi_SetSwitchableWramSlots(0,0); // スイッチ可能WRAMの指定を元に戻す
    FSiWramWork.busy = FALSE;
    return result;
}
