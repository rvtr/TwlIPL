/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     NTRSettings.c

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
#include <sysmenu/settings/common/NTRSettings.h>
#include "spi.h"

// define data----------------------------------------------------------
#if 0
#define DEBUG_Printf					OS_TPrintf
#else
#define DEBUG_Printf(...)				((void)0)
#endif

#define SAVE_COUNT_MAX					0x0080		// NTRSettingsData.saveCountの最大値
#define SAVE_COUNT_MASK					0x007f		// NTRSettingsData.saveCountの値の範囲をマスクする。(0x00-0x7f）
#define NSD_NOT_CORRECT					0x00ff		// NTR設定データが読み出されていない or 有効なものがないことを示す。
#define NVRAM_RETRY_NUM					8			// NVRAMリトライ回数

// NVRAMステータスレジスタ値
#define SR_WIP							0x01		// 0:READY       1:ライト、イレース中
#define SR_WEN							0x02		// 0:ライト禁止  1:ライト許可
#define SR_EER							0x20		// 1:イレースエラー発生（SANYO製FLASHのみ）

#define READ_IPL2_HEADER_ADDR			0x18		// IPL2ヘッダのうち、読み込みが必要な部分の先頭アドレス
#define READ_IPL2_HEADER_SIZE			0x0a		// IPL2ヘッダのうち、読み込みが必要なサイズ
#define NSD_ROM_ADDR_SHIFT				3

// NVRAM関連送信コマンドステート
static enum NvramCommState{
	COMM_READY = 0,
	COMM_RD,
	COMM_WE,
	COMM_WR,
	COMM_RDSR_WE,
	COMM_RDSR_WR,
	COMM_SRST
}NvramCommState;


// IPL2ヘッダの一部（0x18からのデータ)
typedef struct IPL2HeaderPart {
	struct {
		u8	timestamp[5];				// NTR-IPL2タイムスタンプ [0]:分,[1]:時,[2]:日,[3]:月,[4]:年
		u8	ipl2_type;					// NTR-IPL2タイプ
		u8	rsv[2];
	} version;
	
	u16		nsd_rom_addr;
	
	u8		pad[ 0x16 ];				// ※キャッシュラインに合わせるためのパディング。本来は必要なし。
} IPL2HeaderPart;	// 0x20bytes


// function's prototype-------------------------------------------------
u32  NSD_GetNSDRomAddr( void );			// NTRSettingデータのNVRAM格納アドレスを取得
u8   NSD_GetIPL2Type( void );			// NTR-IPL2タイプを取得
const u8 *NSD_GetIPL2Timestamp( void );	// NTR-IPL2のタイムスタンプを取得

static void NSDi_ReadIPL2Header( void );
static BOOL NSDi_CheckCorrectNSD( NSDStoreEx (*pNSDStoreExArray)[2], u8 region );
static BOOL NSDi_CheckDataValue( NSDStoreEx *pNSDStore, u8 region );
static BOOL NVRAMm_ExecuteCommand( int nvState, u32 addr, u16 size, u8 *pSrc );
static void Callback_NVRAM( PXIFifoTag tag, u32 data, BOOL err );

// static variables-----------------------------------------------------
static NSDStoreEx		s_NSDStoreEx ATTRIBUTE_ALIGN(32);
static IPL2HeaderPart	s_IPL2Header ATTRIBUTE_ALIGN(32);
static BOOL				s_isReadIPL2H = FALSE;
static volatile BOOL	s_nvCbOccurred;
static volatile u16		s_nvResult;
static int 				s_indexNSD = NSD_NOT_CORRECT;

#ifndef SDK_FINALROM
static NSDStoreEx (*s_pNSDStoreExArray)[2];
#endif
// global variables-----------------------------------------------------
NTRSettingsData			*g_pNSD   = &s_NSDStoreEx.nsd;
NTRSettingsDataEx		*g_pNSDEx = &s_NSDStoreEx.nsd_ex;

// const data-----------------------------------------------------------
static const u16 s_validLangBitmapList[] = {				// ※TWLに合わせた方が良さそう。
	NTR_LANG_BITMAP_WW,				// TWL_REGION_JAPAN
	NTR_LANG_BITMAP_WW,				// TWL_REGION_AMERICA
	NTR_LANG_BITMAP_WW,				// TWL_REGION_EUROPE
	NTR_LANG_BITMAP_WW,				// TWL_REGION_AUSTRALIA
	NTR_LANG_BITMAP_CHINA,			// TWL_REGION_CHINA
	NTR_LANG_BITMAP_KOREA,			// TWL_REGION_KOREA
};


// function's description-----------------------------------------------

//----------------------------------------------------------------------
// IPL2ヘッダ情報の読み出し
//----------------------------------------------------------------------
// コンポーネント上での使用時

// IPL2ヘッダの読み出し
static void NSDi_ReadIPL2Header( void )
{
	if( !s_isReadIPL2H ) {
		OS_TPrintf( "IPL2Header:%x\n",   sizeof(IPL2HeaderPart) );
		DC_InvalidateRange( &s_IPL2Header, sizeof(IPL2HeaderPart) );
		while( !NVRAMm_ExecuteCommand( COMM_RD, READ_IPL2_HEADER_ADDR, READ_IPL2_HEADER_SIZE, (u8 *)&s_IPL2Header ) ) {}
		s_isReadIPL2H = TRUE;
	}
}

// IPL2タイプの取得
u8 NSD_GetIPL2Type( void )
{
	NSDi_ReadIPL2Header();
	return s_IPL2Header.version.ipl2_type;
}

// IPL2タイムスタンプの取得
const u8 *NSD_GetIPL2Timestamp( void )
{
	NSDi_ReadIPL2Header();
	return s_IPL2Header.version.timestamp;
}

// NSD格納ROMアドレスの取得
u32 NSD_GetNSDRomAddr( void )
{
	NSDi_ReadIPL2Header();
	return (u32)( s_IPL2Header.nsd_rom_addr << NSD_ROM_ADDR_SHIFT );
}

//----------------------------------------------------------------------
// NTR設定データのリード
//----------------------------------------------------------------------

// NTR設定データリード済み
BOOL NSD_IsReadSettings( void )
{
	return ( s_indexNSD != NSD_NOT_CORRECT );
}


BOOL NSD_ReadSettings( u8 region, NSDStoreEx (*pTempBuffer)[2] )
{
	NSDStoreEx *pNSDStoreEx = (NSDStoreEx *)pTempBuffer;
#ifndef SDK_FINALROM
	s_pNSDStoreExArray = pTempBuffer;
	OS_TPrintf( "NSDStoreBuff : %08x %08x\n", &(*s_pNSDStoreExArray)[ 0 ], &(*s_pNSDStoreExArray)[ 1 ] );
#endif
	
	DC_InvalidateRange( pNSDStoreEx, sizeof(NSDStoreEx) * 2 );
	
	// フラッシュからニ重化されているNTR設定データを読み出す。
	while( !NVRAMm_ExecuteCommand( COMM_RD, NSD_GetNSDRomAddr(),                       sizeof(NSDStoreEx), (u8 *)&pNSDStoreEx[ 0 ] ) ) {}
	while( !NVRAMm_ExecuteCommand( COMM_RD, NSD_GetNSDRomAddr() + SPI_NVRAM_PAGE_SIZE, sizeof(NSDStoreEx), (u8 *)&pNSDStoreEx[ 1 ] ) ) {}
	OS_TPrintf("NSD read addr=%08x\n", NSD_GetNSDRomAddr() );
	
	// 読み出したデータのどちらが有効かを判定する。
	if( NSDi_CheckCorrectNSD( pTempBuffer, region ) ) {
		// 有効なNTR設定データを静的バッファに転送
		MI_CpuCopyFast( (void *)&pNSDStoreEx[ s_indexNSD ], (void *)&s_NSDStoreEx, sizeof(NSDStoreEx) );
	}else {
		// 有効なデータがないなら、バッファをクリアする
		OS_TPrintf( "NSD clear.\n" );
		NSD_ClearSettings();
		return FALSE;
	}
	
	OS_TPrintf("Use NSD[%d]   : saveCount = %d\n", s_indexNSD, s_NSDStoreEx.saveCount);
	
	return TRUE;
}


//----------------------------------------------------------------------
// NTR設定データのライト
//----------------------------------------------------------------------
BOOL NSD_WriteSettings( u8 region )
{
	int retry;
	u32 nvramAddr;
	
	// まだNTR設定データがリードされていなければ、リードを行って必要な情報を取得する。
	if( !NSD_IsReadSettings() ) {
		OS_TPrintf( "ERROR: Need call NSD_ReadSetting.\n" );
		return FALSE;
	}
	
	// NSD   のCRC、セーブカウント値、ライトアドレスの算出。
	s_NSDStoreEx.nsd.version    = NTR_SETTINGS_DATA_VERSION;	// バージョンを現在のものに設定。
	s_NSDStoreEx.crc16          = SVC_GetCRC16( 0xffff, (const void *)&s_NSDStoreEx.nsd, sizeof(NTRSettingsData) );
	s_NSDStoreEx.saveCount      = (u8)( ( s_NSDStoreEx.saveCount + 1 ) & SAVE_COUNT_MASK );
	
	// NSD_EXのCRC算出。
	s_NSDStoreEx.nsd_ex.version = NTR_SETTINGS_DATA_EX_VERSION;	// バージョンを現在のものに設定。
	s_NSDStoreEx.nsd_ex.valid_language_bitmap = s_validLangBitmapList[ region ];
	s_NSDStoreEx.crc16_ex       = SVC_GetCRC16( 0xffff, (const void *)&s_NSDStoreEx.nsd_ex, sizeof(NTRSettingsDataEx) );
	
	// NTR設定データのライト
	DC_FlushRange( &s_NSDStoreEx, sizeof(NSDStoreEx) );
	retry = NVRAM_RETRY_NUM;
	while( retry-- ) {
		s_indexNSD ^= 0x01;									// リトライの度に書き込みアドレスを切り替える。
		nvramAddr = NSD_GetNSDRomAddr() + s_indexNSD * SPI_NVRAM_PAGE_SIZE;
		OS_TPrintf("NSD write addr=%08x\n", nvramAddr );
		
		if( NVRAMm_ExecuteCommand( COMM_WE, nvramAddr, sizeof(NSDStoreEx), (u8 *)&s_NSDStoreEx ) ) {
			OS_TPrintf("NVRAM Write succeeded.\n");
			break;
		}
		SVC_WaitByLoop( 0x4000 );
		OS_TPrintf("NVRAM Write retry = %d.\n", NVRAM_RETRY_NUM - retry );
	}
	return TRUE;
}


//----------------------------------------------------------------------
// ミラーリングされているNTR設定データのどちらが有効かを判定
//----------------------------------------------------------------------

static BOOL NSDi_CheckCorrectNSD( NSDStoreEx (*pNSDStoreExArray)[2], u8 region )
{
	NSDStoreEx *pNSDStoreEx = (NSDStoreEx *)pNSDStoreExArray;
	u16 i;
	u16 nsd_valid = 0;
	
	// 各ミラーデータのCRC & saveCount正当性チェック
	for( i = 0; i < 2; i++ ) {
		u16  crc;
		BOOL isInvalid = FALSE;
		
		// NSD のCRCチェックを行う。
		crc = SVC_GetCRC16( 0xffff, (const void *)&pNSDStoreEx[i].nsd, sizeof(NTRSettingsData) );
		
		if(    ( pNSDStoreEx[ i ].crc16       != crc )				// CRCが正しく、saveCount値が0x80以下で、かつバージョンが一致するデータを正当と判断。
			|| ( pNSDStoreEx[ i ].nsd.version != NTR_SETTINGS_DATA_VERSION )
			|| ( pNSDStoreEx[ i ].saveCount   >= SAVE_COUNT_MAX ) ) {
			OS_TPrintf("NSD   crc error.\n");
			isInvalid = TRUE;
		}
		
		// NSDEx のCRCチェックを行う。
		crc = SVC_GetCRC16( 0xffff, (const void *)&pNSDStoreEx[i].nsd_ex, sizeof(NTRSettingsDataEx) );
		
		if(   ( pNSDStoreEx[ i ].crc16_ex       != crc )
		   || ( pNSDStoreEx[ i ].nsd_ex.version != NTR_SETTINGS_DATA_EX_VERSION ) ) {
			OS_TPrintf("NSDEx crc error.\n");
			isInvalid = TRUE;
		}
		
		// NSD, NSDExのCRCが正しいなら、データの中身をチェック。
		if( !isInvalid ) {
			if( NSDi_CheckDataValue( &pNSDStoreEx[ i ], region ) ) {	// データがおかしい値でないかもチェック。
				nsd_valid  |= 0x01 << i;								// "有効"フラグをセット
				s_indexNSD = i;										// NCDのインデックスも切り替え。
			}else {
				isInvalid = TRUE;
			}
		}
		
		if( nsd_valid & ( 0x01 << i ) ) {
			OS_TPrintf("NSD[%d] valid : saveCount = %d\n", i, pNSDStoreEx[i].saveCount);
		}else {
			OS_TPrintf("NSD[%d] invalid.\n", i);
		}
	}
	
	
	if( nsd_valid == 0 ) {
		s_indexNSD = 1;			// 最初のWrite時に"0"になるように"1"にしておく
		return FALSE;
	}else if( nsd_valid == 0x03 ) {									
		// ミラーリングされたNSDが両方ともに正当な場合、セーブカウント値が大きい方を有効とする。
		u16 saveCount = (u8)( ( pNSDStoreEx[ 0 ].saveCount + 1 ) & SAVE_COUNT_MASK );
		s_indexNSD = ( saveCount == pNSDStoreEx[ 1 ].saveCount ) ? (u16)1 : (u16)0;
	}
	return TRUE;
}


// NTR設定データの値が正しい値かチェック。	// FALSE:正しくない。TRUE：正しい。
static BOOL NSDi_CheckDataValue( NSDStoreEx *pNSDStoreEx, u8 region )
{
	NTRSettingsData   *pNSD   = &pNSDStoreEx->nsd;
	NTRSettingsDataEx *pNSDEx = &pNSDStoreEx->nsd_ex;
	u16 validLangBitmap = s_validLangBitmapList[ region ];
	
	//pNSD->option;
	// NSDのlanguageチェック（ NSD側のlanguageは、日・英・独・仏・伊・西の６言語のうちの、対応言語のみの値となる。）
	if( ~( NTR_LANG_BITMAP_WW & validLangBitmap ) & ( 0x0001 << pNSD->option.language ) ) {
		OS_TPrintf("NSD: invalid language        : org:%02d ex:%02d bitmap:%04x\n",
				   pNSD->option.language, pNSDEx->language, pNSDEx->valid_language_bitmap );
		return FALSE;
	}
	
	// NSDExのlanguageチェック（こちらには、中・韓も入る）
	if( ( ~validLangBitmap & ( 0x0001 << pNSDEx->language ) ) ||
		( pNSDEx->valid_language_bitmap != validLangBitmap ) ) {
		OS_TPrintf("NSDEx: invalid language    : org:%02d ex:%02d bitmap:%04x\n",
				   pNSD->option.language, pNSDEx->language, pNSDEx->valid_language_bitmap );
		return FALSE;
	}
	
	//pNSD->owner;
	// favoriteColorは4bitなので範囲外はない。
	
	// birthday
	if( pNSD->option.isSetBirthday ) {
		if( ( pNSD->owner.birthday.month > 12 ) || ( pNSD->owner.birthday.day > 31 ) ) {
			OS_TPrintf("NSD: invalid birthday        : %02d/%02d\n", pNSD->owner.birthday.month, pNSD->owner.birthday.day );
			return FALSE;
		}
	}
	
	// nickname
	if( pNSD->option.isSetNickname ) {
		if( pNSD->owner.nickname.length > NTR_NICKNAME_LENGTH ) {
			OS_TPrintf("NSD: invalid nickname length : %02d\n", pNSD->owner.nickname.length );
			return FALSE;
		}
	}
	
	// comment
	if( pNSD->owner.comment.length  > NTR_COMMENT_LENGTH ) {
		OS_TPrintf("NSD: invalid comment  length     : %02d\n", pNSD->owner.comment.length );
		return FALSE;
	}
	
	//pNSD->alarm;
	if( ( pNSD->alarm.hour > 23 ) || ( pNSD->alarm.minute > 59 ) ) {
		OS_TPrintf("NSD: invalid alarm time          : %02d:%02d\n", pNSD->alarm.hour, pNSD->alarm.minute );
		return FALSE;
	}
	
	//pNSD->tp;
	// TPキャリブレーション値は、TP_CalcCalibrateParamで値のチェックをしているので、チェックしない。
	
//	OS_TPrintf( "NSD: correct data.\n" );
	return TRUE;
}


// NTR設定データのクリア
void NSD_ClearSettings( void )
{
	NSDStoreEx *pNSDStoreEx = &s_NSDStoreEx;
	
	s_indexNSD = 1;							// ライト前に反転されるので、"0"側が選択されるように"1"にしておく
	
	MI_CpuClear16( pNSDStoreEx, sizeof(NSDStoreEx) );
	// 初期値が0以外のもの
	pNSDStoreEx->nsd.version    = NTR_SETTINGS_DATA_VERSION;
	pNSDStoreEx->nsd_ex.version = NTR_SETTINGS_DATA_EX_VERSION;
	pNSDStoreEx->nsd.owner.birthday.month = 1;
	pNSDStoreEx->nsd.owner.birthday.day   = 1;
	OS_TPrintf( "NSDStoreEx cleared.\n" );
}


// NTR設定データのニックネーム・色・誕生日の初期化。
void NSD_ClearOwnerInfo( void )
{
	MI_CpuClear16( &GetNSD()->owner, sizeof(NTROwnerInfo) );
	GetNSD()->owner.birthday.month	= 1;
	GetNSD()->owner.birthday.day	= 1;
	GetNSD()->option.isSetBirthday	= 0;
	GetNSD()->option.isSetUserColor	= 0;
	GetNSD()->option.isSetNickname	= 0;
}


//----------------------------------------------------------------------
// NVRAMへのアクセスルーチン本体 ( nvState <- COMM_RD or COMM_WE )
//----------------------------------------------------------------------
static BOOL NVRAMm_ExecuteCommand( int nvState, u32 addr, u16 size, u8 *pSrc )
{
	static u8 sr_buf[ 32 ] ATTRIBUTE_ALIGN(32);
    OSTick	start;
	BOOL	isSending = FALSE;
	u8		*pSR = (u8 *)sr_buf;
	
	PXI_SetFifoRecvCallback( PXI_FIFO_TAG_NVRAM , Callback_NVRAM );
	
	while( 1 ) {
		//---------------------------------------
		// NVRAMコマンドを発行する
		//---------------------------------------
		if( !isSending ) {
			
			s_nvCbOccurred	= FALSE;
			
			switch( nvState ) {
			  case COMM_RD:
				isSending	= SPI_NvramReadDataBytes( addr, size, pSrc );
				break;
				
			  case COMM_WE:
				isSending	= SPI_NvramWriteEnable();
				break;
				
			  case COMM_WR:
				isSending	= SPI_NvramPageWrite( addr, size , pSrc );
				start		= OS_GetTick();
				break;
				
			  case COMM_RDSR_WE:
			  case COMM_RDSR_WR:
				isSending	= SPI_NvramReadStatusRegister( pSR );
				break;
				
			  case COMM_SRST:
				isSending	= SPI_NvramSoftwareReset();
				break;
			}
		//---------------------------------------
		// コマンド実行結果（コールバック発生）を待って結果を処理する
		//---------------------------------------
		}else { // isSending == TRUE
			if( s_nvCbOccurred == TRUE ) {							// コールバック発生を待つ。
				
				isSending = FALSE;
				
				if( s_nvResult == SPI_PXI_RESULT_SUCCESS ) {
					switch( nvState ) {
					  case COMM_RD:
						return TRUE;
						
					  case COMM_WE:
						nvState = COMM_RDSR_WE;
						break;
						
					  case COMM_WR:
						nvState = COMM_RDSR_WR;
						break;
						
					  case COMM_RDSR_WE:
					  case COMM_RDSR_WR:
						
						DC_InvalidateRange( pSR, 1 );
						
						if( nvState == COMM_RDSR_WE ) {				// ライトイネーブル確認ステートなら
							if( ( *pSR & SR_WEN ) ) {
								nvState = COMM_WR;
							}else {
								OS_TPrintf("NVRAM ERR: Write Enable Invalid.\n");
								return FALSE;
							}
						}else {
							if( ( *pSR & SR_WIP ) == 0 ) {		// ライト／イレース終了
								return TRUE;
							}else {
								if(	  ( *pSR & SR_EER )			// SR_EERが立っていたらエラー
								   || ( OS_TicksToMilliSeconds( OS_GetTick() - start ) > 4000 ) ) {
																		// コマンド発行から4秒経過したらエラー（※保険）
									DEBUG_Printf( "NVRAM SR : %02x\n", *pSR );
									nvState = COMM_SRST;
								}else {
									SVC_WaitByLoop( 0x4000 );
								}
							}
						}
						break;
						
					  case COMM_SRST:
						OS_TPrintf("NVRAM ERR: PageErase Timeout and SoftReset.\n");
						return FALSE;
					}
				}else {  // s_nvResult != SPI_PXI_RESULT_SUCCESS
					OS_TPrintf("NVRAM ERR: NVRAM PXI command failed.\n");
					return FALSE;
				}
			}
		}
	}
}


//----------------------------------------------------------------------
// コールバック
//----------------------------------------------------------------------
static void Callback_NVRAM( PXIFifoTag tag, u32 data, BOOL err )
{
	#pragma unused(tag)
	
	u16 command		= (u16)( ( ( data & SPI_PXI_DATA_MASK ) & 0x7f00 ) >> 8 );
	
	s_nvResult		= (u16)( data & 0x00ff );
	s_nvCbOccurred	= TRUE;											// コールバック発生フラグTRUE
	
	if( err ) {
		OS_TPrintf("NVRAM-ARM9: Received PXI data is error.\n");
		s_nvResult = 0x00ff;
	}
	
	switch(command){												// コマンド名表示
	  case SPI_PXI_COMMAND_NVRAM_READ:
		DEBUG_Printf("NVRAM-ARM9:ReadDataBytes");
		break;
	  case SPI_PXI_COMMAND_NVRAM_WREN:
		DEBUG_Printf("NVRAM-ARM9:WriteEnable");
		break;
	  case SPI_PXI_COMMAND_NVRAM_PW:
		DEBUG_Printf("NVRAM-ARM9:PageWrite");
		break;
	  case SPI_PXI_COMMAND_NVRAM_RDSR:
		DEBUG_Printf("NVRAM-ARM9:ReadStatusRegister");
		break;
	  case SPI_PXI_COMMAND_NVRAM_WRDI:
		DEBUG_Printf("NVRAM-ARM9:WriteDisable");
		break;
	  case SPI_PXI_COMMAND_NVRAM_PE:
		DEBUG_Printf("NVRAM-ARM9:PageErase");
		break;
	  case SPI_PXI_COMMAND_NVRAM_SR:
		DEBUG_Printf("NVRAM-ARM9:SoftwareReset");
		break;
	  default:
		DEBUG_Printf("NVRAM-ARM9:?????");
		break;
	}
	if( s_nvResult != SPI_PXI_RESULT_SUCCESS ) {
		OS_TPrintf(" Error! ->%x", s_nvResult );
	}
	DEBUG_Printf("\n");
}

