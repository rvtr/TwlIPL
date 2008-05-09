/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     TWLBanner.c

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

#ifndef TWL_BANNER_H_
#define TWL_BANNER_H_

#include <twl/types.h>
#include <sysmenu/banner/common/NTRBanner.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TWL_BNR_VER_MIN					0x80
#define TWL_BNR_VER_MAX					0x80
#define	TWL_BNR_ICON_NUM				8
#define TWL_BNR_ICON_DISP_FRAME_MAX		60
#define TWL_BNR_HEADER_RSV_SIZE			( BNR_HEADER_RSV_SIZE - sizeof(BannerAnimeCtrl) )


typedef enum BannerAnimeLoopType {
	BNR_ANIME_LOOP_NORMAL = 1,
	BNR_ANIME_LOOP_BACK   = 2
}BannerAnimeLoopType;


typedef struct BannerAnimeCtrl {
	BannerAnimeLoopType	loopType;
	u8				dispFrameCount[ TWL_BNR_ICON_NUM ];
}BannerAnimeCtrl;


typedef struct TWLBannerHeader {
	u8				version;
	u8				reserved_A;
	u16				crc16_v1;
	u16				crc16_v2;
	u16				crc16_v3;
	u8				reserved_B[ TWL_BNR_HEADER_RSV_SIZE ];
	BannerAnimeCtrl	anime;
}TWLBannerHeader;


typedef struct BannerIcon {
	u8				image[ BNR_IMAGE_SIZE ];
	GXRgba			pltt [ BNR_PLTT_NUM ];
}BannerIcon;

typedef struct BannerCommentRsv {
	u16				comment[ BNR_LANG_MAX - NTR_BNR_LANG_NUM ][ BNR_LANG_LENGTH ];
}BannerCommentRsv;

typedef struct TWLBannerFile {
	TWLBannerHeader	h;
	BannerFileV1	v1;
	BannerFileV2	v2;
	BannerFileV3	v3;
	BannerCommentRsv rsv;
	BannerIcon		icon[ TWL_BNR_ICON_NUM - 1 ];	// êÊì™ÇÃÇPå¬ÇÕBannerFileV1ì‡Ç…Ç†ÇÈÅB
}TWLBannerFile;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_BANNER_H_ */
#endif
