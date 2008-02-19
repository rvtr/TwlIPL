/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ds_wlpatch.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: #$
  $Rev: $
  $Author: $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include <sysmenu/ds.h>
#include "mb_fileinfo.h"

// define data----------------------------------------------------------
#define _PATCH3_SHORT_										// �p�b�`�R�̃T�[�`�p�^�[���팸�X�C�b�`

#define SWI_NO_CPU_SET_FAST     			( 12 << 16 )	// SVC_CpuSetFast()
#define WL_PATCH_SIZE						20
#define RED_RSV_PATCH_FUNC_ADDR				( HW_RED_RESERVED + 0x1c )

#define SDK_VER_30							0x03000000
#define SDK_VER_22_RELEASE_PLUS_3			0x02027533
#define SDK_VER_30_RC3						0x03004f4c

#define SDKVER_OFFSET_IN_PATTERN			-4
#define PATCH1_OFFSET_IN_PATTERN			0x18
#ifdef  _PATCH3_SHORT_
#define PATCH3_OFFSET_IN_PATTERN			0x04
#else
#define PATCH3_OFFSET_IN_PATTERN			0x40
#endif

// function's prototype-------------------------------------------------
void DS_InsertWLPatch( void );
static int  DSi_ExistNitroCard(void);
static void* DSi_GetPatchBaseAddr( void );
static BOOL DSi_IsPatchedSDKVersion( void );
static u32  DSi_SearchBinaryCore( const u32 *patp, int pat_word_size, int patch_offset );
static void DSi_SetPatchCodeToREDRsvArea( u32 patch_addr, const u32 *patchp );
void DSi_WLPatchCore( void );

// extern data----------------------------------------------------------

// const data-----------------------------------------------------------

// SDK�o�[�W�������o�p�}�W�b�N�R�[�h
static const u32 sdk_ver_magic_code[] = {
	0xdec00621,
	0x2106c0de,
};

// �p�b�`�P�ΏۃR�[�h
static const u32 patch1_org[] = {
	0xe59f1028,	0xe59f2028,	0xe1d200b0,	0xe2100001,
	0x1afffffc,	0xe1d100b0, 0xe3500006,	0x0afffff9,
	0xe1d100b0,	0xe3500005,	0x0afffff6,	0xe12fff1e,
	0x04808214,	0x0480819c,
};

// �p�b�`�R�ΏۃR�[�h
static const u32 patch3_org[] = {
#ifndef _PATCH3_SHORT_
	0xE92D4000, 0xE24DD004, 0xE3A02000, 0xE59F10A8, 
	0xE5910000, 0xE2800C03, 0xE1C0FFFF, 0xE5910000, 
	0xE2800B01, 0xE1C0FFFF, 0xE59F1090, 0xE59F2090, 
	0xE1D200B0, 0xE2100001,	0x1AFFFFFC,
#endif
										0xE1D100B0,
	0xE3500006, 0x0AFFFFF9,	0xE1D100B0, 0xE3500005, 	// patch target( top 20bytes )
	0x0AFFFFF6,	0xE3A01000,	0xE59F0068,	0xE1C010B0,
	0xE59F0064, 0xE1C010B0, 0xE59F004C, 0xE5900000, 
	0xE2800C03,
#ifndef _PATCH3_SHORT_
				0xE1D0FFFF, 0xE3500000, 0x0A00000B, 
	0xEB00FFFF, 0xE59F0030, 0xE5900000, 0xE2800C03, 
	0xE1D0FFFF, 0xE3A01020, 0xEB00FFFF, 0xE3A01000, 
	0xE59F0014, 0xE5900000, 0xE2800C03, 0xE1C0FFFF, 
	0xE28DD004, 0xE8BD4000, 0xE12FFF1E, 0x0380FFF4, 
	0x04808214, 0x0480819C, 0x04808028, 0x0480802A,
#endif
};

// �p�b�`�P�R�[�h
static const u32 patch1_code[] = {
	0xe3500005,
	0x312fff1e,
	0xe1d100b0,
	0xe3500008,
	0x9afffff6,
};

// �p�b�`�R�R�[�h
static const u32 patch3_code[] = {
	0xE3500005,
	0x3A000002,
	0xE1D100B0,
	0xE3500008,
	0x9AFFFFF6,
};

// static variables-----------------------------------------------------
static u32	mbSignBuf[MB_AUTHCODE_SIZE / sizeof(u32)] __attribute__ ((aligned(32)));

// function's description-----------------------------------------------


//----------------------------------------------------------------------
//
//�@USG-CPU�ȍ~�ɂ�����ARM7�R�[�h�����s��ւ̃p�b�`����
//�@���Q�[���̃��[�h������������ɖ{�֐����R�[�����ĉ������B
//
//----------------------------------------------------------------------

// �p�b�`�R�[�h�̑}��
void DS_InsertWLPatch( void )
{
	// SDK�o�[�W�������p�b�`�ΏہH
	if( DSi_IsPatchedSDKVersion() ) {
		// patch1, patch3���T�[�`
		const u32 *patchp    = patch1_code;
		u32 patch_addr = DSi_SearchBinaryCore( patch1_org, sizeof( patch1_org ) / sizeof( u32 ), PATCH1_OFFSET_IN_PATTERN );
		
		if( patch_addr == 0 ) {
			patchp     = patch3_code;
			patch_addr = DSi_SearchBinaryCore( patch3_org, sizeof( patch3_org ) / sizeof( u32 ), PATCH3_OFFSET_IN_PATTERN );
		}
		// �p�b�`�ΏۃR�[�h������������A�p�b�`�R�[�h���Z�b�g�B
		if( patch_addr ) {
			ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
			if( SYSMi_GetWork()->cloneBootMode == IPL2_CLONE_BOOT_MODE ) {	// ���N���[���u�[�g���ǂ����́AARM9���Ŏ��O�ɒ����B
				// �N���[���u�[�g�Ȃ�΁A���p�b�`
				SVC_CpuCopyFast( patchp, patch_addr, WL_PATCH_SIZE );
			}else {
				// ����ȊO�Ȃ�΁ARED_RSV�Ƀp�b�`�R�[�h��}���B
				// ���p�b�`�A�h���X��IPL2�̈ꎞ�i�[�o�b�t�@�A�h���X������ۂ�ARM7���[�h�A�h���X�ɕϊ��B
				patch_addr = patch_addr + ((u32)dh->s.sub_entry_address - (u32)DSi_GetPatchBaseAddr());
				DSi_SetPatchCodeToREDRsvArea( patch_addr, patchp );
				dh->s.sub_entry_address = (void *)RED_RSV_PATCH_FUNC_ADDR;
			}
		}
	}
}


// �p�b�`�x�[�X�A�h���X�̎擾
static void* DSi_GetPatchBaseAddr( void )
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
	void* p = (void*)SYSMi_GetWork()->romRelocateInfo[1].src;

	if ( ! p )
	{
		void* h = dh->s.sub_ram_address;
		if ( h >= (void*)SYSM_NTR_ARM7_LOAD_MMEM && h < (void*)(SYSM_NTR_ARM7_LOAD_MMEM_END - WL_PATCH_SIZE) )
		{
			p = h;
		}
	}

	return p;
}


// �p�b�`�Ώۂ�SDK�o�[�W�������ǂ�������
static BOOL DSi_IsPatchedSDKVersion( void )
{
	u32 addr;
	u32 sdk_ver;
	
	// SDK�o�[�W�����̃T�[�`
	addr = DSi_SearchBinaryCore( sdk_ver_magic_code, sizeof( sdk_ver_magic_code ) / sizeof( u32 ), SDKVER_OFFSET_IN_PATTERN );
	if( addr == 0 ) {
		return FALSE;
	}
	
	// SDK�o�[�W�����̔���
	sdk_ver = *(u32 *)addr;
	
	if( sdk_ver >= SDK_VER_30 ) {
		if( sdk_ver >= SDK_VER_30_RC3 ) {
			return FALSE;
		}
	}else if( sdk_ver >= SDK_VER_22_RELEASE_PLUS_3 ) {
		return FALSE;
	}
	return TRUE;
}


//�@�o�C�i���T�[�`
static u32 DSi_SearchBinaryCore( const u32 *patp, int pat_word_size, int patch_offset )
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
	u32 *tgtp;
	int tgt_word_size;
	
	if( patp == sdk_ver_magic_code ) {
		tgtp          = dh->s.main_entry_address;
		tgt_word_size = (int)( dh->s.main_size
						  - ( (int)dh->s.main_entry_address - (int)dh->s.main_ram_address ) ) >> 2;
	}else {
		tgtp          = DSi_GetPatchBaseAddr();
		tgt_word_size = (int)dh->s.sub_size >> 2;
	}
	
	while( tgt_word_size-- ) {
		if( *tgtp++ == *patp ) {
			const u32 *srcp   = patp + 1;
			u32 *checkp       = tgtp;
			int i             = pat_word_size - 1;
			
			if( tgt_word_size < i ) {
				break;
			}
			
			while( i-- ) {
#ifndef _PATCH3_SHORT_
				if( ( *srcp & 0x0000ffff ) == 0x0000ffff ) {
					*checkp |= 0x0000ffff;
				}
#endif
				if( *srcp++ != *checkp++ ) break;
			}
			
			if( i < 0 ) {
				return (u32)( (u32)( tgtp - 1 ) + patch_offset );
			}
		}
	}
	return 0;
}


//----------------------------------------------------------------------
//
//�@�p�b�`�R�[�h�̃Z�b�g����уp�b�`�����̎���
//
//----------------------------------------------------------------------

// �p�b�`�R�[�h���V�X�e����RED�\��̈�ɃZ�b�g
static void DSi_SetPatchCodeToREDRsvArea( u32 patch_addr, const u32 *patchp )
{
#define PATCH_WORD_NUM					5
#define WL_PATCH_CORE_WORD_NUM			10
	
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
	u32 *dstp = (u32 *)HW_RED_RESERVED;
	u32 *srcp = (u32 *)&DSi_WLPatchCore;
	int i;
	
	for( i = 0; i < PATCH_WORD_NUM; i++ ) {
		*dstp++ = *patchp++;
	}
	*dstp++ = patch_addr;
	*dstp++ = (u32)dh->s.sub_entry_address;
	
	for( i = 0; i < WL_PATCH_CORE_WORD_NUM; i++ ) {
		*dstp++ = *srcp++;
	}
}


#include	<nitro/code32.h>


#if 0
#define SWI_NO_CPU_SET_FAST		(12<<16)
// ���v���O�����֑g�ݍ���Ńe�X�g���鎞�̃_�~�[�p�b�h
asm void WLPatch_Dummy( void )
{
	nop		// patch code (20bytes)
	nop
	nop
	nop
	nop
	nop		// target addr
	nop 	// ARM7 entry addr
}
#endif


// WL�փp�b�`�𓖂Ă�R�[�h����
asm void DSi_WLPatchCore( void )
{
	add			r0, pc, #-0x24			// R0 <- patch1 code addr
	ldr			r1, [ pc, #-0x14 ]		// R1 <- target addr
	mov			r2, #( 20 / 4 )			// R2 <- copy size / sizeof(u32)
	swi			SWI_NO_CPU_SET_FAST		// SVC_CpuSetFast
	ldr			r12, [ pc, #-0x1c ]		// ARM7 entry address
	mov			lr,  r12
	mov			r0, #0
	mov			r1, r0
	mov			r2, r0
	bx			r12
}

#include	<nitro/codereset.h>

/*
#define DMA_SRC_INC             0x00000000    // �]���� �C���N�������g �I��
#define SVC_CpuCopyFast(srcp, destp, size) \
                                           \
    SVC_CpuSetFast( (u8 *)(srcp),          \
				    (u8 *)(destp),         \
				    ( DMA_SRC_INC | ( (size)/(32/8) & 0x1fffff ) ) )
*/

// �N���[���u�[�g����
void DS_CheckROMCloneBoot( void )
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
	s32	lockCardID;
	u8 	*buffp         = (u8 *)&mbSignBuf;
	u32 auth_offset = dh->s.rom_valid_size ? dh->s.rom_valid_size : 0x01000000;
	u32 page_offset    = auth_offset & 0xFFFFFE00;
	
	if( !DSi_ExistNitroCard() ) {
		return;
	}
	
	if ((lockCardID = OS_GetLockID()) > 0) {
		(void)OS_LockCard( (u16 )lockCardID );
//		DC_FlushRange( buffp, sizeof(mbSignBuf) );
//		IPL2_ReadCard( (void *)page_offset, buffp, sizeof(mbSignBuf) );
		(void)OS_UnLockCard( (u16 )lockCardID );
		OS_ReleaseLockID( (u16 )lockCardID );
	}
	
	buffp += auth_offset & 0x000001FF;
	if( *buffp++ == 'a' && *buffp == 'c' ) {
		SYSMi_GetWork()->cloneBootMode = CLONE_BOOT_MODE;
	}else {
		SYSMi_GetWork()->cloneBootMode = OTHER_BOOT_MODE;
	}
}

// NITRO�J�[�h���݃`�F�b�N 		�u���^�[���@1�F�J�[�h�F���@0�F�J�[�h�Ȃ��v
static int DSi_ExistNitroCard(void)
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS�݊�ROM�w�b�_
	if((dh->s.nintendo_logo_crc16 == 0xcf56)
	 &&(dh->s.header_crc16 == SYSMi_GetWork()->cardHeaderCrc16)) {
		return TRUE;												// NITRO�J�[�h����iNintendo���SCRC�A�J�[�h�w�b�_CRC���������ꍇ�j
																	// ��Nintendo���S�f�[�^�̃`�F�b�N�́A�����̓s����A���S�\�����[�`���N����ɍs���܂��B
	}else {
		return FALSE;												// NITRO�J�[�h�Ȃ�
	}
}

