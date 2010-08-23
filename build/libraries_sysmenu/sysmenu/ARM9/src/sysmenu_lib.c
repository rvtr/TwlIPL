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
#include <sysmenu/namut.h>
#include <firm/format/from_firm.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
#define SYSM_PM_RETRY_NUM                5   // PMリトライ回数
#define TITLE_ID_CARD_BOOT       0x0004800000000000 // カードブート
// extern data-----------------------------------------------------------------
extern void LCFG_VerifyAndRecoveryNTRSettings( void );

// function's prototype-------------------------------------------------------
static void SYSMi_CopyLCFGDataHWInfo( u32 dst_addr );
static void SYSMi_CopyLCFGDataSettings( void );
static TitleProperty *SYSMi_CheckShortcutBoot1( void );
static TitleProperty *SYSMi_CheckShortcutBoot2( void );
void SYSMi_SendKeysToARM7( void );
static OSTitleId SYSMi_getTitleIdOfMachineSettings( void );
static TitleProperty * SYSMi_ShortcutCardBootSub( void );

// global variable-------------------------------------------------------------
void *(*SYSMi_Alloc)( u32 size  );
void  (*SYSMi_Free )( void *ptr );

#define SYSM_DEBUG_
#ifdef SYSM_DEBUG_
SYSM_work       *pSysm;                                         // デバッガでのSYSMワークのウォッチ用
ROM_Header_Short *pRomHeader;
#endif
// static variable-------------------------------------------------------------
static u8 s_lcfgBuffer[ HW_PARAM_TWL_SETTINGS_DATA_SIZE                             // 0x01fc
                      + HW_PARAM_WIRELESS_FIRMWARE_DATA_SIZE                        // 0x0004
                      + HW_PARAM_TWL_HW_NORMAL_INFO_SIZE ] ATTRIBUTE_ALIGN(32);     // 0x1000

static TitleProperty s_bootTitleBuf;

// const data------------------------------------------------------------------

// ============================================================================
//
// 初期化
//
// ============================================================================

#if 1
#include    <twl/code32.h>
void _start_AutoloadDoneCallback(void* argv[]);
// AutoloadDoneCallbackは、ARMでないと動作しない。ISデバッガがブレークポイント処理のために上乗りしているが、そこからのリターンが怪しい。
// AutoloadDoneCallbackを利用して鍵を引き渡す
// AutoloadDoneCallbackはSTATICセクションの.bssクリア前なのでSDK関数も含めて未初期化変数は使用不可。
void _start_AutoloadDoneCallback(void* argv[])
{
#pragma unused(argv)
    // ARM7で使用する分の鍵を渡す
    SYSMi_SendKeysToARM7();
}
#include    <twl/codereset.h>
#endif


// SystemMenuの初期化
void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
#ifdef SYSM_DEBUG_
    pSysm = SYSMi_GetWork();
    pRomHeader = (ROM_Header_Short *)0x027fc000;
#endif /* SYSM_DEBUG_ */

    // ARM7で使用する分の鍵を渡す
    //SYSMi_SendKeysToARM7();

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
    MI_CpuCopyFast( &((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->ds_blowfish, (void *)&GetDeliverBROM9KeyAddr()->ds_blowfish, sizeof(BLOWFISH_CTX) );
    // AES鍵0をARM7へ渡す
//    MI_CpuCopyFast( &((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->aes_key[ 0 ], (void *)&GetDeliverBROM9KeyAddr()->aes_key[ 0 ], AES_KEY_SIZE );
    DC_FlushRange( (void *)HW_WRAM_0, sizeof(DeliverBROM9Key) );
    MI_SetWramBank(MI_WRAM_ARM7_ALL);

#ifndef SYSM_NO_LOAD
#ifdef INITIAL_KEYTABLE_PRELOAD
    SYSMi_GetWork()->flags.hotsw.isKeyTableLoadReady = TRUE;
#endif
#endif
}


// nandのtmpディレクトリの中身を消す
void SYSM_DeleteTmpDirectory( TitleProperty *pBootTitle )
{
    // bootTypeがLAUNCHER_BOOTTYPE_TEMPでない場合、tmpフォルダ内のデータを消す
    if( !pBootTitle || pBootTitle->flags.bootType != LAUNCHER_BOOTTYPE_TEMP ) {
        if( NAMUT_DeleteNandTmpDirectory() ) {
            OS_TPrintf( "\"nand:/tmp\" delete succeeded.\n" );
        }else {
            OS_TPrintf( "\"nand:/tmp\" delete failed.\n" );
        }
    }
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

    //-----------------------------------------------------
    // FATALエラーチェック
    //-----------------------------------------------------
    if( SYSMi_GetWork()->flags.arm7.isNANDFatalError ) {
        UTL_SetFatalError( FATAL_ERROR_NAND );
    }

    //-----------------------------------------------------
    // HW情報のリード
    //-----------------------------------------------------
    // ノーマル情報リード
    if( !LCFG_ReadHWNormalInfo() ) {
#ifndef SYSM_BUILD_FOR_PRODUCTION_TEST
        OS_TPrintf( "HW Normal Info Broken!\n" );
        UTL_SetFatalError( FATAL_ERROR_HWINFO_NORMAL );
#endif // SYSM_BUILD_FOR_PRODUCTION_TEST
    }
    // セキュア情報リード
    if( !LCFG_ReadHWSecureInfo() ) {
#ifndef SYSM_BUILD_FOR_PRODUCTION_TEST
        OS_TPrintf( "HW Secure Info Broken!\n" );
        UTL_SetFatalError( FATAL_ERROR_HWINFO_SECURE );
#endif // SYSM_BUILD_FOR_PRODUCTION_TEST
    }

    //-----------------------------------------------------
    // システム領域にHWInfoをコピー
    //-----------------------------------------------------
    // NTRカードアプリARM9コードのロード領域とメモリがかち合うが、先頭0x4000はセキュア領域で別バッファに格納されるので、
    // ここでこれらのパラメータをロードしても大丈夫。
    SYSMi_CopyLCFGDataHWInfo( (u32)s_lcfgBuffer );

    //-----------------------------------------------------
    // 本体設定データのリード（※必ずHWSecureInforリード後に実行すること。LanguageBitmapを判定に使うため）
    //-----------------------------------------------------
#ifdef SYSM_NO_SETTINGFILE
    MI_CpuCopy8( (void*)HW_PARAM_TWL_SETTINGS_DATA_DEFAULT, LCFGi_GetTSD(), sizeof(LCFGTWLSettingsData) );
#else // SYSM_NO_SETTINGFILE
    {
        u8 *pBuffer = SYSM_Alloc( LCFG_READ_TEMP );
        if( pBuffer ) {
            // NANDからTWL本体設定データをリード
            BOOL isRead = LCFG_ReadTWLSettings( (u8 (*)[LCFG_READ_TEMP])pBuffer );

#ifndef SYSM_NO_LOAD
            // リード失敗ファイルが存在する場合は、ファイルをリカバリ
            if( LCFG_RecoveryTWLSettings() ) {
                if( !isRead ) {
                    // リードに完全に失敗していた場合は、フラッシュ壊れシーケンスへ。
                    LCFG_TSD_SetFlagFinishedBrokenTWLSettings( FALSE );
                    (void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer ); // LCFG_READ_TEMP > LCFG_WRITE_TEMP なので、pBufferをそのまま流用
                }
            }else {
                // リカバリ失敗時は、FALTALエラー
                UTL_SetFatalError( FATAL_ERROR_TWLSETTINGS );
            }
#endif // SYSM_NO_LOAD
            SYSM_Free( pBuffer );
        }else {
            // メモリ確保ができなかった時は、FATALエラー
            UTL_SetFatalError( FATAL_ERROR_TWLSETTINGS );
        }
#ifndef SYSM_NO_LOAD
        LCFG_VerifyAndRecoveryNTRSettings();                                    // NTR設定データを読み出して、TWL設定データとベリファイし、必要ならリカバリ
#endif // SYSM_NO_LOAD
    }
#endif // SYSM_NO_SETTINGFILE

    //-----------------------------------------------------
    // システム領域に本体設定をコピー
    //-----------------------------------------------------
    // NTRカードアプリARM9コードのロード領域とメモリがかち合うが、先頭0x4000はセキュア領域で別バッファに格納されるので、
    // ここでこれらのパラメータをロードしても大丈夫。
    SYSMi_CopyLCFGDataSettings();

    //-----------------------------------------------------
    // 無線ON/OFFフラグをもとに、LEDを設定する。
    //-----------------------------------------------------
    {
        PMWirelessLEDStatus enable;
        int retry = SYSM_PM_RETRY_NUM;
        if( LCFG_THW_IsForceDisableWireless() ) {
            enable = PM_WIRELESS_LED_OFF;
        }else {
            enable = LCFG_TSD_IsAvailableWireless() ? PM_WIRELESS_LED_ON : PM_WIRELESS_LED_OFF;
        }
        while( retry-- > 0 ) {
            if ( PMi_SetWirelessLED( enable ) == PM_RESULT_SUCCESS ) {
                break;
            }
            OS_Sleep(1);
        }
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

#ifndef SYSM_NO_LOAD

    // RTC補正
    SYSMi_WriteAdjustRTC();
    // RTC値のチェック
    SYSMi_CheckRTC();

#endif // SYSM_NO_LOAD

    //-----------------------------------------------------
    // ARM7の処理待ち
    //-----------------------------------------------------

    // ARM7のランチャーパラメータ取得が完了するのを待つ
    while( !SYSMi_GetWork()->flags.arm7.isARM9Start ) {
        SVC_WaitByLoop( 0x1000 );
    }

#ifndef SYSM_NO_LOAD
//#ifdef DEBUG_USED_CARD_SLOT_B_
    // ARM7のカードチェック完了を待つ
    while( !SYSMi_GetWork()->flags.hotsw.is1stCardChecked ) {
        SVC_WaitByLoop( 0x1000 );
    }
//#endif
#endif // SYSM_NO_LOAD

    //-----------------------------------------------------
    // ランチャーパラメータの判定
    //-----------------------------------------------------
    if( SYSM_IsHotStart() ) {
        // ホットスタート時は、基本ロゴデモスキップ
        SYSM_SetLogoDemoSkip( TRUE );

#if 0
        if( !SYSM_IsRunOnDebugger() && LCFG_TSD_GetLastTimeBootSoftPlatform() == PLATFORM_CODE_NTR ) {
            // 前回ブートがNTRなら、ランチャーパラメータ無効
            SYSM_SetValidLauncherParam(FALSE);
            MI_CpuClear32( &SYSMi_GetWork()->launcherParam, sizeof(LauncherParam) );
        }
#endif

        if( SYSMi_GetWork()->flags.arm7.isValidLauncherParam ) {
            // ロゴデモスキップ無効？
            if( !SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ) {
                SYSM_SetLogoDemoSkip( FALSE );
            }

            // アプリ直接起動の指定があったらロゴデモを飛ばして指定アプリ起動
            if( SYSM_GetLauncherParamBody()->v1.bootTitleID ) {
                if(SYSM_GetLauncherParamBody()->v1.bootTitleID != TITLE_ID_CARD_BOOT)
                {
                    s_bootTitleBuf.titleID = SYSM_GetLauncherParamBody()->v1.bootTitleID;
                    s_bootTitleBuf.flags = SYSM_GetLauncherParamBody()->v1.flags;
                    s_bootTitleBuf.pBanner = (TWLBannerFile *)(*(TWLBannerFile **)(SYSM_GetLauncherParamBody()->v1.rsv));
                    pBootTitle = &s_bootTitleBuf;
                }
                else
                {
                    pBootTitle = SYSMi_ShortcutCardBootSub();
                }
            }
        }
    }

#if 0
    // アプリジャンプでないときには、アプリ間パラメタをクリア
    // ※あらかじめNTRカードのセキュア領域を退避せずに直接0x2000000からロードしている場合も容赦なく消すので注意
    if( !pBootTitle )
    {
        MI_CpuClearFast((void *)HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_SIZE);
    }

    //-----------------------------------------------------
    // 量産工程用ショートカットキー or
    // 検査カード起動
    //-----------------------------------------------------
    if( pBootTitle == NULL ) {
        // ココまでダイレクトブートが設定されていない場合のみ判定
        pBootTitle = SYSMi_CheckShortcutBoot1();
    }

    //-----------------------------------------------------
    // その他のショートカット起動
    //-----------------------------------------------------
    if( pBootTitle == NULL ) {
        // ココまでダイレクトブートが設定されていない場合のみ判定
        pBootTitle = SYSMi_CheckShortcutBoot2();
    }
#endif
    return pBootTitle;
}


// HWInfoのメモリ展開。
static void SYSMi_CopyLCFGDataHWInfo( u32 dst_addr )
{
    // HotStart時にも保持する必要のあるデータをランチャー用に移動するプリロードパラメータバッファにコピー。
    MI_CpuCopy8( (void *)HW_PARAM_WIRELESS_FIRMWARE_DATA, (void *)(dst_addr + HW_PARAM_TWL_SETTINGS_DATA_SIZE),
                 HW_PARAM_WIRELESS_FIRMWARE_DATA_SIZE );    // 無線ファーム用

    // プリロードパラメータアドレスをランチャー向けに変更。
    *(u32 *)HW_PRELOAD_PARAMETER_ADDR = dst_addr;

    // HWノーマル情報、HWセキュア情報をメモリに展開しておく
#ifndef SYSM_NO_HWINFO
    MI_CpuCopyFast( LCFGi_GetHWN(), (void *)HW_PARAM_TWL_HW_NORMAL_INFO, sizeof(LCFGTWLHWNormalInfo) );
    MI_CpuCopyFast( LCFGi_GetHWS(), (void *)HW_HW_SECURE_INFO, HW_HW_SECURE_INFO_END - HW_HW_SECURE_INFO );
#endif // SYSM_NO_HWINFO
}


// 本体設定データのメモリ展開。
static void SYSMi_CopyLCFGDataSettings( void )
{
    // 本体設定データ
    MI_CpuCopyFast( LCFGi_GetTSD(), (void *)HW_PARAM_TWL_SETTINGS_DATA, sizeof(LCFGTWLSettingsData) );

    // 本体設定データのLauncherStatus部分をクリアしておく
    {
        LCFGTWLSettingsData *pSettings = (LCFGTWLSettingsData *)HW_PARAM_TWL_SETTINGS_DATA;
        MI_CpuClear32( &pSettings->launcherStatus, sizeof(LCFGTWLLauncherStatus) );
    }

    // NTR本体設定データをメモリに展開しておく
    {
        LCFG_NSD_SetLanguage( LCFG_NSD_GetLanguageEx() );
        MI_CpuCopy8( LCFGi_GetNSD(), OS_GetSystemWork()->nvramUserInfo, sizeof(LCFGNTRSettingsData) );
    }
}



BOOL SYSM_IsLauncherHidden( void )
{
#ifdef SYSM_DO_NOT_SHOW_LAUNCHER
    return TRUE;
#else
    return FALSE;
#endif
}


static TitleProperty * SYSMi_ShortcutCardBootSub( void )
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
    if( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL ) {
        s_bootTitleBuf.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
    }else{
        // NTRアプリの時は、TitleIDがないので、GameCodeをいじって擬似的にTitleIDとする。
        s_bootTitleBuf.titleID = (u64)( ( SYSM_GetCardRomHeader()->game_code[ 3 ] <<  0 ) |
                                        ( SYSM_GetCardRomHeader()->game_code[ 2 ] <<  8 ) |
                                        ( SYSM_GetCardRomHeader()->game_code[ 1 ] << 16 ) |
                                        ( SYSM_GetCardRomHeader()->game_code[ 0 ] << 24 ) );
    }
    SYSM_SetLogoDemoSkip( s_bootTitleBuf.flags.isLogoSkip );
    return &s_bootTitleBuf;
}

// ショートカット起動のチェックその１
static TitleProperty *SYSMi_CheckShortcutBoot1( void )
{
    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );

    if( SYSM_IsExistCard() ) {
	    //-----------------------------------------------------
	    // 量産工程用ショートカットキー or
	    // 検査カード起動
	    //-----------------------------------------------------
        if( LCFG_THW_IsForceLogoDemoSkip() ||
            SYSM_IsInspectCard() ||
            ( ( PAD_Read() == SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) &&
              ( !LCFG_TSD_IsFinishedBrokenTWLSettings() || !LCFG_TSD_IsFinishedInitialSetting() || !LCFG_TSD_IsFinishedInitialSetting_Launcher() ) )
            ){
            return SYSMi_ShortcutCardBootSub();
        }
	    //-----------------------------------------------------
	    // ISデバッガ起動 or
	    // ISデバッガバナーViewモード起動
	    //-----------------------------------------------------
        if( SYSM_IsRunOnDebugger() &&      // ISデバッガが有効かつJTAGがまだ有効でない時
            !( *(u8 *)( HW_SYS_CONF_BUF + HWi_WSYS09_OFFSET ) & HWi_WSYS09_JTAG_CPUJE_MASK )
            ){
			if( SYSMi_IsDebuggerBannerViewMode() ) {
	        	return NULL;							// バナービューモード時は、通常起動でランチャーメニューを表示
			}else {
				return SYSMi_ShortcutCardBootSub();
			}
        }
    }

    return NULL;                                                    // 「ブート内容未定」でリターン
}

// ショートカット起動のチェックその２
static TitleProperty *SYSMi_CheckShortcutBoot2( void )
{
    BOOL isSetArgument = FALSE;
    BOOL isBootMSET = FALSE;
    u16 argument = 0;
#ifndef SYSM_DISABLE_INITIAL_SETTINGS
	int i;
	char *p = (char *)LCFG_TSD_GetTPCalibrationPtr();
#endif

    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );

#ifndef SYSM_DISABLE_INITIAL_SETTINGS
	// タッチパネルキャリブレーションデータがクリアされていないかチェック
	for( i = 0; i < sizeof(LCFGNTRTPCalibData); i++ ) {
		if( *p != 0 ) break;
		p++;
	}
    //-----------------------------------------------------
    // TWL設定データ破損時のフラッシュ壊れシーケンス起動
    //-----------------------------------------------------
    if( ( i == sizeof(LCFGNTRTPCalibData) ) ||
		!LCFG_TSD_IsFinishedBrokenTWLSettings() ) {
        argument      = 100;        // フラッシュ壊れシーケンス起動
        isSetArgument = TRUE;
        isBootMSET    = TRUE;
    }else
#endif
    //-----------------------------------------------------
    // L+R+Startボタン押下起動で、本体設定のタッチパネル設定を起動
    //-----------------------------------------------------
    if( ( PAD_Read() & SYSM_PAD_SHORTCUT_TP_CALIBRATION ) ==
        SYSM_PAD_SHORTCUT_TP_CALIBRATION ) {
        argument      = 101;
        isSetArgument = TRUE;
        isBootMSET    = TRUE;
    }
#ifndef SYSM_DISABLE_INITIAL_SETTINGS
    //-----------------------------------------------------
    // TWL設定データ未設定時の初回起動シーケンス起動
    //-----------------------------------------------------
    else if( !LCFG_TSD_IsFinishedInitialSetting() ) {
        argument      = 0;
        isSetArgument = FALSE;
        isBootMSET    = TRUE;
    }
#endif

    //-----------------------------------------------------
    // ランチャー画面を表示しないバージョンの場合
    // カードがささっていたらカードを起動する
    // ささっていない場合は本体設定を起動
    //-----------------------------------------------------
#ifdef SYSM_DO_NOT_SHOW_LAUNCHER
    else if( SYSM_IsExistCard() )
    {
        return SYSMi_ShortcutCardBootSub();
    }else
    {
        argument      = 0;
        isSetArgument = FALSE;
        isBootMSET    = TRUE;
    }
#endif

    // 「アプリ間パラメータセット」有効時は、パラメータをセット
    if( isSetArgument ) {
        OSDeliverArgInfo argInfo;
        int result;

        OS_InitDeliverArgInfo(&argInfo, 0);
        OS_DecodeDeliverArg();
        OSi_SetDeliverArgState( OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE );
        result = OS_SetSysParamToDeliverArg( (u16)argument );

        if(result != OS_DELIVER_ARG_SUCCESS )
        {
            OS_Warning("Failed to Set DeliverArgument.");
            return FALSE;
        }
        OS_EncodeDeliverArg();
    }

#ifndef SYSM_NO_ES

    // 「本体設定ブート」有効時は、本体設定プート決定
    if( isBootMSET ) {
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

#endif // SYSM_NO_ES

    return NULL;                                                    // 「ブート内容未定」でリターン
}


// NAM_Initされるようになったので、NAMで本体設定のID取得
// それらしきものがインストールされていない場合は0（NULL）をリターン
static OSTitleId SYSMi_getTitleIdOfMachineSettings( void )
{
    OSTitleId ret = NULL;

#ifndef SYSM_NO_ES

    int l;
    int getNum;
    int validNum = 0;
    NAMTitleId *pTitleIDList = NULL;
    ROM_Header_Short *header = ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;

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

#endif // SYSM_NO_ES

    return ret;
}

//======================================================================
//  デバッグ
//======================================================================
