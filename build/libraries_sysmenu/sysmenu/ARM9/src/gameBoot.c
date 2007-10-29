/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     gameBoot.c

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

#include <twl.h>
#include <sysmenu/mmap.h>

// define data-------------------------------------------------------
#define C1_DTCM_ENABLE          0x00010000		// �f�[�^�s�b�l �C�l�[�u��
#define C1_EXCEPT_VEC_UPPER     0x00002000		// ��O�x�N�^ ��ʃA�h���X�i������ɐݒ肵�ĉ������j
#define C1_SB1_BITSET           0x00000078		// ���W�X�^�P�p�P�Œ�r�b�g��i����A�{�[�g���f���ADATA32�\���V�O�i������APROG32�\���V�O�i������A���C�g�o�b�t�@�C�l�[�u���j

#define INITi_HW_DTCM   SDK_AUTOLOAD_DTCM_START

// extern data-------------------------------------------------------

// from LCF
extern  u32 SDK_IRQ_STACKSIZE[];

// function's prototype----------------------------------------------
void ReturnFromMain(void);
void ResetCP15(void);
void ClearBankregAndStack(void);
void CpuClear32Byte(void);
void BootFuncEnd(void);

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------


#include <twl/code32.h>		// ���̃\�[�X�̓f�t�H���g�ł�ARM�ŃR���p�C�������B

//-----------------------------------------------------------------------
// ���C�����[�`������̃��^�[��
//-----------------------------------------------------------------------

/*
	��ReturnFromMain��Ram�̌���ɃR�s�[������@���A����ReturnFromMain�A�h���X����A
�@�@�@(BootFuncEnd - ReturnFromMain)�T�C�Y�������R�s�[�Ƃ��Ă��邪�A
�@�@�@���̕��@���ƁA�R���p�C���̍œK���d�l�Ŋ֐����т��ς���Ă��܂������ɂ��������Ȃ�B
�@�@�@�������ɂ������@�͂Ȃ����H
*/


asm void ReturnFromMain(void)
{
		//---------------------------------------
		// �f�[�^�L���b�V����S�Ė����ɁB�iDC_InvalidateAll�𔲂��o���Ď����j
		//---------------------------------------
	    mov         r0, #0
    	mcr         p15, 0, r0, c7, c6, 0
		
		//---------------------------------------
		// ARM7�Ƃ̓������Ƃ�isubp_state��2�ɂȂ�̂�҂��āAmainp_state��2�ɂ���B�j
		//---------------------------------------
		ldr			r1, =REG_SUBPINTF_ADDR
@0		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0002
		bne			@0
		mov			r0, #0x0200
		strh		r0, [r1]
		
		//---------------------------------------
		// IS�f�o�b�K����t���O�̊i�[
		//---------------------------------------
#ifdef __IS_DEBUGGER_BUILD
		ldr			r3, =HW_MAIN_MEM_EX_END
		sub			r0, r3, #0x400
		ldrh		r11, [r0, #0x14]					// r11 =  GetMovedInfoFromIPL1Addr()->isOnDebugger
#endif
		//---------------------------------------
		// ARM7�Ƃ̓������Ƃ�isubp_state��1�ɂȂ�̂�҂��āAmainp_state��1�ɂ���B�j
		//---------------------------------------
		ldr			r1, =REG_SUBPINTF_ADDR
@1		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0001
		bne			@1
		mov			r0, #0x0100
		strh		r0, [r1]
		
		//---------------------------------------
		// �o���N���W�X�^���X�^�b�N�N���A
		//---------------------------------------
		bl			ClearBankregAndStack
		
		//---------------------------------------
		// �v���e�N�V�������j�b�g�̉���
		//---------------------------------------
		bl			ResetCP15
		
		//---------------------------------------
		// ARM7�Ƃ̍ŏI�������Ƃ�(subp_state��0�ɂȂ�̂�҂��āAmainp_state��0�ɂ���j
		//---------------------------------------
		ldr			r1, =REG_SUBPINTF_ADDR
@2		ldrh		r0, [r1]
		and			r0, r0, #0x000f
		cmp			r0, #0x0001
		beq			@2
		ldr			r3, =REG_VCOUNT_ADDR			// V�J�E���^��SIPL2�o�[�W�����œ���l(=0)�ŃA�v���Ɉ����n���悤�ɂ���B
@3		ldrh			r0, [r3]
		cmp			r0, #0
		bne			@3
//		mov			r0, #0						// R0�ɓǂ�V�J�E���g�l��"0"�Ȃ̂ŁA����͂���Ȃ�
		strh			r0, [r1]
		
		//---------------------------------------
		// R11�̒l�����ƂɃu�[�g�A�h���X�擾
		//---------------------------------------
		ldr			r3, =HW_MAIN_MEM_EX_END			// �Q�[���E�G���g���|�C���g �l��
		ldr			r12, [r3, #-(0x200 - 0x24)]		// rmhp->arm9->entryAddr
		mov			lr, r12
		
#ifdef __IS_DEBUGGER_BUILD
		cmp			r11, #1							// if (!GetMovedInfoFromIPL1Addr()->isOnDebugger)
		ldreq		r12, [r3, #-(0x200 - 0x168)]	// �f�o�b�K�E�G���g���|�C���g �l��
#endif
		
		//---------------------------------------
		// �ėp���W�X�^�N���A
		//---------------------------------------
		ldr			r11, =INITi_HW_DTCM				// �N���A����DTCM����f�[�^��ǂݏo���āA�ėp���W�X�^���N���A����B
		ldmia		r11, {r0-r10}
		mov			r11, #0
		
		//---------------------------------------
		// �Q�[���u�[�g
		//---------------------------------------
		bx			r12
}


//-----------------------------------------------------------------------
// �V�X�e������R�v���Z�b�T ���Z�b�g
//-----------------------------------------------------------------------
asm void ResetCP15(void)
{
		// �v���e�N�V�������j�b�g���L���b�V����ITCM�����BDTCM�͗L���i�X�^�b�N���N���A���邽�߁j
		ldr     	r0, = C1_DTCM_ENABLE  | C1_EXCEPT_VEC_UPPER | C1_SB1_BITSET
		mcr     	p15, 0, r0, c1, c0, 0
		
		// ITCM�̊��蓖�Ă�����
		mov			r0, #0
		mcr			p15, 0, r0, c6, c5, 0
		
		// DTCM�̊��蓖�Ă�����
//		mov			r0,#0
//		mcr			p15, 0, r0, c9, c1, 0
		
		// �L���b�V��������
		mov     	r0, #0
		mcr     	p15, 0, r0, c7, c5, 0       	// ���߃L���b�V��
		mcr     	p15, 0, r0, c7, c6, 0       	// �f�[�^�L���b�V��
		
		// ���C�g�o�b�t�@ �G���v�e�B�҂�
		mcr			p15, 0, r0, c7, c10, 4
		
		bx			lr
}


//-----------------------------------------------------------------------
// �o���N���W�X�^ ���Z�b�g �� �X�^�b�N�̈� �N���A
//-----------------------------------------------------------------------
asm void ClearBankregAndStack(void)
{
		mov			r12, lr
		
#ifndef IPL2_ONLYMULTIBOOT
		mov			r0, #0xc0 | HW_PSR_SVC_MODE		// SVC���[�h�֐؂芷��  & IRQ/FIQ�s����
		msr			cpsr_cxsf, r0
		ldr			r0, =INITi_HW_DTCM
		add			r0, r0, #0x3fc0
		mov			sp, r0							// SP �̃Z�b�g
		mov			lr,	#0
		msr			spsr_csxf, lr
		
		mov			r0, #0xc0 | HW_PSR_IRQ_MODE		// IRQ���[�h�֐؂芷��  & IRQ/FIQ�s����
		msr			cpsr_cxsf, r0
		ldr			r0, =INITi_HW_DTCM
		add			r0, r0, #0x3fc0
		sub			r0, r0, #HW_SVC_STACK_SIZE
		mov			sp, r0							// SP �̃Z�b�g
		mov			lr,	#0
		msr			spsr_cxsf, lr
		
        ldr			r1, =SDK_IRQ_STACKSIZE
        sub			r1, r0, r1
		mov			r0, #0xc0 | HW_PSR_SYS_MODE		// �V�X�e�����[�h�֐؂芷�� & IRQ/FIQ�s����
		msr			cpsr_cxsf, r0
        sub			sp, r1, #4						// SP �̃Z�b�g & 4byte for stack check code
#endif // IPL2_ONLYMULTIBOOT
		
		ldr			r0, =HW_ITCM					// ITCM�̃N���A
		mov			r1, #HW_ITCM_SIZE
		bl			CpuClear32Byte
		
		ldr			r0, =INITi_HW_DTCM				// �X�^�b�N���܂߂�DTCM�̃N���A
		mov			r1, #HW_DTCM_SIZE
		bl			CpuClear32Byte
		
		bx			r12
}


// 32byte�P�ʂ̃������N���A  r0 dstp,r1 byteSize
asm void CpuClear32Byte(void)
{
		add			r2, r0, r1						// �I���A�h���X�̎Z�o
		mov			r1, r1, lsr #5					// �T�C�Y��32byte�P��
		mov			r3, #0
		mov			r4, r3
		mov			r5, r3
		mov			r6, r3
		mov			r7, r3
		mov			r8, r3
		mov			r9, r3
		mov			r10, r3
@0		cmp			r0, r2							// �N���A�I���H
		stmltia		r0!, {r3-r10}
		blt			@0
		bx  	    lr
}


void BootFuncEnd(void)
{
}
#include <twl/codereset.h>		// �����܂ŁB

