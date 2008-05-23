/*---------------------------------------------------------------------------*
  Project:  TWL_RED_IPL - 
  File:     loadWlanFirm.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-02-15#$
  $Rev: 677 $
  $Author: sato_masaki $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/nam.h>
#include <twl/os/common/format_rom.h>
#include <twl/lcfg.h>
#include <twl/nwm/ARM9/ForLauncher/nwm_init_for_launcher.h>
#include <nitro/nvram/nvram.h>

#include <firm.h>
#include <sysmenu.h>

#include "nwm_common_private.h"
#include "nwm_arm9_private.h"

#include "loadWlanFirm.h"
#include "scanWDS.h"

/*
  definitions
 */

/* LCFGの無線ファームバージョンをタイトルＩＤとしてそのまま使う場合 */
#define USE_LCFG_STRING              0

/* 無線FWダウンロード処理にかかる時間を計測する。 */
#define MEASURE_WIRELESS_INITTIME    0

/* 無線FW認証処理にかかる時間を計測する。 */
#define MEASURE_VERIFY_SIGN_TIME     0

/* ハッシュ比較の情報を出力する。 */
#define REPORT_HASH_COMPARISON       0

/* Index of public key for WLAN firm */
#define WLANFIRM_PUBKEY_INDEX        1
#define SIGN_LENGTH                  128

#define SIGNHEAP_SIZE                0x01000


/*
  internal variables
 */
static BOOL				s_isHotStartWLFirm;
static volatile BOOL	s_isFinished;
static u32*             pNwmBuf;
static u8*              pFwBuffer = 0;
#if (MEASURE_WIRELESS_INITTIME == 1)
static OSTick           startTick = 0;
#endif
static OSMessageQueue   mesq;
static OSMessage        mesAry[1];

static u8 fwType; // must be in main memory

/*
    internal functions
 */
static void  InstallFirmCallback(void* arg);
static BOOL  GetFirmwareFilepath(char *path);
static s32   ReadFirmwareSecurityArea(char *path, u8 *buffer, s32 bufSize);
static s32   ReadFirmwareHeader(char *path, u8 *buffer, s32 bufSize);
static s32   ReadFirmwareBinary(char *path, u32 offset, u8 *buffer, s32 bufSize);
static BOOL  VerifyWlanfirmSignature(u8* buffer, u32 length);
static BOOL  CheckHash(const u8* hash, const u8* buffer, u32 length);
#if (REPORT_HASH_COMPARISON == 1)
static void PrintDigest(u8 *digest);
#endif


void InstallFirmCallback(void* arg)
{
    NWMCallback *cb = (NWMCallback*)arg;
    WLANFirmResult result;

    if (cb->retcode == NWM_RETCODE_SUCCESS) {
#if (MEASURE_WIRELESS_INITTIME == 1)
        OS_TPrintf("[Wlan Firm]  LoadTime=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - startTick));
#endif
        OS_TPrintf("[Wlan Firm]  Wlan firmware has been installed successfully!\n");
        result = WLANFIRM_RESULT_SUCCESS;
    } else { // in case of failure
        OS_TPrintf("[Wlan Firm]  FW download Timeout Error!\n");
        result = WLANFIRM_RESULT_FAILURE;
    }
    
    if (pFwBuffer) {
        SYSM_Free( pFwBuffer );
        pFwBuffer = 0;
    }
    if (pNwmBuf)
    {
        NWM_End();
        SYSM_Free( pNwmBuf );
        pNwmBuf = 0;
    }
    /* メッセージキューにFWダウンロードの結果を通知 */
    // [TODO:] queue溢れはありえないハズだけど、一応対策しておく予定。
    (void)OS_SendMessage(&mesq, (OSMessage)result, OS_MESSAGE_NOBLOCK);

}

BOOL GetFirmwareFilepath(char *path)
{
	u8 title[4] = { 'H','N','C','A' };

#if( USE_LCFG_STRING == 0 )
    char *title0 = "HNCA";
#endif
    u32 titleID_hi;
    u32 titleID_lo;
    u64 titleID = 0;


#if( USE_LCFG_STRING == 0 )
    {
        int i;
        if( title[0] == 0 ) {
            for( i = 0 ; i < 4 ; i++ ) {
                title[i] = (u8)*title0++;
            }
        }
    }
#endif


    titleID_hi = (( 3 /* Nintendo */ << 16) | 8 /* CHANNEL_DATA_ONLY */ | 4 /* CHANNEL_CARD */ | 2 /* isLaunch */ | 1 /* isSystem */);

    titleID_lo =  ((u32)( title[0] ) & 0xff) << 24;
    titleID_lo |= ((u32)( title[1] )& 0xff) << 16;
    titleID_lo |= ((u32)( title[2] )& 0xff) << 8;
    titleID_lo |= (u32)( title[3] ) & 0xff;

    titleID = ((u64)(titleID_hi) << 32)  | (u64)titleID_lo;

    // OS_TPrintf( "[Wlan Firm]  titleID = 0x%08x%08x\n", titleID_hi, titleID_lo);

    if( NAM_OK == NAM_GetTitleBootContentPathFast(path, titleID) ) {
        OS_TPrintf( "[Wlan Firm]  File = %s\n", path);
    }
    else {
        OS_TPrintf( "[Wlan Firm]  Error: NAM_GetTitleBootContentPathFast titleID = 0x%08x0x%08x\n",titleID_hi, titleID_lo);
        return FALSE;
    }
    
    return TRUE;

}

s32 ReadFirmwareSecurityArea(char *path, u8 *buffer, s32 bufSize)
{
    return ReadFirmwareBinary(path, 0, buffer, bufSize);
}

s32 ReadFirmwareHeader(char *path, u8 *buffer, s32 bufSize)
{
    return ReadFirmwareBinary(path, NWM_FW_SECURITY_AREA_SIZE, buffer, bufSize);
}
    
s32 ReadFirmwareBinary(char *path, u32 offset, u8 *buffer, s32 bufSize)
{
    FSFile  file[1];
    s32 flen;

    FS_InitFile( file );
    
    if (!FS_OpenFileEx(file, path, FS_FILEMODE_R)) {
        OS_TWarning("FS_OpenFileEx(%s) failed.\n", path);
        return -1;
    }

    if( FALSE == FS_SeekFile(file, (s32)(offset), FS_SEEK_SET) ) {
        OS_TWarning("FS_SeekFile failed.\n");
        return -1;
    }
    
    flen = FS_ReadFile(file, buffer, bufSize);
    if( flen == -1 ) {
        OS_TWarning("FS_ReadFile failed.\n");
        return -1;
    }

    (void)FS_CloseFile(file);

    return flen;
}

//#define USE_LOCAL_PUBKEY
#ifdef USE_LOCAL_PUBKEY
static const u8 s_pubkey9_1[ 0x80 ] = {
	0xb6, 0x18, 0xd8, 0x61, 0x28, 0xcb, 0x5c, 0x6f, 0x05, 0xfc, 0xd7, 0x09, 0x18, 0x3f, 0xb2, 0xd0, 
	0x6b, 0x7d, 0xee, 0xd9, 0x98, 0xdc, 0x4f, 0xdd, 0xc1, 0xa8, 0x59, 0x18, 0xfb, 0xb0, 0x65, 0xbd, 
	0x65, 0x80, 0x9c, 0xc7, 0x68, 0xa1, 0x4e, 0xdc, 0x18, 0xaa, 0x7b, 0xcb, 0xb9, 0xa0, 0x7c, 0xfc, 
	0x1f, 0xab, 0x86, 0x5d, 0xed, 0x9c, 0x2c, 0x5c, 0x6d, 0x07, 0xd9, 0xfc, 0xc2, 0x9b, 0x7a, 0x9d, 
	0x7c, 0x3a, 0x73, 0x33, 0xb7, 0xe8, 0x04, 0x86, 0x81, 0xc8, 0x5c, 0x7d, 0xb3, 0x95, 0x7d, 0xc9, 
	0xec, 0x66, 0x07, 0x2f, 0x8b, 0xb2, 0x6d, 0x13, 0xc4, 0x6c, 0xf0, 0xba, 0x27, 0x82, 0x33, 0x18, 
	0xd4, 0x31, 0x6a, 0xb2, 0xad, 0xbc, 0x37, 0x06, 0x6a, 0x2e, 0xe9, 0x73, 0x5f, 0x3a, 0x57, 0xc7, 
	0xd7, 0xf8, 0x8e, 0xc1, 0xb9, 0x3d, 0x3f, 0xd4, 0xe5, 0x27, 0x6f, 0xb4, 0x00, 0x8b, 0xb7, 0x19, 
};
#endif

BOOL VerifyWlanfirmSignature(u8* buffer, u32 length)
{
#pragma unused(length)
    NWMFirmSecurityArea *hdr = (NWMFirmSecurityArea*)buffer;
    u8 *pPubkey;
    u8 *pSign;
    u8 *txt;
    u32 txtlen;
    u8 txtDigest[SVC_SHA1_DIGEST_SIZE];
    u8 signDigest[SVC_SHA1_DIGEST_SIZE];
    SVCSHA1Context sctx;
    SVCSignHeapContext rctx;
    u8*   signHeap;
#if (MEASURE_VERIFY_SIGN_TIME == 1)
    OSTick vstart = OS_GetTick();
#endif

#ifdef USE_LOCAL_PUBKEY
	// ランチャー経由でのデバッガ起動では、鍵情報を受け取ることができない。
	// よってリリースビルドの時は、デバッグ動作を優先して鍵を自分で持つ。
	pPubkey = (u8 *)s_pubkey9_1;
#else
    pPubkey = OSi_GetFromFirmAddr()->rsa_pubkey[WLANFIRM_PUBKEY_INDEX];
#endif
    pSign = (u8*)hdr->sign;

    txt = (u8*)hdr->hash;
    txtlen = (u32)NWM_FW_SECURITY_AREA_SIZE - SIGN_LENGTH; /* 署名を除いたSecurity Areaのサイズ */

    /* calculate SHA-1 digest */
    SVC_SHA1Init( &sctx );
    SVC_SHA1Update( &sctx, (const void*)txt, txtlen);
    SVC_SHA1GetHash( &sctx, txtDigest );

#if (REPORT_HASH_COMPARISON == 1)
    OS_TPrintf("[Wlan Firm]  Wlan Firm digest: ");
    PrintDigest((u8*)txtDigest);
#endif

    /* decrypt according to RSA security */
    signHeap = SYSM_Alloc( SIGNHEAP_SIZE );
    SVC_InitSignHeap( &rctx, signHeap, SIGNHEAP_SIZE);
    
    MI_CpuClear8( signDigest, SVC_SHA1_DIGEST_SIZE );

    if (FALSE == SVC_DecryptSign( &rctx, signDigest, (const void*)pSign, (const void*)pPubkey ))
    {
        OS_TPrintf("[Wlan Firm]  !!!! Wlan Firmware authentication has failed !!!!\n");

#ifdef IGNORE_WLFIRM_SIGNCHECK
        OS_TPrintf("[Wlan Firm]  But this failure is ignored.\n");
        if ( 0 )
#endif
        {
            SYSM_Free(signHeap);
            return FALSE;
        }
    }

    SYSM_Free(signHeap);

#if (REPORT_HASH_COMPARISON == 1)
    OS_TPrintf("[Wlan Firm]  Decrypted digest: ");
    PrintDigest((u8*)signDigest);
#endif
    
    /* verify digest */
    if (FALSE == SVC_CompareSHA1( (const void*)txtDigest, (const void*)signDigest ))
    {
        OS_TPrintf("[Wlan Firm]  !!!! Digest verification failed !!!!\n");
#ifdef IGNORE_WLFIRM_SIGNCHECK
        OS_TPrintf("[Wlan Firm]  But this failure is ignored.\n");
        if ( 0 )
#endif
        {
            return FALSE;
        }
    }

#if (MEASURE_VERIFY_SIGN_TIME == 1)
    OS_TPrintf("[Wlan Firm]  Verify signature Time=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - vstart));
#endif
    
    return TRUE;

}

BOOL CheckHash(const u8* hash, const u8* buffer, u32 length)
{
    u8 txtDigest[SVC_SHA1_DIGEST_SIZE];
    SVCSHA1Context sctx;
#if (MEASURE_VERIFY_SIGN_TIME == 1)
    OSTick vstart = OS_GetTick();
#endif

#if (REPORT_HASH_COMPARISON == 1)
    OS_TPrintf("[Wlan Firm]  Digest to compare: ");
    PrintDigest((u8*)hash);
#endif
    
    /* calculate SHA-1 digest */
    SVC_SHA1Init( &sctx );
    SVC_SHA1Update( &sctx, (const void*)buffer, length);
    SVC_SHA1GetHash( &sctx, txtDigest );

#if (REPORT_HASH_COMPARISON == 1)
    OS_TPrintf("[Wlan Firm]  Calculated digest: ");
    PrintDigest((u8*)txtDigest);
#endif
    
    /* verify digest */
    if (FALSE == SVC_CompareSHA1( (const void*)hash, (const void*)txtDigest ))
    {
#if (MEASURE_VERIFY_SIGN_TIME == 1)
    OS_TPrintf("[Wlan Firm]  Verify digest Time=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - vstart));
#endif
        return FALSE;
    }
#if (MEASURE_VERIFY_SIGN_TIME == 1)
    OS_TPrintf("[Wlan Firm]  Verify digest Time=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - vstart));
#endif
    return TRUE;
}

#if (REPORT_HASH_COMPARISON == 1)
void PrintDigest(u8 *digest)
{
    int i;
    
    for (i = 0; i < SVC_SHA1_DIGEST_SIZE; i++ )
    {
        OS_TPrintf("%02X ", digest[i]);
    }
    OS_TPrintf("\n");
}
#endif


BOOL InstallWlanFirmware( BOOL isHotStartWLFirm )
{
    NWMRetCode err;
    NWMFirmDataParam *pFdParam = (NWMFirmDataParam *)NWM_PARAM_FWDATA_ADDRESS;
    NVRAMResult nvRes;
    u8 *pSecBuf = NULL;
    u8 *pHdrBuf = NULL;

    s_isFinished = FALSE;
    pNwmBuf = NULL;
    pFwBuffer = NULL;

    OS_InitMessageQueue(&mesq, mesAry, sizeof(mesAry)/sizeof(mesAry[0]));

    /* Read FW type from NVRAM */
    nvRes = NVRAMi_Read(NWM_NVR_FWTYPE_OFFSET_ADDRESS, 1, &fwType );

    if (nvRes != NVRAM_RESULT_SUCCESS)
    {
        OS_TWarning("Error: Couldn't access NVRAM.\n");
        goto instfirm_error;
    }

    if (fwType == 0xFF)
    {
        // NVRAMの該当領域が0xFF(何も書かれていない)の場合は、fwType=0として扱う。
        OS_TWarning("Firmware Type has not been found in NVRAM.\n");
        fwType = 0;
    }

    pFdParam->fwType = fwType;

    OS_TPrintf("[Wlan Firm]  FWtype is %d\n", fwType);

    /* HotStart/ColdStartのチェック */

    s_isHotStartWLFirm = isHotStartWLFirm;

    if (TRUE == isHotStartWLFirm)  // HOT START
    {
        pNwmBuf = SYSM_Alloc( NWM_SYSTEM_BUF_SIZE );
        if (!pNwmBuf) {
            OS_TWarning("Error: Couldn't allocate memory for NWM.\n");
            goto instfirm_error;
        }

#if (MEASURE_WIRELESS_INITTIME == 1)
        startTick = OS_GetTick();
#endif

        // HotStart
        OS_TPrintf("[Wlan Firm]  Start InstallFirmware (HOT START)\n");
        NWMi_InitForLauncher(pNwmBuf, NWM_SYSTEM_BUF_SIZE, 3); /* 3 -> DMA no. */
        err = NWMi_InstallFirmware(InstallFirmCallback, NULL, 0, FALSE);

    } else {    // COLD START
        s32 flen = 0;
        char path[256];
        u32 fwOffset, fwLen, hdrLen;
        u8 *pHash = NULL;

        // Get Filepath
        if (FALSE == GetFirmwareFilepath(path)) {
            goto instfirm_error;
        }

        /* ------------------------------------
           Stage 1 -- Security Area の署名認証
           ------------------------------------ */

        // Read Security area of WLAN firm
        /* Allocate security area buffer from heap. */
        pSecBuf = SYSM_Alloc( NWM_FW_SECURITY_AREA_SIZE );

        if (!pSecBuf) {
            OS_TWarning("[Wlan Firm]  Error: Couldn't allocate memory for Security Area.\n");
            goto instfirm_error;
        }

        flen = ReadFirmwareSecurityArea(path, pSecBuf, NWM_FW_SECURITY_AREA_SIZE);

        if ( 0 >= flen )
        {
            OS_TPrintf("[Wlan Firm]  Error: Couldn't read wlan firmware security area.\n");
            goto instfirm_error;
        }

        // Check signature data
        if (FALSE == VerifyWlanfirmSignature(pSecBuf, (u32)flen))
        {
            OS_TPrintf("[Wlan Firm]  Error: This Wlan Firmware is quite illegal!\n");
            OS_TPrintf("[Wlan Firm]         It has never been installed.\n");
            goto instfirm_error;
        }

        /* ------------------------------------
           Stage 2 -- Header Area のHashチェック
           ------------------------------------ */
        
        hdrLen = ((NWMFirmSecurityArea*)pSecBuf)->hdrLen;

        /* Allocate header area buffer from heap. */
        pHdrBuf = SYSM_Alloc( hdrLen );

        if (!pHdrBuf) {
            OS_TWarning("[Wlan Firm]  Error: Couldn't allocate memory for Header area.\n");
            goto instfirm_error;
        }

        // Read header of WLAN firm
        flen = ReadFirmwareHeader(path, pHdrBuf, (s32)hdrLen);

        if ( 0 >= flen )
        {
            OS_TPrintf("[Wlan Firm]  Error: Couldn't read wlan firmware header.\n");
            goto instfirm_error;
        }

        pHash = (u8*)pSecBuf + SIGN_LENGTH;
        
        OS_TPrintf("[Wlan Firm]  Check hash of Header Area.\n");
        if (FALSE == CheckHash((const u8*)pHash, (const u8*)pHdrBuf, hdrLen))
        {
            OS_TPrintf("[Wlan Firm]  Error: Header Hash data is illegal.\n");
            goto instfirm_error;
        }
        OS_TPrintf("[Wlan Firm]  Header Area CheckHash ok.\n");

        // Free Security area buffer
        SYSM_Free( pSecBuf );
        pSecBuf = NULL;
        
        /* ------------------------------------
           Stage 3 -- FW image Area のHashチェック
           ------------------------------------ */
        
        // Find corresponding FW image
        fwOffset = NWMi_GetFirmImageOffset(pHdrBuf, (u32)fwType);
        fwLen    = NWMi_GetFirmImageLength(pHdrBuf, (u32)fwType);

        if (fwOffset == 0 || fwLen == 0) {
            OS_TPrintf("[Wlan Firm]  Error: Couldn't get FW image.\n");
            goto instfirm_error;
        }

        /* Allocate FW buffer from heap. */
        pFwBuffer = SYSM_Alloc( fwLen );
        if (!pFwBuffer) {
            OS_TWarning("[Wlan Firm]  Error: Couldn't allocate memory for FW image.\n");
            goto instfirm_error;
        }

        // Read FW image
        flen = ReadFirmwareBinary(path, fwOffset, pFwBuffer, (s32)fwLen);

        if ( 0 >= flen )
        {
            OS_TPrintf("[Wlan Firm]  Error: Couldn't read wlan firmware.\n");
            goto instfirm_error;
        }

        // Compare hashes
        pHash = NWMi_GetFirmImageHashAddress(pHdrBuf, (u32)fwType);
        if (pHash == NULL)
        {
            OS_TPrintf("[Wlan Firm]  Error: Couldn't get hash of wlan firmware image.\n");
            goto instfirm_error;
        }

        OS_TPrintf("[Wlan Firm]  Check hash of FW image.\n");
        if (FALSE == CheckHash((const u8*)pHash, (const u8*)pFwBuffer, fwLen))
        {
            OS_TPrintf("[Wlan Firm]  Error: FW image Hash data is illegal.\n");
            goto instfirm_error;
        }
        OS_TPrintf("[Wlan Firm]  FW image CheckHash ok.\n");

        // Free Header area buffer
        SYSM_Free( pHdrBuf );
        pHdrBuf = NULL;

        pNwmBuf = SYSM_Alloc( NWM_SYSTEM_BUF_SIZE );
        if (!pNwmBuf) {
            OS_TWarning("Error: Couldn't allocate memory for NWM.\n");
            goto instfirm_error;
        }

        // Start FW installation
        NWMi_InitForLauncher(pNwmBuf, NWM_SYSTEM_BUF_SIZE, 3); /* 3 -> DMA no. */

#if (MEASURE_WIRELESS_INITTIME == 1)
        startTick = OS_GetTick();
#endif

        OS_TPrintf("[Wlan Firm]  Start InstallFirmware (COLD START)\n");
        err = NWMi_InstallFirmware(InstallFirmCallback, pFwBuffer, (u32)flen, TRUE);

    }

    /*
        無線ロード処理の完了は、IsWlanFirmwareInstalledでチェックする。
     */

    return TRUE;

    /* エラー処理 */
instfirm_error:
    // Free Security area buffer
    if (pSecBuf)
    {
        SYSM_Free( pSecBuf );
        pSecBuf = NULL;
    }
    // Free Header area buffer
    if (pHdrBuf)
    {
        SYSM_Free( pHdrBuf );
        pHdrBuf = NULL;
    }
    
    if (pFwBuffer)
    {
        SYSM_Free( pFwBuffer );
        pFwBuffer = NULL;
    }
    if (pNwmBuf)
    {
        NWM_End();
        SYSM_Free( pNwmBuf );
        pNwmBuf = NULL;
    }

    // インストール開始すらできなかった時は、FATALエラー
#ifdef SDK_RELEASE
    PMi_SetWirelessLED( PM_WIRELESS_LED_OFF );
#endif
    s_isFinished = TRUE;
    SYSM_SetFatalError( TRUE );

    return FALSE;
}

BOOL GetWlanFirmwareInstallResult(WLANFirmResult *pResult);
BOOL GetWlanFirmwareInstallResult(WLANFirmResult *pResult)
{
    OSMessage msg;
    BOOL retval;

    retval =  OS_ReadMessage(&mesq, &msg, OS_MESSAGE_NOBLOCK);

    *pResult = (WLANFirmResult)msg;

    return retval;
}

// 無線ファームロード完了？
BOOL PollingInstallWlanFirmware( BOOL isStartScanWDS )
{
#ifndef ENABLE_WDS_SCAN
#pragma unused(isStartScanWDS)
#endif
	if ( !s_isFinished ) {
		WLANFirmResult result;
		if( GetWlanFirmwareInstallResult( &result ) ) {
			if( result == WLANFIRM_RESULT_SUCCESS ) {
				OS_TPrintf( "WLFIRM load finished.\n" );
#ifndef DISABLE_WDS_SCAN
				// WDSスキャンがTRUE かつ 無線フラグがONならば、引き続きWDSビーコン受信開始
				if( isStartScanWDS &&
					!LCFG_THW_IsForceDisableWireless() && LCFG_TSD_IsAvailableWireless() ) {
					StartScanWDS();
				}
#endif // DISABLE_WDS_SCAN
			}else {
				// ロード失敗
				if( !s_isHotStartWLFirm ) {
					// ColdStartの無線ファームロードなら、FATALエラー
			        SYSM_SetFatalError( TRUE );
#ifdef SDK_RELEASE	
					PMi_SetWirelessLED( PM_WIRELESS_LED_OFF );
#endif
					s_isFinished = TRUE;
				}else {
					// そうでない場合は、ColdStartロードで再度実行。
					(void)InstallWlanFirmware( FALSE );
					OS_TPrintf( "WLFIRM HotStart load failed... Start retry.\n" );
				}
			}
			s_isFinished = TRUE;
		}
	}
	return s_isFinished;
}


