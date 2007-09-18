/*
** Copyright (c) 2000-2001 Matsushita Electric Industrial Co., Ltd.
** All Rights Reserved.
*/

/*
** $System      IP1.1 without C2 サンプルソフト
** $Subsystem   カードドライバ
** $Filename    CARDDRV.H
** $Version     1.0 版
** $Date        01/02/16
** $Log         01/02/16    rev1.0作成
**              松下電器産業(株)半導体開発本部
*/


#ifndef __SDMC_CONFIG_H__
#define __SDMC_CONFIG_H__


#include <firm.h>

#ifdef    USE_OS
#include <sd_ip_reg.h>                        /* IP 対応レジスタ定義 */
#endif



/*********************************************
 ターゲットOS
*********************************************/
#define    TARGET_OS_CTR                (0)
#define    TARGET_OS_NITRO              (TARGET_OS_CTR ^ 1)


/*********************************************
 SDドライバ コンフィグレーション
*********************************************/
#define    SD_DEBUG_PRINT_ON            0        /* デバッグ表示 */

#define    WP_ena                       1        /* ライトプロテクトのチェック有効 */
#define    TIMEOUT                      1        /* FPGA Timeout none = FALSE */
#define    SCR                          1        /* Send SCR Command = TRUE */
#define    RESID                        1        /* Write Error Resid enable = TRUE */
#define    ATC_ON                       0        /* ATC転送 使用/未使用 */

#define    SecEnable                    1        /* SD_SECCNTレジスタ Enable */
#define    SecDisenable                 0        /* SD_SECCNTレジスタ Disable */
//#define    STANDBYMODE                  0x04    /*** 5772 standby control bit    ***/

#define    SDCARD_TIMER_ID  (OS_TIMER_3)


/*********************************************
 タイムアウト設定値(ms単位)
*********************************************/
#define    SDCARD_RW_TIMEOUT            (2000)
#define    SDCARD_STDBY_TIMEOUT         (50)
#define    SDCARD_CLOCK_WAIT            (500)
#define    SDCARD_SDCLK_WAIT            (10)
#define    SDCARD_INITIAL_TIMEOUT       (800)
#define    SDCARD_RESET_TIMEOUT         (1500)
#define    SDCARD_ERASE_TIMEOUT         (1)
#define    SDCARD_ERRPROC_TIMEOUT       (2000)


/*********************************************
 リトライ回数(Multiple Block R/W のとき)
*********************************************/
#define    SDCARD_RETRY_COUNT           (3)

/*********************************************
 その他
*********************************************/
#define    SECTOR_SIZE                  (512)       /* 1セクタのバイト数 */
#define    SECTOR_MAX                   (255)       /* SYSFPGA アクセス最大セクタ数 */

/*--- 上位レイヤに返すステータス値(SDCARD_Getstatus参照)用 ---*/
#define    SDCARD_FLAG_CLR              (0x3FFF)    /* カード判定部分クリア用 */
#define    SDCARD_FLAG_SD               (0x8000)    /* カード判定部分SDカード */
#define    SDCARD_FLAG_MMC              (0x4000)    /* カード判定部分MMCカード */
#define    SDCARD_PORT1_CLR             (0x0007)    /* カードポート1判定部分クリア用 */




#endif /*__SDMC_CONFIG_H__*/
