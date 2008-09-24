// srl.h のクラス実装

#include "stdafx.h"
#include <apptype.h>
#include "common.h"
#include "srl.h"
#include "utility.h"
#include "keys.h"
#include "crc_whole.h"
#include <acsign/include/acsign.h>
#include <format_rom_private.h>
#include <cstring>
#include <cstdio>

using namespace MasterEditorTWL;

extern const u8 gBannerFont[];

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
	ECSrlResult r;

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

	// エラーリストをクリア
	this->hErrorList = gcnew System::Collections::Generic::List<RCMrcError^>;
	this->hWarnList  = gcnew System::Collections::Generic::List<RCMrcError^>;
	this->hParentalErrorList = gcnew System::Collections::Generic::List<RCMrcError^>;
	this->hErrorList->Clear();
	this->hWarnList->Clear();
	this->hParentalErrorList->Clear();

	// ROMヘッダの値をROM固有情報フィールドに反映させる
	(void)this->setRomInfo();

#ifdef METWL_WHETHER_SIGN_DECRYPT
	// ひととおり読んだあとに署名チェック
	r = this->decryptRomHeader();
	if( r != ECSrlResult::NOERROR )
	{
		(void)fclose(fp);
		return r;
	}
#endif //#ifdef METWL_WHETHER_SIGN_DECRYPT

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
	this->hIsOldDevEncrypt = gcnew System::Boolean( (this->pRomHeader->s.developer_encrypt_old != 0)?true:false ); 

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

	// TWL専用情報
	this->hIsNormalJump = gcnew System::Boolean( (this->pRomHeader->s.permit_landing_normal_jump != 0)?true:false );
	this->hIsTmpJump    = gcnew System::Boolean( (this->pRomHeader->s.permit_landing_tmp_jump    != 0)?true:false );
	this->hNormalRomOffset   = gcnew System::UInt32( (u32)(this->pRomHeader->s.twl_card_normal_area_rom_offset)   * 0x80000 );
	this->hKeyTableRomOffset = gcnew System::UInt32( (u32)(this->pRomHeader->s.twl_card_keytable_area_rom_offset) * 0x80000 );
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

	// SCFG がロックされるか
	if( (this->pRomHeader->s.arm7_scfg_ext >> 31) != 0 )
	{
		this->hIsSCFGAccess = gcnew System::Boolean( true );
	}
	else
	{
		this->hIsSCFGAccess = gcnew System::Boolean( false );
	}


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

	// ペアレンタルコントロール
	this->setParentalControlInfo();

	return ECSrlResult::NOERROR;
} // ECSrlResult RCSrl::setRomInfo(void)

// ROMヘッダ内のペアレンタルコントロール情報をフィールドに反映させる
void RCSrl::setParentalControlInfo(void)
{
	// リージョンに含まれていないものは読み込まない
	this->hArrayParentalIndex = gcnew cli::array<int>(PARENTAL_CONTROL_INFO_SIZE);

	int i;
	for( i=0; i < PARENTAL_CONTROL_INFO_SIZE; i++ )
	{
		this->hArrayParentalIndex[i] = -1;
	}

	// リージョンに含まれている団体をリストアップ
	u32 region = this->pRomHeader->s.card_region_bitmap;
	switch( region )
	{
		case METWL_MASK_REGION_JAPAN:
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_CERO );	// リージョンに含まれない団体の情報は読み込まない
		break;

		case METWL_MASK_REGION_AMERICA:
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_ESRB );
		break;

		case METWL_MASK_REGION_EUROPE:
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_USK );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_GEN );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_PRT );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_BBFC );
		break;

		case METWL_MASK_REGION_AUSTRALIA:
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_OFLC );
		break;

		case (METWL_MASK_REGION_EUROPE|METWL_MASK_REGION_AUSTRALIA):
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_USK );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_GEN );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_PRT );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_BBFC );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_OFLC );
		break;

#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
		case METWL_MASK_REGION_ALL:
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_CERO );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_ESRB );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_USK );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_GEN );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_PRT );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_PEGI_BBFC );
			this->setOneRatingOrgInfo( OS_TWL_PCTL_OGN_OFLC );
		break;
#endif //#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)

		default:
			this->hParentalErrorList->Add( gcnew RCMrcError( 
				"カードリージョン", 0x1b0, 0x1b3, "仕向地の組み合わせが不正です。ペアレンタルコントロール情報は無視して読み込まれました。",
				"Card Region", "Illigal Region. Parental Control Information is ignored in reading.", true, true ) );
		break;
	}
}

// 団体のレーティング情報からコンボボックスのインデックスを決定する
void RCSrl::setOneRatingOrgInfo( int ogn )
{
	System::Boolean enable = ((this->pRomHeader->s.parental_control_rating_info[ ogn ] & OS_TWL_PCTL_OGNINFO_ENABLE_MASK) != 0)?true:false;
	System::Boolean rp     = ((this->pRomHeader->s.parental_control_rating_info[ ogn ] & OS_TWL_PCTL_OGNINFO_ALWAYS_MASK) != 0)?true:false;
	System::Byte    age    = this->pRomHeader->s.parental_control_rating_info[ ogn ] & OS_TWL_PCTL_OGNINFO_AGE_MASK;

	cli::array<System::Byte> ^ages = MasterEditorTWL::getOgnRatingAges( ogn );
	System::String           ^name = MasterEditorTWL::getOgnName( ogn );

	int index = -1;
	if( !enable )	// 未定義
	{
		index = -1;
		this->hParentalErrorList->Add( gcnew RCMrcError( 
			"ペアレンタルコントロール情報", 0x2f0, 0x2ff,
			name + ": 未定義です。",
			"Parental Control Info.", 
			name + ": Undefined.", true, true ) );
	}
	else
	{
		if( rp )	// 審査中
		{
			if( age == 0 ) // 年齢が0のときは審査中とみなす
			{
				index = ages->Length;	// 配列の最後の要素の次が「審査中」
			}
			else
			{
				index = -1;		// それ以外はエラー
				this->hParentalErrorList->Add( gcnew RCMrcError( 
					"ペアレンタルコントロール情報", 0x2f0, 0x2ff,
					name + ": 審査中指定とレーティング年齢が同時に設定されています。",
					"Parental Control Info.", 
					name + ": Both the Rating-Peding setting and the rating age are setting.", true, true ) );
			}
		}
		else
		{
			index = -1;		// 団体が認めていない年齢が格納されているときエラー
			int i;
			for( i=0; i < ages->Length; i++ )
			{
				if( age == ages[i] )
				{
					index = i;
				}
			}
			if( index < 0 )
			{
				this->hParentalErrorList->Add( gcnew RCMrcError( 
					"ペアレンタルコントロール情報", 0x2f0, 0x2ff,
					name + ": レーティング年齢が団体によって指定された値ではありません。",
					"Parental Control Info.", 
					name + ": An age isn't accepted by the organization.", true, true ) );
			}
		}
	}
	this->hArrayParentalIndex[ ogn ] = index;
}

//
// ROMヘッダにROM固有情報フィールドの値を反映させる
//
ECSrlResult RCSrl::setRomHeader(void)
{
	ECSrlResult result;

	// ROMヘッダの[0,0x160)の領域はRead Onlyで変更しない

	// いくつかのフラグをROMヘッダに反映
	this->pRomHeader->s.exFlags.agree_EULA = (*(this->hIsEULA) == true)?1:0;
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
	this->setParentalControlHeader();

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

// ペアレンタルコントロールのプロパティをROMヘッダに反映させる
void RCSrl::setParentalControlHeader(void)
{
	int i;
	for( i=0; i < PARENTAL_CONTROL_INFO_SIZE; i++ )
	{
		cli::array<System::Byte> ^ages = MasterEditorTWL::getOgnRatingAges( i );	// 設定可能年齢リストを取得

		u8 rating;
		if( this->hArrayParentalIndex[i] < 0 )		// 未定義
		{
			rating = 0x00;
		}
		else
		{
			int index = this->hArrayParentalIndex[i];
			if( index == ages->Length )				// 審査中
			{
				rating = OS_TWL_PCTL_OGNINFO_ENABLE_MASK | OS_TWL_PCTL_OGNINFO_ALWAYS_MASK;
			}
			else									// レーティング年齢を設定
			{
				rating = OS_TWL_PCTL_OGNINFO_ENABLE_MASK | ages[ index ];
			}
		}
		this->pRomHeader->s.parental_control_rating_info[i] = rating;
	}
}

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
	BOOL            result = false;
	ROM_Header      rh;
	int             pos;
	u8              *privateKey = (u8*)g_devPrivKey_DER;
	u8              *publicKey  = (u8*)g_devPubKey_DER;

	// ROMヘッダのダイジェストを算出(先頭から証明書領域の直前までが対象)
	ACSign_DigestUnit(
		signSrc.digest,
		this->pRomHeader,
		(u32)&(rh.certificate) - (u32)&(rh)		// this->pRomHeader はマネージヒープ上にあるので実アドレスを取得できない
	);

	// 鍵を選ぶ
#ifdef METWL_VER_APPTYPE_LAUNCHER
	if( *this->hIsAppLauncher )
	{
		privateKey = (u8*)g_devPrivKey_DER_launcher;
		publicKey  = (u8*)g_devPubKey_DER_launcher;
	}
	else
#endif //METWL_VER_APPTYPE_LAUNCHER
#ifdef METWL_VER_APPTYPE_SECURE
	if( *this->hIsAppSecure )
	{
		privateKey = (u8*)g_devPrivKey_DER_secure;
		publicKey  = (u8*)g_devPubKey_DER_secure;
	}
	else
#endif //METWL_VER_APPTYPE_SECURE
#ifdef METWL_VER_APPTYPE_SYSTEM
	if( *this->hIsAppSystem )
	{
		privateKey = (u8*)g_devPrivKey_DER_system;
		publicKey  = (u8*)g_devPubKey_DER_system;
	}
	else
#endif //METWL_VER_APPTYPE_SYSTEM
#ifdef METWL_VER_APPTYPE_USER
	{
		privateKey = (u8*)g_devPrivKey_DER;
		publicKey  = (u8*)g_devPubKey_DER;
	}
#endif //METWL_VER_APPTYPE_USER

	// ダイジェストに署名をかける
	result = ACSign_Encrypto( signDst, privateKey, &signSrc, sizeof(SignatureData) ); 
	if( !result )
	{
		return (ECSrlResult::ERROR_SIGN_ENCRYPT);
	}

	// 署名を解除してダイジェストと一致するかベリファイする
	result = ACSign_Decrypto( decryptBlock, publicKey, signDst, RSA_KEY_LENGTH );
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
	u8    *publicKey = (u8*)g_devPubKey_DER;
	ROM_Header rh;

	// <データの流れ>
	// (1) 公開鍵で復号した結果(ブロック)をローカル変数(original)に格納
	// (2) ブロックから余分な部分を取り除いて引数(pDst)にコピー

	// 鍵を選ぶ
#ifdef METWL_VER_APPTYPE_LAUNCHER
	if( *this->hIsAppLauncher )
	{
		publicKey  = (u8*)g_devPubKey_DER_launcher;
	}
	else
#endif //METWL_VER_APPTYPE_LAUNCHER
#ifdef METWL_VER_APPTYPE_SECURE
	if( *this->hIsAppSecure )
	{
		publicKey  = (u8*)g_devPubKey_DER_secure;
	}
	else
#endif //METWL_VER_APPTYPE_SECURE
#ifdef METWL_VER_APPTYPE_SYSTEM
	if( *this->hIsAppSystem )
	{
		publicKey  = (u8*)g_devPubKey_DER_system;
	}
	else
#endif //METWL_VER_APPTYPE_SYSTEM
#ifdef METWL_VER_APPTYPE_USER
	{
		publicKey  = (u8*)g_devPubKey_DER;
	}
#endif //METWL_VER_APPTYPE_USER

	// 署名の解除 = 公開鍵で復号
	if( !ACSign_Decrypto( original, publicKey, this->pRomHeader->signature, RSA_KEY_LENGTH ) )
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
			//System::Diagnostics::Debug::WriteLine( "relstep = " + relstep.ToString() );

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
				revstr = " plus" + rev.ToString();
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
			if( !spl[1]->StartsWith("BACKUP") && !spl[1]->StartsWith("SSL") && !spl[1]->StartsWith("CPS") )
			{
				this->hLicenseList->Add( gcnew RCLicense( spl[0], spl[1] ) );
			}
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
	this->hErrorList->AddRange( this->hParentalErrorList );		// ペアレンタルコントロールのリストは別につくっているので追加
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
	for( i=0; i < TITLE_NAME_MAX; i++ )
	{
		char c = this->pRomHeader->s.title_name[TITLE_NAME_MAX - i - 1];	// 末尾から探索
		if( ((0x21 <= c ) && (c <= 0x5f)) || (c == 0x00) )					// 00hと20h以外の使用可能な文字が出てくるまでにスペースが存在したらダメ
		{
			break;
		}
		else if( c == 0x20 )
		{
			result = false;
		}
	}
	if( !result )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ソフトタイトル", 0x0, 0xb, "末尾の未使用部分には00hを登録してください。",
			"Game Title", "Please use 00h for an unused part.", false, true ) );
	}

	result = true;
	for( i=1; i < TITLE_NAME_MAX; i++ )
	{
		char prev = this->pRomHeader->s.title_name[i-1];
		char curr = this->pRomHeader->s.title_name[i];
		if( (prev == 0x00) && (0x21 <= curr) && (curr <= 0x5f) )	// 途中に00hがあるとダメ
		{
			result = false;
			break;
		}
	}
	if( !result )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ソフトタイトル", 0x0, 0xb, "スペース部分には20hを登録してください。",
			"Game Title", "Please use 20h for an space part.", false, true ) );
	}

	result = true;
	for( i=0; i < GAME_CODE_MAX; i++ )
	{
		char c = this->pRomHeader->s.game_code[i];
		if( (c < 0x20) || (0x5f < c) )		// 00hを許さない
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
		if( (c < 0x20) || (0x5f < c) )
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
	if( *this->hIsMediaNand == false )	// カードアプリのときのみ
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
	System::Boolean bRsv = true;
	for( i=0; i < 8; i++ )
	{
		if( this->pRomHeader->s.ctrl_reserved_B[i] != 0 )
		{
			bRsv = false;
			break;
		}
	}
	if( !bRsv )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ROM情報予約領域", 0x078, 0x07f, "不正な値が含まれています。この領域をすべて0で埋めてください。",
			"Reserved Area for ROM Info.", "Invalid data is included. Please set 0 into this area.", false, true ) );
	}
	bRsv = true;
	for( i=0; i < 32; i++ )
	{
		if( this->pRomHeader->s.reserved_C[i] != 0 )
		{
			bRsv = false;
			break;
		}
	}
	if( !bRsv )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"予約領域C", 0x160, 0x17f, "不正な値が含まれています。この領域をすべて0で埋めてください。",
			"Reserved Area C", "Invalid data is included. Please set 0 into this area.", false, true ) );
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

// TWL専用
ECSrlResult RCSrl::mrcTWL( FILE *fp )
{
	System::Int32 i;

	// ROMヘッダのチェック (NTR互換領域)

#ifdef METWL_WHETHER_PLATFORM_CHECK
	// プラットフォームのチェック
	if( (this->pRomHeader->s.platform_code != PLATFORM_CODE_TWL_HYBLID) && 
		(this->pRomHeader->s.platform_code != PLATFORM_CODE_TWL_LIMITED) &&
		(this->pRomHeader->s.enable_signature == 0) )
	{
		return ECSrlResult::ERROR_PLATFORM;
	}
#endif

	if( *this->hIsOldDevEncrypt && *this->hHasDSDLPlaySign )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"旧開発用暗号フラグ", 0x1c, 0x1c,
			"このROMはクローンブート対応アプリですが、製品用本体ではクローンブートができなくなります。",
			"Old Development Flag",
			"This ROM supports Clone-Boot, and the flag is old type. Therefore, Clone-Boot can't be done.",
			false, true ) );
	}

	// 値チェック

	fseek( fp, 0, SEEK_END );
	u32  filesize = ftell(fp);	// 実ファイルサイズ(単位Mbit)
	u32  romsize = 1 << (this->pRomHeader->s.rom_size);	// ROM容量
	if( *(this->hIsMediaNand) == false )		// カードアプリのときのみのチェック
	{
		u32 filesizeMb = (filesize / (1024*1024)) * 8;
		if( romsize < filesizeMb )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"デバイス容量", 0x14, 0x14, "実ファイルサイズよりも小さい値が指定されています。",
				"Device Capacity", "Setting data is less than the actual file size.", false, true ) );
		}
		else if( filesizeMb < romsize )
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
		if( (filesizeMb < 1) || (MasterEditorTWL::countBits(filesizeMb) != 1) )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"実ファイルサイズ", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "中途半端な値です。通常では2のべき乗[Mbit]の値です。",
				"Actual File Size", "Invalid size. This size is usually power of 2.", false, true ) );
		}
		// 1Gbit以上のときの最終領域が固定値かどうか
		this->mrcPadding( fp );

		// セグメント3のCRC
		u16  crcseg3;
		BOOL crcret = getSeg3CRCInFp( fp, &crcseg3 );
		if( !crcret || (crcseg3 != METWL_SEG3_CRC) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"セグメント3CRC", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, 
				"セグメント3領域に誤りがあります。",
				"System-Call Library", "This Library is SDK default one.", false, true ) );
		}

		u16  NA = this->pRomHeader->s.twl_card_normal_area_rom_offset & 0x7fffUL;
		u16  KA = this->pRomHeader->s.twl_card_keytable_area_rom_offset;
		if( (NA == 0) || (KA == 0) || (NA > KA) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"TWL ROMオフセット", 0x90, 0x93,
				"TWLノーマル領域ROMオフセット(NA)およびTWL専用領域ROMオフセット(KA)はともに1以上で、かつNAはKAを超えてはいけません。",
				"TWL ROM Offset", 
				"Both TWL Normal Area ROM Offset(NA) and TWL Limited Area ROM Offset(KA) are bigger than 0. And NA must be smaller than KA, or equals to KA.",
				false, true ) );
		}
	} //if( *(this->hIsNAND) == false )
	else	// NANDアプリのときのみのチェック
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

	if( *this->hIsAppLauncher || *this->hIsAppSecure || *this->hIsAppSystem )
	{
		if( this->pRomHeader->s.disable_debug == 0 )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"デバッガ動作禁止フラグ", 0x1f, 0x1f, 
				"デバッガ上で解析可能な設定になっています。デバッガ向けソフトとしてROMをリリースする場合にはセキュリティ上の問題がないかご確認ください。",
				"Disable Launch on Debugger Flag", 
				"This ROM can be analyzed on the debugger. If the ROM is released for the debugger, please check the security settings.", 
				false, true ) );
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
	System::Boolean bReserved = true;
	for( i=0; i < 7; i++ )
	{
		if( this->pRomHeader->s.reserved_A[i] != 0 )
		{
			bReserved = false;
			break;
		}
	}
	if( !bReserved )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"予約領域A", 0x015, 0x01b, "不正な値が含まれています。この領域をすべて0で埋めてください。",
			"Reserved Area A", "Invalid data is included. Please set 0 into this area.", false, true ) );
	}
	bReserved = true;
	for( i=0; i < 39; i++ )
	{
		if( this->pRomHeader->s.reserved_B[i] != 0 )
		{
			bReserved = false;
			break;
		}
	}
	if( !bReserved )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"予約領域B", 0x099, 0x0bf, "不正な値が含まれています。この領域をすべて0で埋めてください。",
			"Reserved Area B", "Invalid data is included. Please set 0 into this area.", false, true ) );
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
		System::String ^appstrJ = gcnew System::String("");
		System::String ^appstrE = gcnew System::String("");

		idH = this->pRomHeader->s.titleID_Hi;
		memcpy( idL, &(this->pRomHeader->s.titleID_Lo[0]), 4 );

		if( (idL[3]=='H') && (idL[2]=='N') && (idL[1]=='A') )   // ランチャアプリかどうかはTitleID_Loの値で決定
		{
			apptype = appLauncher;
			appstrJ = "ランチャーアプリ";
			appstrE = "Launcher-App.";
		}
		else if( idH & TITLE_ID_HI_SECURE_FLAG_MASK )           // 立ってたらセキュアアプリ
		{
			apptype = appSecure;
			appstrJ = "セキュアアプリ";
			appstrE = "Secure-App.";
		}
		else if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 1 )       // 立ってたらシステムアプリ
		{
			apptype = appSystem;
			appstrJ = "システムアプリ";
			appstrE = "System-App.";
		}
		else if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 0 )       // 残るはユーザアプリ
		{
			apptype = appUser;
			appstrJ = "ユーザアプリ";
			appstrE = "User-App.";
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

		System::Boolean bApp = false;
#ifdef METWL_VER_APPTYPE_LAUNCHER
		if( apptype == appLauncher )
		{
			bApp = true;
		}
		else
#endif //#ifdef METWL_VER_APPTYPE_LAUNCHER
#ifdef METWL_VER_APPTYPE_SECURE
		if( apptype == appSecure )
		{
			bApp = true;
		}
		else
#endif //#ifdef METWL_VER_APPTYPE_SECURE
#ifdef METWL_VER_APPTYPE_SYSTEM
		if( apptype == appSystem )
		{
			bApp = true;
		}
		else
#endif //#ifdef METWL_VER_APPTYPE_SYSTEM
#ifdef METWL_VER_APPTYPE_USER
		if( apptype == appUser )
		{
			bApp = true;
		}
#endif //#ifdef METWL_VER_APPTYPE_USER

		if( !bApp )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"アプリ種別", 0x230, 0x237, "このROMデータは" + appstrJ + "です。本プログラムでは非対応です。",
				"Application Type", "This ROM is " + appstrE + " which is unsurpported by this editor.", false, true ) );
		}

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
	if( *this->hIsMediaNand == false )	// カードアプリのときのみ
	{
		if( (this->pRomHeader->s.access_control.nand_access != 0) || (this->pRomHeader->s.access_control.sd_card_access != 0) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"アクセスコントロール情報", 0x1b4, 0x1b7,
				"ゲームカード用ソフトは、NANDフラッシュとSDカードへアクセスできません。アクセスが必要な場合は、弊社窓口にご相談ください。",
				"Access Control Info.",
				"Game soft for Game Card does'nt access to NAND frash memory and SD Card. If the soft wish to access them, please contact us.",
				false, true ) );
		}
	}

	if( (*this->hIsWiFiIcon == true) && (*this->hIsWirelessIcon == true) )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"アイコン表示フラグ", 0x1bf, 0x1bf, 
			"ワイヤレス通信アイコンとWi-Fi通信アイコンは同時に表示できません。設定を無視して読み込みます。",
			"Icon Displaying",
			"Icon displayed on menu is either Wireless Icon or Wi-Fi Icon. This setting was ignored in reading.",
			true, true ) );		// 修正可能
	}

	if( (*this->hIsAppLauncher == false) && (*this->hIsSCFGAccess == true) )
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

	bReserved = true;
	for( i=0; i < (0x2f0 - 0x240); i++ )
	{
		if( this->pRomHeader->s.reserved_ltd_F[i] != 0 )
		{
			bReserved = false;
			break;
		}
	}
	if( !bReserved )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"予約領域F", 0x240, 0x2ef, "不正な値が含まれています。この領域をすべて0で埋めてください。",
			"Reserved Area F", "Invalid data is included. Please set 0 into this area.", false, true ) );
	}

	bReserved = true;
	for( i=0; i < (0xf80 - 0x378); i++ )
	{
		u8 *p = (u8*)this->pRomHeader;
		if( p[ 0x378 + i ] != 0 )
		{
			bReserved = false;
			break;
		}
	}
	if( !bReserved )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"予約領域D", 0x378, 0xf7f, "不正な値が含まれています。この領域をすべて0で埋めてください。",
			"Reserved Area D", "Invalid data is included. Please set 0 into this area.", false, true ) );
	}

	// ROMヘッダ以外の領域のチェック

	this->mrcBanner( fp );

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

// パディングのチェック
void RCSrl::mrcPadding(FILE *fp)
{
	u32  romsize = 1 << (this->pRomHeader->s.rom_size);	// ROM容量
	u32  padsize = 0;	// 単位はByte
	System::String ^padstrJ = gcnew System::String("");
	System::String ^padstrE = gcnew System::String("");
	System::String ^romstr  = gcnew System::String("");

	switch( romsize )
	{
		case 1024:
			padsize = 0x00280000;	// 20Mbit = 2.5MB
			padstrJ = "最終20Mbit";
			padstrE = "Last 20Mbit";
			romstr  = "1Gbit";
		break;
		case 2048:
			padsize = 0x00500000;	// 40Mbit = 5MB
			padstrJ = "最終40Mbit";
			padstrE = "Last 40Mbit";
			romstr  = "2Gbit";
		break;
		case 4096:
			padsize = 0x00A60000;	// 83Mbit = 10.375MB
			padstrJ = "最終83Mbit";
			padstrE = "Last 83Mbit";
			romstr  = "4Gbit";
		break;
		default:
			padsize = 0;
		break;
	}
	if( padsize == 0 )
	{
		return;
	}

	fseek( fp, 0, SEEK_END );
	u32  filesize = ftell(fp);	// 実ファイルサイズ(単位Mbit)
	u32  offset = ((romsize / 8) * 1024 * 1024) - padsize;
	if( (offset + padsize) > filesize )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			padstrJ, METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
			"読み出すことができませんでした。",
			padstrE, "This area couldn't be read", false, true ) );
		return;
	}

	u8 *buf = new u8[ padsize ];
	fseek( fp, offset, SEEK_SET );
	if( padsize != fread( buf, 1, padsize, fp ) )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			padstrJ, METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
			"読み出すことができませんでした。",
			padstrE, "This area couldn't be read", false, true ) );
		delete []buf;
		return;
	}

	System::Boolean bResult = true;
	u32 i;
	for( i=0; i < padsize; i++ )
	{
		if( buf[i] != 0xff )
		{
			bResult = false;
		}
	}
	if( !bResult )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			padstrJ, METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
			"FFh以外の値が格納されています。 " + romstr + " ROMの容量制限が守られていません。",
			padstrE,
			"This area includes illegai data. " + romstr + " Volume constraint is violated.",
			false, true ) );
	}
	delete []buf;
}

// バナー
void RCSrl::mrcBanner(FILE *fp)
{
	u8  map[ 0x10000 ];
	u32 i;

	// 本体設定だけはガイドライン無視
	if( memcmp( this->pRomHeader->s.game_code, "HNB", 3 ) == 0 )
	{
		//this->hWarnList->Add( gcnew RCMrcError( 
		//	"バナーファイル", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
		//	"本体設定アプリに限ってはバナーの文字コードチェックをスキップします。",
		//	"Banner File",
		//	"Only a machine setting app., a charactor code check of the banner file is skip.",
		//	false, true ) );
		return;
	}

	// バナーオフセットにエラーがあるときには調べない
	if( this->pRomHeader->s.banner_offset == 0 )
	{
		return;
	}

	// 使用可能な文字のマップをつくる
	// 文字コードをインデックスとして使用可能なら 1 が入る
	// そうでないなら 0 が入る
	for( i=0; i < 0x10000; i++ )
	{
		map[i] = 0;
	}
	for( i=2; ; i+=2 )	// 0個目の要素はマジックコードなので含めない
	{
		u16 index = (u16)gBannerFont[i+1];
		index = (index << 8) + gBannerFont[i];
		if( index == 0xFEFF )
		{
			break;
		}
		map[ index ] = 1;
	}
	map[ 0x0000 ] = 1;		// 0x0000は許される

	// バナーを読み込む
	u32 size = this->pRomHeader->s.banner_size;
	u8 *banner = new u8[size];
	fseek( fp, this->pRomHeader->s.banner_offset, SEEK_SET );
	if( size != fread( banner, 1, size, fp ) )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"バナーファイル", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
			"読み出すことができませんでした。",
			"Banner File",
			"The file can't be read.",
			false, true ) );
	}

	// バナーの各文字がマップに登録されているかチェック
	System::Boolean bResult = true;
	u32  start = 0x240;
	u32  end   = (size < 0x1240)?(size):(0x1240);	// NTR互換領域までのときはTWL拡張領域をサーチしない
	for( i=0x240; i < end; i+=2 )
	{
		if( (0x840 <= i) && (i < 0xA40) )	// 中韓のフォント箇所はチェックしない
		{
			continue;
		}
		u16 index = banner[i+1];
		u16 indexbak = index;
		index = (index << 8) + banner[i];
		if( map[ index ] == 0 )
		{
			//System::Diagnostics::Debug::WriteLine( i.ToString("X") + ":" + index.ToString("X") + ":" + indexbak.ToString("X") );
			bResult = false;
			break;
		}
	}
	if( !bResult )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"バナーファイル", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
			"使用不可能な文字セットが使用されています。",
			"Banner File",
			"A set of illegal charactor code is used.",
			false, true ) );
	}
	delete []banner;
}

// バナーとして使用可能なフォントリスト(u16リトルエンディアン)
const u8 gBannerFont[] =
{
    0xff, 0xfe, 0x20, 0x00, 0x21, 0x00, 0x22, 0x00, 0x23, 0x00, 0x24, 0x00, 0x25, 0x00, 0x26, 0x00, 
    0x27, 0x00, 0x28, 0x00, 0x29, 0x00, 0x2a, 0x00, 0x2b, 0x00, 0x2c, 0x00, 0x2d, 0x00, 0x2e, 0x00, 
    0x2f, 0x00, 0x30, 0x00, 0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00, 0x35, 0x00, 0x36, 0x00, 
    0x37, 0x00, 0x38, 0x00, 0x39, 0x00, 0x3a, 0x00, 0x3b, 0x00, 0x3c, 0x00, 0x3d, 0x00, 0x3e, 0x00, 
    0x3f, 0x00, 0x40, 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 
    0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x4a, 0x00, 0x4b, 0x00, 0x4c, 0x00, 0x4d, 0x00, 0x4e, 0x00, 
    0x4f, 0x00, 0x50, 0x00, 0x51, 0x00, 0x52, 0x00, 0x53, 0x00, 0x54, 0x00, 0x55, 0x00, 0x56, 0x00, 
    0x57, 0x00, 0x58, 0x00, 0x59, 0x00, 0x5a, 0x00, 0x5b, 0x00, 0x5c, 0x00, 0x5d, 0x00, 0x5e, 0x00, 
    0x5f, 0x00, 0x60, 0x00, 0x61, 0x00, 0x62, 0x00, 0x63, 0x00, 0x64, 0x00, 0x65, 0x00, 0x66, 0x00, 
    0x67, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6c, 0x00, 0x6d, 0x00, 0x6e, 0x00, 
    0x6f, 0x00, 0x70, 0x00, 0x71, 0x00, 0x72, 0x00, 0x73, 0x00, 0x74, 0x00, 0x75, 0x00, 0x76, 0x00, 
    0x77, 0x00, 0x78, 0x00, 0x79, 0x00, 0x7a, 0x00, 0x7b, 0x00, 0x7c, 0x00, 0x7d, 0x00, 0x7e, 0x00, 
    0x7f, 0x00, 0xac, 0x20, 0x1a, 0x20, 0x1e, 0x20, 0x26, 0x20, 0xc6, 0x02, 0x52, 0x01, 0x18, 0x20, 
    0x19, 0x20, 0x1c, 0x20, 0x1d, 0x20, 0x22, 0x20, 0xdc, 0x02, 0x22, 0x21, 0x3a, 0x20, 0x53, 0x01, 
    0xa1, 0x00, 0xa2, 0x00, 0xa3, 0x00, 0xa8, 0x00, 0xa9, 0x00, 0xae, 0x00, 0xb7, 0x00, 0xbf, 0x00, 
    0xc0, 0x00, 0xc1, 0x00, 0xc2, 0x00, 0xc3, 0x00, 0xc4, 0x00, 0xc5, 0x00, 0xc6, 0x00, 0xc7, 0x00, 
    0xc8, 0x00, 0xc9, 0x00, 0xca, 0x00, 0xcb, 0x00, 0xcc, 0x00, 0xcd, 0x00, 0xce, 0x00, 0xcf, 0x00, 
    0xd0, 0x00, 0xd1, 0x00, 0xd2, 0x00, 0xd3, 0x00, 0xd4, 0x00, 0xd5, 0x00, 0xd6, 0x00, 0xd8, 0x00, 
    0xd9, 0x00, 0xda, 0x00, 0xdb, 0x00, 0xdc, 0x00, 0xdd, 0x00, 0xdf, 0x00, 0xe0, 0x00, 0xe1, 0x00, 
    0xe2, 0x00, 0xe3, 0x00, 0xe4, 0x00, 0xe5, 0x00, 0xe6, 0x00, 0xe7, 0x00, 0xe8, 0x00, 0xe9, 0x00, 
    0xea, 0x00, 0xeb, 0x00, 0xec, 0x00, 0xed, 0x00, 0xee, 0x00, 0xef, 0x00, 0xf0, 0x00, 0xf1, 0x00, 
    0xf2, 0x00, 0xf3, 0x00, 0xf4, 0x00, 0xf5, 0x00, 0xf6, 0x00, 0xf7, 0x00, 0xf8, 0x00, 0xf9, 0x00, 
    0xfa, 0x00, 0xfb, 0x00, 0xfc, 0x00, 0xfd, 0x00, 0x00, 0x30, 0x01, 0x30, 0x02, 0x30, 0x0c, 0xff, 
    0x0e, 0xff, 0xfb, 0x30, 0x3a, 0x00, 0x3b, 0x00, 0x1f, 0xff, 0x01, 0xff, 0xfc, 0x30, 0x41, 0x30, 
    0x42, 0x30, 0x43, 0x30, 0x44, 0x30, 0x45, 0x30, 0x46, 0x30, 0x47, 0x30, 0x48, 0x30, 0x49, 0x30, 
    0x4a, 0x30, 0x4b, 0x30, 0x4c, 0x30, 0x4d, 0x30, 0x4e, 0x30, 0x4f, 0x30, 0x50, 0x30, 0x51, 0x30, 
    0x52, 0x30, 0x53, 0x30, 0x54, 0x30, 0x55, 0x30, 0x56, 0x30, 0x57, 0x30, 0x58, 0x30, 0x59, 0x30, 
    0x5a, 0x30, 0x5b, 0x30, 0x5c, 0x30, 0x5d, 0x30, 0x5e, 0x30, 0x5f, 0x30, 0x60, 0x30, 0x61, 0x30, 
    0x62, 0x30, 0x63, 0x30, 0x64, 0x30, 0x65, 0x30, 0x66, 0x30, 0x67, 0x30, 0x68, 0x30, 0x69, 0x30, 
    0x6a, 0x30, 0x6b, 0x30, 0x6c, 0x30, 0x6d, 0x30, 0x6e, 0x30, 0x6f, 0x30, 0x70, 0x30, 0x71, 0x30, 
    0x72, 0x30, 0x73, 0x30, 0x74, 0x30, 0x75, 0x30, 0x76, 0x30, 0x77, 0x30, 0x78, 0x30, 0x79, 0x30, 
    0x7a, 0x30, 0x7b, 0x30, 0x7c, 0x30, 0x7d, 0x30, 0x7e, 0x30, 0x7f, 0x30, 0x80, 0x30, 0x81, 0x30, 
    0x82, 0x30, 0x83, 0x30, 0x84, 0x30, 0x85, 0x30, 0x86, 0x30, 0x87, 0x30, 0x88, 0x30, 0x89, 0x30, 
    0x8a, 0x30, 0x8b, 0x30, 0x8c, 0x30, 0x8d, 0x30, 0x8e, 0x30, 0x8f, 0x30, 0x90, 0x30, 0x91, 0x30, 
    0x92, 0x30, 0x93, 0x30, 0x9b, 0x30, 0x9c, 0x30, 0x9d, 0x30, 0x9e, 0x30, 0xa1, 0x30, 0xa2, 0x30, 
    0xa3, 0x30, 0xa4, 0x30, 0xa5, 0x30, 0xa6, 0x30, 0xa7, 0x30, 0xa8, 0x30, 0xa9, 0x30, 0xaa, 0x30, 
    0xab, 0x30, 0xac, 0x30, 0xad, 0x30, 0xae, 0x30, 0xaf, 0x30, 0xb0, 0x30, 0xb1, 0x30, 0xb2, 0x30, 
    0xb3, 0x30, 0xb4, 0x30, 0xb5, 0x30, 0xb6, 0x30, 0xb7, 0x30, 0xb8, 0x30, 0xb9, 0x30, 0xba, 0x30, 
    0xbb, 0x30, 0xbc, 0x30, 0xbd, 0x30, 0xbe, 0x30, 0xbf, 0x30, 0xc0, 0x30, 0xc1, 0x30, 0xc2, 0x30, 
    0xc3, 0x30, 0xc4, 0x30, 0xc5, 0x30, 0xc6, 0x30, 0xc7, 0x30, 0xc8, 0x30, 0xc9, 0x30, 0xca, 0x30, 
    0xcb, 0x30, 0xcc, 0x30, 0xcd, 0x30, 0xce, 0x30, 0xcf, 0x30, 0xd0, 0x30, 0xd1, 0x30, 0xd2, 0x30, 
    0xd3, 0x30, 0xd4, 0x30, 0xd5, 0x30, 0xd6, 0x30, 0xd7, 0x30, 0xd8, 0x30, 0xd9, 0x30, 0xda, 0x30, 
    0xdb, 0x30, 0xdc, 0x30, 0xdd, 0x30, 0xde, 0x30, 0xdf, 0x30, 0xe0, 0x30, 0xe1, 0x30, 0xe2, 0x30, 
    0xe3, 0x30, 0xe4, 0x30, 0xe5, 0x30, 0xe6, 0x30, 0xe7, 0x30, 0xe8, 0x30, 0xe9, 0x30, 0xea, 0x30, 
    0xeb, 0x30, 0xec, 0x30, 0xed, 0x30, 0xee, 0x30, 0xef, 0x30, 0xf0, 0x30, 0xf1, 0x30, 0xf2, 0x30, 
    0xf3, 0x30, 0xf4, 0x30, 0xf5, 0x30, 0xf6, 0x30, 0x01, 0x30, 0x02, 0x30, 0x0c, 0xff, 0x0e, 0xff, 
    0xfb, 0x30, 0x1a, 0xff, 0x1b, 0xff, 0x1f, 0xff, 0x01, 0xff, 0x9b, 0x30, 0x9c, 0x30, 0xb4, 0x00, 
    0x40, 0xff, 0x3e, 0xff, 0xe3, 0xff, 0x3f, 0xff, 0x05, 0x30, 0xfc, 0x30, 0x15, 0x20, 0x10, 0x20, 
    0x0f, 0xff, 0x3c, 0xff, 0x5e, 0xff, 0x5c, 0xff, 0x26, 0x20, 0x18, 0x20, 0x19, 0x20, 0x1c, 0x20, 
    0x1d, 0x20, 0x08, 0xff, 0x09, 0xff, 0x14, 0x30, 0x15, 0x30, 0x3b, 0xff, 0x3d, 0xff, 0x5b, 0xff, 
    0x5d, 0xff, 0x08, 0x30, 0x09, 0x30, 0x0c, 0x30, 0x0d, 0x30, 0x0b, 0xff, 0x0d, 0xff, 0xb1, 0x00, 
    0xd7, 0x00, 0xf7, 0x00, 0x1d, 0xff, 0x1e, 0x22, 0x34, 0x22, 0xb0, 0x00, 0x32, 0x20, 0x33, 0x20, 
    0x06, 0xff, 0x06, 0x26, 0x05, 0x26, 0xcb, 0x25, 0xcf, 0x25, 0xce, 0x25, 0xc7, 0x25, 0xc6, 0x25, 
    0xa1, 0x25, 0xa0, 0x25, 0xb3, 0x25, 0xb2, 0x25, 0xbd, 0x25, 0xbc, 0x25, 0x3b, 0x20, 0x12, 0x30, 
    0x92, 0x21, 0x90, 0x21, 0x91, 0x21, 0x93, 0x21, 0x6f, 0x26, 0x6d, 0x26, 0x6a, 0x26, 0x0a, 0x00, 
	0xff, 0xfe,		// 終わりを表す意味で0xFEFFを最後につける
};
