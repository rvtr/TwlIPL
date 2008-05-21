/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     HWInfoWriterLib.c

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
#include <sysmenu/acsign.h>
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
#include "hwi.h"

// define data------------------------------------------
#ifdef FIRM_USE_PRODUCT_KEYS                                                      // 鍵選択スイッチ
#define HWINFO_PRIVKEY_PATH     "rom:key/private_HWInfo.der"                // 製品用秘密鍵
#else
#define HWINFO_PRIVKEY_PATH     "rom:key/private_HWInfo_dev.der"            // 開発用秘密鍵
#endif

// extern data------------------------------------------
const LCFGTWLHWNormalInfo *LCFG_THW_GetDefaultNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetDefaultSecureInfo( void );
const LCFGTWLHWNormalInfo *LCFG_THW_GetNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetSecureInfo( void );

// function's prototype declaration---------------------
static HwiInitResult ReadPrivateKey( void );
static void VerifyHWInfo( void );
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len );
static BOOL ReadHWInfoFile( void );
static BOOL ReadTWLSettings( void );

// global variable -------------------------------------

// static variable -------------------------------------
static u8 *s_pPrivKeyBuffer = NULL;
static void *(*spAlloc)( u32 length );
static void  (*spFree)( void *ptr );

// const data  -----------------------------------------

static const u32 s_langBitmapList[ OS_TWL_REGION_MAX ] = {
    LCFG_TWL_LANG_BITMAP_JAPAN,
    LCFG_TWL_LANG_BITMAP_AMERICA,
    LCFG_TWL_LANG_BITMAP_EUROPE,
    LCFG_TWL_LANG_BITMAP_AUSTRALIA,
    LCFG_TWL_LANG_BITMAP_CHINA,
    LCFG_TWL_LANG_BITMAP_KOREA,
};

static char *strLanguage[] = {
    (char *)"LANG_JAPANESE",
    (char *)"LANG_ENGLISH",
    (char *)"LANG_FRENCH",
    (char *)"LANG_GERMAN",
    (char *)"LANG_ITALIAN",
    (char *)"LANG_SPANISH",
    (char *)"LANG_CHINESE",
    (char *)"LANG_KOREAN",
};

static const char *strLauncherGameCode[] = {
    "HNAJ",
    "HNAE",
    "HNAP",
    "HNAU",
    "HNAC",
    "HNAK",
};

//======================================================
// HW情報ライター
//======================================================

/*---------------------------------------------------------------------------*
  Name:         HWI_Init

  Description:  HW情報ライターの初期化

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/
HwiInitResult HWI_Init( void *(*pAlloc)( u32 ), void (*pFree)( void * ) )
{
    HwiInitResult result;

    spAlloc = pAlloc;
    spFree  = pFree;

    ACSign_SetAllocFunc( pAlloc, pFree );
    result = ReadPrivateKey();
    ReadHWInfoFile();
//  VerifyHWInfo();
    if( 1 ) {
        u8 sign[ RSA_KEY_LENGTH ];
        if( !LCFG_ReadHWID_Signature( sign ) ||
            !LCFG_CheckHWID_Signature( sign )
            ) {
            OS_TPrintf( "HWID Signature check failed.\n" );
        }else {
            OS_TPrintf( "HWID Signature check succeeded.\n" );
        }
    }
    // ※LanguageBitmapを判定で使用するので、必ずReadHWInfoの後で実行する必要がある。
    ReadTWLSettings();

    return result;
}


// TWL設定データのリード
static BOOL ReadTWLSettings( void )
{
    u8 *pBuffer = spAlloc( LCFG_READ_TEMP );
    BOOL result;
    if( pBuffer ) {
        result = LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ] )pBuffer );
		// Readに失敗した場合 LCFG_ReadTWLSettings 内部でファイルがリカバリ生成されるが
		// 返り値は FALSE となるためもう一度リードを試みる
		if (!result)
		{
			OS_TPrintf( "TSD read failed. Retry onece more.\n" );
	        result = LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ] )pBuffer );
		}
        spFree( pBuffer );
    }
    if( result ) {
        OS_TPrintf( "TSD read succeeded.\n" );
    }else {
        OS_TPrintf( "TSD read failed.\n" );
    }

	return result;
}

// 秘密鍵のリード
HwiInitResult ReadPrivateKey( void )
{
    BOOL result = FALSE;
    u32 keyLength;
    FSFile file;
    OSTick start = OS_GetTick();

    FS_InitFile( &file );
    if( !FS_OpenFileEx( &file, HWINFO_PRIVKEY_PATH, FS_FILEMODE_R ) )
    {
        OS_TPrintf( "PrivateKey read failed.\n" );
    }
    else
    {
        keyLength = FS_GetFileLength( &file );
        if( keyLength > 0 ) {
            s_pPrivKeyBuffer = spAlloc( keyLength );
            if( FS_ReadFile( &file, s_pPrivKeyBuffer, (s32)keyLength ) == keyLength ) {
                OS_TPrintf( "PrivateKey read succeeded.\n" );
                result = TRUE;
            }else {
                OS_TPrintf( "PrivateKey read failed.\n" );
            }
        }
        FS_CloseFile( &file );
    }

    if( !result && s_pPrivKeyBuffer ) {
        spFree( s_pPrivKeyBuffer );
        s_pPrivKeyBuffer = NULL;
    }
//  OS_TPrintf( "PrivKey read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

    if (result) {
#ifdef FIRM_USE_PRODUCT_KEYS
        return HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE;
#else
        return HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE;
#endif
    }else {
        return HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE;
    }
}

// HW情報全体のリード
static BOOL ReadHWInfoFile( void )
{
    LCFGReadResult retval;
	BOOL result = TRUE;
    OSTick start = OS_GetTick();

    retval = LCFGi_THW_ReadNormalInfo();
    if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        OS_TPrintf( "HW Normal Info read succeeded.\n" );
    }else {
		result = FALSE;
        OS_TPrintf( "HW Normal Info read failed.\n" );
    }

//  OS_TPrintf( "HW Normal Info read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

    start = OS_GetTick();
    retval = LCFGi_THW_ReadSecureInfo();
    if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        OS_TPrintf( "HW Secure Info read succeeded.\n" );
    }else {
		result = FALSE;
        OS_TPrintf( "HW Secure Info read failed.\n" );
    }
//  OS_TPrintf( "HW Secure Info read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

	return result;
}

// HWInfoファイルのベリファイ
static void VerifyHWInfo( void )
{
    if( VerifyData( (const u8 *)LCFG_THW_GetNormalInfo(), (const u8 *)LCFG_THW_GetDefaultNormalInfo(), sizeof(LCFGTWLHWNormalInfo) ) ) {
        OS_TPrintf( "HW normal Info verify succeeded.\n" );
    }else {
        OS_TPrintf( "HW normal Info verify failed.\n" );
    }
    if( VerifyData( (const u8 *)LCFG_THW_GetSecureInfo(), (const u8 *)LCFG_THW_GetDefaultSecureInfo(), sizeof(LCFGTWLHWSecureInfo) ) ) {
        OS_TPrintf( "HW secure Info verify succeeded.\n" );
    }else {
        OS_TPrintf( "HW secure Info verify failed.\n" );
    }

}

// メモリ上のデータベリファイ
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len )
{
    while( len-- ) {
        if( *pTgt++ != *pOrg++ ) {
            return FALSE;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_ModifyLanguage

  Description:  言語コードをリージョン値に合わせて修正する。

  Arguments:

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_ModifyLanguage( u8 region )
{
#pragma unused( region )
    u32 langBitmap = LCFG_THW_GetValidLanguageBitmap();
    u8  nowLanguage = LCFG_TSD_GetLanguage();
	BOOL result = TRUE;

	if (!ReadTWLSettings())
	{
		result = FALSE;
        OS_TPrintf( "HW Normal Info read failed.\n" );
	}

    if( langBitmap & ( 0x0001 << nowLanguage ) ) {
        OS_TPrintf( "Language no change.\n" );
    }else {
        int i;
        for( i = 0; i < LCFG_TWL_LANG_CODE_MAX; i++ ) {
            if( langBitmap & ( 0x0001 << i ) ) {
                break;
            }
        }
        LCFG_TSD_SetLanguage( (LCFGTWLLangCode)i );
        OS_TPrintf( "Language Change \"%s\" -> \"%s\"\n",
                    strLanguage[ nowLanguage ], strLanguage[ LCFG_TSD_GetLanguage() ] );
    }

    // 国コードもクリアしておく。
    LCFG_TSD_SetCountry( LCFG_TWL_COUNTRY_UNDEFINED );

    // ペアレンタルコントロール情報もクリアしておく
    MI_CpuClearFast( (void *)LCFG_TSD_GetPCTLPtr(), sizeof(LCFGTWLParentalControl) );

    // regionが変わった場合は、LANGUAGE_BITMAPも必ず変わるので、それをNTR側に反映させるために必ずTWL設定データの書き込みも行う。
    {
        u8 *pBuffer = spAlloc( LCFG_WRITE_TEMP );
        if( pBuffer ) {
            if (!LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer ))
			{
				result = FALSE;
				OS_TPrintf("Fail! LCFG_WriteTWLSettings()\n");
			}
            spFree( pBuffer );
        }
    }

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWNormalInfoFile

  Description:  HWノーマルInfoファイルのライト

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL HWI_WriteHWNormalInfoFile( void )
{
    LCFGTWLHWNormalInfo Info;
    LCFGReadResult result;

    result = LCFGi_THW_ReadNormalInfo();
    if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        if( !LCFGi_THW_RecoveryNormalInfo( result ) ) {
            OS_TPrintf( "HW Normal Info Recovery failed.\n" );
            return FALSE;
        }
    }

    Info.rtcAdjust = LCFG_THW_GetRTCAdjust();

    if (!LCFGi_THW_WriteNormalInfoDirect( &Info ))
    {
        OS_TPrintf( "HW Normal Info Write failed.\n" );
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWSecureInfoFile

  Description:  HWセキュアInfoファイルのライト

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL HWI_WriteHWSecureInfoFile( u8 region, const u8 *pSerialNo, BOOL isDisableWireless )
{
    BOOL isWrite = TRUE;
    LCFGReadResult result;

    // ファイルのリード
    result = LCFGi_THW_ReadSecureInfo();

    // リードに失敗したらリカバリ
    if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        if( !LCFGi_THW_RecoverySecureInfo( result ) ) {
            OS_TPrintf( "HW Secure Info Recovery failed.\n" );
            isWrite = FALSE;
        }
    }

    LCFG_THW_SetFlagForceDisableWireless( isDisableWireless );
    // リージョンのセット
    LCFG_THW_SetRegion( region );

    // 対応言語ビットマップのセット
    LCFG_THW_SetValidLanguageBitmap( s_langBitmapList[ region ] );

    // シリアルNo.のセット
    if( pSerialNo == NULL ) {
        // 量産工程でないとシリアルNo.は用意できないので、ここではMACアドレスをもとに適当な値をセットする。
        u8 buffer[ 12 ] = "SERIAL";     // 適当な文字列をMACアドレスと結合してSHA1を取り、仮SerialNoとする。
        u8 serialNo[ SVC_SHA1_DIGEST_SIZE ];
        int i;
        int len = ( LCFG_THW_GetRegion() == OS_TWL_REGION_AMERICA ) ?
                    LCFG_TWL_HWINFO_SERIALNO_LEN_AMERICA : LCFG_TWL_HWINFO_SERIALNO_LEN_OTHERS;
        OS_GetMacAddress( buffer + 6 );
        SVC_CalcSHA1( serialNo, buffer, sizeof(buffer) );
        for( i = 3; i < SVC_SHA1_DIGEST_SIZE; i++ ) {
            serialNo[ i ] = (u8)( ( serialNo[ i ] % 10 ) + 0x30 );
        }
        MI_CpuCopy8( "SRN", serialNo, 3 );
        MI_CpuClear8( &serialNo[ len ], sizeof(serialNo) - len );
//      OS_TPrintf( "serialNo : %s\n", serialNo );
        LCFG_THW_SetSerialNo( serialNo );
    }else {
        LCFG_THW_SetSerialNo( pSerialNo );
    }

    // ランチャーTitleID_Loのセット
    {
        int i;
        u8 titleID_Lo[4];
        for( i = 0; i < 4; i++ ) titleID_Lo[ i ] = (u8)strLauncherGameCode[ region ][ 4 - i - 1 ];
        LCFG_THW_SetLauncherTitleID_Lo( (const u8 *)titleID_Lo );
    }

    // ライト
    if( isWrite &&
        !LCFGi_THW_WriteSecureInfo( s_pPrivKeyBuffer ) ) {
        isWrite = FALSE;
        OS_TPrintf( "HW Secure Info Write failed.\n" );
    }

    return isWrite;
}


/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWIDSignFile

  Description:  HWID署名ファイルのライト

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL HWI_WriteHWIDSignFile( void )
{
    BOOL retval;

    (void)FS_DeleteFile( (char *)LCFG_TWL_HWID_SIGN_PATH );
    if( !FS_CreateFile( LCFG_TWL_HWID_SIGN_PATH, FS_PERMIT_R | FS_PERMIT_W ) ) {
        OS_TPrintf( "file create error. %s\n", LCFG_TWL_HWID_SIGN_PATH );
    }
    retval = LCFG_WriteHWID_Signature( s_pPrivKeyBuffer );
    if( !retval ) {
        OS_TPrintf( "HWID Signature Write failed.\n" );
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_DeleteHWNormalInfoFile

  Description:  HWノーマルInfoファイルの消去

  Arguments:    None

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_DeleteHWNormalInfoFile( void )
{
    if (FS_DeleteFile( (char *)LCFG_TWL_HWINFO_NORMAL_PATH ))
    {
        OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
        return TRUE;
    }
    else
    {
        OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         HWI_DeleteHWSecureInfoFile

  Description:  HWセキュアInfoファイルの消去

  Arguments:    None

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_DeleteHWSecureInfoFile( void )
{
    if (FS_DeleteFile( (char *)LCFG_TWL_HWINFO_SECURE_PATH ))
    {
        OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
        return TRUE;
    }
    else
    {
        OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         HWI_DeleteHWIDSignFile

  Description:  HWID署名ファイルの消去

  Arguments:    None

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_DeleteHWIDSignFile( void )
{
    if (FS_DeleteFile( (char *)LCFG_TWL_HWID_SIGN_PATH ))
    {
        OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWID_SIGN_PATH );
        return TRUE;
    }
    else
    {
        OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWID_SIGN_PATH );
        return FALSE;
    }
}
