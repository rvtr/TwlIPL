/*---------------------------------------------------------------------------*
  Project:  TwlIPL - DHT
  File:     dht.h

  Copyright 2008,2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef SYSMENU_DHT_H_
#define SYSMENU_DHT_H_

/*
    Phase、ホワイトリスト、マスタリング済みヘッダの対応関係

    Phase1  DHTDatabase->hash[0]        ROM_Header_Short->nitro_whitelist_phase1_digest
    Phase2  DHTDatabase->hash[1]        ROM_Header_Short->nitro_whitelist_phase2_digest
    Phase3  DHTDatabaseEx->banner_hash  ROM_Header_Short->banner_digest
    Phase4  -                           -

    Phase4のハッシュ値はdht_phase4_list.cに含まれる
*/

#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu/dht/dht_format.h>

#define nitro_whitelist_phase2_digest nitro_whitelist_phase2_diegst // for spell miss

#define DHT_FAT_PAGE_SIZE   512
#define DHT_FAT_CACHE_SIZE  (DHT_FAT_PAGE_SIZE * 2)

#define DHT_PHASE3_MAX      DHT_OVERLAY_MAX

/*
    DHT_CheckHashPhase2で必要なワークメモリ
*/
typedef struct DHTPhase2Work
{
    u8  fatCache[DHT_FAT_CACHE_SIZE];           // for fat cache only
    u32 buffer[DHT_OVERLAY_MAX/sizeof(u32)];    // multiple usage
}
DHTPhase2Work;

/*
    DHT_CheckHashPhase4で必要なワークメモリ
    (DHTPhase4Work <= DHTPhase2Workが必ず成り立つ)
*/

typedef struct DHTPhase4Work
{
    u32 buffer[DHT_PHASE3_MAX/sizeof(u32)];
}
DHTPhase4Work;

/*
    DHT_CheckHashPhase2Exで必要なワークメモリ
*/
typedef struct DHTPhase2ExWork
{
    u8  fatCache[DHT_FAT_CACHE_SIZE];           // for fat cache only
}
DHTPhase2ExWork;

#ifdef __cplusplus
extern "C" {
#endif

/*
    DHT_CheckHashPhase2/DHT_CheckHashPhase2Ex/DHT_CheckHashPhase4で使用するRead関数
    dest        転送先アドレス
    offset      転送元ROMオフセット
    length      転送サイズ
    arg         アプリケーションから渡された値

    回復不能な内部エラー発生時にはFALSEを返すこと
*/
typedef BOOL    (*DHTReadFunc)(void* dest, s32 offset, s32 length, void* arg);

/*
    DHT_CheckHashPhase2Ex/DHT_CheckHashPhase4Exで使用するRead関数
    転送先アドレスは存在せず、代わりに独自バッファに読み込んだ後
    DHT_CheckHashPhase2ExUpdateを呼び出すこと(細分化可能)
    ctx         DHT_CheckHashPhase2ExUpdateに渡す引数
    offset      転送元ROMオフセット
    length      転送サイズ
    arg         アプリケーションから渡された値

    回復不能な内部エラー発生時にはFALSEを返すこと
*/
typedef BOOL    (*DHTReadFuncEx)(SVCHMACSHA1Context* ctx, s32 offset, s32 length, void* arg);
/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabaseLength

  Description:  読み込み済みのデータベースのヘッダからサイズを返す

  Arguments:    pDHT        データベースヘッダの格納先

  Returns:      正しそうなヘッダならサイズ、そうでないなら0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseLength(const DHTFile* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabaseExLength

  Description:  読み込み済みの拡張データベースのヘッダからサイズを返す

  Arguments:    pDHT        拡張データベースヘッダの格納先

  Returns:      正しそうなヘッダならサイズ、そうでないなら0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseExLength(const DHTFileEx* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabaseAdHocLength

  Description:  読み込み済みの個別対応データベースのヘッダからサイズを返す

  Arguments:    pDHT        個別対応データベースヘッダの格納先

  Returns:      正しそうなヘッダならサイズ、そうでないなら0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseAdHocLength(const DHTFileAdHoc* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  FS関数を利用してデータベースを読み込みと検証を行う

  Arguments:    pDHT        データベースの格納先
                fp          ファイル構造体へのポインタ
                            DHTHeaderの先頭までシーク済みである必要がある
                maxLength   読み込みサイズの上限(ヘッダ込み)

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabase(DHTFile* pDHT, FSFile* fp, s32 maxLength);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabaseEx

  Description:  FS関数を利用して拡張データベースを読み込みと検証を行う

  Arguments:    pDHT        拡張データベースの格納先
                fp          ファイル構造体へのポインタ
                            DHTHeaderの先頭までシーク済みである必要がある
                maxLength   読み込みサイズの上限(ヘッダ込み)

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabaseEx(DHTFileEx* pDHT, FSFile* fp, s32 maxLength);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabaseAdHoc

  Description:  FS関数を利用して個別対応データベースを読み込みと検証を行う

  Arguments:    pDHT        個別対応データベースの格納先
                fp          ファイル構造体へのポインタ
                            DHTHeaderの先頭までシーク済みである必要がある
                maxLength   読み込みサイズの上限(ヘッダ込み)

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabaseAdHoc(DHTFileAdHoc* pDHT, FSFile* fp, s32 maxLength);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabase

  Description:  ROMヘッダに対応するデータベースを検索する

  Arguments:    pDHT        データベースの格納先
                pROMHeader  対象となるROMヘッダ格納先

  Returns:      対象エントリへのポインタ、見つからなければNULL
 *---------------------------------------------------------------------------*/
const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabaseEx

  Description:  ROMヘッダに対応する拡張データベースを検索する

  Arguments:    pDHT        拡張データベースの格納先
                pROMHeader  対象となるROMヘッダ格納先

  Returns:      対象エントリへのポインタ、見つからなければNULL
 *---------------------------------------------------------------------------*/
const DHTDatabaseEx* DHT_GetDatabaseEx(const DHTFileEx* pDHT, const ROM_Header_Short* pROMHeader);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1Init

  Description:  ROMヘッダおよびARM9/ARM7スタティック領域の検証の準備

  Arguments:    ctx         検証用のSVCHMACSHA1コンテキスト
                pROMHeader  対象となるROMヘッダ格納先

  Returns:      None
 *---------------------------------------------------------------------------*/
void DHT_CheckHashPhase1Init(SVCHMACSHA1Context* ctx, const ROM_Header_Short* pROMHeader);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1Update

  Description:  ROMヘッダおよびARM9/ARM7スタティック領域の検証のスタティック部分
                いくら分割しても良いが、ARM9スタティック、ARM7スタティックの順に
                呼び出すこと。

  Arguments:    ctx         検証用のSVCHMACSHA1コンテキスト
                ptr         対象となるデータ領域
                length      対象となるデータサイズ

  Returns:      None
 *---------------------------------------------------------------------------*/
void DHT_CheckHashPhase1Update(SVCHMACSHA1Context* ctx, const void* ptr, s32 length);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1

  Description:  ROMヘッダおよびARM9/ARM7スタティック領域の検証の結果判定

  Arguments:    ctx         検証用のSVCHMACSHA1コンテキスト
                hash        対応するハッシュ (db->hash[0])

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase1Final(SVCHMACSHA1Context* ctx, const u8* hash);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1

  Description:  ROMヘッダおよびARM9/ARM7スタティック領域の検証

  Arguments:    hash        対応するハッシュ (db->hash[0])
                pROMHeader  対象となるROMヘッダ格納先
                pARM9       対象となるARM9スタティック格納先
                pARM7       対象となるARM7スタティック格納先

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase1(const u8* hash, const ROM_Header_Short* pROMHeader, const void* pARM9, const void* pARM7);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase2

  Description:  オーバーレイ領域の検証

  Arguments:    hash        対応するハッシュ (db->hash[1])
                pROMHeader  対象となるROMヘッダ格納先
                work        本APIで使用するワーク (513KB)
                func        対象デバイスに応じたRead関数
                arg         Read関数に渡される引数

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase2(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2Work* work, DHTReadFunc func, void* arg);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase2Ex

  Description:  オーバーレイ領域の検証

  Arguments:    hash        対応するハッシュ (db->hash[1])
                pROMHeader  対象となるROMヘッダ格納先
                work        本APIで使用するワーク (1KB)
                func        対象デバイスに応じたRead関数
                funcEx      対象デバイスに応じて独自バッファにデータを読み込み
                            DHT_CheckHashPhase2ExUpdateを呼び出す必要がある
                arg         Read関数に渡される引数

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase2Ex(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2ExWork* work, DHTReadFunc func, DHTReadFuncEx funcEx, void* arg);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase2ExUpdate / DHT_CheckHashPhase4ExUpdate

  Description:  オーバーレイ部分の検証および個別の検証
                DHTReadFuncExから呼び出すこと(さらなる細分化は自由)
                注意: Phase4でも流用している

  Arguments:    ctx         検証用のSVCHMACSHA1コンテキスト
                ptr         対象となるデータ領域
                length      対象となるデータサイズ

  Returns:      None
 *---------------------------------------------------------------------------*/
void DHT_CheckHashPhase2ExUpdate(SVCHMACSHA1Context* ctx, const void* ptr, s32 length);
#define DHT_CheckHashPhase4ExUpdate DHT_CheckHashPhase2ExUpdate

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase3

  Description:  バナー領域の検証
                (メニュー表示に使用したデータを渡すべき)

  Arguments:    hash        対応するハッシュ (dbex->banner_hash)
                pBanner     対象となるバナー格納先

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase3(const u8* hash, const NTRBannerFile* pBanner);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase4

  Description:  個別の検証

  Arguments:    pDHT        個別対応データベースの格納先
                pROMHeader  対象となるROMヘッダ格納先
                work        本APIで使用するワーク (512KB)
                            phase2の使い回しでOK
                func        対象デバイスに応じたRead関数
                arg         Read関数に渡される引数

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase4(const DHTFileAdHoc* pDHT, const ROM_Header_Short* pROMHeader, DHTPhase4Work* work, DHTReadFunc func, void* arg);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase4Ex

  Description:  個別の検証

  Arguments:    pDHT        個別対応データベースの格納先
                pROMHeader  対象となるROMヘッダ格納先
                funcEx      対象デバイスに応じて独自バッファにデータを読み込み
                            DHT_CheckHashPhase2ExUpdateを呼び出す必要がある
                arg         Read関数に渡される引数

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase4Ex(const DHTFileAdHoc* pDHT, const ROM_Header_Short* pROMHeader, DHTReadFuncEx funcEx, void* arg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // SYSMENU_DHT_H_
