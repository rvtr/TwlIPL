// srl.h のクラス実装

#include "stdafx.h"
#include "common.h"
#include "srl.h"
#include "utility.h"
#include "keys.h"
#include <acsign/include/acsign.h>
#include <format_rom_private.h>
#include <cstring>
#include <cstdio>

using namespace MasterEditorTWL;

//
// RCSrl クラス
//

// constructor
RCSrl::RCSrl()
{
	this->pRomHeader = new (ROM_Header);
	std::memset( pRomHeader, 0, sizeof(ROM_Header) );

	this->hMrcSpecialList = gcnew RCMrcSpecialList();
}

// destructor
RCSrl::~RCSrl()
{
	// ポインタは(たぶん) unmanaged なので自主的に解放する
	delete (this->pRomHeader);
}

//
// ROMヘッダをファイルからリード
//
// @arg [in] 入力ファイル名
//
ECSrlResult RCSrl::readFromFile( System::String ^filename )
{
	FILE       *fp = NULL;
	const char *pchFilename = 
		(const char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi( filename ).ToPointer();

	// ファイルを開いてROMヘッダのみ読み出す
	if( fopen_s( &fp, pchFilename, "rb" ) != NULL )
	{
		return (ECSrlResult::ERROR_FILE_OPEN);
	}
	(void)fseek( fp, 0, SEEK_SET );		// ROMヘッダはsrlの先頭から

	// 1バイトをsizeof(~)だけリード (逆だと返り値がsizeof(~)にならないので注意)
	if( fread( (void*)(this->pRomHeader), 1, sizeof(ROM_Header), fp ) != sizeof(ROM_Header) )
	{
		return (ECSrlResult::ERROR_FILE_READ);
	}
	{
		ECSrlResult      r;

#ifdef METWL_WHETHER_SIGN_DECRYPT
		// 署名チェック
		r = this->decryptRomHeader();
		if( r != ECSrlResult::NOERROR )
		{
			(void)fclose(fp);
			return r;
		}
#endif //#ifdef METWL_WHETHER_SIGN_DECRYPT

		// ファイルを閉じる前にROMヘッダ以外の領域から設定を取り出す
		(void)this->hasDSDLPlaySign( fp );
		r = this->searchSDKVersion( fp );
		if( r != ECSrlResult::NOERROR )
		{
			(void)fclose(fp);
			return r;
		}
		r = this->searchLicenses( fp );
		if( r != ECSrlResult::NOERROR )
		{
			(void)fclose(fp);
			return r;
		}
	}

	// ROMヘッダの値をROM固有情報フィールドに反映させる
	(void)this->setRomInfo();

	// すべて設定したあとにMRC
	{
		ECSrlResult r;
		r = this->mrc( fp );
		if( r != ECSrlResult::NOERROR )
		{
			(void)fclose(fp);
			return r;
		}
	}
	(void)fclose( fp );
	return (ECSrlResult::NOERROR);
}

//
// ROMヘッダをファイルにライト
//
// @arg [in] 出力ファイル名
//
ECSrlResult RCSrl::writeToFile( System::String ^filename )
{
	FILE       *fp = NULL;
	const char *pchFilename = 
		(const char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi( filename ).ToPointer();
	ECSrlResult result;

	// ROMヘッダにROM固有情報を反映させる
	result = this->setRomHeader();
	if( result != ECSrlResult::NOERROR )
	{
		return result;
	}

	// ファイルにROMヘッダをライト
	if( fopen_s( &fp, pchFilename, "r+b" ) != NULL )	// 上書き・バイナリ
	{
		return (ECSrlResult::ERROR_FILE_OPEN);
	}
	(void)fseek( fp, 0, SEEK_SET );

	if( fwrite( (const void*)(this->pRomHeader), 1, sizeof(ROM_Header), fp ) != sizeof(ROM_Header) )
	{
		return (ECSrlResult::ERROR_FILE_WRITE);
	}
	(void)fclose( fp );

	return (ECSrlResult::NOERROR);
}

//
// ROMヘッダから取得したROM固有情報をフィールドに反映させる
//
ECSrlResult RCSrl::setRomInfo(void)
{
	System::Int32 i;
	System::Text::UTF8Encoding^ utf8 = gcnew System::Text::UTF8Encoding( true );	// char->String変換に必要

	// NTR互換情報
	this->hTitleName  = gcnew System::String( this->pRomHeader->s.title_name, 0, TITLE_NAME_MAX, utf8 );
	this->hGameCode   = gcnew System::String( this->pRomHeader->s.game_code,  0, GAME_CODE_MAX,  utf8 );
	this->hMakerCode  = gcnew System::String( this->pRomHeader->s.maker_code, 0, MAKER_CODE_MAX, utf8 );
	switch( this->pRomHeader->s.platform_code )
	{
		case PLATFORM_CODE_NTR:         this->hPlatform = gcnew System::String( "NTR Limited" );    break;
		case PLATFORM_CODE_TWL_HYBLID:  this->hPlatform = gcnew System::String( "NTR/TWL Hybrid" ); break;
		case PLATFORM_CODE_TWL_LIMITED: this->hPlatform = gcnew System::String( "TWL Limited" );    break;
		default:
			this->hPlatform = nullptr;
		break;
	}
	this->hRomSize = MasterEditorTWL::transRomSizeToString( this->pRomHeader->s.rom_size );
	//this->hForKorea   = gcnew System::Byte( this->pRomHeader->s.for_korea );
	//this->hForChina   = gcnew System::Byte( this->pRomHeader->s.for_china );
	this->hRomVersion = gcnew System::Byte( this->pRomHeader->s.rom_version );
	this->hHeaderCRC  = gcnew System::UInt16( this->pRomHeader->s.header_crc16 );

	switch( this->pRomHeader->s.game_cmd_param & CARD_LATENCY_MASK )
	{
		case CARD_MROM_GAME_LATENCY:
			this->hLatency = "MROM";
		break;

		case CARD_1TROM_GAME_LATENCY:
			this->hLatency = "1TROM";
		break;

		default:
			this->hLatency = "Illegal";
		break;
	}

	// ペアレンタルコントロール
	this->hArrayParentalRating = gcnew cli::array<System::Byte ^>(PARENTAL_CONTROL_INFO_SIZE);
	this->hArrayParentalEffect = gcnew cli::array<System::Boolean ^>(PARENTAL_CONTROL_INFO_SIZE);
	this->hArrayParentalAlways = gcnew cli::array<System::Boolean ^>(PARENTAL_CONTROL_INFO_SIZE);
	for( i=0; i < PARENTAL_CONTROL_INFO_SIZE; i++ )
	{
		System::Byte rating    = this->pRomHeader->s.parental_control_rating_info[i] & OS_TWL_PCTL_OGNINFO_AGE_MASK;
		System::Boolean effect = ((this->pRomHeader->s.parental_control_rating_info[i] & OS_TWL_PCTL_OGNINFO_ENABLE_MASK) != 0)?true:false;
		System::Boolean force  = ((this->pRomHeader->s.parental_control_rating_info[i] & OS_TWL_PCTL_OGNINFO_ALWAYS_MASK) != 0)?true:false;
		
		this->hArrayParentalRating[i] = gcnew System::Byte( rating );
		this->hArrayParentalEffect[i] = gcnew System::Boolean( effect );
		this->hArrayParentalAlways[i] = gcnew System::Boolean( force );
	}

	// TWL専用情報
	this->hIsNormalJump = gcnew System::Boolean( (this->pRomHeader->s.permit_landing_normal_jump != 0)?true:false );
	this->hIsTmpJump    = gcnew System::Boolean( (this->pRomHeader->s.permit_landing_tmp_jump    != 0)?true:false );
	this->hNormalRomOffset   = gcnew System::UInt32( (u32)(this->pRomHeader->s.twl_card_normal_area_rom_offset)   * 0x80000 );
	this->hKeyTableRomOffset = gcnew System::UInt32( (u32)(this->pRomHeader->s.twl_card_keytable_area_rom_offset) * 0x80000 );
	this->hEULAVersion = gcnew System::Byte( this->pRomHeader->s.agree_EULA_version );
	this->hPublicSize  = gcnew System::UInt32( this->pRomHeader->s.public_save_data_size );
	this->hPrivateSize = gcnew System::UInt32( this->pRomHeader->s.private_save_data_size );

	u8  *idL = this->pRomHeader->s.titleID_Lo;
	u32  idH = this->pRomHeader->s.titleID_Hi;
	//u32  val;
	//val = ((u32)(idL[0]) << 24) | ((u32)(idL[1]) << 16) | ((u32)(idL[2]) << 8) | ((u32)(idL[3]));	// ビッグエンディアン
	//this->hTitleIDLo   = gcnew System::UInt32( val );
	this->hTitleIDLo   = gcnew System::String( (char*)idL, 0, 4, utf8 );
	this->hTitleIDHi   = gcnew System::UInt32( idH );

	// TitleIDからわかる情報
	this->hIsAppLauncher = gcnew System::Boolean( false );
	this->hIsAppUser     = gcnew System::Boolean( false );
	this->hIsAppSystem   = gcnew System::Boolean( false );
	this->hIsAppSecure   = gcnew System::Boolean( false );
	this->hIsLaunch      = gcnew System::Boolean( false );
	this->hIsMediaNand   = gcnew System::Boolean( false );
	this->hIsDataOnly    = gcnew System::Boolean( false );
	if( (idL[3]=='H') && (idL[2]=='N') && (idL[1]=='A') )	// ランチャアプリかどうかはTitleID_Loの値で決定
    {
		*(this->hIsAppLauncher) = true;
    }
    else if( idH & TITLE_ID_HI_SECURE_FLAG_MASK )				// 各ビットは排他的とは限らないのでelse ifにはならない
    {
		*(this->hIsAppSecure) = true;
    }
    else if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 1 )
    {
		*(this->hIsAppSystem) = true;
    }
    else if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 0 )
    {
		*(this->hIsAppUser) = true;
    }
	if( idH & TITLE_ID_HI_DATA_ONLY_FLAG_MASK )
	{
		*(this->hIsDataOnly) = true;
	}
	if( idH & TITLE_ID_HI_MEDIA_MASK )
	{
		*(this->hIsMediaNand) = true;
	}
	if( (idH & TITLE_ID_HI_NOT_LAUNCH_FLAG_MASK) == 0 )		// ビットが0のときLaunch
	{
		*(this->hIsLaunch) = true;
	}
	u16 pub = (u16)((idH & TITLE_ID_HI_PUBLISHER_CODE_MASK) >> TITLE_ID_HI_PUBLISHER_CODE_SHIFT);
	this->hPublisherCode = gcnew System::UInt16( pub );

	// TWL拡張フラグ
	this->hIsCodecTWL  = gcnew System::Boolean( (this->pRomHeader->s.exFlags.codec_mode != 0)?true:false );
	this->hIsEULA      = gcnew System::Boolean( (this->pRomHeader->s.exFlags.agree_EULA != 0)?true:false );
	this->hIsSubBanner = gcnew System::Boolean( (this->pRomHeader->s.exFlags.availableSubBannerFile != 0)?true:false );
	this->hIsWiFiIcon  = gcnew System::Boolean( (this->pRomHeader->s.exFlags.WiFiConnectionIcon != 0)?true:false );
	this->hIsWirelessIcon = gcnew System::Boolean( (this->pRomHeader->s.exFlags.DSWirelessIcon != 0)?true:false );
	this->hIsWL        = gcnew System::Boolean( (this->pRomHeader->s.exFlags.enable_nitro_whitelist_signature != 0)?true:false );

	// TWLアクセスコントロール
	this->hIsCommonClientKey = gcnew System::Boolean( (this->pRomHeader->s.access_control.common_client_key != 0)?true:false );
	this->hIsAesSlotBForES   = gcnew System::Boolean( (this->pRomHeader->s.access_control.hw_aes_slot_B != 0)?true:false );
	this->hIsAesSlotCForNAM  = gcnew System::Boolean( (this->pRomHeader->s.access_control.hw_aes_slot_C != 0)?true:false );
	this->hIsSD              = gcnew System::Boolean( (this->pRomHeader->s.access_control.sd_card_access != 0)?true:false );
	this->hIsNAND            = gcnew System::Boolean( (this->pRomHeader->s.access_control.nand_access != 0)?true:false );
	this->hIsGameCardOn      = gcnew System::Boolean( (this->pRomHeader->s.access_control.game_card_on != 0)?true:false );
	this->hIsShared2         = gcnew System::Boolean( (this->pRomHeader->s.access_control.shared2_file != 0)?true:false );
	this->hIsAesSlotBForJpegEnc = gcnew System::Boolean( (this->pRomHeader->s.access_control.hw_aes_slot_B_SignJPEGForLauncher != 0)?true:false );
	this->hIsGameCardNitro   = gcnew System::Boolean( (this->pRomHeader->s.access_control.game_card_nitro_mode != 0)?true:false );
	this->hIsAesSlotAForSSL  = gcnew System::Boolean( (this->pRomHeader->s.access_control.hw_aes_slot_A_SSLClientCert != 0)?true:false );
	this->hIsCommonClientKeyForDebugger 
		= gcnew System::Boolean( (this->pRomHeader->s.access_control.common_client_key_for_debugger_sysmenu != 0)?true:false );

	// Shared2ファイルサイズ
	this->hShared2SizeArray = gcnew cli::array<System::UInt32^>(METWL_NUMOF_SHARED2FILES);
	for( i=0; i < METWL_NUMOF_SHARED2FILES; i++ )
	{
		this->hShared2SizeArray[i] = gcnew System::UInt32( 0 );
	}
	u32  unit = 16 * 1024;		// 16KBの乗数が格納されている
	if( this->pRomHeader->s.shared2_file0_size != 0 )
	{
		*(this->hShared2SizeArray[0]) = (this->pRomHeader->s.shared2_file0_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file1_size != 0 )
	{
		*(this->hShared2SizeArray[1]) = (this->pRomHeader->s.shared2_file1_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file2_size != 0 )
	{
		*(this->hShared2SizeArray[2]) = (this->pRomHeader->s.shared2_file2_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file3_size != 0 )
	{
		*(this->hShared2SizeArray[3]) = (this->pRomHeader->s.shared2_file3_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file4_size != 0 )
	{
		*(this->hShared2SizeArray[4]) = (this->pRomHeader->s.shared2_file4_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file5_size != 0 )
	{
		*(this->hShared2SizeArray[5]) = (this->pRomHeader->s.shared2_file5_size * unit) + unit;
	}

	// カードリージョン
	const u32  map           = this->pRomHeader->s.card_region_bitmap;
	this->hIsRegionJapan     = gcnew System::Boolean( ((map & METWL_MASK_REGION_JAPAN)     != 0)?true:false );
	this->hIsRegionAmerica   = gcnew System::Boolean( ((map & METWL_MASK_REGION_AMERICA)   != 0)?true:false );
	this->hIsRegionEurope    = gcnew System::Boolean( ((map & METWL_MASK_REGION_EUROPE)    != 0)?true:false );
	this->hIsRegionAustralia = gcnew System::Boolean( ((map & METWL_MASK_REGION_AUSTRALIA) != 0)?true:false );

	return ECSrlResult::NOERROR;
} // ECSrlResult RCSrl::setRomInfo(void)

//
// ROMヘッダにROM固有情報フィールドの値を反映させる
//
ECSrlResult RCSrl::setRomHeader(void)
{
	System::Int32 i;
	ECSrlResult result;

	// ROMヘッダの[0,0x160)の領域はRead Onlyで変更しない

	// いくつかのフラグをROMヘッダに反映
	this->pRomHeader->s.exFlags.agree_EULA = (*(this->hIsEULA) == true)?1:0;
	this->pRomHeader->s.agree_EULA_version = *(this->hEULAVersion);
	this->pRomHeader->s.exFlags.WiFiConnectionIcon = (*(this->hIsWiFiIcon) == true)?1:0;
	this->pRomHeader->s.exFlags.DSWirelessIcon     = (*(this->hIsWirelessIcon) == true)?1:0;

	// レーティング
	u32  map = 0;
	if( *(this->hIsRegionJapan)   == true )  { map |= METWL_MASK_REGION_JAPAN; }
	if( *(this->hIsRegionAmerica) == true )  { map |= METWL_MASK_REGION_AMERICA; }
	if( *(this->hIsRegionEurope)  == true )  { map |= METWL_MASK_REGION_EUROPE; }
	if( *(this->hIsRegionAustralia) == true ){ map |= METWL_MASK_REGION_AUSTRALIA; }
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	if( (*(this->hIsRegionJapan) == true ) && (*(this->hIsRegionAmerica) == true)
		&& (*(this->hIsRegionEurope) == true ) && (*(this->hIsRegionAustralia) == true)
	  )
	{
		map |= METWL_MASK_REGION_ALL;	// オールリージョンを許す
	}
#endif
	this->pRomHeader->s.card_region_bitmap = map;

	// ペアレンタルコントロール
	for( i=0; i < PARENTAL_CONTROL_INFO_SIZE; i++ )
	{
		System::Byte  effect = 0;
		System::Byte  force  = 0;
		if( this->hArrayParentalEffect[i]->Equals(true) )
		{
			effect = OS_TWL_PCTL_OGNINFO_ENABLE_MASK;
		}
		if( this->hArrayParentalAlways[i]->Equals(true) )
		{
			force  = OS_TWL_PCTL_OGNINFO_ALWAYS_MASK;
		}

		this->pRomHeader->s.parental_control_rating_info[i] = effect | force | *(this->hArrayParentalRating[i]);
	}

	// ROMヘッダのCRCと署名を更新する
	result = this->calcRomHeaderCRC();
	if( result != ECSrlResult::NOERROR )
	{
		return result;
	}
	result = this->signRomHeader();
	if( result != ECSrlResult::NOERROR )
	{
		return result;
	}

	return ECSrlResult::NOERROR;
} // ECSrlResult RCSrl::setRomHeader(void)

//
// ヘッダのCRCを算出
//
static u16 calcCRC( const u16 start, const u8 *data, const int allsize )
{
	u16       r1;
	u16       total = start;
	int       size  = allsize;

	// CRCテーブルは固定
	const u16 CRC16Table[16] =
    {
		0x0000, 0xCC01, 0xD801, 0x1400,
		0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401,
		0x5000, 0x9C01, 0x8801, 0x4400
    };

	// CRC計算
	while (size-- > 0)
    {
        // 下位4bit
        r1 = CRC16Table[total & 0xf];
        total = (total >> 4) & 0x0fff;
        total = total ^ r1 ^ CRC16Table[*data & 0xf];

        // 上位4bit
        r1 = CRC16Table[total & 0xf];
        total = (total >> 4) & 0x0fff;
        total = total ^ r1 ^ CRC16Table[(*data >> 4) & 0xf];

        data++;
    }
	return total;
} // calcRomHeaderCRCCore()

ECSrlResult RCSrl::calcRomHeaderCRC(void)
{
	// ROMヘッダのCRC領域を上書き
	this->pRomHeader->s.header_crc16 = calcCRC( CRC16_INIT_VALUE, (u8*)this->pRomHeader, CALC_CRC16_SIZE );

	return (ECSrlResult::NOERROR);
} // ECSrlResult RCSrl::calcRomHeaderCRC(void)

//
// ROMヘッダを再署名
//
ECSrlResult RCSrl::signRomHeader(void)
{
	SignatureData   signSrc;						// 署名のもととなるダイジェスト値
	u8              signDst[ RSA_KEY_LENGTH ];		// 署名の格納先Tmpバッファ
	u8              decryptBlock[ RSA_KEY_LENGTH ];	// 署名を解除後ブロックバッファ
	BOOL            result;
	ROM_Header      rh;
	int             pos;

	// ROMヘッダのダイジェストを算出(先頭から証明書領域の直前までが対象)
	ACSign_DigestUnit(
		signSrc.digest,
		this->pRomHeader,
		(u32)&(rh.certificate) - (u32)&(rh)		// this->pRomHeader はマネージヒープ上にあるので実アドレスを取得できない
	);

	// ダイジェストに署名をかける
	result = ACSign_Encrypto( signDst, g_devPrivKey_DER, &signSrc, sizeof(SignatureData) ); 
	if( !result )
	{
		return (ECSrlResult::ERROR_SIGN_ENCRYPT);
	}

	// 署名を解除してダイジェストと一致するかベリファイする
	result = ACSign_Decrypto( decryptBlock, g_devPubKey_DER, signDst, RSA_KEY_LENGTH );
	for( pos=0; pos < RSA_KEY_LENGTH; pos++ )
	{
		if( decryptBlock[pos] == 0x0 )			// 解除後ブロックから実データをサーチ
			break;
	}
	if( !result || (memcmp( &signSrc, &(decryptBlock[pos+1]), sizeof(SignatureData) ) != 0) )
	{
		return (ECSrlResult::ERROR_SIGN_DECRYPT);
	}

	// ROMヘッダに署名を上書き
	memcpy( this->pRomHeader->signature, signDst, RSA_KEY_LENGTH );

	return (ECSrlResult::NOERROR);
} // ECSrlResult RCSrl::signRomHeader(void)

//
// ROMヘッダの署名を外す
//
ECSrlResult RCSrl::decryptRomHeader(void)
{
	u8     original[ RSA_KEY_LENGTH ];	// 署名外した後のデータ格納先
	s32    pos = 0;						// ブロックの先頭アドレス
	u8     digest[ DIGEST_SIZE_SHA1 ];	// ROMヘッダのダイジェスト
	ROM_Header rh;

	// <データの流れ>
	// (1) 公開鍵で復号した結果(ブロック)をローカル変数(original)に格納
	// (2) ブロックから余分な部分を取り除いて引数(pDst)にコピー

	// 署名の解除 = 公開鍵で復号
	if( !ACSign_Decrypto( original, g_devPubKey_DER, this->pRomHeader->signature, RSA_KEY_LENGTH ) )
	{
		return ECSrlResult::ERROR_SIGN_DECRYPT;
	}
	// 署名前データを復号後ブロックからゲット
	for( pos=0; pos < (RSA_KEY_LENGTH-2); pos++ )   // 本来ブロックの先頭は0x00だが復号化の内部処理によって消える仕様
	{
		// 暗号ブロック形式 = 0x00, BlockType, Padding, 0x00, 実データ
		if( original[pos] == 0x00 )                               // 実データの直前の0x00をサーチ
		{
			break;
		}
	}
	// ベリファイ
	// ROMヘッダのダイジェストを算出(先頭から証明書領域の直前までが対象)
	ACSign_DigestUnit( digest,	this->pRomHeader, (u32)&(rh.certificate) - (u32)&(rh) );
		// this->pRomHeader はマネージヒープ上にあるので実アドレスを取得できない
	if( memcmp( &(original[pos+1]), digest, DIGEST_SIZE_SHA1 ) != 0 )
	{
		return ECSrlResult::ERROR_SIGN_VERIFY;
	}
	return (ECSrlResult::NOERROR);
}

//
// DSダウンロード署名がSRLに格納されているか調べる
//
ECSrlResult RCSrl::hasDSDLPlaySign( FILE *fp )
{
	const s32 DSDLPLAY_SIZE = 0x88;
	u8        buf[ DSDLPLAY_SIZE ];
	u32       offset;

	// 署名領域をSRLから読み込む
	offset = this->pRomHeader->s.rom_valid_size;
	if( fseek( fp, offset, SEEK_SET ) != 0 )
	{
		this->hHasDSDLPlaySign = gcnew System::Boolean( false );	// 署名はNTRのROM領域の最後に入るので
		return ECSrlResult::ERROR_FILE_READ;						// NTRのSRLに署名が格納されていない場合はシークもリードもできない
	}
	if( DSDLPLAY_SIZE != fread( buf, 1, DSDLPLAY_SIZE, fp ) )
	{
		this->hHasDSDLPlaySign = gcnew System::Boolean( false );
		return ECSrlResult::ERROR_FILE_READ;
	}

	// 最初の2バイトが固定の値となることが保証されているのでその値かどうかで判定
	if( (buf[0] == 'a') && (buf[1] == 'c') )
	{
		this->hHasDSDLPlaySign = gcnew System::Boolean( true );
	}
	else
	{
		this->hHasDSDLPlaySign = gcnew System::Boolean( false );
	}
	return (ECSrlResult::NOERROR);
}

//
// SDKバージョンを取得する
//
ECSrlResult RCSrl::searchSDKVersion( FILE *fp )
{
	// SDKバージョンはSRLバイナリ中に以下のマジックナンバとともに埋められている
	const u8  pattern[8] = {0x21, 0x06, 0xc0, 0xde, 0xde, 0xc0, 0x06, 0x21};
	System::Collections::Generic::List<u32> ^list;

	this->hSDKList = gcnew System::Collections::Generic::List<RCSDKVersion ^>;
	this->hSDKList->Clear();

	list = MasterEditorTWL::patternMatch( fp, pattern , 8 );
	if( list == nullptr )
	{
		//System::Diagnostics::Debug::WriteLine( "no list" );
		return ECSrlResult::ERROR_SDK;
	}
	for each( u32 item in list )
	{
		// マジックコードのオフセットの手前4バイトがSDKバージョン
		if( item >= 4 )
		{
			u32       offset;
			u32       sdkcode;

			offset = item - 4;
			fseek( fp, offset, SEEK_SET );
			if( 4 != fread( (void*)&sdkcode, 1, 4, fp ) )
			{
				return ECSrlResult::ERROR_SDK;
			}

			// 解読
			System::Byte   major = (System::Byte)(0xff & (sdkcode >> 24));
			System::Byte   minor = (System::Byte)(0xff & (sdkcode >> 16));
			System::UInt16 relstep = (System::UInt16)(0xffff & sdkcode);
			System::String ^str = nullptr;
			str += (major.ToString() + "." + minor.ToString() + " ");

			// RELSTEPの解釈
			//   PR1=10100 PR2=10200 ...
			//   RC1=20100 RC2=20200 ...
			//   RELEASE=30000
			System::UInt16 patch = relstep;
			while( patch >= 10000 )
			{
				patch -= 10000;
			}
			System::UInt16 rev = patch;
			System::String ^revstr = gcnew System::String( "" );
			while( rev >= 100 )
			{
				rev -= 100;
			}
			if( rev > 0 )
			{
				revstr = "-" + rev.ToString();
			}
			patch = patch / 100;
			switch( relstep / 10000 )
			{
				case 1: str += ("PR " + patch.ToString() + revstr); break;
				case 2: str += ("RC " + patch.ToString() + revstr); break;
				case 3: str += ("RELEASE " + patch.ToString() + revstr); break;
				default: break;
			}
			u32 statbegin = this->pRomHeader->s.main_rom_offset;
			u32 statend   = this->pRomHeader->s.main_rom_offset + this->pRomHeader->s.main_size - 1;
			System::Boolean isstat = ((statbegin <= offset) && (offset <= statend))?true:false;
			this->hSDKList->Add( gcnew RCSDKVersion(str, sdkcode, isstat) );
			//System::Diagnostics::Debug::WriteLine( "SDK " + str );
		}
	}
	return ECSrlResult::NOERROR;
}

//
// 使用ライセンスを取得する
//
#define  LICENSE_LEN_MAX   1024		// これよりもライセンスの文字列が長いとき正しく取得できない
ECSrlResult RCSrl::searchLicenses(FILE *fp)
{
	// ライセンスはSRLバイナリ中に[SDK+(配布元):(ライブラリ名)]のフォーマットで埋められている
	const u8 pattern[5] = { '[', 'S', 'D', 'K', '+' };
	System::Collections::Generic::List<u32> ^list;

	this->hLicenseList = gcnew System::Collections::Generic::List<RCLicense ^>;
	this->hLicenseList->Clear();
	
	fseek( fp, 0, SEEK_END );
	const u32 filesize = ftell( fp );

	list = MasterEditorTWL::patternMatch( fp, pattern, 5 );
	if( list == nullptr )
	{
		return ECSrlResult::NOERROR;	// ライセンスがない場合も存在するのでOKとする
	}
	for each( u32 item in list )
	{
		char  buf[ LICENSE_LEN_MAX + 1 ];	// '\0'の分だけ多めにとっておく
		u32   offset = item + 5;			// "[SDK+"の後からリードする
		u32   len = ((filesize - offset) < LICENSE_LEN_MAX)?(filesize - offset):LICENSE_LEN_MAX;

		// "(配布元):(ライブラリ名)]"を余分な部分を含めてひとまずバッファに格納
		fseek( fp, offset, SEEK_SET );
		if( len != fread( buf, 1, len, fp ) )
		{
			return ECSrlResult::ERROR;
		}
		buf[ len ] = '\0';	// 後のstrlenの成功を保証するため

		// "(配布元):(ライブラリ名)]"を取り出してから(配布元)と(ライブラリ名)に分割
		System::Text::UTF8Encoding^ utf8 = gcnew System::Text::UTF8Encoding( true );	// char->String変換に必要
		System::String ^str = gcnew System::String( buf, 0, strlen(buf), utf8 );		// 探索や分割はStringメソッドに用意されている
		str = (str->Split( ']' ))[0];							// ']'よりも前を取り出す
		cli::array<System::String^> ^spl = str->Split( ':' );	// (配布元):(ライブラリ名)を分割する

		if( spl->Length >= 2  )		// 念のため
		{
			this->hLicenseList->Add( gcnew RCLicense( spl[0], spl[1] ) );
			//System::Diagnostics::Debug::WriteLine( "license " + spl[0] + " " + spl[1] );
		}
	}
	return ECSrlResult::NOERROR;
} //RCSrl::searchLicenses

//
// MRC
//
ECSrlResult RCSrl::mrc( FILE *fp )
{
	this->hErrorList = gcnew System::Collections::Generic::List<RCMrcError^>;
	this->hWarnList  = gcnew System::Collections::Generic::List<RCMrcError^>;

	this->hErrorList->Clear();
	this->hWarnList->Clear();

#ifdef METWL_WHETHER_MRC
	ECSrlResult result;
	result = this->mrcNTR( fp );
	if( result != ECSrlResult::NOERROR )
	{
		return result;
	}
	result = this->mrcTWL( fp );
	if( result != ECSrlResult::NOERROR )
	{
		return result;
	}
#endif
	return ECSrlResult::NOERROR;
}

// NTR互換MRC
ECSrlResult RCSrl::mrcNTR( FILE *fp )
{
	System::Int32  i;
	System::Boolean result;

	// ROMヘッダのチェック

	// 文字コードチェック
	result = true;
	for( i=0; i < TITLE_NAME_MAX; i++ )
	{
		char c = this->pRomHeader->s.title_name[i];
		if( ((c < 0x20) || (0x5f < c)) && (c != 0x00) )
		{
			result = false;
		}
	}
	if( !result )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ソフトタイトル", 0x0, 0xb, "使用不可のASCIIコードが使用されています。",
			"Game Title", "Unusable ASCII code is used.", false, true ) );
	}

	result = true;
	for( i=0; i < GAME_CODE_MAX; i++ )
	{
		char c = this->pRomHeader->s.game_code[i];
		if( (c < 0x20) || (0x5f < c) )
		{
			result = false;
		}
	}
	if( !result )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"イニシャルコード", 0xc, 0xf, "使用不可のASCIIコードが使用されています。",
			"Game Code", "Unusable ASCII code is used.", false, true ) );
	}
	if( memcmp( this->pRomHeader->s.game_code, "NTRJ", GAME_CODE_MAX ) == 0 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"イニシャルコード", 0xc, 0xf, "SDKデフォルトのコード(NTRJ)が使用されています。",
			"Game Code", "SDK default code(NTRJ) is used.", false, true ) );
	}

	result = true;
	for( i=0; i < MAKER_CODE_MAX; i++ )
	{
		char c = this->pRomHeader->s.maker_code[i];
		if( ((c < 0x20) || (0x5f < c)) && (c != 0x00) )
		{
			result = false;
		}
	}
	if( !result )
	{
		this->hErrorList->Add( gcnew RCMrcError(
			"メーカーコード", 0x10, 0x11, "使用不可のASCIIコードが使用されています。",
			"Maker Code", "Unusable ASCII code is used.", false, true ) );
	}

	// 値チェック

	if( this->pRomHeader->s.rom_type != 0x00 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"デバイスタイプ", 0x13, 0x13, "不正な値です。00hを設定してください。",
			"Device Type", "Invalid data. Please set 00h.", false, true ) );
	}

	u8 romver = this->pRomHeader->s.rom_version;
	if( ((romver < 0x00) || (0x09 < romver)) && (romver != 0xE0) )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"リマスターバージョン", 0x1e, 0x1e, "不正な値です。正式版では01h-09hのいずれかの値、事前版ではE0hです。",
			"Release Ver.", "Invalid data. Please set either one of 01h-09h(Regular ver.), or E0h(Preliminary ver.)", false, true ) );
	}
	if( this->pRomHeader->s.banner_offset == 0 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"バナーオフセット", 0x68, 0x6b, "バナーデータが設定されていません。",
			"Banner Offset.", "Banner data is not set.", false, true ) );
	}
	if( this->pRomHeader->s.rom_valid_size == 0 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ROM実効サイズ", 0x80, 0x83, "値が設定されていません。",
			"ROM Valid Size.", "Data is not set.", false, true ) );
	}

	// CRC
	u16  crc;
	// セキュア領域
	{
		// セキュア領域はROMヘッダ外
		u8     *secures;
		s32     secure_size = SECURE_AREA_END - this->pRomHeader->s.main_rom_offset;
		if (secure_size > SECURE_AREA_SIZE)
		{
			secure_size = SECURE_AREA_SIZE;
		}
		if (secure_size <= 0)
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"セキュア領域CRC", 0x15e, 0x15f, "セキュア領域のアドレス指定が不正です。",
				"Secure Area CRC.", "Illegal address of secure area.", false, true ) );
		}
		secures = new u8[secure_size];      // never return if not allocated
		fseek( fp, (u32)this->pRomHeader->s.main_rom_offset, SEEK_SET );
		if( secure_size != fread( secures, 1, secure_size, fp ) )
		{
			delete []secures;
			return ECSrlResult::ERROR_FILE_READ;
		}
		crc = calcCRC(CRC16_INIT_VALUE, (u8 *)secures, secure_size);
		delete []secures;
		if( crc != this->pRomHeader->s.secure_area_crc16 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"セキュア領域CRC", 0x07c, 0x07d, "計算結果と一致しません。セキュア領域が改ざんされた可能性があります。",
				"Secure Area CRC.", "Calclated CRC is different from Registered one.", false, true ) );
		}
	}

	// ロゴ領域
	crc = 0xcf56;
	if( crc != this->pRomHeader->s.nintendo_logo_crc16 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"キャラクターデータCRC", 0x15c, 0x15d, "計算結果と一致しません。キャラクターデータが改ざんされた可能性があります。",
			"Charactor Data CRC.", "Calclated CRC is different from Registered one.", false, true ) );
	}

	// ヘッダCRC
	crc = calcCRC( CRC16_INIT_VALUE, (u8*)this->pRomHeader, CALC_CRC16_SIZE );
	if( crc != this->pRomHeader->s.header_crc16 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ROMヘッダCRC", 0x15e, 0x15f, "計算結果と一致しません。ROMヘッダが改ざんされた可能性があります。",
			"ROM Header CRC.", "Calclated CRC is different from Registered one.", false, true ) );
	}

	// 予約領域
	for( i=0; i < 8; i++ )
	{
		if( this->pRomHeader->s.ctrl_reserved_B[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"ROM情報予約領域", 0x078, 0x07f, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area for ROM Info.", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}
	for( i=0; i < 32; i++ )
	{
		if( this->pRomHeader->s.reserved_C[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"予約領域C", 0x078, 0x07f, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area C", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}

	// ROMヘッダ以外の領域のチェック (ファイルから適宜リードする)
	
	// システムコールライブラリ
	u8  syscall[32];
	u32 offset = this->pRomHeader->s.main_rom_offset;
	fseek( fp, offset, SEEK_SET );
	if( 32 != fread( syscall, 1, 32, fp ) )
	{
		return ECSrlResult::ERROR_FILE_READ;
	}
	for( i=0; i < 32; i++ )
	{
		if( syscall[i] != 0x00 )
			break;
	}
	if( i == 32 )	// 全部0
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"システムコールライブラリ", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "SDKデフォルトです。",
			"System-Call Library", "This Library is SDK default one.", false, true ) );
	}

	return ECSrlResult::NOERROR;
} // mrcNTR()

ECSrlResult RCSrl::mrcTWL( FILE *fp )
{
	System::Int32 i;

	// ROMヘッダのチェック (NTR互換領域)

#ifdef METWL_WHETHER_PLATFORM_CHECK
	// プラットフォームのチェック
	if( (this->pRomHeader->s.platform_code != PLATFORM_CODE_TWL_HYBLID) && 
		(this->pRomHeader->s.platform_code != PLATFORM_CODE_TWL_LIMITED) )
	{
		return ECSrlResult::ERROR_PLATFORM;
	}
#endif

	// 値チェック

	fseek( fp, 0, SEEK_END );
	u32  filesize = ftell(fp);	// 実ファイルサイズ(単位Mbit)
	u32  romsize = 1 << (this->pRomHeader->s.rom_size);	// ROM容量
	if( *(this->hIsMediaNand) == false )		// カードアプリのときのみのチェック
	{
		if( (romsize*1024*1024/8) < filesize )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"デバイス容量", 0x14, 0x14, "実ファイルサイズよりも小さい値が指定されています。",
				"Device Capacity", "Setting data is less than the actual file size.", false, true ) );
		}
		else if( filesize < (romsize*1024*1024/8) )
		{
			this->hWarnList->Add( gcnew RCMrcError(		// 警告
				"デバイス容量", 0x14, 0x14, "実ファイルサイズに比べて無駄のある値が設定されています。",
				"Device Capacity", "Setting data is larger than the actual file size.", false, true ) );
		}
		if( (romsize < METWL_ROMSIZE_MIN) || (METWL_ROMSIZE_MAX < romsize) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"デバイス容量", 0x14, 0x14, "指定可能な容量ではありません。",
				"Device Capacity", "Invalid capacity.", false, true ) );
		}
		if( (filesize % 2) != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"実ファイルサイズ", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "中途半端な値です。通常では2のべき乗の値です。",
				"Actual File Size", "Invalid size. This size is usually power of 2.", false, true ) );
		}
	} //if( *(this->hIsNAND) == false )
	else
	{
		if( (romsize < METWL_ROMSIZE_MIN_NAND) || (METWL_ROMSIZE_MAX_NAND < romsize) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"デバイス容量", 0x14, 0x14, "NANDアプリに対して指定可能な容量ではありません。",
				"Device Capacity", "Invalid capacity.", false, true ) );
		}
		u32  allsizeMB = filesize + this->pRomHeader->s.public_save_data_size + this->pRomHeader->s.private_save_data_size;
		if( allsizeMB > METWL_ALLSIZE_MAX_NAND )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"実ファイルサイズ", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
				"ROMデータの実ファイルサイズとPublicセーブデータおよびPrivateセーブデータのサイズの総和が32MByteを超えています。",
				"Actual File Size", 
				"The sum of this size, the public save data size and private save data size exceed 32MByte.", false, true ) );
		}
	}

	if( this->pRomHeader->s.warning_no_spec_rom_speed != 0 )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"諸フラグ", 0x1f, 0x1f, "rsfファイルでROMSpeedTypeが設定されていません。",
			"Setting Flags", "In a RSF file, the item \"ROMSpeedType\" is not set.", false, true ) );
	}
	if( (this->pRomHeader->s.game_cmd_param & CARD_LATENCY_MASK) != CARD_1TROM_GAME_LATENCY )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ROMコントロール情報", 0x60, 0x67, "TWLではマスクROMは用意されていません。ワンタイムPROM設定にしてください。",
			"ROM Control Info.", "Mask ROM can be set. Please set One-time PROM.", false, true ) );
	}

	// 予約領域
	for( i=0; i < 7; i++ )
	{
		if( this->pRomHeader->s.reserved_A[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"予約領域A", 0x015, 0x01b, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area A", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}
	for( i=0; i < 39; i++ )
	{
		if( this->pRomHeader->s.reserved_B[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"予約領域B", 0x099, 0x0bf, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area B", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}

	// ROMヘッダのチェック (TWL専用領域)

	// 値チェック
	{
		u32  idH;
		u8   idL[4];
		int  apptype;
		const int appUser     = 0;
		const int appSystem   = 1;
		const int appSecure   = 2;
		const int appLauncher = 3;

		idH = this->pRomHeader->s.titleID_Hi;
		memcpy( idL, &(this->pRomHeader->s.titleID_Lo[0]), 4 );

		if( (idL[3]=='H') && (idL[2]=='N') && (idL[1]=='A') )   // ランチャアプリかどうかはTitleID_Loの値で決定
		{
			apptype = appLauncher;
		}
		else if( idH & TITLE_ID_HI_SECURE_FLAG_MASK )           // 立ってたらセキュアアプリ
		{
			apptype = appSecure;
		}
		else if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 1 )       // 立ってたらシステムアプリ
		{
			apptype = appSystem;
		}
		else if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 0 )       // 残るはユーザアプリ
		{
			apptype = appUser;
		}
		else
		{
			apptype = -1;
		}

		if( apptype < 0 )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"アプリ種別", 0x230, 0x237, "不正な値です。",
				"Application Type", "Illigal type.", false, true ) );
		}
#ifdef METWL_VER_APPTYPE_USER
		if( apptype != appUser )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"アプリ種別", 0x230, 0x237, "ユーザアプリではありません。",
				"Application Type", "Not USER application.", false, true ) );
		}
#endif
#ifdef METWL_VER_APPTYPE_SYSTEM
		if( apptype != appSystem )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"アプリ種別", 0x230, 0x237, "システムアプリではありません。",
				"Application Type", "Not SYSTEM application.", false, true ) );
		}
#endif
#ifdef METWL_VER_APPTYPE_SECURE
		if( apptype != appSecure )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"アプリ種別", 0x230, 0x237, "セキュアアプリではありません。",
				"Application Type", "Not SECURE application.", false, true ) );
		}
#endif
#ifdef METWL_VER_APPTYPE_LAUNCHER
		if( apptype != appLauncher )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"アプリ種別", 0x230, 0x237, "ランチャーアプリではありません。",
				"Application Type", "Not LAUNCHER application.", false, true ) );
		}
#endif
	} // アプリ種別のチェック

	if( (this->pRomHeader->s.access_control.game_card_on != 0) &&
		(this->pRomHeader->s.access_control.game_card_nitro_mode != 0) )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"アクセスコントロール情報", 0x1b4, 0x1b7, "ゲームカード電源設定にノーマルモードとNTRモードの両方を設定することはできません。",
			"Access Control Info.", "Game card power setting is either normal mode or NTR mode.", false, true ) );
	}
	if( ((this->pRomHeader->s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK) == 0) &&		// カードアプリ
		((this->pRomHeader->s.access_control.game_card_on != 0) || (this->pRomHeader->s.access_control.game_card_nitro_mode != 0)) )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"アクセスコントロール情報", 0x1b4, 0x1b7, "ゲームカード用ソフトに対してゲームカード電源設定をすることはできません。",
			"Access Control Info.", "Game card power setting is not for Game Card Soft.", false, true ) );
	}
	if( this->pRomHeader->s.access_control.shared2_file == 0 )
	{
		if( (this->pRomHeader->s.shared2_file0_size != 0) || (this->pRomHeader->s.shared2_file1_size != 0) ||
			(this->pRomHeader->s.shared2_file2_size != 0) || (this->pRomHeader->s.shared2_file3_size != 0) ||
			(this->pRomHeader->s.shared2_file4_size != 0) || (this->pRomHeader->s.shared2_file5_size != 0) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"アクセスコントロール情報", 0x1b4, 0x1b7, "Shared2ファイルのサイズが設定されているにもかかわらず不使用設定になっています。",
				"Access Control Info.", "Sizes of shared2 files is setting, but using them is not enabled.", false, true ) );
		}
	}

	if( (this->pRomHeader->s.arm7_scfg_ext >> 31) != 0 )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"SCFG設定", 0x1b8, 0x1bb, "SCFGレジスタへアクセス可能になっています。",
			"SCFG Setting", "In this setting, SCFG register is accessible.", false, true ) );
	}

	if( (this->pRomHeader->s.titleID_Lo[0] != this->pRomHeader->s.game_code[3]) ||
		(this->pRomHeader->s.titleID_Lo[1] != this->pRomHeader->s.game_code[2]) ||
		(this->pRomHeader->s.titleID_Lo[2] != this->pRomHeader->s.game_code[1]) ||
		(this->pRomHeader->s.titleID_Lo[3] != this->pRomHeader->s.game_code[0]) )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"タイトルID", 0x230, 0x233, "下位4バイトがイニシャルコードと一致しません。",
			"Title ID", "Lower 4 bytes don't match ones of Game Code.", false, true ) );
	}

	for( i=0; i < (0x2f0 - 0x240); i++ )
	{
		if( this->pRomHeader->s.reserved_ltd_F[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"予約領域F", 0x240, 0x2ef, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area F", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}

	for( i=0; i < (0xf80 - 0x378); i++ )
	{
		u8 *p = (u8*)this->pRomHeader;
		if( p[ 0x378 + i ] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"予約領域D", 0x378, 0xf7f, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area D", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}

	// ROMヘッダ以外の領域のチェック

	ECSrlResult pctl = this->mrcTWLParentalControl();
	if( pctl != ECSrlResult::NOERROR )
	{
		return pctl;
	}

	// 追加チェック
	if( *(this->hMrcSpecialList->hIsCheck) == true )
	{
		// SDKバージョン
		System::Boolean match = true;
		for each( RCSDKVersion ^sdk in this->hSDKList )
		{
			if( sdk->IsStatic && (sdk->Code != *(this->hMrcSpecialList->hSDKVer) ) )
			{
				match = false;
			}
		}
		if( !match )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"SDKバージョン", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "本プログラムに登録されているバージョン情報と一致しません。",
				"SDK Version", "The data doesn't match one registered in this program.", false, true ) );
		}

		// EULAバージョン
		if( *(this->hEULAVersion) != *(this->hMrcSpecialList->hEULAVer) )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"EULA バージョン", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "本プログラムに登録されているバージョン情報と一致しません。",
				"EULA Version", "The data doesn't match one registered in this program.", false, true ) );
		}

		// Shared2ファイルサイズ
		for( i=0; i < METWL_NUMOF_SHARED2FILES; i++ )
		{
			if( *(this->hShared2SizeArray[i]) > 0 )	// 0は未使用を表すのでチェックしない
			{
				if( *(this->hShared2SizeArray[i]) != *(this->hMrcSpecialList->hShared2SizeArray[i]) )
				{
					this->hWarnList->Add( gcnew RCMrcError( 
						"Shared2ファイル" + i.ToString(), METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, 
						"本プログラムに登録されているファイルサイズ情報と一致しません。",
						"Shared2 file" + i.ToString(), "The size doesn't match one registered in this program.", false, true ) );
				}
			}
		}

	} //if( *(this->hMrcSpecialList->hIsCheck) )

	return ECSrlResult::NOERROR;
} // mrcTWL()

// PCTL専用
ECSrlResult RCSrl::mrcTWLParentalControl(void)
{
	// リージョンが正常か
	u32 region = this->pRomHeader->s.card_region_bitmap;
	if( !this->mrcRegion( region ) )
	{
		return ECSrlResult::NOERROR;	// これ以上チェックしない
	}

	// リージョンに含まれている団体/含まれていない団体の設定をチェック
	this->mrcRegionOrganization( region );

	// リージョンに含まれている団体の設定が正しいかどうか
	if( region == METWL_MASK_REGION_JAPAN )
	{
		this->mrcRating( OS_TWL_PCTL_OGN_CERO );
	}
	if( region == METWL_MASK_REGION_AMERICA )
	{
		this->mrcRating( OS_TWL_PCTL_OGN_ESRB );
	}
	if( region == METWL_MASK_REGION_EUROPE )
	{
		this->mrcRating( OS_TWL_PCTL_OGN_USK );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_GEN );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_PRT );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_BBFC );
	}
	if( region == METWL_MASK_REGION_AUSTRALIA )
	{
		this->mrcRating( OS_TWL_PCTL_OGN_OFLC );
	}
	if( region == (METWL_MASK_REGION_EUROPE | METWL_MASK_REGION_AUSTRALIA) )
	{
		this->mrcRating( OS_TWL_PCTL_OGN_USK );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_GEN );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_PRT );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_BBFC );
		this->mrcRating( OS_TWL_PCTL_OGN_OFLC );
	}
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	if( region == METWL_MASK_REGION_ALL )
	{
		this->mrcRating( OS_TWL_PCTL_OGN_CERO );
		this->mrcRating( OS_TWL_PCTL_OGN_ESRB );
		this->mrcRating( OS_TWL_PCTL_OGN_USK );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_GEN );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_PRT );
		this->mrcRating( OS_TWL_PCTL_OGN_PEGI_BBFC );
		this->mrcRating( OS_TWL_PCTL_OGN_OFLC );
	}
#endif //#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)

	return ECSrlResult::NOERROR;
} //mrcParentalControl()

// PCTLチェックの一部 リージョンが正常かチェック
System::Boolean RCSrl::mrcRegion( System::UInt32 region )
{
	System::Boolean bRegionResult = true;	// リージョン設定が正常であるか
	if( (region != METWL_MASK_REGION_JAPAN)  && (region != METWL_MASK_REGION_AMERICA) &&
		(region != METWL_MASK_REGION_EUROPE) &&	(region != METWL_MASK_REGION_AUSTRALIA) &&
		(region != (METWL_MASK_REGION_EUROPE | METWL_MASK_REGION_AUSTRALIA)) )
	{
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
		if( region != METWL_MASK_REGION_ALL )	// オールリージョンを許す
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"カードリージョン", 0x1b0, 0x1b3, "仕向地の組み合わせが不正です。本ツールを用いて修正できます。",
				"Card Region", "Illigal Region. This tool can modify this information.", true, true ) );
			bRegionResult = false;
		}
#else
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"カードリージョン", 0x1b0, 0x1b3, "仕向地の組み合わせが不正です。本ツールを用いて修正してください。",
				"Card Region", "Illigal Region. Please modify this information using this tool.", true, true ) );
			bRegionResult = false;
		}
#endif
	}
	// リージョンが不正のときすべてのPCTL情報をクリア(ROMヘッダには影響しない)
	if( !bRegionResult )
	{
		int i;
		for( i=0; i < PARENTAL_CONTROL_INFO_SIZE; i++ )
		{
			this->clearParentalControl( i );
		}
		this->hWarnList->Add( gcnew RCMrcError( 
			"ペアレンタルコントロール情報", 0x2f0, 0x2ff, 
			"仕向地の設定が不正のため、ROMデータ内のすべてのレーティング団体の情報を無視して読み込みました。本ツールを用いて修正してください。",
			"Parental Control", 
			"Illigal region. In reading, therefore, settings for all rating organizations are ignored. Please set this infomation using this tool.", 
			true, true ) );
	}
	return bRegionResult;
}

// PCTLチェックの一部 リージョンに含まれる団体/含まれない団体の設定をチェック
void RCSrl::mrcRegionOrganization( System::UInt32 region )
{
	// 表示用の団体ラベル
	cli::array<System::String^> ^ognArray = gcnew cli::array<System::String^>(PARENTAL_CONTROL_INFO_SIZE);
	System::Int32 i;
	for( i=0; i < PARENTAL_CONTROL_INFO_SIZE; i++ )
	{
		switch(i)
		{
			case OS_TWL_PCTL_OGN_CERO:
				ognArray[i] = gcnew System::String( "CERO: " );
			break;
			case OS_TWL_PCTL_OGN_ESRB:
				ognArray[i] = gcnew System::String( "ESRB: " );
			break;
			case OS_TWL_PCTL_OGN_USK:
				ognArray[i] = gcnew System::String( "USK: " );
			break;
			case OS_TWL_PCTL_OGN_PEGI_GEN:
				ognArray[i] = gcnew System::String( "PEGI(General): " );
			break;
			case OS_TWL_PCTL_OGN_PEGI_PRT:
				ognArray[i] = gcnew System::String( "PEGI(Portugal): " );
			break;
			case OS_TWL_PCTL_OGN_PEGI_BBFC:
				ognArray[i] = gcnew System::String( "PEGI and BBFC: " );
			break;
			case OS_TWL_PCTL_OGN_OFLC:
				ognArray[i] = gcnew System::String( "OFLC: " );
			break;
		}
	}
	System::String ^warnEmptyJ = gcnew System::String( "レーティング審査の必要がないソフトだとみなします。審査が必要である場合には、本ツールを用いて修正してください。" );
	System::String ^warnEmptyE = gcnew System::String( "This ROM is not necessary for CERO rating. If necessary, please set this infomation using this tool." );
	System::String ^warnFillJ  = gcnew System::String( "リージョンに含まれない団体の情報が設定されていたため、これらの団体の情報を無視して読み込みました。本ツールを用いて修正してください。" );
	System::String ^warnFillE  = gcnew System::String( "Some organizations is not include in the region. In reading, therefore, settings for all rating organizations are ignored. Please set this infomation using this tool." );

	// リージョンに含まれる団体と含まれない団体をリスト化する
	System::Collections::Generic::List<System::Byte> ^inList = gcnew System::Collections::Generic::List<System::Byte>();
	inList->Clear();
	System::Collections::Generic::List<System::Byte> ^exList = gcnew System::Collections::Generic::List<System::Byte>();
	exList->Clear();

	switch( region )
	{
		case METWL_MASK_REGION_JAPAN:
			inList->Add( OS_TWL_PCTL_OGN_CERO );	// 含まれるリスト
			exList->Add( OS_TWL_PCTL_OGN_ESRB );	// 含まれないリスト
			exList->Add( OS_TWL_PCTL_OGN_USK );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_GEN );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_PRT );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_BBFC );
			exList->Add( OS_TWL_PCTL_OGN_OFLC );
		break;

		case METWL_MASK_REGION_AMERICA:
			exList->Add( OS_TWL_PCTL_OGN_CERO );
			inList->Add( OS_TWL_PCTL_OGN_ESRB );
			exList->Add( OS_TWL_PCTL_OGN_USK );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_GEN );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_PRT );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_BBFC );
			exList->Add( OS_TWL_PCTL_OGN_OFLC );
		break;

		case METWL_MASK_REGION_EUROPE:
			exList->Add( OS_TWL_PCTL_OGN_CERO );
			exList->Add( OS_TWL_PCTL_OGN_ESRB );
			inList->Add( OS_TWL_PCTL_OGN_USK );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_GEN );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_PRT );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_BBFC );
			exList->Add( OS_TWL_PCTL_OGN_OFLC );
		break;

		case METWL_MASK_REGION_AUSTRALIA:
			exList->Add( OS_TWL_PCTL_OGN_CERO );
			exList->Add( OS_TWL_PCTL_OGN_ESRB );
			exList->Add( OS_TWL_PCTL_OGN_USK );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_GEN );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_PRT );
			exList->Add( OS_TWL_PCTL_OGN_PEGI_BBFC );
			inList->Add( OS_TWL_PCTL_OGN_OFLC );
		break;

		case (METWL_MASK_REGION_EUROPE|METWL_MASK_REGION_AUSTRALIA):
			exList->Add( OS_TWL_PCTL_OGN_CERO );
			exList->Add( OS_TWL_PCTL_OGN_ESRB );
			inList->Add( OS_TWL_PCTL_OGN_USK );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_GEN );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_PRT );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_BBFC );
			inList->Add( OS_TWL_PCTL_OGN_OFLC );
		break;

#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
		case METWL_MASK_REGION_ALL:
			inList->Add( OS_TWL_PCTL_OGN_CERO );
			inList->Add( OS_TWL_PCTL_OGN_ESRB );
			inList->Add( OS_TWL_PCTL_OGN_USK );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_GEN );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_PRT );
			inList->Add( OS_TWL_PCTL_OGN_PEGI_BBFC );
			inList->Add( OS_TWL_PCTL_OGN_OFLC );
		break;
#endif //#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	}

	// リージョンに含まれる団体に何も設定されていないときダメ
	for each ( System::Byte ogn in inList )
	{
		if( this->pRomHeader->s.parental_control_rating_info[ ogn ] == 0 )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"ペアレンタルコントロール情報", 0x2f0, 0x2ff, ognArray[ogn] + warnEmptyJ,
				"Parental Control", ognArray[ogn] + warnEmptyE, true, true ) );
			this->clearParentalControl( ogn );
		}
	}
	// リージョンに含まれない団体に何か設定されていたらダメ
	System::Boolean bSet = false;
	for each ( System::Byte ogn in exList )
	{
		if( this->pRomHeader->s.parental_control_rating_info[ ogn ] != 0 )
		{
			bSet = true;
			this->clearParentalControl( ogn );
		}
	}
	if( bSet )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"ペアレンタルコントロール情報", 0x2f0, 0x2ff, warnFillJ,
			"Parental Control", warnFillE, true, true ) );
	}
} //mrcRegion

// レーティング値が正常かどうかチェック
void RCSrl::mrcRating( System::Byte ogn )
{
	System::String ^warnDisableJ = gcnew System::String( "制限が無効であるにもかかわらずレーティングが設定されていため、レーティングを無視して読み込みました。本ツールを用いて修正してください。" );
	System::String ^warnDisableE = gcnew System::String( "Rating can't be set when control isn't enable. rating setting is ignored. Please set this infomation using this tool." );
	System::String ^warnIllegalJ = gcnew System::String( "レーティング情報が不正であるため、無視して読み込みました。本ツールを用いて修正してください。" );
	System::String ^warnIllegalE = gcnew System::String( "Illegal rating. rating setting is ignored. Please set this information using this tool." );
	System::String ^warnPendingJ = gcnew System::String( "Rating Pendingが指定されています。この指定とレーティング値は同時に設定できないため、レーティング値を無視して読み込みました。" );
	System::String ^warnPendingE = gcnew System::String( "Rating Pending is setting. It is exclusive a rating age. The age is ignored." );

	cli::array<System::String^> ^ognArray = gcnew cli::array<System::String^>(PARENTAL_CONTROL_INFO_SIZE);
	System::Int32 i;
	for( i=0; i < PARENTAL_CONTROL_INFO_SIZE; i++ )
	{
		switch(i)
		{
			case OS_TWL_PCTL_OGN_CERO:
				ognArray[i] = gcnew System::String( "CERO: " );
			break;
			case OS_TWL_PCTL_OGN_ESRB:
				ognArray[i] = gcnew System::String( "ESRB: " );
			break;
			case OS_TWL_PCTL_OGN_USK:
				ognArray[i] = gcnew System::String( "USK: " );
			break;
			case OS_TWL_PCTL_OGN_PEGI_GEN:
				ognArray[i] = gcnew System::String( "PEGI(General): " );
			break;
			case OS_TWL_PCTL_OGN_PEGI_PRT:
				ognArray[i] = gcnew System::String( "PEGI(Portugal): " );
			break;
			case OS_TWL_PCTL_OGN_PEGI_BBFC:
				ognArray[i] = gcnew System::String( "PEGI and BBFC: " );
			break;
			case OS_TWL_PCTL_OGN_OFLC:
				ognArray[i] = gcnew System::String( "OFLC: " );
			break;
		}
	}

	if( *(this->hArrayParentalEffect[ ogn ]) == false )
	{
		if( *this->hArrayParentalAlways[ ogn ] || (*this->hArrayParentalRating[ ogn ] != 0 ) )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"ペアレンタルコントロール情報", 0x2f0, 0x2ff, ognArray[ ogn ] + warnDisableJ,
				"Parental Control", ognArray[ ogn ] + warnDisableE, true, true ) );
			this->clearParentalControl( ogn );
		}
	}
	else
	{
		// 間違っていたら"未審査"が返ってくる
		System::String ^str = MasterEditorTWL::transRatingToString( 
			ogn, true, *(this->hArrayParentalRating[ogn]), false );

		if( *(this->hArrayParentalAlways[ogn]) == false )
		{
			// RPが立っていないときレーティング値が設定されていなかったらダメ
			if( (str != nullptr) && str->Equals( L"未審査" ) )
			{
				this->hWarnList->Add( gcnew RCMrcError( 
					"ペアレンタルコントロール情報", 0x2f0, 0x2ff, ognArray[ ogn ] + warnIllegalJ,
					"Parental Control", ognArray[ ogn ] + warnIllegalE, true, true ) );
				this->clearParentalControl( ogn );
			}
		}
		else
		{
			// RPが立っていたら問答無用に警告
			this->hWarnList->Add( gcnew RCMrcError( 
				"ペアレンタルコントロール情報", 0x2f0, 0x2ff, ognArray[ ogn ] + warnPendingJ,
				"Parental Control", ognArray[ ogn ] + warnPendingE, true, true ) );
			this->clearParentalControl( ogn );
		}
	}
} //mrcRating()

// ペアレンタルコントロールのクリア
void RCSrl::clearParentalControl( System::Byte ogn )
{
	this->hArrayParentalRating[ ogn ] = gcnew System::Byte( 0x00 );
	this->hArrayParentalEffect[ ogn ] = gcnew System::Boolean( false );
	this->hArrayParentalAlways[ ogn ] = gcnew System::Boolean( false );
}
