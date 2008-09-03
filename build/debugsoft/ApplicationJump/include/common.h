 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - appjumpTest
  File:     common.h

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
#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <twl.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

/* TitleID */
#define CARDAPP_TITLEID		(u64)(0x0003000034333041)	// 430A
#define CARDAPP_FAIL_TITLEID	(u64)(0x0003000034363041)	// 460A
#define CARDAPP_ANO_TITLEID	(u64)(0x0003000034363141)	// 461A

#define NANDAPP1_TITLEID	(u64)(0x0003000434333141)	// 431A
#define NANDAPP2_TITLEID	(u64)(0x0003000434333241)	// 432A

#define KEY_REPEAT_START    25  // �L�[���s�[�g�J�n�܂ł̃t���[����
#define KEY_REPEAT_SPAN     10  // �L�[���s�[�g�̊Ԋu�t���[����

/* �A�v���ԃp�����[�^�֘A */
#define APPJUMP_STRING_LENGTH		24	// ��������Ƃ��Ď󂯓n������镶����̒�������

/*---------------------------------------------------------------------------*
    �\���� ��`
 *---------------------------------------------------------------------------*/

typedef enum JumpTypeForB
{
	JUMPTYPE_RETURN = 0,
	JUMPTYPE_ANOTHER_CARD,
	JUMPTYPE_SYSMENU,
	JUMPTYPE_FAIL_CARD,
	
	JUMPTYPE_NUM
} JumpTypeForB;

// �L�[���͏��
typedef struct KeyInfo
{
    u16 cnt;    // �����H���͒l
    u16 trg;    // �����g���K����
    u16 up;     // �����g���K����
    u16 rep;    // �����ێ����s�[�g����
} KeyInfo;

// �A�v���ԂŃo�C�i���f�[�^�Ƃ��Ĉ����n���\����
typedef struct AppParam
{
	u32 jumpCount;		// �A�v���W�����v�̎��s��
	u8  isAutoJump;		// ���Ԋu�Ŏ����I�ɃA�v���W�����v�����s���邩�ǂ����̃t���O
	u8  rsv[3];			// 4�o�C�g�A���C�������g�̂���
} AppParam;

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
void InitCommon(void);

void ReadKey(KeyInfo* pKey);

void VBlankIntr(void);

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* COMMON_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
