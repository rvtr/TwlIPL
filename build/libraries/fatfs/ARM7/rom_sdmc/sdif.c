/*
  Project:  TwlBrom SD Card driver
  File:     sd_card.c

  2007, Research and Development Department, Nintendo.
*/

#include <brom.h>
//#include "sdmc_config.h"
//#include "sdif_reg.h"       /*  IP 対応レジスタ定義 */
//#include "sdmc.h"
//#include "sdif_ip.h"        /*  IP 対応フラグ定義 */


/*
#if (SD_DEBUG_PRINT_ON == 1)
    #if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
        #define PRINTDEBUG    osTPrintf
    #else
        #include <ctr/vlink.h>
        #define PRINTDEBUG    vlink_dos_printf
    #endif
#else
    #define PRINTDEBUG( ...) ((void)0)
#endif
*/
//        #define PRINTDEBUG    OS_TPrintf
    #define PRINTDEBUG( ...) ((void)0)


#define ADD_CHECK 1

/***********************************************************************
    グローバル
***********************************************************************/
u16    SD_CID[8];        /* CID保存用 (Card IDentification register) : ID*/
u16    SD_CSD[8];        /* CSD保存用 (Card Specific Data register) : spec*/
u16    SD_OCR[2];        /* OCR保存用 (Operation Condition Register) : voltage and status*/
u16    SD_SCR[4];        /* SCR保存用 (Sd card Configulation Register) : bus-width, card-ver, etc*/
u16    SD_RCA;           /* RCA保存用 (Relative Card Address register) : address*/

s16    SDCARD_MMCFlag;   /* MMCカードフラグ */
s16    SDCARD_SDHCFlag;  /* SDHCカードフラグ(ここではPhysicalLayer2.0の意) */
u16    SD_port_number;   /* 現在のポート番号 */

/***********************************************************************
    外部参照変数
***********************************************************************/
volatile SDMC_ERR_CODE   SDCARD_ErrStatus;        /* エラーステータス */
vu32            SDCARD_Status;           /* カードステータス */
//extern volatile s16    SDCARD_OutFlag;    /* カード排出発生判定フラグ */
//extern void (*func_SDCARD_Out)(void);     /* カード排出イベント用コールバック保存用 */

extern void SDCARD_TimerCheck( void);
extern void SYSFPGA_irq(void);





void SD_DisableInfo( void);



/*---------------------------------------------------------------------------*
  Name:         SD_Init

  Description:  reset and initialize SD card interface
                SDカードIPのリセットと初期設定

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_Init(void)
{
    SD_AndFPGA( SOFT_RST,(~(SOFT_RST_SDIF_RST))); /* SD I/F モジュールをリセット */
    SD_OrFPGA( SOFT_RST,((SOFT_RST_SDIF_RST)));   /* SD I/F モジュールをリセット復帰 */

    SD_AndFPGA( SD_STOP,(~SD_STOP_STP));          /* データ転送終了クリア */
}

/*---------------------------------------------------------------------------*
  Name:         SD_EnableInfo

  Description:  enable SD card insert and remove interrupts.
                SDカードの挿入/抜取 割り込みを許可する

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_EnableInfo( void)
{
    if(SD_port_number == SDCARD_PORT0)
    {
        SD_AndFPGA(SD_INFO1,(~(SD_INFO1_INSERT | SD_INFO1_REMOVE))); /* SD_INFO1レジスタの card inserted removedをクリア */
        SD_AndFPGA(SD_INFO1_MASK,(~(SD_INFO1_MASK_INSERT | SD_INFO1_MASK_REMOVE))); /* 挿/抜 割り込み許可 */
    }
    else if(SD_port_number == SDCARD_PORT1)
    {    /* ポート1はCD端子が繋がっていないので実質無効 */
        SD_AndFPGA(EXT_CD,(~(EXT_CD_PORT1_INSERT | EXT_CD_PORT1_REMOVE)));                /* EXT_CD レジスタの card inserted removedをクリア */
        SD_AndFPGA(EXT_CD_MASK,(~(EXT_CD_MASK_PORT1INSERT | EXT_CD_MASK_PORT1REMOVE)));    /* 挿抜 割り込み許可 */
    }
}
/*---------------------------------------------------------------------------*
  Name:         SD_DisableInfo

  Description:  disable SD card insert and remove interrupts.
                SDカードの挿入/抜取 割り込みを禁止する

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_DisableInfo( void)
{
    if(SD_port_number == SDCARD_PORT0)
    {
        SD_AndFPGA(SD_INFO1,(~(SD_INFO1_INSERT | SD_INFO1_REMOVE))); /* SD_INFO1レジスタの card inserted removedをクリア */
        SD_OrFPGA(SD_INFO1_MASK,(SD_INFO1_MASK_INSERT | SD_INFO1_MASK_REMOVE)); /* 挿/抜 割り込み禁止 */
    }
    else if(SD_port_number == SDCARD_PORT1)
    {    /* ポート1はCD端子が繋がっていないので実質無効 */
        SD_AndFPGA(EXT_CD,(~(EXT_CD_PORT1_INSERT | EXT_CD_PORT1_REMOVE))); /* EXT_CD レジスタの card inserted removedをクリア */
        SD_OrFPGA(EXT_CD_MASK,(EXT_CD_MASK_PORT1INSERT | EXT_CD_MASK_PORT1REMOVE)); /* 挿抜 割り込み禁止 */
    }
}

/*---------------------------------------------------------------------------*
  Name:         SD_Command

  Description:  send command that the card will response only.
                コマンドを送出する（レスポンスが返ってくるだけのコマンド用）

  Arguments:    ucCommand : command number

  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
u16 SD_Command(u16 ucCommand)
{
    PRINTDEBUG( "command\n");
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_ERR_ALLCLR));           /* SD Card I/F の 全エラーをクリア */
    SD_AndFPGA(SD_INFO1,(~SD_INFO1_RES_END));              /* SD_INFO1レジスタの Response end クリア */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_ALLERRMASK)); /* SD Card I/F の 全エラー割込み許可 */

    SD_SetFPGA(SD_CMD,(ucCommand));                        /* コマンド発行 */

    while(!SD_CheckFPGAReg(SD_INFO1,SD_INFO1_RES_END)){    /* Response end 待ち */
        /******************/
        SDCARD_TimerCheck();
        /******************/
        if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IPエラー発生?
            SYSFPGA_irq();
            break;
        }
        if( SDCARD_ErrStatus != 0) {
            break;
        }
    }

    /*ROM版はエラー割り込みが入らないためRES_ENDが立つとエラーを見逃すことがあるので最後にINFO2評価*/
    if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IPエラー発生?
        SYSFPGA_irq();
    }
  
    SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_ALLERRMASK));   /* SD Card I/F の全エラー割込み禁止 */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_AppCommand

  Description:  send apprication command.
                アプリケーションコマンド(CMD55)を送出する

  Arguments:    None

  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
u16 SD_AppCommand(void)
{
    PRINTDEBUG( " CMD55 (APP_CMD)\n");

    /* argumentをセット */
    SD_SetFPGA(SD_ARG0,(0x0000));            /* Argument(15:0) = stuff bits */
    SD_SetFPGA(SD_ARG1,(SD_RCA));            /* Argument(31:16) = RCA */

    SD_Command(SD_CMD_CMD | APP_CMD);        /* CMD55発行、レスポンス(R1)待ち */

#if ADD_CHECK
    /*SD_CheckStatusを行うと、直前にCMD8を発行していた場合、SDHC以外はCardStatusの
      IllegalCommandエラーフラグが立ってしまうので、ここで引っかかることになる
     （このフラグがクリアされるのは1コマンドぶん遅れるため）
      SD Physical Layer 仕様書の Card Status参照*/
//    SD_CheckStatus(FALSE);                 /* コマンドレスポンス(R1) の Card Status チェック */
#endif

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_AppOpCond

  Description:  get operating condition register data of SD card.
                SDカードのACMD41を発行してOCRを取得する

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_AppOpCond(void)
{
    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));      /* タイムアウト以外のエラーをクリア */

    while(!SDCARD_ErrStatus){                      /* エラーが発生しない間は繰り返し */
        /* Argument(31:0) = OCR without busy (0x00100000 = 3.2-3.3V) */
        /*ホスト側で電圧を選択できる場合などは、初回にSD_ARG1を0にすることにより
          カードが対応している電圧を問い合わせることができる。CTRは電圧3.3V決め打ち
          なので問い合わせせず、いきなり3.3V対応をカードに要求する。対応できない
          カードは Inactive Mode に移行する。*/
        SD_SetFPGA(SD_ARG0,(0x0000));
        if( SDCARD_SDHCFlag) {
            SD_SetFPGA(SD_ARG1,(0x4010));
        }else{
            SD_SetFPGA(SD_ARG1,(0x0010));
        }

        PRINTDEBUG( " ACMD41 (SD_SEND_OP_COND)\n");
        SD_Command(SD_CMD_ACMD | SD_APP_OP_COND);  /* ACMD41発行、レスポンス(R3)待ち */

        if(!SDCARD_ErrStatus){                     /* エラーステータスの確認（エラー無し？）*/
            SD_GetFPGA(SD_OCR[0],SD_RSP0);         /* レスポンス(R3)からOCR取得 */
            SD_GetFPGA(SD_OCR[1],SD_RSP1);         /* レスポンス(R3)からOCR取得 */
            if(SD_RSP1 & RSP_R3_OCR31){            /* OCR(レジスタ)の31bit目チェック(busy?) */
                break;
            }
        }
        SD_AppCommand();                           /* 決定済みのRCAを設定しCMD55発行 */
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendOpCond

  Description:  send CMD1 without busy for MMC
                MMC の CMD1を発行する（SDのACMD41に位置するコマンド）

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendOpCond(void)
{
    PRINTDEBUG( " CMD1  (SEND_OP_COND)\n");

    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));  /* タイムアウトエラーをクリア */

    while(!SDCARD_ErrStatus){                  /* エラーが発生しない間は繰り返し */
        SD_SetFPGA(SD_ARG0,(0x0000));          /* Argument(15:0) for MMC (None for SD) */
        SD_SetFPGA(SD_ARG1,(0x0010));          /* Argument(31:16) for MMC (None for SD) */
        SD_Command(SD_CMD_CMD | SEND_OP_COND); /* CMD1発行、レスポンス(R1)待ち */
        if(!SDCARD_ErrStatus){                 /* エラーステータスの確認（エラー無し？） */
            SD_GetFPGA(SD_OCR[0],SD_RSP0);
            SD_GetFPGA(SD_OCR[1],SD_RSP1);
            if(SD_RSP1 & RSP_R3_OCR31){        /* OCR(レジスタ)の31bit目チェック(busy?) */
                break;
            }
        }
    }
    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         SD_SendIfCond

  Description:  send CMD8
                CMD8を発行する（Physical Layer 2.00で追加されたコマンド）

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendIfCond(void)
{
#if 0
    PRINTDEBUG( " CMD8  (SEND_IF_COND)\n");

    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));  /* タイムアウトエラーをクリア */

    /* (31:12) Reserved bits, (11:8) supply voltage(VHS), (7:0) check pattern */
    SD_SetFPGA(SD_ARG0,(0x01AA));              /* Argument */
    SD_SetFPGA(SD_ARG1,(0x0000));              /* Argument */

    SD_Command(SD_CMD_CMD | SEND_IF_COND_EXT); /* CMD8発行、レスポンス(R7)待ち */

    if(!SDCARD_ErrStatus){                     /* エラーステータスの確認（エラー無し？） */
        SDCARD_SDHCFlag = TRUE;                /* SDHC */
//        SD_GetFPGA(SD_R7[0],SD_RSP0);
//        SD_GetFPGA(SD_R7[1],SD_RSP1);
    }else{
#endif
        SDCARD_SDHCFlag = FALSE;
#if 0
    }
#endif
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendRelativeAddr

  Description:  send relative addr
                CMD3を発行し、SDカードの場合はレスポンスのRCAを取得する
                （RCAは他のコマンド発行時にArgumentとして必要）

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendRelativeAddr(void)
{
    PRINTDEBUG( " CMD3  (SEND_RELATIVE_ADDR)\n");

    if(SDCARD_MMCFlag){                          /* MMCカードフラグ ON ？ */
        SD_SetFPGA(SD_ARG0,(0x0000));            /* Argument(15:0) */
        SD_SetFPGA(SD_ARG1,(0x0001));            /* Argument(31:16) = 0x0001 (松下drvはなぜか0x0100にしていた) */
    }                            /* SDカードのときはArgument(31:0) = stuff bits */

    SD_Command(SD_CMD_CMD | SEND_RELATIVE_ADDR); /* CMD3発行、レスポンス(R6)待ち */

    if(!SDCARD_ErrStatus){                       /* エラーステータスの確認（エラー無し？） */
        if(SDCARD_MMCFlag){                      /* MMCカードフラグ ON ？ */
            SD_RCA = 0x0001;                     /* RCA <- 1 (松下drvはなぜか0x0100にしていた) */
        }else{
            SD_GetFPGA(SD_RCA,SD_RSP1);          /* レスポンスレジスタからRCAを取得 */
        }
    }
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SelectCard

  Description:  toggle card between the stand-by and transfer states.
                CMD7を発行してスタンバイモードと転送モードを切り替える

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SelectCard(void)
{
    PRINTDEBUG( " CMD7  (SELECT#/DESELECT_CARD)\n");

    SD_SetFPGA(SD_ARG0,(0x0000));         /* Argument(15:0) = stuff bits*/
    SD_SetFPGA(SD_ARG1,(SD_RCA));         /* Argument(31:16) = RCA */

    SD_Command(SD_CMD_CMD | SELECT_CARD); /* CMD7発行、レスポンス(R1b)待ち */

#if ADD_CHECK
    SD_CheckStatus(FALSE);                /* コマンドレスポンス(R1) の Card Status チェック */
#endif

    return SDCARD_ErrStatus;

}

/*---------------------------------------------------------------------------*
  Name:         SD_SetBlockLength

  Description:  set of block length.
                SDカードのブロックレングスを設定する。
                注意：multiple read のときに SD_SIZEレジスタに512以外の値を
                設定すると正しく動作しない(IP reg spec資料より)。

  Arguments:    ulBlockLength : bytes of a block ( must be multiplier of 2)
                (memo : the default block length is as specified in the CSD.)
                1ブロックの長さ：2の乗数を指定すること

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SetBlockLength(u32 ulBlockLength)
{
    u16 usValue;

    PRINTDEBUG( " CMD16 (SET_BLOCKLEN)\n");

    /*------- IPの transfer length を設定 -------*/
    if(ulBlockLength == 512){
        SD_SetFPGA(SD_SIZE,(SD_SIZE_DATA_LENGTH_512B)); /* SDカードデータ転送サイズ 512Bytes 設定 */
    }else{
        usValue = (u16)ulBlockLength;                   /* 16bitに変換 */
        SD_SetFPGA(SD_SIZE,(usValue));                  /* IPにSDカードデータ転送サイズを設定 */
    }/*------------------------------------------*/

    /*------- カードの設定 -------*/
    /* argument(31:0) = block length */
    SD_SetFPGA(SD_ARG0,(((LELONG *)&ulBlockLength)->dt2word.low));
    SD_SetFPGA(SD_ARG1,(((LELONG *)&ulBlockLength)->dt2word.high));

    SD_Command(SD_CMD_CMD | SET_BLOCKLEN);              /* CMD16発行、レスポンス(R1)待ち */
#if ADD_CHECK
    SD_CheckStatus(FALSE);                              /* コマンドレスポンス(R1) の Card Status チェック */
#endif
    /*----------------------------*/
    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         SD_SendCID

  Description:  get card identification data (128bits).
                SDカードのCID値を取得する

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendCID(void)
{
    PRINTDEBUG( " CMD2  (ALL_SEND_CID)\n");
                                            /* Argument(31:0) = stuff bits */
    SD_Command(SD_CMD_CMD | ALL_SEND_CID);  /* CMD2発行、レスポンス(R2)待ち */

    if(!SDCARD_ErrStatus){                  /* エラーステータスの確認（エラー無し？） */
        SD_GetFPGA(SD_CID[0],SD_RSP0);
        SD_GetFPGA(SD_CID[1],SD_RSP1);
        SD_GetFPGA(SD_CID[2],SD_RSP2);
        SD_GetFPGA(SD_CID[3],SD_RSP3);
        SD_GetFPGA(SD_CID[4],SD_RSP4);
        SD_GetFPGA(SD_CID[5],SD_RSP5);
        SD_GetFPGA(SD_CID[6],SD_RSP6);
        SD_GetFPGA(SD_CID[7],SD_RSP7);
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendCSD

  Description:  get card specific data
                SDカードのCSD値を取得する

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendCSD(void)
{
    PRINTDEBUG( " CMD9  (SEND_CSD)\n");

    SD_SetFPGA(SD_ARG0,(0x0000));       /* Argument(15:0) = stuff bits */
    SD_SetFPGA(SD_ARG1,(SD_RCA));       /* Argument(31:16) = RCA */

    SD_Command(SD_CMD_CMD | SEND_CSD);  /* CMD9発行、レスポンス(R2)待ち */

    if(!SDCARD_ErrStatus){              /* エラーステータスの確認（エラー無し？） */
        SD_GetFPGA(SD_CSD[0],SD_RSP0);
        SD_GetFPGA(SD_CSD[1],SD_RSP1);
        SD_GetFPGA(SD_CSD[2],SD_RSP2);
        SD_GetFPGA(SD_CSD[3],SD_RSP3);
        SD_GetFPGA(SD_CSD[4],SD_RSP4);
        SD_GetFPGA(SD_CSD[5],SD_RSP5);
        SD_GetFPGA(SD_CSD[6],SD_RSP6);
        SD_GetFPGA(SD_CSD[7],SD_RSP7);
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendStatus

  Description:  send status read command (compatible to the MMC protocol).
                マルチメディアカード互換方式でのstatus取得コマンド発行

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendStatus(void)
{
    PRINTDEBUG( " CMD13 (Send STATUS)\n");

    SD_SetFPGA(SD_ARG0,(0x0000));                          /* Argument(15:0) = stuff bits */
    SD_SetFPGA(SD_ARG1,(SD_RCA));                          /* Argument(31:16) = RCA */

    /* カード排出の重複を防ぐため */
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_ERR_ALLCLR));           /* SD Card I/F の 全てのエラーをクリア */
    SD_AndFPGA(SD_INFO1,(~SD_INFO1_RES_END));              /* SD_INFO1レジスタの Response end クリア */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_ALLERRMASK)); /* SD Card I/F の 全エラー割込み許可 */

    SD_SetFPGA(SD_CMD,(SD_CMD_CMD | SD_SEND_STATUS));      /* CMD13発行 */

    while(!SD_CheckFPGAReg(SD_INFO1,SD_INFO1_RES_END)){    /* Response end (R1)待ち */
        /******************/
        SDCARD_TimerCheck();
        /******************/
        if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IPエラー発生?
            SYSFPGA_irq();
            break;
        }
        if( SDCARD_ErrStatus != 0) {
            break;
        }
    }

    /*ROM版はエラー割り込みが入らないためRES_ENDが立つとエラーを見逃すことがあるので最後にINFO2評価*/
    if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IPエラー発生?
        SYSFPGA_irq();
    }
    SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_ALLERRMASK));   /* SD Card I/F の 全エラー割込み禁止 */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendSCR

  Description:  get condition data.
                SDカード内のSCRレジスタ値取得コマンド発行。この後カードは
                DATライン経由で1ブロック(8Bytesに設定しておくこと)送信してくる。
                MultiBlock R/W と異なり、DATライン経由で転送されてくるSDカードの
                レジスタは、MSBから先に送られてくることに注意。
                (Physical Layer Specification 2.00 p12-13参照)

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendSCR(void)
{
    PRINTDEBUG( " ACMD51 (Send SCR)\n");
                                                    /* Argument(31:0) = stuff bits */
#if ADD_CHECK
    SD_TransReadyFPGA();                            /* 関連レジスタ初期化 */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BRE)); /* SDカードからのデータ読出し要求割込み許可*/

    SD_TransCommand((SD_CMD_ACMD | SEND_SCR));      /* SCR取得コマンド発行、レスポンス(R1)待ち */
#endif

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SDStatus

  Description:  send status read command (for SD card only).
                SDカード専用方式でのstatus取得コマンド発行。この後カードは
                DATライン経由で1ブロック(64Bytesに設定しておくこと)送信してくる。
                MultiBlock R/W と異なり、DATライン経由で転送されてくるSDカードの
                レジスタは、MSBから先に送られてくることに注意。
                (Physical Layer Specification 2.00 p12-13参照)

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SDStatus(void)
{
    PRINTDEBUG( " ACMD13 (SD_SD STATUS)\n");
                                                    /* Argument(31:0) = stuff bits */
    SD_TransReadyFPGA();                            /* 関連レジスタ初期化 */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BRE)); /* SDカードからのデータ読出し要求割込み許可*/

    SD_TransCommand((SD_CMD_ACMD | SD_STATUS));     /* ACMD13 Send the SD_CARD status コマンド発行、レスポンス(R1)待ち */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_MultiReadBlock

  Description:  send multiple block read command.
                マルチブロックリードコマンド発行。この後カードは
                DATライン経由でデータを送信してくる。

  Arguments:    ulOffset : offset address to read(BYTE).

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_MultiReadBlock(u32 ulOffset)
{
    PRINTDEBUG( " CMD18 (READ_MULTIPLE_BLOCK)\n");

    SD_TransReadyFPGA();                                /* INFOレジスタ初期化 */
    if( !SDCARD_UseFifoFlag) {                          /* FIFOを使わないとき */
        SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BRE)); /* SDカードからのデータ読出し要求割込み許可*/
    }

    /* 読み込み開始アドレス（オフセット）設定 */
    SD_SetFPGA(SD_ARG0,(((LELONG *)&ulOffset)->dt2word.low));
    SD_SetFPGA(SD_ARG1,(((LELONG *)&ulOffset)->dt2word.high));

    SD_TransCommand((READ_MULTIPLE_BLOCK));             /* CMD18(マルチセクタリードコマンド)発行、レスポンス(R1)待ち */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_ClockDivSet

  Description:  set clock speed into the SD card.
                (notice : clock into IP is 33.51MHz.)
                (memo : default clock is 262KHz.)
                SDカードへのクロック速度を設定する。CTRからIMCLK端子でIPに供給
                される基本クロックは33.51MHz。
                起動直後のデフォルトでは128分周(262KHz)が設定されている。

  Arguments:    CSD responce include "TRAN_SPEED".
                予めCSDをSDカードから読み出しておいた、TRAN_SPEEDが入っている
                部分のCSD(CSD[5])。SDカード側で対応可能な周波数を参照するため。

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
u16 SD_ClockDivSet(u16 usTranSpeed)
{
    u16 usTranTime;

    //CTRでは、IMCLKに入力されるクロックは 33.51Mhz
    //SD_CLK_CTRL の default値は 0x0020( 128分周 = 262kHz)
    /*------*/
    usTranTime = (u16)((usTranSpeed >> 11) & 0x000F); /* CSD[103:96] の time value(=4bit) 取得 */

    usTranSpeed &= CSD_TRANSFER_RATE;                 /* CSD[103:96] の transfer rate unit を取得 */
    usTranSpeed = (u16)(usTranSpeed >> 8);            /* transfer rate unit を下位8ビットに設定する */
    /*------*/


    switch( usTranSpeed) {
      /*--- 100kbit/s(one dat line) = 100KHz, の倍数のとき ---*/
      case CSD_TRAN_SPEED_100K:
        if(usTranTime > 0x000C){                               /* time value が 5.5 より大きい? */
            SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_64));          /* 523KHz */
        }
        else{
            if(usTranTime > 0x0006){                           /* time value が 2.5 より大きい? */
                SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_128));     /* 262KHz */
            }else{
                if( usTranTime == 1) {                         /* time value が 1.0 ? */
                    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_512)); /* 65KHz */
                }else{                                         /* time value が 1.2～2.5 のとき */
                    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_256)); /* 131KHz */
                }
            }
        }
        break;
      /*--- 1Mbit/s(one dat line) = 1MHz, の倍数のとき ---*/
      case CSD_TRAN_SPEED_1M:
        if(usTranTime == 0x0001){                              /* time value が 1.0 ? */
            SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_64));          /* 523KHz */
        }
        else{
            if(usTranTime <= 0x0005){                          /* time value が 2.0 以下? */
                SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_32));      /* 1.05MHz */
            }
            else{
                if(usTranTime <= 0x0009){                      /* time value が 4.0 以下? */
                    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_16));  /* 2.095MHz */
                }else{                                         /* time value が 4.5 以上のとき */
                    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_8));   /* 4.18MHz */
                }
            }
        }
        break;
      /*--- 10Mbit/s(one dat line) = 10MHz, の倍数のとき ---*/
      case CSD_TRAN_SPEED_10M:
        if(usTranTime > 0x0004){                        /* time value が 1.5 より大きい? */
            SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_2));    /* 16.76MHz */
        }else{                                          /* time value が 1.5 以下のとき */
            SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_4));    /* 8.38MHz */
        }
        break;
      /*--- 100Mbit/s(one dat line) = 100MHz, の倍数のとき ---*/
      case CSD_TRAN_SPEED_100M:                         /* time value がどんな値であっても */
           SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_2));     /* 16.76MHz */
        break;
      default:
        if( usTranSpeed != 7) {                         /* reserved値(=7)以外か? */
               SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_2)); /* 16.76MHz */
        }                                               /* (松下drvでは、4以上なら288KHzにしている) */
        break;                                          /* transfer rate unit が 7 のときは何もしない */
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_EnableClock

  Description:  enable clock

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_EnableClock( void)
{
    SD_OrFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_SDCLKEN));
}

/*---------------------------------------------------------------------------*
  Name:         SD_DisableClock

  Description:  disable clock (for power save).

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_DisableClock( void)
{
    SD_AndFPGA(SD_CLK_CTRL,(~SD_CLK_CTRL_SDCLKEN));
}


/*---------------------------------------------------------------------------*
  Name:         SD_SelectBitWidth

  Description:  set bus width for SD Card ( Not for MMC Card).
                (The allowed data bus widths are given in SCR register.)
                SDカードのDATラインバス幅を1bitまたは4bitに設定する。
                SDカードが4bitに対応していないか、MMCカードの場合はバス幅を4bitに
                設定することはできない。SDカードの許容するバス幅は、予め取得済みの
                SCRレジスタによって参照している。

  Arguments:    b4bit : TRUE = 4bit, FALSE = 1bit

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
u16 SD_SelectBitWidth(s16 b4bit)
{
    /*--- カードの仕様(SCR)を参照 ---*/
#if ADD_CHECK
    if( b4bit){                                         /* SCRレジスタがリード済であること! */
        if( !(SD_SCR[0] & SCR_DAT_BUS_WIDTH_4BIT)) {    /* カードが4bit幅に対応していないか? */
            b4bit = FALSE;                              /* 対応していない場合は1bit幅を指定しておく */
        }
    }/*------------------------------*/
#endif
    /*--- 4bit(TRUE) 指定のとき ---*/
    if(b4bit){
        if(!SDCARD_MMCFlag){                            /* MMCカードフラグ OFF？(ONなら何もしない) */
            if(SD_AppCommand()){                        /* RCA設定後 CMD55発行 */
                return SDCARD_ErrStatus;                /* CMD55が正常終了しなかったらエラー終了 */
            }
            SD_SetFPGA(SD_ARG0,(0x0002));                /* Argument(1:0) = (10) to 4bit,(00) to 1bit */
            SD_SetFPGA(SD_ARG1,(0x0000));                /* Argument(31:2) = stuff bits */
            PRINTDEBUG( " ACMD6 (SET_BUS_WIDTH:4bit)\n");
            SD_Command(SD_CMD_ACMD | SET_BUS_WIDTH);     /* ACMD6（ビット幅選択コマンド）発行、レスポンス(R1)待ち */
#if ADD_CHECK
    SD_CheckStatus(FALSE);                               /* コマンドレスポンス(R1)の Card Status チェック */
#endif
            if(!SDCARD_ErrStatus){                       /* エラーなし? */
                SD_AndFPGA(SD_OPTION,(~SD_OPTION_WIDTH_1BIT)); /* IPにビット幅の設定(4bit幅) */
            }
        }
    }/*--- 1bit(FALSE) 指定のとき ---*/
    else{
        if(!SDCARD_MMCFlag){                            /* MMCカードフラグ OFF？(ONなら何もしない) */
            if(SD_AppCommand()){                        /* RCA設定後 CMD55発行処理 */
                return SDCARD_ErrStatus;                /* CMD55が正常終了しなかったらエラー終了 */
            }
            SD_SetFPGA(SD_ARG0,(0x0000));                /* Argument(1:0) = bus width : 1bit */
            SD_SetFPGA(SD_ARG1,(0x0000));                /* Argument(31:2) = stuff bits */
            PRINTDEBUG( " ACMD6 (SET_BUS_WIDTH:1bit)\n");
            SD_Command(SD_CMD_ACMD | SET_BUS_WIDTH);     /* ACMD6（ビット幅選択コマンド）発行、レスポンス(R1)待ち */
#if ADD_CHECK
    SD_CheckStatus(FALSE);                               /* コマンドレスポンス(R1)の Card Status チェック */
#endif
        }
        if(!SDCARD_ErrStatus){                           /* エラーステータスの確認（エラー無し？） */
            SD_OrFPGA(SD_OPTION,(SD_OPTION_WIDTH_1BIT)); /* IPにビット幅の設定(1bit幅) */
        }
    }/*------------------------------*/

    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         MMCP_WriteBusWidth

  Description:  set bus width for SD Card ( Not for MMC Card).
                (The allowed data bus widths are given in SCR register.)
                MMCplusカードのDATラインバス幅を1bitまたは4bitに設定する。
                SDまたはMMCカードの場合はバス幅を4bitに設定することはできない。

  Arguments:    b4bit : TRUE = 4bit, FALSE = 1bit

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
u16 MMCP_WriteBusWidth(s16 b4bit)
{
    if( !SDCARD_MMCFlag) {
        return 1;
    }

    /* 書き込み開始オフセット設定 */
    if( b4bit) {
        SD_SetFPGA(SD_ARG0,(0x0100));    /* Argument(15:8)=(0x1) to 4bit,(0x0) to 1bit */
    }else{
        SD_SetFPGA(SD_ARG0,(0x0000));    /* Argument(15:8)=(0x1) to 4bit,(0x0) to 1bit */
    }
    SD_SetFPGA(SD_ARG1,(0x03B7));        /* Argument(25:24)=(0x3)Write, (23:16)=(183)Index */

    /* CMD6（ビット幅選択コマンド）発行、レスポンス(R1)待ち */
    SD_Command(SD_CMD_CMD | EXT_CSD_ACCESS);

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         MMCP_BusTest

  Description:  バスのテストを行う。
                この後カードにDATライン経由でデータを送信する必要がある。

  Arguments:    readflag : リード時TRUE, ライト時FALSE

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
u16 MMCP_BusTest( BOOL readflag)
{
    /**/
    SD_TransReadyFPGA();                            /* INFOレジスタ初期化 */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BWE)); /* SDカードからのデータ書込み要求割込み許可 */

    /**/
    SD_SetFPGA( SD_ARG0, 0);
    SD_SetFPGA( SD_ARG1, 0);

    if( readflag) {
        SD_TransCommand( 14);
    }else{
        SD_TransCommand( 19);
    }

    return( SDCARD_ErrStatus);
}


/*---------------------------------------------------------------------------*
  Name:         SD_FPGA_irq

  Description:  handler of data transfer request interrupt from card,
                check and clear interrupt register of IP.
                カードからの転送要求（BREまたはBWE）割り込み発生時に
                どちらかを調べて返し、INFO2の当該割り込み要求フラグをクリアする。

  Arguments:    None

  Returns:      TRUE : BRE割り込み発生
                FALSE : BWE割り込み発生
 *---------------------------------------------------------------------------*/
BOOL SD_FPGA_irq(void)
{
    /*--- FIFOを使うとき ---*/
    if( SDCARD_UseFifoFlag) {
        if( SD_CheckFPGAReg( *SDIF_CNT_L, (u16)SDIF_CNT_FFIE)) {   /* FULL割り込み許可のとき */
            return TRUE;
        }else{
            if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BWE)){ /* SDカードからのデータ書込み要求あり？ */
                SD_AndFPGA(SD_INFO2,(~SD_INFO2_BWE));              /* SD用バッファ制御 Write Enable リセット*/
            }
            return FALSE;
        }
    }else{    /*--- FIFOを使わないとき ---*/
        if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BRE)){ /* SDカードからのデータ読出し要求あり？ */
            SD_AndFPGA(SD_INFO2,(~SD_INFO2_BRE));              /* SD用バッファ制御 Read Enable リセット*/
            return TRUE;
        }
        if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BWE)){ /* SDカードからのデータ書込み要求あり？ */
            SD_AndFPGA(SD_INFO2,(~SD_INFO2_BWE));              /* SD用バッファ制御 Write Enable リセット*/
            return FALSE;
        }
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         SD_StopTransmission

  Description:  manual send CMD12 to terminate translate.
                手動でCMD12を発行し、転送終了をFPGAに通知する

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_StopTransmission(void)
{
    SD_OrFPGA(SD_STOP,(SD_STOP_STP));        /* データ転送終了設定 */
}

/*---------------------------------------------------------------------------*
  Name:         SD_TransEndFPGA

  Description:  clear the transfer interrupt.
                カード転送終了時に転送関連の割り込みを禁止に戻しておく

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_TransEndFPGA(void)
{
    SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_ALLERRMASK));   /* SD Card I/F の 全エラー割込み禁止 */
    SD_OrFPGA(SD_INFO1_MASK,(SD_INFO1_MASK_ALL_END));      /* R/W アクセス終了割込み禁止 */

    if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BRE)){ /* SDカードからのデータ読込み要求割込み許可か? */
        SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_BRE));      /* SDカードからのデータ読込み要求割込み禁止 */
    }
    if(!SD_CheckFPGAReg(SD_INFO2_MASK,SD_INFO2_MASK_BWE)){ /* SDカードからのデータ書込み要求割込み許可か? */
        SD_OrFPGA(SD_INFO2_MASK,(SD_INFO2_MASK_BWE));      /* SDカードからのデータ書込み要求割込み禁止 */
    }
}

/*---------------------------------------------------------------------------*
  Name:         SD_CheckStatus

  Description:  check the card status in the R1.
                R1レスポンスのカードステータスをチェックする

  Arguments:    bRead : 読み込み処理時 = TRUE

  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
u16 SD_CheckStatus(BOOL bRead)
{
    /* コマンドレスポンス(R1)の[39:8] から Card status を取得 */
    SD_GetFPGA((((LELONG *)&SDCARD_Status)->dt2word.low),SD_RSP0);
    SD_GetFPGA((((LELONG *)&SDCARD_Status)->dt2word.high),SD_RSP1);

    /*--- 松下サンプルドライバでやっている処理 ---*/
    if(bRead){                                            /* リード時か? */
        if(!(SDCARD_ErrStatus & SDMC_ERR_TIMEOUT)){       /* タイムアウト発生していないか? */
            SDCARD_Status &= ~SDCARD_STATUS_OUT_OF_RANGE; /* OUT_OF_RANGEフラグを落とす */
        }
    }/*-------------------------------------------*/

    PRINTDEBUG( " SD_CheckStatus ======== 0x%x\n", SDCARD_Status);

    if(SDCARD_Status & RSP_R1_STATUS_ERR){ /* コマンドレスポンス(R1)のカードステータスがエラーか確認 */
        SD_SetErr(SDMC_ERR_R1_STATUS);     /* コマンドレスポンス(R1)のカードステータス エラー */
    }
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_SwapByte

  Description:  swap bytes in a 16bit data.
                16bitデータの上位と下位を入れ換える

  Arguments:    data : 16bit data

  Returns:      swapped data
 *---------------------------------------------------------------------------*/
u16    SD_SwapByte(u16 *data)
{
    u16    usDATA;

    usDATA = *data;
    usDATA = (u16)(((usDATA & 0x00FF) << 8) | ((usDATA & 0xFF00) >> 8));

    return usDATA;
}

/*---------------------------------------------------------------------------*
  Name:         SD_EnableSeccnt

  Description:  enable SD_SECCNT register.
                SD_SECCNT を有効化して値をセットする。

  Arguments:    ulSDCARD_SectorCount : セクタカウント値（1セクタ=512byte）

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_EnableSeccnt( u32 ulSDCARD_SectorCount)
{
    u16    usSector;

    usSector = (u16)ulSDCARD_SectorCount;
    SD_OrFPGA(SD_STOP,SD_STOP_SEC_ENABLE); /* SD_SECCNTレジスタを有効にする */
    SD_SetFPGA(SD_SECCNT,usSector);        /* SD_SECCNTレジスタに転送セクタカウントを設定 */
}

/*---------------------------------------------------------------------------*
  Name:         SD_DisableSeccnt＜現在は未使用関数＞

  Description:  disable SD_SECCNT register.
                SD_SECCNT を無効化する。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_DisableSeccnt( void)
{
    u16    i;

    /*--- 松下ドライバでやっていること ---*/
    for ( i=0; i<50000; i++)                   /* カウント値50000では足りないことあり */
    {    /* SD_INFO1レジスタの R/W access all end 待ち */
        if(SD_CheckFPGAReg(SD_INFO1,SD_INFO1_ALL_END)) {
            break;
        }
    }/*-----------------------------------*/
    SD_AndFPGA(SD_STOP,(~SD_STOP_SEC_ENABLE)); /* SD_SECCNTレジスタを無効にする */
}

/*---------------------------------------------------------------------------*
  Name:         SD_SetErr

  Description:  set error flag
                指定のエラーフラグをセットする

  Arguments:    Error : error flag to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void    SD_SetErr(u16 Error)
{
    OSIntrMode irq_core_flag;

#if (TARGET_OS_CTR == 1)
    irq_core_flag = osDisableInterrupts(); /* 割込み禁止 */
        SDCARD_ErrStatus |= Error;         /* エラーステータスを設定 */
    osRestoreInterrupts( irq_core_flag);   /* 割り込み設定を元に戻す */
#else
    irq_core_flag = OS_DisableInterrupts();
        SDCARD_ErrStatus |= Error;         /* エラーステータスを設定 */
    OS_RestoreInterrupts( irq_core_flag);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         SD_ClrErr

  Description:  clear error flag
                指定のエラーフラグをクリアする

  Arguments:    Error : error flag to clear

  Returns:      None
 *---------------------------------------------------------------------------*/
void    SD_ClrErr(u16 Error)
{
    OSIntrMode irq_core_flag;

#if (TARGET_OS_CTR == 1)
    irq_core_flag = osDisableInterrupts(); /* 割込み禁止 */
        SDCARD_ErrStatus &= ~(Error);      /* エラーステータスをクリア */
    osRestoreInterrupts( irq_core_flag);   /* 割り込み設定を元に戻す */
#else
    irq_core_flag = OS_DisableInterrupts(); /* 割込み禁止 */
        SDCARD_ErrStatus &= ~(Error);       /* エラーステータスをクリア */
    OS_RestoreInterrupts( irq_core_flag);   /* 割り込み設定を元に戻す */
#endif
}


/*---------------------------------------------------------------------------*
  Name:         SD_TransReadyFPGA

  Description:  setup for the command that the card will response and request data transfer.
                DATラインでのデータ転送が発生するコマンド発行前の転送準備
                （BRE, BWE割り込みの許可は別途行うこと）

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SD_TransReadyFPGA(void)
{
    /* 関連レジスタ初期化 */
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_ERR_ALLCLR));                    /* SD Card I/F の 全てのエラーをクリア */
    SD_AndFPGA(SD_INFO1,(~(SD_INFO1_RES_END | SD_INFO1_ALL_END)));    /* SD_INFO1レジスタの Response end と access all end クリア */
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_BRE));                            /* SD用バッファ制御 Read Enable クリア*/
    SD_AndFPGA(SD_INFO2,(~SD_INFO2_BWE));                            /* SD用バッファ制御 Write Enable クリア*/
    SD_AndFPGA(SD_STOP,(~SD_STOP_STP));                                /* データ転送終了クリア */

    /* 割り込み関連許可 */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_ALLERRMASK));            /* SD Card I/F の 全エラー割込み許可 */
    SD_AndFPGA(SD_INFO1_MASK,(~SD_INFO1_MASK_ALL_END));                /* R/W access all end 割込み許可(Responseはポーリングで見るため許可しない) */
}


/*---------------------------------------------------------------------------*
  Name:         SD_TransCommand

  Description:  send command that the card will response and request data transfer.
                コマンドを送出する（DATラインでのデータ転送が発生するコマンド用）

  Arguments:    ucCommand : command number

  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
u16    SD_TransCommand(u16 ucCommand)
{
    SD_SetFPGA(SD_CMD,(ucCommand));                     /* コマンド発行 */

    while(!SD_CheckFPGAReg(SD_INFO1,SD_INFO1_RES_END)){ /* Response end 待ち */
        /******************/
        SDCARD_TimerCheck();
        /******************/
        if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IPエラー発生?
            SYSFPGA_irq();
            break;
        }
        if( SDCARD_ErrStatus != 0) {
            break;
        }
    }
  
    /*ROM版はエラー割り込みが入らないためRES_ENDが立つとエラーを見逃すことがあるので最後にINFO2評価*/
    if( (SD_INFO2 & SD_INFO2_MASK_ALLERRMASK) != 0) { //IPエラー発生?
        SYSFPGA_irq();
    }
  
    SD_CheckStatus(FALSE);                              /* R1レスポンスのcard statusチェック*/

    if(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS){
        SD_StopTransmission();                          /* カード転送終了をFPGAに通知 */
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_CheckFPGAReg

  Description:  check register bits.
                レジスタの状態をチェックする

  Arguments:    reg : 16bit register data
                value : mask data

  Returns:      TRUE : some of mask data bits are 1 on the register.
                FALSE : no mask data bits is 1 on the register.
 *---------------------------------------------------------------------------*/
BOOL    SD_CheckFPGAReg(u16 reg,u16 value)
{
    if( reg & value) {
        return TRUE;
    }else{
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SD_SendNumWRSectors

  Description:  send "number of well written blocks" command (for SD card only).
                SDカード専用方式でのライト済みセクタ数取得コマンド発行。この後カードは
                DATライン経由で1ブロック(4Bytesに設定しておくこと)送信してくる。
                MultiBlock R/W と異なり、DATライン経由で転送されてくるSDカードの
                レジスタは、MSBから先に送られてくることに注意。
                (Physical Layer Specification 2.00 p12-13参照)

  Arguments:    None

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_SendNumWRSectors(void)
{
    PRINTDEBUG( " ACMD22 (SEND_NUM_WR_SECTORS)\n");
                                                          /* Argument(31:0) = stuff bits */
    SD_TransReadyFPGA();                                  /* 関連レジスタ初期化 */
    SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BRE));       /* SDカードからのデータ読込み要求割込み許可 */

    SD_TransCommand((SD_CMD_ACMD | SEND_NUM_WR_SECTORS)); /* ACMD22（書きこみ完了セクタ数取得コマンド）発行、レスポンス(R1)待ち */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SD_MultiWriteBlock

  Description:  send multiple block write command.
                マルチブロックライトコマンド発行。この後カードに
                DATライン経由でデータを送信する必要がある。

  Arguments:    ulOffset : offset address to write(BYTE).

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
u16 SD_MultiWriteBlock(u32 ulOffset)
{
    PRINTDEBUG( " CMD25 (WRITE_MULTIPLE_BLOCK)\n");

    SD_TransReadyFPGA();                                /* INFOレジスタ初期化 */
//    if( !SDCARD_UseFifoFlag) {                        /* FIFOを使わないとき */
        SD_AndFPGA(SD_INFO2_MASK,(~SD_INFO2_MASK_BWE)); /* SDカードからのデータ書込み要求割込み許可 */
//    }

    /* 書き込み開始オフセット設定 */
    SD_SetFPGA(SD_ARG0,(((LELONG *)&ulOffset)->dt2word.low));
    SD_SetFPGA(SD_ARG1,(((LELONG *)&ulOffset)->dt2word.high));

    SD_TransCommand(WRITE_MULTIPLE_BLOCK);              /* CMD25（マルチセクタライトコマンド）発行 */

    return SDCARD_ErrStatus;
}


