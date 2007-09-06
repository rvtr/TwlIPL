/*---------------------------------------------------------------------------*
  Project:  TwlFirm - format - gcdfirm
  File:     gcdfirm.h

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
#ifndef	FIRM_FORMAT_GCDFIRM_H_
#define	FIRM_FORMAT_GCDFIRM_H_

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
typedef FIRMHeader_ModuleInfo  GCDHeader_ModuleInfo;

// TWL-GCD�t�@�[���w�b�_
typedef struct
{
    /* 0x000-0x020 [�V�X�e���\��̈�] */
    u8      reserved_0h[0x14];
    u8      rom_size;                  // Rom size (2��rom_size�� Mbit: ex. 128Mbit�̂Ƃ�rom_size = 7)
    u8      reserved_16h[0xb];

    /* 0x020-0x040 [�풓���W���[���p�p�����[�^] */
    u32     main_rom_offset;           /* ARM9 �]���� ROM �I�t�Z�b�g */
    u32     main_decomp_size;          /* ARM9 �W�J�T�C�Y */
    void   *main_ram_address;          /* ARM9 �]���� RAM �I�t�Z�b�g */
    u32     main_size;                 /* ARM9 �]���T�C�Y */
    u32     sub_rom_offset;            /* ARM7 �]���� ROM �I�t�Z�b�g */
    u32     sub_decomp_size;           /* ARM9 �W�J�T�C�Y */
    void   *sub_ram_address;           /* ARM7 �]���� RAM �I�t�Z�b�g */
    u32     sub_size;                  /* ARM7 �]���T�C�Y */

    /* 0x040-0x080 [�V�X�e���\��̈�] */
    u8      reserved_40h[0x40];

    /* 0x080-0x090 [�t�@�[���o�C�i�����] */
    u32     nandfirm_offset;           // address of rom_valid_size
    u32     nandfirm_size;             // address of rom_header_size
    u32     norfirm_offset;            // address of main_module_param
    u32     norfirm_size;              // address of sub_module_param

    /* 0x090-0x094 [TWL-ROM�R���g���[��] */
    u16     normal_area_offset;
    u16     twl_area_offset;

    /* 0x094-0x0c0 [�V�X�e���\��̈�] */
    u8      reserved_98h[0x2c];

    /* 0x0c0-0x100 [DS�J�[�hNINTENDO���S�d���̈�] */
    u8      reserved_C0h[0x3f];

    u8      comp_arm9_boot_area:1;     // Compress arm9 boot area
    u8      comp_arm7_boot_area:1;     // Compress arm7 boot area
    u8      arm9_x2:1;
    u8      :0;
}
GCDHeaderLow;

typedef struct
{
    /* 0x180-0x1b0 [WRAM���W�X�^�p�����[�^] */
    MIHeader_WramRegs w;

    /* 0x1b0-0x200 [�V�X�e���\��̈�] */
    u8      reserved_footer[0x50];
}
GCDHeaderHigh;

// GCD�w�b�_
typedef struct
{
    /* 0x000-0x100 */
	GCDHeaderLow	l;

    /* 0x100-0x180 */
    FIRMPaddedSign sign;

    /* 0x180-0x200 */
    GCDHeaderHigh	h;
}
GCDHeader;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //FIRM_FORMAT_GCDFIRM_H_
