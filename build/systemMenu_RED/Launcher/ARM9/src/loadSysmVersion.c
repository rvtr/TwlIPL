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
#define VER_TITLEID					0x0003000F484E4C41 //HNLA

#define VERSION_DATA_SIGN_SIZE		128
#define VERSION_DATA_HEADER_SIZE	96
#define VERSION_DATA_PADDING1_SIZE	12
#define VERSION_DATA_PADDING2_SIZE	44

typedef struct VersionDataHeader
{
	u8 rsa_sign[VERSION_DATA_SIGN_SIZE];
	union
	{
		u8 header[VERSION_DATA_HEADER_SIZE];
		struct
		{
			u32 timestamp;
			u32 version;
			u32 userAreaSize;
			u32 data1Offset;
			u32 data1Size;
			u8 padding1[VERSION_DATA_PADDING1_SIZE];
			u8 data1Digest;
			u8 padding2[VERSION_DATA_PADDING2_SIZE];
		};
	};
}
VersionDataHeader;

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
	VersionDataHeader vdh;
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

	len = FS_ReadFile(file, &vdh, sizeof(vdh));
	if( len != sizeof(vdh) )
	{
OS_TPrintf("LoadSysmVersion failed: read file error!\n");
		(void)FS_CloseFile(file);
		return FALSE;
	}
	
	(void)FS_CloseFile(file);
	
	// 検証
	// [TODO:]署名処理
	
	s_version = vdh.version;
	if( vdh.timestamp > 0 ) OS_TPrintf( "VersionData timestamp : %08x\n", vdh.timestamp );
	
	return TRUE;
}

u32 GetSysmVersion( void )
{
	return s_version;
}

u16 GetSysmMajorVersion( void )
{
	return (u16)( ( 0xffff0000 & s_version ) >> 16 );
}

u16 GetSysmMinorVersion( void )
{
	return (u16)( 0xffff & s_version );
}

