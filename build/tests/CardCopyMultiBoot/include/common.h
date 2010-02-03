/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     common.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
*---------------------------------------------------------------------------*/
#ifndef MB_DEMO_COMMON_H_
#define MB_DEMO_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>

#define CARD_SECURE_AREA_OFFSET       0x4000 // 16KB
#define CARD_GAME_AREA_OFFSET         0x8000 // 32KB


//============================================================================
//  �֐��錾
//============================================================================


/*
 * ���̊֐��� multiboot-Model �̐e�@���� NitroMain() ���������̂ł�.
 * �{�T���v���ł� MB_IsMultiBootChild() == FALSE ����ŌĂяo����܂�.
 */
void    ParentMain(void);

/*
 * ���̊֐��� multiboot-Model �̎q�@���� NitroMain() ���������̂ł�.
 * �{�T���v���ł� MB_IsMultiBootChild() == TRUE ����ŌĂяo����܂�.
 */
void    ChildMain(void);

/*
 * ���̊֐��͐e�@��p�̈� .parent �Z�N�V�����ɔz�u����Ă��܂�.
 * ���e��, �P�� ParentMain() ���Ăяo�������ł�.
 */
void    ParentIdentifier(void);

/* ����ȊO�͑S�� multiboot-Model �Ɠ����ł� */

void    CommonInit();
void    ReadKey(void);
u16     GetPressKey(void);
u16     GetTrigKey(void);
void    InitAllocateSystem(void);

/*---------------------------------------------------------------------------*
  Name:         IS_PAD_PRESS

  Description:  �L�[����

  Arguments:    ���肷��L�[�t���O

  Returns:      �w�肵���L�[����������Ă���ꍇ�� TRUE
                ����Ă��Ȃ��ꍇ�� FALSE
 *---------------------------------------------------------------------------*/
static inline BOOL IS_PAD_PRESS(u16 flag)
{
    return (GetPressKey() & flag) == flag;
}

/*---------------------------------------------------------------------------*
  Name:         IS_PAD_TRIGGER

  Description:  �L�[�g���K����

  Arguments:    ���肷��L�[�t���O

  Returns:      �w�肵���L�[�̃g���K�������Ă���ꍇ�� TRUE
                �����Ă��Ȃ��ꍇ�� FALSE
 *---------------------------------------------------------------------------*/
static inline BOOL IS_PAD_TRIGGER(u16 flag)
{
    return (GetTrigKey() & flag) == flag;
}

/*---------------------------------------------------------------------------*
  Name:         MpSendCallback

  Description:  MP�f�[�^���M��ɌĂяo�����֐��B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MpSendCallback(BOOL result);

/*---------------------------------------------------------------------------*
  Name:         MpReceiveCallback

  Description:  MP�f�[�^��M���ɌĂяo�����֐��B

  Arguments:    aid     - ���M���q�@�� aid( 0 �̏ꍇ�͐e�@����̃f�[�^ )
                data    - ��M�f�[�^�ւ̃|�C���^ (NULL �Őؒf�ʒm)
                length  - ��M�f�[�^�̃T�C�Y

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MpReceiveCallback(u16 aid, u16 *data, u16 length);

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  �e�@ �ʐM��ʂł̏����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeParent(void);

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  �q�@ �ʐM��ʂł̏����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeChild(void);

/*---------------------------------------------------------------------------*
  Name:         BlockTransferMain

  Description:  �u���b�N�]����Ԓʒm�֐��B

  Arguments:    arg     - �ʒm�� WM �֐��̃R�[���o�b�N�|�C���^

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BlockTransferMain(void);

/*---------------------------------------------------------------------------*
  Name:         BlockTransferCallback

  Description:  �u���b�N�]����Ԓʒm�֐��B

  Arguments:    arg     - �ʒm�� WM �֐��̃R�[���o�b�N�|�C���^

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BlockTransferCallback(void *arg);

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // MB_DEMO_COMMON_H_
