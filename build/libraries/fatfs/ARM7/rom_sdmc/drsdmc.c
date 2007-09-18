/*---------------------------------------------------------------------------*
  Project:  TwlBrom - rtfs interface for SD Memory Card
  File:     drsdmc.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include <rtfs.h>
#include <portconf.h>
//#if (INCLUDE_SD)

#include "sdmc_config.h"
//#include "sdmc.h"
#include "sdif_ip.h"
#include "sdif_reg.h"

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


#define NUM_SD_PAGES
#define SD_PAGE_SIZE


/*---------------------------------------------------------------------------*
  extern変数
 *---------------------------------------------------------------------------*/
//extern ER_ID    sdmc_dtq_id;
//extern ER_ID    sdmc_result_dtq_id;
extern void (*func_SDCARD_In)(void);    /* カード挿入イベント用コールバック保存用 */
extern void (*func_SDCARD_Out)(void);   /* カード排出イベント用コールバック保存用 */
extern volatile s16 SDCARD_OutFlag;     /* カード排出発生判定フラグ */

extern int  rtfs_first_stat_flag[26];

/*SDメモリカードのスペック構造体*/
extern SdmcSpec    sdmc_current_spec;


/*---------------------------------------------------------------------------*
  extern関数
 *---------------------------------------------------------------------------*/
extern SDMC_ERR_CODE sdmcGoIdle(void (*func1)(),void (*func2)());


/*---------------------------------------------------------------------------*
  static変数
 *---------------------------------------------------------------------------*/
static int  sdmc_drive_no;
void (*func_usr_sdmc_out)(void) = NULL;    /* カード排出イベントのユーザコールバック */


/*---------------------------------------------------------------------------*
  static関数
 *---------------------------------------------------------------------------*/
void i_sdmcRemovedIntr( void);
static void sdi_get_CHS_params( void);
static u32  sdi_get_ceil( u32 cval, u32 mval);
static void sdi_get_nom( void);
static void sdi_get_fatparams( void);
static void sdi_build_partition_table( void);


/*---------------------------------------------------------------------------*
  Name:         sdmcCheckMedia

  Description:  MBRのシグネチャおよび
                パーティションのフォーマット種別をチェックする

  Arguments:    

  Returns:      TRUE/FALSE
                （FALSEなら pc_format_media が必要）
 *---------------------------------------------------------------------------*/
BOOL sdmcCheckMedia( void)
{
    u16             i;
    SdmcResultInfo  SdResult;
    u8*             bufp;
    u32             buffer[512/4];
    u8              systemid;

    /**/
    if( sdmcReadFifo( buffer, 1, 0, NULL, &SdResult)) {
        return( FALSE);
    }

    bufp = (u8*)buffer;

    /* Check the Signature Word. */
    if( (bufp[510]!=0x55) || (bufp[511]!=0xAA)) {
        return( FALSE);
    }
    /* Check the System ID of partition. */
    systemid = bufp[450];
    if( (systemid!=0x01) && (systemid!=0x04) && (systemid!=0x06) &&
        (systemid!=0x0B) && (systemid!=0x0C)) {
        return( FALSE);
    }
    /* Check the System ID of unuse partitions. */
    for( i=1; i<4; i++) {
        systemid = bufp[450+(16*i)];
        if( systemid != 0x00) {
            return( FALSE);
        }
    }
    /**/
    /*もっと厳密にチェックするならパーティション0開始位置の
        正当性も調べる*/
    return( TRUE);
}


/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsIo

  Description:  上位層からのセクタリード／ライト要求を受ける

  Arguments:    driveno : ドライブ番号
                block : 開始ブロック番号
                buffer : 
                count : ブロック数
                reading : リード要求時にTRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
BOOL sdmcRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading)
{
    u16               result;
    SdmcResultInfo    SdResult;
    
    if( reading) {
        PRINTDEBUG( "DEVCTL_IO_READ ... block:%x, count:%x -> buf:%x\n", block, count, buffer);
        result = sdmcReadFifo( buffer, count, block, NULL, &SdResult);
//        result = sdmcRead( buffer, count, block, NULL, &SdResult);
    }else{
        PRINTDEBUG( "DEVCTL_IO_WRITE ... block:%x, count:%x <- buf:%x\n", block, count, buffer);
        result = sdmcWriteFifo( buffer, count, block, NULL, &SdResult);
//        result = sdmcWrite( buffer, count, block, NULL, &SdResult);
    }
    if( result) {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsCtrl

  Description:  上位層からのコントロール要求を受ける

  Arguments:    driveno : ドライブ番号
                opcode : 要求の種類
                pargs : 

  Returns:
  Memo :        DRIVE_FLAGS_REMOVABLEの場合、ドライバのIO関数を呼ぶ前に
                CTRL関数のDEVCTL_CHECK_STATUSが呼ばれる。
                DEVTEST_CHANGED→DEVTEST_NOCHANGEが確認されるとRTFSはセクタ0を
                読みに行き、FATパラメータを取得した上で目的のセクタを読みに行く。
                CTRL関数の前にはDEVCTL_CHECK_STATUSが呼ばれないので、DEVCTL_
                GET_GEOMETRYでは自前で再挿入をチェックする必要がある。
 *---------------------------------------------------------------------------*/
int sdmcRtfsCtrl( int driveno, int opcode, void* pargs)
{
    DDRIVE       *pdr;
    DEV_GEOMETRY gc;
    int          heads, secptrack;

    pdr = pc_drno_to_drive_struct( driveno);
    
    switch( opcode) {
      case DEVCTL_GET_GEOMETRY:    //formatまたはpartirionするときにRTFSが使うパラメータ
        PRINTDEBUG( "DEVCTL_GET_GEOMETRY\n");
        if( (pdr->drive_flags & DRIVE_FLAGS_INSERTED) == 0) {    /* 抜かれていた場合 */
            if( SDCARD_OutFlag == TRUE) {                        /* 今現在も抜かれているとき */
                return( -1);
            }else{                                               /* 今現在は再挿入されているとき */
                /*-- GoIdleセット --*/
                if( func_SDCARD_Out != i_sdmcRemovedIntr) {
                    func_usr_sdmc_out = func_SDCARD_Out;    //ユーザコールバック取得
                }
                sdmcGoIdle( func_SDCARD_In, i_sdmcRemovedIntr);    //カード初期化シーケンス
                /*------------------*/
            }
        }

        rtfs_memset( &gc, (byte)0, sizeof(gc));
        
        sdi_get_CHS_params();    //最初に呼ぶこと
        sdi_get_fatparams();
        sdi_get_nom();

        PRINTDEBUG( "heads     : 0x%x\n", sdmc_current_spec.heads);
        PRINTDEBUG( "secptrack : 0x%x\n", sdmc_current_spec.secptrack);
        PRINTDEBUG( "cylinders : 0x%x\n", sdmc_current_spec.cylinders);
        PRINTDEBUG( "SC        : 0x%x\n", sdmc_current_spec.SC);
        PRINTDEBUG( "BU        : 0x%x\n", sdmc_current_spec.BU);
        PRINTDEBUG( "RDE       : 0x%x\n", sdmc_current_spec.RDE);
        PRINTDEBUG( "SS        : 0x%x\n", sdmc_current_spec.SS);
        PRINTDEBUG( "RSC       : 0x%x\n", sdmc_current_spec.RSC);
        PRINTDEBUG( "FATBITS   : 0x%x\n", sdmc_current_spec.FATBITS);
        PRINTDEBUG( "SF        : 0x%x\n", sdmc_current_spec.SF);
        PRINTDEBUG( "SSA       : 0x%x\n", sdmc_current_spec.SSA);
        PRINTDEBUG( "NOM       : 0x%x\n", sdmc_current_spec.NOM);
        
        gc.dev_geometry_lbas = (sdmc_current_spec.adjusted_memory_capacity);// - sdmc_current_spec.NOM);
        gc.dev_geometry_heads         = sdmc_current_spec.heads;
        gc.dev_geometry_cylinders     = sdmc_current_spec.cylinders;
        gc.dev_geometry_secptrack     = sdmc_current_spec.secptrack;
        /**/
        gc.fmt_parms_valid     = TRUE;
        gc.fmt.oemname[0]     = 'C';
        gc.fmt.oemname[1]     = 'T';
        gc.fmt.oemname[2]     = 'R';
        gc.fmt.oemname[3]     = '\0';
        gc.fmt.secpalloc     = sdmc_current_spec.SC;    /*sectors per cluster(FIX by capacity)*/
        gc.fmt.secreserved     = sdmc_current_spec.RSC;//sdmc_current_spec.RSC;/*reserved sectors(FIX 1 at FAT12,16)*/
        gc.fmt.numfats         = 2;
        gc.fmt.secpfat        = sdmc_current_spec.SF;
        gc.fmt.numhide         = sdmc_current_spec.NOM;    /**/
        gc.fmt.numroot         = sdmc_current_spec.RDE;    /*FIX*/
        gc.fmt.mediadesc     = 0xF8;
        gc.fmt.secptrk         = sdmc_current_spec.secptrack;    //CHS Recommendation
        gc.fmt.numhead         = sdmc_current_spec.heads;
        gc.fmt.numcyl         = sdmc_current_spec.cylinders;
        gc.fmt.physical_drive_no = driveno;
        gc.fmt.binary_volume_label = BIN_VOL_LABEL;
        gc.fmt.text_volume_label[0] = '\0';
        //TODO:dev_geometry_lbasもセットする必要あるか調べること
        PRINTDEBUG( "heads : 0x%x, secptrack : 0x%x, cylinders : 0x%x\n", gc.dev_geometry_heads, gc.dev_geometry_secptrack, gc.dev_geometry_cylinders);

#if (TARGET_OS_CTR == 1)
        miCpuCopy8( &gc, pargs, sizeof(gc));
//        copybuff( pargs, &gc, sizeof(gc));
#else
        MI_CpuCopy8( &gc, pargs, sizeof(gc));
#endif
        return( 0);
        
      case DEVCTL_FORMAT:
        PRINTDEBUG( "DEVCTL_FORMAT\n");
        sdi_build_partition_table();    //MBRセクタ(パーティションテーブル含む)書き込み
        return( 0);

      case DEVCTL_REPORT_REMOVE:        //抜かれたとき
        PRINTDEBUG( "DEVCTL_REPORT_REMOVE\n");
        pdr->drive_flags &= ~DRIVE_FLAGS_INSERTED;
        return( 0);
        
      case DEVCTL_CHECKSTATUS:    //REMOVABLEの場合、毎回R/W前に呼ばれる
        PRINTDEBUG( "DEVCTL_CHECKSTATUS\n");
        if (!(pdr->drive_flags & DRIVE_FLAGS_REMOVABLE)) {    //リムーバブルでない場合
            return(DEVTEST_NOCHANGE);
        }
        if (pdr->drive_flags & DRIVE_FLAGS_INSERTED) {        /* 抜かれてない場合 */
            if( rtfs_first_stat_flag[driveno]) {    //初回のCHECKSTATUS時
                rtfs_first_stat_flag[driveno] = 0;
                PRINTDEBUG( "CHANGED!\n");
                return(DEVTEST_CHANGED);
            }else{
                PRINTDEBUG( "DEVTEST_NOCHANGE\n");
                return( DEVTEST_NOCHANGE);
            }
        }else{                                                /* 抜かれていた場合 */
            if( SDCARD_OutFlag == FALSE) {                    /* 排出フラグ参照 */
                pdr->drive_flags |= DRIVE_FLAGS_INSERTED;
                /*-- GoIdleセット --*/
                if( func_SDCARD_Out != i_sdmcRemovedIntr) {
                    func_usr_sdmc_out = func_SDCARD_Out;    //ユーザコールバック取得
                }
                sdmcGoIdle( func_SDCARD_In, i_sdmcRemovedIntr);    //カード初期化シーケンス
                /*------------------*/
                PRINTDEBUG( "DEVTEST_CHANGED\n");
                return( DEVTEST_CHANGED);        //挿さっている
            }else{
                PRINTDEBUG( "DEVTEST_NOMEDIA\n");
                return( DEVTEST_NOMEDIA);        //挿さってない
            }
        }
        
      case DEVCTL_WARMSTART:    //attachのときしか呼ばれない
        PRINTDEBUG( "DEVCTL_WARMSTART\n");
        /*-- GoIdleセット --*/
        if( func_SDCARD_Out != i_sdmcRemovedIntr) {
            func_usr_sdmc_out = func_SDCARD_Out;    //ユーザコールバック取得
        }
        sdmcGoIdle( func_SDCARD_In, i_sdmcRemovedIntr);    //カード初期化シーケンス
        /*------------------*/
        pdr->drive_flags |= (DRIVE_FLAGS_VALID | DRIVE_FLAGS_REMOVABLE | DRIVE_FLAGS_PARTITIONED);
        pdr->partition_number = 0;
        
        if( SDCARD_OutFlag == FALSE) {                    /* 排出フラグ参照 */
            pdr->drive_flags |= DRIVE_FLAGS_INSERTED;
        }else{                                            /* カード未挿入のとき */
            pdr->drive_flags &= (~(DRIVE_FLAGS_INSERTED));    //抜かれているだけではVALIDフラグは落とさないらしい
        }
        return( 0);
        
      case DEVCTL_POWER_RESTORE:
        PRINTDEBUG( "DEVCTL_POWER_RESTORE\n");
        break;
        
      case DEVCTL_POWER_LOSS:
        PRINTDEBUG( "DEVCTL_POWER_LOSS\n");
        break;
        
      default:
        PRINTDEBUG( "DEVCTL_unknown\n");
        break;
    }
    return( 0);
}

/*---------------------------------------------------------------------------*
  抜取コールバック関数

  RTFSが次回"DEVCTL_GET_GEOMETRY"を行うとき、ここでセットしたパラメータを知る
 *---------------------------------------------------------------------------*/
void i_sdmcRemovedIntr( void)
{
    DDRIVE    *pdr;
    
    pdr = pc_drno_to_drive_struct( sdmc_drive_no);
    if( pdr) {
        pdr->dev_table_perform_device_ioctl( pdr->driveno,
                                             DEVCTL_REPORT_REMOVE,
                                             (void*)0);
    }
    
    if( func_usr_sdmc_out) {
        func_usr_sdmc_out();    //ユーザコールバック
    }
}


/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsAttach

  Description:  sdmcドライバをドライブに割り当てる

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
BOOL sdmcRtfsAttach( int driveno)
{
    BOOLEAN   result;
    DDRIVE    pdr;

    pdr.dev_table_drive_io     = sdmcRtfsIo;
    pdr.dev_table_perform_device_ioctl = sdmcRtfsCtrl;
    pdr.register_file_address  = (dword) 0; /* Not used  */
    pdr.interrupt_number       = 0;            /* Not used */
    pdr.drive_flags            = 0;//DRIVE_FLAGS_FAILSAFE;
    pdr.partition_number       = 0;            /* Not used */
    pdr.pcmcia_slot_number     = 0;            /* Not used */
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;

    result = rtfs_attach( driveno, &pdr, "SD");    //構造体がFSライブラリ側にコピーされる
    
    /*drivenoをグローバル変数に記憶*/
    sdmc_drive_no = driveno;

    return( result);
}



/*SD File System Specification(仕様書)に基づいた値を出す*/
static void sdi_get_CHS_params( void)
{
    int mbytes;

//    mbytes = (sdmc_current_spec.card_capacity / (1024 * 1024)) * 512;
    mbytes = (sdmc_current_spec.card_capacity >> 11);

    while( 1) {
        if( mbytes <= 2) {
            sdmc_current_spec.heads     = 2;
            sdmc_current_spec.secptrack = 16;
            break;
        }
        if( mbytes <= 16) {
            sdmc_current_spec.heads     = 2;
            sdmc_current_spec.secptrack = 32;
            break;
        }
        if( mbytes <= 32) {
            sdmc_current_spec.heads     = 4;
            sdmc_current_spec.secptrack = 32;
            break;
        }
        if( mbytes <= 128) {
            sdmc_current_spec.heads     = 8;
            sdmc_current_spec.secptrack = 32;
            break;
        }
        if( mbytes <= 256) {
            sdmc_current_spec.heads     = 16;
            sdmc_current_spec.secptrack = 32;
            break;
        }
        if( mbytes <= 504) {
            sdmc_current_spec.heads     = 16;
            sdmc_current_spec.secptrack = 63;
            break;
        }
        if( mbytes <= 1008) {
            sdmc_current_spec.heads     = 32;
            sdmc_current_spec.secptrack = 63;
            break;
        }
        if( mbytes <= 2016) {
            sdmc_current_spec.heads     = 64;
            sdmc_current_spec.secptrack = 63;
            break;
        }
        if( mbytes <= 2048) {
            sdmc_current_spec.heads     = 128;
            sdmc_current_spec.secptrack = 63;
            break;
        }
        if( mbytes <= 4032) {
            sdmc_current_spec.heads     = 128;
            sdmc_current_spec.secptrack = 63;
            break;
        }
        if( mbytes <= 32768) {
            sdmc_current_spec.heads     = 255;
            sdmc_current_spec.secptrack = 63;
            break;
        }
    }

    /*シリンダ数を計算*/
    sdmc_current_spec.cylinders = (sdmc_current_spec.memory_capacity /
                    (sdmc_current_spec.heads * sdmc_current_spec.secptrack));
    
    /*memory_capacityを再計算してadjusted_memory_capacityに格納*/
    sdmc_current_spec.adjusted_memory_capacity = sdmc_current_spec.cylinders *
                    (sdmc_current_spec.heads * sdmc_current_spec.secptrack);
}


/*引数を超える最も小さい整数を算出する*/
static u32 sdi_get_ceil( u32 cval, u32 mval)
{
   return( (cval / mval) + (1 * (cval % mval != 0)));
}


/*マスターブートセクタのセクタ数を返す*/
static void sdi_get_nom( void)
{
    u32 RSC = 1;      //FAT12,16では1
    u32 RDE = 512;    //ルートディレクトリエントリ。FIX
    u32 SS  = 512;    //セクタサイズ。FIX
    u32 TS, SC, n;
    u32 MAX, SFdash;

    TS = sdmc_current_spec.adjusted_memory_capacity;
    SC = sdmc_current_spec.SC;
    
    sdmc_current_spec.SF = sdi_get_ceil( TS/SC * sdmc_current_spec.FATBITS, SS*8);

    /*-----------------------SDHCのとき----------------------------*/
    if( sdmc_current_spec.csd_ver2_flag) {
        sdmc_current_spec.NOM = sdmc_current_spec.BU;
        do {
            n = sdi_get_ceil( 2*sdmc_current_spec.SF, sdmc_current_spec.BU);
            sdmc_current_spec.RSC = (sdmc_current_spec.BU * n) - ( 2 * sdmc_current_spec.SF);
            if( sdmc_current_spec.RSC < 9) {
                sdmc_current_spec.RSC += sdmc_current_spec.BU;
            }
            sdmc_current_spec.SSA = sdmc_current_spec.RSC + (2 * sdmc_current_spec.SF);
            do {
                MAX = ((TS - sdmc_current_spec.NOM - sdmc_current_spec.SSA) / SC) + 1;
                SFdash = sdi_get_ceil( (2+(MAX-1)) * sdmc_current_spec.FATBITS, SS*8);
                if( SFdash > sdmc_current_spec.SF) {
                    sdmc_current_spec.SSA += sdmc_current_spec.BU;
                    sdmc_current_spec.RSC += sdmc_current_spec.BU;
                }else{
                    break;
                }
            }while( 1);
            if( SFdash != sdmc_current_spec.SF) {
                sdmc_current_spec.SF -= 1;
            }else{
                break;
            }
        }while( 1);
    }else{    /*-------------------------SDのとき-------------------------------*/
        do {
            sdmc_current_spec.SSA = RSC + ( 2 * sdmc_current_spec.SF) + sdi_get_ceil( 32*RDE, SS);
            n = sdi_get_ceil( sdmc_current_spec.SSA, sdmc_current_spec.BU);
            sdmc_current_spec.NOM = (sdmc_current_spec.BU * n) - sdmc_current_spec.SSA;
            if( sdmc_current_spec.NOM != sdmc_current_spec.BU) {
                sdmc_current_spec.NOM += sdmc_current_spec.BU;
            }
            do {
                MAX = ((TS - sdmc_current_spec.NOM - sdmc_current_spec.SSA) / SC) + 1;
                SFdash = sdi_get_ceil( (2+(MAX-1)) * sdmc_current_spec.FATBITS, SS*8);
                if( SFdash > sdmc_current_spec.SF) {
                    sdmc_current_spec.NOM += sdmc_current_spec.BU;
                }else{
                    break;
                }
            }while( 1);
            if( SFdash != sdmc_current_spec.SF) {
                sdmc_current_spec.SF = SFdash;
            }else{
                break;    //complete
            }
        }while( 1);
    }

    return;
}

/*FATのビット数を返す*/
static void sdi_get_fatparams( void)
{
    int mbytes;

//    mbytes = (sdmc_current_spec.card_capacity / (1024 * 1024)) * 512;
    mbytes = (sdmc_current_spec.card_capacity >> 11);

    if( mbytes <= 64) {
        sdmc_current_spec.FATBITS = 12;
        sdmc_current_spec.RDE = 512;
        sdmc_current_spec.RSC = 1;
    }else{
        if( mbytes <= 2048) {
            sdmc_current_spec.FATBITS = 16;
            sdmc_current_spec.RDE = 512;
            sdmc_current_spec.RSC = 1;
        }else{
            sdmc_current_spec.FATBITS = 32;
            sdmc_current_spec.RDE = 0;    //FAT32のときは未使用。0にしておかないとRTFSが BAD FORMAT を返す。 
            sdmc_current_spec.RSC = 1;
        }
    }
    
    if( mbytes <= 8) {
        sdmc_current_spec.SC = 16;
        sdmc_current_spec.BU = 16;
        return;
    }
    if( mbytes <= 64) {
        sdmc_current_spec.SC = 32;
        sdmc_current_spec.BU = 32;
        return;
    }
    if( mbytes <= 256) {
        sdmc_current_spec.SC = 32;
        sdmc_current_spec.BU = 64;
        return;
    }
    if( mbytes <= 1024) {
        sdmc_current_spec.SC = 32;
        sdmc_current_spec.BU = 128;
        return;
    }
    if( mbytes <= 2048) {
        sdmc_current_spec.SC = 64;
        sdmc_current_spec.BU = 128;
        return;
    }
    if( mbytes <= 32768) {
        sdmc_current_spec.SC = 64;
        sdmc_current_spec.BU = 8192;
        return;
    }
}

/*MBRセクタ(パーティションセクタ含む)を生成して書き込む*/
static void sdi_build_partition_table( void)
{
    u16 MbrSectDat[512/2];
    u32 starting_head, starting_sect, starting_cyl;
    u32 ending_head, ending_sect, ending_cyl;
    u32 total_sect;
#if (SD_DEBUG_PRINT_ON == 1)    
    u32 starting_data, ending_data;
#endif
    u32 systemid;
    SdmcResultInfo    SdResult;

    /**/
    starting_head = sdmc_current_spec.NOM % (sdmc_current_spec.heads *
                                             sdmc_current_spec.secptrack);
    starting_head /= sdmc_current_spec.secptrack;

    /**/
    starting_sect = (sdmc_current_spec.NOM % sdmc_current_spec.secptrack) + 1;

    /**/
    starting_cyl = sdmc_current_spec.NOM / (sdmc_current_spec.heads *
                                             sdmc_current_spec.secptrack);

    /**/
    total_sect = (sdmc_current_spec.adjusted_memory_capacity - sdmc_current_spec.NOM);
    ending_head = (sdmc_current_spec.NOM + total_sect - 1) %
                    (sdmc_current_spec.heads * sdmc_current_spec.secptrack);
    ending_head /= sdmc_current_spec.secptrack;

    /**/
    ending_sect = ((sdmc_current_spec.NOM + total_sect - 1) %
                    sdmc_current_spec.secptrack) + 1;

    /**/
    ending_cyl = (sdmc_current_spec.NOM + total_sect - 1) /
                    (sdmc_current_spec.heads * sdmc_current_spec.secptrack);

    /**/
    if( sdmc_current_spec.FATBITS == 32) {    //FAT32のとき
        if( total_sect < 0xFB0400) {        //8032.5MBが閾値(SD FileSystemSpec2.00参照)
            systemid = 0x0B;        /* FAT32 */
        }else{
            systemid = 0x0C;        /* FAT32(拡張INT13対応) */
        }
    }else{                                    //FAT12,FAT16のとき
        if( total_sect < 32680) {
            systemid = 0x01;        /* FAT12 */
        }else if( total_sect < 65536) {
            systemid = 0x04;        /* FAT16(16MB～32MB未満) */
        }else{
            systemid = 0x06;        /* FAT16(32MB～4GB) */
        }
    }
        
    /*MBRセクタ(パーティションテーブル含む)作成*/
#if (TARGET_OS_CTR == 1)
    miCpuFill8( MbrSectDat, 0, 512);
#else
    MI_CpuFill8( MbrSectDat, 0, 512);
#endif
    MbrSectDat[446/2] = (starting_head<<8);
    //上位8bit:starting_cylの下位8bit, 下位8bit:starting_cylの上位2bit + starting_sect 6bit.
    MbrSectDat[448/2] = (starting_cyl<<8) + ((starting_cyl>>2) & 0xC0) + starting_sect;
    MbrSectDat[450/2] = (ending_head<<8) + systemid;
    //上位8bit:ending_cylの下位8bit, 下位8bit:ending_cylの上位2bit + ending_sect 6bit.
    MbrSectDat[452/2] = (ending_cyl<<8) + ((ending_cyl>>2) & 0xC0) + ending_sect;
    MbrSectDat[454/2] = sdmc_current_spec.NOM;
    MbrSectDat[456/2] = (sdmc_current_spec.NOM>>16);
    MbrSectDat[458/2] = total_sect;
    MbrSectDat[460/2] = (total_sect>>16);
    MbrSectDat[510/2] = 0xAA55;
    /*セクタ0に書き込み*/
    sdmcWriteFifo( MbrSectDat, 1, 0, NULL, &SdResult);
    
    /**/
    PRINTDEBUG( "total    sect : 0x%x\n", total_sect);
    PRINTDEBUG( "starting head : 0x%x\n", starting_head);
    PRINTDEBUG( "starting sect : 0x%x\n", starting_sect);
    PRINTDEBUG( "starting cyl  : 0x%x\n", starting_cyl);
    PRINTDEBUG( "ending   head : 0x%x\n", ending_head);
    PRINTDEBUG( "ending   sect : 0x%x\n", ending_sect);
    PRINTDEBUG( "ending   cyl  : 0x%x\n", ending_cyl);
    PRINTDEBUG( "\n");
#if (SD_DEBUG_PRINT_ON == 1)    
    starting_data = (starting_cyl<<8) + ((starting_cyl>>2) & 0xC0) + starting_sect;
    PRINTDEBUG( "starting data : 0x%x\n", starting_data);
    ending_data = (ending_cyl<<8) + ((ending_cyl>>2) & 0xC0) + ending_sect;
    PRINTDEBUG( "endign   data : 0x%x\n", ending_data);
#endif
}

//#endif /*(INCLUDE_SD)*/
