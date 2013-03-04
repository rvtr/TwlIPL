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
#include <sysmenu/sysmenu_lib/common/sysmenu_api.h>
#include <sysmenu/memorymap.h>

// define data-----------------------------------------------------------

// extern data-----------------------------------------------------------

// function's prototype--------------------------------------------------
static void UnCompNintendoLogo2(u16 *pLogoData, u16 *pDst, u32 *pTemp );
static void SVC_DiffUnFilter16_16_2( u16 *pSrc, u16 *pDst );
static s32  MEMBm_InitFunc( const u8 *pDevice, void *pRAM, const void *pParam );
static s32  MEMBm_TerminateFunc( const u8 *pDevice );
static u8   MEMBm_ByteStreamFunc( const u8 *pDevice );
static u32  MEMBm_WordStreamFunc( const u8 *pDevice );


// global variable-------------------------------------------------------

// static variable-------------------------------------------------------
static MIUnpackBitsParam Nin_UnPackBitsParam2 = { (8 * 8 / 2) * ( 7 * 2 ),
													1,
													4,	// カラーbit数(4=4bit=16色, 8=8bit=256色)
													0,
													0 };

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

// Nintendoロゴチェック			「リターン　1:Nintendoロゴ認識成功　0：失敗」
BOOL SYSM_CheckNintendoLogo( u16 *pLogoData )
{
	u16 *pLogoOrg = (u16 *)SYSROM9_NINLOGO_ADR;					// ARM9のシステムROMのロゴデータとカートリッジ内のものを比較
	u16 length    = NINTENDO_LOGO_DATA_LENGTH >> 1;
	
	while( length-- ) {
		if( *pLogoOrg++ != *pLogoData++ ) {
			return FALSE;
		}
	}
	return TRUE;
}


// Nintendoロゴデータの展開ルーチン（OBJ2Dマップモードで展開）
// ※tempBuffpには、0x700byte必要です。
void SYSM_LoadNintendoLogo2D( u16 *pLogoData, u16 *pDst, int paletteColorIndex )
{
	u32 work[ 0x100 / sizeof(u32) ];
	u16 *pBuffer = SYSM_Alloc( 0x700 );
	if( pBuffer == NULL ) {
		OS_Panic( "memory allocation failed.\n" );
	}
	Nin_UnPackBitsParam2.destOffset = paletteColorIndex - 1;
	UnCompNintendoLogo2( pLogoData, (u16 *)pBuffer, work );
	MI_CpuCopy16( (u16 *)( (u32)pBuffer + 0    ), pDst + 0x0000 / sizeof(u16), 0x1a0 );
	MI_CpuCopy16( (u16 *)( (u32)pBuffer + 0x1a0), pDst + 0x0400 / sizeof(u16), 0x1a0 );
	SYSM_Free( pBuffer );
}


void SYSM_LoadNintendoLogo1D( u16 *pLogoData, u16 *pDst, int paletteColorIndex )
{
	u32 work[ 0x100 / sizeof(u32) ];
	u16 *pBuffer = SYSM_Alloc( 0x700 );
	if( pBuffer == NULL ) {
		OS_Panic( "memory allocation failed.\n" );
	}
	Nin_UnPackBitsParam2.destOffset = paletteColorIndex - 1;
	UnCompNintendoLogo2( pLogoData, (u16 *)pBuffer, work );
	MI_CpuCopy16( (u16 *)pBuffer, pDst, 0x340 );
	SYSM_Free( pBuffer );
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
static asm void UnCompNintendoLogo2( u16 *pLogoData, u16 *pDst, u32 *pTemp )
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
		mov			r2, #NINTENDO_LOGO_DATA_LENGTH
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
static asm void SVC_DiffUnFilter16_16_2( u16 *pSrc, u16 *pDst )
{
		swi			24
		bx			lr
}
#include <twl/codereset.h>


// ============================================================================
// バイトアクセス可能メモリ用アクセスルーチン群
// ============================================================================
static s32 MEMBm_InitFunc( const u8 *pDevice, void *pRAM, const void *pParam )
{
	#pragma unused(pRAM)
	if(pParam)	return (s32)MEMBm_WordStreamFunc(pDevice);
	else		return 0;
}

static s32 MEMBm_TerminateFunc( const u8 *pDevice )
{
	#pragma unused(pDevice)
	return 0;
}

static u8  MEMBm_ByteStreamFunc( const u8 *pDevice )
{
	return *pDevice;
}

static u32 MEMBm_WordStreamFunc( const u8 *pDevice )
{
	return *(u32 *)pDevice;
}
