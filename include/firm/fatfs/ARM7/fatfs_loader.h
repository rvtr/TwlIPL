/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fatfs
  File:     fatfs_loader.h

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

#ifndef FIRM_FATFS_FATFS_LOADER_H_
#define FIRM_FATFS_FATFS_LOADER_H_

#include <twl/types.h>
#include <twl/fatfs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenRecentMenu

  Description:  open recent menu file

  Arguments:    driveno     drive number ('A' is 0)

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenRecentMenu( int driveno );

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenSpecifiedSrl

  Description:  open specified menu file

  Arguments:    menufile    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenSpecifiedSrl( const char* menufile );

/*---------------------------------------------------------------------------*
  Name:         FATFS_SaveSrlFilename

  Description:  store filename to HW_TWL_FS_BOOT_SRL_PATH_BUF

                �t�@�C������HW_TWL_FS_BOOT_SRL_PATH_BUF�ɏ������݂܂��B

  Arguments:    media       media type
                filename    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_SaveSrlFilename( FATFSMediaType media, const char* filename );

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetSrlDescriptor

  Description:  open specified menu file

                �C�ӂ̃t�@�C�����I�[�v�����A�t�@�C��ID��menu_fd�ɃZ�b�g���܂��B

  Arguments:    None

  Returns:      int
 *---------------------------------------------------------------------------*/
int FATFS_GetSrlDescriptor( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetSrlDescriptor

  Description:  set current file descriptor that was opened outside

                �I�[�v���ς݂̃t�@�C��ID��menu_fd�ɃZ�b�g���܂��B

  Arguments:    None

  Returns:      int
 *---------------------------------------------------------------------------*/
void FATFS_SetSrlDescriptor( int fd );

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadBuffer

  Description:  load data and pass to ARM9 via WRAM[B]

                LoadBuffer���J�j�Y���ŁAFAT���̃t�@�C���̓��e��ARM9�ɓ]�����܂��B

                [LoadBuffer���J�j�Y��]
                WRAM[B]�𗘗p���āAARM7,ARM9�Ԃ̃f�[�^�]�����s���܂��B
                WRAM[B]�̊e�X���b�g���o�P�c�����[�����œn���܂��B
                1�X���b�g���̃f�[�^�܂��͑S�f�[�^���i�[�ł����Ƃ��AARM9��
                FIRM_PXI_ID_LOAD_PIRIOD�𑗐M���܂��B
                �f�[�^�c������ꍇ�͎��̃X���b�g�̏����Ɉڂ�܂��B
                2��ڈȍ~�̌Ăяo���ł́A�O��Ō�̃X���b�g�̑�������g�p���܂��B
                �g�p�������X���b�g��ARM9���Ɋ��蓖�Ă��Ă���Ƃ��́AARM7����
                �Ȃ�܂ŃX�g�[�����܂��B

                [�g�p����]
                WRAM[B]�����b�N�����A������ԂƂ���ARM7���ɓ|���Ă������ƁB

                [���ӓ_]
                offset��size��ARM9�ɒʒm����܂���B�ʂ̌o�H�œ���������Ă��������B
                SRL�t�@�C����ǂݍ��ޏꍇ�́A�݂���ROM�w�b�_���Q�Ƃł���Ώ\���ł��B
                (ROM�w�b�_�����͌�����m���Ă���͂�)

                �⑫:
                �����ł́A���郉�C�u��������ARM7/ARM9���ŕ��������킹���邱�Ƃ�
                �O��ɂ��Ă��邪�A�ėp�I�ɂ���ɂ�(�Ɨ����C�u����������Ȃ�)�A
                ����M�ŃX���b�g�𔼕����Ƃ��A���ꂼ��Ɏ�M����PXI�R�[���o�b�N
                ���X���b�h��p�ӂ��A���M��API���f�[�^��WRAM�Ɋi�[������A������
                dest��size��ʒm����Ƃ����`��OK�ł͂Ȃ����H
                (�Ŋ���������Ԏ���Ԃ�)

                �⑫�Q�F
                ��x�ɕ����X���b�g���g�������ȏꍇ�́A��C��po_read�������������B
                ARM9����f�[�^���e��G��\�肪�Ȃ��Ȃ�A�{API���g�킸�A���ʂ�
                ���ڃ��C���������ɓ]���������������B

  Arguments:    offset      offset of the file to load (512 bytes alignment)
                size        size to load

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadBuffer(u32 offset, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadHeader

  Description:  load menu header

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadHeader( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadStatic

  Description:  load static binary

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadStatic( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_Boot

  Description:  boot

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_Boot( void );


#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FATFS_FATFS_LOADER_H_ */
#endif
