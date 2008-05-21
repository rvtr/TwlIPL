/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     banner.c

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

#ifndef SYSM_BANNER_H_
#define SYSM_BANNER_H_

#include <twl/types.h>
#include <twl/os/common/banner.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------
// global variable------------------------------------------------------
// function-------------------------------------------------------------

#ifdef SDK_ARM9

// カードからのバナーリード
BOOL BANNER_ReadBannerFromCARD( u32 bannerOffset, TWLBannerFile *pDst );

// NANDからのバナーリード
BOOL BANNER_ReadBannerFromNAND( OSTitleId titleID, TWLBannerFile *pDst );

// バナーのフォーマットが正しいかチェック（NTRバナー、TWLバナーのどちらでもOK）
BOOL BANNER_CheckBanner( TWLBannerFile *pBanner );

// サブバナーチェック
BOOL BANNER_CheckSubBanner( TWLSubBannerFile *pBanner );

#endif //SDK_ARM9


#ifdef __cplusplus
} /* extern "C" */
#endif

/* SYSM_BANNER_H_ */
#endif
