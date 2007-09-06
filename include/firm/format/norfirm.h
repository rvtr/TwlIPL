/*---------------------------------------------------------------------------*
  Project:  TwlFirm - format - norfirm
  File:     norfirm.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef	FIRM_FORMAT_NOR_H_
#define	FIRM_FORMAT_NOR_H_

#include <firm/format/firm_common.h>
#include <firm/format/sign.h>
#include <firm/format/wram_regs.h>


#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================*
 *  NOR FORMAT
 *===========================================================================*/

//---------------------------------------------------------------------------
//  Section A   NOR HEADER
//---------------------------------------------------------------------------

// �풓���W���[�����
typedef FIRMHeader_ModuleInfo  NORHeader_ModuleInfo;

// TWL-NOR�t�@�[���w�b�_
typedef struct
{
    /* 0x000-0x020 [�V�X�e���\��̈�] */
    u8      reserved_0h[0x20];         /* �V�X�e���\�� A */

    /* 0x020-0x040 [�풓���W���[���p�p�����[�^] */
    u32     main_rom_offset;           /* ARM9 �]���� ROM �I�t�Z�b�g */
    u32     main_decomp_size;          /* ARM9 �W�J�T�C�Y */
    void   *main_ram_address;          /* ARM9 �]���� RAM �I�t�Z�b�g */
    u32     main_size;                 /* ARM9 �]���T�C�Y */
    u32     sub_rom_offset;            /* ARM7 �]���� ROM �I�t�Z�b�g */
    u32     sub_decomp_size;           /* ARM9 �W�J�T�C�Y */
    void   *sub_ram_address;           /* ARM7 �]���� RAM �I�t�Z�b�g */
    u32     sub_size;                  /* ARM7 �]���T�C�Y */

    /* 0x040-0x0c0 [�V�X�e���\��̈�] */
    u8      reserved_40h[0x80];

    /* 0x0c0-0x100 [DS�J�[�hNINTENDO���S�d���̈�] */
    u8      reserved_C0h[0x3f];

    u8      comp_arm9_boot_area:1;     // Compress arm9 boot area
    u8      comp_arm7_boot_area:1;     // Compress arm7 boot area
    u8      arm9_x2:1;
    u8      arm9_decomp:1;
    u8      :2;
    u8      baudrate:1;
    u8      boot_nandfirm:1;
}
NORHeaderLow;

typedef struct
{
    /* 0x180-0x1b0 [WRAM���W�X�^�p�����[�^] */
    MIHeader_WramRegs w;

    /* 0x1b0-0x200 [�V�X�e���\��̈�] */
    u8      reserved_footer[0x50];
}
NORHeaderHigh;

// NOR�w�b�_
typedef struct
{
    /* 0x000-0x028 */
	NORHeaderDS		d;

    /* 0x028-0x200 */
	u8		wl_params[472];

    /* 0x200-0x300 */
	NORHeaderLow	l;

    /* 0x300-0x380 */
    FIRMPaddedSign sign;

    /* 0x380-0x400 */
    NORHeaderHigh	h;
}
NORHeader;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //FIRM_FORMAT_NOR_H_
