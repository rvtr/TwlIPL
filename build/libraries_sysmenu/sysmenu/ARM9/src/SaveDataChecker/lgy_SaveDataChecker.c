/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     lgy_SaveDataChecker.cpp

  Copyright (C)2009-2011 Nintendo Co., Ltd.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Rev$
 *---------------------------------------------------------------------------*/

#include <twl.h>

#include "lgy_SaveDataChecker.h"
#include "lgy_VCK.h"
#include "lgy_VCW.h"

// title.c より定義を拝借
#define WRAM_SIZE_FOR_FS    MI_WRAM_SIZE_96KB
#ifdef USE_HYENA_COMPONENT
#define WRAM_SLOT_FOR_FS    5
#else
#define WRAM_SLOT_FOR_FS    0
#endif

enum GameCodeList
{
    TARGET_ROM_VCK = 0,
    TARGET_ROM_VCW,
    TARGET_ROM_MAX
}GameCodeList;

char *game_code_list[TARGET_ROM_MAX] = { "VCK", "VCW" };


BOOL CheckBackupData( TitleProperty *rhs)
{
    u8 list_num;
    char test[5];

    {
        char* n = (char*)&rhs->titleID;
        test[0] = n[3];
        test[1] = n[2];
        test[2] = n[1];
        test[3] = n[0];
        test[4] = '\0';
    }

    for( list_num = 0; list_num < TARGET_ROM_MAX; list_num++)
    {
        if( 0 == STD_CompareNString( test, game_code_list[list_num], 3))
        {
            switch( list_num)
            {
              case TARGET_ROM_VCK:
                return checkVCK( rhs);
              case TARGET_ROM_VCW:
                if( test[3] == 'V')
                {
                    return checkVCW( rhs, 0x00800354);
                }
                else if( test[3] == 'F')
                {
                    return checkVCW( rhs, 0x00800355);
                }
                else if( test[3] == 'E')
                {
                    return checkVCW( rhs, 0x00400810);
                }
              default:
                return TRUE;
            }
        }
    }
    return TRUE;
}


void InitializeBackup( void)
{
    u16 lock_id;

    // HOTSWライブラリのケア（終了処理待ちなど）は必要ない
#if 0
    // デバッグ用に表示するだけ
    if( HOTSW_isEnableHotSW())
    {
//        HOTSW_InvalidHotSW();
        OS_TPrintf( "HOTSW is enable!\n");
    }
    else
    {
        OS_TPrintf( "ok, HOTSW is not enable.\n");
    }
    if( !HOTSW_isFinalized())
    {
        OS_TPrintf( "HOTSW is not Finalized!\n");
    }
    else
    {
        OS_TPrintf( "ok, HOTSW is Finalized.\n");
    }
    if( !HOTSW_isCardLoadCompleted())
    {
        OS_TPrintf( "HOTSW is not card load completed!\n");
    }
    else
    {
        OS_TPrintf( "ok, HOTSW is is card load completed.\n");
    }
#endif
    
    if( !CARD_IsAvailable())
    {
        CARD_Init();
        OS_TPrintf( "CARD is not available, CARD_Init() is done.\n");
    }
    if( !CARD_IsEnabled())
    {
        CARD_Enable( TRUE);
        OS_TPrintf( "CARD is not enable, CARD_Enable() is done.\n");
    }
    
    lock_id = (u16)OS_GetLockID();
    CARD_LockBackup( lock_id);
    if( !CARD_IdentifyBackup( CARD_BACKUP_TYPE_EEPROM_64KBITS))
    {
        OS_TPrintf( "CARD_IdentifyBackup failed.\n");
    }
    CARD_UnlockBackup( lock_id);
    OS_TPrintf( "InitializeBackup done.\n");
}

void FinalizeBackup( void)
{
//    while( 1) {};
}


BOOL readEEPROM( u32 src, void *dst, u32 len)
{
    BOOL result;
    u16 lock_id;

    lock_id = (u16)OS_GetLockID();
    CARD_LockBackup( lock_id);
     /* CARD_ReadEeprom() は、 一時バッファ(CARDi_backup_cache_page_buf)に読み出したデータを
        A9-CPUでdstへコピーしてくれるので、dstのメモリ空間はA7から見えなくても良い */
    result = CARD_ReadEeprom( src, dst, len);
    if( !result)
    {
        OS_TPrintf( "CARD_ReadEeprom failed! err:%d\n", (int)CARD_GetResultCode());
    }
    CARD_UnlockBackup( lock_id);

//    DC_FlushRange( dst, len);
    
    return result;
}

BOOL writeEEPROM( u32 dst, const void *src, u32 len)
{
    BOOL result;
    u16 lock_id;

//    DC_FlushRange( src, len);
    
    lock_id = (u16)OS_GetLockID();
    CARD_LockBackup( lock_id);
     /* CARD_WriteEeprom() は、 src上のデータをA9-CPUで一時バッファ(CARDi_backup_cache_page_buf)に
        コピーした上でデバイスに書き込んでいるので、srcのメモリ空間はA7から見えなくても良い */
    result = CARD_WriteEeprom( dst, src, len);
    if( !result)
    {
        OS_TPrintf( "CARD_WriteEeprom failed! err:%d\n", (int)CARD_GetResultCode());
    }
    CARD_UnlockBackup( lock_id);
    return result;
}

#if 1
BOOL writeAndVerifyEEPROM( u32 address, const void *buffer, void *reload_buffer, u32 len)
{
    BOOL w_result, r_result;
    int  m_result;
    u16  lock_id;
    u32  pos;

    // バッファ同士がオーバーラップしていたらエラー
    pos = (u32)reload_buffer;
    if( ((u32)buffer <= pos) && (((u32)buffer + len - 1) >= pos))
    {
        return FALSE;
    }
    pos = (u32)reload_buffer + len - 1;
    if( ((u32)buffer <= pos) && (((u32)buffer + len - 1) >= pos))
    {
        return FALSE;
    }
    
    lock_id = (u16)OS_GetLockID();
    CARD_LockBackup( lock_id);
     /* CARD_WriteAndVerifyEeprom() は、 src上のデータをA9-CPUで一時バッファ(CARDi_backup_cache_page_buf)に
        コピーした上で処理しているので、srcのメモリ空間はA7から見えなくても良い */
    w_result = CARD_WriteEeprom( address, buffer, len);
    if( !w_result)
    {
        OS_TPrintf( "CARD_WriteEeprom failed! err:%d\n", (int)CARD_GetResultCode());
    }
    r_result = CARD_ReadEeprom( address, reload_buffer, len);
    if( !r_result)
    {
        OS_TPrintf( "CARD_ReadEeprom failed! err:%d\n", (int)CARD_GetResultCode());
    }
    m_result = MI_CpuComp8( buffer, reload_buffer, len);
    CARD_UnlockBackup( lock_id);
    return (w_result && r_result && (m_result == 0));
}
#else
BOOL writeAndVerifyEEPROM( u32 dst, const void *src, u32 len)
{
    BOOL result;
    u16 lock_id;

//    DC_FlushRange( src, len);
    
    lock_id = (u16)OS_GetLockID();
    CARD_LockBackup( lock_id);
     /* CARD_WriteAndVerifyEeprom() は、 src上のデータをA9-CPUで一時バッファ(CARDi_backup_cache_page_buf)に
        コピーした上で処理しているので、srcのメモリ空間はA7から見えなくても良い */
    result = CARD_WriteAndVerifyEeprom( dst, src, len);
    if( !result)
    {
        OS_TPrintf( "CARD_WriteAndVerifyEeprom failed! err:%d\n", (int)CARD_GetResultCode());
    }
    CARD_UnlockBackup( lock_id);
    return result;
}
#endif
