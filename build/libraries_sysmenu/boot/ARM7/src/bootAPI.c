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
#include <nitro/os.h>
#include <nitro/code32.h>
#include <sysmenu.h>
#include <sysmenu/hotsw.h>
#include <sysmenu/ds.h>
#include <firm/hw/ARM7/mmap_firm.h>
#include <firm/mi/mainMemory.h>
#include <firm/format/from_firm.h>
#include <firm/aes/ARM7/aes_init.h>
#include "reboot.h"
#include "internal_api.h"
#include "../../../hotsw/ARM7/include/hotswTypes.h"
#include "targetCode.h"

#include <twl/mcu/ARM7/control.h>

extern void SPI_Lock(u32 id);
extern void SPI_Unlock(u32 id);

// define data-------------------------------------------------------
#define MAINP_SEND_IF		0x2000
#define reg_MI_MC_SWP		(*(REGType8v *) ( REG_MC1_ADDR + 1 ) )

#define PRE_CLEAR_NUM_MAX		(7*2)
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
static void BOOTi_SetMainMemModeForNTR( void );
void BOOTi_SetMainMemModeForNTRCore( u32 addr );

static u32 SearchBinaryArm( void );
#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
static u32 SearchBinaryThumb( void );
#endif

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static REBOOTTarget target;

// ���������X�g
static u32 mem_list[PRE_CLEAR_NUM_MAX + 1 + COPY_NUM_MAX + COPY_HEADER_NUM_MAX + 2 + POST_CLEAR_NUM_MAX + 1] = 
{
	// pre clear
	HW_WRAM_B_OR_C_MIRROR,   SYSM_OWN_ARM7_WRAM_ADDR_END - HW_WRAM_B_OR_C_MIRROR, // 0x03780000 - 0x0380f000 SYSM_OWN_ARM7_WRAM_ADDR��HW_WRAM_A, HW_WRAM_B���܂Ƃ߂ăN���A
	SYSM_OWN_ARM7_MMEM_ADDR, SYSM_OWN_ARM7_MMEM_ADDR_END - SYSM_OWN_ARM7_MMEM_ADDR,
	SYSM_OWN_ARM9_MMEM_ADDR, SYSM_OWN_ARM9_MMEM_ADDR_END - SYSM_OWN_ARM9_MMEM_ADDR,
	OS_BOOT_CODE_BUF_END, 1,     // REBOOT�R�A�R�[�h�ƃX�^�b�N�̌��ԃT�C�Y�̓��������X�g������ɍ����ւ���iNULL�ł�REBOOT_GetCoreStackSize�����s����j
	HW_LAUNCHER_DELIVER_PARAM_BUF_END, OS_BOOT_CODE_BUF - (HW_LAUNCHER_DELIVER_PARAM_BUF_END + (HW_WRAM_AREA_END - HW_PRV_WRAM_END)), // �����`���[�̃X�^�b�N�̈�
	HW_WRAM_BASE, HW_WRAM_SIZE,  // ���LWRAM�@�@Launcher�̓���z�u�Ȃ̂ŁABASE����T�C�Y�Ԃ�
	HW_WRAM_C, HW_WRAM_C_SIZE,   // 0x03700000 - 0x03740000
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
	SYSM_TWL_ARM7_LOAD_MMEM_END, OS_BOOT_A9CODE_BUF,
	OS_BOOT_A9CODE_BUF + OS_BOOT_CODE_SIZE, SYSM_TWL_ARM9_LTD_LOAD_MMEM,
	SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN,
	HW_EXCP_VECTOR_MAIN, HW_EXCP_VECTOR_MAIN + 4, // ARM9��O�x�N�^
	HW_EXCP_VECTOR_BUF,  HW_EXCP_VECTOR_BUF  + 4, // ARM7��O�x�N�^
	NULL,
};

static u32 nitro_post_clear_list[POST_CLEAR_NUM_MAX + 1] = 
{
	SYSM_NTR_ARM7_LOAD_MMEM_END, OS_BOOT_A9CODE_BUF,
	SYSM_TWL_ARM9_LTD_LOAD_MMEM_END, SYSM_DBG_NTR_SYSTEM_BUF,
	SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, SYSM_TWL_MOUNT_INFO_TMP_BUFFER + SYSM_MOUNT_INFO_SIZE + OS_MOUNT_PATH_LEN,
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
    *( REGType16v *)0x04000180 = 0x0000;
//	reg_PXI_MAINPINTF = 0x0000;
}


static u32 SearchBinaryArm( void )
{
    u32 target_command_address = 0;
    u32 elem[TARGET_ARM_CODE_NUM];
    u32 i;
    
    OS_TPrintf("=====================================\n");
    for( i = 0; i < TARGET_ARM_CODE_NUM; i++ )
    {
        u32 count = 0;
        u32 *p;

        elem[i] = 0;
        p = (u32 *)MI_CpuFind32( target_code_list_arm[i], 0x0, TARGET_ARM_CODE_MAX_SIZE * sizeof(u32) );
        if( p )
        {
            elem[i] = (u32)(p - target_code_list_arm[i]) * sizeof(u32);
        }
    }

    for( i = 0; i < TARGET_ARM_CODE_NUM; i++ )
    {
        u32 search_size = MAJIKON_APP_ARM7_STATIC_BUFFER_SIZE;
        u32 current     = MAJIKON_APP_ARM7_STATIC_BUFFER;
        u32 *codep      = target_code_list_arm[i];
        u32 hit         = 0;
        BOOL isFinish   = FALSE;

        while( search_size >= elem[i] || hit )
	    {
            if( *(u32 *)current != *codep )
            {
                current     += sizeof(u32);
                search_size -= sizeof(u32);
                continue;
            }
            
            while( *(u32 *)current == *codep )
            {
                hit += sizeof(u32);
                
	            if( *(u32 *)current == MAJIKON_APP_TARGET_COMMAND_ARM )
	            {
	                OS_TPrintf("*** Target Command Find!!\n");
	                target_command_address = current;
	            }

                if( hit == elem[i] )
                {
                    isFinish = TRUE;
                    break;
                }

                codep++;
                current += sizeof(u32);
                search_size -= sizeof(u32);
            }
            
            if( isFinish )
            {
                OS_TPrintf("*** Target Code Find!!\n");
                break;
            }
            target_command_address = 0;
            hit                    = 0;
            codep                  = target_code_list_arm[i];
	    }

        if( isFinish )
        {
            OS_TPrintf("Match!!\n");
            break;
        }
    }

    OS_TPrintf("\ntarget address : 0x%08x\n", target_command_address);
    OS_TPrintf("=====================================\n");
    
    return target_command_address;
}


#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
static u32 SearchBinaryThumb( void )
{
    u32 target_command_address = 0;
    u32 elem[TARGET_THUMB_CODE_NUM];
    u32 i;
    
    OS_TPrintf("=====================================\n");
    for( i = 0; i < TARGET_THUMB_CODE_NUM; i++ )
    {
        u32 count = 0;
        u32 *p;

        elem[i] = 0;
        p = (u32 *)MI_CpuFind32( target_code_list_thumb[i], 0x0, TARGET_THUMB_CODE_MAX_SIZE * sizeof(u16) );
        if( p )
        {
            elem[i] = (u32)((u16 *)p - target_code_list_thumb[i]) * sizeof(u16);
        }
    }

    for( i = 0; i < TARGET_THUMB_CODE_NUM; i++ )
    {
        u32 search_size = MAJIKON_APP_ARM7_STATIC_BUFFER_SIZE;
        u32 current     = MAJIKON_APP_ARM7_STATIC_BUFFER;
        u32 hit         = 0;
        BOOL isFinish   = FALSE;
        u16 *codep      = target_code_list_thumb[i];

        OS_TPrintf("search code %d start\n", i);
        while( search_size >= elem[i] || hit )
	    {
            if( *(u16 *)current != *codep )
            {
                current     += sizeof(u16);
                search_size -= sizeof(u16);
                continue;
            }
            
            while( *(u16 *)current == *codep )
            {
                hit += sizeof(u16);

	            if( *(u16 *)current == MAJIKON_APP_TARGET_COMMAND_THUMB )
	            {
	                OS_TPrintf("*** Target Command Find!!\n");
	                target_command_address = current;
	            }

                if( hit == elem[i] )
                {
                    isFinish = TRUE;
                    break;
                }

                codep++;
                current += sizeof(u16);
                search_size -= sizeof(u16);
            }
            
            if( isFinish )
            {
                OS_TPrintf("*** Target Code Find!!\n");
                break;
            }
            target_command_address = 0;
            hit                    = 0;
            codep                  = target_code_list_thumb[i];
	    }

        if( isFinish )
        {
            OS_TPrintf("Match!!\n");
            break;
        }
    }

    OS_TPrintf("\ntarget address : 0x%08x\n", target_command_address);
    OS_TPrintf("=====================================\n");

    return target_command_address;
}
#endif


BOOL BOOT_WaitStart( void )
{
	if( (reg_PXI_MAINPINTF & 0x000f ) == 0x000f ) {
        u32 target_address_arm;
#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
        u32 target_address_thumb;
#endif
        
		// �œK�������ƃ|�C���^�����������������ł͉����R�[�h�͐�������܂���
		ROM_Header *th = (ROM_Header *)SYSM_APP_ROM_HEADER_BUF;          // TWL�g��ROM�w�b�_�i�L���b�V���̈�ADS�A�v���ɂ͖����j
		ROM_Header *dh = (ROM_Header *)(SYSMi_GetWork()->romHeaderNTR);  // DS�݊�ROM�w�b�_�i��L���b�V���̈�j
		// ���u�[�g
		REBOOTi_SetTwlRomHeaderAddr( th );
		REBOOTi_SetRomHeaderAddr( dh );
		REBOOTi_SetPostFinalizeCallback( BOOTi_RebootCallback );

        // ARM7�o�b�t�@( 0x0238_0000 )�������o�C�i�����T�[�`
        target_address_arm = SearchBinaryArm();

#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
        if( target_address_arm == 0x0 )
        {
            target_address_thumb = SearchBinaryThumb();
        }
#endif
        
        if( target_address_arm )
        {
            // �p�b�`���ߍ���
            MI_CpuCopy8( patch_core_arm, (u32 *)MAJIKON_PATCH_ADDR, sizeof(patch_core_arm));
            
            // �p�b�`�ɔ�΂��������ߍ���
            // ���ߍ��ރR�[�h��2���߂���̂ŁA1�O�̃A�h���X��Ԃ�
            MI_CpuCopy8( patch_jump_arm, (u32 *)(target_address_arm - sizeof(u32)), sizeof(patch_jump_arm));
        }
#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
        else if( target_address_thumb )
        {
            // �p�b�`���ߍ���
            MI_CpuCopy8( patch_core_arm, (u32 *)MAJIKON_PATCH_ADDR, sizeof(patch_core_arm));
            
            // �p�b�`�ɔ�΂��������ߍ���
            // dcd������4�o�C�g�A���C�����g�Ƃ�ĂȂ��ƃ_���Ȃ̂Œ����B
            if( !(target_address_thumb % 0x4) )
            {
                target_address_thumb -= 0x2;
            }
            MI_CpuCopy8( patch_jump_thumb, (u32 *)target_address_thumb, sizeof(patch_jump_thumb));
        }
#endif

		OS_Boot( OS_BOOT_ENTRY_FROM_ROMHEADER, mem_list, target );
	}
	return FALSE;
}

// SDK��Finalize������ɌĂяo�����
static void BOOTi_RebootCallback( void** entryp, void* mem_list_v, REBOOTTarget* target )
{
#pragma unused(entryp)
		u32* mem_list = mem_list_v;
		ROM_Header *th = (void*)REBOOTi_GetTwlRomHeaderAddr();
		ROM_Header *dh = (void*)REBOOTi_GetRomHeaderAddr();
		ROM_Header *ch = (void*)HW_CARD_ROM_HEADER;
		BOOL isNtrMode;
		u32  spiLockId;

		(void)OS_DisableIrq();							// �����Ŋ��荞�݋֎~�ɂ��Ȃ��ƃ_���B
		(void)OS_SetIrqMask(0);							// SDK�o�[�W�����̃T�[�`�Ɏ��Ԃ�������ƁAARM9��HALT�ɂ������Ă��܂��AARM7�̃T�E���h�X���b�h��ARM9��FIFO�Ńf�[�^���M���悤�Ƃ��Ă�FIFO����t�ő��M�ł��Ȃ���ԂŖ������[�v�ɓ����Ă��܂��B
		(void)OS_SetIrqMaskEx(0);

		MI_StopAllDma();                                // ���荞�݋֎~��Ԃ�DMA��~
		MI_StopAllNDma();

		// �N���A�v��NTR-ROM�w�b�_�ւ̃p�b�`�����̂��߃R�s�[
		MI_CpuCopyFast( th, dh, HW_CARD_ROM_HEADER_SIZE );

		// �J�[�hNTR-ROM�w�b�_��NAND�A�v����DS�_�E�����[�h�v���C�̂��߃R�s�[
		MI_CpuCopyFast( (void*)SYSM_CARD_ROM_HEADER_BAK, ch, HW_CARD_ROM_HEADER_SIZE );

    	if ( (((ROM_Header *)SYSM_CARD_ROM_HEADER_BAK)->s.platform_code & PLATFORM_CODE_FLAG_TWL) && (dh->s.platform_code & PLATFORM_CODE_FLAG_TWL) ){
			// �J�[�hTWL-ROM�w�b�_��HW_TWL_CARD_ROM_HEADER_BUF�ɃR�s�[
			MI_CpuCopyFast( (void*)SYSM_CARD_ROM_HEADER_BAK, (void*)HW_TWL_CARD_ROM_HEADER_BUF, HW_TWL_CARD_ROM_HEADER_BUF_SIZE );
        }

        // �f�o�b�K�ɂ��ROM�G�~�����[�V��������NTR-ROM�w�b�_�o�b�t�@��
        // �Q�[���R�}���h�p�����[�^���X�N�����u��OFF�ݒ�ɏ���������
        if ( SYSM_IsRunOnDebugger() )
        {
            // NitroSDK�o�O�΍�Ńu�[�g���f�B�A��ʂɊւ�炸ROM�w�b�_���펞��������
            dh->s.game_cmd_param &= ~SCRAMBLE_MASK;
        }
        // �J�[�hROM�w�b�_�i��L���b�V���̈�j�͏펞�ݒ�
        ch->s.game_cmd_param = SYSMi_GetWork()->gameCommondParam;

		// ���̏����́ADS�_�E�����[�h�v���C���ōs���B
		// MI_CpuCopyFast ( ch, (void *)MB_CARD_ROM_HEADER_ADDRESS, 0x160);

		// �u���b�N���X�g���`�F�b�N���A�N��������������
		BOOTi_CheckTitleBlackList( (void*)th );
		
		// �}�E���g�����ꎞ�I��SYSM_TWL_MOUNT_INFO_TMP_BUFFER�ɓo�^
		// �����܂ł�SYSM_APP_ROM_HEADER_BUF�̃L���b�V�������C�g�o�b�N����Ă���K�v����
		SYSMi_SetBootAppMountInfo( &SYSMi_GetWork2()->bootTitleProperty );
		
		// FS�ɂ����shared�̈�ɃR�s�[���ꂽ�����`���[���g�̃}�E���g�p�X�̃N���A
		MI_CpuClearFast((char *)HW_TWL_FS_BOOT_SRL_PATH_BUF, OS_MOUNT_PATH_LEN);
		
		// NAND/TMP�A�v���pKeyTable�̐���
		if( ! SYSMi_GetWork()->flags.arm9.isCardBoot )
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
		
		// NTR���[�h���ɂ�ARM9��DMA�����C�����������A�N�Z�X���Ă��Ȃ��^�C�~���O�Ńf�[�^��ێ����[�h�֐؂�ւ��i�t���[�Y�΍�j
		if ( isNtrMode )
		{
            // ARM9��OS_Halt�ɒB����܂ő҂i0.1msec�j
            OS_SpinWait(HW_CPU_CLOCK_ARM7 / 10000);
			BOOTi_SetMainMemModeForNTR();
		}
		
		BOOTi_ClearREG_RAM();							// ARM7���̃����������W�X�^�N���A�B
		reg_MI_MBK9 = 0;								// �SWRAM�̃��b�N����
		reg_PXI_MAINPINTF = MAINP_SEND_IF | 0x0100;		// ARM9�ɑ΂��ău�[�g����悤IRQ�ŗv���{ARM7�̃X�e�[�g���P�ɂ���B
		
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
				// [TODO:] ���ł�post_clear_list�Ɋ܂܂�Ă���C������B�v�m�F
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
			}
			else
			{
				*target = REBOOT_TARGET_DS_APP;
			}

			// I2S��~�iMCLK�͓���p���j
			reg_SND_SMX_CNT &= ~REG_SND_SMX_CNT_E_MASK;

			spiLockId = (u32)OS_GetLockID();
			if (spiLockId == OS_LOCK_ID_ERROR)
			{
		        OS_Warning("%s: OS_GetLockID failed.\n", __FUNCTION__);
			}
			SPI_Lock(spiLockId); // CODEC�pSPI�r�����b�N

			// CODEC�ď�����
			CDC_Init();

			// CODEC-DS���[�h�ւ̑J��
            if ( isNtrMode || th->s.exFlags.codec_mode == OS_CODECMODE_NITRO )
            {
				// DS�^�C�g���̃X�s�[�J�ݒ���s��
	            if ( *target == REBOOT_TARGET_DS_APP )
	            {
                    DS_SetSpeakerVolume( dh );
				}
				CDC_GoDsMode();
            }

			SPI_Unlock(spiLockId); // CODEC�pSPI�r�����b�N

			// I2S�ĊJ
			// DS�T�E���h�FDSP = 8:0
			// 32KHz
			reg_SND_SMX_CNT = REG_SND_SMX_CNT_MIX_RATE_MASK |
							  REG_SND_SMX_CNT_E_MASK;

            // USG�ȑO��DS�A�v���ɂ͖����p�b�`��K�p
            // ARM9�̓��[�h�A�h���X���L���b�V���X�g�A���Ă����K�v����
            if ( *target == REBOOT_TARGET_DS_APP )
            {
                DS_InsertWLPatch( dh );
            }
		}
}

static void BOOTi_SetMainMemModeForNTR( void )
{
    BOOTi_SetMainMemModeForNTRCore(HW_TWL_MAIN_MEM_END - 2);
    BOOTi_SetMainMemModeForNTRCore(HW_TWL_MAIN_MEM_EX_END - 2);
}

asm void BOOTi_SetMainMemModeForNTRCore( u32 addr )
{
        ldr     r1,  =MMEM_TCR0
        ldr     r2,  =MMEM_TCR1_DS
        ldrh    r3, [r0]
        strh    r3, [r0]
        strh    r3, [r0]
        ldr     r3,  =MMEM_TCR2
        strh    r1, [r0]
        strh    r2, [r0]
        strh    r3, [r0]
        bx      lr
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
static void BOOTi_DeleteElementFromList( u32 *list, u32 index )
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
		BOOTi_DeleteElementFromList( regionlist, (u32)n );
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
