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

#include <firm/fatfs.h>
#include <firm/sdmc.h>
#include <firm/format/format_rom.h>
#include <twl/aes.h>
#include <twl/aes/ARM7/lo.h>
#include <rtfs.h>

#include <twl/os/ARM7/debugLED.h>

extern u32 NAND_FAT_PARTITION_COUNT;

#define DMA_PIPE         2
#define DMA_RECV         3

/*
    専用DMA関数
*/
#define MIi_SRC_INC         (MI_NDMA_SRC_INC      | MI_NDMA_SRC_RELOAD_DISABLE)
#define MIi_SRC_DEC         (MI_NDMA_SRC_DEC      | MI_NDMA_SRC_RELOAD_DISABLE)
#define MIi_SRC_FIX         (MI_NDMA_SRC_FIX      | MI_NDMA_SRC_RELOAD_DISABLE)
#define MIi_SRC_FILLDATA    (MI_NDMA_SRC_FILLDATA | MI_NDMA_SRC_RELOAD_DISABLE)

#define MIi_DEST_INC        (MI_NDMA_DEST_INC     | MI_NDMA_DEST_RELOAD_DISABLE)
#define MIi_DEST_DEC        (MI_NDMA_DEST_DEC     | MI_NDMA_DEST_RELOAD_DISABLE)
#define MIi_DEST_FIX        (MI_NDMA_DEST_FIX     | MI_NDMA_DEST_RELOAD_DISABLE)
#define MIi_DEST_INC_RELOAD (MI_NDMA_SRC_INC      | MI_NDMA_DEST_RELOAD_ENABLE)

#define MIi_IMM             (MI_NDMA_IMM_MODE_ON)

#define MIi_CONT            (MI_NDMA_CONTINUOUS_ON)
//---------------- register setting
static inline void MIi_SetSrc( u32 ndmaNo, u32 src )
{
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_SAD_WOFFSET ) = src;
}
static inline void MIi_SetDest( u32 ndmaNo, u32 dest )
{
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_DAD_WOFFSET ) = dest;
}
static inline void MIi_SetTotalWordCount( u32 ndmaNo, u32 size )
{
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_TCNT_WOFFSET ) = size;
}
static inline void MIi_SetWordCount( u32 ndmaNo, u32 size )
{
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_WCNT_WOFFSET ) = size;
}
static inline void MIi_SetInterval( u32 ndmaNo, u32 intervalTimer, u32 prescaler )
{
#ifdef SDK_ARM7
    //---- In case of ARM7, intervalTimer==1 is nonsense
    SDK_ASSERT(intervalTimer != 1);
#endif
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_BCNT_WOFFSET ) = intervalTimer | prescaler;
}
static inline void MIi_SetFillData( u32 ndmaNo, u32 data )
{
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_FDATA_WOFFSET ) = data;
}
static inline void MIi_SetControl( u32 ndmaNo, u32 contData )
{
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) = contData;
}
static inline void MIi_NDmaRecv(u32 ndmaNo, const void *src, void *dest, u32 size)
{
    MIi_SetSrc( ndmaNo, (u32)src );
    MIi_SetDest( ndmaNo, (u32)dest );
    MIi_SetInterval( ndmaNo, MI_NDMA_NO_INTERVAL, MI_NDMA_INTERVAL_PS_1 );
    MIi_SetTotalWordCount( ndmaNo, size/4 );
    MIi_SetWordCount( ndmaNo, size/4 );
    MIi_SetControl( ndmaNo, MI_NDMA_BWORD_16 | MI_NDMA_SRC_FIX | MIi_DEST_INC | MIi_IMM | MI_NDMA_ENABLE );
}

static inline void MIi_NDmaPipeSetup(u32 ndmaNo, const void *src, void *dest, u32 size)
{
    MIi_SetSrc( ndmaNo, (u32)src );
    MIi_SetDest( ndmaNo, (u32)dest );
    MIi_SetInterval( ndmaNo, MI_NDMA_NO_INTERVAL, MI_NDMA_INTERVAL_PS_1 );
    MIi_SetTotalWordCount( ndmaNo, size/4 );
    MIi_SetWordCount( ndmaNo, size/4 );
    MIi_SetControl( ndmaNo, MI_NDMA_BWORD_16 | MI_NDMA_SRC_FIX | MIi_DEST_FIX | MIi_IMM );
}

static inline void MIi_NDmaRestart(u32 ndmaNo)
{
    MI_WaitNDma( ndmaNo );
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) |= MI_NDMA_ENABLE;
}

/*
    専用NAND関数
*/
//extern void SDCARD_TimerStart(u32 tim);    /* タイムアウト計測スタート */
extern volatile SDMC_ERR_CODE   SDCARD_ErrStatus;
extern s16  SDCARD_SDHCFlag;          /* SDHCカードフラグ */

static inline void WaitFifoFull( void )
{
//    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);   /* タイムアウト判定用タイマスタート(2000msec) */
    while( (*SDIF_CNT & SDIF_CNT_FULL) == 0 )
    {
        if ( SDCARD_ErrStatus != SDMC_NORMAL )   // an error was occurred
        {
            break;
        }
    }
}

static void StartToRead(u32 block, u32 count)
{
    *SDIF_FSC = count;
    *SDIF_FDS = SECTOR_SIZE;
    *SDIF_CNT = (*SDIF_CNT & ~SDIF_CNT_FEIE) | SDIF_CNT_FFIE | SDIF_CNT_FCLR | SDIF_CNT_USEFIFO;
    CC_EXT_MODE = CC_EXT_MODE_DMA;

    SDCARD_ErrStatus = SDMC_NORMAL;
//    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);   /* タイムアウト判定用タイマスタート(2000msec) */
    SD_EnableClock();
    SD_EnableSeccnt(count);
    if ( SDCARD_SDHCFlag )
    {
        SD_MultiReadBlock( block );
    }
    else
    {
        SD_MultiReadBlock(block * SECTOR_SIZE);
    }
}

static void StopToRead( void )
{
    if( !SD_CheckFPGAReg(SD_STOP,SD_STOP_SEC_ENABLE) ){
        SD_StopTransmission();      /* カード転送終了をFPGAに通知（CMD12発行） */
    }
    SD_TransEndFPGA();              /* 転送終了処理(割り込みマスクを禁止に戻す) */
    SD_DisableClock();              /* クロック供給停止 */

    *SDIF_CNT = (*SDIF_CNT & ~SDIF_CNT_USEFIFO) | SDIF_CNT_FCLR; /* FIFO使用フラグOFF */
    CC_EXT_MODE = CC_EXT_MODE_PIO;  /* PIOモード(DMAモードOFF) */
}

#define AES_GET_CNT_BITS(regValue, name)                        \
    ((regValue) & (REG_AES_AES_CNT_##name##_MASK))

static BOOL useAES = FALSE;

/*---------------------------------------------------------------------------*
  Name:         FATFS_EnableAES

  Description:  enable AES data path

  Arguments:    slot        aes key slot number
                counter     initial counter value

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_EnableAES( AESKeySlot slot, const AESCounter* pCounter )
{
    useAES = TRUE;
    AESi_WaitKey();
    AESi_LoadKey( slot );
    AESi_SetCounter( pCounter );
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_DisableAES

  Description:  bypass AES

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

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      None
 *---------------------------------------------------------------------------*/
static void ReadNormal(u32 block, void *dest, u16 count)
{
OS_SetDebugLED((u8)(0x80 | block));
    //MI_StopNDma( DMA_PIPE );   // already stopped
    //MI_StopNDma( DMA_RECV );   // already stopped

    StartToRead( block, count );
    if ( SDCARD_ErrStatus != SDMC_NORMAL )
    {
        return;
    }

    while ( count-- )   // TODO: 自動起動DMA1つで十分なはず
    {
        WaitFifoFull();
        MIi_NDmaRecv( DMA_PIPE, (void*)SDIF_FI, dest, SECTOR_SIZE );
        MI_WaitNDma( DMA_PIPE );
        dest = (void*)((u32)dest + SECTOR_SIZE);
    }
    StopToRead();
OS_SetDebugLED((u8)(0x90 | block));
}

/*---------------------------------------------------------------------------*
  Name:         ReadAES

  Description:  AES read

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      None
 *---------------------------------------------------------------------------*/
#define PIPE_SIZE   64
static void ReadAES(u32 block, void *dest, u16 count)
{
    u32 offset = 0; // in bytes
OS_SetDebugLED((u8)(0xC0 | block));
    //MI_StopNDma( DMA_PIPE );   // already stopped
    //MI_StopNDma( DMA_RECV );   // already stopped

    MIi_NDmaPipeSetup( DMA_PIPE, (void*)SDIF_FI, (void*)REG_AES_IFIFO_ADDR, PIPE_SIZE );

/*
    AESのセットアップ＆出力DMA設定
*/
    AESi_Reset();
    AESi_Reset();
    AESi_DmaRecv( DMA_RECV, dest, (u32)(count * SECTOR_SIZE), NULL, NULL );
//  AESi_SetCounter( &aesCounter ); // remain???
//  FATFSi_AddCounter( count * SECTOR_SIZE );  // update for next read
    AESi_Run( AES_MODE_CTR, 0, (u32)(count * SECTOR_SIZE / AES_BLOCK_SIZE), NULL, NULL );

    StartToRead( block, count );
    if ( SDCARD_ErrStatus != SDMC_NORMAL )
    {
        return;
    }

    while ( block * SECTOR_SIZE > offset )
    {
        while ( AES_GET_CNT_BITS( reg_AES_AES_CNT, IFIFO_CNT ) )
        {
        }
        if ( (offset & SECTOR_SIZE) == 0 )
        {
            WaitFifoFull();
        }
        MIi_NDmaRestart( DMA_PIPE );
        offset += PIPE_SIZE;
    }
    MI_WaitNDma( DMA_PIPE );
    StopToRead();
    MI_WaitNDma( DMA_RECV );
OS_SetDebugLED((u8)(0xD0 | block));
}

/*---------------------------------------------------------------------------*
  Name:         nandRtfsIoFirm

  Description:  上位層からのセクタリード／ライト要求を受ける

  Arguments:    driveno : ドライブ番号
                block : 開始ブロック番号
                buffer :
                count : ブロック数
                reading : リード要求時にTRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
static BOOL nandRtfsIoFirm( int driveno, u32 block, void* buffer, u16 count, BOOL reading)
{
    u16               result;
    SdmcResultInfo    SdResult;
#pragma unused( driveno)

    /**/
    sdmcSelect( (u16)SDMC_PORT_NAND);

    if( reading) {
        if (useAES)
        {
            ReadAES(block, buffer, count);
        }
        else
        {
            ReadNormal(block, buffer, count);
        }
        result = 0; // always success
//        result = sdmcReadFifo( buffer, count, block, NULL, &SdResult);
//        result = sdmcRead( buffer, count, block, NULL, &SdResult);
    }else{
        result = sdmcWriteFifo( buffer, count, block, NULL, &SdResult);
//        result = sdmcWrite( buffer, count, block, NULL, &SdResult);
    }
    if( result) {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsIoFirm

  Description:  上位層からのセクタリード／ライト要求を受ける

  Arguments:    driveno : ドライブ番号
                block : 開始ブロック番号
                buffer :
                count : ブロック数
                reading : リード要求時にTRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
static BOOL sdmcRtfsIoFirm( int driveno, u32 block, void* buffer, u16 count, BOOL reading)
{
    u16               result;
    SdmcResultInfo    SdResult;
#pragma unused( driveno)

    /**/
    sdmcSelect( (u16)SDMC_PORT_CARD);

    if( reading) {
        if (useAES)
        {
            ReadAES(block, buffer, count);
        }
        else
        {
            ReadNormal(block, buffer, count);
        }
        result = 0; // always success
//        result = sdmcReadFifo( buffer, count, block, NULL, &SdResult);
//        result = sdmcRead( buffer, count, block, NULL, &SdResult);
    }else{
        result = sdmcWriteFifo( buffer, count, block, NULL, &SdResult);
//        result = sdmcWrite( buffer, count, block, NULL, &SdResult);
    }
    if( result) {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         nandRtfsAttachFirm

  Description:  sdmcドライバをドライブに割り当てる

  Arguments:    driveno : ドライブ番号

  Returns:
 *---------------------------------------------------------------------------*/
#define nandRtfsCtrl                    FATFSi_nandRtfsCtrl
extern int nandRtfsCtrl( int driveno, int opcode, void* pargs);
static BOOL nandRtfsAttachFirm( int driveno, int partition_no)
{
    BOOLEAN   result;
    DDRIVE    pdr;

    if( partition_no >= NAND_FAT_PARTITION_COUNT) {
        return( FALSE);
    }

    pdr.dev_table_drive_io     = nandRtfsIoFirm;
    pdr.dev_table_perform_device_ioctl = nandRtfsCtrl;
    pdr.register_file_address  = (dword) 0;    /* Not used */
    pdr.interrupt_number       = 0;            /* Not used */
    pdr.drive_flags            = (DRIVE_FLAGS_VALID | DRIVE_FLAGS_PARTITIONED);//DRIVE_FLAGS_FAILSAFE;
    pdr.partition_number       = partition_no; /* Not used */
    pdr.pcmcia_slot_number     = 0;            /* Not used */
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;

    switch( partition_no) {
      case 0:
        result = rtfs_attach( driveno, &pdr, "SD1p0"); //構造体がFSライブラリ側にコピーされる
        break;
      case 1:
        result = rtfs_attach( driveno, &pdr, "SD1p1"); //構造体がFSライブラリ側にコピーされる
        break;
      case 2:
        result = rtfs_attach( driveno, &pdr, "SD1p2"); //構造体がFSライブラリ側にコピーされる
        break;
      case 3:
        result = rtfs_attach( driveno, &pdr, "SD1p3"); //構造体がFSライブラリ側にコピーされる
        break;
      default:
        result = FALSE;
        break;
    }

    return( result);
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRtfsAttachFirm

  Description:  sdmcドライバをドライブに割り当てる

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

    result = rtfs_attach( driveno, &pdr, "SD0");    //構造体がFSライブラリ側にコピーされる

    return( result);
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_InitFIRM

  Description:  init file system

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_InitFIRM( void )
{
    /* RTFSライブラリを初期化 */
    if(!FATFSi_rtfs_init())
    {
        return FALSE;
    }

    /* SDドライバ初期化 */
    if (FATFSi_sdmcInit(SDMC_NOUSE_DMA, NULL, NULL) != SDMC_NORMAL)
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountDriveFirm

  Description:  mount nand partition

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountDriveFirm( int driveno, FATFSMediaType media, int partition_no )
{
    if (media == FATFS_MEDIA_TYPE_NAND)
    {
        // CAUTION!: 同じ関数を2回呼び出す理由について要確認。
        if ( !nandRtfsAttachFirm(driveno, partition_no) || nandRtfsAttachFirm(driveno, partition_no))
        {
            return FALSE;
        }
    }
    else
    {
        if (partition_no)   // support only 0
        {
            return FALSE;
        }
        // CAUTION!: 同じ関数を2回呼び出す理由について要確認。
        if ( !sdmcRtfsAttachFirm(driveno) || sdmcRtfsAttachFirm(driveno))
        {
            return FALSE;
        }
    }
    return TRUE;
}
