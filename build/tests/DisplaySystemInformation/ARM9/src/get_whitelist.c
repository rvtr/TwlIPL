/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     get_whitelist.c

  Copyright **** Nintendo.  All rights reserved.

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
#include <sysmenu/dht/dht.h>
#include <es.h>
#include <estypes.h>

#include "viewSystemInfo.h"
#include "strResource.h"
#include "getInformation.h"
#include "util.h"
#include "misc.h"

#define WHITELIST_TITLEID 0x0003000f484e4841
#define DS_HASH_TABLE_SIZE  (256*1024)

void getWhitelistHash( void );
void getNumDHTEntry( void );
BOOL cmpHash( void *src1, void *src2 );

void getWhiteListInfo( void )
{
	OS_TPrintf("...Whitelist Information\n");
	
	getWhitelistHash();
	getNumDHTEntry();
}

void getWhitelistHash( void )
{
	u32 tmdSize=0, fileSize;
	ESTitleMeta *tmd;

	FSFile file;
	char filePath[NAM_PATH_LEN+1];
	u8 *fileBuf, digestBuf[MATH_SHA1_DIGEST_SIZE];

	
	// ESライブラリを使ってハッシュ値を取得する
	// 一回目の呼び出しでサイズを取得して二回目で値をもらう
	ES_GetTmd( WHITELIST_TITLEID, NULL, &tmdSize );
	tmd = (ESTitleMeta*) Alloc ( tmdSize );
	SDK_ASSERT( tmd );
	ES_GetTmd( WHITELIST_TITLEID , tmd, &tmdSize );
	
	if( tmd == NULL )
	{
		OS_TPrintf("getting tmd hash failed.\n" );
		return;
	}
	
	putBinary( (u8*)&tmd->contents[0].hash , MATH_SHA1_DIGEST_SIZE );
	
	// ホワイトリストのファイルの実態を引っ張ってくる
	FS_InitFile( &file );
	NAM_GetTitleBootContentPath( filePath , WHITELIST_TITLEID );

	OS_TPrintf("filepath : %s\n", filePath );
	
	if( ! FS_OpenFileEx( &file, filePath, FS_FILEMODE_R ) )
	{
		OS_TPrintf("whitelist info error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(filePath) );
		return;
	}
	
	fileSize = FS_GetFileLength( &file );
	
	fileBuf = (u8*) Alloc ( fileSize );
	SDK_ASSERT( fileBuf );
	
	if( fileSize != FS_ReadFile( &file, fileBuf, (s32)fileSize ) )
	{
		OS_TPrintf("whitelist info error: FS_ReadFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(filePath) );
		return;
	}
	
	MATH_CalcSHA1( digestBuf, fileBuf, fileSize );
	putBinary( digestBuf, MATH_SHA1_DIGEST_SIZE );
	
	gAllInfo[MENU_WHITE][WHITE_HASH].str.sjis = s_strCorrect [ cmpHash( digestBuf, &tmd->contents[0].hash ) ];
	
	Free( tmd );
	Free( fileBuf );

}

void getNumDHTEntry( void )
{
	DHTFile *dht;
	FSFile file;
	char filePath[NAM_PATH_LEN+1];
	
	dht = (DHTFile*) Alloc (DS_HASH_TABLE_SIZE);	
	FS_InitFile( &file );
	NAM_GetTitleBootContentPathFast( filePath , WHITELIST_TITLEID );
	
	OS_TPrintf("DHTEntry reading...\n");
	
	if( ! FS_OpenFileEx( &file, filePath, FS_FILEMODE_R ) )
	{
		OS_TPrintf("whitelist info error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(filePath) );
		return;
	}
	
/*	if( ! FS_SeekFile( &file, sizeof(ROM_Header), FS_SEEK_SET ) )
	{
		OS_TPrintf("whitelist info error: FS_SeekFile() failed. FSResult: %d\n", FS_GetArchiveResultCode(filePath) );
		return;
	}
*/	
	if( ! DHT_PrepareDatabase( dht, &file ) )
	{
		OS_TPrintf("whitelist info error: PrepareDatabase() failed.\n" );
		return;
	}
	
	DC_FlushRange( dht, DHT_GetDatabaseLength( dht ));
	OS_TPrintf(" dht.header.nums : %d\n", dht->header.nums);

	gAllInfo[MENU_WHITE][WHITE_NUM].iValue = (int)dht->header.nums;
	gAllInfo[MENU_WHITE][WHITE_NUM].isNumData = TRUE;
	
	Free(dht);
}

// 二つのハッシュを比較して一致してたら返す
// でもsrc2はビッグエンディアンやねん
BOOL cmpHash( void *src1, void *src2 )
{
	u8 *u1 = (u8*)src1;
	u8 *u2 = (u8*)src2;
	
	return MI_CpuComp8( u1, u2, MATH_SHA1_DIGEST_SIZE ) == 0 ;

}