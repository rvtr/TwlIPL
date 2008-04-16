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

typedef struct DHTPhase2Work
{
    u32 buffer[DHT_OVERLAY_MAX/sizeof(u32)];    // multiple usage
    u8  fatCache[DHT_FAT_CACHE_SIZE];           // for fat cache only
}
DHTPhase2Work;

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL    (*DHTReadFunc)(void* dest, s32 offset, s32 length, void* arg);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  読み込み済みのデータベースのヘッダからサイズを返す

  Arguments:    pDHT        データベースヘッダの格納先

  Returns:      正しそうなヘッダならサイズ、そうでないなら0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseLength(const DHTFile* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  読み込み済みの全データベースの署名を検証する

  Arguments:    pDHT        データベースの格納先

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckDatabase(const DHTFile* pDHT);

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
void DHT_CheckHashPhase1Update(SVCHMACSHA1Context* ctx, const void* ptr, u32 length);

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
                (デバイスのRead APIを登録できるべき)

  Arguments:    hash        対応するハッシュ (db->hash[1])
                pROMHeader  対象となるROMヘッダ格納先
                fctx        (FS版) FSFile構造体へのポインタ
                            (CARD版) dma番号をvoid*にキャストしたもの
                            (HOTSW版) CardBootData構造体へのポインタ
                work        本APIで使用するワーク (DHT_OVERLAY_MAXだけ必要)

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase2(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2Work* work, DHTReadFunc func, void* arg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // SYSMENU_DHT_H_
