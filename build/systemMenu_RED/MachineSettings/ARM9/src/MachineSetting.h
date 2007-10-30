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
#define TP_CSR_TOUCH_COUNT					2						// TPカーソルのチャタリング吸収のためのカウント値
#define TP_CSR_DETACH_COUNT					2						// TPカーソルを「選択」と判定するTPデタッチからのカウント値

#define HANDLE_MENU							48
#define HANDLE_RTC_VIEW						240
#define HANDLE_OK_BUTTON					255
#define HANDLE_CANCEL_BUTTON				256

// 数値入力インターフェース用ワーク（void InputDecimal()で使用）
typedef struct InputNumParam {
	u16			pos_x;						// 入力値の表示X位置
	u16			pos_y;						// 〃　　　　  Y位置
	int			up_count;
	int			down_count;
	int			keta_max;					// 最大桁
	int			value_min;					// 入力値の最小
	int			value_max;					// 入力値の最大
	int			y_offset;					// タッチパネル入力の基準位置からのYオフセット
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
