#pragma once

// ROM�f�[�^(SRL)�N���X�Ɗ֘A�N���X�̐錾

#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/ownerInfoEx.h>

namespace MasterEditorTWL
{
	// -------------------------------------------------------------------
	// Type : enum class
	// Name : ECSrlResult
	//
	// Description : RCSrl�N���X�̑���ł̃G���[��錾
	// -------------------------------------------------------------------
	enum class ECSrlResult
	{
		NOERROR   = 0,
		// �G���[���肵�Ȃ��Ă��������킩��Ƃ��̕Ԃ�l
		// (�G���[��������\���̂���ӏ���1�� etc.)
		ERROR,
		// �t�@�C������ł̃G���[
		ERROR_FILE_OPEN,
		ERROR_FILE_READ,
		ERROR_FILE_WRITE,
		// �����ł̃G���[
		ERROR_SIGN_ENCRYPT,
		ERROR_SIGN_DECRYPT,
		// CRC�Z�o�ł̃G���[
		ERROR_SIGN_CRC,
	};

	// -------------------------------------------------------------------
	// Type : ref class
	// Name : RCSrl
	//
	// Description : ROM�f�[�^(SRL)�̐ݒ���N���X
	// 
	// Role : ROM�f�[�^�̃t�@�C�����o�́E�������̍X�V
	// -------------------------------------------------------------------
	ref class RCSrl
	{
		// field
	private:
		// ROM�w�b�_
		ROM_Header *pRomHeader;

	public:
		// (GUI�ɕ\�������)ROM�w�b�_�ŗL���

		// NTR�݊���� ReadOnly
		property System::String  ^hTitleName;
		property System::String  ^hGameCode;
		property System::String  ^hMakerCode;
		property System::String  ^hPlatform;
		property System::String  ^hRomSize;
		//property System::Byte    ^hForKorea;
		//property System::Byte    ^hForChina;
		property System::Byte    ^hRomVersion;
		property System::UInt16  ^hHeaderCRC;
		property System::String  ^hLatency;		// MROM/1TROM/Illegal

		// �y�A�����^���R���g���[��
		property cli::array<System::Byte^>    ^hArrayParentalRating;	// �e�c�̂ł̐����N��
		property cli::array<System::Boolean^> ^hArrayParentalEffect;	// �����L���t���O
		property cli::array<System::Boolean^> ^hArrayParentalAlways;	// ���������L���t���O

		// TWL��p��� �ꕔ�ҏW�\
		property System::UInt32  ^hNormalRomOffset;
		property System::UInt32  ^hKeyTableRomOffset;
		property System::Byte    ^hEULAVersion;		// �ҏW�\
		property System::UInt32  ^hTitleIDLo;
		property System::UInt32  ^hTitleIDHi;
		property System::Boolean ^hIsAppLauncher;	// TitleIDLo����킩��A�v�����
		property System::Boolean ^hIsAppUser;		// TitleIDHi����킩��A�v�����
		property System::Boolean ^hIsAppSystem;		//
		property System::Boolean ^hIsAppSecure;		//
		property System::Boolean ^hIsLaunch;		//
		property System::Boolean ^hIsMediaNand;		//
		property System::Boolean ^hIsDataOnly;		//
		property System::UInt16  ^hPublisherCode;	//
		property System::UInt32  ^hPublicSize;
		property System::UInt32  ^hPrivateSize;
		property System::Boolean ^hIsNormalJump;
		property System::Boolean ^hIsTmpJump;
		property System::Boolean ^hHasDSDLPlaySign;	// ROM�w�b�_�O��SRL����킩�鏐���̗L��

		// TWL�g���t���O �ꕔ�ҏW�\
		property System::Boolean ^hIsCodecTWL;
		property System::Boolean ^hIsEULA;			// �ҏW�\
		property System::Boolean ^hIsSubBanner;
		property System::Boolean ^hIsWiFiIcon;		// �ҏW�\
		property System::Boolean ^hIsWirelessIcon;	// �ҏW�\
		property System::Boolean ^hIsWL;

		// TWL�A�N�Z�X�R���g���[�� Read Only
		property System::Boolean ^hIsCommonClientKey;
		property System::Boolean ^hIsAesSlotBForES;
		property System::Boolean ^hIsAesSlotCForNAM;
		property System::Boolean ^hIsSD;
		property System::Boolean ^hIsNAND;
		property System::Boolean ^hIsGameCardOn;
		property System::Boolean ^hIsShared2;
		property System::Boolean ^hIsAesSlotBForJpegEnc;
		property System::Boolean ^hIsGameCardNitro;
		property System::Boolean ^hIsAesSlotAForSSL;
		property System::Boolean ^hIsCommonClientKeyForDebugger;

		// Shared2�t�@�C���T�C�Y Read Only
		property System::UInt32  ^hShared2Size0;
		property System::UInt32  ^hShared2Size1;
		property System::UInt32  ^hShared2Size2;
		property System::UInt32  ^hShared2Size3;
		property System::UInt32  ^hShared2Size4;
		property System::UInt32  ^hShared2Size5;

		// �J�[�h���[�W���� Read Only
		property System::Boolean ^hIsRegionJapan;
		property System::Boolean ^hIsRegionAmerica;
		property System::Boolean ^hIsRegionEurope;
		property System::Boolean ^hIsRegionAustralia;
		//property System::Boolean ^hRegionChina;
		//property System::Boolean ^hRegionKorea;

		// constructor and destructor
	public:
		RCSrl();
		~RCSrl();

		// method
	public:

		//
		// ROM�w�b�_�̃t�@�C�����o��
		//
		// @arg [in/out] ���o�̓t�@�C����
		//
		ECSrlResult readFromFile ( System::String ^filename );
		ECSrlResult writeToFile( System::String ^filename );

		// internal method
	private:
		// ROM�ŗL����ROM�w�b�_�̐ݒ�
		ECSrlResult setRomInfo(void);		// ROM�w�b�_����擾����ROM�ŗL�����t�B�[���h�ɔ��f������
		ECSrlResult setRomHeader(void);		// ROM�w�b�_��ROM�ŗL���t�B�[���h�̒l�𔽉f������

		// ROM�w�b�_�̍X�V
		ECSrlResult calcRomHeaderCRC(void);	// ROM�w�b�_��CRC���Čv�Z
		ECSrlResult signRomHeader(void);	// ROM�w�b�_�X�V��̍ď���

		// SRL�o�C�i���������Ȑݒ�𒲂ׂ�
		ECSrlResult hasDSDLPlaySign( FILE *fp );
				// DS�_�E�����[�h������SRL�Ɋi�[����Ă��邩���ׂ�
				// @arg [in]  ���̓t�@�C����FP (->SRL�ǂݍ��ݎ��Ɏ��s�����ׂ�)

	}; // end of ref class RCSrl

} // end of namespace MasterEditorTWL
