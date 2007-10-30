/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     MachineSetting.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef	__MACHINE_SETTING_H__
#define	__MACHINE_SETTING_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <twl.h>


// define data----------------------------------------------------------
#define TP_CSR_TOUCH_COUNT					2						// TP�J�[�\���̃`���^�����O�z���̂��߂̃J�E���g�l
#define TP_CSR_DETACH_COUNT					2						// TP�J�[�\�����u�I���v�Ɣ��肷��TP�f�^�b�`����̃J�E���g�l

#define HANDLE_MENU							48
#define HANDLE_RTC_VIEW						240
#define HANDLE_OK_BUTTON					255
#define HANDLE_CANCEL_BUTTON				256

// ���l���̓C���^�[�t�F�[�X�p���[�N�ivoid InputDecimal()�Ŏg�p�j
typedef struct InputNumParam {
	u16			pos_x;						// ���͒l�̕\��X�ʒu
	u16			pos_y;						// �V�@�@�@�@  Y�ʒu
	int			up_count;
	int			down_count;
	int			keta_max;					// �ő包
	int			value_min;					// ���͒l�̍ŏ�
	int			value_max;					// ���͒l�̍ő�
	int			y_offset;					// �^�b�`�p�l�����͂̊�ʒu�����Y�I�t�Z�b�g
}InputNumParam;

// global variable------------------------------------------------------
extern NNSFndAllocator g_allocator;
extern BOOL g_initialSet;
extern int (*g_pNowProcess)( void );

// function-------------------------------------------------------------
extern void MachineSettingInit( void );
extern int  MachineSettingMain( void );
extern void SetOwnerInfoInit( void );
extern int  SetOwnerInfoMain( void );
extern void SetRTCInit( void );
extern int  SetRTCMain( void );
extern void SelectLanguageInit( void );
extern int  SelectLanguageMain( void );
extern void TP_CalibrationInit( void );
extern int  TP_CalibrationMain( void );

extern void DrawOKCancelButton( void );
extern void CheckOKCancelButton(BOOL *tp_ok, BOOL *tp_cancel);
extern void InputDecimal(int *tgtp, InputNumParam *inpp);

extern void ClearRTC( void );

#ifdef __cplusplus
}
#endif

#endif  // __MACHINE_SETTING_H__
