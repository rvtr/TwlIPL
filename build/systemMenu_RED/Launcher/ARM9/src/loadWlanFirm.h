/*---------------------------------------------------------------------------*
  Project:  TWL_RED_IPL - 
  File:     loadWlanFirm.h

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

#ifndef	__LAUNCHER_WIRELESS_H__
#define	__LAUNCHER_WIRELESS_H__

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WLANFIRM_RESULT_SUCCESS = 1,
    WLANFIRM_RESULT_FAILURE = 2
} WLANFirmResult;

/*

  InstallWlanFirmware

  引数　：BOOL isForceLoad : TRUE->強制ファームロード, FALSE->ファームロードせずリスタート
  返り値：TRUE  … 無線ファームウェアインストールの非同期処理中
　　　　　FALSE … 無線ファームウェアインストールに失敗した
 */

BOOL InstallWlanFirmware( BOOL isHotStartWLFirm );


/*

  PollingInstallWlanFirmware

  引数　：なし
  返り値：TRUE  … 無線ファームウェアインストール処理が完了
          FALSE … 無線ファームウェアインストール処理が未完了
 */

BOOL PollingInstallWlanFirmware( void );


#ifdef __cplusplus
}
#endif

#endif  // __LAUNCHER_WIRELESS_H__
