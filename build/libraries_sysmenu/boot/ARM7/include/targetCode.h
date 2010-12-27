/*---------------------------------------------------------------------------*
  Project:  TwlIPL - HOTSW - include
  File:     romSpec.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: $
  $Rev: $
  $Author: $
 *---------------------------------------------------------------------------*/
#ifndef __BOOT_TARGET_CODE_H__
#define __BOOT_TARGET_CODE_H__

#include <twl.h>

#define MAJIKON_APP_ARM7_STATIC_BUFFER        0x02380000
#define MAJIKON_APP_ARM7_STATIC_BUFFER_SIZE   0x40000
#define MAJIKON_PATCH_ADDR                    0x02fff800

#define MAJIKON_APP_TARGET_COMMAND            0xE12FFF1E    // [bx  lr] ����

#define TARGET_CODE_NUM                       4
#define TARGET_CODE_MAX_SIZE                  0x100


#ifdef __cplusplus
extern "C" {
#endif

u32 target_code_list[TARGET_CODE_NUM][TARGET_CODE_MAX_SIZE] = 
{
	{
	    0xE92D40F8, 0xE59F00E4, 0xE5900004, 0xE3500000,
	    0x1A000034, 0xEBFFD80C, 0xE3500001, 0x1A000031,
	    0xE59F10CC, 0xE3E00000, 0xE5913004, 0xE1530000,
	    0x059F00C0, 0x05900000, 0x0280000A, 0x05810004,
	    0x0A000028, 0xE59F20AC, 0xE5920000, 0xE1500003,
	    0x3A000024, 0xE5920000, 0xE280000A, 0xE5810004,
	    0xEBFFFFA6, 0xE3500000, 0x0A00000A, 0xE59F007C,
	    0xE3A01001, 0xE5801004, 0xEBFFFBBE, 0xE590000C,
	    0xE3500000, 0x1A000003, 0xE59F0064, 0xE5900000,
	    0xE3500000, 0x1A000013, 0xE59F0050, 0xE59F1050,
	    0xE5900004, 0xE3A02000, 0xE5812000, 0xE3500000,
	    0x0A00000C, 0xE3A07064, 0xE3A0600E, 0xE3A05011,
	    0xE1A04002, 0xEA000001, 0xE1A00007, 0xEBFFCB91,
	    0xE1A00006, 0xE1A01005, 0xE1A02004, 0xEBFFDAA2,
	    0xE3500000, 0x1AFFFFF7, 0xE8BD40F8, 0x00000001,
	},
    {
	    0xE92D40F8, 0xE59F00E4, 0xE5900004, 0xE3500000,
	    0x1A000034, 0xEBFFD80C, 0xE3500001, 0x1A000031,
	    0xE59F10CC, 0xE3E00000, 0xE5913004, 0xE1530000,
	    0x059F00C0, 0x05900000, 0x0280000A, 0x05810004,
	    0x0A000028, 0xE59F20AC, 0xE5920000, 0xE1500003,
	    0x3A000024, 0xE5920000, 0xE280000A, 0xE5810004,
	    0xEBFFFFA6, 0xE3500000, 0x0A00000A, 0xE59F007C,
	    0xE3A01001, 0xE5801004, 0xEBFFFBBE, 0xE590000C,
	    0xE3500000, 0x1A000003, 0xE59F0064, 0xE5900000,
	    0xE3500000, 0x1A000013, 0xE59F0050, 0xE59F1050,
	    0xE5900004, 0xE3A02000, 0xE5812000, 0xE3500000,
	    0x0A00000C, 0xE3A07064, 0xE3A0600E, 0xE3A05011,
	    0xE1A04002, 0xEA000001, 0xE1A00007, 0xEBFFCB91,
	    0xE1A00006, 0xE1A01005, 0xE1A02004, 0xEBFFDAA2,
	    0xE3500000, /*0x1AFFFFF7,*/ 0xE8BD40F8, 0xE12FFF1E,
        0x00000001,
	},
	{
	    0xE92D40F8, 0xE59F00E4, 0xE5900004, 0xE3500000,
	    0x1A000034, 0xEBFFD80C, 0xE3500001, 0x1A000031,
	    0xE59F10CC, 0xE3E00000, 0xE5913004, 0xE1530000,
	    0x059F00C0, 0x05900000, 0x0280000A, 0x05810004,
	    0x0A000028, 0xE59F20AC, 0xE5920000, 0xE1500003,
	    0x3A000024, 0xE5920000, 0xE280000A, 0xE5810004,
	    0xEBFFFFA6, 0xE3500000, 0x0A00000A, 0xE59F007C,
	    0xE3A01001, 0xE5801004, 0xEBFFFBBE, 0xE590000C,
	    0xE3500000, 0x1A000003, 0xE59F0064, 0xE5900000,
	    0xE3500000, 0x1A000013, 0xE59F0050, 0xE59F1050,
	    0xE5900004, 0xE3A02000, 0xE5812000, 0xE3500000,
	    0x0A00000C, 0xE3A07064, 0xE3A0600E, 0xE3A05011,
	    0xE1A04002, 0xEA000001, 0xE1A00007, 0xEBFFCB91,
	    0xE1A00006, 0xE1A01005, 0xE1A02004, 0xEBFFDAA2,
	    0xE3500000, 0x1AFFFFF7, 0xE8BD40F8, 0xE12FFF1E,
	    0x03803F60, 0x037E9D34,
	},
    {
	    0xE92D40F8, 0xE59F00E4, 0xE5900004, 0xE3500000,
	    0x1A000034, 0xEBFFD80C, 0xE3500001, 0x1A000031,
	    0xE59F10CC, 0xE3E00000, 0xE5913004, 0xE1530000,
	    0x059F00C0, 0x05900000, 0x0280000A, 0x05810004,
	    0x0A000028, 0xE59F20AC, 0xE5920000, 0xE1500003,
	    0x3A000024, 0xE5920000, 0xE280000A, 0xE5810004,
	    0xEBFFFFA6, 0xE3500000, 0x0A00000A, 0xE59F007C,
	    0xE3A01001, 0xE5801004, 0xEBFFFBBE, 0xE590000C,
	    0xE3500000, 0x1A000003, 0xE59F0064, 0xE5900000,
	    0xE3500000, 0x1A000013, 0xE59F0050, 0xE59F1050,
	    0xE5900004, 0xE3A02000, 0xE5812000, 0xE3500000,
	    0x0A00000C, 0xE3A07064, 0xE3A0600E, 0xE3A05011,
	    0xE1A04002, 0xEA000001, 0xE1A00007, 0xEBFFCB91,
	    0xE1A00006, 0xE1A01005, 0xE1A02004, 0xEBFFDAA2,
	    0xE3500000, 0x1AFFFFF7, 0xE8BD40F8, 0xE12FFF1E,
	    0x03803F60, 0x00000001,
	},
};

#ifdef __cplusplus
} /* extern "C" */

#endif

/* __BOOT_TARGET_CODE_H__ */
#endif
