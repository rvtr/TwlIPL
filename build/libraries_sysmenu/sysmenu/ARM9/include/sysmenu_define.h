/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_define.h

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

#ifndef __SYSM_DEFINE_H__
#define __SYSM_DEFINE_H__

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data-------------------------------------------

	// �萔
#define POW_LCDC_SW_TOP					0x0000			// LCD��ɉ�ʕ\��
#define POW_LCDC_SW_BOTTOM				0x8000			// LCD���ɉ�ʕ\��
#define POW_E2D2						0x0200			// 2D��H2 �̓d��ON
#define POW_E2D1						0x0002			// 2D��H1 �̓d��ON
#define POW_ELC							0x0001			// LCDC��H�̓d��ON

#define SUBP_RECV_IF_ENABLE				0x4000			// ���荞�ݗv����M ����

	// �r������p���b�NID
#define BOOTFLAG_LOCK_ID				(OS_MAINP_LOCK_ID_END-1)
#define CARTRIDGE_LOCK_ID				(OS_MAINP_LOCK_ID_END-2)

	// SetBootFlag�֐��̈���int set_clear_flag�Ŏw�肷��萔
#define SBF_CLEAR						0
#define SBF_SET							1

#ifdef __cplusplus
}
#endif

#endif  // __SYSM_DEFINE_H__
