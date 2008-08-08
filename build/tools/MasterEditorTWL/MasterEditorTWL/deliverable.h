#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include "srl.h"

namespace MasterEditorTWL
{
	// -------------------------------------------------------------------
	// Type : enum class
	// Name : ECDeliverableResult
	//
	// Description : RCDeliverable �N���X�̑���ł̃G���[��錾
	// -------------------------------------------------------------------
	enum class ECDeliverableResult
	{
		NOERROR   = 0,
		// �G���[���肵�Ȃ��Ă��������킩��Ƃ��̕Ԃ�l
		// (�G���[��������\���̂���ӏ���1�� etc.)
		ERROR,
		// �t�@�C������ł̃G���[
		ERROR_FILE_OPEN,
		ERROR_FILE_READ,
		ERROR_FILE_WRITE,
	};

	// -------------------------------------------------------------------
	// Type : ref class
	// Name : RCDeliverable
	//
	// Description : ��o���ރN���X
	// 
	// Role : ��o���̓��o��
	// -------------------------------------------------------------------
	ref class RCDeliverable
	{
		// field
	public:
		// ��o���
		property System::String ^hProductName;		// ���i��
		property System::String ^hProductCode1;		// ���i�R�[�h
		property System::String ^hProductCode2;		// ���i�R�[�h
		property System::Int32  ^hReleaseYear;		// �����\���
		property System::Int32  ^hReleaseMonth;
		property System::Int32  ^hReleaseDay;
		property System::Int32  ^hSubmitYear;		// ��o��
		property System::Int32  ^hSubmitMonth;
		property System::Int32  ^hSubmitDay;
		property System::String ^hSubmitWay;		// ��o���@
		property System::String ^hUsage;			// �p�r
		property System::String ^hUsageOther;		// ���̑��̗p�r
		property System::Int32  ^hSubmitVersion;	// ��o�o�[�W����
		property System::String ^hSDK;				// SDK�o�[�W����
		property System::Boolean ^hReleaseForeign;	// �C�O�ł̗\��
		property System::String  ^hProductNameForeign;
		property System::String  ^hProductCode1Foreign;
		property System::String  ^hProductCode2Foreign;

		// ��Џ��

		// �S����(1�l��)
		property System::String  ^hCompany1;		// ��Ж�
		property System::String  ^hDepart1;			// ������
		property System::String  ^hPerson1;			// ���O
		property System::String  ^hFurigana1;		// �ӂ肪��
		property System::String  ^hTel1;			// �d�b�ԍ�
		property System::String  ^hFax1;			// FAX�ԍ�
		property System::String  ^hMail1;			// ���A�h
		property System::String  ^hNTSC1;			// NTSC User ID
		// �S����(2�l��)
		property System::Boolean ^hIsPerson2;		// 2�l�ڏ�����͂�����
		property System::String  ^hCompany2;
		property System::String  ^hDepart2;
		property System::String  ^hPerson2;
		property System::String  ^hFurigana2;
		property System::String  ^hTel2;
		property System::String  ^hFax2;
		property System::String  ^hMail2;
		property System::String  ^hNTSC2;

		// �v���O�������Ȑ\���d�l
		property System::Boolean ^hIsWireless;			// ���C�����X�ʐM�Ή�
		property System::Boolean ^hIsTouch;				// �^�b�`�X�N���[���Ή�
		property System::Boolean ^hIsMic;				// �}�C�N�Ή�
		property System::Boolean ^hIsWiFi;				// Wi-Fi�Ή�
		property System::Boolean ^hIsGBACartridge;		// GBA�J�[�g���b�W�Ή�
		property System::Boolean ^hIsDSCartridge;			// DS�J�[�h�Ή�
		property System::Boolean ^hIsSoftReset;			// �\�t�g���Z�b�g�@�\����
		property System::Boolean ^hIsPictoChatSearch;	// �s�N�g�`���b�g�T�[�`����
		property System::Boolean ^hIsClock;				// ���v�@�\�g�p
		property System::Boolean ^hIsAutoBackLightOff;	// �����o�b�N���C�gOFF�@�\�g�p
		property System::Int32   ^hTimeAutoBackLightOff;// ...................���鎞��
		property System::Boolean ^hIsAutoLcdOff;		// ����LCDOFF�@�\���g�p
		property System::Int32   ^hTimeAutoLcdOff;		// ................���鎞��
		property System::Boolean ^hIsSleepMode;			// �X���[�v���[�h�Ή�
		property System::Boolean ^hIsNotSleepClose;		// �{�̂���Ă��X���[�v���[�h�Ɉڍs���Ȃ��ꍇ����
		property System::Int32   ^hTimeSleepClose;		// ........................................����
		property System::Boolean ^hIsSleepAlarm;		// RTC�A���[���ŕ��A����ꍇ����
		property System::String  ^hProcSleepAlarm;		// .........................�̏������e
		property System::Boolean ^hIsIPLUserComment;	// IPL�̃��[�U������уR�����g�g�p
		property System::String  ^hSceneIPLUserComment;	// ��L���g�p���Ă�����(������œ���)
		property System::Boolean ^hIsAllIPLFonts;		// IPL�Őݒ�\�ȃt�H���g�����ׂĕ\���ł���

		// �v���O�������Ȑ\���d�l2
		property System::Boolean ^hIsLangJ;				// �Q�[�����ł̎g�p����
		property System::Boolean ^hIsLangE;
		property System::Boolean ^hIsLangF;
		property System::Boolean ^hIsLangG;
		property System::Boolean ^hIsLangI;
		property System::Boolean ^hIsLangS;
		property System::Boolean ^hIsLangC;
		property System::Boolean ^hIsLangK;
		property System::Boolean ^hIsLangOther;
		property System::String  ^hLangOther;
		property System::Boolean ^hIsIPLLang;			// IPL�̌���ݒ�

		// �g�p���C�Z���X
		property System::Boolean ^hUseLcFont;			// LC�t�H���g(SHARP)
		property System::Boolean ^hUseVx;				// VX Middleware(Actimagine)
		property System::Boolean ^hUseAtok;				// ATOK(JUSTSYSTEM)
		property System::Boolean ^hUseVoiceChat;		// VoiceChat(Abiosso)
		property System::Boolean ^hUseWiFiLib;			// WiFi���C�u����(NINTENDO)
		property System::Boolean ^hUseVoiceRecog;		// �����F��(����)
		property System::Boolean ^hUseCharRecog;		// �����F��(Zi)
		property System::Boolean ^hUseVoiceCombine;		// ��������(SHARP)
		property System::Boolean ^hUseNetFront;			// NetFront Browser(ACCESS)
		property System::String  ^hUseOthers;			// ���̑�(������œ���)

		// ���l
		property System::String  ^hCaption;

		// ROM�w�b�_�s�L�ڂ�ROM�o�C�i��(SRL)�ŗL���
		property System::String  ^hBackupMemory;		// �o�b�N�A�b�v�������̎��

		// constructor and destructor
	public:

		// method
	public:

		//
		// ���ޏo��
		//
		// @arg [out] �o�̓t�@�C����
		// @arg [in]  ROM�o�C�i��(SRL)�ŗL���
		// @arg [in]  �t�@�C���S�̂�CRC
		// @arg [in]  SRL�̃t�@�C����(���ނɋL�q���邽�߂Ɏg�p)
		// @arg [in]  �p��t���O
		//
		ECDeliverableResult writeSpreadsheet( 
			System::String ^hFilename, RCSrl ^hSrl, System::UInt16 ^hCRC, System::String ^hSrlFilename, System::Boolean english );

	}; // end of ref class RCDeliverable

} // end of namespace MasterEditorTWL
