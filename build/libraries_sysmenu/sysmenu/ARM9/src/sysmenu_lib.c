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
#include <sysmenu/boot/common/boot.h>
#include "sysmenu_define.h"
#include "spi.h"

// define data-----------------------------------------------------------------

typedef struct BannerCheckParam {
	u8		*srcp;
	u32		size;
}BannerCheckParam;

// extern data-----------------------------------------------------------------
extern void SYSM_SetMountInfo( NAMTitleId titleID );				// マウント情報のセット
extern void SYSM_SetBootSRLPath( NAMTitleId titleID );				// SRL起動パスのセット

// function's prototype-------------------------------------------------------
static BOOL SYSMi_IsDebuggerBannerViewMode( void );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );
static void SYSMi_WriteAdjustRTC( void );
static int  SYSMi_IsValidCard( void );
static void SYSMi_ReadCardBannerFile( void );
static BOOL SYSMi_CheckEntryAddress( void );
static void SYSMi_CheckCardCloneBoot( void );
static void SYSMi_CheckRTC( void );

static s32 ReadFile(FSFile* pf, void* buffer, s32 size);

// global variable-------------------------------------------------------------
void *(*SYSM_Alloc)( u32 size  );
void  (*SYSM_Free )( void *ptr );

#ifdef __SYSM_DEBUG
SYSM_work		*pSysm;											// デバッガでのSYSMワークのウォッチ用
#endif

// static variable-------------------------------------------------------------
static BOOL				s_isBanner = FALSE;
static NTRBannerFile	s_bannerBuf;

// const data------------------------------------------------------------------

static BannerCheckParam s_bannerCheckList[ NTR_BNR_VER_MAX ] = {
	{ (u8 *)&s_bannerBuf.v1, sizeof( BannerFileV1 ) },
	{ (u8 *)&s_bannerBuf.v2, sizeof( BannerFileV2 ) },
	{ (u8 *)&s_bannerBuf.v3, sizeof( BannerFileV3 ) },
};


// ============================================================================
//
// 初期化
//
// ============================================================================

// SystemMenuの初期化
void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
#ifdef __SYSM_DEBUG
	pSysm = SYSMi_GetWork();
#endif /* __SYSM_DEBUG */
	
    // ARM7コンポーネント用プロテクションユニット領域変更
    OS_SetProtectionRegion( 2, SYSM_OWN_ARM7_MMEM_ADDR, 512KB );
	
	// ARM9用ブートコード配置のため、アリーナHi位置を下げる
	OS_SetMainArenaHi( (void *)SYSM_OWN_ARM9_MMEM_ADDR_END );
	
	SYSM_SetAllocFunc( pAlloc, pFree );
	
	// WRAM設定はいる？
//	MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
//	MI_SetWramBank(MI_WRAM_ARM7_ALL);
	
	reg_OS_PAUSE |= REG_OS_PAUSE_CHK_MASK;							// PAUSEレジスタのチェックフラグのセット
}


// システムメニューライブラリ用メモリアロケータの設定
void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
	SYSM_Alloc = pAlloc;
	SYSM_Free  = pFree;
}


// ============================================================================
//
// 情報取得
//
// ============================================================================

// パラメータリード
void SYSM_ReadParameters( void )
{
	// ARM7のリセットパラメータ取得が完了するのを待つ
	while( !SYSMi_GetWork()->isARM9Start ) {
		SVC_WaitByLoop( 0x1000 );
	}
	
	// リセットパラメータの判定
	if( SYSM_GetResetParamBody()->v1.flags.isLogoSkip &&
		SYSMi_IsDebuggerBannerViewMode() ) {
		SYSM_SetLogoDemoSkip( TRUE );
	}
	
#if 0
	// アプリロード済みで再配置要求があるなら、再配置処理
	if( SYSM_GetResetParamBody()->v1.flags.isAppLoadCompleted &&
		SYSM_GetResetParamBody()->v1.flags.reqAppRelocate ) {
		// 再配置処理
	}
#endif
	
	// 本体設定データのリード
	if( SYSM_ReadTWLSettingsFile() ) {								// NANDからTWL本体設定データをリード
		SYSM_SetBackLightBrightness( (u8)TSD_GetBacklightBrightness() ); // 読み出したTWL本体設定データをもとにバックライト輝度設定
		SYSM_CaribrateTP();											// 読み出したTWL本体設定データをもとにTPキャリブレーション。
	}
//	SYSM_ReadHWInfo();												// NANDからHW情報をリード
	SYSMi_WriteAdjustRTC();											// RTCクロック補正値をセット。
	SYSMi_CheckRTC();
	
	SYSM_VerifyAndRecoveryNTRSettings();							// NTR設定データを読み出して、TWL設定データとベリファイし、必要ならリカバリ
	
//	SYSMi_CheckCardCloneBoot();										// カードがクローンブートかチェック
//	SYSMi_ReadCardBannerFile();										// カードバナーファイルの読み出し。
	
	//NAMの初期化
	//NAM_Init(AllocForNAM,FreeForNAM);

}


// カードタイトルの取得
int SYSM_GetCardTitleList( TitleProperty *pTitleList_Card )
{
#pragma unused( pTitleList_Card )
	return 0;
}


// 指定ファイルリード
static s32 ReadFile(FSFile* pf, void* buffer, s32 size)
{
    u8* p = (u8*)buffer;
    s32 remain = size;

    while( remain > 0 )
    {
        const s32 len = MATH_IMin(1024, remain);
        const s32 readLen = FS_ReadFile(pf, p, len);

        if( readLen < 0 )
        {
            return readLen;
        }
        if( readLen != len )
        {
            return size - remain + readLen;
        }

        remain -= readLen;
        p      += readLen;
    }

    return size;
}

#include <twl/ese.h>
ESTitleMeta dst[1];

// NANDタイトルリストの取得
int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size)
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, 等の条件を指定してタイトルリストを取得する。
	// とりあえずALL
	int l;
	int gotten;
	NAMTitleId titleIdArray[ LAUNCHER_TITLE_LIST_NUM - 1 ];
	static TWLBannerFile bannerBuf[ LAUNCHER_TITLE_LIST_NUM ];
	gotten = NAM_GetTitleList( titleIdArray, LAUNCHER_TITLE_LIST_NUM-1 );
	
	for(l=0;l<gotten;l++)
	{
		//ヘッダからバナーを読み込む
		FSFile  file[1];
		BOOL bSuccess;
		static const int PATH_LENGTH=1024;
		char path[PATH_LENGTH];
		static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
		s32 readLen;
		s32 offset;
		
		readLen = NAM_GetTitleBootContentPath(path, titleIdArray[l]);
		
		if(readLen != NAM_OK){
			OS_TPrintf("NAM_GetTitleBootContentPath failed %d,%lld,%d\n",l,titleIdArray[l],readLen);
		}
		
		bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);

		if( ! bSuccess )
		{
		OS_TPrintf("SYSM_GetNandTitleList failed: cant open file %s\n",path);
		    return -1;
		}
		
		// バナーデータオフセットを読み込む
		bSuccess = FS_SeekFile(file, 0x68, FS_SEEK_SET);
		if( ! bSuccess )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant seek file(0)\n");
			FS_CloseFile(file);
		    return -1;
		}
		readLen = FS_ReadFile(file, &offset, sizeof(offset));
		if( readLen != sizeof(offset) )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant read file\n");
			FS_CloseFile(file);
		    return -1;
		}
		
		bSuccess = FS_SeekFile(file, offset, FS_SEEK_SET);
		if( ! bSuccess )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant seek file(offset)\n");
			FS_CloseFile(file);
		    return -1;
		}
		readLen = ReadFile(file, &bannerBuf[l], (s32)sizeof(TWLBannerFile));
		if( readLen != (s32)sizeof(TWLBannerFile) )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant read file2\n");
			FS_CloseFile(file);
		    return -1;
		}
		
		FS_CloseFile(file);
	}
	for(l=gotten;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		// 念のため0にクリア
		titleIdArray[l] = 0;
	}
	
	for(l=0;l<size;l++)
	{
		pTitleList_Nand[l].titleID = 0;
		pTitleList_Nand[l].pBanner = 0;
	}

	size = (gotten<size) ? gotten : size;
	
	for(l=0;l<size;l++)
	{
		pTitleList_Nand[l+1].titleID = titleIdArray[l];
		pTitleList_Nand[l+1].pBanner = &bannerBuf[l];
	}
	// return : *TitleProperty Array
	return size;
}


// リセットパラメータの取得
const ResetParamBody *SYSM_GetResetParamBody( void )
{
	return (const ResetParamBody *)&SYSMi_GetWork()->resetParam.body;
}


// ロゴデモスキップかどうかをセット
void SYSM_SetLogoDemoSkip( BOOL skip )
{
	SYSMi_GetWork()->isLogoSkip = skip;
}


// ロゴデモスキップか？
BOOL SYSM_IsLogoDemoSkip( void )
{
	// ※システムアプリからのハードリセットによるロゴデモ飛ばしも判定に入れる。
	
	return SYSMi_GetWork()->isLogoSkip;
}


// ISデバッガのバナービューモード起動かどうか？
static BOOL SYSMi_IsDebuggerBannerViewMode( void )
{
#ifdef __IS_DEBUGGER_BUILD
	return ( SYSMi_GetWork()->isOnDebugger &&
			 SYSMi_IsValidCard() &&
			 SYSM_GetCardRomHeader()->dbgRomSize == 0 ) ? TRUE : FALSE;
#else
	return FALSE;
#endif	// __IS_DEBUGGER_BUILD
}

// 有効なTWL/NTRカードが差さっているか？
BOOL SYSM_IsExistCard( void )
{
	return SYSMi_GetWork()->isExistCard;
}


// 検査用カードが差さっているか？
BOOL SYSM_IsInspectCard( void )
{
	return ( SYSM_IsExistCard() && SYSM_GetCardRomHeader()->inspect_card );
}




// 指定タイトルがブート可能なポインタかチェック
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle )
{
#pragma unused( pBootTitle )
	
	return TRUE;
}


// TPリード可能状態か？
BOOL SYSM_IsTPReadable( void )
{
	return TRUE;
}


// ============================================================================
//
// アプリ起動
//
// ============================================================================

static BOOL s_load_success = FALSE;

static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle )
{	enum
	{
	    region_header,
	    region_arm9_ntr,
	    region_arm7_ntr,
	    region_arm9_twl,
	    region_arm7_twl,
	    region_max
	};
	// メインメモリのクリア
	// DSダウンロードプレイの時は、ROMヘッダを退避する
	// アプリロード
	// アプリ認証
	
	// 実験用。namを改造している。ロードするだけ。
	//NAM_LaunchTitle(pBootTitle->titleID);
	
	// ロード
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    NAM_GetTitleBootContentPath(path, pBootTitle->titleID);

    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);

    if( ! bSuccess )
    {
OS_TPrintf("RebootSystem failed: cant open file\n");
        return;
    }

    {
        int     i;
        u32     source[region_max];
        u32     length[region_max];
        u32     destaddr[region_max];
        static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
        s32 readLen;

        // まずROMヘッダを読み込む
        // (本来ならここでSRLの正当性判定)
        bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);

        if( ! bSuccess )
        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
            FS_CloseFile(file);
            return;
        }

        readLen = ReadFile(file, header, (s32)sizeof(header));

        if( readLen != (s32)sizeof(header) )
        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", header, 0, sizeof(header), readLen);
            FS_CloseFile(file);
            return;
        }

        if( header[0x15C] != 0x56 || header[0x15D] != 0xCF )
        {
int i, j;
for( i = 0; i < 0x20; ++i )
{
for( j = 0; j < 0x10; ++j )
{
OS_TPrintf("%02X ", header[i * 0x10 + j]);
}
OS_TPrintf("\n");
}
OS_TPrintf("RebootSystem failed: logo CRC error\n");
            FS_CloseFile(file);
            return;
        }

        // 各領域を読み込む
        source  [region_header  ] = 0x00000000;
        length  [region_header  ] = HW_TWL_ROM_HEADER_BUF_SIZE;
        destaddr[region_header  ] = HW_TWL_ROM_HEADER_BUF;
		
        source  [region_arm9_ntr] = *(const u32*)&header[0x020];
        length  [region_arm9_ntr] = *(const u32*)&header[0x02C];
        destaddr[region_arm9_ntr] = *(const u32*)&header[0x028];
		
        source  [region_arm7_ntr] = *(const u32*)&header[0x030];
        length  [region_arm7_ntr] = *(const u32*)&header[0x03C];
        destaddr[region_arm7_ntr] = *(const u32*)&header[0x038];
		
        source  [region_arm9_twl] = *(const u32*)&header[0x1C0];
        length  [region_arm9_twl] = *(const u32*)&header[0x1CC];
        destaddr[region_arm9_twl] = *(const u32*)&header[0x1C8];
		
        source  [region_arm7_twl] = *(const u32*)&header[0x1D0];
        length  [region_arm7_twl] = *(const u32*)&header[0x1DC];
        destaddr[region_arm7_twl] = *(const u32*)&header[0x1D8];

        for (i = region_header; i < region_max; ++i)
        {
            u32 len = length[i];

            bSuccess = FS_SeekFile(file, (s32)source[i], FS_SEEK_SET);

            if( ! bSuccess )
            {
OS_TPrintf("RebootSystem failed: cant seek file(%d)\n", source[i]);
                FS_CloseFile(file);
                return;
            }

            readLen = ReadFile(file, (void *)destaddr[i], (s32)len);

            if( readLen != (s32)len )
            {
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
                FS_CloseFile(file);
                return;
            }
        }

        (void)FS_CloseFile(file);
    }

	// ROMヘッダバッファをコピー
	MI_CpuCopy32( (void *)HW_TWL_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	
	s_load_success = TRUE;
}

// スタック用
#define THREAD_PRIO 17
#define STACK_SIZE 5120 // 適当
static OSThread thread;
static u64 stack[ STACK_SIZE / sizeof(u64) ];

// 指定タイトルを別スレッドでロード開始する
OSThread* SYSM_StartLoadTitle( TitleProperty *pBootTitle )
{
	s_load_success = FALSE;
	OS_InitThread();
	OS_CreateThread( &thread, (void (*)(void *))SYSMi_LoadTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
	OS_WakeupThreadDirect( &thread );
	
	return &thread;
}

// ロード済みの指定タイトルの認証とブートを行う
AuthResult SYSM_AuthenticateTitle( TitleProperty *pBootTitle )
{
	// ロード成功しているか
	if(s_load_success == FALSE)
	{
		return AUTH_RESULT_TITLE_POINTER_ERROR;
	}
	// パラメータチェック
	if( !SYSMi_CheckTitlePointer( pBootTitle ) ) {
		return AUTH_RESULT_TITLE_POINTER_ERROR;
	}
#if 0
	// エントリアドレスの正当性をチェック
	if( !SYSMi_CheckEntryAddress() ) {
		return AUTH_RESULT_ENTRY_ADDRESS_ERROR;
	}
#endif

	// マウント情報の登録
	SYSM_SetMountInfo  ( pBootTitle->titleID );
	SYSM_SetBootSRLPath( pBootTitle->titleID );
	
	BOOT_Ready();	// never return.
	
	return AUTH_RESULT_SUCCEEDED;
}

// 指定タイトルの認証＆ロード　※１フレームじゃ終わらん。
// もしかすると使わないかも
AuthResult SYSM_LoadAndAuthenticateTitle( TitleProperty *pBootTitle )
{
	OSThread *t;
	// 指定タイトルのロード
	t = SYSM_StartLoadTitle( pBootTitle );
	
	OS_JoinThread(t);
	
	// 認証
	return SYSM_AuthenticateTitle( pBootTitle );
}


// ============================================================================
//
// デバイス制御
//
// ============================================================================

// バックライト輝度調整
void SYSM_SetBackLightBrightness( u8 brightness )
{
	if( brightness > BACKLIGHT_LEVEL_MAX ) {
		OS_Panic( "Backlight brightness over : %d\n", brightness );
	}
	( void )PMi_WriteRegister( 0x20, (u16)brightness );
	TSD_SetBacklightBrightness( brightness );
	SYSM_WriteTWLSettingsFile();
}


// タッチパネルキャリブレーション
void SYSM_CaribrateTP( void )
{
#ifndef __TP_OFF
	TPCalibrateParam calibrate;
	
	( void )TP_CalcCalibrateParam( &calibrate,							// タッチパネル初期化
			GetTSD()->tp.data.raw_x1, GetTSD()->tp.data.raw_y1, (u16)GetTSD()->tp.data.dx1, (u16)GetTSD()->tp.data.dy1,
			GetTSD()->tp.data.raw_x2, GetTSD()->tp.data.raw_y2, (u16)GetTSD()->tp.data.dx2, (u16)GetTSD()->tp.data.dy2 );
	TP_SetCalibrateParam( &calibrate );
	OS_Printf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			GetTSD()->tp.data.raw_x1, GetTSD()->tp.data.raw_y1, (u16)GetTSD()->tp.data.dx1, (u16)GetTSD()->tp.data.dy1,
			GetTSD()->tp.data.raw_x2, GetTSD()->tp.data.raw_y2, (u16)GetTSD()->tp.data.dx2, (u16)GetTSD()->tp.data.dy2 );
#endif
}


// RTCクロック補正値をセット
static void SYSMi_WriteAdjustRTC( void )
{
	// ※TWLの時は、NANDの"/sys/HWINFO.dat"ファイルから該当する情報を取得する。
#if 0
	FS_OpenFile( "/sys/HWINFO.dat" );
	FS_ReadFile( xxxx );
	raw = xxxx.rtcRaw;
	( void )RTCi_SetRegAdjust( &raw );
#endif
	
#ifndef __IS_DEBUGGER_BUILD											// デバッガ用ビルド時は補正しない。
	RTCRawAdjust raw;
	raw.adjust = 0;
//	raw.adjust = GetTSD()->rtcClockAdjust;							// isValidTSD時にはrtcClockAdjustは
																	// 0クリアされているため補正機能は使用されない
	( void )RTCi_SetRegAdjust( &raw );
#endif /* __IS_DEBUGGER_BUILD */
}


// ============================================================================
//
// バナー
//
// ============================================================================

// バナーファイルの読み込みの実体
static void SYSMi_ReadCardBannerFile( void )
{
	NTRBannerFile *pBanner = &s_bannerBuf;
	
	if( ( !SYSMi_IsValidCard() ) || ( *(void** )BANNER_ROM_OFFSET == NULL ) ) {
		s_isBanner = FALSE;
		return;
	}
	
	// ROMカードからのバナーデータのリード
	DC_FlushRange( pBanner, sizeof(NTRBannerFile) );
	CARD_ReadRom( 4, *(void** )BANNER_ROM_OFFSET, pBanner, sizeof(NTRBannerFile) );
	
	// バナーデータの正誤チェック
	{
		int i;
		u16 calc_crc = 0xffff;
		u16 *hd_crcp = (u16 *)&pBanner->h.crc16_v1;
		BannerCheckParam *chkp = &s_bannerCheckList[ 0 ];
		
		s_isBanner  = TRUE;
		
		for( i = 0; i < NTR_BNR_VER_MAX; i++ ) {
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
			MI_CpuClear16( &s_bannerBuf, sizeof(NTRBannerFile) );
		}
	}
}


//======================================================================
//
//  各種チェック
//
//======================================================================

// 有効なTWLカードが差さっているか？
BOOL SYSM_IsTWLCard( void );
BOOL SYSM_IsTWLCard( void )
{
	return ( SYSM_IsExistCard() && ( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL ) );
}


// 有効なNTRカードが差さっているか？
BOOL SYSM_IsNTRCard( void );
BOOL SYSM_IsNTRCard( void )
{
	return ( SYSM_IsExistCard() && ( SYSM_GetCardRomHeader()->platform_code == PLATFORM_CODE_NTR ) );
}


// NTR,TWLカード存在チェック 		「リターン　1：カード認識　0：カードなし」
static int SYSMi_IsValidCard( void )
{
	if( ( SYSM_GetCardRomHeader()->nintendo_logo_crc16 == 0xcf56 ) &&
	    ( SYSM_GetCardRomHeader()->header_crc16 == SYSMi_GetWork()->cardHeaderCrc16 ) ) {
		return TRUE;												// NTR,TWLカードあり（NintendoロゴCRC、カードヘッダCRCが正しい場合）
																	// ※Nintendoロゴデータのチェックは、特許の都合上、ロゴ表示ルーチン起動後に行います。
	}else {
		return FALSE;												// NTR,TWLカードなし
	}
}

// エントリアドレスの正当性チェック
static BOOL SYSMi_CheckEntryAddress( void )
{
	// エントリアドレスがROM内登録エリアかAGBカートリッジエリアなら、無限ループに入る。
	if( !( ( (u32)SYSM_GetCardRomHeader()->main_entry_address >= HW_MAIN_MEM ) &&
		   ( (u32)SYSM_GetCardRomHeader()->main_entry_address <  SYSM_ARM9_MMEM_ENTRY_ADDR_LIMIT )
		 ) ||
		!( ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_MAIN_MEM ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  SYSM_ARM7_LOAD_MMEM_LAST_ADDR ) ) ||
		   ( ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  >= HW_WRAM    ) &&
			 ( (u32)SYSM_GetCardRomHeader()->sub_entry_address  <  SYSM_ARM7_LOAD_WRAM_LAST_ADDR ) )
		 )
	 ) {
		OS_TPrintf("entry address invalid.\n");
#ifdef __DEBUG_SECURITY_CODE
		DispSingleColorScreen( SCREEN_YELLOW );
#endif
		return FALSE;
	}
	OS_TPrintf("entry address valid.\n");
	return TRUE;
}


// クローンブート判定
static void SYSMi_CheckCardCloneBoot( void )
{
	u8 	*buffp         = (u8 *)&s_bannerBuf;		// バナー用バッファをテンポラリとして使用
	u32 total_rom_size = SYSM_GetCardRomHeader()->rom_valid_size ? SYSM_GetCardRomHeader()->rom_valid_size : 0x01000000;
	u32 file_offset    = total_rom_size & 0xFFFFFE00;
	
	if( !SYSMi_IsValidCard() ) {
		return;
	}
	
	DC_FlushRange( buffp, BNR_IMAGE_SIZE );
	CARD_ReadRom( 4, (void *)file_offset, buffp, BNR_IMAGE_SIZE );
	
	buffp += total_rom_size & 0x000001FF;
	if( *buffp++ == 'a' && *buffp == 'c' ) {
		SYSMi_GetWork()->cloneBootMode = CLONE_BOOT_MODE;
	}else {
		SYSMi_GetWork()->cloneBootMode = OTHER_BOOT_MODE;
	}
}


// 起動時のRTCチェック
static void SYSMi_CheckRTC( void )
{
	RTCDate date;
	RTCTime	time;
	
	// RTCのリセット or おかしい値を検出した場合は初回起動シーケンスへ。
	( void )RTC_GetDateTime( &date, &time );
	if( !SYSM_CheckRTCDate( &date ) ||
	    !SYSM_CheckRTCTime( &time )
#ifndef __IS_DEBUGGER_BUILD											// 青デバッガではRTCの電池がないので、毎回ここにひっかかって設定データが片方クリアされてしまう。これを防ぐスイッチ。
		||
		( SYSMi_GetWork()->rtcStatus & 0x01 )
#endif
		) {							// RTCの異常を検出したら、rtc入力フラグ＆rtcOffsetを0にしてNVRAMに書き込み。
		OS_TPrintf("\"RTC reset\" or \"Illegal RTC data\" detect!\n");
		GetTSD()->flags.isSetDateTime	= 0;
		GetTSD()->rtcOffset				= 0;
		GetTSD()->rtcLastSetYear		= 0;
		// ※※ライトする？
		SYSM_WriteTWLSettingsFile();
	}
}


//======================================================================
//  デバッグ
//======================================================================
