/*---------------------------------------------------------------------------*
  Project:  TwlIPL - DHT
  File:     dht.h

  Copyright 2008,2009 Nintendo.  All rights reserved.

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

/*
    Phase�A�z���C�g���X�g�A�}�X�^�����O�ς݃w�b�_�̑Ή��֌W

    Phase1  DHTDatabase->hash[0]        ROM_Header_Short->nitro_whitelist_phase1_digest
    Phase2  DHTDatabase->hash[1]        ROM_Header_Short->nitro_whitelist_phase2_digest
    Phase3  DHTDatabaseEx->banner_hash  ROM_Header_Short->banner_digest
    Phase4  -                           -

    Phase4�̃n�b�V���l��dht_phase4_list.c�Ɋ܂܂��
*/

#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu/dht/dht_format.h>

#define nitro_whitelist_phase2_digest nitro_whitelist_phase2_diegst // for spell miss

#define DHT_FAT_PAGE_SIZE   512
#define DHT_FAT_CACHE_SIZE  (DHT_FAT_PAGE_SIZE * 2)

#define DHT_PHASE3_MAX      DHT_OVERLAY_MAX

/*
    DHT_CheckHashPhase2�ŕK�v�ȃ��[�N������
*/
typedef struct DHTPhase2Work
{
    u8  fatCache[DHT_FAT_CACHE_SIZE];           // for fat cache only
    u32 buffer[DHT_OVERLAY_MAX/sizeof(u32)];    // multiple usage
}
DHTPhase2Work;

/*
    DHT_CheckHashPhase4�ŕK�v�ȃ��[�N������
    (DHTPhase4Work <= DHTPhase2Work���K�����藧��)
*/

typedef struct DHTPhase4Work
{
    u32 buffer[DHT_PHASE3_MAX/sizeof(u32)];
}
DHTPhase4Work;

/*
    DHT_CheckHashPhase2Ex�ŕK�v�ȃ��[�N������
*/
typedef struct DHTPhase2ExWork
{
    u8  fatCache[DHT_FAT_CACHE_SIZE];           // for fat cache only
}
DHTPhase2ExWork;

#ifdef __cplusplus
extern "C" {
#endif

/*
    DHT_CheckHashPhase2/DHT_CheckHashPhase2Ex/DHT_CheckHashPhase4�Ŏg�p����Read�֐�
    dest        �]����A�h���X
    offset      �]����ROM�I�t�Z�b�g
    length      �]���T�C�Y
    arg         �A�v���P�[�V��������n���ꂽ�l

    �񕜕s�\�ȓ����G���[�������ɂ�FALSE��Ԃ�����
*/
typedef BOOL    (*DHTReadFunc)(void* dest, s32 offset, s32 length, void* arg);

/*
    DHT_CheckHashPhase2Ex/DHT_CheckHashPhase4Ex�Ŏg�p����Read�֐�
    �]����A�h���X�͑��݂����A����ɓƎ��o�b�t�@�ɓǂݍ��񂾌�
    DHT_CheckHashPhase2ExUpdate���Ăяo������(�ו����\)
    ctx         DHT_CheckHashPhase2ExUpdate�ɓn������
    offset      �]����ROM�I�t�Z�b�g
    length      �]���T�C�Y
    arg         �A�v���P�[�V��������n���ꂽ�l

    �񕜕s�\�ȓ����G���[�������ɂ�FALSE��Ԃ�����
*/
typedef BOOL    (*DHTReadFuncEx)(SVCHMACSHA1Context* ctx, s32 offset, s32 length, void* arg);
/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabaseLength

  Description:  �ǂݍ��ݍς݂̃f�[�^�x�[�X�̃w�b�_����T�C�Y��Ԃ�

  Arguments:    pDHT        �f�[�^�x�[�X�w�b�_�̊i�[��

  Returns:      ���������ȃw�b�_�Ȃ�T�C�Y�A�����łȂ��Ȃ�0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseLength(const DHTFile* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabaseExLength

  Description:  �ǂݍ��ݍς݂̊g���f�[�^�x�[�X�̃w�b�_����T�C�Y��Ԃ�

  Arguments:    pDHT        �g���f�[�^�x�[�X�w�b�_�̊i�[��

  Returns:      ���������ȃw�b�_�Ȃ�T�C�Y�A�����łȂ��Ȃ�0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseExLength(const DHTFileEx* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabaseAdHocLength

  Description:  �ǂݍ��ݍς݂̌ʑΉ��f�[�^�x�[�X�̃w�b�_����T�C�Y��Ԃ�

  Arguments:    pDHT        �ʑΉ��f�[�^�x�[�X�w�b�_�̊i�[��

  Returns:      ���������ȃw�b�_�Ȃ�T�C�Y�A�����łȂ��Ȃ�0
 *---------------------------------------------------------------------------*/
u32 DHT_GetDatabaseAdHocLength(const DHTFileAdHoc* pDHT);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabase

  Description:  FS�֐��𗘗p���ăf�[�^�x�[�X��ǂݍ��݂ƌ��؂��s��

  Arguments:    pDHT        �f�[�^�x�[�X�̊i�[��
                fp          �t�@�C���\���̂ւ̃|�C���^
                            DHTHeader�̐擪�܂ŃV�[�N�ς݂ł���K�v������
                maxLength   �ǂݍ��݃T�C�Y�̏��(�w�b�_����)

  Returns:      ���������TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabase(DHTFile* pDHT, FSFile* fp, s32 maxLength);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabaseEx

  Description:  FS�֐��𗘗p���Ċg���f�[�^�x�[�X��ǂݍ��݂ƌ��؂��s��

  Arguments:    pDHT        �g���f�[�^�x�[�X�̊i�[��
                fp          �t�@�C���\���̂ւ̃|�C���^
                            DHTHeader�̐擪�܂ŃV�[�N�ς݂ł���K�v������
                maxLength   �ǂݍ��݃T�C�Y�̏��(�w�b�_����)

  Returns:      ���������TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabaseEx(DHTFileEx* pDHT, FSFile* fp, s32 maxLength);

/*---------------------------------------------------------------------------*
  Name:         DHT_PrepareDatabaseAdHoc

  Description:  FS�֐��𗘗p���ČʑΉ��f�[�^�x�[�X��ǂݍ��݂ƌ��؂��s��

  Arguments:    pDHT        �ʑΉ��f�[�^�x�[�X�̊i�[��
                fp          �t�@�C���\���̂ւ̃|�C���^
                            DHTHeader�̐擪�܂ŃV�[�N�ς݂ł���K�v������
                maxLength   �ǂݍ��݃T�C�Y�̏��(�w�b�_����)

  Returns:      ���������TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_PrepareDatabaseAdHoc(DHTFileAdHoc* pDHT, FSFile* fp, s32 maxLength);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabase

  Description:  ROM�w�b�_�ɑΉ�����f�[�^�x�[�X����������

  Arguments:    pDHT        �f�[�^�x�[�X�̊i�[��
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��

  Returns:      �ΏۃG���g���ւ̃|�C���^�A������Ȃ����NULL
 *---------------------------------------------------------------------------*/
const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader);

/*---------------------------------------------------------------------------*
  Name:         DHT_GetDatabaseEx

  Description:  ROM�w�b�_�ɑΉ�����g���f�[�^�x�[�X����������

  Arguments:    pDHT        �g���f�[�^�x�[�X�̊i�[��
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��

  Returns:      �ΏۃG���g���ւ̃|�C���^�A������Ȃ����NULL
 *---------------------------------------------------------------------------*/
const DHTDatabaseEx* DHT_GetDatabaseEx(const DHTFileEx* pDHT, const ROM_Header_Short* pROMHeader);

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
void DHT_CheckHashPhase1Update(SVCHMACSHA1Context* ctx, const void* ptr, s32 length);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1

  Description:  ROM�w�b�_�����ARM9/ARM7�X�^�e�B�b�N�̈�̌��؂̌��ʔ���

  Arguments:    ctx         ���ؗp��SVCHMACSHA1�R���e�L�X�g
                hash        �Ή�����n�b�V�� (db->hash[0])

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase1Final(SVCHMACSHA1Context* ctx, const u8* hash);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase1

  Description:  ROM�w�b�_�����ARM9/ARM7�X�^�e�B�b�N�̈�̌���

  Arguments:    hash        �Ή�����n�b�V�� (db->hash[0])
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��
                pARM9       �ΏۂƂȂ�ARM9�X�^�e�B�b�N�i�[��
                pARM7       �ΏۂƂȂ�ARM7�X�^�e�B�b�N�i�[��

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase1(const u8* hash, const ROM_Header_Short* pROMHeader, const void* pARM9, const void* pARM7);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase2

  Description:  �I�[�o�[���C�̈�̌���

  Arguments:    hash        �Ή�����n�b�V�� (db->hash[1])
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��
                work        �{API�Ŏg�p���郏�[�N (513KB)
                func        �Ώۃf�o�C�X�ɉ�����Read�֐�
                arg         Read�֐��ɓn��������

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase2(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2Work* work, DHTReadFunc func, void* arg);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase2Ex

  Description:  �I�[�o�[���C�̈�̌���

  Arguments:    hash        �Ή�����n�b�V�� (db->hash[1])
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��
                work        �{API�Ŏg�p���郏�[�N (1KB)
                func        �Ώۃf�o�C�X�ɉ�����Read�֐�
                funcEx      �Ώۃf�o�C�X�ɉ����ēƎ��o�b�t�@�Ƀf�[�^��ǂݍ���
                            DHT_CheckHashPhase2ExUpdate���Ăяo���K�v������
                arg         Read�֐��ɓn��������

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase2Ex(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2ExWork* work, DHTReadFunc func, DHTReadFuncEx funcEx, void* arg);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase2ExUpdate / DHT_CheckHashPhase4ExUpdate

  Description:  �I�[�o�[���C�����̌��؂���ьʂ̌���
                DHTReadFuncEx����Ăяo������(����Ȃ�ו����͎��R)
                ����: Phase4�ł����p���Ă���

  Arguments:    ctx         ���ؗp��SVCHMACSHA1�R���e�L�X�g
                ptr         �ΏۂƂȂ�f�[�^�̈�
                length      �ΏۂƂȂ�f�[�^�T�C�Y

  Returns:      None
 *---------------------------------------------------------------------------*/
void DHT_CheckHashPhase2ExUpdate(SVCHMACSHA1Context* ctx, const void* ptr, s32 length);
#define DHT_CheckHashPhase4ExUpdate DHT_CheckHashPhase2ExUpdate

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase3

  Description:  �o�i�[�̈�̌���
                (���j���[�\���Ɏg�p�����f�[�^��n���ׂ�)

  Arguments:    hash        �Ή�����n�b�V�� (dbex->banner_hash)
                pBanner     �ΏۂƂȂ�o�i�[�i�[��

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase3(const u8* hash, const NTRBannerFile* pBanner);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase4

  Description:  �ʂ̌���

  Arguments:    pDHT        �ʑΉ��f�[�^�x�[�X�̊i�[��
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��
                work        �{API�Ŏg�p���郏�[�N (512KB)
                            phase2�̎g���񂵂�OK
                func        �Ώۃf�o�C�X�ɉ�����Read�֐�
                arg         Read�֐��ɓn��������

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase4(const DHTFileAdHoc* pDHT, const ROM_Header_Short* pROMHeader, DHTPhase4Work* work, DHTReadFunc func, void* arg);

/*---------------------------------------------------------------------------*
  Name:         DHT_CheckHashPhase4Ex

  Description:  �ʂ̌���

  Arguments:    pDHT        �ʑΉ��f�[�^�x�[�X�̊i�[��
                pROMHeader  �ΏۂƂȂ�ROM�w�b�_�i�[��
                funcEx      �Ώۃf�o�C�X�ɉ����ēƎ��o�b�t�@�Ƀf�[�^��ǂݍ���
                            DHT_CheckHashPhase2ExUpdate���Ăяo���K�v������
                arg         Read�֐��ɓn��������

  Returns:      ���Ȃ����TRUE
 *---------------------------------------------------------------------------*/
BOOL DHT_CheckHashPhase4Ex(const DHTFileAdHoc* pDHT, const ROM_Header_Short* pROMHeader, DHTReadFuncEx funcEx, void* arg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // SYSMENU_DHT_H_
