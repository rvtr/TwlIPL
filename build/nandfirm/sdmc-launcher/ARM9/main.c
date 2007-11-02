/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - sdmc-launcher
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

#define RSA_KEY_ADDR    rsa_key
static const u8 rsa_key[128] =
{
    0xdf, 0x56, 0x30,
    0xc9, 0xae, 0x05, 0x55, 0xe8, 0xdf, 0xbe, 0xe6, 0xb9, 0x30, 0xb9, 0x76, 0x93, 0xb4, 0xc2, 0x20,
    0xe7, 0xae, 0x4c, 0x3e, 0xc3, 0xed, 0x27, 0xcf, 0x5d, 0x4f, 0xb5, 0x7d, 0xde, 0x38, 0xbc, 0xfe,
    0x25, 0x32, 0xd8, 0x23, 0x98, 0x52, 0xb5, 0xda, 0xf7, 0x39, 0xdc, 0xb3, 0x0a, 0x94, 0x7a, 0x2b,
    0x79, 0xe6, 0xe0, 0x4c, 0xbc, 0x21, 0xbd, 0x59, 0xb2, 0xc7, 0xf1, 0xc0, 0xf1, 0xfb, 0x29, 0x75,
    0xa1, 0x21, 0x93, 0x01, 0x29, 0x1c, 0x9a, 0xe1, 0x2d, 0x55, 0xfc, 0x7b, 0xb8, 0xcb, 0x07, 0x33,
    0xc5, 0x91, 0x0d, 0xc8, 0x45, 0x59, 0xef, 0xbe, 0x58, 0xc7, 0xc1, 0x1d, 0xd5, 0xf2, 0xcf, 0x1f,
    0xe0, 0x6d, 0x21, 0x00, 0xcd, 0x42, 0xd8, 0x84, 0x85, 0xe3, 0xb2, 0x02, 0x1a, 0xa5, 0x89, 0x02,
    0xa1, 0x96, 0xc6, 0xf7, 0x61, 0x68, 0x66, 0xe6, 0x65, 0x12, 0xb7, 0xf1, 0x49
};

#define RSA_HEAP_SIZE   (4*1024)    // RSA�p�q�[�v�T�C�Y (�T�C�Y�����K�v)

static u8 acHeap[RSA_HEAP_SIZE] __attribute__ ((aligned (32)));
static SVCSignHeapContext acPool;

/*
    Profile
*/
#ifndef SDK_FINALROM
#define PROFILE_MAX  0x100
u32 profile[PROFILE_MAX];
u32 pf_cnt = 0;
#endif

/***************************************************************
    PreInit

    FromBoot�̑Ή����܂Ƃ߂違���C���������̏�����
    OS_Init�O�Ȃ̂Œ���
    MI_LoadHeader�O�ɂ��Ȃ�(��100msec)���Ԃ�����̂ŁA�\�Ȃ�
    OS_Init��ɂ��낢�돈���������I
***************************************************************/
static void PreInit(void)
{
    /*
     ���C���������֘A
    */

    // SHARED�̈�N���A (IS-TWL-DEBUGGER�̍X�V�҂�)
#ifdef SDK_FINALROM
    //MIi_CpuClearFast( 0, (void*)HW_MAIN_MEM_SHARED, HW_MAIN_MEM_SHARED_END-HW_MAIN_MEM_SHARED );
#endif
    // SHARED�̈�N���A (����������OK?)
    MIi_CpuClearFast( 0, (void*)HW_PXI_SIGNAL_PARAM_ARM9, HW_MAIN_MEM_SHARED_END-HW_PXI_SIGNAL_PARAM_ARM9);

    /*
        FromBrom�֘A
    */

    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );
}

/***************************************************************
    PostInit

    MI_LoadHeader�O�ɂ��Ȃ�(��100msec)���Ԃ�����̂ŁA�\�Ȃ�
    OS_Init��ɂ��낢�돈���������I
    ���C���������̏�����
***************************************************************/
static void PostInit(void)
{
    /*
     ���C���������֘A (ARM9�p�̗̈��S�N���A)
    */

    MIi_CpuClearFast( 0, (void*)HW_DELIVER_ARG_BUF_END, HW_TWL_MAIN_MEM_MAIN_SIZE-HW_DELIVER_ARG_BUF_SIZE );
    DC_FlushAll();
}

/***************************************************************
    CheckHeader

    �w�b�_���V�X�e�����j���[�Ƃ��Ė��Ȃ����`�F�b�N
    �擪32B�͌Œ�l�Ǝv��� (���}�X�^�[�o�[�W�����͈Ⴄ����)
***************************************************************/
static BOOL CheckHeader(void)
{
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

#ifndef SDK_FINALROM
    // 0: before PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSecondsBROM(OS_GetTick());
#endif

    OS_InitFIRM();
#ifndef SDK_FINALROM
    OS_EnableIrq();
    OS_InitTick();
    // 1: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    SVC_InitSignHeap( &acPool, acHeap, sizeof(acHeap) );

    PostInit();

#ifndef SDK_FINALROM
        // 2: after PostInit
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    // load menu
    if ( MI_LoadHeader( &acPool, RSA_KEY_ADDR ) && CheckHeader() && MI_LoadStatic() )
    {
#ifndef SDK_FINALROM
        // 127: before Boot
        pf_cnt = PROFILE_MAX-1;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        {
            int i;
            OS_TPrintf("\n[ARM9] Begin\n");
            for (i = 0; i < PROFILE_MAX; i++)
            {
                OS_TPrintf("0x%08X\n", profile[i]);
            }
            OS_TPrintf("\n[ARM9] End\n");
            PXI_NotifyID( FIRM_PXI_ID_NULL );
        }
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

