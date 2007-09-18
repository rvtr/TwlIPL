/*---------------------------------------------------------------------------*
  Project:  TwlBrom - SD driver
  File:     sd_ip_reg.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef __SD_IP_REG_H__
#define __SD_IP_REG_H__

/*********************************************
 SD IPレジスタ
    
 (R/W) : readable and writable
 (RO)  : read only
*********************************************/
/*#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
#define    SD_IP_BASE          (0x08030000)    // NTR用ブレッドボード設定
#else
#define    SD_IP_BASE          (0x400B0000)    // IOP実機設定
#endif*/
#define    SD_IP_BASE          (0x04004800)    // TWL ARM7設定


#define    SD_CMD              (*(vu16 *)(SD_IP_BASE + 0x00))    /* CMD発行レジスタ(R/W) */
#define    SD_PORTSEL          (*(vu16 *)(SD_IP_BASE + 0x02))    /* ポート選択レジスタ(R/W) */
#define    SD_ARG0             (*(vu16 *)(SD_IP_BASE + 0x04))    /* Argument[15:0]  (R/W) */
#define    SD_ARG1             (*(vu16 *)(SD_IP_BASE + 0x06))    /* Argument[31:16] (R/W) */
#define    SD_STOP             (*(vu16 *)(SD_IP_BASE + 0x08))    /* 転送終了後STOP、SD_SECCNT有効 (R/W) */
#define    SD_SECCNT           (*(vu16 *)(SD_IP_BASE + 0x0A))    /* 転送セクタ数制御  (R/W) */
#define    SD_RSP0             (*(vu16 *)(SD_IP_BASE + 0x0C))    /* Response [23:8]    (RO) */
#define    SD_RSP1             (*(vu16 *)(SD_IP_BASE + 0x0E))    /* Response [39:24]   (RO) */
#define    SD_RSP2             (*(vu16 *)(SD_IP_BASE + 0x10))    /* Response [55:40]   (RO) */
#define    SD_RSP3             (*(vu16 *)(SD_IP_BASE + 0x12))    /* Response [71:56]   (RO) */
#define    SD_RSP4             (*(vu16 *)(SD_IP_BASE + 0x14))    /* Response [87:72]   (RO) */
#define    SD_RSP5             (*(vu16 *)(SD_IP_BASE + 0x16))    /* Response [103:88]  (RO) */
#define    SD_RSP6             (*(vu16 *)(SD_IP_BASE + 0x18))    /* Response [119:104] (RO) */
#define    SD_RSP7             (*(vu16 *)(SD_IP_BASE + 0x1A))    /* Response [127:120] (RO) */
#define    SD_INFO1            (*(vu16 *)(SD_IP_BASE + 0x1C))    /* SDメモリーカードの状態 (R/W) */
#define    SD_INFO2            (*(vu16 *)(SD_IP_BASE + 0x1E))    /* バッファ制御とエラー情報 (R/W) */
#define    SD_INFO1_MASK       (*(vu16 *)(SD_IP_BASE + 0x20))    /* SD_INFO1割込みマスク (R/W) */
#define    SD_INFO2_MASK       (*(vu16 *)(SD_IP_BASE + 0x22))    /* SD_INFO2割込みマスク (R/W) */
#define    SD_CLK_CTRL         (*(vu16 *)(SD_IP_BASE + 0x24))    /* SDクロック設定 (R/W) */
#define    SD_SIZE             (*(vu16 *)(SD_IP_BASE + 0x26))    /* ビット幅と転送データ長の設定 (R/W) */
#define    SD_OPTION           (*(vu16 *)(SD_IP_BASE + 0x28))    /* タイムアウト、width、CD検出タイム (R/W) */
#define    SD_ERR_STS1         (*(vu16 *)(SD_IP_BASE + 0x2C))    /* CMD, CRC, ENDエラー割込み原因 (RO) */
#define    SD_ERR_STS2         (*(vu16 *)(SD_IP_BASE + 0x2E))    /* タイムアウトエラー割込み原因  (RO) */
#define    SD_BUF0             (vu16 *)(SD_IP_BASE + 0x30)       /* SDバッファ読込／書込データポート (R/W) */
#define    CC_EXT_MODE         (*(vu16 *)(SD_IP_BASE + 0xD8))    /* DMAモード/PIOモード切り替え */
#define    SOFT_RST            (*(vu16 *)(SD_IP_BASE + 0xE0))    /* ソフトウェアリセット (R/W) */
#define    VERSION             (*(vu16 *)(SD_IP_BASE + 0xE2))    /* Version レジスタ (RO) */
#define    EXT_WP              (*(vu16 *)(SD_IP_BASE + 0xF6))    /* 拡張SD Card ライトプロテクト (RO) */
#define    EXT_CD              (*(vu16 *)(SD_IP_BASE + 0xF8))    /* 拡張SD Card 検出、挿入、抜け フラグ (R/W) */
#define    EXT_CD_DAT3         (*(vu16 *)(SD_IP_BASE + 0xFA))    /* 拡張SD Card 検出、挿入、抜け フラグ (R/W) */
#define    EXT_CD_MASK         (*(vu16 *)(SD_IP_BASE + 0xFC))    /* 拡張SD Card 検出、挿入、抜け 割込みマスク (R/W) */
#define    EXT_CD_DAT3_MASK    (*(vu16 *)(SD_IP_BASE + 0xFE))    /* 拡張SD Card 検出、挿入、抜け 割込みマスク (R/W) */



/*********************************************
 SD I/F（ラッパー）レジスタ
*********************************************/
/*#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
#define    SD_IF_BASE          (0x08030100)    // NTR用ブレッドボード設定
#else
#define    SD_IF_BASE          (0x400B0100)    // IOP実機設定
#endif*/
#define    SD_IF_BASE          (0x04004900)    // IOP実機設定


#define    SDIF_CNT            ((vu32 *)(SD_IF_BASE+0x00))    /* コントロール */
#define    SDIF_FDS            ((vu32 *)(SD_IF_BASE+0x04))    /* FIFOサイズ */
#define    SDIF_FSC            ((vu32 *)(SD_IF_BASE+0x08))    /* セクタカウント */
#define    SDIF_FI             ((vu32 *)(SD_IF_BASE+0x0c))    /* FIFOウィンドウ */

#define    SDIF_CNT_L          ((vu16 *)(SD_IF_BASE+0x00))
#define    SDIF_CNT_H          ((vu16 *)(SD_IF_BASE+0x02))
#define    SDIF_FDS_L          ((vu16 *)(SD_IF_BASE+0x04))
#define    SDIF_FDS_H          ((vu16 *)(SD_IF_BASE+0x06))
#define    SDIF_FSC_L          ((vu16 *)(SD_IF_BASE+0x08))
#define    SDIF_FSC_H          ((vu16 *)(SD_IF_BASE+0x0a))
#define    SDIF_FI_L           ((vu16 *)(SD_IF_BASE+0x0c))
#define    SDIF_FI_H           ((vu16 *)(SD_IF_BASE+0x0e))



/*********************************************
 INTCレジスタ
*********************************************/
/*#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
#define    CTR_INT_BASE      (0x08000000)
#else
#define    CTR_INT_BASE      (0x40010000)
#endif


#define    CTR_INT_SE        (CTR_INT_BASE + 0x00)    //割り込み要求選択レジスタ
#define    CTR_INT_IE        (CTR_INT_BASE + 0x04)    //IE
#define    CTR_INT_IF        (CTR_INT_BASE + 0x08)    //IF

#define    CTR_IE_SD_MASK    (1<<13)                  //SD割り込みフラグ
*/
#define    CTR_INT_BASE      (0x04000000)

#define    CTR_INT_SE        (CTR_INT_BASE + 0x208)    //割り込み要求選択レジスタ
#define    CTR_INT_IE        (CTR_INT_BASE + 0x218)    //IE
#define    CTR_INT_IF        (CTR_INT_BASE + 0x21C)    //IF

#define    CTR_IE_SD_MASK    (1<<8)                  //SD割り込みフラグ


#endif /* __SD_IP_REG_H__ */
