// srl.h のクラス実装

#include "stdafx.h"
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
	// ファイルを閉じる前にROMヘッダ以外の領域から設定を取り出す
	{
		ECSrlResult      r;
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
		r = this->mrc( fp );
		if( r != ECSrlResult::NOERROR )
		{
			(void)fclose(fp);
			return r;
		}
	}
	(void)fclose( fp );

	// ROMヘッダの値をROM固有情報フィールドに反映させる
	(void)this->setRomInfo();

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
	u32  val;
	val = ((u32)(idL[0]) << 24) | ((u32)(idL[1]) << 16) | ((u32)(idL[2]) << 8) | ((u32)(idL[3]));	// ビッグエンディアン
	this->hTitleIDLo   = gcnew System::UInt32( val );
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
    if( idH & TITLE_ID_HI_SECURE_FLAG_MASK )				// 各ビットは排他的とは限らないのでelse ifにはならない
    {
		*(this->hIsAppSecure) = true;
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
    if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 1 )
    {
		*(this->hIsAppSystem) = true;
    }
    if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 0 )
    {
		*(this->hIsAppUser) = true;
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
	this->hShared2Size0 = gcnew System::UInt32( 0 );
	this->hShared2Size1 = gcnew System::UInt32( 0 );
	this->hShared2Size2 = gcnew System::UInt32( 0 );
	this->hShared2Size3 = gcnew System::UInt32( 0 );
	this->hShared2Size4 = gcnew System::UInt32( 0 );
	this->hShared2Size5 = gcnew System::UInt32( 0 );
	u32  unit = 16 * 1024;		// 16KBの乗数が格納されている
	if( this->pRomHeader->s.shared2_file0_size != 0 )
	{
		*(this->hShared2Size0) = (this->pRomHeader->s.shared2_file0_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file1_size != 0 )
	{
		*(this->hShared2Size1) = (this->pRomHeader->s.shared2_file1_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file2_size != 0 )
	{
		*(this->hShared2Size2) = (this->pRomHeader->s.shared2_file2_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file3_size != 0 )
	{
		*(this->hShared2Size3) = (this->pRomHeader->s.shared2_file3_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file4_size != 0 )
	{
		*(this->hShared2Size4) = (this->pRomHeader->s.shared2_file4_size * unit) + unit;
	}
	if( this->pRomHeader->s.shared2_file5_size != 0 )
	{
		*(this->hShared2Size5) = (this->pRomHeader->s.shared2_file5_size * unit) + unit;
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
			while( patch > 10000 )
			{
				patch -= 10000;
			}
			patch = patch / 100;
			switch( relstep / 10000 )
			{
				case 1: str += ("PR " + patch.ToString()); break;
				case 2: str += ("RC " + patch.ToString()); break;
				case 3: str += ("RELEASE " + patch.ToString()); break;
				default: break;
			}
			u32 statbegin = this->pRomHeader->s.main_rom_offset;
			u32 statend   = this->pRomHeader->s.main_rom_offset + this->pRomHeader->s.main_size - 1;
			System::Boolean isstat = ((statbegin <= offset) && (offset <= statend))?true:false;
			this->hSDKList->Add( gcnew RCSDKVersion(str, isstat) );
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
}

//
// MRC
//
ECSrlResult RCSrl::mrc( FILE *fp )
{
	this->hErrorList = gcnew System::Collections::Generic::List<RCMRCError^>;
	this->hWarnList  = gcnew System::Collections::Generic::List<RCMRCError^>;

	this->hErrorList->Clear();
	this->hWarnList->Clear();

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
	//this->hErrorList->Add( gcnew RCMRCError( "テスト", 0x01, 0xfe, "テスト要因", "test", "reason", false ) );
	//this->hWarnList->Add( gcnew RCMRCError( "テスト2", 0x01, 0xfe, "テスト要因2", "test2", "reason2", false ) );

	return ECSrlResult::NOERROR;
}

// NTR互換MRC
ECSrlResult RCSrl::mrcNTR( FILE *fp )
{
	System::Int32  i;

	// ROMヘッダのチェック

	// 文字コードチェック
	for( i=0; i < TITLE_NAME_MAX; i++ )
	{
		char c = this->pRomHeader->s.title_name[i];
		if( ((c < 0x20) || (0x5f < c)) && (c != 0x00) )
		{
			this->hErrorList->Add( gcnew RCMRCError( 
				"ソフトタイトル", 0x0, 0xb, "使用不可のASCIIコードが使用されています。",
				"Game Title", "Unusable ASCII code is used.", false ) );
		}
	}
	for( i=0; i < GAME_CODE_MAX; i++ )
	{
		char c = this->pRomHeader->s.game_code[i];
		if( (c < 0x20) || (0x5f < c) )
		{
			this->hErrorList->Add( gcnew RCMRCError( 
				"イニシャルコード", 0xc, 0xf, "使用不可のASCIIコードが使用されています。",
				"Game Code", "Unusable ASCII code is used.", false ) );
		}
	}
	if( memcmp( this->pRomHeader->s.game_code, "NTRJ", GAME_CODE_MAX ) == 0 )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"イニシャルコード", 0xc, 0xf, "SDKデフォルトのコード(NTRJ)が使用されています。",
			"Game Code", "SDK default code(NTRJ) is used.", false ) );
	}
	for( i=0; i < MAKER_CODE_MAX; i++ )
	{
		char c = this->pRomHeader->s.maker_code[i];
		if( (c < 0x20) || (0x5f < c) )
		{
			this->hErrorList->Add( gcnew RCMRCError(
				"メーカーコード", 0x10, 0x11, "使用不可のASCIIコードが使用されています。",
				"Maker Code", "Unusable ASCII code is used.", false ) );
		}
	}

	// 値チェック
	if( this->pRomHeader->s.rom_type != 0x00 )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"デバイスタイプ", 0x13, 0x13, "不正な値です。00hを設定してください。",
			"Device Type", "Invalid data. Please set 00h.", false ) );
	}
	fseek( fp, 0, SEEK_END );
	u32  filesize = ftell(fp);	// 実ファイルサイズ(単位Mbit)
	u32  romsize = 1 << (this->pRomHeader->s.rom_size);	// ROM容量
	if( (romsize*1024*1024/8) < filesize )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"デバイス容量", 0x14, 0x14, "実ファイルサイズよりも小さい値が指定されています。",
			"Device Capacity", "Setting data is less than the actual file size.", false ) );
	}
	else if( filesize < (romsize*1024*1024/8) )
	{
		this->hWarnList->Add( gcnew RCMRCError(		// 警告
			"デバイス容量", 0x14, 0x14, "実ファイルサイズに比べて無駄のある値が設定されています。",
			"Device Capacity", "Setting data is larger than the actual file size.", false ) );
	}
	if( (filesize % 2) != 0 )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"実ファイルサイズ", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "中途半端な値です。通常では2のべき乗の値です。",
			"Actual File Size", "Invalid size. This size is usually power of 2.", false ) );
	}
	u8 romver = this->pRomHeader->s.rom_version;
	if( ((romver < 0x00) || (0x09 < romver)) && (romver != 0xE0) )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"リマスターバージョン", 0x1e, 0x1e, "不正な値です。正式版では01h-09hのいずれかの値、事前版ではE0hです。",
			"Release Ver.", "Invalid data. Please set either one of 01h-09h(Regular ver.), or E0h(Preliminary ver.)", false ) );
	}
	if( this->pRomHeader->s.warning_no_spec_rom_speed != 0 )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"諸フラグ", 0x1f, 0x1f, "rsfファイルでROMSpeedTypeが設定されていません。",
			"Setting Flags", "In a RSF file, the item \"ROMSpeedType\" is not set.", false ) );
	}
	if( this->pRomHeader->s.banner_offset == 0 )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"バナーオフセット", 0x68, 0x6b, "バナーデータが設定されていません。",
			"Banner Offset.", "Banner data is not set.", false ) );
	}
	if( this->pRomHeader->s.rom_valid_size == 0 )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"ROM実効サイズ", 0x80, 0x83, "値が設定されていません。",
			"ROM Valid Size.", "Data is not set.", false ) );
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
			this->hErrorList->Add( gcnew RCMRCError( 
				"セキュア領域CRC", 0x15e, 0x15f, "セキュア領域のアドレス指定が不正です。",
				"Secure Area CRC.", "Illegal address of secure area.", false ) );
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
			this->hErrorList->Add( gcnew RCMRCError( 
				"セキュア領域CRC", 0x07c, 0x07d, "計算結果と一致しません。セキュア領域が改ざんされた可能性があります。",
				"Secure Area CRC.", "Calclated CRC is different from Registered one.", false ) );
		}
	}

	// ロゴ領域
	crc = 0xcf56;
	if( crc != this->pRomHeader->s.nintendo_logo_crc16 )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"キャラクターデータCRC", 0x15c, 0x15d, "計算結果と一致しません。キャラクターデータが改ざんされた可能性があります。",
			"Charactor Data CRC.", "Calclated CRC is different from Registered one.", false ) );
	}

	// ヘッダCRC
	crc = calcCRC( CRC16_INIT_VALUE, (u8*)this->pRomHeader, CALC_CRC16_SIZE );
	if( crc != this->pRomHeader->s.header_crc16 )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"ROMヘッダCRC", 0x15e, 0x15f, "計算結果と一致しません。ROMヘッダが改ざんされた可能性があります。",
			"ROM Header CRC.", "Calclated CRC is different from Registered one.", false ) );
	}

	// 予約領域
	for( i=0; i < 8; i++ )
	{
		if( this->pRomHeader->s.ctrl_reserved_B[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMRCError( 
				"ROM情報予約領域", 0x078, 0x07f, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area for ROM Info.", "Invalid data is included. Please set 0 into this area.", false ) );
		}
	}
	for( i=0; i < 32; i++ )
	{
		if( this->pRomHeader->s.reserved_C[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMRCError( 
				"予約領域C", 0x078, 0x07f, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area C", "Invalid data is included. Please set 0 into this area.", false ) );
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
		this->hErrorList->Add( gcnew RCMRCError( 
			"システムコールライブラリ", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "SDKデフォルトです。",
			"System-Call Library", "This Library is SDK default one.", false ) );
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
	u32  romsize = 1 << (this->pRomHeader->s.rom_size);	// ROM容量
	if( (romsize < METWL_ROMSIZE_MIN) || (METWL_ROMSIZE_MAX < romsize) )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"デバイス容量", 0x14, 0x14, "指定可能な容量ではありません。",
			"Device Capacity", "Invalid capacity.", false ) );
	}
	if( (this->pRomHeader->s.game_cmd_param & CARD_LATENCY_MASK) != CARD_1TROM_GAME_LATENCY )
	{
		this->hErrorList->Add( gcnew RCMRCError( 
			"ROMコントロール情報", 0x60, 0x67, "TWLではマスクROMは用意されていません。ワンタイムPROM設定にしてください。",
			"ROM Control Info.", "Mask ROM can be set. Please set One-time PROM.", false ) );
	}

	// 予約領域
	for( i=0; i < 7; i++ )
	{
		if( this->pRomHeader->s.reserved_A[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMRCError( 
				"予約領域A", 0x015, 0x01b, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area A", "Invalid data is included. Please set 0 into this area.", false ) );
		}
	}
	for( i=0; i < 39; i++ )
	{
		if( this->pRomHeader->s.reserved_B[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMRCError( 
				"予約領域B", 0x099, 0x0bf, "不正な値が含まれています。この領域をすべて0で埋めてください。",
				"Reserved Area B", "Invalid data is included. Please set 0 into this area.", false ) );
		}
	}

	// ROMヘッダのチェック (TWL専用領域)

	// 値チェック
	u32 region = this->pRomHeader->s.card_region_bitmap;
	if( (region != METWL_MASK_REGION_JAPAN) &&
		(region != METWL_MASK_REGION_AMERICA) &&
		(region != METWL_MASK_REGION_EUROPE) &&
		(region != (METWL_MASK_REGION_EUROPE | METWL_MASK_REGION_AUSTRALIA)) )
	{
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
		if( region != METWL_MASK_REGION_ALL )	// オールリージョンを許す
		{
			this->hErrorList->Add( gcnew RCMRCError( 
				"カードリージョン", 0x1b0, 0x1b3, "仕向地の組み合わせが不正です。本ツールを用いて修正できます。",
				"Card Region", "Illigal Region. This tool can modify this information.", true ) );
		}
#else
		{
			this->hErrorList->Add( gcnew RCMRCError( 
				"カードリージョン", 0x1b0, 0x1b3, "仕向地の組み合わせが不正です。本ツールを用いて修正できます。",
				"Card Region", "Illigal Region. This tool can modify this information.", true ) );
		}
#endif
	}

	// ROMヘッダ以外の領域のチェック

	return ECSrlResult::NOERROR;
}