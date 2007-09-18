/*
** Copyright (c) 2000-2001 Matsushita Electric Industrial Co., Ltd.
** All Rights Reserved.
*/

/*
** $Module      SDカードアクセスモジュール・インクルード
** $Filename    SD_CARD_IP.H
** $Version     1.0 版
** $Date        01/02/16
** $Log         01/02/16    rev1.0作成
**              松下電器産業(株)半導体開発本部
*/


#ifndef __SD_CARD_IP_H__
#define __SD_CARD_IP_H__

//#define    IO3 0            /* Insert Remove  SW = FALSE IO3 = TRUE */

//#define    MAX_SD_CLOCK_4M        0            /* MAX SD Clock 4.608MHz */
//#define    MAX_SD_CLOCK_9M        1            /* MAX SD Clock 9.216MHz */


/*---------------------------------------------
 ラッパーレジスタのビット制御（フラグ定義）
---------------------------------------------*/
/* SD_CNTレジスタ */
#define    SDIF_CNT_USEDTC                  (0x0001)    /* DTC使用フラグ (R/W) */
#define    SDIF_CNT_USEFIFO                 (0x0002)    /* FIFO使用フラグ (R/W) */
#define    SDIF_CNT_FULL                    (0x0100)    /* FIFO FULLフラグ (RO) */
#define    SDIF_CNT_NEMP                    (0x0200)    /* FIFO NOT EMPTYフラグ (RO) */
#define    SDIF_CNT_FCLR                    (0x0400)    /* FIFO クリアフラグ (WO) */
#define    SDIF_CNT_FFIE                    (0x0800)    /* FIFO FULLで割り込み (R/W) */
#define    SDIF_CNT_FEIE                    (0x1000)    /* FIFO EMPTYで割り込み (R/W) */

#define SDCARD_UseFifoFlag                  ((*SDIF_CNT) & SDIF_CNT_USEFIFO)

/*-------------------------------------
レジスタのビット制御（フラグ定義）
-------------------------------------*/
/* SD_CMDレジスタ*/
#define    SD_CMD_CMD                       0x0000    /* SDカードへの通常アクセス(CMD) */
#define    SD_CMD_ACMD                      0x0040    /* SDカードへのセキュリティアクセス(ACMD) */

/* SD_STOPレジスタ */
#define    SD_STOP_STP                      0x0001    /* データ転送終了を知らせる */
#define    SD_STOP_SEC_ENABLE               0x0100    /* SD_SECCNTレジスタ有効（セクタカウントレジスタ） */

/* SD_SECCNTレジスタ */
#define    SD_SECCNT_END                    0x0000    /* SD_SECCNTレジスタ カウントチェック */

/* SD_SIZEレジスタ */
#define    SD_SIZE_DATA_LENGTH_1B           0x0001    /* SDカード転送データサイズ 1Bytes */
#define    SD_SIZE_DATA_LENGTH_2B           0x0002    /* SDカード転送データサイズ 2Bytes */
#define    SD_SIZE_DATA_LENGTH_4B           0x0004    /* SDカード転送データサイズ 4Bytes */
#define    SD_SIZE_DATA_LENGTH_8B           0x0008    /* SDカード転送データサイズ 8Bytes (SCR) */
#define    SD_SIZE_DATA_LENGTH_16B          0x0010    /* SDカード転送データサイズ 16Bytes */
#define    SD_SIZE_DATA_LENGTH_32B          0x0020    /* SDカード転送データサイズ 32Bytes */
#define    SD_SIZE_DATA_LENGTH_64B          0x0040    /* SDカード転送データサイズ 64Bytes (SD_Status) */
#define    SD_SIZE_DATA_LENGTH_128B         0x0080    /* SDカード転送データサイズ 128Bytes */
#define    SD_SIZE_DATA_LENGTH_256B         0x0100    /* SDカード転送データサイズ 256Bytes */
#define    SD_SIZE_DATA_LENGTH_512B         0x0200    /* SDカード転送データサイズ 512Bytes (データ) */

/* SD_OPTIONレジスタ */
#define    SD_OPTION_WIDTH_1BIT             0x8000    /* ビット幅の選択 1bit幅 */
#define    SD_OPTION_MSEL_C2NOUSE           0x4000    /* C2モジュール未使用 */
#define    SD_CD_DETECT_TIME                0xFFF0    /* CD 検出タイムだけをクリアするためのマスク */

/* SD_INFO2レジスタ */
#define    SD_INFO2_ERR_ILA                 0x8000    /* イリーガルアクセスエラー */
#define    SD_INFO2_BWE                     0x0200    /* SDカードから512byteのデータ書込み要求 */
#define    SD_INFO2_BRE                     0x0100    /* SDカードから512byteのデータ読込み要求 */
#define    SD_INFO2_ERR_ALLCLR              0x807F    /* SD Card エラーレジスタクリア */
#define    SD_INFO2_ERR_SDDAT0              0x0080    /* SD Card Busy bit */
#define    SD_INFO2_ERR_RESTIMEOUT          0x0040    /* レスポンスタイムアウトエラー */
#define    SD_INFO2_ERR_UNDERFLOW           0x0020    /* FIFO アンダーフローエラー */
#define    SD_INFO2_ERR_OVERFLOW            0x0010    /* FIFO オーバーフローエラー */
#define    SD_INFO2_ERR_TIMEOUT             0x0008    /* レスポンス以外のタイムアウトエラー */
#define    SD_INFO2_ERR_END                 0x0004    /* フレーム終了認識できないときの（END）エラー */
#define    SD_INFO2_ERR_CRC                 0x0002    /* CRC エラー */
#define    SD_INFO2_ERR_CMD                 0x0001    /* CMDエラー */

#define    SD_INFO2_RW_SET                  0x0300    /* SDカード Read/Write 要求割込み要因チェック */
#define    SD_INFO2_ERROR_SET               0x807F    /* SDカード エラー割込み要因チェック */

/* SD_INFO2_MASKレジスタ */
#define    SD_INFO2_MASK_ILA                0x8000    /* イリーガルアクセスエラー割込みマスク */
#define    SD_INFO2_MASK_BWE                0x0200    /* SDカードからのデータ書込み要求割込み禁止 */
#define    SD_INFO2_MASK_BRE                0x0100    /* SDカードからのデータ読込み要求割込み禁止 */
#define    SD_INFO2_MASK_ALLERRMASK         0x807F    /* 全エラー割り込み禁止 */
#define    SD_INFO2_MASK_EXCEPT_OVERFLOW    0x802F    /* 全エラー割り込み禁止 FIFO Overflow Errorを除く */
#define    SD_INFO2_MASK_RESTIMEOUT         0x0040    /* Time out 割込みEnable */
#define    SD_INFO2_MASK_UNDERFLOW          0x0020    /* FIFO アンダーフロー 割込みEnable */
#define    SD_INFO2_MASK_OVERFLOW           0x0010    /* FIFO オーバーフロー 割込みEnable */
#define    SD_INFO2_MASK_TIMEOUT            0x0008    /* Time out 割込みEnable */
#define    SD_INFO2_MASK_END                0x0004    /* END エラー 割込みEnable */
#define    SD_INFO2_MASK_CRC                0x0002    /* CRC エラー 割込みEnable */
#define    SD_INFO2_MASK_CMD                0x0001    /* CMD エラー 割込みEnable */
#define    SD_INFO2_MASK_ERRSET             0x807F    /* SDカード エラー割込み要因チェック */

/* SD_INFO1レジスタ */
#define    SD_INFO1_DAT3DETECT              0x0400    /* (IO3検出) card detect(検出=1) : CTRでは使用できない*/
#define    SD_INFO1_DAT3INSERT              0x0200    /* (IO3検出) card inserted(挿入=1) : CTRでは使用できない */
#define    SD_INFO1_DAT3REMOVE              0x0100    /* (IO3検出) card removed(抜け=1) : CTRでは使用できない */
#define    SD_INFO1_DAT3INIT                0x0300    /* (IO3検出) の初期化 */
#define    SD_INFO1_WRITEPROTECT            0x0080    /* write protect(書き込み禁止=1) */
#define    SD_INFO1_DETECT                  0x0020    /* card detect(検出=1) */

#define    SD_INFO1_INSERT                  0x0010    /* card inserted(挿入=1) */
#define    SD_INFO1_REMOVE                  0x0008    /* card removed(抜け=1) */

#define    SD_INFO1_ALL_END                 0x0004    /* R/W access all end */
#define    SD_INFO1_RES_END                 0x0001    /* Response end */
#define    SD_INFO1_INIT                    0x0005    /* SD Cardの状態を初期化 */

#define    SD_INFO1_SET                     0x031D    /* SDカード 挿抜 and RWアクセス終了 and レスポンス終了 要求割込み要因チェック */

/* SD_INFO1_MASKレジスタ (0:割り込み許可、1:割り込み禁止)*/
#define    SD_INFO1_MASK_DAT3INSERT         0x0200    /* (IO3検出) card inserted(挿入) 割込み禁止 */
#define    SD_INFO1_MASK_DAT3REMOVE         0x0100    /* (IO3検出) card removed(抜け) 割込み禁止 */
#define    SD_INFO1_MASK_INSERT             0x0010    /* card inserted(挿入) 割込み禁止 */
#define    SD_INFO1_MASK_REMOVE             0x0008    /* card removed(抜け) 割込み禁止 */
#define    SD_INFO1_MASK_ALL_END            0x0004    /* R/W access all end 割込み禁止 */
#define    SD_INFO1_MASK_RES_END            0x0001    /* Response end 割込み禁止 */

/* CC_EXT_MODEレジスタ */
#define CC_EXT_MODE_PIO                     0x0000    /* PIOモード */
#define CC_EXT_MODE_DMA                     0x0002    /* DMAモード */

/* SOFT_RSTレジスタ */
#define    SOFT_RST_SDIF_RST                0x0001    /* SD I/Fモジュールをリセット */

/* SD_CLK_CTRLレジスタ */
#define    SD_CLK_CTRL_SDCLKEN              0x0100    /* SDカードクロック出力イネーブル */
#define    SD_CLK_CTRL_512                  0x0180    /* SDクロックの周波数（分周比512）*/
#define    SD_CLK_CTRL_256                  0x0140    /* SDクロックの周波数（分周比256）*/
#define    SD_CLK_CTRL_128                  0x0120    /* SDクロックの周波数（分周比128）*/
#define    SD_CLK_CTRL_64                   0x0110    /* SDクロックの周波数（分周比 64）*/
#define    SD_CLK_CTRL_32                   0x0108    /* SDクロックの周波数（分周比 32）*/
#define    SD_CLK_CTRL_16                   0x0104    /* SDクロックの周波数（分周比 16）*/
#define    SD_CLK_CTRL_8                    0x0102    /* SDクロックの周波数（分周比  8）*/
#define    SD_CLK_CTRL_4                    0x0101    /* SDクロックの周波数（分周比  4）*/
#define    SD_CLK_CTRL_2                    0x0100    /* SDクロックの周波数（分周比  2）*/


/*-------------------------------------
マルチポート対応ビット制御（フラグ定義）
-------------------------------------*/
#define    SDCARD_PORT_NO                   0x0300    /* カードポート選択数         */
#define    SDCARD_PORT_NO_MAX               0x04//0x02    /* カードポート最大選択数    */
#define    SDCARD_PORT_NO_MIN               0x01      /* カードポート最小選択数    */
#define    SDCARD_PORT_SELECT_NO            0x0001    /* カードポート番号ビット     */
#define    SDCARD_PORT0                     0x0000    /* カードポート番号ビット     */
#define    SDCARD_PORT1                     0x0001    /* カードポート番号ビット     */

/* EXT_WPレジスタ（ポート1以降のライトプロテクト） */
#define    EXT_WP_PORT1                     0x0001    /* ポート1write protect(書き込み禁止=1)*/

/* EXT_CDレジスタ */
#define    EXT_CD_PORT1_REMOVE              0x0001    /* ポート1 card detect（検出=1） */
#define    EXT_CD_PORT1_INSERT              0x0002    /* ポート1 card inserted（挿入=1） */
#define    EXT_CD_PORT1_DETECT              0x0004    /* ポート1 card removed（抜け=1） */

/* EXT_CD_DAT3レジスタ */
//#define EXT_CD_PORT1_DAT3INIT        0x0003    /* ポート1 card の状態 (IO3検出) の初期化 */
#define    EXT_CD_PORT1_DAT3REMOVE          0x0001    /* ポート1 dat3 card detect（検出=1） */
#define    EXT_CD_PORT1_DAT3INSERT          0x0002    /* ポート1 dat3 card inserted（挿入=1） */
#define    EXT_CD_PORT1_DAT3DETECT          0x0004    /* ポート1 dat3 card removed（抜け=1) */

/* EXT_CD_DAT3_MASKレジスタ */
#define    EXT_CD_MASK_PORT1INSERT          0x0002    /* port1 card inserted(挿入) 割込み禁止 */
#define    EXT_CD_MASK_PORT1REMOVE          0x0001    /* port1 card removed(抜け) 割込み禁止 */
#define    EXT_CD_MASK_PORT1DAT3INSERT      0x0002    /* port1 (IO3検出) card inserted(挿入) 割込み禁止 */
#define    EXT_CD_MASK_PORT1DAT3REMOVE      0x0001    /* port1 (IO3検出) card removed(抜け) 割込み禁止 */

/*-------------------------------------
カードの選択
--------------------------------------*/
#define    SDCARD_DETECT_VISUAL_BIT         0x0400    /***    upper layer card detected visual bit                     ***/

/*-------------------------------------
ライトプロテクト
-------------------------------------*/
#define    SDCARD_WP_PERMANENT_BIT          0x0020    /*** permanent write protection        ***/
#define    SDCARD_WP_TEMPORARY_BIT          0x0010    /*** temporary write protection        ***/

/*-------------------------------------
SD or MMCカードコマンド
-------------------------------------*/
#define    GO_IDLE_STATE            (0)    /* resets all cards to idle state */
#define    SEND_OP_COND             (1)    /* Asks all cards in idle state to send their operation conditions */
#define    ALL_SEND_CID             (2)    /* send CID numbers */
#define    SEND_RELATIVE_ADDR       (3)    /* ask the card to publish a new relative address(RCA) */
#define    SET_BUS_WIDTH            (6)    /* ビット幅の選択 */
#define    SELECT_CARD              (7)    /* Command toggles acard between thr Stand-by and Transfer states */
#define    SEND_CSD                 (9)    /* addressed card sends its card-specific data(CSD) */
#define    STOP_TRANSMISSION        (12)   /* forces the card to stop transmission */
#define    SD_SEND_STATUS           (13)   /* addressed card sends its status register */
#define    SET_BLOCKLEN             (16)   /* sets the block length */
#define    READ_MULTIPLE_BLOCK      (18)   /* マルチブロックリード */
#define    WRITE_MULTIPLE_BLOCK     (25)   /* マルチブロックライト */
#define    APP_CMD                  (55)   /* CMD55 */
#define    SD_STATUS                (13)   /* ACMD13 Send the SD_CARD status */
#define    SEND_NUM_WR_SECTORS      (22)   /* ACMD22 書きこみ完了セクタ数取得 */
#define    SD_APP_OP_COND           (41)   /* ACMD41 */
#define    SEND_SCR                 (51)   /* SD configuration register (SCR) */

/* MMCplus, eMMCの定義 */
#define    EXT_CSD_ACCESS           (6)

/* Extended Commandの定義 */
#define    EXT_NORMAL               (0)
#define    EXT_SDIO                 (0x4000)

#define    EXT_COM_R3               (0x0700)

#define    EXT_CMD                  (0x00C0)

#define    SEND_IF_COND             (8)    /* Physical Layer 2.0 で追加されたコマンド */
#define    SEND_IF_COND_EXT         (EXT_SDIO | EXT_COM_R3 | EXT_CMD | SEND_IF_COND)


/*-------------------------------------
IP レジスタアクセス マクロ関数
-------------------------------------*/
#define    SD_OrFPGA(reg,value)      ((reg) |= (value)); 
#define    SD_AndFPGA(reg,value)     ((reg) &= (value));
#define    SD_SetFPGA(reg,value)     ((reg) = (value));
#define    SD_GetFPGA(dest,reg)      ((dest) = (reg));

/***********************************************************************
    構造体定数
***********************************************************************/
typedef    union{
    u32    dat;
    struct    {
        u16    low;
        u16    high;
    }    dt2word;
}    LELONG;

/*-------------------------------------
その他（ビット制御）
-------------------------------------*/
#define    RSP_R3_OCR31                 0x8000        /* Use OCR Busy bit Check */
#define    RSP_R3_OCR_VDD               0x0030        /* Use OCR VDD bit Check (3.2-3.3v,3.3-3.4v is OK)*/

/*--- SCR[0]に対する4bitバス幅対応フラグのマスク ---*/
//SCRのbit50に相当するが、SCRとSD_STATUSはMSBから送られてくることを考慮してある
#define    SCR_DAT_BUS_WIDTH_4BIT       0x0400        /* SCR DAT Bus width supported 4bit */

/*--- 127bitCSD (CSD[0]～CSD[7])関連 ---*/
#define    CSD_VDD_R_CURR_MIN           0x0038        /* "VDD_R_CURR_MIN" (for CSD[3]) */
#define    CSD_VDD_R_CURR_MAX           0x0007        /* "VDD_R_CURR_MAX" (for CSD[3]) */
#define    CSD_VDD_W_CURR_MIN           0xE000        /* "VDD_W_CURR_MIN" (for CSD[2]) */
#define    CSD_VDD_W_CURR_MAX           0x1C00        /* "VDD_W_CURR_MAX" (for CSD[2]) */

#define    CSD_READ_BL_LEN              0x0F00        /* "READ_BL_LEN" (for CSD[4])*/
#define    CSD_WRITE_BL_LEN_BIT_25_24   0x0003        /* "WRITE_BL_LEN" (for CSD[1])*/
#define    CSD_WRITE_BL_LEN_BIT_23_22   0xC000        /* "WRITE_BL_LEN" (for CSD[0]*/
#define    CSD_READ_BL_PARTIAL          0x0080        /* "READ_BL_PARTIAL" (for CSD[4]) */
#define    CSD_TRANSFER_RATE            0x0700        /* "Transfer rate unit" of "TRAN_SPEED" (for CSD[5]) */
#define    CSD_TRAN_SPEED_100K          0x0000        /* 100Kbit/s (for CSD Transfer rate) */
#define    CSD_TRAN_SPEED_1M            0x0001        /* 1Mbit/s (for CSD Transfer rate) */
#define    CSD_TRAN_SPEED_10M           0x0002        /* 10Mbit/s (for CSD Transfer rate) */
#define    CSD_TRAN_SPEED_100M          0x0003        /* 100Mbit/s (for CSD Transfer rate) */
#define    CSD_TRAN_SPEED_OTHER         0x0004        /* Reserve (for CSD Transfer rate) */

#define    CSD_C_SIZE_MULT              0x0380        /* RSP2 の bit[49:47] */
#define    CSD_C_SIZE_BIT_73_72         0x0003        /* RSP3 の bit[73:72](C_SIZE) */
#define    CSD_C_SIZE_BIT_71_62         0xFFC0        /* RSP3 の bit[71:62](C_SIZE) */
//SDHC(CSD format version2)の場合
#define    CSD_C_SIZE_BIT_69_56         0x3FFF        /* SD_CSD[3] */
#define    CSD_C_SIZE_BIT_55_48         0xFF00        /* SD_CSD[2] */

#define    CSD_STRUCT_BIT_127_126       0x00C0        /* SD_CSD[7] */

//#define    VDD_R_CURR_MIN                0x0000        /* CSD max read current@VDD min */
//#define    VDD_R_CURR_MAX                0x0007        /* CSD max read current@VDD max */
//#define    VDD_W_CURR_MIN                0x0000        /* CSD max write current@VDD min */
//#define    VDD_W_CURR_MAX                0x0007        /* CSD max write current@VDD max */

/*--- R1レスポンスの card status(32bit)に対するマスク ---*/
#define    RSP_R1_STATUS_ERR            0xF9FF0008    /* R1（レスポンス）のカードステータスのチェック */
#define    SDCARD_STATUS_OUT_OF_RANGE   0x80000000    /* Card Status OUT_OF_RANGE のチェック */
#define    RSP_R1_CURRENT_STATE         0x1E00        /* CARD current state */
/*-------------------------------------------------------*/

/* レスポンスのRSP0 & RSP_R1_CURRENT_STATEを1ビット右シフトした値に対するフラグ */
#define    CURRENT_STATE_DATA           0x0500        /* CARD current state data */
#define    CURRENT_STATE_RCV            0x0600        /* CARD current state rcv */

/* カードステータス */
#define    SD_MEMORY_CARD               0x00FF        /* SD_CARD_TYPE SD memory card */


/*-------------------------------------
プロトタイプ宣言
-------------------------------------*/
void    SD_Init(void);                          /* SD Cardインターフェース部をリセット&初期設定 */
void    SD_EnableInfo(void);                    /* SD Card 挿抜 割り込みイネーブル・ディスエーブル */
u16     SD_Command(u16 ucCommand);              /* SDカードコマンド送出 */
u16     SD_AppCommand(void);                    /* SDカード RCA = 1をセット後 CMD55 発行 */
u16     SD_AppOpCond(void);                     /* ACMD41 発行 busyでなくなるまで繰り返し */
u16     SD_SendOpCond(void);                    /* CMD1 発行 busyでなくなるまで繰り返し */
u16     SD_SendIfCond(void);                    /* CMD8 発行 (SDHCのみ反応してくる) */
u16     SD_SendRelativeAddr(void);              /* CMD3 発行 正常終了時 RCA<-ResのRCA */
u16     SD_SelectCard(void);                    /* CMD7 発行 Command toggles acard between the Stand-by and Transfer states */
u16     SD_SetBlockLength(u32 ulBlockLength);   /* ブロックレングス(1セクタの転送量)の設定 */
u16     SD_SendCID(void);                       /* card identification data の取得コマンド発行 */
u16     SD_SendCSD(void);                       /* card-specific data の取得コマンド発行 */
u16     SD_SendSCR(void);                       /* SD register の取得コマンド発行 */
u16     SD_SDStatus(void);                      /* SD STATUS の取得コマンド発行 */
u16     SD_SendStatus(void);                    /* SD status register の取得コマンド発行 */
u16     SD_MultiReadBlock(u32 ulOffset);        /* マルチセクタリードコマンド発行 */
u16     SD_ClockDivSet(u16 usTranSpeed);        /* カードの動作クロック設定 */

void    SD_EnableClock( void);                  /* SDカードのクロック有効 */
void    SD_DisableClock( void);                 /* SDカードのクロック無効（省電力） */

u16     SD_SelectBitWidth(s16 b4bit);           /* ビット幅の選択 */

u16     MMCP_WriteBusWidth(s16 b4bit);
u16     MMCP_BusTest( BOOL readflag);


BOOL    SD_FPGA_irq(void);                      /* カード転送要求時のFPGAの制御 */
void    SD_StopTransmission(void);              /* カード転送終了をFPGAに通知する。 */
void    SD_TransEndFPGA(void);                  /* カード転送の終了処理(割り込みマスクを戻す) */
u16     SD_CheckStatus(BOOL bRead);             /* Normal response command カードステータスのチェック */
u16     SD_SwapByte(u16 *data);                 /* 上位byte、下位byteを入れ替える関数 */

void    SD_EnableSeccnt( u32 ulSDCARD_SectorCount); /* SD_SECCNTレジスタ有効化&値設定 */
void    SD_DisableSeccnt( void);                    /* SD_SECCNTレジスタ無効化 */

void    SD_SetErr(u16 Error);                   /* エラーステータスを設定する */
void    SD_ClrErr(u16 Error);                   /* エラーステータスをクリアする */

BOOL    SD_CheckFPGAReg(u16 reg,u16 value);     /* IPレジスタにフラグが立っているか判定 */

void    SD_TransReadyFPGA(void);                /* 転送処理準備FPGA設定 */
u16     SD_TransCommand(u16 ucCommand);         /* 命令発行処理 */

u16     SD_MultiWriteBlock(u32 ulOffset);       /* マルチセクタライトコマンド発行 */
u16     SD_SendNumWRSectors(void);              /* ACMD22 書きこみ完了セクタ数取得コマンド発行 */


#endif /* __SD_CARD_IP_H__ */
