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
#ifndef _DHT_H_
#define _DHT_H_

#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <firm/dshashtable.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL DHT_PrepareDatabase(DHTFile* pDHT);

const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader);

BOOL DHT_CheckHashPhase1(const DHTDatabase *db, const ROM_Header_Short* pROMHeader, const void* pARM9, const void* pARM7);

BOOL DHT_CheckHashPhase2(const DHTDatabase *db, const ROM_Header_Short* pROMHeader, FSFile* fp, void* buffer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // _DHT_H_
