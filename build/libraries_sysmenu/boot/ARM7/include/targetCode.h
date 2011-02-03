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

// 下記を宣言するとカード抜けチェック関数を検出する
#define MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC

#define MAJIKON_APP_ARM7_STATIC_BUFFER        0x02380000
#define MAJIKON_APP_ARM7_STATIC_BUFFER_SIZE   0x40000
#define MAJIKON_PATCH_ADDR                    0x02fff000

#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
// カード抜け関数チェック用
#define MAJIKON_APP_TARGET_COMMAND_ARM        0xE12FFF1E    // [bx  lr] 命令
#define MAJIKON_APP_TARGET_COMMAND_THUMB      0x4718        // [bx  r3] 命令
#define TARGET_ARM_CODE_MAX_SIZE              0x40
#define TARGET_THUMB_CODE_MAX_SIZE            0x80
#define TARGET_ARM_CODE_NUM                   7
#define TARGET_THUMB_CODE_NUM                 1
#else
// _start関数チェック用
#define MAJIKON_APP_TARGET_COMMAND_ARM        0xE12FFF11    // [bx  r1] 命令
#define TARGET_ARM_CODE_MAX_SIZE              0x48
#define TARGET_ARM_CODE_NUM                   6
#endif

#ifdef __cplusplus
extern "C" {
#endif


// ↓ パッチコードにジャンプするコード。処理が戻ってこなくていいのでPCの退避は行わない
u32 patch_jump_arm[] =
{
    0xE51FF004, // ldr     pc, [pc, #-4]
    0x023FF000  // dcd     0x023ff000;
};

#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
// 注：dcdで定義される値は4バイトアライメントがとれているところに置かないとデータがずれる
u16 patch_jump_thumb[] =
{  
    0x4801,         // ldr     r0, [pc, #4] ※#2が指定できないのでnopを入れて調整する
    0x4700,         // bx      r0
    0x46C0,         // nop     nop
    0xF000, 0x023F  // dcd     MAJIKON_PATCH_ADDR;
};
#endif

// ↓ MCU_SetCameraLedStatus( MCU_CAMERA_LED_ON ); 相当の処理 (size 0x15c)
const u32 patch_core_arm[] =
{
	// カメラLED点灯
	0xE3A00000, 0xEA00004B, 0xE59F3140, 0xE5D31000,
	0xE3110080, 0x1AFFFFFC, 0xE59F2134, 0xE3A0104A,
	0xE5C21000, 0xE3A010C2, 0xE5C31000, 0xE5D21001,
	0xE3110080, 0x1AFFFFFC, 0xE5D21001, 0xE2011010,
	0xE1B01241, 0x0A00003A, 0xE59F2100, 0xE5D21000,
	0xE3110080, 0x1AFFFFFC, 0xE59F20F0, 0xE3A03000,
	0xEA000000, 0xE5D21000, 0xE3530E15, 0xE2833001,
	0xBAFFFFFB, 0xE59F20D8, 0xE3A01031, 0xE5C21000,
	0xE3A010C0, 0xE5C21001, 0xE5D21001, 0xE3110080,
	0x1AFFFFFC, 0xE5D21001, 0xE2011010, 0xE1B01241,
	0x0A000023, 0xE59F20A4, 0xE5D21000, 0xE3110080,
	0x1AFFFFFC, 0xE59F2094, 0xE3A03000, 0xEA000000,
	0xE5D21000, 0xE3530E15, 0xE2833001, 0xBAFFFFFB,
	0xE59F207C, 0xE3A01001, 0xE5C21000, 0xE3A010C0,
	0xE5C21001, 0xE5D21001, 0xE3110080, 0x1AFFFFFC,
	0xE59F2058, 0xE3A03000, 0xEA000000, 0xE5D21000,
	0xE3530E15, 0xE2833001, 0xBAFFFFFB, 0xE59F203C,
	0xE3A010C5, 0xE5C21000, 0xE5D21000, 0xE3110080,
	0x1AFFFFFC, 0xE5D21000, 0xE2011010, 0xE1B01241,
	0x1A000002, 0xE2800001, 0xE3500008, 0xBAFFFFB1,
	0xE59F0010, 0xE3A01000, 0xE1C010B0, 0xEAFFFFFE,
	0x04004501, 0x04004500, 0x04000208,
/*
	// 電源LEDが赤になる
	0xE3A00000, 0xEA00004B, 0xE59F3140, 0xE5D31000,
	0xE3110080, 0x1AFFFFFC, 0xE59F2134, 0xE3A0104A,
	0xE5C21000, 0xE3A010C2, 0xE5C31000, 0xE5D21001,
	0xE3110080, 0x1AFFFFFC, 0xE5D21001, 0xE2011010,
	0xE1B01241, 0x0A00003A, 0xE59F2100, 0xE5D21000,
	0xE3110080, 0x1AFFFFFC, 0xE59F20F0, 0xE3A03000,
	0xEA000000, 0xE5D21000, 0xE3530E15, 0xE2833001,
	0xBAFFFFFB, 0xE59F20D8, 0xE3A01063, 0xE5C21000,
	0xE3A010C0, 0xE5C21001, 0xE5D21001, 0xE3110080,
	0x1AFFFFFC, 0xE5D21001, 0xE2011010, 0xE1B01241,
	0x0A000023, 0xE59F20A4, 0xE5D21000, 0xE3110080,
	0x1AFFFFFC, 0xE59F2094, 0xE3A03000, 0xEA000000,
	0xE5D21000, 0xE3530E15, 0xE2833001, 0xBAFFFFFB,
	0xE59F207C, 0xE3A01001, 0xE5C21000, 0xE3A010C0,
	0xE5C21001, 0xE5D21001, 0xE3110080, 0x1AFFFFFC,
	0xE59F2058, 0xE3A03000, 0xEA000000, 0xE5D21000,
	0xE3530E15, 0xE2833001, 0xBAFFFFFB, 0xE59F203C,
	0xE3A010C5, 0xE5C21000, 0xE5D21000, 0xE3110080,
	0x1AFFFFFC, 0xE5D21000, 0xE2011010, 0xE1B01241,
	0x1A000002, 0xE2800001, 0xE3500008, 0xBAFFFFB1,
	0xE59F0010, 0xE3A01000, 0xE1C010B0, 0xEAFFFFFE,
	0x04004501, 0x04004500, 0x04000208,
*/
};


u32 target_code_list_arm[TARGET_ARM_CODE_NUM][TARGET_ARM_CODE_MAX_SIZE] = 
{
#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
    // カード抜けチェック関数を検出する
	{
		0xe92d4000, 0xe24dd004, 0xe59f00b4, 0xe5900000,
		0xe3500000, 0x1a000027, 0xe59f00a8, 0xe1d000b0,
		0xe3500002, 0x0a000023, 0xe59f109c, 0xe5913000,
		0xe3e00000, 0xe1530000, 0x059f0090, 0x05900000,
		0x0280000a, 0x05810000, 0x0a00001a, 0xe59f207c,
		0xe5920000, 0xe1500003, 0x3a000016, 0xe5920000,
		0xe280000a, 0xe5810000, 0xeb00003f, 0xe3500000,
		0x0a000006, 0xe3a01001, 0xe59f0044, 0xe5801000,
		0xe59f004c, 0xe5900000, 0xe3500000, 0x1a000009,
		0xe3a01000, 0xe59f0038, 0xe5801000, 0xe59f0020,
		0xe5900000, 0xe3500000, 0x0a000002, 0xe3a00011,
		0xe3a01064, 0xebffffb3, 0xe28dd004, 0xe8bd4000,
		0xe12fff1e, 0x03808460, 0x027ffc40, 0x038061c8,
		0x027ffc3c, 0x038061cc,
	},
	{
		0xe92d4000, 0xe24dd004, 0xe59f00b4, 0xe5900000,
		0xe3500000, 0x1a000027, 0xe59f00a8, 0xe1d000b0,
		0xe3500002, 0x0a000023, 0xe59f109c, 0xe5913000,
		0xe3e00000, 0xe1530000, 0x059f0090, 0x05900000,
		0x0280000a, 0x05810000, 0x0a00001a, 0xe59f207c,
		0xe5920000, 0xe1500003, 0x3a000016, 0xe5920000,
		0xe280000a, 0xe5810000, 0xeb00003f, 0xe3500000,
		0x0a000006, 0xe3a01001, 0xe59f0044, 0xe5801000,
		0xe59f004c, 0xe5900000, 0xe3500000, 0x1a000009,
		0xe3a01000, 0xe59f0038, 0xe5801000, 0xe59f0020,
		0xe5900000, 0xe3500000, 0x0a000002, 0xe3a00011,
		0xe3a01064, 0xebffffb7, 0xe28dd004, 0xe8bd4000,
		0xe12fff1e, 0x03809f20, 0x027ffc40, 0x03807c78,
		0x027ffc3c, 0x03807c7c,
	},
	{
		0xe92d4000, 0xe24dd004, 0xe59f00b4, 0xe5900000,
		0xe3500000, 0x1a000027, 0xe59f00a8, 0xe1d000b0,
		0xe3500002, 0x0a000023, 0xe59f109c, 0xe5913000,
		0xe3e00000, 0xe1530000, 0x059f0090, 0x05900000,
		0x0280000a, 0x05810000, 0x0a00001a, 0xe59f207c,
		0xe5920000, 0xe1500003, 0x3a000016, 0xe5920000,
		0xe280000a, 0xe5810000, 0xeb00003f, 0xe3500000,
		0x0a000006, 0xe3a01001, 0xe59f0044, 0xe5801000,
		0xe59f004c, 0xe5900000, 0xe3500000, 0x1a000009,
		0xe3a01000, 0xe59f0038, 0xe5801000, 0xe59f0020,
		0xe5900000, 0xe3500000, 0x0a000002, 0xe3a00011,
		0xe3a01064, 0xebffffb7, 0xe28dd004, 0xe8bd4000,
		0xe12fff1e, 0x03809f60, 0x027ffc40, 0x03807cc0,
		0x027ffc3c, 0x03807cc4,
	},
	{
		0xe92d4000, 0xe24dd004, 0xe59f00b4, 0xe5900000,
		0xe3500000, 0x1a000027, 0xe59f00a8, 0xe1d000b0,
		0xe3500002, 0x0a000023, 0xe59f109c, 0xe5913000,
		0xe3e00000, 0xe1530000, 0x059f0090, 0x05900000,
		0x0280000a, 0x05810000, 0x0a00001a, 0xe59f207c,
		0xe5920000, 0xe1500003, 0x3a000016, 0xe5920000,
		0xe280000a, 0xe5810000, 0xeb00003f, 0xe3500000,
		0x0a000006, 0xe3a01001, 0xe59f0044, 0xe5801000,
		0xe59f004c, 0xe5900000, 0xe3500000, 0x1a000009,
		0xe3a01000, 0xe59f0038, 0xe5801000, 0xe59f0020,
		0xe5900000, 0xe3500000, 0x0a000002, 0xe3a00011,
		0xe3a01064, 0xebffffb5, 0xe28dd004, 0xe8bd4000,
		0xe12fff1e, 0x0380a4c0, 0x027ffc40, 0x03808190,
		0x027ffc3c, 0x03808194,
	},
	{
		0xe92d4000, 0xe24dd004, 0xe59f00b4, 0xe5900000,
		0xe3500000, 0x1a000027, 0xe59f00a8, 0xe1d000b0,
		0xe3500002, 0x0a000023, 0xe59f109c, 0xe5913000,
		0xe3e00000, 0xe1530000, 0x059f0090, 0x05900000,
		0x0280000a, 0x05810000, 0x0a00001a, 0xe59f207c,
		0xe5920000, 0xe1500003, 0x3a000016, 0xe5920000,
		0xe280000a, 0xe5810000, 0xeb00003f, 0xe3500000,
		0x0a000006, 0xe3a01001, 0xe59f0044, 0xe5801000,
		0xe59f004c, 0xe5900000, 0xe3500000, 0x1a000009,
		0xe3a01000, 0xe59f0038, 0xe5801000, 0xe59f0020,
		0xe5900000, 0xe3500000, 0x0a000002, 0xe3a00011,
		0xe3a01064, 0xebffffb5, 0xe28dd004, 0xe8bd4000,
		0xe12fff1e, 0x0380a4c0, 0x027ffc40, 0x03808194,
		0x027ffc3c, 0x03808198,
	},
	{
		0xe92d4000, 0xe24dd004, 0xe59f00b4, 0xe5900000,
		0xe3500000, 0x1a000027, 0xe59f00a8, 0xe1d000b0,
		0xe3500002, 0x0a000023, 0xe59f109c, 0xe5913000,
		0xe3e00000, 0xe1530000, 0x059f0090, 0x05900000,
		0x0280000a, 0x05810000, 0x0a00001a, 0xe59f207c,
		0xe5920000, 0xe1500003, 0x3a000016, 0xe5920000,
		0xe280000a, 0xe5810000, 0xeb00003f, 0xe3500000,
		0x0a000006, 0xe3a01001, 0xe59f0044, 0xe5801000,
		0xe59f004c, 0xe5900000, 0xe3500000, 0x1a000009,
		0xe3a01000, 0xe59f0038, 0xe5801000, 0xe59f0020,
		0xe5900000, 0xe3500000, 0x0a000002, 0xe3a00011,
		0xe3a01064, 0xebffffb5, 0xe28dd004, 0xe8bd4000,
		0xe12fff1e, 0x0380a4e0, 0x027ffc40, 0x038081a4,
		0x027ffc3c, 0x038081a8,
	},
	{
		0xe92d40f8, 0xe59f00e0, 0xe5900004, 0xe3500000,
		0x1a000033, 0xe59f20d4, 0xe1d200b0, 0xe3500002,
		0x0a00002f, 0xe59f10c8, 0xe3e00000, 0xe5913000,
		0xe1530000, 0x05120004, 0x0280000a, 0x05810000,
		0x0a000027, 0xe5320004, 0xe1500003, 0x3a000024,
		0xe5920000, 0xe280000a, 0xe5810000, 0xebffffb4,
		0xe3500000, 0x0a00000a, 0xe59f007c, 0xe3a01001,
		0xe5801004, 0xebfffc36, 0xe590000c, 0xe3500000,
		0x1a000003, 0xe59f0068, 0xe5900004, 0xe3500000,
		0x1a000013, 0xe59f0050, 0xe59f1054, 0xe5900004,
		0xe3a02000, 0xe5812004, 0xe3500000, 0x0a00000c,
		0xe3a07064, 0xe3a0600e, 0xe3a05011, 0xe1a04002,
		0xea000001, 0xe1a00007, 0xebffe12b, 0xe1a00006,
		0xe1a01005, 0xe1a02004, 0xebffeb59, 0xe3500000,
		0x1afffff7, 0xe8bd40f8, 0xe12fff1e, 0x03809420,
		0x027ffc40, 0x038070b4,
	},
/*
      // デバッグ用
    {
        0xE92D40F8, 0xE59F00E4, 0xE5900004, 0xE3500000,
        0x1A000034, 0xEBFFDCE4, 0xE3500001, 0x1A000031,
		0xE59F10CC, 0xE3E00000, 0xE5913004, 0xE1530000,
		0x059F00C0, 0x05900000, 0x0280000A, 0x05810004,
		0x0A000028, 0xE59F20AC, 0xE5920000, 0xE1500003,
		0x3A000024, 0xE5920000, 0xE280000A, 0xE5810004,
		0xEBFFFFA6, 0xE3500000, 0x0A00000A, 0xE59F007C,
		0xE3A01001, 0xE5801004, 0xEBFFFBD3, 0xE590000C,
		0xE3500000, 0x1A000003, 0xE59F0064, 0xE5900000,
		0xE3500000, 0x1A000013, 0xE59F0050, 0xE59F1050,
		0xE5900004, 0xE3A02000, 0xE5812000, 0xE3500000,
		0x0A00000C, 0xE3A07064, 0xE3A0600E, 0xE3A05011,
		0xE1A04002, 0xEA000001, 0xE1A00007, 0xEBFFD0B6,
		0xE1A00006, 0xE1A01005, 0xE1A02004, 0xEBFFDE56,
		0xE3500000, 0x1AFFFFF7, 0xE8BD40F8, 0xE12FFF1E,
		0x0380C6A0, 0x03809EA8, 0x02FFFC3C
    },
*/
#else
    
    // crt0.cの_start関数を検出する
    // ARM7CompoPack-2_0rc4p3/t/NitroSDK/components/ferret/ARM7-TS/Rom/ferret_sub.sbin
	{
		0xe3a0c301, 0xe58cc208, 0xe59f1088, 0xe3a0050e,
		0xe1500001, 0x51a01000, 0xe59f207c, 0xe3a00000,
		0xe1510002, 0xb8a10001, 0xbafffffc, 0xe3a00013,
		0xe121f000, 0xe59fd064, 0xe3a00012, 0xe121f000,
		0xe59f005c, 0xe1a0d000, 0xe59f1058, 0xe0401001,
		0xe3a0001f, 0xe12ff000, 0xe241d004, 0xeb000017,
		0xe59f0044, 0xe590100c, 0xe5902010, 0xe3a00000,
		0xe1510002, 0x34810004, 0x3afffffc, 0xeb000026,
		0xe59f1028, 0xe59f0028, 0xe5810000, 0xe59f1024,
		0xe59fe024, 0xe12fff11, 0x02380170, 0x0380ff00,
		0x0380ffc0, 0x0380ff80, 0x00000200, 0x02380158,
		0x0380fffc, 0x037f8374, 0x037f82b8, 0xffff0000,
	},
	// ARM7CompoPack-2_2p2/t/NitroSDK/components/mongoose/ARM7-TS/Rom/mongoose_sub.sbin
	{
		0xe3a0c301, 0xe58cc208, 0xe59f10bc, 0xe3a0050e,
		0xe1500001, 0x51a01000, 0xe59f20b0, 0xe3a00000,
		0xe1510002, 0xb8a10001, 0xbafffffc, 0xe3a00013,
		0xe121f000, 0xe59fd098, 0xe3a00012, 0xe121f000,
		0xe59f0090, 0xe1a0d000, 0xe59f108c, 0xe0401001,
		0xe3a0001f, 0xe12ff000, 0xe241d004, 0xe59f007c,
		0xe59f107c, 0xe2812e16, 0xe4903004, 0xe4813004,
		0xe1510002, 0x4afffffb, 0xe59f0068, 0xe2812020,
		0xe4903004, 0xe4813004, 0xe1510002, 0x4afffffb,
		0xeb00001a, 0xe59f0050, 0xe590100c, 0xe5902010,
		0xe3a00000, 0xe1510002, 0x34810004, 0x3afffffc,
		0xeb000029, 0xe59f1034, 0xe59f0034, 0xe5810000,
		0xe59f1030, 0xe59fe030, 0xe12fff11, 0x023801b0,
		0x0380ff00, 0x0380ffc0, 0x0380ff80, 0x00000400,
		0x023fe940, 0x027ffa80, 0x023fe904, 0x02380198,
		0x0380fffc, 0x037fb6c0, 0x037f846c, 0xffff0000,
	},
	// ARM7CompoPack-2_2p3/t/NitroSDK/components/mongoose/ARM7-TS/Rom/mongoose_sub.sbin
	{
		0xe3a0c301, 0xe58cc208, 0xe59f10bc, 0xe3a0050e,
		0xe1500001, 0x51a01000, 0xe59f20b0, 0xe3a00000,
		0xe1510002, 0xb8a10001, 0xbafffffc, 0xe3a00013,
		0xe121f000, 0xe59fd098, 0xe3a00012, 0xe121f000,
		0xe59f0090, 0xe1a0d000, 0xe59f108c, 0xe0401001,
		0xe3a0001f, 0xe12ff000, 0xe241d004, 0xe59f007c,
		0xe59f107c, 0xe2812e16, 0xe4903004, 0xe4813004,
		0xe1510002, 0x4afffffb, 0xe59f0068, 0xe2812020,
		0xe4903004, 0xe4813004, 0xe1510002, 0x4afffffb,
		0xeb00001a, 0xe59f0050, 0xe590100c, 0xe5902010,
		0xe3a00000, 0xe1510002, 0x34810004, 0x3afffffc,
		0xeb000029, 0xe59f1034, 0xe59f0034, 0xe5810000,
		0xe59f1030, 0xe59fe030, 0xe12fff11, 0x023801b0,
		0x0380ff00, 0x0380ffc0, 0x0380ff80, 0x00000400,
		0x023fe940, 0x027ffa80, 0x023fe904, 0x02380198,
		0x0380fffc, 0x037fb708, 0x037f846c, 0xffff0000,
	},
	// ARM7CompoPack-3_0p1/t/NitroSDK/components/mongoose/ARM7-TS/Rom/mongoose_sub.sbin
	// ARM7CompoPack-3_1/t/NitroSDK/components/mongoose/ARM7-TS/Rom/mongoose_sub.sbin
	// ARM7CompoPack-3_1p4/t/NitroSDK/components/mongoose/ARM7-TS/Rom/mongoose_sub.sbin
	{
		0xe3a0c301, 0xe58cc208, 0xe59f10bc, 0xe3a0050e,
		0xe1500001, 0x51a01000, 0xe59f20b0, 0xe3a00000,
		0xe1510002, 0xb8a10001, 0xbafffffc, 0xe3a00013,
		0xe121f000, 0xe59fd098, 0xe3a00012, 0xe121f000,
		0xe59f0090, 0xe1a0d000, 0xe59f108c, 0xe0401001,
		0xe3a0001f, 0xe12ff000, 0xe241d004, 0xe59f007c,
		0xe59f107c, 0xe2812e16, 0xe4903004, 0xe4813004,
		0xe1510002, 0x4afffffb, 0xe59f0068, 0xe2812020,
		0xe4903004, 0xe4813004, 0xe1510002, 0x4afffffb,
		0xeb00001a, 0xe59f0050, 0xe590100c, 0xe5902010,
		0xe3a00000, 0xe1510002, 0x34810004, 0x3afffffc,
		0xeb000029, 0xe59f1034, 0xe59f0034, 0xe5810000,
		0xe59f1030, 0xe59fe030, 0xe12fff11, 0x023801b0,
		0x0380ff00, 0x0380ffc0, 0x0380ff80, 0x00000400,
		0x023fe940, 0x027ffa80, 0x023fe904, 0x02380198,
		0x0380fffc, 0x037fb8f4, 0x037f8468, 0xffff0000,
	},
	// ARM7CompoPack-4_0p1/t/NitroSDK/components/mongoose/ARM7-TS/Rom/mongoose_sub.sbin
	{
		0xe3a0c301, 0xe58cc208, 0xe59f10bc, 0xe3a0050e,
		0xe1500001, 0x51a01000, 0xe59f20b0, 0xe3a00000,
		0xe1510002, 0xb8a10001, 0xbafffffc, 0xe3a00013,
		0xe121f000, 0xe59fd098, 0xe3a00012, 0xe121f000,
		0xe59f0090, 0xe1a0d000, 0xe59f108c, 0xe0401001,
		0xe3a0001f, 0xe12ff000, 0xe241d004, 0xe59f007c,
		0xe59f107c, 0xe2812e16, 0xe4903004, 0xe4813004,
		0xe1510002, 0x4afffffb, 0xe59f0068, 0xe2812020,
		0xe4903004, 0xe4813004, 0xe1510002, 0x4afffffb,
		0xeb00001a, 0xe59f0050, 0xe590100c, 0xe5902010,
		0xe3a00000, 0xe1510002, 0x34810004, 0x3afffffc,
		0xeb000029, 0xe59f1034, 0xe59f0034, 0xe5810000,
		0xe59f1030, 0xe59fe030, 0xe12fff11, 0x023801b0,
		0x0380ff00, 0x0380ffc0, 0x0380ff80, 0x00000400,
		0x023fe940, 0x027ffa80, 0x023fe904, 0x02380198,
		0x0380fffc, 0x037fb488, 0x037f8000, 0xffff0000,
	},
	// ARM7CompoPack-4_2p1/t/NitroSDK/components/mongoose/ARM7-TS.thumb/Rom/mongoose_sub.sbin
	{
		0xe3a0c301, 0xe58cc208, 0xe59f10d4, 0xe3a0050e,
		0xe1500001, 0x5a000000, 0xea000000, 0xe1a01000,
		0xe59f20c0, 0xe3a00000, 0xe1510002, 0xba000000,
		0xea000000, 0xe8a10001, 0xbafffffa, 0xe3a00013,
		0xe121f000, 0xe59fd0a0, 0xe3a00012, 0xe121f000,
		0xe59f0098, 0xe1a0d000, 0xe59f1094, 0xe0401001,
		0xe3a0001f, 0xe12ff000, 0xe241d004, 0xe59f0084,
		0xe59f1084, 0xe2812e16, 0xe4903004, 0xe4813004,
		0xe1510002, 0x4afffffb, 0xe59f0070, 0xe2812020,
		0xe4903004, 0xe4813004, 0xe1510002, 0x4afffffb,
		0xeb00001c, 0xe59f0058, 0xe590100c, 0xe5902010,
		0xe3a00000, 0xe1510002, 0x3a000000, 0xea000000,
		0xe4810004, 0x3afffffa, 0xeb00002f, 0xe59f1034,
		0xe59f0034, 0xe5810000, 0xe59f1030, 0xe59fe030,
		0xe12fff11, 0x023801e8, 0x0380ff00, 0x0380ffc0,
		0x0380ff80, 0x00000400, 0x023fe940, 0x027ffa80,
		0x023fe904, 0x023801d0, 0x0380fffc, 0x037fb418,
		0x037f8001, 0xffff0000,
	},
#endif
};


#ifdef MAJIKON_APP_CHECK_BY_CARD_PULLOUT_FUNC
u16 target_code_list_thumb[TARGET_THUMB_CODE_NUM][TARGET_THUMB_CODE_MAX_SIZE] =
{
    {
        0xb5f8, 0x4821, 0x6840, 0x2800, 0xd13b, 0x4a20, 0x8810, 0x2802,
        0xd037, 0x491f, 0x2000, 0x680b, 0x43c0, 0x4283, 0xd104, 0x1f10,
        0x6800, 0x300a, 0x6008, 0xe02c, 0x1f12, 0x6810, 0x4298, 0xd328,
        0x6810, 0x300a, 0x6008, 0xf7ff, 0xff6b, 0x2800, 0xd00b, 0x4812,
        0x2101, 0x6041, 0xf7ff, 0xfa56, 0x68c0, 0x2800, 0xd103, 0x4810,
        0x6840, 0x2800, 0xd115, 0x480e, 0x2100, 0x6041, 0x480a, 0x6840,
        0x2800, 0xd00e, 0x2564, 0x240e, 0x2611, 0x1c0f, 0xe002, 0x1c28,
        0xf000, 0xf812, 0x1c20, 0x1c31, 0x1c3a, 0xf7fc, 0xfadf, 0x2800,
        0xd1f5, 0xbcf8, 0xbc08, 0x4718, 0x6fe0, 0x0380, 0xfc40, 0x027f,
        0x4c94, 0x0380,
    }
};
#endif

#ifdef __cplusplus
} /* extern "C" */

#endif

/* __BOOT_TARGET_CODE_H__ */
#endif
