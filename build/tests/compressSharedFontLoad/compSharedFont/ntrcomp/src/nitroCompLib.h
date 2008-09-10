/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     nitroCompLib.h

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

// nitroCompLib.h : nitroCompLib.DLL �̃��C�� �w�b�_�[ �t�@�C��
//

#ifndef __NITROCOMPLIB_H__
#define __NITROCOMPLIB_H__

//===========================================================================================
// �C���N���[�h
//===========================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"

#ifdef WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

//===========================================================================================
// �v���g�^�C�v�錾
//===========================================================================================
// C++�p
#ifdef __cplusplus
  extern "C"
  {
#endif

//BOOL WINAPI DllMain( HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved);
u8*  STDCALL    nitroCompMalloc( u32 size );
void STDCALL    nitroCompFree( u8 *p );
u32  STDCALL    nitroCompress  ( const u8 *srcp, u32 srcSize, u8 *dstp, char *compList, u8 rawHeader );
s32  STDCALL    nitroDecompress( const u8 *srcp, u32 srcSize, u8 *dstp, s8 depth );
void STDCALL    debugMemPrint   ( FILE * fp, u8 *str, u32 size );
void STDCALL    debugMemBitPrint( FILE * fp, u8 *str, u32 size );
int  STDCALL    matchingCheck( u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize );
u32  STDCALL    nitroGetDecompFileSize( const void* srcp );

#ifdef __cplusplus
  }
#endif

//===========================================================================================
// �֐��̎g�p�@
//===========================================================================================
//----------------------------------------------------------------------------------
//  ���k��̃f�[�^��u�����߂̃������̈���m��
//    ���k�O�̃f�[�^��2�{�̗̈���m�ۂ���
//    ����
//      u32  size   ���k�O�̃f�[�^�T�C�Y
//                     u32 �͕������������^�ł����Ă��́A
//                     unsigned int (�����n�ˑ�)
//    �Ԃ�l
//      u8    *        ���k��̃f�[�^�̈���w���|�C���^
//                     (�ėp�|�C���^�Ȃ̂ŁA�D���Ȍ^�ɃL���X�g)
//                                              �ł͂Ȃ�
//----------------------------------------------------------------------------------
//u8 *nitroCompMalloc(u32 size);

//----------------------------------------------------------------------------------
//  ���k��̃f�[�^��u���Ă����������̈�����
//    ����
//      u8    *p   ���k��̃f�[�^�̈���w���|�C���^
//----------------------------------------------------------------------------------
//void nitroCompFree(u8 *p);

//----------------------------------------------------------------------------------
//  �f�[�^�����k����
//  ���k�����A���k�����͈���compList��p���Ďw������
//    ����
//              u8       *srcp                  ���k�Ώۃf�[�^���w���|�C���^
//              u32       size                  ���k�Ώۃf�[�^�̃T�C�Y(�P�ʂ̓o�C�g)
//              u8       *dstp                  ���k��̃f�[�^��ێ�����f�[�^�̈���w���|�C���^
//                                                      �\���ȗ̈���m�ۂ��Ă����K�v����
//                                                      nitroCompMalloc�̕Ԃ�l�ł悢
//              char *compList          ���k�����A���k�������i�[�������X�g (C����̃k��������)
//                                                      d8      : 8�r�b�g�����t�B���^
//                                                      d16     : 16�r�b�g�����t�B���^
//                                                      r       : ���������O�X������
//                                                      lx      : LZ77�Dx�ɂ͓���f�[�^�����̊J�n�_�������I�t�Z�b�g
//                                                              : 2�ȏ�łȂ���΂Ȃ�Ȃ��D�����255.
//                                                      h4      : 4�r�b�g�E�n�t�}�����k
//                                                      h8      : 8�r�b�g�E�n�t�}�����k
//              u8        rawHeaderFlag ���k�O�̃f�[�^�ł��邱�Ƃ������w�b�_����t�����邩�ǂ�����
//                                                      �w������t���O�D0�ł���Εt�������A1�ł���΁A�W�J��������
//                                                      �f�[�^�ɂ��w�b�_���t�������D
//        �Ԃ�l
//          ���k��̃f�[�^�T�C�Y
//----------------------------------------------------------------------------------
//u32   nitroCompress(u8 *srcp, u32 size, u8 *dstp, char *compList, u8 rawHeader);

//----------------------------------------------------------------------------------
//  �f�[�^��W�J����
//  �W�J�����A�W�J�����̓f�[�^�̃w�b�_�����Ĕ��f����
//    ����
//              u8       *srcp                  �W�J�Ώۃf�[�^���w���|�C���^
//              u32       size                  �W�J�Ώۃf�[�^�̃T�C�Y(�P�ʂ̓o�C�g)
//              u8       *dstp                  �W�J��̃f�[�^��ێ�����f�[�^�̈���w���|�C���^
//                                                      �\���ȗ̈���m�ۂ��Ă����K�v����
//                                                      nitroCompMalloc�̕Ԃ�l�ł悢
//              u8        depth                 �W�J����[��(��)
//                          0�����̏ꍇ�́A���k�O�f�[�^�������w�b�_��񂪓�����܂œW�J��������
//        �Ԃ�l
//          �W�J��̃f�[�^�T�C�Y
//----------------------------------------------------------------------------------
//u32   nitroDecompress(u8 *srcp, u32 size, u8 *dstp, s8 depth);

#endif // __NITROCOMPLIB_H__
