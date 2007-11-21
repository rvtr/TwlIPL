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
#include <sysmenu/boot/common/boot.h>
#include <firm/format/wram_regs.h>
#include "reboot.h"
//#include <nitro/mb.h>
//#include "IPL2_work.h"
//#include "define.h"


// define data-------------------------------------------------------
#define SUBP_RECV_IF_ENABLE     0x4000

#define C1_DTCM_ENABLE          0x00010000      // �f�[�^�s�b�l �C�l�[�u��
#define C1_EXCEPT_VEC_UPPER     0x00002000      // ��O�x�N�^ ��ʃA�h���X�i������ɐݒ肵�ĉ������j
#define C1_SB1_BITSET           0x00000078      // ���W�X�^�P�p�P�Œ�r�b�g��i����A�{�[�g���f���ADATA32�\���V�O�i������APROG32�\���V�O�i������A���C�g�o�b�t�@�C�l�[�u���j

// extern data-------------------------------------------------------

// function's prototype----------------------------------------------
static void BOOTi_ClearREG_RAM( void );
static void BOOTi_StartBOOT( void );

static void ResetCP15( void );

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
    // �������N���A���X�g�̐ݒ�
    static u32 clr_list[] =
    {
        HW_ITCM, HW_ITCM_SIZE,
        //HW_DTCM, HW_DTCM_SIZE,
        NULL
    };

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

    //BOOT_Core();          // never return

    // �v���e�N�V�������j�b�g�̏�����
    ResetCP15();

    // SDK���ʃ��u�[�g
    OS_Boot( (void *)*(u32 *)(HW_TWL_ROM_HEADER_BUF + 0x24), clr_list );
}


// �g�p�������W�X�^���������̃N���A
static void BOOTi_ClearREG_RAM( void )
{
    // �Ōオ�T�u�v���Z�b�T���荞�ݑ҂��Ȃ̂ŁAIME�̓N���A���Ȃ��B
    (void)OS_SetIrqMask( 0 );
    (void)OS_ResetRequestIrqMask( (u16)~0 );

    // �������N���A
    GX_SetBankForLCDC( GX_VRAM_LCDC_ALL );                          // VRAM     �N���A
    MI_CpuClearFast( (void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE );
    (void)GX_DisableBankForLCDC();
//  MI_CpuClearFast( (void *)HW_ITCM,   HW_ITCM_SIZE );         // ITCM     �N���A  ��ITCM�ɂ�SDK�̃R�[�h�������Ă���̂ŁABOOT_Core�ŃN���A����B
//  MI_CpuClearFast( (void *)HW_DTCM,   HW_DTCM_SIZE - 0x800 ); // DTCM     �N���A  ��DTCM�̓X�^�b�N&SDK�ϐ�����Ȃ̂ŁA�Ō��BOOT_Core�ŃN���A����B
    MI_CpuClearFast( (void *)HW_OAM,    HW_OAM_SIZE );          // OAM      �N���A
    MI_CpuClearFast( (void *)HW_PLTT,   HW_PLTT_SIZE );         // �p���b�g �N���A
    MI_CpuClearFast( (void *)HW_DB_OAM, HW_DB_OAM_SIZE );       // OAM      �N���A
    MI_CpuClearFast( (void *)HW_DB_PLTT,HW_DB_PLTT_SIZE );      // �p���b�g �N���A

    // ���W�X�^�N���A
    MI_CpuClearFast( (void*)( HW_REG_BASE + 0x8 ),    0x12c );  // BG0CNT    �` KEYCNT
    MI_CpuClearFast( (void*)( HW_REG_BASE + 0x280 ),  0x40 );   // DIVCNT    �` SQRTD3
    MI_CpuClearFast( (void*)( HW_REG_BASE + 0x1000 ), 0x6e );   // DISP1CNT1 �` DISPBRTCNT1
    CP_SetDiv32_32( 0, 1 );
    reg_PXI_SUBP_FIFO_CNT = 0x4008;
    reg_GX_DISPCNT  = 0;
    reg_GX_DISPSTAT = 0;                                        // �� reg_GX_VCOUNT�̓x�^�N���A�ł��Ȃ��̂ŁA���̐擪�����̃N���A�𕪗�����B

    // �N���A���Ă��Ȃ����W�X�^�́AVCOUNT, PIFCNT, MC-, EXMEMCNT, IME, RBKCNT1, PAUSE, POWLCDCNT, �S3D�n�B
}

//-----------------------------------------------------------------------
// �V�X�e������R�v���Z�b�T ���Z�b�g
//-----------------------------------------------------------------------
#include <twl/code32.h>
asm static void ResetCP15( void )
{
        // �v���e�N�V�������j�b�g���L���b�V����ITCM�����BDTCM�͗L���i�X�^�b�N���N���A���邽�߁j
        ldr         r0, = C1_DTCM_ENABLE  | C1_EXCEPT_VEC_UPPER | C1_SB1_BITSET
        mcr         p15, 0, r0, c1, c0, 0

        // ITCM�̊��蓖�Ă�����
        mov         r0, #0
        mcr         p15, 0, r0, c6, c5, 0

        // DTCM�̊��蓖�Ă�����
//      mov         r0,#0
//      mcr         p15, 0, r0, c9, c1, 0

        // �L���b�V��������
        mov         r0, #0
        mcr         p15, 0, r0, c7, c5, 0           // ���߃L���b�V��
        mcr         p15, 0, r0, c7, c6, 0           // �f�[�^�L���b�V��

        // ���C�g�o�b�t�@ �G���v�e�B�҂�
        mcr         p15, 0, r0, c7, c10, 4

        bx          lr
}
#include <twl/codereset.h>
