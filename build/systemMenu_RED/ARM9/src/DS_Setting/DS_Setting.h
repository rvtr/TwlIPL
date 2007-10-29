/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_Setting/DS_Setting.h

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

#ifndef	__DS_SETTING_H__
#define	__DS_SETTING_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <twl.h>
#include "font.h"
#include "unicode.h"

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
extern u16 csrMenu;
extern BOOL initialSet;

// function-------------------------------------------------------------
extern int	DS_SettingMain( void );

extern void SEQ_MainMenu_init(void);
extern int  SEQ_MainMenu(void);
extern void SEQ_Setting_init(void);
extern int  SEQ_Setting(void);
extern void SEQ_OwnerInfo_init(void);
extern int  SEQ_OwnerInfo(void);
extern void SEQ_RtcSet_init(void);
extern int  SEQ_RtcSet(void);
extern void SEQ_LangSelect_init(void);
extern int  SEQ_LangSelect(void);
extern void SEQ_TP_Calibration_init(void);
extern int  SEQ_TP_Calibration(void);
extern void SEQ_AgbLcdSelect_init(void);
extern int  SEQ_AgbLcdSelect(void);
extern void SEQ_AutoBootSelect_init(void);
extern int  SEQ_AutoBootSelect(void);

extern void DrawMenu(u16 nowCsr, const MenuComponent *menu);
extern BOOL SelectMenuByTp(u16 *nowCsr, const MenuComponent *menu);
//extern BOOL InRangeTp(u16 lt_x, u16 lt_y, u16 rb_x, u16 rb_y, TPData *tgt);
extern BOOL InRangeTp(int lt_x, int lt_y, int rb_x, int rb_y, TPData *tgt);

extern void DrawOKCancelButton(void);
extern void CheckOKCancelButton(BOOL *tp_ok, BOOL *tp_cancel);
extern void InputDecimal(int *tgtp, InputNumParam *inpp);

extern void ClearRTC( void );

#ifdef __cplusplus
}
#endif

#endif  // __DS_SETTING_H__
