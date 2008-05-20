/*---------------------------------------------------------------------------*
  Project:  TwlIPL - DHT
  File:     dht.h

  Copyright 2008 Nintendo.  All rights reserved.

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

#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu/dht/dht_format.h>

#define DHT_FAT_PAGE_SIZE   512
#define DHT_FAT_CACHE_SIZE  (DHT_FAT_PAGE_SIZE * 2)

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
    DHT_CheckHashPhase2/DHT_CheckHashPhase2Exで使用するRead関数
    dest        転送先アドレス
    offset      転送元ROMオフセット
    length      転送サイズ
    arg         アプリケーションから渡された値

    回復不能な内部エラー発生時にはFALSEを返すこと
*/
typedef BOOL    (*DHTReadFunc)(void* dest, s32 offset, s32 length, void* arg);

/*
    DHT_CheckHashPhase2Exで使用するRead関数
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
  Name:         DHT_PrepareDatabase

  Description:  読み込み済みのデータベースのヘッダからサイズを返す

  Arguments:    pDHT        データベースヘッダの格納先

  Returns:      正しそうなヘッダならサイズ、そうでないなら0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseLength(const DHTFile* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  FS関数を利用して全データベースを読み込みと検証を行う

  Arguments:    pDHT        全データベースの格納先
                fp          ファイル構造体へのポインタ
                            DHTHeaderの先頭までシーク済みである必要がある

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabase(DHTFile* pDHT, FSFile* fp);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabase

  Description:  ROMヘッダに対応するデータベースを検索する

  Arguments:    pDHT        全データベースの格納先
                pROMHeader  対象となるROMヘッダ格納先

  Returns:      対象データベースへのポインタ
 *---------------------------------------------------------------------------*/
const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader);

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
                (デバイスのRead APIを登録できるべき)

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
  Name:         DHT_CheckHashPhase2ExUpdate

  Description:  オーバーレイ部分の検証
                DHTReadFuncExから呼び出すこと(さらなる細分化は自由)

  Arguments:    ctx         検証用のSVCHMACSHA1コンテキスト
                ptr         対象となるデータ領域
                length      対象となるデータサイズ

  Returns:      None
 *---------------------------------------------------------------------------*/
void DHT_CheckHashPhase2ExUpdate(SVCHMACSHA1Context* ctx, const void* ptr, s32 length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // SYSMENU_DHT_H_
