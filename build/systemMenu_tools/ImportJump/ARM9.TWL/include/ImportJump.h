/*---------------------------------------------------------------------------*
  Project:  ImportJump
  File:     import.h

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

#ifndef IMPORT_JUMP_H_
#define IMPORT_JUMP_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

typedef struct _ImportJumpSetting
{
	u32	 magicCode;		             	// = TWLD
	u32	 clearPublicSaveData :1;	    // public�Z�[�u�f�[�^���N���A����i�f�t�H���gOFF�j
	u32	 clearPrivateSaveData :1;	 	// privare�Z�[�u�f�[�^���N���A����i�f�t�H���gOFF�j
	u32	 clearSaveBannerFile:1;	     	// �Z�[�u�o�i�[�t�@�C�����N���A����i�f�t�H���gOFF�j
	u32  importTad:1;                  	// �p�X�Ŏw�肳�ꂽTAD�t�@�C�����C���|�[�g���邩�iTAD�̍X�V�L���Ɉˑ��j
	u32	 rsv :28;                       // �\��
	u32  tadRomOffset;					// TAD�����[�h�����G�~�����[�V����ROM�I�t�Z�b�g
	u32	 tadLength;		                // TAD�t�@�C���̒���
} ImportJump;

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define IMPORT_TAD_ROM_OFS       0x00800000
#define IMPORT_JUMP_SETTING_OFS  (IMPORT_TAD_ROM_OFS - CARD_ROM_PAGE_SIZE)

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/

ImportJump* GetImportJumpSetting( void );

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* IMPORT_JUMP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
