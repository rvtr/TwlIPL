/*---------------------------------------------------------------------------*
  Project:  
  File:     sdmc_port.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: sdmc_port.c,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <firm.h>
#include "sdmc_config.h"
#include "sdif_reg.h"
#include <firm/devices/firm_sdmc/ARM7/sdmc.h>
#include "sdif_ip.h"


//        #define PRINTDEBUG    OS_TPrintf
    #define PRINTDEBUG( ...) ((void)0)



/*extern変数(sdmc.c)*/
extern u32             SDCARD_SectorSize;       /* セクタサイズ デフォルト 512bytes */
extern SDMC_ERR_CODE   SDCARD_ErrStatus;        /* エラーステータス */
extern volatile s16    SDCARD_OutFlag;          /* カード排出発生判定フラグ */


extern SDPortContext SDPort0Context;
extern SDPortContext SDPort1Context;


/*extern関数(sdmc.c)*/
extern void i_sdmcCalcSize( void);
extern void          SDCARD_Backup_port1(void);
extern void          SDCARD_Restore_port1(void);
extern SDMC_ERR_CODE SDCARD_Layer_Init(void);
extern void          SDCARD_TimerStop(void);        /* タイムアウト計測停止 */

static SDMC_ERR_CODE i_sdmcSavePortContext( SDPortContext* buf_adr, u16 port_no);
static SDMC_ERR_CODE i_sdmcLoadPortContext( SDPortContext* buf_adr, u16* port_no);

static void sdmcPrintContext( SDPortContext* targ);


/*---------------------------------------------------------------------------*
  Name:         sdmcClearPortContext

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void sdmcClearPortContext( SDPortContext* buf_adr)
{
    MI_CpuFill8( buf_adr, 0x00, sizeof(SDPortContext));
}

/*---------------------------------------------------------------------------*
  Name:         sdmcCheckPortContext

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcCheckPortContext( SDPortContext* buf_adr)
{
    if( (buf_adr->SD_CID[0] != 0)&&(buf_adr->port_no < 2)) {
        return( SDMC_NORMAL);
    }else{
      return( SDMC_ERR_PARAM);
    }
}

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
  Name:         i_sdmcMPInitFirm

  Description:  initialize SD card in multi ports.
                マルチポートのSDカード初期化

  Arguments:    

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE i_sdmcMPInitFirm( void)
{
    u16 load_port_no;
    SDPortContext* SDNandContext;

    //予約領域のポートコンテキスト参照
    SDNandContext = (SDPortContext*)&(((OSFromBromBuf*)OSi_GetFromBromAddr())->SDNandContext);

#if 0
  sdmcPrintContext( SDNandContext);
#endif
  
    //NANDスロットの初期化
    SD_SetFPGA( SD_PORTSEL, SDMC_PORT_NAND); /* NANDポート選択 */

    PRINTDEBUG( "SDNandContext : 0x%x\n", SDNandContext);
    //初期化済みでないときだけ初期化
    if( sdmcCheckPortContext( SDNandContext) != SDMC_NORMAL) {
        PRINTDEBUG( "sdmcCheckPortContext : ERR!\n");
        SDCARD_ErrStatus = SDCARD_Layer_Init();

        SDCARD_Backup_port1(); //TODO:ポート番号
        //ポートコンテキストの保存
        if( i_sdmcSavePortContext( SDNandContext, 1) != SDMC_NORMAL) {
            PRINTDEBUG( "i_sdmcSavePortContext failed\n");
            return( SDMC_ERR_PARAM);
        }
    }else{ //ポートコンテキストの復帰
        PRINTDEBUG( "sdmcCheckPortContext : NORMAL\n");
      
        /*SDCARD_Layer_Init()の代わり*/
        SDCARD_SectorSize = SECTOR_SIZE;         /* セクタサイズ デフォルト 512bytes */
//        SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_128)); /* SDクロックの周波数 261KHz(初期化時は100～400khz) */
      
        if( i_sdmcLoadPortContext( SDNandContext, &load_port_no) != SDMC_NORMAL) {
            PRINTDEBUG( "i_sdmcLoadPortContext failed\n");
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


/*デバッグ用*/
static void sdmcPrintContext( SDPortContext* targ)
{
    OS_TPrintf( "CID[0]:0x%x\n", targ->SD_CID[0]);
    OS_TPrintf( "CID[1]:0x%x\n", targ->SD_CID[1]);
    OS_TPrintf( "CID[2]:0x%x\n", targ->SD_CID[2]);
    OS_TPrintf( "CID[3]:0x%x\n", targ->SD_CID[3]);
    OS_TPrintf( "CID[4]:0x%x\n", targ->SD_CID[4]);
    OS_TPrintf( "CID[5]:0x%x\n", targ->SD_CID[5]);
    OS_TPrintf( "CID[6]:0x%x\n", targ->SD_CID[6]);
    OS_TPrintf( "CID[7]:0x%x\n\n", targ->SD_CID[7]);

    OS_TPrintf( "CSD[0]:0x%x\n", targ->SD_CSD[0]);
    OS_TPrintf( "CSD[1]:0x%x\n", targ->SD_CSD[1]);
    OS_TPrintf( "CSD[2]:0x%x\n", targ->SD_CSD[2]);
    OS_TPrintf( "CSD[3]:0x%x\n", targ->SD_CSD[3]);
    OS_TPrintf( "CSD[4]:0x%x\n", targ->SD_CSD[4]);
    OS_TPrintf( "CSD[5]:0x%x\n", targ->SD_CSD[5]);
    OS_TPrintf( "CSD[6]:0x%x\n", targ->SD_CSD[6]);
    OS_TPrintf( "CSD[7]:0x%x\n\n", targ->SD_CSD[7]);
  
    OS_TPrintf( "OCR[0]:0x%x\n", targ->SD_OCR[0]);
    OS_TPrintf( "OCR[1]:0x%x\n\n", targ->SD_OCR[1]);
  
    OS_TPrintf( "SCR[0]:0x%x\n", targ->SD_SCR[0]);
    OS_TPrintf( "SCR[1]:0x%x\n\n", targ->SD_SCR[1]);
  
    OS_TPrintf( "RCA:0x%x\n\n", targ->SD_RCA);
  
    OS_TPrintf( "MMCFlag :0x%x\n", targ->MMCFlag);
    OS_TPrintf( "SDHCFlag:0x%x\n", targ->SDHCFlag);
    OS_TPrintf( "SDFlag  :0x%x\n\n", targ->SDFlag);
  
    OS_TPrintf( "ErrStatus:0x%x\n", targ->ErrStatus);
    OS_TPrintf( "Status:0x%x\n\n", targ->Status);
  
    OS_TPrintf( "SD_CLK_CTRL_VALUE:0x%x\n", targ->SD_CLK_CTRL_VALUE);
    OS_TPrintf( "SD_OPTION_VALUE  :0x%x\n\n", targ->SD_OPTION_VALUE);
  
    OS_TPrintf( "OutFlag:0x%x\n\n", targ->OutFlag);
  
    OS_TPrintf( "port_no:0x%x\n", targ->port_no);
}
