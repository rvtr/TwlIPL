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
#include <twl/nam.h>
#include <sysmenu/banner.h>

// define data-----------------------------------------------------------------
typedef struct BannerCheckParam {
	u8		*pSrc;
	u32		size;
}BannerCheckParam;

#define MEASURE_BANNER_LOAD_TIME     0

// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// ============================================================================
//
// バナー
//
// ============================================================================


// カードアプリバナーリード
BOOL BANNER_ReadBannerFromCARD( u32 bannerOffset, TWLBannerFile *pBanner )
{
	BOOL isRead;
	u16 id = (u16)OS_GetLockID();
	
	// ROMカードからのバナーデータのリード
	DC_FlushRange( pBanner, sizeof(TWLBannerFile) );
	CARD_LockRom( id );
	CARD_ReadRom( 4, (void *)bannerOffset, pBanner, sizeof(TWLBannerFile) );
	CARD_UnlockRom( id );
	OS_ReleaseLockID( id );
	
	isRead = BANNER_CheckBanner( (TWLBannerFile *)pBanner );
	
	if( !isRead ) {
		MI_CpuClearFast( pBanner, sizeof(TWLBannerFile) );
	}
	return isRead;
}


// NANDアプリバナーリード
BOOL BANNER_ReadBannerFromNAND( OSTitleId titleID, TWLBannerFile *pDst, TitleListMakerInfo *pTitleListMakerInfo )
{
#define PATH_LENGTH		1024

#if (MEASURE_BANNER_LOAD_TIME == 1)
	OSTick start;
#endif

	FSFile  file[1];
	BOOL bSuccess;
	char path[PATH_LENGTH];
	s32 readLen;
	u32 offset;
	ROM_Header_Short hs;
	
	FS_InitFile(file);
	
#if (MEASURE_BANNER_LOAD_TIME == 1)
	start = OS_GetTick();
#endif

	readLen = NAM_GetTitleBootContentPathFast( path, titleID );
	
#if (MEASURE_BANNER_LOAD_TIME == 1)
	OS_TPrintf( "NAM_GetTitleBootContentPath : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
#endif
	
	// ファイルパスを取得
	if(readLen != NAM_OK){
		OS_TPrintf("NAM_GetTitleBootContentPath failed %lld,%d\n", titleID, readLen );
		return FALSE;
	}
	
#if (MEASURE_BANNER_LOAD_TIME == 1)
	start = OS_GetTick();
#endif

	// ファイルオープン
	bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);
	if( ! bSuccess )
	{
		OS_TPrintf("BANNER_GetNandTitleList failed: cant open file %s\n",path);
		return FALSE;
	}
	
#if (MEASURE_BANNER_LOAD_TIME == 1)
	OS_TPrintf( "OpenFileEX : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	start = OS_GetTick();
#endif
	
	readLen = FS_ReadFile(file, &hs, sizeof(hs));
	if( readLen != sizeof(hs) )
	{
		OS_TPrintf("BANNER_GetNandTitleList failed: cant read file\n");
		FS_CloseFile(file);
		return FALSE;
	}
	
	offset = hs.banner_offset;

#if (MEASURE_BANNER_LOAD_TIME == 1)
	OS_TPrintf( "FS_ReadFile header : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
#endif
	
	// バナーが存在する場合のみリード
	if( offset ) {
		
#if (MEASURE_BANNER_LOAD_TIME == 1)
		start = OS_GetTick();
#endif

		bSuccess = FS_SeekFile(file, (s32)offset, FS_SEEK_SET);
		if( ! bSuccess )
		{
			OS_TPrintf("BANNER_GetNandTitleList failed: cant seek file(offset)\n");
			FS_CloseFile(file);
			return FALSE;
		}

#if (MEASURE_BANNER_LOAD_TIME == 1)
		OS_TPrintf( "FS_SeekFile banner: %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
		start = OS_GetTick();
#endif
		
		readLen = FS_ReadFile( file, pDst, (s32)sizeof(TWLBannerFile) );
		if( readLen != (s32)sizeof(TWLBannerFile) )
		{
			OS_TPrintf("BANNER_GetNandTitleList failed: cant read file2\n");
			FS_CloseFile(file);
			return FALSE;
		}

#if (MEASURE_BANNER_LOAD_TIME == 1)
		OS_TPrintf( "FS_ReadFile banner: %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
		start = OS_GetTick();
#endif
		
		if( !BANNER_CheckBanner( pDst ) )
		{
			// 正当性チェック失敗の場合はバッファクリア
			MI_CpuClearFast( pDst, sizeof(TWLBannerFile) );
		}

#if (MEASURE_BANNER_LOAD_TIME == 1)
		OS_TPrintf( "check banner: %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
#endif

	}else {
		// バナーが存在しない場合はバッファクリア
		MI_CpuClearFast( pDst, sizeof(TWLBannerFile) );
	}
	
#if (MEASURE_BANNER_LOAD_TIME == 1)
	start = OS_GetTick();
#endif

	FS_CloseFile(file);
	
#if (MEASURE_BANNER_LOAD_TIME == 1)
	OS_TPrintf( "close file : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	start = OS_GetTick();
#endif
	
	// サブバナーファイルを読み込んでみる
	if(NAM_OK == NAM_GetTitleBannerFilePath( path, titleID ))
	{

#if (MEASURE_BANNER_LOAD_TIME == 1)
		OS_TPrintf( "NAM_GetTitleBannerFilePath : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
		start = OS_GetTick();
#endif
		if( FS_OpenFileEx(file, path, FS_FILEMODE_R) )
		{
			TWLSubBannerFile subBanner;
			readLen = FS_ReadFile(file, &subBanner, sizeof(TWLSubBannerFile));
			FS_CloseFile(file);
			if( readLen == sizeof(TWLSubBannerFile) )
			{
				// 読み込みには成功したので正当性チェック
				if( BANNER_CheckSubBanner(&subBanner) )
				{
					// 成功したのでコピーする
					pDst->h = subBanner.h;
					pDst->anime = subBanner.anime;
//					OS_TPrintf("BANNER_ReadBanner_NAND : subbanner check succeed. id=%.16x\n", titleID);
				}else
				{
//					OS_TPrintf("BANNER_ReadBanner_NAND : subbanner check failed. id=%.16x\n", titleID);
				}
			}else
			{
				OS_TPrintf("BANNER_ReadBanner_NAND : subbanner read failed. id=%.16x\n", titleID);
			}
		}

#if (MEASURE_BANNER_LOAD_TIME == 1)
		OS_TPrintf( "open-read-close-check subbanner : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
#endif

	}
	
	// タイトルリスト用情報の生成
	if(!SYSM_MakeTitleListMakerInfoFromHeader( pTitleListMakerInfo, &hs ))
	{
		return FALSE;
	}
	
	return TRUE;
}


	// バナーデータの正誤チェック
BOOL BANNER_CheckBanner( TWLBannerFile *pBanner )
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


// サブバナーデータの正誤チェック
BOOL BANNER_CheckSubBanner( TWLSubBannerFile *pBanner )
{
	BOOL retval = TRUE;
	
	// アニメ部チェック
	if( pBanner->h.crc16_anime != SVC_GetCRC16( 0xffff, &pBanner->anime, sizeof(BannerAnime) ) ) {
		retval = FALSE;
	}
	return retval;
}

