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

#ifndef SDK_ASM
#include <twl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------

// SystemMenu自身のマップ情報定義
#define SYSM_OWN_ARM9_MMEM_ADDR				SYSM_TWL_ARM9_LTD_LOAD_MMEM_END
#define SYSM_OWN_ARM9_MMEM_ADDR_END			SYSM_TWL_ARM7_LTD_LOAD_MMEM
#define SYSM_OWN_ARM9_MMEM_CODE_ADDR		(SYSM_OWN_ARM9_MMEM_ADDR + SYSM_DMA_BUFOVER_BARRIER_SIZE)
#define SYSM_OWN_ARM7_MMEM_ADDR				0x02280000
#define SYSM_OWN_ARM7_MMEM_ADDR_END			0x02380000
#define SYSM_OWN_ARM7_MMEM_CODE_ADDR		(SYSM_OWN_ARM7_MMEM_ADDR + SYSM_DMA_BUFOVER_BARRIER_SIZE)
extern  u32 SDK_AUTOLOAD_WRAM_START[];
#ifndef SDK_ASM
#define SYSM_OWN_ARM7_WRAM_ADDR				((u32)SDK_AUTOLOAD_WRAM_START)
#else
#define SYSM_OWN_ARM7_WRAM_ADDR				SDK_AUTOLOAD_WRAM_START
#endif
#define SYSM_OWN_ARM7_WRAM_ADDR_END			0x0380f000

#define SYSROM9_NINLOGO_ADR					0xffff0020					// ARM9システムROM内の任天堂ロゴ格納アドレス

#define SYSM_DMA_BUFOVER_BARRIER_SIZE		0x0200

// カードアプリ格納バッファ（ROMヘッダバッファはNANDアプリでも使用）
#define SYSM_APP_ROM_HEADER_SIZE			0x1000
#define SYSM_APP_ROM_HEADER_BUF				( 0x02fe6000 )
#define SYSM_APP_ROM_HEADER_BUF_END			( SYSM_APP_ROM_HEADER_BUF + SYSM_APP_ROM_HEADER_SIZE )
#define SYSM_CARD_ROM_HEADER_BAK			( SYSM_APP_ROM_HEADER_BUF_END )
#define SYSM_CARD_ROM_HEADER_BAK_END		( SYSM_CARD_ROM_HEADER_BAK + SYSM_APP_ROM_HEADER_SIZE )
#define	SYSM_CARD_NTR_SECURE_BUF			( SYSM_CARD_ROM_HEADER_BAK_END )
#define	SYSM_CARD_NTR_SECURE_BUF_END		( SYSM_CARD_NTR_SECURE_BUF + SECURE_AREA_SIZE )
#define	SYSM_CARD_TWL_SECURE_BUF			( SYSM_CARD_NTR_SECURE_BUF_END )
#define	SYSM_CARD_TWL_SECURE_BUF_END		( SYSM_CARD_TWL_SECURE_BUF + SECURE_AREA_SIZE )
#define SYSM_CARD_BANNER_BUF				( SYSM_CARD_TWL_SECURE_BUF_END )
#define SYSM_CARD_BANNER_BUF_END			( SYSM_CARD_BANNER_BUF + 0x3000 )

#define SYSM_NTR_ROM_HEADER_BUF				0x023ffe00
#define SYSM_DBG_NTR_ROM_HEADER_BUF			0x027ffe00
#define SYSM_DBG_NTR_SYSTEM_BUF				0x027ff000

// ※アプリをWRAMに直接配置してブートしようとすると、SystemMenuのコードとぶつかっていろいろややこしい状態になるので、検討が必要

// アプリロード可能領域のマップ情報定義
#define SYSM_NTR_ARM9_LOAD_MMEM				0x02000000					// ロード可能なARM9 static MMEM アドレス
#define SYSM_NTR_ARM9_LOAD_MMEM_END			0x02280000					// ロード可能なARM9 static MMEM 最終アドレス
#define SYSM_NTR_ARM7_LOAD_MMEM				0x02380000					// ロード可能なARM7 static MMEM アドレス
#define SYSM_NTR_ARM7_LOAD_MMEM_END			0x023c0000					// ロード可能なARM7 static MMEM 最終アドレス
#define SYSM_NTR_ARM7_LOAD_WRAM				0x037f8000					// ロード可能なARM7 static WRAM アドレス
#define SYSM_NTR_ARM7_LOAD_WRAM_END			0x0380f000					// ロード可能なARM7 static WRAM 最終アドレス

#define SYSM_TWL_ARM9_LOAD_MMEM				HW_MAIN_MEM_PARAMETER_BUF_END	// ロード可能なARM9 static MMEM アドレス      <- SDKでズレたので要修正
#define SYSM_TWL_ARM9_LOAD_MMEM_END			SYSM_NTR_ARM9_LOAD_MMEM_END	// ロード可能なARM9 static MMEM 最終アドレス
#define SYSM_TWL_ARM7_LOAD_MMEM				SYSM_NTR_ARM7_LOAD_MMEM		// ロード可能なARM7 static MMEM アドレス
#define SYSM_TWL_ARM7_LOAD_MMEM_END			SYSM_NTR_ARM7_LOAD_MMEM_END	// ロード可能なARM7 static MMEM 最終アドレス
#define SYSM_TWL_ARM7_LOAD_WRAM				SYSM_NTR_ARM7_LOAD_WRAM		// ロード可能なARM7 static WRAM アドレス
#define SYSM_TWL_ARM7_LOAD_WRAM_END			SYSM_NTR_ARM7_LOAD_WRAM_END	// ロード可能なARM7 static WRAM 最終アドレス

#define SYSM_TWL_ARM9_LTD_LOAD_MMEM			0x02400000					// ロード可能なARM9 LTD static MMEM アドレス
#define SYSM_TWL_ARM9_LTD_LOAD_MMEM_END		0x02680000					// ロード可能なARM9 LTD static MMEM 最終アドレス
#define SYSM_TWL_ARM7_LTD_LOAD_MMEM			0x02e80000					// ロード可能なARM7 LTD static MMEM アドレス
#define SYSM_TWL_ARM7_LTD_LOAD_MMEM_END		0x02f88000					// ロード可能なARM7 LTD static MMEM 最終アドレス
#define SYSM_TWL_ARM7_LTD_LOAD_WRAM			0x037c0000					// ロード可能なARM7 LTD static WRAM アドレス
#define SYSM_TWL_ARM7_LTD_LOAD_WRAM_END		SYSM_NTR_ARM7_LOAD_WRAM_END	// ロード可能なARM7 LTD static WRAM 最終アドレス
#define SYSM_TWL_ARM7_LTD_HYB_LOAD_WRAM		0x037f8000					// ロード可能なARM7 LTD static WRAM アドレス
#define SYSM_TWL_ARM7_LTD_HYB_LOAD_WRAM_END	SYSM_NTR_ARM7_LOAD_WRAM_END	// ロード可能なARM7 LTD static WRAM 最終アドレス

// マウント情報一時バッファ
#define SYSM_TWL_MOUNT_INFO_TMP_BUFFER		0x02ff3400					// マウント情報の一時バッファ

#ifdef __cplusplus
}
#endif

#endif // _SYSMENU_MMAP_H_

