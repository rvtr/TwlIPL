/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     common.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
*---------------------------------------------------------------------------*/


#include <nitro.h>
#include <nitro/wm.h>
#include <nitro/mb.h>

#include "wh.h"
#include "common.h"
#include "disp.h"
#include "text.h"
#include "bt.h"

#define     DEFAULT_CHAN            1

extern OSMessageQueue mesgQueue;

static void ModeSelect(void);          // 親機/子機 選択画面
static void ModeStartParent(void);     // 使用率が低いチャンネルを計算し終えた状態

// データ送信時に呼び出される関数
void    ParentSendCallback(void);
void    ChildSendCallback(void);

static void VBlankIntr(void);

/*
 * このデモ全体で使用する共通機能.
 */
static u16 padPress;
static u16 padTrig;

// データ受信時に呼び出される関数
void ParentReceiveCallback(u16 aid, u16 *data, u16 length);
void ChildReceiveCallback(u16 aid, u16 *data, u16 length);

// ブロック転送状態通知関数
void BlockTransferCallback(void *arg);

// 表示用送受信バッファ
static u8 gSendBuf[256] ATTRIBUTE_ALIGN(32);
u8 gRecvBuf[1 + WM_NUM_MAX_CHILD][256] ATTRIBUTE_ALIGN(32);
static BOOL gRecvFlag[1 + WM_NUM_MAX_CHILD];

static u32 send_counter[16];
static u32 recv_counter[16];

TEXT_CTRL *tc[NUM_OF_SCREEN];

static BOOL gFirstSendAtChild;

static BOOL wbt_available = FALSE;
static u16 connected_bitmap = 0;

/*---------------------------------------------------------------------------*
  Name:         ReadKey

  Description:  キーの読み込み処理

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void ReadKey(void)
{
    u16     currData = PAD_Read();

    padTrig = (u16)(~padPress & currData);
    padPress = currData;
}

/*---------------------------------------------------------------------------*
  Name:         GetPressKey

  Description:  押下キー取得

  Arguments:    None

  Returns:      押下されているキーのビットマップ
 *---------------------------------------------------------------------------*/
u16 GetPressKey(void)
{
    return padPress;
}


/*---------------------------------------------------------------------------*
  Name:         GetTrigKey

  Description:  キートリガ取得

  Arguments:    None

  Returns:      キートリガのビットマップ
 *---------------------------------------------------------------------------*/
u16 GetTrigKey(void)
{
    return padTrig;
}


/*---------------------------------------------------------------------------*
  Name:         CommonInit

  Description:  共通初期化関数

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CommonInit(void)
{
    /* OS 初期化 */
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    OS_InitThread();
    FX_Init();
    CARD_Init();

    /* GX 初期化 */
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    /* Vブランク割込設定 */
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)GX_VBlankIntr(TRUE);

    // キーを一回空読み
    ReadKey();
}


/*---------------------------------------------------------------------------*
  Name:         InitAllocateSystem

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    // OS_Initは呼ばれているという前提
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}




/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  キートリガ取得

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    DispVBlankFunc();

    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         MpSendCallback

  Description:  MPデータ送信後に呼び出される関数。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MpSendCallback(BOOL result)
{
#pragma unused( result )
    CARDRomHeader* rh = (void*)CARD_GetRomHeader();
    u32 rh_size = sizeof(CARDRomHeaderNTR);
    u32 limit = rh->rom_size + CARD_ROM_PAGE_SIZE;
    u32 offset = WH_CHILD_SIZE * send_counter[0];

    BgSetMessage(PLTT_YELLOW, " Sending: ROM addr=0x%x", offset);

    if ( offset < limit )
    {
        if ( offset < rh_size )
        {
            u32 rem = rh_size%WH_CHILD_SIZE;
            MI_CpuCopy8( &((u8*)rh)[offset], gSendBuf, WH_CHILD_SIZE );
            if ( offset >= MATH_ROUNDDOWN(rh_size, WH_CHILD_SIZE) )
            {
                MI_CpuFill8( &gSendBuf[rem], 0, WH_CHILD_SIZE - rem );
            }
        }
        else
        if ( offset < CARD_GAME_AREA_OFFSET )
        {
            MI_CpuFill8(gSendBuf, 0, WH_CHILD_SIZE);
        }
        else
        {
            CARD_ReadRom( MI_DMA_NOT_USE, (void*)offset, gSendBuf, WH_CHILD_SIZE );
        }
        gSendBuf[WH_CHILD_SIZE] = FALSE;
        if ( (offset + WH_CHILD_SIZE) >= limit )
        {
            gSendBuf[WH_CHILD_SIZE] = TRUE;
        }
        (void)WH_SendData(gSendBuf, WH_CHILD_MAX_SIZE, NULL);
        send_counter[0]++;
    }
    else
    {
        BgSetMessage(PLTT_RED, " Sent ROM size=0x%x      ", MATH_ROUNDUP(limit, WH_CHILD_SIZE) - CARD_GAME_AREA_OFFSET);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MpReceiveCallback

  Description:  MPデータ受信時に呼び出される関数。

  Arguments:    aid     - 送信元子機の aid( 0 の場合は親機からのデータ )
                data    - 受信データへのポインタ (NULL で切断通知)
                length  - 受信データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MpReceiveCallback(u16 aid, u16 *data, u16 length)
{
    SDK_MAX_ASSERT(aid, 15);

    // 子機
    if ( MB_IsMultiBootChild() )
    {
        if (aid == 0)
        {
            if (data && *data == TRUE)
            {
                MpSendCallback( TRUE );
            }
        }

        return;
    }

    // 親機
    if (data != NULL)
    {
        gRecvFlag[aid] = TRUE;
        // コピー元は2バイトアライン(4バイトアラインでない)
        if (aid == 0)
        {
            // 親機から受信した場合
            MI_CpuCopy8(data, &gRecvBuf[aid][0], length);
        }
        else
        {
            static u32 offset = CARD_GAME_AREA_OFFSET;
            offset += WH_CHILD_SIZE;
            MI_CpuCopy8(data, &gRecvBuf[aid][0], length);
            BgSetMessage(PLTT_YELLOW, " Receiving: ROM addr=0x%x", offset);
            if ( gRecvBuf[aid][WH_CHILD_SIZE] == TRUE )
            {
                BgSetMessage(PLTT_RED, " Received ROM size=0x%x   ", offset - CARD_GAME_AREA_OFFSET);
            }
            (void)OS_SendMessage(&mesgQueue, (OSMessage)&gRecvBuf[aid][0], OS_MESSAGE_BLOCK);
        }
    }
    else
    {
        gRecvFlag[aid] = FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  親機/子機 選択画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeSelect(void)
{
    // カウンタクリア
    MI_CpuClear(send_counter, sizeof(send_counter));
    MI_CpuClear(recv_counter, sizeof(recv_counter));

    if (!MB_IsMultiBootChild())
    {
        BgSetMessage(PLTT_YELLOW, " Connect as PARENT");
        //********************************
        (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, 0x0000, DEFAULT_CHAN); // WH_GetMeasureChannel()
        WH_SetReceiver(MpReceiveCallback);
        //********************************
    }
    else
    {
        static const u8 ANY_PARENT[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        BgSetMessage(PLTT_YELLOW, " Connect as CHILD");
        //********************************
        (void)WH_ChildConnectAuto(WH_CONNECTMODE_MP_CHILD, ANY_PARENT, DEFAULT_CHAN);
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  親機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeParent(void)
{
    BgPutString(6, 1, PLTT_RED, "Card Copy Multi Boot");

    BgPrintStr(11, 3, PLTT_YELLOW, "Parent mode");
//    BgPrintStr(4, 3, 0x4, "Send:     %08X", gSendBuf[0]);
    BgPrintStr(4, 5, 0x4, "Receive:");
    {
        s32     i;

        for (i = 1; i < (WM_NUM_MAX_CHILD + 1); i++)
        {
            if (gRecvFlag[i])
            {
                BgPrintStr(5, (s16)(6 + i), 0x4, "Child%02d: %08X", i, gRecvBuf[i][0]);
            }
            else
            {
                BgPrintStr(5, (s16)(6 + i), 0x7, "No child");
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  子機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeChild(void)
{
    BgPutString(6, 1, PLTT_RED, "Card Copy Multi Boot");

    BgPutString(11, 3, PLTT_YELLOW, "Child mode");

    if ( !gFirstSendAtChild )
    {
        BgSetMessage(PLTT_WHITE, " Push A Button to start   ");
    }

    if ( !gFirstSendAtChild ) // (GetTrigKey() & PAD_BUTTON_A) )
    {
        MpSendCallback( TRUE );
        gFirstSendAtChild = TRUE;
    }
}

