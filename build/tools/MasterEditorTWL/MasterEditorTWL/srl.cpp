// srl.h �̃N���X����

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
// RCSrl �N���X
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
	// �t�@�C�������O��ROM�w�b�_�ȊO�̗̈悩��ݒ�����o��
	{
		//ECSrlResult      r;
		(void)this->hasDSDLPlaySign( fp );
		//if( r != ECSrlResult::NOERROR )
		//{
		//	(void)fclose(fp);
		//	return r;
		//}
	}
	(void)fclose( fp );

	// ROM�w�b�_�̒l��ROM�ŗL���t�B�[���h�ɔ��f������
	(void)this->setRomInfo();

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
	u32  val;
	val = ((u32)(idL[0]) << 24) | ((u32)(idL[1]) << 16) | ((u32)(idL[2]) << 8) | ((u32)(idL[3]));	// �r�b�O�G���f�B�A��
	this->hTitleIDLo   = gcnew System::UInt32( val );
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
    if( idH & TITLE_ID_HI_SECURE_FLAG_MASK )				// �e�r�b�g�͔r���I�Ƃ͌���Ȃ��̂�else if�ɂ͂Ȃ�Ȃ�
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
	if( (idH & TITLE_ID_HI_NOT_LAUNCH_FLAG_MASK) == 0 )		// �r�b�g��0�̂Ƃ�Launch
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
	this->hShared2Size0 = gcnew System::UInt32( 0 );
	this->hShared2Size1 = gcnew System::UInt32( 0 );
	this->hShared2Size2 = gcnew System::UInt32( 0 );
	this->hShared2Size3 = gcnew System::UInt32( 0 );
	this->hShared2Size4 = gcnew System::UInt32( 0 );
	this->hShared2Size5 = gcnew System::UInt32( 0 );
	u32  unit = 16 * 1024;		// 16KB�̏搔���i�[����Ă���
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

	// �J�[�h���[�W����
	const u32  maskJapan     = 0x00000001;
	const u32  maskAmerica   = 0x00000002;
	const u32  maskEurope    = 0x00000004;
	const u32  maskAustralia = 0x00000008;
	const u32  maskChina     = 0x00000010;
	const u32  maskKorea     = 0x00000020;
	const u32  map           = this->pRomHeader->s.card_region_bitmap;
	this->hIsRegionJapan     = gcnew System::Boolean( ((map & maskJapan)     != 0)?true:false );
	this->hIsRegionAmerica   = gcnew System::Boolean( ((map & maskAmerica)   != 0)?true:false );
	this->hIsRegionEurope    = gcnew System::Boolean( ((map & maskEurope)    != 0)?true:false );
	this->hIsRegionAustralia = gcnew System::Boolean( ((map & maskAustralia) != 0)?true:false );
	//this->hIsRegionChina     = gcnew System::Boolean( ((map & maskChina)     != 0)?true:false );
	//this->hIsRegionKorea     = gcnew System::Boolean( ((map & maskKorea)     != 0)?true:false );

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
#define     HEADER_CRC16_INIT_VALUE          (0xffff)
#define     HEADER_CALC_CRC16_SIZE           (0x15e)
ECSrlResult RCSrl::calcRomHeaderCRC(void)
{
	u16       r1;
	u16       total = HEADER_CRC16_INIT_VALUE;
	int       size  = HEADER_CALC_CRC16_SIZE;
	const u8 *data  = (u8*)this->pRomHeader;

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

	// ROM�w�b�_��CRC�̈���㏑��
	this->pRomHeader->s.header_crc16 = total;

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
