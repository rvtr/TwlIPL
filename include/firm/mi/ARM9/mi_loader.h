/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - mi
  File:     mi_loader.h

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

#ifndef FIRM_MI_LOADER_H_
#define FIRM_MI_LOADER_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         MIi_LoadBuffer

  Description:  receive data from ARM7 and store(move) via WRAM[B]

                LoadBuffer���J�j�Y���ŁA�t�@�C���̓��e��ARM7����󂯎��܂��B
                ������SVCSHA1Context���w�肵�Ă����ꍇ�A�R�s�[�̂��ł�SHA1��
                �v�Z���s���܂��B

                [LoadBuffer���J�j�Y��]
                WRAM[B]�𗘗p���āAARM7,ARM9�Ԃ̃f�[�^�]�����s���܂��B
                WRAM[B]�̊e�X���b�g���o�P�c�����[�����œn���܂��B
                1�X���b�g���̃f�[�^�܂��͑S�f�[�^���i�[���ꂽ�Ƃ��AARM7����
                FIRM_PXI_ID_LOAD_PIRIOD����M���܂��B
                ARM9�͎�M��ɂ��̃X���b�g�̎g�p����ARM9�ɕύX���ăf�[�^��
                ���o���A������Ƀ��������N���A����(�Z�L�����e�B)�A�g�p����
                ARM7�ɖ߂��܂��B

                [�g�p����]
                WRAM[B]�����b�N�����A������ԂƂ���ARM7���ɓ|���Ă������ƁB

                [���ӓ_]
                offset��size��ARM7����ʒm����܂���B�ʂ̌o�H�œ���������Ă��������B
                SRL�t�@�C����ǂݍ��ޏꍇ�́A�݂���ROM�w�b�_���Q�Ƃł���Ώ\���ł��B
                (ROM�w�b�_�����͌�����m���Ă���͂�)

                �⑫:
                �����ł́A���郉�C�u��������ARM7/ARM9���ŕ��������킹���邱�Ƃ�
                �O��ɂ��Ă��邪�A�ėp�I�ɂ���ɂ�(�Ɨ����C�u����������Ȃ�)�A
                ����M�ŃX���b�g�𔼕����Ƃ��A���ꂼ��Ɏ�M����PXI�R�[���o�b�N
                ���X���b�h��p�ӂ��A���M��API���f�[�^��WRAM�Ɋi�[������A������
                dest��size��ʒm����Ƃ����`��OK�ł͂Ȃ����H
                (�Ŋ���������Ԏ���Ԃ�)

  Arguments:    dest        destination address for received data
                size        size to load
                ctx         context for SHA1 if execute SVC_SHA1Update

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadBuffer(u8* dest, u32 size, SVCSHA1Context *ctx);

/*---------------------------------------------------------------------------*
  Name:         MI_LoadHeader

  Description:  load header

  Arguments:    pool        pointer to the pool info for SVCSignHeapContext
                rsa_key     key address

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadHeader( SVCSignHeapContext* pool, const void* rsa_key );

/*---------------------------------------------------------------------------*
  Name:         MI_LoadStatic

  Description:  load static binary

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadStatic( void );

/*---------------------------------------------------------------------------*
  Name:         MI_Boot

  Description:  boot

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_Boot( void );

#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_MI_LOADER_H_ */
#endif
