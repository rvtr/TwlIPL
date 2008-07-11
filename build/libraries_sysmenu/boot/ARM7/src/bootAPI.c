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
#include "reboot.h"
#include "internal_api.h"


// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

#define PRE_CLEAR_NUM_MAX		(6*2)
#define COPY_NUM_MAX			(6*3)
#define COPY_HEADER_NUM_MAX		(1*3)
#define POST_CLEAR_NUM_MAX		(12 + 6*2)

#define CLRLIST_REBOOT_STACK_PAD_SIZE_IDX	(2*3+1)

#define TITLE_ID_NAND_INITIALIZER		0x00030011304E4941ULL	// 0NIA

// �N��������������^�C�g���ꗗ
typedef struct TitleBlackList {
	OSTitleId	titleID;
	int			rom_version;
}TitleBlackList;

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------

static void BOOTi_ClearREG_RAM( void );
static void BOOTi_CutAwayRegionList( u32 *regionlist, u32 start, u32 end );
static void BOOTi_CheckTitleBlackList( ROM_Header_Short *pROMH );
static void BOOTi_RebootCallback( void** entryp, void* mem_list, REBOOTTarget* target );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static REBOOTTarget target;

// ���������X�g
static u32 mem_list[PRE_CLEAR_NUM_MAX + 1 + COPY_NUM_MAX + COPY_HEADER_NUM_MAX + 2 + POST_CLEAR_NUM_MAX + 1] = 
{
	// pre clear
	HW_WRAM_B_OR_C_MIRROR,   SYSM_OWN_ARM7_WRAM_ADDR_END - HW_WRAM_B_OR_C_MIRROR, // SYSM_OWN_ARM7_WRAM_ADDR��HW_WRAM_B���܂Ƃ߂ăN���A
	SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
	OS_BOOT_CODE_BUF_END, 1,     // REBOOT�R�A�R�[�h�ƃX�^�b�N�̌��ԃT�C�Y�̓��������X�g������ɍ����ւ���iNULL�ł�REBOOT_GetCoreStackSize�����s����j
	HW_WRAM_BASE, HW_WRAM_SIZE,  // ���LWRAM�@�@Launcher�̓���z�u�Ȃ̂ŁABASE����T�C�Y�Ԃ�
	HW_WRAM_C, HW_WRAM_C_SIZE,
	NULL,
	// copy forward
	NULL,
	// copy backward
	NULL,
	// post clear
	NULL,
};

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

// �N��������������^�C�g���ꗗ
static const TitleBlackList s_blackList[] = {
	{ TITLE_ID_NAND_INITIALIZER, 0 },
	{ 0ULL, -1 },
};


void BOOT_Init( void )
{
	reg_PXI_MAINPINTF = 0x0000;
}


BOOL BOOT_WaitStart( void )
{
	if( (reg_PXI_MAINPINTF & 0x000f ) == 0x000f ) {
		// �œK�������ƃ|�C���^�����������������ł͉����R�[�h�͐�������܂���
		ROM_Header *th = (ROM_Header *)SYSM_APP_ROM_HEADER_BUF;          // TWL�g��ROM�w�b�_�iDS�A�v���ɂ͖����j
		ROM_Header *dh = (ROM_Header *)(SYSMi_GetWork()->romHeaderNTR);  // DS�݊�ROM�w�b�_
		// ���u�[�g
		REBOOTi_SetTwlRomHeaderAddr( th );
		REBOOTi_SetRomHeaderAddr( dh );
		REBOOTi_SetPostFinalizeCallback( BOOTi_RebootCallback );
		OS_Boot( OS_BOOT_ENTRY_FROM_ROMHEADER, mem_list, target );
	}
	return FALSE;
}

static void BOOTi_RebootCallback( void** entryp, void* mem_list_v, REBOOTTarget* target )
{
#pragma unused(entryp)
		u32* mem_list = mem_list_v;
		ROM_Header *th = (void*)REBOOTi_GetTwlRomHeaderAddr();
		ROM_Header *dh = (void*)REBOOTi_GetRomHeaderAddr();
		BOOL isNtrMode;

		(void)OS_DisableIrq();							// �����Ŋ��荞�݋֎~�ɂ��Ȃ��ƃ_���B
		(void)OS_SetIrqMask(0);							// SDK�o�[�W�����̃T�[�`�Ɏ��Ԃ�������ƁAARM9��HALT�ɂ������Ă��܂��AARM7�̃T�E���h�X���b�h��ARM9��FIFO�Ńf�[�^���M���悤�Ƃ��Ă�FIFO����t�ő��M�ł��Ȃ���ԂŖ������[�v�ɓ����Ă��܂��B
		(void)OS_SetIrqMaskEx(0);

		// �N���A�v��NTR-ROM�w�b�_�ւ̃p�b�`�����̂��߃R�s�[
		MI_CpuCopyFast( th, dh, HW_CARD_ROM_HEADER_SIZE );

		// �J�[�hNTR-ROM�w�b�_��NAND�A�v����DS�_�E�����[�h�v���C�̂��߃R�s�[
		MI_CpuCopyFast( (void*)SYSM_CARD_ROM_HEADER_BAK, (void*)HW_CARD_ROM_HEADER, HW_CARD_ROM_HEADER_SIZE );

    	if ( dh->s.platform_code ){
			// �J�[�hTWL-ROM�w�b�_��HW_TWL_CARD_ROM_HEADER_BUF�ɃR�s�[
			MI_CpuCopyFast( (void*)SYSM_CARD_ROM_HEADER_BAK, (void*)HW_TWL_CARD_ROM_HEADER_BUF, HW_TWL_CARD_ROM_HEADER_BUF_SIZE );
        }

		// ���̏����́ADS�_�E�����[�h�v���C���ōs���B
		// MI_CpuCopyFast ( (void *)HW_CARD_ROM_HEADER, (void *)MB_CARD_ROM_HEADER_ADDRESS, 0x160);

		// �u���b�N���X�g���`�F�b�N���A�N��������������
		BOOTi_CheckTitleBlackList( (void*)th );
		
		// �}�E���g�����ꎞ�I��SYSM_TWL_MOUNT_INFO_TMP_BUFFER�ɓo�^
		// �����܂ł�SYSM_APP_ROM_HEADER_BUF�̃L���b�V�������C�g�o�b�N����Ă���K�v����
		SYSMi_SetBootAppMountInfo( &SYSMi_GetWork2()->bootTitleProperty );
		
		// FS�ɂ����shared�̈�ɃR�s�[���ꂽ�����`���[���g�̃}�E���g�p�X�̃N���A
		MI_CpuClearFast((char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, OS_MOUNT_PATH_LEN);
		
		// NAND/TMP�A�v���pKeyTable�̐���
		if( ! SYSMi_GetWork()->flags.common.isCardBoot )
		{
			HOTSWi_MakeBlowfishTableDSForNAND();
		}
		// ARM9FLX�̐擪2k�Í��I�u�W�F�N�g���f�N���v�g
		{
			// �Ĕz�u��񂪂���΂�����̃A�h���X�A�Ȃ���΃w�b�_�w��̃A�h���X
			void *addr = (SYSMi_GetWork()->romRelocateInfo[0].src != NULL) ? SYSMi_GetWork()->romRelocateInfo[0].src : dh->s.main_ram_address;
			HOTSWi_SetSecureSegmentBuffer(HOTSW_MODE1, addr, SECURE_AREA_SIZE);
			HOTSW_DecryptObjectFile( addr );
		}
		
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
		SYSMi_SetAESKeysForAccessControl( isNtrMode, th );
		
		// SDK���ʃ��u�[�g
		{
			int list_count = PRE_CLEAR_NUM_MAX + 1;
			int l;
			u32 *post_clear_list;
			
			*target = REBOOT_TARGET_TWL_SYSTEM;
			
			// copy forward���X�g�ݒ�
			// �}�E���g���
			if( !isNtrMode )
			{
				mem_list[list_count++] = SYSM_TWL_MOUNT_INFO_TMP_BUFFER;
				mem_list[list_count++] = (u32)th->s.sub_mount_info_ram_address;
				mem_list[list_count++] = SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN;
			}
			// ���W���[���Ĕz�u�R�s�[forward
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
			// ���W���[���Ĕz�u�R�s�[backward
			for( l=0; l<RELOCATE_INFO_NUM ; l++ )
			{
				if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL && SYSMi_GetWork()->romRelocateInfo[l].rev )
				{
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].src;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].dest;
					mem_list[list_count++] = SYSMi_GetWork()->romRelocateInfo[l].length;
				}
			}
			// NTR�V�X�e���̈�R�s�[�ipre clear�̏����͈͂֊܂܂��悤�ɂȂ������߁j
			if( isNtrMode )
			{
				mem_list[list_count++] = HW_MAIN_MEM_SHARED;
				mem_list[list_count++] = 0x02800000 - HW_MAIN_MEM_SHARED_SIZE;
				mem_list[list_count++] = HW_MAIN_MEM_SHARED_SIZE;
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
				// NTR-IPL�Ɠ��l�ɃV�X�e���̈撼�O���N���A���Ă���
				mem_list[list_count++] = 0x02800000 - OS_BOOT_SYS_CLR_SIZE;
				mem_list[list_count++] = OS_BOOT_SYS_CLR_SIZE - HW_MAIN_MEM_SHARED_SIZE;
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
						*target = REBOOT_TARGET_TWL_SECURE;
					}
					else
					{
						*target = REBOOT_TARGET_TWL_SYSTEM;
					}
				}
				else
				{
					*target = REBOOT_TARGET_TWL_APP;
				}
#ifdef SYSMENU_DISABLE_TWL_BOOT
                while (1)
                {
                }
#endif // SYSMENU_DISABLE_TWL_BOOT
			}
			else
			{
				*target = REBOOT_TARGET_DS_APP;
			}

            // USG�ȑO��DS�A�v���ɂ͖����p�b�`��K�p
            // ARM9�̓��[�h�A�h���X���L���b�V���X�g�A���Ă����K�v����
            if ( *target == REBOOT_TARGET_DS_APP )
            {
                DS_InsertWLPatch( dh );
            }

			// I2S��~�iMCLK�͓���p���j
			reg_SND_SMX_CNT &= ~REG_SND_SMX_CNT_E_MASK;

            if ( isNtrMode || th->s.exFlags.codec_mode == OS_CODECMODE_NITRO )
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
		}
}

static void BOOTi_ClearREG_RAM( void )
{
#ifdef DEBUG_USED_CARD_SLOT_B_
	reg_MI_MC_SWP ^= 0x80;												// �J�[�h�X���b�g�̃X���b�v
#endif
																		// �J�[�h�����`�F�b�N�o�b�t�@�ɃJ�[�hID���Z�b�g
	((SDKBootCheckInfo*)HW_BOOT_CHECK_INFO_BUF)->nCardID = SYSMi_GetWork()->appCardID;
	
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

// �N��������������u���b�N���X�gTITLE�̃`�F�b�N
static void BOOTi_CheckTitleBlackList( ROM_Header_Short *pROMH )
{
	const TitleBlackList *pBlackList = &s_blackList[ 0 ];
	
	while( pBlackList->rom_version >= 0 ) {
		if( ( pBlackList->titleID ==  pROMH->titleID ) &&
			( pBlackList->rom_version ==  pROMH->rom_version ) ) {
			OS_TPrintf( "Hit black list : %c%c%c%c ver.%d...Terminate.\n",
						pROMH->titleID_Lo[ 3 ], pROMH->titleID_Lo[ 2 ], pROMH->titleID_Lo[ 1 ], pROMH->titleID_Lo[ 0 ],
						pROMH->rom_version );
			OS_Terminate();
		}
		pBlackList++;
	}
}
