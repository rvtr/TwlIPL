/*---------------------------------------------------------------------------*
  Project:  TWL_RED_IPL - 
  File:     loadWlanFirm.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 
  $Rev: 
  $Author: 
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/nam.h>
#include <twl/os/common/format_rom.h>
#include "loadWlanFirm.h"


static int   isNwmActive;
static u32   nwmBuf[NWM_SYSTEM_BUF_SIZE/sizeof(u32)] ATTRIBUTE_ALIGN(32);
static u32   fwBuffer[256*1024/sizeof(u32)]  ATTRIBUTE_ALIGN(32);



static void nwmcallback(void* arg)
{
  NWMCallback *cb = (NWMCallback*)arg;
  switch (cb->apiid)
    {
    case NWM_APIID_LOAD_DEVICE:
      if (cb->retcode == NWM_RETCODE_SUCCESS) {
	isNwmActive = 1;
	/* osSendMessage */
	OS_TPrintf("Wlan firm:Load Device success!\n");
      } else {
	OS_TPrintf("Wlan firm:Load Device Timeout Error!\n");
      }
      break;
    case NWM_APIID_UNLOAD_DEVICE:
	OS_TPrintf("Wlan firm:Unload Device success!\n");
      break;
    default:
	OS_TPrintf("Wlan firm:Error(apiid=default)!\n");
      break;
    }

}


BOOL WirelessFirmwareDownloadStart(void)
{
  char path[256];
  FSFile  file[1];

  char *title = "WFW0";

  u32 titleID_hi;
  u32 titleID_lo;

  u64 titleID = 0;
  s32 flen = 0;
  NWMRetCode err;

  titleID_hi = (( 3 /* Nintendo */ << 16) | 4 /* */ | 1 /* */);

  titleID_lo =  ((u32)( *title++ ) & 0xff) << 24;
  titleID_lo |= ((u32)( *title++ )& 0xff) << 16;
  titleID_lo |= ((u32)( *title++ )& 0xff) << 8;
  titleID_lo |= (u32)( *title++ ) & 0xff;

  titleID = ((u64)(titleID_hi) << 32)  | (u64)titleID_lo;
  
  // OS_TPrintf( "titleID = 0x%08x%08x\n", titleID_hi, titleID_lo); 

  isNwmActive  = 0;

  if( NAM_OK == NAM_GetTitleBootContentPathFast(path, titleID) ) {
    OS_TPrintf( "File = %s\n", path);
  }
  else {
    OS_TPrintf( "Error: NAM_GetTitleBootContentPathFast titleID = 0x%08x0x%08x\n",titleID_hi, titleID_lo);
    return FALSE;
  }

  if (!FS_OpenFileEx(file, path, FS_FILEMODE_R)) {
    OS_TWarning("FS_OpenFileEx(%s) failed.\n", path);
    return FALSE;
  }

  if( FALSE == FS_SeekFile(file, sizeof(ROM_Header), FS_SEEK_SET) ) {
    OS_TWarning("FS_SeekFile failed.\n");
    return FALSE;
  } 

  flen = FS_ReadFile(file, fwBuffer, sizeof(fwBuffer));
  if( flen == -1 ) {
    OS_TWarning("FS_ReadFile failed.\n");
    return FALSE;
  }

  (void)FS_CloseFile(file);


  /*************************************************************/
  
  NWM_Init(nwmBuf, sizeof(nwmBuf), 3); /* 3 -> DMA no. */

  (void)NWMi_InstallFirmware(fwBuffer, (u32)flen);

  err = NWM_LoadDevice(nwmcallback);

  /* osRecvMessage */

  return TRUE;
}
