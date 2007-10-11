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

extern u32 NAND_FAT_PARTITION_COUNT;

#define DMA_PIPE         2
#define DMA_RECV         3

/*
    専用DMA関数
*/
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
static inline void MIi_SetControl( u32 ndmaNo, u32 contData )
{
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) = contData;
}
static inline void MIi_Sd1_NDmaRecv(u32 ndmaNo, void *dest, u32 size)
{
    MIi_SetSrc( ndmaNo, (u32)SDIF_FI );
    MIi_SetDest( ndmaNo, (u32)dest );
    MIi_SetInterval( ndmaNo, MI_NDMA_NO_INTERVAL, MI_NDMA_INTERVAL_PS_1 );
    MIi_SetTotalWordCount( ndmaNo, size/4 );
    MIi_SetWordCount( ndmaNo, SECTOR_SIZE/4 );
    MIi_SetControl( ndmaNo, MI_NDMA_BWORD_128 | MI_NDMA_SRC_FIX | MI_NDMA_DEST_INC | MI_NDMA_TIMING_SD_1 | MI_NDMA_ENABLE );
}

static inline void MIi_NDmaPipeSetup(u32 ndmaNo, const void *src, void *dest, u32 size)
{
    MIi_SetSrc( ndmaNo, (u32)src );
    MIi_SetDest( ndmaNo, (u32)dest );
    MIi_SetInterval( ndmaNo, MI_NDMA_NO_INTERVAL, MI_NDMA_INTERVAL_PS_1 );
    MIi_SetTotalWordCount( ndmaNo, size/4 );
    MIi_SetWordCount( ndmaNo, size/4 );
    MIi_SetControl( ndmaNo, MI_NDMA_BWORD_8 | MI_NDMA_SRC_FIX | MI_NDMA_DEST_FIX | MI_NDMA_IMM_MODE_ON );   // AESi_Run is required BWORD_8
}

static inline void MIi_NDmaRestart(u32 ndmaNo)
{
    MI_WaitNDma( ndmaNo );
    MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) |= MI_NDMA_ENABLE;
}

/*
    専用SD関数
*/
extern volatile SDMC_ERR_CODE   SDCARD_ErrStatus;
extern s16  SDCARD_SDHCFlag;          /* SDHCカードフラグ */
extern SDPortContext*   SDNandContext;  /* NAND初期化パラメータ */

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

static void StartToRead(u32 block, u32 count)
{
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

static void StopToRead( void )
{
    if( !SD_CheckFPGAReg( SD_STOP,SD_STOP_SEC_ENABLE ) ){
        SD_StopTransmission();      /* カード転送終了をFPGAに通知（CMD12発行） */
    }
    SD_TransEndFPGA();              /* 転送終了処理(割り込みマスクを禁止に戻す) */
    SD_DisableClock();              /* クロック供給停止 */

    *SDIF_CNT = (*SDIF_CNT & ~SDIF_CNT_USEFIFO) | SDIF_CNT_FCLR; /* FIFO使用フラグOFF */
    CC_EXT_MODE = CC_EXT_MODE_PIO;  /* PIOモード(DMAモードOFF) */
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

  Returns:      0 if success
 *---------------------------------------------------------------------------*/
static u16 ReadNormal(u32 block, void *dest, u16 count)
{
    //OS_TPrintf("ReadNormal(%d, 0x%08X, %d) is calling.\n", block, dest, count);

    MIi_Sd1_NDmaRecv( DMA_PIPE, dest, (u32)(count * SECTOR_SIZE) );
    StartToRead( block, count );
    MI_WaitNDma( DMA_PIPE );
    StopToRead();

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         ReadAES

  Description:  AES read

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      0 if success
 *---------------------------------------------------------------------------*/
#define PIPE_SIZE   64
static u16 ReadAES(u32 block, void *dest, u16 count)
{
    u32 offset = 0; // in bytes

    OS_TPrintf("ReadAES(%d, 0x%08X, %d) is calling.\n", block, dest, count);

    MIi_NDmaPipeSetup( DMA_PIPE, (void*)SDIF_FI, (void*)REG_AES_IFIFO_ADDR, PIPE_SIZE );

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

    StartToRead( block, count );
    if ( SDCARD_ErrStatus != SDMC_NORMAL )
    {
        goto err;
    }

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
        MIi_NDmaRestart( DMA_PIPE );
        offset += PIPE_SIZE;
    }
    MI_WaitNDma( DMA_PIPE );
    StopToRead();
    MI_WaitNDma( DMA_RECV );
    return SDCARD_ErrStatus;

err:
    MI_StopNDma( DMA_RECV );
    MI_StopNDma( DMA_PIPE );
    StopToRead();
    AESi_Reset();
    return SDCARD_ErrStatus;
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

  Arguments:    driveno : ドライブ番号

  Returns:
 *---------------------------------------------------------------------------*/
#define nandRtfsCtrl                    FATFSi_nandRtfsCtrl
extern int nandRtfsCtrl( int driveno, int opcode, void* pargs);
static BOOL nandRtfsAttachFirm( int driveno, int partition_no)
{
    BOOLEAN   result;
    DDRIVE    pdr;

    if( partition_no >= NAND_FAT_PARTITION_COUNT )
    {
        return FALSE;
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

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_InitFIRM( void* nandContext )
{
    /* RTFSライブラリを初期化 */
    if( !FATFSi_rtfs_init() )
    {
        return FALSE;
    }

    /* NAND初期化パラメータの設定 */
    SDNandContext = (SDPortContext*)nandContext;

    /* SDドライバ初期化 */
    if ( FATFSi_sdmcInit( SDMC_NOUSE_DMA ) != SDMC_NORMAL )
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
