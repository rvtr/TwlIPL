/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     boot.c

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
#include <sysmenu/boot/common/boot.h>
//#include <nitro/mb.h>
//#include "IPL2_work.h"
//#include "define.h"

// define data-------------------------------------------------------
#define C1_DTCM_ENABLE          0x00010000		// �f�[�^�s�b�l �C�l�[�u��
#define C1_EXCEPT_VEC_UPPER     0x00002000		// ��O�x�N�^ ��ʃA�h���X�i������ɐݒ肵�ĉ������j
#define C1_SB1_BITSET           0x00000078		// ���W�X�^�P�p�P�Œ�r�b�g��i����A�{�[�g���f���ADATA32�\���V�O�i������APROG32�\���V�O�i������A���C�g�o�b�t�@�C�l�[�u���j
#define INITi_HW_DTCM   		SDK_AUTOLOAD_DTCM_START


// extern data-------------------------------------------------------
// from LCF
//extern  void    SDK_IRQ_STACKSIZE( void );

// function's prototype----------------------------------------------
void BOOT_Core( void );

static void ResetCP15( void );
static void ClearBankregAndStack( void );
static void CpuClear32Byte( void );

#ifdef ISDBG_MB_CHILD_
// ���j�^API�ԍ�
typedef enum { MONAPI_ONLOADCHILD = 0 } APITYPE;
static u32 _isdcallmon( APITYPE nIndex, void *pParam );
static void	_ISDbgLib_OnLoadChildBinary(void);
#endif // ISDBG_MB_CHILD_

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------
#include <twl/code32.h>		// ���̃\�[�X�̓f�t�H���g�ł�ARM�ŃR���p�C�������B

asm void BOOT_Core( void )
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
		// IS�f�o�b�K�ł�DL�q�@�v���O�����̃f�o�b�O���ݒ�
		//---------------------------------------
#ifdef ISDBG_MB_CHILD_
		mov			r0, #MONAPI_ONLOADCHILD
		mov			r1, #0
		bl			_isdcallmon
//		bl			_ISDbgLib_OnLoadChildBinary
#endif // ISDBG_MB_CHILD_
		
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
		mov			r0, #0
		strh		r0, [r1]
		
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
asm static void ResetCP15( void )
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
asm static void ClearBankregAndStack( void )
{
		mov			r12, lr
		
#ifndef ISDBG_MB_CHILD_
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
#endif // ISDBG_MB_CHILD_
		
		ldr			r0, =HW_ITCM					// ITCM�̃N���A
		mov			r1, #HW_ITCM_SIZE
		bl			CpuClear32Byte
		
		ldr			r0, =INITi_HW_DTCM				// �X�^�b�N���܂߂�DTCM�̃N���A
		mov			r1, #HW_DTCM_SIZE
		bl			CpuClear32Byte
		
		bx			r12
}


// 32byte�P�ʂ̃������N���A  r0 dstp,r1 byteSize
asm static void CpuClear32Byte( void )
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




#ifdef ISDBG_MB_CHILD_
///////////////////////////////////////////////////////////////////////
//  Project:  ���j�^�̌Ăяo�� - IS�f�o�b�O���C�u�������R�[�h�����̎���
//  Copyright (C)2007 INTELIGENT SYSTEMS Co.,Ltd.
///////////////////////////////////////////////////////////////////////

#ifdef SDK_ARM9
// ARM9�̏ꍇ��`
#define	NITRO9
#endif	// SDK_ARM9

///////////////////////////////////////////////////////////////////////

#define	MON_ENTRY_SIGNATURE_OFFSET				0x00010
#define	MON_INTAPI_FUNC_OFFSET					0x002A0
#define	MON_INTAPI_FUNC_SIGN					0x4346564f		// OVFC
#define	MON_INTAPI_OFFSET_VERSION				0x04
#define	MON_INTAPI_OFFSET_COMMONENTRY_A9		0x20
#define	MON_INTAPI_OFFSET_COMMONENTRY_A7		0x24

#define	PERM_MASK								0x0F
#define	PERM_SHIFT_REGION7						28
#define	PERM_SYS_RW_USER_RW						0x03

///////////////////////////////////////////////////////////////////////


// �������ی�֘A
typedef struct tagMemoryProtection {
	OSIntrMode				modeIntr;
#ifdef	NITRO9
	u32						nCP15R7State;
	u32						nInstProtection;
	u32						nDataProtection;
#endif	// NITRO9
} MemoryProtection;

///////////////////////////////////////////////////////////////////////

static asm OSIntrMode OSm_DisableInterrupts_IrqAndFiq( void )
{
    mrs     r0, cpsr
    orr     r1, r0, #HW_PSR_IRQ_FIQ_DISABLE
    msr     cpsr_c, r1
    and     r0, r0, #HW_PSR_IRQ_FIQ_DISABLE

    bx      lr
}
static asm OSIntrMode OSm_RestoreInterrupts_IrqAndFiq( register OSIntrMode state )
{
    mrs     r1, cpsr
    bic     r2, r1, #HW_PSR_IRQ_FIQ_DISABLE
    orr     r2, r2, r0
    msr     cpsr_c, r2
    and     r0, r1, #HW_PSR_IRQ_FIQ_DISABLE

    bx      lr
}
#ifdef	NITRO9
static asm u32  OSm_SetIPermissionsForProtectionRegionEx( u32 nVal )
{
    mcr     p15, 0, r0, c5, c0, 3
    bx      lr
}

static asm u32  OSm_SetDPermissionsForProtectionRegionEx( u32 nVal )
{
    mcr     p15, 0, r0, c5, c0, 2
    bx      lr
}
static asm void OSm_SetProtectionRegion7( u32 param )
{
    mcr     p15, 0, r0, c6, c7, 0
    bx      lr
}
static asm u32  OSm_GetIPermissionsForProtectionRegion( void )
{
    mrc     p15, 0, r0, c5, c0, 3
    bx      lr
}
static asm u32  OSm_GetDPermissionsForProtectionRegion( void )
{
    mrc     p15, 0, r0, c5, c0, 2
    bx      lr
}
static asm u32  OSm_GetProtectionRegion7( void )
{
    mrc     p15, 0, r0, c6, c7, 0
    bx      lr
}
#endif	// NITRO9

///////////////////////////////////////////////////////////////////////
// �v���e�N�g���������܂�
///////////////////////////////////////////////////////////////////////
#ifdef	NITRO9
static void	_isdcallmon_unlockProtect( MemoryProtection *pParam, u32 dwRegion7 )
{
	// ���荞�݂������I�Ɏ~�߂鏈��
	pParam->modeIntr	= OSm_DisableInterrupts_IrqAndFiq();

	// �v���e�N�V�������j�b�g�̃��[�W����7�̐ݒ��ۑ�
	pParam->nCP15R7State = OSm_GetProtectionRegion7();
	pParam->nInstProtection = OSm_GetIPermissionsForProtectionRegion();
	pParam->nDataProtection = OSm_GetDPermissionsForProtectionRegion();

	// ���[�W����7�ɃI�[�o�[���C�]�����̈悩�烁�C���������I����̃v���e�N�g�ݒ���s��
	if( dwRegion7 ) OSm_SetProtectionRegion7( dwRegion7 ) ;

	// ���[�U�[�E�V�X�e�����ŏ������߂�悤�ɏC��
	OSm_SetIPermissionsForProtectionRegionEx( (pParam->nInstProtection & (~(PERM_MASK << PERM_SHIFT_REGION7))) | ( PERM_SYS_RW_USER_RW << PERM_SHIFT_REGION7) );
	OSm_SetDPermissionsForProtectionRegionEx( (pParam->nDataProtection & (~(PERM_MASK << PERM_SHIFT_REGION7))) | ( PERM_SYS_RW_USER_RW << PERM_SHIFT_REGION7) );
}
		// NITRO9
#else
		// NITRO7
static void	_isdcallmon_unlockProtect( MemoryProtection *pParam )
{
	// ���荞�݂������I�Ɏ~�߂鏈��
	pParam->modeIntr	= OSm_DisableInterrupts_IrqAndFiq();
}
#endif	// NITRO7


///////////////////////////////////////////////////////////////////////
// �v���e�N�g��Ԃ����ɖ߂��܂�
///////////////////////////////////////////////////////////////////////
static void	_isdcallmon_restoreProtect( const MemoryProtection *pParam )
{
#ifdef	NITRO9
	// ���[�U�[�E�V�X�e���̕ی��Ԃ�ݒ�
	OSm_SetDPermissionsForProtectionRegionEx( pParam->nDataProtection );
	OSm_SetIPermissionsForProtectionRegionEx( pParam->nInstProtection );

	// �v���e�N�V�������j�b�g�̃��[�W����7�̐ݒ�����ɖ߂�
	OSm_SetProtectionRegion7( pParam->nCP15R7State );
#endif	// NITRO9

	// ���荞�݂�߂�����
	(void)OSm_RestoreInterrupts_IrqAndFiq( pParam->modeIntr );
}


////////////////////////////////////////////////////////////////////////
// ���j�^�R�[���o�b�N
//
//  �E�Ăяo���O��_ISDbgLib_Initialize���s�Ȃ��Ă����K�v�͂���܂���B
//  �E�q�@�o�C�i���_�E�����[�h��ɌĂяo���Ă��������B
//  �Eloadrun����ISNITRO.dll��p�����_�E�����[�h���s����
//    v1.75�ȑO�̌Â��f�o�b�K���ł͉��������ɖ߂�܂��B
//
////////////////////////////////////////////////////////////////////////
static u32		_isdcallmon( APITYPE nIndex, void *pParam )
{
	MemoryProtection	prot;
	u32					nMonitorEntry;
	u32					nResult = (u32)-1;

	// ���j�^�G���g���̃`�F�b�N
	nMonitorEntry = *(vu32*)0x027FFF68;
	if ((nMonitorEntry >= HW_MAIN_MEM) && (nMonitorEntry < (HW_MAIN_MEM+0x00800000))) {

#ifdef	NITRO9
		u32		dwRegion7;

		// Unlock����̈�̐؂蕪��
		if( nMonitorEntry >= (HW_MAIN_MEM+0x00780000) ) {
			dwRegion7 = 0x02780000 | HW_C6_PR_512KB | HW_C6_PR_ENABLE;
		} else
		if( nMonitorEntry >= (HW_MAIN_MEM+0x00700000) ) {
			dwRegion7 = 0x02700000 | HW_C6_PR_1MB | HW_C6_PR_ENABLE;
		} else
		if( nMonitorEntry >= (HW_MAIN_MEM+0x00600000) ) {
			dwRegion7 = 0x02600000 | HW_C6_PR_2MB | HW_C6_PR_ENABLE;
		} else
		if( nMonitorEntry >= (HW_MAIN_MEM+0x00400000) ) {
			dwRegion7 = 0x02400000 | HW_C6_PR_4MB | HW_C6_PR_ENABLE;
		} else {
			dwRegion7 = 0x00000000;
		} 

		_isdcallmon_unlockProtect( &prot, dwRegion7 );
#else
		_isdcallmon_unlockProtect( &prot );
#endif

		// �V�O�l�`���̊m�F�ƃW�����v
		{
			u32	nSignature = nMonitorEntry + MON_ENTRY_SIGNATURE_OFFSET;
			u32 nOVFC = nMonitorEntry + MON_INTAPI_FUNC_OFFSET;

			if(    *(u64 *)nSignature == (u64)0x47494c4c45544e49LL // INTELLIG
				&& *(u32 *)nOVFC == MON_INTAPI_FUNC_SIGN 
				&& *(u32 *)(nOVFC + MON_INTAPI_OFFSET_VERSION) >= 2
			){
				// �ʒm�֐��̃A�h���X�f�f
				u32		nAddr;
#ifdef	NITRO9
				nAddr = *(u32 *)(nOVFC + MON_INTAPI_OFFSET_COMMONENTRY_A9);
#else
				nAddr = *(u32 *)(nOVFC + MON_INTAPI_OFFSET_COMMONENTRY_A7);
#endif
				if ((nAddr >= HW_MAIN_MEM) && (nAddr < (HW_MAIN_MEM+0x00800000))) {
					// �ʒm�֐��փW�����v����
					// (bx or blx�Ńu�����`���Ă��邩�m�F���邱��)
					nResult = (*(u32(*)( u32, void*))nAddr)( nIndex, pParam );
				}
			}
		}

		// �������ی�̕���
		_isdcallmon_restoreProtect( &prot );
	}

	return nResult;
}

#if 0
// �q�@�o�C�i���_�E�����[�h��̃R�[���o�b�N����
static void	_ISDbgLib_OnLoadChildBinary(void)
{
	(void)_isdcallmon( MONAPI_ONLOADCHILD, NULL );
}
#endif // 0

#endif // ISDBG_MB_CHILD_


#include <twl/codereset.h>		// �����܂ŁB

