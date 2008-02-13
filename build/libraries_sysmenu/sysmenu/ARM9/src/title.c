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
#include <es.h>
#include <firm/format/from_firm.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
#define CARD_BANNER_INDEX			( LAUNCHER_TITLE_LIST_NUM - 1 )

#define SYSTEM_APP_KEY_OFFSET		1 // ファームから送られてくるキーのためのオフセット
#define ROM_HEADER_HASH_OFFSET		(0x10) // 署名からROMヘッダハッシュを取り出すためのオフセット

#define SIGN_HEAP_ADDR	0x023c0000	// 署名計算のためのヒープ領域開始アドレス
#define SIGN_HEAP_SIZE	0x1000		// 署名計算のためのヒープサイズ

#define	DIGEST_HASH_BLOCK_SIZE_SHA1					(512/8)
#define ROM_HEADER_HASH_CALC_DATA_LEN	0xe00 // ROMヘッダのハッシュ計算する部分の長さ

// extern data-----------------------------------------------------------------
extern const u8 g_devPubKey[ 3 ][ 0x80 ];

// function's prototype-------------------------------------------------------
static BOOL SYSMi_ReadBanner_NAND( NAMTitleId titleID, u8 *pDst );
static s32  ReadFile( FSFile* pf, void* buffer, s32 size );
static void SYSMi_EnableHotSW( BOOL enable );
static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle );
static void SYSMi_Relocate( void );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static OSThread			s_thread;
static TWLBannerFile	s_bannerBuf[ LAUNCHER_TITLE_LIST_NUM ] ATTRIBUTE_ALIGN(32);

// const data------------------------------------------------------------------
static const OSBootType s_launcherToOSBootType[ LAUNCHER_BOOTTYPE_MAX ] = {
    OS_BOOTTYPE_ILLEGAL,	// ILLEGAL
    OS_BOOTTYPE_ROM,		// ROM
    OS_BOOTTYPE_NAND,		// TEMP
    OS_BOOTTYPE_NAND,		// NAND
    OS_BOOTTYPE_MEMORY,		// MEMORY
};

static const u8 s_digestDefaultKey[ DIGEST_HASH_BLOCK_SIZE_SHA1 ] = 
{
    0x21, 0x06, 0xc0, 0xde,
    0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d,
    0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08,
    0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf,
    0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c,
    0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23,
    0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe,
    0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37,
    0x87, 0x46, 0x58, 0x24,
};

//================================================================================
// for register SCFG_OP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFG_GetBondingOption

  Description:  Get bonding option data

  Arguments:    None

  Returns:      option data
 *---------------------------------------------------------------------------*/
// SharedArea Access ver.
static inline u16 SCFG_GetBondingOption(void)
{
	return (u16)(*(u8*)(HW_SYS_CONF_BUF+HWi_WSYS08_OFFSET) & HWi_WSYS08_OP_OPT_MASK);
}

// ============================================================================
//
// 情報取得
//
// ============================================================================

// カードタイトルの取得
BOOL SYSM_GetCardTitleList( TitleProperty *pTitleList_Card )
{
	BOOL retval = FALSE;
	
	// [TODO:] ROMヘッダの platform_code がNTR,TWL-HYB,TWL-LTD以外のもの
	//                     region_codeが本体情報と違うもの
	//         の場合は、正常に認識できないタイトルであることを示す。
	
	if( SYSMi_GetWork()->flags.common.isCardStateChanged ) {
		
		MI_CpuClear32( pTitleList_Card, sizeof(TitleProperty) );
		
		// ROMヘッダバッファのコピー
		if( SYSM_IsExistCard() ) {
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );						// ARM7と排他制御する
			
			// ROMヘッダのリード
			DC_InvalidateRange( (void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );	// キャッシュケア
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BAK, (void *)SYSM_CARD_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );	// ROMヘッダコピー
			SYSMi_GetWork()->cardHeaderCrc16 = SYSMi_GetWork()->cardHeaderCrc16;				// ROMヘッダCRCコピー
			
			// バナーデータのリード
			SYSMi_ReadCardBannerFile( SYSM_GetCardRomHeader()->banner_offset, &s_bannerBuf[ CARD_BANNER_INDEX ] );
			pTitleList_Card->pBanner = &s_bannerBuf[ CARD_BANNER_INDEX ];
			
			SYSMi_GetWork()->flags.common.isCardStateChanged = FALSE;							// カード情報更新フラグを落とす
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );					// ARM7と排他制御する
			OS_ReleaseLockID( id );

			pTitleList_Card->flags.isValid = TRUE;
			pTitleList_Card->flags.isAppLoadCompleted = TRUE;
			pTitleList_Card->flags.isAppRelocate = TRUE;
			pTitleList_Card->pBanner = NULL;
		}
		
		retval = TRUE;
	}
	
	// タイトル情報フラグのセット
	pTitleList_Card->flags.bootType = LAUNCHER_BOOTTYPE_ROM;
	pTitleList_Card->titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
	
	return retval;
}


// NANDタイトルリストの取得
int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int listNum )
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, 等の条件を指定してタイトルリストを取得する。
	// とりあえずALL
	OSTick start;
	int l;
	int getNum;
	int validNum = 0;
	NAMTitleId titleIDArray[ LAUNCHER_TITLE_LIST_NUM ];
	NAMTitleId *pTitleIDList = NULL;
	
	
	if( listNum > LAUNCHER_TITLE_LIST_NUM ) {
		OS_TPrintf( "Warning: TitleList_Nand num over LAUNCHER_TITLE_LIST_NUM(%d)\n", LAUNCHER_TITLE_LIST_NUM );
	}
	
	// インストールされているタイトルの取得
	start = OS_GetTick();
	getNum = NAM_GetNumTitles();
	OS_TPrintf( "NAM_GetNumTitles : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	pTitleIDList = SYSM_Alloc( sizeof(NAMTitleId) * getNum );
	if( pTitleIDList == NULL ) {
		OS_TPrintf( "%s: alloc error.\n", __FUNCTION__ );
		return 0;
	}
	start = OS_GetTick();
	(void)NAM_GetTitleList( pTitleIDList, (u32)getNum );
	OS_TPrintf( "NAM_GetTitleList : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	
	// 取得したタイトルがローンチ対象かどうかをチェック
	for( l = 0; l < getNum; l++ ) {
		// "Not Launch"でない　かつ　"Data Only"でない　なら有効なタイトルとしてリストに追加
		if( ( pTitleIDList[ l ] & ( TITLE_ID_NOT_LAUNCH_FLAG_MASK | TITLE_ID_DATA_ONLY_FLAG_MASK ) ) == 0 ) {
			titleIDArray[ validNum ] = pTitleIDList[ l ];
			SYSMi_ReadBanner_NAND( pTitleIDList[ l ], (u8 *)&s_bannerBuf[ validNum ] );
			validNum++;
		}
	}
	SYSM_Free( pTitleIDList );
	
	// 念のため残り領域を0クリア
	for( l = validNum; l < LAUNCHER_TITLE_LIST_NUM; l++ ) {
		titleIDArray[ l ] = 0;
	}
	
	// 最終リストに対して、カードアプリ部分を除いた部分をクリア
	MI_CpuClearFast( &pTitleList_Nand[ 1 ], sizeof(TitleProperty) * ( listNum - 1 ) );
	
	listNum = ( validNum < listNum ) ? validNum : listNum;
	
	for(l=0;l<listNum;l++)
	{
		pTitleList_Nand[l+1].titleID = titleIDArray[l];
		pTitleList_Nand[l+1].pBanner = &s_bannerBuf[l];
		if( titleIDArray[l] ) {
			pTitleList_Nand[l+1].flags.isValid = TRUE;
			pTitleList_Nand[l+1].flags.bootType = LAUNCHER_BOOTTYPE_NAND;
		}
	}
	// return : *TitleProperty Array
	return listNum;
}

static BOOL SYSMi_ReadBanner_NAND( NAMTitleId titleID, u8 *pDst )
{
#define PATH_LENGTH		1024
	OSTick start;
	FSFile  file[1];
	BOOL bSuccess;
	char path[PATH_LENGTH];
	s32 readLen;
	s32 offset;
	
	start = OS_GetTick();
	readLen = NAM_GetTitleBootContentPathFast( path, titleID );
	OS_TPrintf( "NAM_GetTitleBootContentPath : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	
	// ファイルパスを取得
	if(readLen != NAM_OK){
		OS_TPrintf("NAM_GetTitleBootContentPath failed %lld,%d\n", titleID, readLen );
		return FALSE;
	}
	
	// ファイルオープン
	bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);
	if( ! bSuccess )
	{
		OS_TPrintf("SYSM_GetNandTitleList failed: cant open file %s\n",path);
		return FALSE;
	}
	
	// ROMヘッダのバナーデータオフセットを読み込む
	bSuccess = FS_SeekFile(file, 0x68, FS_SEEK_SET);
	if( ! bSuccess )
	{
		OS_TPrintf("SYSM_GetNandTitleList failed: cant seek file(0)\n");
		FS_CloseFile(file);
		return FALSE;
	}
	readLen = FS_ReadFile(file, &offset, sizeof(offset));
	if( readLen != sizeof(offset) )
	{
		OS_TPrintf("SYSM_GetNandTitleList failed: cant read file\n");
		FS_CloseFile(file);
		return FALSE;
	}
	
	// バナーが存在する場合のみリード
	if( offset ) {
		bSuccess = FS_SeekFile(file, offset, FS_SEEK_SET);
		if( ! bSuccess )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant seek file(offset)\n");
			FS_CloseFile(file);
			return FALSE;
		}
		readLen = ReadFile( file, pDst, (s32)sizeof(TWLBannerFile) );
		if( readLen != (s32)sizeof(TWLBannerFile) )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant read file2\n");
			FS_CloseFile(file);
			return FALSE;
		}
	}else {
		// バナーが存在しない場合はバッファクリア
		MI_CpuClearFast( pDst, sizeof(TWLBannerFile) );
	}
	
	FS_CloseFile(file);
	return TRUE;
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
	
	switch( pBootTitle->flags.bootType )
	{
	case LAUNCHER_BOOTTYPE_NAND:
		// NAND
    	NAM_GetTitleBootContentPathFast(path, pBootTitle->titleID);
		break;
	case LAUNCHER_BOOTTYPE_ROM:
		// TODO:CARD未読の場合の処理
		break;
	case LAUNCHER_BOOTTYPE_TEMP:
		// tmpフォルダ
		STD_TSNPrintf( path, 31, "nand:/tmp/%.16llx.srl", pBootTitle->titleID );
		break;
	default:
		// unknown
		return;
	}

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
		/*
		else if( pBootTitle->titleID != *((NAMTitleId *)(&header[0x230])) )
		{
			//TWL対応ROMで、ヘッダのtitleIDが起動指定されたIDと違う
OS_TPrintf("RebootSystem failed: header TitleID error\n");
OS_TPrintf("RebootSystem failed: selectedTitleID=%.16llx\n",pBootTitle->titleID);
OS_TPrintf("RebootSystem failed: headerTitleID=%.16llx\n",*((NAMTitleId *)(&header[0x230])));
            FS_CloseFile(file);
            return;
		}
		*/
		
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
	
	SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
}


// 指定タイトルを別スレッドでロード開始する
void SYSM_StartLoadTitle( TitleProperty *pBootTitle )
{
#define THREAD_PRIO 17
#define STACK_SIZE 0xc00
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	
	SYSMi_EnableHotSW( FALSE );
	
	// アプリ未ロード状態なら、ロード開始
	if( !pBootTitle->flags.isAppLoadCompleted ) {
		SYSMi_GetWork()->flags.common.isLoadSucceeded = FALSE;
		OS_InitThread();
		OS_CreateThread( &s_thread, (void (*)(void *))SYSMi_LoadTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
		OS_WakeupThreadDirect( &s_thread );
	}else if( pBootTitle->flags.isAppRelocate ) {
	// アプリロード済みで、再配置要求ありなら、再配置（カードのみ対応）
		SYSMi_Relocate();
		SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
	}else
	{
		// アプリロード済みで、再配置要求なし
		SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
	}
	
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM ) {
		SYSMi_GetWork()->flags.common.isCardBoot = TRUE;
	}else if(pBootTitle->flags.isAppLoadCompleted)
	{
		// カードブートでなく、ロード済みの場合、再配置情報をランチャーパラメタから読み込み
		MI_CpuCopy8( SYSM_GetLauncherParamBody()->v1.relocInfo, SYSMi_GetWork()->romRelocateInfo, sizeof(Relocate_Info)*RELOCATE_INFO_NUM );
		// 更にヘッダを再配置
		if( ((ROM_Header_Short *)(OS_TWL_HEADER_PRELOAD_MMEM))->platform_code & PLATFORM_CODE_FLAG_TWL ) {
			// TWLモード
			// TWL-ROMヘッダ情報の再配置
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		}else {
			// NTRモード
			// TWL-ROMヘッダ情報の再配置
			//   ランチャーのROMヘッダが残っている非コピー領域もクリア
			MI_CpuClearFast( (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
			// NTR-ROMヘッダ情報の再配置は、rebootライブラリで行う。
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
		// TWLモード
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
		// NTRモード
		// TWL-ROMヘッダ情報の再配置
		//   ランチャーのROMヘッダが残っている非コピー領域もクリア
		MI_CpuClearFast( (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	}
}


// アプリロード済み？
BOOL SYSM_IsLoadTitleFinished( TitleProperty *pBootTitle )
{
	if( pBootTitle->flags.isAppLoadCompleted ) {
		return TRUE;
	}
	return OS_IsThreadTerminated( &s_thread );
}

static AuthResult SYSMi_AuthenticateTWLHeader( TitleProperty *pBootTitle )
{
	// [TODO:] NANDアプリの場合、NAM_CheckTitleLaunchRights()を呼んでチェック
	// [TODO:] TWLアプリの場合、pBootTitle->titleIDとROMヘッダのtitleIDの一致確認を必ずする。
	
	// TWLアプリの場合、署名チェック、鍵の場合わけと署名の解読、ハッシュ値をヘッダに格納されているものと比較
    // titleIDからアプリ種別を読み取る
    if( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->platform_code ) != 0 )
    {
		const u8 *key;
		u16 prop;
		u8 keynum;
		u8 buf[0x80];
		u8 calculated_hash[SVC_SHA1_DIGEST_SIZE];
		SVCSignHeapContext con;
		int l;
		u32 *module_addr[RELOCATE_INFO_NUM];
		u32 module_size[RELOCATE_INFO_NUM];
		u8 *hash_addr[RELOCATE_INFO_NUM];
		
	    prop = ((u16 *)&(pBootTitle->titleID))[2];
	    prop = (u16)(prop & 0x1); // prop = 0:UserApp 1:SystemApp 2:ShopApp?
	    keynum = (u8)( prop == 0 ? 2 : (prop == 1 ? 0 : 1) );// keynum = 0:SystemApp 1:ShopApp 2:UserApp
		// アプリ種別とボンディングオプションによって使う鍵を分ける
	    if( SCFG_GetBondingOption() == 0 ) {
			// 製品版鍵取得
			key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[SYSTEM_APP_KEY_OFFSET + keynum];
	    }else {
			// 開発版
			key = g_devPubKey[keynum];
	    }
	    // 署名を鍵で復号
	    MI_CpuClear8( buf, 0x80 );
	    SVC_InitSignHeap( &con, (void *)SIGN_HEAP_ADDR, SIGN_HEAP_SIZE );// ヒープの初期化
	    if( !SVC_DecryptSign( &con, buf, (( ROM_Header *)HW_TWL_ROM_HEADER_BUF)->signature, key ))
	    {
			OS_TPrintf("Authenticate failed: Sign decryption failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
		// ヘッダのハッシュ(SHA1)計算
		SVC_CalcSHA1( &calculated_hash, (const void*)HW_TWL_ROM_HEADER_BUF, ROM_HEADER_HASH_CALC_DATA_LEN );
	    // 署名のハッシュ値とヘッダのハッシュ値を比較
	    if(!SVC_CompareSHA1((const void *)(&buf[ROM_HEADER_HASH_OFFSET]), (const void *)&calculated_hash))
	    {
			OS_TPrintf("Authenticate failed: Sign check failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf("Authenticate : Sign check succeed.\n");
		}
	    
		// それぞれARM9,7のFLXおよびLTDについてハッシュを計算してヘッダに格納されているハッシュと比較
		module_addr[ARM9_STATIC] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->main_ram_address;
		module_addr[ARM7_STATIC] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->sub_ram_address;
		module_addr[ARM9_LTD_STATIC] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->main_ltd_ram_address;
		module_addr[ARM7_LTD_STATIC] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->sub_ltd_ram_address;
		
		module_size[ARM9_STATIC] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->main_size;
		module_size[ARM7_STATIC] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->sub_size;
		module_size[ARM9_LTD_STATIC] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->main_ltd_size;
		module_size[ARM7_LTD_STATIC] = (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->sub_ltd_size;
		
		hash_addr[ARM9_STATIC] = &((( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->main_static_digest[0]);
		hash_addr[ARM7_STATIC] = &((( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->sub_static_digest[0]);
		hash_addr[ARM9_LTD_STATIC] = &((( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->main_ltd_static_digest[0]);
		hash_addr[ARM7_LTD_STATIC] = &((( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->sub_ltd_static_digest[0]);
		
		for( l=0; l<RELOCATE_INFO_NUM ; l++ )
		{
			static const char *str[4]={"ARM9_STATIC","ARM7_STATIC","ARM9_LTD_STATIC","ARM7_LTD_STATIC"};
			// 一時的に格納位置をずらしている場合は、再配置情報からモジュール格納アドレスを取得
			if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL )
			{
				module_addr[l] = (u32 *)SYSMi_GetWork()->romRelocateInfo[l].src;
			}
			// ハッシュ計算
			SVC_CalcHMACSHA1( &calculated_hash, (const void*)module_addr[l], module_size[l],
							 (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
			// 比較
		    if(!SVC_CompareSHA1((const void *)hash_addr[l], (const void *)&calculated_hash))
		    {
				OS_TPrintf("Authenticate failed: rom header hash check failed.\n");
				return AUTH_RESULT_AUTHENTICATE_FAILED;
			}else
			{
				OS_TPrintf("Authenticate : %s module hash check succeed.\n", str[l]);
			}
		}
	}
	return AUTH_RESULT_SUCCEEDED;
}

// ロード済みの指定タイトルの認証とブートを行う
AuthResult SYSM_AuthenticateTitle( TitleProperty *pBootTitle )
{
	AuthResult res;
	
	// ロード中
	if( !SYSM_IsLoadTitleFinished( pBootTitle ) ) {
		return AUTH_RESULT_PROCESSING;
	}
	// ロード成功？
	if( SYSMi_GetWork()->flags.common.isLoadSucceeded == FALSE )
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
	
	// BOOTTYPE_MEMORYでNTRモードのFSありでブートすると、旧NitroSDKでビルドされたアプリの場合、
	// ROMアーカイブにカードが割り当てられて、FSで関係ないカードにアクセスにいってしまうので、それを防止する。
	if( ( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_MEMORY ) &&
		( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->platform_code ) == 0 ) &&
		( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->fat_size ) > 0 )
		) {
		return AUTH_RESULT_TITLE_BOOTTYPE_ERROR;
	}
	
	// ※ROMヘッダ認証
	res = SYSMi_AuthenticateTWLHeader( pBootTitle );
	if( res != AUTH_RESULT_SUCCEEDED )
	{
		return res;
	}
	
	// マウント情報の登録
	SYSMi_SetBootAppMountInfo( pBootTitle );
	
	// HW_WM_BOOT_BUFへのブート情報セット
	( (OSBootInfo *)OS_GetBootInfo() )->boot_type = s_launcherToOSBootType[ pBootTitle->flags.bootType ];
	
	BOOT_Ready();	// never return.
	
	return AUTH_RESULT_SUCCEEDED;
}


#if 0
// 指定タイトルの認証＆ロード　※１フレームじゃ終わらん。
// もしかすると使わないかも
void SYSM_LoadAndAuthenticateTitleThread( TitleProperty *pBootTitle )
{
	SYSMi_LoadTitleThreadFunc( pBootTitle );
	OS_JoinThread(&s_thread);
	
	// 認証
	return SYSM_AuthenticateTitle( pBootTitle );
}
#endif


// 指定タイトルがブート可能なポインタかチェック
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle )
{
#pragma unused( pBootTitle )
	
	return TRUE;
}

#if 0
void CheckDigest( void )
{
	int i;
	for( i = 0; i < 4; i++ ) {
		if( SYSMi_GetWork()->reloc_info[ i ].src ) {
			
		}else {
		}
	}
}
#endif


// 活線挿抜有効／無効をセット
void SYSMi_EnableHotSW( BOOL enable )
{
	enable = enable ? 1 : 0;
	
	// 現在の値と同じなら何もせずリターン
	if( SYSMi_GetWork()->flags.common.isEnableHotSW == enable ) {
		return;
	}
	
	{
		u16 id = (u16)OS_GetLockID();
		(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
		if( !SYSMi_GetWork()->flags.common.isBusyHotSW ) {
			// ARM7側がビジーでなければ、直接書き換え
			SYSMi_GetWork()->flags.common.isEnableHotSW = enable;
		}else {
			// ARM7側がビジーなら、変更要求をしてARM7が値を書き換えてくれるのを待つ。
			SYSMi_GetWork()->flags.arm9.reqChangeHotSW = 1;
			SYSMi_GetWork()->flags.arm9.nextHotSWStatus = enable;
		}
		(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
		OS_ReleaseLockID( id );
	}
	
	// 値が変化するまでスリープして待つ。
	while( SYSMi_GetWork()->flags.common.isEnableHotSW != enable ) {
		OS_Sleep( 2 );
	}
}
