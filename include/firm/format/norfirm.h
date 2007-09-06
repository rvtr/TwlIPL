/*---------------------------------------------------------------------------*
  Project:  TwlFirm - format - norfirm
  File:     norfirm.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
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

// 常駐モジュール情報
typedef FIRMHeader_ModuleInfo  NORHeader_ModuleInfo;

// TWL-NORファームヘッダ
typedef struct
{
    /* 0x000-0x020 [システム予約領域] */
    u8      reserved_0h[0x20];         /* システム予約 A */

    /* 0x020-0x040 [常駐モジュール用パラメータ] */
    u32     main_rom_offset;           /* ARM9 転送元 ROM オフセット */
    u32     main_decomp_size;          /* ARM9 展開サイズ */
    void   *main_ram_address;          /* ARM9 転送先 RAM オフセット */
    u32     main_size;                 /* ARM9 転送サイズ */
    u32     sub_rom_offset;            /* ARM7 転送元 ROM オフセット */
    u32     sub_decomp_size;           /* ARM9 展開サイズ */
    void   *sub_ram_address;           /* ARM7 転送先 RAM オフセット */
    u32     sub_size;                  /* ARM7 転送サイズ */

    /* 0x040-0x0c0 [システム予約領域] */
    u8      reserved_40h[0x80];

    /* 0x0c0-0x100 [DSカードNINTENDOロゴ重複領域] */
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
    /* 0x180-0x1b0 [WRAMレジスタパラメータ] */
    MIHeader_WramRegs w;

    /* 0x1b0-0x200 [システム予約領域] */
    u8      reserved_footer[0x50];
}
NORHeaderHigh;

// NORヘッダ
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
