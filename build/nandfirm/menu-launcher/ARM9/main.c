/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - menu-launcher
  File:     main.c

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
#include <firm.h>

/* ���͂ǂ��ցH */
#define RSA_KEY_ADDR    OSi_GetFromBromAddr()->rsa_pubkey[7]

#define RSA_HEAP_SIZE   (4*1024)    // RSA�p�q�[�v�T�C�Y (�T�C�Y�����K�v)

static u8 acHeap[RSA_HEAP_SIZE] __attribute__ ((aligned (32)));
static SVCSignHeapContext acPool;

/*
    PROFILE_ENABLE ���`����Ƃ�����x�̃p�t�H�[�}���X�`�F�b�N���ł��܂��B
    ���p���邽�߂ɂ́Amain.c���ǂ����ɁAu32 profile[256]; u32 pf_cnt = 0; ��
    ��`����K�v������܂��B
*/
#define PROFILE_ENABLE

#ifdef SDK_FINALROM // FINALROM�Ŗ�����
#undef PROFILE_ENABLE
#endif

#ifdef PROFILE_ENABLE
#define PRFILE_MAX  128
u32 profile[PRFILE_MAX];
u32 pf_cnt = 0;
#endif

/***************************************************************
    PreInit

    FromBoot�̑Ή���OS_Init�O�ɕK�v�ȃ��C���������̏�����
***************************************************************/
static void PreInit(void)
{
    /*
     ���C���������֘A
    */
    // SHARED�̈�N���A (����������OK?)
    MIi_CpuClearFast( 0, (void*)HW_PXI_SIGNAL_PARAM_ARM9, HW_MAIN_MEM_SHARED_END-HW_PXI_SIGNAL_PARAM_ARM9);

    /*
        FromBrom�֘A
    */
    if ( !OSi_FromBromToMenu() )
    {
        OS_Terminate();
    }
}

/***************************************************************
    PostInit

    MI_LoadHeader�O�ɂ��Ȃ�(��100msec)���Ԃ�����̂ŁA�\�Ȃ�
    OS_Init��ɂ��낢�돈���������I
    ���C���������̏�����
***************************************************************/
extern u32 SDK_SECTION_ARENA_DTCM_START;
static void PostInit(void)
{
    /*
     ���C���������֘A
    */
    // (DTCM�̎�O�܂ł̗̈��S�N���A)
    //MI_CpuClearFast( (void*)HW_DELIVER_ARG_BUF_END, SDK_SECTION_ARENA_DTCM_START-HW_DELIVER_ARG_BUF_END );
    // (ARM9�̈��S�N���A)
    MI_CpuClearFast( (void*)HW_DELIVER_ARG_BUF_END, HW_MAIN_MEM_MAIN_END-HW_DELIVER_ARG_BUF_END );

    DC_FlushAll();
}

/***************************************************************
    CheckHeader

    �w�b�_���V�X�e�����j���[�Ƃ��Ė��Ȃ����`�F�b�N
    �擪32B�͌Œ�l�Ǝv��� (���}�X�^�[�o�[�W�����͈Ⴄ����)
***************************************************************/
static BOOL CheckHeader(void)
{
    // TODO
    return TRUE;
}

/***************************************************************
    EraseAll

    �s���I�����܂���
    ���낢������Ă�������
    DS���[�h�ɂ��ďI���̂��悢���H
***************************************************************/
static void EraseAll(void)
{
#ifdef SDK_FINALROM
    // TODO
#endif
}

void TwlMain( void )
{
    PreInit();

#ifdef PROFILE_ENABLE
    // 0: before PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_InitFIRM();
#ifdef PROFILE_ENABLE
    OS_InitTick();
    // 1: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    SVC_InitSignHeap( &acPool, acHeap, sizeof(acHeap) );

    PostInit();

    // load menu
    if ( MI_LoadHeader( &acPool, RSA_KEY_ADDR ) && CheckHeader() && MI_LoadStatic() )
    {
#ifdef PROFILE_ENABLE
        // 127: before Boot
        pf_cnt = PRFILE_MAX-1;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

        MI_Boot();
    }

    EraseAll();

    // failed
    while (1)
    {
        PXI_NotifyID( FIRM_PXI_ID_NULL );
    }
}

