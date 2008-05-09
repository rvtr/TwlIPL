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
#define SWI_NO_CPU_SET_FAST     			( 12 << 16 )	// SVC_CpuSetFast()

#define SDK_VER_30							0x03000000
#define SDK_VER_22_RELEASE_PLUS_3			0x02027533
#define SDK_VER_30_RC3						0x03004f4c

#define SDKVER_OFFSET_IN_PATTERN			(-4)
#define PATCH1_OFFSET_IN_PATTERN			0x18
#define PATCH3_OFFSET_IN_PATTERN			0x04

// function's prototype-------------------------------------------------
static int  DSi_ExistNitroCard(void);
static void* DSi_GetPatchBaseAddr( void );
static BOOL DSi_IsPatchedSDKVersion( void );
static u32  DSi_SearchBinaryCore( const u32 *patp, int pat_word_size, int patch_offset );
static void DSi_SetPatchCodeToREDRsvArea( u32 patch_addr, const u32 *patchp );
void DSi_CopyWLPatch( void );

// extern data----------------------------------------------------------

// const data-----------------------------------------------------------

// SDKバージョン検出用マジックコード
static const u32 sdk_ver_magic_code[] = {
	0xdec00621,
	0x2106c0de,
};

// パッチ１対象コード
static const u32 patch1_org[] = {
	0xe59f1028,	0xe59f2028,	0xe1d200b0,	0xe2100001,
	0x1afffffc,	0xe1d100b0, 0xe3500006,	0x0afffff9,
	0xe1d100b0,	0xe3500005,	0x0afffff6,	0xe12fff1e,
	0x04808214,	0x0480819c,
};

// パッチ３対象コード
static const u32 patch3_org[] = {
										0xE1D100B0,
	0xE3500006, 0x0AFFFFF9,	0xE1D100B0, 0xE3500005, 	// patch target( top 20bytes )
	0x0AFFFFF6,	0xE3A01000,	0xE59F0068,	0xE1C010B0,
	0xE59F0064, 0xE1C010B0, 0xE59F004C, 0xE5900000, 
	0xE2800C03,
};

// パッチ１コード
static const u32 patch1_code[] = {
	0xe3500005,
	0x312fff1e,
	0xe1d100b0,
	0xe3500008,
	0x9afffff6,
};

// パッチ３コード
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
//　USG-CPU以降におけるARM7コード無線不具合へのパッチ処理
//　※ゲームのロードが完了した後に本関数をコールして下さい。
//
//----------------------------------------------------------------------

// パッチコードの挿入
void DS_InsertWLPatch( void )
{
	// SDKバージョンがパッチ対象？
	if( DSi_IsPatchedSDKVersion() ) {
		// patch1, patch3をサーチ
		const u32 *patchp    = patch1_code;
		u32 patch_addr = DSi_SearchBinaryCore( patch1_org, sizeof( patch1_org ) / sizeof( u32 ), PATCH1_OFFSET_IN_PATTERN );
		
		if( patch_addr == 0 ) {
			patchp     = patch3_code;
			patch_addr = DSi_SearchBinaryCore( patch3_org, sizeof( patch3_org ) / sizeof( u32 ), PATCH3_OFFSET_IN_PATTERN );
		}
		// パッチ対象コードが見つかったら、パッチコードをセット。
		if( patch_addr ) {
			ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
			if( SYSMi_GetWork()->cloneBootMode == SYSM_CLONE_BOOT_MODE ) {	// ※クローンブートかどうかはDS_CheckROMCloneBoot()で事前に調査。
				// クローンブートならば、直パッチ
				SVC_CpuCopyFast( patchp, patch_addr, DS_WLPATCH_SIZE );
			}else {
				// それ以外ならば、RED_RSVにパッチコードを挿入。
				// ※パッチアドレスをIPL2の一時格納バッファアドレスから実際のARM7ロードアドレスに変換。
				patch_addr = patch_addr + ((u32)dh->s.sub_entry_address - (u32)DSi_GetPatchBaseAddr());
				DSi_SetPatchCodeToREDRsvArea( patch_addr, patchp );
				dh->s.sub_entry_address = (void *)DS_REDRSV_PATCH_FUNC_ADDR;
			}
		}
	}
}


// パッチベースアドレスの取得
static void* DSi_GetPatchBaseAddr( void )
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
	void* p = (void*)SYSMi_GetWork()->romRelocateInfo[1].src;

    // 再配置しない場合は
	if ( ! p )
	{
		void* h = dh->s.sub_ram_address;
		if ( h >= (void*)SYSM_NTR_ARM7_LOAD_MMEM && h < (void*)(SYSM_NTR_ARM7_LOAD_MMEM_END - DS_WLPATCH_SIZE) )
		{
			p = h;
		}
	}

	return p;
}


// パッチ対象のSDKバージョンかどうか判定
static BOOL DSi_IsPatchedSDKVersion( void )
{
	u32 addr;
	u32 sdk_ver;
	
	// SDKバージョンのサーチ
	addr = DSi_SearchBinaryCore( sdk_ver_magic_code, sizeof( sdk_ver_magic_code ) / sizeof( u32 ), SDKVER_OFFSET_IN_PATTERN );
	if( addr == 0 ) {
		return FALSE;
	}
	
	// SDKバージョンの判定
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


//　バイナリサーチ
static u32 DSi_SearchBinaryCore( const u32 *patp, int pat_word_size, int patch_offset )
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
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
//　パッチコードのセットおよびパッチ処理の実体
//
//----------------------------------------------------------------------

// パッチコードをシステムのRED予約領域にセット
static void DSi_SetPatchCodeToREDRsvArea( u32 patch_addr, const u32 *patchp )
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
	u32 *dstp = (u32 *)HW_RED_RESERVED;
	u32 *srcp = (u32 *)&DSi_CopyWLPatch;
	int i;
	
	for( i = 0; i < DS_WLPATCH_SIZE/sizeof(u32); i++ ) {
		*dstp++ = *patchp++;
	}
	*dstp++ = patch_addr;
	*dstp++ = (u32)dh->s.sub_entry_address;
	
	for( i = 0; i < DS_WLPATCH_COPYCODE_SIZE/sizeof(u32); i++ ) {
		*dstp++ = *srcp++;
	}
}


#include	<nitro/code32.h>


#if 0
#define SWI_NO_CPU_SET_FAST		(12<<16)
// 他プログラムへ組み込んでテストする時のダミーパッド
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


// WLへパッチを当てるコード実体
asm void DSi_CopyWLPatch( void )
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
#define DMA_SRC_INC             0x00000000    // 転送元 インクリメント 選択
#define SVC_CpuCopyFast(srcp, destp, size) \
                                           \
    SVC_CpuSetFast( (u8 *)(srcp),          \
				    (u8 *)(destp),         \
				    ( DMA_SRC_INC | ( (size)/(32/8) & 0x1fffff ) ) )
*/

// クローンブート判定
void DS_CheckROMCloneBoot( void )
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
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
		SYSMi_GetWork()->cloneBootMode = SYSM_CLONE_BOOT_MODE;
	}else {
		SYSMi_GetWork()->cloneBootMode = SYSM_OTHER_BOOT_MODE;
	}
}

// NITROカード存在チェック 		「リターン　1：カード認識　0：カードなし」
static int DSi_ExistNitroCard(void)
{
	ROM_Header *dh = (ROM_Header *)HW_ROM_HEADER_BUF;      // DS互換ROMヘッダ
	if((dh->s.nintendo_logo_crc16 == 0xcf56)
	 &&(dh->s.header_crc16 == SYSMi_GetWork()->cardHeaderCrc16)) {
		return TRUE;												// NITROカードあり（NintendoロゴCRC、カードヘッダCRCが正しい場合）
																	// ※Nintendoロゴデータのチェックは、特許の都合上、ロゴ表示ルーチン起動後に行います。
	}else {
		return FALSE;												// NITROカードなし
	}
}

