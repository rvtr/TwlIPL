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
#include <sysmenu/util_menuAppManager.h>
#include <sysmenu/util_recoveryFile.h>
#include "internal_api.h"
#include "fs_wram.h"

// define data-----------------------------------------------------------------

#define MEASURE_MAKELIST_TIME     0

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

#define THREAD_PRIO_FS_WRAM	3
#define WRAM_SIZE_FOR_FS	MI_WRAM_SIZE_96KB

#ifdef USE_HYENA_COMPONENT
#define WRAM_SLOT_FOR_FS	5
#else
#define WRAM_SLOT_FOR_FS	0
#endif

#include <sysmenu/dht/dht.h>
#define DS_HASH_TABLE_SIZE  (256*1024)

#define SYSM_TITLE_MESSAGE_ARRAY_MAX		1

#define SIZE_16KB	( 16 * 1024 )

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

static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );
static void SYSMi_makeTitleIdList( void );
static BOOL SYSMi_AuthenticateHeader( TitleProperty *pBootTitle, ROM_Header *head );
static void SYSMi_applyPatchToBandBrothers( void );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static OSThread			s_thread;
static OSThread			s_auth_thread;
static TWLBannerFile	s_card_bannerBuf;

static MbAuthCode s_authcode;

static BOOL				s_loadstart = FALSE;

static BOOL				s_loadPaused = FALSE;

static OSMessageQueue	s_msgQ;
static OSMessage		s_msgArray[SYSM_TITLE_MESSAGE_ARRAY_MAX];

static int s_listLength = 0;

static u8 *s_calc_hash = NULL;
static BOOL s_b_dev = FALSE;
static BOOL s_result_phase1 = FALSE;

static u8 dht_buffer[DS_HASH_TABLE_SIZE] ATTRIBUTE_ALIGN(256);
static DHTFile *const dht = (DHTFile*)dht_buffer;
static const u8* hash0 = NULL;
static const u8* hash1 = NULL;

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

// ============================================================================
// DHT
// ============================================================================

static BOOL GetDatabaseFilepath(char *path)
{
    u8 title[4] = { 'H','N','H','A' };

#if( USE_LCFG_STRING == 0 )
    char *title0 = "HNGA";
#endif
    u32 titleID_hi;
    u32 titleID_lo;
    u64 titleID = 0;


#if( USE_LCFG_STRING == 0 )
    {
        int i;
        if( title[0] == 0 ) {
            for( i = 0 ; i < 4 ; i++ ) {
                title[i] = (u8)*title0++;
            }
        }
    }
#endif

    titleID_hi = (( 3 /* Nintendo */ << 16) | 8 /* CHANNEL_DATA_ONLY */ | 4 /* CHANNEL_CARD */ | 2 /* isLaunch */ | 1 /* isSystem */);

    titleID_lo =  ((u32)( title[0] ) & 0xff) << 24;
    titleID_lo |= ((u32)( title[1] )& 0xff) << 16;
    titleID_lo |= ((u32)( title[2] )& 0xff) << 8;
    titleID_lo |= (u32)( title[3] ) & 0xff;

    titleID = ((u64)(titleID_hi) << 32)  | (u64)titleID_lo;

    // OS_TPrintf( "[DHT]  titleID = 0x%08x%08x\n", titleID_hi, titleID_lo);

    if( NAM_OK == NAM_GetTitleBootContentPathFast(path, titleID) ) {
        OS_TPrintf( "[DHT]  File = %s\n", path);
    }
    else {
        OS_TPrintf( "[DHT]  Error: NAM_GetTitleBootContentPathFast titleID = 0x%08x0x%08x\n",titleID_hi, titleID_lo);
        return FALSE;
    }

    return TRUE;
}

static void PrepareDHTDatabase(void)
{
    char path[256];
    if ( GetDatabaseFilepath( path ) )
    {
        FSFile file;
        if ( FS_OpenFileEx(&file, path, FS_FILEMODE_R) )
        {
#if 0   // 1 if using attach_dummyromheader
            if ( FS_SeekFile(&file, sizeof(ROM_Header), FS_SEEK_SET) )
#endif
            {
                DHT_PrepareDatabase(dht, &file);
                DC_FlushRange(dht, DHT_GetDatabaseLength(dht));
            }
            FS_CloseFile(&file);
        }
    }
    else
    {
        MI_CpuClear8(dht, sizeof(DHTHeader));
    }
}

static BOOL WrapperFunc_ReadCardData(void* dest, s32 offset, s32 length, void* arg)
{
#pragma unused(arg)
	HOTSW_ReadCardData( (void *)offset, dest, (u32)length);
	return TRUE;
}


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
//
// 情報取得
//
//
// ============================================================================

// カードタイトルの取得
TitleProperty *SYSM_GetCardTitleList( BOOL *changed )
{
	TitleProperty *pTitleList_Card = AMN_getTitlePropertyList();
	if(changed) *changed = FALSE;
	
	if(s_loadstart)
	{
		// ロード開始していたら、もうヘッダやタイトル情報は変更しない
		return pTitleList_Card;
	}
	
	if( SYSMi_GetWork()->flags.hotsw.isCardStateChanged ) {
		u16 id = (u16)OS_GetLockID();
		
		MI_CpuClear32( pTitleList_Card, sizeof(TitleProperty) );
		
		(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );						// ARM7と排他制御する
		
		// ROMヘッダバッファのコピー
		if( SYSM_IsExistCard() ) {
			
			// ROMヘッダのリード
			(void)SYSMi_CopyCardRomHeader();
			// バナーデータのリード
			(void)SYSMi_CopyCardBanner();
			
			pTitleList_Card->pBanner = &s_card_bannerBuf;
			AMN_stepBannerAnime(0, TRUE); // バナーカウンタセットしなおし
			pTitleList_Card->flags.isValid = TRUE;
			pTitleList_Card->flags.isAppLoadCompleted = FALSE;
			pTitleList_Card->flags.isAppRelocate = TRUE;
			MI_CpuCopy8( SYSM_GetCardRomHeader(), AMN_getRomHeaderList(), sizeof(ROM_Header_Short) );
		}else {
			// ROMヘッダのクリア
			MI_CpuClearFast( (void *)SYSM_APP_ROM_HEADER_BUF, SYSM_APP_ROM_HEADER_SIZE );
			// バナーデータのクリア
			MI_CpuClearFast( &s_card_bannerBuf, sizeof(TWLBannerFile) );
		}
		
		SYSMi_GetWork()->flags.hotsw.isCardStateChanged = FALSE;							// カード情報更新フラグを落とす
		(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );					// ARM7と排他制御する
		OS_ReleaseLockID( id );
		
		// タイトル情報フラグのセット
		pTitleList_Card->flags.bootType = LAUNCHER_BOOTTYPE_ROM;
		pTitleList_Card->titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
		if(changed) *changed = TRUE;
	}
	
	return pTitleList_Card;
}

// カードROMヘッダのARM7バッファからARM9バッファへのコピー
BOOL SYSMi_CopyCardRomHeader( void )
{
	BOOL retval = FALSE;

	if( SYSM_IsExistCard() ) {
		// ROMヘッダのリード
		DC_InvalidateRange( (void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_APP_ROM_HEADER_SIZE );	// キャッシュケア
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BAK, (void *)SYSM_APP_ROM_HEADER_BUF, SYSM_APP_ROM_HEADER_SIZE );	// ROMヘッダコピー

		retval = TRUE;
	}

	return retval;
}


// カードバナーのARM7バッファからARM9バッファへのコピー
BOOL SYSMi_CopyCardBanner( void )
{
	BOOL retval = FALSE;

	if( SYSM_IsExistCard() ) {
		// バナーデータのコピー
		TWLBannerFile *pBanner = &s_card_bannerBuf;
		if( SYSMi_GetWork()->flags.hotsw.isValidCardBanner ) {
			DC_InvalidateRange( (void *)SYSM_CARD_BANNER_BUF, 0x3000 );
			MI_CpuCopyFast( (void *)SYSM_CARD_BANNER_BUF, pBanner, sizeof(TWLBannerFile) );
		}
		retval = AMN_checkBannerFile( pBanner );
		
		if( !retval ) {
			MI_CpuClearFast( pBanner, sizeof(TWLBannerFile) );
		}
	}

	return retval;
}

void SYSM_InitTitleList( void )
{
	AMN_init( SYSM_Alloc, SYSM_Free );
}

// SYSM_InitTitleListを事前に呼ぶ必要あり
void SYSM_MakeNandTitleListMakerInfo( void )
{
	AMN_restartWithReadNandTitleHeaderShort();
	while (!AMN_isNandTitleListReady()) {
	    OS_Sleep(1);
	}
}

// ローンチ対象となるNANDタイトルリストの取得
// SYSM_InitTitleListを事前に呼ぶ必要あり
// return:Titleリストのポインタ
TitleProperty *SYSM_GetNandTitleList( void )
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, 等の条件を指定してタイトルリストを取得する。
	AMN_restartWithReadNandTitle();
	while (!AMN_isNandTitleListReady()) {
	    OS_Sleep(1);
	}
	return AMN_getTitlePropertyList();
}

// SYSM_InitTitleListを事前に呼ぶ必要あり
void SYSM_MakeNandTitleListMakerInfoAsync( void )
{
	AMN_restartWithReadNandTitleHeaderShort();
}

void SYSM_MakeNandTitleListAsync( void )
{
	AMN_restartWithReadNandTitle();
}

BOOL SYSM_isNandTitleListReady( void )
{
	return AMN_isNandTitleListReady();
}

TitleProperty *SYSM_GetTitlePropertyList( void )
{
	return AMN_getTitlePropertyList();
}

// ============================================================================
//
//
// アプリロード
//
//
// ============================================================================

static void CallbackSub_DecryptAES(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot)
{
	OSIntrMode enabled = OS_DisableInterrupts();// WRAM切り替え途中で割り込み発生→別スレッドでWRAM切り替え→死亡の可能性があるので暫定対応
	MI_SwitchWramSlot( wram, slot, MI_WRAM_SIZE_32KB, MI_WRAM_ARM9, MI_WRAM_ARM7 );// Wramを7にスイッチ
	SYSM_StartDecryptAESRegion_W( addr, orig_addr, len ); // AES領域デクリプト
	MI_SwitchWramSlot( wram, slot, MI_WRAM_SIZE_32KB, MI_WRAM_ARM7, MI_WRAM_ARM9 );// Wramが7にスイッチしてしまっているので戻す
	OS_RestoreInterrupts(enabled);// 割り込み許可
}

static void SYSMi_CalcHMACSHA1Callback(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg)
{
	CalcHMACSHA1CallbackArg *cba = (CalcHMACSHA1CallbackArg *)arg;
	u32 calc_len = ( cba->hash_length < len ? cba->hash_length : len );
	CallbackSub_DecryptAES( addr, orig_addr, len, wram, slot );
	if( calc_len == 0 ) return;
	cba->hash_length -= calc_len;
	SVC_HMACSHA1Update( &cba->ctx, addr, calc_len );
}

static void SYSMi_CalcSHA1Callback(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg)
{
	CalcSHA1CallbackArg *cba = (CalcSHA1CallbackArg *)arg;
	u32 calc_len = ( cba->hash_length < len ? cba->hash_length : len );
	CallbackSub_DecryptAES( addr, orig_addr, len, wram, slot );
	if( calc_len == 0 ) return;
	cba->hash_length -= calc_len;
	SVC_SHA1Update( &cba->ctx, addr, calc_len );
}

static void SYSMi_DHTPhase1Callback(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg)
{
	CalcHMACSHA1CallbackArg *cba = (CalcHMACSHA1CallbackArg *)arg;
	u32 calc_len = ( cba->hash_length < len ? cba->hash_length : len );
	CallbackSub_DecryptAES( addr, orig_addr, len, wram, slot );
	if( calc_len == 0 ) return;
	cba->hash_length -= calc_len;
	DHT_CheckHashPhase1Update( &cba->ctx, addr, (s32)calc_len );
}

static void SYSMi_FinalizeHotSWAsync( TitleProperty *pBootTitle, ROM_Header *head )
{
	HotSwCardState card_state;
    ROM_Header* rh = (void*)SYSM_APP_ROM_HEADER_BUF;

	DC_StoreRange( head, sizeof(ROM_Header) );

	switch( pBootTitle->flags.bootType )
	{
		case LAUNCHER_BOOTTYPE_NAND:
		case LAUNCHER_BOOTTYPE_TEMP:
			// ROMヘッダのアクセスコントロール情報をもとにカード状態を制御するのは "NAND/TMPブートのTWL-LTD" アプリのみ
			if ( ( head->s.platform_code & PLATFORM_CODE_TWL_LIMITED ) == PLATFORM_CODE_TWL_LIMITED )
			{
				if(rh->s.access_control.game_card_nitro_mode){
                	card_state = HOTSW_CARD_STATE_GAME_MODE;
        		}
                else if(rh->s.access_control.game_card_on){
                	card_state = HOTSW_CARD_STATE_NORMAL_MODE;
				}else
				{
					card_state = HOTSW_CARD_STATE_POWER_OFF;
				}
			}
			// それ以外は強制的にNTR互換Game領域にアクセス可能な状態となる（DSダウンロードプレイも含む）
			else
			{
				card_state = HOTSW_CARD_STATE_GAME_MODE;
			}
			break;
		case LAUNCHER_BOOTTYPE_ROM:
		default:
			card_state = HOTSW_CARD_STATE_KEEP;
			break;
	}

	HOTSW_FinalizeHotSWAsync( card_state );
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
	// DSダウンロードプレイおよびpictochat等のNTR拡張NANDアプリの時は、ROMヘッダを退避する
	// が、NTR-ROMヘッダは旧無線パッチとデバッガパッチを当てる必要があるため、再配置はrebootライブラリで行う。
	
	// ロード
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    BOOL isTwlApp = TRUE;
    BOOL isCardApp = FALSE;
	
	switch( pBootTitle->flags.bootType )
	{
	case LAUNCHER_BOOTTYPE_NAND:
		// NAND
    	NAM_GetTitleBootContentPathFast(path, pBootTitle->titleID);
		break;
	case LAUNCHER_BOOTTYPE_ROM:
		// CARD
		isCardApp = TRUE;
		break;
	case LAUNCHER_BOOTTYPE_TEMP:
		// tmpフォルダ
		STD_TSNPrintf( path, 256, OS_TMP_APP_PATH, pBootTitle->titleID );
		break;
	default:
		// unknown
		return;
	}

	if(!isCardApp)
	{
		FS_InitFile( file );
	    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);
    }else
    {
		bSuccess = TRUE;
	}

    if( ! bSuccess )
    {
OS_TPrintf("RebootSystem failed: cant open file\n");
		UTL_SetFatalError(FATAL_ERROR_LOAD_OPENFILE_FAILED);
		goto ERROR;
    }

    {
        int     i;
        u32     source[region_max];
        u32     length[region_max];
        u32     destaddr[region_max];
        static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
        s32 readLen;
        ROM_Header *head = (ROM_Header *)header;
        CalcHMACSHA1CallbackArg dht_arg;

		// WRAM利用Read関数の準備、
		// 使用するコンポーネントに応じて、WRAMのスロットを解放しておく
		//		hyena  : WRAM_C		slot 5-7
		//		jackal : WRAM_C		slot 0-2
		FS_InitWramTransfer( THREAD_PRIO_FS_WRAM );
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
		
		// ハッシュ格納用バッファ（ヒープから取っているけど変更するかも）
		s_calc_hash = SYSM_Alloc( region_max * SVC_SHA1_DIGEST_SIZE );
		if(!s_calc_hash)
		{
OS_TPrintf("RebootSystem failed: Alloc Failed.\n");
			UTL_SetFatalError(FATAL_ERROR_LOAD_MEMALLOC_FAILED);
			goto ERROR;
		}

        // まずROMヘッダを読み込む
        if(!isCardApp)
        {
        	bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);
		}else
		{
			bSuccess = TRUE;
		}

        if( ! bSuccess )
        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
			UTL_SetFatalError(FATAL_ERROR_LOAD_SEEKFILE_FAILED);
			goto ERROR;
        }

		//ヘッダ読み込みと同時に各種ハッシュ計算……できない（NTRかTWLか判別できないため）ので読み込みのみ
		{
            BOOL result;
            u32 len = MATH_ROUNDUP( (s32)sizeof(header), SYSM_ALIGNMENT_LOAD_MODULE );
            if(!isCardApp)
	        {
	            result = FS_ReadFileViaWram(file, (void *)header, (s32)len, MI_WRAM_C,
	            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, NULL, NULL );
	        }else
	        {
				result = HOTSW_ReadCardViaWram((void*) 0, (void*)header, (s32)len, MI_WRAM_C,
	            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, NULL, NULL );
			}
			if ( !result )
			{
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", 0, len);
				UTL_SetFatalError(FATAL_ERROR_LOAD_READHEADER_FAILED);
				goto ERROR;
			}
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
			UTL_SetFatalError(FATAL_ERROR_LOAD_LOGOCRC_ERROR);
			goto ERROR;
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
					UTL_SetFatalError(FATAL_ERROR_LOAD_SEEKFILE_FAILED);
					goto ERROR;
		        }
		        readLen = FS_ReadFile(file, &s_authcode, (s32)sizeof(s_authcode));
		        if( readLen != (s32)sizeof(s_authcode) )
		        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", &s_authcode, 0, sizeof(s_authcode), readLen);
					UTL_SetFatalError(FATAL_ERROR_LOAD_READDLSIGN_FAILED);
					goto ERROR;
		        }
			}
		}
		
		// ヘッダのハッシュ計算
		SVC_CalcSHA1( s_calc_hash, header, (u32)( ( isTwlApp || ( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND ) || head->s.exFlags.enable_nitro_whitelist_signature ) ?
												TWL_ROM_HEADER_HASH_CALC_DATA_LEN : NTR_ROM_HEADER_HASH_CALC_DATA_LEN ));
		
		//この時点でヘッダの正当性検証
		// ※ROMヘッダ認証
		if( !SYSMi_AuthenticateHeader( pBootTitle, head ) )
		{
			goto ERROR;
		}
		
		// 正当性の検証されたヘッダを、本来のヘッダバッファへコピー
		MI_CpuCopy8( head, (void*)SYSM_APP_ROM_HEADER_BUF, HW_TWL_ROM_HEADER_BUF_SIZE );
		
		// NTRカードアプリはDHTのPhase1のための計算が必要
		if( !isTwlApp && pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM )
		{
			DHT_CheckHashPhase1Init(&dht_arg.ctx, &head->s);
		}
		
		// ヘッダ読み込み完了直後の処理
		// ヘッダ読み込み完了フラグを立てる
		SYSMi_GetWork()->flags.common.isHeaderLoadCompleted = TRUE;
		// HOTSW終了処理有効化
		SYSMi_FinalizeHotSWAsync( pBootTitle, (void*)SYSM_APP_ROM_HEADER_BUF );
		
        // 各領域を読み込む
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
				UTL_SetFatalError(FATAL_ERROR_LOAD_RELOCATEINFO_FAILED);
				goto ERROR;
			}
		}
		
		// AES初期化（ヘッダと再配置情報がそろってから）
		(void)SYSM_InitDecryptAESRegion_W( (ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF );

        for (i = region_arm9_ntr; i < region_max; ++i)
        {
			BOOL result;
			
            u32 len = MATH_ROUNDUP( length[i], SYSM_ALIGNMENT_LOAD_MODULE );// AESおよびDMA転送サイズの仕様で、ロードサイズは32バイトアライメントに補正
            
            if ( !isTwlApp && i >= region_arm9_twl ) continue;// nitroでは読み込まない領域
	        if(!isCardApp)
	        {
	            bSuccess = FS_SeekFile(file, (s32)source[i], FS_SEEK_SET);
            }else
            {
				bSuccess = TRUE;
			}

            if( ! bSuccess )
            {
OS_TPrintf("RebootSystem failed: cant seek file(%d)\n", source[i]);
				UTL_SetFatalError(FATAL_ERROR_LOAD_SEEKFILE_FAILED);
				goto ERROR;
            }

OS_TPrintf("RebootSystem : Load VIA WRAM %d.\n", i);
            // ここでロード処理と同時にハッシュ計算とAES処理もやってしまう
            // 別スレッドで同じWRAM使おうとすると多分コケるので注意
            
            // コールバック関数に与える引数を初期化してRead
            if( !isTwlApp && pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP )
            {
				// NTRダウンロードアプリのモジュール
				CalcSHA1CallbackArg arg;
	            SVC_SHA1Init( &arg.ctx );
	            arg.hash_length = (u32)length[i];
	            if(!isCardApp)
		        {
		            result = FS_ReadFileViaWram(file, (void *)destaddr[i], (s32)len, MI_WRAM_C,
		            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcSHA1Callback, &arg );
		        }else
		        {
					result = HOTSW_ReadCardViaWram((void *)source[i], (void *)destaddr[i], (s32)len, MI_WRAM_C,
		            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcSHA1Callback, &arg );
				}
	            SVC_SHA1GetHash( &arg.ctx, &s_calc_hash[i * SVC_SHA1_DIGEST_SIZE] );
	        }else if( !isTwlApp && pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM )
	        {
				// NTRカードアプリはDHTのPhase1のための計算が必要
				// DHTチェックphase1用のハッシュを計算（DHT_CheckHashPhase1Update 関数）し、結果まで出しておく
	            dht_arg.hash_length = (u32)length[i];
				result = HOTSW_ReadCardViaWram((void *)source[i], (void *)destaddr[i], (s32)len, MI_WRAM_C,
	            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_DHTPhase1Callback, &dht_arg );
			}else
			{
				// それ以外
				CalcHMACSHA1CallbackArg arg;
	            SVC_HMACSHA1Init( &arg.ctx, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
	            arg.hash_length = length[i];
	            if(!isCardApp)
		        {
		            result = FS_ReadFileViaWram(file, (void *)destaddr[i], (s32)len, MI_WRAM_C,
		            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcHMACSHA1Callback, &arg );
		        }else
		        {
					result = HOTSW_ReadCardViaWram((void *)source[i], (void *)destaddr[i], (s32)len, MI_WRAM_C,
		            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcHMACSHA1Callback, &arg );
		            //speed test code
		            /*
		            HOTSW_ReadCardData((void *)source[i], (void *)destaddr[i], (u32)len);
		            SVC_HMACSHA1Update( &arg.ctx, (void *)destaddr[i], length[i] );
		            */
				}
	            SVC_HMACSHA1GetHash( &arg.ctx, &s_calc_hash[i * SVC_SHA1_DIGEST_SIZE] );
			}
			if ( !result )
			{
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
				UTL_SetFatalError(FATAL_ERROR_LOAD_READMODULE_FAILED);
				goto ERROR;
			}
        }
        
        // NTRカードアプリはDHTのPhase1最終計算を行う
        if( !isTwlApp && pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM )
        {
			SVC_HMACSHA1GetHash(&dht_arg.ctx, &s_calc_hash[1 * SVC_SHA1_DIGEST_SIZE]);
		}

		if(!isCardApp)
		{
	        (void)FS_CloseFile(file);
        }

    }
	SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
	return;
	
ERROR:
	if(!isCardApp)
	{
        (void)FS_CloseFile(file);
    }
}


// 指定タイトルを別スレッドでロード開始する
void SYSM_StartLoadTitle( TitleProperty *pBootTitle )
{
#define THREAD_PRIO 17
#define STACK_SIZE 0xc00
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	
	HOTSW_InvalidHotSWAsync();
	// 値が変化するまでスリープして待つ。
	while( HOTSW_isEnableHotSW() != FALSE ) {
		OS_Sleep( 2 );
	}
	
	// DataOnlyなアプリはロードも起動もしない
	if( pBootTitle->titleID & TITLE_ID_DATA_ONLY_FLAG_MASK )
	{
		OS_TPrintf("SYSM_StartLoadTitle failed: This App has Data_Only flag.\n");
		return;
	}
    
	s_loadstart = TRUE;
	// このあとCardRomヘッダバッファにROMヘッダを上書きで読み込むので
	// この時点でHotSWが止まっていないと、さらにカードのROMヘッダ
	// を上書きしてしまう可能性がある

	// アプリ未ロード状態なら、ロード開始
	if( !pBootTitle->flags.isAppLoadCompleted ) {
		SYSMi_GetWork()->flags.common.isLoadFinished  = FALSE;
		
		SYSMi_GetWork()->flags.common.isLoadSucceeded = FALSE;
		OS_InitThread();
		OS_CreateThread( &s_thread, (void (*)(void *))SYSMi_LoadTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
		OS_WakeupThreadDirect( &s_thread );

	}else {
		// アプリロード済み
		SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
		SYSMi_GetWork()->flags.common.isLoadFinished  = TRUE;
	}
	
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM ) {
		SYSMi_GetWork()->flags.common.isCardBoot = TRUE;
	}else if(pBootTitle->flags.isAppLoadCompleted)
	{
		// カードブートでなく、ロード済みの場合は今のところ何もしない
	}
}

// アプリロード済みかどうかをチェック
BOOL SYSM_IsLoadTitleFinished( void )
{
	// ロード済みの時は、常にTRUE
	if( !SYSMi_GetWork()->flags.common.isLoadFinished ) {
		/*
		if( SYSMi_GetWork()->flags.common.isCardBoot ) {
			// カードブートの時は、HOTSWライブラリのロード完了をチェック。
			SYSMi_GetWork()->flags.common.isLoadFinished  = SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted;
			SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
		}else {
		*/
		// NANDブートの時は、ロードスレッドの完了をチェック。
		SYSMi_GetWork()->flags.common.isLoadFinished = OS_IsThreadTerminated( &s_thread );
	}
	return SYSMi_GetWork()->flags.common.isLoadFinished ? TRUE : FALSE;
}


// ============================================================================
//
//
// アプリ認証
//
//
// ============================================================================

// 署名つきアプリ（≠DSダウンロードアプリ署名）共通のヘッダ認証処理
static BOOL SYSMi_AuthenticateHeaderWithSign( TitleProperty *pBootTitle, ROM_Header *head )
{
	// 署名処理
	const u8 *key;
	u32 hi;
	u8 keynum;
	SignatureData sigbuf;
	SVCSignHeapContext con;
	char *gamecode = (char *)&(pBootTitle->titleID);
	OSTick start,prev;
	start = OS_GetTick();
	
	prev = OS_GetTick();
	hi = head->s.titleID_Hi;
	// Launcherは専用の鍵を使う
	if( ( 0 == STD_CompareNString( &gamecode[1], "ANH", 3 ) )
#ifdef DEV_UIG_LAUNCHER
	 || ( ( 0 == STD_CompareNString( &gamecode[1], "AN4", 3 ) ) && ( SCFG_GetBondingOption() != 0 ) )
#endif
	)
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
//#define LNC_PDTKEY_DBG
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
		if( SYSM_IsRunOnDebugger() && SYSMi_GetWork()->romEmuInfo.isTlfRom )
		{
			s_b_dev = TRUE;
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
		if( SYSM_IsRunOnDebugger() && SYSMi_GetWork()->romEmuInfo.isTlfRom )
		{
			s_b_dev = TRUE;
		}
    }
#endif
    // 署名を鍵で復号
    MI_CpuClear8( &sigbuf, sizeof(sigbuf) );
    SVC_InitSignHeap( &con, (void *)SIGN_HEAP_ADDR, SIGN_HEAP_SIZE );// ヒープの初期化
    if( !SVC_DecryptSign( &con, sigbuf.digest, head->signature, key ))
    {
		OS_TPrintf("Authenticate_Header failed: Sign decryption failed.\n");
		if(!s_b_dev) {
			UTL_SetFatalError(FATAL_ERROR_SIGN_DECRYPTION_FAILED);
			return FALSE;
		}
	}
	if(s_calc_hash)
	{
	    // 署名のハッシュ値とヘッダのハッシュ値を比較
	    if(!SVC_CompareSHA1(sigbuf.digest, (const void *)&s_calc_hash[0]))
	    {
			OS_TPrintf("Authenticate_Header failed: Sign compare failed.\n");
			if(!s_b_dev) {
				UTL_SetFatalError(FATAL_ERROR_SIGN_COMPARE_FAILED);
				return FALSE;
			}
		}else
		{
			OS_TPrintf("Authenticate_Header : Sign check succeed. %dms.\n", OS_TicksToMilliSeconds(OS_GetTick() - prev));
		}
	}else
	{
		OS_TPrintf("Authenticate_Header failed: Header Hash calc failed.\n");
		if(!s_b_dev) {
			UTL_SetFatalError(FATAL_ERROR_HEADER_HASH_CALC_FAILED);
			return FALSE;
		}
	}
	OS_TPrintf("Authenticate_Header : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
	
	return TRUE;
}

// TWLアプリ、NTR拡張NANDアプリ 共通のヘッダ認証処理
static BOOL SYSMi_AuthenticateTWLHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
	// pBootTitle->titleIDとROMヘッダのtitleIDの一致確認をする。
	// ホワイトリストマスタリングされたNTRアプリでも行う場合はSYSMi_AuthenticateTWLHeaderへ移動
	if( pBootTitle->titleID != head->s.titleID )
	{
		//TWL対応ROMで、ヘッダのtitleIDが起動指定されたIDと違う
		OS_TPrintf( "Authenticate_Header failed: header TitleID error\n" );
		OS_TPrintf( "Authenticate_Header failed: selectedTitleID=%.16llx\n", pBootTitle->titleID );
		OS_TPrintf( "Authenticate_Header failed: headerTitleID=%.16llx\n", head->s.titleID );
		UTL_SetFatalError(FATAL_ERROR_TITLEID_COMPARE_FAILED);
		return FALSE;
	}else
	{
		OS_TPrintf( "Authenticate_Header : header TitleID check succeed.\n" );
	}
	
	if( head->s.enable_signature || ( SYSM_IsRunOnDebugger() && SYSMi_GetWork()->romEmuInfo.isTlfRom))
	{
		return SYSMi_AuthenticateHeaderWithSign( pBootTitle, head );
	}else
	{
		// 署名有効フラグが立っていない　且つ　デバッガが有効でTLFを読み込んでいるのでなければFAILED
		OS_TPrintf("Authenticate_Header failed: Sign check flag is OFF!\n");
		UTL_SetFatalError(FATAL_ERROR_VALID_SIGN_FLAG_OFF);
		return FALSE;
	}
}

static s32 s_nam_error = NAM_OK;

s32 SYSMi_getCheckTitleLaunchRightsResult( void )
{
	return s_nam_error;
}

// TWLアプリ、NTR拡張NANDアプリ 共通の認証
static BOOL SYSMi_AuthenticateTWLTitle( TitleProperty *pBootTitle )
{
	ROM_Header *head;
	OSTick start,prev;
	start = OS_GetTick();
	
	head = ( ROM_Header *)SYSM_APP_ROM_HEADER_BUF;
	
	// NANDアプリの場合、NAM_CheckTitleLaunchRights()を呼んでチェック
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND )
	{
		s32 result = NAM_CheckTitleLaunchRights( pBootTitle->titleID );
		if( NAM_OK != result)
		{
			s_nam_error = result;
			OS_TPrintf("Authenticate failed: NAM_CheckTitleLaunchRights failed. %d \n",result);
			UTL_SetFatalError(FATAL_ERROR_CHECK_TITLE_LAUNCH_RIGHTS_FAILED);
			return FALSE;
		}else
		{
			OS_TPrintf("Authenticate : NAM_CheckTitleLaunchRights succeed. %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
		}
	}

	// ハッシュ比較
    {
		int l;
		u32 *module_addr[RELOCATE_INFO_NUM];
		u32 module_size[RELOCATE_INFO_NUM];
		u8 *hash_addr[RELOCATE_INFO_NUM];
		int module_num;
		char *gamecode = (char *)&(pBootTitle->titleID);
	    
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
			
			if(s_calc_hash)
			{
				// アプリをロードする時に計算したハッシュを検証
			    if(!SVC_CompareSHA1((const void *)hash_addr[l], (const void *)&s_calc_hash[(l+1) * SVC_SHA1_DIGEST_SIZE]))
			    {
					OS_TPrintf("Authenticate failed: %s module hash check failed.\n", str[l]);
					if(!s_b_dev) {
						UTL_SetFatalError(FATAL_ERROR_MODULE_HASH_CHECK_FAILED);
						return FALSE;
					}
				}else
				{
					OS_TPrintf("Authenticate : %s module hash check succeed. %dms.\n", str[l], OS_TicksToMilliSeconds(OS_GetTick() - prev));
				}
			}else
			{
				OS_TPrintf("Authenticate failed: %s module hash calc failed.\n", str[l]);
				if(!s_b_dev) {
					UTL_SetFatalError(FATAL_ERROR_MODULE_HASH_CALC_FAILED);
					return FALSE;
				}
			}
		}
	}
	OS_TPrintf("Authenticate : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );

	// デバッガ動作以外の時はNANDアプリはNAND、カードアプリはカードからのみブート許可
	if ( ! SYSM_IsRunOnDebugger() )
	{
		if ( ( (pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND ||
				pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP)  && !(head->s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK) ) ||
			   (pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM    &&  (head->s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK) ) )
		{
			UTL_SetFatalError(FATAL_ERROR_MEDIA_CHECK_FAILED);
			return FALSE;
		}
	}

	return TRUE;
}

// NTR版特殊NANDアプリ（pictochat等）のヘッダ認証処理
static BOOL SYSMi_AuthenticateNTRNandAppHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
	return SYSMi_AuthenticateTWLHeader( pBootTitle, head );
}

// NTR版特殊NANDアプリ（pictochat等）の認証
static BOOL SYSMi_AuthenticateNTRNandTitle( TitleProperty *pBootTitle)
{
	return SYSMi_AuthenticateTWLTitle( pBootTitle );
}

// NTR版ダウンロードアプリ（TMPアプリ）のヘッダ認証処理
static BOOL SYSMi_AuthenticateNTRDownloadAppHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
#pragma unused(pBootTitle, head)
	// 署名はstaticに絡んでくるので、ここでチェックするものは特になし。
	return TRUE;
}

// NTR版ダウンロードアプリ（TMPアプリ）の認証
static BOOL SYSMi_AuthenticateNTRDownloadTitle( TitleProperty *pBootTitle)
{
#pragma unused(pBootTitle)
	ROM_Header *head;
	OSTick start;
	start = OS_GetTick();
	
	head = ( ROM_Header *)SYSM_APP_ROM_HEADER_BUF;

	// 署名処理
    {
		u8 buf[0x80];
		SVCSignHeapContext con;
		u8 final_hash[SVC_SHA1_DIGEST_SIZE];

		// NTRダウンロードアプリ署名のマジックコードチェック
		if( s_authcode.magic_code[0] != 'a' || s_authcode.magic_code[1] != 'c' ) {
			OS_TPrintf("Authenticate failed: Invalid AuthCode.\n");
			UTL_SetFatalError(FATAL_ERROR_DL_MAGICCODE_CHECK_FAILED);
			return FALSE;
		}

		// NTRダウンロードアプリ署名（DERフォーマット）の計算、ハッシュの取得。
	    MI_CpuClear8( buf, 0x80 );
	    SVC_InitSignHeap( &con, (void *)SIGN_HEAP_ADDR, SIGN_HEAP_SIZE );// ヒープの初期化
	    if( !SVC_DecryptSignDER( &con, buf, s_authcode.sign, nitro_dl_sign_key ))
	    {
			OS_TPrintf("Authenticate failed: Sign decryption failed.\n");
			UTL_SetFatalError(FATAL_ERROR_DL_SIGN_DECRYPTION_FAILED);
			return FALSE;
		}
		
		// それぞれheader,ARM9FLX,ARM7FLXについてハッシュを計算して、それら3つを並べたものに対してまたハッシュをとる
		if(s_calc_hash)
		{
			// シリアルナンバー付加
			*(u32 *)(&(s_calc_hash[SVC_SHA1_DIGEST_SIZE * 3])) = s_authcode.serial_number;
			// 最終ハッシュ計算
			SVC_CalcSHA1( final_hash, s_calc_hash, SVC_SHA1_DIGEST_SIZE * 3 + sizeof(u32));
		}else
		{
			OS_TPrintf("Authenticate failed: hash calc failed.\n");
			UTL_SetFatalError(FATAL_ERROR_DL_HASH_CALC_FAILED);
			return FALSE;
		}
		
		// 計算した最終ハッシュと、署名から得たハッシュとを比較
	    if(!SVC_CompareSHA1((const void *)buf, (const void *)final_hash))
	    {
			OS_TPrintf("Authenticate failed: hash check failed.\n");
			UTL_SetFatalError(FATAL_ERROR_DL_SIGN_COMPARE_FAILED);
			return FALSE;
		}else
		{
			OS_TPrintf("Authenticate : hash check succeed.\n");
		}
	}
	OS_TPrintf("Authenticate : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
	
	return TRUE;
}

BOOL SYSM_IsLoadTitlePaused(void)
{
	return s_loadPaused;
}

void SYSM_ResumeLoadingThread( BOOL force )
{
	if( !s_loadPaused )
	{
		return;
	}
	s_loadPaused = FALSE;
	// メッセージ送信
	if(!OS_SendMessage(&s_msgQ, (OSMessage)force, OS_MESSAGE_NOBLOCK))
	{
		OS_TPrintf( "SYSM_ResumeLoadingThread:Message send error.\n" );
	}
}

// NTR版カードアプリのヘッダ認証処理
static BOOL SYSMi_AuthenticateNTRCardAppHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
	BOOL ret = TRUE;
	
#define DEV_WHITELIST_CHECK_SKIP
#ifdef DEV_WHITELIST_CHECK_SKIP
	// 開発版ではハッシュチェックスルーフラグを立てる
	if( SCFG_GetBondingOption() != 0 )
	{
		s_b_dev = TRUE;
	}
#endif
	
	if( head->s.exFlags.enable_nitro_whitelist_signature )
	{
		// マスタリング済みNTRカードアプリの署名チェック（実はTWLアプリと同じ）
		ret = SYSMi_AuthenticateHeaderWithSign( pBootTitle, head );
		if( ret == TRUE )
		{
			hash0 = head->s.nitro_whitelist_phase1_digest;
			hash1 = head->s.nitro_whitelist_phase2_diegst;
		}
	}else
	{
		// ホワイトリスト検索
		const DHTDatabase* db;
		PrepareDHTDatabase();// 60msくらいなので、ここでやってしまってOKとする。
		if(!dht)
		{
		    OS_TPrintf(" Search DHT : database init Failed.\n");
		    if(!s_b_dev)
		    {
				UTL_SetFatalError(FATAL_ERROR_WHITELIST_INITDB_FAILED);
			    ret = FALSE;
		    }
		}else
		{
			OS_TPrintf("Searching DHT for %.4s(%02X)...", head->s.game_code, head->s.rom_version);
			db = DHT_GetDatabase(dht, &head->s);
			if ( !db )
			{
			    OS_TPrintf(" Search DHT : Failed.\n");
			    if(!s_b_dev)
			    {
					UTL_SetFatalError(FATAL_ERROR_WHITELIST_NOTFOUND);
				    ret = FALSE;
			    }
			}else
			{
				hash0 = db->hash[0];
				hash1 = db->hash[1];
				ret = TRUE;
			}
		}
	}
	
	return ret;
}

// NTR版カードアプリの認証
static BOOL SYSMi_AuthenticateNTRCardTitle( TitleProperty *pBootTitle)
{
#pragma unused(pBootTitle)
	DHTPhase2Work* p2work;
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;

	// phase1最終検証
	if(s_calc_hash)
	{
		// アプリをロードする時に計算したハッシュを検証
	    if( !hash0 || !SVC_CompareSHA1( (const void *)hash0, (const void *)&s_calc_hash[1 * SVC_SHA1_DIGEST_SIZE] ) )
	    {
			OS_TPrintf("DHT Phase1 failed: hash check failed.\n");
			if(!s_b_dev) {
				UTL_SetFatalError(FATAL_ERROR_DHT_PHASE1_FAILED);
				return FALSE;
			}
		}else
		{
			OS_TPrintf("DHT Phase1 : hash check succeed..\n");
		}
	}else
	{
		OS_TPrintf("DHT Phase1 failed: hash calc failed.\n");
		if(!s_b_dev) {
			UTL_SetFatalError(FATAL_ERROR_DHT_PHASE1_FAILED);
			return FALSE;
		}
	}
	
	// DHTチェックphase2
	OS_TPrintf("DHT Phase2...");
	p2work = SYSM_Alloc( sizeof(DHTPhase2Work) );
	if ( !hash1 || !DHT_CheckHashPhase2(hash1, hs, p2work, WrapperFunc_ReadCardData, NULL) )
	{
	    OS_TPrintf(" DHT Phase2 : Failed.\n");
	    if(!s_b_dev){
		    SYSM_Free(p2work);
			UTL_SetFatalError(FATAL_ERROR_DHT_PHASE2_FAILED);
			return FALSE;
		}
	}
	SYSM_Free(p2work);

	return TRUE;
}

// ヘッダ認証
static BOOL SYSMi_AuthenticateHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
	ROM_Header_Short *hs = ( ROM_Header_Short *)head;
	if( hs->platform_code & PLATFORM_CODE_FLAG_TWL )
	{
		// TWLアプリ
		// 認証処理
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate_Header :TWL_NAND start.\n" );
				return SYSMi_AuthenticateTWLHeader( pBootTitle, head );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate_Header :TWL_ROM start.\n" );
				return SYSMi_AuthenticateTWLHeader( pBootTitle, head );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate_Header :TWL_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					UTL_SetFatalError(FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF);
					return FALSE;
				}
				return SYSMi_AuthenticateTWLHeader( pBootTitle, head );
			default:
				UTL_SetFatalError(FATAL_ERROR_TWL_BOOTTYPE_UNKNOWN);
				return FALSE;
		}
	}
	else
	{
		if( hs->platform_code & PLATFORM_CODE_FLAG_NOT_NTR )
		{
			// TWLでもNTRでもない不正なアプリ
			OS_TPrintf( "Authenticate_Header failed :NOT NTR NOT TWL.\n" );
			UTL_SetFatalError(FATAL_ERROR_PLATFORM_UNKNOWN);
			return FALSE;
		}
		// NTRアプリ
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate_Header :NTR_NAND start.\n" );
				return SYSMi_AuthenticateNTRNandAppHeader( pBootTitle, head );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate_Header :NTR_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate_Header failed : TMP flag error.\n");
					UTL_SetFatalError(FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF);
					return FALSE;
				}
				return SYSMi_AuthenticateNTRDownloadAppHeader( pBootTitle, head );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate_Header :NTR_ROM start.\n" );
				return SYSMi_AuthenticateNTRCardAppHeader( pBootTitle, head );
			default:
				UTL_SetFatalError(FATAL_ERROR_NTR_BOOTTYPE_UNKNOWN);
				return FALSE;
		}
	}
}

// 認証
static BOOL SYSMi_AuthenticateTitleCore( TitleProperty *pBootTitle)
{
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;
	if( hs->platform_code & PLATFORM_CODE_FLAG_TWL )
	{
		// TWLアプリ
		// 認証処理
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate :TWL_NAND start.\n" );
				return SYSMi_AuthenticateTWLTitle( pBootTitle );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate :TWL_ROM start.\n" );
				return SYSMi_AuthenticateTWLTitle( pBootTitle );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate :TWL_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					UTL_SetFatalError(FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF);
					return FALSE;
				}
				return SYSMi_AuthenticateTWLTitle( pBootTitle );
			default:
				UTL_SetFatalError(FATAL_ERROR_TWL_BOOTTYPE_UNKNOWN);
				return FALSE;
		}
	}
	else
	{
		if( hs->platform_code & PLATFORM_CODE_FLAG_NOT_NTR )
		{
			// TWLでもNTRでもない不正なアプリ
			OS_TPrintf( "Authenticate :NOT NTR NOT TWL.\n" );
			UTL_SetFatalError(FATAL_ERROR_PLATFORM_UNKNOWN);
			return FALSE;
		}
		// NTRアプリ
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate :NTR_NAND start.\n" );
				return SYSMi_AuthenticateNTRNandTitle( pBootTitle );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate :NTR_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					UTL_SetFatalError(FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF);
					return FALSE;
				}
				return SYSMi_AuthenticateNTRDownloadTitle( pBootTitle );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate :NTR_ROM start.\n" );
				return SYSMi_AuthenticateNTRCardTitle( pBootTitle );
			default:
				UTL_SetFatalError(FATAL_ERROR_NTR_BOOTTYPE_UNKNOWN);
				return FALSE;
		}
	}
}

// 認証処理のスレッド
static void SYSMi_AuthenticateTitleThreadFunc( TitleProperty *pBootTitle )
{
	// ロード中
	if( !SYSM_IsLoadTitleFinished() ) {
		UTL_SetFatalError(FATAL_ERROR_LOAD_UNFINISHED);
		return;
	}
	// ロード成功？
	if( SYSMi_GetWork()->flags.common.isLoadSucceeded == FALSE )
	{
		UTL_SetFatalError(FATAL_ERROR_TITLE_LOAD_FAILED);
		return;
	}
	// パラメータチェック
	if( !SYSMi_CheckTitlePointer( pBootTitle ) ) {
		UTL_SetFatalError(FATAL_ERROR_TITLE_POINTER_ERROR);
		return;
	}
#if 0
	// エントリアドレスの正当性をチェック
	if( !SYSMi_CheckEntryAddress() ) {
		UTL_SetFatalError(FATAL_ERROR_ENTRY_ADDRESS_ERROR);
		return;
	}
#endif
	
	// BOOTTYPE_MEMORYでNTRモードのFSありでブートすると、旧NitroSDKでビルドされたアプリの場合、
	// ROMアーカイブにカードが割り当てられて、FSで関係ないカードにアクセスにいってしまうので、それを防止する。
	if( ( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_MEMORY ) &&
		( ( (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->platform_code ) == 0 ) &&
		( ( (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->fat_size ) > 0 )
		) {
		UTL_SetFatalError(FATAL_ERROR_TITLE_BOOTTYPE_ERROR);
		return;
	}
	
	// 認証
	(void)SYSMi_AuthenticateTitleCore( pBootTitle );
}


// ロード済みの指定タイトルを別スレッドで検証開始する
#define AUTH_STACK_SIZE 0x1400
void SYSM_StartAuthenticateTitle( TitleProperty *pBootTitle )
{
	static u64 stack[ AUTH_STACK_SIZE / sizeof(u64) ];
	OS_InitThread();
	OS_CreateThread( &s_auth_thread, (void (*)(void *))SYSMi_AuthenticateTitleThreadFunc, (void*)pBootTitle, stack+AUTH_STACK_SIZE/sizeof(u64), AUTH_STACK_SIZE,THREAD_PRIO );
	OS_WakeupThreadDirect( &s_auth_thread );
    
    // ROMヘッダのNintendoロゴ 正当性チェック
    if( !UTL_CheckNintendoLogoData((ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF) ){
		UTL_SetFatalError( FATAL_ERROR_NINTENDO_LOGO_CHECK_FAILED );
    }
}

// 検証済み？
BOOL SYSM_IsAuthenticateTitleFinished( void )
{
	return OS_IsThreadTerminated( &s_auth_thread );
}

// アプリ起動に必要なセーブファイルやSharedファイルのリカバリ
static char *s_strResult[] = {
	"Target file exists and file size matched.",
	"File size didn't match. Changing size succeeded.",
	"Target file didn't exist. Creating file and setting size succeeded.",
	"ERROR: File Recovery Failed."
};
static void SYSMi_FileRecovery( TitleProperty *pBootTitle )
{
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;
	char path[2][ FS_ENTRY_LONGNAME_MAX ];
	UTL_RecoveryStatus stat;
	
	// TWL非対応のときは不要なのでreturn
	if ( !(hs->platform_code & PLATFORM_CODE_FLAG_TWL) )
	{
		return;
	}
	
	// NANDアプリのときだけ
	if ( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND )
	{
		// get savedata_path
		s32 result = NAM_GetTitleSaveFilePath( path[ 0 ], path[ 1 ], hs->titleID );
		
		// pub_save
		if( result == NAM_OK && hs->public_save_data_size != 0 )
		{
			stat = UTL_RecoveryFile( path[0], hs->public_save_data_size );
			OS_TPrintf("pub_save recovery result : %s \n", s_strResult[stat]);
		}
		
		// prv_save
		if( result == NAM_OK && hs->private_save_data_size != 0 )
		{
			stat = UTL_RecoveryFile( path[1], hs->private_save_data_size );
			OS_TPrintf("prv_save recovery result : %s \n", s_strResult[stat]);
		}
		
		// sub_banner
		if( hs->exFlags.availableSubBannerFile && NAM_GetTitleBannerFilePath( path[0], hs->titleID ) == NAM_OK )
		{
			stat = UTL_RecoveryFile( path[0], SIZE_16KB );
			OS_TPrintf("sub_banner recovery result : %s \n", s_strResult[stat]);
		}
	}
	
	// shared2 (size+1) * 16kb
	if( hs->shared2_file0_size != 0 )
	{
		stat = UTL_RecoveryFile( "nand:/shared2/0000", (u32)( hs->shared2_file0_size + 1 ) * SIZE_16KB );
		OS_TPrintf("shared2_0 recovery result : %s \n", s_strResult[stat]);
	}
	if( hs->shared2_file1_size != 0 )
	{
		stat = UTL_RecoveryFile( "nand:/shared2/0001", (u32)( hs->shared2_file1_size + 1 ) * SIZE_16KB );
		OS_TPrintf("shared2_1 recovery result : %s \n", s_strResult[stat]);
	}
	if( hs->shared2_file2_size != 0 )
	{
		stat = UTL_RecoveryFile( "nand:/shared2/0002", (u32)( hs->shared2_file2_size + 1 ) * SIZE_16KB );
		OS_TPrintf("shared2_2 recovery result : %s \n", s_strResult[stat]);
	}
	if( hs->shared2_file3_size != 0 )
	{
		stat = UTL_RecoveryFile( "nand:/shared2/0003", (u32)( hs->shared2_file3_size + 1 ) * SIZE_16KB );
		OS_TPrintf("shared2_3 recovery result : %s \n", s_strResult[stat]);
	}
	if( hs->shared2_file4_size != 0 )
	{
		stat = UTL_RecoveryFile( "nand:/shared2/0004", (u32)( hs->shared2_file4_size + 1 ) * SIZE_16KB );
		OS_TPrintf("shared2_4 recovery result : %s \n", s_strResult[stat]);
	}
	if( hs->shared2_file5_size != 0 )
	{
		stat = UTL_RecoveryFile( "nand:/shared2/0005", (u32)( hs->shared2_file5_size + 1 ) * SIZE_16KB );
		OS_TPrintf("shared2_5 recovery result : %s \n", s_strResult[stat]);
	}
}

// ロード済みの指定タイトルの認証とブートを行う
// SYSM_GetNandTitleListまたはSYSM_GetNandTitleListMakerInfoのどちらかをSYSM_TryToBootTitle前に呼ぶ必要あり
void SYSM_TryToBootTitle( TitleProperty *pBootTitle )
{
	// [TODO:] この内部の処理で失敗した場合にそのままブートしてしまって大丈夫か？
	
	if(s_calc_hash)
	{
		// ハッシュ値保存領域解放
		SYSM_Free( s_calc_hash );
		s_calc_hash = NULL;
	}
	
	// デバッガ接続中以外の時のみTWL設定データにブートするタイトルのTitleIDとplatformCodeを保存。
    if( !SYSM_IsRunOnDebugger() ||                          // スタンドアロン
        (OSi_DetectDebugger() & OS_CONSOLE_TWLDEBUGGER) ) // デバッグ時
    {
        // NANDフラッシュ延命のためブートタイトルが変更された時のみ保存
        // LCFGはSYSM_ReadParametersでリード済み
        if( (pBootTitle->titleID != LCFG_TSD_GetLastTimeBootSoftTitleID()) ||
            ((u8)SYSM_GetAppRomHeader()->platform_code != LCFG_TSD_GetLastTimeBootSoftPlatform()) )
        {
			u8 *pBuffer = SYSM_Alloc( LCFG_WRITE_TEMP );
			if( pBuffer != NULL ) {
				LCFG_TSD_SetLastTimeBootSoftPlatform( (u8)SYSM_GetAppRomHeader()->platform_code );
				(void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
				SYSM_Free( pBuffer );
			}
		}
	}
	
	// マウント情報の登録
	SYSMi_GetWork2()->bootTitleProperty = *pBootTitle;
	SYSMi_SetBootSRLPathToWork2( pBootTitle );
	
	// ブート種別仮セット
	SYSMi_GetWork()->appBootType = s_launcherToOSBootType[ pBootTitle->flags.bootType ];
	
#if 0
	// ブート時ファイルリカバリ処理
	SYSMi_FileRecovery( pBootTitle );
#endif

	// タイトルIDリストの作成
	SYSMi_makeTitleIdList();
	
	// バンブラパッチ
	// SYSMi_applyPatchToBandBrothers();
	
	BOOT_Ready();	// never return.
	
}

// バンブラパッチを当てる関数
static void SYSMi_applyPatchToBandBrothers( void )
{
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;

	if( ( 0 == STD_CompareNString( hs->game_code , "AXBJ", 4 ) ) && ( hs->rom_version == 0 ) )
	{
		s32 len = 0;
		s32 llen;
		FSFile src;
		void *dest;

		// データ読み込み + パッチ
		FS_InitFile( &src );
		if ( !FS_OpenFileEx( &src, "rom:/bandbroth_7flx.bin", FS_FILEMODE_R ) ) return;
		len = (int)FS_GetFileLength( &src );
		
		if( SYSMi_GetWork()->romRelocateInfo[ARM7_STATIC].src != NULL )
		{
			// ARM7FLXが再配置の場合
			dest = (void *)SYSMi_GetWork()->romRelocateInfo[ARM7_STATIC].src;
		}else
		{
			// 再配置なし
			dest = hs->sub_ram_address;
		}

		for(llen = 0; llen < len; )
		{
			int rd;
			rd = FS_ReadFile( &src, dest, len );
			if(rd == -1)
			{
				FS_CloseFile( &src );
				return;
			}
			dest = (void *)((u32)dest + rd);
			llen += rd;
		}
		if ( !FS_CloseFile( &src ) ) return;
		if (len != llen) return;
		
		OS_TPrintf("bandbrothers patch : apply succeeded! \n");
	}
	
	return;
}

// タイトルIDリストの作成
static void SYSMi_makeTitleIdList( void )
{
	OSTitleIDList *list = ( OSTitleIDList * )HW_OS_TITLE_ID_LIST;
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;
	int l;
	u8 count = 0;
	int max = AMN_getRomHeaderListLength();
	OSTick start;
	
	// 時間計測総合
	start = OS_GetTick();
	
	// とりあえずゼロクリア
	MI_CpuClear8( (void *)HW_OS_TITLE_ID_LIST, HW_OS_TITLE_ID_LIST_SIZE );

	// これから起動するアプリがTWLアプリでない
	if( !hs->platform_code )
	{
		return;
	}

	for(l=0;l<max;l++)
	{
		int m;
		BOOL same_maker_code = TRUE;
		char *gamecode;
		OSTitleId id;
		ROM_Header_Short *pe_hs;

		pe_hs = &((AMN_getRomHeaderList())[l]);
		id = pe_hs->titleID;
		
		for(m=0;m<MAKER_CODE_MAX;m++)
		{
			if(hs->maker_code[m] != pe_hs->maker_code[m])
			{
				same_maker_code = FALSE;
			}
		}
		
		// 無効なTitleIDはスキップ
		if( id == NULL )
		{
			continue;
		}
		
		gamecode = (char *)&(id);
		// バージョン情報の特殊処理
		if( ( 0 == STD_CompareNString( &gamecode[1], "LNH", 3 ) ) )
		{
			char path[ FS_ENTRY_LONGNAME_MAX ];
			char *p;
			NAM_GetTitleBootContentPathFast(path, id);
			p = STD_SearchString( path, ".app" );
			if( p == NULL)
			{
				// 失敗
				continue;
			}
			MI_CpuCopy8( p-8, (void *)HW_SYSM_VER_INFO_CONTENT_ID, 8 );
			((char *)HW_SYSM_VER_INFO_CONTENT_ID )[8] = '\0';
			((char *)HW_SYSM_VER_INFO_CONTENT_ID )[9] = gamecode[0];
		}
	
		// ランチャーはリストに入れない
		if( ( 0 == STD_CompareNString( &gamecode[1], "ANH", 3 ) )
#ifdef DEV_UIG_LAUNCHER
		 || ( ( 0 == STD_CompareNString( &gamecode[1], "AN4", 3 ) ) && ( SCFG_GetBondingOption() != 0 ) )
#endif
		)
		{
			continue;
		}
		
		if( same_maker_code )
		{
			// リストに追加
			list->TitleID[count] = id;
			// sameMakerFlagをON
			list->sameMakerFlag[count/8] |= (u8)(0x1 << (count%8));
		}
		
		// ジャンプ可能フラグON or ブートアプリ自身 or ジャンプ元アプリ ならばジャンプ可能
		if( pe_hs->permit_landing_normal_jump || hs->titleID == id ||
			( SYSMi_GetWork()->flags.common.isValidLauncherParam && SYSM_GetLauncherParamBody()->v1.bootTitleID && ( SYSM_GetLauncherParamBody()->v1.prevTitleID == id ) )
		  )
		{
			// リストに追加してジャンプ可能フラグON
			list->TitleID[count] = id;
			list->appJumpFlag[count/8] |= (u8)(0x1 << (count%8));
		}
		
		// ブートアプリがセキュアアプリの場合
		if( hs->titleID & TITLE_ID_SECURE_FLAG_MASK )
		{
			// Prv,Pubそれぞれセーブデータがあるか見て、存在すればフラグON
			if(pe_hs->public_save_data_size != 0)
			{
				list->publicFlag[count/8] |= (u8)(0x1 << (count%8));
			}
			if(pe_hs->private_save_data_size != 0)
			{
				list->privateFlag[count/8] |= (u8)(0x1 << (count%8));
			}
			// リストに強制追加
			list->TitleID[count] = id;
		}else
		{
			// セキュアアプリでない && メーカーコードが同じ
			if( !(id & TITLE_ID_SECURE_FLAG_MASK) && same_maker_code )
			{
				// Prv,Pubそれぞれセーブデータがあるか見て、存在すればフラグON
				if(pe_hs->public_save_data_size != 0)
				{
					list->publicFlag[count/8] |= (u8)(0x1 << (count%8));
					// リストに追加
					list->TitleID[count] = id;
				}
				if(pe_hs->private_save_data_size != 0)
				{
					list->privateFlag[count/8] |= (u8)(0x1 << (count%8));
					// リストに追加
					list->TitleID[count] = id;
				}
			}
		}
		
		// ここまでのうちに、list->TitleID[count]が編集されていたらcountインクリメント
		if( list->TitleID[count] != NULL )
		{
			count++;
		}
	}
	list->num = count;
	// end時間計測総合
	OS_TPrintf("SYSMi_makeTitleIdList : total %dms\n",OS_TicksToMilliSeconds(OS_GetTick() - start));
}


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
