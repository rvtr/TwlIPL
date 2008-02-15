/*---------------------------------------------------------------------------*
  Project:  TWL_RED_IPL - 
  File:     loadWlanFirm.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: $
  $Rev: $
  $Author: $
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

#define WLANFIRM_PUBKEY_INDEX        1

#define MEASURE_WIRELESS_INITTIME    1

static u32   nwmBuf[NWM_SYSTEM_BUF_SIZE/sizeof(u32)] ATTRIBUTE_ALIGN(32);
static u32   fwBuffer[256*1024/sizeof(u32)]  ATTRIBUTE_ALIGN(32);
#if (MEASURE_WIRELESS_INITTIME == 1)
static OSTick startTick;
#endif

static s32   readFirmwareBinary(u8 *buffer, s32 bufSize);
static BOOL  verifyWlanfirmSignature(u8* buffer);


static void nwmcallback(void* arg)
{
    NWMCallback *cb = (NWMCallback*)arg;
    switch (cb->apiid)
    {
    case NWM_APIID_LOAD_DEVICE:
        if (cb->retcode == NWM_RETCODE_SUCCESS) {
            NWMRetCode err;
            OS_TPrintf("Wlan firm:Load Device success!\n");
            err = NWM_UnloadDevice(nwmcallback);
        } else {
            OS_TPrintf("Wlan firm:Load Device Timeout Error!\n");
        }
        break;
    case NWM_APIID_UNLOAD_DEVICE:
        OS_TPrintf("Wlan firm:Unload Device success!\n");
#if (MEASURE_WIRELESS_INITTIME == 1)
        OS_TPrintf("Wlan firm:LoadTime=%dmsec\n", OS_TicksToMilliSeconds(OS_GetTick() - startTick));
#endif
        OS_TPrintf("Wlan firm:Wlan firmware has been installed successfully!\n");
        /* [TODO:] osSendMessage */
        break;
    default:
        OS_TWarning("Wlan firm:Error(invalid apiid=0x%04X)!\n", cb->apiid);
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

BOOL verifyWlanfirmSignature(u8* buffer)
{
    NWMFirmHeader *hdr = (NWMFirmHeader*)buffer;
    u8 *pPubkey;

    pPubkey = OSi_GetFromFirmAddr()->rsa_pubkey[WLANFIRM_PUBKEY_INDEX];
//    OS_TWarning("Pubkey addr %x\n", pPubkey);

    /* calculate SHA-1 digest */

    /* decrypt according to RSA security */

    /* verify digest */

    return TRUE;

}

BOOL InstallWirelessFirmware(void)
{
    s32 flen = 0;
    NWMRetCode err;

    /* ColdStartのチェック(HotStartでは呼ばれない筈だが) */
    if (TRUE == SYSMi_GetWork()->flags.common.isHotStart)
    {
        OS_TWarning("It isn't Cold start.\n");
        return FALSE;
    }

    /* [TODO:] fwBuffer should be allocated from heap. */

    flen = readFirmwareBinary((u8*)fwBuffer, sizeof(fwBuffer));

    if ( 0 > flen )
    {
        OS_TWarning("Couldn't read wlan firmware.\n");
        return FALSE;
    }

    /*
            [TODO:] check signature data
     */

    /*************************************************************/

    NWM_Init(nwmBuf, sizeof(nwmBuf), 3); /* 3 -> DMA no. */

    /* In the case of cold start, should register appropriate firmware. */
    if ( 0 < flen )
    {
        (void)NWMi_InstallFirmware(fwBuffer, (u32)flen);
    }

#if (MEASURE_WIRELESS_INITTIME == 1)
    startTick = OS_GetTick();
#endif
    err = NWM_LoadDevice(nwmcallback);

    /* osRecvMessage */
    /*
        [TODO:] 無線ロード処理の完了をメインルーチンへ通知するための仕組みを考える必要あり。
     */

    return TRUE;
}

