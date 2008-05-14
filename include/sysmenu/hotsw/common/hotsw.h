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
//#define USE_WRAM_LOAD

// enum   -------------------------------------------------------------------
// �X���b�h�ɑ��郁�b�Z�[�W�̃X�e�[�g
typedef enum HotSwMessageType{
	HOTSW_INSERT = 0,
    HOTSW_PULLOUT,
    HOTSW_CONTROL
} HotSwMessageType;

typedef enum ModeType{
	HOTSW_MODE1 = 0,
    HOTSW_MODE2
} ModeType;

typedef enum HotSwApliType{
    HOTSW_APLITYPE_CARD = 0,
    HOTSW_APLITYPE_NTR_NAND,
    HOTSW_APLITYPE_TWL_NAND
} HotSwApliType;

// union  -------------------------------------------------------------------
// PXI�p���b�Z�[�W
typedef union HotSwPxiMessage{
    struct {
    	u32		value	:1;
    	u32		ctrl	:1;
        u32		finalize:1;
        u32		read	:1;
        u32		bootType:8;
    	u32		:20;
    } msg;
    u32 data;
} HotSwPxiMessage;

// struct -------------------------------------------------------------------
// �X���b�h�p���b�Z�[�W
typedef struct HotSwMessage{
    u32				 value;
    BOOL			 ctrl;
    BOOL			 finalize;
    BOOL			 read;
	HotSwMessageType type;
    HotSwApliType    apli;
} HotSwMessage;


// Function prototype -------------------------------------------------------
// --- ARM9
#ifdef SDK_ARM9
// PXI�ʐM��ARM7�Ɋ����}���L���^������ʒm
void HOTSW_EnableHotSWAsync( BOOL enable );

// PXI�ʐM��ARM7�Ɋ����}��Finalize������ʒm
void HOTSW_FinalizeHotSWAsync( HotSwApliType apliType );

// �����}���̋���/�}���̏�Ԃ�Ԃ�
BOOL HOTSW_isEnableHotSW(void);

// �J�[�h�A�v���̃��[�h���������Ă��邩��Ԃ�
BOOL HOTSW_isCardLoadCompleted(void);

#ifdef USE_WRAM_LOAD
// �J�[�h�f�[�^��ǂݏo���֐�
CardDataReadState HOTSW_ReadCardData(void* src, void* dest, u32 size);

// �J�[�h���Q�[�����[�h�ɂȂ������ǂ���
BOOL HOTSW_isGameMode(void);
#endif

// --- ARM7
#else
// ����}�������̏�����
void HOTSW_Init(u32 threadPrio);

// �J�[�h�̑��ݔ���
BOOL HOTSW_IsCardExist(void);

// �J�[�h�ɃA�N�Z�X�ł����Ԃ�����
BOOL HOTSW_IsCardAccessible(void);

// Boot Segment �o�b�t�@�̎w��
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment �o�b�t�@�̎w��
void HOTSW_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);

// IS�f�o�b�K��œ��삵�Ă��邩�H
BOOL HOTSWi_IsRunOnDebugger(void);

// ROM���G�~�����[�V�������Ă��邩�H
BOOL HOTSWi_IsRomEmulation(void);

// �f�o�b�K�ʐM�p�ɃJ�[�h�X���b�g�̓d����ON�ɂ���B
void HOTSWi_TurnCardPowerOn(u32 slot);
#endif

// Rom�G�~�����[�V���������i�[���Ă���o�b�t�@�̃|�C���^��Ԃ�
SDK_INLINE void* HOTSW_GetRomEmulationBuffer(void)
{
	return (void*)&SYSMi_GetWork()->romEmuInfo;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __SYSMENU_HOTSW_COMMON_HOTSW_H__
