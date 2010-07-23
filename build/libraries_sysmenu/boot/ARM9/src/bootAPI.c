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
#include <twl/dsp.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu.h>
#include <sysmenu/hotsw.h>
#include <sysmenu/ds.h>
#include <firm/format/wram_regs.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include <firm/format/from_firm.h>
#include "reboot.h"
#include "../../../hotsw/ARM7/include/hotswTypes.h"


// define data-------------------------------------------------------
#define SUBP_RECV_IF_ENABLE     		0x4000

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------
static void BOOTi_ClearREG_RAM( void );
static void BOOTi_RebootCallback( void** entryp, void* mem_list, REBOOTTarget* target );

// global variables--------------------------------------------------

// static variables--------------------------------------------------

static REBOOTTarget target;

// ���������X�g
// �o�b�t�@�I�[�o�����̃��X�N����̂��ߕs�v�Ȍ���pre clear�ŏ����B
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

void BOOT_Ready( void )
{
	// �œK�������ƃ|�C���^�����������������ł͉����R�[�h�͐�������܂���
	ROM_Header *th = (ROM_Header *)SYSM_APP_ROM_HEADER_BUF;          // TWL�g��ROM�w�b�_�i�L���b�V���̈�ADS�A�v���ɂ͖����j
	ROM_Header *dh = (ROM_Header *)(SYSMi_GetWork()->romHeaderNTR);  // DS�݊�ROM�w�b�_�i��L���b�V���̈�j

	// HOTSW�I�������҂�
#ifndef SYSM_NO_LOAD
	while( ! HOTSW_isFinalized() ) {
		OS_Sleep( 1 );
	}
#endif

	// ���u�[�g
	REBOOTi_SetTwlRomHeaderAddr( th );
	REBOOTi_SetRomHeaderAddr( dh );
	REBOOTi_SetPostFinalizeCallback( BOOTi_RebootCallback );
	OS_Boot( OS_BOOT_ENTRY_FROM_ROMHEADER, mem_list, target );
}

// �u�[�g���������āAARM7����̒ʒm��҂B
// SDK��Finalize������ɌĂяo�����
static void BOOTi_RebootCallback( void** entryp, void* mem_list_v, REBOOTTarget* target )
{
#pragma unused(entryp)
	u32* mem_list = mem_list_v;
	ROM_Header *th = (void*)REBOOTi_GetTwlRomHeaderAddr();
	ROM_Header *dh = (void*)REBOOTi_GetRomHeaderAddr();
    BOOL isNtrMode;

    // �G���g���A�h���X�̐��������`�F�b�N���A�����ȏꍇ�͖������[�v�ɓ���B
//  SYSMi_CheckEntryAddress();

//  FinalizeCardPulledOut();                                // �J�[�h�������o�I������
	BOOTi_ClearREG_RAM();                                   // ���W�X�^��RAM�N���A
    (void)GX_VBlankIntr( FALSE );
	DC_StoreAll();

    MI_StopAllDma();                                        // ���荞�݋֎~��Ԃ�DMA��~
    MI_StopAllNDma();

    (void)OS_SetIrqFunction( OS_IE_SUBP, ie_subphandler );
    OS_EnableInterrupts();
    (void)OS_SetIrqMask( OS_IE_SUBP );                      // �T�u�v���Z�b�T���荞�݂݂̂����B
    MI_SetWramBank(MI_WRAM_ARM7_ALL);                       // WRAM0/1�̍ŏI�z�u��OS_Boot�ōs��
    reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;        // ARM9�X�e�[�g�� "0x0f" ��
                                                            // ������FIFO�̓N���A�ς݂Ȃ̂ŁA�g��Ȃ��B
    // ARM7����̒ʒm�҂�
    // ���̎��_��ARM7�ɂ��dh�ւ�NTR-ROM�w�b�_���R�s�[���ۏ؂����
    OS_WaitInterrupt( 1, OS_IE_SUBP );

    OS_TPrintf( "INTR SUBP passed!!\n" );

    // SDK��Finalize����������Ƀu�[�g��ʂ��A�v���̂��̂֕ύX
    ( (OSBootInfo *)OS_GetBootInfo() )->boot_type = SYSMi_GetWork()->appBootType;

    // ���荞�݂��N���A���čŏI�u�[�g�V�[�P���X�ցB
    reg_PXI_SUBPINTF &= 0x0f00;                             // �T�u�v���Z�b�T���荞�݋��t���O���N���A
    (void)OS_DisableIrq();
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u32)~0 );

    // TWL/NTR���[�h����
    if ( ! dh->s.platform_code
#ifndef SYSM_NO_LOAD
         || (SYSM_IsRunOnDebugger() && ((SYSMRomEmuInfo*)HOTSW_GetRomEmulationBuffer())->isForceNTRMode)
#endif
       )
    {
        isNtrMode = TRUE;
    }
    else
    {
        isNtrMode = FALSE;
    }
	
    // WRAM�̔z�u
    {
        MIHeader_WramRegs *pWRAMREGS = (MIHeader_WramRegs *)th->s.main_wram_config_data;
        int i;
        reg_GX_VRAMCNT_C    = pWRAMREGS->main_vrambnk_c;
        reg_GX_VRAMCNT_D    = pWRAMREGS->main_vrambnk_d;
        // WRAM0/1�̍ŏI�z�u��OS_Boot�ōs��

        // DSP��~
        DSP_ResetOn();     			// DSP�u���b�N������
        DSP_ResetInterfaceCore();   // DSP-A9IF�̏�����
        DSP_PowerOff();    			// DSP��OFF

        // TWL�g��WRAM
        // ARM7��reboot�ŃN���A
        for (i=0; i<MI_WRAM_C_MAX_NUM; i++)
        {
            MIi_SetWramBankEnable_B(i, MI_WRAM_ENABLE);
            MIi_SetWramBankEnable_C(i, MI_WRAM_ENABLE);
        }
        MI_SwitchWram_B(MI_WRAM_DSP,  MI_WRAM_ARM7);
        MI_SwitchWram_B(MI_WRAM_ARM9, MI_WRAM_ARM7);
        MI_SwitchWram_C(MI_WRAM_DSP,  MI_WRAM_ARM7);
        MI_SwitchWram_C(MI_WRAM_ARM9, MI_WRAM_ARM7);
    }

    // SDK���ʃ��u�[�g
	{
		*target = REBOOT_TARGET_TWL_SYSTEM;
		
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

		// ���͕s�v�ɂȂ�̂ŁA�����Ă���
		{
			OSFromFirmBuf* fromFirm = (void*)HW_FIRM_FROM_FIRM_BUF;
			MI_CpuClearFast(fromFirm, sizeof(OSFromFirmBuf));
		}
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

