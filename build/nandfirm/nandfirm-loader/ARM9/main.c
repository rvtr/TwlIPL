/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - nandrfirm-loader
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
#include <firm/mi.h>

//#define BOOT_SECURE_SRL   // 本番SRLをブートするときにだけ定義する

//#ifndef BOOT_SECURE_SRL
//#define RSA_KEY_ADDR    rsa_key
//static const rsa_key[128] =
//{
//    0x00
//};
//#else
#define RSA_KEY_ADDR    OSi_GetFromBromAddr()->rsa_pubkey[7]
//#endif

u8 acHeap[4*1024] __attribute__ ((aligned (32)));
int acPool[3];

void TwlMain( void )
{
//    OS_TPrintf( "\nNAND Boot time is %d msec.\n", OS_TicksToMilliSecondsBROM32(OS_GetTick()));

#ifndef BOOT_SECURE_SRL
#define FIRM_SHARD      HW_TWL_SECONDARY_ROM_HEADER_BUF
#define FIRM_SHARD_END  HW_TWL_MAIN_MEM_END
#define FIRM_SHARD_SIZE (FIRM_SHARD_END-FIRM_SHARD)
    MIi_CpuClearFast( 0, (void*)FIRM_SHARD, FIRM_SHARD_SIZE );
    DC_FlushRange( (void*)FIRM_SHARD, FIRM_SHARD_SIZE );
//    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );
#endif
    reg_GX_VRAMCNT_C = REG_GX_VRAMCNT_C_FIELD( TRUE, 0, 0x2);

    OS_InitFIRM();

    SVC_InitSignHeap( acPool, acHeap, sizeof(acHeap) );

    // load menu
    if ( MI_LoadHeader( acPool, RSA_KEY_ADDR ) && MI_LoadMenu() )
    {
        MI_BootMenu();
    }

    OS_Terminate();

}

