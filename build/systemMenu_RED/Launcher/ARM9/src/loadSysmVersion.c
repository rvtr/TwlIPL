/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     loadSysmVersion.c

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
#include "launcher.h"
#include "misc.h"
#include "loadSysmVersion.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------
#define VER_TITLEID					0x0003000F484E5641 //HNVA

#define VERSION_DATA_SIGN_SIZE		128
#define VERSION_DATA_BODY_SIZE		32
#define VERSION_DATA_PADDING_SIZE	(VERSION_DATA_BODY_SIZE - 4 - 4)

typedef struct VersionData
{
	u8 rsa_sign[VERSION_DATA_SIGN_SIZE];
	union
	{
		u8 body[VERSION_DATA_BODY_SIZE];
		struct
		{
			u32 timestamp;
			u32 version;
			u8 res[VERSION_DATA_PADDING_SIZE];
		};
	};
}
VersionData;

// function's prototype-------------------------------------------------------

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static u32 s_version = 0;
// const data------------------------------------------------------------------


// ============================================================================
// バージョン
// ============================================================================
BOOL LoadSysmVersion( void )
{
	char path[256];
	VersionData vd;
	FSFile file[1];
	BOOL bSuccess;
	s32 len;
	
	// ファイル読み込み
	NAM_GetTitleBootContentPathFast(path, VER_TITLEID);
	
	FS_InitFile( file );
	bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);
	
	if( ! bSuccess )
	{
OS_TPrintf("LoadSysmVersion failed: cant open file\n");
		(void)FS_CloseFile(file);
		return FALSE;
	}

	len = FS_ReadFile(file, &vd, sizeof(vd));
	if( len != sizeof(vd) )
	{
OS_TPrintf("LoadSysmVersion failed: read file error!\n");
		(void)FS_CloseFile(file);
		return FALSE;
	}
	
	(void)FS_CloseFile(file);
	
	// 検証
	// [TODO:]署名処理
	
	s_version = vd.version;
	if( vd.timestamp > 0 ) OS_TPrintf( "VersionData timestamp : %08x\n", vd.timestamp );
	
	return TRUE;
}

u32 GetSysmVersion( void )
{
	return s_version;
}

