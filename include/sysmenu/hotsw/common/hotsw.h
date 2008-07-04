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
#define USE_WRAM_LOAD
//#define USE_NEW_DMA

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
typedef union HotSwPxiMessageForArm7{
    struct {
    	u32		value	:1;
    	u32		ctrl	:1;
        u32		finalize:1;
        u32		read	:1;
        u32		bootType:8;
    	u32		:20;
    } msg;
    u32 data;
} HotSwPxiMessageForArm7;

typedef union HotSwPxiMessageForArm9{
    struct {
    	u32		mode	:1;
    	u32		insert	:1;
        u32		pullout :1;
        u32		read	:1;
        u32		result  :8;
    	u32		:20;
    } msg;
    u32 data;
} HotSwPxiMessageForArm9;

// struct -------------------------------------------------------------------
// �X���b�h�p���b�Z�[�W
typedef struct HotSwMessageForArm7{
    u32				 value;
    BOOL			 ctrl;
    BOOL			 finalize;
    BOOL			 read;
	HotSwMessageType type;
    HotSwApliType    apli;
} HotSwMessageForArm7;

typedef struct HotSwMessageForArm9{
    BOOL			 	isGameMode;
    BOOL			 	isInsert;
    BOOL			 	isPulledOut;
    BOOL				isReadComplete;
	CardDataReadState 	result;
} HotSwMessageForArm9;


// Function prototype -------------------------------------------------------
// --- ARM9
#ifdef SDK_ARM9
// PXI�ʐM��ARM7�Ɋ����}���L���^������ʒm�B�J�[�h�֘A�̃t���O�������s��(�񓯊���)
void HOTSW_EnableHotSWAsync( BOOL enable );

// PXI�ʐM��ARM7�Ɋ����}���L���^������ʒm�B�J�[�h�֘A�̃t���O�������s��(������)
void HOTSW_EnableHotSW( BOOL enable );

// PXI�ʐM��ARM7�Ɋ����}��������ʒm�B(�񓯊���)
void HOTSW_InvalidHotSWAsync( void );

// PXI�ʐM��ARM7�Ɋ����}��������ʒm�B(������)
void HOTSW_InvalidHotSW( void );

// PXI�ʐM��ARM7�Ɋ����}��Finalize������ʒm
void HOTSW_FinalizeHotSWAsync( HotSwApliType apliType );

// �����}���̋���/�}���̏�Ԃ�Ԃ�
BOOL HOTSW_isEnableHotSW(void);

// �J�[�h�A�v���̃��[�h���������Ă��邩��Ԃ�
BOOL HOTSW_isCardLoadCompleted(void);

//  �I��������������������Ԃ�
BOOL HOTSW_isFinalized(void);

// �����}�����������ǂ�����Ԃ�
BOOL HOTSW_isBusyHotSW(void);

#ifdef USE_WRAM_LOAD
// ����}�������̏�����
void HOTSW_Init();

// �J�[�h�f�[�^��ǂݏo���֐�(������)
CardDataReadState HOTSW_ReadCardData(void* src, void* dest, u32 size);

// �J�[�h�f�[�^��ǂݏo���֐�(�񓯊���)
CardDataReadState HOTSW_ReadCardDataAsync(void* src, void* dest, u32 size);

// �J�[�h��Game���[�h�ɂȂ������̃R�[���o�b�N�֐���ݒ�
void HOTSW_SetGameModeCallBackFunction(OSIrqFunction function);

// �J�[�h���h���������̃R�[���o�b�N�֐���ݒ�
void HOTSW_SetCardInsertCallBackFunction(OSIrqFunction function);

// �J�[�h�����������̃R�[���o�b�N�֐���ݒ�
void HOTSW_SetCardPullOutCallBackFunction(OSIrqFunction function);

// �J�[�h���Q�[�����[�h�ɂȂ������ǂ���
BOOL HOTSW_isGameMode(void);

// Game���[�h�̃f�[�^�ǂ݊֐�
CardDataReadState HOTSW_ReadCardDataOnGameMode(const void* src, void* dest, u32 size);
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

#ifdef USE_WRAM_LOAD
// �����ŗ^����ꂽ�o�b�t�@����2KB���̗̈��Blowfish�ŕ���������
BOOL HOTSW_DecryptObjectFile(void* dest);
#endif
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
