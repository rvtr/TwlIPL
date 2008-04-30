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
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------

// dev_commonKeyはベタで持っていい。
static const u8 dev_commonKey[] = {
	0xA1, 0x60, 0x4A, 0x6A, 0x71, 0x23, 0xB5, 0x29,
	0xAE, 0x8B, 0xEC, 0x32, 0xC8, 0x16, 0xFC, 0xAA
};

static const u8 dev_seedES[] = {
	0x2D, 0xD4, 0x03, 0x98, 0xA7, 0x6B, 0x03, 0x28,
	0xCE, 0x61, 0x04, 0xBB, 0x0A, 0xBB, 0x03, 0x5B,
};

static const u8 dev_seedNAM[] = {
	0x4D, 0x04, 0xA4, 0x7F, 0xE3, 0x02, 0x30, 0x2E,
	0x2A, 0x07, 0x06, 0xE6, 0xD9, 0x06, 0x47, 0x76,
};

static const u8 dev_seedSlotC[] = {
	0x3B, 0x06, 0x86, 0x57, 0x33, 0x04, 0x88, 0x11,
	0x49, 0x04, 0x6B, 0x33, 0x12, 0x02, 0xAC, 0xF3,
};


// ============================================================================
//
// AES鍵スロット設定
//
// ============================================================================

void SYSMi_SetAESKeysForAccessControl( BOOL isNtrMode, ROM_Header *pROMH )
{
	BOOL isClearSlotB = TRUE;
	BOOL isClearSlotC = TRUE;
	
	// 鍵のセット
	MI_CpuClearFast( (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, HW_LAUNCHER_DELIVER_PARAM_BUF_SIZE );
	if( !isNtrMode &&
		( pROMH->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK ) ) {
		SYSMi_SetAESKeysForAccessControlCore( pROMH, (u8 *)HW_LAUNCHER_DELIVER_PARAM_BUF, &isClearSlotB, &isClearSlotC );
	}
	
	// ブートするアプリに応じて、AESキースロットのクリアを行う。
	{
		AESi_ResetAesKeyA();
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


void SYSMi_SetAESKeysForAccessControlCore( ROM_Header *pROMH, u8 *pDst, BOOL *pIsClearSlotB, BOOL *pIsClearSlotC )
{
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


