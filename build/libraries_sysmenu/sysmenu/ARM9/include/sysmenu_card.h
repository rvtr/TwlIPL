/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_card.h

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

#ifndef SYSM_CARD_H_
#define SYSM_CARD_H_

#include <twl.h>


#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
//     					�J�[�h�������o
//
//�EARM7 ���J�[�h����̃_�E�����[�h����������܂ł͌��o���J�n���܂���B
//�E���o�������̓J�[�h�o�X�ւ̃A�N�Z�X�����b�N���܂��B
//�E�A�v���P�[�V�������N������O�� SYSM_FinalizeCardPulledOut()
//  ���Ăяo���Ċm���Ɍ��o����������������悤�ɂ��ĉ������B
//----------------------------------------------------------------------

BOOL SYSM_IsCardPulledOut(void);


//----------------------------------------------------------------------
//     					�J�[�h�������o�I������
//----------------------------------------------------------------------

void SYSM_FinalizeCardPulledOut(void);


//----------------------------------------------------------------------
//     					�J�[�h�������o��������
//----------------------------------------------------------------------

BOOL SYSM_IsDetectingCardPulledOut(void);


//----------------------------------------------------------------------
//     					�J�[�h�������o������
//----------------------------------------------------------------------

void SYSMi_InitCardPulledOut(void);


//----------------------------------------------------------------------
//     					�J�[�hID�ǂݍ���
//----------------------------------------------------------------------

u32 SYSMi_ReadCardID(void);


//----------------------------------------------------------------------
//     					�J�[�h�f�[�^�ǂݍ���
//----------------------------------------------------------------------

void SYSM_ReadCard(void *romp, void *ramp, s32 size);


//----------------------------------------------------------------------
//                      �J�[�h�̃f�[�^�]���̓��f�B���H
//----------------------------------------------------------------------

#define SYSMi_IsCardDataReady()                             \
                                                            \
    (*(vu32 *)REG_CARDCNT & CARD_DATA_READY)

//�E�J�[�h�̃f�[�^�]���̓��f�B���ǂ�����Ԃ��܂��B

//----------------------------------------------------------------------
//                      �J�[�h�f�[�^�҂�
//----------------------------------------------------------------------

#define SYSMi_WaitCardData()                                \
{                                                           \
    while (!SYSMi_IsCardDataReady())    ;                   \
}

//�E�J�[�h�f�[�^�]���̏I����҂��܂��B


//----------------------------------------------------------------------
//                      �J�[�h�̓r�W�[���H
//----------------------------------------------------------------------

#define SYSMi_IsCardBusy()                                  \
                                                            \
    (*(vu32 *)REG_CARDCNT & CARD_START)

//�E�J�[�h���r�W�[���ǂ�����Ԃ��܂��B

//----------------------------------------------------------------------
//                      �J�[�h�҂�
//----------------------------------------------------------------------

#define SYSMi_WaitCard()                                    \
{                                                           \
    while (SYSMi_IsCardBusy())   ;                          \
}

//�E�J�[�h�̏I����҂��܂��B


//----------------------------------------------------------------------
//                      �R���g���[���p�����[�^�l���iGAME���[�h�j
//----------------------------------------------------------------------

#define SYSMi_GetCardCnt4Game()                             \
                                                            \
    (*(vu32 *)MROMCNT_GAME_BUF)

//�EGAME���[�h�̃R���g���[���p�����[�^���l�����܂��B



//----------------------------------------------------------------------
//                      �q�n�l�G���A�E�}�b�v
//----------------------------------------------------------------------

#define MROM_SECURE_AREA    0x4000              // SECURE�G���A
#define MROM_GAME_AREA      0x8000              //   GAME�G���A

//----------------------------------------------------------------------
//                      �q�n�l�G���A�E�T�C�Y
//----------------------------------------------------------------------

#define MROM_SEGMENT_SIZE   0x1000              // �Z�O�����g�T�C�Y
#define MROM_SECURE_SIZE    0x4000              // SECURE�G���A�T�C�Y

#ifndef MROM_PAGE_SIZE
#define MROM_PAGE_SIZE          512                 // �}�X�N�q�n�l�E�y�[�W
#endif

//----------------------------------------------------------------------
//                      �������E�}�b�v
//----------------------------------------------------------------------

#define MROMCNT_GAME_BUF    (HW_ROM_HEADER_BUF + 0x60) //   GAME���[�h�E�R���g���[���f�[�^
#define MROMCNT_SECURE_BUF  (HW_ROM_HEADER_BUF + 0x64) // SECURE���[�h�E�R���g���[���f�[�^


//----------------------------------------------------------------------
//                      ���W�X�^�E�A�h���X
//----------------------------------------------------------------------

#ifndef REG_BASE
#define REG_BASE                0x04000000          // ���W�X�^�Q
#endif
#ifndef REG_IME
#define REG_IME             (REG_BASE + 0x208)  // ���荞�݃}�X�^�C�l�[�u��
#endif

#define REG_CARDMST_SPI_CNT (REG_BASE + 0x1a0)  // �J�[�h�}�X�^�[���r�o�h�R���g���[��

#define REG_CARD_MASTER_CNT (REG_BASE + 0x1a1)  // �J�[�h�}�X�^�[�R���g���[��

#define REG_CARD_SPI_CNT    (REG_BASE + 0x1a0)  // �J�[�h�r�o�h�R���g���[��
#define REG_CARD_SPI_DATA   (REG_BASE + 0x1a2)  //             �f�[�^

#define REG_CARDCNT         (REG_BASE + 0x1a4)  // �J�[�h�R���g���[��
#define REG_CARD_CMD        (REG_BASE + 0x1a8)  //       �R�}���h�ݒ�
#define REG_CARD_DATA       (REG_BASE + 0x100010) //     �f�[�^


//----------------------------------------------------------------------
//                      �J�[�h �}�X�^�[�R���g���[��
//----------------------------------------------------------------------

#define CARDMST_SEL_DEVICE          0x20        // �f�o�C�X�I��
#define CARDMST_SEL_ROM             0x00        // �}�X�N�q�n�l�^�R�c�������I��
#define CARDMST_SEL_SPI             0x20        // �r�o�h�I��

#define CARDMST_IF_ENABLE           0x40        // ���荞�ݗv�� ����
#define CARDMST_ENABLE              0x80        // �J�[�h�C�l�[�u��


//----------------------------------------------------------------------
//                      �J�[�h�A�N�Z�X �R���g���[��
//----------------------------------------------------------------------

#define CARD_LATENCY1_CYCLES_MASK   0x00001fff  // ���C�e���V�P�̃T�C�N����
#define CARD_LATENCY2_CYCLES_MASK   0x003f0000  // ���C�e���V�Q�̃T�C�N����
#define CARD_LATENCY_MASK           0x003f1fff  // ��L�����킹���}�X�N

#define CARD_LATENCY1_CYCLES_SHIFT  0
#define CARD_LATENCY2_CYCLES_SHIFT  16

#define CARD_DATA_SCRAMBLE_ON      	0x00002000	// �f�[�^�X�N�����u�� �n�m
#define CARD_SCRAMBLE_UNIT_ON      	0x00004000	// �X�N�����u����H �n�m
#define CARD_CMD_SCRAMBLE_ON       	0x00400000	// �R�}���h�X�N�����u�� �n�m
                                              	// �X�N�����u���t���O�Q�̃Z�b�g
#define CARD_SCRAMBLE_SET_MASK     	( CARD_SCRAMBLE_UNIT_ON | CARD_DATA_SCRAMBLE_ON \
                                   	| CARD_CMD_SCRAMBLE_ON)

#define CARD_DATA_READY             0x00800000  // �f�[�^ ���f�B

#define CARD_1_PAGE                 0x01000000  //   �P�y�[�W
#define CARD_STATUS                 0x07000000  // �X�e�[�^�X���[�h

#define CARD_RESET_LO               0x00000000  // ���Z�b�g�M�����x�� �k��
#define CARD_RESET_HI               0x20000000  //                    �g��
#define CARD_ACCESS_MODE            0x40000000  // �A�N�Z�X ���[�h
#define CARD_READ_MODE              0x00000000  // ���[�h���[�h
#define CARD_WRITE_MODE             0x40000000  // ���C�g���[�h
#define CARD_START                  0x80000000  // �X�^�[�g

// �\���̃����o�p�萔

#define ST_CARD_1_PAGE              1           //   �P�y�[�W
#define ST_CARD_STATUS              7           // �X�e�[�^�X���[�h

#define ST_CARD_READ_MODE           0           // ���[�h���[�h
#define ST_CARD_WRITE_MODE          1           // ���C�g���[�h


//----------------------------------------------------------------------
//                      �}�X�N�q�n�l�R�}���h
//----------------------------------------------------------------------

// GAME���[�h

#define MROMOP_G_OP_MASK              0xff000000  // �R�}���h�}�X�N

#define MROMOP_G_READ_ID              0xb8000000  // �h�c�ǂݍ���
#define MROMOP_G_READ_PAGE            0xb7000000  // �y�[�W�ǂݍ���



#ifdef __cplusplus
}       // extern "C"
#endif

#endif  // SYSM_CARD_H_
