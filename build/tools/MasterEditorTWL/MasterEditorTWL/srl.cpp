// srl.h �̃N���X����

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
// RCSrl �N���X
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
	// �|�C���^��(���Ԃ�) unmanaged �Ȃ̂Ŏ���I�ɉ������
	delete (this->pRomHeader);
}

//
// ROM�w�b�_���t�@�C�����烊�[�h
//
// @arg [in] ���̓t�@�C����
//
ECSrlResult RCSrl::readFromFile( System::String ^filename )
{
	FILE       *fp = NULL;
	const char *pchFilename = 
		(const char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi( filename ).ToPointer();

	// �t�@�C�����J����ROM�w�b�_�̂ݓǂݏo��
	if( fopen_s( &fp, pchFilename, "rb" ) != NULL )
	{
		return (ECSrlResult::ERROR_FILE_OPEN);
	}
	(void)fseek( fp, 0, SEEK_SET );		// ROM�w�b�_��srl�̐擪����

	// 1�o�C�g��sizeof(~)�������[�h (�t���ƕԂ�l��sizeof(~)�ɂȂ�Ȃ��̂Œ���)
	if( fread( (void*)(this->pRomHeader), 1, sizeof(ROM_Header), fp ) != sizeof(ROM_Header) )
	{
		return (ECSrlResult::ERROR_FILE_READ);
	}
	{
		ECSrlResult      r;

#ifdef METWL_WHETHER_SIGN_DECRYPT
		// �����`�F�b�N
		r = this->decryptRomHeader();
		if( r != ECSrlResult::NOERROR )
		{
			(void)fclose(fp);
			return r;
		}
#endif //#ifdef METWL_WHETHER_SIGN_DECRYPT

		// �t�@�C�������O��ROM�w�b�_�ȊO�̗̈悩��ݒ�����o��
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

	// ROM�w�b�_�̒l��ROM�ŗL���t�B�[���h�ɔ��f������
	(void)this->setRomInfo();

	// ���ׂĐݒ肵�����Ƃ�MRC
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
// ROM�w�b�_���t�@�C���Ƀ��C�g
//
// @arg [in] �o�̓t�@�C����
//
ECSrlResult RCSrl::writeToFile( System::String ^filename )
{
	FILE       *fp = NULL;
	const char *pchFilename = 
		(const char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi( filename ).ToPointer();
	ECSrlResult result;

	// ROM�w�b�_��ROM�ŗL���𔽉f������
	result = this->setRomHeader();
	if( result != ECSrlResult::NOERROR )
	{
		return result;
	}

	// �t�@�C����ROM�w�b�_�����C�g
	if( fopen_s( &fp, pchFilename, "r+b" ) != NULL )	// �㏑���E�o�C�i��
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
// ROM�w�b�_����擾����ROM�ŗL�����t�B�[���h�ɔ��f������
//
ECSrlResult RCSrl::setRomInfo(void)
{
	System::Int32 i;
	System::Text::UTF8Encoding^ utf8 = gcnew System::Text::UTF8Encoding( true );	// char->String�ϊ��ɕK�v

	// NTR�݊����
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

	// �y�A�����^���R���g���[��
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

	// TWL��p���
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
	//val = ((u32)(idL[0]) << 24) | ((u32)(idL[1]) << 16) | ((u32)(idL[2]) << 8) | ((u32)(idL[3]));	// �r�b�O�G���f�B�A��
	//this->hTitleIDLo   = gcnew System::UInt32( val );
	this->hTitleIDLo   = gcnew System::String( (char*)idL, 0, 4, utf8 );
	this->hTitleIDHi   = gcnew System::UInt32( idH );

	// TitleID����킩����
	this->hIsAppLauncher = gcnew System::Boolean( false );
	this->hIsAppUser     = gcnew System::Boolean( false );
	this->hIsAppSystem   = gcnew System::Boolean( false );
	this->hIsAppSecure   = gcnew System::Boolean( false );
	this->hIsLaunch      = gcnew System::Boolean( false );
	this->hIsMediaNand   = gcnew System::Boolean( false );
	this->hIsDataOnly    = gcnew System::Boolean( false );
	if( (idL[3]=='H') && (idL[2]=='N') && (idL[1]=='A') )	// �����`���A�v�����ǂ�����TitleID_Lo�̒l�Ō���
    {
		*(this->hIsAppLauncher) = true;
    }
    else if( idH & TITLE_ID_HI_SECURE_FLAG_MASK )				// �e�r�b�g�͔r���I�Ƃ͌���Ȃ��̂�else if�ɂ͂Ȃ�Ȃ�
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
	if( (idH & TITLE_ID_HI_NOT_LAUNCH_FLAG_MASK) == 0 )		// �r�b�g��0�̂Ƃ�Launch
	{
		*(this->hIsLaunch) = true;
	}
	u16 pub = (u16)((idH & TITLE_ID_HI_PUBLISHER_CODE_MASK) >> TITLE_ID_HI_PUBLISHER_CODE_SHIFT);
	this->hPublisherCode = gcnew System::UInt16( pub );

	// TWL�g���t���O
	this->hIsCodecTWL  = gcnew System::Boolean( (this->pRomHeader->s.exFlags.codec_mode != 0)?true:false );
	this->hIsEULA      = gcnew System::Boolean( (this->pRomHeader->s.exFlags.agree_EULA != 0)?true:false );
	this->hIsSubBanner = gcnew System::Boolean( (this->pRomHeader->s.exFlags.availableSubBannerFile != 0)?true:false );
	this->hIsWiFiIcon  = gcnew System::Boolean( (this->pRomHeader->s.exFlags.WiFiConnectionIcon != 0)?true:false );
	this->hIsWirelessIcon = gcnew System::Boolean( (this->pRomHeader->s.exFlags.DSWirelessIcon != 0)?true:false );
	this->hIsWL        = gcnew System::Boolean( (this->pRomHeader->s.exFlags.enable_nitro_whitelist_signature != 0)?true:false );

	// TWL�A�N�Z�X�R���g���[��
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

	// Shared2�t�@�C���T�C�Y
	this->hShared2SizeArray = gcnew cli::array<System::UInt32^>(METWL_NUMOF_SHARED2FILES);
	for( i=0; i < METWL_NUMOF_SHARED2FILES; i++ )
	{
		this->hShared2SizeArray[i] = gcnew System::UInt32( 0 );
	}
	u32  unit = 16 * 1024;		// 16KB�̏搔���i�[����Ă���
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

	// �J�[�h���[�W����
	const u32  map           = this->pRomHeader->s.card_region_bitmap;
	this->hIsRegionJapan     = gcnew System::Boolean( ((map & METWL_MASK_REGION_JAPAN)     != 0)?true:false );
	this->hIsRegionAmerica   = gcnew System::Boolean( ((map & METWL_MASK_REGION_AMERICA)   != 0)?true:false );
	this->hIsRegionEurope    = gcnew System::Boolean( ((map & METWL_MASK_REGION_EUROPE)    != 0)?true:false );
	this->hIsRegionAustralia = gcnew System::Boolean( ((map & METWL_MASK_REGION_AUSTRALIA) != 0)?true:false );

	return ECSrlResult::NOERROR;
} // ECSrlResult RCSrl::setRomInfo(void)

//
// ROM�w�b�_��ROM�ŗL���t�B�[���h�̒l�𔽉f������
//
ECSrlResult RCSrl::setRomHeader(void)
{
	System::Int32 i;
	ECSrlResult result;

	// ROM�w�b�_��[0,0x160)�̗̈��Read Only�ŕύX���Ȃ�

	// �������̃t���O��ROM�w�b�_�ɔ��f
	this->pRomHeader->s.exFlags.agree_EULA = (*(this->hIsEULA) == true)?1:0;
	this->pRomHeader->s.agree_EULA_version = *(this->hEULAVersion);
	this->pRomHeader->s.exFlags.WiFiConnectionIcon = (*(this->hIsWiFiIcon) == true)?1:0;
	this->pRomHeader->s.exFlags.DSWirelessIcon     = (*(this->hIsWirelessIcon) == true)?1:0;

	// ���[�e�B���O
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
		map |= METWL_MASK_REGION_ALL;	// �I�[�����[�W����������
	}
#endif
	this->pRomHeader->s.card_region_bitmap = map;

	// �y�A�����^���R���g���[��
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

	// ROM�w�b�_��CRC�Ə������X�V����
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
// �w�b�_��CRC���Z�o
//
static u16 calcCRC( const u16 start, const u8 *data, const int allsize )
{
	u16       r1;
	u16       total = start;
	int       size  = allsize;

	// CRC�e�[�u���͌Œ�
	const u16 CRC16Table[16] =
    {
		0x0000, 0xCC01, 0xD801, 0x1400,
		0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401,
		0x5000, 0x9C01, 0x8801, 0x4400
    };

	// CRC�v�Z
	while (size-- > 0)
    {
        // ����4bit
        r1 = CRC16Table[total & 0xf];
        total = (total >> 4) & 0x0fff;
        total = total ^ r1 ^ CRC16Table[*data & 0xf];

        // ���4bit
        r1 = CRC16Table[total & 0xf];
        total = (total >> 4) & 0x0fff;
        total = total ^ r1 ^ CRC16Table[(*data >> 4) & 0xf];

        data++;
    }
	return total;
} // calcRomHeaderCRCCore()

ECSrlResult RCSrl::calcRomHeaderCRC(void)
{
	// ROM�w�b�_��CRC�̈���㏑��
	this->pRomHeader->s.header_crc16 = calcCRC( CRC16_INIT_VALUE, (u8*)this->pRomHeader, CALC_CRC16_SIZE );

	return (ECSrlResult::NOERROR);
} // ECSrlResult RCSrl::calcRomHeaderCRC(void)

//
// ROM�w�b�_���ď���
//
ECSrlResult RCSrl::signRomHeader(void)
{
	SignatureData   signSrc;						// �����̂��ƂƂȂ�_�C�W�F�X�g�l
	u8              signDst[ RSA_KEY_LENGTH ];		// �����̊i�[��Tmp�o�b�t�@
	u8              decryptBlock[ RSA_KEY_LENGTH ];	// ������������u���b�N�o�b�t�@
	BOOL            result;
	ROM_Header      rh;
	int             pos;

	// ROM�w�b�_�̃_�C�W�F�X�g���Z�o(�擪����ؖ����̈�̒��O�܂ł��Ώ�)
	ACSign_DigestUnit(
		signSrc.digest,
		this->pRomHeader,
		(u32)&(rh.certificate) - (u32)&(rh)		// this->pRomHeader �̓}�l�[�W�q�[�v��ɂ���̂Ŏ��A�h���X���擾�ł��Ȃ�
	);

	// �_�C�W�F�X�g�ɏ�����������
	result = ACSign_Encrypto( signDst, g_devPrivKey_DER, &signSrc, sizeof(SignatureData) ); 
	if( !result )
	{
		return (ECSrlResult::ERROR_SIGN_ENCRYPT);
	}

	// �������������ă_�C�W�F�X�g�ƈ�v���邩�x���t�@�C����
	result = ACSign_Decrypto( decryptBlock, g_devPubKey_DER, signDst, RSA_KEY_LENGTH );
	for( pos=0; pos < RSA_KEY_LENGTH; pos++ )
	{
		if( decryptBlock[pos] == 0x0 )			// ������u���b�N������f�[�^���T�[�`
			break;
	}
	if( !result || (memcmp( &signSrc, &(decryptBlock[pos+1]), sizeof(SignatureData) ) != 0) )
	{
		return (ECSrlResult::ERROR_SIGN_DECRYPT);
	}

	// ROM�w�b�_�ɏ������㏑��
	memcpy( this->pRomHeader->signature, signDst, RSA_KEY_LENGTH );

	return (ECSrlResult::NOERROR);
} // ECSrlResult RCSrl::signRomHeader(void)

//
// ROM�w�b�_�̏������O��
//
ECSrlResult RCSrl::decryptRomHeader(void)
{
	u8     original[ RSA_KEY_LENGTH ];	// �����O������̃f�[�^�i�[��
	s32    pos = 0;						// �u���b�N�̐擪�A�h���X
	u8     digest[ DIGEST_SIZE_SHA1 ];	// ROM�w�b�_�̃_�C�W�F�X�g
	ROM_Header rh;

	// <�f�[�^�̗���>
	// (1) ���J���ŕ�����������(�u���b�N)�����[�J���ϐ�(original)�Ɋi�[
	// (2) �u���b�N����]���ȕ�������菜���Ĉ���(pDst)�ɃR�s�[

	// �����̉��� = ���J���ŕ���
	if( !ACSign_Decrypto( original, g_devPubKey_DER, this->pRomHeader->signature, RSA_KEY_LENGTH ) )
	{
		return ECSrlResult::ERROR_SIGN_DECRYPT;
	}
	// �����O�f�[�^�𕜍���u���b�N����Q�b�g
	for( pos=0; pos < (RSA_KEY_LENGTH-2); pos++ )   // �{���u���b�N�̐擪��0x00�����������̓��������ɂ���ď�����d�l
	{
		// �Í��u���b�N�`�� = 0x00, BlockType, Padding, 0x00, ���f�[�^
		if( original[pos] == 0x00 )                               // ���f�[�^�̒��O��0x00���T�[�`
		{
			break;
		}
	}
	// �x���t�@�C
	// ROM�w�b�_�̃_�C�W�F�X�g���Z�o(�擪����ؖ����̈�̒��O�܂ł��Ώ�)
	ACSign_DigestUnit( digest,	this->pRomHeader, (u32)&(rh.certificate) - (u32)&(rh) );
		// this->pRomHeader �̓}�l�[�W�q�[�v��ɂ���̂Ŏ��A�h���X���擾�ł��Ȃ�
	if( memcmp( &(original[pos+1]), digest, DIGEST_SIZE_SHA1 ) != 0 )
	{
		return ECSrlResult::ERROR_SIGN_VERIFY;
	}
	return (ECSrlResult::NOERROR);
}

//
// DS�_�E�����[�h������SRL�Ɋi�[����Ă��邩���ׂ�
//
ECSrlResult RCSrl::hasDSDLPlaySign( FILE *fp )
{
	const s32 DSDLPLAY_SIZE = 0x88;
	u8        buf[ DSDLPLAY_SIZE ];
	u32       offset;

	// �����̈��SRL����ǂݍ���
	offset = this->pRomHeader->s.rom_valid_size;
	if( fseek( fp, offset, SEEK_SET ) != 0 )
	{
		this->hHasDSDLPlaySign = gcnew System::Boolean( false );	// ������NTR��ROM�̈�̍Ō�ɓ���̂�
		return ECSrlResult::ERROR_FILE_READ;						// NTR��SRL�ɏ������i�[����Ă��Ȃ��ꍇ�̓V�[�N�����[�h���ł��Ȃ�
	}
	if( DSDLPLAY_SIZE != fread( buf, 1, DSDLPLAY_SIZE, fp ) )
	{
		this->hHasDSDLPlaySign = gcnew System::Boolean( false );
		return ECSrlResult::ERROR_FILE_READ;
	}

	// �ŏ���2�o�C�g���Œ�̒l�ƂȂ邱�Ƃ��ۏ؂���Ă���̂ł��̒l���ǂ����Ŕ���
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
// SDK�o�[�W�������擾����
//
ECSrlResult RCSrl::searchSDKVersion( FILE *fp )
{
	// SDK�o�[�W������SRL�o�C�i�����Ɉȉ��̃}�W�b�N�i���o�ƂƂ��ɖ��߂��Ă���
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
		// �}�W�b�N�R�[�h�̃I�t�Z�b�g�̎�O4�o�C�g��SDK�o�[�W����
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

			// ���
			System::Byte   major = (System::Byte)(0xff & (sdkcode >> 24));
			System::Byte   minor = (System::Byte)(0xff & (sdkcode >> 16));
			System::UInt16 relstep = (System::UInt16)(0xffff & sdkcode);
			System::String ^str = nullptr;
			str += (major.ToString() + "." + minor.ToString() + " ");

			// RELSTEP�̉���
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
// �g�p���C�Z���X���擾����
//
#define  LICENSE_LEN_MAX   1024		// ����������C�Z���X�̕����񂪒����Ƃ��������擾�ł��Ȃ�
ECSrlResult RCSrl::searchLicenses(FILE *fp)
{
	// ���C�Z���X��SRL�o�C�i������[SDK+(�z�z��):(���C�u������)]�̃t�H�[�}�b�g�Ŗ��߂��Ă���
	const u8 pattern[5] = { '[', 'S', 'D', 'K', '+' };
	System::Collections::Generic::List<u32> ^list;

	this->hLicenseList = gcnew System::Collections::Generic::List<RCLicense ^>;
	this->hLicenseList->Clear();
	
	fseek( fp, 0, SEEK_END );
	const u32 filesize = ftell( fp );

	list = MasterEditorTWL::patternMatch( fp, pattern, 5 );
	if( list == nullptr )
	{
		return ECSrlResult::NOERROR;	// ���C�Z���X���Ȃ��ꍇ�����݂���̂�OK�Ƃ���
	}
	for each( u32 item in list )
	{
		char  buf[ LICENSE_LEN_MAX + 1 ];	// '\0'�̕��������߂ɂƂ��Ă���
		u32   offset = item + 5;			// "[SDK+"�̌ォ�烊�[�h����
		u32   len = ((filesize - offset) < LICENSE_LEN_MAX)?(filesize - offset):LICENSE_LEN_MAX;

		// "(�z�z��):(���C�u������)]"��]���ȕ������܂߂ĂЂƂ܂��o�b�t�@�Ɋi�[
		fseek( fp, offset, SEEK_SET );
		if( len != fread( buf, 1, len, fp ) )
		{
			return ECSrlResult::ERROR;
		}
		buf[ len ] = '\0';	// ���strlen�̐�����ۏ؂��邽��

		// "(�z�z��):(���C�u������)]"�����o���Ă���(�z�z��)��(���C�u������)�ɕ���
		System::Text::UTF8Encoding^ utf8 = gcnew System::Text::UTF8Encoding( true );	// char->String�ϊ��ɕK�v
		System::String ^str = gcnew System::String( buf, 0, strlen(buf), utf8 );		// �T���╪����String���\�b�h�ɗp�ӂ���Ă���
		str = (str->Split( ']' ))[0];							// ']'�����O�����o��
		cli::array<System::String^> ^spl = str->Split( ':' );	// (�z�z��):(���C�u������)�𕪊�����

		if( spl->Length >= 2  )		// �O�̂���
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

// NTR�݊�MRC
ECSrlResult RCSrl::mrcNTR( FILE *fp )
{
	System::Int32  i;
	System::Boolean result;

	// ROM�w�b�_�̃`�F�b�N

	// �����R�[�h�`�F�b�N
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
			"�\�t�g�^�C�g��", 0x0, 0xb, "�g�p�s��ASCII�R�[�h���g�p����Ă��܂��B",
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
			"�C�j�V�����R�[�h", 0xc, 0xf, "�g�p�s��ASCII�R�[�h���g�p����Ă��܂��B",
			"Game Code", "Unusable ASCII code is used.", false, true ) );
	}
	if( memcmp( this->pRomHeader->s.game_code, "NTRJ", GAME_CODE_MAX ) == 0 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"�C�j�V�����R�[�h", 0xc, 0xf, "SDK�f�t�H���g�̃R�[�h(NTRJ)���g�p����Ă��܂��B",
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
			"���[�J�[�R�[�h", 0x10, 0x11, "�g�p�s��ASCII�R�[�h���g�p����Ă��܂��B",
			"Maker Code", "Unusable ASCII code is used.", false, true ) );
	}

	// �l�`�F�b�N

	if( this->pRomHeader->s.rom_type != 0x00 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"�f�o�C�X�^�C�v", 0x13, 0x13, "�s���Ȓl�ł��B00h��ݒ肵�Ă��������B",
			"Device Type", "Invalid data. Please set 00h.", false, true ) );
	}

	u8 romver = this->pRomHeader->s.rom_version;
	if( ((romver < 0x00) || (0x09 < romver)) && (romver != 0xE0) )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"���}�X�^�[�o�[�W����", 0x1e, 0x1e, "�s���Ȓl�ł��B�����łł�01h-09h�̂����ꂩ�̒l�A���O�łł�E0h�ł��B",
			"Release Ver.", "Invalid data. Please set either one of 01h-09h(Regular ver.), or E0h(Preliminary ver.)", false, true ) );
	}
	if( this->pRomHeader->s.banner_offset == 0 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"�o�i�[�I�t�Z�b�g", 0x68, 0x6b, "�o�i�[�f�[�^���ݒ肳��Ă��܂���B",
			"Banner Offset.", "Banner data is not set.", false, true ) );
	}
	if( this->pRomHeader->s.rom_valid_size == 0 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ROM�����T�C�Y", 0x80, 0x83, "�l���ݒ肳��Ă��܂���B",
			"ROM Valid Size.", "Data is not set.", false, true ) );
	}

	// CRC
	u16  crc;
	// �Z�L���A�̈�
	{
		// �Z�L���A�̈��ROM�w�b�_�O
		u8     *secures;
		s32     secure_size = SECURE_AREA_END - this->pRomHeader->s.main_rom_offset;
		if (secure_size > SECURE_AREA_SIZE)
		{
			secure_size = SECURE_AREA_SIZE;
		}
		if (secure_size <= 0)
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�Z�L���A�̈�CRC", 0x15e, 0x15f, "�Z�L���A�̈�̃A�h���X�w�肪�s���ł��B",
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
				"�Z�L���A�̈�CRC", 0x07c, 0x07d, "�v�Z���ʂƈ�v���܂���B�Z�L���A�̈悪�����񂳂ꂽ�\��������܂��B",
				"Secure Area CRC.", "Calclated CRC is different from Registered one.", false, true ) );
		}
	}

	// ���S�̈�
	crc = 0xcf56;
	if( crc != this->pRomHeader->s.nintendo_logo_crc16 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"�L�����N�^�[�f�[�^CRC", 0x15c, 0x15d, "�v�Z���ʂƈ�v���܂���B�L�����N�^�[�f�[�^�������񂳂ꂽ�\��������܂��B",
			"Charactor Data CRC.", "Calclated CRC is different from Registered one.", false, true ) );
	}

	// �w�b�_CRC
	crc = calcCRC( CRC16_INIT_VALUE, (u8*)this->pRomHeader, CALC_CRC16_SIZE );
	if( crc != this->pRomHeader->s.header_crc16 )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ROM�w�b�_CRC", 0x15e, 0x15f, "�v�Z���ʂƈ�v���܂���BROM�w�b�_�������񂳂ꂽ�\��������܂��B",
			"ROM Header CRC.", "Calclated CRC is different from Registered one.", false, true ) );
	}

	// �\��̈�
	for( i=0; i < 8; i++ )
	{
		if( this->pRomHeader->s.ctrl_reserved_B[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"ROM���\��̈�", 0x078, 0x07f, "�s���Ȓl���܂܂�Ă��܂��B���̗̈�����ׂ�0�Ŗ��߂Ă��������B",
				"Reserved Area for ROM Info.", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}
	for( i=0; i < 32; i++ )
	{
		if( this->pRomHeader->s.reserved_C[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�\��̈�C", 0x078, 0x07f, "�s���Ȓl���܂܂�Ă��܂��B���̗̈�����ׂ�0�Ŗ��߂Ă��������B",
				"Reserved Area C", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}

	// ROM�w�b�_�ȊO�̗̈�̃`�F�b�N (�t�@�C������K�X���[�h����)
	
	// �V�X�e���R�[�����C�u����
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
	if( i == 32 )	// �S��0
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"�V�X�e���R�[�����C�u����", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "SDK�f�t�H���g�ł��B",
			"System-Call Library", "This Library is SDK default one.", false, true ) );
	}

	return ECSrlResult::NOERROR;
} // mrcNTR()

ECSrlResult RCSrl::mrcTWL( FILE *fp )
{
	System::Int32 i;

	// ROM�w�b�_�̃`�F�b�N (NTR�݊��̈�)

#ifdef METWL_WHETHER_PLATFORM_CHECK
	// �v���b�g�t�H�[���̃`�F�b�N
	if( (this->pRomHeader->s.platform_code != PLATFORM_CODE_TWL_HYBLID) && 
		(this->pRomHeader->s.platform_code != PLATFORM_CODE_TWL_LIMITED) )
	{
		return ECSrlResult::ERROR_PLATFORM;
	}
#endif

	// �l�`�F�b�N

	fseek( fp, 0, SEEK_END );
	u32  filesize = ftell(fp);	// ���t�@�C���T�C�Y(�P��Mbit)
	u32  romsize = 1 << (this->pRomHeader->s.rom_size);	// ROM�e��
	if( *(this->hIsMediaNand) == false )		// �J�[�h�A�v���̂Ƃ��݂̂̃`�F�b�N
	{
		if( (romsize*1024*1024/8) < filesize )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�f�o�C�X�e��", 0x14, 0x14, "���t�@�C���T�C�Y�����������l���w�肳��Ă��܂��B",
				"Device Capacity", "Setting data is less than the actual file size.", false, true ) );
		}
		else if( filesize < (romsize*1024*1024/8) )
		{
			this->hWarnList->Add( gcnew RCMrcError(		// �x��
				"�f�o�C�X�e��", 0x14, 0x14, "���t�@�C���T�C�Y�ɔ�ׂĖ��ʂ̂���l���ݒ肳��Ă��܂��B",
				"Device Capacity", "Setting data is larger than the actual file size.", false, true ) );
		}
		if( (romsize < METWL_ROMSIZE_MIN) || (METWL_ROMSIZE_MAX < romsize) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�f�o�C�X�e��", 0x14, 0x14, "�w��\�ȗe�ʂł͂���܂���B",
				"Device Capacity", "Invalid capacity.", false, true ) );
		}
		if( (filesize % 2) != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"���t�@�C���T�C�Y", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "���r���[�Ȓl�ł��B�ʏ�ł�2�ׂ̂���̒l�ł��B",
				"Actual File Size", "Invalid size. This size is usually power of 2.", false, true ) );
		}
	} //if( *(this->hIsNAND) == false )
	else
	{
		if( (romsize < METWL_ROMSIZE_MIN_NAND) || (METWL_ROMSIZE_MAX_NAND < romsize) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�f�o�C�X�e��", 0x14, 0x14, "NAND�A�v���ɑ΂��Ďw��\�ȗe�ʂł͂���܂���B",
				"Device Capacity", "Invalid capacity.", false, true ) );
		}
		u32  allsizeMB = filesize + this->pRomHeader->s.public_save_data_size + this->pRomHeader->s.private_save_data_size;
		if( allsizeMB > METWL_ALLSIZE_MAX_NAND )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"���t�@�C���T�C�Y", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
				"ROM�f�[�^�̎��t�@�C���T�C�Y��Public�Z�[�u�f�[�^�����Private�Z�[�u�f�[�^�̃T�C�Y�̑��a��32MByte�𒴂��Ă��܂��B",
				"Actual File Size", 
				"The sum of this size, the public save data size and private save data size exceed 32MByte.", false, true ) );
		}
	}

	if( this->pRomHeader->s.warning_no_spec_rom_speed != 0 )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"���t���O", 0x1f, 0x1f, "rsf�t�@�C����ROMSpeedType���ݒ肳��Ă��܂���B",
			"Setting Flags", "In a RSF file, the item \"ROMSpeedType\" is not set.", false, true ) );
	}
	if( (this->pRomHeader->s.game_cmd_param & CARD_LATENCY_MASK) != CARD_1TROM_GAME_LATENCY )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ROM�R���g���[�����", 0x60, 0x67, "TWL�ł̓}�X�NROM�͗p�ӂ���Ă��܂���B�����^�C��PROM�ݒ�ɂ��Ă��������B",
			"ROM Control Info.", "Mask ROM can be set. Please set One-time PROM.", false, true ) );
	}

	// �\��̈�
	for( i=0; i < 7; i++ )
	{
		if( this->pRomHeader->s.reserved_A[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�\��̈�A", 0x015, 0x01b, "�s���Ȓl���܂܂�Ă��܂��B���̗̈�����ׂ�0�Ŗ��߂Ă��������B",
				"Reserved Area A", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}
	for( i=0; i < 39; i++ )
	{
		if( this->pRomHeader->s.reserved_B[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�\��̈�B", 0x099, 0x0bf, "�s���Ȓl���܂܂�Ă��܂��B���̗̈�����ׂ�0�Ŗ��߂Ă��������B",
				"Reserved Area B", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}

	// ROM�w�b�_�̃`�F�b�N (TWL��p�̈�)

	// �l�`�F�b�N
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

		if( (idL[3]=='H') && (idL[2]=='N') && (idL[1]=='A') )   // �����`���A�v�����ǂ�����TitleID_Lo�̒l�Ō���
		{
			apptype = appLauncher;
		}
		else if( idH & TITLE_ID_HI_SECURE_FLAG_MASK )           // �����Ă���Z�L���A�A�v��
		{
			apptype = appSecure;
		}
		else if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 1 )       // �����Ă���V�X�e���A�v��
		{
			apptype = appSystem;
		}
		else if( (idH & TITLE_ID_HI_APP_TYPE_MASK) == 0 )       // �c��̓��[�U�A�v��
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
				"�A�v�����", 0x230, 0x237, "�s���Ȓl�ł��B",
				"Application Type", "Illigal type.", false, true ) );
		}
#ifdef METWL_VER_APPTYPE_USER
		if( apptype != appUser )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"�A�v�����", 0x230, 0x237, "���[�U�A�v���ł͂���܂���B",
				"Application Type", "Not USER application.", false, true ) );
		}
#endif
#ifdef METWL_VER_APPTYPE_SYSTEM
		if( apptype != appSystem )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"�A�v�����", 0x230, 0x237, "�V�X�e���A�v���ł͂���܂���B",
				"Application Type", "Not SYSTEM application.", false, true ) );
		}
#endif
#ifdef METWL_VER_APPTYPE_SECURE
		if( apptype != appSecure )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"�A�v�����", 0x230, 0x237, "�Z�L���A�A�v���ł͂���܂���B",
				"Application Type", "Not SECURE application.", false, true ) );
		}
#endif
#ifdef METWL_VER_APPTYPE_LAUNCHER
		if( apptype != appLauncher )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"�A�v�����", 0x230, 0x237, "�����`���[�A�v���ł͂���܂���B",
				"Application Type", "Not LAUNCHER application.", false, true ) );
		}
#endif
	} // �A�v����ʂ̃`�F�b�N

	if( (this->pRomHeader->s.access_control.game_card_on != 0) &&
		(this->pRomHeader->s.access_control.game_card_nitro_mode != 0) )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"�A�N�Z�X�R���g���[�����", 0x1b4, 0x1b7, "�Q�[���J�[�h�d���ݒ�Ƀm�[�}�����[�h��NTR���[�h�̗�����ݒ肷�邱�Ƃ͂ł��܂���B",
			"Access Control Info.", "Game card power setting is either normal mode or NTR mode.", false, true ) );
	}
	if( ((this->pRomHeader->s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK) == 0) &&		// �J�[�h�A�v��
		((this->pRomHeader->s.access_control.game_card_on != 0) || (this->pRomHeader->s.access_control.game_card_nitro_mode != 0)) )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"�A�N�Z�X�R���g���[�����", 0x1b4, 0x1b7, "�Q�[���J�[�h�p�\�t�g�ɑ΂��ăQ�[���J�[�h�d���ݒ�����邱�Ƃ͂ł��܂���B",
			"Access Control Info.", "Game card power setting is not for Game Card Soft.", false, true ) );
	}
	if( this->pRomHeader->s.access_control.shared2_file == 0 )
	{
		if( (this->pRomHeader->s.shared2_file0_size != 0) || (this->pRomHeader->s.shared2_file1_size != 0) ||
			(this->pRomHeader->s.shared2_file2_size != 0) || (this->pRomHeader->s.shared2_file3_size != 0) ||
			(this->pRomHeader->s.shared2_file4_size != 0) || (this->pRomHeader->s.shared2_file5_size != 0) )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�A�N�Z�X�R���g���[�����", 0x1b4, 0x1b7, "Shared2�t�@�C���̃T�C�Y���ݒ肳��Ă���ɂ�������炸�s�g�p�ݒ�ɂȂ��Ă��܂��B",
				"Access Control Info.", "Sizes of shared2 files is setting, but using them is not enabled.", false, true ) );
		}
	}

	if( (this->pRomHeader->s.arm7_scfg_ext >> 31) != 0 )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"SCFG�ݒ�", 0x1b8, 0x1bb, "SCFG���W�X�^�փA�N�Z�X�\�ɂȂ��Ă��܂��B",
			"SCFG Setting", "In this setting, SCFG register is accessible.", false, true ) );
	}

	if( (this->pRomHeader->s.titleID_Lo[0] != this->pRomHeader->s.game_code[3]) ||
		(this->pRomHeader->s.titleID_Lo[1] != this->pRomHeader->s.game_code[2]) ||
		(this->pRomHeader->s.titleID_Lo[2] != this->pRomHeader->s.game_code[1]) ||
		(this->pRomHeader->s.titleID_Lo[3] != this->pRomHeader->s.game_code[0]) )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"�^�C�g��ID", 0x230, 0x233, "����4�o�C�g���C�j�V�����R�[�h�ƈ�v���܂���B",
			"Title ID", "Lower 4 bytes don't match ones of Game Code.", false, true ) );
	}

	for( i=0; i < (0x2f0 - 0x240); i++ )
	{
		if( this->pRomHeader->s.reserved_ltd_F[i] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�\��̈�F", 0x240, 0x2ef, "�s���Ȓl���܂܂�Ă��܂��B���̗̈�����ׂ�0�Ŗ��߂Ă��������B",
				"Reserved Area F", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}

	for( i=0; i < (0xf80 - 0x378); i++ )
	{
		u8 *p = (u8*)this->pRomHeader;
		if( p[ 0x378 + i ] != 0 )
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�\��̈�D", 0x378, 0xf7f, "�s���Ȓl���܂܂�Ă��܂��B���̗̈�����ׂ�0�Ŗ��߂Ă��������B",
				"Reserved Area D", "Invalid data is included. Please set 0 into this area.", false, true ) );
		}
	}

	// ROM�w�b�_�ȊO�̗̈�̃`�F�b�N

	ECSrlResult pctl = this->mrcTWLParentalControl();
	if( pctl != ECSrlResult::NOERROR )
	{
		return pctl;
	}

	// �ǉ��`�F�b�N
	if( *(this->hMrcSpecialList->hIsCheck) == true )
	{
		// SDK�o�[�W����
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
				"SDK�o�[�W����", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "�{�v���O�����ɓo�^����Ă���o�[�W�������ƈ�v���܂���B",
				"SDK Version", "The data doesn't match one registered in this program.", false, true ) );
		}

		// EULA�o�[�W����
		if( *(this->hEULAVersion) != *(this->hMrcSpecialList->hEULAVer) )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"EULA �o�[�W����", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, "�{�v���O�����ɓo�^����Ă���o�[�W�������ƈ�v���܂���B",
				"EULA Version", "The data doesn't match one registered in this program.", false, true ) );
		}

		// Shared2�t�@�C���T�C�Y
		for( i=0; i < METWL_NUMOF_SHARED2FILES; i++ )
		{
			if( *(this->hShared2SizeArray[i]) > 0 )	// 0�͖��g�p��\���̂Ń`�F�b�N���Ȃ�
			{
				if( *(this->hShared2SizeArray[i]) != *(this->hMrcSpecialList->hShared2SizeArray[i]) )
				{
					this->hWarnList->Add( gcnew RCMrcError( 
						"Shared2�t�@�C��" + i.ToString(), METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE, 
						"�{�v���O�����ɓo�^����Ă���t�@�C���T�C�Y���ƈ�v���܂���B",
						"Shared2 file" + i.ToString(), "The size doesn't match one registered in this program.", false, true ) );
				}
			}
		}

	} //if( *(this->hMrcSpecialList->hIsCheck) )

	return ECSrlResult::NOERROR;
} // mrcTWL()

// PCTL��p
ECSrlResult RCSrl::mrcTWLParentalControl(void)
{
	// ���[�W���������킩
	u32 region = this->pRomHeader->s.card_region_bitmap;
	if( !this->mrcRegion( region ) )
	{
		return ECSrlResult::NOERROR;	// ����ȏ�`�F�b�N���Ȃ�
	}

	// ���[�W�����Ɋ܂܂�Ă���c��/�܂܂�Ă��Ȃ��c�̂̐ݒ���`�F�b�N
	this->mrcRegionOrganization( region );

	// ���[�W�����Ɋ܂܂�Ă���c�̂̐ݒ肪���������ǂ���
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

// PCTL�`�F�b�N�̈ꕔ ���[�W���������킩�`�F�b�N
System::Boolean RCSrl::mrcRegion( System::UInt32 region )
{
	System::Boolean bRegionResult = true;	// ���[�W�����ݒ肪����ł��邩
	if( (region != METWL_MASK_REGION_JAPAN)  && (region != METWL_MASK_REGION_AMERICA) &&
		(region != METWL_MASK_REGION_EUROPE) &&	(region != METWL_MASK_REGION_AUSTRALIA) &&
		(region != (METWL_MASK_REGION_EUROPE | METWL_MASK_REGION_AUSTRALIA)) )
	{
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
		if( region != METWL_MASK_REGION_ALL )	// �I�[�����[�W����������
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�J�[�h���[�W����", 0x1b0, 0x1b3, "�d���n�̑g�ݍ��킹���s���ł��B�{�c�[����p���ďC���ł��܂��B",
				"Card Region", "Illigal Region. This tool can modify this information.", true, true ) );
			bRegionResult = false;
		}
#else
		{
			this->hErrorList->Add( gcnew RCMrcError( 
				"�J�[�h���[�W����", 0x1b0, 0x1b3, "�d���n�̑g�ݍ��킹���s���ł��B�{�c�[����p���ďC�����Ă��������B",
				"Card Region", "Illigal Region. Please modify this information using this tool.", true, true ) );
			bRegionResult = false;
		}
#endif
	}
	// ���[�W�������s���̂Ƃ����ׂĂ�PCTL�����N���A(ROM�w�b�_�ɂ͉e�����Ȃ�)
	if( !bRegionResult )
	{
		int i;
		for( i=0; i < PARENTAL_CONTROL_INFO_SIZE; i++ )
		{
			this->clearParentalControl( i );
		}
		this->hWarnList->Add( gcnew RCMrcError( 
			"�y�A�����^���R���g���[�����", 0x2f0, 0x2ff, 
			"�d���n�̐ݒ肪�s���̂��߁AROM�f�[�^���̂��ׂẴ��[�e�B���O�c�̂̏��𖳎����ēǂݍ��݂܂����B�{�c�[����p���ďC�����Ă��������B",
			"Parental Control", 
			"Illigal region. In reading, therefore, settings for all rating organizations are ignored. Please set this infomation using this tool.", 
			true, true ) );
	}
	return bRegionResult;
}

// PCTL�`�F�b�N�̈ꕔ ���[�W�����Ɋ܂܂��c��/�܂܂�Ȃ��c�̂̐ݒ���`�F�b�N
void RCSrl::mrcRegionOrganization( System::UInt32 region )
{
	// �\���p�̒c�̃��x��
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
	System::String ^warnEmptyJ = gcnew System::String( "���[�e�B���O�R���̕K�v���Ȃ��\�t�g���Ƃ݂Ȃ��܂��B�R�����K�v�ł���ꍇ�ɂ́A�{�c�[����p���ďC�����Ă��������B" );
	System::String ^warnEmptyE = gcnew System::String( "This ROM is not necessary for CERO rating. If necessary, please set this infomation using this tool." );
	System::String ^warnFillJ  = gcnew System::String( "���[�W�����Ɋ܂܂�Ȃ��c�̂̏�񂪐ݒ肳��Ă������߁A�����̒c�̂̏��𖳎����ēǂݍ��݂܂����B�{�c�[����p���ďC�����Ă��������B" );
	System::String ^warnFillE  = gcnew System::String( "Some organizations is not include in the region. In reading, therefore, settings for all rating organizations are ignored. Please set this infomation using this tool." );

	// ���[�W�����Ɋ܂܂��c�̂Ɗ܂܂�Ȃ��c�̂����X�g������
	System::Collections::Generic::List<System::Byte> ^inList = gcnew System::Collections::Generic::List<System::Byte>();
	inList->Clear();
	System::Collections::Generic::List<System::Byte> ^exList = gcnew System::Collections::Generic::List<System::Byte>();
	exList->Clear();

	switch( region )
	{
		case METWL_MASK_REGION_JAPAN:
			inList->Add( OS_TWL_PCTL_OGN_CERO );	// �܂܂�郊�X�g
			exList->Add( OS_TWL_PCTL_OGN_ESRB );	// �܂܂�Ȃ����X�g
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

	// ���[�W�����Ɋ܂܂��c�̂ɉ����ݒ肳��Ă��Ȃ��Ƃ��_��
	for each ( System::Byte ogn in inList )
	{
		if( this->pRomHeader->s.parental_control_rating_info[ ogn ] == 0 )
		{
			this->hWarnList->Add( gcnew RCMrcError( 
				"�y�A�����^���R���g���[�����", 0x2f0, 0x2ff, ognArray[ogn] + warnEmptyJ,
				"Parental Control", ognArray[ogn] + warnEmptyE, true, true ) );
			this->clearParentalControl( ogn );
		}
	}
	// ���[�W�����Ɋ܂܂�Ȃ��c�̂ɉ����ݒ肳��Ă�����_��
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
			"�y�A�����^���R���g���[�����", 0x2f0, 0x2ff, warnFillJ,
			"Parental Control", warnFillE, true, true ) );
	}
} //mrcRegion

// ���[�e�B���O�l�����킩�ǂ����`�F�b�N
void RCSrl::mrcRating( System::Byte ogn )
{
	System::String ^warnDisableJ = gcnew System::String( "�����������ł���ɂ�������炸���[�e�B���O���ݒ肳��Ă����߁A���[�e�B���O�𖳎����ēǂݍ��݂܂����B�{�c�[����p���ďC�����Ă��������B" );
	System::String ^warnDisableE = gcnew System::String( "Rating can't be set when control isn't enable. rating setting is ignored. Please set this infomation using this tool." );
	System::String ^warnIllegalJ = gcnew System::String( "���[�e�B���O��񂪕s���ł��邽�߁A�������ēǂݍ��݂܂����B�{�c�[����p���ďC�����Ă��������B" );
	System::String ^warnIllegalE = gcnew System::String( "Illegal rating. rating setting is ignored. Please set this information using this tool." );
	System::String ^warnPendingJ = gcnew System::String( "Rating Pending���w�肳��Ă��܂��B���̎w��ƃ��[�e�B���O�l�͓����ɐݒ�ł��Ȃ����߁A���[�e�B���O�l�𖳎����ēǂݍ��݂܂����B" );
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
				"�y�A�����^���R���g���[�����", 0x2f0, 0x2ff, ognArray[ ogn ] + warnDisableJ,
				"Parental Control", ognArray[ ogn ] + warnDisableE, true, true ) );
			this->clearParentalControl( ogn );
		}
	}
	else
	{
		// �Ԉ���Ă�����"���R��"���Ԃ��Ă���
		System::String ^str = MasterEditorTWL::transRatingToString( 
			ogn, true, *(this->hArrayParentalRating[ogn]), false );

		if( *(this->hArrayParentalAlways[ogn]) == false )
		{
			// RP�������Ă��Ȃ��Ƃ����[�e�B���O�l���ݒ肳��Ă��Ȃ�������_��
			if( (str != nullptr) && str->Equals( L"���R��" ) )
			{
				this->hWarnList->Add( gcnew RCMrcError( 
					"�y�A�����^���R���g���[�����", 0x2f0, 0x2ff, ognArray[ ogn ] + warnIllegalJ,
					"Parental Control", ognArray[ ogn ] + warnIllegalE, true, true ) );
				this->clearParentalControl( ogn );
			}
		}
		else
		{
			// RP�������Ă�����ⓚ���p�Ɍx��
			this->hWarnList->Add( gcnew RCMrcError( 
				"�y�A�����^���R���g���[�����", 0x2f0, 0x2ff, ognArray[ ogn ] + warnPendingJ,
				"Parental Control", ognArray[ ogn ] + warnPendingE, true, true ) );
			this->clearParentalControl( ogn );
		}
	}
} //mrcRating()

// �y�A�����^���R���g���[���̃N���A
void RCSrl::clearParentalControl( System::Byte ogn )
{
	this->hArrayParentalRating[ ogn ] = gcnew System::Byte( 0x00 );
	this->hArrayParentalEffect[ ogn ] = gcnew System::Boolean( false );
	this->hArrayParentalAlways[ ogn ] = gcnew System::Boolean( false );
}
