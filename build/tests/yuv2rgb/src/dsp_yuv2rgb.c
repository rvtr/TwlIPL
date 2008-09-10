/*---------------------------------------------------------------------------*
  Project:  TwlSDK - YUV2RGB
  File:     dsp_yuv2rgb.c

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
#include <twl/dsp.h>
#include <twl/dsp/common/pipe.h>
#include "yuv2rgb.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

#define THREAD_YUV2RGB_PRIO    10       // メインスレッドより優先
#define STACK_SIZE           1024
#define DSP_BUFFER_SIZE     0x6000      // 24KB

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/

OSThread threadYUV2RGB;                     // YUV2RGBスレッド
u64      stack[STACK_SIZE / sizeof(u64)];   // YUV2RGBスレッド用スタック

// メッセージ関連
OSMessage mesgBuffer[10];
OSMessageQueue mesgQueue;

static YUV2RGBCallback sCallback;
static u8* sSrc;
static u8* sDst;
static u32 sSize;
static u32 sDmaNo;
static u8  sBusy;

DSPPipe binout[1];
DSPPipe binin[1];

/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/
BOOL YUV2RGB_Init(void);
static void DSPi_Yuv2RgbThread(void *arg);
static void DSPi_Yuv2RgbLoadProgram();
static void DSPi_Yuv2RgbConvertCore(void);

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_Yuv2RgbInit

  Description:  初期化関数

  Arguments:    ***

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL DSP_Yuv2RgbInit(u32 dmaNo)
{
    // DSPファームをロード
    DSPi_Yuv2RgbLoadProgram();

    // DSPパイプ情報をロード（DSP->ARM）
    (void)DSP_LoadPipe(
            binout,             // パイプ情報の格納先 (DSP側ではNULLでよい)
            DSP_PIPE_BINARY,    // パイプのポート番号（1）
            DSP_PIPE_OUTPUT);   // DSP_PIPE_INPUT または DSP_PIPE_OUTPUT

    (void)DSP_LoadPipe(
            binin,              // パイプ情報の格納先 (DSP側ではNULLでよい)
            DSP_PIPE_BINARY,    // パイプのポート番号（1）
            DSP_PIPE_INPUT);    // DSP_PIPE_INPUT または DSP_PIPE_OUTPUT

    // メッセージキューの初期化
    OS_InitMessageQueue(&mesgQueue, &mesgBuffer[0], 10);

    // YUV2RGB変換スレッド生成・起動
    OS_CreateThread(&threadYUV2RGB, DSPi_Yuv2RgbThread, (void *)0, stack + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD_YUV2RGB_PRIO);
    OS_WakeupThreadDirect(&threadYUV2RGB);

    // WRAM-C Slot1 の割り当てを DSP->ARM へ変更
    if ( MI_SwitchWramSlot_C( 1, MI_WRAM_SIZE_32KB, MI_WRAM_DSP, MI_WRAM_ARM9 ) != 1 )
    {
            OS_TPanic("can't allocate WRAM Slot");
    }

    // 使用するDMA番号を保存
    sDmaNo = dmaNo;

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_Yuv2RgbLoadProgram

  Description:  DSPテストプログラムをロード。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_Yuv2RgbLoadProgram(void)
{
    MI_FreeWram_B(MI_WRAM_ARM9);
    MI_CancelWram_B(MI_WRAM_ARM9);
    MI_FreeWram_C(MI_WRAM_ARM9);
    MI_CancelWram_C(MI_WRAM_ARM9);
    {
        extern const u8 _binary_simple_dat[];
        extern const u8 _binary_simple_dat_end[];
        if (!DSP_LoadFileAuto(_binary_simple_dat))
        {
            OS_TPanic("can't allocate WRAM Slot");
        }
        // 必ずこのタイミングでONしないといけないのかどうかは未確認。
        {
            static BOOL once = FALSE;
            if (!once)
            {
                DSP_PowerOn();
                once = TRUE;
            }
        }
//      DSP_ResetInterface();
        DSP_ResetOff();
        OS_TPrintf("dsp app run...\n");
        DSP_EnableRecvDataInterrupt(0);
        DSP_EnableRecvDataInterrupt(1);
        DSP_EnableRecvDataInterrupt(2);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_Yuv2RgbThread

  Description:  YUV2RGB専用スレッド

  Arguments:    ***

  Returns:      None
 *---------------------------------------------------------------------------*/

static void DSPi_Yuv2RgbThread(void *arg)
{
#pragma unused( arg )
    OSMessage message;

    while (1)
    {
        (void)OS_ReceiveMessage(&mesgQueue, &message, OS_MESSAGE_BLOCK);
        
        switch ((MessageYuv2Rgb)message)
        {
        case MESSAGE_YUV2RGB_CONVERT:
            DSPi_Yuv2RgbConvertCore();
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Yuv2RgbConvertAsync

  Description:  YUV->RGB変換を行います（非同期版）

  Arguments:    ***

  Returns:      依頼成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_Yuv2RgbConvertAsync(void* src, void* dest, u32 size, YUV2RGBCallback callback)
{
    OSMessage message;

    message = (OSMessage)MESSAGE_YUV2RGB_CONVERT;

    if (!sBusy)
    {
        sCallback = callback;   // コンバート完了後に呼び出すコールバックを保存
        sSrc  = src;
        sDst  = dest;
        sSize = size;
        return OS_SendMessage(&mesgQueue, message, OS_MESSAGE_NOBLOCK);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_Yuv2RgbConvertCore

  Description:  YUV->RGB変換のためにDSPとのやり取りや
                データコピーなどを行います

  Arguments:    ***

  Returns:      None
 *---------------------------------------------------------------------------*/
static void DSPi_Yuv2RgbConvertCore(void)
{
    u16 command;
    u16 reply;
    u32 adress =  MI_GetWramMapStart_C() + MI_WRAM_C_SLOT_SIZE;		// 暫定的にスロット1固定
    OSTick begin, current, tick_copy1, tick_convert, tick_copy2;
    u32 offset =0;

    // ビジーフラグＯＮ
    sBusy = TRUE;

    tick_copy1=tick_convert=tick_copy2=0;

    // 現状DSP側にバッファを24KBしか確保できていないため256x192(96Kb）などの
    // 大きいサイズの変換は4回に分けて行っている。

    while (sSize > 0)
    {
        u16 size;
    
        if (sSize > DSP_BUFFER_SIZE) { size = DSP_BUFFER_SIZE; }
        else { size = (u16)sSize; }

        sSize -= size;
        
begin = OS_GetTick();

        // src -> WRAM-C に書き込む
        if (sDmaNo == DSP_DMA_NOT_USE)
        {
            MI_CpuCopy16(sSrc + offset, (void*)adress, size);
            DC_FlushRange((void*)adress, size);
        }
        else
        {
            MI_NDmaCopy( 3, sSrc + offset, (void*)adress,  size );
        }

        // WRAM-C Slot1 の割り当てを ARM->DSP へ変更
        if ( MI_SwitchWramSlot_C( 1, MI_WRAM_SIZE_32KB, MI_WRAM_ARM9, MI_WRAM_DSP ) != 1 )
        {
            OS_Printf("====== WRAM Allocation Fail ! =======\n");
        }

current = OS_GetTick();
tick_copy1 += (current - begin);
begin = current;

        // DSPへコマンド送信
        command = (u16)(size>>1);       // ピクセル数として渡す
        DSP_WritePipe(binout, &command, sizeof(u16));

        // DSPからコマンド受信（ここでスリープに入り、DSP_HookPipeNotificationで起床）
        DSP_ReadPipe(binin, &reply, sizeof(u16));

current = OS_GetTick();
tick_convert += (current - begin);
begin = current;

        // WRAM-C Slot1 の割り当てを DSP->ARM へ変更
        if ( MI_SwitchWramSlot_C( 1, MI_WRAM_SIZE_32KB, MI_WRAM_DSP, MI_WRAM_ARM9 ) != 1 )
        {
            OS_Printf("====== WRAM Allocation Fail ! =======\n");
        }

        // WRAM-C -> dst に書き込む
        if (sDmaNo == DSP_DMA_NOT_USE)
        {
            DC_InvalidateRange((void*)adress, size);
            MI_CpuCopy16((void*)adress, sDst + offset, size);
        }
        else
        {
            MI_NDmaCopy( 3, (void*)adress, sDst + offset, size);
        }

current = OS_GetTick();
tick_copy2 += (current - begin);

        // オフセット更新
        offset += size;
    }

    OS_Printf("copy to wram = %d us, ", OS_TicksToMicroSeconds(tick_copy1));
    OS_Printf("convert = %d us, ", OS_TicksToMicroSeconds(tick_convert));
    OS_Printf("from wram = %d us, ", OS_TicksToMicroSeconds(tick_copy2));
    OS_Printf("total = %d us\n", OS_TicksToMicroSeconds(tick_copy1+tick_convert+tick_copy2));

    // ビジーフラグＯＦＦ
    sBusy = FALSE;
    
    if (sCallback)
    {
        sCallback();
    }
}

