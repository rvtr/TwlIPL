/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     acsign.h

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

#ifndef _ACSIGN_H_
#define _ACSIGN_H_

#ifdef __cplusplus
extern "C" {
#endif

#if     defined( RSA_ENC_DEC )
#define     ACSIGN_BUFFER  ((160/8)*4)  // �o�̓o�b�t�@��
#else
#define     ACSIGN_BUFFER  ((160/8)*1)  // �o�̓o�b�t�@��
#endif

//
int     ACSign_Decrypto(
                    void*   buffer,     //  �o�͗̈�
                    void*   sgn_ptr,    //  �f�[�^�ւ̃|�C���^
                    void*   key_ptr     //  �L�[�ւ̃|�C���^
                    );

// 
int      ACSign_Digest(
                    void*   buffer,     //  �o�͗̈�
                    void*   romh_ptr,   //  �f�[�^�ւ̃|�C���^
                    void*   mbin_ptr,   //  �f�[�^�ւ̃|�C���^
                    int     mbin_len,   //  �f�[�^�̒���
                    void*   sbin_ptr,   //  �f�[�^�ւ̃|�C���^
                    int     sbin_len,   //  �f�[�^�̒���
                    u32     serial_num  //  �V���A���i���o�[
                    );

//
int     ACSign_Compare(
                    void* dercypto,     //  ACSign_Decrypto�̏o��
                    void* digest        //  ACSign_Digest�̏o��
                      );

#ifdef __cplusplus
}
#endif

#endif //_ACSIGN_H_
