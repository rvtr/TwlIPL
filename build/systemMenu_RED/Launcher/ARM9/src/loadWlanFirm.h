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

  �����@�FBOOL isForceLoad : TRUE->�����t�@�[�����[�h, FALSE->�t�@�[�����[�h�������X�^�[�g
  �Ԃ�l�FTRUE  �c �����t�@�[���E�F�A�C���X�g�[���̔񓯊�������
�@�@�@�@�@FALSE �c �����t�@�[���E�F�A�C���X�g�[���Ɏ��s����
 */

BOOL InstallWlanFirmware( BOOL isHotStartWLFirm );


/*

  PollingInstallWlanFirmware

  �����@�F�Ȃ�
  �Ԃ�l�FTRUE  �c �����t�@�[���E�F�A�C���X�g�[������������
          FALSE �c �����t�@�[���E�F�A�C���X�g�[��������������
 */

BOOL PollingInstallWlanFirmware( void );


#ifdef __cplusplus
}
#endif

#endif  // __LAUNCHER_WIRELESS_H__
