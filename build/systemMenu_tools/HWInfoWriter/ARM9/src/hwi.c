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
#include <nitro/crypto.h>
#include <sysmenu.h>
#include <sysmenu/acsign.h>
#include <sysmenu/namut.h>
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
#include "hwi.h"
#if 0
#include "kami_pxi.h"
#endif

// define data------------------------------------------
#ifdef FIRM_USE_PRODUCT_KEYS                                                // 鍵選択スイッチ
#define HWINFO_PRIVKEY_PATH     "rom:key/private_HWInfo.der"                // 製品用秘密鍵
#define HWID_PRIVKEY_PATH       "rom:key/private_HWID.der"
#else
#define HWINFO_PRIVKEY_PATH     "rom:key/private_HWInfo_dev.der"            // 開発用秘密鍵
#define HWID_PRIVKEY_PATH       "rom:key/private_HWID_dev.der"
#endif

// extern data------------------------------------------
const LCFGTWLHWNormalInfo *LCFG_THW_GetDefaultNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetDefaultSecureInfo( void );
const LCFGTWLHWNormalInfo *LCFG_THW_GetNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetSecureInfo( void );

// function's prototype declaration---------------------
static HwiInitResult ReadPrivateKey( void **ppBuffer, u32 *pKeyFileLen, char *pPath );
static void VerifyHWInfo( void );
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len );
static BOOL CheckHWID_Signature( BOOL isPrintf );
static BOOL ReadHWInfoFile( void );
static BOOL ReadTWLSettings( void );
static u8 HWIi_CalcCheckCode(u8 src[8]);

// global variable -------------------------------------

// static variable -------------------------------------
static void *s_pPrivKeyBuffer = NULL;
static void *s_pPrivKeyBufferHWID = NULL;
static u32  s_privKeyBufferLen;
static u32  s_privKeyBufferLenHWID;
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

static const char regionAsciiForSerialNo[] = {
	'J',	// 日本
	'W',	// 米国
	'E',	// 欧州
	'A',	// 豪州
	'C',	// 中国
	'K'		// 韓国
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

	CRYPTO_SetMemAllocator( pAlloc, pFree, NULL );
	
    result = ReadPrivateKey( &s_pPrivKeyBuffer, &s_privKeyBufferLen, HWINFO_PRIVKEY_PATH );
	if( result != HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE ) {
	    result = ReadPrivateKey( &s_pPrivKeyBufferHWID, &s_privKeyBufferLenHWID, HWID_PRIVKEY_PATH );
	}
    ReadHWInfoFile();
//  VerifyHWInfo();
	(void)CheckHWID_Signature( TRUE );
	
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
		// Readに失敗した場合ファイルのリカバリ生成を試みる
		if (!result)
		{
			OS_TPrintf( "TSD read failed. Retry onece more.\n" );
	        result = LCFG_RecoveryTWLSettings();
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
static HwiInitResult ReadPrivateKey( void **ppBuffer, u32 *pKeyFileLen, char *pPath )
{
    BOOL result = FALSE;
    FSFile file;
    OSTick start = OS_GetTick();

    FS_InitFile( &file );
    if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R ) )
    {
        OS_TPrintf( "PrivateKey read failed.\n" );
    }
    else
    {
        *pKeyFileLen = FS_GetFileLength( &file );
        if( *pKeyFileLen > 0 ) {
            *ppBuffer = spAlloc( *pKeyFileLen );
			if( FS_ReadFile( &file, *ppBuffer, (s32)*pKeyFileLen ) == *pKeyFileLen ) {
                OS_TPrintf( "PrivateKey read succeeded.\n" );
                result = TRUE;
            }else {
                OS_TPrintf( "PrivateKey read failed.\n" );
            }
        }
        FS_CloseFile( &file );
    }

	if( !result && *ppBuffer ) {
		spFree( *ppBuffer );
        *ppBuffer = NULL;
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

    retval = LCFGi_THW_ReadSecureInfo();
    if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
	    // HWセキュア情報をメモリに展開しておく
	    MI_CpuCopyFast( LCFGi_GetHWS(), (void *)HW_HW_SECURE_INFO, HW_HW_SECURE_INFO_END - HW_HW_SECURE_INFO );
        OS_TPrintf( "HW Secure Info read succeeded.\n" );
    }else {
		result = FALSE;
        OS_TPrintf( "HW Secure Info read failed.\n" );
    }

    retval = LCFGi_THW_ReadNormalInfo();
    if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
	    // HWノーマル情報をメモリに展開しておく
    	MI_CpuCopyFast( LCFGi_GetHWN(), (void *)HW_PARAM_TWL_HW_NORMAL_INFO, sizeof(LCFGTWLHWNormalInfo) );
        OS_TPrintf( "HW Normal Info read succeeded.\n" );
    }else {
		result = FALSE;
        OS_TPrintf( "HW Normal Info read failed.\n" );
    }

	return result;
}

// HWInfoファイルのベリファイ
static void VerifyHWInfo( void )
{
    if( VerifyData( (const u8 *)LCFG_THW_GetSecureInfo(), (const u8 *)LCFG_THW_GetDefaultSecureInfo(), sizeof(LCFGTWLHWSecureInfo) ) ) {
        OS_TPrintf( "HW secure Info verify succeeded.\n" );
    }else {
        OS_TPrintf( "HW secure Info verify failed.\n" );
    }
    if( VerifyData( (const u8 *)LCFG_THW_GetNormalInfo(), (const u8 *)LCFG_THW_GetDefaultNormalInfo(), sizeof(LCFGTWLHWNormalInfo) ) ) {
        OS_TPrintf( "HW normal Info verify succeeded.\n" );
    }else {
        OS_TPrintf( "HW normal Info verify failed.\n" );
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


// HWID署名の確認
static BOOL CheckHWID_Signature( BOOL isPrintf )
{
    u8 sign[ LCFG_TWL_HWID_SIGN_LENGTH ];
    if( !LCFG_ReadHWID_Signature( sign ) ||
        !LCFG_CheckHWID_Signature( sign )
        ) {
        if( isPrintf ) OS_TPrintf( "HWID Signature check failed.\n" );
		return FALSE;
    }else {
        if( isPrintf ) OS_TPrintf( "HWID Signature check succeeded.\n" );
		return TRUE;
    }
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
	u8  installedSoftBoxCount;
	u8  freeSoftBoxCount;
	BOOL result = TRUE;

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

	// ソフトボックスカウントを更新
	if (!NAMUT_GetSoftBoxCount(&installedSoftBoxCount, &freeSoftBoxCount))
	{
		return FALSE;
	}

	// LCFGライブラリの静的変数に対する更新
    LCFG_TSD_SetInstalledSoftBoxCount( installedSoftBoxCount );	
    LCFG_TSD_SetFreeSoftBoxCount( freeSoftBoxCount );
	
    // regionが変わった場合は、LANGUAGE_BITMAPも必ず変わるので、それをNTR側に反映させるために必ずTWL設定データの書き込みも行う。
    {
        u8 *pBuffer = spAlloc( LCFG_WRITE_TEMP );
        if( pBuffer ) 
		{
			int i;
			// TWLCFG0.datとTWLCFG1.datの両方について生成/書き込みを行うため2回実行する
			for (i=0;i<2;i++)
			{
				// 内部変数を変更せずにリード状態(s_rdResult)をFALSEにするためにベリファイする
				(void)LCFGi_TSD_VerifySettings( (u8 (*)[ sizeof(LCFGTWLSettingsData) ])pBuffer );

	            if (!LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer ))
				{
					// NANDをフォーマットした直後でTWL設定ファイルが存在しない場合は書き込みに失敗するため
					// TWL設定ファイルをリカバリ生成して再チャレンジする
		        	if (!LCFG_RecoveryTWLSettings() && !LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer ))
					{
						result = FALSE;
						OS_TPrintf("Fail! LCFG_WriteTWLSettings()\n");
					}
				}
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

const u8 s_serialNo_mask[] = {
	0x82, 0xb7, 0x0e, 0xf3, 0xad, 0x0a, 0x35, 0x85,
	0xaa, 0x84, 0xae, 0x14, 0xe7, 0x06, 0xd5, 0x00,
};

BOOL HWI_WriteHWNormalInfoFile( void )
{
    LCFGTWLHWNormalInfo Info;
    LCFGReadResult result;

	MI_CpuClear8( &Info, sizeof(LCFGTWLHWNormalInfo) );
	
    result = LCFGi_THW_ReadNormalInfo();
    if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        if( !LCFGi_THW_RecoveryNormalInfo( result ) ) {
            OS_TPrintf( "HW Normal Info Recovery failed.\n" );
            return FALSE;
        }
    }

#if 0
	{
		// 基板検査器でNAND RAWパーティションに書き込まれるRTC補正値を読み出そうと試みたが、完成品検査器で消去されていた。。。
#define NAND_BLOCK_BYTE             0x200
#define NAND_RTC_BLOCK              (0x100000*1-NAND_BLOCK_BYTE)/NAND_BLOCK_BYTE
		int value = 0;
		u8 *p = OS_Alloc( NAND_BLOCK_BYTE );
		if ( p == NULL ) return FALSE;
		DC_FlushRange( p, NAND_BLOCK_BYTE );
		if ( kamiNandRead( NAND_RTC_BLOCK, p, 1 ) == KAMI_RESULT_SEND_ERROR) return FALSE;
		if( STD_TSScanf( (const char *)p, "RTC CORRECTION=%d", &value ) < 0 ) return FALSE;
		Info.rtcAdjust = (u8)value;
		{
#define TEST_OUTFILE_PATH	"sdmc:/nanddump.sbin"
			FSFile file;
		    (void)FS_DeleteFile( (char *)TEST_OUTFILE_PATH );
		    if( !FS_CreateFile( TEST_OUTFILE_PATH, FS_PERMIT_R | FS_PERMIT_W ) ) return FALSE;
		    FS_InitFile( &file );
		    if( !FS_OpenFileEx( &file, TEST_OUTFILE_PATH, FS_FILEMODE_W ) ) return FALSE;
			if( !FS_WriteFile( &file, p, NAND_BLOCK_BYTE ) ) return FALSE;
			(void)FS_CloseFile( &file );
		}
		OS_Free( p );
	}
#else
	{
		// HWI_Init関数でメモリに展開されたHWNormalInfoを使えば、NAND上のHWNormalInfoが消去されていても大丈夫
	    OSTWLHWNormalInfo *p = (OSTWLHWNormalInfo*)HW_PARAM_TWL_HW_NORMAL_INFO;
		Info.rtcAdjust = p->rtcAdjust;
	}
#endif
	{
		int i;
		u8 serialNo[ LCFG_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN ];
		
		MI_CpuClear8( serialNo, LCFG_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN );
		OS_GetSerialNo( serialNo );
		
		for( i = 0; i < LCFG_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN; i++ ) {
			Info.movableUniqueID[ i ] = (u8)( serialNo[ i ] ^ s_serialNo_mask[ i ] );
		}
	}

    if (!LCFGi_THW_WriteNormalInfoDirect( &Info ))
    {
        OS_TPrintf( "HW Normal Info Write failed.\n" );
        return FALSE;
    }

	// MMEMのシステム領域にセット
	MI_CpuCopyFast( &Info, (void *)HW_PARAM_TWL_HW_NORMAL_INFO, sizeof(LCFGTWLHWNormalInfo) );

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWSecureInfoFile

  Description:  HWセキュアInfoファイルのライト

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL HWI_WriteHWSecureInfoFile( u8 region, const u8 *pSerialNo, BOOL isDisableWireless, BOOL logoDemoSkipForce )
{
    BOOL isWrite = TRUE;
    LCFGReadResult result;
	u8 old_region;

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
    LCFG_THW_SetFlagForceLogoDemoSkip( logoDemoSkipForce );

	// 旧リージョンを保存
	old_region = OS_GetRegion();

    // リージョンのセット
    LCFG_THW_SetRegion( region );

    // 対応言語ビットマップのセット
    LCFG_THW_SetValidLanguageBitmap( s_langBitmapList[ region ] );

    // シリアルNo.のセット
    if( pSerialNo != NULL ) 
	{
        LCFG_THW_SetSerialNo( pSerialNo );
    }
	else
	{
        // 量産工程でないとシリアルNo.は用意できないので、ここではMACアドレスをもとに適当な値をセットする。
        u8 buffer[ 12 ] = "SERIAL";     // 適当な文字列をMACアドレスと結合してSHA1を取り、仮SerialNoとする。
        u8 serialNoOld[ SVC_SHA1_DIGEST_SIZE ];
		u8 serialNoNew[ SVC_SHA1_DIGEST_SIZE ];
		u8 sha1_buffer[ SVC_SHA1_DIGEST_SIZE ];
        int i,j;
        int len;
		int offset;

		if ( region == OS_TWL_REGION_AMERICA ) 
		{
			len    = LCFG_TWL_HWINFO_SERIALNO_LEN_AMERICA;
			offset = LCFG_TWL_HWINFO_SERIALNO_UNIQUE_OFFSET_AMERICA;
		}
		else
		{
            len    = LCFG_TWL_HWINFO_SERIALNO_LEN_OTHERS;
			offset = LCFG_TWL_HWINFO_SERIALNO_UNIQUE_OFFSET_OTHERS;
		}

		// 現在のシリアルNo取得
		OS_GetSerialNo( serialNoOld );

		// 新しいシリアルNoをクリアしておく
		MI_CpuClear8( serialNoNew, sizeof(serialNoNew) );

		// シリアルNoの先頭が'T'(量産用UTL)、'V'(開発用TWL)、'W'(量産用UTL)、'D'(開発用UTL)
		// でなければ不正なので仮のシリアルNo.を作成する
		if ( serialNoOld[0] != 'T' && serialNoOld[0] != 'V' && serialNoOld[0] != 'W' && serialNoOld[0] != 'D' )
		{
			// 1バイト目は量産用なら'T'、その他なら'V'
			// TWLorUTLは判別できない
			if (SCFG_ReadBondingOption() == SCFG_OP_PRODUCT)
			{
				serialNoNew[0] = 'T';
			}
			else
			{
				serialNoNew[0] = 'V';
			}
			// 2バイト目はリージョン別ASCII
			serialNoNew[1] = (u8)regionAsciiForSerialNo[region];			
			// 米国リージョン以外は3バイト目にEMS（仮シリアルNo.なので任天堂の'N'）
			if ( region != OS_TWL_REGION_AMERICA )
			{
				serialNoNew[2] = 'N';
			}

			// 数字8桁
	        OS_GetMacAddress( buffer + 6 );
	        SVC_CalcSHA1( sha1_buffer, buffer, sizeof(buffer) );
	        for( i=offset,j=0; i < len-1; i++,j++ ) {
	            serialNoNew[ i ] = (u8)( ( sha1_buffer[ j ] % 10 ) + 0x30 );
	        }

			// チェックコード取得
			serialNoNew[len-1] = HWIi_CalcCheckCode(&serialNoNew[offset]);

			// 仮シリアルNo.であることの印として14バイト目を'K'とする
			serialNoNew[13] = 'K';
		}
		// シリアルNoの先頭が'T'(量産用UTL)、'V'(開発用TWL)、'W'(量産用UTL)、'D'(開発用UTL)
		// である場合先頭文字とユニーク数字８桁はそのままで他を変更する
		else
		{
			// 1バイト目はキープ
			serialNoNew[0] = serialNoOld[0];

			// 2バイト目はリージョン別ASCII
			serialNoNew[1] = (u8)regionAsciiForSerialNo[region];
			// 米国リージョン以外は3バイト目にEMS
			if ( region != OS_TWL_REGION_AMERICA )
			{
				// EMS情報が存在するなら引き継ぐ
				if ( old_region != OS_TWL_REGION_AMERICA )
				{
					serialNoNew[2] = serialNoOld[2];
				}
				// EMS情報が存在しないため仮に任天堂の'N'とする
				else
				{
					serialNoNew[2] = 'N';
				}
			}

			// ユニーク数字をコピー
			if ( old_region == OS_TWL_REGION_AMERICA )
			{
				MI_CpuCopy( &serialNoOld[LCFG_TWL_HWINFO_SERIALNO_UNIQUE_OFFSET_AMERICA], &serialNoNew[offset], LCFG_TWL_HWINFO_SERIALNO_UNIQUE_LENGTH );
			}
			else
			{
				MI_CpuCopy( &serialNoOld[LCFG_TWL_HWINFO_SERIALNO_UNIQUE_OFFSET_OTHERS], &serialNoNew[offset], LCFG_TWL_HWINFO_SERIALNO_UNIQUE_LENGTH );
			}

			// チェックコード取得
			serialNoNew[len-1] = HWIi_CalcCheckCode(&serialNoNew[offset]);

			// 14バイト目は旧から新へコピー
			serialNoNew[13] = serialNoOld[13];
		}

     	OS_TPrintf( "serialNo : %s\n", serialNoNew );

        LCFG_THW_SetSerialNo( serialNoNew );
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
        !LCFGi_THW_WriteSecureInfo( s_pPrivKeyBuffer, s_privKeyBufferLen ) ) {
        isWrite = FALSE;
        OS_TPrintf( "HW Secure Info Write failed.\n" );
    }

	// MMEMのシステム領域にセット
	MI_CpuCopyFast( LCFGi_GetHWS(), (void *)HW_HW_SECURE_INFO, HW_HW_SECURE_INFO_END - HW_HW_SECURE_INFO );

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
    BOOL retval = TRUE;

    (void)FS_DeleteFile( (char *)LCFG_TWL_HWID_SIGN_PATH );
    if( !FS_CreateFile( LCFG_TWL_HWID_SIGN_PATH, FS_PERMIT_R | FS_PERMIT_W ) ) {
        OS_TPrintf( "file create error. %s\n", LCFG_TWL_HWID_SIGN_PATH );
    }
    retval &= LCFG_WriteHWID_Signature( s_pPrivKeyBufferHWID, s_privKeyBufferLenHWID );
	retval &= CheckHWID_Signature( FALSE );
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

/*---------------------------------------------------------------------------*
  Name:         HWIi_CalcCheckCode

  Description:  シリアル番号の数字８桁のチェックコードを計算します

  Arguments:    Ascii 数字文字列

  Returns:      Ascii 数字
 *---------------------------------------------------------------------------*/
static u8 HWIi_CalcCheckCode(u8 src[8])
{
	// モジュラス10 ウェイト3･1（M10W31）
	int temp = (src[7] - '0')*3 +
               (src[6] - '0')*1 +
               (src[5] - '0')*3 +
               (src[4] - '0')*1 +
               (src[3] - '0')*3 +
               (src[2] - '0')*1 +
               (src[1] - '0')*3 +
               (src[0] - '0')*1;
	temp %= 10;
	if (temp > 0)
	{
		temp = 10-temp;
	}

	return (u8)('0' + temp);
}

