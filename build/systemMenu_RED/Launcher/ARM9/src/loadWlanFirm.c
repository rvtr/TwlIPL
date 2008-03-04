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

#include <firm.h>
#include <sysmenu.h>
#include "loadWlanFirm.h"

/*
  definitions
 */

/* LCFGの無線ファームバージョンをタイトルＩＤとしてそのまま使う場合 */
#define USE_LCFG_STRING              0
/* 無線FWダウンロード処理にかかる時間を計測する。 */
#define MEASURE_WIRELESS_INITTIME    1
/* 無線FW認証処理にかかる時間を計測する。 */
#define MEASURE_VERIFY_SIGN_TIME     1

/* Index of public key for WLAN firm */
#define WLANFIRM_PUBKEY_INDEX        1
#define SIGN_LENGTH                  128

#define FWBUFFER_SIZE                0x40000
#define SIGNHEAP_SIZE                0x01000

/*
  internal variables
 */

static u32*             pNwmBuf;
static u8*              pFwBuffer = 0;
#if (MEASURE_WIRELESS_INITTIME == 1)
static OSTick           startTick;
#endif
static OSMessageQueue   mesq;
static OSMessage        mesAry[1];

/*
    internal functions
 */
static void  InstallFirmCallback(void* arg);
static s32   ReadFirmwareBinary(u8 *buffer, s32 bufSize);
static BOOL  VerifyWlanfirmSignature(u8* buffer, u32 length);


extern NWMRetCode NWMi_InstallFirmware(NWMCallbackFunc callback, void* addr, u32 size, BOOL isColdstart);



void InstallFirmCallback(void* arg)
{
    NWMCallback *cb = (NWMCallback*)arg;
    WLANFirmResult result;

    if (cb->retcode == NWM_RETCODE_SUCCESS) {
#if (MEASURE_WIRELESS_INITTIME == 1)
        OS_TPrintf("Wlan firm:LoadTime=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - startTick));
#endif
        OS_TPrintf("Wlan firm:Wlan firmware has been installed successfully!\n");
        result = WLANFIRM_RESULT_SUCCESS;
    } else { // in case of failure
        OS_TPrintf("Wlan firm:FW download Timeout Error!\n");
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

s32 ReadFirmwareBinary(u8 *buffer, s32 bufSize)
{
    char path[256];
    FSFile  file[1];
    u8 title[4];
    s32 flen;

#if( USE_LCFG_STRING == 0 )
    char *title0 = "WFW0";
    char *title1 = "WFW1";
#endif
    u32 titleID_hi;
    u32 titleID_lo;
    u64 titleID = 0;

    LCFG_THW_GetWirelessFirmTitleID_Lo( title );

#if( USE_LCFG_STRING == 0 )
    {
        int i;
        if( title[0] == 0 ) {
            for( i = 0 ; i < 4 ; i++ ) {
                title[i] = (u8)*title1++;
            }
        }
        else {
            for( i = 0 ; i < 4 ; i++ ) {
                title[i] = (u8)*title0++;
            }
        }
    }
#endif


    titleID_hi = (( 3 /* Nintendo */ << 16) | 4 /* */ | 2 /* */ | 1 /* */);

    titleID_lo =  ((u32)( title[0] ) & 0xff) << 24;
    titleID_lo |= ((u32)( title[1] )& 0xff) << 16;
    titleID_lo |= ((u32)( title[2] )& 0xff) << 8;
    titleID_lo |= (u32)( title[3] ) & 0xff;

    titleID = ((u64)(titleID_hi) << 32)  | (u64)titleID_lo;

    // OS_TPrintf( "titleID = 0x%08x%08x\n", titleID_hi, titleID_lo);

    if( NAM_OK == NAM_GetTitleBootContentPathFast(path, titleID) ) {
        OS_TPrintf( "File = %s\n", path);
    }
    else {
        OS_TPrintf( "Error: NAM_GetTitleBootContentPathFast titleID = 0x%08x0x%08x\n",titleID_hi, titleID_lo);
        return -1;
    }

    if (!FS_OpenFileEx(file, path, FS_FILEMODE_R)) {
        OS_TWarning("FS_OpenFileEx(%s) failed.\n", path);
        return -1;
    }

    if( FALSE == FS_SeekFile(file, sizeof(ROM_Header), FS_SEEK_SET) ) {
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

BOOL VerifyWlanfirmSignature(u8* buffer, u32 length)
{
    NWMFirmHeader *hdr = (NWMFirmHeader*)buffer;
    u8 *pPubkey;
    u8 *pSign;
    u8 *txtVector[2];
    u32 txtlenVector[2];
    u8 txtDigest[SVC_SHA1_DIGEST_SIZE];
    u8 signDigest[0x80];
    SVCSHA1Context sctx;
    SVCSignHeapContext rctx;
    int i;
    u8*   signHeap;
#if (MEASURE_VERIFY_SIGN_TIME == 1)
    OSTick vstart = OS_GetTick();
#endif
    
    pPubkey = OSi_GetFromFirmAddr()->rsa_pubkey[WLANFIRM_PUBKEY_INDEX];
    pSign = (u8*)((u32)buffer + (u32)hdr->sofs);

    txtVector[0] = buffer;
    txtlenVector[0] = (u32)hdr->sofs; /* 署名の直前までのLength */
    txtVector[1] = (u8*)(txtVector[0] + txtlenVector[0] + (u32)SIGN_LENGTH);
    txtlenVector[1] = length - txtlenVector[0] - (u32)SIGN_LENGTH;

    /* calculate SHA-1 digest */
    SVC_SHA1Init( &sctx );
    for (i = 0; i < 2; i++ )
    {
        SVC_SHA1Update( &sctx, (const void*)txtVector[i],txtlenVector[i]);
    }
    SVC_SHA1GetHash( &sctx, txtDigest );

    OS_TPrintf("Wlan Firm digest: ");
    for (i = 0; i < SVC_SHA1_DIGEST_SIZE; i++ )
    {
        OS_TPrintf("%02X ", txtDigest[i]);
    }
    OS_TPrintf("\n");

    /* decrypt according to RSA security */
    signHeap = SYSM_Alloc( SIGNHEAP_SIZE );
    SVC_InitSignHeap( &rctx, signHeap, SIGNHEAP_SIZE);
    
    MI_CpuClear8( signDigest, 0x80 );

    if (FALSE == SVC_DecryptSign( &rctx, signDigest, (const void*)pSign, (const void*)pPubkey ))
    {
        OS_TPrintf("Wlan Firmware authentication has failed.\n");
        /* continue verifying process even though decryption fails
           in the case of bonding option = 0x01 (support ARM9/ARM7) */
        if (!( HWi_WSYS08_OP_OP0_MASK == SCFG_ReadBondingOption() ))
        {
            SYSM_Free(signHeap);
            return FALSE;
        }
        OS_TPrintf("But installation continues.\n");
    }

    SYSM_Free(signHeap);

    OS_TPrintf("Decrypted digest: ");
    for (i = 0; i < SVC_SHA1_DIGEST_SIZE; i++ )
    {
        OS_TPrintf("%02X ", signDigest[i]);
    }
    OS_TPrintf("\n");

    /*
      skip comparing SHA1 digests in the case of bonding option = 0x01 (support ARM9/ARM7)
      this restriction is for debugging TWL wireless firmware.
     */
    if (!( HWi_WSYS08_OP_OP0_MASK == SCFG_ReadBondingOption() ))
    {
        /* verify digest */
        if (FALSE == SVC_CompareSHA1( (const void*)txtDigest, (const void*)signDigest ))
        {
            return FALSE;
        }
    }

#if (MEASURE_VERIFY_SIGN_TIME == 1)
    OS_TPrintf("Wlan firm:Verify signature Time=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - vstart));
#endif
    
    return TRUE;

}

BOOL InstallWlanFirmware(void)
{
    NWMRetCode err;

    pNwmBuf = 0;
    pFwBuffer = 0;

    OS_InitMessageQueue(&mesq, mesAry, sizeof(mesAry)/sizeof(mesAry[0]));
    
    /* HotStart/ColdStartのチェック */
    if (TRUE == SYSMi_GetWork()->flags.common.isHotStart)
    {
        pNwmBuf = SYSM_Alloc( NWM_SYSTEM_BUF_SIZE );
        if (!pNwmBuf) {
            OS_TWarning("Error: Couldn't allocate memory for NWM.\n");
            goto instfirm_error;
        }
        // HotStart
        NWM_Init(pNwmBuf, NWM_SYSTEM_BUF_SIZE, 3); /* 3 -> DMA no. */
        err = NWMi_InstallFirmware(InstallFirmCallback, NULL, 0, FALSE);
    } else {
        s32 flen = 0;

        // ColdStart

        /* pFwBuffer should be allocated from heap. */
        pFwBuffer = SYSM_Alloc( FWBUFFER_SIZE );
        if (!pFwBuffer) {
            OS_TWarning("Error: Couldn't allocate memory for WlanFirmware.\n");
            goto instfirm_error;
        }

        flen = ReadFirmwareBinary(pFwBuffer, FWBUFFER_SIZE);

        if ( 0 >= flen )
        {
            OS_TPrintf("Error: Couldn't read wlan firmware.\n");
            goto instfirm_error;
        }

        /*
            check signature data
         */
        if (FALSE == VerifyWlanfirmSignature(pFwBuffer, (u32)flen))
        {
            OS_TPrintf("Error: This Wlan Firmware is quite illegal!\n");
            OS_TPrintf("       It has never been installed.\n");
            goto instfirm_error;
        }

        /*************************************************************/

        pNwmBuf = SYSM_Alloc( NWM_SYSTEM_BUF_SIZE );
        if (!pNwmBuf) {
            OS_TWarning("Error: Couldn't allocate memory for NWM.\n");
            goto instfirm_error;
        }
        NWM_Init(pNwmBuf, NWM_SYSTEM_BUF_SIZE, 3); /* 3 -> DMA no. */

#if (MEASURE_WIRELESS_INITTIME == 1)
        startTick = OS_GetTick();
#endif

        err = NWMi_InstallFirmware(InstallFirmCallback, pFwBuffer, (u32)flen, TRUE);
    }
    
    /*
        無線ロード処理の完了は、IsWlanFirmwareInstalledでチェックする。
     */

    return TRUE;

    /* エラー処理 */
instfirm_error:
    if (pFwBuffer)
    {
        SYSM_Free( pFwBuffer );
        pFwBuffer = 0;
    }
    if (pNwmBuf)
    {
        NWM_End();
        SYSM_Free( pNwmBuf );
        pNwmBuf = 0;
    }
    return FALSE;
}


BOOL GetWlanFirmwareInstallResult(WLANFirmResult *pResult)
{
    OSMessage msg;
    BOOL retval;

    retval =  OS_ReadMessage(&mesq, &msg, OS_MESSAGE_NOBLOCK);

    *pResult = (WLANFirmResult)msg;

    return retval;
}
