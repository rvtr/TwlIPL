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

/* LCFGの無線ファームバージョンをタイトルＩＤとしてそのまま使う場合 */
#define USE_LCFG_STRING              0
/* 無線FWダウンロード処理にかかる時間を計測する。 */
#define MEASURE_WIRELESS_INITTIME    1
/* 無線FW認証処理にかかる時間を計測する。 */
#define MEASURE_VERIFY_SIGN_TIME     1

#define WLANFIRM_PUBKEY_INDEX        1
#define USE_ACSIGN                   0 /* for experimental purpose */
#define SIGN_LENGTH                  128

#define FWBUFFER_SIZE                0x40000
#define SIGNHEAP_SIZE                0x01000

static u32   nwmBuf[NWM_SYSTEM_BUF_SIZE/sizeof(u32)] ATTRIBUTE_ALIGN(32);
static u8*   fwBuffer = 0;
#if (MEASURE_WIRELESS_INITTIME == 1)
static OSTick startTick;
#endif

static void  nwmCallback(void* arg);
static s32   readFirmwareBinary(u8 *buffer, s32 bufSize);
static BOOL  verifyWlanfirmSignature(u8* buffer, u32 length);


void nwmCallback(void* arg)
{
    NWMCallback *cb = (NWMCallback*)arg;
    switch (cb->apiid)
    {
    case NWM_APIID_LOAD_DEVICE:
        if (cb->retcode == NWM_RETCODE_SUCCESS) {
            NWMRetCode err;
            OS_TPrintf("Wlan firm:Load Device success!\n");
            err = NWM_UnloadDevice(nwmCallback);
        } else {
            OS_TPrintf("Wlan firm:Load Device Timeout Error!\n");
            SYSM_Free( fwBuffer );
        }
        break;
    case NWM_APIID_UNLOAD_DEVICE:
        OS_TPrintf("Wlan firm:Unload Device success!\n");
#if (MEASURE_WIRELESS_INITTIME == 1)
        OS_TPrintf("Wlan firm:LoadTime=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - startTick));
#endif
        OS_TPrintf("Wlan firm:Wlan firmware has been installed successfully!\n");
        SYSM_Free( fwBuffer );
        /* [TODO:] osSendMessage */
        break;
    default:
        OS_TWarning("Wlan firm:Error(invalid apiid=0x%04X)!\n", cb->apiid);
        SYSM_Free( fwBuffer );
        break;
    }

}

s32 readFirmwareBinary(u8 *buffer, s32 bufSize)
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


    titleID_hi = (( 3 /* Nintendo */ << 16) | 4 /* */ | 1 /* */);

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

BOOL verifyWlanfirmSignature(u8* buffer, u32 length)
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
    pSign = (u8*)((u32)buffer + (u32)hdr->rsv);

    txtVector[0] = buffer;
    txtlenVector[0] = (u32)hdr->rsv; /* 署名の直前までのLength */
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
#if ( USE_ACSIGN == 1)
    ACSign_SetAllocFunc( SYSM_Alloc, SYSM_Free );
#else
    signHeap = SYSM_Alloc( SIGNHEAP_SIZE );
    SVC_InitSignHeap( &rctx, signHeap, SIGNHEAP_SIZE);
#endif
    
    MI_CpuClear8( signDigest, 0x80 );

#if ( USE_ACSIGN == 1)
    ACSign_Decrypto(signDigest, (void*)pSign, (void*)pPubkey);
#else
    if (FALSE == SVC_DecryptSign( &rctx, signDigest, (const void*)pSign, (const void*)pPubkey ))
    {
        SYSM_Free(signHeap);
        return FALSE;
    }
#endif

    SYSM_Free(signHeap);

    OS_TPrintf("Decrypted digest: ");
    for (i = 0; i < SVC_SHA1_DIGEST_SIZE; i++ )
    {
        OS_TPrintf("%02X ", signDigest[i]);
    }
    OS_TPrintf("\n");

    /* verify digest */
    if (FALSE == SVC_CompareSHA1( (const void*)txtDigest, (const void*)signDigest ))
    {
        return FALSE;
    }

#if (MEASURE_VERIFY_SIGN_TIME == 1)
    OS_TPrintf("Wlan firm:Verify signature Time=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - vstart));
#endif
    
    return TRUE;

}

BOOL InstallWirelessFirmware(void)
{
    s32 flen = 0;
    NWMRetCode err;

    /* ColdStartのチェック(HotStartでは呼ばれない筈だが) */
    if (TRUE == SYSMi_GetWork()->flags.common.isHotStart)
    {
        OS_TPrintf("Error: It isn't Cold start.\n");
        return FALSE;
    }

    /* fwBuffer should be allocated from heap. */
    fwBuffer = SYSM_Alloc( FWBUFFER_SIZE );

    flen = readFirmwareBinary(fwBuffer, FWBUFFER_SIZE);

    if ( 0 > flen )
    {
        OS_TPrintf("Error: Couldn't read wlan firmware.\n");
        SYSM_Free( fwBuffer );
        return FALSE;
    }

    /*
            check signature data
     */
    if (FALSE == verifyWlanfirmSignature(fwBuffer, (u32)flen))
    {
        OS_TPrintf("Error: This Wlan Firmware is quite illegal!\n");
        OS_TPrintf("       It has never been installed.\n");
        SYSM_Free( fwBuffer );
        return FALSE;
    }

    /*************************************************************/

    NWM_Init(nwmBuf, sizeof(nwmBuf), 3); /* 3 -> DMA no. */

    if ( 0 < flen )
    {
        (void)NWMi_InstallFirmware(fwBuffer, (u32)flen);
    }

#if (MEASURE_WIRELESS_INITTIME == 1)
    startTick = OS_GetTick();
#endif
    err = NWM_LoadDevice(nwmCallback);

    /* osRecvMessage */
    /*
        [TODO:] 無線ロード処理の完了をメインルーチンへ通知するための仕組みを考える必要あり。
     */

    return TRUE;
}

