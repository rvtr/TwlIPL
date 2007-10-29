/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ninLogoFunc.c

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
#include <sysmenu/rom_header.h>
#include <sysmenu/sysmenu_lib/ARM9/sysmenu_api.h>

// define data-----------------------------------------------------------

// extern data-----------------------------------------------------------

// function's prototype--------------------------------------------------
static void UnCompNintendoLogo2(u16 *NintendoLogoDatap, u16 *dstp, u32 *temp);
static void SVC_DiffUnFilter16_16_2(u16 *srcp,u16 *dstp);
static s32  MEMBm_InitFunc(const u8 *devicep, void *ramp, const void *paramp);
static s32  MEMBm_TerminateFunc(const u8 *devicep);
static u8   MEMBm_ByteStreamFunc(const u8 *devicep);
static u32  MEMBm_WordStreamFunc(const u8 *devicep);


// global variable-------------------------------------------------------

// static variable-------------------------------------------------------
static MIUnpackBitsParam Nin_UnPackBitsParam2 = { (8 * 8 / 2) * ( 7 * 2 ), 1, 4, 0, 0 };

// const data------------------------------------------------------------
static const u8 Nin_Char_Diff_Huff_Table2[]={
	0x24,0xd4,0x00,0x00,
	0x0f,0x40,0x00,0x00,0x00,0x01,0x81,0x82,0x82,0x83,0x0f,0x83,0x0c,0xc3,0x03,0x83,
	0x01,0x83,0x04,0xc3,0x08,0x0e,0x02,0xc2,0x0d,0xc2,0x07,0x0b,0x06,0x0a,0x05,0x09,
};


const MIReadStreamCallbacks memb_ifp2={
	MEMBm_InitFunc,
	MEMBm_TerminateFunc,
	MEMBm_ByteStreamFunc,
	NULL,
	MEMBm_WordStreamFunc,
};


// function's description--------------------------------------------

// Nintendoロゴデータの展開ルーチン（OBJ2Dマップモードで展開）
// ※tempBuffpには、0x700byte必要です。
void SYSM_LoadNintendoLogo2D( u16 *ninLogoDatap, u16 *dstp, u16 color, u32 *tempBuffp )
{
	u32 work[ 0x100 / sizeof(u32) ];
	
	Nin_UnPackBitsParam2.destOffset = color - 1;
	UnCompNintendoLogo2( ninLogoDatap, (u16 *)tempBuffp, work );
	MI_CpuCopyFast( (u16 *)( (u32)tempBuffp + 0    ), dstp + 0x0000 / sizeof(u16), 0x1a0 );
	MI_CpuCopyFast( (u16 *)( (u32)tempBuffp + 0x1a0), dstp + 0x0400 / sizeof(u16), 0x1a0 );
}


void SYSM_LoadNintendoLogo1D( u16 *ninLogoDatap, u16 *dstp, u16 color, u32 *tempBuffp )
{
	u32 work[ 0x100 / sizeof(u32) ];
	
	Nin_UnPackBitsParam2.destOffset = color - 1;
	UnCompNintendoLogo2( ninLogoDatap, (u16 *)tempBuffp, work );
	MI_CpuCopyFast( (u16 *)tempBuffp, dstp, 0x340 );
}

/*	UnCompNintendoLogo2ワーク内訳

	＜配置先＞					dstp(0x700)		temp(0x100)
	
	Nintendoロゴ＋ヘッダ						0x0c0
	ハフマン展開バッファ		0x200
	ハフマン展開後のバッファ	0x0d0
	diff後のデータ								0x0d0
	UnpackBits後のデータ		0x700

*/

//　Nintendoロゴ展開ルーチン　(r0=ロゴ圧縮データ  r1=展開先アドレス)
#include <twl/code16.h>
static asm void UnCompNintendoLogo2(u16 *NintendoLogoDatap, u16 *dstp, u32 *temp)
{
		push		{r0-r2,r4, lr}
		
		ldr			r0, =Nin_Char_Diff_Huff_Table2
		mov			r1, r2								// r1 <- temp
		mov			r4, r1								// r4 <- temp
		mov			r2, #36
		bl			MIi_CpuCopy16						// Nintendoロゴの圧縮テーブル部分のみをコピーしてくる
		
		ldr			r0, [sp, #0]						// r0 <- NintendoLogoDatap
		mov			r2, #36
		add			r1, r4, r2							// r1 <- temp + 36
		mov			r2, #NINTENDO_LOGO_LENGTH
		bl			MIi_CpuCopy16						// NintendoLogoDatapからNintendoロゴデータ本体をコピーしてくる
		
		mov			r0, r4								// r0 <- temp
		ldr			r1, [sp, #4]						// r1 <- dstp
		mov			r2, #1
		lsl			r2, r2, #8
		add			r2, r2, r1							// r2 <- dstp + 0x100
		ldr			r3, =memb_ifp2						// r3 <- memb_ifp2
		bl			SVC_UncompressHuffmanFromDevice		// ハフマン展開
		
		ldr			r0, [sp, #4]
		ldr			r2, =0x0000d082
		str			r2, [r0,#0]
		
		mov			r1, r4								// temp
		bl			SVC_DiffUnFilter16_16_2				// Diff展開
		
		mov			r0, r4								// temp
		ldr			r1, [sp, #4]						// dstp
		ldr			r2, =Nin_UnPackBitsParam2
		bl			SVC_UnpackBits						// ビット展開
		
		pop			{r0-r2,r4, pc}
}


//  差分フィルタ展開システムコール（16Bit→16Bit） (r0=Srcp, r1=Destp)
static asm void SVC_DiffUnFilter16_16_2(u16 *srcp,u16 *dstp)
{
		swi			24
		bx			lr
}
#include <twl/codereset.h>


// ============================================================================
// バイトアクセス可能メモリ用アクセスルーチン群
// ============================================================================
static s32 MEMBm_InitFunc(const u8 *devicep, void *ramp, const void *paramp)
{
	#pragma unused(ramp)
	if(paramp)	return (s32)MEMBm_WordStreamFunc(devicep);
	else		return 0;
}

static s32 MEMBm_TerminateFunc(const u8 *devicep)
{
	#pragma unused(devicep)
	return 0;
}

static u8  MEMBm_ByteStreamFunc(const u8 *devicep)
{
	return *devicep;
}

static u32 MEMBm_WordStreamFunc(const u8 *devicep)
{
	return *(u32 *)devicep;
}
