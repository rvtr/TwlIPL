/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - sdmc-launcher
  File:     main.c

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
#include <firm.h>
#include <twl/mcu.h>

/*
    デバッグLEDをFINALROMとは別にOn/Offできます。
*/
#define USE_DEBUG_LED

/*
    PRINT_MEMORY_ADDR を定義すると、そのアドレスからSPrintfを行います(このファイルのみ)
    FINALROM版でもコードが残るので注意してください。
*/
//#define PRINT_MEMORY_ADDR       0x02FFC800

/*
    AES鍵設定API
*/
extern void SYSMi_SetAESKeysForAccessControl( BOOL isNtrMode, ROM_Header *pROMH );

#ifdef USE_DEBUG_LED
static u8 step = 0x80;
#define InitDebugLED()          I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x03, 0x00)
#define SetDebugLED(pattern)    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, (pattern));
#else
#define InitDebugLED()          ((void)0)
#define SetDebugLED(pattern)    ((void)0)
#endif

#ifdef PRINT_MEMORY_ADDR
static char* debugPtr = (char*)PRINT_MEMORY_ADDR;
#undef OS_TPrintf
//#define OS_TPrintf(...) (debugPtr = (char*)((u32)(debugPtr + STD_TSPrintf(debugPtr, __VA_ARGS__) + 0xf) & ~0xf))
#define OS_TPrintf(...) (debugPtr += STD_TSPrintf(debugPtr, __VA_ARGS__))
#endif

#define THREAD_PRIO_AES     12
#define THREAD_PRIO_FATFS   8
#define DMA_FATFS_1         0
#define DMA_FATFS_2         1

/***************************************************************
    PreInit

    FromBootの対応＆メインメモリの初期化
    OS_Init前なので注意 (ARM9によるメインメモリ初期化で消されないように注意)
***************************************************************/
static void PreInit(void)
{
    /*
        FromBrom関連
    */
    if ( !OSi_FromBromToMenu() )
    {
        OS_Terminate();
    }
    /*
        リセットパラメータ(1バイト)を共有領域(1バイト)にコピー
    */
#define HOTSTART_FLAG_ENABLE    0x80
    *(u8 *)HW_NAND_FIRM_HOTSTART_FLAG = (u8)(MCUi_ReadRegister( (u16)(MCU_REG_TEMP_ADDR + OS_MCU_RESET_VALUE_OFS) ) | HOTSTART_FLAG_ENABLE);
}

/***************************************************************
    PostInit

    各種初期化
***************************************************************/
static void PostInit(void)
{
    PM_BackLightOn( TRUE ); // ARM9側画面表示のため

    /*
        AES関連 (NAND暗号化の鍵変更を含む)
    */
    if ( OSi_GetFromFirmAddr()->aes_key[2][0] )
    {
        AESi_PreInitKeys();
    }
    // AESの初期化
    AES_Init(THREAD_PRIO_AES);           // for encrypted NAND
    /*
        バッテリー残量チェック
    */
    MCUi_WriteRegister( MCU_REG_MODE_ADDR, MCU_SYSTEMMODE_TWL );
    SetDebugLED(++step); // 0x84
    if ( (MCUi_ReadRegister( MCU_REG_POWER_INFO_ADDR ) & MCU_REG_POWER_INFO_LEVEL_MASK) == 0 )
    {
#ifndef SDK_FINALROM
        OS_TPanic("Battery is empty.\n");
#else
        PM_Shutdown();
#endif
    }
}

/***************************************************************
    EraseAll

    不正終了しました
    いろいろ消してください
    DSモードにして終わるのがよいか？
***************************************************************/
static void EraseAll(void)
{
    AESi_ResetAesKeyA();
    AESi_ResetAesKeyB();
    AESi_ResetAesKeyC();
    MI_CpuClearFast( OSi_GetFromFirmAddr(), sizeof(OSFromFirmBuf) );
#ifdef SDK_FINALROM
    MI_CpuClearFast( (void*)HW_TWL_ROM_HEADER_BUF, HW_TWL_ROM_HEADER_BUF_SIZE );
    OS_BootFromFIRM();
#endif
}

/*
   MI_LoadStatic関連
*/
static ROM_Header* const rh= (ROM_Header*)HW_TWL_ROM_HEADER_BUF;

#define MODULE_ALIGNMENT    0x20    // 16*2バイト単位で読み込む
//#define MODULE_ALIGNMENT  0x200   // 512バイト単位で読み込む
#define RoundUpModuleSize(value)    (((value) + MODULE_ALIGNMENT - 1) & -MODULE_ALIGNMENT)

/*
    start point
    main static
    sub static
    main ltd
    sub ltd
    end point
*/
#define ROM_HEADER_RAM_OFFSET   0xe00
#define TEMP_OFFSET             0x800000    // 一時待避場所

static BOOL         aesFlag;
static AESCounter   aesCounter;

#define DMA_SEND         2
#define DMA_RECV         3
static void MI_DmaCopyWithAes( void* src, void* dest, u32 size )
{
    AES_Lock();
    AES_Reset();
    AES_Reset();
    AES_WaitKey();
    AES_LoadKey( AES_KEY_SLOT_A );
    AES_WaitKey();
    AES_DmaSend( DMA_SEND, src,  size, NULL, NULL );
    AES_DmaRecv( DMA_RECV, dest, size, NULL, NULL );
    AES_SetCounter( &aesCounter );
    AES_Run( AES_MODE_CTR, 0, size / AES_BLOCK_SIZE, NULL, NULL );
    AES_AddToCounter( &aesCounter, size / AES_BLOCK_SIZE );
    MI_WaitNDma( DMA_RECV );
    AES_Unlock();
}

static void EnableAes( u32 offset )
{
    aesFlag = TRUE;
    MI_CpuCopy8( rh->s.main_static_digest, &aesCounter, AES_BLOCK_SIZE );
    AES_AddToCounter( &aesCounter, (offset - rh->s.aes_target_rom_offset) / AES_BLOCK_SIZE );
}
static void EnableAes2( u32 offset )
{
    aesFlag = TRUE;
    MI_CpuCopy8( rh->s.sub_static_digest, &aesCounter, AES_BLOCK_SIZE );
    AES_AddToCounter( &aesCounter, (offset - rh->s.aes_target2_rom_offset) / AES_BLOCK_SIZE );
}
static void DisableAes( void )
{
    aesFlag = FALSE;
}

static u32 GetTransferSize( u32 offset, u32 size )
{
    if ( rh->s.enable_aes )
    {
        u32 end = offset + RoundUpModuleSize(size);
        u32 aes_offset = rh->s.aes_target_rom_offset;
        u32 aes_end = aes_offset + RoundUpModuleSize(rh->s.aes_target_size);
        u32 aes_offset2 = rh->s.aes_target2_rom_offset;
        u32 aes_end2 = aes_offset2 + RoundUpModuleSize(rh->s.aes_target2_size);

        if ( offset >= aes_offset && offset < aes_end )
        {
            if ( end > aes_end )
            {
                size = aes_end - offset;
            }
            EnableAes( offset );
        }
        else if ( offset >= aes_offset2 && offset < aes_end2 )
        {
            if ( end > aes_end2 )
            {
                size = aes_end2 - offset;
            }
            EnableAes2( offset );
        }
        else
        {
            if ( offset < aes_offset && offset + size > aes_offset )
            {
                size = aes_offset - offset;
            }
            if ( offset < aes_offset2 && offset + size > aes_offset2 )
            {
                size = aes_offset2 - offset;
            }
            DisableAes();
        }
    }
    else
    {
        DisableAes();
    }
    return size;
}
static void MI_LoadModule( u8* src, u8* dest, u32 offset, u32 size )
{
    size = RoundUpModuleSize( size );
    while ( size > 0 )
    {
        u32 unit = GetTransferSize( offset, size );
        if ( aesFlag )
        {
            MI_DmaCopyWithAes( src, dest, unit );
        }
        else
        {
            MI_CpuCopyFast( src, dest, unit );
        }
        dest += unit;
        src += unit;
        offset += unit;
        size -= unit;
    }
}

static void MI_LoadStatic( void )
{
    u8** ram_offset = (u8**)((u32)rh + ROM_HEADER_RAM_OFFSET);
    if ( ram_offset[5] + TEMP_OFFSET > (void*)HW_TWL_MAIN_MEM_SHARED )
    {
        return; // overflow
    }
    /* 一旦移動させる */
    MI_CpuCopyFast( ram_offset[0], ram_offset[0] + TEMP_OFFSET, (u32)RoundUpModuleSize(ram_offset[5] - ram_offset[0]));
    if ( rh->s.main_size > 0 )
    {
        MI_LoadModule( ram_offset[1] + TEMP_OFFSET, rh->s.main_ram_address, rh->s.main_rom_offset, rh->s.main_size );
    }
    if ( rh->s.sub_size > 0 )
    {
        MI_LoadModule( ram_offset[2] + TEMP_OFFSET, rh->s.sub_ram_address, rh->s.sub_rom_offset, rh->s.sub_size );
    }
    if ( rh->s.main_ltd_size > 0 )
    {
        MI_LoadModule( ram_offset[3] + TEMP_OFFSET, rh->s.main_ltd_ram_address, rh->s.main_ltd_rom_offset, rh->s.main_ltd_size );
    }
    if ( rh->s.sub_ltd_size > 0 )
    {
        MI_LoadModule( ram_offset[4] + TEMP_OFFSET, rh->s.sub_ltd_ram_address, rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size );
    }
}

void TwlSpMain( void )
{
    InitDebugLED();
    SetDebugLED(++step);  // 0x81

    PreInit();
    SetDebugLED(++step);  // 0x82

    OS_InitFIRM();
    OS_EnableIrq();
    OS_EnableInterrupts();
    // 2: after OS_InitFIRM
    SetDebugLED(++step); // 0x83

    PostInit();
    // 3: after PostInit
    SetDebugLED(++step); // 0x85

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_HEADER )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_HEADER).\n");
        goto end;
    }
    // 4: after PXI
    SetDebugLED(++step); // 0x87

    AESi_InitKeysFIRM();
    AESi_InitSeed();
    // 5: after AESi_InitSeed
    SetDebugLED(++step); // 0x88

    MI_LoadStatic();
    PXI_NotifyID( FIRM_PXI_ID_NULL );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_STATIC )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_STATIC).\n");
        goto end;
    }
    // 6: after PXI
    SetDebugLED( 0 );

    SYSMi_SetAESKeysForAccessControl(FALSE, rh);
    MI_CpuClearFast( OSi_GetFromFirmAddr(), sizeof(OSFromFirmBuf) );
    FS_SetMountInfoForSrl();
    OS_BootFromFIRM();

end:
    SetDebugLED( (u8)(0xF0 | step));

    MCUi_WriteRegister( MCU_REG_MODE_ADDR, MCU_SYSTEMMODE_NITRO );
    EraseAll();

    // failed
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    OS_Terminate();
}

