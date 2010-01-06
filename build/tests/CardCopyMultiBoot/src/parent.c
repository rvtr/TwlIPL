/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     parent.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

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
#include <nitro/wm.h>
#include <nitro/mb.h>

#include "mbp.h"
#include "common.h"
#include "disp.h"
#include "gmain.h"
#include "wh.h"
#include "bt.h"


/******************************************************************************/

void sd_proc(void *p1);

static void GetChannelMain(void);
static BOOL ConnectMain(u16 tgid);
static void PrintChildState(void);
static BOOL JudgeConnectableChild(WMStartParentCallback *cb);

// ブロック転送状態通知関数
void BlockTransferCallback(void *arg);


//============================================================================
//  定数定義
//============================================================================

#define STACK_SIZE       1024
#define THREAD1_PRIO     15
#define MESSAGE_RECV     (OSMessage)100


/* このデモで使用する GGID */
#define WH_GGID                 SDK_MAKEGGID_SYSTEM(0x22)


/* このデモがダウンロードさせるプログラム情報 */
const MBGameRegistry mbGameList = {
    /*
     * クローンブートではプログラムのパス名に NULL を指定します.
     * ただしこれは MBP モジュールの MBP_RegistFile() における仕様で,
     * 実際に MB_RegisterFile() へ与える引数としては何でも構いません.
     */
    NULL,
    (u16 *)L"CardCopyMultiBoot",       // ゲーム名
    (u16 *)L"CardCopyMultiBoot(cloneboot)",      // ゲーム内容説明
    "/data/icon.char",                 // アイコンキャラクタデータ
    "/data/icon.plt",                  // アイコンパレットデータ
    WH_GGID,                           // GGID
    MBP_CHILD_MAX + 1,                 // 最大プレイ人数、親機の数も含めた人数
};



//============================================================================
//   変数定義
//============================================================================

OSThread sd_thread;
u32     stack1[STACK_SIZE / sizeof(u32)];
OSMessage mesgBuffer[10];
OSMessageQueue mesgQueue;
FSFile file;

BOOL writable = TRUE;

static s32 gFrame;                     // フレームカウンタ

extern u8 gRecvBuf[];

//-----------------------
// 通信経路の保持用
//-----------------------
static u16 sChannel = 0;
static const MBPChildInfo *sChildInfo[MBP_CHILD_MAX];

//============================================================================
//   関数定義
//============================================================================

void sd_proc(void *p1)
{
#pragma unused(p1)
    OSMessage src;

    while (1)
    {
        writable = TRUE;
        (void)WH_SendData(&writable, sizeof(writable), NULL);

        (void)OS_ReceiveMessage(&mesgQueue, &src, OS_MESSAGE_BLOCK);

        writable = FALSE;

        if (FS_WriteFile(&file, (void*)src, WH_CHILD_SIZE) == -1)
        {
            BgPutString(8, 3, 0x2, "Write SD File error!");
            OS_WaitVBlankIntr();
            OS_Terminate();
        }
        if ( ((u8*)src)[WH_CHILD_SIZE] == TRUE )
        {
            (void)FS_CloseFile(&file);
        }
    }
}


/*****************************************************************************/
/* 親機専用領域 .parent セクションの定義範囲を開始します */
#include <nitro/parent_begin.h>
/*****************************************************************************/


/*---------------------------------------------------------------------------*
  Name:         ParentMain

  Description:  親機メインルーチン

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ParentMain(void)
{
    FSFile dir;
    u16     tgid = 0;

    // 画面、OSの初期化
    CommonInit();

    // 画面初期化
    DispInit();

    // ヒープの初期化
    InitAllocateSystem();

    // WH に情報を設定
    WH_SetGgid(WH_GGID);

    // 割り込み有効
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    DispOn();

    // FS 初期化
    {
        static u32 fs_tablework[0x100 / 4];
        FS_Init(FS_DMA_NOT_USE);
        (void)FS_LoadTable(fs_tablework, sizeof(fs_tablework));
    }
    FS_InitFatDriver();

    {
        static const char *path = "sdmc:/card_dump.sbin";
        FS_InitFile(&file);
        (void)FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_RW);
        (void)FS_CreateFile(path, FS_PERMIT_R|FS_PERMIT_W);
        if (!FS_OpenFileEx(&file, path, FS_FILEMODE_RW))
        {
            BgPutString(8, 1, 0x2, "Cannot open writable SD File!");
            OS_WaitVBlankIntr();
            OS_Terminate();
        }
        (void)FS_SetFileLength(&file, 0);
    }

    while (TRUE)
    {
        OS_WaitVBlankIntr();

        // トラフィックの少ないチャンネルの検索処理
        GetChannelMain();

        /*
         * tgidは親機が起動の度に基本的には前回と違う値を設定します。
         * ただしマルチブート子機との再接続時には同じtgidで起動しなければ
         * 再スキャンを行なわなければ接続できなくなるため注意が必要です。
         * もう一度スキャンを行なってから再接続をする場合にはtgidを保存しておく
         * 必要はありません。
         */
        // マルチブート配信処理
        if (ConnectMain(++tgid))
        {
            // マルチブート子機の起動に成功
            break;
        }
        else
        {
            // WH モジュールを終了させて繰り返し
            WH_Finalize();
            while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
            (void)WH_End();
            while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
        }
    }

    //--------------
    // マルチブート後は子機がリセットされ通信が一旦切断されます。
    // また現状親機も一度MB_End()から通信を終了する必要があります。
    // 親子共一度完全に切断された状態で一から通信を確立してください。
    // 
    // またこの時子機のaidがシャッフルされるため、もし必要があれば
    // マルチブート前のaidとMACアドレスの組み合わせを保存しておき、
    // 再接続時に新しいaidとの結びつけを行なってください。
    //--------------


#if !defined(MBP_USING_MB_EX)
    if (!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }
#endif

    // 接続子機の判定用関数を設定
    WH_SetJudgeAcceptFunc(JudgeConnectableChild);

    // SDスレッド生成
    OS_InitMessageQueue(&mesgQueue, &mesgBuffer[0], 10);
    OS_CreateThread(&sd_thread, sd_proc, NULL, stack1 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD1_PRIO);

    /* メインルーチン */
    while (TRUE)
    {
        OS_WaitVBlankIntr();

        ReadKey();

//        BgClear();

        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_ERROR:
        case WH_SYSSTATE_CONNECT_FAIL:
            WH_Reset();
            break;

        case WH_SYSSTATE_IDLE:
            /* ----------------
             * 子機側で再スキャンなしに同じ親機に再接続させたい場合には
             * 子機側とtgid及びchannelを合わせる必要があります。
             * このデモでは、マルチブート時と同じchannelとマルチブート時のtgid+1を
             * 親子ともに使用することで、再スキャンなしでも接続できるようにしています。
             * 
             * MACアドレスを指定して再スキャンさせる場合には同じtgid, channelでなくても
             * 問題ありません。
             * ---------------- */
            (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, (u16)(tgid + 1), sChannel);
            WH_SetReceiver(MpReceiveCallback);

            // SDスレッド起動
            OS_WakeupThreadDirect(&sd_thread);
            break;

        case WH_SYSSTATE_CONNECTED:
            {
                BgPutString(8, 1, 0x2, "Parent mode");
//                ModeChild();
            }
            break;
        }

        // 子機状態を表示する
        ModeParent();
    }
}

/*---------------------------------------------------------------------------*
  Name:         GetChannelMain

  Description:  使用するチャンネルを電波使用率を調べてまじめに求める。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void GetChannelMain(void)
{

    /*-----------------------------------------------*
     * チャンネルの電波使用率をきちんと調べた上で、
     * 一番使用率の低いチャンネルを選択します。
     * WM_MeasureChannel()を実行するにはIDLE状態になる必要があり
     * マルチブート状態ではIDLE状態に止まる事がないので実行できません。
     * 一旦WM_Initializeを呼んで電波使用率を調べてからWM_Endで終了し、
     * あらためてMB_Initを実行する。
     *-----------------------------------------------*/
    if (!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }

    while (TRUE)
    {
        ReadKey();
        BgClear();
        BgSetMessage(PLTT_YELLOW, " Search Channel ");

        switch (WH_GetSystemState())
        {
            //-----------------------------------------
            // 初期化完了
        case WH_SYSSTATE_IDLE:
//            BgSetMessage(PLTT_WHITE, " Push A Button to start   ");
//            if (IS_PAD_TRIGGER(PAD_BUTTON_A))
            {
                BgSetMessage(PLTT_YELLOW, "Check Traffic ratio       ");
                (void)WH_StartMeasureChannel();
            }
            break;
            //-----------------------------------------
            // チャンネル検索完了
        case WH_SYSSTATE_MEASURECHANNEL:
            {
                sChannel = WH_GetMeasureChannel();
#if !defined(MBP_USING_MB_EX)
                (void)WH_End();
#else
                /* IDLE 状態を維持したままマルチブート処理へ */
                return;
#endif
            }
            break;
            //-----------------------------------------
            // WM終了
        case WH_SYSSTATE_STOP:
            /* WM_Endが完了したらマルチブート処理へ */
            return;
            //-----------------------------------------
            // ビジー中
        case WH_SYSSTATE_BUSY:
            break;
            //-----------------------------------------
            // エラー発生
        case WH_SYSSTATE_ERROR:
            (void)WH_Reset();
            break;
            //-----------------------------------------
        default:
            OS_Panic("Illegal State\n");
        }
        OS_WaitVBlankIntr();           // Vブランク割込終了待ち
    }
}


/*---------------------------------------------------------------------------*
  Name:         ConnectMain

  Description:  マルチブートで接続する。

  Arguments:    tgid        親機として起動する場合のtgidを指定します.

  Returns:      子機への転送に成功した場合には TRUE,
                失敗したりキャンセルされた場合には  FALSE を返します。
 *---------------------------------------------------------------------------*/
static BOOL ConnectMain(u16 tgid)
{
    MBP_Init(mbGameList.ggid, tgid);

    while (TRUE)
    {
        ReadKey();

        BgClear();

        BgPutString(6, 1, PLTT_RED, "Card Copy Multi Boot");

        BgSetMessage(PLTT_YELLOW, " MB Parent ");

        switch (MBP_GetState())
        {
            //-----------------------------------------
            // アイドル状態
        case MBP_STATE_IDLE:
            {
                MBP_Start(&mbGameList, sChannel);
            }
            break;

            //-----------------------------------------
            // 子機からのエントリー受付中
        case MBP_STATE_ENTRY:
            {
                BgSetMessage(PLTT_YELLOW, " Wait for list on MB menu ");
//                BgSetMessage(PLTT_WHITE, " Now Accepting            ");

                if (IS_PAD_TRIGGER(PAD_BUTTON_B))
                {
                    // Bボタンでマルチブートキャンセル
//                    MBP_Cancel();
                    break;
                }

                // エントリー中の子機が一台でも存在すれば開始可能とする
                if (MBP_GetChildBmp(MBP_BMPTYPE_ENTRY) ||
                    MBP_GetChildBmp(MBP_BMPTYPE_DOWNLOADING) ||
                    MBP_GetChildBmp(MBP_BMPTYPE_BOOTABLE))
                {
                    BgSetMessage(PLTT_WHITE, " Push START Button to start   ");

//                    if (IS_PAD_TRIGGER(PAD_BUTTON_START))
                    {
                        // ダウンロード開始
                        MBP_StartDownloadAll();
                    }
                }
            }
            break;

            //-----------------------------------------
            // プログラム配信処理
        case MBP_STATE_DATASENDING:
            {

                // 全員がダウンロード完了しているならばスタート可能.
                if (MBP_IsBootableAll())
                {
                    // ブート開始
                    MBP_StartRebootAll();
                }
            }
            break;

            //-----------------------------------------
            // リブート処理
        case MBP_STATE_REBOOTING:
            {
                BgSetMessage(PLTT_WHITE, " Rebooting now                ");
            }
            break;

            //-----------------------------------------
            // 再接続処理
        case MBP_STATE_COMPLETE:
            {
                // 全員無事に接続完了したらマルチブート処理は終了し
                // 通常の親機として無線を再起動する。
                BgSetMessage(PLTT_WHITE, " Reconnecting now             ");

                OS_WaitVBlankIntr();
                return TRUE;
            }
            break;

            //-----------------------------------------
            // エラー発生
        case MBP_STATE_ERROR:
            {
                // 通信をキャンセルする
                MBP_Cancel();
            }
            break;

            //-----------------------------------------
            // 通信キャンセル処理中
        case MBP_STATE_CANCEL:
            // None
            break;

            //-----------------------------------------
            // 通信異常終了
        case MBP_STATE_STOP:
            OS_WaitVBlankIntr();
            return FALSE;
        }

        // 子機状態を表示する
        PrintChildState();

        OS_WaitVBlankIntr();           // Vブランク割込終了待ち
    }
}


/*---------------------------------------------------------------------------*
  Name:         PrintChildState

  Description:  子機情報を画面に表示する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintChildState(void)
{
    static const char *STATE_NAME[] = {
        "NONE       ",
        "CONNECTING ",
        "REQUEST    ",
        "ENTRY      ",
        "DOWNLOADING",
        "BOOTABLE   ",
        "BOOTING    ",
    };
    enum
    {
        STATE_DISP_X = 2,
        INFO_DISP_X = 15,
        BASE_DISP_Y = 2
    };

    u16     i;

    /* 子機リストの表示 */
    for (i = 1; i <= MBP_CHILD_MAX; i++)
    {
        const MBPChildInfo *childInfo;
        MBPChildState childState = MBP_GetChildState(i);
        const u8 *macAddr;

        SDK_ASSERT(childState < MBP_CHILDSTATE_NUM);

        // 状態表示
        BgPutString(STATE_DISP_X, i + BASE_DISP_Y, PLTT_WHITE, STATE_NAME[childState]);

        // ユーザー情報表示
        childInfo = MBP_GetChildInfo(i);
        macAddr = MBP_GetChildMacAddress(i);

        if (macAddr != NULL)
        {
            BgPrintStr(INFO_DISP_X, i + BASE_DISP_Y, PLTT_WHITE,
                       "%02x%02x%02x%02x%02x%02x",
                       macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         JudgeConnectableChild

  Description:  再接続時に接続可能な子機かどうかを判定する関数

  Arguments:    cb      接続してきた子機の情報.

  Returns:      接続を受け付ける場合は TRUE.
                受け付けない場合は FALSE.
 *---------------------------------------------------------------------------*/
static BOOL JudgeConnectableChild(WMStartParentCallback *cb)
{
    u16     playerNo;

    /*  cb->aid の子機のマルチブート時のaidをMACアドレスから検索します */
    playerNo = MBP_GetPlayerNo(cb->macAddress);

    OS_TPrintf("MB child(%d) -> MP child(%d)\n", playerNo, cb->aid);

    if (playerNo == 0)
    {
        return FALSE;
    }

    sChildInfo[playerNo - 1] = MBP_GetChildInfo(playerNo);
    return TRUE;
}


/*****************************************************************************/
/* 親機専用領域 .parent セクションの定義範囲を終了します */
#include <nitro/parent_end.h>
/*****************************************************************************/
