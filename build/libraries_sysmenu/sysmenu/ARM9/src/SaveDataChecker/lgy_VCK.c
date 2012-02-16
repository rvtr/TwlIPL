/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     lgy_VCK.cpp

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
#include "lgy_VCK.h"
#include "lgy_SaveDataChecker.h"

// title.c より定義を拝借
#define WRAM_SIZE_FOR_FS    MI_WRAM_SIZE_96KB
#ifdef USE_HYENA_COMPONENT
#define WRAM_SLOT_FOR_FS    5
#else
#define WRAM_SLOT_FOR_FS    0
#endif

#define VCK_BACKUP_READ_SIZE (896)
//NN_COMPILER_ASSERT(VCK_BACKUP_READ_SIZE >= sizeof(VCK_SlotHeader) + sizeof(VCK_ProfileSlotBody));
//NN_COMPILER_ASSERT(VCK_BACKUP_READ_SIZE % 32 == 0);

// ヘッダのチェックサムをチェックサムに変換する
static u16 VCK_HeaderChecksumToChecksum( VCK_SlotHeader* header);
// checksum を算出する
static u16 getChecksum( void* data);
// データが工場出荷状態か破壊されていれば TRUE を返す
static BOOL isFactoryOrBrokenForVCK( TitleProperty* tp, void* data, u16* ret_checksum);
// データが正常範囲内に収まっていれば TRUE を返す
static BOOL checkEepromRangeForVCK( void* data);

//using namespace nn::drivers::cardnor::CTR::ARM946ES;


typedef struct TitleID_num
{
    u32 Lo;
    u32 Hi;
}
TitleID_num;



// チェックAPI（VCK*タイトル起動前にこれを呼べばOK）
BOOL checkVCK( TitleProperty* tp)
{
    u32                 i;
    u32                 *data;
    u32                 *verify_data;
    VCK_SlotHeader      *header;
    VCK_ProfileSlotBody *body;
    u8                  *body_u8;
    u16                 S_checksum = 0;
    u16                 B_checksum;
    u32  calculatedSha1[ MATH_SHA1_DIGEST_SIZE / sizeof(u32)];
    MATHRandContext16   rc16;
    u32                 rseed;
    
    OS_TPrintf("check VCK backup start.\n");
    OS_TPrintf( "header size:0x%x, body size:0x%x\n", sizeof(VCK_SlotHeader), sizeof(VCK_ProfileSlotBody));
    
    // EEPROM から読み出し
    data = (u32*)(MI_AllocWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9));
    verify_data = (u32*)((u32)data + VCK_BACKUP_READ_SIZE);
    OS_TPrintf("Buffer:0x%x\n", data);
    OS_TPrintf("verifyBuffer:0x%x\n", verify_data);
    
    InitializeBackup();
    readEEPROM( 0, (u32*)data, VCK_BACKUP_READ_SIZE);//sizeof(VCK_SlotHeader) + sizeof(VCK_ProfileSlotBody)が32Bytesの倍数でないため
    header = (VCK_SlotHeader*)data;
    body = (VCK_ProfileSlotBody*)((u8*)data + sizeof(VCK_SlotHeader));
    body_u8 = (u8*)body;

    // 工場出荷状態またはデータ破壊状態かどうか確認
    if( !isFactoryOrBrokenForVCK( tp, data, &S_checksum))
    {
        if( checkEepromRangeForVCK( data)) // データ範囲を確認
        {
            OS_TPrintf("VCK backup is correct.\n");
            FinalizeBackup();
            MI_FreeWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9);
            return TRUE;
        }
    }
    else
    {
        if( !S_checksum)
        {
            S_checksum = getChecksum( body);
        }
    }
    
    // checksumがBになるようにランダムな1箇所へ書き込み
    {
        MATH_CalcSHA1( calculatedSha1, (const void*)data, sizeof(VCK_ProfileSlotBody)); 
    }
    rseed = (u32)(OS_GetTick()) + *(u32*)(calculatedSha1);
    MATH_InitRand16( &rc16, rseed);
    for( i=0; i<sizeof(VCK_ProfileSlotBody); i++)
    {
        body_u8[i] = (u8)(MATH_Rand16( &rc16, 0xFF));
    }
    B_checksum = getChecksum( body);
    // 書き込んだ後の checksum が、S_checksum でも マジコンヘッダのchecksum にもならないようにする
    while( (B_checksum == S_checksum) ||
	   ( B_checksum == VCK_HeaderChecksumToChecksum( header)))
    {
        B_checksum -= body_u8[sizeof(VCK_ProfileSlotBody) - 1];
        body_u8[sizeof(VCK_ProfileSlotBody) - 1]++;
        B_checksum += body_u8[sizeof(VCK_ProfileSlotBody) - 1];
    }

    // Backup デバイスへ反映
    if( writeAndVerifyEEPROM( 0, data, verify_data, VCK_BACKUP_READ_SIZE))
    {
        OS_TPrintf("launch ok.\n");
        OS_TPrintf("S_checksum:0x%x, A_ckecksum:0x%x, B_checksum:0x%x\n", S_checksum, VCK_HeaderChecksumToChecksum(header), B_checksum);
        FinalizeBackup();
        MI_FreeWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9);
        return TRUE;
    }

    OS_TPrintf("launch NG.\n");
    OS_TPrintf("S_checksum:0x%x, A_ckecksum:0x%x, B_checksum:0x%x\n", S_checksum, VCK_HeaderChecksumToChecksum(header), B_checksum);
    FinalizeBackup();
    MI_FreeWramSlot( MI_WRAM_C, WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9);
    return FALSE;
}

// ヘッダのチェックサムをチェックサムに変換する
static u16 VCK_HeaderChecksumToChecksum( VCK_SlotHeader* header)
{
    return (u16)(0xFFFF - (header->m_uniqueIdentifier) - (header->m_slotCheckSum));
}

// checksum を算出する
static u16 getChecksum( void* data)
{
    u8             *data_u8;
    u16            checksum = 0;
    u16            i;

    data_u8 = (u8*)data;

    for( i=0; i<sizeof(VCK_ProfileSlotBody); ++i)
    {
        checksum += data_u8[i];
    }
    
    return checksum;
}

// データが工場出荷状態か破壊されていれば TRUE を返す
static BOOL isFactoryOrBrokenForVCK( TitleProperty* tp, void* data, u16* ret_checksum)
{
    VCK_SlotHeader *header;
    u8*            body;
    u16            checksum, i;
    u8*            pTitle_id;
    u8*            pIdentifier;

    header = (VCK_SlotHeader*)data;
    body = (u8*)data + sizeof(VCK_SlotHeader);
    pTitle_id = (u8*)&(tp->titleID); // TWL:0x56434B45, CTR:0x454B4356
    pIdentifier = (u8*)&(header->m_uniqueIdentifier); // 0x56434B45
    
    /* 工場出荷状態のチェック */
#if 1
    for( i=0; i<4; i++)
    {
        if( pTitle_id[i] != pIdentifier[i])
        {
            OS_TPrintf("%s, factory.\n", __FUNCTION__);
            return TRUE;
        }
    }
    OS_TPrintf("%s, not factory.\n", __FUNCTION__);
#endif

    /* 破壊のチェック */
    checksum = 0;
    for( i=0; i<sizeof(VCK_ProfileSlotBody); i++)
    {
        checksum += body[i];
    }
    OS_TPrintf("%s : S_checksum:0x%x\n", __FUNCTION__, checksum);
    *ret_checksum = checksum; // checksumを返す
    checksum += (u16)(header->m_uniqueIdentifier);
    checksum += header->m_slotCheckSum;
    if( checksum != 0xFFFF)
    {
        OS_TPrintf("%s, broken.\n", __FUNCTION__);
        return TRUE; // broken
    }
    
    OS_TPrintf("%s, not broken.\n", __FUNCTION__);
    return FALSE;
}

// データが正常範囲内に収まっていれば TRUE を返す
static BOOL checkEepromRangeForVCK( void* data)
{
    VCK_ProfileSlotBody *buf;
    buf = (VCK_ProfileSlotBody*)((u8*)data + sizeof(VCK_SlotHeader));
    
    if( buf->m_playerName[10] != '\0')
    {
        OS_TPrintf("%s, playerName is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    else if( (buf->m_averageCookingTime < -1) || (buf->m_averageCookingTime > 2))
    {
        OS_TPrintf("%s, cookingTime is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    else if( (buf->m_culinaryLevel < -1) || (buf->m_culinaryLevel > 2))
    {
        OS_TPrintf("%s, culinaryLevel is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    else if( (buf->m_costImportance < -1) || (buf->m_costImportance > 2))
    {
        OS_TPrintf("%s, costImportance is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    else if( (buf->m_numberCalories < -1) || (buf->m_numberCalories > 2))
    {
        OS_TPrintf("%s, numberCalories is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    else if( buf->m_ROTD_index > 9)
    {
        OS_TPrintf("%s, ROTD_index is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    else if( buf->m_uBirthDate_Day >0x1F)
    {
        OS_TPrintf("%s, birthdate_day is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    else if( buf->m_uBirthDate_Month > 0x0C)
    {
        OS_TPrintf("%s, birthdate_month is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    else if( buf->m_numServings > 8)
    {
        OS_TPrintf("%s, numServings is out of range.\n", __FUNCTION__);
        return FALSE;
    }
    OS_TPrintf("%s, good range.\n", __FUNCTION__);
    return TRUE;
}
