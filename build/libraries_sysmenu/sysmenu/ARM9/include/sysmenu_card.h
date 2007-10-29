/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_card.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef SYSM_CARD_H_
#define SYSM_CARD_H_

#include <twl.h>


#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
//     					カード抜け検出
//
//・ARM7 がカードからのダウンロードを完了するまでは検出を開始しません。
//・検出処理中はカードバスへのアクセスをロックします。
//・アプリケーションを起動する前に SYSM_FinalizeCardPulledOut()
//  を呼び出して確実に検出処理を完了させるようにして下さい。
//----------------------------------------------------------------------

BOOL SYSM_IsCardPulledOut(void);


//----------------------------------------------------------------------
//     					カード抜け検出終了処理
//----------------------------------------------------------------------

void SYSM_FinalizeCardPulledOut(void);


//----------------------------------------------------------------------
//     					カード抜け検出処理中か
//----------------------------------------------------------------------

BOOL SYSM_IsDetectingCardPulledOut(void);


//----------------------------------------------------------------------
//     					カード抜け検出初期化
//----------------------------------------------------------------------

void SYSMi_InitCardPulledOut(void);


//----------------------------------------------------------------------
//     					カードID読み込み
//----------------------------------------------------------------------

u32 SYSMi_ReadCardID(void);


//----------------------------------------------------------------------
//     					カードデータ読み込み
//----------------------------------------------------------------------

void SYSM_ReadCard(void *romp, void *ramp, s32 size);


//----------------------------------------------------------------------
//                      カードのデータ転送はレディか？
//----------------------------------------------------------------------

#define SYSMi_IsCardDataReady()                             \
                                                            \
    (*(vu32 *)REG_CARDCNT & CARD_DATA_READY)

//・カードのデータ転送はレディかどうかを返します。

//----------------------------------------------------------------------
//                      カードデータ待ち
//----------------------------------------------------------------------

#define SYSMi_WaitCardData()                                \
{                                                           \
    while (!SYSMi_IsCardDataReady())    ;                   \
}

//・カードデータ転送の終了を待ちます。


//----------------------------------------------------------------------
//                      カードはビジーか？
//----------------------------------------------------------------------

#define SYSMi_IsCardBusy()                                  \
                                                            \
    (*(vu32 *)REG_CARDCNT & CARD_START)

//・カードがビジーかどうかを返します。

//----------------------------------------------------------------------
//                      カード待ち
//----------------------------------------------------------------------

#define SYSMi_WaitCard()                                    \
{                                                           \
    while (SYSMi_IsCardBusy())   ;                          \
}

//・カードの終了を待ちます。


//----------------------------------------------------------------------
//                      コントロールパラメータ獲得（GAMEモード）
//----------------------------------------------------------------------

#define SYSMi_GetCardCnt4Game()                             \
                                                            \
    (*(vu32 *)MROMCNT_GAME_BUF)

//・GAMEモードのコントロールパラメータを獲得します。



//----------------------------------------------------------------------
//                      ＲＯＭエリア・マップ
//----------------------------------------------------------------------

#define MROM_SECURE_AREA    0x4000              // SECUREエリア
#define MROM_GAME_AREA      0x8000              //   GAMEエリア

//----------------------------------------------------------------------
//                      ＲＯＭエリア・サイズ
//----------------------------------------------------------------------

#define MROM_SEGMENT_SIZE   0x1000              // セグメントサイズ
#define MROM_SECURE_SIZE    0x4000              // SECUREエリアサイズ

#ifndef MROM_PAGE_SIZE
#define MROM_PAGE_SIZE          512                 // マスクＲＯＭ・ページ
#endif

//----------------------------------------------------------------------
//                      メモリ・マップ
//----------------------------------------------------------------------

#define MROMCNT_GAME_BUF    (HW_ROM_HEADER_BUF + 0x60) //   GAMEモード・コントロールデータ
#define MROMCNT_SECURE_BUF  (HW_ROM_HEADER_BUF + 0x64) // SECUREモード・コントロールデータ


//----------------------------------------------------------------------
//                      レジスタ・アドレス
//----------------------------------------------------------------------

#ifndef REG_BASE
#define REG_BASE                0x04000000          // レジスタ群
#endif
#ifndef REG_IME
#define REG_IME             (REG_BASE + 0x208)  // 割り込みマスタイネーブル
#endif

#define REG_CARDMST_SPI_CNT (REG_BASE + 0x1a0)  // カードマスター＆ＳＰＩコントロール

#define REG_CARD_MASTER_CNT (REG_BASE + 0x1a1)  // カードマスターコントロール

#define REG_CARD_SPI_CNT    (REG_BASE + 0x1a0)  // カードＳＰＩコントロール
#define REG_CARD_SPI_DATA   (REG_BASE + 0x1a2)  //             データ

#define REG_CARDCNT         (REG_BASE + 0x1a4)  // カードコントロール
#define REG_CARD_CMD        (REG_BASE + 0x1a8)  //       コマンド設定
#define REG_CARD_DATA       (REG_BASE + 0x100010) //     データ


//----------------------------------------------------------------------
//                      カード マスターコントロール
//----------------------------------------------------------------------

#define CARDMST_SEL_DEVICE          0x20        // デバイス選択
#define CARDMST_SEL_ROM             0x00        // マスクＲＯＭ／３Ｄメモリ選択
#define CARDMST_SEL_SPI             0x20        // ＳＰＩ選択

#define CARDMST_IF_ENABLE           0x40        // 割り込み要求 許可
#define CARDMST_ENABLE              0x80        // カードイネーブル


//----------------------------------------------------------------------
//                      カードアクセス コントロール
//----------------------------------------------------------------------

#define CARD_LATENCY1_CYCLES_MASK   0x00001fff  // レイテンシ１のサイクル数
#define CARD_LATENCY2_CYCLES_MASK   0x003f0000  // レイテンシ２のサイクル数
#define CARD_LATENCY_MASK           0x003f1fff  // 上記を合わせたマスク

#define CARD_LATENCY1_CYCLES_SHIFT  0
#define CARD_LATENCY2_CYCLES_SHIFT  16

#define CARD_DATA_SCRAMBLE_ON      	0x00002000	// データスクランブル ＯＮ
#define CARD_SCRAMBLE_UNIT_ON      	0x00004000	// スクランブル回路 ＯＮ
#define CARD_CMD_SCRAMBLE_ON       	0x00400000	// コマンドスクランブル ＯＮ
                                              	// スクランブルフラグ群のセット
#define CARD_SCRAMBLE_SET_MASK     	( CARD_SCRAMBLE_UNIT_ON | CARD_DATA_SCRAMBLE_ON \
                                   	| CARD_CMD_SCRAMBLE_ON)

#define CARD_DATA_READY             0x00800000  // データ レディ

#define CARD_1_PAGE                 0x01000000  //   １ページ
#define CARD_STATUS                 0x07000000  // ステータスリード

#define CARD_RESET_LO               0x00000000  // リセット信号レベル Ｌｏ
#define CARD_RESET_HI               0x20000000  //                    Ｈｉ
#define CARD_ACCESS_MODE            0x40000000  // アクセス モード
#define CARD_READ_MODE              0x00000000  // リードモード
#define CARD_WRITE_MODE             0x40000000  // ライトモード
#define CARD_START                  0x80000000  // スタート

// 構造体メンバ用定数

#define ST_CARD_1_PAGE              1           //   １ページ
#define ST_CARD_STATUS              7           // ステータスリード

#define ST_CARD_READ_MODE           0           // リードモード
#define ST_CARD_WRITE_MODE          1           // ライトモード


//----------------------------------------------------------------------
//                      マスクＲＯＭコマンド
//----------------------------------------------------------------------

// GAMEモード

#define MROMOP_G_OP_MASK              0xff000000  // コマンドマスク

#define MROMOP_G_READ_ID              0xb8000000  // ＩＤ読み込み
#define MROMOP_G_READ_PAGE            0xb7000000  // ページ読み込み



#ifdef __cplusplus
}       // extern "C"
#endif

#endif  // SYSM_CARD_H_
