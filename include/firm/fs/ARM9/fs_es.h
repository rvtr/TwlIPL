/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fs
  File:     fs_es.h

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

#ifndef FIRM_FS_ES_FIRM_H_
#define FIRM_FS_ES_FIRM_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FS_GetTitleBootContentPathFast

  Description:  NAND �ɃC���X�g�[������Ă���A�v���̎��s�t�@�C���̃p�X��
                �擾���܂��B
                �擾������̐����������؂��Ȃ����ߍ����ł����A
                ��񂪉�₂���Ă���\�������邱�Ƃɒ��ӂ��Ȃ���΂Ȃ�܂���B

  Arguments:    buf:        �p�X���i�[����o�b�t�@�ւ̃|�C���^�B
                            FS_ENTRY_LONGNAME_MAX �ȏ�̃T�C�Y���K�v�ł��B
                titleId:    �p�X���擾����A�v���� Title ID�B

  Returns:      ����ɏ������s��ꂽ�Ȃ� TRUE ��Ԃ��܂��B
 *---------------------------------------------------------------------------*/
BOOL FS_GetTitleBootContentPathFast(char* buf, u64 titleId);


#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FS_ES_FIRM_H_ */
#endif
