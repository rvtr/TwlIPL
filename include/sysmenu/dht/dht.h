/*---------------------------------------------------------------------------*
  Project:  TwlIPL - DHT
  File:     dht.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef SYSMENU_DHT_H_
#define SYSMENU_DHT_H_

#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu/dht/dht_format.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL    (*DHTReadFunc)(void* dest, s32 offset, s32 length, void* arg);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  �ǂݍ��ݍς݂̃f�[�^�x�[�X�̃w�b�_����T�C�Y��Ԃ�

  Arguments:    pDHT        �f�[�^�x�[�X�w�b�_�̊i�[��

  Returns:      ���������ȃw�b�_�Ȃ�T�C�Y�A�����łȂ��Ȃ�0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseLength(const DHTFile* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  �ǂݍ��ݍς݂̑S�f�[�^�x�[�X�̏��������؂���

  Arguments:    pDHT        �f�[�^�x�[�X�̊i�[��

  Returns:      ���������TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckDatabase(const DHTFile* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  FS�֐��𗘗p���đS�f�[�^�x�[�X��ǂݍ��݌��؂܂ōs��
                �t�@�C������/sign/DSHashTable.bin�Œ�

  Arguments:    pDHT        �S�f�[�^�x�[�X�̊i�[��

  Returns:      ���������TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabase(DHTFile* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabase

  Description:  ROM�w�b�_�ɑΉ�����f�[�^�x�[�X����������

  Arguments:    pDHT        �S�f�[�^�x�[�X�̊i�[��
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��

  Returns:      �Ώۃf�[�^�x�[�X�ւ̃|�C���^
 *---------------------------------------------------------------------------*/
const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1Init

  Description:  ROM�w�b�_�����ARM9/ARM7�X�^�e�B�b�N�̈�̌��؂̏���

  Arguments:    ctx         ���ؗp��SVCHMACSHA1�R���e�L�X�g
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��

  Returns:      None
 *---------------------------------------------------------------------------*/
void DHT_CheckHashPhase1Init(SVCHMACSHA1Context* ctx, const ROM_Header_Short* pROMHeader);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1Update

  Description:  ROM�w�b�_�����ARM9/ARM7�X�^�e�B�b�N�̈�̌��؂̃X�^�e�B�b�N����
                �����番�����Ă��ǂ����AARM9�X�^�e�B�b�N�AARM7�X�^�e�B�b�N�̏���
                �Ăяo�����ƁB

  Arguments:    ctx         ���ؗp��SVCHMACSHA1�R���e�L�X�g
                ptr         �ΏۂƂȂ�f�[�^�̈�
                length      �ΏۂƂȂ�f�[�^�T�C�Y

  Returns:      None
 *---------------------------------------------------------------------------*/
void DHT_CheckHashPhase1Update(SVCHMACSHA1Context* ctx, const void* ptr, u32 length);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1

  Description:  ROM�w�b�_�����ARM9/ARM7�X�^�e�B�b�N�̈�̌��؂̌��ʔ���

  Arguments:    ctx         ���ؗp��SVCHMACSHA1�R���e�L�X�g
                db          �Ώۃf�[�^�x�[�X�ւ̃|�C���^

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase1Final(SVCHMACSHA1Context* ctx, const DHTDatabase *db);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1

  Description:  ROM�w�b�_�����ARM9/ARM7�X�^�e�B�b�N�̈�̌���

  Arguments:    db          �Ώۃf�[�^�x�[�X�ւ̃|�C���^
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��
                pARM9       �ΏۂƂȂ�ARM9�X�^�e�B�b�N�i�[��
                pARM7       �ΏۂƂȂ�ARM7�X�^�e�B�b�N�i�[��

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase1(const DHTDatabase *db, const ROM_Header_Short* pROMHeader, const void* pARM9, const void* pARM7);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase2

  Description:  �I�[�o�[���C�̈�̌���
                (�f�o�C�X��Read API��o�^�ł���ׂ�)

  Arguments:    db          �Ώۃf�[�^�x�[�X�ւ̃|�C���^
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��
                fctx        (FS��) FSFile�\���̂ւ̃|�C���^
                            (CARD��) dma�ԍ���void*�ɃL���X�g��������
                            (HOTSW��) CardBootData�\���̂ւ̃|�C���^
                buffer      �{API�Ŏg�p���郏�[�N (DHT_OVERLAY_MAX�����K�v)

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase2(const DHTDatabase *db, const ROM_Header_Short* pROMHeader, void* buffer, DHTReadFunc func, void* arg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // SYSMENU_DHT_H_