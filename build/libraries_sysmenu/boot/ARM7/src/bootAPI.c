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
#include "reboot.h"
//#include "loader.h"
//#include "mb_child.h"

// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void BOOTi_ClearREG_RAM( void );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------

void BOOT_Init( void )
{
	reg_PXI_MAINPINTF = 0x0000;
}
		
BOOL BOOT_WaitStart( void )
{
	if( (reg_PXI_MAINPINTF & 0x000f ) == 0x000f ) {
		// �������N���A���X�g�̐ݒ�
		static u32 clr_list[] = 
		{
			SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
			SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
			SYSM_OWN_ARM7_WRAM_ADDR, SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR,

//			HW_PRV_WRAM_END - 0x600, 0x600,
#ifdef	ISDBG_MB_CHILD_
			HW_PRV_WRAM_END - 0x600, (HW_PRV_WRAM_END - HW_PRV_WRAM_SYSRV_SIZE) - (HW_PRV_WRAM_END - 0x600),
			HW_PRV_WRAM_END - 0x600 + 0x20, HW_PRV_WRAM_END - (HW_PRV_WRAM_END - 0x600 + 0x20),
#endif
			//SYSM_BOOTCODE_ARM7_ADDR, ClearMemory - SYSM_BOOTCODE_ARM7_ADDR,
//			0x02fff000, 0x800,
//			0xda0, 0x60,
//			0xf80, 0x80,
			
			NULL
		};

		(void)OS_DisableIrq();							// �����Ŋ��荞�݋֎~�ɂ��Ȃ��ƃ_���B
		(void)OS_SetIrqMask(0);							// SDK�o�[�W�����̃T�[�`�Ɏ��Ԃ�������ƁAARM9��HALT�ɂ������Ă��܂��AARM7�̃T�E���h�X���b�h��ARM9��FIFO�Ńf�[�^���M���悤�Ƃ��Ă�FIFO����t�ő��M�ł��Ȃ���ԂŖ������[�v�ɓ����Ă��܂��B
/*
#ifdef ISDBG_MB_CHILD_
		if( ( GetIpl2WorkAddr()->ipl2_type != 0xff ) && ( GetIpl2WorkAddr()->ipl2_type & 0x28 ) )
#endif // ISDBG_MB_CHILD_								// 	USG or NAT�Ȃ疳���p�b�`�𓖂Ă�
		{
			InsertWLPatch();
		}
*/
		BOOTi_ClearREG_RAM();							// ARM7���̃����������W�X�^�N���A�B
		reg_MI_MBK9 = 0;								// �SWRAM�̃��b�N����
		reg_PXI_MAINPINTF = MAINP_SEND_IF | 0x0100;		// ARM9�ɑ΂��ău�[�g����悤IRQ�ŗv���{ARM7�̃X�e�[�g���P�ɂ���B
		// BOOT_Core();									// never return

		// SDK���ʃ��u�[�g
		OS_Boot( (void *)*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x34), clr_list );
	}
	return FALSE;
}

static void BOOTi_ClearREG_RAM( void )
{
	int i ;
	
//	SVC_CpuClearFast(0x0000, IPL2_PRV_WRAM_ARENA_LO, RETURN_FROM_MAIN_ARM7_FUNCP - IPL2_PRV_WRAM_ARENA_LO );
																	// �A���[�i�̃N���A
	for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {						// DMA�̒�~
		MI_StopDma( (u16)i );
	}
	// ���W�X�^�̃N���A
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x0b0), (0x13c - 0x0b0) );
																		// DMA0SAD  �` RCNT1
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x400), 0x104 );	// SG0CNT_L �` SGMCNT
	SVC_CpuClearFast( 0x0000, (void*)(HW_REG_BASE + 0x508), 0x14 );		// SGRVCNT  �` SGRV1CLEN
	reg_GX_DISPSTAT			= 0;
	reg_SPI_SPICNT			= 0;
	reg_PXI_MAINP_FIFO_CNT	= 0x4008;
	
	*(vu32 *)HW_RESET_PARAMETER_BUF = 0;								// ���Z�b�g�o�b�t�@���N���A
	
	// �N���A���Ă��Ȃ����W�X�^�́AVCOUNT, JOY, PIFCNT, MC-, EXMEMCNT, IME, PAUSE, POWLCDCNT, ���Z�L�����e�B�n�ł��B
	(void)OS_ResetRequestIrqMask((u16)~0);
}
