/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     sharedFont.c

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
#include "fs_wram.h"

// compile switch--------------------------------------------------------------
//#define HASH_CHECK_OFF
#define USE_FONT_WRAM_LOAD
// extern data-----------------------------------------------------------------
// define data-----------------------------------------------------------------
#define WRAM_SLOT_FOR_FS					4
#define WRAM_SIZE_FOR_FS					MI_WRAM_SIZE_96KB

#define SHARED_FONT_TABLE_PATH				"nand:/sys/TWLFontTable.dat"
#define SHARED_FONT_FILE_NAME_LENGTH		0x20
#define SHARED_FONT_SIGN_SIZE				0x80

typedef struct SFONTHeader {
	u32 timestamp;
	u16 fontNum;
	u8  pad[ 6 ];
	u8  digest[ SVC_SHA1_DIGEST_SIZE ];
}SFONTHeader;

typedef struct SFONTInfo {
	u8		fileName[ SHARED_FONT_FILE_NAME_LENGTH ];
	u8		pad[ 4 ];
	u32		offset;
	u32		length;
	u8		digest[ SVC_SHA1_DIGEST_SIZE ];
}SFONTInfo;

typedef struct SFONTWork {
	SFONTHeader	header;
	SFONTInfo	*pInfoTable;
}SFONTWork;

typedef struct CalcSHA1CallbackArg
{
	SVCSHA1Context	ctx;
	u32				hash_length;
} CalcSHA1CallbackArg;

// function's prototype-------------------------------------------------------
static void CalcSHA1Callback(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg);

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static SFONTWork s_work;
static BOOL s_isInitialized = FALSE;

// const data------------------------------------------------------------------
static const u8 s_sharedFontPubKey[] = {
	0x9f, 0x80, 0xbc, 0x5f, 0xb6, 0xb6, 0x1d, 0x2a, 0x46, 0x02, 0x52, 0x64, 0xb2, 0xa3, 0x86, 0xce,
	0xe6, 0x54, 0xd3, 0xa9, 0x70, 0x5b, 0xe3, 0xc2, 0x10, 0xa9, 0xb5, 0x2f, 0x38, 0xc5, 0x51, 0xfb,
	0xb5, 0xd1, 0x80, 0xfd, 0xff, 0x20, 0x65, 0xc1, 0x28, 0x4d, 0x56, 0xbe, 0xfb, 0xbd, 0x3f, 0xe4,
	0xba, 0xf7, 0x9c, 0x3a, 0x33, 0x74, 0x74, 0x9d, 0xdb, 0xdd, 0x9e, 0x86, 0x05, 0x2c, 0xad, 0xfc,
	0x93, 0xfa, 0xfb, 0x08, 0xea, 0x71, 0x18, 0x36, 0xc5, 0xdc, 0x4c, 0x06, 0x34, 0x57, 0xa7, 0x8f,
	0x4e, 0x82, 0xf7, 0xb3, 0xe2, 0x9c, 0xe4, 0x72, 0xe3, 0xdc, 0x60, 0xaf, 0xcc, 0x18, 0xe2, 0xd4,
	0xef, 0xd2, 0x76, 0x47, 0x31, 0xe6, 0x14, 0x0e, 0x1d, 0x26, 0xb5, 0x85, 0x97, 0xbc, 0xc6, 0xb6,
	0xd8, 0xe7, 0x69, 0x2d, 0x2c, 0x26, 0xfb, 0x5f, 0x70, 0x9e, 0x19, 0x9c, 0x6b, 0x02, 0x6d, 0x97
};

// 共有フォント初期化
BOOL SFONT_Init( void )
{
	FSFile file[1];
	u8 signature[ SHARED_FONT_SIGN_SIZE ];
	
	if( s_isInitialized ) {
		return TRUE;
	}
	
	MI_CpuClear32( &s_work, sizeof(s_work) );
	
	if( !FS_OpenFileEx( file, SHARED_FONT_TABLE_PATH, FS_FILEMODE_R ) ) {
		return FALSE;
	}
	
	// 署名リード
	if( FS_ReadFile( file, signature, SHARED_FONT_SIGN_SIZE ) != SHARED_FONT_SIGN_SIZE ){
		goto ERROR;
	}
	
	// ヘッダリード
	if( FS_ReadFile( file, &s_work.header, sizeof(SFONTHeader) ) != sizeof(SFONTHeader) ){
		goto ERROR;
	}
	
	FS_CloseFile( file );
	
#ifndef HASH_CHECK_OFF
	// ヘッダ署名チェック
	{
		u8 calc_digest[ SVC_SHA1_DIGEST_SIZE ];
		u8 sign_digest[ SVC_SHA1_DIGEST_SIZE ];
	    static u32 heap[ 4096 / sizeof(u32) ];
		
	    SVCSignHeapContext acmemoryPool;
	    SVC_CalcSHA1( calc_digest, &s_work.header, sizeof(SFONTHeader) );
	    SVC_InitSignHeap( &acmemoryPool, heap, 4096 );
		if( !SVC_DecryptSign( &acmemoryPool, sign_digest, signature, s_sharedFontPubKey ) ) {
			return FALSE;
		}
		if( !SVC_CompareSHA1( calc_digest, sign_digest ) ) {
			return FALSE;
		}
	}
#endif
	
#ifdef USE_FONT_WRAM_LOAD
	// WRAM利用Read関数の準備、WRAMCの後半だけ解放しておく
	FS_InitWramTransfer(3);
	MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
	MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
	MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
	MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
	MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
	MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
#endif // USE_FONT_WRAM_LOAD
	
	s_isInitialized = TRUE;
	return TRUE;
	
ERROR:
	FS_CloseFile( file );
	return FALSE;
}


// 共有フォント　テーブルサイズ取得
int SFONT_GetInfoTableSize( void )
{
	if( s_isInitialized ) {
		return (int)( s_work.header.fontNum * sizeof(SFONTInfo) );
	}else {
		return -1;
	}
}


// 共有フォント　テーブルロード
BOOL SFONT_LoadInfoTable( void *pBuffer )
{
	FSFile file[1];
	u32 tableLen = sizeof(SFONTInfo) * s_work.header.fontNum;
	
	if( ( !s_isInitialized ) ||
		( s_work.header.fontNum == 0 ) ||
		( pBuffer == NULL ) ||
		( s_work.pInfoTable ) ) {
		return FALSE;
	}
	
	// フォントInfoテーブルリード
	if( !FS_OpenFileEx( file, SHARED_FONT_TABLE_PATH, FS_FILEMODE_R ) ) {
		return FALSE;
	}
	if( !FS_SeekFile( file, SHARED_FONT_SIGN_SIZE + sizeof(SFONTHeader), FS_SEEK_SET ) ){
		goto ERROR;
	}
	if( FS_ReadFile( file, pBuffer, (int)tableLen ) != tableLen ){
		goto ERROR;
	}
	FS_CloseFile( file );
	
#ifndef HASH_CHECK_OFF
	// フォントInfoテーブル　ハッシュチェック
	{
		u8 calc_digest[ SVC_SHA1_DIGEST_SIZE ];
		SVC_CalcSHA1( calc_digest, pBuffer, tableLen );
		if( !SVC_CompareSHA1( calc_digest, s_work.header.digest ) ) {
			return FALSE;
		}
	}
#endif
	
	s_work.pInfoTable = pBuffer;
	return TRUE;
	
ERROR:
	FS_CloseFile( file );
	return FALSE;
}


// 共有フォント　フォントサイズ取得
int SFONT_GetFontSize( SFONT_Index index )
{
	if( ( s_isInitialized == NULL ) ||
		( s_work.pInfoTable == NULL ) ||
		( index >= s_work.header.fontNum ) ||
		( index >= SHARED_FONT_MAX ) ) {
		return -1;
	}
	
	return (int)s_work.pInfoTable[ index ].length;
}


// 共有フォント　フォントネーム取得
const u8 *SFONT_GetFontName( SFONT_Index index )
{
	if( ( s_isInitialized == NULL ) ||
		( s_work.pInfoTable == NULL ) ||
		( index >= s_work.header.fontNum ) ||
		( index >= SHARED_FONT_MAX ) ) {
		return NULL;
	}
	
	return s_work.pInfoTable[ index ].fileName;
}


// 共有フォント　タイムスタンプ取得
u32 SFONT_GetFontTimestamp( void )
{
	if( ( s_isInitialized == NULL ) ) {
		return 0;
	}
	return s_work.header.timestamp;
}


// 共有フォント　フォントロード
BOOL SFONT_LoadFont( SFONT_Index index, void *pBuffer )
{
	FSFile file[1];
	SFONTInfo *pInfo = &s_work.pInfoTable[ index ];
	u8 calc_digest[ SVC_SHA1_DIGEST_SIZE ];

	if( ( s_isInitialized == NULL ) ||
		( s_work.pInfoTable == NULL ) ||
		( index >= s_work.header.fontNum ) ||
		( index >= SHARED_FONT_MAX ) ) {
		return FALSE;
	}
	
	// フォント　リード
	if( !FS_OpenFileEx( file, SHARED_FONT_TABLE_PATH, FS_FILEMODE_R ) ) {
		return FALSE;
	}
	if( !FS_SeekFile( file, (int)pInfo->offset, FS_SEEK_SET ) ){
		goto ERROR;
	}
#ifdef USE_FONT_WRAM_LOAD
	{
		CalcSHA1CallbackArg arg;
        SVC_SHA1Init( &arg.ctx );
		arg.hash_length = pInfo->length;
		if( !FS_ReadFileViaWram( file, pBuffer, (s32)MATH_ROUNDUP( pInfo->length, 0x20 ), MI_WRAM_C,
								 WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS,
#ifndef HASH_CHECK_OFF
								 CalcSHA1Callback,
#else
								 NULL,
#endif // HASH_CHECK_OFF
								 &arg ) ) {
			goto ERROR;
		}
		SVC_SHA1GetHash( &arg.ctx, &calc_digest );
	}
#else
	if( FS_ReadFile( file, pBuffer, (int)pInfo->length ) != pInfo->length ){
		goto ERROR;
	}
#ifndef HASH_CHECK_OFF
    SVC_CalcSHA1( calc_digest, pBuffer, pInfo->length );
#endif // HASH_CHECK_OFF
#endif // USE_FONT_WRAM_LOAD
	
	FS_CloseFile( file );
	
#ifndef HASH_CHECK_OFF
	// フォント　ハッシュチェック
	if( !SVC_CompareSHA1( calc_digest, pInfo->digest ) ) {
		return FALSE;
	}
#endif // HASH_CHECK_OFF
	
	return TRUE;
	
ERROR:
	FS_CloseFile( file );
	return FALSE;
}


#ifdef USE_FONT_WRAM_LOAD
// FS-WRAM転送時のSHA1計算コールバック
static void CalcSHA1Callback(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg)
{
#pragma unused(orig_addr)
#pragma unused(wram)
#pragma unused(slot)
	CalcSHA1CallbackArg *cba = (CalcSHA1CallbackArg *)arg;
	u32 calc_len = ( cba->hash_length < len ? cba->hash_length : len );
	if( calc_len == 0 ) return;
	cba->hash_length -= calc_len;
	SVC_SHA1Update( &cba->ctx, addr, calc_len );
}
#endif // USE_FONT_WRAM_LOAD
