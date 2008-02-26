/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.c

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
#include <sysmenu.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
typedef struct BannerCheckParam {
	u8		*pSrc;
	u32		size;
}BannerCheckParam;

// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
static BOOL SYSMi_CheckBannerFile( TWLBannerFile *pBanner );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// ============================================================================
//
// バナー
//
// ============================================================================

// カードバナーリード（※NTR-IPL2仕様）
BOOL SYSMi_ReadCardBannerFile( u32 bannerOffset, TWLBannerFile *pBanner )
{
#pragma unused(bannerOffset)
	BOOL isRead;
	if( SYSMi_GetWork()->flags.hotsw.isValidCardBanner ) {
		DC_InvalidateRange( (void *)SYSM_CARD_BANNER_BUF, 0x3000 );
		MI_CpuCopyFast( (void *)SYSM_CARD_BANNER_BUF, pBanner, sizeof(TWLBannerFile) );
	}
	isRead = SYSMi_CheckBannerFile( pBanner );
	
	if( !isRead ) {
		MI_CpuClearFast( pBanner, sizeof(TWLBannerFile) );
	}
	return isRead;
/*
	// ※カードライブラリでは、スロットAからのリードなら問題ないが、スロットBからは読めないのでとりあえず使わない
	BOOL isRead;
	u16 id = (u16)OS_GetLockID();
	
	// ROMカードからのバナーデータのリード
	DC_FlushRange( pBanner, sizeof(TWLBannerFile) );
	CARD_LockRom( id );
	CARD_ReadRom( 4, (void *)bannerOffset, pBanner, sizeof(TWLBannerFile) );
	CARD_UnlockRom( id );
	OS_ReleaseLockID( id );
	
	isRead = SYSMi_CheckBannerFile( (TWLBannerFile *)pBanner );
	
	if( !isRead ) {
		MI_CpuClearFast( pBanner, sizeof(TWLBannerFile) );
	}
	return isRead;
*/
}


	// バナーデータの正誤チェック
static BOOL SYSMi_CheckBannerFile( TWLBannerFile *pBanner )
{
	int i;
	BOOL retval = TRUE;
	u16 calc_crc = 0xffff;
	u16 *pHeaderCRC = (u16 *)&pBanner->h.crc16_v1;
	BannerCheckParam bannerCheckList[ BANNER_VER_NTR_MAX ];
	BannerCheckParam *pChk = &bannerCheckList[ 0 ];
	
	// NTR互換部分は標準でチェック
	bannerCheckList[ 0 ].pSrc = (u8 *)&( pBanner->v1 );
	bannerCheckList[ 0 ].size = sizeof( BannerFileV1 );
	bannerCheckList[ 1 ].pSrc = (u8 *)&( pBanner->v2 );
	bannerCheckList[ 1 ].size = sizeof( BannerFileV2 );
	bannerCheckList[ 2 ].pSrc = (u8 *)&( pBanner->v3 );
	bannerCheckList[ 2 ].size = sizeof( BannerFileV3 );
	
	for( i = 0; i < BANNER_VER_NTR_MAX; i++ ) {
		if( i < pBanner->h.version ) {
			calc_crc = SVC_GetCRC16( calc_crc, pChk->pSrc, pChk->size );
			if( calc_crc != *pHeaderCRC++ ) {
				retval = FALSE;
				break;
			}
		}else {
			MI_CpuClear16( pChk->pSrc, pChk->size );
		}
		pChk++;
	}
	
	// TWLバナーなら、バナーアニメ部もチェック
	if( pBanner->h.platform == BANNER_PLATFORM_TWL ) {
		if( pBanner->h.crc16_anime != SVC_GetCRC16( 0xffff, &pBanner->anime, sizeof(BannerAnime) ) ) {
			retval = FALSE;
		}
	}
	return retval;
}

