/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.c

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
#include <sysmenu.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
extern void LCFG_VerifyAndRecoveryNTRSettings( void );

// function's prototype-------------------------------------------------------
static TitleProperty *SYSMi_CheckShortcutBoot( void );
static void SYSMi_CheckCardCloneBoot( void );

// global variable-------------------------------------------------------------
void *(*SYSMi_Alloc)( u32 size  );
void  (*SYSMi_Free )( void *ptr );

#define SYSM_DEBUG_
#ifdef SYSM_DEBUG_
SYSM_work		*pSysm;											// デバッガでのSYSMワークのウォッチ用
ROM_Header_Short *pRomHeader;
#endif
// static variable-------------------------------------------------------------

static TitleProperty s_bootTitleBuf;

// const data------------------------------------------------------------------

// ============================================================================
//
// 初期化
//
// ============================================================================

// SystemMenuの初期化
void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
#ifdef SYSM_DEBUG_
	pSysm = SYSMi_GetWork();
	pRomHeader = (ROM_Header_Short *)0x027fc000;
#endif /* SYSM_DEBUG_ */
	
	// ランチャーのマウント情報セット
	SYSMi_SetLauncherMountInfo();
	
    // ARM7コンポーネント用プロテクションユニット領域変更
    OS_SetProtectionRegion( 2, SYSM_OWN_ARM7_MMEM_ADDR, 512KB );
	
	SYSM_SetAllocFunc( pAlloc, pFree );
	
	//	PXI_SetFifoRecvCallback( SYSMENU_PXI_FIFO_TAG, SYSMi_PXIFifoRecvCallback );
	
	reg_OS_PAUSE |= REG_OS_PAUSE_CHK_MASK;							// PAUSEレジスタのチェックフラグのセット
}


// アリーナ再設定
void SYSM_SetArena( void )
{
	// ARM9用ブートコード配置のため、アリーナHi位置を下げる
	OS_SetMainArenaHi( (void *)SYSM_OWN_ARM9_MMEM_ADDR_END );
}


// システムメニューライブラリ用メモリアロケータの設定
void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
	SYSMi_Alloc = pAlloc;
	SYSMi_Free  = pFree;
}


// メモリAlloc
void *SYSM_Alloc( u32 size )
{
	void *p = SYSMi_Alloc( size );
	OS_TPrintf( "SYSM_Alloc : %08x  %xbytes\n", p, size );
	return p;
}


// メモリFree
void SYSM_Free( void *ptr )
{
	OS_TPrintf( "SYSM_Free  : %08x\n", ptr );
	SYSMi_Free( ptr );
}


// ============================================================================
//
// 情報取得
//
// ============================================================================

// パラメータリード
TitleProperty *SYSM_ReadParameters( void )
{
	TitleProperty *pBootTitle = NULL;
	u8 brightness = LCFG_TWL_BACKLIGHT_LEVEL_MAX;
	
	// ARM7のリセットパラメータ取得が完了するのを待つ
	while( !SYSMi_GetWork()->flags.common.isARM9Start ) {
		SVC_WaitByLoop( 0x1000 );
	}
//#ifdef DEBUG_USED_CARD_SLOT_B_
	// ARM7のカードチェック完了を待つ
	while( !SYSMi_GetWork()->flags.common.is1stCardChecked ) {
		SVC_WaitByLoop( 0x1000 );
	}
//#endif

	//-----------------------------------------------------
	// リセットパラメータの判定（リセットパラメータが有効かどうかは、ARM7でやってくれている）
	//-----------------------------------------------------
	{
		if( SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ||		// ロゴデモスキップ？
			SYSMi_IsDebuggerBannerViewMode() ) {
			SYSM_SetLogoDemoSkip( TRUE );
		}
		
		if( SYSM_GetLauncherParamBody()->v1.bootTitleID ) {			// アプリ直接起動の指定があったらロゴデモを飛ばして指定アプリ起動
			s_bootTitleBuf.titleID = SYSM_GetLauncherParamBody()->v1.bootTitleID;
			s_bootTitleBuf.flags = SYSM_GetLauncherParamBody()->v1.flags;
			s_bootTitleBuf.pBanner = (TWLBannerFile *)(*(TWLBannerFile **)(SYSM_GetLauncherParamBody()->v1.rsv));
			pBootTitle = &s_bootTitleBuf;
		}
	}
	
	//-----------------------------------------------------
	// 量産工程用ショートカットキー or
	// 検査カード起動
	//-----------------------------------------------------
	if( pBootTitle == NULL ) {
		pBootTitle = SYSMi_CheckShortcutBoot();
	}
	
	//-----------------------------------------------------
	// HW情報のリード
	//-----------------------------------------------------
	// ノーマル情報リード
	if( !LCFG_ReadHWNormalInfo() ) {
		OS_TPrintf( "HW Normal Info Broken!\n" );
		SYSMi_GetWork()->flags.common.isBrokenHWNormalInfo = TRUE;
	}
	// セキュア情報リード
	if( !LCFG_ReadHWSecureInfo() ) {
		OS_TPrintf( "HW Secure Info Broken!\n" );
		SYSMi_GetWork()->flags.common.isBrokenHWSecureInfo = TRUE;
		SYSMi_GetWork()->flags.common.isFatalError = TRUE;
	}
	
	//-----------------------------------------------------
	// 本体設定データのリード
	//-----------------------------------------------------
	if( LCFG_ReadTWLSettings() ) {									// NANDからTWL本体設定データをリード
		SYSM_CaribrateTP();											// 読み出したTWL本体設定データをもとにTPキャリブレーション。
		brightness = (u8)LCFG_TSD_GetBacklightBrightness();
	}
	
	//-----------------------------------------------------
	// 各種デバイス設定
	//-----------------------------------------------------
	// バックライト輝度設定
	SYSM_SetBackLightBrightness( brightness );
	// RTC補正
	SYSMi_WriteAdjustRTC();
	// RTC値のチェック
	SYSMi_CheckRTC();
	
	LCFG_VerifyAndRecoveryNTRSettings();							// NTR設定データを読み出して、TWL設定データとベリファイし、必要ならリカバリ
	
	//NAMの初期化
	NAM_Init( SYSM_Alloc, SYSM_Free );
	
	return pBootTitle;
}


// ショートカット起動のチェック
static TitleProperty *SYSMi_CheckShortcutBoot( void )
{
	static TitleProperty s_bootTitle;
	
	MI_CpuClear8( &s_bootTitle, sizeof(TitleProperty) );
	
	//-----------------------------------------------------
	// ISデバッガ起動 or
	// 量産工程用ショートカットキー or
	// 検査カード起動
	//-----------------------------------------------------
	if( SYSM_IsExistCard() ) {
		if( ( SYSMi_GetWork()->flags.common.isOnDebugger &&		// ISデバッガが有効かつJTAGがまだ有効でない時
			  !( *(u8 *)( HW_SYS_CONF_BUF + HWi_WSYS09_OFFSET ) & HWi_WSYS09_JTAG_CPUJE_MASK ) ) ||
			SYSM_IsInspectCard() ||
			( ( PAD_Read() & SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) ==
			  SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT )
			) {
			s_bootTitle.flags.isAppRelocate = TRUE;
			s_bootTitle.flags.isAppLoadCompleted = TRUE;
			s_bootTitle.flags.isInitialShortcutSkip = TRUE;			// 初回起動シーケンスを飛ばす
			s_bootTitle.flags.isLogoSkip = TRUE;					// ロゴデモを飛ばす
			s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_ROM;
			s_bootTitle.flags.isValid = TRUE;
			s_bootTitle.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
			SYSM_SetLogoDemoSkip( s_bootTitle.flags.isLogoSkip );
			return &s_bootTitle;
		}
	}
	
	//-----------------------------------------------------
	// TWL設定データ未入力時の初回起動シーケンス起動
	//-----------------------------------------------------
#if 0
#ifdef ENABLE_INITIAL_SETTINGS_
	if( !LCFG_TSD_IsSetTP() ||
		!LCFG_TSD_IsSetLanguage() ||
		!LCFG_TSD_IsSetDateTime() ||
		!LCFG_TSD_IsSetUserColor() ||
		!LCFG_TSD_IsSetNickname() ) {
		s_bootTitle.titleID = TITLE_ID_MACHINE_SETTINGS;
		s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
		s_bootTitle.flags.isValid = TRUE;
		return &s_bootTitle;
	}
#endif // ENABLE_INITIAL_SETTINGS_
#endif
	
	return NULL;													// 「ブート内容未定」でリターン
}


// クローンブート判定
static void SYSMi_CheckCardCloneBoot( void )
{
#if 0
	u8 	*buffp         = (u8 *)&pTempBuffer;
	u32 total_rom_size = SYSM_GetCardRomHeader()->rom_valid_size ? SYSM_GetCardRomHeader()->rom_valid_size : 0x01000000;
	u32 file_offset    = total_rom_size & 0xFFFFFE00;
	
	DC_FlushRange( buffp, BNR_IMAGE_SIZE );
	CARD_ReadRom( 4, (void *)file_offset, buffp, BNR_IMAGE_SIZE );
	
	buffp += total_rom_size & 0x000001FF;
	if( *buffp++ == 'a' && *buffp == 'c' ) {
		SYSMi_GetWork()->flags.common.cloneBootMode = CLONE_BOOT_MODE;
	}else {
		SYSMi_GetWork()->flags.common.cloneBootMode = OTHER_BOOT_MODE;
	}
#endif
}


//======================================================================
//  デバッグ
//======================================================================
