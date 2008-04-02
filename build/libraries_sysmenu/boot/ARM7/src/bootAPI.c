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
#include <twl/aes/ARM7/lo.h>
#include <sysmenu.h>
#include <sysmenu/hotsw.h>
#include <sysmenu/ds.h>
#include <firm/hw/ARM7/mmap_firm.h>
#include <firm/format/from_firm.h>
#include <firm/aes/ARM7/aes_init.h>
#include <firm/os/common/boot.h>
#include "reboot.h"
#include "internal_api.h"


// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

#define PRE_CLEAR_NUM_MAX		(5*2)
#define COPY_NUM_MAX			(5*3)
#define POST_CLEAR_NUM_MAX		(12 + 6*2)

#define CLRLIST_OWN_ARM7_WRAM_SIZE_IDX		1
#define CLRLIST_REBOOT_STACK_PAD_SIZE_IDX	(2*3+1)

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void BOOTi_ClearREG_RAM( void );
static void BOOTi_CutAwayRegionList( u32 *regionlist, u32 start, u32 end );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static u32 twl_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	HW_MAIN_MEM_PARAMETER_BUF_END, SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM7_MMEM_ADDR_END, OS_BOOT_A9CODE_BUF,
	OS_BOOT_A9CODE_BUF + OS_BOOT_CODE_SIZE, SYSM_OWN_ARM9_MMEM_ADDR,
	SYSM_OWN_ARM9_MMEM_ADDR_END, SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN,
	HW_EXCP_VECTOR_MAIN, HW_EXCP_VECTOR_MAIN + 4, // ARM9��O�x�N�^
	HW_EXCP_VECTOR_BUF,  HW_EXCP_VECTOR_BUF  + 4, // ARM7��O�x�N�^
	NULL,
};

static u32 nitro_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	HW_MAIN_MEM_PARAMETER_BUF, SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM7_MMEM_ADDR_END, OS_BOOT_A9CODE_BUF,
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_DBG_NTR_SYSTEM_BUF,
	SYSM_OWN_ARM9_MMEM_ADDR_END, SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN,
	HW_EXCP_VECTOR_MAIN, HW_EXCP_VECTOR_MAIN + 4, // ARM9��O�x�N�^
	HW_EXCP_VECTOR_BUF,  HW_EXCP_VECTOR_BUF  + 4, // ARM7��O�x�N�^
	NULL,
};

// const data--------------------------------------------------------

// dev_commonKey�̓x�^�Ŏ����Ă����B
static const u8 dev_commonKey[] = {
	0xA1, 0x60, 0x4A, 0x6A, 0x71, 0x23, 0xB5, 0x29,
	0xAE, 0x8B, 0xEC, 0x32, 0xC8, 0x16, 0xFC, 0xAA
};

static const u8 dev_seedES[] = {
	0x2D, 0xD4, 0x03, 0x98, 0xA7, 0x6B, 0x03, 0x28,
	0xCE, 0x61, 0x04, 0xBB, 0x0A, 0xBB, 0x03, 0x5B,
};

static const u8 dev_seedNAM[] = {
	0x4D, 0x04, 0xA4, 0x7F, 0xE3, 0x02, 0x30, 0x2E,
	0x2A, 0x07, 0x06, 0xE6, 0xD9, 0x06, 0x47, 0x76,
};

static const u8 dev_seedSlotC[] = {
	0x3B, 0x06, 0x86, 0x57, 0x33, 0x04, 0x88, 0x11,
	0x49, 0x04, 0x6B, 0x33, 0x12, 0x02, 0xAC, 0xF3,
};


void BOOT_Init( void )
{
	reg_PXI_MAINPINTF = 0x0000;
}
		
BOOL BOOT_WaitStart( void )
{
	if( (reg_PXI_MAINPINTF & 0x000f ) == 0x000f ) {
		// �œK�������ƃ|�C���^�����������������ł͉����R�[�h�͐�������܂���
		ROM_Header *th = (ROM_Header *)HW_TWL_ROM_HEADER_BUF;  // TWL�g��ROM�w�b�_�iDS�A�v���ɂ͖����j
		ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
		BOOL isNtrMode;

		// �w�b�_���Ĕz�u
		if( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL )
		{
			// TWL-ROM�w�b�_���̍Ĕz�u
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		}else {
			// NTR���[�h
			// TWL-ROM�w�b�_���̍Ĕz�u
			//   �����`���[��ROM�w�b�_���c���Ă����R�s�[�̈���N���A
			MI_CpuClearFast( (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		}

		(void)OS_DisableIrq();							// �����Ŋ��荞�݋֎~�ɂ��Ȃ��ƃ_���B
		(void)OS_SetIrqMask(0);							// SDK�o�[�W�����̃T�[�`�Ɏ��Ԃ�������ƁAARM9��HALT�ɂ������Ă��܂��AARM7�̃T�E���h�X���b�h��ARM9��FIFO�Ńf�[�^���M���悤�Ƃ��Ă�FIFO����t�ő��M�ł��Ȃ���ԂŖ������[�v�ɓ����Ă��܂��B
		(void)OS_SetIrqMaskEx(0);
		
		// �}�E���g�����ꎞ�I��SYSM_TWL_MOUNT_INFO_TMP_BUFFER�ɓo�^
		// �����܂ł�HW_TWL_ROM_HEADER_BUF�̃w�b�_�����̃A�v���̂��̂ɕύX����Ă���K�v����
		SYSMi_SetBootAppMountInfo( &SYSMi_GetWork2()->bootTitleProperty );
		
		// FS�ɂ����shared�̈�ɃR�s�[���ꂽ�����`���[���g�̃}�E���g�p�X�̃N���A
		MI_CpuClearFast((char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, OS_MOUNT_PATH_LEN);
		
		BOOTi_ClearREG_RAM();							// ARM7���̃����������W�X�^�N���A�B
		reg_MI_MBK9 = 0;								// �SWRAM�̃��b�N����
		reg_PXI_MAINPINTF = MAINP_SEND_IF | 0x0100;		// ARM9�ɑ΂��ău�[�g����悤IRQ�ŗv���{ARM7�̃X�e�[�g���P�ɂ���B
		
        // TWL/NTR���[�h����
        if ( ! dh->s.platform_code ||
             (SYSM_IsRunOnDebugger() && ((SYSMRomEmuInfo*)HOTSW_GetRomEmulationBuffer())->isForceNTRMode) )
        {
            isNtrMode = TRUE;
        }
        else
        {
            isNtrMode = FALSE;
        }
		
		// �����̈��n�����s���B
		// �u�[�g�A�v����ROM�w�b�_��accessKeyControl�������Ĕ���
		// ���n���́AIRQ�X�^�b�N�̈���g���̂ŁA���荞�݂��֎~���Ă���Z�b�g����B
		{
			BOOL isClearSlotB = TRUE;
			BOOL isClearSlotC = TRUE;
			
			MI_CpuClearFast( (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, HW_LAUNCHER_DELIVER_PARAM_BUF_SIZE );
			if( ! isNtrMode ) {
				if( th->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK ) {
					// commonClientKey
					if( th->s.access_control.common_client_key ) {
						void *pCommonKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
											OSi_GetFromFirmAddr()->aes_key[ 0 ] : (void *)dev_commonKey;
						MI_CpuCopy8( pCommonKey, (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, AES_BLOCK_SIZE );
					}
					// commonClientKeyForDebugger
					else if( th->s.access_control.common_client_key_for_debugger_sysmenu ) {
						MI_CpuCopy8( OSi_GetFromFirmAddr()->aes_key[ 1 ], (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, AES_BLOCK_SIZE );
					}
					
					// HW AES Slot B
					if( th->s.access_control.hw_aes_slot_B ) {
						void *pSeedES  =  ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
											&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0 ] ) : (void *)dev_seedES;
						MI_CpuCopy8( pSeedES,  (void *)( HW_LAUNCHER_DELIVER_PARAM_BUF + 0x10 ), AES_BLOCK_SIZE );
						isClearSlotB = FALSE;
						// AES�X���b�g�̃f�t�H���g�l�Z�b�g�͕s�v
					}
					// HW AES Slot C
					if( th->s.access_control.hw_aes_slot_C ) {
						void *pSeedNAM =  ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
											&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x10 ] ) : (void *)dev_seedNAM;
						void *pSeedSlotC = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
											&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x20 ] ) : (void *)dev_seedSlotC;
						MI_CpuCopy8( pSeedNAM, (void *)( HW_LAUNCHER_DELIVER_PARAM_BUF + 0x20 ), AES_BLOCK_SIZE );
						isClearSlotC = FALSE;
						// AES�X���b�g�̃f�t�H���g�l�Z�b�g
						AES_Lock();
						AES_SetKeySeedC( pSeedSlotC );
						AES_Unlock();
					}
				}
			}
			// �u�[�g����A�v���ɉ����āAAES�L�[�X���b�g�̃N���A���s���B
			AESi_ResetAesKeyA();
			if( isClearSlotB ) AESi_ResetAesKeyB();
			if( isClearSlotC ) AESi_ResetAesKeyC();
			
			// ���͕s�v�ɂȂ�̂ŁA�����Ă���
			{
				OSFromFirmBuf* fromFirm = (void*)HW_FIRM_FROM_FIRM_BUF;
				MI_CpuClearFast(fromFirm, sizeof(OSFromFirmBuf));
			}
		}
		// SDK���ʃ��u�[�g
		{
			REBOOTTarget target = REBOOT_TARGET_TWL_SYSTEM;
			int list_count = PRE_CLEAR_NUM_MAX + 1;
			int l;
			u32 *post_clear_list;
			// ���������X�g�̐ݒ�
			// [TODO:] �V���b�v�A�v���Ō����c���ꍇ�ANAND�t�@�[�������̗̈�iWRAM�ɂ���j�������Ȃ��悤�ɒ��ӁB
			//         WRAM���}�b�v��̏����R���o�b�t�@�I�[�o�����̌��O����̂��ߕs�v�Ȍ���pre clear�ŏ����B
			// [TODO:] DSP�̎g���Ă���WRAM���ǂ�����ǂ��܂ŏ������H
			static u32 mem_list[PRE_CLEAR_NUM_MAX + 1 + COPY_NUM_MAX + 2 + POST_CLEAR_NUM_MAX + 1] = 
			{
				// pre clear
				SYSM_OWN_ARM7_WRAM_ADDR, NULL, // SYSM_OWN_ARM7_WRAM_ADDR�iSDK_AUTOLOAD_WRAM_START�j�̓����J����^������̂Œ萔�łȂ�
				SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
				SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
				OS_BOOT_CODE_BUF_END, 1,     // REBOOT�R�A�R�[�h�ƃX�^�b�N�̌��ԃT�C�Y�̓��������X�g������ɍ����ւ���iNULL�ł�REBOOT_GetCoreStackSize�����s����j
				HW_WRAM_BASE, HW_WRAM_SIZE,  // ���LWRAM�@�@Launcher�̓���z�u�Ȃ̂ŁABASE����T�C�Y�Ԃ�
				NULL,
				// copy forward
				NULL,
				// copy backward
				NULL,
				// post clear
				NULL,
			};
			
			mem_list[CLRLIST_OWN_ARM7_WRAM_SIZE_IDX] = SYSM_OWN_ARM7_WRAM_ADDR_END - SYSM_OWN_ARM7_WRAM_ADDR;
			
			// copy forward���X�g�ݒ�
			mem_list[list_count++] = SYSM_TWL_MOUNT_INFO_TMP_BUFFER;
			mem_list[list_count++] = (u32)th->s.sub_mount_info_ram_address;
			mem_list[list_count++] = SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN;
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
			if ( ! isNtrMode )
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
			// REBOOT�R�A�R�[�h�ƃX�^�b�N�̌��ԃT�C�Y���Z�o
			mem_list[CLRLIST_REBOOT_STACK_PAD_SIZE_IDX] = OS_BOOT_STACK_TOP - REBOOT_GetCoreStackSize(mem_list) - OS_BOOT_CODE_BUF_END;
			
			// �T�E���h��~
			SND_Shutdown();
			
			// �A�v���P�[�V�����I��
			if ( ! isNtrMode )
			{
				if ( th->s.titleID_Hi & TITLE_ID_HI_APP_TYPE_MASK )
				{
					if ( th->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK )
					{
						target = REBOOT_TARGET_TWL_SECURE;
					}
					else
					{
						target = REBOOT_TARGET_TWL_SYSTEM;
					}
				}
				else
				{
					target = REBOOT_TARGET_TWL_APP;
				}
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

			// �O���f�|�b�v��H��L���ɂ���
			CDC_EnableExternalDepop();

			// I2S��~�iMCLK�͓���p���j
			reg_SND_SMX_CNT &= ~REG_SND_SMX_CNT_E_MASK;

            if ( isNtrMode || th->s.codec_mode == OS_CODECMODE_NITRO )
            {
				// �iCODEC-DS���[�h�j
				CDC_GoDsMode();
            }
			else
			{
				// �ď������iCODEC-TWL���[�h�j
				CDC_Init();
			}

			// I2S�ĊJ
			// DS�T�E���h�FDSP = 8:0
			// 32KHz
			reg_SND_SMX_CNT = REG_SND_SMX_CNT_MIX_RATE_MASK |
							  REG_SND_SMX_CNT_E_MASK;

			// �f�|�b�v���Ԃ̂��ƊO���f�|�b�v��H�𖳌��ɂ���
			// �P���ȃE�F�C�g�ɂȂ邽�߁A�����I�ɂ̓R���|�[�l���g�Ńf�|�b�v��
			// �����ɂ���悤�ɕύX����\��iTODO)
			OS_SpinWait(OS_MilliSecondsToTicks(100) * 64);
			CDC_DisableExternalDepop();

#ifdef SDK_ARM7
            // �f�o�b�K�ł�TWL�J�[�h�X���b�g�Q��d��ON
            if ( SYSM_IsRunOnDebugger() )
            {
                HOTSWi_TurnCardPowerOn( 2 );
            }
#endif // SDK_ARM7

#if defined(FIRM_USE_TWLSDK_KEYS) || defined(SYSMENU_DISABLE_RETAIL_BOOT)
            // TwlSDK���̌����g���Ă��鎞�͐��i�pCPU�ł�TWL�A�v���̓u�[�g���Ȃ�
            if ( ! (*(u8*)HWi_WSYS08_ADDR & HWi_WSYS08_OP_OPT_MASK) )
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
		reg_MI_MC_SWP ^= 0x80;											// �J�[�h�X���b�g�̃X���b�v
#endif
		*(u32 *)HW_BOOT_CHECK_INFO_BUF = SYSMi_GetWork()->nCardID;		// �J�[�h�����`�F�b�N�o�b�t�@�ɃJ�[�hID���Z�b�g
	}else {
		*(u32 *)HW_BOOT_CHECK_INFO_BUF = 0;
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
