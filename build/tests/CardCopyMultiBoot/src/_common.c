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


static void ModeSelect(void);          // 親機/子機 選択画面
static void ModeStartParent(void);     // 使用率が低いチャンネルを計算し終えた状態
static void ModeChild(void);           // 子機 通信画面

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
static BOOL gRecvFlag[1 + WM_NUM_MAX_CHILD];

static int send_counter[16];
static int recv_counter[16];

static BOOL gFirstSendAtChild = TRUE;

TEXT_CTRL *tc[NUM_OF_SCREEN];

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
    FX_Init();

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
  Name:         BlockTransferMain

  Description:  ブロック転送状態通知関数。

  Arguments:    arg     - 通知元 WM 関数のコールバックポインタ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BlockTransferMain(void)
{
        // 通信状態により処理を振り分け
        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_IDLE:
            ModeSelect();
            break;
        case WH_SYSSTATE_MEASURECHANNEL:
            ModeStartParent();
            break;
        case WH_SYSSTATE_ERROR:
        case WH_SYSSTATE_CONNECT_FAIL:
            WH_Reset();
            break;
        case WH_SYSSTATE_FATAL:
            break;
        case WH_SYSSTATE_SCANNING:
        case WH_SYSSTATE_BUSY:
            break;
        case WH_SYSSTATE_CONNECTED:
            // 親機か子機かでさらに分岐する
            switch (WH_GetConnectMode())
            {
            case WH_CONNECTMODE_MP_PARENT:
//                ModeParent();
                break;
            case WH_CONNECTMODE_MP_CHILD:
                ModeChild();
                break;
            }
            break;
        }
}

/*---------------------------------------------------------------------------*
  Name:         BlockTransferCallback

  Description:  ブロック転送状態通知関数。

  Arguments:    arg     - 通知元 WM 関数のコールバックポインタ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BlockTransferCallback(void *arg)
{
    int connectMode = WH_GetConnectMode();
    
    switch (((WMCallback*)arg)->apiid)
    {
    case WM_APIID_START_MP:
        {                              /* MP ステート開始 */
           WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                if (connectMode == WH_CONNECTMODE_MP_PARENT)
                {
                    ParentSendCallback();
                }
                else if (connectMode == WH_CONNECTMODE_MP_CHILD)
                {
                    WBT_SetOwnAid(WH_GetCurrentAid());
//                    mfprintf(tc[2], "aid = %d\n", WH_GetCurrentAid());
                    bt_start();
                    ChildSendCallback();
                }
                break;
            }
        }
        break;
    case WM_APIID_SET_MP_DATA:
        {                              /* 単発の MP 通信完了 */
            if (connectMode == WH_CONNECTMODE_MP_PARENT)
            {
                if (connected_bitmap != 0)
                {
                    ParentSendCallback();
                }
            }
            else if (connectMode == WH_CONNECTMODE_MP_CHILD)
            {
                ChildSendCallback();
            }
        }
        break;
    case WM_APIID_START_PARENT:
        {                              /* 新規の子機接続 */
            WMStartParentCallback *cb = (WMStartParentCallback *)arg;
            if (connectMode == WH_CONNECTMODE_MP_PARENT)
            {
                switch (cb->state)
                {
                case WM_STATECODE_CONNECTED:
                    if (connected_bitmap == 0)
                    {
                        ParentSendCallback();
                    }
                    connected_bitmap |= (1 << cb->aid);
                    break;
                case WM_STATECODE_DISCONNECTED:
                    connected_bitmap &= ~(1 << cb->aid);
                    break;
                }
            }
        }
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ChildSendCallback

  Description:  子機として親機からのデータ受信時に呼び出される関数。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChildSendCallback(void)
{
    const u16 size = (u16)WBT_MpChildSendHook(gSendBuf, WC_CHILD_DATA_SIZE_MAX);
    send_counter[0]++;
    (void)WH_SendData(gSendBuf, size, NULL);
}


/*---------------------------------------------------------------------------*
  Name:         ParentSendCallback

  Description:  親機として子機へのデータ送信時に呼び出される関数。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ParentSendCallback(void)
{
    const u16 size = (u16)WBT_MpParentSendHook(gSendBuf, WC_PARENT_DATA_SIZE_MAX);
    send_counter[0]++;
    (void)WH_SendData(gSendBuf, size, NULL);
}


/*---------------------------------------------------------------------------*
  Name:         ParentReceiveCallback

  Description:  親機として子機からのデータ受信時に呼び出される関数。

  Arguments:    aid     - 送信元子機の aid
                data    - 受信データへのポインタ (NULL で切断通知)
                length  - 受信データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ParentReceiveCallback(u16 aid, u16 *data, u16 length)
{
    BgSetMessage(PLTT_YELLOW, " Receive: p=0x%x, len=0x%x", data, length);

	recv_counter[aid]++;
    if (data != NULL)
    {
        gRecvFlag[aid] = TRUE;
        // コピー元は2バイトアライン(4バイトアラインでない)
//        recv_counter[aid]++;
        WBT_MpParentRecvHook((u8 *)data, length, aid);
    }
    else
    {
        gRecvFlag[aid] = FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ChildReceiveCallback

  Description:  子機として親機からのデータ受信時に呼び出される関数。

  Arguments:    aid     - 送信元親機の aid (常に 0)
                data    - 受信データへのポインタ (NULL で切断通知)
                length  - 受信データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChildReceiveCallback(u16 aid, u16 *data, u16 length)
{
    BgSetMessage(PLTT_YELLOW, " Receive: p=0x%x, len=0x%x", data, length);

    (void)aid;
    recv_counter[0]++;
    if (data != NULL)
    {
        gRecvFlag[0] = TRUE;
        // コピー元は2バイトアライン(4バイトアラインでない)
        WBT_MpChildRecvHook((u8 *)data, length);
    }
    else
    {
        gRecvFlag[0] = FALSE;
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
    
    gFirstSendAtChild = TRUE;
    
    if (wbt_available)
    {
        bt_stop();
        WBT_End();
        wbt_available = FALSE;
    }

    if (!MB_IsMultiBootChild())
    {
        BgSetMessage(PLTT_YELLOW, " Connect as PARENT");
        //********************************
        WBT_InitParent(BT_PARENT_PACKET_SIZE, BT_CHILD_PACKET_SIZE, bt_callback);
        WH_SetReceiver(ParentReceiveCallback);
        bt_register_blocks();
        (void)WH_StartMeasureChannel();
        wbt_available = TRUE;
        //********************************
    }
    else
    {
        static const u8 ANY_PARENT[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        BgSetMessage(PLTT_YELLOW, " Connect as CHILD");
        //********************************
        WBT_InitChild(bt_callback);
        WH_SetReceiver(ChildReceiveCallback);
        (void)WH_ChildConnectAuto(WH_CONNECTMODE_MP_CHILD, ANY_PARENT, 0);
        wbt_available = TRUE;
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeStartParent

  Description:  使用率の低いチャンネルを計算し終えたときの処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeStartParent(void)
{
    (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, 0x0000, WH_GetMeasureChannel());
}

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  子機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeChild(void)
{
    if (gFirstSendAtChild)
    {
        // 1回目のデータ送信
        ChildSendCallback();
        gFirstSendAtChild = FALSE;
    }

#if 0
    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        WH_Finalize();
        //********************************
    }
    else if (gKey.trg & PAD_BUTTON_Y)
    {
        bt_start();
    }
    else if (gKey.trg & PAD_BUTTON_X)
    {
        bt_stop();
    }
#endif
}

