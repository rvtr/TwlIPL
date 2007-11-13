/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     Card.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include 	<twl/os/common/format_rom.h>

#include	<nitro/card/types.h>
#include 	"blowfish.h"
#include 	"Card.h"
#include	"dsCardType1.h"

// define -------------------------------------------------------------------
#define		STACK_SIZE							1024		// �X�^�b�N�T�C�Y
#define		MC_THREAD_PRIO						11			// �J�[�h�d��ON �� �Q�[�����[�h�̃X���b�h�D��x

// Function prototype -------------------------------------------------------
static BOOL IsCardExist(void);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCard(void);
static void InterruptCallbackCardDet(void);
static void InterruptCallbackCardData(void);

static void McThread(void *arg);
static void McPowerOn(void);
static void SetMCSCR(void);

static void ShowRegisterData(void);
static void ShowRomHeaderData(void);

// Static Values ------------------------------------------------------------
static u64  				s_MCStack[STACK_SIZE / sizeof(u64)];
static OSThread 			s_MCThread;

static u32					s_SecureSegBufSize, s_BootSegBufSize;

static u32					*s_pSecureSegBuffer;	// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�
static BootSegmentData		*s_pBootSegBuffer;		// �J�[�h�����Ă��o�b�t�@�̏ꏊ�o���Ƃ�

static CardBootData			s_cbData;

static CardBootFunction  	s_funcTable[] = {
    {ReadIDNormal_DSType1, ReadBootSegNormal_DSType1, ChangeModeNormal_DSType1,
     ReadIDSecure_DSType1, ReadSegSecure_DSType1, SwitchONPNGSecure_DSType1, ChangeModeSecure_DSType1,
     ReadIDGame_DSType1}
};

// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         Card_Init
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void Card_Init(void)
{
	OS_InitTick();
    OS_InitThread();
    
	// ���荞�݃}�X�N�̐ݒ�
	SetInterrupt();

    // ���荞�݂̗L����
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

#ifdef SDK_ARM7
	// �`���b�^�����O�J�E���^�̒l��ݒ�
	reg_MI_MC1 = (u32)((reg_MI_MC1 & 0xffff) | 0xfff0000);

	// Counter-A�̒l��ݒ�
    reg_MI_MC2 = 0xfff;
#endif

	// �J�[�h�u�[�g�p�X���b�h�̐���
	OS_CreateThread(&s_MCThread,
                    McThread,
                    NULL,
                    s_MCStack + STACK_SIZE / sizeof(u64),
                    STACK_SIZE,
                    MC_THREAD_PRIO
                    );

    // �X���b�h�N��
    OS_WakeupThreadDirect(&s_MCThread);

	// �J�[�h�u�[�g�p�\���̂̏�����
	MI_CpuClear32(&s_cbData, sizeof(CardBootData));
}

/* -----------------------------------------------------------------
 * Card_Boot�֐�
 *
 * �J�[�h�N�����X�^�[�g
 *
 * ��BootSegmentBuffer SecureSegmentBuffer�̐ݒ���s���Ă���
 *   ���̊֐����Ă�ł��������B
 * ----------------------------------------------------------------- */
void Card_Boot(void)
{
	OS_TPrintf("---------------- Card Boot Start ---------------\n");
    
	// �J�[�h�d��ON
	McPowerOn();
    
	// VAE�EVBI�EVD�l�̐ݒ�
    s_cbData.vae = VAE_VALUE;
    s_cbData.vbi = VBI_VALUE;
	s_cbData.vd  = VD_VALUE;

	// �Z�L���A�̈�̓ǂݍ��݃Z�O�����g�擪�ԍ�(Segment4 �` Segment 7)
    s_cbData.secureSegNum = 4;

	// �o�b�t�@��ݒ�
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

	if(IsCardExist()){
    	// ---------------------- Normal Mode ----------------------
    	// �J�[�hID�ǂݍ���
		s_funcTable[0].ReadID_N(&s_cbData);
    	
    	// Boot Segment�ǂݍ���
    	s_funcTable[0].ReadBootSegment_N(&s_cbData);

		// TWL�J�[�h��NTR�J�[�h������ (Platform code : bit0 : not support NTR,  bit1 : support TWL)
        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
			OS_TPrintf("TWL Card.\n");
            
            s_cbData.twlFlg = TRUE;
        }

    	// Key Table������
    	GCDm_MakeBlowfishTableDS(&s_cbData.keyTable, &s_pBootSegBuffer->rh.s, s_cbData.keyBuf, 8);

    	// �Z�L���A���[�h�Ɉڍs
    	s_funcTable[0].ChangeMode_N(&s_cbData);

    	// ---------------------- Secure Mode ----------------------
		// PNG�ݒ�
		s_funcTable[0].SetPNG_S(&s_cbData);
    
		// DS������������H�����l�ݒ� (���W�X�^�ݒ�)
		SetMCSCR();

		// ID�ǂݍ���
    	s_funcTable[0].ReadID_S(&s_cbData);

    	// Secure�̈��Segment�ǂݍ���
    	s_funcTable[0].ReadSegment_S(&s_cbData);

    	// �Q�[�����[�h�Ɉڍs
		s_funcTable[0].ChangeMode_S(&s_cbData);
    
    	// ---------------------- Game Mode ----------------------
    	// ID�ǂݍ���
		s_funcTable[0].ReadID_G(&s_cbData);

		OS_TPrintf("-----------------------------------------------\n\n");
    }
    else{
		OS_TPrintf("Card Not Found\n");
    }
}

/* -----------------------------------------------------------------
 * IsCardExist�֐�
 *
 * �J�[�h�̑��ݔ���
 *
 * ��SCFG_MC1��Slot B ���[�h�I���t���O�����Ă���
 *
 * ���[�h�I���t���O�� 10 (�S�Ă̒[�q����L���o��) �̎��������Ă���Ɣ���
   		Slot A �̏ꍇ if((reg_MI_MC1 & 0x0c) == 0x08)
		Slot B �̏ꍇ if((reg_MI_MC1 & 0xc0) == 0x80)
 * ----------------------------------------------------------------- */
static BOOL IsCardExist(void)
{
    if((reg_MI_MC1 & SLOT_STATUS_MODE_SELECT_MSK) == SLOT_STATUS_MODE_10){
		return TRUE;
    }
    else{
		return FALSE;
    }
}

/* -----------------------------------------------------------------
 * Card_SetBootSegmentBuffer�֐�
 *
 * Boot Segment �o�b�t�@�̎w��
 *
 * ���F�J�[�h�u�[�g�������͌Ăяo���Ȃ��悤�ɂ���
 * ----------------------------------------------------------------- */
void Card_SetBootSegmentBuffer(void* buf, u32 size)
{
	SDK_ASSERT(size > BOOT_SEGMENT_SIZE);

    s_pBootSegBuffer = (BootSegmentData *)buf;
    s_BootSegBufSize = size;

    s_cbData.pBootSegBuf = s_pBootSegBuffer;

    // �o�b�t�@�̏�����
    MI_CpuClear8(s_pBootSegBuffer, size);
}

/* -----------------------------------------------------------------
 * Card_SetSecureSegmentBuffer�֐�
 *
 * Secure Segment �o�b�t�@�̎w��
 * 
 * ���F�J�[�h�u�[�g�������͌Ăяo���Ȃ��悤�ɂ���
 * ----------------------------------------------------------------- */
void Card_SetSecureSegmentBuffer(void* buf, u32 size)
{
    SDK_ASSERT(size > SECURE_SEGMENT_SIZE);
    
	s_pSecureSegBuffer = (u32 *)buf;
	s_SecureSegBufSize = size;

	s_cbData.pSecureSegBuf = s_pSecureSegBuffer;
    
    // �o�b�t�@�̏�����
    MI_CpuClear8(s_pSecureSegBuffer, size);
}

/* -----------------------------------------------------------------
 * McThread B �֐�
 * ----------------------------------------------------------------- */
static void McThread(void *arg)
{
	#pragma unused( arg )
    
    while(1){
		OS_SleepThread(NULL);

        // �J�[�h�u�[�g
        Card_Boot();
    }
}

/* -----------------------------------------------------------------
 * McPowerOn�֐�
 * ----------------------------------------------------------------- */
static void McPowerOn(void)
{
	OSTick start;

    // SCFG_MC1 �� Slot2 Status �� M1,M0 �� 01 �ɂ���
    reg_MI_MC1  = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_01);
    
	// 100ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 100){}

    // SCFG_MC1 �� Slot2 Status �� M1,M0 �� 10 �ɂ���
	reg_MI_MC1 	= (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_10);

    // ���Z�b�g��high�� (RESB = 1�ɂ���)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(1,1,0,1,1,1,1,1,1,1,1,1,1)) |
                             		 CNT1_FLD(0,0,1,0,0,0,0,0,0,0,0,0,0));

	// 100ms�҂�
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 100){}

    OS_TPrintf("MC Power ON\n");
}

/*---------------------------------------------------------------------------*
  Name:         SetMCSCR
  
  Description:  ����������H�����l�ݒ背�W�X�^��ݒ肷��

  �����F���̊֐��̓Z�L���A���[�h�ŁA
		sPNG_ON�R�}���h�����s���Ă���Ăяo���Ă��������B
 *---------------------------------------------------------------------------*/
static void SetMCSCR(void)
{
	u32 pna_l = (u32)(PNA_BASE_VALUE | (s_cbData.vd << 15));
    u32 pna_h = (u32)(s_cbData.vd >> 17);
    
    // SCR A
	reg_MCSCR0 = pna_l;

    // SCR B
	reg_MCSCR1 = PNB_L_VALUE;

    // [d0 -d6 ] -> SCR A
    // [d16-d22] -> SCR B
    reg_MCSCR2 = (u32)(pna_h | PNB_H_VALUE << 16);

	// MCCNT1 ���W�X�^�ݒ� (SCR = 1��)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(1,1,1,1,  1,1,  1,1,  1,  0,1,1,  1)) |
           		             		 CNT1_FLD(0,0,0,0,  0,0,  0,0,  0,  1,0,0,  0));
}

/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCard
				InterruptCallbackCardDet
  				InterruptCallbackCardData
  
  Description:  �e�튄�荞�݃R�[���o�b�N�֐�
 *---------------------------------------------------------------------------*/
// �J�[�hB����
static void InterruptCallbackCard(void)
{
    // M�� 10 ���� 11 �ɑJ��
	reg_MI_MC1 = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_11);
    
    // �J�[�h�u�[�g�p�\���̂̏�����
	MI_CpuClear32(&s_cbData, sizeof(CardBootData));

    // �o�b�t�@�̏�����
    MI_CpuClear8(s_pBootSegBuffer, s_BootSegBufSize);
    MI_CpuClear8(s_pSecureSegBuffer, s_SecureSegBufSize);

    // MC_CNT1��������
    reg_MCCNT1 = 0x0;

#ifdef USE_SLOT_A
	OS_SetIrqCheckFlagEx(OS_IE_CARD_A_IREQ);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_IREQ);
#endif
}

// �J�[�hB�}��
static void InterruptCallbackCardDet(void)
{
    // �J�[�h�u�[�g�X���b�h���N������
	OS_WakeupThreadDirect(&s_MCThread);

#ifdef USE_SLOT_A
    OS_SetIrqCheckFlagEx(OS_IE_CARD_A_DET);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_DET);
#endif
}

// �J�[�hB �f�[�^�]���I��
static void InterruptCallbackCardData(void)
{
#ifdef USE_SLOT_A
    OS_SetIrqCheckFlagEx(OS_IE_CARD_A_DATA);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_DATA);
#endif
}

/*---------------------------------------------------------------------------*
  Name:			SetInterruptCallback
				SetInterruptCallbackEx

  Description:  ���荞�݃R�[���o�b�N�֐��Ɗ��荞�݋��̐ݒ���s��
 *---------------------------------------------------------------------------*/
static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func )
{
    (void)OS_SetIrqFunction(intr_bit, func);
  	(void)OS_EnableIrqMask(intr_bit);
}

static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func )
{
    (void)OS_SetIrqFunctionEx(intr_bit, func);
  	(void)OS_EnableIrqMaskEx(intr_bit);
}

/*---------------------------------------------------------------------------*
  Name:         SetInterrupt

  Description:  ���荞�݃R�[���o�b�N�֐�����x�ɐݒ肷��֐�
 *---------------------------------------------------------------------------*/
static void SetInterrupt(void)
{
#ifdef USE_SLOT_A
  	SetInterruptCallback( OS_IE_CARD_A_IREQ , InterruptCallbackCard );
  	SetInterruptCallback( OS_IE_CARD_A_DET  , InterruptCallbackCardDet );
  	SetInterruptCallback( OS_IE_CARD_A_DATA , InterruptCallbackCardData );
#else
  	SetInterruptCallback( OS_IE_CARD_B_IREQ , InterruptCallbackCard );
  	SetInterruptCallback( OS_IE_CARD_B_DET  , InterruptCallbackCardDet );
  	SetInterruptCallback( OS_IE_CARD_B_DATA , InterruptCallbackCardData );
#endif
}



// **************************************************************************
//
//							   Debug�p�\���֐�
//
// **************************************************************************
/*---------------------------------------------------------------------------*
  Name:         ShowRomHeaderData

  Description:  
 *---------------------------------------------------------------------------*/
static void ShowRomHeaderData(void)
{
	OS_TPrintf("Rom Header Data -------------------------------\n");
    OS_TPrintf("titleName           : %s\n", s_pBootSegBuffer->rh.s.title_name);
    OS_TPrintf("initialCode         : %x\n", *(u32 *)s_pBootSegBuffer->rh.s.game_code);
    OS_TPrintf("-----------------------------------------------\n");
}

/*---------------------------------------------------------------------------*
  Name:         ShowRegisterData

  Description:  
 *---------------------------------------------------------------------------*/
static void ShowRegisterData(void)
{
    OS_TPrintf("----------------------------------------------------------\n");
    OS_TPrintf("�g���@�\���䃌�W�X�^		 (MC_B(d24))   : %08x\n", reg_SCFG_EXT);
    OS_TPrintf("MC I/F���䃌�W�X�^�P		 (slot status) : %08x\n", reg_MI_MC1);
    OS_TPrintf("MC I/F���䃌�W�X�^�Q		 (Counter-A)   : %04x\n", reg_MI_MC2);
    OS_TPrintf("MC �R���g���[�����W�X�^0	 (SEL etc)     : %04x\n", reg_MCCNT0);
    OS_TPrintf("MC �R���g���[�����W�X�^1	 (START etc)   : %08x\n", reg_MCCNT1);
    OS_TPrintf("----------------------------------------------------------\n");
}
