/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     lgy_VCW.cpp

  Copyright (C)2009-2011 Nintendo Co., Ltd.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Rev$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "lgy_VCW.h"
#include "lgy_SaveDataChecker.h"

// title.c より定義を拝借
#define WRAM_SIZE_FOR_FS    MI_WRAM_SIZE_96KB
#ifdef USE_HYENA_COMPONENT
#define WRAM_SLOT_FOR_FS    5
#else
#define WRAM_SLOT_FOR_FS    0
#endif

#define VCW_BACKUP_READ_SIZE (7808)
//NN_COMPILER_ASSERT(VCW_BACKUP_READ_SIZE >= ((sizeof(VCW_SavegameHeader) + sizeof(VCW_BodyForCheck)) * 2));
//NN_COMPILER_ASSERT(VCW_BACKUP_READ_SIZE % 32 == 0);

/* VCW メソッド */
static u16 VCW_HeaderChecksumToChecksum( VCW* vcw);
static void VCW_Initialize( VCW* vcw, void* buffer, u32 ggid);
static BOOL VCW_IsFactoryOrBroken( VCW* vcw);
static BOOL VCW_CheckRange( VCW* vcw);
static u16  VCW_Modify( VCW* vcw);
static BOOL VCW_IsModified( VCW* vcw);
static void VCW_UpdateChecksum( VCW* vcw);
static u16  VCW_GetChecksum( VCW* vcw);
static void VCW_Finalize( VCW* vcw);

//using namespace nn::drivers::cardnor::CTR::ARM946ES;


// チェックAPI（VCW*タイトル起動前にこれを呼べばOK）
BOOL checkVCW( TitleProperty* tp, u32 ggid)
{
#pragma unused( tp)
    u32                 i;
    u32                 *data;
    u32                 *verify_data;
    u8                  *buf;
    u16                 B_checksum[2];
    VCW                 vcw[2];
    
    OS_TPrintf("check VCW backup start. ggid = 0x%x\n", ggid);
    OS_TPrintf( "header size:0x%x, body size:0x%x\n", sizeof(VCW_SavegameHeader), sizeof(VCW_BodyForCheck));

    // EEPROM から読み出し
//    base = MI_AllocWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9);
    data = (u32*)(MI_AllocWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9));
    verify_data = (u32*)((u32)data + VCW_BACKUP_READ_SIZE);
    OS_TPrintf("Buffer:0x%x\n", data);
    OS_TPrintf("verifyBuffer:0x%x\n", verify_data);
    
    InitializeBackup();
    if( !readEEPROM( 0, (u32*)data, VCW_BACKUP_READ_SIZE))
    {
        OS_TPrintf("launch NG!\n");
        FinalizeBackup();
        MI_FreeWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9);
        return FALSE;
    }
    buf = (u8*)data;
    VCW_Initialize( &vcw[0], buf, ggid);
    buf = (u8*)data + (sizeof(VCW_SavegameHeader) + sizeof(VCW_BodyForCheck));
    VCW_Initialize( &vcw[1], buf, ggid);

    
    for( i=0; i<2; i++)
    {
        
        // 工場出荷状態またはデータ破壊状態かどうか確認
        if( !VCW_IsFactoryOrBroken( &vcw[i]))
        {
            if( VCW_CheckRange( &vcw[i]))
            {
                OS_TPrintf("VCW backup is correct.\n");
                continue;
            }
        }

        // ランダムな位置に書き込み
        B_checksum[i] = VCW_Modify( &vcw[i]);
    }

    
    // バッファに書き込みしたならデバイスへの書き込みテストが必要
    if( VCW_IsModified( &vcw[0]) || VCW_IsModified( &vcw[1]))
    {
        OS_TPrintf( "backup write testing..\n");
        if( !writeAndVerifyEEPROM( 0, data, verify_data, VCW_BACKUP_READ_SIZE))
        {
            OS_TPrintf("launch NG!\n");
            VCW_Finalize( &vcw[0]);
            VCW_Finalize( &vcw[1]);
            FinalizeBackup();
            MI_FreeWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9);
            return FALSE;
        }
    }

    OS_TPrintf("launch ok.\n");
    VCW_Finalize( &vcw[0]);
    VCW_Finalize( &vcw[1]);
    FinalizeBackup();
    MI_FreeWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9);
    return TRUE;
}





/* VCWメソッド*/
static u16 VCW_HeaderChecksumToChecksum( VCW* vcw)
{
    return (u16)(0xFFFF - (vcw->header->uniqueIdentifier) - (vcw->header->checkSum));
}

static void VCW_Initialize( VCW* vcw, void* buffer, u32 ggid)
{
    vcw->buf = (u8*)buffer;
    if( (u32)(vcw->buf) % 4)
    {
        OS_TPrintf( "Buffer alignment error!\n");
        while( 1) {};
    }
    vcw->ggid = ggid;
    vcw->header = (VCW_SavegameHeader*)(vcw->buf);
    vcw->body = (VCW_BodyForCheck*)(vcw->buf + sizeof(VCW_SavegameHeader));
    vcw->B_checksum = VCW_HeaderChecksumToChecksum( vcw);
}

/* 工場出荷状態かデータ破壊状態なら TRUE を返す */
static BOOL VCW_IsFactoryOrBroken( VCW* vcw)
{
    u16 checksum;

    /**/
    VCW_UpdateChecksum( vcw);
    
    /* 工場出荷状態のチェック */
    if( vcw->ggid != vcw->header->uniqueIdentifier) // VCWV:0x00800354, VCWF:0x00800355, VCWE:0x00400810
    {
        OS_TPrintf("%s, factory.\n", __FUNCTION__);
        return TRUE;
    }
    OS_TPrintf("%s, not factory.\n", __FUNCTION__);

    /* 破壊のチェック */
    checksum = VCW_GetChecksum( vcw);
    checksum += (u16)(vcw->ggid);
    checksum += vcw->header->checkSum;
    if( checksum != 0xFFFF)
    {
        OS_TPrintf("%s, broken.\n", __FUNCTION__);
        return TRUE; // broken
    }
    
    OS_TPrintf("%s, not broken.\n", __FUNCTION__);
    return FALSE;
}

// データが正常範囲内に収まっていれば TRUE を返す
static BOOL VCW_CheckRange( VCW* vcw)
{
    if( vcw->body->m_name[UBI_PLAYERINFO_NAME_LENGTH] != '\0')
    {
        OS_TPrintf("%s, playerName is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    OS_TPrintf("%s, good range.\n", __FUNCTION__);
    return TRUE;
}

/* ランダムに一箇所書き換え、書き換え後のchecksumを返す */
static u16  VCW_Modify( VCW* vcw)
{
    u16          S_checksum_tmp;
    u32          i;
    u8*          body_u8 = (u8*)(vcw->body);
    u32  calculatedSha1[ MATH_SHA1_DIGEST_SIZE /sizeof(u32)];
    MATHRandContext16   rc16;
    u32                 rseed;

    S_checksum_tmp = VCW_GetChecksum( vcw);
    
    // checksumがBになるようにランダムな1箇所へ書き込み
    {
        MATH_CalcSHA1( calculatedSha1, (const void*)vcw->body, sizeof(VCW_BodyForCheck));
        
        rseed = (u32)(OS_GetTick()) + *(u32*)(calculatedSha1);
        MATH_InitRand16( &rc16, rseed);
        for( i=0; i<sizeof(VCW_BodyForCheck); i++)
        {
            body_u8[i] = (u8)(MATH_Rand16( &rc16, 0xFF));
        }
    }

    VCW_UpdateChecksum( vcw);
    vcw->B_checksum = VCW_GetChecksum( vcw);
    // 書き込んだ後の checksum が、S_checksum でも マジコンヘッダのchecksum にもならないようにする
    while( (vcw->B_checksum == S_checksum_tmp) ||
	   ( vcw->B_checksum == VCW_HeaderChecksumToChecksum( vcw)))
    {
        vcw->B_checksum -= body_u8[sizeof(VCW_BodyForCheck) - 1];
        body_u8[sizeof(VCW_BodyForCheck) - 1]++;
        vcw->B_checksum += body_u8[sizeof(VCW_BodyForCheck) - 1];
    }
    OS_TPrintf( "S_checksum:0x%x, A_checksum:0x%x, B_checksum:0x%x\n",
                S_checksum_tmp,
                VCW_HeaderChecksumToChecksum( vcw),
                vcw->B_checksum);

    return vcw->B_checksum;
}

/* Modifyされたかどうかを返す */
static BOOL VCW_IsModified( VCW* vcw)
{
    return ( vcw->B_checksum != VCW_HeaderChecksumToChecksum( vcw));
}

/* Checksum をアップデートする */
static void VCW_UpdateChecksum( VCW* vcw)
{
    u32 i;
    u8* body_u8 = (u8*)vcw->body;

    vcw->S_checksum = 0;
    for( i=0; i<sizeof(VCW_BodyForCheck); ++i)
    {
        vcw->S_checksum += body_u8[i];
    }
}

/* Checksum を取得する */
static u16 VCW_GetChecksum( VCW* vcw)
{
    return vcw->S_checksum;
}

/*  */
static void VCW_Finalize( VCW* vcw)
{
#pragma unused( vcw)
}

