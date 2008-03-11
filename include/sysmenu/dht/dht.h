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

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  全データベースを読み込む

  Arguments:    pDHT        全データベースの格納先

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabase(DHTFile* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabase

  Description:  ROMヘッダに対応するデータベースを検索する

  Arguments:    pDHT        全データベースの格納先
                pROMHeader  対象となるROMヘッダ格納先

  Returns:      対象データベースへのポインタ
 *---------------------------------------------------------------------------*/
const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1

  Description:  ROMヘッダおよびARM9/ARM7スタティック領域の検証

  Arguments:    db          対象データベースへのポインタ
                pROMHeader  対象となるROMヘッダ格納先
                pARM9       対象となるARM9スタティック格納先
                pARM7       対象となるARM7スタティック格納先

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase1(const DHTDatabase *db, const ROM_Header_Short* pROMHeader, const void* pARM9, const void* pARM7);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase2

  Description:  オー馬齢領域の検証

  Arguments:    db          対象データベースへのポインタ
                pROMHeader  対象となるROMヘッダ格納先
                fctx        (FS版) FSFile構造体へのポインタ
                            (CARD版) dma番号をvoid*にキャストしたもの
                buffer      本APIで使用するワーク (DHT_OVERLAY_MAXだけ必要)

  Returns:      問題なければTRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase2(const DHTDatabase *db, const ROM_Header_Short* pROMHeader, void* fctx, void* buffer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // SYSMENU_DHT_H_
