/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     logoData.c

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
#include "misc.h"

// define data-----------------------------------------------------------
#define NIN_LOGO_LENGTH			0x9c		// Nintendoロゴデータサイズ

#define OAM_OBJ_BLEND           0x00000400  // ＯＢＪ半透明モード
#define OAM_SIZE_64x32          0xc0004000  // ＯＢＪ ６４ｘ３２ドット
#define OAM_COLOR_256           0x00002000  // ２５６色 選択
#define OAM_AFFINE_NONE         0x00000000  // アフィン無効モード
#define OAM_V_POS_SHIFT         0
#define OAM_H_POS_SHIFT         16
#define OAM_AFFINE_NO_SHIFT     25
#define OAM_PRIORITY_SHIFT      10
#define OAM_PLTT_SHIFT          12


// extern data-----------------------------------------------------------

// function's prototype--------------------------------------------------
void LoadLogoData( void );
static void UnCompNintendoLogo( u16 *NintendoLogoDatap, u16 *dstp, u32 *temp );
static void SVC_DiffUnFilter16_16( u16 *srcp,u16 *dstp );
static s32  MEMB_InitFunc( const u8 *devicep, void *ramp, const void *paramp );
static s32  MEMB_TerminateFunc( const u8 *devicep );
static u8   MEMB_ByteStreamFunc( const u8 *devicep );
static u32  MEMB_WordStreamFunc( const u8 *devicep );

// global variable-------------------------------------------------------

// static variable-------------------------------------------------------

// const data------------------------------------------------------------
const u32 OamLogoData[ 2 ][ 2 ] = {
	{
		OAM_OBJ_BLEND | OAM_SIZE_64x32 | OAM_AFFINE_NONE | OAM_COLOR_256 |
		71  << OAM_H_POS_SHIFT |
		88  << OAM_V_POS_SHIFT |
		0   << OAM_AFFINE_NO_SHIFT,
		0   << 1 | 5 << OAM_PLTT_SHIFT | 2 << OAM_PRIORITY_SHIFT
	},
	
	{
		OAM_OBJ_BLEND | OAM_SIZE_64x32 | OAM_AFFINE_NONE | OAM_COLOR_256 |
		71 + 64 << OAM_H_POS_SHIFT |
		88      << OAM_V_POS_SHIFT |
		0       << OAM_AFFINE_NO_SHIFT,
		8       << 1 | 5 << OAM_PLTT_SHIFT | 2 << OAM_PRIORITY_SHIFT
	},
};

static const MIReadStreamCallbacks memb_ifp={
	MEMB_InitFunc,
	MEMB_TerminateFunc,
	MEMB_ByteStreamFunc,
	NULL,
	MEMB_WordStreamFunc,
};

static const MIUnpackBitsParam Nin_UnPackBitsParam={
	(8*8/2)*( 7*2), 1, 8, 0x1e, 0
};

static const u8 Nin_Char_Diff_Huff_Table[] = {
	0x24,0xd4,0x00,0x00,
	0x0f,0x40,0x00,0x00,0x00,0x01,0x81,0x82,0x82,0x83,0x0f,0x83,0x0c,0xc3,0x03,0x83,
	0x01,0x83,0x04,0xc3,0x08,0x0e,0x02,0xc2,0x0d,0xc2,0x07,0x0b,0x06,0x0a,0x05,0x09,
};

static const u8 Nin_Char_Diff_Huff[] ATTRIBUTE_ALIGN( 2 ) = {
	0x24, 0xff, 0xae, 0x51, 0x69, 0x9a, 0xa2, 0x21, 0x3d, 0x84, 0x82, 0x0a, 0x84, 0xe4, 0x09, 0xad,
	0x11, 0x24, 0x8b, 0x98, 0xc0, 0x81, 0x7f, 0x21, 0xa3, 0x52, 0xbe, 0x19, 0x93, 0x09, 0xce, 0x20,
	0x10, 0x46, 0x4a, 0x4a, 0xf8, 0x27, 0x31, 0xec, 0x58, 0xc7, 0xe8, 0x33, 0x82, 0xe3, 0xce, 0xbf,
	0x85, 0xf4, 0xdf, 0x94, 0xce, 0x4b, 0x09, 0xc1, 0x94, 0x56, 0x8a, 0xc0, 0x13, 0x72, 0xa7, 0xfc,
	0x9f, 0x84, 0x4d, 0x73, 0xa3, 0xca, 0x9a, 0x61, 0x58, 0x97, 0xa3, 0x27, 0xfc, 0x03, 0x98, 0x76,
	0x23, 0x1d, 0xc7, 0x61, 0x03, 0x04, 0xae, 0x56, 0xbf, 0x38, 0x84, 0x00, 0x40, 0xa7, 0x0e, 0xfd,
	0xff, 0x52, 0xfe, 0x03, 0x6f, 0x95, 0x30, 0xf1, 0x97, 0xfb, 0xc0, 0x85, 0x60, 0xd6, 0x80, 0x25,
	0xa9, 0x63, 0xbe, 0x03, 0x01, 0x4e, 0x38, 0xe2, 0xf9, 0xa2, 0x34, 0xff, 0xbb, 0x3e, 0x03, 0x44,
	0x78, 0x00, 0x90, 0xcb, 0x88, 0x11, 0x3a, 0x94, 0x65, 0xc0, 0x7c, 0x63, 0x87, 0xf0, 0x3c, 0xaf,
	0xd6, 0x25, 0xe4, 0x8b, 0x38, 0x0a, 0xac, 0x72, 0x21, 0xd4, 0xf8, 0x07, 0x56, 0xcf, 0x00, 0x00,
};


// ============================================================================
// ロゴデータロード
// ============================================================================
void LoadLogoData(void)
{
	u32 temp[ 0x500 / sizeof(u32) ];
	u16 *pBuff = NNS_FndAllocFromAllocator( &g_allocator, 0x1000 );
	MI_CpuClear32( pBuff, 0x1000 );
	
	if( pBuff == NULL ) {
		OS_TPrintf( " %s : memory allocate error.\n", __FUNCTION__ );
		return;
	}
	
	UnCompNintendoLogo( (u16 *)Nin_Char_Diff_Huff, pBuff, temp );		// NintendoロゴをpBuffに展開
	MI_CpuCopy32( pBuff, (u32 *)HW_OBJ_VRAM, 0x340 );					// OBJ-VRAMにロード
	MI_CpuCopy32( (void *)( (u32)pBuff + 0x340 ), (u32 *)( HW_OBJ_VRAM + 0x400 ), 8*8*13 );
	
	*(vu16 *)( HW_OBJ_PLTT + 0x3e ) = 0x0000;							// OBJパレットセット
	*(vu16 *)HW_BG_PLTT = 0xffff;										// バックドロップを「白」にする。
	
	MI_CpuCopy32( OamLogoData, (u32 *)HW_OAM, sizeof(OamLogoData) );	// Nintendoロゴ用OAMデータセット
	NNS_FndFreeToAllocator( &g_allocator, pBuff );
}


//　Nintendoロゴ展開ルーチン　(r0=ロゴ圧縮データ  r1=展開先アドレス)
#include <nitro/code16.h>

asm void UnCompNintendoLogo(u16 *NintendoLogoDatap, u16 *dstp, u32 *temp)
{
		push		{r0-r2,r4, lr}
		
		ldr			r0, =Nin_Char_Diff_Huff_Table
		mov			r2, #0x0e
		lsl			r2, r2, #8
		add			r1, r1, r2							// 引数１+0xe00
		mov			r4, r1
		mov			r2, #36
		bl			MIi_CpuCopy16						// Nintendoロゴの圧縮テーブル部分のみをコピーしてくる
		
		ldr			r0, [sp, #0]						// 引数０（NinLogoBak[36])
		mov			r2, #36
		add			r1, r4, r2							// 引数１+0xe00+36
		mov			r2, #NIN_LOGO_LENGTH
		bl			MIi_CpuCopy16						// 引数０からNintendoロゴデータ本体をコピーしてくる
		
		mov			r0, r4								// 引数１+0xe00
		ldr			r1, [sp, #4]						// 引数１
		ldr			r2, [sp, #8]						// 引数２
		ldr			r3, =memb_ifp
		bl			SVC_UncompressHuffmanFromDevice		// ハフマン展開
		
		ldr			r0, [sp, #4]
		ldr			r2, =0x0000d082
		str			r2, [r0,#0]
		
		mov			r1, r4								// 引数１+0xe00
		bl			SVC_DiffUnFilter16_16				// Diff展開
		
		mov			r0, r4								// 引数１+0xe00
		ldr			r1, [sp, #4]						// 引数１
		ldr			r2, =Nin_UnPackBitsParam
		bl			SVC_UnpackBits						// ビット展開
		
		pop			{r0-r2,r4, pc}
}


//  差分フィルタ展開システムコール（16Bit→16Bit） (r0=Srcp, r1=Destp)
static asm void SVC_DiffUnFilter16_16(u16 *srcp,u16 *dstp)
{
		swi			24
		bx			lr
}
#include <nitro/codereset.h>


// ============================================================================
// バイトアクセス可能メモリ用アクセスルーチン群
// ============================================================================
static s32 MEMB_InitFunc(const u8 *devicep, void *ramp, const void *paramp)
{
#pragma unused(ramp)
	if(paramp)	return (s32)MEMB_WordStreamFunc(devicep);
	else		return 0;
}

static s32 MEMB_TerminateFunc(const u8 *devicep)
{
#pragma unused(devicep)
	return 0;
}

static u8  MEMB_ByteStreamFunc(const u8 *devicep)
{
	return *devicep;
}

static u32 MEMB_WordStreamFunc(const u8 *devicep)
{
	return *(u32 *)devicep;
}

