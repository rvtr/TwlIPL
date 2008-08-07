/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     setAESKey.c

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
#include <twl/aes/ARM7/lo.h>
#include <firm/aes/ARM7/aes_init.h>
#include <firm/hw/ARM7/mmap_firm.h>
#include <firm/format/from_firm.h>
#include <firm/os/common/boot.h>
#include <sysmenu.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
void SYSMi_SetAESKeysForAccessControlCore( ROM_Header *pROMH, u8 *pDst, BOOL *pIsClearSlotB, BOOL *pIsClearSlotC );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------

static const u8 dev_sslClientCert[] ATTRIBUTE_ALIGN(4) = {
	0x79, 0xf9, 0x51, 0xbc, 0x3b, 0xb7, 0xe4, 0xca,
	0x18, 0x8f, 0xaf, 0x91, 0x8d, 0x89, 0xd3, 0x46,
};

// dev_commonKeyはベタで持っていい。
static const u8 dev_commonKey[] ATTRIBUTE_ALIGN(4) = {
	0xA1, 0x60, 0x4A, 0x6A, 0x71, 0x23, 0xB5, 0x29,
	0xAE, 0x8B, 0xEC, 0x32, 0xC8, 0x16, 0xFC, 0xAA
};

static const u8 dev_seedES[] ATTRIBUTE_ALIGN(4) = {
	0x2D, 0xD4, 0x03, 0x98, 0xA7, 0x6B, 0x03, 0x28,
	0xCE, 0x61, 0x04, 0xBB, 0x0A, 0xBB, 0x03, 0x5B,
};

static const u8 dev_seedNAM[] ATTRIBUTE_ALIGN(4) = {
	0x4D, 0x04, 0xA4, 0x7F, 0xE3, 0x02, 0x30, 0x2E,
	0x2A, 0x07, 0x06, 0xE6, 0xD9, 0x06, 0x47, 0x76,
};

static const u8 dev_seedSlotC[] ATTRIBUTE_ALIGN(4) = {
	0x3B, 0x06, 0x86, 0x57, 0x33, 0x04, 0x88, 0x11,
	0x49, 0x04, 0x6B, 0x33, 0x12, 0x02, 0xAC, 0xF3,
};

static const u8 dev_jpegEncodeKeyForLauncher[] ATTRIBUTE_ALIGN(4) = {
	0xEF, 0x9A, 0xB3, 0x39, 0x48, 0x3C, 0x2B, 0x13,
	0x39, 0x31, 0xA5, 0x3F, 0x86, 0x25, 0x9B, 0xB3,
};

static const u8 dev_jpegEncodeKeyForNormal[] ATTRIBUTE_ALIGN(4) = {
	0x79, 0xAF, 0xFE, 0xA7, 0xF3, 0x6A, 0xB7, 0xBE,
	0x83, 0xB6, 0x41, 0xFD, 0xFC, 0x42, 0xD7, 0x3B,
};


// ============================================================================
//
// AES鍵スロット設定
//
// ============================================================================

void SYSMi_SetAESKeysForAccessControl( BOOL isNtrMode, ROM_Header *pROMH )
{
	BOOL isClearSlotA = TRUE;
	BOOL isClearSlotB = TRUE;
	BOOL isClearSlotC = TRUE;
	
	// 鍵のセット
	MI_CpuClearFast( (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, HW_LAUNCHER_DELIVER_PARAM_BUF_SIZE );
	if( !isNtrMode ) {
		SYSMi_SetAESKeysForSSLClientCert( pROMH, &isClearSlotA );
		SYSMi_SetAESKeysForSignJPEG( pROMH, &isClearSlotB, &isClearSlotC );
		SYSMi_SetAESKeysForAccessControlCore( pROMH, (u8 *)HW_LAUNCHER_DELIVER_PARAM_BUF, &isClearSlotB, &isClearSlotC );
	}
	
	// ブートするアプリに応じて、AESキースロットのクリアを行う。
	{
		if( isClearSlotA ) AESi_ResetAesKeyA();
		if( isClearSlotB ) AESi_ResetAesKeyB();
		if( isClearSlotC ) AESi_ResetAesKeyC();
		
		// NANDにアクセスしないアプリは、スロットDの鍵をクリアする　
//		if( th->s.access_control.nand_access == 0 ) AESi_ResetAesKeyD();　※rebootの中のREBOOTi_DetachAllDrivesでNANDにアクセスする場合があるので、ここでクリアはできない。やるならその後ろで。
	}
	
	// その他の鍵は不要になるので、消しておく
	{
		OSFromFirmBuf* fromFirm = (void*)HW_FIRM_FROM_FIRM_BUF;
		MI_CpuClearFast(fromFirm, sizeof(OSFromFirmBuf));
	}
}


void SYSMi_SetAESKeysForSSLClientCert( ROM_Header *pROMH, BOOL *pIsClearSlotA )
{
	if ( pROMH->s.access_control.hw_aes_slot_A_SSLClientCert == TRUE ) {
		void *pAESKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
						&( OSi_GetFromFirmAddr()->aes_key[ 3 ] ) : (void *)dev_sslClientCert;
		
		if( pIsClearSlotA ) {
			*pIsClearSlotA = FALSE;
		}
		// AESスロットのデフォルト値セット
		AES_Lock();
		AES_SetKeyA( pAESKey );
		AES_Unlock();
	}
}


void SYSMi_SetAESKeysForSignJPEG( ROM_Header *pROMH, BOOL *pIsClearSlotB, BOOL *pIsClearSlotC )
{
	void *pAESKey;
	if( ( pROMH->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK ) &&
	( ( 0 == STD_CompareNString( (const char *)&pROMH->s.titleID_Lo[ 1 ], "ANH", 3 ) )
#ifdef DEV_UIG_LAUNCHER
	 || ( ( 0 == STD_CompareNString( (const char *)&pROMH->s.titleID_Lo[ 1 ], "AN4", 3 ) ) && ( SCFG_GetBondingOption() != 0 ) )
#endif
	)
		 ) {
		// for Launcher
		pAESKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
					&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x30 ] ) : (void *)dev_jpegEncodeKeyForLauncher;
		if( pIsClearSlotC ) {
			*pIsClearSlotC = FALSE;
		}
		// AESスロットのデフォルト値セット
		AES_Lock();
		AES_SetKeyC( pAESKey );
		AES_Unlock();
		
	}else if( ( pROMH->s.titleID_Hi & TITLE_ID_HI_APP_TYPE_MASK ) &&
			  ( 0 == STD_CompareNString( (const char *)&pROMH->s.titleID_Lo[ 1 ], "INH", 3 ) )
		) {
		// for 写真帳
		pAESKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
						&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x40 ] ) : (void *)dev_jpegEncodeKeyForNormal;
		if( pIsClearSlotB ) {
			*pIsClearSlotB = FALSE;
		}
		// AESスロットのデフォルト値セット
		AES_Lock();
		AES_SetKeyB( pAESKey );
		AES_Unlock();
		
		pAESKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
					&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x30 ] ) : (void *)dev_jpegEncodeKeyForLauncher;
		if( pIsClearSlotC ) {
			*pIsClearSlotC = FALSE;
		}
		// AESスロットのデフォルト値セット
		AES_Lock();
		AES_SetKeyC( pAESKey );
		AES_Unlock();
		
	}else if( pROMH->s.access_control.nand_access == TRUE || pROMH->s.access_control.sd_card_access == TRUE ) {
		// SignJPEG用AESキー
		if ( pROMH->s.access_control.hw_aes_slot_B_SignJPEGForLauncher == TRUE) {
			// for Launcher
			pAESKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
						&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x30 ] ) : (void *)dev_jpegEncodeKeyForLauncher;
		}else {
			// for ノーマルアプリ
			pAESKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
						&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x40 ] ) : (void *)dev_jpegEncodeKeyForNormal;
		}
		if( pIsClearSlotB ) {
			*pIsClearSlotB = FALSE;
		}
		// AESスロットのデフォルト値セット
		AES_Lock();
		AES_SetKeyB( pAESKey );
		AES_Unlock();
	}
}


void SYSMi_SetAESKeysForAccessControlCore( ROM_Header *pROMH, u8 *pDst, BOOL *pIsClearSlotB, BOOL *pIsClearSlotC )
{
	// セキュアアプリ以外はセットなし
	if ( !( pROMH->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK ) ) {
		return;
	}
	
	// commonClientKey
	if( pROMH->s.access_control.common_client_key ) {
		void *pCommonKey = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
							OSi_GetFromFirmAddr()->aes_key[ 0 ] : (void *)dev_commonKey;
		MI_CpuCopy8( pCommonKey, pDst, AES_BLOCK_SIZE );
	}
	// commonClientKeyForDebugger
	else if( pROMH->s.access_control.common_client_key_for_debugger_sysmenu ) {
		MI_CpuCopy8( OSi_GetFromFirmAddr()->aes_key[ 1 ], pDst, AES_BLOCK_SIZE );
	}
	
	// HW AES Slot B
	if( pROMH->s.access_control.hw_aes_slot_B ) {
		void *pSeedES  =  ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
							&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0 ] ) : (void *)dev_seedES;
		MI_CpuCopy8( pSeedES,  pDst + 0x10, AES_BLOCK_SIZE );
		if( pIsClearSlotB ) {
			*pIsClearSlotB = FALSE;
		}
		// AESスロットのデフォルト値セットは不要
	}
	// HW AES Slot C
	if( pROMH->s.access_control.hw_aes_slot_C ) {
		void *pSeedNAM =  ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
							&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x10 ] ) : (void *)dev_seedNAM;
		void *pSeedSlotC = ( SCFG_GetBondingOption() == SCFG_OP_PRODUCT ) ?
							&( OSi_GetFromFirmAddr()->rsa_pubkey[ 3 ][ 0x20 ] ) : (void *)dev_seedSlotC;
		MI_CpuCopy8( pSeedNAM, pDst + 0x20, AES_BLOCK_SIZE );
		if( pIsClearSlotC ) {
			*pIsClearSlotC = FALSE;
		}
		// AESスロットのデフォルト値セット
		AES_Lock();
		AES_SetKeySeedC( pSeedSlotC );
		AES_Unlock();
	}
}


