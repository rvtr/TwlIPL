/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     TWLFileStore.h

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


#ifndef	TWL_STORE_FILE_H_
#define	TWL_STORE_FILE_H_
#if		defined(SDK_CW)


#include <twl.h>
#include <twl/os/common/format_rom.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define TSF_VERSION_TERMINATOR			0xff		// version終端
#define SAVE_COUNT_MAX					0x80		// saveCountの最大値
#define SAVE_COUNT_MASK					0x7f		// saveCountの値の範囲をマスクする。(0x00-0x7f）

// TSFリード結果
typedef enum TSFReadResult {
	TSF_READ_RESULT_SUCCEEDED = 0,
	TSF_READ_RESULT_ERROR_FILE_EXIST = 1,
	TSF_READ_RESULT_ERROR_FILE_LENGTH = 2,
	TSF_READ_RESULT_ERROR_DATA = 3
}TSFReadResult;


// TSFアクセス用パラメータ
typedef struct TSFParam {	// TSF ( TWL Store File )
	u32			dataLength;								// 保存するデータ長
	u32			fileLength;								// 保存するファイル長
	const u8	*conpatibleVerList;						// 過去ver.の互換ver.リスト。TSF_VERSION_TERMINATORで終端。
	void		(*pClearFunc)( void *pDst );			// 保存するデータの値クリア関数
	BOOL		(*pCheckDigestFunc)( void *pTgt, u32 len, u8 *pDigest );	// ダイジェストチェック関数へのポインタ
	BOOL		(*pCheckValueFunc)( void *pTgt );		// 値チェック関数へのポインタ
}TSFParam;


// TSFヘッダ
typedef struct TSFHeader{
	union digest {
		u8				sha1[ SVC_SHA1_DIGEST_SIZE ];	// SHA-1ダイジェスト
		u8				rsa[ RSA_KEY_LENGTH ];			// RSA署名
		u8				dst[ RSA_KEY_LENGTH ];			// 転送用の最大サイズ要素
	}digest;
	u8					version;						// データver.
	u8					saveCount;						// セーブカウント（ミラーリングしないファイルは使用しない）
	u8					rsv[2];							// 予約
	u32					bodyLength;						// データ長
}TSFHeader;	// 134bytes

// ※「TSFHeader+データ本体」の形でファイルに保存されます。

#ifdef SDK_ARM9

//=========================================================
// NANDファイルへのリードライト関数
//=========================================================

// TSFフォーマットのファイルのリード
extern TSFReadResult TSF_ReadFile ( char *pPath, void *pDstBody, const TSFParam *pParam, u8 *pSaveCount );
// TSFフォーマットのファイルのライト（pSaveCountにNULLを与えた場合は、saveCountを"0"固定にします。）
extern BOOL          TSF_WriteFile( char *pPath, TSFHeader *pHeader, const void *pSrcBody, u8 *pSaveCount );
// TSFフォーマットのファイルのリカバリ
extern BOOL          TSF_RecoveryFile( TSFReadResult err, char *pPath, u32 fileLength );

#endif // SDK_ARM9


#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// TWL_STORE_FILE_H_
