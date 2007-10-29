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

// Nintendo���S�f�[�^�̓W�J���[�`���iOBJ2D�}�b�v���[�h�œW�J�j
// ��tempBuffp�ɂ́A0x700byte�K�v�ł��B
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

/*	UnCompNintendoLogo2���[�N����

	���z�u�恄					dstp(0x700)		temp(0x100)
	
	Nintendo���S�{�w�b�_						0x0c0
	�n�t�}���W�J�o�b�t�@		0x200
	�n�t�}���W�J��̃o�b�t�@	0x0d0
	diff��̃f�[�^								0x0d0
	UnpackBits��̃f�[�^		0x700

*/

//�@Nintendo���S�W�J���[�`���@(r0=���S���k�f�[�^  r1=�W�J��A�h���X)
#include <twl/code16.h>
static asm void UnCompNintendoLogo2(u16 *NintendoLogoDatap, u16 *dstp, u32 *temp)
{
		push		{r0-r2,r4, lr}
		
		ldr			r0, =Nin_Char_Diff_Huff_Table2
		mov			r1, r2								// r1 <- temp
		mov			r4, r1								// r4 <- temp
		mov			r2, #36
		bl			MIi_CpuCopy16						// Nintendo���S�̈��k�e�[�u�������݂̂��R�s�[���Ă���
		
		ldr			r0, [sp, #0]						// r0 <- NintendoLogoDatap
		mov			r2, #36
		add			r1, r4, r2							// r1 <- temp + 36
		mov			r2, #NINTENDO_LOGO_LENGTH
		bl			MIi_CpuCopy16						// NintendoLogoDatap����Nintendo���S�f�[�^�{�̂��R�s�[���Ă���
		
		mov			r0, r4								// r0 <- temp
		ldr			r1, [sp, #4]						// r1 <- dstp
		mov			r2, #1
		lsl			r2, r2, #8
		add			r2, r2, r1							// r2 <- dstp + 0x100
		ldr			r3, =memb_ifp2						// r3 <- memb_ifp2
		bl			SVC_UncompressHuffmanFromDevice		// �n�t�}���W�J
		
		ldr			r0, [sp, #4]
		ldr			r2, =0x0000d082
		str			r2, [r0,#0]
		
		mov			r1, r4								// temp
		bl			SVC_DiffUnFilter16_16_2				// Diff�W�J
		
		mov			r0, r4								// temp
		ldr			r1, [sp, #4]						// dstp
		ldr			r2, =Nin_UnPackBitsParam2
		bl			SVC_UnpackBits						// �r�b�g�W�J
		
		pop			{r0-r2,r4, pc}
}


//  �����t�B���^�W�J�V�X�e���R�[���i16Bit��16Bit�j (r0=Srcp, r1=Destp)
static asm void SVC_DiffUnFilter16_16_2(u16 *srcp,u16 *dstp)
{
		swi			24
		bx			lr
}
#include <twl/codereset.h>


// ============================================================================
// �o�C�g�A�N�Z�X�\�������p�A�N�Z�X���[�`���Q
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
