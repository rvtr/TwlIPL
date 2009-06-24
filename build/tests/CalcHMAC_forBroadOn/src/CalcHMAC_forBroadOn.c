/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include <nitro.h>
#include <nitro/misc.h>
#include <twl/crypto/rsa.h>
#include <nitro/crypto/util.h>
#include <twl/lcfg/common/TWLHWInfo.h>
#include <twl/lcfg/common/api.h>
#include "TWLStoreFile.h"

// define data------------------------------------------------------------------
#define HWINFO_PRIVKEY_PATH         "rom:key/private_HWInfo.der"                // 製品用秘密鍵
#define RSA_KEY_LENGTH_1024			RSA_KEY_LENGTH
#define HWINFO_S_BODY_SIZE			0x1C

#define PATH_K00A317_JP				"sdmc:/hwinfo_s/HWInfo_S.K00A317_JP.dat"
#define PATH_K00A317_US				"sdmc:/hwinfo_s/HWInfo_S.K00A317_US.dat"
#define PATH_K00A317_EU				"sdmc:/hwinfo_s/HWInfo_S.K00A317_EU.dat"
#define PATH_K00A317_AU				"sdmc:/hwinfo_s/HWInfo_S.K00A317_AU.dat"
#define PATH_K00A319_JP				"sdmc:/hwinfo_s/HWInfo_S.K00A319_JP.dat"
#define PATH_K00A319_US				"sdmc:/hwinfo_s/HWInfo_S.K00A319_US.dat"
#define PATH_K00A319_EU				"sdmc:/hwinfo_s/HWInfo_S.K00A319_EU.dat"
#define PATH_K00A319_AU				"sdmc:/hwinfo_s/HWInfo_S.K00A319_AU.dat"

//------------------------------------------------------------------------------
extern s32 CRYPTO_RSA_Sign_custom(CRYPTORSASignContext *context, CRYPTORSASignParam *param);

//------------------------------------------------------------------------------
static u8 s_privKey[ 4096 ] ATTRIBUTE_ALIGN(32);
static TSFHeader s_header;

/*----------------------------------------------------------------------------*/
static const u64 FuseID_K00A317 = 0x08a1080105112134LLU;
static const u64 FuseID_K00A319 = 0x08a1080105112122LLU;

/*----------------------------------------------------------------------------*/
static const u8 HWINFO_S_K00A317_JP[HWINFO_S_BODY_SIZE] =
{
    0x01, 0x00, 0x00, 0x00,                                 /* validLanguageBitmap */
    0x00, 0x00, 0x00, 0x00,                                 /* flags, rsv */
    0x00,                                                   /* region */
    'T', 'J', 'N', '5', '6', '3', '2', '0',
    '4', '0', '2', '0', 0x00, 'K', 0x00,                    /* serialNo */
    'J', 'A', 'N', 'H'                                      /* launcherTitleID_Lo */
};

static const u8 HWINFO_S_K00A317_US[HWINFO_S_BODY_SIZE] =
{
    0x26, 0x00, 0x00, 0x00,                                 /* validLanguageBitmap */
    0x00, 0x00, 0x00, 0x00,                                 /* flags, rsv */
    0x01,                                                   /* region */
    'T', 'N', '5', '6', '3', '2', '0', '4',
    '0', '2', '0', 0x00, 0x00, 'K', 0x00,                   /* serialNo */
    'E', 'A', 'N', 'H'                                      /* launcherTitleID_Lo */
};

static const u8 HWINFO_S_K00A317_EU[HWINFO_S_BODY_SIZE] =
{
    0x3E, 0x00, 0x00, 0x00,                                 /* validLanguageBitmap */
    0x00, 0x00, 0x00, 0x00,                                 /* flags, rsv */
    0x02,                                                   /* region */
    'T', 'E', 'N', '5', '6', '3', '2', '0',
    '4', '0', '2', '0', 0x00, 'K', 0x00,                    /* serialNo */
    'P', 'A', 'N', 'H'                                      /* launcherTitleID_Lo */
};

static const u8 HWINFO_S_K00A317_AU[HWINFO_S_BODY_SIZE] =
{
    0x02, 0x00, 0x00, 0x00,                                 /* validLanguageBitmap */
    0x00, 0x00, 0x00, 0x00,                                 /* flags, rsv */
    0x03,                                                   /* region */
    'T', 'A', 'N', '5', '6', '3', '2', '0',
    '4', '0', '2', '0', 0x00, 'K', 0x00,                    /* serialNo */
    'U', 'A', 'N', 'H'                                      /* launcherTitleID_Lo */
};


/*----------------------------------------------------------------------------*/
static const u8 HWINFO_S_K00A319_JP[HWINFO_S_BODY_SIZE] =
{
    0x01, 0x00, 0x00, 0x00,                                 /* validLanguageBitmap */
    0x00, 0x00, 0x00, 0x00,                                 /* flags, rsv */
    0x00,                                                   /* region */
    'T', 'J', 'N', '3', '5', '3', '8', '9',
    '0', '3', '3', '4', 0x00, 'K', 0x00,                    /* serialNo */
    'J', 'A', 'N', 'H'                                      /* launcherTitleID_Lo */
};

static const u8 HWINFO_S_K00A319_US[HWINFO_S_BODY_SIZE] =
{
    0x26, 0x00, 0x00, 0x00,                                 /* validLanguageBitmap */
    0x00, 0x00, 0x00, 0x00,                                 /* flags, rsv */
    0x01,                                                   /* region */
    'T', 'N', '3', '5', '3', '8', '9', '0',
    '3', '3', '4', 0x00, 0x00, 'K', 0x00,                   /* serialNo */
    'E', 'A', 'N', 'H'                                      /* launcherTitleID_Lo */
};

static const u8 HWINFO_S_K00A319_EU[HWINFO_S_BODY_SIZE] =
{
    0x3E, 0x00, 0x00, 0x00,                                 /* validLanguageBitmap */
    0x00, 0x00, 0x00, 0x00,                                 /* flags, rsv */
    0x02,                                                   /* region */
    'T', 'E', 'N', '3', '5', '3', '8', '9',
    '0', '3', '3', '4', 0x00, 'K', 0x00,                    /* serialNo */
    'P', 'A', 'N', 'H'                                      /* launcherTitleID_Lo */
};

static const u8 HWINFO_S_K00A319_AU[HWINFO_S_BODY_SIZE] =
{
    0x02, 0x00, 0x00, 0x00,                                 /* validLanguageBitmap */
    0x00, 0x00, 0x00, 0x00,                                 /* flags, rsv */
    0x03,                                                   /* region */
    'T', 'A', 'N', '3', '5', '3', '8', '9',
    '0', '3', '3', '4', 0x00, 'K', 0x00,                    /* serialNo */
    'U', 'A', 'N', 'H'                                      /* launcherTitleID_Lo */
};

//================================================================================

static void CalcHWSecureInfoHMAC( void *pDstHMAC, const void* body, u64 fuseID )
{
    u8  key[SVC_SHA1_DIGEST_SIZE];
    int i;
    SVC_CalcSHA1(key, &fuseID, sizeof(u64));
    SVC_CalcHMACSHA1( pDstHMAC, body, HWINFO_S_BODY_SIZE, key, SVC_SHA1_DIGEST_SIZE);
	
	OS_TPrintf( "key : " );
	for( i = 0; i < SVC_SHA1_DIGEST_SIZE; i++ ) {
		OS_TPrintf( "%02x", key[ i ] );
	}
	OS_TPrintf( "\n" );
	OS_TPrintf( "hmac: " );
	for( i = 0; i < SVC_SHA1_DIGEST_SIZE; i++ ) {
		OS_TPrintf( "%02x", ((u8 *)pDstHMAC)[ i ] );
	}
	OS_TPrintf( "\n" );
	
    return;
}

// 署名算出（BERエンコードなし）
static BOOL CalcSignature( void *pDstSign, const void *pSrc, u32 len, const u8 *pPrivKeyDER, u32 keyFileLen, u32 keyLen )
{
	CRYPTORSASignContext   context;
	CRYPTORSASignInitParam initParam;
	CRYPTORSASignParam     param;
	int result_len;
	
	initParam.key     = (void*)pPrivKeyDER;
	initParam.key_len = keyFileLen;
	if( CRYPTO_RSA_SignInit(&context, &initParam) != 0 ) {
		return FALSE;
	}
	
	param.in       = (void *)pSrc;
	param.in_len   = len;
	param.out      = pDstSign;
	param.out_size = keyLen;
	
	result_len = CRYPTO_RSA_Sign_custom(&context, &param);
	if( result_len != keyLen ) {
		return FALSE;
	}
	
	return CRYPTO_RSA_SignTerminate(&context) == 0 ? TRUE : FALSE;
}


static BOOL LCFGi_THW_WriteSecureInfoCustom( char *pPath, const LCFGTWLHWSecureInfo *pSrcInfo, u64 fuseID, const u8 *pPrivKeyDER, u32 keyFileLen )
{
    TSFHeader header;

	// ヘッダ初期化
    MI_CpuClear8( &header, sizeof(TSFHeader) );
    header.version = LCFG_TWL_HWINFO_SECURE_VERSION;
    header.bodyLength  = sizeof( LCFGTWLHWSecureInfo );
	
	// ヘッダ署名算出
	{
	    u8  hmac_sha1[ SVC_SHA1_DIGEST_SIZE ];
		CalcHWSecureInfoHMAC( hmac_sha1, pSrcInfo, fuseID );
		if( !CalcSignature( (void *)header.digest.rsa, hmac_sha1, SVC_SHA1_DIGEST_SIZE, pPrivKeyDER, keyFileLen, RSA_KEY_LENGTH_1024 ) ) {
			return FALSE;
		}
	}

	// ファイル生成
	(void)FS_CreateFileAuto( pPath, FS_PERMIT_R | FS_PERMIT_W );
	{
#define HWINFO_FILE_LEN		( 16 * 1024 )
		FSFile file[1];
		char *pBuffer = OS_Alloc( HWINFO_FILE_LEN );
		
		if( !FS_OpenFileEx( file, pPath, FS_FILEMODE_RW ) ) {
			return FALSE;
		}
		if( FS_SetFileLength( file, HWINFO_FILE_LEN ) != FS_RESULT_SUCCESS ) {
			(void)FS_CloseFile( file );
			return FALSE;
		}
		if( pBuffer ) {
			MI_CpuFillFast( pBuffer, 0xffffffff, HWINFO_FILE_LEN );
			(void)FS_WriteFile( file, pBuffer, HWINFO_FILE_LEN );
		}
		(void)FS_CloseFile( file );
	}
	
    // ライト
    if( !LCFGi_TSF_WriteFile( pPath, &header, (const void *)pSrcInfo, NULL ) ) {
        return FALSE;
    }
	return TRUE;
}


// 秘密鍵のリード
static BOOL ReadPrivateKey( void *pBuffer, u32 *pKeyFileLen, char *pPath )
{
    BOOL retval = TRUE;
    FSFile file;

    FS_InitFile( &file );
    if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R ) )
    {
        OS_TPrintf( "PrivateKey read failed.\n" );
		retval = FALSE;
    }else {
        *pKeyFileLen = FS_GetFileLength( &file );
        if( *pKeyFileLen > 0 ) {
			if( FS_ReadFile( &file, pBuffer, (s32)*pKeyFileLen ) == *pKeyFileLen ) {
                OS_TPrintf( "PrivateKey read succeeded.\n" );
            }else {
                OS_TPrintf( "PrivateKey read failed.\n" );
				retval = FALSE;
            }
        }
        FS_CloseFile( &file );
    }
	return retval;
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocSystem

  Description:  ヒープを作成して OS_Alloc が使えるようにします。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitAllocSystem()
{
    void* newArenaLo;
    OSHeapHandle hHeap;

    // メインアリーナのアロケートシステムを初期化
    newArenaLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo(newArenaLo);

    // メインアリーナ上にヒープを作成
    hHeap = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}

extern u64 g_HWInfoFuseROM;
extern char *g_pHwInfoPath;


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
	BOOL retval;
	u32 keyLen;
	
    OS_Init();
	InitAllocSystem();
    (void)OS_SetIrqMask(0);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);
    FS_Init( FS_DMA_NOT_USE );
	CRYPTO_SetMemAllocator( OS_AllocFromMain, OS_FreeToMain, NULL );
	
    OS_TPrintf("---- HMAC over HWInfo_S for PROD boards @ BroadOn ----\n");
	
	if( !ReadPrivateKey( s_privKey, &keyLen, HWINFO_PRIVKEY_PATH ) ) {
	    OS_TPrintf("PrivateKey read failed.\n");
	}else {
	    OS_TPrintf("TS-Board: K00A317\n");
		retval = LCFGi_THW_WriteSecureInfoCustom( PATH_K00A317_JP, (const LCFGTWLHWSecureInfo *)HWINFO_S_K00A317_JP, FuseID_K00A317, s_privKey, keyLen );
		OS_TPrintf("  JP: %s\n", retval ? "succeeded" : "failed" );
		retval = LCFGi_THW_WriteSecureInfoCustom( PATH_K00A317_US, (const LCFGTWLHWSecureInfo *)HWINFO_S_K00A317_US, FuseID_K00A317, s_privKey, keyLen );
		OS_TPrintf("  US: %s\n", retval ? "succeeded" : "failed" );
		retval = LCFGi_THW_WriteSecureInfoCustom( PATH_K00A317_EU, (const LCFGTWLHWSecureInfo *)HWINFO_S_K00A317_EU, FuseID_K00A317, s_privKey, keyLen );
		OS_TPrintf("  EU: %s\n", retval ? "succeeded" : "failed" );
		retval = LCFGi_THW_WriteSecureInfoCustom( PATH_K00A317_AU, (const LCFGTWLHWSecureInfo *)HWINFO_S_K00A317_AU, FuseID_K00A317, s_privKey, keyLen );
		OS_TPrintf("  AU: %s\n", retval ? "succeeded" : "failed" );
	    
	    OS_TPrintf("TS-Board: K00A319\n");
		retval = LCFGi_THW_WriteSecureInfoCustom( PATH_K00A319_JP, (const LCFGTWLHWSecureInfo *)HWINFO_S_K00A319_JP, FuseID_K00A319, s_privKey, keyLen );
		OS_TPrintf("  JP: %s\n", retval ? "succeeded" : "failed" );
		retval = LCFGi_THW_WriteSecureInfoCustom( PATH_K00A319_US, (const LCFGTWLHWSecureInfo *)HWINFO_S_K00A319_US, FuseID_K00A319, s_privKey, keyLen );
		OS_TPrintf("  US: %s\n", retval ? "succeeded" : "failed" );
		retval = LCFGi_THW_WriteSecureInfoCustom( PATH_K00A319_EU, (const LCFGTWLHWSecureInfo *)HWINFO_S_K00A319_EU, FuseID_K00A319, s_privKey, keyLen );
		OS_TPrintf("  EU: %s\n", retval ? "succeeded" : "failed" );
		retval = LCFGi_THW_WriteSecureInfoCustom( PATH_K00A319_AU, (const LCFGTWLHWSecureInfo *)HWINFO_S_K00A319_AU, FuseID_K00A319, s_privKey, keyLen );
		OS_TPrintf("  AU: %s\n", retval ? "succeeded" : "failed" );
	}
#if 0
    OS_TPrintf("---- Verify phase ----\n");
	{
	    OS_TPrintf("TS-Board: K00A317\n");
		g_HWInfoFuseROM = FuseID_K00A317;
		g_pHwInfoPath   = PATH_K00A317_JP;
		retval =  LCFG_ReadHWSecureInfo();
		OS_TPrintf("  JP: %s\n", retval ? "succeeded" : "failed" );
		g_pHwInfoPath   = PATH_K00A317_US;
		retval =  LCFG_ReadHWSecureInfo();
		OS_TPrintf("  US: %s\n", retval ? "succeeded" : "failed" );
		g_pHwInfoPath   = PATH_K00A317_EU;
		retval =  LCFG_ReadHWSecureInfo();
		OS_TPrintf("  EU: %s\n", retval ? "succeeded" : "failed" );
		g_pHwInfoPath   = PATH_K00A317_AU;
		retval =  LCFG_ReadHWSecureInfo();
		OS_TPrintf("  AU: %s\n", retval ? "succeeded" : "failed" );
		
	    OS_TPrintf("TS-Board: K00A319\n");
		g_HWInfoFuseROM = FuseID_K00A319;
		g_pHwInfoPath   = PATH_K00A319_JP;
		retval =  LCFG_ReadHWSecureInfo();
		OS_TPrintf("  JP: %s\n", retval ? "succeeded" : "failed" );
		g_pHwInfoPath   = PATH_K00A319_US;
		retval =  LCFG_ReadHWSecureInfo();
		OS_TPrintf("  US: %s\n", retval ? "succeeded" : "failed" );
		g_pHwInfoPath   = PATH_K00A319_EU;
		retval =  LCFG_ReadHWSecureInfo();
		OS_TPrintf("  EU: %s\n", retval ? "succeeded" : "failed" );
		g_pHwInfoPath   = PATH_K00A319_AU;
		retval =  LCFG_ReadHWSecureInfo();
		OS_TPrintf("  AU: %s\n", retval ? "succeeded" : "failed" );
	}
#endif
    OS_Terminate();
}
