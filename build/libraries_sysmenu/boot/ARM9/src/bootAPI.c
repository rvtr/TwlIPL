/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     bootAPI.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

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
#include <twl/camera.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu.h>
#include <sysmenu/ds.h>
#include <firm/format/wram_regs.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include <firm/format/from_brom.h>
#include "reboot.h"


// define data-------------------------------------------------------
#define SUBP_RECV_IF_ENABLE     0x4000

#define C1_DTCM_ENABLE          0x00010000      // �f�[�^�s�b�l �C�l�[�u��
#define C1_EXCEPT_VEC_UPPER     0x00002000      // ��O�x�N�^ ��ʃA�h���X�i������ɐݒ肵�ĉ������j
#define C1_SB1_BITSET           0x00000078      // ���W�X�^�P�p�P�Œ�r�b�g��i����A�{�[�g���f���ADATA32�\���V�O�i������APROG32�\���V�O�i������A���C�g�o�b�t�@�C�l�[�u���j

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------
static void BOOTi_ClearREG_RAM( void );
static void BOOTi_StartBOOT( void );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

// const data--------------------------------------------------------
void BOOT_Init( void )
{
    reg_PXI_SUBPINTF = 0x0000;
}

static void ie_subphandler( void )
{
    OS_TPrintf( "INTR SUBP!!\n" );
    OS_SetIrqCheckFlag( OS_IE_SUBP );
}

// �u�[�g���������āAARM7����̒ʒm��҂B
void BOOT_Ready( void )
{
    int i;

    // �G���g���A�h���X�̐��������`�F�b�N���A�����ȏꍇ�͖������[�v�ɓ���B
//  SYSMi_CheckEntryAddress();

//  FinalizeCardPulledOut();                                // �J�[�h�������o�I������
    BOOTi_ClearREG_RAM();                                   // ���W�X�^��RAM�N���A
    (void)GX_VBlankIntr( FALSE );

    for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {                // ���荞�݋֎~��Ԃ�DMA��~
        MI_StopDma( (u16)i );
        MI_StopNDma( (u16)i );
    }

    (void)OS_SetIrqFunction( OS_IE_SUBP, ie_subphandler );
    OS_EnableInterrupts();
    (void)OS_SetIrqMask( OS_IE_SUBP );                      // �T�u�v���Z�b�T���荞�݂݂̂����B
    MI_SetWramBank(MI_WRAM_ARM7_ALL);                       // WRAM0/1�̍ŏI�z�u��OS_Boot�ōs��
    reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;        // ARM9�X�e�[�g�� "0x0f" ��
                                                            // ������FIFO�̓N���A�ς݂Ȃ̂ŁA�g��Ȃ��B
    // ARM7����̒ʒm�҂�
    OS_WaitIrq( 1, OS_IE_SUBP );

    OS_TPrintf( "INTR SUBP passed!!\n" );
    // ���荞�݂��N���A���čŏI�u�[�g�V�[�P���X�ցB
    reg_PXI_SUBPINTF &= 0x0f00;                             // �T�u�v���Z�b�T���荞�݋��t���O���N���A
    (void)OS_DisableIrq();
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u32)~0 );

    // WRAM�̔z�u
    {
        ROM_Header_Short *pROMH = (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
        MIHeader_WramRegs *pWRAMREGS = (MIHeader_WramRegs *)pROMH->main_wram_config_data;
        reg_GX_VRAMCNT_C    = pWRAMREGS->main_vrambnk_c;
        reg_GX_VRAMCNT_D    = pWRAMREGS->main_vrambnk_d;
        // WRAM0/1�̍ŏI�z�u��OS_Boot�ōs��
    }

    // SDK���ʃ��u�[�g
	{
	    // ���������X�g�̐ݒ�
		// [TODO:] �V���b�v�A�v���Ō����c���ꍇ�ANAND�t�@�[�������̗̈�(ITCM�ɂ���)�������Ȃ��悤�ɒ��ӁB
		//         �o�b�t�@�I�[�o�����̌��O����̂��ߕs�v�Ȍ���pre clear�ŏ����B
	    static u32 mem_list[] =
	    {
            // pre clear
	        HW_ITCM, HW_ITCM_SIZE,
	        //HW_DTCM, HW_DTCM_SIZE,
	        NULL,
            // copy forward
	        NULL,
            // copy backward
	        NULL,
            // post clear
	        NULL,
	    };
	    
	    // �Ĕz�u���X�g�̍쐬�Ɛݒ�i�ق�ARM7���ł��̂ł�����͋�j
	    static u32 relocate_list[] =
	    {
			NULL
		};
	    
		REBOOTTarget target = REBOOT_TARGET_TWL_SYSTEM;
        BOOL ds = FALSE;
		ROM_Header *th = (ROM_Header *)HW_TWL_ROM_HEADER_BUF;  // TWL�g��ROM�w�b�_�iDS�A�v���ɂ͖����j
		ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
		
		// �A�v���P�[�V�����I��
		if ( dh->s.platform_code )
		{
			if ( th->s.titleID_Hi & TITLE_ID_HI_APP_TYPE_MASK )
			{
				if ( th->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK )
				{
					target = REBOOT_TARGET_TWL_SECURE_SYSTEM;
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
            OS_Terminate();
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

#if defined(FIRM_USE_TWLSDK_KEYS) || defined(SYSMENU_DISABLE_RETAIL_BOOT)
        // TwlSDK���̌����g���Ă��鎞�͐��i�pCPU�ł�TWL�A�v���̓u�[�g���Ȃ�
        if ( ! (*(u8*)OS_CHIPTYPE_DEBUGGER_ADDR & OS_CHIPTYPE_DEBUGGER_MASK) && !ds )
        {
            OS_Terminate();
        }
#endif // FIRM_USE_SDK_KEYS || SYSMENU_DISABLE_RETAIL_BOOT

        // USG�ȑO��DS�A�v���ɂ͖����p�b�`��K�p
        // �i�L���b�V���̈�̔r������ȗ����̂���ARM9�ōs���j
        if ( REBOOT_TARGET_DS_APP )
        {
            DS_InsertWLPatch();
        }

        // �f�o�b�K�ɂ��ROM�G�~�����[�V��������NTR-ROM�w�b�_�o�b�t�@��
        // �Q�[���R�}���h�p�����[�^���X�N�����u��OFF�ݒ�ɏ���������
        dh->s.game_cmd_param = SYSMi_GetWork()->gameCommondParam;

		// �Z�L���A�V�X�e���ȊO�͌��������Ă���
		if ( target != REBOOT_TARGET_TWL_SECURE_SYSTEM )
		{
			MI_CpuClearFast((void*)HW_FIRM_FROM_BROM_BUF, HW_FIRM_FROM_BROM_BUF_SIZE);
		}

		// �N������^�[�Q�b�g�̎�ނ��w�肷��K�v����
		OS_Boot( dh->s.main_entry_address, mem_list, target );
	}
}


// �g�p�������W�X�^���������̃N���A
static void BOOTi_ClearREG_RAM( void )
{
    // �Ōオ�T�u�v���Z�b�T���荞�ݑ҂��Ȃ̂ŁAIME�̓N���A���Ȃ��B
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u32)~0 );

	// ���W�X�^�N���A�͊�{�I�� OS_Boot �ōs��
}

