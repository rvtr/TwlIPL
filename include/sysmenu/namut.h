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

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Format

  Description:  NAND�̋[���t�H�[�}�b�g
               �i�V�X�e���n�̕K�v�ȃt�@�C���݂̂��c�������������܂��j

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format(void);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_SearchInstalledSoftBoxCount

  Description:  InstalledSoftBoxCount�̐��𒲂ׂĕԂ��܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
u32 NAMUT_SearchInstalledSoftBoxCount( void );

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DrawNandTree

  Description:  NAND�̃c���[�����v�����g�o�͂��܂�

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NAMUT_DrawNandTree(void);

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


#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif	/* NAM_UTILITY_H_ */
