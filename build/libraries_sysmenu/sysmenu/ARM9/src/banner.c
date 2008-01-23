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
static BOOL SYSMi_CheckBannerFile( NTRBannerFile *pBanner );

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
#ifndef DEBUG_USED_CARD_SLOT_B_
	// ※スロットAからのリードなら問題ないが、スロットBからは直接読めないので
	BOOL isRead;
	u16 id = (u16)OS_GetLockID();
	
	// ROMカードからのバナーデータのリード
	DC_FlushRange( pBanner, sizeof(TWLBannerFile) );
	CARD_LockRom( id );
	CARD_ReadRom( 4, (void *)bannerOffset, pBanner, sizeof(TWLBannerFile) );
	CARD_UnlockRom( id );
	OS_ReleaseLockID( id );
	
	isRead = SYSMi_CheckBannerFile( (NTRBannerFile *)pBanner );
	
	if( !isRead ) {
		MI_CpuClearFast( pBanner, sizeof(TWLBannerFile) );
	}
	return isRead;
#else
#pragma unused(bannerOffset)
	if( SYSMi_GetWork()->isValidCardBanner ) {
		DC_InvalidateRange( (void *)SYSM_CARD_BANNER_BUF, 0x3000 );
		MI_CpuCopyFast( (void *)SYSM_CARD_BANNER_BUF, pBanner, sizeof(TWLBannerFile) );
	}
	return (BOOL)SYSMi_GetWork()->isValidCardBanner;
#endif
}


	// バナーデータの正誤チェック
static BOOL SYSMi_CheckBannerFile( NTRBannerFile *pBanner )
{
	int i;
	BOOL retval = TRUE;
	u16 calc_crc = 0xffff;
	u16 *pHeaderCRC = (u16 *)&pBanner->h.crc16_v1;
	BannerCheckParam bannerCheckList[ NTR_BNR_VER_MAX ];
	BannerCheckParam *pChk = &bannerCheckList[ 0 ];
	
	bannerCheckList[ 0 ].pSrc = (u8 *)&( pBanner->v1 );
	bannerCheckList[ 0 ].size = sizeof( BannerFileV1 );
	bannerCheckList[ 1 ].pSrc = (u8 *)&( pBanner->v2 );
	bannerCheckList[ 1 ].size = sizeof( BannerFileV2 );
	bannerCheckList[ 2 ].pSrc = (u8 *)&( pBanner->v3 );
	bannerCheckList[ 2 ].size = sizeof( BannerFileV3 );
	
	for( i = 0; i < NTR_BNR_VER_MAX; i++ ) {
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
	
	return retval;
}

