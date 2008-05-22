/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     pxi.c

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

#include <twl.h>
#include <twl/mcu.h>
#include <sysmenu.h>
#include <sysmenu/mcu.h>
#include "internal_api.h"
#ifdef SDK_ARM7
#include <pm_pmic.h>
#endif // SDK_ARM7

#ifdef DHT_TEST
#include <sysmenu/dht/dht.h>
#ifdef SDK_ARM9
#define DS_HASH_TABLE_SIZE  (256*1024)
static u8 dht_buffer[DS_HASH_TABLE_SIZE] ATTRIBUTE_ALIGN(256);
static DHTFile *const dht = (DHTFile*)dht_buffer;
#else
extern DHTFile* dht;
#endif
#endif

// define data-----------------------------------------------------------------
#define SYSM_PXI_COMM_STAT_REQ      0
#define SYSM_PXI_COMM_STAT_ACK      1

// PXIコマンド
typedef union SYSMPXIPacket {
    struct {
        u16 data;
        u8  cmd;  // PXI_FIFOMESSAGE_BITSZ_DATA = 26
        u8  stat : 2;
    };
    u32 raw;
}SYSMPXIPacket;


// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static volatile BOOL s_sending[SYSM_PXI_COMM_NUM];
// const data------------------------------------------------------------------

// PXI初期化
#ifdef SDK_ARM9
void SYSM_InitPXI( void )
#else // SDK_ARM7
void SYSM_InitPXI( u32 mcu_prio )
#endif // SDK_ARM7
{
    static BOOL isInitialized;
    int i;

    if (isInitialized)
    {
        return;
    }
    isInitialized = TRUE;

    // マイコンPXI初期化とマイコンバージョン取得
#ifdef SDK_ARM9
    SYSM_InitMcuPxi();
#else // SDK_ARM7
    SYSM_InitMcuPxi( mcu_prio );
#endif // SDK_ARM7

    for (i=0; i<SYSM_PXI_COMM_NUM; i++)
    {
        s_sending[i] = FALSE;
    }

    //---- setting PXI
    PXI_Init();
#ifdef SDK_ARM9
    while ( ! PXI_IsCallbackReady( SYSMENU_PXI_FIFO_TAG, PXI_PROC_ARM7 ) )
    {
    }
#endif // SDK_ARM9
    PXI_SetFifoRecvCallback( SYSMENU_PXI_FIFO_TAG, SYSMi_PXIFifoRecvCallback );
    SYSM_InitDecryptAESPXICallback();
}

#ifdef DHT_TEST
#ifdef SDK_ARM9
static BOOL GetDatabaseFilepath(char *path)
{
    u8 title[4] = { 'H','N','H','A' };

#if( USE_LCFG_STRING == 0 )
    char *title0 = "HNGA";
#endif
    u32 titleID_hi;
    u32 titleID_lo;
    u64 titleID = 0;


#if( USE_LCFG_STRING == 0 )
    {
        int i;
        if( title[0] == 0 ) {
            for( i = 0 ; i < 4 ; i++ ) {
                title[i] = (u8)*title0++;
            }
        }
    }
#endif


    titleID_hi = (( 3 /* Nintendo */ << 16) | 8 /* CHANNEL_DATA_ONLY */ | 4 /* CHANNEL_CARD */ | 2 /* isLaunch */ | 1 /* isSystem */);

    titleID_lo =  ((u32)( title[0] ) & 0xff) << 24;
    titleID_lo |= ((u32)( title[1] )& 0xff) << 16;
    titleID_lo |= ((u32)( title[2] )& 0xff) << 8;
    titleID_lo |= (u32)( title[3] ) & 0xff;

    titleID = ((u64)(titleID_hi) << 32)  | (u64)titleID_lo;

    // OS_TPrintf( "[DHT]  titleID = 0x%08x%08x\n", titleID_hi, titleID_lo);

    if( NAM_OK == NAM_GetTitleBootContentPathFast(path, titleID) ) {
        OS_TPrintf( "[DHT]  File = %s\n", path);
    }
    else {
        OS_TPrintf( "[DHT]  Error: NAM_GetTitleBootContentPathFast titleID = 0x%08x0x%08x\n",titleID_hi, titleID_lo);
        return FALSE;
    }

    return TRUE;

}

void SYSMi_PrepareDatabase(void)
{
    char path[256];
    if ( GetDatabaseFilepath( path ) )
    {
        FSFile file;
        if ( FS_OpenFileEx(&file, path, FS_FILEMODE_R) )
        {
#if 0   // 1 if using attach_dummyromheader
            if ( FS_SeekFile(&file, sizeof(ROM_Header), FS_SEEK_SET) )
#endif
            {
                DHT_PrepareDatabase(dht, &file);
                DC_FlushRange(dht, DHT_GetDatabaseLength(dht));
            }
            FS_CloseFile(&file);
        }
    }
    else
    {
        MI_CpuClear8(dht, sizeof(DHTHeader));
    }
    OS_TPrintf("[ARM9] dht address: %08X\n", dht);
    SYSMi_SendPXICommand( SYSM_PXI_COMM_DS_HASH_TABLE, (u16)(((u32)dht - 0x2000000) >> 8) );
}
#endif
#endif

// PXIコマンド送信
BOOL SYSMi_TrySendPXICommand( SYSMPXICommand cmd, u16 data )
{
    SYSMPXIPacket packet;

    OSIntrMode saved = OS_DisableInterrupts();
    if( s_sending[cmd] )
    {
        OS_RestoreInterrupts( saved );
        return FALSE;
    }
    s_sending[cmd] = TRUE;
    OS_RestoreInterrupts( saved );

    packet.stat = SYSM_PXI_COMM_STAT_REQ;
    packet.cmd  = cmd;
    packet.data = data;

    while( PXI_SendWordByFifo( SYSMENU_PXI_FIFO_TAG, packet.raw, FALSE) != PXI_FIFO_SUCCESS )
    {
        SVC_WaitByLoop(1);
    }

    return TRUE;
}

BOOL SYSMi_SendPXICommand( SYSMPXICommand cmd, u16 data )
{
    while( ! SYSMi_TrySendPXICommand( cmd, data ) )
    {
        OS_WaitAnyIrq();
    }
    while( s_sending[cmd] )
    {
        OS_WaitAnyIrq();
    }

    return TRUE;
}

void SYSMi_PXIFifoRecvCallback( PXIFifoTag tag, u32 data, BOOL err )
{
#pragma unused( tag, err )
    SYSMPXIPacket packet;
    u8 cmd;
    packet.raw = data;
    cmd = packet.cmd;

    if( packet.stat == SYSM_PXI_COMM_STAT_ACK )
    {
        s_sending[cmd]  = FALSE;
    }
    if( packet.stat == SYSM_PXI_COMM_STAT_REQ )
    {
        packet.stat = SYSM_PXI_COMM_STAT_ACK;
    }

#ifdef SDK_ARM7

    switch( cmd )
    {
        case SYSM_PXI_COMM_BL_BRIGHT:
#ifdef SDK_SUPPORT_PMIC_2
            if ( SYSMi_GetMcuVersion() <= 1 )
            {
                PMi_SetRegister( REG_PMIC_BL_BRT_B_ADDR, (u8)packet.data );
            }
            else
#endif // SDK_SUPPORT_PMIC_2
            {
                MCU_WriteRegister( MCU_REG_BL_ADDR, (u8)packet.data );
            }
            break;
#ifdef DHT_TEST
        case SYSM_PXI_COMM_DS_HASH_TABLE:
            dht = (void*)(0x2000000 + (packet.data << 8));
            OS_TPrintf("[ARM7] dht address: %08X\n", dht);
            break;
#endif
        default:
#ifndef SDK_FINALROM
            OS_Panic( "illegal SYSM pxi command." );
#else
            OS_Panic("");
#endif
            break;
    }

    // PXI応答返信
    PXI_SendWordByFifo( SYSMENU_PXI_FIFO_TAG, packet.raw, FALSE );

#endif // SDK_ARM7
}
