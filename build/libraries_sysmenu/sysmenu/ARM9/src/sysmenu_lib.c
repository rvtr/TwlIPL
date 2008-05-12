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
#include <sysmenu/mcu.h>
#include <firm/format/from_firm.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
extern void LCFG_VerifyAndRecoveryNTRSettings( void );

// function's prototype-------------------------------------------------------
static void SYSMi_CopyLCFGData( void );
static TitleProperty *SYSMi_CheckDebuggerBannerViewModeBoot( void );
static TitleProperty *SYSMi_CheckShortcutBoot1( void );
static TitleProperty *SYSMi_CheckShortcutBoot2( void );
static void SYSMi_CheckCardCloneBoot( void );
void SYSMi_SendKeysToARM7( void );
static OSTitleId SYSMi_getTitleIdOfMachineSettings( void );

// global variable-------------------------------------------------------------
void *(*SYSMi_Alloc)( u32 size  );
void  (*SYSMi_Free )( void *ptr );

#define SYSM_DEBUG_
#ifdef SYSM_DEBUG_
SYSM_work       *pSysm;                                         // デバッガでのSYSMワークのウォッチ用
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

    // ARM7で使用する分の鍵を渡す
    SYSMi_SendKeysToARM7();

    // ランチャーのマウント情報セット
    //SYSMi_SetLauncherMountInfo();

    // ARM7コンポーネント用プロテクションユニット領域変更
    OS_SetProtectionRegion( 2, SYSM_OWN_ARM7_MMEM_ADDR, 512KB );

    SYSM_SetAllocFunc( pAlloc, pFree );

    //  PXI_SetFifoRecvCallback( SYSMENU_PXI_FIFO_TAG, SYSMi_PXIFifoRecvCallback );

    reg_OS_PAUSE |= REG_OS_PAUSE_CHK_MASK;                          // PAUSEレジスタのチェックフラグのセット
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
    OS_TPrintf( "SYSM_Alloc : 0x%08x  0x%xbytes\n", p, size );
    return p;
}


// メモリFree
void SYSM_Free( void *ptr )
{
    OS_TPrintf( "SYSM_Free  : 0x%08x\n", ptr );
    SYSMi_Free( ptr );
}


// ARM7で使用する分の鍵を渡す
void SYSMi_SendKeysToARM7( void )
{
    MI_SetWramBank(MI_WRAM_ARM9_ALL);
    // DS互換BlowfishテーブルをARM7へ渡す
    MI_CpuCopyFast( &((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->ds_blowfish, (void *)HW_WRAM_0, sizeof(BLOWFISH_CTX) );
    DC_FlushRange( (void *)HW_WRAM_0, sizeof(BLOWFISH_CTX) );
    MI_SetWramBank(MI_WRAM_ARM7_ALL);
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

    //NAMの初期化
    NAM_Init( SYSM_Alloc, SYSM_Free );

    //-----------------------------------------------------
    // HW情報のリード
    //-----------------------------------------------------
    // ノーマル情報リード
    if( !LCFG_ReadHWNormalInfo() ) {
        OS_TPrintf( "HW Normal Info Broken!\n" );
        SYSMi_GetWork()->flags.common.isBrokenHWNormalInfo = TRUE;
        SYSM_SetFatalError( TRUE );
    }
    // セキュア情報リード
    if( !LCFG_ReadHWSecureInfo() ) {
        OS_TPrintf( "HW Secure Info Broken!\n" );
        SYSMi_GetWork()->flags.common.isBrokenHWSecureInfo = TRUE;
        SYSM_SetFatalError( TRUE );
    }

    //-----------------------------------------------------
    // 本体設定データのリード（※必ずHWSecureInforリード後に実行すること。LanguageBitmapを判定に使うため）
    //-----------------------------------------------------
    {
        u8 *pBuffer = SYSM_Alloc( LCFG_READ_TEMP );
        if( pBuffer ) {
			LCFG_ReadTWLSettings( (u8 (*)[LCFG_READ_TEMP])pBuffer );		   // NANDからTWL本体設定データをリード
            SYSM_Free( pBuffer );
        }else {
	        SYSM_SetFatalError( TRUE );
		}
	    LCFG_VerifyAndRecoveryNTRSettings();  		                          	// NTR設定データを読み出して、TWL設定データとベリファイし、必要ならリカバリ
    }
	
    //-----------------------------------------------------
    // システム領域に本体設定などをコピー
    //-----------------------------------------------------
	// NTRカードアプリARM9コードのロード領域とメモリがかち合うが、先頭0x4000はセキュア領域で別バッファに格納されるので、
	// ここでこれらのパラメータをロードしても大丈夫。
	SYSMi_CopyLCFGData();
	
    //-----------------------------------------------------
    // 無線ON/OFFフラグをもとに、LEDを設定する。
    //-----------------------------------------------------
	{
		BOOL enable;
		if( LCFG_THW_IsForceDisableWireless() ) {
			enable = FALSE;
		}else {
			enable = LCFG_TSD_IsAvailableWireless();
		}
		SYSMi_SetWirelessLED( enable );
	}
	
    //-----------------------------------------------------
    // 各種デバイス設定
    //-----------------------------------------------------
    // バックライト輝度設定
#ifdef SDK_SUPPORT_PMIC_2
    if ( SYSMi_GetMcuVersion() <= 1 )
    {
        // X2ボード以前だけ輝度設定する
        SYSM_SetBackLightBrightness( LCFG_TWL_BACKLIGHT_LEVEL_MAX );
    }
#endif // SDK_SUPPORT_PMIC_2
	
    // TPキャリブレーション
	SYSM_CaribrateTP();
    // RTC補正
    SYSMi_WriteAdjustRTC();
    // RTC値のチェック
    SYSMi_CheckRTC();

    //-----------------------------------------------------
	// ARM7の処理待ち
    //-----------------------------------------------------
	
    // ARM7のランチャーパラメータ取得が完了するのを待つ
    while( !SYSMi_GetWork()->flags.common.isARM9Start ) {
        SVC_WaitByLoop( 0x1000 );
    }
//#ifdef DEBUG_USED_CARD_SLOT_B_
    // ARM7のカードチェック完了を待つ
    while( !SYSMi_GetWork()->flags.hotsw.is1stCardChecked ) {
        SVC_WaitByLoop( 0x1000 );
    }
//#endif


	//-----------------------------------------------------
    // ランチャーパラメータの判定
    //-----------------------------------------------------
	if( SYSM_IsHotStart() ) {
		// ホットスタート時は、基本ロゴデモスキップ
		SYSM_SetLogoDemoSkip( TRUE );
		
		if( LCFG_TSD_GetLastTimeBootSoftPlatform() == PLATFORM_CODE_NTR ) {
		    // 前回ブートがNTRなら、ランチャーパラメータ無効
			SYSMi_GetWork()->flags.common.isValidLauncherParam = 0;
			MI_CpuClear32( &SYSMi_GetWork()->launcherParam, sizeof(LauncherParam) );
		}
		
		if( SYSMi_GetWork()->flags.common.isValidLauncherParam ) {
		    // ロゴデモスキップ無効？
			if( !SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ) {
	            SYSM_SetLogoDemoSkip( FALSE );
	        }
			
	        // アプリ直接起動の指定があったらロゴデモを飛ばして指定アプリ起動
			if( SYSM_GetLauncherParamBody()->v1.bootTitleID ) {
	            s_bootTitleBuf.titleID = SYSM_GetLauncherParamBody()->v1.bootTitleID;
	            s_bootTitleBuf.flags = SYSM_GetLauncherParamBody()->v1.flags;
	            s_bootTitleBuf.pBanner = (TWLBannerFile *)(*(TWLBannerFile **)(SYSM_GetLauncherParamBody()->v1.rsv));
	            pBootTitle = &s_bootTitleBuf;
	        }
		}
	}
	
    // アプリジャンプでないときには、アプリ間パラメタをクリア
    // [TODO:]あらかじめNTRカードのセキュア領域を退避せずに直接0x2000000からロードしている場合も容赦なく消すので注意
    if( !pBootTitle )
    {
    	MI_CpuClearFast((void *)HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_SIZE);
	}

    //-----------------------------------------------------
    // ISデバッガバナーViewモード起動
    //-----------------------------------------------------
    if( pBootTitle == NULL ) {
		// ランチャーパラメータによるダイレクトブートがない場合のみ判定
        pBootTitle = SYSMi_CheckDebuggerBannerViewModeBoot();
    }
    
    //-----------------------------------------------------
    // 量産工程用ショートカットキー or
    // 検査カード起動
    //-----------------------------------------------------
    if( pBootTitle == NULL && // ココまでダイレクトブートが設定されていない場合のみ判定
        !( SYSMi_GetWork()->flags.common.isValidLauncherParam && SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ) )
        // 「ランチャー再起動指定（直接起動指定無し 且つ ランチャパラメタ有効 且つ ロゴスキップ指定）でない」
    {
        pBootTitle = SYSMi_CheckShortcutBoot1();
    }
    
    //-----------------------------------------------------
    // その他のショートカット起動
    //-----------------------------------------------------
    if( pBootTitle == NULL ) {
		// ココまでダイレクトブートが設定されていない場合のみ判定
        pBootTitle = SYSMi_CheckShortcutBoot2();
    }

    return pBootTitle;
}


// 本体設定データなどのメモリ展開。
static void SYSMi_CopyLCFGData( void )
{
	// 本体設定データ、HWノーマル情報、HWセキュア情報をメモリに展開しておく
	MI_CpuCopyFast( LCFGi_GetTSD(), (void *)HW_PARAM_TWL_SETTINGS_DATA, sizeof(LCFGTWLSettingsData) );
	MI_CpuCopyFast( LCFGi_GetHWN(), (void *)HW_PARAM_TWL_HW_NORMAL_INFO, sizeof(LCFGTWLHWNormalInfo) );
	MI_CpuCopyFast( LCFGi_GetHWS(), (void *)HW_HW_SECURE_INFO, HW_HW_SECURE_INFO_END - HW_HW_SECURE_INFO );
	
	// 本体設定データのLauncherStatus部分をクリアしておく
	{
		LCFGTWLSettingsData *pSettings = (LCFGTWLSettingsData *)HW_PARAM_TWL_SETTINGS_DATA;
		MI_CpuClear32( &pSettings->launcherStatus, sizeof(LCFGTWLLauncherStatus) );
	}
}



BOOL SYSM_IsLauncherHidden( void )
{
#ifdef DO_NOT_SHOW_LAUNCHER
	return TRUE;
#else
	return FALSE;
#endif
}


static TitleProperty *SYSMi_CheckDebuggerBannerViewModeBoot( void )
{
    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );

    //-----------------------------------------------------
    // ISデバッガバナーViewモード起動
    //-----------------------------------------------------
	//[TODO]未実装
#if 0
	if( SYSMi_IsDebuggerBannerViewMode() ) {
		return NULL;
	}
#endif

	return NULL;
}

// ショートカット起動のチェックその１
static TitleProperty *SYSMi_CheckShortcutBoot1( void )
{
    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );
	
    //-----------------------------------------------------
    // ISデバッガ起動 or
    // 量産工程用ショートカットキー or
    // 検査カード起動
    //-----------------------------------------------------
    if( SYSM_IsExistCard() ) {
        if( ( SYSM_IsRunOnDebugger() &&      // ISデバッガが有効かつJTAGがまだ有効でない時
              !( *(u8 *)( HW_SYS_CONF_BUF + HWi_WSYS09_OFFSET ) & HWi_WSYS09_JTAG_CPUJE_MASK ) ) ||
            SYSM_IsInspectCard() ||
            ( ( PAD_Read() & SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) ==
              SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT )
            ){
            s_bootTitleBuf.flags.isAppRelocate = TRUE;
            s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
            s_bootTitleBuf.flags.isInitialShortcutSkip = TRUE;         // 初回起動シーケンスを飛ばす
            s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // ロゴデモを飛ばす
            s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_ROM;
            s_bootTitleBuf.flags.isValid = TRUE;
            // ROMヘッダバッファのコピー
            {
                u16 id = (u16)OS_GetLockID();
                (void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );     // ARM7と排他制御する
                (void)SYSMi_CopyCardRomHeader();
                (void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );   // ARM7と排他制御する
                OS_ReleaseLockID( id );
            }
            s_bootTitleBuf.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
            SYSM_SetLogoDemoSkip( s_bootTitleBuf.flags.isLogoSkip );
            return &s_bootTitleBuf;
        }
    }

    return NULL;                                                    // 「ブート内容未定」でリターン
}

// ショートカット起動のチェックその２
static TitleProperty *SYSMi_CheckShortcutBoot2( void )
{
    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );

    //-----------------------------------------------------
    // スタンドアロン起動時、ショートカットキー(select)
    // を押しながらの起動で本体設定の直接起動
    //-----------------------------------------------------
    if( ( PAD_Read() & SYSM_PAD_SHORTCUT_MACHINE_SETTINGS ) ==
		SYSM_PAD_SHORTCUT_MACHINE_SETTINGS )
    {
        s_bootTitleBuf.titleID = SYSMi_getTitleIdOfMachineSettings();
        if(s_bootTitleBuf.titleID != 0)
		{
            s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // 本体設定を起動できる時だけロゴデモを飛ばす
		}
        s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitleBuf.flags.isValid = TRUE;
        s_bootTitleBuf.flags.isAppRelocate = FALSE;
        s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitleBuf;
    }

	// スタンドアロン起動時
    // ランチャー画面を表示しないバージョンの場合
    // カードがささっていたらカードを起動する
    // ささっていない場合は本体設定を起動
#ifdef DO_NOT_SHOW_LAUNCHER
	if( SYSM_IsExistCard() )
	{
        s_bootTitleBuf.flags.isAppRelocate = TRUE;
        s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
        s_bootTitleBuf.flags.isInitialShortcutSkip = TRUE;         // 初回起動シーケンスを飛ばす
        s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // ロゴデモを飛ばす
        s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_ROM;
        s_bootTitleBuf.flags.isValid = TRUE;
        // ROMヘッダバッファのコピー
        {
            u16 id = (u16)OS_GetLockID();
            (void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );     // ARM7と排他制御する
            (void)SYSMi_CopyCardRomHeader();
            (void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );   // ARM7と排他制御する
            OS_ReleaseLockID( id );
        }
        s_bootTitleBuf.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
        SYSM_SetLogoDemoSkip( s_bootTitleBuf.flags.isLogoSkip );
        return &s_bootTitleBuf;
	}else
	{
        s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // ロゴデモを飛ばす
        s_bootTitleBuf.titleID = SYSMi_getTitleIdOfMachineSettings();
        s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitleBuf.flags.isValid = TRUE;
        s_bootTitleBuf.flags.isAppRelocate = FALSE;
        s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitleBuf;
	}
#endif

    //-----------------------------------------------------
    // TWL設定データ未入力時の初回起動シーケンス起動
    //-----------------------------------------------------
#if 0
#ifdef ENABLE_INITIAL_SETTINGS_
    if( !LCFG_TSD_IsFinishedInitialSetting() ) {
        s_bootTitleBuf.titleID = SYSMi_getTitleIdOfMachineSettings();
        if(s_bootTitleBuf.titleID != 0)
		{
            s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // 本体設定を起動できる時だけロゴデモを飛ばす
		}
        s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitleBuf.flags.isValid = TRUE;
        s_bootTitleBuf.flags.isAppRelocate = FALSE;
        s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitleBuf;
    }
#endif // ENABLE_INITIAL_SETTINGS_
#endif

    return NULL;                                                    // 「ブート内容未定」でリターン
}


// クローンブート判定
static void SYSMi_CheckCardCloneBoot( void )
{
#if 0
    u8  *buffp         = (u8 *)&pTempBuffer;
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

// NAM_Initされるようになったので、NAMで本体設定のID取得
// それらしきものがインストールされていない場合は0（NULL）をリターン
static OSTitleId SYSMi_getTitleIdOfMachineSettings( void )
{
	OSTitleId ret = NULL;
	int l;
	int getNum;
	int validNum = 0;
	NAMTitleId *pTitleIDList = NULL;
	
	// インストールされているタイトルの取得
	getNum = NAM_GetNumTitles();
	pTitleIDList = SYSM_Alloc( sizeof(NAMTitleId) * getNum );
	if( pTitleIDList == NULL ) {
		OS_TPrintf( "%s: alloc error.\n", __FUNCTION__ );
		return 0;
	}
	(void)NAM_GetTitleList( pTitleIDList, (u32)getNum );
	
	// 取得したタイトルに本体情報のIDがあるかチェック
	for( l = 0; l < getNum; l++ ) {
		char *code = ((char *)&pTitleIDList[l]) + 1;
		if( 0 == STD_CompareNString( code, "BNH", 3 ) )
		{
			ret = (OSTitleId)pTitleIDList[l];
			break;
		}
	}
	SYSM_Free( pTitleIDList );

	return ret;
}

//======================================================================
//  デバッグ
//======================================================================
