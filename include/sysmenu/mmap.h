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

#ifndef	__MMAP_H__
#define	__MMAP_H__

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------

#define RETURN_FROM_MAIN_ARM9_FUNCP			0x023fee00					// NITROゲームブート時のARM9最終処理の動作アドレス
#define RETURN_FROM_MAIN_ARM7_FUNCP			0x0380f600					// NITROゲームブート時のARM7最終処理の動作アドレス

#define SYSM_ADDR_TOP						0x02300000					// SYSMが配置される先頭アドレス
#define SYSM_ADDR_BOTTOM					0x023fe000					// SYSMが配置される最終アドレス

#define SYSM_ARM9_MMEM_ENTRY_ADDR_LIMIT		( SYSM_ADDR_TOP - 0x80000 )					// 0x02800000

#define SYSM_ARM9_LOAD_MMEM_LAST_ADDR		( SYSM_ADDR_TOP - 0x80000 )					// SYSMがロード可能なNITROカード初期ブートコードのメインメモリ最終アドレス
#define SYSM_ARM7_LOAD_MMEM_LAST_ADDR		( SYSM_ADDR_BOTTOM )						// SYSMがロード可能なNITROカード初期ブートコードのメインメモリ最終アドレス
#define SYSM_ARM7_LOAD_WRAM_LAST_ADDR		( RETURN_FROM_MAIN_ARM7_FUNCP & ~0x0fff )	// SYSMがロード可能なNITROカード初期ブートコードのメインメモリ最終アドレス
#define SYSM_ARM7_LOAD_BUF_ADDR				( SYSM_ADDR_TOP - 0x40000 )					// SYSMがNITROカードARM7初期ブートコードのロードを行う際のロードバッファアドレス
#define SYSM_ARM7_LOAD_BUF_SIZE				( SYSM_ADDR_TOP - SYSM_ARM7_LOAD_BUF_ADDR )	// SYSMのNITROカードARM7コードロードバッファサイズ

#define UNCOMP_TEMP_BUF						( SYSM_ARM7_LOAD_BUF_ADDR )					// 圧縮展開用データ一時格納バッファアドレス
#define UNCOMP_TEMP_BUF_SIZE				( SYSM_ARM7_LOAD_BUF_SIZE )					// 圧縮展開用データ一時格納バッファサイズ

#define NITRO_CARD_SECURE_SIZE				0x4000										// NITROカードのセキュア領域サイズ(16Kbytes)

#define SYSROM9_NINLOGO_ADR					0xffff0020					// ARM9システムROM内の任天堂ロゴ格納アドレス
#define AGB_CARTRIDGE_NIN_LOGO_DATA			(HW_CTRDG_ROM + 4)			// AGBカートリッジのNintendoロゴデータ格納アドレス


#ifdef __cplusplus
}
#endif

#endif		// __MMAP_H__

