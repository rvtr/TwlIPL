/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mmap.c

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

#ifndef	_SYSMENU_MMAP_H_
#define	_SYSMENU_MMAP_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------
#define BOOTCORE_ARM9_ADDR					0x02e7fc00					// ARM9最終ブートコードアドレス
#define BOOTCORE_ARM7_ADDR					0x0380f100					// ARM7最終ブートコードアドレス
#define SYSROM9_NINLOGO_ADR					0xffff0020					// ARM9システムROM内の任天堂ロゴ格納アドレス

#define SYSM_ARM9_LOAD_MMEM_LAST_ADDR		0x02280000					// ロード可能なARM9 staticメインメモリ最終アドレス
#define SYSM_ARM7_LOAD_MMEM_LAST_ADDR		0x023c0000					// ロード可能なARM7 staticメインメモリ最終アドレス
#define SYSM_ARM7_LOAD_WRAM_LAST_ADDR		BOOTCORE_ARM7_ADDR			// ロード可能なARM7 staticメインメモリ最終アドレス
#define SYSM_ARM9_MMEM_ENTRY_ADDR_LIMIT		SYSM_ARM9_LOAD_MMEM_LAST_ADDR

#ifdef __cplusplus
}
#endif

#endif // _SYSMENU_MMAP_H_

