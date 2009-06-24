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

// Define -------------------------------------------------------------------
#define SYSM_HOTSW_ENABLE_ROMEMU
#define USE_WRAM_LOAD
#define INITIAL_KEYTABLE_PRELOAD
#define HOTSW_FINAL_VERSION
//#define USE_NEW_DMA
//#define HOTSW_NO_MESSAGE					// Printf�}���X�C�b�`

#ifndef SDK_FINALROM
	#ifdef  HOTSW_NO_MESSAGE
		#define HOTSW_TPrintf( ... )        ((void)0)
		#define HOTSW_PutString( ... )      ((void)0)
	#else
		#define HOTSW_TPrintf				OS_TPrintf
		#define HOTSW_PutString				OS_PutString
	#endif
#else
	#define HOTSW_TPrintf( ... )        	((void)0)
	#define HOTSW_PutString( ... )      	((void)0)
#endif

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

typedef enum HotSwCardState{
    HOTSW_CARD_STATE_POWER_OFF = 0,
    HOTSW_CARD_STATE_NORMAL_MODE,
    HOTSW_CARD_STATE_GAME_MODE,
    HOTSW_CARD_STATE_KEEP
} HotSwCardState; // �� HotSwApliType

// union  -------------------------------------------------------------------
// PXI�p���b�Z�[�W
typedef union HotSwPxiMessageForArm7{
    struct {
    	u32		value	 :1;
    	u32		ctrl	 :1;
        u32		finalize :1;
        u32		read	 :1;
        u32		cardState:8;
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
    HotSwCardState   state;
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
void HOTSW_FinalizeHotSWAsync( HotSwCardState cardState );

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

#ifdef INITIAL_KEYTABLE_PRELOAD
// DS�݊�Blowfish�e�[�u����WRAM�o�R�Ń��[�J���ɃR�s�[����
void HOTSW_CopyInitialKeyTable(void);
#endif

// �J�[�h�̑��ݔ���
BOOL HOTSW_IsCardExist(void);

// �J�[�h�ɃA�N�Z�X�ł����Ԃ�����
BOOL HOTSW_IsCardAccessible(void);

// Boot Segment �o�b�t�@�̎w��
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment �o�b�t�@�̎w��
void HOTSW_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);
// ��L�̃o�b�t�@��N���A�o�[�W����
void HOTSWi_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size);

// ROM���G�~�����[�V�������Ă��邩�H
BOOL HOTSWi_IsRomEmulation(void);

// �f�o�b�K�ʐM�p�ɃJ�[�h�X���b�g�̓d����ON�ɂ���B
void HOTSWi_TurnCardPowerOn(u32 slot);

// �J�[�h���[�N�擾
void *HOTSWi_GetCardBootData(void);

#ifdef USE_WRAM_LOAD
// NAND�A�v���pKeyTable�̐���
void HOTSWi_MakeBlowfishTableDSForNAND(void);
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
