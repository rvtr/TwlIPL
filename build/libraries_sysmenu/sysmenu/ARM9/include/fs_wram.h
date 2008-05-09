/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - fs
  File:     fs_wram.h

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

#ifndef TWL_FS_WRAM_H_
#define TWL_FS_WRAM_H_

#ifdef SDK_TWL
#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    FSWramCallback
        Read/Write����WRAM�Ƀf�[�^�������ԂŌĂяo�����API
        �����񂵂��ꍇ�́A�������̃f�[�^���L���ƂȂ邱�Ƃɒ���
        addr        �f�[�^������A�h���X
        orig_addr   �{����Read�i�[��/Write���A�h���X
        len         �L���ȃf�[�^�T�C�Y
        wram        �g�p���Ă���WRAM
        slot        �g�p���Ă���X���b�g
        arg         API�ɓn��������
*/
typedef void (*FSWramCallback)(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg);

/*
    FS_InitWramTransfer
        ��x�����Ăяo���Ă����K�v������
        priority        �����グ��X���b�h�̗D�揇��
*/
BOOL FS_InitWramTransfer( u32 priority );
/*
    FS_ReadFileViaWram
        FS/FATFS�ɑ΂��ē���Read���s��
        ��������܂ŕԂ��Ă��Ȃ��_�ɒ���
        p_file      FS�ŃI�[�v�������t�@�C�� (�V�[�N�ς�)
        dst         �ǂݏo����
        len         �ǂݏo���T�C�Y
        wram        ���o����WRAM (B or C)
        slot        ���o����擪�X���b�g (������m�ۂ��Ă��Ȃ���ARM7/ARM9�����Ŋm�ۉ\�ł��邱��)
        size        ���o����T�C�Y
        callback    Read����WRAM�Ƀf�[�^������i�K�Ő����Ăяo�����R�[���o�b�N
        arg         �R�[���o�b�N�ɓn��������
*/
BOOL FS_ReadFileViaWram( FSFile *p_file, void *dst, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg );
/*
    FS_WriteFileViaWram
        FS/FATFS�ɑ΂��ē���Write���s��
        ��������܂ŕԂ��Ă��Ȃ��_�ɒ���
        p_file      FS�ŃI�[�v�������t�@�C�� (�V�[�N�ς�)
        src         �������݌�
        len         �������݃T�C�Y
        wram        ���o����WRAM (B or C)
        slot        ���o����擪�X���b�g (������m�ۂ��Ă��Ȃ���ARM7/ARM9�����Ŋm�ۉ\�ł��邱��)
        size        ���o����T�C�Y
        callback    Write����WRAM�Ƀf�[�^������i�K�Ő����Ăяo�����R�[���o�b�N
        arg         �R�[���o�b�N�ɓn��������
*/
BOOL FS_WriteFileViaWram( FSFile *p_file, const void *src, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg );
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_FS_WRAM_H_ */
