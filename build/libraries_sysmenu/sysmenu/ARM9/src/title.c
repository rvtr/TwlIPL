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
#include <firm/format/from_firm.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include "internal_api.h"
#include "fs_wram.h"

// define data-----------------------------------------------------------------
#define CARD_BANNER_INDEX			( LAUNCHER_TITLE_LIST_NUM - 1 )

#define LAUNCHER_KEY_INDEX			0 // ファームから送られてくる鍵のうちLauncherキーのインデックス
#define SYSTEM_APP_KEY_INDEX		1 // ファームから送られてくるSYSTEMアプリキーのインデックス
#define SECURE_APP_KEY_INDEX		2 // ファームから送られてくるSECUREアプリキーのインデックス
#define USER_APP_KEY_INDEX			3 // ファームから送られてくるUSERアプリキーのインデックス

#define ROM_HEADER_HASH_OFFSET		(0x0) // 署名からROMヘッダハッシュを取り出すためのオフセット

#define SIGN_HEAP_ADDR	0x023c0000	// 署名計算のためのヒープ領域開始アドレス
#define SIGN_HEAP_SIZE	0x1000		// 署名計算のためのヒープサイズ
#define ARM9_ENCRYPT_DEF_SIZE	0x800	// ARM9FLXの先頭暗号化部分のサイズ

#define	DIGEST_HASH_BLOCK_SIZE_SHA1					(512/8)
#define TWL_ROM_HEADER_HASH_CALC_DATA_LEN	0xe00 // ROMヘッダのハッシュ計算する部分の長さTWL版
#define NTR_ROM_HEADER_HASH_CALC_DATA_LEN	0x160 // ROMヘッダのハッシュ計算する部分の長さDS版

#define AUTH_KEY_BUFFER_LEN 128
#define MB_AUTH_SIGN_SIZE				(128)	/* digital sign size */

#define WRAM_SLOT_FOR_FS	4
#define WRAM_SIZE_FOR_FS	MI_WRAM_SIZE_128KB

typedef	struct	MbAuthCode
{
	char		magic_code[2];			// マジックナンバー
	u16		version;			// バージョン
	u8		sign[MB_AUTH_SIGN_SIZE];	// 署名
	u32		serial_number;			// シリアル番号
} MbAuthCode;	// 16byte

typedef struct CalcHMACSHA1CallbackArg
{
	SVCHMACSHA1Context	ctx;
	u32					hash_length;
} CalcHMACSHA1CallbackArg;

typedef struct CalcSHA1CallbackArg
{
	SVCSHA1Context	ctx;
	u32					hash_length;
} CalcSHA1CallbackArg;

// extern data-----------------------------------------------------------------
extern const u8 g_devPubKey[ 4 ][ 0x80 ];

// function's prototype-------------------------------------------------------
static s32  ReadFile( FSFile* pf, void* buffer, s32 size );
static void SYSMi_EnableHotSW( BOOL enable );
static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle );
static void SYSMi_Relocate( void );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );
static void SYSMi_makeTitleIdList( void );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static OSThread			s_thread;
static OSThread			s_auth_thread;
static TWLBannerFile	s_bannerBuf[ LAUNCHER_TITLE_LIST_NUM ] ATTRIBUTE_ALIGN(32);
static AuthResult		s_authResult = AUTH_RESULT_PROCESSING;	// ROM検証結果

static MbAuthCode s_authcode;

static BOOL				s_loadstart = FALSE;

static NAMTitleId *s_pTitleIDList = NULL;
static int s_listLength = 0;

static u8 *s_calc_hash = NULL;

// const data------------------------------------------------------------------
static const OSBootType s_launcherToOSBootType[ LAUNCHER_BOOTTYPE_MAX ] = {
    OS_BOOTTYPE_ILLEGAL,	// ILLEGAL
    OS_BOOTTYPE_ROM,		// ROM
    OS_BOOTTYPE_NAND,		// TEMP
    OS_BOOTTYPE_NAND,		// NAND
    OS_BOOTTYPE_MEMORY,		// MEMORY
};

// HMAC_SHA1用鍵
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

// ダウンロードアプリ署名用公開鍵
static const u8 nitro_dl_sign_key[AUTH_KEY_BUFFER_LEN] = {
  0x9E,0xC1,0xCC,0xC0,0x4A,0x6B,0xD0,0xA0,0x6D,0x62,0xED,0x5F,0x15,0x67,0x87,0x12,
  0xE6,0xF4,0x77,0x1F,0xD8,0x5C,0x81,0xCE,0x0C,0xD0,0x22,0x31,0xF5,0x89,0x08,0xF5,
  0xBE,0x04,0xCB,0xC1,0x4F,0x63,0xD9,0x5A,0x98,0xFF,0xEB,0x36,0x0F,0x9C,0x5D,0xAD,
  0x15,0xB9,0x99,0xFB,0xC6,0x86,0x2C,0x0A,0x0C,0xFC,0xE6,0x86,0x03,0x60,0xD4,0x87,
  0x28,0xD5,0x66,0x42,0x9C,0xF7,0x04,0x14,0x4E,0x6F,0x73,0x20,0xC3,0x3E,0x3F,0xF5,
  0x82,0x2E,0x78,0x18,0xD6,0xCD,0xD5,0xC2,0xDC,0xAA,0x1D,0x34,0x91,0xEC,0x99,0xC9,
  0xF7,0xBF,0xBF,0xA0,0x0E,0x1E,0xF0,0x25,0xF8,0x66,0x17,0x54,0x34,0x28,0x2D,0x28,
  0xA3,0xAE,0xF0,0xA9,0xFA,0x3A,0x70,0x56,0xD2,0x34,0xA9,0xC5,0x9E,0x5D,0xF5,0xE1
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
	
	if(s_loadstart)
	{
		// ロード開始していたら、もうヘッダやタイトル情報は変更しない
		return retval;
	}
	// [TODO:] ROMヘッダの platform_code がNTR,TWL-HYB,TWL-LTD以外のもの
	//                     region_codeが本体情報と違うもの
	//         の場合は、正常に認識できないタイトルであることを示す。
	
	if( SYSMi_GetWork()->flags.hotsw.isCardStateChanged ) {
		
		MI_CpuClear32( pTitleList_Card, sizeof(TitleProperty) );
		
		// ROMヘッダバッファのコピー
		if( SYSM_IsExistCard() ) {
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );						// ARM7と排他制御する
			
			// ROMヘッダのリード
			(void)SYSMi_CopyCardRomHeader();
			
			// バナーデータのリード
			(void)SYSMi_CopyCardBanner();
			
			SYSMi_GetWork()->flags.hotsw.isCardStateChanged = FALSE;							// カード情報更新フラグを落とす
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );					// ARM7と排他制御する
			OS_ReleaseLockID( id );

			pTitleList_Card->pBanner = &s_bannerBuf[ CARD_BANNER_INDEX ];
			pTitleList_Card->flags.isValid = TRUE;
			pTitleList_Card->flags.isAppLoadCompleted = TRUE;
			pTitleList_Card->flags.isAppRelocate = TRUE;
		}
		
		// タイトル情報フラグのセット
		pTitleList_Card->flags.bootType = LAUNCHER_BOOTTYPE_ROM;
		pTitleList_Card->titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
		retval = TRUE;
	}
	
	return retval;
}

// カードROMヘッダのARM7バッファからARM9バッファへのコピー
BOOL SYSMi_CopyCardRomHeader( void )
{
	BOOL retval = FALSE;

	if( SYSM_IsExistCard() ) {
		// ROMヘッダのリード
		DC_InvalidateRange( (void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );	// キャッシュケア
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BAK, (void *)SYSM_CARD_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );	// ROMヘッダコピー
		SYSMi_GetWork()->cardHeaderCrc16 = SYSMi_GetWork()->cardHeaderCrc16_bak;			// ROMヘッダCRCコピー

		retval = TRUE;
	}

	return retval;
}

// カードバナーのARM7バッファからARM9バッファへのコピー
BOOL SYSMi_CopyCardBanner( void )
{
	BOOL retval = FALSE;

	if( SYSM_IsExistCard() ) {
		// バナーデータのリード
		SYSMi_ReadCardBannerFile( SYSM_GetCardRomHeader()->banner_offset, &s_bannerBuf[ CARD_BANNER_INDEX ] );

		retval = TRUE;
	}

	return retval;
}

// インポートされているすべてのNANDアプリを列挙したリストの準備
// SYSM_GetNandTitleListおよびSYSM_TryToBootTitle前に呼ぶ必要あり
BOOL SYSM_InitNandTitleList( void )
{
	OSTick start;

	if( s_pTitleIDList != NULL ) return TRUE;

	// インポートされているタイトルの取得
	start = OS_GetTick();
	s_listLength = NAM_GetNumTitles();
	OS_TPrintf( "NAM_GetNumTitles : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	s_pTitleIDList = SYSM_Alloc( sizeof(NAMTitleId) * s_listLength );
	if( s_pTitleIDList == NULL ) {
		OS_TPrintf( "%s: alloc error.\n", __FUNCTION__ );
		return FALSE;
	}
	start = OS_GetTick();
	(void)NAM_GetTitleList( s_pTitleIDList, (u32)s_listLength );
	OS_TPrintf( "NAM_GetTitleList : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	
	return TRUE;
}

// NANDアプリリストの解放
void SYSM_FreeNandTitleList( void )
{
	if(s_pTitleIDList != NULL)
	{
		SYSM_Free( s_pTitleIDList );
		s_pTitleIDList = NULL;
	}
}

// ローンチ対象となるNANDタイトルリストの取得
// listNumには、pTitleList_Nandの長さを与える
// 得られる最大のタイトル数は、(LAUNCHER_TITLE_LIST_NUM - 1)に制限される（ランチャーが表示できる最大数からカードぶんを引いた数）
// return:取得したNANDタイトルの数
int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int listNum )
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, 等の条件を指定してタイトルリストを取得する。
	// とりあえずALL
	int l;
	int validNum = 0;
	NAMTitleId titleIDArray[ LAUNCHER_TITLE_LIST_NUM - 1 ];// ローンチ可能なタイトルリストの一時置き場
	
	if( s_pTitleIDList == NULL ) return -1;
	
	// 取得したタイトルがローンチ対象かどうかをチェック
	for( l = 0; l < s_listLength; l++ ) {
		// "Not Launch"でない　かつ　"Data Only"でない　なら有効なタイトルとしてリストに追加
		if( ( s_pTitleIDList[ l ] & ( TITLE_ID_NOT_LAUNCH_FLAG_MASK | TITLE_ID_DATA_ONLY_FLAG_MASK ) ) == 0 ) {
			titleIDArray[ validNum ] = s_pTitleIDList[ l ];
			SYSMi_ReadBanner_NAND( s_pTitleIDList[ l ], &s_bannerBuf[ validNum ] );
			validNum++;
			if( !( validNum < LAUNCHER_TITLE_LIST_NUM - 1 ) )// 最大(LAUNCHER_TITLE_LIST_NUM - 1)まで
			{
				break;
			}
		}
	}
	
	// 念のため残り領域を0クリア
	for( l = validNum; l < LAUNCHER_TITLE_LIST_NUM - 1; l++ ) {
		titleIDArray[ l ] = 0;
	}
	
	// 最終リストに対して、カードアプリ部分を除いた部分をクリア
	MI_CpuClearFast( &pTitleList_Nand[ 1 ], sizeof(TitleProperty) * ( listNum - 1 ) );
	
	listNum--; // カードのぶん引いておく
	
	// 引数に与えられたリストの長さ-1 と、ローンチ可能タイトルリストの長さの比較
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

static void SYSMi_CalcHMACSHA1Callback(const void* addr, u32 len, void* arg)
{
	CalcHMACSHA1CallbackArg *cba = (CalcHMACSHA1CallbackArg *)arg;
	u32 calc_len = ( cba->hash_length < len ? cba->hash_length : len );
	if( calc_len == 0 ) return;
	cba->hash_length -= calc_len;
	SVC_HMACSHA1Update( &cba->ctx, addr, calc_len );
}

static void SYSMi_CalcSHA1Callback(const void* addr, u32 len, void* arg)
{
	CalcSHA1CallbackArg *cba = (CalcSHA1CallbackArg *)arg;
	u32 calc_len = ( cba->hash_length < len ? cba->hash_length : len );
	if( calc_len == 0 ) return;
	cba->hash_length -= calc_len;
	SVC_SHA1Update( &cba->ctx, addr, calc_len );
}

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
	// [TODO:]DSダウンロードプレイおよびpictochat等のNTR拡張NANDアプリの時は、ROMヘッダを退避する
	// が、どうもNTR-ROMヘッダ情報の再配置は、rebootライブラリで行う。らしい。
	// とりあえず現状のままにしておき、動かなければ考える。
	
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
		STD_TSNPrintf( path, 256, OS_TMP_APP_PATH, pBootTitle->titleID );
		break;
	default:
		// unknown
		return;
	}

	FS_InitFile( file );
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
        ROM_Header *head = (ROM_Header *)header;

        // まずROMヘッダを読み込む
        // (本来ならここでSRLの正当性判定)
        bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);

        if( ! bSuccess )
        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
            FS_CloseFile(file);
            return;
        }

        readLen = FS_ReadFile(file, header, (s32)sizeof(header));

        if( readLen != (s32)sizeof(header) )
        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", header, 0, sizeof(header), readLen);
            FS_CloseFile(file);
            return;
        }

        if( head->s.nintendo_logo_crc16 != 0xCF56 )
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
        
        if( !(head->s.platform_code & PLATFORM_CODE_FLAG_TWL) )
        {
			//NTR専用ROM or NTR TWL両方非対応のアプリ
			isTwlApp = FALSE;
			if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP)
			{
				// NTR-DLアプリの場合はDLアプリ署名データを取得しておく
				u32 valid_size = ( head->s.rom_valid_size ? head->s.rom_valid_size : 0x01000000 );

		        bSuccess = FS_SeekFile(file, (s32)valid_size, FS_SEEK_SET);
		        if( ! bSuccess )
		        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
		            FS_CloseFile(file);
		            return;
		        }
		        readLen = FS_ReadFile(file, &s_authcode, (s32)sizeof(s_authcode));
		        if( readLen != (s32)sizeof(s_authcode) )
		        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", &s_authcode, 0, sizeof(s_authcode), readLen);
		            FS_CloseFile(file);
		            return;
		        }
			}
		}
		
        // 各領域を読み込む
        source  [region_header  ] = 0x00000000;
        length  [region_header  ] = HW_TWL_ROM_HEADER_BUF_SIZE;
        destaddr[region_header  ] = SYSM_CARD_ROM_HEADER_BUF;
		
        source  [region_arm9_ntr] = head->s.main_rom_offset;
        length  [region_arm9_ntr] = head->s.main_size;
        destaddr[region_arm9_ntr] = (u32)head->s.main_ram_address;
		
        source  [region_arm7_ntr] = head->s.sub_rom_offset;
        length  [region_arm7_ntr] = head->s.sub_size;
        destaddr[region_arm7_ntr] = (u32)head->s.sub_ram_address;
		
		if( isTwlApp )
		{
	        source  [region_arm9_twl] = head->s.main_ltd_rom_offset;
	        length  [region_arm9_twl] = head->s.main_ltd_size;
	        destaddr[region_arm9_twl] = (u32)head->s.main_ltd_ram_address;
			
	        source  [region_arm7_twl] = head->s.sub_ltd_rom_offset;
	        length  [region_arm7_twl] = head->s.sub_ltd_size;
	        destaddr[region_arm7_twl] = (u32)head->s.sub_ltd_ram_address;
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

		// WRAM利用Read関数の準備、WRAMCの後半だけ解放しておく
		FS_InitWramTransfer(3);
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
		
		s_calc_hash = SYSM_Alloc( region_max * SVC_SHA1_DIGEST_SIZE );

        for (i = region_header; i < region_max; ++i)
        {
			BOOL result;
            u32 len = MATH_ROUNDUP( length[i], SYSM_ALIGNMENT_LOAD_MODULE );// AES暗号化領域の関係で、ロードサイズは32バイトアライメントに補正
            
            if ( !isTwlApp && i >= region_arm9_twl ) continue;// nitroでは読み込まない領域

            bSuccess = FS_SeekFile(file, (s32)source[i], FS_SEEK_SET);

            if( ! bSuccess )
            {
OS_TPrintf("RebootSystem failed: cant seek file(%d)\n", source[i]);
                FS_CloseFile(file);
                return;
            }

#ifdef LOAD_APP_VIA_WRAM
OS_TPrintf("RebootSystem : Load VIA WRAM %d.\n", i);
            // [TODO:]ここで同時にハッシュ計算やAES処理もやってしまう予定
            // 別スレッドで同じWRAM使おうとすると多分コケるので注意
            
            // コールバック関数に与える引数を初期化してRead
            if(region_header == i || (!isTwlApp && pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP ) )
            {
				// ヘッダか、NTRダウンロードアプリのモジュール
				CalcSHA1CallbackArg arg;
	            SVC_SHA1Init( &arg.ctx );
	            arg.hash_length = (u32)(region_header != i ? length[i] : 
	            						(isTwlApp ? TWL_ROM_HEADER_HASH_CALC_DATA_LEN : NTR_ROM_HEADER_HASH_CALC_DATA_LEN) );
	            result = FS_ReadFileViaWram(file, (void *)destaddr[i], (s32)len, MI_WRAM_C,
	            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcSHA1Callback, &arg );
	            SVC_SHA1GetHash( &arg.ctx, &s_calc_hash[i * SVC_SHA1_DIGEST_SIZE] );
			}else
			{
				// それ以外
				CalcHMACSHA1CallbackArg arg;
	            SVC_HMACSHA1Init( &arg.ctx, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
	            arg.hash_length = length[i];
	            result = FS_ReadFileViaWram(file, (void *)destaddr[i], (s32)len, MI_WRAM_C,
	            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcHMACSHA1Callback, &arg );
	            SVC_HMACSHA1GetHash( &arg.ctx, &s_calc_hash[i * SVC_SHA1_DIGEST_SIZE] );
			}
			if ( !result )
			{
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
                FS_CloseFile(file);
                return;
			}
#else
OS_TPrintf("RebootSystem : Load VIA PRIMAL FS %d.\n", i);
            readLen = FS_ReadFile(file, (void *)destaddr[i], (s32)len);

            if( readLen < 0 )
            {
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
                FS_CloseFile(file);
                return;
            }
#endif // LOAD_APP_VIA_WRAM

			// ヘッダ読み込み完了フラグ
			if( i == region_header )
			{
				SYSMi_GetWork()->flags.common.isHeaderLoadCompleted = TRUE;
			}

        }

        (void)FS_CloseFile(file);

    }
	SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
}


// 指定タイトルを別スレッドでロード開始する
void SYSM_StartLoadTitle( TitleProperty *pBootTitle )
{
#define THREAD_PRIO 17
#define STACK_SIZE 0xc00
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	
	SYSMi_EnableHotSW( FALSE );
	s_loadstart = TRUE;
	// このあとCardRomヘッダバッファにROMヘッダを上書きで読み込むので
	// この時点でHotSWが止まっていないと、さらにカードのROMヘッダ
	// を上書きしてしまう可能性がある

	// 一応、アプリロード開始前に検証結果をPROCESSINGにセット
	s_authResult = AUTH_RESULT_PROCESSING;
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
		// カードブートでなく、ロード済みの場合は今のところ何もしない
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
	}
}

// アプリロード済み？
BOOL SYSM_IsLoadTitleFinished( void )
{
	if( SYSMi_GetWork()->flags.common.isLoadSucceeded ) {
		return TRUE;
	}
	return OS_IsThreadTerminated( &s_thread );
}

// TWLアプリおよびNTR拡張NANDアプリ共通のヘッダ認証処理
static AuthResult SYSMi_AuthenticateTWLHeader( TitleProperty *pBootTitle )
{
	ROM_Header *head;
	OSTick start,prev;
	start = OS_GetTick();
	
	head = ( ROM_Header *)SYSM_CARD_ROM_HEADER_BUF;
	
	// NANDアプリの場合、NAM_CheckTitleLaunchRights()を呼んでチェック
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND )
	{
		if( NAM_OK != NAM_CheckTitleLaunchRights( pBootTitle->titleID ))
		{
			OS_TPrintf("Authenticate failed: NAM_CheckTitleLaunchRights failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf("Authenticate : NAM_CheckTitleLaunchRights succeed. %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
		}
	}

	// 署名処理
    {
		const u8 *key;
		u32 hi;
		u8 keynum;
		SignatureData sigbuf;
		u8 calculated_hash[SVC_SHA1_DIGEST_SIZE];
		SVCSignHeapContext con;
		int l;
		u32 *module_addr[RELOCATE_INFO_NUM];
		u32 module_size[RELOCATE_INFO_NUM];
		u8 *hash_addr[RELOCATE_INFO_NUM];
		int module_num;
		BOOL b_dev = FALSE;
		char *gamecode = (char *)&(pBootTitle->titleID);
		
		// pBootTitle->titleIDとROMヘッダのtitleIDの一致確認をする。
		if( pBootTitle->titleID != head->s.titleID )
		{
			//TWL対応ROMで、ヘッダのtitleIDが起動指定されたIDと違う
			OS_TPrintf( "Authenticate failed: header TitleID error\n" );
			OS_TPrintf( "Authenticate failed: selectedTitleID=%.16llx\n", pBootTitle->titleID );
			OS_TPrintf( "Authenticate failed: headerTitleID=%.16llx\n", head->s.titleID );
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf( "Authenticate : header TitleID check succeed.\n" );
		}
		
		prev = OS_GetTick();
		hi = head->s.titleID_Hi;
		// Launcherは専用の鍵を使う
		if(  0 == STD_CompareNString( &gamecode[1], "ANH", 3 ) )
		{
			keynum = LAUNCHER_KEY_INDEX;
		}else
		{
			// keynum = 1:SystemApp 2:SecureApp 3:UserApp
			keynum = (u8)( (hi & TITLE_ID_HI_SECURE_FLAG_MASK) ? SECURE_APP_KEY_INDEX
							: ( (hi & TITLE_ID_HI_APP_TYPE_MASK) ? SYSTEM_APP_KEY_INDEX : USER_APP_KEY_INDEX )
						);
		}
		// アプリ種別とボンディングオプションによって使う鍵を分ける
// #define LNC_PDTKEY_DBG
#ifdef LNC_PDTKEY_DBG
		{
			// 注：デバグ用コード。
			// 開発用TSボードで開発版ROMおよび製品版ROMの署名チェックとAESデクリプトをデバグするためのコード
			if( head->s.developer_encrypt )
			{
				// 開発版鍵取得
				key = g_devPubKey[keynum];
			}else
			{
				// 製品版鍵取得
				key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[keynum];
			}
			// デバッガが有効でTLF読み込みならば、ハッシュチェックスルーフラグを立てる
			if(SYSMi_GetWork()->flags.hotsw.isOnDebugger && SYSMi_GetWork()->romEmuInfo.isTlfRom )
			{
				b_dev = TRUE;
			}
		}
#else
	    if( SCFG_GetBondingOption() == 0 ) {
			// 製品版鍵取得
			key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[keynum];
	    }else {
			// 開発版
			key = g_devPubKey[keynum];
			// デバッガが有効でTLF読み込みならば、ハッシュチェックスルーフラグを立てる
			if(SYSMi_GetWork()->flags.hotsw.isOnDebugger && SYSMi_GetWork()->romEmuInfo.isTlfRom )
			{
				b_dev = TRUE;
			}
	    }
#endif
	    // 署名を鍵で復号
	    MI_CpuClear8( &sigbuf, sizeof(sigbuf) );
	    SVC_InitSignHeap( &con, (void *)SIGN_HEAP_ADDR, SIGN_HEAP_SIZE );// ヒープの初期化
	    if( !SVC_DecryptSign( &con, sigbuf.digest, head->signature, key ))
	    {
			OS_TPrintf("Authenticate failed: Sign decryption failed.\n");
			if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
#ifdef LOAD_APP_VIA_WRAM
		if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM )
#endif
		{
			// ヘッダのハッシュ(SHA1)計算
			SVC_CalcSHA1( calculated_hash, (const void*)head, TWL_ROM_HEADER_HASH_CALC_DATA_LEN );
		    // 署名のハッシュ値とヘッダのハッシュ値を比較
		    if(!SVC_CompareSHA1(sigbuf.digest, (const void *)calculated_hash))
		    {
				OS_TPrintf("Authenticate failed: Sign check failed.\n");
				if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
			}else
			{
				OS_TPrintf("Authenticate : Sign check succeed. %dms.\n", OS_TicksToMilliSeconds(OS_GetTick() - prev));
			}
		}
#ifdef LOAD_APP_VIA_WRAM
		else if(s_calc_hash)
		{
		    // 署名のハッシュ値とヘッダのハッシュ値を比較
		    if(!SVC_CompareSHA1(sigbuf.digest, (const void *)&s_calc_hash[0]))
		    {
				OS_TPrintf("Authenticate failed: Sign check failed.\n");
				if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
			}else
			{
				OS_TPrintf("Authenticate : Sign check succeed. %dms.\n", OS_TicksToMilliSeconds(OS_GetTick() - prev));
			}
		}else
		{
			OS_TPrintf("Authenticate failed: Sign check failed.\n");
			if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
#endif
		
		// TWL以降のアプリはモジュールの特定領域がAES暗号化されているので、ハッシュチェック前にデクリプトする必要がある。
		// ヘッダのデータを使うので、署名チェック後が望ましい。よってこのタイミング。
		SYSM_StartDecryptAESRegion( &(head->s) );
	    
		// それぞれARM9,7のFLXおよびLTDについてハッシュを計算してヘッダに格納されているハッシュと比較
		module_addr[ARM9_STATIC] = head->s.main_ram_address;
		module_addr[ARM7_STATIC] = head->s.sub_ram_address;
		module_size[ARM9_STATIC] = head->s.main_size;
		module_size[ARM7_STATIC] = head->s.sub_size;
		hash_addr[ARM9_STATIC] = &(head->s.main_static_digest[0]);
		hash_addr[ARM7_STATIC] = &(head->s.sub_static_digest[0]);
		module_num = 2;
		
		// NITROアプリの拡張では使わない領域
		if( head->s.platform_code != 0 )
		{
			module_addr[ARM9_LTD_STATIC] = head->s.main_ltd_ram_address;
			module_addr[ARM7_LTD_STATIC] = head->s.sub_ltd_ram_address;
			module_size[ARM9_LTD_STATIC] = head->s.main_ltd_size;
			module_size[ARM7_LTD_STATIC] = head->s.sub_ltd_size;
			hash_addr[ARM9_LTD_STATIC] = &(head->s.main_ltd_static_digest[0]);
			hash_addr[ARM7_LTD_STATIC] = &(head->s.sub_ltd_static_digest[0]);
			module_num = RELOCATE_INFO_NUM;
		}
		
		for( l=0; l<module_num ; l++ )
		{
			static const char *str[4]={"ARM9_STATIC","ARM7_STATIC","ARM9_LTD_STATIC","ARM7_LTD_STATIC"};
			prev = OS_GetTick();
			
#ifdef LOAD_APP_VIA_WRAM
			// とりあえず、カードアプリはここでハッシュ計算
			if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM )
#endif
			{
				// 一時的に格納位置をずらしている場合は、再配置情報からモジュール格納アドレスを取得
				if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL )
				{
					module_addr[l] = (u32 *)SYSMi_GetWork()->romRelocateInfo[l].src;
				}
				// ハッシュ計算
#ifdef LOAD_APP_VIA_WRAM
				if( l == 0)
#else
				if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM && l == 0)
#endif
				{
					// カードのARM9_STATICハッシュチェック
					// カード読み込み時、work2に暗号化オブジェクト部分のハッシュ計算済みのコンテキストが保存されるので
					// それを用いてARM9_STATIC残りの部分を計算
					SVCHMACSHA1Context ctx;
					u16 id;
	                
					SVC_HMACSHA1Init( &ctx, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
	                
					// ARM7とのhmac_sha1_contextの排他制御開始
	                id = (u16)OS_GetLockID();
					(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
	                
					SYSMi_GetWork2()->hmac_sha1_context.sha1_ctx.sha_block = ctx.sha1_ctx.sha_block;// この関数ポインタだけARM7とARM9で変えないとダメ
					ctx = SYSMi_GetWork2()->hmac_sha1_context;										// SYSMi_GetWork2は非キャッシュなのでスタック（DTCMまたはキャッシュ領域）へコピー

	                // ARM7とのhmac_sha1_contextの排他制御終了
					(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
					OS_ReleaseLockID( id );
	                
					SVC_HMACSHA1Update( &ctx,
										(const void*)((u32)module_addr[l] + ARM9_ENCRYPT_DEF_SIZE),
										(module_size[l] - ARM9_ENCRYPT_DEF_SIZE) );
					SVC_HMACSHA1GetHash( &ctx, calculated_hash );
				}else
				{
					SVC_CalcHMACSHA1( calculated_hash, (const void*)module_addr[l], module_size[l],
									 (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
				}
				// 比較
			    if(!SVC_CompareSHA1((const void *)hash_addr[l], (const void *)calculated_hash))
			    {
					OS_TPrintf("Authenticate failed: %s module hash check failed.\n", str[l]);
					if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
				}else
				{
					OS_TPrintf("Authenticate : %s module hash check succeed. %dms.\n", str[l], OS_TicksToMilliSeconds(OS_GetTick() - prev));
				}
			}
#ifdef LOAD_APP_VIA_WRAM
			else if(s_calc_hash)
			{
				// カード以外のアプリをロードする時に計算したハッシュを検証
			    if(!SVC_CompareSHA1((const void *)hash_addr[l], (const void *)&s_calc_hash[(l+1) * SVC_SHA1_DIGEST_SIZE]))
			    {
					OS_TPrintf("Authenticate failed: %s module hash check failed.\n", str[l]);
					if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
				}else
				{
					OS_TPrintf("Authenticate : %s module hash check succeed. %dms.\n", str[l], OS_TicksToMilliSeconds(OS_GetTick() - prev));
				}
			}else
			{
				OS_TPrintf("Authenticate failed: %s module hash check failed.\n", str[l]);
				if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
			}
#endif
		}
	}
	OS_TPrintf("Authenticate : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );

	// 製品or開発実機ではNANDアプリはNAND、カードアプリはカードからのみブート許可
	if ( ! SYSM_IsRunOnDebugger() )
	{
		if ( ( (pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND ||
				pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP)  && !(head->s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK) ) ||
			   (pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM    &&  (head->s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK) ) )
		{
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
	}

	return AUTH_RESULT_SUCCEEDED;
}

// NTR版特殊NANDアプリ（pictochat等）のヘッダ認証処理
static AuthResult SYSMi_AuthenticateNTRNandAppHeader( TitleProperty *pBootTitle)
{
	return SYSMi_AuthenticateTWLHeader( pBootTitle );
}

// NTR版ダウンロードアプリ（TMPアプリ）のヘッダ認証処理
static AuthResult SYSMi_AuthenticateNTRDownloadAppHeader( TitleProperty *pBootTitle)
{
#pragma unused(pBootTitle)
	ROM_Header *head;
	OSTick start;
	start = OS_GetTick();
	
	head = ( ROM_Header *)SYSM_CARD_ROM_HEADER_BUF;

	// 署名処理
    {
		u8 buf[0x80];
		SVCSignHeapContext con;
		u8 final_hash[SVC_SHA1_DIGEST_SIZE];
		u32 *module_addr[RELOCATE_INFO_NUM];
		u32 module_size[RELOCATE_INFO_NUM];
		u8 *hash_addr[RELOCATE_INFO_NUM];
		int module_num;
#ifndef LOAD_APP_VIA_WRAM
		u8 calculated_hash[SVC_SHA1_DIGEST_SIZE * 3 + sizeof(u32)];
		int l;
#endif

		// [TODO:]pBootTitle->titleIDと、それにこじつけたNTRヘッダのなんらかのデータとの一致確認をする。

		// NTRダウンロードアプリ署名のマジックコードチェック
		if( s_authcode.magic_code[0] != 'a' || s_authcode.magic_code[1] != 'c' ) {
			OS_TPrintf("Authenticate failed: Invalid AuthCode.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}

		// NTRダウンロードアプリ署名（DERフォーマット）の計算、ハッシュの取得。
	    MI_CpuClear8( buf, 0x80 );
	    SVC_InitSignHeap( &con, (void *)SIGN_HEAP_ADDR, SIGN_HEAP_SIZE );// ヒープの初期化
	    if( !SVC_DecryptSignDER( &con, buf, s_authcode.sign, nitro_dl_sign_key ))
	    {
			OS_TPrintf("Authenticate failed: Sign decryption failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
		
		// それぞれARM9,7のFLXについてハッシュを計算して、それら3つを並べたものに対してまたハッシュをとる
		module_addr[ARM9_STATIC] = head->s.main_ram_address;
		module_addr[ARM7_STATIC] = head->s.sub_ram_address;
		module_size[ARM9_STATIC] = head->s.main_size;
		module_size[ARM7_STATIC] = head->s.sub_size;
		hash_addr[ARM9_STATIC] = &(head->s.main_static_digest[0]);
		hash_addr[ARM7_STATIC] = &(head->s.sub_static_digest[0]);
		module_num = 2;

#ifdef LOAD_APP_VIA_WRAM
		if(s_calc_hash)
		{
			// シリアルナンバー付加
			*(u32 *)(&(s_calc_hash[SVC_SHA1_DIGEST_SIZE * 3])) = s_authcode.serial_number;
			// 最終ハッシュ計算
			SVC_CalcSHA1( final_hash, s_calc_hash, SVC_SHA1_DIGEST_SIZE * 3 + sizeof(u32));
		}
#else
		// ヘッダ
		SVC_CalcSHA1( calculated_hash, (const void*)head, NTR_ROM_HEADER_HASH_CALC_DATA_LEN );
		// モジュール
		for( l=0; l<module_num ; l++ )
		{
			// 一時的に格納位置をずらしている場合は、再配置情報からモジュール格納アドレスを取得
			if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL )
			{
				module_addr[l] = (u32 *)SYSMi_GetWork()->romRelocateInfo[l].src;
			}
			// ハッシュ計算
			{
				SVC_CalcSHA1( &calculated_hash[SVC_SHA1_DIGEST_SIZE * (l+1)], (const void*)module_addr[l], module_size[l]);
			}
		}
		// シリアルナンバー付加
		*(u32 *)(&(calculated_hash[SVC_SHA1_DIGEST_SIZE * 3])) = s_authcode.serial_number;
		// 最終ハッシュ計算
		SVC_CalcSHA1( final_hash, calculated_hash, SVC_SHA1_DIGEST_SIZE * 3 + sizeof(u32));
#endif
		
		// 計算した最終ハッシュと、署名から得たハッシュとを比較
	    if(!SVC_CompareSHA1((const void *)buf, (const void *)final_hash))
	    {
			OS_TPrintf("Authenticate failed: hash check failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf("Authenticate : hash check succeed.\n");
		}
	}
	OS_TPrintf("Authenticate : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
	
	return AUTH_RESULT_SUCCEEDED;
}

// ヘッダ認証
static AuthResult SYSMi_AuthenticateHeader( TitleProperty *pBootTitle)
{
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_CARD_ROM_HEADER_BUF;
	// [TODO:]認証結果はどこかワークに保存しておく
	// [TODO:]ヘッダに署名ビットがあるはずなので、それを確認して署名チェックを行う
	if( hs->platform_code & PLATFORM_CODE_FLAG_TWL )
	{
		// TWLアプリ
		// 認証処理
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate :TWL_NAND start.\n" );
				return SYSMi_AuthenticateTWLHeader( pBootTitle );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate :TWL_ROM start.\n" );
				return SYSMi_AuthenticateTWLHeader( pBootTitle );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate :TWL_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					return AUTH_RESULT_AUTHENTICATE_FAILED;
				}
				return SYSMi_AuthenticateTWLHeader( pBootTitle );
			default:
				return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
	}
	else
	{
		if( hs->platform_code & PLATFORM_CODE_FLAG_NOT_NTR )
		{
			// TWLでもNTRでもない不正なアプリ
			OS_TPrintf( "Authenticate :NOT NTR NOT TWL.\n" );
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
		// NTRアプリ
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate :NTR_NAND start.\n" );
				return SYSMi_AuthenticateNTRNandAppHeader( pBootTitle );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate :NTR_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					return AUTH_RESULT_AUTHENTICATE_FAILED;
				}
				return SYSMi_AuthenticateNTRDownloadAppHeader( pBootTitle );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate :NTR_ROM start.\n" );
				// NTRカードは今のところ認証予定無し
				return AUTH_RESULT_SUCCEEDED;
			default:
				return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
	}
}

// 認証処理のスレッド
static void SYSMi_AuthenticateTitleThreadFunc( TitleProperty *pBootTitle )
{
	// ロード中
	if( !SYSM_IsLoadTitleFinished() ) {
		s_authResult = AUTH_RESULT_PROCESSING;
		return;
	}
	// ロード成功？
	if( SYSMi_GetWork()->flags.common.isLoadSucceeded == FALSE )
	{
		s_authResult = AUTH_RESULT_TITLE_LOAD_FAILED;
		return;
	}
	// パラメータチェック
	if( !SYSMi_CheckTitlePointer( pBootTitle ) ) {
		s_authResult = AUTH_RESULT_TITLE_POINTER_ERROR;
		return;
	}
#if 0
	// エントリアドレスの正当性をチェック
	if( !SYSMi_CheckEntryAddress() ) {
		s_authResult = AUTH_RESULT_ENTRY_ADDRESS_ERROR;
		return;
	}
#endif
	
	// BOOTTYPE_MEMORYでNTRモードのFSありでブートすると、旧NitroSDKでビルドされたアプリの場合、
	// ROMアーカイブにカードが割り当てられて、FSで関係ないカードにアクセスにいってしまうので、それを防止する。
	if( ( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_MEMORY ) &&
		( ( (( ROM_Header_Short *)SYSM_CARD_ROM_HEADER_BUF)->platform_code ) == 0 ) &&
		( ( (( ROM_Header_Short *)SYSM_CARD_ROM_HEADER_BUF)->fat_size ) > 0 )
		) {
		s_authResult = AUTH_RESULT_TITLE_BOOTTYPE_ERROR;
		return;
	}
	
	// ※ROMヘッダ認証
	s_authResult = SYSMi_AuthenticateHeader( pBootTitle );
}


// ロード済みの指定タイトルを別スレッドで検証開始する
void SYSM_StartAuthenticateTitle( TitleProperty *pBootTitle )
{
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	s_authResult = AUTH_RESULT_PROCESSING;
	OS_InitThread();
	OS_CreateThread( &s_auth_thread, (void (*)(void *))SYSMi_AuthenticateTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
	OS_WakeupThreadDirect( &s_auth_thread );
}

// 検証済み？
BOOL SYSM_IsAuthenticateTitleFinished( void )
{
	if(s_authResult == AUTH_RESULT_SUCCEEDED)
	{
		return TRUE;
	}
	return OS_IsThreadTerminated( &s_auth_thread );
}

// ロード済みの指定タイトルの認証とブートを行う
AuthResult SYSM_TryToBootTitle( TitleProperty *pBootTitle )
{
	if(s_authResult != AUTH_RESULT_SUCCEEDED)
	{
		return s_authResult;
	}
	
	if(s_calc_hash)
	{
		// ハッシュ値保存領域解放
		SYSM_Free( s_calc_hash );
		s_calc_hash = NULL;
	}
	
	// TWL設定データにブートするタイトルのTitleIDとplatformCodeを保存。
	{
		u8 *pBuffer = SYSM_Alloc( LCFG_WRITE_TEMP );
		if( pBuffer != NULL ) {
			LCFG_TSD_SetLastTimeBootSoftTitleID ( pBootTitle->titleID );
			LCFG_TSD_SetLastTimeBootSoftPlatform( (u8)SYSM_GetCardRomHeader()->platform_code );
			(void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
			SYSM_Free( pBuffer );
		}
	}
	
	// マウント情報の登録
	SYSMi_GetWork2()->bootTitleProperty = *pBootTitle;
	SYSMi_SetBootSRLPathToWork2( pBootTitle );
	
	// HW_WM_BOOT_BUFへのブート情報セット
	( (OSBootInfo *)OS_GetBootInfo() )->boot_type = s_launcherToOSBootType[ pBootTitle->flags.bootType ];
	
	// タイトルIDリストの作成
	SYSMi_makeTitleIdList();
	SYSM_FreeNandTitleList();
	
	BOOT_Ready();	// never return.
	
	return AUTH_RESULT_SUCCEEDED;
}

// タイトルIDリストの作成
static void SYSMi_makeTitleIdList( void )
{
	// [TODO:]現在ブート不可タイトルについても入れるようにしているが
	// これで良いのか？
	OSTitleIDList *list = ( OSTitleIDList * )HW_OS_TITLE_ID_LIST;
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_CARD_ROM_HEADER_BUF;
	int l;
	u8 count = 0;
	
	if( s_pTitleIDList == NULL )
	{
		OS_TPrintf("SYSMi_makeTitleIdList failed: SYSM_InitNandTitleList() is not called.\n");
		return;
	}
	
	// とりあえずゼロクリア
	MI_CpuClear8( (void *)HW_OS_TITLE_ID_LIST, HW_OS_TITLE_ID_LIST_SIZE );

	// これから起動するアプリがTWLアプリでない
	if( !hs->platform_code )
	{
		return;
	}

	for(l=-1;l<s_listLength;l++) // -1はカードアプリの特別処理用
	{
		ROM_Header_Short e_hs;
		ROM_Header_Short *pe_hs;
		int m;
		BOOL same_maker_code = TRUE;
		char path[256];
		FSFile file[1];
		BOOL bSuccess;
		s32 readLen;
		char *gamecode;
		if(l==-1)
		{
			// カードアプリ
			if(SYSM_IsExistCard())
			{
				pe_hs = (ROM_Header_Short *)SYSM_CARD_ROM_HEADER_BAK;// BAKの値を使う
			}
		}else
		{
			if(s_pTitleIDList[l] == NULL)
			{
				continue;
			}
			// romヘッダ読み込み
			NAM_GetTitleBootContentPathFast(path, s_pTitleIDList[l]);
			FS_InitFile( file );
		    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);
			if( ! bSuccess )
			{
				OS_TPrintf("SYSMi_makeTitleIdList failed: cant open file(%s)\n",path);
			    FS_CloseFile(file);
			    continue;
			}
			bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);
			if( ! bSuccess )
			{
				OS_TPrintf("SYSMi_makeTitleIdList failed: cant seek file(0)\n");
			    FS_CloseFile(file);
			    continue;
			}
			readLen = FS_ReadFile(file, &e_hs, (s32)sizeof(e_hs));
			if( readLen != (s32)sizeof(e_hs) )
			{
			OS_TPrintf("SYSMi_makeTitleIdList failed: cant read file(%p, %d, %d, %d)\n", e_hs, 0, sizeof(e_hs), readLen);
			    FS_CloseFile(file);
			    continue;
			}
			FS_CloseFile(file);
			pe_hs = (ROM_Header_Short *)&e_hs;
		}
		
		for(m=0;m<MAKER_CODE_MAX;m++)
		{
			if(hs->maker_code[m] != pe_hs->maker_code[m])
			{
				same_maker_code = FALSE;
			}
		}
		
		// ランチャーはリストに入れない
		gamecode = (char *)&(pe_hs->titleID);
		if(  0 == STD_CompareNString( &gamecode[1], "ANH", 3 ) )
		{
			continue;
		}
		
		// セキュアアプリの場合か、メーカーコードが同じ場合は
		if( (hs->titleID & TITLE_ID_SECURE_FLAG_MASK) ||
		    ( same_maker_code ) )
		{
			// セキュアアプリのデータはマウントさせない
			if( !(pe_hs->titleID & TITLE_ID_SECURE_FLAG_MASK) )
			{
				// リストに追加
				list->TitleID[count] = pe_hs->titleID;
				// sameMakerFlagをON
				list->sameMakerFlag[count/8] |= (u8)(0x1 << (count%8));
				// Prv,Pubそれぞれセーブデータがあるか見て、存在すればフラグON
				if(pe_hs->public_save_data_size != 0)
				{
					list->publicFlag[count/8] |= (u8)(0x1 << (count%8));
				}
				if(pe_hs->private_save_data_size != 0)
				{
					list->privateFlag[count/8] |= (u8)(0x1 << (count%8));
				}
			}
		}
		
		// ジャンプ可能ならば(一応Data Onlyフラグも見ておくが、ジャンプAPIでも見る事)
		if( pe_hs->permit_landing_normal_jump && !( hs->titleID & TITLE_ID_DATA_ONLY_FLAG_MASK ) )
		{
			// リストに追加してジャンプ可能フラグON
			list->TitleID[count] = pe_hs->titleID;
			list->appJumpFlag[count/8] |= (u8)(0x1 << (count%8));
		}
		
		// ここまでのうちに、list->TitleID[count]が編集されていたらcountインクリメント
		if( list->TitleID[count] != NULL )
		{
			count++;
		}
	}
	list->num = count;
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
	if( SYSMi_GetWork()->flags.hotsw.isEnableHotSW == enable ) {
		return;
	}
	
	{
		HotSwPxiMessage msg;

        msg.msg.value = enable;
        msg.msg.ctrl  = TRUE;

	    while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
    	{
        	// do nothing
    	}
        
	}
	
	// 値が変化するまでスリープして待つ。
	while( SYSMi_GetWork()->flags.hotsw.isEnableHotSW != enable ) {
		OS_Sleep( 2 );
	}
}