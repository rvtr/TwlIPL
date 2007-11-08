/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fatfs
  File:     fatfs_firm.c

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

#include <symbols.h>

#include <firm.h>
#include <rtfs.h>
#include <devices/sdif_reg.h>
#include <devices/sdif_ip.h>
#include <devices/sdmc_config.h>
#include <twl/devices/sdmc/ARM7/sdmc.h>

//#define USE_SPECULATIVE_READ

extern u32 NAND_FAT_PARTITION_COUNT;

#define DMA_PIPE         2
#define DMA_RECV         3

/*
    専用SD関数
*/
extern volatile SDMC_ERR_CODE   SDCARD_ErrStatus;
extern s16  SDCARD_SDHCFlag;          /* SDHCカードフラグ */
extern SDPortContext*   SDNandContext;  /* NAND初期化パラメータ */
#ifdef USE_SPECULATIVE_READ
static u32 currentSector;
#endif
/*---------------------------------------------------------------------------*
  Name:         WaitFifoFull

  Description:  waiting to fill the SD FIFO

                SDカードからの読み込みデータがFIFOから読み込める状態になるまで
                ストールします。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void WaitFifoFull( void )
{
    while( (*SDIF_CNT & SDIF_CNT_FULL) == 0 )
    {
        if ( SDCARD_ErrStatus != SDMC_NORMAL )   // an error was occurred
        {
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StopToRead

  Description:  stop to read from SD I/F

                SDカードからの読み込みの完了処理を行います。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void StopToRead( void )
{
#ifdef NAND_FAT_PARTITION_COUNT
    if ( *SDIF_CNT & SDIF_CNT_USEFIFO )
    {
        SD_AndFPGA( SD_INFO2,(~(SD_INFO2_ERROR_SET)));              /* SD_INFO2のエラーフラグを全て落とす */
        SD_OrFPGA( SD_INFO2_MASK, SD_INFO2_MASK_ERRSET);        /* 全てのエラー割り込みを禁止 */
        SD_OrFPGA( SD_INFO1_MASK, SD_INFO1_MASK_ALL_END);       /* INFO1の access all end 割込み禁止 */
        SD_TransEndFPGA();                      /* 強制的にカード転送の終了処理 */
        SD_StopTransmission();                  /* 強制的にカード転送終了設定 */
        // SDCARD_EndFlag
        if ( SDCARD_ErrStatus )
        {
            //OS_TPrintf("R1_STATUS (1st).\n");
            SD_SendStatus();                   /* CMD13 addressed card sends its status register 発行、レスポンス待ち */

            if( !SDCARD_ErrStatus )              /* エラーステータスの確認（エラー有り？）*/
            {
                u16 usRSP0;
                //OS_TPrintf("R1_STATUS (2nd).\n");
                SD_GetFPGA( usRSP0, SD_RSP0);
                usRSP0 = (u16)(( usRSP0 & RSP_R1_CURRENT_STATE) >> 1);               /* カレントステートを取り出す */
                if((usRSP0 == CURRENT_STATE_DATA) || (usRSP0 == CURRENT_STATE_RCV)){ /* SDCARD Status が data rcv の時 */
                    SD_Command(SD_CMD_CMD | STOP_TRANSMISSION);                      /* CMD12（StopTransmission）発行処理 */
                }
            }
        }
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS))       /* コマンドレスポンス(R1)のカードステータスがエラーでないか? */
        {
            //OS_TPrintf("R1_STATUS (3rd).\n");
            SD_CheckStatus(TRUE);                          /* コマンドレスポンス(R1)の Card Status チェック */
            if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS))   /* コマンドレスポンス(R1)のカードステータスがエラーでないか? */
            {
                //OS_TPrintf("R1_STATUS (4th).\n");
                SD_SendStatus();                           /* カードステータスの取得コマンド発行 */
                SD_CheckStatus(TRUE);                      /* コマンドレスポンス(R1)の Card Status チェック */
            }
        }
        if( !SD_CheckFPGAReg( SD_STOP,SD_STOP_SEC_ENABLE ) ){
            //OS_TPrintf("SD_StopTransmission.\n");
            SD_StopTransmission();      /* カード転送終了をFPGAに通知（CMD12発行） */
        }
        SD_DisableClock();              /* クロック供給停止 */

        *SDIF_CNT = (*SDIF_CNT & ~SDIF_CNT_USEFIFO) | SDIF_CNT_FCLR; /* FIFO使用フラグOFF */
        CC_EXT_MODE = CC_EXT_MODE_PIO;  /* PIOモード(DMAモードOFF) */
        //OS_TPrintf("DONE\n");
    }
#else
    if ( *SDIF_CNT & SDIF_CNT_USEFIFO )
    {
        if( !SD_CheckFPGAReg( SD_STOP,SD_STOP_SEC_ENABLE ) ){
            SD_StopTransmission();      /* カード転送終了をFPGAに通知（CMD12発行） */
        }
        SD_TransEndFPGA();              /* 転送終了処理(割り込みマスクを禁止に戻す) */
        SD_DisableClock();              /* クロック供給停止 */

        *SDIF_CNT = (*SDIF_CNT & ~SDIF_CNT_USEFIFO) | SDIF_CNT_FCLR; /* FIFO使用フラグOFF */
        CC_EXT_MODE = CC_EXT_MODE_PIO;  /* PIOモード(DMAモードOFF) */
    }
#endif
}

/*---------------------------------------------------------------------------*
  Name:         StartToRead

  Description:  start to read from SD I/F

                SDカードからの読み込みの開始処理を行います。

  Arguments:    block       begining sector to transfer
                count       number of setctors to transfer

  Returns:      None
 *---------------------------------------------------------------------------*/
static void StartToRead(u32 block, u32 count)
{
#ifdef USE_SPECULATIVE_READ
    if ( currentSector == block )
    {
        return;
    }
    StopToRead();
    currentSector = block;
    count = 0xFFFF;
#endif
    *SDIF_FSC = count;
    *SDIF_FDS = SECTOR_SIZE;
    *SDIF_CNT = (*SDIF_CNT & ~(SDIF_CNT_FEIE | SDIF_CNT_FFIE)) | SDIF_CNT_FCLR | SDIF_CNT_USEFIFO;
    CC_EXT_MODE = CC_EXT_MODE_DMA;

    SDCARD_ErrStatus = SDMC_NORMAL;
    SD_EnableClock();
    SD_EnableSeccnt(count);
    if ( SDCARD_SDHCFlag )
    {
        SD_MultiReadBlock( block );
    }
    else
    {
        SD_MultiReadBlock( block * SECTOR_SIZE );
    }
}

/*
    FATFS-SDMCの間にAESを組み込む
    一部の設定は、FATFSを迂回して設定することになる。
*/

#define AES_GET_CNT_BITS(regValue, name)                        \
    ((regValue) & (REG_AES_AES_CNT_##name##_MASK))

static BOOL useAES = FALSE;
static AESCounter   aesCounter;

/*---------------------------------------------------------------------------*
  Name:         FATFS_EnableAES

  Description:  enable AES data path

                次に読み込むデータがAES暗号化されていることを、外部から
                IO関数に通知するためのAPIです。

                このAPIを呼び出した直後に読み込むデータのAESの初期値を指定
                しておけば、以後のシーケンシャルな呼び出しでの初期値は自動
                計算されます。
                ランダムアクセスを行う場合は、そのたびにこのAPIを呼び出す
                必要があります。

  Arguments:    counter     initial counter value

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_EnableAES( const AESCounter* pCounter )
{
    useAES = TRUE;
    aesCounter = *pCounter;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_DisableAES

  Description:  bypass AES

                次に読み込むデータがAES暗号化されていないことを、外部から
                IO関数に通知するためのAPIです。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_DisableAES( void )
{
    useAES = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ReadNormal

  Description:  normal read

                普通にNAND/SDカードを読み込みます。

                割り込み禁止状態で高速に動作する仕様になっています。

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      0 if success
 *---------------------------------------------------------------------------*/
static u16 ReadNormal(u32 block, void *dest, u16 count)
{
#if 1   // use TIMING_SD_1 or not
    MINDmaConfig config =
    {
        MI_NDMA_NO_INTERVAL,
        MI_NDMA_INTERVAL_PS_1,
        MI_NDMA_BWORD_128,
        SECTOR_SIZE/4
    };
//    OS_TPrintf("ReadNormal(0x%X, 0x%08X, 0x%X) is calling.\n", block, dest, count);
#ifdef USE_SPECULATIVE_READ
    if (block == currentSector)
    {
        //StartToRead( block, 1 );
        WaitFifoFull();
        MI_NDmaRecvAsync( DMA_PIPE, SDIF_FI, dest, SECTOR_SIZE, NULL, NULL );
        block++;
        dest = (u8*)dest + SECTOR_SIZE;
        count--;
        currentSector++;
        if (count == 0)
        {
            MI_WaitNDma( DMA_PIPE );
            return SDCARD_ErrStatus;
        }
        // ここまでで次のFIFO FULL前であると仮定しているが、問題ない？
    }
#endif
    MI_NDmaRecvExAsync_Dev( DMA_RECV, SDIF_FI, dest, (u32)(count * SECTOR_SIZE), NULL, NULL, &config, MI_NDMA_TIMING_SD_1 );
    StartToRead( block, count );
    if ( SDCARD_ErrStatus != SDMC_NORMAL )
    {
        goto err;
    }
#else
    u32 offset = 0; // in bytes

//    OS_TPrintf("ReadNormal(0x%X, 0x%08X, 0x%X) is calling.\n", block, dest, count);
    MI_NDmaRecvAsync_SetUp( DMA_RECV, (void*)SDIF_FI, dest, SECTOR_SIZE, NULL, NULL );
    StartToRead( block, count );
    if ( SDCARD_ErrStatus != SDMC_NORMAL )
    {
        goto err;
    }
    while ( count * SECTOR_SIZE > offset )
    {
        WaitFifoFull();
        if ( SDCARD_ErrStatus != SDMC_NORMAL )
        {
            goto err;
        }
//        MI_NDMA_REG( DMA_RECV, MI_NDMA_REG_DAD_WOFFSET ) = (u32)dest + offset;
        MIi_SetNDmaDest( DMA_RECV, (u8*)dest + offset );
        MI_NDmaRestart( DMA_RECV );
        offset += SECTOR_SIZE;
    }
#endif

    MI_WaitNDma( DMA_RECV );
#ifdef USE_SPECULATIVE_READ
    currentSector += count;
#else
    StopToRead();
#endif
    return SDCARD_ErrStatus;

err:
    MI_StopNDma( DMA_RECV );
#ifdef USE_SPECULATIVE_READ
    currentSector = 0xFFFFFFFF;
#endif
    StopToRead();
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         ReadAES

  Description:  AES read

                AESをかけながらNAND/SDカードを読み込みます。
                AESの鍵の設定はあらかじめ行っておく必要があります。
                AESの初期値の設定は、FATFS_EnableAESの引数から計算されます。

                割り込み禁止状態で高速に動作する仕様になっています。

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      0 if success
 *---------------------------------------------------------------------------*/
#define PIPE_SIZE   64
static u16 ReadAES(u32 block, void *dest, u16 count)
{
    u32 offset = 0; // in bytes

//    OS_TPrintf("ReadAES(0x%X, 0x%08X, 0x%X) is calling.\n", block, dest, count);
    MI_NDmaPipeAsync_SetUp( DMA_PIPE, (void*)SDIF_FI, (void*)REG_AES_IFIFO_ADDR, PIPE_SIZE, NULL, NULL );
    StartToRead( block, count );
    if ( SDCARD_ErrStatus != SDMC_NORMAL )
    {
        goto err;
    }

/*
    AESのセットアップ＆出力DMA設定
*/
    AESi_Reset();
    AESi_Reset();
    AESi_DmaRecv( DMA_RECV, dest, (u32)(count * SECTOR_SIZE), NULL, NULL );
    AESi_SetCounter( &aesCounter );
    AESi_Run( AES_MODE_CTR, 0, (u32)(count * SECTOR_SIZE / AES_BLOCK_SIZE), NULL, NULL );

    // update for next read
    AESi_AddCounter( &aesCounter, (u32)(count * SECTOR_SIZE / AES_BLOCK_SIZE) );


    while ( count * SECTOR_SIZE > offset )
    {
        while ( AES_GET_CNT_BITS( reg_AES_AES_CNT, IFIFO_CNT ) )
        {
        }
        if ( (offset & (SECTOR_SIZE-1)) == 0 )
        {
            WaitFifoFull();
            if ( SDCARD_ErrStatus != SDMC_NORMAL )
            {
                goto err;
            }
        }
        MI_NDmaRestart( DMA_PIPE );
        offset += PIPE_SIZE;
    }
    MI_WaitNDma( DMA_PIPE );
#ifdef USE_SPECULATIVE_READ
    currentSector += count;
#else
    StopToRead();
#endif
    MI_WaitNDma( DMA_RECV );
    return SDCARD_ErrStatus;

err:
    MI_StopNDma( DMA_RECV );
    MI_StopNDma( DMA_PIPE );
#ifdef USE_SPECULATIVE_READ
    currentSector = 0xFFFFFFFF;
#endif
    StopToRead();
    AESi_Reset();
    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         nandRtfsIoFirm

  Description:  上位層からのセクタリード／ライト要求を受ける

                Readに対してのみ、独自の関数を使用するドライバのIO関数です。
                AESの有無の判定は断片化時に未検証です。
                (間に論理領域の読み込みが挟まる可能性があるかも)

  Arguments:    driveno : ドライブ番号
                block : 開始ブロック番号
                buffer :
                count : ブロック数
                reading : リード要求時にTRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
static BOOL nandRtfsIoFirm( int driveno, u32 block, void* buffer, u16 count, BOOL reading )
{
    u16 result;
#pragma unused( driveno)

    sdmcSelect( (u16)SDMC_PORT_NAND);

    if( reading )
    {
        result = useAES ?
                ReadAES( block, buffer, count ) :
                ReadNormal( block, buffer, count );
    }
    else
    {
        SdmcResultInfo SdResult;
        result = sdmcWriteFifo( buffer, count, block, NULL, &SdResult );
    }

    return result ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsIoFirm

  Description:  上位層からのセクタリード／ライト要求を受ける

                Readに対してのみ、独自の関数を使用するドライバのIO関数です。

  Arguments:    driveno : ドライブ番号
                block : 開始ブロック番号
                buffer :
                count : ブロック数
                reading : リード要求時にTRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
static BOOL sdmcRtfsIoFirm( int driveno, u32 block, void* buffer, u16 count, BOOL reading )
{
    u16 result;
#pragma unused( driveno)

    sdmcSelect( (u16)SDMC_PORT_CARD );

    if( reading )
    {
        result = useAES ?
                ReadAES( block, buffer, count ) :
                ReadNormal( block, buffer, count );
    }
    else
    {
        SdmcResultInfo SdResult;
        result = sdmcWriteFifo( buffer, count, block, NULL, &SdResult );
    }

    return result ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         nandRtfsAttachFirm

  Description:  sdmcドライバをドライブに割り当てる

                独自のIO関数を使用するように初期化します。

  Arguments:    driveno : ドライブ番号

  Returns:
 *---------------------------------------------------------------------------*/
#define nandRtfsCtrl                    FATFSi_nandRtfsCtrl
extern int nandRtfsCtrl( int driveno, int opcode, void* pargs);
static BOOL nandRtfsAttachFirm( int driveno, int partition_no)
{
    BOOLEAN   result;
    DDRIVE    pdr;

    pdr.dev_table_drive_io     = nandRtfsIoFirm;
    pdr.dev_table_perform_device_ioctl = nandRtfsCtrl;
    pdr.register_file_address  = (dword) 0;    /* Not used */
    pdr.interrupt_number       = 0;            /* Not used */
    pdr.drive_flags            = (DRIVE_FLAGS_VALID | DRIVE_FLAGS_PARTITIONED);//DRIVE_FLAGS_FAILSAFE;
    pdr.partition_number       = partition_no; /* Not used */
    pdr.pcmcia_slot_number     = 0;            /* Not used */
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;

    switch( partition_no )
    {
    case 0:
        result = rtfs_attach( driveno, &pdr, "SD1p0" ); //構造体がFSライブラリ側にコピーされる
        break;
    case 1:
        result = rtfs_attach( driveno, &pdr, "SD1p1" ); //構造体がFSライブラリ側にコピーされる
        break;
    case 2:
        result = rtfs_attach( driveno, &pdr, "SD1p2" ); //構造体がFSライブラリ側にコピーされる
        break;
    case 3:
        result = rtfs_attach( driveno, &pdr, "SD1p3" ); //構造体がFSライブラリ側にコピーされる
        break;
    default:
        result = FALSE;
        break;
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsAttachFirm

  Description:  sdmcドライバをドライブに割り当てる

                独自のIO関数を使用するように初期化します。

  Arguments:    driveno : ドライブ番号

  Returns:
 *---------------------------------------------------------------------------*/
#define sdmcRtfsCtrl                    FATFSi_sdmcRtfsCtrl
extern int sdmcRtfsCtrl( int driveno, int opcode, void* pargs);
static BOOL sdmcRtfsAttachFirm( int driveno)
{
    BOOLEAN   result;
    DDRIVE    pdr;

    pdr.dev_table_drive_io     = sdmcRtfsIoFirm;
    pdr.dev_table_perform_device_ioctl = sdmcRtfsCtrl;
    pdr.register_file_address  = (dword) 0; /* Not used  */
    pdr.interrupt_number       = 0;            /* Not used */
    pdr.drive_flags            = 0;//DRIVE_FLAGS_FAILSAFE;
    pdr.partition_number       = 0;            /* Not used */
    pdr.pcmcia_slot_number     = 0;            /* Not used */
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;

    result = rtfs_attach( driveno, &pdr, "SD0" );    //構造体がFSライブラリ側にコピーされる

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_InitFIRM

  Description:  init file system

                FATFSを初期化します。
                標準のFATFS_Initの使用しない部分を省略しているだけです。
                以前のNANDコンテキストが残っているなら、初期化時間を著しく
                短縮できます。
                FATFS用のメモリヒープ(OS_ARENA_MAIN_SUBPRIVの関連とヒープ)は
                あらかじめ確保しておいてください。

  Arguments:    nandContext     context of nand driver's previous life

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_InitFIRM( void* nandContext )
{
    /* RTFSライブラリを初期化 */
    if( !rtfs_init() )
    {
        return FALSE;
    }

    /* NAND初期化パラメータの設定 */
    SDNandContext = (SDPortContext*)nandContext;

#ifdef USE_SPECULATIVE_READ
    currentSector = 0xFFFFFFFF;
#endif

    /* SDドライバ初期化 */
    if ( sdmcInit( SDMC_NOUSE_DMA ) != SDMC_NORMAL )
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountDriveFIRM

  Description:  mount specified partition

                指定したデバイスの指定したパーティションを指定したドライブに
                マウントします。NANDまたはSDカードのみ対応しています。
                このAPIでマウントした場合、独自のIO関数を使用するようになります。

  Arguments:    driveno         drive number "A:" is 0
                media           media type
                partition_no    pertition number

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountDriveFIRM( int driveno, FATFSMediaType media, int partition_no )
{
    if ( media == FATFS_MEDIA_TYPE_NAND )
    {
        // CAUTION!: 同じ関数を2回呼び出す理由について要確認。
        if ( !nandRtfsAttachFirm( driveno, partition_no ) || nandRtfsAttachFirm( driveno, partition_no ) )
        {
            return FALSE;
        }
    }
    else
    {
        if ( partition_no )   // support only 0
        {
            return FALSE;
        }
        // CAUTION!: 同じ関数を2回呼び出す理由について要確認。
        if ( !sdmcRtfsAttachFirm( driveno ) || sdmcRtfsAttachFirm( driveno ) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_UnmountDriveFIRM

  Description:  unmount specified partition

                特殊なドライバの終了処理をします。

  Arguments:    driveno         drive number "A:" is 0

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_UnmountDriveFIRM( int driveno )
{
#ifdef USE_SPECULATIVE_READ
    StopToRead();
    currentSector = 0xFFFFFFFF;
#endif
    return TRUE;
    return rtfs_detach( driveno );
}
