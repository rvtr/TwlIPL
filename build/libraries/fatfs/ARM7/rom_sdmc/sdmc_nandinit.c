/*
    Project:    TwlBrom SD port driver
    File:       Carddrv.c

    2006, Research and Development Department, Nintendo.
*/

#include "sdmc_config.h"
#include "sdmc.h"
#include "sdif_reg.h"
#include "sdif_ip.h"
#include <firm.h>
//#include <twl/hw/ARM7/ioreg_MI.h>
//#include "sdmc_config.h"
//#include "sdif_reg.h"            /*  IP 対応レジスタ定義 */
//#include <brom/rom_sdmc.h>
//#include "sdif_ip.h"            /*  IP 対応フラグ定義 */

//        #define PRINTDEBUG    OS_TPrintf
    #define PRINTDEBUG( ...) ((void)0)


/***********************************************************************
 定数
***********************************************************************/
static BOOL sdmcGetErrStat( void);


/***********************************************************************
 static関数の宣言
***********************************************************************/
static void SDCARD_Backup_port1(void);
static void SDCARD_Restore_port1(void);

static int  MMCP_SetBusWidth( BOOL b4bit);       /* ビット幅の選択(MMCplus, eMMC, moviNAND) */

static SDMC_ERR_CODE SDCARD_Layer_Init(void);
static SDMC_ERR_CODE i_sdmcMPInit( void);     /* カードドライバ初期化(マルチポート対応) */

/***********************************************************************
    外部参照変数
***********************************************************************/
extern void SDCARD_Timer_irq(void* arg);      /* タイムアウト割り込みハンドラ */
extern void SDCARD_TimerCheck( void);
extern void SDCARD_TimerStart(u32 tim);    /* タイムアウト計測スタート */
extern void SDCARD_TimerStop(void);        /* タイムアウト計測停止 */


extern u16  SD_CID[8];                /* CID値保存用 */
extern u16  SD_CSD[8];                /* CSD値保存用 */
extern u16  SD_OCR[2];                /* OCR値保存用 */
extern u16  SD_SCR[4];                /* SCR値保存用 */
extern u16  SD_RCA;                   /* RCA値保存用 */

extern s16  SDCARD_MMCFlag;           /* MMCカードフラグ */
extern s16  SDCARD_SDHCFlag;          /* SDHCカードフラグ */
extern u16  SD_port_number;           /* 現在ポート番号  */


/***********************************************************************
    グローバル
***********************************************************************/
static u16  SD_SDSTATUS[32];            /* SD_STATUSレジスタ保存用 */
static u16  SDCARD_WP_FLAG0;            /* カードライトプロテクトフラグ。0=なし、1=有り    */
static u16  SDCARD_WP_FLAG1;            /* カードライトプロテクトフラグ。0=なし、1=有り    */
static u16  SDCARD_WP_PERMANENT;        /* カードライトプロテクト永久フラグ。0=なし、1=有り    */
static u16  SDCARD_WP_TEMPORARY;        /* カードライトプロテクト一時フラグ。0=なし、1=有り    */

static u32  SDCARD_SectorSize;          /* セクタサイズ デフォルト 512bytes */

static u16  SD_INFO1_VALUE;             /* SD_INFO1レジスタ取得用変数 */
static u16  SD_INFO1_MASK_VALUE;        /* SD_INFO1割込みマスク用変数(0で許可, 1で禁止) */
static u16  SD_INFO2_VALUE;             /* SD_INFO2レジスタ取得用変数 */
static u16  SD_INFO2_MASK_VALUE;        /* SD_INFO2割り込みマスク用変数(0で許可, 1で禁止) */
static u16  SD_INFO_ERROR_VALUE;        /* SD_INFO2, SD_INFO1のエラービット確認用変数 */


/*ポート状態保存*/
static SDPortContext SDPort0Context;
static SDPortContext SDPort1Context;
//SDPortContext *SDPortCurrentContext = &SDPort0Context; /*TODO*/



extern volatile SDMC_ERR_CODE    SDCARD_ErrStatus;    /* エラーステータス */
extern volatile u32    SDCARD_Status;       /* カードステータス */

static s16             SDCARD_SDFlag;           /* SDカードフラグ */

static volatile s16    SDCARD_OutFlag = 0;          /* カード排出発生判定フラグ */
static SdmcResultInfo  *pSDCARD_info = NULL;           /* 保存用実行結果構造体ポインタ */


static SDMC_ERR_CODE i_sdmcSavePortContext( SDPortContext* buf_adr, u16 port_no);
static SDMC_ERR_CODE i_sdmcLoadPortContext( SDPortContext* buf_adr, u16* port_no);


/*---------------------------------------------------------------------------*
  Name:         sdmcSavePortContext

  Description:  ポート0のレジスタや変数をユーザバッファに退避する

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcSavePortContext( SDPortContext* buf_adr, u16 port_no)
{
    if( buf_adr == NULL) {
        return( SDMC_ERR_PARAM);
    }
    switch( port_no) {
      case 0:
        MI_CpuCopy8( &SDPort0Context, buf_adr, sizeof(SDPortContext));
        buf_adr->port_no = 0;
        break;
      case 1:
        MI_CpuCopy8( &SDPort1Context, buf_adr, sizeof(SDPortContext));
        buf_adr->port_no = 1;
        break;
      default: return( SDMC_ERR_PARAM);
    }
   return( SDMC_NORMAL);
}

/*---------------------------------------------------------------------------*
  Name:         sdmcLoadPortContext

  Description:  ポート0のレジスタや変数をユーザバッファから復帰する

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcLoadPortContext( SDPortContext* buf_adr, u16* port_no)
{
    if( buf_adr == NULL) {
        return( SDMC_ERR_PARAM);
    }
    switch( buf_adr->port_no) {
      case 0:
        MI_CpuCopy8( buf_adr, &SDPort0Context, sizeof(SDPortContext));
        *port_no = 0;
        break;
      case 1:
        MI_CpuCopy8( buf_adr, &SDPort1Context, sizeof(SDPortContext));
        *port_no = 1;
        break;
      default: return( SDMC_ERR_PARAM);
    }
   return( SDMC_NORMAL);
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_Backup_port1

  Description:  backup registers and variables of port1.
                ポート1のレジスタや変数をバックアップする

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Backup_port1(void)
{
    /* registers */
    SD_GetFPGA(SDPort1Context.SD_CLK_CTRL_VALUE,SD_CLK_CTRL);
    SD_GetFPGA(SDPort1Context.SD_OPTION_VALUE,  SD_OPTION);

    /* variables */
    SDPort1Context.SD_RCA       = SD_RCA;
    SDPort1Context.ErrStatus = SDCARD_ErrStatus;
    SDPort1Context.Status    = SDCARD_Status;
    SDPort1Context.MMCFlag   = SDCARD_MMCFlag;
    SDPort1Context.SDHCFlag  = SDCARD_SDHCFlag;
    SDPort1Context.SDFlag    = SDCARD_SDFlag;

    SDPort1Context.OutFlag   = SDCARD_OutFlag;

    /*media registers*/
    MI_CpuCopy8( SD_CID, SDPort1Context.SD_CID, 16);
    MI_CpuCopy8( SD_CSD, SDPort1Context.SD_CSD, 16);
    MI_CpuCopy8( SD_OCR, SDPort1Context.SD_OCR,  4);
    MI_CpuCopy8( SD_SCR, SDPort1Context.SD_SCR,  8);
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_Restore_port1

  Description:  restore registers and variables of port0.
                ポート1のレジスタや変数を復帰する。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Restore_port1(void)
{
    /* registers */
    SD_SetFPGA( SD_CLK_CTRL, SDPort1Context.SD_CLK_CTRL_VALUE);
    SD_SetFPGA( SD_OPTION,   SDPort1Context.SD_OPTION_VALUE);

    /* variables */
    SD_RCA           = SDPort1Context.SD_RCA;
    SDCARD_ErrStatus = SDPort1Context.ErrStatus;
    SDCARD_Status    = SDPort1Context.Status;
    SDCARD_MMCFlag   = SDPort1Context.MMCFlag;
    SDCARD_SDHCFlag  = SDPort1Context.SDHCFlag;
    SDCARD_SDFlag    = SDPort1Context.SDFlag;

    SDCARD_OutFlag = SDPort1Context.OutFlag;
    pSDCARD_info   = NULL;

    /*media registers*/
    MI_CpuCopy8( SDPort1Context.SD_CID, SD_CID, 16);
    MI_CpuCopy8( SDPort1Context.SD_CSD, SD_CSD, 16);
    MI_CpuCopy8( SDPort1Context.SD_OCR, SD_OCR,  4);
    MI_CpuCopy8( SDPort1Context.SD_SCR, SD_SCR,  8);

    /*recalc*/
    SDCARD_WP_PERMANENT = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_PERMANENT_BIT));
    SDCARD_WP_TEMPORARY = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_TEMPORARY_BIT));
}


/*---------------------------------------------------------------------------*
  Name:         sdmcNandInit

  Description:  Initialize SD interface and SD card.
                初期化

  Arguments:    dma_no : 使用するDMA番号
                func1 : カード挿入時コールバック関数
                func2 : カード排出時コールバック関数

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcNandInit( void)
{
    /* SD初期化 */
    SDCARD_ErrStatus = sdmcReset();

    if(!SDCARD_ErrStatus) {
        SDCARD_ErrStatus = i_sdmcMPInit();
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcReset

  Description:  reset SD card.
                リセット

  Arguments:

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE sdmcReset( void)
{
    OSIntrMode irq_core_flag;
    SDCARD_MMCFlag  = FALSE;           /* MMCカード判定フラグクリア */
    SDCARD_SDHCFlag = FALSE;
    SDCARD_SDFlag   = FALSE;           /* SDカード判定フラグクリア */

    /*** カードステータスをクリア ***/
    SDCARD_ErrStatus = SDMC_NORMAL;
    SDCARD_Status    = SDMC_NORMAL;

    /*** カードCSD WPビットをクリア ***/
    SDCARD_WP_FLAG0 = 0;
    SDCARD_WP_FLAG1 = 0;
    SDCARD_WP_PERMANENT = 0;
    SDCARD_WP_TEMPORARY = 0;

    irq_core_flag = OS_DisableInterrupts();    /* 割込み禁止 */

        *SDIF_CNT_L = 0x0402;                  //ラッパーレジスタ
        *SDIF_CNT_L = 0x0000;                  //ラッパーレジスタ
        *SDIF_FDS_L = 0;
        *SDIF_FSC_L = 1;
        SD_Init();                                 /* SD Card I/F 初期化処理 */
        SD_AndFPGA( SD_OPTION, SD_CD_DETECT_TIME); /* CD 検出タイムをゼロクリア */

        SD_port_number = SDCARD_PORT1;             /*** 現在のポート番号をデフォルトに設定      ***/

//        SDCARD_Backup_port1();                   /* port1 backup    */

    OS_RestoreInterrupts( irq_core_flag);      /* 割り込み設定を元に戻す */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Layer_Init

  Description:  initialize sequence for SD card.
                SDカード規定の初期化手順

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARD_Layer_Init(void)
{
    u32     ulCSize;
    u16     read_block_len_val, mult_val;

//    u16 memory_exist, function_number;
    SDCARD_Status = SDMC_NORMAL;    /* カードステータスをクリア */
    SDCARD_MMCFlag = FALSE;         /* MMCカード判定フラグクリア */
    SDCARD_SDHCFlag = FALSE;
    SDCARD_SDFlag = FALSE;          /* SDカード判定フラグクリア */

    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_128)); /* SDクロックの周波数 261KHz(初期化時は100～400khz) */
    SD_EnableClock();                          /* SDカードのクロックをイネーブルにする */

    /* SD I/F部ダミー80クロック(1mSec)転送待ち（タイマーで待ちを実装しても良い） */
    SVC_WaitByLoop( 17900); //179*4サイクル=716サイクル=10024ns=10us

    SDCARD_ErrStatus = SDMC_NORMAL;            /* エラーステータスをクリア */

    SDCARD_TimerStop();                        /* タイムアウト判定用タイマストップ */

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RESET_TIMEOUT);    /* タイムアウト判定用タイマスタート */
#endif

    PRINTDEBUG( " CMD0(GO_IDLE_STATE)\n");
    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));   /* タイムアウト以外のエラーをクリア */
    SD_Command(SD_CMD_CMD | GO_IDLE_STATE);     /* CMD0発行、レスポンス確認 */
    if(SDCARD_ErrStatus){                       /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }

    /*------- idle state -------*/
    SVC_WaitByLoop( 17900); //179*4サイクル=716サイクル=10024ns=10us

    while(!(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT)){ /* タイムアウトになったら抜ける */
        /******************/
        SDCARD_TimerCheck();
        /******************/
        SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));

        SD_RCA = 0;                             /* RCA = 0をセット */

        if(!SDCARD_MMCFlag){                    /* MMCカードフラグが 0(OFF) か? */

            if( SD_AppCommand() == SDMC_NORMAL) {    /* CMD55 発行処理が正常か? */

                SDCARD_MMCFlag = FALSE;         /* MMCカードフラグクリア */
                if(!SD_AppOpCond()){            /* ACMD41発行処理が正常か?(OCR31bit = L の時 No Response) */

                    SDCARD_SDFlag = TRUE;       /* SDカードフラグセット */
                    break;
                }
            }else{                                        /* CMD55 が正常終了しない */
                if(SDCARD_ErrStatus == SDMC_ERR_TIMEOUT){ /* タイムアウト(==No Response)か? */
                    SDCARD_MMCFlag = TRUE;                /* MMCカードフラグセット */
                }else{
//                    break;                //コメントアウトしないとSDカードの初期化に失敗する
                }
            }
        }
        if(SDCARD_MMCFlag){                     /* MMCカードフラグが 1(ON) のとき */
            SD_RCA = 1;                         /* RCA = 1をセット */
            if(!SD_SendOpCond()){               /* CMD1発行処理が正常か? */
                break;
            }
        }
    }

    SDCARD_TimerStop();                               /* タイムアウト判定用タイマストップ */
    if(SDCARD_ErrStatus){                             /* エラーステータスの確認（エラー有り？）*/
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーかチェック */
            SD_ClrErr(SDMC_ERR_FPGA_TIMEOUT);         /* タイムアウトエラーの設定クリア */
            SD_SetErr(SDMC_ERR_RESET);                /* 初期化カードリセットコマンド時1.5秒タイムアウトエラーの設定 */
        }
        SDCARD_MMCFlag = FALSE;                       /* MMCカードフラグクリア */
        return SDCARD_ErrStatus;
    }

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_INITIAL_TIMEOUT); /* タイムアウト判定用タイマスタート */
#endif

    SD_SendCID();                            /* CMD2発行 レスポンス確認 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }

    while(1){
        /******************/
        SDCARD_TimerCheck();
        /******************/
        SD_SendRelativeAddr();               /* CMD3発行 レスポンス確認 正常終了時 RCA<-ResのRCA */
        if(SDCARD_ErrStatus){                /* エラーステータスの確認（エラー有り？） */
            return SDCARD_ErrStatus;
        }
        if(SD_RCA != 0){
            break;
        }
    }

    /*------- standby state -------*/
    SD_SendCSD();                            /* CMD9発行 レスポンス確認 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }
    SDCARD_WP_PERMANENT = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_PERMANENT_BIT));
    SDCARD_WP_TEMPORARY = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_TEMPORARY_BIT));

    /* 転送速度設定 */
    SD_ClockDivSet(SD_RSP5);                 /* SDカードの動作クロック設定 (CSD[5]) */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }

    /* Command toggles acard between the Stand-by and Transfer states */
    SD_SelectCard();                         /* CMD7発行 レスポンス確認 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }

    /*------- translate state -------*/

    SDCARD_SectorSize = SECTOR_SIZE;         /* セクタサイズ デフォルト 512bytes */
    SD_SetBlockLength(SDCARD_SectorSize);    /* CMD16 ブロックサイズの設定 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }

  SD_SCR[0] |= SCR_DAT_BUS_WIDTH_4BIT; //SCRの必要な部分のみ

    SD_EnableClock();                        /* SD-CLK Enable */

    if(SDCARD_MMCFlag){                      /* MMCカード ON かチェック */
        if( ((SD_CSD[7] & 0x3C)>>2) >= 4) {
            MMCP_SetBusWidth( TRUE);
        }
//        SD_SelectBitWidth(FALSE);            /* CMD55->ACMD6 ビット幅の選択 1bit */
    }else{
        SD_SelectBitWidth(TRUE);             /* CMD55->ACMD6 ビット幅の選択 4bit */
    }

    SDCARD_TimerStop();                      /* タイムアウト判定用タイマストップ */
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcMPInit

  Description:  initialize SD card in multi ports.
                マルチポートのSDカード初期化

  Arguments:

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcMPInit( void)
{
    u16 load_port_no;
    SDPortContext* SDNandContext;

    //予約領域のポートコンテキスト参照
    SDNandContext = (SDPortContext*)&(((OSFromBromBuf*)OSi_GetFromBromAddr())->SDNandContext);

    //NANDスロットの初期化
    SD_SetFPGA( SD_PORTSEL, SDMC_PORT_NAND); /* NANDポート選択 */

//    OS_TPrintf( "SDNandContext : 0x%x\n", SDNandContext);
    //初期化済みでないときだけ初期化
    if( sdmcCheckPortContext( SDNandContext) != SDMC_NORMAL) {
//        OS_TPrintf( "sdmcCheckPortContext : ERR!\n");
        SDCARD_ErrStatus = SDCARD_Layer_Init();

        SDCARD_Backup_port1();
        //ポートコンテキストの保存
        if( i_sdmcSavePortContext( SDNandContext, 1) != SDMC_NORMAL) {
//            OS_TPrintf( "i_sdmcSavePortContext failed\n");
            return( SDMC_ERR_PARAM);
        }
    }else{ //ポートコンテキストの復帰
//        OS_TPrintf( "sdmcCheckPortContext : NORMAL\n");

        /*SDCARD_Layer_Init()の代わり*/
        SDCARD_SectorSize = SECTOR_SIZE;         /* セクタサイズ デフォルト 512bytes */

        if( i_sdmcLoadPortContext( SDNandContext, &load_port_no) != SDMC_NORMAL) {
//            OS_TPrintf( "i_sdmcLoadPortContext failed\n");
            return( SDMC_ERR_PARAM);
        }
        SDCARD_Restore_port1(); //TODO:load_port_no値判定
    }

    SDCARD_OutFlag = FALSE;     /* 排出フラグをリセット */

    SDCARD_TimerStop();         /* タイムアウト判定用タイマストップ */
    SD_DisableClock();          /* SD-CLK Disable */
    SD_EnableInfo();            /* SD Card  挿抜 割り込み許可 */

    return SDCARD_ErrStatus;
}

#if 0
/*---------------------------------------------------------------------------*
  Name:         sdmcGetStatus

  Description:  get card status
                カードの状態を取得する
                bit15 SDカード判別ビット(検出したら1)
                bit14 MMCカード判別ビット(検出したら1)
                bit10 IO3 card detect(検出したら1)     ※CTRではプルアップのため使えない
                bit9 IO3 card inserted(挿入動作で1) ※CTRではプルアップのため使えない
                bit8 IO3 card removed(脱動作で1)     ※CTRではプルアップのため使えない
                bit7 write protect(書き込み禁止の場合1)
                bit5 card detect(検出したら1)
                bit4 card inserted(挿入動作で1)
                bit3 card removed(脱動作で1)
                bit2 R/W access all end
                bit0 Response end

  Arguments:    *status : カードの状態を格納する変数へのポインタ

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------
SD_INFO1レジスタ
bit[10,9,8,7] = DAT3CD, DAT3IN, DAT3OUT, WP
bit[5, 4, 3, 2] = CD, INS, REM, ALLEND
bit0 = RESEND
-------------------------------------------------
EXT_CDレジスタ
bit[2,1,0] = P1CD, P1INS, P1REM
-------------------------------------------------
EXT_CD_DAT3レジスタ
bit[2, 1, 0] = P1DCD, P1DINS, P1DREM
-------------------------------------------------
EXT_WPレジスタ
bit0 = P1WP
-----------------------------------------------*/
SDMC_ERR_CODE sdmcGetStatus(u16 *status)
{
    u16 SD_INFO1_STATUS;

    SD_INFO1_STATUS = SD_INFO1;         /* SD_INFO1レジスタ読み出し */
    *status = SD_INFO1_STATUS;          /* 論理反転 */

    /*--- ポート0のとき ---*/
    if(SD_port_number == SDCARD_PORT0)
    {
        *status &= SDCARD_FLAG_CLR;     /* SD/MMCフラグクリア */
    }
    /*--- ポート1のとき ---*/
    else if (SD_port_number == SDCARD_PORT1)
    {
        *status &= SDCARD_PORT1_CLR;    /* port1に関係ない部分をクリア */
        SD_INFO1_STATUS = (u16)((EXT_CD & 0x0007) << 3);
        SD_INFO1_STATUS |= ((EXT_CD_DAT3 & 0x0007) << 8);
        SD_INFO1_STATUS |= ((EXT_WP & 0x0001) << 7);
        *status |= SD_INFO1_STATUS;     /* カードport1フラグ設定   */
    }
    /*--- SD/MMCフラグをセット ---*/
    if( SDCARD_MMCFlag) {               /* 検出したカードがMMCカードの時 */
        *status |= SDCARD_FLAG_MMC;     /* カード判定部分MMCカード */
    }
    if( SDCARD_SDFlag) {                /* 検出したカードがSDカードの時 */
        *status |= SDCARD_FLAG_SD;      /* カード判定部分SDカード */
    }

    return SDMC_NORMAL;
}
#endif


/*******************************************************************************/
static int MMCP_SetBusWidth( BOOL b4bit)
{
    SD_EnableClock();                        /* SD-CLK Enable */

    /*コマンド6発行*/
    MMCP_WriteBusWidth( b4bit);
    SD_AndFPGA(SD_OPTION,(~SD_OPTION_WIDTH_1BIT)); /* IPにビット幅の設定(4bit幅) */

    return( 0);
}
/*******************************************************************************/



/*---------------------------------------------------------------------------*
  Name:         sdmcGetErrStat

  Description:  エラー取得

  Arguments:

  Returns:      TRUE : success
                FALSE : error
 *---------------------------------------------------------------------------*/
static BOOL sdmcGetErrStat( void)
{
    PRINTDEBUG( "SD_INFO2:0x%x\n", SD_INFO2);

    /*--- 取得 ---*/
    SD_GetFPGA( SD_INFO2_VALUE,            SD_INFO2);
    /*------------------------------------------*/

    /*--- SD_INFO2のエラーフラグ作成 ---*/
    SD_INFO_ERROR_VALUE = (u16)(SD_INFO2_VALUE & 0x807F);//
    /*--- エラーステータス作成 (RESTIMEOUTとILAエラーのフラグは反映しない) ---*/
    SDCARD_ErrStatus |= SD_INFO_ERROR_VALUE & (~(SD_INFO2_ERR_RESTIMEOUT)) &
                        (~(SD_INFO2_ERR_ILA)) & SD_INFO2_MASK_ERRSET;

    /*--- RESTIMEOUTとILAエラーはフラグの位置をずらして反映する ---*/
    if( SD_INFO_ERROR_VALUE & SD_INFO2_ERR_ILA) {
        SDCARD_ErrStatus |= SDMC_ERR_ILA;                       /* イリーガルアクセスエラー発生 */
    }
    if( SD_INFO_ERROR_VALUE & SD_INFO2_ERR_RESTIMEOUT) {
        SDCARD_ErrStatus |= SDMC_ERR_TIMEOUT;                   /* Response Time out エラー発生 */
    }/*------------------------------------------------------------*/

    SD_AndFPGA( SD_INFO2,(~(SD_INFO2_ERROR_SET)));              /* SD_INFO2のエラーフラグを全て落とす */

    if( (SD_INFO_ERROR_VALUE) != 0) {
        return( FALSE);
    }else{
        return( TRUE);
    }
}
