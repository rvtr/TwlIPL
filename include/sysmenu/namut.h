/*---------------------------------------------------------------------------*
  Project:  TwlIPL - NAMUT
  File:     namut.h

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

#ifndef NAM_UTILITY_H_
#define NAM_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9

typedef void* (*NAMUTAlloc)(u32 size);
typedef void  (*NAMUTFree)(void* ptr);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Init

  Description:  NAMUT ���C�u�����̏��������s���܂��B

  Arguments:    allocFunc:  �������m�ۊ֐��ւ̃|�C���^�B(�v�F32byte�A���C�����g�j
                freeFunc:   ����������֐��ւ̃|�C���^�B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
void NAMUT_Init(NAMUTAlloc allocFunc, NAMUTFree freeFunc);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Format

  Description:  NAND�̋[���t�H�[�}�b�g
               �i�V�X�e���n�̕K�v�ȃt�@�C���݂̂��c�������������܂��j

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format(void);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_FormatCore

  Description:  �{�̏�����(NAND������)���s���܂��B
               �i�V�X�e���n�̕K�v�ȃt�@�C���݂̂��c�������������܂�
                 ���[�U�[�A�v���� common, personalized�Ɋւ�炸�S�ď������邩�A
	�@�@�@�@�@�@ personalized�̂ݏ������邩�������őI���ł��܂��B

  Arguments:    isForceEraseCommonETicket: TRUE �̎��́Acommon, personalized�Ɋւ�炸���[�U�[�A�v����S����
                                           FALSE�̎��́AcommonETicket���c���i�A�v�����g�͏����j
	            isDeleteWifiSettings: WiFi�ݒ���폜���邩�H�iTRUE�ō폜�j
  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_FormatCore( BOOL isForceEraseCommonETicket, BOOL isDeleteWiFiSettings );

/*---------------------------------------------------------------------------*
  Name:         NAMUT_GetSoftBoxCount

  Description:  NAND�� installed�J�E���g�Afree�J�E���g�𒲂ׂ�
                �w�肳�ꂽ�ϐ��Ɋi�[���܂��B

  Arguments:    installed : installed�J�E���g�i�[�ϐ�
                free      : free�J�E���g�i�[�ϐ�

  Returns:      �����Ȃ�TRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_GetSoftBoxCount( u8* installed, u8* free );

/*---------------------------------------------------------------------------*
  Name:         NAMUT_UpdateSoftBoxCount

  Description:  InstalledSoftBoxCount, FreeSoftBoxCount �̒l��
                ���݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B

  Arguments:    None.

  Returns:      �����Ȃ�TRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_UpdateSoftBoxCount( void );

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataPublic

  Description:  �w�肵���Z�[�u�f�[�^�t�@�C���ɑ΂���
                �e�e�N���A���t�H�[�}�b�g���s���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_ClearSavedataPublic(const char* path, u64 titleID);

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataPrivate

  Description:  �w�肵���Z�[�u�f�[�^�t�@�C���ɑ΂���
                �e�e�N���A���t�H�[�}�b�g���s���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_ClearSavedataPrivate(const char* path, u64 titleID);

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DestroySubBanner

  Description:  �w�肵���T�u�o�i�[��CRC�j������݂܂��B
                �w�肵���T�u�o�i�[�����݂��Ȃ��\��������܂���
                ���̏ꍇ�ł�TRUE��Ԃ��܂��B�i�R�[�h��OS_DeleteSubBannerFile�̃p�N���j

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_DestroySubBanner(const char* path);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DeleteNandDirectory

  Description:  �w��f�B���N�g���ȉ����������܂��B
                �w��f�B���N�g�����͎̂c��܂��B

  Arguments:    path : ��΃p�X�i�X���b�V�����܂߂Ȃ��j

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_DeleteNandDirectory(const char *path);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DeleteNandTmpDirectory

  Description:  "nand:/tmp" �f�B���N�g���ȉ����������܂��B
                ��O�Ƃ��āA"nand:/tmp/es" �ȉ����c���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_DeleteNandTmpDirectory(void);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_ClearTWLSettings

  Description:  TWL�{�̐ݒ�f�[�^�̃N���A���s���܂��B

  Arguments:    TRUE : �N���A�����l��NAND�Ƀ��C�g�o�b�N���܂��B
                FALSE: NAND�ւ̃��C�g�o�b�N�͍s���܂���B

  Returns:      �����Ȃ�TRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_ClearTWLSettings( BOOL doWriteback );


#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif  /* NAM_UTILITY_H_ */
