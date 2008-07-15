/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     setWireless.c

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
#include <twl/na.h>
#include <nitro/fs/sysarea.h>
#include "getSysMenuVersion.h"

// テスト表示
#if 1
#define DEBUGPRINT              OS_TPrintf
#else
#define DEBUGPRINT(...)         ((void)0)
#endif


// define data----------------------------------
#define FS_VERSION_FILE					"verinfo:/version.bin"
#define FS_EULA_URL_FILE				"verinfo:/eula_url.bin"
#define FS_NUP_HOSTNAME_FILE			"verinfo:/nup_host.bin"
#define FS_TIMESTAMP_FILE				"verinfo:/time_stamp.bin"

typedef struct SystemMenuVersion {
	u16		major;
	u16		minor;
	u16		str[ TWL_SYSMENU_VER_STR_LEN / sizeof(u16) ];
}SystemMenuVersion;

// extern data----------------------------------
// function's prototype-------------------------
// static variable------------------------------
static BOOL s_isRead = FALSE;
static SystemMenuVersion s_version;
static u8 s_eulaURL[ TWL_EULA_URL_LEN ];
static u8 s_nupHostName[ TWL_NUP_HOSTNAME_LEN ];
static u32 s_timestamp;

// const data-----------------------------------


// システムメニューバージョンのリード
BOOL ReadSystemMenuVersionInfo( void *pWork, u32 workSize )
{
	BOOL retval = TRUE;
	
	// 既にリード実行済みなら何もせず終了
	if( s_isRead ) {
		return TRUE;
	}
	s_isRead = TRUE;
	
	// SystemMenuVersionのマウント（認証含む）
	if( !NA_LoadVersionInfoArchive( pWork, workSize ) ) {
		return FALSE;
	}
	
	// ついでにユーザー領域MAXサイズも読み込んでおく
	(void)FSi_GetTotalUserAreaSize();
	
	// バージョンの読み出し
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if (!FS_OpenFileEx(&file, FS_VERSION_FILE, FS_FILEMODE_R)) {
            return FALSE;
        }
		
        len = FS_ReadFile(&file, &s_version, (s32)sizeof(s_version));
        FS_CloseFile(&file);
		
        if (len < 0) {
            retval = FALSE;
        }
    }
	
	// EULA URLの読み出し
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if (!FS_OpenFileEx(&file, FS_EULA_URL_FILE, FS_FILEMODE_R)) {
            return FALSE;
        }
		
        len = FS_ReadFile(&file, s_eulaURL, (s32)sizeof(s_eulaURL));
        FS_CloseFile(&file);
		
        if (len < 0) {
            retval = FALSE;
        }
    }
	
	// NUP HOST NAME の読み出し
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if (!FS_OpenFileEx(&file, FS_NUP_HOSTNAME_FILE, FS_FILEMODE_R)) {
            return FALSE;
        }
		
        len = FS_ReadFile(&file, s_nupHostName, (s32)sizeof(s_nupHostName));
        FS_CloseFile(&file);
		
        if (len < 0) {
            retval = FALSE;
        }
    }
	
	// タイムスタンプ の読み出し
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if (!FS_OpenFileEx(&file, FS_TIMESTAMP_FILE, FS_FILEMODE_R)) {
            return FALSE;
        }
		
        len = FS_ReadFile(&file, &s_timestamp, (s32)sizeof(s_timestamp));
        FS_CloseFile(&file);
		
        if (len < 0) {
            retval = FALSE;
        }
    }
	
	// SystemMenuVersionのアンマウント
	if( !NA_UnloadVersionInfoArchive() ) {
		return FALSE;
	}
	
	return retval;
}


// バージョン文字列の取得
const u16 *GetSystemMenuVersionString( void )
{
	return s_isRead ? s_version.str : NULL;
}


// メジャーバージョンの取得
u16 GetSystemMenuMajorVersion( void )
{
	return s_isRead ? s_version.major : (u16)65535;
}


// マイナーバージョンの取得
u16 GetSystemMenuMinorVersion( void )
{
	return s_isRead ? s_version.minor : (u16)65535;
}


// EULA URLの取得
const u8 *GetEULA_URL( void )
{
	return s_isRead ? s_eulaURL : NULL;
}


// NUP HostNameの取得
const u8 *GetNUP_HostName( void )
{
	return s_isRead ? s_nupHostName : NULL;
}


// SystemMenuVersion情報のタイムスタンプの取得
u32 GetSystemMenuVersionTimeStamp( void )
{
	return s_isRead ? s_timestamp : (u32)0;
}
