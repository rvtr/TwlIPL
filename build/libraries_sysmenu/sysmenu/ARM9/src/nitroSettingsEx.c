/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     nitroSettingsEx.c

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
#include <sysmenu/machineSettings/common/nitroSettings.h>
#include "spi.h"

#ifndef USED_COMPONENT
#include <sysmenu/sysmenu_work.h>
#endif

// define data----------------------------------------------------------

#define NCD_EX_FORCE_ENABLE							// このスイッチを定義すると、SYSMバージョンに関わらず強制的にNitroConfigDataExが有効になる。
													// ※実機上では、このフラグのON,OFFに関係なく正常動作するが、アプリビルドをデバッガや他実機で動作させる際に、ここがONでないとダメ。
													//   読み込まれていないNCDEXをリードして死亡してしまうので注意。

#define SAVE_COUNT_MAX					0x0080		// NitroConfigData.saveCountの最大値
#define SAVE_COUNT_MASK					0x007f		// NitroConfigData.saveCountの値の範囲をマスクする。(0x00-0x7f）
#define NCD_NOT_CORRECT					0x00ff		// NITRO設定データが読み出されていない or 有効なものがないことを示す。
#define NVRAM_RETRY_NUM					8			// NVRAMリトライ回数

// NVRAMステータスレジスタ値
#define SR_WIP							0x01		// 0:READY       1:ライト、イレース中
#define SR_WEN							0x02		// 0:ライト禁止  1:ライト許可
#define SR_EER							0x20		// 1:イレースエラー発生（SANYO製FLASHのみ）


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


// SYSMヘッダ情報
typedef struct SYSMHeader {
	u16		bm_arm9_rom_addr;
	u16		bm_arm7_rom_addr;
	u16		bm_crc16;
	u16		ipl2_crc16;
	u8		ipl2_key[4];
	u16		ipl2_arm9_rom_addr;
	u16		ipl2_arm9_ram_addr;
	u16		ipl2_arm7_rom_addr;
	u16		ipl2_arm7_ram_addr;
	u16		ipl2_arm9_romOffsetUnit:3;
	u16		ipl2_arm9_ramOffsetUnit:3;
	u16		ipl2_arm7_romOffsetUnit:3;
	u16		ipl2_arm7_ramOffsetUnit:3;
	u16		ipl2_arm7_ramSelect:1;
	u16		ipl2_header_ver:3;
	u16		ipl2_data_rom_addr;
	union {
		struct {
			u8	timestamp[5];				// SYSMタイムスタンプ [0]:分,[1]:時,[2]:日,[3]:月,[4]:年
			u8	ipl2_type;					// SYSMタイプ(nitroConfigData.hで定義のSYSM_TYPE...）
			u8	rsv[2];
		} version;
		u8		card_key[8];
	} info;
	u16		ncd_rom_addr;
	union {									// SYSMHeader.info.version.type == SYSM_TYPE_I_DISPLAY_JAPANもしくは
		u16		sys_rsv_rom_addr;			// SYSMHeader.info.version.timestamp >= 0x0502280851(SYSM_TYPE_NORMAL 3rd.以降）
		u16		font_bncmp_offset;			// ならば、font_bncmp_offset, font_bnfr_offsetが有効。
	} rom_addr1;							// ※なお、sys_rsv_rom_addr, app_rsv_rom_addrは、アドレスが変わったので
	union {									// 　ここに入っている値は無効。ncd_rom_addrからの相対値で算出。
		u16		app_rsv_rom_addr;
		u16		font_bnfr_offset;
	} rom_addr2;
	u16		ipl2_data_crc16;
	
	u8		pad[ 0x18 ];					// ※キャッシュラインに合わせるためのパディング。本来は必要なし。
	
} SYSMHeader;	// 0x40bytes


// function's prototype-------------------------------------------------
static int  NVRAMm_checkCorrectNCD(NCDStoreEx *ncdsp);
static BOOL NCD_CheckDataValue( NCDStoreEx *ncdsp );
static BOOL NVRAMm_ExecuteCommand( int nv_state, u32 addr, u16 size, u8 *srcp );
static void Callback_NVRAM(PXIFifoTag tag, u32 data, BOOL err);

// const data-----------------------------------------------------------

// global variables-----------------------------------------------------
NitroConfigDataEx 		ncdEx;

// static variables-----------------------------------------------------
static volatile BOOL	nv_cb_occurred;
static volatile u16		nv_result;
static u16 				ena_ncd_num = NCD_NOT_CORRECT;
static u16 				next_saveCount;
static NCDStoreEx		ncds[2] ATTRIBUTE_ALIGN(32);

#ifdef USED_COMPONENT
static SYSMHeader		ipl2Header ATTRIBUTE_ALIGN(32);
static BOOL				read_ipl2h = FALSE;
#endif


// function's description-----------------------------------------------

//----------------------------------------------------------------------
// SYSMヘッダ情報の読み出し
//----------------------------------------------------------------------
#ifndef USED_COMPONENT
// SYSM上での使用時

// SYSMタイプの取得
#define NCD_GetSYSMType()			( (u32)GetSYSMWork()->ipl2_type )

// NCD格納ROMアドレスの取得
#define NCD_GetNCDRomAddr()			( (u32)GetSYSMWork()->ncd_rom_adr )

#else
// コンポーネント上での使用時

// SYSMヘッダの読み出し
void NCD_ReadSYSMHeader( void )
{
	if( !read_ipl2h ) {
		OS_TPrintf( "SYSMHeader:%x\n", sizeof(SYSMHeader) );
		DC_InvalidateRange( &ipl2Header, sizeof(SYSMHeader) );
		while( !NVRAMm_ExecuteCommand( COMM_RD, 0, sizeof(SYSMHeader), (u8 *)&ipl2Header ) ) {}
		read_ipl2h		= TRUE;
	}
}

// SYSMタイプの取得
u8 NCD_GetSYSMType( void )
{
	NCD_ReadSYSMHeader();
	return ipl2Header.info.version.ipl2_type;
}

// SYSMバージョンの取得
u8 *NCD_GetSYSMVersion( void )
{
	NCD_ReadSYSMHeader();
	return ipl2Header.info.version.timestamp;
}

// NCD格納ROMアドレスの取得
u32 NCD_GetNCDRomAddr( void )
{
	NCD_ReadSYSMHeader();
	return (u32)( ipl2Header.ncd_rom_addr << NCD_ROM_ADDR_SHIFT );
}

// システム予約領域ROMアドレスの取得
u32 NCD_GetSysRsvRomAddr( void )
{
	NCD_ReadSYSMHeader();
	return (u32)( ipl2Header.ncd_rom_addr << NCD_ROM_ADDR_SHIFT ) - NCD_SYS_RSV_SIZE;
}

// アプリ予約領域ROMアドレスの取得
u32 NCD_GetAppRsvRomAddr( void )
{
	NCD_ReadSYSMHeader();
	return (u32)( ipl2Header.ncd_rom_addr << NCD_ROM_ADDR_SHIFT ) - NCD_SYS_RSV_SIZE - NCD_APP_RSV_SIZE;
}

// SYSMデータ格納ROMアドレスの取得
u32 NCD_GetSYSMDataRomAddr( void )
{
	NCD_ReadSYSMHeader();
	return (u32)( ipl2Header.ipl2_data_rom_addr << NCD_ROM_ADDR_SHIFT );
}

// bncmpフォントデータ格納ROMアドレスの取得
u32 NCD_GetFontBncmpRomAddr( void )
{
	NCD_ReadSYSMHeader();
	return (u32)( ipl2Header.rom_addr1.font_bncmp_offset << FONT_ROM_ADDR_SHIFT );
}

// bnfrフォントデータ格納ROMアドレスの取得
u32 NCD_GetFontBnfrRomAddr( void )
{
	NCD_ReadSYSMHeader();
	return (u32)( ipl2Header.rom_addr2.font_bnfr_offset << FONT_ROM_ADDR_SHIFT );
}
#endif


//----------------------------------------------------------------------
// NITRO設定データのリード
//----------------------------------------------------------------------
int NVRAMm_ReadNitroConfigData(NitroConfigData *dstp)
{
	int			result = 0;
	NCDStoreEx	*ncdsp = &ncds[ 0 ];
	
	DC_InvalidateRange( ncdsp, sizeof(NCDStoreEx) * 2 );
	
	// フラッシュからニ重化されているNITRO設定データを読み出す。
	while( !NVRAMm_ExecuteCommand( COMM_RD, NCD_GetNCDRomAddr(),                       sizeof(NCDStoreEx), (u8 *)&ncdsp[0]) ) {}
	while( !NVRAMm_ExecuteCommand( COMM_RD, NCD_GetNCDRomAddr() + SPI_NVRAM_PAGE_SIZE, sizeof(NCDStoreEx), (u8 *)&ncdsp[1]) ) {}
	OS_TPrintf("NCD read addr=%08x\n", NCD_GetNCDRomAddr() );
	
	// 読み出したデータのどちらが有効かを判定する。
	if(NVRAMm_checkCorrectNCD(ncdsp)) {
		next_saveCount	= 1;
		result			= 1;
		goto END;													// 有効なデータがなければエラー終了。
	}
	next_saveCount = (u8)((ncdsp[ena_ncd_num].saveCount + 1) & SAVE_COUNT_MASK);
	
	// 有効なNITRO設定データをバッファに転送
	if(dstp!=NULL) {
		SVC_CpuCopy( (void *)&ncdsp[ ena_ncd_num ].ncd,    (void *)dstp, sizeof(NitroConfigData), 16);
		SVC_CpuCopy( (void *)&ncdsp[ ena_ncd_num ].ncd_ex, (void *)&ncdEx, sizeof(NitroConfigDataEx), 16);
	}
	
END:
	return result;
}


//----------------------------------------------------------------------
// NITRO設定データのライト
//----------------------------------------------------------------------
void NVRAMm_WriteNitroConfigData( NitroConfigData *srcp )
{
	NCDStoreEx *ncdsp = &ncds[ 0 ];
	u16			size  = sizeof(NCDStore);
	u32			flash_addr;
	int			retry;
	
	// まだNITRO設定データがリードされていなければ、リードを行って必要な情報を取得する。
	if( ena_ncd_num == NCD_NOT_CORRECT ) {
		if( NVRAMm_ReadNitroConfigData( NULL ) ) {
			ena_ncd_num = 0;										// 有効なデータがなければ"0"側にライトする。
		}
	}
	
	// NCD   のCRC、セーブカウント値、ライトアドレスの算出。
	ncdsp->ncd				= *srcp;								// *GetNCDWork();　でも一緒やん。
	ncdsp->ncd.version		= NITRO_CONFIG_DATA_VERSION;			// バージョンを現在のものに設定。
	ncdsp->crc16	 		= SVC_GetCRC16( 0xffff, (const void *)&ncdsp->ncd,    sizeof(NitroConfigData) );
	ncdsp->saveCount 		= next_saveCount;
	next_saveCount	 		= (u8)( ( next_saveCount + 1 ) & SAVE_COUNT_MASK );
	
	// NCD_EXのCRC算出。
#ifndef NCD_EX_FORCE_ENABLE
	if( ( NCD_GetSYSMType() != SYSM_TYPE_NTR_WW ) && ( NCD_GetSYSMType() & SYSM_TYPE_NCD_EX_FLAG ) )
#endif
	{
		ncdsp->ncd_ex			= *GetNCDExWork();
		ncdsp->ncd_ex.version	= NITRO_CONFIG_DATA_EX_VERSION;		// バージョンを現在のものに設定。
		ncdsp->crc16_ex		 	= SVC_GetCRC16( 0xffff, (const void *)&ncdsp->ncd_ex, sizeof(NitroConfigDataEx) );
		size					= sizeof(NCDStoreEx);				// ※書き込みサイズをNCDStoreExに拡張。
	}
	
	// NITRO設定データのライト
	DC_FlushRange(ncdsp, sizeof(NCDStoreEx));
	retry = NVRAM_RETRY_NUM;
	while( retry-- ) {
		ena_ncd_num	   ^= 0x01;										// リトライの度に書き込みアドレスを切り替える。
		flash_addr		= NCD_GetNCDRomAddr() + ena_ncd_num * SPI_NVRAM_PAGE_SIZE;
		OS_TPrintf("NCD write addr=%08x\n", flash_addr );
		
		if( NVRAMm_ExecuteCommand( COMM_WE, flash_addr, size, (u8 *)ncdsp) ) {
			OS_TPrintf("NVRAM Write succeeded.\n");
			break;
		}
		SVC_WaitByLoop( 0x4000 );
		OS_TPrintf("NVRAM Write retry = %d.\n", NVRAM_RETRY_NUM - retry );
	}
}


//----------------------------------------------------------------------
// ミラーリングされているNITRO設定データのどちらが有効かを判定
//----------------------------------------------------------------------

static int NVRAMm_checkCorrectNCD(NCDStoreEx *ncdsp)
{
	u16 i;
	u16 ncd_valid = 0;
	
	// 各ミラーデータのCRC & saveCount正当性チェック
	for(i = 0; i < 2; i++) {
		u16  crc;
		BOOL invalid = FALSE;
		
		crc = SVC_GetCRC16( 0xffff, (const void *)&ncdsp[i].ncd, sizeof(NitroConfigData) );
		
		if(    ( ncdsp[ i ].crc16          != crc )				// CRCが正しく、saveCount値が0x80以下で、かつバージョンが一致するデータを正当と判断。
			|| ( ncdsp[ i ].ncd.version    != NITRO_CONFIG_DATA_VERSION )
			|| ( ncdsp[ i ].saveCount      >= SAVE_COUNT_MAX ) ) {
			OS_TPrintf("NCD   crc error.\n");
			invalid = TRUE;
		}
		
		// NCDExが有効なSYSMTypeならば、NCDExのCRCチェックを行う。
#ifndef NCD_EX_FORCE_ENABLE
		if( ( NCD_GetSYSMType() != SYSM_TYPE_NTR_WW ) && ( NCD_GetSYSMType() & SYSM_TYPE_NCD_EX_FLAG ) )
#endif
		{
			crc = SVC_GetCRC16( 0xffff, (const void *)&ncdsp[i].ncd_ex, sizeof(NitroConfigDataEx) );
			
			if(   ( ncdsp[ i ].crc16_ex       != crc )
			   || ( ncdsp[ i ].ncd_ex.version != NITRO_CONFIG_DATA_EX_VERSION ) ) {
				OS_TPrintf("NCDEx crc error.\n");
				invalid = TRUE;
			}
		}
		// NCD, NCDExのCRCが正しいなら、データの中身をチェック。
		if( !invalid ) {
			if( NCD_CheckDataValue( &ncdsp[ i ] ) ) {
				ncd_valid  |= 0x01 << i;						// データがおかしい値でないかもチェック。
				ena_ncd_num = i;								// 有効なNCDのインデックスを切り替える。
			}else {
				invalid = TRUE;
			}
		}
		
		if( ncd_valid & ( 0x01 << i ) ) {
			OS_TPrintf("NCD mirror%d is valid.:saveCount = %d\n", i, ncdsp[i].saveCount);
		}else {
			OS_TPrintf("NCD mirror%d is invalid.\n", i);
		}
	}
	
	
	if( ncd_valid == 0 ) {
		return 1;
	}else if( ncd_valid == 0x03 ) {									
	// ミラーリングされたNCDが両方ともに正当な場合、セーブカウント値が大きい方を有効とする。
		u16 saveCount = (u8)( ( ncdsp[ 0 ].saveCount + 1 ) & SAVE_COUNT_MASK );
		if( saveCount != ncdsp[ 1 ].saveCount ) {
			ena_ncd_num = 0;
		}
	}
	
	OS_TPrintf("use NCD mirror%d.:saveCount = %d\n", ena_ncd_num, ncdsp[ena_ncd_num].saveCount);
	
	return 0;
}


// NITRO設定データの値が正しい値かチェック。	// FALSE:正しくない。TRUE：正しい。
static BOOL NCD_CheckDataValue( NCDStoreEx *ncdsp )
{
	NitroConfigData   *ncdp   = &ncdsp->ncd;
	NitroConfigDataEx *ncdexp = &ncdsp->ncd_ex;
	
	//ncdp->option;
	// NCDのlanguageチェック
	if( ~( LANG_BITMAP_WW & VALID_LANG_BITMAP ) & ( 0x0001 << ncdp->option.language ) ) {
		OS_TPrintf("NCD: invalid language        : org:%02d ex:%02d bitmap:%04x\n",
				   ncdp->option.language, ncdexp->language, ncdexp->valid_language_bitmap );
		return FALSE;
	}
	// NCDExのlanguageチェック（NCDExが有効なのは、下記のSYSMタイプのもの）
#ifndef NCD_EX_FORCE_ENABLE
	if( ( NCD_GetSYSMType() != SYSM_TYPE_NTR_WW ) && ( NCD_GetSYSMType() & SYSM_TYPE_NCD_EX_FLAG ) )
#endif
	{
		if(   ( ~VALID_LANG_BITMAP & ( 0x0001 << ncdexp->language ) )
		   || ( ncdexp->valid_language_bitmap != VALID_LANG_BITMAP  ) ) {
			
			OS_TPrintf("NCDEx: invalid language    : org:%02d ex:%02d bitmap:%04x\n",
					   ncdp->option.language, ncdexp->language, ncdexp->valid_language_bitmap );
			return FALSE;
		}
	}
	
	//ncdp->owner;
	// favoriteColorは4bitなので範囲外はない。
	// birthday
	if( ncdp->option.input_birthday ) {
		if( ( ncdp->owner.birthday.month > 12 ) || ( ncdp->owner.birthday.day > 31 ) ) {
			OS_TPrintf("NCD: invalid birthday        : %02d/%02d\n", ncdp->owner.birthday.month, ncdp->owner.birthday.day );
			return FALSE;
		}
	}
	
	// nickname
	if( ncdp->option.input_nickname ) {
		if( ncdp->owner.nickname.length > NCD_NICKNAME_LENGTH ) {
			OS_TPrintf("NCD: invalid nickname length : %02d\n", ncdp->owner.nickname.length );
			return FALSE;
		}
	}
	
	// comment
	if( ncdp->owner.comment.length  > NCD_COMMENT_LENGTH ) {
		OS_TPrintf("NCD: invalid comment  length     : %02d\n", ncdp->owner.comment.length );
		return FALSE;
	}
	
	//ncdp->alarm;
	if( ( ncdp->alarm.hour > 23 ) || ( ncdp->alarm.minute > 59 ) ) {
		OS_TPrintf("NCD: invalid alarm time          : %02d:%02d\n", ncdp->alarm.hour, ncdp->alarm.minute );
		return FALSE;
	}
	
	//ncdp->tp;
	// TPキャリブレーション値は、TP_CalcCalibrateParamで値のチェックをしているので、チェックしない。
	
	OS_TPrintf( "NCD: correct data.\n" );
	return TRUE;
}


//----------------------------------------------------------------------
// NVRAMへのアクセスルーチン本体 ( nv_state <- COMM_RD or COMM_WE )
//----------------------------------------------------------------------
static BOOL NVRAMm_ExecuteCommand( int nv_state, u32 addr, u16 size, u8 *srcp )
{
    OSTick	start;
	BOOL	nv_sending	 = FALSE;
	u8		*nvram_srp	 = (u8 *)&ncds[1];
	
	PXI_SetFifoRecvCallback( PXI_FIFO_TAG_NVRAM , Callback_NVRAM );
	
	while( 1 ) {
		//---------------------------------------
		// NVRAMコマンドを発行する
		//---------------------------------------
		if( !nv_sending ) {
			
			nv_cb_occurred	= FALSE;
			
			switch( nv_state ) {
			  case COMM_RD:
				nv_sending	= SPI_NvramReadDataBytes( addr, size, srcp );
				break;
				
			  case COMM_WE:
				nv_sending	= SPI_NvramWriteEnable();
				break;
				
			  case COMM_WR:
				nv_sending	= SPI_NvramPageWrite( addr, size , srcp );
				start		= OS_GetTick();
				break;
				
			  case COMM_RDSR_WE:
			  case COMM_RDSR_WR:
				nv_sending	= SPI_NvramReadStatusRegister( nvram_srp );
				break;
				
			  case COMM_SRST:
				nv_sending	= SPI_NvramSoftwareReset();
				break;
			}
		//---------------------------------------
		// コマンド実行結果（コールバック発生）を待って結果を処理する
		//---------------------------------------
		}else { // nv_sending == TRUE
			if( nv_cb_occurred == TRUE ) {							// コールバック発生を待つ。
				
				nv_sending = FALSE;
				
				if( nv_result == SPI_PXI_RESULT_SUCCESS ) {
					switch( nv_state ) {
					  case COMM_RD:
						return TRUE;
						
					  case COMM_WE:
						nv_state = COMM_RDSR_WE;
						break;
						
					  case COMM_WR:
						nv_state = COMM_RDSR_WR;
						break;
						
					  case COMM_RDSR_WE:
					  case COMM_RDSR_WR:
						
						DC_InvalidateRange( nvram_srp, 1 );
						
						if( nv_state == COMM_RDSR_WE ) {				// ライトイネーブル確認ステートなら
							if( ( *nvram_srp & SR_WEN ) ) {
								nv_state = COMM_WR;
							}else {
								OS_TPrintf("NVRAM ERR: Write Enable Invalid.\n");
								return FALSE;
							}
						}else {
							if( ( *nvram_srp & SR_WIP ) == 0 ) {		// ライト／イレース終了
								return TRUE;
							}else {
								if(	  ( *nvram_srp & SR_EER )			// SR_EERが立っていたらエラー
								   || ( OS_TicksToMilliSeconds( OS_GetTick() - start ) > 4000 ) ) {
																		// コマンド発行から4秒経過したらエラー（※保険）
									OS_TPrintf( "NVRAM SR : %02x\n", *nvram_srp );
									nv_state = COMM_SRST;
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
				}else {  // nv_result != SPI_PXI_RESULT_SUCCESS
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
	
	nv_result		= (u16)( data & 0x00ff );
	nv_cb_occurred	= TRUE;											// コールバック発生フラグTRUE
	
	if( err ) {
		OS_TPrintf("NVRAM-ARM9: Received PXI data is error.\n");
		nv_result = 0x00ff;
	}
	
	switch(command){												// コマンド名表示
	  case SPI_PXI_COMMAND_NVRAM_READ:
		OS_TPrintf("NVRAM-ARM9:ReadDataBytes");
		break;
	  case SPI_PXI_COMMAND_NVRAM_WREN:
		OS_TPrintf("NVRAM-ARM9:WriteEnable");
		break;
	  case SPI_PXI_COMMAND_NVRAM_PW:
		OS_TPrintf("NVRAM-ARM9:PageWrite");
		break;
	  case SPI_PXI_COMMAND_NVRAM_RDSR:
		OS_TPrintf("NVRAM-ARM9:ReadStatusRegister");
		break;
	  case SPI_PXI_COMMAND_NVRAM_WRDI:
		OS_TPrintf("NVRAM-ARM9:WriteDisable");
		break;
	  case SPI_PXI_COMMAND_NVRAM_PE:
		OS_TPrintf("NVRAM-ARM9:PageErase");
		break;
	  case SPI_PXI_COMMAND_NVRAM_SR:
		OS_TPrintf("NVRAM-ARM9:SoftwareReset");
		break;
	  default:
		OS_TPrintf("NVRAM-ARM9:?????");
		break;
	}
	if( nv_result != SPI_PXI_RESULT_SUCCESS ) {
		OS_TPrintf(" Error! ->%x", nv_result );
	}
	OS_TPrintf("\n");
}

