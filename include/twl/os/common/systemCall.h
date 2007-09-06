/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     systemCall.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_SYSTEMCALL_H_
#define TWL_OS_SYSTEMCALL_H_


#ifdef __cplusplus
extern "C" {
#endif

int SVC_InitSignHeap(
                    int             acmemory_pool[3],
                    void*           heap,
                    unsigned int    length
                    );

int SVC_DecryptoRSA(
                    const void*     acmemory_pool,
                    const void*     pData,
                    unsigned int*   len        // �o�̓T�C�Y
                    );

int SVC_DecryptoSign(
                    const void*     acmemory_pool,
                    void*           buffer,     //  �o�͗̈�
                    const void*     sgn_ptr,    //  �f�[�^�ւ̃|�C���^
                    const void*     key_ptr     //  �L�[�ւ̃|�C���^
                    );

int SVC_DecryptoSignDER(
                    const void*     acmemory_pool,
                    void*           buffer,     //  �o�͗̈�
                    const void*     sgn_ptr,    //  �f�[�^�ւ̃|�C���^
                    const void*     key_ptr     //  �L�[�ւ̃|�C���^
                    );

void SVC_SHA1Init( void *c );
void SVC_SHA1Update( void *c, const unsigned char *data, unsigned long len );
void SVC_SHA1GetHash( unsigned char *md, void *c );

int SVC_CalcSHA1(
                    void*         buffer,     //  �o�͗̈�
                    const void*   buf,        //  �f�[�^�ւ̃|�C���^
                    unsigned int  len         //  �f�[�^�̒���
                    );

int SVC_CompareSHA1(
                    const void* decedHash,    //  SVC_Decrypto*�̏o��
                    const void* digest        //  SVC_GetDigest�̏o��
                    );

int SVC_RandomSHA1(
                    void*           dest_ptr,   // �o�̓f�[�^�ւ̃|�C���^
                    unsigned int    dest_len,   // �o�̓f�[�^�̒���
                    const void*     src_ptr,    // ���̓f�[�^�ւ̃|�C���^
                    unsigned int    src_len     // ���̓f�[�^�̒���
                    );

int SVC_UncompressLZ8FromDevice( const void* srcp,
                                  void* destp,
                                  const void* paramp,
                                  const MIReadStreamCallbacks *callbacks
                                  );

int SVC_UncompressLZ16FromDeviceIMG( const void* srcp,
                                  void* destp,
                                  const void* paramp,
                                  const MIReadStreamCallbacks *callbacks
                                  );



#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_SYSTEMCALL_H_ */
#endif
