/*---------------------------------------------------------------------------*
  Project:  TwlIPL - DHT
  File:     dht.c

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
#include <sysmenu/dht/dht.h>

#define HASH_PATH   "/sign/DSHashTable.bin"

/*
    ��`����Ə������Ԃ�\������
*/
#define PRINT_PROFILE

#ifdef PRINT_PROFILE
static int      count;
static OSTick   profile[0x10];
#define PROFILE_INIT()  (count = 0)
#define PROFILE_COUNT() (profile[count++] = OS_GetTick())
#else
#define PROFILE_INIT()  ((void)0)
#define PROFILE_COUNT() ((void)0)
#define
#endif

static const u8 g_pubkey_DER[ 0xa2 ] = {
    0x30, 0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
    0x05, 0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xc7, 0xf4, 0x1d,
    0x27, 0x3f, 0xe8, 0xae, 0x7f, 0x7c, 0xbc, 0x9a, 0xae, 0x09, 0x8d, 0x19, 0x26, 0x2e, 0x90, 0x04,
    0x03, 0x13, 0x93, 0xbc, 0xb2, 0xe0, 0x8b, 0x1f, 0x85, 0x48, 0xf5, 0xf6, 0x94, 0x69, 0x3e, 0x05,
    0x1b, 0x97, 0x85, 0x44, 0x6d, 0xa3, 0xcd, 0xa8, 0x01, 0xfe, 0xdc, 0x77, 0x5d, 0xd1, 0xb1, 0x36,
    0x21, 0xfc, 0x80, 0xe8, 0xa6, 0x0e, 0xde, 0x59, 0x76, 0xca, 0x96, 0xcc, 0x87, 0x4c, 0xc3, 0x90,
    0xc6, 0x3b, 0xc8, 0x17, 0x9d, 0x2d, 0xac, 0x45, 0xbc, 0xa7, 0x15, 0xb2, 0xe3, 0xd7, 0x76, 0xfa,
    0x09, 0x8c, 0x55, 0x09, 0x22, 0x95, 0x4b, 0xe7, 0xde, 0xc0, 0x82, 0xf2, 0x02, 0x1a, 0x8a, 0x42,
    0x38, 0x7f, 0xbb, 0x31, 0xd6, 0xa8, 0x36, 0xdc, 0x8d, 0x2c, 0x42, 0x56, 0x51, 0xc1, 0xa3, 0x30,
    0x21, 0x30, 0xef, 0x06, 0x72, 0x0c, 0xa6, 0x55, 0xb7, 0x4f, 0x30, 0x35, 0x1b, 0x02, 0x03, 0x01,
    0x00, 0x01
};

static const u8 hmac_key[] = DHT_HMAC_KEY;


/*
    ���Ɛ�bsearch
*/

static void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
    u32 left = 0;
    u32 right = nmemb;

    if ( compar((u8*)base + size * (right - 1), key) < 0 )
    {
        return NULL;
    }
    if ( compar((u8*)base + size * left, key) > 0 )
    {
        return NULL;
    }

    while (left <= right)
    {
        u32 mid = (left + right) >> 1;
        const void* data = (u8*)base + size * mid;
        int result = compar(data, key);
//OS_TPrintf("left = %d, mid = %d, right = %d\n", left, mid, right);
        if ( !result )
        {
            return (void*)data;
        }
        if ( result < 0 )
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return NULL;
}
static int CompareGameCodeAndVersion(const void* a, const void* b)
{
    return MI_CpuComp8(a, b, 5);
}

/*
�f�[�^�x�[�X��ǂݍ��� (�O����)
*/
BOOL DHT_PrepareDatabase(DHTFile* pDHT)
{
    FSFile file;
    u32 length;
    s32 result;
    SVCSignHeapContext pool;
    u8 heap[4*1024];
    u8 md1[20];
    u8 md2[20];
    PROFILE_INIT();

    // �t�@�C���I�[�v��
    PROFILE_COUNT();
    if (!FS_OpenFileEx(&file, HASH_PATH, FS_FILEMODE_R))
    {
        OS_TPrintf("Cannot open " HASH_PATH ".\n");
        return FALSE;
    }
    // �w�b�_�ǂݍ���
    PROFILE_COUNT();
    result = FS_ReadFile(&file, &pDHT->header, sizeof(DHTHeader));
    if ( result != sizeof(DHTHeader) )
    {
        OS_TPrintf("Cannot read the header of " HASH_PATH " (result=%d).\n", result);
        return FALSE;
    }
    if ( pDHT->header.magic_code != DHT_MAGIC_CODE )
    {
        OS_TPrintf("Invalid " HASH_PATH " magic code (magic=0x%08X).\n", pDHT->header.magic_code);
        return FALSE;
    }
    // �T�C�Y�`�F�b�N
    PROFILE_COUNT();
    length = FS_GetFileLength(&file);
    if ( length != sizeof(DHTHeader) + pDHT->header.nums * sizeof(DHTDatabase) )
    {
        OS_TPrintf("Invalid " HASH_PATH " size (%d != %d).\n", length, sizeof(DHTHeader) + pDHT->header.nums * sizeof(DHTDatabase));
        return FALSE;
    }
    // database�T�C�Y�̕ۑ�
    length = pDHT->header.nums * sizeof(DHTDatabase);
    // �f�[�^�x�[�X�ǂݍ���
    PROFILE_COUNT();
    result = FS_ReadFile(&file, pDHT->database, (s32)length);
    if ( result != length )
    {
        OS_TPrintf("Cannot read the database of " HASH_PATH " (result=%d).\n", result);
        return FALSE;
    }
    FS_CloseFile(&file);
    // �t�@�C���������o��
    PROFILE_COUNT();
    SVC_InitSignHeap(&pool, heap, sizeof(heap));
    SVC_DecryptSign(&pool, md1, pDHT->header.sign, &g_pubkey_DER[29]);
    // �n�b�V���v�Z
    PROFILE_COUNT();
    SVC_CalcSHA1(md2, DHT_GET_SIGN_TARGET_ADDR(&pDHT->header), DHT_GET_SIGN_TARGET_SIZE(&pDHT->header));
    // ����
    PROFILE_COUNT();
    result = SVC_CompareSHA1(md1, md2);
    if ( !result )
    {
        OS_TPrintfEx("SIGN = % 20B\n", md1);
        OS_TPrintfEx("HASH = % 20B\n", md2);
        OS_TPrintf("Signature is not valid.\n");
        return FALSE;
    }
    // ���ʕ�
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to prepare the database.\n");
    OS_TPrintf("%10d msec for file open.\n",        (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for reading header.\n",   (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for size check.\n",       (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("%10d msec for reading database.\n", (int)OS_TicksToMilliSeconds(profile[4]-profile[3]));
    OS_TPrintf("%10d msec for decrypt sign.\n",     (int)OS_TicksToMilliSeconds(profile[5]-profile[4]));
    OS_TPrintf("%10d msec for hashing database.\n", (int)OS_TicksToMilliSeconds(profile[6]-profile[5]));
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[7]-profile[6]));
    OS_TPrintf("\nTotal: %10d msec.\n",             (int)OS_TicksToMilliSeconds(profile[7]-profile[0]));
#endif
    return TRUE;
}
/*
ROM�w�b�_�ɑΉ�����f�[�^�x�[�X����ɓ����
*/
const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader)
{
    u8 data[5];
    DHTDatabase* db;
    PROFILE_INIT();

    // ����
    PROFILE_COUNT();
    MI_CpuCopy8( pROMHeader->game_code, data, 4 );
    data[4] = pROMHeader->rom_version;
    db = (DHTDatabase*)bsearch(data, pDHT->database, pDHT->header.nums, sizeof(DHTDatabase), CompareGameCodeAndVersion);
    if ( !db )
    {
        OS_TPrintf("Cannot find the database.\n");
    }
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("%10d msec for searching database.\n", (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
    return db;
}
/*
�n�b�V���v�Z (1)
�ǂݍ��ݍς݃f�[�^���`�F�b�N����
*/
BOOL DHT_CheckHashPhase1(const DHTDatabase *db, const ROM_Header_Short* pROMHeader, const void* pARM9, const void* pARM7)
{
    SVCHMACSHA1Context ctx;
    u8 md[20];
    BOOL result;
    PROFILE_INIT();

    // ����
    PROFILE_COUNT();
    SVC_HMACSHA1Init(&ctx, hmac_key, sizeof(hmac_key));
    // �w�b�_
    PROFILE_COUNT();
    SVC_HMACSHA1Update(&ctx, pROMHeader, DHT_DS_HEADER_SIZE);
    // ARM9 Static
    PROFILE_COUNT();
    SVC_HMACSHA1Update(&ctx, pARM9, pROMHeader->main_size);
    // ARM7 Static
    PROFILE_COUNT();
    SVC_HMACSHA1Update(&ctx, pARM7, pROMHeader->sub_size);
    // ����
    PROFILE_COUNT();
    SVC_HMACSHA1GetHash(&ctx, md);
    result = SVC_CompareSHA1(db->hash[0], md);
    if ( !result )
    {
        OS_TPrintfEx("DB   = % 20B\n", db->hash[0]);
        OS_TPrintfEx("HASH = % 20B\n", md);
        OS_TPrintf("%s: hash[0] is not valid.\n", __func__);
    }
    // ���ʕ�
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to check the hash (phase 1).\n");
    OS_TPrintf("%10d msec for preparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for scanning header.\n",  (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for scanning ARM9.\n",    (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("%10d msec for scanning ARM7.\n",    (int)OS_TicksToMilliSeconds(profile[4]-profile[3]));
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[5]-profile[4]));
    OS_TPrintf("\nTotal: %10d msec.\n",     (int)OS_TicksToMilliSeconds(profile[5]-profile[0]));
#endif
    return result;
}
/*
�n�b�V���v�Z (2)
�Ώۗ̈�̓ǂݍ��݂ƃ`�F�b�N���s��
FS��p�����e�X�g�̏ꍇ��CARD�A�v���̏ꍇ�ňقȂ�
*/
#if 1
// FS�� (fctx == FSFile*)
static BOOL ReadImage(void* fctx, void* dest, s32 offset, s32 length)
{
    FSFile* fp = fctx;
    s32 result;
    if ( !FS_SeekFile(fp, offset, FS_SEEK_SET) )
    {
        OS_TPrintf("Cannot seek to the offset (%d bytes).\n", offset);
        return FALSE;
    }
    result = FS_ReadFile(fp, dest, length);
    if ( result != length )
    {
        OS_TPrintf("Cannot read the data (%d bytes).\n", length);
        return FALSE;
    }
    return TRUE;
}
#else
// CARD�� (fctx == dma no)
static BOOL ReadImage(void* fctx, void* dest, s32 offset, s32 length)
{
    u32 dma = (u32)fctx;
    CARD_ReadRom(dma, (void*)offset, dest, (u32)length);
    return TRUE;
}
#endif

static BOOL ImageHMACSHA1Update(SVCHMACSHA1Context* ctx, void* fctx, s32 offset, s32 length, void* buffer)
{
    if ( !ReadImage(fctx, buffer, offset, length) )
    {
        return FALSE;
    }
    SVC_HMACSHA1Update(ctx, buffer, (u32)length);
    return TRUE;
}

static BOOL GetOverlayInfo(int no, void* fctx, int fat_offset, int* pOffset, int* pLength)
{
    ROM_FAT fat;
    if ( !ReadImage(fctx, &fat, fat_offset + no * (s32)sizeof(ROM_FAT), sizeof(ROM_FAT)) )
    {
        return FALSE;
    }
    if ( pOffset )
    {
        *pOffset = (s32)fat.top.offset;
    }
    if ( pLength )
    {
        *pLength = (s32)(fat.bottom.offset - fat.top.offset);
    }
    return TRUE;
}

BOOL DHT_CheckHashPhase2(const DHTDatabase *db, const ROM_Header_Short* pROMHeader, void* fctx, void* buffer)
{
    int overlay_nums = (int)(pROMHeader->main_ovt_size / sizeof(ROM_OVT));
    u8 md[20];
    PROFILE_INIT();

    if ( overlay_nums )
    {
        SVCHMACSHA1Context ctx;
        int total_sectors;
        int i;

        // ����
        PROFILE_COUNT();
        SVC_HMACSHA1Init(&ctx, hmac_key, sizeof(hmac_key));
        // OVT
        PROFILE_COUNT();
        if ( !ImageHMACSHA1Update(&ctx, fctx, (s32)pROMHeader->main_ovt_offset, (s32)pROMHeader->main_ovt_size, buffer) )
        {
            OS_TPrintf("Cannot calc HMAC-SHA1 for OVT.\n");
            return FALSE;
        }
        // FAT
        PROFILE_COUNT();
        if ( !ImageHMACSHA1Update(&ctx, fctx, (s32)pROMHeader->fat_offset, overlay_nums * (s32)sizeof(ROM_FAT), buffer) )
        {
            OS_TPrintf("Cannot calc HMAC-SHA1 for %d of FAT.\n", overlay_nums);
            return FALSE;
        }
        // �e�I�[�o�[���C
        PROFILE_COUNT();
        total_sectors = 0;
        for (i = 0; i < overlay_nums; i++)
        {
            int max_sectors = (DHT_OVERLAY_MAX/512 - total_sectors) / (overlay_nums - i);
            int offset;
            int length;
            if ( !GetOverlayInfo(i, fctx, (s32)pROMHeader->fat_offset, &offset, &length) )
            {
                OS_TPrintf("Cannot get %d of overlay info.\n", i);
                return FALSE;
            }
            length = (length + 511) / 512;  // bytes -> sectors
            if ( length > max_sectors )
            {
                length = max_sectors;
            }
            if ( !ImageHMACSHA1Update(&ctx, fctx, offset, length * 512, buffer) )
            {
                OS_TPrintf("Cannot calc HMAC-SHA1 for %d of overlay.\n", i);
                return FALSE;
            }
            total_sectors += length;
        }
        // ����
        PROFILE_COUNT();
        SVC_HMACSHA1GetHash(&ctx, md);
    }
    else
    {
        PROFILE_COUNT();
        PROFILE_COUNT();
        PROFILE_COUNT();
        PROFILE_COUNT();
        PROFILE_COUNT();
        MI_CpuClear8(md, sizeof(md));
    }
    if ( !SVC_CompareSHA1(md, db->hash[1]) )
    {
        OS_TPrintfEx("DB   = % 20B\n", db->hash[1]);
        OS_TPrintfEx("HASH = % 20B\n", md);
        OS_TPrintf("%s: hash[1] is not valid.\n", __func__);
        return FALSE;
    }
    // ���ʕ�
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to check the hash (phase 2).\n");
    OS_TPrintf("%10d msec for preparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for scanning OVT.\n",     (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for scanning FAT.\n",     (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("%10d msec for scanning every overlays.\n",  (int)OS_TicksToMilliSeconds(profile[4]-profile[3]));
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[5]-profile[4]));
    OS_TPrintf("\nTotal: %10d msec.\n",     (int)OS_TicksToMilliSeconds(profile[5]-profile[0]));
#endif
    return TRUE;
}


