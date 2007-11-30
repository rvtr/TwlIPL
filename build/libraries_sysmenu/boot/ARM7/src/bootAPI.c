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
#include <sysmenu.h>
#include "reboot.h"


// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

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
		
		// SDK���ʃ��u�[�g
		{
			// �������N���A���X�g�̐ݒ�
			static u32 clr_list[] = 
			{
				SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
				SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
				SYSM_OWN_ARM7_WRAM_ADDR, SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR,
				SYSM_OWN_ARM7_WRAM_ADDR, SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR,
#ifdef	ISDBG_MB_CHILD_
				HW_PRV_WRAM_END - 0x600, (HW_PRV_WRAM_END - HW_PRV_WRAM_SYSRV_SIZE) - (HW_PRV_WRAM_END - 0x600),
				HW_PRV_WRAM_END - 0x600 + 0x20, HW_PRV_WRAM_END - (HW_PRV_WRAM_END - 0x600 + 0x20),
#endif
				HW_MAIN_MEM_SHARED, HW_RED_RESERVED - HW_MAIN_MEM_SHARED,
				HW_ARENA_INFO_BUF, HW_ROM_HEADER_BUF - HW_ARENA_INFO_BUF,
				HW_PXI_SIGNAL_PARAM_ARM9, HW_MAIN_MEM_SYSTEM_END - HW_PXI_SIGNAL_PARAM_ARM9,
				NULL
			};

/*
#define SYSM_TWL_ARM9_LOAD_MMEM				0x02000400					// ���[�h�\��ARM9 static MMEM �A�h���X
#define SYSM_TWL_ARM9_LOAD_MMEM_END			SYSM_NTR_ARM9_LOAD_MMEM_END	// ���[�h�\��ARM9 static MMEM �ŏI�A�h���X
#define SYSM_TWL_ARM7_LOAD_MMEM				SYSM_NTR_ARM7_LOAD_MMEM		// ���[�h�\��ARM7 static MMEM �A�h���X
#define SYSM_TWL_ARM7_LOAD_MMEM_END			SYSM_NTR_ARM7_LOAD_MMEM_END	// ���[�h�\��ARM7 static MMEM �ŏI�A�h���X

#define SYSM_TWL_ARM9_LTD_LOAD_MMEM			0x02400000					// ���[�h�\��ARM9 LTD static MMEM �A�h���X
#define SYSM_TWL_ARM9_LTD_LOAD_MMEM_END		0x02800000					// ���[�h�\��ARM9 LTD static MMEM �ŏI�A�h���X
#define SYSM_TWL_ARM7_LTD_LOAD_MMEM			0x02e80000					// ���[�h�\��ARM7 LTD static MMEM �A�h���X
#define SYSM_TWL_ARM7_LTD_LOAD_MMEM_END		0x02f88000					// ���[�h�\��ARM7 LTD static MMEM �ŏI�A�h���X
*/

		    // [TODO]�Ĕz�u���X�g�̍쐬�Ɛݒ�
		    // �����ȊO�̃f�t�H���g���[�h�̈�ɔ��ꍇ�͑�NG
		    static u32 relocate_list[13] =
		    {
				NULL
			};
			u32 revCopy = 0;
			
		    ROM_Header *pHeader = (ROM_Header *)HW_TWL_ROM_HEADER_BUF;
		    u32 arm9ltd_dest = (u32)pHeader->s.main_ltd_ram_address;
		    u32 arm9ltd_size = (u32)pHeader->s.main_ltd_size;
		    u32 arm9flx_dest = (u32)pHeader->s.main_ram_address;
		    u32 arm9flx_size = (u32)pHeader->s.main_size;
		    u32 arm7ltd_dest = (u32)pHeader->s.sub_ltd_ram_address;
		    u32 arm7ltd_size = (u32)pHeader->s.sub_ltd_size;
		    u32 arm7flx_dest = (u32)pHeader->s.sub_ram_address;
		    u32 arm7flx_size = (u32)pHeader->s.sub_size;
		    
		    {
		    	// �`�F�b�N�R�[�h��
		    	// �Ƃ肠����arm9ltd�̂݁B�����悤�ȏ����͊֐������Ȃ��Ƃ����Ⴎ����BNTR�̏ꍇ�͗̈悪�኱�Ⴄ�̂Œ��ӁB
				if( SYSM_TWL_ARM9_LTD_LOAD_MMEM <= arm9ltd_dest && arm9ltd_dest + arm9ltd_size < SYSM_TWL_ARM9_LTD_LOAD_MMEM_END)
				{
					// �����P�[�g�K�v�Ȃ�
				}else
				{
					// �����P�[�g�K�v����
					if( SYSM_TWL_ARM7_LTD_LOAD_MMEM <= arm9ltd_dest && arm9ltd_dest < SYSM_TWL_ARM7_LTD_LOAD_MMEM_END ) return FALSE;//NG
					if( SYSM_TWL_ARM7_LTD_LOAD_MMEM <= arm9ltd_dest + arm9ltd_size && arm9ltd_dest + arm9ltd_size < SYSM_TWL_ARM7_LTD_LOAD_MMEM_END ) return FALSE;//NG
					if( SYSM_TWL_ARM9_LOAD_MMEM <= arm9ltd_dest && arm9ltd_dest < SYSM_TWL_ARM9_LOAD_MMEM_END ) return FALSE;//NG
					if( SYSM_TWL_ARM9_LOAD_MMEM <= arm9ltd_dest + arm9ltd_size && arm9ltd_dest + arm9ltd_size < SYSM_TWL_ARM9_LOAD_MMEM_END ) return FALSE;//NG
					if( SYSM_TWL_ARM7_LOAD_MMEM <= arm9ltd_dest && arm9ltd_dest < SYSM_TWL_ARM7_LOAD_MMEM_END ) return FALSE;//NG
					if( SYSM_TWL_ARM7_LOAD_MMEM <= arm9ltd_dest + arm9ltd_size && arm9ltd_dest + arm9ltd_size < SYSM_TWL_ARM7_LOAD_MMEM_END ) return FALSE;//NG
					
					// �����P�[�g�\
					if( SYSM_TWL_ARM9_LTD_LOAD_MMEM <= arm9ltd_dest && SYSM_TWL_ARM9_LTD_LOAD_MMEM_END <= arm9ltd_dest + arm9ltd_size )
					{
						revCopy = 1; // �������R�s�[����t���OON
					}
					
					relocate_list[0] = SYSM_TWL_ARM9_LTD_LOAD_MMEM;
					relocate_list[1] = arm9ltd_dest;
					relocate_list[2] = arm9ltd_size;
					relocate_list[3] = revCopy;
				}
			}

			// [TODO]�N������^�[�Q�b�g�̎�ނ��w�肷��K�v����
			OS_Boot( (void *)*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x34), clr_list, REBOOT_TARGET_TWL_SYSTEM );
		}
	}
	return FALSE;
}

static void BOOTi_ClearREG_RAM( void )
{
	int i ;
	
	for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {						// DMA�̒�~
		MI_StopDma( (u16)i );
	}
	
	if( SYSMi_GetWork()->isCardBoot ) {
#ifdef DEBUG_USED_CARD_SLOT_B_
		reg_MI_MC_SWP = 0x80;											// �J�[�h�X���b�g�̃X���b�v
#endif
		*(u32 *)HW_BOOT_CHECK_INFO_BUF = SYSMi_GetWork()->nCardID;		// �J�[�h�����`�F�b�N�o�b�t�@�ɃJ�[�hID���Z�b�g
		*(u32 *)HW_RED_RESERVED = SYSMi_GetWork()->nCardID;
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
