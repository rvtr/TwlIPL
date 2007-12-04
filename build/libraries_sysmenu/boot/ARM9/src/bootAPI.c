/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     bootAPI.c

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
#include <twl/os/common/format_rom.h>
#include <sysmenu.h>
#include <firm/format/wram_regs.h>
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

    for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {                // DMA�̒�~
        MI_StopDma( (u16)i );
    }

//  FinalizeCardPulledOut();                                // �J�[�h�������o�I������
    BOOTi_ClearREG_RAM();                                   // ���W�X�^��RAM�N���A
    (void)GX_VBlankIntr( FALSE );
    (void)OS_SetIrqFunction( OS_IE_SUBP, ie_subphandler );
    OS_EnableInterrupts();
    (void)OS_SetIrqMask( OS_IE_SUBP );                      // �T�u�v���Z�b�T���荞�݂݂̂����B
    reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;        // ARM9�X�e�[�g�� "0x0f" ��
                                                            // ������FIFO�̓N���A�ς݂Ȃ̂ŁA�g��Ȃ��B
    // ARM7����̒ʒm�҂�
    OS_WaitIrq( 1, OS_IE_SUBP );

    OS_TPrintf( "INTR SUBP passed!!\n" );
    // ���荞�݂��N���A���čŏI�u�[�g�V�[�P���X�ցB
    reg_PXI_SUBPINTF &= 0x0f00;                             // �T�u�v���Z�b�T���荞�݋��t���O���N���A
    (void)OS_DisableIrq();
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u16)~0 );

    // WRAM�̔z�u
    {
        ROM_Header_Short *pROMH = (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
        MIHeader_WramRegs *pWRAMREGS = (MIHeader_WramRegs *)pROMH->main_wram_config_data;
        reg_GX_VRAMCNT_C    = pWRAMREGS->main_vrambnk_c;
        reg_GX_VRAMCNT_D    = pWRAMREGS->main_vrambnk_d;
        reg_GX_VRAMCNT_WRAM = pWRAMREGS->main_wrambnk_01;
    }
	
	// ROM�w�b�_�o�b�t�@���R�s�[
	MI_CpuCopy32( (void *)HW_TWL_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	
    // SDK���ʃ��u�[�g
	{
	    // ���������X�g�̐ݒ�
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
	    
	    // [TODO]�Ĕz�u���X�g�̍쐬�Ɛݒ�i�ق�ARM7���ł��̂ł�����͋�j
	    static u32 relocate_list[] =
	    {
			NULL
		};
	    
		REBOOTTarget target = REBOOT_TARGET_TWL_SECURE_SYSTEM;
		ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
		
		// �A�v���P�[�V�����I��
		if ( dh->s.platform_code )
		{
//			target = REBOOT_TARGET_TWL_APP;
		}
		else
		{
			target = REBOOT_TARGET_DS_APP;
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
    (void)OS_ResetRequestIrqMask( (u16)~0 );

	// ���W�X�^�N���A�͊�{�I�� OS_Boot �ōs��
}

