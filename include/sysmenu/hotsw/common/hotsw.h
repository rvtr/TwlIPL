/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     hotsw.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: #$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef __SYSMENU_HOTSW_COMMON_HOTSW_H__
#define __SYSMENU_HOTSW_COMMON_HOTSW_H__

#include 	<twl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYSM_HOTSW_ENABLE_ROMEMU

// enum   -------------------------------------------------------------------
// �X���b�h�ɑ��郁�b�Z�[�W�̃X�e�[�g
typedef enum HotSwMessageType{
	HOTSW_INSERT = 0,
    HOTSW_PULLOUT,
    HOTSW_CONTROL
} HotSwMessageType;

typedef enum ModeType{
	HOTSW_MODE1,
    HOTSW_MODE2
} ModeType;

// union  -------------------------------------------------------------------
// PXI�p���b�Z�[�W
typedef union HotSwPxiMessage{
    struct {
    	u32		value	:1;
    	u32		ctrl	:1;
    	u32		:30;
    } msg;
    u32 data;
} HotSwPxiMessage;

// struct -------------------------------------------------------------------
// �X���b�h�p���b�Z�[�W
typedef struct HotSwMessage{
    u32				 value;
    BOOL			 ctrl;
	HotSwMessageType type;
} HotSwMessage;


// Function prototype -------------------------------------------------------
// ����}�������̏�����
void HOTSW_Init(void);

// �J�[�h�̑��ݔ���
BOOL HOTSW_IsCardExist(void);

// �J�[�h�ɃA�N�Z�X�ł����Ԃ�����
BOOL HOTSW_IsCardAccessible(void);

// Boot Segment �o�b�t�@�̎w��
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment �o�b�t�@�̎w��
void HOTSW_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);

// Rom�G�~�����[�V���������i�[���Ă���o�b�t�@�̃|�C���^��Ԃ�
#ifdef SDK_ARM7
void* HOTSW_GetRomEmulationBuffer(void);
#else // SDK_ARM9
SDK_INLINE void* HOTSW_GetRomEmulationBuffer(void)
{
	return (void*)HW_ISD_RESERVED;
}
#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __SYSMENU_HOTSW_COMMON_HOTSW_H__
