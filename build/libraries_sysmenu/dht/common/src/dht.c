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


/*
    定義すると処理時間を表示する
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

static const u8 hmac_key[]      = DHT_HMAC_KEY;     // for phase1, phase2
static const u8 hmac_key2[]     = DHT_HMAC_KEY2;    // for phase3, phase4

static DHTReadFunc  imageReadFunc;
static void*        imageBuffer;
static u8*          fatCache;
static int          fatPage;

/*
    自家製bsearch
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
データベースを読み込む (前準備)
*/
u32 DHT_GetDatabaseLength(const DHTFile* pDHT)
{
    if ( pDHT->header.magic_code != DHT_MAGIC_CODE )    // magic codeチェック
    {
        OS_TPrintf("Invalid magic code (magic=0x%08X).\n", pDHT->header.magic_code);
        return 0;
    }
    return sizeof(DHTHeader) + pDHT->header.nums * sizeof(DHTDatabase);
}
u32 DHT_GetDatabaseExLength(const DHTFileEx* pDHT)
{
    if ( pDHT->header.magic_code != DHT_MAGIC_CODE_EX ) // magic codeチェック
    {
        OS_TPrintf("Invalid magic code (magic=0x%08X) [EX].\n", pDHT->header.magic_code);
        return 0;
    }
    return sizeof(DHTHeader) + pDHT->header.nums * sizeof(DHTDatabaseEx);
}
u32 DHT_GetDatabaseAdHocLength(const DHTFileAdHoc* pDHT)
{
    if ( pDHT->header.magic_code != DHT_MAGIC_CODE_ADHOC ) // magic codeチェック
    {
        OS_TPrintf("Invalid magic code (magic=0x%08X) [AdHoc].\n", pDHT->header.magic_code);
        return 0;
    }
    return sizeof(DHTHeader) + pDHT->header.nums * sizeof(DHTDatabaseAdHoc);
}

static BOOL DHT_CheckDatabase(const DHTFile* pDHT)
{
    SVCSignHeapContext pool;
    static u8 heap[4*1024]; // avoid stack overflow
    u8 md1[20];
    u8 md2[20];
    s32 result;
    // ファイル署名取り出し
    SVC_InitSignHeap(&pool, heap, sizeof(heap));
    SVC_DecryptSign(&pool, md1, pDHT->header.sign, &g_pubkey_DER[29]);
    // ハッシュ計算
    SVC_CalcSHA1(md2, DHT_GET_SIGN_TARGET_ADDR(&pDHT->header), DHT_GET_SIGN_TARGET_SIZE(&pDHT->header));
    // 検証
    result = SVC_CompareSHA1(md1, md2);
    if ( !result )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("SIGN = % 20B\n", md1);
        OS_TPrintfEx("HASH = % 20B\n", md2);
        OS_TPrintf("Signature is not valid.\n");
        return FALSE;
    }
    return TRUE;
}

BOOL DHT_PrepareDatabase(DHTFile* pDHT, FSFile* fp, s32 maxLength)
{
    s32 result;
    s32 length;
    PROFILE_INIT();

    if ( fp )
    {
        // ヘッダ読み込み
        PROFILE_COUNT();
        result = FS_ReadFile(fp, &pDHT->header, sizeof(DHTHeader));
        if ( result != sizeof(DHTHeader) )
        {
            OS_TPrintf("Cannot read the DHT header (result=%d).\n", result);
            return FALSE;
        }
        // データベース読み込み
        PROFILE_COUNT();
        length = (s32)DHT_GetDatabaseLength(pDHT);
        if ( length < sizeof(DHTHeader) )
        {
            OS_TPrintf("Invalid DHT header.\n");
            return FALSE;
        }
        if ( length > maxLength )
        {
            OS_TPrintf("Too large size specified in the header.\n");
            return FALSE;
        }
        length -= (s32)sizeof(DHTHeader);   // ヘッダを除く
        result = FS_ReadFile(fp, pDHT->database, length);
        if ( result != length )
        {
            OS_TPrintf("Cannot read the DHT database (result=%d).\n", result);
            return FALSE;
        }
    }
    else
    {
        PROFILE_COUNT();
        PROFILE_COUNT();
    }

    // データベースの検証
    PROFILE_COUNT();
    result = DHT_CheckDatabase(pDHT);

    // 結果報告
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to prepare the database.\n");
    OS_TPrintf("%10d msec for reading header.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for reading database.\n", (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("\nTotal: %10d msec.\n",             (int)OS_TicksToMilliSeconds(profile[3]-profile[0]));
#endif
    return result;
}

/*
ROMヘッダに対応するデータベースを手に入れる
*/
const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader)
{
    u8 data[5];
    DHTDatabase* db;
    PROFILE_INIT();

    // 準備
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
拡張データベースを読み込む (前準備)
*/

static BOOL DHT_CheckDatabaseEx(const DHTFileEx* pDHT)
{
    SVCSignHeapContext pool;
    static u8 heap[4*1024]; // avoid stack overflow
    u8 md1[20];
    u8 md2[20];
    s32 result;
    // ファイル署名取り出し
    SVC_InitSignHeap(&pool, heap, sizeof(heap));
    SVC_DecryptSign(&pool, md1, pDHT->header.sign, &g_pubkey_DER[29]);
    // ハッシュ計算
    SVC_CalcSHA1(md2, DHT_GET_SIGN_TARGET_ADDR(&pDHT->header), DHT_GET_SIGN_TARGET_SIZE_EX(&pDHT->header));
    // 検証
    result = SVC_CompareSHA1(md1, md2);
    if ( !result )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("SIGN = % 20B [EX]\n", md1);
        OS_TPrintfEx("HASH = % 20B [EX]\n", md2);
        OS_TPrintf("Signature is not valid. [EX]\n");
        return FALSE;
    }
    return TRUE;
}

BOOL DHT_PrepareDatabaseEx(DHTFileEx* pDHT, FSFile* fp, s32 maxLength)
{
    s32 result;
    s32 length;
    PROFILE_INIT();

    if ( fp )
    {
        // ヘッダ読み込み
        PROFILE_COUNT();
        result = FS_ReadFile(fp, &pDHT->header, sizeof(DHTHeader));
        if ( result != sizeof(DHTHeader) )
        {
            OS_TPrintf("Cannot read the DHT header (result=%d). [EX]\n", result);
            return FALSE;
        }
        // 拡張データベース読み込み
        PROFILE_COUNT();
        length = (s32)DHT_GetDatabaseExLength(pDHT);
        if ( length < sizeof(DHTHeader) )
        {
            OS_TPrintf("Invalid DHT header. [EX]\n");
            return FALSE;
        }
        if ( length > maxLength )
        {
            OS_TPrintf("Too large size specified in the header.\n");
            return FALSE;
        }
        length -= (s32)sizeof(DHTHeader); // ヘッダを除く
        result = FS_ReadFile(fp, pDHT->database, length);
        if ( result != length )
        {
            OS_TPrintf("Cannot read the DHT database (result=%d). [EX]\n", result);
            return FALSE;
        }
    }
    else
    {
        PROFILE_COUNT();
        PROFILE_COUNT();
    }

    // 拡張データベースの検証
    PROFILE_COUNT();
    result = DHT_CheckDatabaseEx(pDHT);

    // 結果報告
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to prepare the database. [EX]\n");
    OS_TPrintf("%10d msec for reading header. [EX]\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for reading database. [EX]\n", (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for comparing hash. [EX]\n",   (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("\nTotal: %10d msec. [EX]\n",             (int)OS_TicksToMilliSeconds(profile[3]-profile[0]));
#endif
    return result;
}

/*
ROMヘッダに対応する拡張データベースを手に入れる
*/
const DHTDatabaseEx* DHT_GetDatabaseEx(const DHTFileEx* pDHT, const ROM_Header_Short* pROMHeader)
{
    u8 data[5];
    DHTDatabaseEx* db;
    PROFILE_INIT();

    // 準備
    PROFILE_COUNT();
    MI_CpuCopy8( pROMHeader->game_code, data, 4 );
    data[4] = pROMHeader->rom_version;
    db = (DHTDatabaseEx*)bsearch(data, pDHT->database, pDHT->header.nums, sizeof(DHTDatabaseEx), CompareGameCodeAndVersion);
    if ( !db )
    {
        OS_TPrintf("Cannot find the database. [EX]\n");
    }
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("%10d msec for searching database. [EX]\n", (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
    return db;
}
/*
個別対応データベースを読み込む (前準備)
*/

static BOOL DHT_CheckDatabaseAdHoc(const DHTFileAdHoc* pDHT)
{
    SVCSignHeapContext pool;
    static u8 heap[4*1024]; // avoid stack overflow
    u8 md1[20];
    u8 md2[20];
    s32 result;
    // ファイル署名取り出し
    SVC_InitSignHeap(&pool, heap, sizeof(heap));
    SVC_DecryptSign(&pool, md1, pDHT->header.sign, &g_pubkey_DER[29]);
    // ハッシュ計算
    SVC_CalcSHA1(md2, DHT_GET_SIGN_TARGET_ADDR(&pDHT->header), DHT_GET_SIGN_TARGET_SIZE_ADHOC(&pDHT->header));
    // 検証
    result = SVC_CompareSHA1(md1, md2);
    if ( !result )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("SIGN = % 20B [AdHoc]\n", md1);
        OS_TPrintfEx("HASH = % 20B [AdHoc]\n", md2);
        OS_TPrintf("Signature is not valid. [AdHoc]\n");
        return FALSE;
    }
    return TRUE;
}

BOOL DHT_PrepareDatabaseAdHoc(DHTFileAdHoc* pDHT, FSFile* fp, s32 maxLength)
{
    s32 result;
    s32 length;
    PROFILE_INIT();

    if ( fp )
    {
        // ヘッダ読み込み
        PROFILE_COUNT();
        result = FS_ReadFile(fp, &pDHT->header, sizeof(DHTHeader));
        if ( result != sizeof(DHTHeader) )
        {
            OS_TPrintf("Cannot read the DHT header (result=%d). [AdHoc]\n", result);
            return FALSE;
        }
        // 個別対応データベース読み込み
        PROFILE_COUNT();
        length = (s32)DHT_GetDatabaseAdHocLength(pDHT);
        if ( length < sizeof(DHTHeader) )
        {
            OS_TPrintf("Invalid DHT header. [AdHoc]\n");
            return FALSE;
        }
        if ( length > maxLength )
        {
            OS_TPrintf("Too large size specified in the header.\n");
            return FALSE;
        }
        length -=  (s32)sizeof(DHTHeader); // ヘッダを除く
        result = FS_ReadFile(fp, pDHT->database, length);
        if ( result != length )
        {
            OS_TPrintf("Cannot read the DHT database (result=%d). [AdHoc]\n", result);
            return FALSE;
        }
    }
    else
    {
        PROFILE_COUNT();
        PROFILE_COUNT();
    }

    // 個別対応データベースの検証
    PROFILE_COUNT();
    result = DHT_CheckDatabaseAdHoc(pDHT);

    // 結果報告
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to prepare the database. [AdHoc]\n");
    OS_TPrintf("%10d msec for reading header. [AdHoc]\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for reading database. [AdHoc]\n", (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for comparing hash. [AdHoc]\n",   (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("\nTotal: %10d msec. [AdHoc]\n",             (int)OS_TicksToMilliSeconds(profile[3]-profile[0]));
#endif
    return result;
}

/*
ROMヘッダに対応する個別対応データベースを手に入れる
*/
static const DHTDatabaseAdHoc* DHT_GetDatabaseAdHoc(const DHTFileAdHoc* pDHT, const ROM_Header_Short* pROMHeader)
{
    u8 data[5];
    DHTDatabaseAdHoc* db;
    PROFILE_INIT();

    // 準備
    PROFILE_COUNT();
    MI_CpuCopy8( pROMHeader->game_code, data, 4 );
    data[4] = pROMHeader->rom_version;
    db = (DHTDatabaseAdHoc*)bsearch(data, pDHT->database, pDHT->header.nums, sizeof(DHTDatabaseAdHoc), CompareGameCodeAndVersion);
    if ( !db )
    {
        OS_TPrintf("Cannot find the database. [AdHoc]\n");
    }
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("%10d msec for searching database. [AdHoc]\n", (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
    return db;
}

/*
ハッシュ計算 (1)
読み込み済みデータをチェックする
*/
void DHT_CheckHashPhase1Init(SVCHMACSHA1Context* ctx, const ROM_Header_Short* pROMHeader)
{
    PROFILE_INIT();
    PROFILE_COUNT();
    // 準備
    SVC_HMACSHA1Init(ctx, hmac_key, sizeof(hmac_key));
    // ヘッダ
    SVC_HMACSHA1Update(ctx, pROMHeader, DHT_DS_HEADER_SIZE);
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\n%10d msec for scanning header.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
}
void DHT_CheckHashPhase1Update(SVCHMACSHA1Context* ctx, const void* ptr, s32 length)
{
    PROFILE_INIT();
    PROFILE_COUNT();
    // ARM9 or ARM7 static
    if ( length > 0 )
    {
        SVC_HMACSHA1Update(ctx, ptr, (u32)length);
    }
    // 結果報告
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("%10d msec for scanning %d bytes.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]), length);
#endif
}
BOOL DHT_CheckHashPhase1Final(SVCHMACSHA1Context* ctx, const u8 *hash)
{
    u8 md[20];
    BOOL result;
    PROFILE_INIT();
    PROFILE_COUNT();
    SVC_HMACSHA1GetHash(ctx, md);
    result = SVC_CompareSHA1(hash, md);
    if ( !result )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("DB   = % 20B\n", hash);
        OS_TPrintfEx("HASH = % 20B\n", md);
        OS_TPrintf("%s: hash[0] is not valid.\n", __func__);
    }
    // 結果報告
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
    return result;
}
BOOL DHT_CheckHashPhase1(const u8* hash, const ROM_Header_Short* pROMHeader, const void* pARM9, const void* pARM7)
{
    SVCHMACSHA1Context ctx;
    BOOL result;
    PROFILE_INIT();

    // 準備＆ヘッダ
    PROFILE_COUNT();
    DHT_CheckHashPhase1Init(&ctx, pROMHeader);

    // ARM9 Static
    DHT_CheckHashPhase1Update(&ctx, pARM9, (s32)pROMHeader->main_size);
    // ARM7 Static
    DHT_CheckHashPhase1Update(&ctx, pARM7, (s32)pROMHeader->sub_size);
    // 検証
    result = DHT_CheckHashPhase1Final(&ctx, hash);
    // 結果報告
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to check the hash (phase 1).\n");
    OS_TPrintf("\nTotal: %10d msec.\n",     (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
    return result;
}

/*
ハッシュ計算 (2)
対象領域の読み込みとチェックを行う
*/
static BOOL ImageHMACSHA1Update(SVCHMACSHA1Context* ctx, s32 offset, s32 length, void* arg)
{
    if ( !imageBuffer || !imageReadFunc )
    {
        return FALSE;
    }
    if ( !imageReadFunc(imageBuffer, offset, length, arg) )
    {
        return FALSE;
    }
    SVC_HMACSHA1Update(ctx, imageBuffer, (u32)length);
    return TRUE;
}

static BOOL GetOverlayInfo(int no, int fat_offset, int* pOffset, int* pLength, DHTReadFunc func, void* arg)
{
    ROM_FAT *fat;
    int page = (fat_offset + no * (s32)sizeof(ROM_FAT)) / DHT_FAT_PAGE_SIZE;
    if ( !fatCache )
    {
        return FALSE;
    }
    if ( fatPage != page )
    {
        if ( fatPage + 1 == page )    // 1ページはキャッシュ済み
        {
            MI_CpuCopy8( &fatCache[DHT_FAT_PAGE_SIZE], &fatCache[0], DHT_FAT_PAGE_SIZE );
            if ( !func(&fatCache[DHT_FAT_PAGE_SIZE], (page+1) * DHT_FAT_PAGE_SIZE, DHT_FAT_PAGE_SIZE, arg) )
            {
                return FALSE;
            }
        }
        else    // 通常は2ページ読み
        {
            if ( !func(fatCache, page * DHT_FAT_PAGE_SIZE, DHT_FAT_CACHE_SIZE, arg) )
            {
                return FALSE;
            }
        }
        fatPage = page;
    }
    fat = (ROM_FAT*)(fatCache + fat_offset + no * sizeof(ROM_FAT) - page * DHT_FAT_PAGE_SIZE);
    if ( pOffset )
    {
        *pOffset = (s32)fat->top.offset;
    }
    if ( pLength )
    {
        *pLength = (s32)(fat->bottom.offset - fat->top.offset);
    }
    return TRUE;
}

BOOL DHT_CheckHashPhase2(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2Work* work, DHTReadFunc func, void* arg)
{
    imageBuffer = work->buffer;
    imageReadFunc = func;
    return DHT_CheckHashPhase2Ex(hash, pROMHeader, (DHTPhase2ExWork*)work->fatCache, func, ImageHMACSHA1Update, arg);
}

BOOL DHT_CheckHashPhase2Ex(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2ExWork* work, DHTReadFunc func, DHTReadFuncEx funcEx, void* arg)
{
    int overlay_nums = (int)(pROMHeader->main_ovt_size / sizeof(ROM_OVT));
    u8 md[20];
    PROFILE_INIT();

    if ( overlay_nums )
    {
        SVCHMACSHA1Context ctx;
        int total_sectors;
        int i;

        if ( !func || !funcEx || !work )
        {
            return FALSE;
        }
        fatCache    = work->fatCache;
        fatPage     = -2;   // default value = out of range

        // 準備
        PROFILE_COUNT();
        SVC_HMACSHA1Init(&ctx, hmac_key, sizeof(hmac_key));
        // OVT
        PROFILE_COUNT();
        if ( !funcEx(&ctx, (s32)pROMHeader->main_ovt_offset, (s32)pROMHeader->main_ovt_size, arg) )
        {
            OS_TPrintf("Cannot calc HMAC-SHA1 for OVT.\n");
            return FALSE;
        }
        // FAT
        PROFILE_COUNT();
        if ( !funcEx(&ctx, (s32)pROMHeader->fat_offset, overlay_nums * (s32)sizeof(ROM_FAT), arg) )
        {
            OS_TPrintf("Cannot calc HMAC-SHA1 for %d of FAT.\n", overlay_nums);
            return FALSE;
        }
        // 各オーバーレイ
        PROFILE_COUNT();
        total_sectors = 0;
        for (i = 0; i < overlay_nums; i++)
        {
            int max_sectors = (DHT_OVERLAY_MAX/512 - total_sectors) / (overlay_nums - i);
            int offset;
            int length;
            if ( !GetOverlayInfo(i, (s32)pROMHeader->fat_offset, &offset, &length, func, arg) )
            {
                OS_TPrintf("Cannot get %d of overlay info.\n", i);
                return FALSE;
            }
            length = (length + 511) / 512;  // bytes -> sectors
            if ( length > max_sectors )
            {
                length = max_sectors;
            }
            if ( length < 0 || offset < sizeof(ROM_Header) )
            {
                OS_TPrintf("Broken FAT for %d of overlay.\n", i);
                return FALSE;
            }
            if ( !funcEx(&ctx, offset, length * 512, arg) )
            {
                OS_TPrintf("Cannot calc HMAC-SHA1 for %d of overlay.\n", i);
                return FALSE;
            }
            total_sectors += length;
        }
        // 検証
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
    if ( !SVC_CompareSHA1(md, hash) )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("DB   = % 20B\n", hash);
        OS_TPrintfEx("HASH = % 20B\n", md);
        OS_TPrintf("%s: hash[1] is not valid.\n", __func__);
        return FALSE;
    }
    // 結果報告
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

void DHT_CheckHashPhase2ExUpdate(SVCHMACSHA1Context* ctx, const void* ptr, s32 length)
{
    // ARM9 or ARM7 static
    if ( length > 0 )
    {
        SVC_HMACSHA1Update(ctx, ptr, (u32)length);
    }
}

/*
ハッシュ計算 (3)
バナーデータをチェックする
*/
BOOL DHT_CheckHashPhase3(const u8 *hash, const NTRBannerFile* pBanner)
{
    SVCHMACSHA1Context ctx;
    u8 md[20];

    PROFILE_INIT();
    PROFILE_COUNT();
    SVC_HMACSHA1Init(&ctx, hmac_key2, sizeof(hmac_key2));

    // バナーの読み込み (ヘッダ)
    PROFILE_COUNT();
    SVC_HMACSHA1Update(&ctx, &pBanner->h, sizeof(BannerHeader));

    // バナーの読み込み (ボディ)
    if (pBanner->h.version == 0)
    {
        OS_TPrintf("Invalid banner format.\n");
        return FALSE;
    }
    PROFILE_COUNT();
    if ( pBanner->h.version >= 1 )
    {
        SVC_HMACSHA1Update(&ctx, &pBanner->v1, sizeof(BannerFileV1));
    }
    PROFILE_COUNT();
    if ( pBanner->h.version >= 2 )
    {
        SVC_HMACSHA1Update(&ctx, &pBanner->v2, sizeof(BannerFileV2));
    }
    PROFILE_COUNT();
    if ( pBanner->h.version >= 3 )
    {
        SVC_HMACSHA1Update(&ctx, &pBanner->v3, sizeof(BannerFileV3));
    }

    // 検証
    PROFILE_COUNT();
    SVC_HMACSHA1GetHash(&ctx, md);
    if ( !SVC_CompareSHA1(md, hash) )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("DB   = % 20B\n", hash);
        OS_TPrintfEx("HASH = % 20B\n", md);
        OS_TPrintf("%s: banner_hash is not valid.\n", __func__);
        return FALSE;
    }
    // 結果報告
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to check the hash (phase 3).\n");
    OS_TPrintf("%10d msec for preparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for scanning header.\n",     (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for scanning V1 body.\n",     (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("%10d msec for scanning V2 body.\n",  (int)OS_TicksToMilliSeconds(profile[4]-profile[3]));
    OS_TPrintf("%10d msec for scanning V3 body.\n",  (int)OS_TicksToMilliSeconds(profile[5]-profile[4]));
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[6]-profile[5]));
    OS_TPrintf("\nTotal: %10d msec.\n",     (int)OS_TicksToMilliSeconds(profile[6]-profile[0]));
#endif
    return TRUE;
}

/*
ハッシュ計算 (4)
個別対応
*/
BOOL DHT_CheckHashPhase4(const DHTFileAdHoc* pDHT, const ROM_Header_Short* pROMHeader, DHTPhase4Work* work, DHTReadFunc func, void* arg)
{
    imageBuffer = work->buffer;
    imageReadFunc = func;
    return DHT_CheckHashPhase4Ex(pDHT, pROMHeader, ImageHMACSHA1Update, arg);
}

BOOL DHT_CheckHashPhase4Ex(const DHTFileAdHoc* pDHT, const ROM_Header_Short* pROMHeader, DHTReadFuncEx funcEx, void* arg)
{
    const DHTDatabaseAdHoc* plist;
    SVCHMACSHA1Context ctx;
    int i;
    u8 md[20];
    PROFILE_INIT();

    if ( !funcEx || !pDHT || !pROMHeader )
    {
        return FALSE;

    }

    plist = DHT_GetDatabaseAdHoc(pDHT, pROMHeader);

    if ( !plist )
    {
        return TRUE;    // not found in individual list
    }

    // 準備
    PROFILE_COUNT();
    SVC_HMACSHA1Init(&ctx, hmac_key2, sizeof(hmac_key2));

    for (i = 0; i < DHT_INDIVIDUAL_ENTRY_MAX; i++)
    {
        if (plist->entry[i].offset == 0)
        {
            break;
        }
        if ( !funcEx(&ctx, (s32)plist->entry[i].offset, (s32)plist->entry[i].length, arg) )
        {
            OS_TPrintf("Cannot read the phase 3 for %.4s(%x).\n", pROMHeader->game_code, pROMHeader->rom_version);
            return FALSE;
        }
    }
    // 検証
    PROFILE_COUNT();
    SVC_HMACSHA1GetHash(&ctx, md);

    if ( !SVC_CompareSHA1(md, plist->hash) )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("DB   = % 20B\n", plist->hash);
        OS_TPrintfEx("HASH = % 20B\n", md);
        OS_TPrintf("%s: phase4list->hash is not valid.\n", __func__);
        return FALSE;
    }
    // 結果報告
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to check the hash (phase 3).\n");
    OS_TPrintf("%10d msec for preparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for scanning regions.\n",  (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("\nTotal: %10d msec.\n",     (int)OS_TicksToMilliSeconds(profile[3]-profile[0]));
#endif
    return TRUE;
}

