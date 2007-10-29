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
#include "sysmenu_define.h"
#include "sysmenu_card.h"
#include "spi.h"
#include "mb_child.h"

// define data-----------------------------------------------------------------

#define SCREEN_RED						0
#define SCREEN_YELLOW					1

typedef struct BannerCheckParam {
	u8		*srcp;
	u32		size;
}BannerCheckParam;


typedef struct TitleProperty {	// この情報は、ランチャー時には認証通ってないけど、起動時には認証通すので大丈夫だろう。
	u64		titleID;		// アプリケーション識別ID
	u32		platform;		// NTR, TWL  (HYBLIDはTWLを返す）
	void	*pBanner;		// 固定長フォーマットなら偽造されても大丈夫だろう。
}TitleProperty;

typedef enum AuthState {
	AUTH_PROCESSING = 0,
	AUTH_RESULT_SUCCEEDED = 1,
	AUTH_RESULT_TITLE_POINTER_ERROR = 2,
	AUTH_RESULT_AUTHENTICATE_FAILED = 3,
	AUTH_RESULT_ENTRY_ADDRESS_ERROR = 4
}AuthState;

// extern data-----------------------------------------------------------------
extern void ReturnFromMain( void );
extern void	BootFuncEnd( void );

FS_EXTERN_OVERLAY( ipl2_data );
FS_EXTERN_OVERLAY( bm_mainp );

// function's prototype-------------------------------------------------------
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );
AuthState SYSM_AuthAndLoadTitle( TitleProperty *pBootTitle );
void SYSM_Finalize( void );


static void INTR_SubpIRQ( void );

static void SYSMi_CheckCardLoadAddressAndSize( void );
static void LoadRomRegSizeAdjust( CARDRomRegion *romRegp, u32 load_limit_lo, u32 load_limit_hi );
static void SYSMi_ReadyBootNitroGame( void );
static BOOL SYSMi_CheckARM7LoadNITROCard( void );
static void SYSMi_MainpRegisterAndRamClear( BOOL isPlatformTWL );
static void ClearMemory( int addr1, int addr2 );

static void SYSMi_CopyInfoFromIPL1( void );

static void SYSMi_ReadNTRSetting( void );
static void SYSMi_ReadTWLSetting( void );
static void SYSMi_VerifyNTRSetting( void );
static void SYSMi_CheckEntryAddress( void );
static void SYSMi_CaribrateTP( void );
static void SYSMi_WriteAdjustRTC( void );
static BOOL	SYSMi_SendMessageToARM7( u32 msg );
static BOOL SYSMi_CheckNitroCardRightly( void );
static int  SYSMi_ExistCard( void );
static u32  SYSMi_SelectBootType( void );
static void SYSMi_DispInitialDebugData( void );
static void SYSMi_DispDebugData( void );

static void DispSingleColorScreen( int mode );

static void SYSMi_ReadCardBannerFile( void );
static void SYSMi_CheckCardCloneBoot( void );


// global variable-------------------------------------------------------------
#ifdef __SYSM_DEBUG
SharedWork		*swp;												// デバッガでのIPL1SharedWorkのウォッチ用
SYSM_work		*pSysm;											// デバッガでのSYSMワークのウォッチ用
NitroConfigData *ncdp;												// デバッガでのNCデータ　のウォッチ用
#endif

// static variable-------------------------------------------------------------
static BOOL			s_isBanner = FALSE;
static BannerFile	s_bannerBuf;

// const data------------------------------------------------------------------

static BannerCheckParam s_bannerCheckList[ BNR_VER_MAX ] = {
	{ (u8 *)&s_bannerBuf.v1, sizeof( BannerFileV1 ) },
	{ (u8 *)&s_bannerBuf.v2, sizeof( BannerFileV2 ) },
	{ (u8 *)&s_bannerBuf.v3, sizeof( BannerFileV3 ) },
};

#ifdef __DEBUG_SECURITY_CODE
static GXRgb security_detection_color[] = { GX_RGB( 31,  0,  0 ),
											GX_RGB( 31, 31,  0 ), };
#endif

// inline functions------------------------------------------------------------

static inline void DBG_SetRed(u32 y_pos)
{
	*(u16 *)(HW_DB_BG_VRAM + 0xf000 + 0x20*2*y_pos) = (1<<12) | 0x100;
	MI_CpuFill16(((u8 *)HW_DB_BG_VRAM + 0x20*0x100), 0x1111, 0x20);
}

// ============================================================================
// function's description
// ============================================================================

// SystemMenuの初期化
void SYSM_Init( void )
{
#ifdef __SYSM_DEBUG
	pSysm = GetSYSMWork();
	ncdp  = GetNCDWork();
//	SYSMi_DispInitialDebugData();									// 初期デバッグ情報表示
#endif /* __SYSM_DEBUG */

	// WRAM設定はいる？
//	MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
//	MI_SetWramBank(MI_WRAM_ARM7_ALL);
	
	SVC_CpuClearFast(0x0000, (u16 *)GetSYSMWork(), sizeof(SYSM_work));	// SYSMワークのクリア
	
	// ※ISデバッガかどうかの判定。　BootROMからのパラメータ引渡し？
}


// ARM7側の初期化待ち
BOOL SYSM_WaitARM7Init( void )
{
/*	while( !( SYSM_GetBootFlag() & BFLG_ARM7_INIT_COMPLETED ) ) {
		SVC_WaitByLoop(0x1000);										// ARM7の初期化が終了するのを待つ。
	}
*/
	reg_OS_PAUSE |= REG_OS_PAUSE_CHK_MASK;							// PAUSEレジスタのチェックフラグのセット
	
	SYSMi_ReadNTRSetting();											// NOR からNTR本体設定データをリード
	SYSMi_ReadTWLSetting();											// NANDからTWL本体設定データをリード
	PMm_SetBackLightBrightness();
	SYSMi_CaribrateTP();											// 読み出したTWL本体設定データをもとにTPキャリブレーション。
	SYSMi_WriteAdjustRTC();											// 読み出したTWL本体設定データをもとにRTCクロック補正値をセット。
	
	SYSMi_VerifyNTRSetting();										// NVRAMのNTR本体設定データをリードし、不一致箇所があればNTR側をリカバリ。
	
	SYSMi_CheckCardCloneBoot();										// カードがクローンブートかチェック
	SYSMi_ReadCardBannerFile();										// カードバナーファイルの読み出し。
	
	// ==============================================================
	// デバッガ対応コード
#ifdef __IS_DEBUGGER_BUILD
	if( GetSYSMWork()->isOnDebugger ) {
		if( !SYSM_IsDebuggerBannerViewMode() ){						// デバッガ上動作の場合は、この中でカードブートまでやってしまう。
			
			( void )OS_EnableIrqMask( OS_IE_V_BLANK );
			( void )OS_EnableIrq();
			( void )OS_EnableInterrupts();
			( void )GX_VBlankIntr( TRUE );
			
			if( SYSMi_ExistCard() ) {
				( void )SYSM_Main();
				SYSM_SetBootFlag( BFLG_BOOT_CARD );
				( void )SYSM_BootCARD();
				SYSM_PermitToBootSelectedTarget();
				while(1) {
					OS_WaitIrq( 1, OS_IE_V_BLANK );					// Vブランク割込終了待ち
					if( SYSM_Main() ) {								// システムのメイン
						return TRUE;								// TRUEが帰ってきたらメインループからリターン（カード起動）
					}
				}
			}
		}
	}else {
		while( 1 ) {}												// ISデバッガビルドでISデバッガが検出できなかったら停止。
	}
#endif // __IS_DEBUGGER_BUILD
	// ==============================================================

	return FALSE;
}


// 指定タイトルがブート可能なポインタかチェック
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle )
{
#pragma unused( pBootTitle )
	
	return TRUE;
}


// 指定タイトルの認証＆ロード　※１フレームじゃ終わらん。
AuthState SYSM_AuthAndLoadTitle( TitleProperty *pBootTitle )
{
#pragma unused( pBootTitle )
	// メインメモリのクリア
	// DSダウンロードプレイの時は、ROMヘッダを退避する
	// アプリロード
	// アプリ認証
	
	// エントリアドレスの正当性をチェックし、無効な場合は無限ループに入る。
	SYSMi_CheckEntryAddress();
	
	return AUTH_RESULT_SUCCEEDED;
}


// ブートのための終了処理
void SYSM_Finalize( void )
{
	// ARM7へのブート通知
	// レジスタ・RAMクリア
	
	// ※ブート時にプロテクションユニットをOFFにしなければ、不正なアドレスでの起動を防げるのでは？
	
	u32 i;
	
	// ブートの前準備
	MI_CpuCopyFast( (void *)ReturnFromMain, (void *)RETURN_FROM_MAIN_ARM9_FUNCP, (u32)( (u32)BootFuncEnd - (u32)ReturnFromMain ) );
	DC_StoreRange ( (void *)ReturnFromMain, 0x200 );		// ゲームブート時の最終処理をメインメモリの後ろの方にコピー（※SYSM実行時のスタック上昇で破壊されないように、このタイミングでコピーする。）
	
	for( i = 0; i <= MI_DMA_MAX_NUM; i++ ) {				// DMAの停止
		MI_StopDma( i );
	}
	SYSM_FinalizeCardPulledOut();							// カード抜け検出終了処理
	SYSMi_MainpRegisterAndRamClear( TRUE );					// レジスタ＆RAMクリア
	( void )GX_VBlankIntr(FALSE);
	( void )OS_SetIrqFunction(OS_IE_SUBP, INTR_SubpIRQ);
	( void )OS_SetIrqMask(OS_IE_SUBP);						// サブプロセッサ割り込みのみを許可。
	reg_PXI_SUBPINTF = SUBP_RECV_IF_ENABLE | 0x0f00;		// ARM9ステートを "0x0f" に
	GetSYSMWork()->mainp_state = MAINP_STATE_WAIT_BOOT_REQ;
															// ※もうFIFOはクリア済みなので、使わない。
	// ARM7からの通知待ち
	OS_WaitIrq(1, OS_IE_SUBP);								// SVC_WaitIntr(0,OS_IE_SUBP);から変更。
	
	// 割り込みをクリアして最終ブートシーケンスへ。
	reg_PXI_SUBPINTF &= 0x0f00;								// サブプロセッサ割り込み許可フラグをクリア
	( void )OS_DisableIrq();
	( void )OS_SetIrqMask(0);
	( void )OS_ResetRequestIrqMask( (u32)~0 );
}


#if 0
// NITRO起動をARM7に通知
BOOL SYSM_BootCard( void )
{																	// Nintendoロゴチェックは、このタイミングで行う。

	( void )SYSMi_SendMessageToARM7(MSG_BOOT_TYPE_CARD);	// ARM7にカード起動を通知。

	if( SYSM_CheckNinLogo( (u16 *)GetRomHeaderAddr()->nintendo_logo ) == FALSE
	 || GetSYSMWork()->enableCardNormalOnly == TRUE ) {	// NORMALカード非対応化
		SYSM_SetBootFlag( BFLG_ILLEGAL_NITRO_CARD );
		return FALSE;
	}else {
		SYSM_SetBootFlag( BFLG_BOOT_DECIDED | BFLG_BOOT_NITRO );
		return TRUE;
	}
}
#endif


// TPリード可能かどうかを調べる。
BOOL SYSM_IsTPReadable( void )
{
	if( SYSM_GetBootFlag() & BFLG_BOOT_DECIDED )	return FALSE;
	else											return TRUE;
}


// ARM7-ARM9共有リソースのbootFlagへの値のセット
void SYSM_SetBootFlag( u32 value )
{
	BOOL preIrq = OS_DisableIrq();
	LockVariable *lockp = &GetSYSMWork()->boot_flag;
	( void )OS_LockByWord(  BOOTFLAG_LOCK_ID, &(lockp->lock), (void (*)( void ))0x00000000);
	lockp->value |=  value;
	( void )OS_UnLockByWord(BOOTFLAG_LOCK_ID, &(lockp->lock), (void (*)( void ))0x00000000);
	( void )OS_RestoreIrq( preIrq );
}


void SYSM_ClearBootFlag( u32 value )
{
	BOOL preIrq = OS_DisableIrq();
	LockVariable *lockp = &GetSYSMWork()->boot_flag;
	( void )OS_LockByWord(  BOOTFLAG_LOCK_ID, &(lockp->lock), (void (*)( void ))0x00000000);
	lockp->value &= ~value;
	( void )OS_UnLockByWord(BOOTFLAG_LOCK_ID, &(lockp->lock), (void (*)( void ))0x00000000);
	( void )OS_RestoreIrq( preIrq );
}


// ============================================================================
// 割り込み処理
// ============================================================================

// サブプロセッサ割り込み
static void INTR_SubpIRQ( void )
{
	OS_SetIrqCheckFlag( OS_IE_SUBP );
}


// ============================================================================
// アプリ起動処理
// ============================================================================

// エントリアドレスの正当性チェック
static void SYSMi_CheckEntryAddress( void )
{
	// エントリアドレスがROM内登録エリアかAGBカートリッジエリアなら、無限ループに入る。
	if(   !(   ( (u32)GetRomHeaderAddr()->main_entry_address >= HW_MAIN_MEM              )
			&& ( (u32)GetRomHeaderAddr()->main_entry_address <  SYSM_ARM9_MMEM_ENTRY_ADDR_LIMIT ) )
	   || !(    (   ( (u32)GetRomHeaderAddr()->sub_entry_address  >= HW_MAIN_MEM      )
			     && ( (u32)GetRomHeaderAddr()->sub_entry_address  <  SYSM_ARM7_LOAD_MMEM_LAST_ADDR ) )
			 || (   ( (u32)GetRomHeaderAddr()->sub_entry_address  >= HW_WRAM    )
				 && ( (u32)GetRomHeaderAddr()->sub_entry_address  <  SYSM_ARM7_LOAD_WRAM_LAST_ADDR ) ) ) )
	{
		OS_TPrintf("entry address invalid.\n");
#ifdef __DEBUG_SECURITY_CODE
		DispSingleColorScreen( SCREEN_YELLOW );
#endif
		while( 1 ) {}
	}
	OS_TPrintf("entry address valid.\n");
}


// ARM7によるNITROゲームのロード完了を確認する。
static BOOL SYSMi_CheckARM7LoadNITROCard( void )
{
	if( SYSMi_ExistCard()
//		&& !( SYSM_GetBootFlag() & BFLG_LOAD_CARD_COMPLETED )
		) {
		return FALSE;
	}
	return TRUE;
}



// SystemMenuで使用したレジスタ＆メモリのクリア
static void SYSMi_MainpRegisterAndRamClear( BOOL isPlatformTWL )
{
	// 最後がサブプロセッサ割り込み待ちなので、IMEはクリアしない。
	( void )OS_SetIrqMask(0);
	( void )OS_ResetRequestIrqMask( (u32)~0 );
	
	// メモリクリア
	GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);							// VRAM     クリア
	MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
	( void )GX_DisableBankForLCDC();
//	MI_CpuClearFast((void *)HW_ITCM,		HW_ITCM_SIZE);			// ITCM     クリア  ※ITCMにはSDKのコードが入っているので、gameBoot.cでクリアする。
//	MI_CpuClearFast((void *)HW_DTCM,		HW_DTCM_SIZE-0x800);	// DTCM     クリア	※DTCMはスタック&SDK変数入りなので、最後にgameBoot.cでクリアしている。
	MI_CpuClearFast((void *)HW_OAM,			HW_OAM_SIZE);			// OAM      クリア
	MI_CpuClearFast((void *)HW_PLTT,		HW_PLTT_SIZE);			// パレット クリア
	MI_CpuClearFast((void *)HW_DB_OAM,		HW_DB_OAM_SIZE);		// OAM      クリア
	MI_CpuClearFast((void *)HW_DB_PLTT,		HW_DB_PLTT_SIZE);		// パレット クリア
	
	// レジスタクリア
	MI_CpuClearFast((void*)(HW_REG_BASE + 0x8),    0x12c);			// BG0CNT    ～ KEYCNT
	MI_CpuClearFast((void*)(HW_REG_BASE + 0x280),  0x40);			// DIVCNT    ～ SQRTD3
	MI_CpuClearFast((void*)(HW_REG_BASE + 0x1000), 0x6e);			// DISP1CNT1 ～ DISPBRTCNT1
	CP_SetDiv32_32( 0, 1 );
	reg_PXI_SUBP_FIFO_CNT	= 0x4008;
	reg_GX_DISPCNT			= 0;
	reg_GX_DISPSTAT			= 0;									// ※ reg_GX_VCOUNTはベタクリアできないので、この先頭部分のクリアを分離する。
	
	
	// NTRの時には、バナーがある時は、MCCNTのカードイネーブルビットが"1"で、無いときには"0"になっていたが、
	// NTR起動の時には、ここでもそれを踏襲しないとダメかも。。。
	
	// クリアしていないレジスタは、VCOUNT, PIFCNT, MC-, EXMEMCNT, IME, RBKCNT1, PAUSE, POWLCDCNT, 全3D系です。
	if( isPlatformTWL ) {
		// TWL専用レジスタのクリア
	}
}


// ============================================================================
// サブルーチン
// ============================================================================

// ISデバッガのバナービューモード起動かどうか？
BOOL SYSM_IsDebuggerBannerViewMode( void )
{
#ifdef __IS_DEBUGGER_BUILD
	return ( GetSYSMWork()->isOnDebugger &&
			 SYSMi_ExistCard() &&
			 GetRomHeaderAddr()->dbgRomSize == 0 ) ? TRUE : FALSE;
#else
	return FALSE;
#endif	// __IS_DEBUGGER_BUILD
}


// NITRO設定データの読み出し。
static void SYSMi_ReadNTRSetting( void )
{
	RTCDate date;
	RTCTime	time;
	
	GetSYSMWork()->ncd_invalid = NVRAMm_ReadNitroConfigData( GetNCDWork() );
																	// NVRAMからNITRO設定データをロード。
	if( GetSYSMWork()->ncd_invalid ) {								// リードしたNITRO設定データが無効だったら、0クリアしたものを使用。
		OS_TPrintf(" NCD destroyed.\n" );
		SVC_CpuClearFast( 0x0000, GetNCDExWork(), sizeof(NitroConfigDataEx) );
		GetNCDExWork()->version					= NITRO_CONFIG_DATA_EX_VERSION;
		GetNCDWork()->option.backLightBrightness= 2;				// 出荷時と同じ値に。
		GetNCDWork()->option.language			= LANG_ENGLISH;		// プリセットが必要なデータはプリセットしておく
		GetNCDWork()->option.destroyFlashFlag	= 1;
		GetNCDWork()->owner.birthday.month		= 1;
		GetNCDWork()->owner.birthday.day		= 1;
		GetNCDExWork()->valid_language_bitmap	= VALID_LANG_BITMAP;
	}
	
	// RTCのリセット or おかしい値を検出した場合は初回起動シーケンスへ。
	( void )RTC_GetDateTime( &date, &time );
	if( !SYSM_CheckRTCDate( &date ) ||
	    !SYSM_CheckRTCTime( &time )
#ifndef __IS_DEBUGGER_BUILD											// 青デバッガではRTCの電池がないので、毎回ここにひっかかって設定データが片方クリアされてしまう。これを防ぐスイッチ。
		|| ( GetSYSMWork()->rtcStatus & 0x01 )
#endif
		) {							// RTCの異常を検出したら、rtc入力フラグ＆rtcOffsetを0にしてNVRAMに書き込み。
		OS_TPrintf("\"RTC reset\" or \"Illegal RTC data\" detect!\n");
		GetNCDWork()->option.input_rtc		= 0;
		GetNCDWork()->option.rtcOffset		= 0;
		GetNCDWork()->option.rtcLastSetYear = 0;
		( void )NVRAMm_WriteNitroConfigData( GetNCDWork() );
	}
}


// TWL設定データの読み出し
static void SYSMi_ReadTWLSetting( void )
{
	
}


// NTR設定とTWL設定をベリファイして、不一致があれば、NTR設定を更新
static void SYSMi_VerifyNTRSetting( void )
{
}


// RTCの日付が正しいかチェック
BOOL SYSM_CheckRTCDate( RTCDate *datep )
{
	if(	 ( datep->year >= 100 )
	  || ( datep->month < 1 ) || ( datep->month > 12 )
	  || ( datep->day   < 1 ) || ( datep->day   > 31 )
	  || ( datep->week >= RTC_WEEK_MAX ) ) {
		return FALSE;
	}
	return TRUE;
}


// RTCの時刻が正しいかチェック
BOOL SYSM_CheckRTCTime( RTCTime *timep )
{
	if(  ( timep->hour   > 23 )
	  || ( timep->minute > 59 )
	  || ( timep->second > 59 ) ) {
		return FALSE;
	}
	return TRUE;
}


// バナーファイルの読み込みの実体
static void SYSMi_ReadCardBannerFile( void )
{
	s32 lockCardID;
	BannerFile *pBanner = &s_bannerBuf;
	
	if( ( !SYSMi_ExistCard() ) || ( *(void** )BANNER_ROM_OFFSET == NULL ) ) {
		s_isBanner = FALSE;
		return;
	}
	
	// ROMカードからのバナーデータのリード
	if ( ( lockCardID = OS_GetLockID() ) > 0 ) {
		( void )OS_LockCard( (u16 )lockCardID );
		DC_FlushRange( pBanner, sizeof(BannerFile) );
		SYSM_ReadCard(*(void** )BANNER_ROM_OFFSET, pBanner, sizeof(BannerFile) );
		( void )OS_UnLockCard( (u16 )lockCardID );
		OS_ReleaseLockID( (u16 )lockCardID );
	}
	
	// バナーデータの正誤チェック
	{
		int i;
		u16 calc_crc = 0xffff;
		u16 *hd_crcp = (u16 *)&pBanner->h.crc16_v1;
		BannerCheckParam *chkp = &s_bannerCheckList[ 0 ];
		
		s_isBanner  = TRUE;
		
		for( i = 0; i < BNR_VER_MAX; i++ ) {
			if( i < pBanner->h.version ) {
			    calc_crc = SVC_GetCRC16( calc_crc, chkp->srcp, chkp->size );
				if( calc_crc != *hd_crcp++ ) {
					s_isBanner =  FALSE;
					break;
				}
			}else {
				MI_CpuClear16( chkp->srcp, chkp->size );
			}
			chkp++;
		}
		if( !s_isBanner ) {
			MI_CpuClear16( &s_bannerBuf, sizeof(BannerFile) );
		}
	}
}


// クローンブート判定
static void SYSMi_CheckCardCloneBoot( void )
{
	s32	lockCardID;
	u8 	*buffp         = (u8 *)&s_bannerBuf;		// バナー用バッファをテンポラリとして使用
	u32 total_rom_size = GetRomHeaderAddr()->total_rom_size ? GetRomHeaderAddr()->total_rom_size : 0x01000000;
	u32 file_offset    = total_rom_size & 0xFFFFFE00;
	
	if( !SYSMi_ExistCard() ) {
		return;
	}
	
	if ( ( lockCardID = OS_GetLockID() ) > 0 ) {
		( void )OS_LockCard( (u16 )lockCardID );
		DC_FlushRange( buffp, BNR_IMAGE_SIZE );
		SYSM_ReadCard( (void *)file_offset, buffp, BNR_IMAGE_SIZE );
		( void )OS_UnLockCard( (u16 )lockCardID );
		OS_ReleaseLockID( (u16 )lockCardID );
	}
	
	buffp += total_rom_size & 0x000001FF;
	if( *buffp++ == 'a' && *buffp == 'c' ) {
		GetSYSMWork()->clone_boot_mode = CLONE_BOOT_MODE;
	}else {
		GetSYSMWork()->clone_boot_mode = OTHER_BOOT_MODE;
	}
}


// タッチパネルキャリブレーション
static void SYSMi_CaribrateTP( void )
{
#ifndef __TP_OFF
	TPCalibrateParam calibrate;
	
	( void )TP_CalcCalibrateParam(&calibrate,							// タッチパネル初期化
			GetNCDWork()->tp.raw_x1, GetNCDWork()->tp.raw_y1, (u16)GetNCDWork()->tp.dx1, (u16)GetNCDWork()->tp.dy1,
			GetNCDWork()->tp.raw_x2, GetNCDWork()->tp.raw_y2, (u16)GetNCDWork()->tp.dx2, (u16)GetNCDWork()->tp.dy2 );
	TP_SetCalibrateParam(&calibrate);
	OS_Printf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			GetNCDWork()->tp.raw_x1, GetNCDWork()->tp.raw_y1, (u16)GetNCDWork()->tp.dx1, (u16)GetNCDWork()->tp.dy1,
			GetNCDWork()->tp.raw_x2, GetNCDWork()->tp.raw_y2, (u16)GetNCDWork()->tp.dx2, (u16)GetNCDWork()->tp.dy2 );
#endif
}


// RTCクロック補正値をセット
static void SYSMi_WriteAdjustRTC( void )
{	
#ifndef __IS_DEBUGGER_BUILD											// デバッガ用ビルド時は補正しない。
	RTCRawAdjust raw;
	
	raw.adjust = GetNCDWork()->option.rtcClockAdjust;			// ncd_invalid時にはrtcClockAdjustは
																	// 0クリアされているため補正機能は使用されない
	( void )RTCi_SetRegAdjust( &raw );
#endif /* __IS_DEBUGGER_BUILD */
}


// FIFO経由でARM7にメッセージ通知。※PXI_FIFO_TAG_USER_1を使用。
static BOOL	SYSMi_SendMessageToARM7(u32 msg)
{
#pragma unused(msg)
	return TRUE;
}


// NTR,TWLカード存在チェック 		「リターン　1：カード認識　0：カードなし」
static int SYSMi_ExistCard( void )
{
	if( ( GetRomHeaderAddr()->nintendo_logo_crc16 == 0xcf56 ) &&
	    ( GetRomHeaderAddr()->header_crc16 == GetSYSMWork()->cardHeaderCrc16) ) {
		return TRUE;												// NTR,TWLカードあり（NintendoロゴCRC、カードヘッダCRCが正しい場合）
																	// ※Nintendoロゴデータのチェックは、特許の都合上、ロゴ表示ルーチン起動後に行います。
	}else {
		return FALSE;												// NTR,TWLカードなし
	}
}


// Nintendoロゴチェック			「リターン　1:Nintendoロゴ認識成功　0：失敗」
BOOL SYSM_CheckNinLogo(u16 *logo_cardp)
{
	u16 *logo_orgp	= (u16 *)SYSROM9_NINLOGO_ADR;					// ARM9のシステムROMのロゴデータとカートリッジ内のものを比較
	u16 length		= NINTENDO_LOGO_LENGTH >> 1;
	
	while(length--) {
		if(*logo_orgp++ != *logo_cardp++) return FALSE;
	}
	return TRUE;
}


// スリープモードへの遷移
void SYSM_GoSleepMode( void )
{
#ifndef __IS_DEBUGGER_BUILD											// デバッガ用ビルド時はスリープしない。
	PM_GoSleepMode( (PMWakeUpTrigger)( (PAD_DetectFold() ? PM_TRIGGER_COVER_OPEN : 0) | PM_TRIGGER_RTC_ALARM ),
					0,
					0 );
#endif /* __IS_DEBUGGER_BUILD */
}


// バックライト輝度調整
void PMm_SetBackLightBrightness( void )
{
	( void )PMi_WriteRegister( 4, (u16)NCD_GetBackLightBrightness() );
	( void )PM_SetBackLight( PM_LCD_ALL, PM_BACKLIGHT_ON );
}


//======================================================================
//  デバッグ関数
//======================================================================

// 初期データのデバッグ表示
#ifdef __SYSM_DEBUG
static void SYSMi_DispInitialDebugData( void )
{
	OS_Printf("SYSM version      :20%x\n", SYSMENU_VER);
	if( GetMovedInfoFromIPL1Addr()->isOnDebugger )	OS_Printf("Run On IS-DEBUGGER\n");
	else 											OS_Printf("Run On IS-EMULATOR\n");
	if(GetMovedInfoFromIPL1Addr()->rtcStatus & 0x01)	OS_Printf("RTC reset is detected!\n");
	if(GetMovedInfoFromIPL1Addr()->rtcError)			OS_Printf("RTC error is detected!\n");
#if 0
	OS_Printf("NvDate       :%4d\n",sizeof(NvDate));
	OS_Printf("NvNickname   :%4d\n",sizeof(NvNickname));
	OS_Printf("NvComment    :%4d\n",sizeof(NvComment));
	OS_Printf("NvOwnerInfo  :%4d\n",sizeof(NvOwnerInfo));
	OS_Printf("NvAlarm      :%4d\n",sizeof(NvAlarm));
	OS_Printf("NvTpCalibData:%4d\n",sizeof(NvTpCalibData));
	OS_Printf("NvOption     :%4d\n",sizeof(NvOption));
	OS_Printf("NCD          :%4d\n",sizeof(NitroConfigData));
	OS_Printf("NCDStore     :%4d\n",sizeof(NCDStore));
#endif
#if 0
	{	// ROM_HEADER_BUFFの内容を書き出し
		int i,j;
		u32 *romhp = (u32 *)GetRomHeaderAddr();
		OS_Printf("ROM Header Buff\n  ");
		for(i = 0; i < 6; i++) {
			for(j = 0; j < 4; j++) OS_Printf("    0x%8x", *romhp++);
			OS_Printf("\n  ");
		}
		OS_Printf("\n");
	}
	{	// ROM_HEADER_BUFFの内容を書き出し
		int i,j;
		u32 *romhp = (u32 *)MB_CARD_ROM_HEADER_ADDRESS;
		OS_Printf("MB Card ROM Header Buff\n  ");
		for(i = 0; i < 6; i++) {
			for(j = 0; j < 4; j++) OS_Printf("    0x%8x", *romhp++);
			OS_Printf("\n  ");
		}
		OS_Printf("\n");
	}
#endif  /* 0 */
}
#endif /* __SYSM_DEBUG */



#ifdef __DEBUG_SECURITY_CODE
// セキュリティがきちんと働いているかを確認するデバッグコード
static void DispSingleColorScreen( int mode )
{
	( void )OS_DisableIrq();
	GX_LoadBGPltt  ( &security_detection_color[ mode ], 0, sizeof(GXRgb) );
	GXS_LoadBGPltt ( &security_detection_color[ mode ], 0, sizeof(GXRgb) );
	GX_DispOn();
	GXS_DispOn();
	GX_SetGraphicsMode ( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
	GXS_SetGraphicsMode( GX_BGMODE_0 );
	GX_SetMasterBrightness( 0 );
	GXS_SetMasterBrightness( 0 );
    GX_SetVisiblePlane ( GX_PLANEMASK_NONE );
    GXS_SetVisiblePlane( GX_PLANEMASK_NONE );
}
#endif


