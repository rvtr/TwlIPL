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
#include <spi.h>
#include <es.h>

// define data-----------------------------------------------------------------
#define CARD_BANNER_INDEX			( LAUNCHER_TITLE_LIST_NUM - 1 )

typedef struct BannerCheckParam {
	u8		*pSrc;
	u32		size;
}BannerCheckParam;

// extern data-----------------------------------------------------------------
extern void SYSMi_SetLauncherMountInfo( void );
extern void SYSM_SetBootAppMountInfo( TitleProperty *pBootTitle );		// マウント情報のセット

// function's prototype-------------------------------------------------------
static TitleProperty *SYSMi_CheckShortcutBoot( void );
static BOOL SYSMi_IsDebuggerBannerViewMode( void );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );
static void SYSMi_WriteAdjustRTC( void );
static int  SYSMi_IsValidCard( void );
static BOOL SYSMi_CheckEntryAddress( void );
static void SYSMi_CheckCardCloneBoot( void );
static void SYSMi_CheckRTC( void );

static s32 ReadFile(FSFile* pf, void* buffer, s32 size);

static void SYSMi_Relocate( void );
static BOOL SYSMi_ReadCardBannerFile( u32 bannerOffset, TWLBannerFile *pBanner );
static BOOL SYSMi_CheckBannerFile( NTRBannerFile *pBanner );


// global variable-------------------------------------------------------------
void *(*SYSM_Alloc)( u32 size  );
void  (*SYSM_Free )( void *ptr );

#ifdef SYSM_DEBUG_
SYSM_work		*pSysm;											// デバッガでのSYSMワークのウォッチ用
#endif

// static variable-------------------------------------------------------------
static OSThread			thread;
static TWLBannerFile	s_bannerBuf[ LAUNCHER_TITLE_LIST_NUM ] ATTRIBUTE_ALIGN(32);


// const data------------------------------------------------------------------

typedef struct RomSegmentRange {
	u32		start;
	u32		end;
}RomSegmentRange;

static RomSegmentRange romSegmentRange[RELOCATE_INFO_NUM] = {
	{ SYSM_TWL_ARM9_LOAD_MMEM,     SYSM_TWL_ARM9_LOAD_MMEM_END },
	{ SYSM_TWL_ARM7_LOAD_MMEM,     SYSM_TWL_ARM7_LOAD_MMEM_END },
	{ SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END },
	{ SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END },
};

static RomSegmentRange romSegmentRangeNitro[RELOCATE_INFO_NUM] = {
	{ SYSM_NTR_ARM9_LOAD_MMEM,     SYSM_NTR_ARM9_LOAD_MMEM_END },
	{ SYSM_NTR_ARM7_LOAD_MMEM,     SYSM_NTR_ARM7_LOAD_MMEM_END },
	{ SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END },
	{ SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END },
};

static u32 load_region_check_list[RELOCATE_INFO_NUM][RELOCATE_INFO_NUM * 2 - 1] = 
{
	{SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END, 
	SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END, 
	SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END, 
	NULL }, 
};


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
#endif /* SYSM_DEBUG_ */
	
	// ランチャーのマウント情報セット
	SYSMi_SetLauncherMountInfo();
	
	OS_Init();
	
    // ARM7コンポーネント用プロテクションユニット領域変更
    OS_SetProtectionRegion( 2, SYSM_OWN_ARM7_MMEM_ADDR, 512KB );
	
	// ARM9用ブートコード配置のため、アリーナHi位置を下げる
	OS_SetMainArenaHi( (void *)SYSM_OWN_ARM9_MMEM_ADDR_END );
	
	SYSM_SetAllocFunc( pAlloc, pFree );
	
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
TitleProperty *SYSM_ReadParameters( void )
{
	TitleProperty *pBootTitle = NULL;
	u8 brightness = TWL_BACKLIGHT_LEVEL_MAX;
	
	// ARM7のリセットパラメータ取得が完了するのを待つ
	while( !SYSMi_GetWork()->isARM9Start ) {
		SVC_WaitByLoop( 0x1000 );
	}
#ifdef DEBUG_USED_CARD_SLOT_B_
	// ARM7のカードチェック完了を待つ
	while( !SYSMi_GetWork()->is1stCardChecked ) {
		SVC_WaitByLoop( 0x1000 );
	}
#endif
	
	//-----------------------------------------------------
	// リセットパラメータの判定（リセットパラメータが有効かどうかは、ARM7でやってくれている）
	//-----------------------------------------------------
	{
		if( SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ||		// ロゴデモスキップ？
			SYSMi_IsDebuggerBannerViewMode() ) {
			SYSM_SetLogoDemoSkip( TRUE );
		}
		
		if( SYSM_GetLauncherParamBody()->v1.bootTitleID ) {			// アプリ直接起動の指定があったらロゴデモを飛ばして指定アプリ起動
			pBootTitle = (TitleProperty *)&SYSM_GetLauncherParamBody()->v1;
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
	if( THW_ReadNormalInfo() != TSF_READ_RESULT_SUCCEEDED ) {
		OS_TPrintf( "HW Normal Info Broken!\n" );
		SYSMi_GetWork()->isBrokenHWNormalInfo = TRUE;
	}
	// セキュア情報リード
	if( THW_ReadSecureInfo() != TSF_READ_RESULT_SUCCEEDED ) {
		OS_TPrintf( "HW Secure Info Broken!\n" );
		SYSMi_GetWork()->isBrokenHWSecureInfo = TRUE;
	}
	
	//-----------------------------------------------------
	// 本体設定データのリード
	//-----------------------------------------------------
	if( SYSM_ReadTWLSettingsFile() ) {								// NANDからTWL本体設定データをリード
		SYSM_CaribrateTP();											// 読み出したTWL本体設定データをもとにTPキャリブレーション。
		brightness = (u8)TSD_GetBacklightBrightness();
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
	
	SYSM_VerifyAndRecoveryNTRSettings();							// NTR設定データを読み出して、TWL設定データとベリファイし、必要ならリカバリ
	
	//NAMの初期化
	//NAM_Init(AllocForNAM,FreeForNAM);
	
	return pBootTitle;
}


// ショートカット起動のチェック
static TitleProperty *SYSMi_CheckShortcutBoot( void )
{
#if 0	// ※未実装
	static TitleProperty s_bootTitle;
	
	MI_CpuClear8( &s_bootTitle, sizoef(TitleProperty) );
	
	//-----------------------------------------------------
	// 量産工程用ショートカットキー or
	// 検査カード起動
	//-----------------------------------------------------
	if( SYSM_IsInspectCard() ||
		( SYSM_IsExistCard() &&
		  ( ( PAD_Read() & PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) ==
			PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) )
	) {
		if( SYSM_GetCardTitleProperty( &s_bootTitle ) ) {			// ※未実装
			s_bootTitle.flags.isInitialShortcutSkip = TRUE;			// 初回起動シーケンスを飛ばす
			s_bootTitle.flags.isLogoSkip = TRUE;					// ロゴデモを飛ばす
			s_bootTitle.flags.media = TITLE_MEDIA_CARD;
			s_bootTitle.flags.isValid = TRUE;
			s_bootTitle.titleID = xxxx;
			SYSM_SetLogoDemoSkip( TRUE );
			return &s_bootTitle;
		}
	}
	
	//-----------------------------------------------------
	// TWL設定データ未入力時の初回起動シーケンス起動
	//-----------------------------------------------------
#ifdef ENABLE_INITIAL_SETTINGS_
	if( !TSD_IsSetTP() ||
		!TSD_IsSetLanguage() ||
		!TSD_IsSetDateTime() ||
		!TSD_IsSetUserColor() ||
		!TSD_IsSetNickname() ) {
		s_bootTitle.titleID = TITLE_ID_MACHINE_SETTINGS;
		s_bootTitle.flags.media = TITLE_MEDIA_NAND;
		s_bootTitle.flags.isValid = TRUE;
		return &s_bootTitle;
	}
#endif // ENABLE_INITIAL_SETTINGS_
	
#endif	// 0
	return NULL;													// 「ブート内容未定」でリターン
}


// カードタイトルの取得
BOOL SYSM_GetCardTitleList( TitleProperty *pTitleList_Card )
{
	BOOL retval = FALSE;
	
	if( SYSMi_GetWork()->isCardStateChanged ) {
		
		MI_CpuClear32( pTitleList_Card, sizeof(TitleProperty) );
		
		// ROMヘッダバッファのコピー
		if( SYSM_IsExistCard() ) {
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );		// ARM7と排他制御する
			DC_InvalidateRange( (void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );	// キャッシュケア
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BAK, (void *)SYSM_CARD_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );	// ROMヘッダコピー
			SYSMi_GetWork()->cardHeaderCrc16 = SYSMi_GetWork()->cardHeaderCrc16;	// ROMヘッダCRCコピー
			SYSMi_GetWork()->isCardStateChanged = FALSE;							// カード情報更新フラグを落とす
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );	// ARM7と排他制御する
			OS_ReleaseLockID( id );
			
			pTitleList_Card->flags.isValid = TRUE;
			pTitleList_Card->flags.isAppLoadCompleted = TRUE;
			pTitleList_Card->flags.isAppRelocate = TRUE;
			pTitleList_Card->pBanner = NULL;
			
			// バナーデータのリード
			if( SYSM_GetCardRomHeader()->banner_offset &&
				SYSMi_ReadCardBannerFile( SYSM_GetCardRomHeader()->banner_offset, &s_bannerBuf[ CARD_BANNER_INDEX ] ) ) {
				pTitleList_Card->pBanner = &s_bannerBuf[ CARD_BANNER_INDEX ];
			}else {
				MI_CpuClearFast( &s_bannerBuf[ CARD_BANNER_INDEX ], sizeof(TWLBannerFile) );
			}
		}
		
		retval = TRUE;
	}
	
	// タイトル情報フラグのセット
	pTitleList_Card->flags.media = TITLE_MEDIA_CARD;
	pTitleList_Card->titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
	
	return retval;
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


// NANDタイトルリストの取得
int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int listNum )
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, 等の条件を指定してタイトルリストを取得する。
	// とりあえずALL
	int l;
	int gotten;
	NAMTitleId titleIdArray[ LAUNCHER_TITLE_LIST_NUM ];
	
	if( listNum > LAUNCHER_TITLE_LIST_NUM ) {
		OS_TPrintf( "Warning: TitleList_Nand num over LAUNCHER_TITLE_LIST_NUM(%d)\n", LAUNCHER_TITLE_LIST_NUM );
	}
	gotten = NAM_GetTitleList( &titleIdArray[ 0 ], LAUNCHER_TITLE_LIST_NUM - 1 );
	gotten = NAM_GetNumTitles();	// [TODO:]本来だったら必要ないが、現在はNAM_GetTitleListがアプリ個数をちゃんと返してくれないので。
	
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
		readLen = ReadFile(file, &s_bannerBuf[l], (s32)sizeof(TWLBannerFile));
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

	// カードアプリ部分を除いたリストクリア
	MI_CpuClearFast( &pTitleList_Nand[ 1 ], sizeof(TitleProperty) * ( listNum - 1 ) );
	
	listNum = (gotten<listNum) ? gotten : listNum;
	
	for(l=0;l<listNum;l++)
	{
		pTitleList_Nand[l+1].titleID = titleIdArray[l];
		pTitleList_Nand[l+1].pBanner = &s_bannerBuf[l];
		if( titleIdArray[l] ) {
			pTitleList_Nand[l+1].flags.isValid = TRUE;
			pTitleList_Nand[l+1].flags.media = TITLE_MEDIA_NAND;
		}
	}
	// return : *TitleProperty Array
	return listNum;
}


// ランチャーパラメータの取得
const LauncherParamBody *SYSM_GetLauncherParamBody( void )
{
	return (const LauncherParamBody *)&SYSMi_GetWork()->launcherParam.body;
}


// ロゴデモスキップかどうかをセット
void SYSM_SetLogoDemoSkip( BOOL skip )
{
	SYSMi_GetWork()->isLogoSkip = skip;
}


// ロゴデモスキップか？
BOOL SYSM_IsLogoDemoSkip( void )
{
	return (BOOL)SYSMi_GetWork()->isLogoSkip;
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


// TSD有効/無効をセット
void SYSM_SetValidTSD( BOOL valid )
{
	SYSMi_GetWork()->isValidTSD = valid;
}


// TSD有効？
BOOL SYSM_IsValidTSD( void )
{
	return (BOOL)SYSMi_GetWork()->isValidTSD;
}

// ============================================================================
//
// アプリ起動
//
// ============================================================================

static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle )
{
	enum
	{
	    region_header = 0,
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
	
	
	// ロード
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    BOOL isTwlApp = TRUE;
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
        
        if( header[0x12] && 0x03 == 0 )
        {
			//NTR専用ROM
			isTwlApp = FALSE;
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
		
		if( isTwlApp )
		{
	        source  [region_arm9_twl] = *(const u32*)&header[0x1C0];
	        length  [region_arm9_twl] = *(const u32*)&header[0x1CC];
	        destaddr[region_arm9_twl] = *(const u32*)&header[0x1C8];
			
	        source  [region_arm7_twl] = *(const u32*)&header[0x1D0];
	        length  [region_arm7_twl] = *(const u32*)&header[0x1DC];
	        destaddr[region_arm7_twl] = *(const u32*)&header[0x1D8];
        }
        
        // 領域読み込み先のチェック及び再配置情報データの作成
        // ゲームカードの再配置情報が書き込まれているので、nandアプリロード前に一旦クリア
        MI_CpuClearFast(SYSMi_GetWork()->romRelocateInfo, sizeof(Relocate_Info) * RELOCATE_INFO_NUM);
		for( i=0; i<RELOCATE_INFO_NUM; i++ )
		{
			if ( !isTwlApp && i >= ARM9_LTD_STATIC ) continue;// nitroでは読み込まない領域
			if ( !SYSM_CheckLoadRegionAndSetRelocateInfo( (RomSegmentName)i, &(destaddr[i+region_arm9_ntr]), length[i+region_arm9_ntr],
				 &(SYSMi_GetWork()->romRelocateInfo[i]), isTwlApp ) )
			{
	OS_TPrintf("RebootSystem failed: ROM Load Region error\n");
	            FS_CloseFile(file);
				return;
			}
		}

        for (i = region_header; i < region_max; ++i)
        {
            u32 len = length[i];
            
            if ( !isTwlApp && i >= region_arm9_twl ) continue;// nitroでは読み込まない領域

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
	
	SYSMi_GetWork()->isLoadSucceeded = TRUE;
}


// 指定タイトルを別スレッドでロード開始する
void SYSM_StartLoadTitle( TitleProperty *pBootTitle )
{
#define THREAD_PRIO 17
#define STACK_SIZE 0xc00
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	
	// アプリ未ロード状態なら、ロード開始
	if( !pBootTitle->flags.isAppLoadCompleted ) {
		SYSMi_GetWork()->isLoadSucceeded = FALSE;
		OS_InitThread();
		OS_CreateThread( &thread, (void (*)(void *))SYSMi_LoadTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
		OS_WakeupThreadDirect( &thread );
	}else if( pBootTitle->flags.isAppRelocate ) {
	// アプリロード済みで、再配置要求ありなら、再配置（カードのみ対応）
		SYSMi_Relocate();
		SYSMi_GetWork()->isLoadSucceeded = TRUE;
	}else
	{
		// アプリロード済みで、再配置要求なし
		SYSMi_GetWork()->isLoadSucceeded = TRUE;
	}
	
	if( pBootTitle->flags.media == TITLE_MEDIA_CARD ) {
		SYSMi_GetWork()->isCardBoot = TRUE;
	}else if(pBootTitle->flags.isAppLoadCompleted)
	{
		// カードブートでなく、ロード済みの場合、再配置情報をランチャーパラメタから読み込み
		MI_CpuCopy8( SYSM_GetLauncherParamBody()->v1.relocInfo, SYSMi_GetWork()->romRelocateInfo, sizeof(Relocate_Info)*RELOCATE_INFO_NUM );
		// 更にヘッダを再配置
		if( ((ROM_Header_Short *)(OS_TWL_HEADER_PRELOAD_MMEM))->platform_code & PLATFORM_CODE_FLAG_TWL ) {
			// TWL-ROMヘッダ情報の再配置
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		}else {
			// TWL-ROMヘッダ情報の再配置
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
			// NTR-ROMヘッダ情報の再配置
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)0x027ffe00, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );	// 8Mのケツへ（TWLデバッガでのNTRモードデバッグ用）
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)0x023ffe00, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );	// 4Mのケツへ
		}
	}
}


// カードアプリケーションの再配置
static void SYSMi_Relocate( void )
{
	u32 size;
	u32 *dest = SYSM_GetCardRomHeader()->main_ram_address;
	// NTRセキュア領域の再配置
	DC_InvalidateRange( (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );	// キャッシュケア
	size = ( SYSM_GetCardRomHeader()->main_size < SECURE_AREA_SIZE ) ?
			 SYSM_GetCardRomHeader()->main_size : SECURE_AREA_SIZE;
	// romの再配置情報を参照して、セキュア領域の再配置先を変更する必要が無いか調べる
	if( SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC].src != NULL )
	{
		dest = (u32 *)SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC].src;
	}
	MI_CpuCopyFast( (void *)SYSM_CARD_NTR_SECURE_BUF, dest, size );
	
	if( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL ) {
		// TWLセキュア領域の再配置
		dest = SYSM_GetCardRomHeader()->main_ltd_ram_address;
		DC_InvalidateRange( (void *)SYSM_CARD_TWL_SECURE_BUF, SECURE_AREA_SIZE );	// キャッシュケア
		size = ( SYSM_GetCardRomHeader()->main_ltd_size < SECURE_AREA_SIZE ) ?
				 SYSM_GetCardRomHeader()->main_ltd_size : SECURE_AREA_SIZE;
		// romの再配置情報を参照して、セキュア領域の再配置先を変更する必要が無いか調べる
		if( SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC].src != NULL )
		{
			dest = (u32 *)SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC].src;
		}
		MI_CpuCopyFast( (void *)SYSM_CARD_TWL_SECURE_BUF, dest, size );
		// TWL-ROMヘッダ情報の再配置
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	}else {
		// TWL-ROMヘッダ情報の再配置
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		// NTR-ROMヘッダ情報の再配置
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)0x027ffe00, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );	// 8Mのケツへ（TWLデバッガでのNTRモードデバッグ用）
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)0x023ffe00, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );	// 4Mのケツへ
	}
}


// アプリロード済み？
BOOL SYSM_IsLoadTitleFinished( TitleProperty *pBootTitle )
{
	if( pBootTitle->flags.isAppLoadCompleted ) {
		return TRUE;
	}
	return OS_IsThreadTerminated( &thread );
}


// ロード済みの指定タイトルの認証とブートを行う
AuthResult SYSM_AuthenticateTitle( TitleProperty *pBootTitle )
{
	// ロード中
	if( !SYSM_IsLoadTitleFinished( pBootTitle ) ) {
		return AUTH_RESULT_PROCESSING;
	}
	// ロード成功？
	if( SYSMi_GetWork()->isLoadSucceeded == FALSE )
	{
		return AUTH_RESULT_TITLE_LOAD_FAILED;
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
	
	
	// ※ROMヘッダ認証
	
	
	// マウント情報の登録
	SYSM_SetBootAppMountInfo( pBootTitle );
	
	BOOT_Ready();	// never return.
	
	return AUTH_RESULT_SUCCEEDED;
}

#if 0
// 指定タイトルの認証＆ロード　※１フレームじゃ終わらん。
// もしかすると使わないかも
void SYSM_LoadAndAuthenticateTitleThread( TitleProperty *pBootTitle )
{
	SYSMi_LoadTitleThreadFunc( pBootTitle );
	OS_JoinThread(&thread);
	
	// 認証
	return SYSM_AuthenticateTitle( pBootTitle );
}
#endif


// ============================================================================
//
// デバイス制御
//
// ============================================================================

// バックライト輝度調整
void SYSM_SetBackLightBrightness( u8 brightness )
{
	if( brightness > TWL_BACKLIGHT_LEVEL_MAX ) {
		OS_Panic( "Backlight brightness over : %d\n", brightness );
	}
	( void )PMi_WriteRegister( 0x20, (u16)brightness );
	TSD_SetBacklightBrightness( brightness );
	
	// [TODO:] バックライト輝度は毎回セーブせずに、アプリ起動やリセット、電源OFF時に値が変わっていたらセーブするようにする。
	SYSM_WriteTWLSettingsFile();
}


// タッチパネルキャリブレーション
void SYSM_CaribrateTP( void )
{
	TWLTPCalibData store;
	TPCalibrateParam calibParam;
	
	// 本体設定データからキャリブレーション情報を取得
	TSD_GetTPCalibration( &store );
	
	// TPキャリブレーション
	( void )TP_CalcCalibrateParam( &calibParam,							// タッチパネル初期化
			store.data.raw_x1, store.data.raw_y1, (u16)store.data.dx1, (u16)store.data.dy1,
			store.data.raw_x2, store.data.raw_y2, (u16)store.data.dx2, (u16)store.data.dy2 );
	TP_SetCalibrateParam( &calibParam );
	OS_Printf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			store.data.raw_x1, store.data.raw_y1, (u16)store.data.dx1, (u16)store.data.dy1,
			store.data.raw_x2, store.data.raw_y2, (u16)store.data.dx2, (u16)store.data.dy2 );
}


// RTCクロック補正値をセット
static void SYSMi_WriteAdjustRTC( void )
{
	RTCRawAdjust raw;
	raw.adjust = THW_GetRTCAdjust();
	( void )RTCi_SetRegAdjust( &raw );
}


// ============================================================================
//
// バナー
//
// ============================================================================

// バナーファイルの読み込みの実体
static BOOL SYSMi_ReadCardBannerFile( u32 bannerOffset, TWLBannerFile *pBanner )
{
#ifndef DEBUG_USED_CARD_SLOT_B_
	// ※スロットAからのリードなら問題ないが、スロットBからは直接読めないので
	BOOL isRead;
	u16 id = (u16)OS_GetLockID();
	
	// ROMカードからのバナーデータのリード
	DC_FlushRange( pBanner, sizeof(TWLBannerFile) );
	CARD_LockRom( id );
	CARD_ReadRom( 4, (void *)bannerOffset, pBanner, sizeof(TWLBannerFile) );
	CARD_UnlockRom( id );
	OS_ReleaseLockID( id );
	
	isRead = SYSMi_CheckBannerFile( (NTRBannerFile *)pBanner );
	
	if( !isRead ) {
		MI_CpuClearFast( pBanner, sizeof(TWLBannerFile) );
	}
	return isRead;
#else
#pragma unused(bannerOffset)
	if( SYSMi_GetWork()->isValidCardBanner ) {
		DC_InvalidateRange( (void *)SYSM_CARD_BANNER_BUF, 0x3000 );
		MI_CpuCopyFast( (void *)SYSM_CARD_BANNER_BUF, pBanner, sizeof(TWLBannerFile) );
	}
	return (BOOL)SYSMi_GetWork()->isValidCardBanner;
#endif
}


	// バナーデータの正誤チェック
static BOOL SYSMi_CheckBannerFile( NTRBannerFile *pBanner )
{
	int i;
	BOOL retval = TRUE;
	u16 calc_crc = 0xffff;
	u16 *pHeaderCRC = (u16 *)&pBanner->h.crc16_v1;
	BannerCheckParam bannerCheckList[ NTR_BNR_VER_MAX ];
	BannerCheckParam *pChk = &bannerCheckList[ 0 ];
	
	bannerCheckList[ 0 ].pSrc = (u8 *)&( pBanner->v1 );
	bannerCheckList[ 0 ].size = sizeof( BannerFileV1 );
	bannerCheckList[ 1 ].pSrc = (u8 *)&( pBanner->v2 );
	bannerCheckList[ 1 ].size = sizeof( BannerFileV2 );
	bannerCheckList[ 2 ].pSrc = (u8 *)&( pBanner->v3 );
	bannerCheckList[ 2 ].size = sizeof( BannerFileV3 );
	
	for( i = 0; i < NTR_BNR_VER_MAX; i++ ) {
		if( i < pBanner->h.version ) {
			calc_crc = SVC_GetCRC16( calc_crc, pChk->pSrc, pChk->size );
			if( calc_crc != *pHeaderCRC++ ) {
				retval = FALSE;
				break;
			}
		}else {
			MI_CpuClear16( pChk->pSrc, pChk->size );
		}
		pChk++;
	}
	
	return retval;
}


//======================================================================
//
//  各種チェック
//
//======================================================================

// 有効なTWL/NTRカードが差さっているか？
BOOL SYSM_IsExistCard( void )
{
	return (BOOL)SYSMi_GetWork()->isExistCard;
}


// 検査用カードが差さっているか？
BOOL SYSM_IsInspectCard( void )
{
	return ( SYSM_IsExistCard() && SYSM_GetCardRomHeader()->inspect_card );
}


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
#if 0
	u8 	*buffp         = (u8 *)&pTempBuffer;
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
#endif
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
		SYSMi_GetWork()->isResetRTC
#endif
		) {							// RTCの異常を検出したら、rtc入力フラグ＆rtcOffsetを0にしてNVRAMに書き込み。
		OS_TPrintf("\"RTC reset\" or \"Illegal RTC data\" detect!\n");
		TSD_SetFlagDateTime( FALSE );
		TSD_SetRTCOffset( 0 );
		TSD_SetRTCLastSetYear( 0 );
		SYSM_WriteTWLSettingsFile();
	}
}


//======================================================================
//  デバッグ
//======================================================================
