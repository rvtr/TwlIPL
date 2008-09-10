/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_sharedFont.c

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
#include <twl/os/common/sharedFont.h>

#ifdef USE_FONT_WRAM_LOAD
#include "fs_wram.h"
#endif // USE_FONT_WRAM_LOAD

// compile switch--------------------------------------------------------------
//#define DISABLE_HASH_CHECK

// extern data-----------------------------------------------------------------
// define data-----------------------------------------------------------------
#ifdef USE_FONT_WRAM_LOAD
#define THREAD_PRIO_FS_FRAM					3
#define WRAM_TOP_SLOT_FOR_FS				4
#define WRAM_SIZE_FOR_FS					MI_WRAM_SIZE_96KB
#endif // USE_FONT_WRAM_LOAD

#define OS_SHARED_FONT_TABLE_PATH			"nand:/<sharedFont>"
#define OS_SHARED_FONT_FILE_NAME_LENGTH		0x20
#define OS_SHARED_FONT_SIGN_SIZE			0x80

// 共有フォントテーブルヘッダ
typedef struct OSSharedFontHeader {
	u32 timestamp;
	u16 fontNum;
	u8  pad[ 6 ];
	u8  digest[ SVC_SHA1_DIGEST_SIZE ];
}OSSharedFontHeader;

// 共有フォントテーブル
typedef struct OSSharedFontEntry {
	u8		fileName[ OS_SHARED_FONT_FILE_NAME_LENGTH ];
//	u8		pad[ 4 ];
    u32     compLength;     // 圧縮後サイズ
	u32		offset;
	u32		origLength;     // 元のサイズ
	u8		digest[ SVC_SHA1_DIGEST_SIZE ];
}OSSharedFontEntry;

// 共有フォントライブラリワーク
typedef struct OSSharedFontWork {
	OSSharedFontHeader	header;
	OSSharedFontEntry	*pInfoTable;
}OSSharedFontWork;

#ifdef USE_FONT_WRAM_LOAD
// SHA1コールバック引数
typedef struct CalcSHA1CallbackArg
{
	SVCSHA1Context	ctx;
	u32				hash_length;
} CalcSHA1CallbackArg;
#endif // USE_FONT_WRAM_LOAD

// function's prototype-------------------------------------------------------
#ifdef USE_FONT_WRAM_LOAD
static void CalcSHA1Callback(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg);
#endif // USE_FONT_WRAM_LOAD

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static OSSharedFontWork s_work;
static BOOL s_isInitialized = FALSE;

// const data------------------------------------------------------------------
// 共有フォント署名確認用公開鍵
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
BOOL OS_InitSharedFont( void )
{
	FSFile file[1];
	u8 signature[ OS_SHARED_FONT_SIGN_SIZE ];
	
	if( s_isInitialized ) {
		return TRUE;
	}
	
	MI_CpuClear32( &s_work, sizeof(s_work) );
	
	if( !FS_OpenFileEx( file, OS_SHARED_FONT_TABLE_PATH, FS_FILEMODE_R ) ) {
		return FALSE;
	}
	
	// 署名リード
	if( FS_ReadFile( file, signature, OS_SHARED_FONT_SIGN_SIZE ) != OS_SHARED_FONT_SIGN_SIZE ){
		goto ERROR;
	}
	
	// ヘッダリード
	if( FS_ReadFile( file, &s_work.header, sizeof(OSSharedFontHeader) ) != sizeof(OSSharedFontHeader) ){
		goto ERROR;
	}
	
	FS_CloseFile( file );
	
#ifndef DISABLE_HASH_CHECK
	// ヘッダ署名チェック
	{
		u8 calc_digest[ SVC_SHA1_DIGEST_SIZE ];
		u8 sign_digest[ SVC_SHA1_DIGEST_SIZE ];
	    static u32 heap[ 4096 / sizeof(u32) ];
		
	    SVCSignHeapContext acmemoryPool;
	    SVC_CalcSHA1( calc_digest, &s_work.header, sizeof(OSSharedFontHeader) );
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
	FS_InitWramTransfer( THREAD_PRIO_FS_FRAM );
	MI_FreeWramSlot_C( WRAM_TOP_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
	MI_FreeWramSlot_C( WRAM_TOP_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
	MI_FreeWramSlot_C( WRAM_TOP_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
	MI_CancelWramSlot_C( WRAM_TOP_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
	MI_CancelWramSlot_C( WRAM_TOP_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
	MI_CancelWramSlot_C( WRAM_TOP_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
#endif // USE_FONT_WRAM_LOAD
	
	s_isInitialized = TRUE;
	return TRUE;
	
ERROR:
	FS_CloseFile( file );
	return FALSE;
}


// 共有フォント　テーブルサイズ取得
int OS_GetSharedFontTableSize( void )
{
	if( s_isInitialized ) {
		return (int)( s_work.header.fontNum * sizeof(OSSharedFontEntry) );
	}else {
		return -1;
	}
}


// 共有フォント　テーブルロード
BOOL OS_LoadSharedFontTable( void *pBuffer )
{
	FSFile file[1];
	u32 tableLen = sizeof(OSSharedFontEntry) * s_work.header.fontNum;
	
	if( ( !s_isInitialized ) ||
		( s_work.header.fontNum == 0 ) ||
		( pBuffer == NULL ) ||
		( s_work.pInfoTable ) ) {
		return FALSE;
	}
	
	// フォントテーブルリード
	if( !FS_OpenFileEx( file, OS_SHARED_FONT_TABLE_PATH, FS_FILEMODE_R ) ) {
		return FALSE;
	}
	if( !FS_SeekFile( file, OS_SHARED_FONT_SIGN_SIZE + sizeof(OSSharedFontHeader), FS_SEEK_SET ) ){
		goto ERROR;
	}
	if( FS_ReadFile( file, pBuffer, (int)tableLen ) != tableLen ){
		goto ERROR;
	}
	FS_CloseFile( file );
	
#ifndef DISABLE_HASH_CHECK
	// フォントテーブル　ハッシュチェック
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
int OS_GetSharedFontSize( OSSharedFontIndex index )
{
	if( ( s_isInitialized == NULL ) ||
		( s_work.pInfoTable == NULL ) ||
		( index >= s_work.header.fontNum ) ||
		( index <  0 ) ||
		( index >= OS_SHARED_FONT_MAX ) ) {
		return -1;
	}
	
	return (int)s_work.pInfoTable[ index ].origLength;
}


// 共有フォント 圧縮後フォントサイズ取得
int OS_GetSharedFontCompressedSize( OSSharedFontIndex index )
{
	if( ( s_isInitialized == NULL ) ||
		( s_work.pInfoTable == NULL ) ||
		( index >= s_work.header.fontNum ) ||
		( index <  0 ) ||
		( index >= OS_SHARED_FONT_MAX ) ) {
		return -1;
	}
	
	return (int)s_work.pInfoTable[ index ].compLength;
}


// 共有フォント　フォントネーム取得
const u8 *OS_GetSharedFontName( OSSharedFontIndex index )
{
	if( ( s_isInitialized == NULL ) ||
		( s_work.pInfoTable == NULL ) ||
		( index >= s_work.header.fontNum ) ||
		( index <  0 ) ||
		( index >= OS_SHARED_FONT_MAX ) ) {
		return NULL;
	}
	
	return s_work.pInfoTable[ index ].fileName;
}


// 共有フォント　タイムスタンプ取得
u32 OS_GetSharedFontTimestamp( void )
{
	if( ( s_isInitialized == NULL ) ) {
		return 0;
	}
	return s_work.header.timestamp;
}


// 共有フォント　フォントロード
BOOL OS_LoadSharedFont( OSSharedFontIndex index, void *pBuffer )
{
	FSFile file[1];
	OSSharedFontEntry *pInfo = &s_work.pInfoTable[ index ];
	u8 calc_digest[ SVC_SHA1_DIGEST_SIZE ];

	if( ( s_isInitialized == NULL ) ||
		( s_work.pInfoTable == NULL ) ||
		( index >= s_work.header.fontNum ) ||
		( index <  0 ) ||
		( index >= OS_SHARED_FONT_MAX ) ) {
		return FALSE;
	}
	
	// フォント　リード
	if( !FS_OpenFileEx( file, OS_SHARED_FONT_TABLE_PATH, FS_FILEMODE_R ) ) {
		return FALSE;
	}
	if( !FS_SeekFile( file, (int)pInfo->offset, FS_SEEK_SET ) ){
		goto ERROR;
	}
#ifdef USE_FONT_WRAM_LOAD
	{
		CalcSHA1CallbackArg arg;
        SVC_SHA1Init( &arg.ctx );
		arg.hash_length = pInfo->compLength;
		if( !FS_ReadFileViaWram( file, pBuffer, (s32)MATH_ROUNDUP( pInfo->compLength, 0x20 ), MI_WRAM_C,
								 WRAM_TOP_SLOT_FOR_FS, WRAM_SIZE_FOR_FS,
#ifndef DISABLE_HASH_CHECK
								 CalcSHA1Callback,
#else
								 NULL,
#endif // DISABLE_HASH_CHECK
								 &arg ) ) {
			goto ERROR;
		}
		SVC_SHA1GetHash( &arg.ctx, &calc_digest );
	}
#else
	if( FS_ReadFile( file, pBuffer, (int)pInfo->compLength ) != pInfo->compLength ){
		goto ERROR;
	}
#ifndef DISABLE_HASH_CHECK
    SVC_CalcSHA1( calc_digest, pBuffer, pInfo->compLength );
#endif // DISABLE_HASH_CHECK
#endif // USE_FONT_WRAM_LOAD
	
	FS_CloseFile( file );
	
#ifndef DISABLE_HASH_CHECK
	// フォント　ハッシュチェック
	if( !SVC_CompareSHA1( calc_digest, pInfo->digest ) ) {
		return FALSE;
	}
#endif // DISABLE_HASH_CHECK
	
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
