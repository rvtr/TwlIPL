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
#include <twl/mcu.h>
#include <twl/cdc.h>
#include <symbols.h>
#include <twl/rtfs.h>
#include <sysmenu.h>
#include "reboot.h"


// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

#ifdef	ISDBG_MB_CHILD_
#define PRE_CLEAR_NUM_MAX		(6*2)
#else
#define PRE_CLEAR_NUM_MAX		(4*2)
#endif

#define COPY_NUM_MAX			(4*3)
#define POST_CLEAR_NUM_MAX		(12 + 4*2)

#define SYSMi_ARM9_BOOT_CODE_BUF	0x023fee00

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void BOOTi_ClearREG_RAM( void );
static void BOOTi_CutAwayRegionList( u32 *regionlist, u32 start, u32 end );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static u32 twl_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	HW_PARAM_RESERVED_END, SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM7_MMEM_ADDR_END, SYSMi_ARM9_BOOT_CODE_BUF,
	SYSMi_ARM9_BOOT_CODE_BUF + OS_BOOT_CODE_SIZE, SYSM_OWN_ARM9_MMEM_ADDR,
	SYSM_OWN_ARM9_MMEM_ADDR_END, HW_TWL_MAIN_MEM_SHARED,
	NULL,
};

static u32 nitro_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_END,
	HW_PARAM_RESERVED_END, SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM7_MMEM_ADDR_END, SYSMi_ARM9_BOOT_CODE_BUF,
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_DBG_NTR_SYSTEM_BUF,
	SYSM_OWN_ARM9_MMEM_ADDR_END, HW_TWL_MAIN_MEM_SHARED,
	NULL,
};

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
		(void)OS_SetIrqMaskEx(0);
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
			REBOOTTarget target = REBOOT_TARGET_TWL_SYSTEM;
            BOOL ds = FALSE;
			ROM_Header *th = (ROM_Header *)HW_TWL_ROM_HEADER_BUF;  // TWL�g��ROM�w�b�_�iDS�A�v���ɂ͖����j
			ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
			int list_count = PRE_CLEAR_NUM_MAX + 1;
			int l;
			u32 *post_clear_list;
			// ���������X�g�̐ݒ�
			static u32 mem_list[PRE_CLEAR_NUM_MAX + 1 + COPY_NUM_MAX + 2 + POST_CLEAR_NUM_MAX + 1] = 
			{
				// pre clear
				SYSM_OWN_ARM7_WRAM_ADDR, NULL, // SYSM_OWN_ARM7_WRAM_ADDR�iSDK_AUTOLOAD_WRAM_START�j�̓����J����^������
				SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
				SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
#ifdef	ISDBG_MB_CHILD_
				HW_PRV_WRAM_END - 0x600, (HW_PRV_WRAM_END - HW_PRV_WRAM_SYSRV_SIZE) - (HW_PRV_WRAM_END - 0x600),
				HW_PRV_WRAM_END - 0x600 + 0x20, HW_PRV_WRAM_END - (HW_PRV_WRAM_END - 0x600 + 0x20),
#endif
				HW_MAIN_MEM_SHARED, HW_RED_RESERVED - HW_MAIN_MEM_SHARED,
				NULL,
				// copy forward
				NULL,
				// copy backward
				NULL,
				// post clear
				NULL,
			};
			mem_list[1] = SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR;
			
			// copy forward���X�g�ݒ�
			for( l=0; l<RELOCATE_INFO_NUM ; l++ )
			{
				if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL && !SYSMi_GetWork()->romRelocateInfo[l].rev )
				{
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].src;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].dest;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].length;
				}
			}
			mem_list[list_count++] = NULL;
			
			// copy backward���X�g�ݒ�
			for( l=0; l<RELOCATE_INFO_NUM ; l++ )
			{
				if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL && SYSMi_GetWork()->romRelocateInfo[l].rev )
				{
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].src;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].dest;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].length;
				}
			}
			mem_list[list_count++] = NULL;
			
			// post clear���X�g�ݒ�
			if ( dh->s.platform_code )
			{
				post_clear_list = twl_post_clear_list;
				BOOTi_CutAwayRegionList( post_clear_list, (u32)th->s.main_ltd_ram_address, (u32)th->s.main_ltd_ram_address + th->s.main_ltd_size);
				BOOTi_CutAwayRegionList( post_clear_list, (u32)th->s.sub_ltd_ram_address, (u32)th->s.sub_ltd_ram_address + th->s.sub_ltd_size);
			}else
			{
				post_clear_list = nitro_post_clear_list;
			}
			BOOTi_CutAwayRegionList( post_clear_list, (u32)dh->s.main_ram_address, (u32)dh->s.main_ram_address + dh->s.main_size);
			BOOTi_CutAwayRegionList( post_clear_list, (u32)dh->s.sub_ram_address, (u32)dh->s.sub_ram_address + dh->s.sub_size);
			for( l=0; post_clear_list[l]!=NULL ; l+=2 )
			{
				mem_list[list_count++] = post_clear_list[l];
				mem_list[list_count++] = post_clear_list[l+1] - post_clear_list[l];
			}
			mem_list[list_count] = NULL;
			
			// �T�E���h��~
			SND_Disable();
			
			// �A�v���P�[�V�����I��
			if ( dh->s.platform_code )
			{
//				target = REBOOT_TARGET_TWL_APP;
#ifdef SYSMENU_DISABLE_TWL_BOOT
                while (1)
                {
                }
#endif // SYSMENU_DISABLE_TWL_BOOT
			}
			else
			{
				target = REBOOT_TARGET_DS_APP;
			}
			
            if ( target == REBOOT_TARGET_DS_APP || target == REBOOT_TARGET_DS_WIFI )
            {
                ds = TRUE;
            }

            if ( ds || th->s.codec_mode == OS_CODECMODE_NITRO )
            {
				// I2S��~�iMCLK�͓���p���j
				reg_SND_SMX_CNT &= ~REG_SND_SMX_CNT_E_MASK;
				CDC_GoDsMode();
				// DS�T�E���h�FDSP = 8:0
				// 32KHz
				reg_SND_SMX_CNT = REG_SND_SMX_CNT_MIX_RATE_MASK |
								  REG_SND_SMX_CNT_E_MASK;
            }

#if defined(FIRM_USE_TWLSDK_KEYS) || defined(SYSMENU_DISABLE_RETAIL_BOOT)
            // TwlSDK���̌����g���Ă��鎞�͐��i�pCPU�ł�TWL�A�v���̓u�[�g���Ȃ�
            if ( ! (*(u8*)HWi_WSYS08_ADDR & HWi_WSYS08_OP_OPT_MASK) && !ds )
            {
                OS_Terminate();
            }
#endif // FIRM_USE_SDK_KEYS || SYSMENU_DISABLE_RETAIL_BOOT

			// ���u�[�g
			OS_Boot( dh->s.sub_entry_address, mem_list, target );
		}
	}
	return FALSE;
}

static void BOOTi_ClearREG_RAM( void )
{
	if( SYSMi_GetWork()->flags.common.isCardBoot ) {
#ifdef DEBUG_USED_CARD_SLOT_B_
		reg_MI_MC_SWP = 0x80;											// �J�[�h�X���b�g�̃X���b�v
#endif
		*(u32 *)HW_BOOT_CHECK_INFO_BUF = SYSMi_GetWork()->nCardID;		// �J�[�h�����`�F�b�N�o�b�t�@�ɃJ�[�hID���Z�b�g
	}
	
	*(vu32 *)HW_RESET_PARAMETER_BUF = 0;								// ���Z�b�g�o�b�t�@���N���A
	
	// ���W�X�^�N���A�͊�{�I�� OS_Boot �ōs��
	
	// �N���A���Ă��Ȃ����W�X�^�́AVCOUNT, JOY, PIFCNT, MC-, EXMEMCNT, IME, PAUSE, POWLCDCNT, ���Z�L�����e�B�n�ł��B
	(void)OS_ResetRequestIrqMask((u32)~0);
	(void)OS_ResetRequestIrqMaskEx((u32)~0);
}

// �P�����X�g�v�f�폜
static void BOOTi_DeliteElementFromList( u32 *list, u32 index )
{
	int l;
	for( l=(int)index; list[l]!=NULL; l++ )
	{
		list[l] = list[l+1];
	}
}

// �P�����X�g�v�f�ǉ�
static void BOOTi_InsertElementToList( u32 *list, u32 index, u32 value )
{
	int l = (int)index;
	while(list[l]!=NULL)
	{
		l++;
	}
	list[l+1] = NULL;
	for( ; index<l; l-- )
	{
		list[l] = list[l-1];
	}
	list[l] = value;
}

// {first1, last1, first2, last2, ... , NULL}�Ƃ����`���̗̈惊�X�g����
// {start, end}�̗̈��؂��������X�g��Ԃ��֐�
// �����ɗ^���郊�X�g�͗v�f���ő�2�ǉ�����邽�߁A�\���ȑ傫�����K�v
// �܂��A�̈惊�X�g�̗v�f�́A�Ō����NULL�ȊO�����ɕ���ł���K�v������B
static void BOOTi_CutAwayRegionList( u32 *regionlist, u32 start, u32 end )
{
	int l, m, n;
	if( end <= start ) return;
	for( l=0; regionlist[l]!=NULL; l++ )
	{
		if( regionlist[l] >= start )
		{
			break;
		}
	}
	for( m=l; regionlist[m]!=NULL; m++ )
	{
		if( regionlist[m] > end )
		{
			break;
		}
	}
	// ���̎��_��regionlist[l]�����regionlist[m]�́Astart <= regionlist[l], end < regionlist[m]�ŁA���ł������Ȓl
	
	if( m % 2 == 1 )
	{
		BOOTi_InsertElementToList( regionlist, (u32)m, end );
		// end�����X�g�ɒǉ������ꍇ�Am�͒ǉ������v�f���w���悤��
	}
	if( l % 2 == 1 )
	{
		BOOTi_InsertElementToList( regionlist, (u32)l, start );
		m++;
		// start�����X�g�ɒǉ������ꍇ�Am��1������
		l++;
		// start�����X�g�ɒǉ������ꍇ�Al�͒ǉ������v�f�̎��̗v�f���w���悤��
	}
	
	// regionlist[l]����regionlist[m-1]�܂ł̗v�f������
	for( n=l; l<m; l++ )
	{
		BOOTi_DeliteElementFromList( regionlist, (u32)n );
	}
}
