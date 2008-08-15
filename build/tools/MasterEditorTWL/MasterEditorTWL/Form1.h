#pragma once

#include "srl.h"
#include "deliverable.h"
#include "crc_whole.h"

namespace MasterEditorTWL {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Form1 �̊T�v
	///
	/// �x��: ���̃N���X�̖��O��ύX����ꍇ�A���̃N���X���ˑ����邷�ׂĂ� .resx �t�@�C���Ɋ֘A�t����ꂽ
	///          �}�l�[�W ���\�[�X �R���p�C�� �c�[���ɑ΂��� 'Resource File Name' �v���p�e�B��
	///          �ύX����K�v������܂��B���̕ύX���s��Ȃ��ƁA
	///          �f�U�C�i�ƁA���̃t�H�[���Ɋ֘A�t����ꂽ���[�J���C�Y�ς݃��\�[�X�Ƃ��A
	///          ���������݂ɗ��p�ł��Ȃ��Ȃ�܂��B
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	// �Ǝ��ǉ��t�B�[���h
	private:
		// SRL���(ROM�w�b�_���܂�)
		RCSrl ^hSrl;

		// �}�X�^����
		RCDeliverable ^hDeliv;

		// ���ޏo�̓��[�h(�m�[�}��XML or XML Spread Sheet)
		System::Boolean ^hIsSpreadSheet;

	// VC�����ǉ��t�B�[���h
	private: System::Windows::Forms::GroupBox^  gboxCRC;
	private: System::Windows::Forms::TextBox^  tboxWholeCRC;
	private: System::Windows::Forms::Button^  butMakeMaster;
	private: System::Windows::Forms::Label^  labTitleName;
	private: System::Windows::Forms::Label^  labGameCode;
	private: System::Windows::Forms::TextBox^  tboxGameCode;
	private: System::Windows::Forms::Label^  labMakerCode;
	private: System::Windows::Forms::TextBox^  tboxMakerCode;
	private: System::Windows::Forms::Label^  labRomType;
	private: System::Windows::Forms::ComboBox^  combBackup;
	private: System::Windows::Forms::Label^  labRomSize;
	private: System::Windows::Forms::TextBox^  tboxRomLatency;
	private: System::Windows::Forms::Label^  labBackup;
	private: System::Windows::Forms::Label^  labPlatform;
	private: System::Windows::Forms::Label^  labRomCRC;
	private: System::Windows::Forms::Label^  labHeaderCRC;
	private: System::Windows::Forms::TextBox^  tboxHeaderCRC;
	private: System::Windows::Forms::TextBox^  tboxBackupOther;
	private: System::Windows::Forms::Label^  labCaption;
	private: System::Windows::Forms::TextBox^  tboxCaption;
	private: System::Windows::Forms::GroupBox^  gboxSelectLang;
	private: System::Windows::Forms::RadioButton^  rSelectE;
	private: System::Windows::Forms::RadioButton^  rSelectJ;
	private: System::Windows::Forms::TextBox^  tboxRomSize;




























































	private: System::Windows::Forms::Label^  labPEGIBBFC2;
	private: System::Windows::Forms::Label^  labOFLC;
	private: System::Windows::Forms::Label^  labPEGIBBFC;
	private: System::Windows::Forms::Label^  labPEGIPRT;
	private: System::Windows::Forms::Label^  labPEGI;
	private: System::Windows::Forms::Label^  labUSK;
	private: System::Windows::Forms::Label^  labESRB;
	private: System::Windows::Forms::Label^  labCERO;
	private: System::Windows::Forms::CheckBox^  cboxAlwaysOFLC;
	private: System::Windows::Forms::CheckBox^  cboxAlwaysPEGIBBFC;
	private: System::Windows::Forms::CheckBox^  cboxAlwaysPEGIPRT;
	private: System::Windows::Forms::CheckBox^  cboxAlwaysPEGI;
	private: System::Windows::Forms::CheckBox^  cboxAlwaysUSK;
	private: System::Windows::Forms::CheckBox^  cboxAlwaysESRB;
	private: System::Windows::Forms::CheckBox^  cboxAlwaysCERO;
	private: System::Windows::Forms::ComboBox^  combOFLC;
	private: System::Windows::Forms::ComboBox^  combPEGIBBFC;
	private: System::Windows::Forms::ComboBox^  combPEGIPRT;
	private: System::Windows::Forms::ComboBox^  combPEGI;
	private: System::Windows::Forms::ComboBox^  combUSK;
	private: System::Windows::Forms::ComboBox^  combESRB;
	private: System::Windows::Forms::ComboBox^  combCERO;
	private: System::Windows::Forms::CheckBox^  cboxOFLC;
	private: System::Windows::Forms::CheckBox^  cboxPEGIBBFC;
	private: System::Windows::Forms::CheckBox^  cboxPEGIPRT;
	private: System::Windows::Forms::CheckBox^  cboxPEGI;
	private: System::Windows::Forms::CheckBox^  cboxUSK;
	private: System::Windows::Forms::CheckBox^  cboxESRB;
	private: System::Windows::Forms::CheckBox^  cboxCERO;
	private: System::Windows::Forms::Label^  labParentalForceEnable;
	private: System::Windows::Forms::Label^  labParentalRating;
	private: System::Windows::Forms::Label^  labParentalEnable;


































	private: System::Windows::Forms::GroupBox^  gboxTWLInfoWritable;
	private: System::Windows::Forms::CheckBox^  cboxIsEULA;
	private: System::Windows::Forms::CheckBox^  cboxIsWiFiIcon;
	private: System::Windows::Forms::Label^  labEULA;
	private: System::Windows::Forms::CheckBox^  cboxIsWirelessIcon;
	private: System::Windows::Forms::NumericUpDown^  numEULA;























	private: System::Windows::Forms::CheckBox^  cboxIsInputPerson2;
	private: System::Windows::Forms::GroupBox^  gboxPerson2;
	private: System::Windows::Forms::Label^  labNTSC2Sur;
	private: System::Windows::Forms::TextBox^  tboxNTSC2;
	private: System::Windows::Forms::Label^  labNTSC2Pre;
	private: System::Windows::Forms::TextBox^  tboxFax2;
	private: System::Windows::Forms::TextBox^  tboxMail2;
	private: System::Windows::Forms::TextBox^  tboxTel2;
	private: System::Windows::Forms::TextBox^  tboxFurigana2;
	private: System::Windows::Forms::TextBox^  tboxPerson2;
	private: System::Windows::Forms::TextBox^  tboxDepart2;
	private: System::Windows::Forms::Label^  labDepart2;
	private: System::Windows::Forms::TextBox^  tboxCompany2;
	private: System::Windows::Forms::Label^  labMail2;
	private: System::Windows::Forms::Label^  labFax2;
	private: System::Windows::Forms::Label^  labTel2;
	private: System::Windows::Forms::Label^  labFurigana2;
	private: System::Windows::Forms::Label^  labPerson2;
	private: System::Windows::Forms::Label^  labCompany2;
	private: System::Windows::Forms::GroupBox^  gboxPerson1;
	private: System::Windows::Forms::Label^  labNTSC1Sur;
	private: System::Windows::Forms::Label^  labNTSC1Pre;
	private: System::Windows::Forms::TextBox^  tboxNTSC1;
	private: System::Windows::Forms::TextBox^  tboxFax1;
	private: System::Windows::Forms::TextBox^  tboxMail1;
	private: System::Windows::Forms::TextBox^  tboxTel1;
	private: System::Windows::Forms::TextBox^  tboxFurigana1;
	private: System::Windows::Forms::TextBox^  tboxPerson1;
	private: System::Windows::Forms::TextBox^  tboxDepart1;
	private: System::Windows::Forms::Label^  labDepart1;
	private: System::Windows::Forms::TextBox^  tboxCompany1;
	private: System::Windows::Forms::Label^  labMail1;
	private: System::Windows::Forms::Label^  labFax1;
	private: System::Windows::Forms::Label^  labTel1;
	private: System::Windows::Forms::Label^  labFurigana1;
	private: System::Windows::Forms::Label^  labPerson1;
	private: System::Windows::Forms::Label^  labCompany1;

	private: System::Windows::Forms::TextBox^  tboxRemasterVer;
	private: System::Windows::Forms::TextBox^  tboxProductCode2;

	private: System::Windows::Forms::TextBox^  tboxProductCode1;
	private: System::Windows::Forms::TextBox^  tboxProductName;









	private: System::Windows::Forms::Label^  labProductCode2;
	private: System::Windows::Forms::Label^  labProductCode1;
	private: System::Windows::Forms::DateTimePicker^  dateSubmit;
	private: System::Windows::Forms::DateTimePicker^  dateRelease;

	private: System::Windows::Forms::CheckBox^  cboxRemasterVerE;
	private: System::Windows::Forms::Label^  labCapSubmitVer;
	private: System::Windows::Forms::NumericUpDown^  numSubmitVersion;
	private: System::Windows::Forms::Label^  labSubmitVer;
	private: System::Windows::Forms::Label^  labRemasterVer;


	private: System::Windows::Forms::GroupBox^  gboxUsage;
	private: System::Windows::Forms::TextBox^  tboxUsageOther;
	private: System::Windows::Forms::RadioButton^  rUsageOther;
	private: System::Windows::Forms::RadioButton^  rUsageDst;
	private: System::Windows::Forms::RadioButton^  rUsageSample;
	private: System::Windows::Forms::RadioButton^  rUsageSale;

	private: System::Windows::Forms::GroupBox^  gboxSubmitWay;
	private: System::Windows::Forms::RadioButton^  rSubmitHand;
	private: System::Windows::Forms::RadioButton^  rSubmitPost;

	private: System::Windows::Forms::Label^  labSubmiteDate;
	private: System::Windows::Forms::Label^  labReleaseDate;
	private: System::Windows::Forms::Label^  labProductCode;
	private: System::Windows::Forms::Label^  labProductName;
private: System::Windows::Forms::ComboBox^  combRegion;



















private: System::Windows::Forms::Label^  labRegion;
private: System::Windows::Forms::TabControl^  tabDoc;

private: System::Windows::Forms::TabPage^  tabSubmitInfo;
private: System::Windows::Forms::TabPage^  tabForeignInfo;





private: System::Windows::Forms::Label^  labProductCode2Foreign;
private: System::Windows::Forms::CheckBox^  cboxReleaseForeign;
private: System::Windows::Forms::Label^  labProductNameForeign;
private: System::Windows::Forms::TextBox^  tboxProductNameForeign;
private: System::Windows::Forms::Label^  labProductCode1Foreign;
private: System::Windows::Forms::TextBox^  tboxProductCode1Foreign;
private: System::Windows::Forms::Label^  labProductCodeForeign;
private: System::Windows::Forms::TextBox^  tboxProductCode2Foreign1;



private: System::Windows::Forms::TabPage^  tabCompanyInfo1;
private: System::Windows::Forms::TabPage^  tabCompanyInfo2;
private: System::Windows::Forms::TabPage^  tabCaption;
private: System::Windows::Forms::TabPage^  tabTWLSpec4;





private: System::Windows::Forms::GroupBox^  gboxShared2Size;
private: System::Windows::Forms::Label^  labShared2Size5;
private: System::Windows::Forms::Label^  labShared2Size4;
private: System::Windows::Forms::Label^  labShared2Size3;
private: System::Windows::Forms::Label^  labShared2Size2;
private: System::Windows::Forms::Label^  labShared2Size1;
private: System::Windows::Forms::Label^  labShared2Size0;
private: System::Windows::Forms::Label^  labKB5;
private: System::Windows::Forms::Label^  labKB4;
private: System::Windows::Forms::Label^  labKB3;
private: System::Windows::Forms::Label^  labKB2;
private: System::Windows::Forms::Label^  labKB1;
private: System::Windows::Forms::Label^  labKB0;
private: System::Windows::Forms::TextBox^  tboxShared2Size5;
private: System::Windows::Forms::TextBox^  tboxShared2Size4;
private: System::Windows::Forms::TextBox^  tboxShared2Size3;
private: System::Windows::Forms::TextBox^  tboxShared2Size2;
private: System::Windows::Forms::TextBox^  tboxShared2Size1;
private: System::Windows::Forms::TextBox^  tboxShared2Size0;
private: System::Windows::Forms::CheckBox^  cboxIsShared2;
private: System::Windows::Forms::TabPage^  tabSDK;
private: System::Windows::Forms::Label^  labLib;
private: System::Windows::Forms::TextBox^  tboxSDK;
private: System::Windows::Forms::Label^  labSDK;
private: System::Windows::Forms::TabPage^  tabTWLSpec3;
private: System::Windows::Forms::GroupBox^  gboxTWLExInfo;


private: System::Windows::Forms::Label^  labByte2;
private: System::Windows::Forms::Label^  labByte1;
private: System::Windows::Forms::Label^  labHex4;
private: System::Windows::Forms::Label^  labHex3;
private: System::Windows::Forms::TextBox^  tboxIsCodec;
private: System::Windows::Forms::Label^  labIsCodec;
private: System::Windows::Forms::Label^  labNormalRomOffset;
private: System::Windows::Forms::TextBox^  tboxNormalRomOffset;
private: System::Windows::Forms::CheckBox^  cboxIsSubBanner;
private: System::Windows::Forms::Label^  labKeyTableRomOffset;
private: System::Windows::Forms::CheckBox^  cboxIsWL;
private: System::Windows::Forms::TextBox^  tboxPrivateSize;
private: System::Windows::Forms::Label^  labPrivateSize;
private: System::Windows::Forms::TextBox^  tboxKeyTableRomOffset;
private: System::Windows::Forms::CheckBox^  cboxIsNormalJump;
private: System::Windows::Forms::CheckBox^  cboxIsTmpJump;
private: System::Windows::Forms::Label^  labPublicSize;
private: System::Windows::Forms::TextBox^  tboxPublicSize;
private: System::Windows::Forms::TabPage^  tabTWLSpec2;

private: System::Windows::Forms::GroupBox^  gboxAccess;
private: System::Windows::Forms::Label^  labAccessOther;
private: System::Windows::Forms::TextBox^  tboxAccessOther;
private: System::Windows::Forms::TextBox^  tboxIsGameCardOn;
private: System::Windows::Forms::Label^  labIsGameCardOn;
private: System::Windows::Forms::CheckBox^  cboxIsNAND;
private: System::Windows::Forms::CheckBox^  cboxIsSD;
private: System::Windows::Forms::TabPage^  tabTWLSpec1;


private: System::Windows::Forms::GroupBox^  gboxTitleID;
private: System::Windows::Forms::Label^  labHex2;
private: System::Windows::Forms::Label^  labHex1;
private: System::Windows::Forms::TextBox^  tboxTitleIDLo;
private: System::Windows::Forms::Label^  labTitleIDLo;
private: System::Windows::Forms::Label^  labTitleIDHi;
private: System::Windows::Forms::TextBox^  tboxTitleIDHi;
private: System::Windows::Forms::TextBox^  tboxAppType;
private: System::Windows::Forms::Label^  labAppType;
private: System::Windows::Forms::Label^  labCaptionEx;
private: System::Windows::Forms::TextBox^  tboxCaptionEx;
private: System::Windows::Forms::TabControl^  tabCheck;

private: System::Windows::Forms::TextBox^  tboxLib;
private: System::Windows::Forms::TextBox^  tboxProductCode2Foreign3;

private: System::Windows::Forms::TextBox^  tboxProductCode2Foreign2;
private: System::Windows::Forms::Label^  labMultiForeign1;
private: System::Windows::Forms::Label^  labMultiForeign2;
private: System::Windows::Forms::Label^  labCautionInput;
private: System::Windows::Forms::Label^  labCautionCheck;



























	private: System::Windows::Forms::TextBox^  tboxPlatform;

	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: �����ɃR���X�g���N�^ �R�[�h��ǉ����܂�
			//
			this->hSrl   = gcnew (RCSrl);
			this->hDeliv = gcnew (RCDeliverable);

			// �f�t�H���g�l
			this->hIsSpreadSheet = gcnew System::Boolean( true );
			this->dateRelease->Value = System::DateTime::Now;
			this->dateSubmit->Value  = System::DateTime::Now;

			this->loadInit();	// �ݒ�t�@�C���̓ǂݍ���
		}

	protected:
		/// <summary>
		/// �g�p���̃��\�[�X�����ׂăN���[���A�b�v���܂��B
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::TextBox^  tboxFile;
	protected: 
	private: System::Windows::Forms::Button^  butOpen;
	private: System::Windows::Forms::TextBox^  tboxMsg;
	private: System::Windows::Forms::GroupBox^  gboxSrl;
	private: System::Windows::Forms::TextBox^  tboxTitleName;
	private: System::Windows::Forms::GroupBox^  gboxFileOpen;
	private: System::Windows::Forms::Button^  butSaveAs;

	private:
		/// <summary>
		/// �K�v�ȃf�U�C�i�ϐ��ł��B
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// �f�U�C�i �T�|�[�g�ɕK�v�ȃ��\�b�h�ł��B���̃��\�b�h�̓��e��
		/// �R�[�h �G�f�B�^�ŕύX���Ȃ��ł��������B
		/// </summary>
		void InitializeComponent(void)
		{
			this->tboxFile = (gcnew System::Windows::Forms::TextBox());
			this->butOpen = (gcnew System::Windows::Forms::Button());
			this->tboxMsg = (gcnew System::Windows::Forms::TextBox());
			this->gboxSrl = (gcnew System::Windows::Forms::GroupBox());
			this->labBackup = (gcnew System::Windows::Forms::Label());
			this->combBackup = (gcnew System::Windows::Forms::ComboBox());
			this->tboxBackupOther = (gcnew System::Windows::Forms::TextBox());
			this->tboxRemasterVer = (gcnew System::Windows::Forms::TextBox());
			this->tboxRomSize = (gcnew System::Windows::Forms::TextBox());
			this->tboxPlatform = (gcnew System::Windows::Forms::TextBox());
			this->labPlatform = (gcnew System::Windows::Forms::Label());
			this->tboxRomLatency = (gcnew System::Windows::Forms::TextBox());
			this->labRomSize = (gcnew System::Windows::Forms::Label());
			this->labRomType = (gcnew System::Windows::Forms::Label());
			this->tboxMakerCode = (gcnew System::Windows::Forms::TextBox());
			this->cboxRemasterVerE = (gcnew System::Windows::Forms::CheckBox());
			this->labMakerCode = (gcnew System::Windows::Forms::Label());
			this->labGameCode = (gcnew System::Windows::Forms::Label());
			this->tboxGameCode = (gcnew System::Windows::Forms::TextBox());
			this->labTitleName = (gcnew System::Windows::Forms::Label());
			this->labRemasterVer = (gcnew System::Windows::Forms::Label());
			this->tboxTitleName = (gcnew System::Windows::Forms::TextBox());
			this->gboxCRC = (gcnew System::Windows::Forms::GroupBox());
			this->labRomCRC = (gcnew System::Windows::Forms::Label());
			this->labHeaderCRC = (gcnew System::Windows::Forms::Label());
			this->tboxHeaderCRC = (gcnew System::Windows::Forms::TextBox());
			this->tboxWholeCRC = (gcnew System::Windows::Forms::TextBox());
			this->gboxFileOpen = (gcnew System::Windows::Forms::GroupBox());
			this->butSaveAs = (gcnew System::Windows::Forms::Button());
			this->butMakeMaster = (gcnew System::Windows::Forms::Button());
			this->labCaption = (gcnew System::Windows::Forms::Label());
			this->tboxCaption = (gcnew System::Windows::Forms::TextBox());
			this->gboxSelectLang = (gcnew System::Windows::Forms::GroupBox());
			this->rSelectE = (gcnew System::Windows::Forms::RadioButton());
			this->rSelectJ = (gcnew System::Windows::Forms::RadioButton());
			this->labPEGIBBFC2 = (gcnew System::Windows::Forms::Label());
			this->labOFLC = (gcnew System::Windows::Forms::Label());
			this->labPEGIBBFC = (gcnew System::Windows::Forms::Label());
			this->labPEGIPRT = (gcnew System::Windows::Forms::Label());
			this->labPEGI = (gcnew System::Windows::Forms::Label());
			this->labUSK = (gcnew System::Windows::Forms::Label());
			this->labESRB = (gcnew System::Windows::Forms::Label());
			this->labCERO = (gcnew System::Windows::Forms::Label());
			this->cboxAlwaysOFLC = (gcnew System::Windows::Forms::CheckBox());
			this->cboxAlwaysPEGIBBFC = (gcnew System::Windows::Forms::CheckBox());
			this->cboxAlwaysPEGIPRT = (gcnew System::Windows::Forms::CheckBox());
			this->cboxAlwaysPEGI = (gcnew System::Windows::Forms::CheckBox());
			this->cboxAlwaysUSK = (gcnew System::Windows::Forms::CheckBox());
			this->cboxAlwaysESRB = (gcnew System::Windows::Forms::CheckBox());
			this->cboxAlwaysCERO = (gcnew System::Windows::Forms::CheckBox());
			this->combOFLC = (gcnew System::Windows::Forms::ComboBox());
			this->combPEGIBBFC = (gcnew System::Windows::Forms::ComboBox());
			this->combPEGIPRT = (gcnew System::Windows::Forms::ComboBox());
			this->combPEGI = (gcnew System::Windows::Forms::ComboBox());
			this->combUSK = (gcnew System::Windows::Forms::ComboBox());
			this->combESRB = (gcnew System::Windows::Forms::ComboBox());
			this->combCERO = (gcnew System::Windows::Forms::ComboBox());
			this->cboxOFLC = (gcnew System::Windows::Forms::CheckBox());
			this->cboxPEGIBBFC = (gcnew System::Windows::Forms::CheckBox());
			this->cboxPEGIPRT = (gcnew System::Windows::Forms::CheckBox());
			this->cboxPEGI = (gcnew System::Windows::Forms::CheckBox());
			this->cboxUSK = (gcnew System::Windows::Forms::CheckBox());
			this->cboxESRB = (gcnew System::Windows::Forms::CheckBox());
			this->cboxCERO = (gcnew System::Windows::Forms::CheckBox());
			this->labParentalForceEnable = (gcnew System::Windows::Forms::Label());
			this->labParentalRating = (gcnew System::Windows::Forms::Label());
			this->labParentalEnable = (gcnew System::Windows::Forms::Label());
			this->gboxTWLInfoWritable = (gcnew System::Windows::Forms::GroupBox());
			this->labRegion = (gcnew System::Windows::Forms::Label());
			this->cboxIsEULA = (gcnew System::Windows::Forms::CheckBox());
			this->combRegion = (gcnew System::Windows::Forms::ComboBox());
			this->cboxIsWiFiIcon = (gcnew System::Windows::Forms::CheckBox());
			this->labEULA = (gcnew System::Windows::Forms::Label());
			this->cboxIsWirelessIcon = (gcnew System::Windows::Forms::CheckBox());
			this->numEULA = (gcnew System::Windows::Forms::NumericUpDown());
			this->cboxIsInputPerson2 = (gcnew System::Windows::Forms::CheckBox());
			this->gboxPerson2 = (gcnew System::Windows::Forms::GroupBox());
			this->labNTSC2Sur = (gcnew System::Windows::Forms::Label());
			this->tboxNTSC2 = (gcnew System::Windows::Forms::TextBox());
			this->labFax2 = (gcnew System::Windows::Forms::Label());
			this->labNTSC2Pre = (gcnew System::Windows::Forms::Label());
			this->tboxFax2 = (gcnew System::Windows::Forms::TextBox());
			this->tboxMail2 = (gcnew System::Windows::Forms::TextBox());
			this->tboxTel2 = (gcnew System::Windows::Forms::TextBox());
			this->tboxFurigana2 = (gcnew System::Windows::Forms::TextBox());
			this->tboxPerson2 = (gcnew System::Windows::Forms::TextBox());
			this->tboxDepart2 = (gcnew System::Windows::Forms::TextBox());
			this->labDepart2 = (gcnew System::Windows::Forms::Label());
			this->tboxCompany2 = (gcnew System::Windows::Forms::TextBox());
			this->labMail2 = (gcnew System::Windows::Forms::Label());
			this->labTel2 = (gcnew System::Windows::Forms::Label());
			this->labFurigana2 = (gcnew System::Windows::Forms::Label());
			this->labPerson2 = (gcnew System::Windows::Forms::Label());
			this->labCompany2 = (gcnew System::Windows::Forms::Label());
			this->gboxPerson1 = (gcnew System::Windows::Forms::GroupBox());
			this->labNTSC1Sur = (gcnew System::Windows::Forms::Label());
			this->labFax1 = (gcnew System::Windows::Forms::Label());
			this->labNTSC1Pre = (gcnew System::Windows::Forms::Label());
			this->tboxNTSC1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxFax1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxMail1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxTel1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxFurigana1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxPerson1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxDepart1 = (gcnew System::Windows::Forms::TextBox());
			this->labDepart1 = (gcnew System::Windows::Forms::Label());
			this->tboxCompany1 = (gcnew System::Windows::Forms::TextBox());
			this->labMail1 = (gcnew System::Windows::Forms::Label());
			this->labTel1 = (gcnew System::Windows::Forms::Label());
			this->labFurigana1 = (gcnew System::Windows::Forms::Label());
			this->labPerson1 = (gcnew System::Windows::Forms::Label());
			this->labCompany1 = (gcnew System::Windows::Forms::Label());
			this->tboxProductCode2 = (gcnew System::Windows::Forms::TextBox());
			this->tboxProductCode1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxProductName = (gcnew System::Windows::Forms::TextBox());
			this->labProductCode2 = (gcnew System::Windows::Forms::Label());
			this->labProductCode1 = (gcnew System::Windows::Forms::Label());
			this->dateSubmit = (gcnew System::Windows::Forms::DateTimePicker());
			this->dateRelease = (gcnew System::Windows::Forms::DateTimePicker());
			this->gboxUsage = (gcnew System::Windows::Forms::GroupBox());
			this->tboxUsageOther = (gcnew System::Windows::Forms::TextBox());
			this->rUsageOther = (gcnew System::Windows::Forms::RadioButton());
			this->rUsageDst = (gcnew System::Windows::Forms::RadioButton());
			this->rUsageSample = (gcnew System::Windows::Forms::RadioButton());
			this->rUsageSale = (gcnew System::Windows::Forms::RadioButton());
			this->gboxSubmitWay = (gcnew System::Windows::Forms::GroupBox());
			this->rSubmitHand = (gcnew System::Windows::Forms::RadioButton());
			this->rSubmitPost = (gcnew System::Windows::Forms::RadioButton());
			this->labSubmiteDate = (gcnew System::Windows::Forms::Label());
			this->labReleaseDate = (gcnew System::Windows::Forms::Label());
			this->labProductCode = (gcnew System::Windows::Forms::Label());
			this->labProductName = (gcnew System::Windows::Forms::Label());
			this->labCapSubmitVer = (gcnew System::Windows::Forms::Label());
			this->numSubmitVersion = (gcnew System::Windows::Forms::NumericUpDown());
			this->labSubmitVer = (gcnew System::Windows::Forms::Label());
			this->tabDoc = (gcnew System::Windows::Forms::TabControl());
			this->tabSubmitInfo = (gcnew System::Windows::Forms::TabPage());
			this->labCautionInput = (gcnew System::Windows::Forms::Label());
			this->tabForeignInfo = (gcnew System::Windows::Forms::TabPage());
			this->labMultiForeign2 = (gcnew System::Windows::Forms::Label());
			this->labMultiForeign1 = (gcnew System::Windows::Forms::Label());
			this->tboxProductCode2Foreign3 = (gcnew System::Windows::Forms::TextBox());
			this->tboxProductCode2Foreign2 = (gcnew System::Windows::Forms::TextBox());
			this->labProductCode2Foreign = (gcnew System::Windows::Forms::Label());
			this->cboxReleaseForeign = (gcnew System::Windows::Forms::CheckBox());
			this->labProductNameForeign = (gcnew System::Windows::Forms::Label());
			this->tboxProductNameForeign = (gcnew System::Windows::Forms::TextBox());
			this->labProductCode1Foreign = (gcnew System::Windows::Forms::Label());
			this->tboxProductCode1Foreign = (gcnew System::Windows::Forms::TextBox());
			this->labProductCodeForeign = (gcnew System::Windows::Forms::Label());
			this->tboxProductCode2Foreign1 = (gcnew System::Windows::Forms::TextBox());
			this->tabCompanyInfo1 = (gcnew System::Windows::Forms::TabPage());
			this->tabCompanyInfo2 = (gcnew System::Windows::Forms::TabPage());
			this->tabCaption = (gcnew System::Windows::Forms::TabPage());
			this->tabTWLSpec4 = (gcnew System::Windows::Forms::TabPage());
			this->gboxShared2Size = (gcnew System::Windows::Forms::GroupBox());
			this->labShared2Size5 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size4 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size3 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size2 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size1 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size0 = (gcnew System::Windows::Forms::Label());
			this->labKB5 = (gcnew System::Windows::Forms::Label());
			this->labKB4 = (gcnew System::Windows::Forms::Label());
			this->labKB3 = (gcnew System::Windows::Forms::Label());
			this->labKB2 = (gcnew System::Windows::Forms::Label());
			this->labKB1 = (gcnew System::Windows::Forms::Label());
			this->labKB0 = (gcnew System::Windows::Forms::Label());
			this->tboxShared2Size5 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size4 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size3 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size2 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size0 = (gcnew System::Windows::Forms::TextBox());
			this->cboxIsShared2 = (gcnew System::Windows::Forms::CheckBox());
			this->tabSDK = (gcnew System::Windows::Forms::TabPage());
			this->tboxLib = (gcnew System::Windows::Forms::TextBox());
			this->labLib = (gcnew System::Windows::Forms::Label());
			this->tboxSDK = (gcnew System::Windows::Forms::TextBox());
			this->labSDK = (gcnew System::Windows::Forms::Label());
			this->tabTWLSpec3 = (gcnew System::Windows::Forms::TabPage());
			this->gboxTWLExInfo = (gcnew System::Windows::Forms::GroupBox());
			this->labByte2 = (gcnew System::Windows::Forms::Label());
			this->labByte1 = (gcnew System::Windows::Forms::Label());
			this->labHex4 = (gcnew System::Windows::Forms::Label());
			this->labHex3 = (gcnew System::Windows::Forms::Label());
			this->tboxIsCodec = (gcnew System::Windows::Forms::TextBox());
			this->labIsCodec = (gcnew System::Windows::Forms::Label());
			this->labNormalRomOffset = (gcnew System::Windows::Forms::Label());
			this->tboxNormalRomOffset = (gcnew System::Windows::Forms::TextBox());
			this->cboxIsSubBanner = (gcnew System::Windows::Forms::CheckBox());
			this->labKeyTableRomOffset = (gcnew System::Windows::Forms::Label());
			this->cboxIsWL = (gcnew System::Windows::Forms::CheckBox());
			this->tboxPrivateSize = (gcnew System::Windows::Forms::TextBox());
			this->labPrivateSize = (gcnew System::Windows::Forms::Label());
			this->tboxKeyTableRomOffset = (gcnew System::Windows::Forms::TextBox());
			this->cboxIsNormalJump = (gcnew System::Windows::Forms::CheckBox());
			this->cboxIsTmpJump = (gcnew System::Windows::Forms::CheckBox());
			this->labPublicSize = (gcnew System::Windows::Forms::Label());
			this->tboxPublicSize = (gcnew System::Windows::Forms::TextBox());
			this->tabTWLSpec2 = (gcnew System::Windows::Forms::TabPage());
			this->gboxAccess = (gcnew System::Windows::Forms::GroupBox());
			this->labAccessOther = (gcnew System::Windows::Forms::Label());
			this->tboxAccessOther = (gcnew System::Windows::Forms::TextBox());
			this->tboxIsGameCardOn = (gcnew System::Windows::Forms::TextBox());
			this->labIsGameCardOn = (gcnew System::Windows::Forms::Label());
			this->cboxIsNAND = (gcnew System::Windows::Forms::CheckBox());
			this->cboxIsSD = (gcnew System::Windows::Forms::CheckBox());
			this->tabTWLSpec1 = (gcnew System::Windows::Forms::TabPage());
			this->labCautionCheck = (gcnew System::Windows::Forms::Label());
			this->gboxTitleID = (gcnew System::Windows::Forms::GroupBox());
			this->labHex2 = (gcnew System::Windows::Forms::Label());
			this->labHex1 = (gcnew System::Windows::Forms::Label());
			this->tboxTitleIDLo = (gcnew System::Windows::Forms::TextBox());
			this->labTitleIDLo = (gcnew System::Windows::Forms::Label());
			this->labTitleIDHi = (gcnew System::Windows::Forms::Label());
			this->tboxTitleIDHi = (gcnew System::Windows::Forms::TextBox());
			this->tboxAppType = (gcnew System::Windows::Forms::TextBox());
			this->labAppType = (gcnew System::Windows::Forms::Label());
			this->labCaptionEx = (gcnew System::Windows::Forms::Label());
			this->tboxCaptionEx = (gcnew System::Windows::Forms::TextBox());
			this->tabCheck = (gcnew System::Windows::Forms::TabControl());
			this->gboxSrl->SuspendLayout();
			this->gboxCRC->SuspendLayout();
			this->gboxFileOpen->SuspendLayout();
			this->gboxSelectLang->SuspendLayout();
			this->gboxTWLInfoWritable->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numEULA))->BeginInit();
			this->gboxPerson2->SuspendLayout();
			this->gboxPerson1->SuspendLayout();
			this->gboxUsage->SuspendLayout();
			this->gboxSubmitWay->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numSubmitVersion))->BeginInit();
			this->tabDoc->SuspendLayout();
			this->tabSubmitInfo->SuspendLayout();
			this->tabForeignInfo->SuspendLayout();
			this->tabCompanyInfo1->SuspendLayout();
			this->tabCompanyInfo2->SuspendLayout();
			this->tabCaption->SuspendLayout();
			this->tabTWLSpec4->SuspendLayout();
			this->gboxShared2Size->SuspendLayout();
			this->tabSDK->SuspendLayout();
			this->tabTWLSpec3->SuspendLayout();
			this->gboxTWLExInfo->SuspendLayout();
			this->tabTWLSpec2->SuspendLayout();
			this->gboxAccess->SuspendLayout();
			this->tabTWLSpec1->SuspendLayout();
			this->gboxTitleID->SuspendLayout();
			this->tabCheck->SuspendLayout();
			this->SuspendLayout();
			// 
			// tboxFile
			// 
			this->tboxFile->AllowDrop = true;
			this->tboxFile->Location = System::Drawing::Point(6, 18);
			this->tboxFile->Name = L"tboxFile";
			this->tboxFile->ReadOnly = true;
			this->tboxFile->Size = System::Drawing::Size(329, 19);
			this->tboxFile->TabIndex = 0;
			this->tboxFile->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &Form1::tboxFile_DragDrop);
			this->tboxFile->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &Form1::tboxFile_DragEnter);
			// 
			// butOpen
			// 
			this->butOpen->Location = System::Drawing::Point(341, 16);
			this->butOpen->Name = L"butOpen";
			this->butOpen->Size = System::Drawing::Size(112, 23);
			this->butOpen->TabIndex = 1;
			this->butOpen->Text = L"ROM�f�[�^���J��";
			this->butOpen->UseVisualStyleBackColor = true;
			this->butOpen->Click += gcnew System::EventHandler(this, &Form1::butOpen_Click);
			// 
			// tboxMsg
			// 
			this->tboxMsg->Location = System::Drawing::Point(12, 678);
			this->tboxMsg->Name = L"tboxMsg";
			this->tboxMsg->ReadOnly = true;
			this->tboxMsg->Size = System::Drawing::Size(797, 19);
			this->tboxMsg->TabIndex = 2;
			// 
			// gboxSrl
			// 
			this->gboxSrl->Controls->Add(this->labBackup);
			this->gboxSrl->Controls->Add(this->combBackup);
			this->gboxSrl->Controls->Add(this->tboxBackupOther);
			this->gboxSrl->Controls->Add(this->tboxRemasterVer);
			this->gboxSrl->Controls->Add(this->tboxRomSize);
			this->gboxSrl->Controls->Add(this->tboxPlatform);
			this->gboxSrl->Controls->Add(this->labPlatform);
			this->gboxSrl->Controls->Add(this->tboxRomLatency);
			this->gboxSrl->Controls->Add(this->labRomSize);
			this->gboxSrl->Controls->Add(this->labRomType);
			this->gboxSrl->Controls->Add(this->tboxMakerCode);
			this->gboxSrl->Controls->Add(this->cboxRemasterVerE);
			this->gboxSrl->Controls->Add(this->labMakerCode);
			this->gboxSrl->Controls->Add(this->labGameCode);
			this->gboxSrl->Controls->Add(this->tboxGameCode);
			this->gboxSrl->Controls->Add(this->labTitleName);
			this->gboxSrl->Controls->Add(this->labRemasterVer);
			this->gboxSrl->Controls->Add(this->tboxTitleName);
			this->gboxSrl->Location = System::Drawing::Point(12, 90);
			this->gboxSrl->Name = L"gboxSrl";
			this->gboxSrl->Size = System::Drawing::Size(326, 235);
			this->gboxSrl->TabIndex = 3;
			this->gboxSrl->TabStop = false;
			this->gboxSrl->Text = L"ROM�f�[�^���";
			// 
			// labBackup
			// 
			this->labBackup->AutoSize = true;
			this->labBackup->Location = System::Drawing::Point(13, 207);
			this->labBackup->Name = L"labBackup";
			this->labBackup->Size = System::Drawing::Size(79, 12);
			this->labBackup->TabIndex = 12;
			this->labBackup->Text = L"�o�b�N�A�b�v������";
			// 
			// combBackup
			// 
			this->combBackup->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combBackup->FormattingEnabled = true;
			this->combBackup->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"4Kbit EEPROM", L"64Kbit EEPROM", L"512Kbit EEPROM", 
				L"256Kbit FRAM", L"2Mbit FLASH", L"4Mbit FLASH", L"8Mbit FLASH", L"�Ȃ�", L"���̑�"});
			this->combBackup->Location = System::Drawing::Point(104, 203);
			this->combBackup->MaxDropDownItems = 9;
			this->combBackup->Name = L"combBackup";
			this->combBackup->Size = System::Drawing::Size(100, 20);
			this->combBackup->TabIndex = 5;
			this->combBackup->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::combBackup_SelectedIndexChanged);
			// 
			// tboxBackupOther
			// 
			this->tboxBackupOther->Enabled = false;
			this->tboxBackupOther->Location = System::Drawing::Point(210, 204);
			this->tboxBackupOther->Name = L"tboxBackupOther";
			this->tboxBackupOther->Size = System::Drawing::Size(72, 19);
			this->tboxBackupOther->TabIndex = 6;
			// 
			// tboxRemasterVer
			// 
			this->tboxRemasterVer->Location = System::Drawing::Point(104, 175);
			this->tboxRemasterVer->Name = L"tboxRemasterVer";
			this->tboxRemasterVer->ReadOnly = true;
			this->tboxRemasterVer->Size = System::Drawing::Size(35, 19);
			this->tboxRemasterVer->TabIndex = 7;
			// 
			// tboxRomSize
			// 
			this->tboxRomSize->Location = System::Drawing::Point(104, 150);
			this->tboxRomSize->Name = L"tboxRomSize";
			this->tboxRomSize->ReadOnly = true;
			this->tboxRomSize->Size = System::Drawing::Size(100, 19);
			this->tboxRomSize->TabIndex = 15;
			// 
			// tboxPlatform
			// 
			this->tboxPlatform->Location = System::Drawing::Point(104, 100);
			this->tboxPlatform->Name = L"tboxPlatform";
			this->tboxPlatform->ReadOnly = true;
			this->tboxPlatform->Size = System::Drawing::Size(100, 19);
			this->tboxPlatform->TabIndex = 14;
			// 
			// labPlatform
			// 
			this->labPlatform->AutoSize = true;
			this->labPlatform->Location = System::Drawing::Point(14, 102);
			this->labPlatform->Name = L"labPlatform";
			this->labPlatform->Size = System::Drawing::Size(73, 12);
			this->labPlatform->TabIndex = 13;
			this->labPlatform->Text = L"�v���b�g�t�H�[��";
			// 
			// tboxRomLatency
			// 
			this->tboxRomLatency->Location = System::Drawing::Point(104, 125);
			this->tboxRomLatency->Name = L"tboxRomLatency";
			this->tboxRomLatency->ReadOnly = true;
			this->tboxRomLatency->Size = System::Drawing::Size(100, 19);
			this->tboxRomLatency->TabIndex = 11;
			// 
			// labRomSize
			// 
			this->labRomSize->AutoSize = true;
			this->labRomSize->Location = System::Drawing::Point(14, 153);
			this->labRomSize->Name = L"labRomSize";
			this->labRomSize->Size = System::Drawing::Size(54, 12);
			this->labRomSize->TabIndex = 9;
			this->labRomSize->Text = L"ROM�e��";
			// 
			// labRomType
			// 
			this->labRomType->AutoSize = true;
			this->labRomType->Location = System::Drawing::Point(14, 128);
			this->labRomType->Name = L"labRomType";
			this->labRomType->Size = System::Drawing::Size(80, 12);
			this->labRomType->TabIndex = 7;
			this->labRomType->Text = L"ROM�^�C�v�ݒ�";
			// 
			// tboxMakerCode
			// 
			this->tboxMakerCode->Location = System::Drawing::Point(104, 73);
			this->tboxMakerCode->MaxLength = 2;
			this->tboxMakerCode->Name = L"tboxMakerCode";
			this->tboxMakerCode->ReadOnly = true;
			this->tboxMakerCode->Size = System::Drawing::Size(35, 19);
			this->tboxMakerCode->TabIndex = 2;
			// 
			// cboxRemasterVerE
			// 
			this->cboxRemasterVerE->AutoSize = true;
			this->cboxRemasterVerE->Enabled = false;
			this->cboxRemasterVerE->Location = System::Drawing::Point(145, 176);
			this->cboxRemasterVerE->Name = L"cboxRemasterVerE";
			this->cboxRemasterVerE->Size = System::Drawing::Size(75, 16);
			this->cboxRemasterVerE->TabIndex = 8;
			this->cboxRemasterVerE->Text = L"E(���O��)";
			this->cboxRemasterVerE->UseVisualStyleBackColor = true;
			this->cboxRemasterVerE->CheckedChanged += gcnew System::EventHandler(this, &Form1::cboxRemasterVerE_CheckedChanged);
			// 
			// labMakerCode
			// 
			this->labMakerCode->AutoSize = true;
			this->labMakerCode->Location = System::Drawing::Point(14, 76);
			this->labMakerCode->Name = L"labMakerCode";
			this->labMakerCode->Size = System::Drawing::Size(59, 12);
			this->labMakerCode->TabIndex = 4;
			this->labMakerCode->Text = L"���[�J�R�[�h";
			// 
			// labGameCode
			// 
			this->labGameCode->AutoSize = true;
			this->labGameCode->Location = System::Drawing::Point(14, 49);
			this->labGameCode->Name = L"labGameCode";
			this->labGameCode->Size = System::Drawing::Size(78, 12);
			this->labGameCode->TabIndex = 3;
			this->labGameCode->Text = L"�C�j�V�����R�[�h";
			// 
			// tboxGameCode
			// 
			this->tboxGameCode->Location = System::Drawing::Point(104, 46);
			this->tboxGameCode->MaxLength = 4;
			this->tboxGameCode->Name = L"tboxGameCode";
			this->tboxGameCode->ReadOnly = true;
			this->tboxGameCode->Size = System::Drawing::Size(59, 19);
			this->tboxGameCode->TabIndex = 1;
			// 
			// labTitleName
			// 
			this->labTitleName->AutoSize = true;
			this->labTitleName->Location = System::Drawing::Point(16, 24);
			this->labTitleName->Name = L"labTitleName";
			this->labTitleName->Size = System::Drawing::Size(65, 12);
			this->labTitleName->TabIndex = 1;
			this->labTitleName->Text = L"�\�t�g�^�C�g��";
			// 
			// labRemasterVer
			// 
			this->labRemasterVer->AutoSize = true;
			this->labRemasterVer->Location = System::Drawing::Point(5, 178);
			this->labRemasterVer->Name = L"labRemasterVer";
			this->labRemasterVer->Size = System::Drawing::Size(93, 12);
			this->labRemasterVer->TabIndex = 22;
			this->labRemasterVer->Text = L"���}�X�^�[�o�[�W����";
			// 
			// tboxTitleName
			// 
			this->tboxTitleName->ImeMode = System::Windows::Forms::ImeMode::NoControl;
			this->tboxTitleName->Location = System::Drawing::Point(104, 21);
			this->tboxTitleName->MaxLength = 12;
			this->tboxTitleName->Name = L"tboxTitleName";
			this->tboxTitleName->ReadOnly = true;
			this->tboxTitleName->Size = System::Drawing::Size(100, 19);
			this->tboxTitleName->TabIndex = 0;
			// 
			// gboxCRC
			// 
			this->gboxCRC->Controls->Add(this->labRomCRC);
			this->gboxCRC->Controls->Add(this->labHeaderCRC);
			this->gboxCRC->Controls->Add(this->tboxHeaderCRC);
			this->gboxCRC->Controls->Add(this->tboxWholeCRC);
			this->gboxCRC->Location = System::Drawing::Point(185, 14);
			this->gboxCRC->Name = L"gboxCRC";
			this->gboxCRC->Size = System::Drawing::Size(153, 70);
			this->gboxCRC->TabIndex = 5;
			this->gboxCRC->TabStop = false;
			this->gboxCRC->Text = L"CRC";
			// 
			// labRomCRC
			// 
			this->labRomCRC->AutoSize = true;
			this->labRomCRC->Location = System::Drawing::Point(6, 46);
			this->labRomCRC->Name = L"labRomCRC";
			this->labRomCRC->Size = System::Drawing::Size(63, 12);
			this->labRomCRC->TabIndex = 3;
			this->labRomCRC->Text = L"�S�̂�CRC";
			// 
			// labHeaderCRC
			// 
			this->labHeaderCRC->AutoSize = true;
			this->labHeaderCRC->Location = System::Drawing::Point(6, 20);
			this->labHeaderCRC->Name = L"labHeaderCRC";
			this->labHeaderCRC->Size = System::Drawing::Size(55, 12);
			this->labHeaderCRC->TabIndex = 2;
			this->labHeaderCRC->Text = L"�w�b�_CRC";
			// 
			// tboxHeaderCRC
			// 
			this->tboxHeaderCRC->Location = System::Drawing::Point(75, 17);
			this->tboxHeaderCRC->Name = L"tboxHeaderCRC";
			this->tboxHeaderCRC->ReadOnly = true;
			this->tboxHeaderCRC->Size = System::Drawing::Size(55, 19);
			this->tboxHeaderCRC->TabIndex = 1;
			// 
			// tboxWholeCRC
			// 
			this->tboxWholeCRC->Location = System::Drawing::Point(75, 43);
			this->tboxWholeCRC->Name = L"tboxWholeCRC";
			this->tboxWholeCRC->ReadOnly = true;
			this->tboxWholeCRC->Size = System::Drawing::Size(55, 19);
			this->tboxWholeCRC->TabIndex = 0;
			// 
			// gboxFileOpen
			// 
			this->gboxFileOpen->Controls->Add(this->butSaveAs);
			this->gboxFileOpen->Controls->Add(this->tboxFile);
			this->gboxFileOpen->Controls->Add(this->butOpen);
			this->gboxFileOpen->Location = System::Drawing::Point(12, 625);
			this->gboxFileOpen->Name = L"gboxFileOpen";
			this->gboxFileOpen->Size = System::Drawing::Size(622, 47);
			this->gboxFileOpen->TabIndex = 4;
			this->gboxFileOpen->TabStop = false;
			this->gboxFileOpen->Text = L"ROM�f�[�^�t�@�C���̓��o��";
			// 
			// butSaveAs
			// 
			this->butSaveAs->Location = System::Drawing::Point(462, 16);
			this->butSaveAs->Name = L"butSaveAs";
			this->butSaveAs->Size = System::Drawing::Size(148, 23);
			this->butSaveAs->TabIndex = 2;
			this->butSaveAs->Text = L"���͏��𔽉f�����ĕۑ�";
			this->butSaveAs->UseVisualStyleBackColor = true;
			this->butSaveAs->Click += gcnew System::EventHandler(this, &Form1::butSaveAs_Click);
			// 
			// butMakeMaster
			// 
			this->butMakeMaster->Location = System::Drawing::Point(656, 641);
			this->butMakeMaster->Name = L"butMakeMaster";
			this->butMakeMaster->Size = System::Drawing::Size(149, 23);
			this->butMakeMaster->TabIndex = 7;
			this->butMakeMaster->Text = L"�}�X�^��o���ނ��쐬";
			this->butMakeMaster->UseVisualStyleBackColor = true;
			this->butMakeMaster->Click += gcnew System::EventHandler(this, &Form1::butMakeMaster_Click);
			// 
			// labCaption
			// 
			this->labCaption->AutoSize = true;
			this->labCaption->Location = System::Drawing::Point(23, 22);
			this->labCaption->Name = L"labCaption";
			this->labCaption->Size = System::Drawing::Size(191, 12);
			this->labCaption->TabIndex = 8;
			this->labCaption->Text = L"���̑��A������������΂��L����������";
			// 
			// tboxCaption
			// 
			this->tboxCaption->Location = System::Drawing::Point(25, 37);
			this->tboxCaption->Multiline = true;
			this->tboxCaption->Name = L"tboxCaption";
			this->tboxCaption->Size = System::Drawing::Size(328, 181);
			this->tboxCaption->TabIndex = 8;
			// 
			// gboxSelectLang
			// 
			this->gboxSelectLang->Controls->Add(this->rSelectE);
			this->gboxSelectLang->Controls->Add(this->rSelectJ);
			this->gboxSelectLang->Location = System::Drawing::Point(12, 12);
			this->gboxSelectLang->Name = L"gboxSelectLang";
			this->gboxSelectLang->Size = System::Drawing::Size(153, 72);
			this->gboxSelectLang->TabIndex = 9;
			this->gboxSelectLang->TabStop = false;
			this->gboxSelectLang->Text = L"Select Language";
			// 
			// rSelectE
			// 
			this->rSelectE->AutoSize = true;
			this->rSelectE->Location = System::Drawing::Point(16, 22);
			this->rSelectE->Name = L"rSelectE";
			this->rSelectE->Size = System::Drawing::Size(60, 16);
			this->rSelectE->TabIndex = 1;
			this->rSelectE->Text = L"English";
			this->rSelectE->UseVisualStyleBackColor = true;
			this->rSelectE->CheckedChanged += gcnew System::EventHandler(this, &Form1::rSelectE_CheckedChanged);
			// 
			// rSelectJ
			// 
			this->rSelectJ->AutoSize = true;
			this->rSelectJ->Checked = true;
			this->rSelectJ->Location = System::Drawing::Point(16, 44);
			this->rSelectJ->Name = L"rSelectJ";
			this->rSelectJ->Size = System::Drawing::Size(72, 16);
			this->rSelectJ->TabIndex = 0;
			this->rSelectJ->TabStop = true;
			this->rSelectJ->Text = L"Japanese";
			this->rSelectJ->UseVisualStyleBackColor = true;
			this->rSelectJ->CheckedChanged += gcnew System::EventHandler(this, &Form1::rSelectJ_CheckedChanged);
			// 
			// labPEGIBBFC2
			// 
			this->labPEGIBBFC2->AutoSize = true;
			this->labPEGIBBFC2->Location = System::Drawing::Point(14, 266);
			this->labPEGIBBFC2->Name = L"labPEGIBBFC2";
			this->labPEGIBBFC2->Size = System::Drawing::Size(46, 12);
			this->labPEGIBBFC2->TabIndex = 35;
			this->labPEGIBBFC2->Text = L"+ BBFC";
			// 
			// labOFLC
			// 
			this->labOFLC->AutoSize = true;
			this->labOFLC->Location = System::Drawing::Point(50, 284);
			this->labOFLC->Name = L"labOFLC";
			this->labOFLC->Size = System::Drawing::Size(34, 12);
			this->labOFLC->TabIndex = 33;
			this->labOFLC->Text = L"OFLC";
			// 
			// labPEGIBBFC
			// 
			this->labPEGIBBFC->AutoSize = true;
			this->labPEGIBBFC->Location = System::Drawing::Point(14, 254);
			this->labPEGIBBFC->Name = L"labPEGIBBFC";
			this->labPEGIBBFC->Size = System::Drawing::Size(77, 12);
			this->labPEGIBBFC->TabIndex = 32;
			this->labPEGIBBFC->Text = L"PEGI(General)";
			// 
			// labPEGIPRT
			// 
			this->labPEGIPRT->AutoSize = true;
			this->labPEGIPRT->Location = System::Drawing::Point(14, 232);
			this->labPEGIPRT->Name = L"labPEGIPRT";
			this->labPEGIPRT->Size = System::Drawing::Size(76, 12);
			this->labPEGIPRT->TabIndex = 31;
			this->labPEGIPRT->Text = L"PEGI Portugal";
			// 
			// labPEGI
			// 
			this->labPEGI->AutoSize = true;
			this->labPEGI->Location = System::Drawing::Point(14, 206);
			this->labPEGI->Name = L"labPEGI";
			this->labPEGI->Size = System::Drawing::Size(77, 12);
			this->labPEGI->TabIndex = 30;
			this->labPEGI->Text = L"PEGI(General)";
			// 
			// labUSK
			// 
			this->labUSK->AutoSize = true;
			this->labUSK->Location = System::Drawing::Point(50, 180);
			this->labUSK->Name = L"labUSK";
			this->labUSK->Size = System::Drawing::Size(27, 12);
			this->labUSK->TabIndex = 29;
			this->labUSK->Text = L"USK";
			// 
			// labESRB
			// 
			this->labESRB->AutoSize = true;
			this->labESRB->Location = System::Drawing::Point(50, 154);
			this->labESRB->Name = L"labESRB";
			this->labESRB->Size = System::Drawing::Size(35, 12);
			this->labESRB->TabIndex = 28;
			this->labESRB->Text = L"ESRB";
			// 
			// labCERO
			// 
			this->labCERO->AutoSize = true;
			this->labCERO->Location = System::Drawing::Point(50, 128);
			this->labCERO->Name = L"labCERO";
			this->labCERO->Size = System::Drawing::Size(36, 12);
			this->labCERO->TabIndex = 27;
			this->labCERO->Text = L"CERO";
			// 
			// cboxAlwaysOFLC
			// 
			this->cboxAlwaysOFLC->AutoSize = true;
			this->cboxAlwaysOFLC->Location = System::Drawing::Point(375, 284);
			this->cboxAlwaysOFLC->Name = L"cboxAlwaysOFLC";
			this->cboxAlwaysOFLC->Size = System::Drawing::Size(15, 14);
			this->cboxAlwaysOFLC->TabIndex = 20;
			this->cboxAlwaysOFLC->UseVisualStyleBackColor = true;
			// 
			// cboxAlwaysPEGIBBFC
			// 
			this->cboxAlwaysPEGIBBFC->AutoSize = true;
			this->cboxAlwaysPEGIBBFC->Location = System::Drawing::Point(375, 258);
			this->cboxAlwaysPEGIBBFC->Name = L"cboxAlwaysPEGIBBFC";
			this->cboxAlwaysPEGIBBFC->Size = System::Drawing::Size(15, 14);
			this->cboxAlwaysPEGIBBFC->TabIndex = 17;
			this->cboxAlwaysPEGIBBFC->UseVisualStyleBackColor = true;
			// 
			// cboxAlwaysPEGIPRT
			// 
			this->cboxAlwaysPEGIPRT->AutoSize = true;
			this->cboxAlwaysPEGIPRT->Location = System::Drawing::Point(375, 232);
			this->cboxAlwaysPEGIPRT->Name = L"cboxAlwaysPEGIPRT";
			this->cboxAlwaysPEGIPRT->Size = System::Drawing::Size(15, 14);
			this->cboxAlwaysPEGIPRT->TabIndex = 14;
			this->cboxAlwaysPEGIPRT->UseVisualStyleBackColor = true;
			// 
			// cboxAlwaysPEGI
			// 
			this->cboxAlwaysPEGI->AutoSize = true;
			this->cboxAlwaysPEGI->Location = System::Drawing::Point(375, 206);
			this->cboxAlwaysPEGI->Name = L"cboxAlwaysPEGI";
			this->cboxAlwaysPEGI->Size = System::Drawing::Size(15, 14);
			this->cboxAlwaysPEGI->TabIndex = 11;
			this->cboxAlwaysPEGI->UseVisualStyleBackColor = true;
			// 
			// cboxAlwaysUSK
			// 
			this->cboxAlwaysUSK->AutoSize = true;
			this->cboxAlwaysUSK->Location = System::Drawing::Point(375, 180);
			this->cboxAlwaysUSK->Name = L"cboxAlwaysUSK";
			this->cboxAlwaysUSK->Size = System::Drawing::Size(15, 14);
			this->cboxAlwaysUSK->TabIndex = 8;
			this->cboxAlwaysUSK->UseVisualStyleBackColor = true;
			// 
			// cboxAlwaysESRB
			// 
			this->cboxAlwaysESRB->AutoSize = true;
			this->cboxAlwaysESRB->Location = System::Drawing::Point(375, 154);
			this->cboxAlwaysESRB->Name = L"cboxAlwaysESRB";
			this->cboxAlwaysESRB->Size = System::Drawing::Size(15, 14);
			this->cboxAlwaysESRB->TabIndex = 5;
			this->cboxAlwaysESRB->UseVisualStyleBackColor = true;
			// 
			// cboxAlwaysCERO
			// 
			this->cboxAlwaysCERO->AutoSize = true;
			this->cboxAlwaysCERO->Location = System::Drawing::Point(375, 128);
			this->cboxAlwaysCERO->Name = L"cboxAlwaysCERO";
			this->cboxAlwaysCERO->Size = System::Drawing::Size(15, 14);
			this->cboxAlwaysCERO->TabIndex = 2;
			this->cboxAlwaysCERO->UseVisualStyleBackColor = true;
			// 
			// combOFLC
			// 
			this->combOFLC->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combOFLC->FormattingEnabled = true;
			this->combOFLC->Items->AddRange(gcnew cli::array< System::Object^  >(5) {L"G", L"PG", L"M", L"MA15+", L"���R��"});
			this->combOFLC->Location = System::Drawing::Point(97, 281);
			this->combOFLC->Name = L"combOFLC";
			this->combOFLC->Size = System::Drawing::Size(164, 20);
			this->combOFLC->TabIndex = 18;
			// 
			// combPEGIBBFC
			// 
			this->combPEGIBBFC->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combPEGIBBFC->FormattingEnabled = true;
			this->combPEGIBBFC->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"3�Έȏ�", L"4�Έȏ㐄��", L"7�Έȏ�", L"8�Έȏ㐄��", L"12�Έȏ�", 
				L"15�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"���R��"});
			this->combPEGIBBFC->Location = System::Drawing::Point(97, 255);
			this->combPEGIBBFC->Name = L"combPEGIBBFC";
			this->combPEGIBBFC->Size = System::Drawing::Size(164, 20);
			this->combPEGIBBFC->TabIndex = 15;
			// 
			// combPEGIPRT
			// 
			this->combPEGIPRT->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combPEGIPRT->FormattingEnabled = true;
			this->combPEGIPRT->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"4�Έȏ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", 
				L"���R��"});
			this->combPEGIPRT->Location = System::Drawing::Point(97, 229);
			this->combPEGIPRT->Name = L"combPEGIPRT";
			this->combPEGIPRT->Size = System::Drawing::Size(164, 20);
			this->combPEGIPRT->TabIndex = 12;
			// 
			// combPEGI
			// 
			this->combPEGI->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combPEGI->FormattingEnabled = true;
			this->combPEGI->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"3�Έȏ�", L"7�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", 
				L"���R��"});
			this->combPEGI->Location = System::Drawing::Point(97, 203);
			this->combPEGI->Name = L"combPEGI";
			this->combPEGI->Size = System::Drawing::Size(164, 20);
			this->combPEGI->TabIndex = 9;
			// 
			// combUSK
			// 
			this->combUSK->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combUSK->FormattingEnabled = true;
			this->combUSK->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"�N����Ȃ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"���N�ɂ͕s�K��", 
				L"���R��"});
			this->combUSK->Location = System::Drawing::Point(97, 177);
			this->combUSK->Name = L"combUSK";
			this->combUSK->Size = System::Drawing::Size(164, 20);
			this->combUSK->TabIndex = 6;
			// 
			// combESRB
			// 
			this->combESRB->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combESRB->FormattingEnabled = true;
			this->combESRB->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"EC (3�Έȏ�)", L"E (6�Έȏ�)", L"E10+ (10�Έȏ�)", L"T (13�Έȏ�)", 
				L"M (17�Έȏ�)", L"���R��"});
			this->combESRB->Location = System::Drawing::Point(97, 151);
			this->combESRB->Name = L"combESRB";
			this->combESRB->Size = System::Drawing::Size(164, 20);
			this->combESRB->TabIndex = 3;
			// 
			// combCERO
			// 
			this->combCERO->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combCERO->FormattingEnabled = true;
			this->combCERO->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"A (�S�N��)", L"B (12�Έȏ�)", L"C (15�Έȏ�)", L"D (17�Έȏ�)", 
				L"Z (18�Έȏ�)", L"���R��"});
			this->combCERO->Location = System::Drawing::Point(97, 125);
			this->combCERO->Name = L"combCERO";
			this->combCERO->Size = System::Drawing::Size(164, 20);
			this->combCERO->TabIndex = 0;
			// 
			// cboxOFLC
			// 
			this->cboxOFLC->AutoSize = true;
			this->cboxOFLC->Location = System::Drawing::Point(298, 284);
			this->cboxOFLC->Name = L"cboxOFLC";
			this->cboxOFLC->Size = System::Drawing::Size(15, 14);
			this->cboxOFLC->TabIndex = 19;
			this->cboxOFLC->UseVisualStyleBackColor = true;
			// 
			// cboxPEGIBBFC
			// 
			this->cboxPEGIBBFC->AutoSize = true;
			this->cboxPEGIBBFC->Location = System::Drawing::Point(298, 258);
			this->cboxPEGIBBFC->Name = L"cboxPEGIBBFC";
			this->cboxPEGIBBFC->Size = System::Drawing::Size(15, 14);
			this->cboxPEGIBBFC->TabIndex = 16;
			this->cboxPEGIBBFC->UseVisualStyleBackColor = true;
			// 
			// cboxPEGIPRT
			// 
			this->cboxPEGIPRT->AutoSize = true;
			this->cboxPEGIPRT->Location = System::Drawing::Point(298, 232);
			this->cboxPEGIPRT->Name = L"cboxPEGIPRT";
			this->cboxPEGIPRT->Size = System::Drawing::Size(15, 14);
			this->cboxPEGIPRT->TabIndex = 13;
			this->cboxPEGIPRT->UseVisualStyleBackColor = true;
			// 
			// cboxPEGI
			// 
			this->cboxPEGI->AutoSize = true;
			this->cboxPEGI->Location = System::Drawing::Point(298, 206);
			this->cboxPEGI->Name = L"cboxPEGI";
			this->cboxPEGI->Size = System::Drawing::Size(15, 14);
			this->cboxPEGI->TabIndex = 10;
			this->cboxPEGI->UseVisualStyleBackColor = true;
			// 
			// cboxUSK
			// 
			this->cboxUSK->AutoSize = true;
			this->cboxUSK->Location = System::Drawing::Point(298, 180);
			this->cboxUSK->Name = L"cboxUSK";
			this->cboxUSK->Size = System::Drawing::Size(15, 14);
			this->cboxUSK->TabIndex = 7;
			this->cboxUSK->UseVisualStyleBackColor = true;
			// 
			// cboxESRB
			// 
			this->cboxESRB->AutoSize = true;
			this->cboxESRB->Location = System::Drawing::Point(298, 154);
			this->cboxESRB->Name = L"cboxESRB";
			this->cboxESRB->Size = System::Drawing::Size(15, 14);
			this->cboxESRB->TabIndex = 4;
			this->cboxESRB->UseVisualStyleBackColor = true;
			// 
			// cboxCERO
			// 
			this->cboxCERO->AutoSize = true;
			this->cboxCERO->Location = System::Drawing::Point(298, 128);
			this->cboxCERO->Name = L"cboxCERO";
			this->cboxCERO->Size = System::Drawing::Size(15, 14);
			this->cboxCERO->TabIndex = 1;
			this->cboxCERO->UseVisualStyleBackColor = true;
			// 
			// labParentalForceEnable
			// 
			this->labParentalForceEnable->AutoSize = true;
			this->labParentalForceEnable->Location = System::Drawing::Point(349, 109);
			this->labParentalForceEnable->Name = L"labParentalForceEnable";
			this->labParentalForceEnable->Size = System::Drawing::Size(82, 12);
			this->labParentalForceEnable->TabIndex = 0;
			this->labParentalForceEnable->Text = L"Rating Pending";
			// 
			// labParentalRating
			// 
			this->labParentalRating->AutoSize = true;
			this->labParentalRating->Location = System::Drawing::Point(141, 109);
			this->labParentalRating->Name = L"labParentalRating";
			this->labParentalRating->Size = System::Drawing::Size(58, 12);
			this->labParentalRating->TabIndex = 2;
			this->labParentalRating->Text = L"���[�e�B���O";
			// 
			// labParentalEnable
			// 
			this->labParentalEnable->AutoSize = true;
			this->labParentalEnable->Location = System::Drawing::Point(265, 109);
			this->labParentalEnable->Name = L"labParentalEnable";
			this->labParentalEnable->Size = System::Drawing::Size(62, 12);
			this->labParentalEnable->TabIndex = 1;
			this->labParentalEnable->Text = L"������L��";
			// 
			// gboxTWLInfoWritable
			// 
			this->gboxTWLInfoWritable->Controls->Add(this->labRegion);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxIsEULA);
			this->gboxTWLInfoWritable->Controls->Add(this->combRegion);
			this->gboxTWLInfoWritable->Controls->Add(this->labPEGIBBFC2);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxIsWiFiIcon);
			this->gboxTWLInfoWritable->Controls->Add(this->labParentalRating);
			this->gboxTWLInfoWritable->Controls->Add(this->labEULA);
			this->gboxTWLInfoWritable->Controls->Add(this->labOFLC);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxIsWirelessIcon);
			this->gboxTWLInfoWritable->Controls->Add(this->labParentalEnable);
			this->gboxTWLInfoWritable->Controls->Add(this->numEULA);
			this->gboxTWLInfoWritable->Controls->Add(this->labPEGIBBFC);
			this->gboxTWLInfoWritable->Controls->Add(this->combPEGIBBFC);
			this->gboxTWLInfoWritable->Controls->Add(this->labParentalForceEnable);
			this->gboxTWLInfoWritable->Controls->Add(this->combOFLC);
			this->gboxTWLInfoWritable->Controls->Add(this->labPEGIPRT);
			this->gboxTWLInfoWritable->Controls->Add(this->combPEGIPRT);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxCERO);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxAlwaysCERO);
			this->gboxTWLInfoWritable->Controls->Add(this->labPEGI);
			this->gboxTWLInfoWritable->Controls->Add(this->combPEGI);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxESRB);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxAlwaysESRB);
			this->gboxTWLInfoWritable->Controls->Add(this->labUSK);
			this->gboxTWLInfoWritable->Controls->Add(this->combUSK);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxUSK);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxAlwaysUSK);
			this->gboxTWLInfoWritable->Controls->Add(this->labESRB);
			this->gboxTWLInfoWritable->Controls->Add(this->combESRB);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxPEGI);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxAlwaysPEGI);
			this->gboxTWLInfoWritable->Controls->Add(this->labCERO);
			this->gboxTWLInfoWritable->Controls->Add(this->combCERO);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxPEGIPRT);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxAlwaysPEGIPRT);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxAlwaysOFLC);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxOFLC);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxPEGIBBFC);
			this->gboxTWLInfoWritable->Controls->Add(this->cboxAlwaysPEGIBBFC);
			this->gboxTWLInfoWritable->Location = System::Drawing::Point(358, 12);
			this->gboxTWLInfoWritable->Name = L"gboxTWLInfoWritable";
			this->gboxTWLInfoWritable->Size = System::Drawing::Size(451, 313);
			this->gboxTWLInfoWritable->TabIndex = 30;
			this->gboxTWLInfoWritable->TabStop = false;
			this->gboxTWLInfoWritable->Text = L"ROM�f�[�^�ҏW�\���(�K�v���ł���ΕύX���Ă�������)";
			// 
			// labRegion
			// 
			this->labRegion->AutoSize = true;
			this->labRegion->Location = System::Drawing::Point(15, 81);
			this->labRegion->Name = L"labRegion";
			this->labRegion->Size = System::Drawing::Size(75, 12);
			this->labRegion->TabIndex = 37;
			this->labRegion->Text = L"�J�[�h���[�W����";
			// 
			// cboxIsEULA
			// 
			this->cboxIsEULA->AutoSize = true;
			this->cboxIsEULA->Location = System::Drawing::Point(18, 23);
			this->cboxIsEULA->Name = L"cboxIsEULA";
			this->cboxIsEULA->Size = System::Drawing::Size(77, 16);
			this->cboxIsEULA->TabIndex = 0;
			this->cboxIsEULA->Text = L"EULA����";
			this->cboxIsEULA->UseVisualStyleBackColor = true;
			// 
			// combRegion
			// 
			this->combRegion->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combRegion->FormattingEnabled = true;
			this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5) {L"���{�̂�", L"�č��̂�", L"���B�̂�", L"���B�̂�", L"���B����э��B"});
			this->combRegion->Location = System::Drawing::Point(97, 78);
			this->combRegion->Name = L"combRegion";
			this->combRegion->Size = System::Drawing::Size(216, 20);
			this->combRegion->TabIndex = 36;
			this->combRegion->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::combRegion_SelectedIndexChanged);
			// 
			// cboxIsWiFiIcon
			// 
			this->cboxIsWiFiIcon->AutoSize = true;
			this->cboxIsWiFiIcon->Location = System::Drawing::Point(231, 45);
			this->cboxIsWiFiIcon->Name = L"cboxIsWiFiIcon";
			this->cboxIsWiFiIcon->Size = System::Drawing::Size(135, 16);
			this->cboxIsWiFiIcon->TabIndex = 3;
			this->cboxIsWiFiIcon->Text = L"Wi-Fi�ʐM�A�C�R���\��";
			this->cboxIsWiFiIcon->UseVisualStyleBackColor = true;
			// 
			// labEULA
			// 
			this->labEULA->AutoSize = true;
			this->labEULA->Location = System::Drawing::Point(15, 49);
			this->labEULA->Name = L"labEULA";
			this->labEULA->Size = System::Drawing::Size(103, 12);
			this->labEULA->TabIndex = 7;
			this->labEULA->Text = L"EULA���Ӄo�[�W����";
			// 
			// cboxIsWirelessIcon
			// 
			this->cboxIsWirelessIcon->AutoSize = true;
			this->cboxIsWirelessIcon->Location = System::Drawing::Point(231, 23);
			this->cboxIsWirelessIcon->Name = L"cboxIsWirelessIcon";
			this->cboxIsWirelessIcon->Size = System::Drawing::Size(168, 16);
			this->cboxIsWirelessIcon->TabIndex = 2;
			this->cboxIsWirelessIcon->Text = L"DS���C�����X�ʐM�A�C�R���\��";
			this->cboxIsWirelessIcon->UseVisualStyleBackColor = true;
			// 
			// numEULA
			// 
			this->numEULA->Location = System::Drawing::Point(131, 44);
			this->numEULA->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {255, 0, 0, 0});
			this->numEULA->Name = L"numEULA";
			this->numEULA->Size = System::Drawing::Size(68, 19);
			this->numEULA->TabIndex = 1;
			this->numEULA->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// cboxIsInputPerson2
			// 
			this->cboxIsInputPerson2->AutoSize = true;
			this->cboxIsInputPerson2->Location = System::Drawing::Point(38, 11);
			this->cboxIsInputPerson2->Name = L"cboxIsInputPerson2";
			this->cboxIsInputPerson2->Size = System::Drawing::Size(150, 16);
			this->cboxIsInputPerson2->TabIndex = 7;
			this->cboxIsInputPerson2->Text = L"�S����2����͂���(�C��)";
			this->cboxIsInputPerson2->UseVisualStyleBackColor = true;
			this->cboxIsInputPerson2->CheckedChanged += gcnew System::EventHandler(this, &Form1::cboxIsInputPerson2_CheckedChanged);
			// 
			// gboxPerson2
			// 
			this->gboxPerson2->Controls->Add(this->labNTSC2Sur);
			this->gboxPerson2->Controls->Add(this->tboxNTSC2);
			this->gboxPerson2->Controls->Add(this->labFax2);
			this->gboxPerson2->Controls->Add(this->labNTSC2Pre);
			this->gboxPerson2->Controls->Add(this->tboxFax2);
			this->gboxPerson2->Controls->Add(this->tboxMail2);
			this->gboxPerson2->Controls->Add(this->tboxTel2);
			this->gboxPerson2->Controls->Add(this->tboxFurigana2);
			this->gboxPerson2->Controls->Add(this->tboxPerson2);
			this->gboxPerson2->Controls->Add(this->tboxDepart2);
			this->gboxPerson2->Controls->Add(this->labDepart2);
			this->gboxPerson2->Controls->Add(this->tboxCompany2);
			this->gboxPerson2->Controls->Add(this->labMail2);
			this->gboxPerson2->Controls->Add(this->labTel2);
			this->gboxPerson2->Controls->Add(this->labFurigana2);
			this->gboxPerson2->Controls->Add(this->labPerson2);
			this->gboxPerson2->Controls->Add(this->labCompany2);
			this->gboxPerson2->Enabled = false;
			this->gboxPerson2->Location = System::Drawing::Point(27, 35);
			this->gboxPerson2->Name = L"gboxPerson2";
			this->gboxPerson2->Size = System::Drawing::Size(304, 210);
			this->gboxPerson2->TabIndex = 14;
			this->gboxPerson2->TabStop = false;
			this->gboxPerson2->Text = L"�S����2";
			// 
			// labNTSC2Sur
			// 
			this->labNTSC2Sur->AutoSize = true;
			this->labNTSC2Sur->Location = System::Drawing::Point(18, 189);
			this->labNTSC2Sur->Name = L"labNTSC2Sur";
			this->labNTSC2Sur->Size = System::Drawing::Size(44, 12);
			this->labNTSC2Sur->TabIndex = 12;
			this->labNTSC2Sur->Text = L"User ID";
			// 
			// tboxNTSC2
			// 
			this->tboxNTSC2->Location = System::Drawing::Point(66, 175);
			this->tboxNTSC2->Name = L"tboxNTSC2";
			this->tboxNTSC2->Size = System::Drawing::Size(220, 19);
			this->tboxNTSC2->TabIndex = 11;
			// 
			// labFax2
			// 
			this->labFax2->AutoSize = true;
			this->labFax2->Location = System::Drawing::Point(159, 128);
			this->labFax2->Name = L"labFax2";
			this->labFax2->Size = System::Drawing::Size(27, 12);
			this->labFax2->TabIndex = 4;
			this->labFax2->Text = L"FAX";
			// 
			// labNTSC2Pre
			// 
			this->labNTSC2Pre->AutoSize = true;
			this->labNTSC2Pre->Location = System::Drawing::Point(18, 175);
			this->labNTSC2Pre->Name = L"labNTSC2Pre";
			this->labNTSC2Pre->Size = System::Drawing::Size(35, 12);
			this->labNTSC2Pre->TabIndex = 11;
			this->labNTSC2Pre->Text = L"NTSC";
			// 
			// tboxFax2
			// 
			this->tboxFax2->Location = System::Drawing::Point(192, 125);
			this->tboxFax2->Name = L"tboxFax2";
			this->tboxFax2->Size = System::Drawing::Size(94, 19);
			this->tboxFax2->TabIndex = 13;
			// 
			// tboxMail2
			// 
			this->tboxMail2->Location = System::Drawing::Point(66, 150);
			this->tboxMail2->Name = L"tboxMail2";
			this->tboxMail2->Size = System::Drawing::Size(220, 19);
			this->tboxMail2->TabIndex = 14;
			// 
			// tboxTel2
			// 
			this->tboxTel2->Location = System::Drawing::Point(66, 125);
			this->tboxTel2->Name = L"tboxTel2";
			this->tboxTel2->Size = System::Drawing::Size(84, 19);
			this->tboxTel2->TabIndex = 12;
			// 
			// tboxFurigana2
			// 
			this->tboxFurigana2->Location = System::Drawing::Point(66, 100);
			this->tboxFurigana2->Name = L"tboxFurigana2";
			this->tboxFurigana2->Size = System::Drawing::Size(220, 19);
			this->tboxFurigana2->TabIndex = 11;
			// 
			// tboxPerson2
			// 
			this->tboxPerson2->Location = System::Drawing::Point(66, 75);
			this->tboxPerson2->Name = L"tboxPerson2";
			this->tboxPerson2->Size = System::Drawing::Size(220, 19);
			this->tboxPerson2->TabIndex = 10;
			// 
			// tboxDepart2
			// 
			this->tboxDepart2->Location = System::Drawing::Point(66, 50);
			this->tboxDepart2->Name = L"tboxDepart2";
			this->tboxDepart2->Size = System::Drawing::Size(220, 19);
			this->tboxDepart2->TabIndex = 9;
			// 
			// labDepart2
			// 
			this->labDepart2->AutoSize = true;
			this->labDepart2->Location = System::Drawing::Point(12, 53);
			this->labDepart2->Name = L"labDepart2";
			this->labDepart2->Size = System::Drawing::Size(41, 12);
			this->labDepart2->TabIndex = 7;
			this->labDepart2->Text = L"������";
			// 
			// tboxCompany2
			// 
			this->tboxCompany2->Location = System::Drawing::Point(66, 25);
			this->tboxCompany2->Name = L"tboxCompany2";
			this->tboxCompany2->Size = System::Drawing::Size(220, 19);
			this->tboxCompany2->TabIndex = 8;
			// 
			// labMail2
			// 
			this->labMail2->AutoSize = true;
			this->labMail2->Location = System::Drawing::Point(18, 153);
			this->labMail2->Name = L"labMail2";
			this->labMail2->Size = System::Drawing::Size(38, 12);
			this->labMail2->TabIndex = 5;
			this->labMail2->Text = L"e-mail";
			// 
			// labTel2
			// 
			this->labTel2->AutoSize = true;
			this->labTel2->Location = System::Drawing::Point(28, 128);
			this->labTel2->Name = L"labTel2";
			this->labTel2->Size = System::Drawing::Size(25, 12);
			this->labTel2->TabIndex = 3;
			this->labTel2->Text = L"TEL";
			// 
			// labFurigana2
			// 
			this->labFurigana2->AutoSize = true;
			this->labFurigana2->Location = System::Drawing::Point(9, 103);
			this->labFurigana2->Name = L"labFurigana2";
			this->labFurigana2->Size = System::Drawing::Size(51, 12);
			this->labFurigana2->TabIndex = 2;
			this->labFurigana2->Text = L"(�ӂ肪��)";
			// 
			// labPerson2
			// 
			this->labPerson2->AutoSize = true;
			this->labPerson2->Location = System::Drawing::Point(12, 78);
			this->labPerson2->Name = L"labPerson2";
			this->labPerson2->Size = System::Drawing::Size(38, 12);
			this->labPerson2->TabIndex = 1;
			this->labPerson2->Text = L"������";
			// 
			// labCompany2
			// 
			this->labCompany2->AutoSize = true;
			this->labCompany2->Location = System::Drawing::Point(12, 28);
			this->labCompany2->Name = L"labCompany2";
			this->labCompany2->Size = System::Drawing::Size(41, 12);
			this->labCompany2->TabIndex = 0;
			this->labCompany2->Text = L"�M�Ж�";
			// 
			// gboxPerson1
			// 
			this->gboxPerson1->Controls->Add(this->labNTSC1Sur);
			this->gboxPerson1->Controls->Add(this->labFax1);
			this->gboxPerson1->Controls->Add(this->labNTSC1Pre);
			this->gboxPerson1->Controls->Add(this->tboxNTSC1);
			this->gboxPerson1->Controls->Add(this->tboxFax1);
			this->gboxPerson1->Controls->Add(this->tboxMail1);
			this->gboxPerson1->Controls->Add(this->tboxTel1);
			this->gboxPerson1->Controls->Add(this->tboxFurigana1);
			this->gboxPerson1->Controls->Add(this->tboxPerson1);
			this->gboxPerson1->Controls->Add(this->tboxDepart1);
			this->gboxPerson1->Controls->Add(this->labDepart1);
			this->gboxPerson1->Controls->Add(this->tboxCompany1);
			this->gboxPerson1->Controls->Add(this->labMail1);
			this->gboxPerson1->Controls->Add(this->labTel1);
			this->gboxPerson1->Controls->Add(this->labFurigana1);
			this->gboxPerson1->Controls->Add(this->labPerson1);
			this->gboxPerson1->Controls->Add(this->labCompany1);
			this->gboxPerson1->Location = System::Drawing::Point(27, 35);
			this->gboxPerson1->Name = L"gboxPerson1";
			this->gboxPerson1->Size = System::Drawing::Size(304, 210);
			this->gboxPerson1->TabIndex = 8;
			this->gboxPerson1->TabStop = false;
			this->gboxPerson1->Text = L"�S����1";
			// 
			// labNTSC1Sur
			// 
			this->labNTSC1Sur->AutoSize = true;
			this->labNTSC1Sur->Location = System::Drawing::Point(18, 189);
			this->labNTSC1Sur->Name = L"labNTSC1Sur";
			this->labNTSC1Sur->Size = System::Drawing::Size(44, 12);
			this->labNTSC1Sur->TabIndex = 10;
			this->labNTSC1Sur->Text = L"User ID";
			// 
			// labFax1
			// 
			this->labFax1->AutoSize = true;
			this->labFax1->Location = System::Drawing::Point(159, 128);
			this->labFax1->Name = L"labFax1";
			this->labFax1->Size = System::Drawing::Size(27, 12);
			this->labFax1->TabIndex = 4;
			this->labFax1->Text = L"FAX";
			// 
			// labNTSC1Pre
			// 
			this->labNTSC1Pre->AutoSize = true;
			this->labNTSC1Pre->Location = System::Drawing::Point(18, 175);
			this->labNTSC1Pre->Name = L"labNTSC1Pre";
			this->labNTSC1Pre->Size = System::Drawing::Size(35, 12);
			this->labNTSC1Pre->TabIndex = 9;
			this->labNTSC1Pre->Text = L"NTSC";
			// 
			// tboxNTSC1
			// 
			this->tboxNTSC1->Location = System::Drawing::Point(66, 175);
			this->tboxNTSC1->Name = L"tboxNTSC1";
			this->tboxNTSC1->Size = System::Drawing::Size(220, 19);
			this->tboxNTSC1->TabIndex = 8;
			// 
			// tboxFax1
			// 
			this->tboxFax1->Location = System::Drawing::Point(192, 125);
			this->tboxFax1->Name = L"tboxFax1";
			this->tboxFax1->Size = System::Drawing::Size(94, 19);
			this->tboxFax1->TabIndex = 5;
			// 
			// tboxMail1
			// 
			this->tboxMail1->Location = System::Drawing::Point(66, 150);
			this->tboxMail1->Name = L"tboxMail1";
			this->tboxMail1->Size = System::Drawing::Size(220, 19);
			this->tboxMail1->TabIndex = 6;
			// 
			// tboxTel1
			// 
			this->tboxTel1->Location = System::Drawing::Point(66, 125);
			this->tboxTel1->Name = L"tboxTel1";
			this->tboxTel1->Size = System::Drawing::Size(84, 19);
			this->tboxTel1->TabIndex = 4;
			// 
			// tboxFurigana1
			// 
			this->tboxFurigana1->Location = System::Drawing::Point(66, 100);
			this->tboxFurigana1->Name = L"tboxFurigana1";
			this->tboxFurigana1->Size = System::Drawing::Size(220, 19);
			this->tboxFurigana1->TabIndex = 3;
			// 
			// tboxPerson1
			// 
			this->tboxPerson1->Location = System::Drawing::Point(66, 75);
			this->tboxPerson1->Name = L"tboxPerson1";
			this->tboxPerson1->Size = System::Drawing::Size(220, 19);
			this->tboxPerson1->TabIndex = 2;
			// 
			// tboxDepart1
			// 
			this->tboxDepart1->Location = System::Drawing::Point(66, 50);
			this->tboxDepart1->Name = L"tboxDepart1";
			this->tboxDepart1->Size = System::Drawing::Size(220, 19);
			this->tboxDepart1->TabIndex = 1;
			// 
			// labDepart1
			// 
			this->labDepart1->AutoSize = true;
			this->labDepart1->Location = System::Drawing::Point(12, 53);
			this->labDepart1->Name = L"labDepart1";
			this->labDepart1->Size = System::Drawing::Size(41, 12);
			this->labDepart1->TabIndex = 7;
			this->labDepart1->Text = L"������";
			// 
			// tboxCompany1
			// 
			this->tboxCompany1->Location = System::Drawing::Point(66, 25);
			this->tboxCompany1->Name = L"tboxCompany1";
			this->tboxCompany1->Size = System::Drawing::Size(220, 19);
			this->tboxCompany1->TabIndex = 0;
			// 
			// labMail1
			// 
			this->labMail1->AutoSize = true;
			this->labMail1->Location = System::Drawing::Point(18, 153);
			this->labMail1->Name = L"labMail1";
			this->labMail1->Size = System::Drawing::Size(38, 12);
			this->labMail1->TabIndex = 5;
			this->labMail1->Text = L"e-mail";
			// 
			// labTel1
			// 
			this->labTel1->AutoSize = true;
			this->labTel1->Location = System::Drawing::Point(28, 128);
			this->labTel1->Name = L"labTel1";
			this->labTel1->Size = System::Drawing::Size(25, 12);
			this->labTel1->TabIndex = 3;
			this->labTel1->Text = L"TEL";
			// 
			// labFurigana1
			// 
			this->labFurigana1->AutoSize = true;
			this->labFurigana1->Location = System::Drawing::Point(9, 103);
			this->labFurigana1->Name = L"labFurigana1";
			this->labFurigana1->Size = System::Drawing::Size(51, 12);
			this->labFurigana1->TabIndex = 2;
			this->labFurigana1->Text = L"(�ӂ肪��)";
			// 
			// labPerson1
			// 
			this->labPerson1->AutoSize = true;
			this->labPerson1->Location = System::Drawing::Point(12, 78);
			this->labPerson1->Name = L"labPerson1";
			this->labPerson1->Size = System::Drawing::Size(38, 12);
			this->labPerson1->TabIndex = 1;
			this->labPerson1->Text = L"������";
			// 
			// labCompany1
			// 
			this->labCompany1->AutoSize = true;
			this->labCompany1->Location = System::Drawing::Point(12, 28);
			this->labCompany1->Name = L"labCompany1";
			this->labCompany1->Size = System::Drawing::Size(41, 12);
			this->labCompany1->TabIndex = 0;
			this->labCompany1->Text = L"�M�Ж�";
			// 
			// tboxProductCode2
			// 
			this->tboxProductCode2->Location = System::Drawing::Point(166, 92);
			this->tboxProductCode2->MaxLength = 4;
			this->tboxProductCode2->Name = L"tboxProductCode2";
			this->tboxProductCode2->Size = System::Drawing::Size(56, 19);
			this->tboxProductCode2->TabIndex = 2;
			// 
			// tboxProductCode1
			// 
			this->tboxProductCode1->Location = System::Drawing::Point(131, 92);
			this->tboxProductCode1->MaxLength = 1;
			this->tboxProductCode1->Name = L"tboxProductCode1";
			this->tboxProductCode1->Size = System::Drawing::Size(18, 19);
			this->tboxProductCode1->TabIndex = 1;
			// 
			// tboxProductName
			// 
			this->tboxProductName->Location = System::Drawing::Point(97, 63);
			this->tboxProductName->Name = L"tboxProductName";
			this->tboxProductName->Size = System::Drawing::Size(266, 19);
			this->tboxProductName->TabIndex = 0;
			// 
			// labProductCode2
			// 
			this->labProductCode2->AutoSize = true;
			this->labProductCode2->Location = System::Drawing::Point(155, 95);
			this->labProductCode2->Name = L"labProductCode2";
			this->labProductCode2->Size = System::Drawing::Size(11, 12);
			this->labProductCode2->TabIndex = 33;
			this->labProductCode2->Text = L"-";
			// 
			// labProductCode1
			// 
			this->labProductCode1->AutoSize = true;
			this->labProductCode1->Location = System::Drawing::Point(95, 95);
			this->labProductCode1->Name = L"labProductCode1";
			this->labProductCode1->Size = System::Drawing::Size(37, 12);
			this->labProductCode1->TabIndex = 32;
			this->labProductCode1->Text = L"TWL -";
			// 
			// dateSubmit
			// 
			this->dateSubmit->Format = System::Windows::Forms::DateTimePickerFormat::Short;
			this->dateSubmit->Location = System::Drawing::Point(97, 121);
			this->dateSubmit->MaxDate = System::DateTime(2099, 12, 31, 0, 0, 0, 0);
			this->dateSubmit->MinDate = System::DateTime(2008, 1, 1, 0, 0, 0, 0);
			this->dateSubmit->Name = L"dateSubmit";
			this->dateSubmit->Size = System::Drawing::Size(103, 19);
			this->dateSubmit->TabIndex = 4;
			this->dateSubmit->Value = System::DateTime(2008, 7, 30, 17, 43, 18, 405);
			// 
			// dateRelease
			// 
			this->dateRelease->Format = System::Windows::Forms::DateTimePickerFormat::Short;
			this->dateRelease->Location = System::Drawing::Point(97, 152);
			this->dateRelease->MaxDate = System::DateTime(2099, 12, 31, 0, 0, 0, 0);
			this->dateRelease->MinDate = System::DateTime(2008, 1, 1, 0, 0, 0, 0);
			this->dateRelease->Name = L"dateRelease";
			this->dateRelease->Size = System::Drawing::Size(103, 19);
			this->dateRelease->TabIndex = 3;
			this->dateRelease->Value = System::DateTime(2008, 7, 30, 17, 43, 18, 420);
			// 
			// gboxUsage
			// 
			this->gboxUsage->Controls->Add(this->tboxUsageOther);
			this->gboxUsage->Controls->Add(this->rUsageOther);
			this->gboxUsage->Controls->Add(this->rUsageDst);
			this->gboxUsage->Controls->Add(this->rUsageSample);
			this->gboxUsage->Controls->Add(this->rUsageSale);
			this->gboxUsage->Location = System::Drawing::Point(14, 181);
			this->gboxUsage->Name = L"gboxUsage";
			this->gboxUsage->Size = System::Drawing::Size(364, 74);
			this->gboxUsage->TabIndex = 6;
			this->gboxUsage->TabStop = false;
			this->gboxUsage->Text = L"�p�r";
			// 
			// tboxUsageOther
			// 
			this->tboxUsageOther->Enabled = false;
			this->tboxUsageOther->Location = System::Drawing::Point(71, 42);
			this->tboxUsageOther->Name = L"tboxUsageOther";
			this->tboxUsageOther->Size = System::Drawing::Size(278, 19);
			this->tboxUsageOther->TabIndex = 4;
			// 
			// rUsageOther
			// 
			this->rUsageOther->AutoSize = true;
			this->rUsageOther->Location = System::Drawing::Point(6, 43);
			this->rUsageOther->Name = L"rUsageOther";
			this->rUsageOther->Size = System::Drawing::Size(54, 16);
			this->rUsageOther->TabIndex = 3;
			this->rUsageOther->Text = L"���̑�";
			this->rUsageOther->UseVisualStyleBackColor = true;
			this->rUsageOther->CheckedChanged += gcnew System::EventHandler(this, &Form1::rUsageOther_CheckedChanged);
			// 
			// rUsageDst
			// 
			this->rUsageDst->AutoSize = true;
			this->rUsageDst->Location = System::Drawing::Point(198, 17);
			this->rUsageDst->Name = L"rUsageDst";
			this->rUsageDst->Size = System::Drawing::Size(87, 16);
			this->rUsageDst->TabIndex = 2;
			this->rUsageDst->Text = L"�f�[�^�z�M�p";
			this->rUsageDst->UseVisualStyleBackColor = true;
			// 
			// rUsageSample
			// 
			this->rUsageSample->AutoSize = true;
			this->rUsageSample->Location = System::Drawing::Point(106, 17);
			this->rUsageSample->Name = L"rUsageSample";
			this->rUsageSample->Size = System::Drawing::Size(71, 16);
			this->rUsageSample->TabIndex = 1;
			this->rUsageSample->Text = L"���V��p";
			this->rUsageSample->UseVisualStyleBackColor = true;
			// 
			// rUsageSale
			// 
			this->rUsageSale->AutoSize = true;
			this->rUsageSale->Checked = true;
			this->rUsageSale->Location = System::Drawing::Point(6, 17);
			this->rUsageSale->Name = L"rUsageSale";
			this->rUsageSale->Size = System::Drawing::Size(83, 16);
			this->rUsageSale->TabIndex = 0;
			this->rUsageSale->TabStop = true;
			this->rUsageSale->Text = L"��ʔ̔��p";
			this->rUsageSale->UseVisualStyleBackColor = true;
			// 
			// gboxSubmitWay
			// 
			this->gboxSubmitWay->Controls->Add(this->rSubmitHand);
			this->gboxSubmitWay->Controls->Add(this->rSubmitPost);
			this->gboxSubmitWay->Location = System::Drawing::Point(244, 114);
			this->gboxSubmitWay->Name = L"gboxSubmitWay";
			this->gboxSubmitWay->Size = System::Drawing::Size(94, 63);
			this->gboxSubmitWay->TabIndex = 5;
			this->gboxSubmitWay->TabStop = false;
			this->gboxSubmitWay->Text = L"��o���@";
			// 
			// rSubmitHand
			// 
			this->rSubmitHand->AutoSize = true;
			this->rSubmitHand->Location = System::Drawing::Point(6, 40);
			this->rSubmitHand->Name = L"rSubmitHand";
			this->rSubmitHand->Size = System::Drawing::Size(56, 16);
			this->rSubmitHand->TabIndex = 1;
			this->rSubmitHand->Text = L"��n��";
			this->rSubmitHand->UseVisualStyleBackColor = true;
			// 
			// rSubmitPost
			// 
			this->rSubmitPost->AutoSize = true;
			this->rSubmitPost->Checked = true;
			this->rSubmitPost->Location = System::Drawing::Point(6, 18);
			this->rSubmitPost->Name = L"rSubmitPost";
			this->rSubmitPost->Size = System::Drawing::Size(47, 16);
			this->rSubmitPost->TabIndex = 0;
			this->rSubmitPost->TabStop = true;
			this->rSubmitPost->Text = L"�X��";
			this->rSubmitPost->UseVisualStyleBackColor = true;
			// 
			// labSubmiteDate
			// 
			this->labSubmiteDate->AutoSize = true;
			this->labSubmiteDate->Location = System::Drawing::Point(9, 124);
			this->labSubmiteDate->Name = L"labSubmiteDate";
			this->labSubmiteDate->Size = System::Drawing::Size(65, 12);
			this->labSubmiteDate->TabIndex = 11;
			this->labSubmiteDate->Text = L"���ޒ�o��";
			// 
			// labReleaseDate
			// 
			this->labReleaseDate->AutoSize = true;
			this->labReleaseDate->Location = System::Drawing::Point(9, 156);
			this->labReleaseDate->Name = L"labReleaseDate";
			this->labReleaseDate->Size = System::Drawing::Size(65, 12);
			this->labReleaseDate->TabIndex = 7;
			this->labReleaseDate->Text = L"�����\���";
			// 
			// labProductCode
			// 
			this->labProductCode->AutoSize = true;
			this->labProductCode->Location = System::Drawing::Point(9, 95);
			this->labProductCode->Name = L"labProductCode";
			this->labProductCode->Size = System::Drawing::Size(56, 12);
			this->labProductCode->TabIndex = 6;
			this->labProductCode->Text = L"���i�R�[�h";
			// 
			// labProductName
			// 
			this->labProductName->AutoSize = true;
			this->labProductName->Location = System::Drawing::Point(10, 66);
			this->labProductName->Name = L"labProductName";
			this->labProductName->Size = System::Drawing::Size(41, 12);
			this->labProductName->TabIndex = 5;
			this->labProductName->Text = L"���i��";
			// 
			// labCapSubmitVer
			// 
			this->labCapSubmitVer->AutoSize = true;
			this->labCapSubmitVer->Location = System::Drawing::Point(138, 37);
			this->labCapSubmitVer->Name = L"labCapSubmitVer";
			this->labCapSubmitVer->Size = System::Drawing::Size(233, 12);
			this->labCapSubmitVer->TabIndex = 26;
			this->labCapSubmitVer->Text = L"* ���}�X�^�[�o�[�W�������オ��ƍĂ�0����J�E���g";
			// 
			// numSubmitVersion
			// 
			this->numSubmitVersion->Location = System::Drawing::Point(97, 34);
			this->numSubmitVersion->Name = L"numSubmitVersion";
			this->numSubmitVersion->Size = System::Drawing::Size(35, 19);
			this->numSubmitVersion->TabIndex = 9;
			// 
			// labSubmitVer
			// 
			this->labSubmitVer->AutoSize = true;
			this->labSubmitVer->Location = System::Drawing::Point(9, 36);
			this->labSubmitVer->Name = L"labSubmitVer";
			this->labSubmitVer->Size = System::Drawing::Size(74, 12);
			this->labSubmitVer->TabIndex = 24;
			this->labSubmitVer->Text = L"��o�o�[�W����";
			// 
			// tabDoc
			// 
			this->tabDoc->Controls->Add(this->tabSubmitInfo);
			this->tabDoc->Controls->Add(this->tabForeignInfo);
			this->tabDoc->Controls->Add(this->tabCompanyInfo1);
			this->tabDoc->Controls->Add(this->tabCompanyInfo2);
			this->tabDoc->Controls->Add(this->tabCaption);
			this->tabDoc->Location = System::Drawing::Point(12, 331);
			this->tabDoc->Name = L"tabDoc";
			this->tabDoc->SelectedIndex = 0;
			this->tabDoc->Size = System::Drawing::Size(392, 288);
			this->tabDoc->TabIndex = 31;
			// 
			// tabSubmitInfo
			// 
			this->tabSubmitInfo->Controls->Add(this->labCautionInput);
			this->tabSubmitInfo->Controls->Add(this->tboxProductCode2);
			this->tabSubmitInfo->Controls->Add(this->labSubmitVer);
			this->tabSubmitInfo->Controls->Add(this->tboxProductCode1);
			this->tabSubmitInfo->Controls->Add(this->numSubmitVersion);
			this->tabSubmitInfo->Controls->Add(this->tboxProductName);
			this->tabSubmitInfo->Controls->Add(this->labCapSubmitVer);
			this->tabSubmitInfo->Controls->Add(this->labProductCode2);
			this->tabSubmitInfo->Controls->Add(this->labProductName);
			this->tabSubmitInfo->Controls->Add(this->labProductCode1);
			this->tabSubmitInfo->Controls->Add(this->labProductCode);
			this->tabSubmitInfo->Controls->Add(this->dateSubmit);
			this->tabSubmitInfo->Controls->Add(this->labReleaseDate);
			this->tabSubmitInfo->Controls->Add(this->dateRelease);
			this->tabSubmitInfo->Controls->Add(this->labSubmiteDate);
			this->tabSubmitInfo->Controls->Add(this->gboxSubmitWay);
			this->tabSubmitInfo->Controls->Add(this->gboxUsage);
			this->tabSubmitInfo->Location = System::Drawing::Point(4, 21);
			this->tabSubmitInfo->Name = L"tabSubmitInfo";
			this->tabSubmitInfo->Padding = System::Windows::Forms::Padding(3);
			this->tabSubmitInfo->Size = System::Drawing::Size(384, 263);
			this->tabSubmitInfo->TabIndex = 0;
			this->tabSubmitInfo->Text = L"��o���";
			this->tabSubmitInfo->UseVisualStyleBackColor = true;
			// 
			// labCautionInput
			// 
			this->labCautionInput->AutoSize = true;
			this->labCautionInput->ForeColor = System::Drawing::SystemColors::ActiveCaption;
			this->labCautionInput->Location = System::Drawing::Point(9, 8);
			this->labCautionInput->Name = L"labCautionInput";
			this->labCautionInput->Size = System::Drawing::Size(312, 12);
			this->labCautionInput->TabIndex = 34;
			this->labCautionInput->Text = L"* �����̍��ڂ͏��ޒ�o�ɕK�v�ȏ��ł��B���͂��Ă��������B";
			// 
			// tabForeignInfo
			// 
			this->tabForeignInfo->Controls->Add(this->labMultiForeign2);
			this->tabForeignInfo->Controls->Add(this->labMultiForeign1);
			this->tabForeignInfo->Controls->Add(this->tboxProductCode2Foreign3);
			this->tabForeignInfo->Controls->Add(this->tboxProductCode2Foreign2);
			this->tabForeignInfo->Controls->Add(this->labProductCode2Foreign);
			this->tabForeignInfo->Controls->Add(this->cboxReleaseForeign);
			this->tabForeignInfo->Controls->Add(this->labProductNameForeign);
			this->tabForeignInfo->Controls->Add(this->tboxProductNameForeign);
			this->tabForeignInfo->Controls->Add(this->labProductCode1Foreign);
			this->tabForeignInfo->Controls->Add(this->tboxProductCode1Foreign);
			this->tabForeignInfo->Controls->Add(this->labProductCodeForeign);
			this->tabForeignInfo->Controls->Add(this->tboxProductCode2Foreign1);
			this->tabForeignInfo->Location = System::Drawing::Point(4, 21);
			this->tabForeignInfo->Name = L"tabForeignInfo";
			this->tabForeignInfo->Padding = System::Windows::Forms::Padding(3);
			this->tabForeignInfo->Size = System::Drawing::Size(384, 263);
			this->tabForeignInfo->TabIndex = 1;
			this->tabForeignInfo->Text = L"�C�O��";
			this->tabForeignInfo->UseVisualStyleBackColor = true;
			// 
			// labMultiForeign2
			// 
			this->labMultiForeign2->AutoSize = true;
			this->labMultiForeign2->Location = System::Drawing::Point(221, 152);
			this->labMultiForeign2->Name = L"labMultiForeign2";
			this->labMultiForeign2->Size = System::Drawing::Size(103, 12);
			this->labMultiForeign2->TabIndex = 45;
			this->labMultiForeign2->Text = L"* ��������ꍇ�̂�";
			// 
			// labMultiForeign1
			// 
			this->labMultiForeign1->AutoSize = true;
			this->labMultiForeign1->Location = System::Drawing::Point(221, 127);
			this->labMultiForeign1->Name = L"labMultiForeign1";
			this->labMultiForeign1->Size = System::Drawing::Size(103, 12);
			this->labMultiForeign1->TabIndex = 44;
			this->labMultiForeign1->Text = L"* ��������ꍇ�̂�";
			// 
			// tboxProductCode2Foreign3
			// 
			this->tboxProductCode2Foreign3->Enabled = false;
			this->tboxProductCode2Foreign3->Location = System::Drawing::Point(169, 149);
			this->tboxProductCode2Foreign3->MaxLength = 4;
			this->tboxProductCode2Foreign3->Name = L"tboxProductCode2Foreign3";
			this->tboxProductCode2Foreign3->Size = System::Drawing::Size(46, 19);
			this->tboxProductCode2Foreign3->TabIndex = 43;
			// 
			// tboxProductCode2Foreign2
			// 
			this->tboxProductCode2Foreign2->Enabled = false;
			this->tboxProductCode2Foreign2->Location = System::Drawing::Point(169, 124);
			this->tboxProductCode2Foreign2->MaxLength = 4;
			this->tboxProductCode2Foreign2->Name = L"tboxProductCode2Foreign2";
			this->tboxProductCode2Foreign2->Size = System::Drawing::Size(46, 19);
			this->tboxProductCode2Foreign2->TabIndex = 42;
			// 
			// labProductCode2Foreign
			// 
			this->labProductCode2Foreign->AutoSize = true;
			this->labProductCode2Foreign->Location = System::Drawing::Point(156, 105);
			this->labProductCode2Foreign->Name = L"labProductCode2Foreign";
			this->labProductCode2Foreign->Size = System::Drawing::Size(11, 12);
			this->labProductCode2Foreign->TabIndex = 41;
			this->labProductCode2Foreign->Text = L"-";
			// 
			// cboxReleaseForeign
			// 
			this->cboxReleaseForeign->AutoSize = true;
			this->cboxReleaseForeign->Location = System::Drawing::Point(25, 33);
			this->cboxReleaseForeign->Name = L"cboxReleaseForeign";
			this->cboxReleaseForeign->Size = System::Drawing::Size(144, 16);
			this->cboxReleaseForeign->TabIndex = 11;
			this->cboxReleaseForeign->Text = L"�C�O�ł𔭔�����(�\��)";
			this->cboxReleaseForeign->UseVisualStyleBackColor = true;
			this->cboxReleaseForeign->CheckedChanged += gcnew System::EventHandler(this, &Form1::cboxReleaseForeign_CheckedChanged);
			// 
			// labProductNameForeign
			// 
			this->labProductNameForeign->AutoSize = true;
			this->labProductNameForeign->Location = System::Drawing::Point(19, 67);
			this->labProductNameForeign->Name = L"labProductNameForeign";
			this->labProductNameForeign->Size = System::Drawing::Size(41, 12);
			this->labProductNameForeign->TabIndex = 37;
			this->labProductNameForeign->Text = L"���i��";
			// 
			// tboxProductNameForeign
			// 
			this->tboxProductNameForeign->Enabled = false;
			this->tboxProductNameForeign->Location = System::Drawing::Point(96, 64);
			this->tboxProductNameForeign->Name = L"tboxProductNameForeign";
			this->tboxProductNameForeign->Size = System::Drawing::Size(187, 19);
			this->tboxProductNameForeign->TabIndex = 12;
			// 
			// labProductCode1Foreign
			// 
			this->labProductCode1Foreign->AutoSize = true;
			this->labProductCode1Foreign->Location = System::Drawing::Point(94, 105);
			this->labProductCode1Foreign->Name = L"labProductCode1Foreign";
			this->labProductCode1Foreign->Size = System::Drawing::Size(37, 12);
			this->labProductCode1Foreign->TabIndex = 40;
			this->labProductCode1Foreign->Text = L"TWL -";
			// 
			// tboxProductCode1Foreign
			// 
			this->tboxProductCode1Foreign->Enabled = false;
			this->tboxProductCode1Foreign->Location = System::Drawing::Point(133, 100);
			this->tboxProductCode1Foreign->MaxLength = 1;
			this->tboxProductCode1Foreign->Name = L"tboxProductCode1Foreign";
			this->tboxProductCode1Foreign->Size = System::Drawing::Size(18, 19);
			this->tboxProductCode1Foreign->TabIndex = 13;
			// 
			// labProductCodeForeign
			// 
			this->labProductCodeForeign->AutoSize = true;
			this->labProductCodeForeign->Location = System::Drawing::Point(19, 105);
			this->labProductCodeForeign->Name = L"labProductCodeForeign";
			this->labProductCodeForeign->Size = System::Drawing::Size(56, 12);
			this->labProductCodeForeign->TabIndex = 38;
			this->labProductCodeForeign->Text = L"���i�R�[�h";
			// 
			// tboxProductCode2Foreign1
			// 
			this->tboxProductCode2Foreign1->Enabled = false;
			this->tboxProductCode2Foreign1->Location = System::Drawing::Point(169, 99);
			this->tboxProductCode2Foreign1->MaxLength = 4;
			this->tboxProductCode2Foreign1->Name = L"tboxProductCode2Foreign1";
			this->tboxProductCode2Foreign1->Size = System::Drawing::Size(46, 19);
			this->tboxProductCode2Foreign1->TabIndex = 14;
			// 
			// tabCompanyInfo1
			// 
			this->tabCompanyInfo1->Controls->Add(this->gboxPerson1);
			this->tabCompanyInfo1->Location = System::Drawing::Point(4, 21);
			this->tabCompanyInfo1->Name = L"tabCompanyInfo1";
			this->tabCompanyInfo1->Size = System::Drawing::Size(384, 263);
			this->tabCompanyInfo1->TabIndex = 2;
			this->tabCompanyInfo1->Text = L"��Џ��1";
			this->tabCompanyInfo1->UseVisualStyleBackColor = true;
			// 
			// tabCompanyInfo2
			// 
			this->tabCompanyInfo2->Controls->Add(this->cboxIsInputPerson2);
			this->tabCompanyInfo2->Controls->Add(this->gboxPerson2);
			this->tabCompanyInfo2->Location = System::Drawing::Point(4, 21);
			this->tabCompanyInfo2->Name = L"tabCompanyInfo2";
			this->tabCompanyInfo2->Size = System::Drawing::Size(384, 263);
			this->tabCompanyInfo2->TabIndex = 3;
			this->tabCompanyInfo2->Text = L"��Џ��2";
			this->tabCompanyInfo2->UseVisualStyleBackColor = true;
			// 
			// tabCaption
			// 
			this->tabCaption->Controls->Add(this->labCaption);
			this->tabCaption->Controls->Add(this->tboxCaption);
			this->tabCaption->Location = System::Drawing::Point(4, 21);
			this->tabCaption->Name = L"tabCaption";
			this->tabCaption->Padding = System::Windows::Forms::Padding(3);
			this->tabCaption->Size = System::Drawing::Size(384, 263);
			this->tabCaption->TabIndex = 5;
			this->tabCaption->Text = L"���l";
			this->tabCaption->UseVisualStyleBackColor = true;
			// 
			// tabTWLSpec4
			// 
			this->tabTWLSpec4->Controls->Add(this->gboxShared2Size);
			this->tabTWLSpec4->Location = System::Drawing::Point(4, 21);
			this->tabTWLSpec4->Name = L"tabTWLSpec4";
			this->tabTWLSpec4->Padding = System::Windows::Forms::Padding(3);
			this->tabTWLSpec4->Size = System::Drawing::Size(391, 263);
			this->tabTWLSpec4->TabIndex = 12;
			this->tabTWLSpec4->Text = L"TWL�d�l4";
			this->tabTWLSpec4->UseVisualStyleBackColor = true;
			// 
			// gboxShared2Size
			// 
			this->gboxShared2Size->Controls->Add(this->labShared2Size5);
			this->gboxShared2Size->Controls->Add(this->labShared2Size4);
			this->gboxShared2Size->Controls->Add(this->labShared2Size3);
			this->gboxShared2Size->Controls->Add(this->labShared2Size2);
			this->gboxShared2Size->Controls->Add(this->labShared2Size1);
			this->gboxShared2Size->Controls->Add(this->labShared2Size0);
			this->gboxShared2Size->Controls->Add(this->labKB5);
			this->gboxShared2Size->Controls->Add(this->labKB4);
			this->gboxShared2Size->Controls->Add(this->labKB3);
			this->gboxShared2Size->Controls->Add(this->labKB2);
			this->gboxShared2Size->Controls->Add(this->labKB1);
			this->gboxShared2Size->Controls->Add(this->labKB0);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size5);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size4);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size3);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size2);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size1);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size0);
			this->gboxShared2Size->Controls->Add(this->cboxIsShared2);
			this->gboxShared2Size->Location = System::Drawing::Point(21, 17);
			this->gboxShared2Size->Name = L"gboxShared2Size";
			this->gboxShared2Size->Size = System::Drawing::Size(270, 220);
			this->gboxShared2Size->TabIndex = 1;
			this->gboxShared2Size->TabStop = false;
			this->gboxShared2Size->Text = L"Shared2�t�@�C���T�C�Y";
			// 
			// labShared2Size5
			// 
			this->labShared2Size5->AutoSize = true;
			this->labShared2Size5->Location = System::Drawing::Point(27, 185);
			this->labShared2Size5->Name = L"labShared2Size5";
			this->labShared2Size5->Size = System::Drawing::Size(34, 12);
			this->labShared2Size5->TabIndex = 20;
			this->labShared2Size5->Text = L"File 5";
			// 
			// labShared2Size4
			// 
			this->labShared2Size4->AutoSize = true;
			this->labShared2Size4->Location = System::Drawing::Point(26, 160);
			this->labShared2Size4->Name = L"labShared2Size4";
			this->labShared2Size4->Size = System::Drawing::Size(34, 12);
			this->labShared2Size4->TabIndex = 19;
			this->labShared2Size4->Text = L"File 4";
			// 
			// labShared2Size3
			// 
			this->labShared2Size3->AutoSize = true;
			this->labShared2Size3->Location = System::Drawing::Point(27, 135);
			this->labShared2Size3->Name = L"labShared2Size3";
			this->labShared2Size3->Size = System::Drawing::Size(34, 12);
			this->labShared2Size3->TabIndex = 18;
			this->labShared2Size3->Text = L"File 3";
			// 
			// labShared2Size2
			// 
			this->labShared2Size2->AutoSize = true;
			this->labShared2Size2->Location = System::Drawing::Point(27, 110);
			this->labShared2Size2->Name = L"labShared2Size2";
			this->labShared2Size2->Size = System::Drawing::Size(34, 12);
			this->labShared2Size2->TabIndex = 17;
			this->labShared2Size2->Text = L"File 2";
			// 
			// labShared2Size1
			// 
			this->labShared2Size1->AutoSize = true;
			this->labShared2Size1->Location = System::Drawing::Point(27, 85);
			this->labShared2Size1->Name = L"labShared2Size1";
			this->labShared2Size1->Size = System::Drawing::Size(34, 12);
			this->labShared2Size1->TabIndex = 16;
			this->labShared2Size1->Text = L"File 1";
			// 
			// labShared2Size0
			// 
			this->labShared2Size0->AutoSize = true;
			this->labShared2Size0->Location = System::Drawing::Point(27, 60);
			this->labShared2Size0->Name = L"labShared2Size0";
			this->labShared2Size0->Size = System::Drawing::Size(34, 12);
			this->labShared2Size0->TabIndex = 15;
			this->labShared2Size0->Text = L"File 0";
			// 
			// labKB5
			// 
			this->labKB5->AutoSize = true;
			this->labKB5->Location = System::Drawing::Point(199, 185);
			this->labKB5->Name = L"labKB5";
			this->labKB5->Size = System::Drawing::Size(20, 12);
			this->labKB5->TabIndex = 14;
			this->labKB5->Text = L"KB";
			// 
			// labKB4
			// 
			this->labKB4->AutoSize = true;
			this->labKB4->Location = System::Drawing::Point(199, 160);
			this->labKB4->Name = L"labKB4";
			this->labKB4->Size = System::Drawing::Size(20, 12);
			this->labKB4->TabIndex = 13;
			this->labKB4->Text = L"KB";
			// 
			// labKB3
			// 
			this->labKB3->AutoSize = true;
			this->labKB3->Location = System::Drawing::Point(199, 135);
			this->labKB3->Name = L"labKB3";
			this->labKB3->Size = System::Drawing::Size(20, 12);
			this->labKB3->TabIndex = 12;
			this->labKB3->Text = L"KB";
			// 
			// labKB2
			// 
			this->labKB2->AutoSize = true;
			this->labKB2->Location = System::Drawing::Point(199, 110);
			this->labKB2->Name = L"labKB2";
			this->labKB2->Size = System::Drawing::Size(20, 12);
			this->labKB2->TabIndex = 11;
			this->labKB2->Text = L"KB";
			// 
			// labKB1
			// 
			this->labKB1->AutoSize = true;
			this->labKB1->Location = System::Drawing::Point(199, 85);
			this->labKB1->Name = L"labKB1";
			this->labKB1->Size = System::Drawing::Size(20, 12);
			this->labKB1->TabIndex = 10;
			this->labKB1->Text = L"KB";
			// 
			// labKB0
			// 
			this->labKB0->AutoSize = true;
			this->labKB0->Location = System::Drawing::Point(199, 60);
			this->labKB0->Name = L"labKB0";
			this->labKB0->Size = System::Drawing::Size(20, 12);
			this->labKB0->TabIndex = 9;
			this->labKB0->Text = L"KB";
			// 
			// tboxShared2Size5
			// 
			this->tboxShared2Size5->Location = System::Drawing::Point(66, 182);
			this->tboxShared2Size5->Name = L"tboxShared2Size5";
			this->tboxShared2Size5->ReadOnly = true;
			this->tboxShared2Size5->Size = System::Drawing::Size(126, 19);
			this->tboxShared2Size5->TabIndex = 8;
			this->tboxShared2Size5->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size4
			// 
			this->tboxShared2Size4->Location = System::Drawing::Point(67, 157);
			this->tboxShared2Size4->Name = L"tboxShared2Size4";
			this->tboxShared2Size4->ReadOnly = true;
			this->tboxShared2Size4->Size = System::Drawing::Size(126, 19);
			this->tboxShared2Size4->TabIndex = 7;
			this->tboxShared2Size4->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size3
			// 
			this->tboxShared2Size3->Location = System::Drawing::Point(67, 132);
			this->tboxShared2Size3->Name = L"tboxShared2Size3";
			this->tboxShared2Size3->ReadOnly = true;
			this->tboxShared2Size3->Size = System::Drawing::Size(126, 19);
			this->tboxShared2Size3->TabIndex = 6;
			this->tboxShared2Size3->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size2
			// 
			this->tboxShared2Size2->Location = System::Drawing::Point(67, 107);
			this->tboxShared2Size2->Name = L"tboxShared2Size2";
			this->tboxShared2Size2->ReadOnly = true;
			this->tboxShared2Size2->Size = System::Drawing::Size(126, 19);
			this->tboxShared2Size2->TabIndex = 5;
			this->tboxShared2Size2->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size1
			// 
			this->tboxShared2Size1->Location = System::Drawing::Point(67, 82);
			this->tboxShared2Size1->Name = L"tboxShared2Size1";
			this->tboxShared2Size1->ReadOnly = true;
			this->tboxShared2Size1->Size = System::Drawing::Size(126, 19);
			this->tboxShared2Size1->TabIndex = 4;
			this->tboxShared2Size1->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size0
			// 
			this->tboxShared2Size0->Location = System::Drawing::Point(67, 57);
			this->tboxShared2Size0->Name = L"tboxShared2Size0";
			this->tboxShared2Size0->ReadOnly = true;
			this->tboxShared2Size0->Size = System::Drawing::Size(126, 19);
			this->tboxShared2Size0->TabIndex = 3;
			this->tboxShared2Size0->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// cboxIsShared2
			// 
			this->cboxIsShared2->AutoSize = true;
			this->cboxIsShared2->Enabled = false;
			this->cboxIsShared2->Location = System::Drawing::Point(28, 30);
			this->cboxIsShared2->Name = L"cboxIsShared2";
			this->cboxIsShared2->Size = System::Drawing::Size(123, 16);
			this->cboxIsShared2->TabIndex = 2;
			this->cboxIsShared2->Text = L"Shared2�t�@�C���g�p";
			this->cboxIsShared2->UseVisualStyleBackColor = true;
			// 
			// tabSDK
			// 
			this->tabSDK->Controls->Add(this->tboxLib);
			this->tabSDK->Controls->Add(this->labLib);
			this->tabSDK->Controls->Add(this->tboxSDK);
			this->tabSDK->Controls->Add(this->labSDK);
			this->tabSDK->Location = System::Drawing::Point(4, 21);
			this->tabSDK->Name = L"tabSDK";
			this->tabSDK->Size = System::Drawing::Size(391, 263);
			this->tabSDK->TabIndex = 11;
			this->tabSDK->Text = L"SDK";
			this->tabSDK->UseVisualStyleBackColor = true;
			// 
			// tboxLib
			// 
			this->tboxLib->Location = System::Drawing::Point(21, 149);
			this->tboxLib->Multiline = true;
			this->tboxLib->Name = L"tboxLib";
			this->tboxLib->ReadOnly = true;
			this->tboxLib->Size = System::Drawing::Size(337, 80);
			this->tboxLib->TabIndex = 32;
			// 
			// labLib
			// 
			this->labLib->AutoSize = true;
			this->labLib->Location = System::Drawing::Point(19, 134);
			this->labLib->Name = L"labLib";
			this->labLib->Size = System::Drawing::Size(70, 12);
			this->labLib->TabIndex = 31;
			this->labLib->Text = L"�g�p���C�u����";
			// 
			// tboxSDK
			// 
			this->tboxSDK->Location = System::Drawing::Point(21, 41);
			this->tboxSDK->Multiline = true;
			this->tboxSDK->Name = L"tboxSDK";
			this->tboxSDK->ReadOnly = true;
			this->tboxSDK->Size = System::Drawing::Size(337, 65);
			this->tboxSDK->TabIndex = 10;
			// 
			// labSDK
			// 
			this->labSDK->AutoSize = true;
			this->labSDK->Location = System::Drawing::Point(19, 26);
			this->labSDK->Name = L"labSDK";
			this->labSDK->Size = System::Drawing::Size(72, 12);
			this->labSDK->TabIndex = 30;
			this->labSDK->Text = L"SDK�o�[�W����";
			// 
			// tabTWLSpec3
			// 
			this->tabTWLSpec3->Controls->Add(this->gboxTWLExInfo);
			this->tabTWLSpec3->Location = System::Drawing::Point(4, 21);
			this->tabTWLSpec3->Name = L"tabTWLSpec3";
			this->tabTWLSpec3->Size = System::Drawing::Size(391, 263);
			this->tabTWLSpec3->TabIndex = 10;
			this->tabTWLSpec3->Text = L"TWL�d�l3";
			this->tabTWLSpec3->UseVisualStyleBackColor = true;
			// 
			// gboxTWLExInfo
			// 
			this->gboxTWLExInfo->Controls->Add(this->labByte2);
			this->gboxTWLExInfo->Controls->Add(this->labByte1);
			this->gboxTWLExInfo->Controls->Add(this->labHex4);
			this->gboxTWLExInfo->Controls->Add(this->labHex3);
			this->gboxTWLExInfo->Controls->Add(this->tboxIsCodec);
			this->gboxTWLExInfo->Controls->Add(this->labIsCodec);
			this->gboxTWLExInfo->Controls->Add(this->labNormalRomOffset);
			this->gboxTWLExInfo->Controls->Add(this->tboxNormalRomOffset);
			this->gboxTWLExInfo->Controls->Add(this->cboxIsSubBanner);
			this->gboxTWLExInfo->Controls->Add(this->labKeyTableRomOffset);
			this->gboxTWLExInfo->Controls->Add(this->cboxIsWL);
			this->gboxTWLExInfo->Controls->Add(this->tboxPrivateSize);
			this->gboxTWLExInfo->Controls->Add(this->labPrivateSize);
			this->gboxTWLExInfo->Controls->Add(this->tboxKeyTableRomOffset);
			this->gboxTWLExInfo->Controls->Add(this->cboxIsNormalJump);
			this->gboxTWLExInfo->Controls->Add(this->cboxIsTmpJump);
			this->gboxTWLExInfo->Controls->Add(this->labPublicSize);
			this->gboxTWLExInfo->Controls->Add(this->tboxPublicSize);
			this->gboxTWLExInfo->Location = System::Drawing::Point(16, 17);
			this->gboxTWLExInfo->Name = L"gboxTWLExInfo";
			this->gboxTWLExInfo->Size = System::Drawing::Size(359, 225);
			this->gboxTWLExInfo->TabIndex = 24;
			this->gboxTWLExInfo->TabStop = false;
			this->gboxTWLExInfo->Text = L"TWL�g�����";
			// 
			// labByte2
			// 
			this->labByte2->AutoSize = true;
			this->labByte2->Location = System::Drawing::Point(585, 54);
			this->labByte2->Name = L"labByte2";
			this->labByte2->Size = System::Drawing::Size(29, 12);
			this->labByte2->TabIndex = 31;
			this->labByte2->Text = L"Byte";
			// 
			// labByte1
			// 
			this->labByte1->AutoSize = true;
			this->labByte1->Location = System::Drawing::Point(585, 29);
			this->labByte1->Name = L"labByte1";
			this->labByte1->Size = System::Drawing::Size(29, 12);
			this->labByte1->TabIndex = 30;
			this->labByte1->Text = L"Byte";
			// 
			// labHex4
			// 
			this->labHex4->AutoSize = true;
			this->labHex4->Location = System::Drawing::Point(270, 58);
			this->labHex4->Name = L"labHex4";
			this->labHex4->Size = System::Drawing::Size(11, 12);
			this->labHex4->TabIndex = 29;
			this->labHex4->Text = L"h";
			// 
			// labHex3
			// 
			this->labHex3->AutoSize = true;
			this->labHex3->Location = System::Drawing::Point(270, 29);
			this->labHex3->Name = L"labHex3";
			this->labHex3->Size = System::Drawing::Size(11, 12);
			this->labHex3->TabIndex = 8;
			this->labHex3->Text = L"h";
			// 
			// tboxIsCodec
			// 
			this->tboxIsCodec->Location = System::Drawing::Point(177, 126);
			this->tboxIsCodec->Name = L"tboxIsCodec";
			this->tboxIsCodec->ReadOnly = true;
			this->tboxIsCodec->Size = System::Drawing::Size(87, 19);
			this->tboxIsCodec->TabIndex = 28;
			// 
			// labIsCodec
			// 
			this->labIsCodec->AutoSize = true;
			this->labIsCodec->Location = System::Drawing::Point(60, 129);
			this->labIsCodec->Name = L"labIsCodec";
			this->labIsCodec->Size = System::Drawing::Size(75, 12);
			this->labIsCodec->TabIndex = 27;
			this->labIsCodec->Text = L"CODEC Mode";
			// 
			// labNormalRomOffset
			// 
			this->labNormalRomOffset->AutoSize = true;
			this->labNormalRomOffset->Location = System::Drawing::Point(12, 29);
			this->labNormalRomOffset->Name = L"labNormalRomOffset";
			this->labNormalRomOffset->Size = System::Drawing::Size(155, 12);
			this->labNormalRomOffset->TabIndex = 9;
			this->labNormalRomOffset->Text = L"TWL�m�[�}���̈�ROM�I�t�Z�b�g";
			// 
			// tboxNormalRomOffset
			// 
			this->tboxNormalRomOffset->Location = System::Drawing::Point(177, 26);
			this->tboxNormalRomOffset->Name = L"tboxNormalRomOffset";
			this->tboxNormalRomOffset->ReadOnly = true;
			this->tboxNormalRomOffset->Size = System::Drawing::Size(87, 19);
			this->tboxNormalRomOffset->TabIndex = 8;
			this->tboxNormalRomOffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// cboxIsSubBanner
			// 
			this->cboxIsSubBanner->AutoSize = true;
			this->cboxIsSubBanner->Enabled = false;
			this->cboxIsSubBanner->Location = System::Drawing::Point(159, 168);
			this->cboxIsSubBanner->Name = L"cboxIsSubBanner";
			this->cboxIsSubBanner->Size = System::Drawing::Size(131, 16);
			this->cboxIsSubBanner->TabIndex = 26;
			this->cboxIsSubBanner->Text = L"�T�u�o�i�[�t�@�C���L��";
			this->cboxIsSubBanner->UseVisualStyleBackColor = true;
			// 
			// labKeyTableRomOffset
			// 
			this->labKeyTableRomOffset->AutoSize = true;
			this->labKeyTableRomOffset->Location = System::Drawing::Point(12, 54);
			this->labKeyTableRomOffset->Name = L"labKeyTableRomOffset";
			this->labKeyTableRomOffset->Size = System::Drawing::Size(142, 12);
			this->labKeyTableRomOffset->TabIndex = 11;
			this->labKeyTableRomOffset->Text = L"TWL��p�̈�ROM�I�t�Z�b�g";
			// 
			// cboxIsWL
			// 
			this->cboxIsWL->AutoSize = true;
			this->cboxIsWL->Enabled = false;
			this->cboxIsWL->Location = System::Drawing::Point(159, 196);
			this->cboxIsWL->Name = L"cboxIsWL";
			this->cboxIsWL->Size = System::Drawing::Size(155, 16);
			this->cboxIsWL->TabIndex = 25;
			this->cboxIsWL->Text = L"NTR�z���C�g���X�g�����L��";
			this->cboxIsWL->UseVisualStyleBackColor = true;
			// 
			// tboxPrivateSize
			// 
			this->tboxPrivateSize->Location = System::Drawing::Point(177, 101);
			this->tboxPrivateSize->Name = L"tboxPrivateSize";
			this->tboxPrivateSize->ReadOnly = true;
			this->tboxPrivateSize->Size = System::Drawing::Size(87, 19);
			this->tboxPrivateSize->TabIndex = 13;
			this->tboxPrivateSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// labPrivateSize
			// 
			this->labPrivateSize->AutoSize = true;
			this->labPrivateSize->Location = System::Drawing::Point(36, 104);
			this->labPrivateSize->Name = L"labPrivateSize";
			this->labPrivateSize->Size = System::Drawing::Size(123, 12);
			this->labPrivateSize->TabIndex = 15;
			this->labPrivateSize->Text = L"Private Save Data Size";
			// 
			// tboxKeyTableRomOffset
			// 
			this->tboxKeyTableRomOffset->Location = System::Drawing::Point(177, 51);
			this->tboxKeyTableRomOffset->Name = L"tboxKeyTableRomOffset";
			this->tboxKeyTableRomOffset->ReadOnly = true;
			this->tboxKeyTableRomOffset->Size = System::Drawing::Size(87, 19);
			this->tboxKeyTableRomOffset->TabIndex = 10;
			this->tboxKeyTableRomOffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// cboxIsNormalJump
			// 
			this->cboxIsNormalJump->AutoSize = true;
			this->cboxIsNormalJump->Enabled = false;
			this->cboxIsNormalJump->Location = System::Drawing::Point(14, 168);
			this->cboxIsNormalJump->Name = L"cboxIsNormalJump";
			this->cboxIsNormalJump->Size = System::Drawing::Size(121, 16);
			this->cboxIsNormalJump->TabIndex = 16;
			this->cboxIsNormalJump->Text = L"�m�[�}���W�����v����";
			this->cboxIsNormalJump->UseVisualStyleBackColor = true;
			// 
			// cboxIsTmpJump
			// 
			this->cboxIsTmpJump->AutoSize = true;
			this->cboxIsTmpJump->Enabled = false;
			this->cboxIsTmpJump->Location = System::Drawing::Point(14, 196);
			this->cboxIsTmpJump->Name = L"cboxIsTmpJump";
			this->cboxIsTmpJump->Size = System::Drawing::Size(103, 16);
			this->cboxIsTmpJump->TabIndex = 17;
			this->cboxIsTmpJump->Text = L"tmp�W�����v����";
			this->cboxIsTmpJump->UseVisualStyleBackColor = true;
			// 
			// labPublicSize
			// 
			this->labPublicSize->AutoSize = true;
			this->labPublicSize->Location = System::Drawing::Point(36, 79);
			this->labPublicSize->Name = L"labPublicSize";
			this->labPublicSize->Size = System::Drawing::Size(118, 12);
			this->labPublicSize->TabIndex = 14;
			this->labPublicSize->Text = L"Public Save Data Size";
			// 
			// tboxPublicSize
			// 
			this->tboxPublicSize->Location = System::Drawing::Point(177, 76);
			this->tboxPublicSize->Name = L"tboxPublicSize";
			this->tboxPublicSize->ReadOnly = true;
			this->tboxPublicSize->Size = System::Drawing::Size(87, 19);
			this->tboxPublicSize->TabIndex = 12;
			this->tboxPublicSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tabTWLSpec2
			// 
			this->tabTWLSpec2->Controls->Add(this->gboxAccess);
			this->tabTWLSpec2->Location = System::Drawing::Point(4, 21);
			this->tabTWLSpec2->Name = L"tabTWLSpec2";
			this->tabTWLSpec2->Size = System::Drawing::Size(391, 263);
			this->tabTWLSpec2->TabIndex = 9;
			this->tabTWLSpec2->Text = L"TWL�d�l2";
			this->tabTWLSpec2->UseVisualStyleBackColor = true;
			// 
			// gboxAccess
			// 
			this->gboxAccess->Controls->Add(this->labAccessOther);
			this->gboxAccess->Controls->Add(this->tboxAccessOther);
			this->gboxAccess->Controls->Add(this->tboxIsGameCardOn);
			this->gboxAccess->Controls->Add(this->labIsGameCardOn);
			this->gboxAccess->Controls->Add(this->cboxIsNAND);
			this->gboxAccess->Controls->Add(this->cboxIsSD);
			this->gboxAccess->Location = System::Drawing::Point(16, 17);
			this->gboxAccess->Name = L"gboxAccess";
			this->gboxAccess->Size = System::Drawing::Size(340, 220);
			this->gboxAccess->TabIndex = 0;
			this->gboxAccess->TabStop = false;
			this->gboxAccess->Text = L"�A�N�Z�X�R���g���[�����";
			// 
			// labAccessOther
			// 
			this->labAccessOther->AutoSize = true;
			this->labAccessOther->Location = System::Drawing::Point(24, 89);
			this->labAccessOther->Name = L"labAccessOther";
			this->labAccessOther->Size = System::Drawing::Size(36, 12);
			this->labAccessOther->TabIndex = 5;
			this->labAccessOther->Text = L"���̑�";
			// 
			// tboxAccessOther
			// 
			this->tboxAccessOther->Location = System::Drawing::Point(26, 104);
			this->tboxAccessOther->Multiline = true;
			this->tboxAccessOther->Name = L"tboxAccessOther";
			this->tboxAccessOther->ReadOnly = true;
			this->tboxAccessOther->Size = System::Drawing::Size(292, 104);
			this->tboxAccessOther->TabIndex = 4;
			// 
			// tboxIsGameCardOn
			// 
			this->tboxIsGameCardOn->Location = System::Drawing::Point(117, 57);
			this->tboxIsGameCardOn->Name = L"tboxIsGameCardOn";
			this->tboxIsGameCardOn->ReadOnly = true;
			this->tboxIsGameCardOn->Size = System::Drawing::Size(201, 19);
			this->tboxIsGameCardOn->TabIndex = 3;
			// 
			// labIsGameCardOn
			// 
			this->labIsGameCardOn->AutoSize = true;
			this->labIsGameCardOn->Location = System::Drawing::Point(24, 61);
			this->labIsGameCardOn->Name = L"labIsGameCardOn";
			this->labIsGameCardOn->Size = System::Drawing::Size(87, 12);
			this->labIsGameCardOn->TabIndex = 2;
			this->labIsGameCardOn->Text = L"�Q�[���J�[�h�d��";
			// 
			// cboxIsNAND
			// 
			this->cboxIsNAND->AutoSize = true;
			this->cboxIsNAND->Enabled = false;
			this->cboxIsNAND->Location = System::Drawing::Point(150, 30);
			this->cboxIsNAND->Name = L"cboxIsNAND";
			this->cboxIsNAND->Size = System::Drawing::Size(121, 16);
			this->cboxIsNAND->TabIndex = 1;
			this->cboxIsNAND->Text = L"NAND�t���b�V��������";
			this->cboxIsNAND->UseVisualStyleBackColor = true;
			// 
			// cboxIsSD
			// 
			this->cboxIsSD->AutoSize = true;
			this->cboxIsSD->Enabled = false;
			this->cboxIsSD->Location = System::Drawing::Point(26, 30);
			this->cboxIsSD->Name = L"cboxIsSD";
			this->cboxIsSD->Size = System::Drawing::Size(67, 16);
			this->cboxIsSD->TabIndex = 0;
			this->cboxIsSD->Text = L"SD�J�[�h";
			this->cboxIsSD->UseVisualStyleBackColor = true;
			// 
			// tabTWLSpec1
			// 
			this->tabTWLSpec1->Controls->Add(this->labCautionCheck);
			this->tabTWLSpec1->Controls->Add(this->gboxTitleID);
			this->tabTWLSpec1->Controls->Add(this->labCaptionEx);
			this->tabTWLSpec1->Controls->Add(this->tboxCaptionEx);
			this->tabTWLSpec1->Location = System::Drawing::Point(4, 21);
			this->tabTWLSpec1->Name = L"tabTWLSpec1";
			this->tabTWLSpec1->Size = System::Drawing::Size(391, 263);
			this->tabTWLSpec1->TabIndex = 8;
			this->tabTWLSpec1->Text = L"TWL�d�l1";
			this->tabTWLSpec1->UseVisualStyleBackColor = true;
			// 
			// labCautionCheck
			// 
			this->labCautionCheck->AutoSize = true;
			this->labCautionCheck->ForeColor = System::Drawing::SystemColors::ActiveCaption;
			this->labCautionCheck->Location = System::Drawing::Point(16, 8);
			this->labCautionCheck->Name = L"labCautionCheck";
			this->labCautionCheck->Size = System::Drawing::Size(336, 12);
			this->labCautionCheck->TabIndex = 35;
			this->labCautionCheck->Text = L"* �����̍��ڂ�ROM�f�[�^�̊m�F�p�ł��B���͂̕K�v�͂���܂���B";
			// 
			// gboxTitleID
			// 
			this->gboxTitleID->Controls->Add(this->labHex2);
			this->gboxTitleID->Controls->Add(this->labHex1);
			this->gboxTitleID->Controls->Add(this->tboxTitleIDLo);
			this->gboxTitleID->Controls->Add(this->labTitleIDLo);
			this->gboxTitleID->Controls->Add(this->labTitleIDHi);
			this->gboxTitleID->Controls->Add(this->tboxTitleIDHi);
			this->gboxTitleID->Controls->Add(this->tboxAppType);
			this->gboxTitleID->Controls->Add(this->labAppType);
			this->gboxTitleID->Location = System::Drawing::Point(17, 37);
			this->gboxTitleID->Name = L"gboxTitleID";
			this->gboxTitleID->Size = System::Drawing::Size(348, 151);
			this->gboxTitleID->TabIndex = 23;
			this->gboxTitleID->TabStop = false;
			this->gboxTitleID->Text = L"TitleID";
			// 
			// labHex2
			// 
			this->labHex2->AutoSize = true;
			this->labHex2->Location = System::Drawing::Point(209, 51);
			this->labHex2->Name = L"labHex2";
			this->labHex2->Size = System::Drawing::Size(11, 12);
			this->labHex2->TabIndex = 7;
			this->labHex2->Text = L"h";
			// 
			// labHex1
			// 
			this->labHex1->AutoSize = true;
			this->labHex1->Location = System::Drawing::Point(209, 23);
			this->labHex1->Name = L"labHex1";
			this->labHex1->Size = System::Drawing::Size(11, 12);
			this->labHex1->TabIndex = 6;
			this->labHex1->Text = L"h";
			// 
			// tboxTitleIDLo
			// 
			this->tboxTitleIDLo->Location = System::Drawing::Point(83, 20);
			this->tboxTitleIDLo->Name = L"tboxTitleIDLo";
			this->tboxTitleIDLo->ReadOnly = true;
			this->tboxTitleIDLo->Size = System::Drawing::Size(121, 19);
			this->tboxTitleIDLo->TabIndex = 1;
			this->tboxTitleIDLo->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// labTitleIDLo
			// 
			this->labTitleIDLo->AutoSize = true;
			this->labTitleIDLo->Location = System::Drawing::Point(11, 23);
			this->labTitleIDLo->Name = L"labTitleIDLo";
			this->labTitleIDLo->Size = System::Drawing::Size(63, 12);
			this->labTitleIDLo->TabIndex = 0;
			this->labTitleIDLo->Text = L"TitleID Low";
			// 
			// labTitleIDHi
			// 
			this->labTitleIDHi->AutoSize = true;
			this->labTitleIDHi->Location = System::Drawing::Point(11, 51);
			this->labTitleIDHi->Name = L"labTitleIDHi";
			this->labTitleIDHi->Size = System::Drawing::Size(66, 12);
			this->labTitleIDHi->TabIndex = 2;
			this->labTitleIDHi->Text = L"TitleID High";
			// 
			// tboxTitleIDHi
			// 
			this->tboxTitleIDHi->Location = System::Drawing::Point(83, 48);
			this->tboxTitleIDHi->Name = L"tboxTitleIDHi";
			this->tboxTitleIDHi->ReadOnly = true;
			this->tboxTitleIDHi->Size = System::Drawing::Size(121, 19);
			this->tboxTitleIDHi->TabIndex = 3;
			this->tboxTitleIDHi->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxAppType
			// 
			this->tboxAppType->Location = System::Drawing::Point(13, 96);
			this->tboxAppType->Multiline = true;
			this->tboxAppType->Name = L"tboxAppType";
			this->tboxAppType->ReadOnly = true;
			this->tboxAppType->Size = System::Drawing::Size(304, 47);
			this->tboxAppType->TabIndex = 4;
			// 
			// labAppType
			// 
			this->labAppType->AutoSize = true;
			this->labAppType->Location = System::Drawing::Point(12, 81);
			this->labAppType->Name = L"labAppType";
			this->labAppType->Size = System::Drawing::Size(91, 12);
			this->labAppType->TabIndex = 5;
			this->labAppType->Text = L"Application Type";
			// 
			// labCaptionEx
			// 
			this->labCaptionEx->AutoSize = true;
			this->labCaptionEx->Location = System::Drawing::Point(28, 195);
			this->labCaptionEx->Name = L"labCaptionEx";
			this->labCaptionEx->Size = System::Drawing::Size(53, 12);
			this->labCaptionEx->TabIndex = 11;
			this->labCaptionEx->Text = L"���L����";
			// 
			// tboxCaptionEx
			// 
			this->tboxCaptionEx->Location = System::Drawing::Point(30, 210);
			this->tboxCaptionEx->Multiline = true;
			this->tboxCaptionEx->Name = L"tboxCaptionEx";
			this->tboxCaptionEx->ReadOnly = true;
			this->tboxCaptionEx->Size = System::Drawing::Size(304, 45);
			this->tboxCaptionEx->TabIndex = 10;
			// 
			// tabCheck
			// 
			this->tabCheck->Controls->Add(this->tabTWLSpec1);
			this->tabCheck->Controls->Add(this->tabTWLSpec2);
			this->tabCheck->Controls->Add(this->tabTWLSpec3);
			this->tabCheck->Controls->Add(this->tabTWLSpec4);
			this->tabCheck->Controls->Add(this->tabSDK);
			this->tabCheck->Location = System::Drawing::Point(410, 331);
			this->tabCheck->Name = L"tabCheck";
			this->tabCheck->SelectedIndex = 0;
			this->tabCheck->Size = System::Drawing::Size(399, 288);
			this->tabCheck->TabIndex = 6;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(825, 701);
			this->Controls->Add(this->tabDoc);
			this->Controls->Add(this->gboxSelectLang);
			this->Controls->Add(this->gboxTWLInfoWritable);
			this->Controls->Add(this->butMakeMaster);
			this->Controls->Add(this->tabCheck);
			this->Controls->Add(this->gboxFileOpen);
			this->Controls->Add(this->gboxCRC);
			this->Controls->Add(this->gboxSrl);
			this->Controls->Add(this->tboxMsg);
			this->Name = L"Form1";
			this->Text = L"MasterEditor for TWL";
			this->gboxSrl->ResumeLayout(false);
			this->gboxSrl->PerformLayout();
			this->gboxCRC->ResumeLayout(false);
			this->gboxCRC->PerformLayout();
			this->gboxFileOpen->ResumeLayout(false);
			this->gboxFileOpen->PerformLayout();
			this->gboxSelectLang->ResumeLayout(false);
			this->gboxSelectLang->PerformLayout();
			this->gboxTWLInfoWritable->ResumeLayout(false);
			this->gboxTWLInfoWritable->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numEULA))->EndInit();
			this->gboxPerson2->ResumeLayout(false);
			this->gboxPerson2->PerformLayout();
			this->gboxPerson1->ResumeLayout(false);
			this->gboxPerson1->PerformLayout();
			this->gboxUsage->ResumeLayout(false);
			this->gboxUsage->PerformLayout();
			this->gboxSubmitWay->ResumeLayout(false);
			this->gboxSubmitWay->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numSubmitVersion))->EndInit();
			this->tabDoc->ResumeLayout(false);
			this->tabSubmitInfo->ResumeLayout(false);
			this->tabSubmitInfo->PerformLayout();
			this->tabForeignInfo->ResumeLayout(false);
			this->tabForeignInfo->PerformLayout();
			this->tabCompanyInfo1->ResumeLayout(false);
			this->tabCompanyInfo2->ResumeLayout(false);
			this->tabCompanyInfo2->PerformLayout();
			this->tabCaption->ResumeLayout(false);
			this->tabCaption->PerformLayout();
			this->tabTWLSpec4->ResumeLayout(false);
			this->gboxShared2Size->ResumeLayout(false);
			this->gboxShared2Size->PerformLayout();
			this->tabSDK->ResumeLayout(false);
			this->tabSDK->PerformLayout();
			this->tabTWLSpec3->ResumeLayout(false);
			this->gboxTWLExInfo->ResumeLayout(false);
			this->gboxTWLExInfo->PerformLayout();
			this->tabTWLSpec2->ResumeLayout(false);
			this->gboxAccess->ResumeLayout(false);
			this->gboxAccess->PerformLayout();
			this->tabTWLSpec1->ResumeLayout(false);
			this->tabTWLSpec1->PerformLayout();
			this->gboxTitleID->ResumeLayout(false);
			this->gboxTitleID->PerformLayout();
			this->tabCheck->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	/////////////////////////////////////////////
	// �������\�b�h
	/////////////////////////////////////////////
	private:
		// �ݒ�t�@�C���̓ǂݍ���
		void loadInit(void)
		{
			System::Xml::XmlDocument ^doc = gcnew System::Xml::XmlDocument();

			// xml�t�@�C���̓ǂݍ���
			try
			{
				doc->Load( "../resource/ini.xml" );
			}
			catch( System::IO::FileNotFoundException ^s )
			{
				(void)s;
				this->tboxMsg->Text = "�ݒ�t�@�C�����J�����Ƃ��ł��܂���ł����B";
				return;
			}

			// <init>�^�O : ���[�g
			System::Xml::XmlElement ^root = doc->DocumentElement;
			System::String ^msg = gcnew System::String("");

			// <rw>�^�O
			System::Xml::XmlNodeList ^rwlist = root->GetElementsByTagName( "rw" );
			if( rwlist != nullptr )
			{
				System::Xml::XmlNode     ^rw = rwlist->Item(0);
				if( rw->FirstChild->Value->Equals( "r" ) )
				{
					// ���[�h�I�������[�h
					this->disableForms();
					if( !System::String::IsNullOrEmpty(msg) )
					{
						msg += "�E";
					}
					msg += "���[�h�I�������[�h";
				}
			}

			// <output>�^�O
			System::Xml::XmlNodeList ^outlist = root->GetElementsByTagName( "output" );
			if( outlist != nullptr )
			{
				System::Xml::XmlNode     ^out = outlist->Item(0);
				if( out->FirstChild->Value->Equals( "XML" ) )
				{
					// �m�[�}��XML�o�̓��[�h
					this->disableForms();
					if( !System::String::IsNullOrEmpty(msg) )
					{
						msg += "�E";
					}
					msg += "XML�����o�̓��[�h";
				}
			}
			if( !System::String::IsNullOrEmpty(msg) )
			{
				msg += "�œ��삵�Ă��܂��B";
			}
			this->tboxMsg->Text = msg;
		}

		// �ݒ�\�ȃt�H�[�������ׂ� disable �ɂ���
		void disableForms( void )
		{
			this->cboxIsEULA->Enabled = false;
			this->numEULA->Enabled    = false;
			this->cboxIsWirelessIcon->Enabled = false;
			this->cboxIsWiFiIcon->Enabled     = false;

			this->combCERO->Enabled = false;
			this->cboxCERO->Enabled = false;
			this->cboxAlwaysCERO->Enabled = false;
			this->combESRB->Enabled = false;
			this->cboxESRB->Enabled = false;
			this->cboxAlwaysESRB->Enabled = false;
			this->combUSK->Enabled = false;
			this->cboxUSK->Enabled = false;
			this->cboxAlwaysUSK->Enabled = false;
			this->combPEGI->Enabled = false;
			this->cboxPEGI->Enabled = false;
			this->cboxAlwaysPEGI->Enabled = false;
			this->combPEGIPRT->Enabled = false;
			this->cboxPEGIPRT->Enabled = false;
			this->cboxAlwaysPEGIPRT->Enabled = false;
			this->combPEGIBBFC->Enabled = false;
			this->cboxPEGIBBFC->Enabled = false;
			this->cboxAlwaysPEGIBBFC->Enabled = false;
			this->combOFLC->Enabled = false;
			this->cboxOFLC->Enabled = false;
			this->cboxAlwaysOFLC->Enabled = false;
		}

	private:
		// ROM�����t�H�[������擾����SRL�N���X�̃v���p�e�B�ɔ��f������
		// (ROM�w�b�_�ւ̔��f��CRC�Ə����̍Čv�Z�����Ȃ�)
		void setSrlPropaties(void)
		{
			// ROM�w�b�_��[0,0x160)�̗̈��Read Only�ŕύX���Ȃ�

			// TWL�g���̈�̂������̏���ROM�w�b�_�ɔ��f������
			this->hSrl->hIsEULA         = this->cboxIsEULA->Checked;
			this->hSrl->hEULAVersion    = gcnew System::Byte( System::Decimal::ToByte( this->numEULA->Value ) );
			this->hSrl->hIsWiFiIcon     = this->cboxIsWiFiIcon->Checked;
			this->hSrl->hIsWirelessIcon = this->cboxIsWirelessIcon->Checked;

			// Srl�N���X�̃v���p�e�B�ւ̔��f
			this->setParentalSrlPropaties();
		}

		// SRL��ROM�����t�H�[���ɔ��f������(�t�@�C�����ǂݍ��܂�Ă��邱�Ƃ��O��)
		void setSrlForms(void)
		{
			// NTR�݊����
			this->tboxTitleName->Text = this->hSrl->hTitleName;
			this->tboxGameCode->Text  = this->hSrl->hGameCode;
			this->tboxMakerCode->Text = this->hSrl->hMakerCode;
			this->tboxPlatform->Text  = this->hSrl->hPlatform;
			this->tboxRomSize->Text   = this->hSrl->hRomSize;
			this->tboxRomLatency->Text = this->hSrl->hLatency;
			if( *(this->hSrl->hRomVersion) == 0xE0 )
			{
				this->tboxRemasterVer->Text = gcnew System::String("E");
				this->cboxRemasterVerE->Checked = true;
			}
			else
			{
				this->tboxRemasterVer->Text = this->hSrl->hRomVersion->ToString();
				this->cboxRemasterVerE->Checked = false;
			}

			this->tboxHeaderCRC->Clear();
			this->tboxHeaderCRC->AppendText( "0x" );
			this->tboxHeaderCRC->AppendText( this->hSrl->hHeaderCRC->ToString("X") );

			if( this->hSrl->hPlatform == nullptr )
			{
				if( this->rSelectJ->Checked == true )
					this->errMsg( "�v���b�g�z�[���w�肪�s���ł��BROM�f�[�^�̃r���h�ݒ���������Ă��������B" );
				else
					this->errMsg( "Illegal Platform: Please check build settings of the ROM data." );
			}

			// TWL�g�����
			this->tboxTitleIDLo->Text = this->hSrl->hTitleIDLo->ToString("X8");
			this->tboxTitleIDHi->Text = this->hSrl->hTitleIDHi->ToString("X8");
			this->tboxNormalRomOffset->Text   = this->hSrl->hNormalRomOffset->ToString("X8");
			this->tboxKeyTableRomOffset->Text = this->hSrl->hKeyTableRomOffset->ToString("X8");
			this->tboxPublicSize->Text  = this->hSrl->hPublicSize->ToString();
			this->tboxPrivateSize->Text = this->hSrl->hPrivateSize->ToString();
			this->cboxIsNormalJump->Checked = *(this->hSrl->hIsNormalJump);
			this->cboxIsTmpJump->Checked    = *(this->hSrl->hIsTmpJump);
			this->cboxIsSubBanner->Checked  = *(this->hSrl->hIsSubBanner);
			this->cboxIsWL->Checked         = *(this->hSrl->hIsWL);
			this->cboxIsEULA->Checked       = *(this->hSrl->hIsEULA);
			this->numEULA->Value            = *(this->hSrl->hEULAVersion);
			this->cboxIsWiFiIcon->Checked   = *(this->hSrl->hIsWiFiIcon);
			this->cboxIsWirelessIcon->Checked = *(this->hSrl->hIsWirelessIcon);
			if( *(this->hSrl->hIsCodecTWL) == true )
			{
				this->tboxIsCodec->Text = gcnew System::String( "TWL" );
			}
			else
			{
				this->tboxIsCodec->Text = gcnew System::String( "NTR" );
			}
			//this->cboxIsRegionJapan->Checked     = *(this->hSrl->hIsRegionJapan);
			//this->cboxIsRegionAmerica->Checked   = *(this->hSrl->hIsRegionAmerica);
			//this->cboxIsRegionEurope->Checked    = *(this->hSrl->hIsRegionEurope);
			//this->cboxIsRegionAustralia->Checked = *(this->hSrl->hIsRegionAustralia);
			//this->cboxIsRegionChina->Checked     = *(this->hSrl->hIsRegionChina);
			//this->cboxIsRegionKorea->Checked     = *(this->hSrl->hIsRegionKorea);
			this->cboxIsSD->Checked   = *(this->hSrl->hIsSD);
			this->cboxIsNAND->Checked = *(this->hSrl->hIsNAND);
			if( *(this->hSrl->hIsGameCardNitro) == true )
			{
				this->tboxIsGameCardOn->Text = gcnew System::String( "ON(NTR)" );
			}
			else if( *(this->hSrl->hIsGameCardOn) == true )
			{
				this->tboxIsGameCardOn->Text = gcnew System::String( "ON(normal)" );
			}
			else
			{
				this->tboxIsGameCardOn->Text = gcnew System::String( "OFF" );
			}
			this->cboxIsShared2->Checked = *(this->hSrl->hIsShared2);
			this->tboxShared2Size0->Text = this->hSrl->hShared2Size0->ToString();
			this->tboxShared2Size1->Text = this->hSrl->hShared2Size1->ToString();
			this->tboxShared2Size2->Text = this->hSrl->hShared2Size2->ToString();
			this->tboxShared2Size3->Text = this->hSrl->hShared2Size3->ToString();
			this->tboxShared2Size4->Text = this->hSrl->hShared2Size4->ToString();
			this->tboxShared2Size5->Text = this->hSrl->hShared2Size5->ToString();

			// �A�v�����
			System::String ^app = gcnew System::String("");
			if( *(this->hSrl->hIsAppUser) == true )
			{
				app += "Type:User. ";
			}
			if( *(this->hSrl->hIsAppSystem) == true )
			{
				app += "Type:System. ";
			}
			if( *(this->hSrl->hIsAppLauncher) == true )
			{
				app += "Type:Launcher. ";
			}
			if( *(this->hSrl->hIsAppSecure) == true )
			{
				app += "Type:Secure. ";
			}
			if( *(this->hSrl->hIsMediaNand) == true )
			{
				app += "Media:NAND. ";
			}
			else
			{
				app += "Media:Card. ";
			}
			if( *(this->hSrl->hIsLaunch) == true )
			{
				app += "Launch. ";
			}
			else
			{
				app += "Not-Launch. ";
			}
			if( *(this->hSrl->hIsDataOnly) == true )
			{
				app += "DataOnly. ";
			}
			this->tboxAppType->Text = app;

			// �A�N�Z�X�R���g���[�� ���̑�
			System::String ^acc = gcnew System::String("");
			if( *(this->hSrl->hIsCommonClientKey) == true )
			{
				acc += "commonClientKey. ";
			}
			if( *(this->hSrl->hIsAesSlotBForES) == true )
			{
				acc += "AES-SlotB(ES). ";
			}
			if( *(this->hSrl->hIsAesSlotCForNAM) == true )
			{
				acc += "AES-SlotC(NAM). ";
			}
			if( *(this->hSrl->hIsAesSlotBForJpegEnc) == true )
			{
				acc += "AES-SlotB(JpegEnc.). ";
			}
			if( *(this->hSrl->hIsAesSlotAForSSL) == true )
			{
				acc += "AES-SlotA(SSL). ";
			}
			if( *(this->hSrl->hIsCommonClientKeyForDebugger) == true )
			{
				acc += "commonClientKey(Debug.). ";
			}
			this->tboxAccessOther->Text = acc;

			// ����Ȑݒ���e�L�X�g�{�b�N�X�ɔ��f
			this->setSrlFormsCaptionEx();

			// �y�A�����^���R���g���[���֘A
			this->setParentalForms();
		}

		// �t�H�[���̓��͂��`�F�b�N����
		System::Boolean checkSrlForms(void)
		{
			// �s���ȏꍇ�̓_�C�A���O�Œ��ӂ���return
			if( this->checkTextForm( this->tboxTitleName->Text, this->labTitleName->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxGameCode->Text, this->labGameCode->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxMakerCode->Text, this->labMakerCode->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxPlatform->Text, this->labPlatform->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxRomSize->Text, this->labPlatform->Text ) == false )
				return false;

			// ���f����O�Ƀt�H�[�������������ǂ����`�F�b�N
			if( this->checkParentalForms( this->combCERO, this->cboxCERO, this->labCERO->Text ) == false )
				return false;
			if( this->checkParentalForms( this->combESRB, this->cboxESRB, this->labESRB->Text ) == false)
				return false;
			if( this->checkParentalForms( this->combUSK, this->cboxUSK, this->labUSK->Text ) == false )
				return false;
			if( this->checkParentalForms( this->combPEGI, this->cboxPEGI, this->labPEGI->Text ) == false )
				return false;
			if( this->checkParentalForms( this->combPEGIPRT, this->cboxPEGIPRT, this->labPEGIPRT->Text ) == false )
				return false;
			if( this->checkParentalForms( this->combPEGIBBFC, this->cboxPEGIBBFC, this->labPEGIBBFC->Text ) == false )
				return false;
			if( this->checkParentalForms( this->combOFLC, this->cboxOFLC, this->labOFLC->Text ) == false )
				return false;

			return true;
		}

		// SRL�̓���Ȑݒ���t�H�[���ɃZ�b�g����(����؂�ւ��ŕ\����ς������̂œƗ�������)
		void setSrlFormsCaptionEx()
		{
			// ����Ȑݒ�͔��l���ɏ��������Ă���
			this->tboxCaptionEx->Clear();
			if( (this->hSrl->hHasDSDLPlaySign != nullptr) && (*(this->hSrl->hHasDSDLPlaySign) == true) )
			{
				if( this->rSelectJ->Checked == true )
					this->tboxCaptionEx->Text += gcnew System::String( "DS�N���[���u�[�g�Ή�. " );
				else
					this->tboxCaptionEx->Text += gcnew System::String( "DS Clone Boot. " );
			}
		}

		// �y�A�����^���R���g���[������SRL���ɂ��邪�ݒ肪��ςȂ̂Ő؂�o��

		// �y�A�����^���R���g���[���֘A�̏����t�H�[������擾����SRL�ɔ��f������
		void setParentalSrlPropaties(void)
		{
			System::Byte rating;
			// CERO
			switch( this->combCERO->SelectedIndex )
			{
				case 0: rating =  0; break;
				case 1: rating = 12; break;
				case 2: rating = 15; break;
				case 3: rating = 17; break;
				case 4: rating = 18; break;
				default:
					rating = 0;		// ���R��
				break;
			}
			this->hSrl->hArrayParentalRating[OS_TWL_PCTL_OGN_CERO] = gcnew System::Byte( rating );
			this->hSrl->hArrayParentalEffect[OS_TWL_PCTL_OGN_CERO] = gcnew System::Boolean( this->cboxCERO->Checked );
			this->hSrl->hArrayParentalAlways[OS_TWL_PCTL_OGN_CERO] = gcnew System::Boolean( this->cboxAlwaysCERO->Checked );

			// ESRB
			switch( this->combESRB->SelectedIndex )
			{
				case 0: rating =  3; break;
				case 1: rating =  6; break;
				case 2: rating = 10; break;
				case 3: rating = 13; break;
				case 4: rating = 17; break;
				default:
					rating = 0;
				break;
			}
			this->hSrl->hArrayParentalRating[OS_TWL_PCTL_OGN_ESRB] = gcnew System::Byte( rating );
			this->hSrl->hArrayParentalEffect[OS_TWL_PCTL_OGN_ESRB] = gcnew System::Boolean( this->cboxESRB->Checked );
			this->hSrl->hArrayParentalAlways[OS_TWL_PCTL_OGN_ESRB] = gcnew System::Boolean( this->cboxAlwaysESRB->Checked );

			// USK
			switch( this->combUSK->SelectedIndex )
			{
				case 0: rating =  0; break;
				case 1: rating =  6; break;
				case 2: rating = 12; break;
				case 3: rating = 16; break;
				case 4: rating = 18; break;
				default:
					rating = 0;
				break;
			}
			this->hSrl->hArrayParentalRating[OS_TWL_PCTL_OGN_USK] = gcnew System::Byte( rating );
			this->hSrl->hArrayParentalEffect[OS_TWL_PCTL_OGN_USK] = gcnew System::Boolean( this->cboxUSK->Checked );
			this->hSrl->hArrayParentalAlways[OS_TWL_PCTL_OGN_USK] = gcnew System::Boolean( this->cboxAlwaysUSK->Checked );

			// PEGI
			switch( this->combPEGI->SelectedIndex )
			{
				case 0: rating =  3; break;
				case 1: rating =  7; break;
				case 2: rating = 12; break;
				case 3: rating = 16; break;
				case 4: rating = 18; break;
				default:
					rating = 0;
				break;
			}
			this->hSrl->hArrayParentalRating[OS_TWL_PCTL_OGN_PEGI_GEN] = gcnew System::Byte( rating );
			this->hSrl->hArrayParentalEffect[OS_TWL_PCTL_OGN_PEGI_GEN] = gcnew System::Boolean( this->cboxPEGI->Checked );
			this->hSrl->hArrayParentalAlways[OS_TWL_PCTL_OGN_PEGI_GEN] = gcnew System::Boolean( this->cboxAlwaysPEGI->Checked );

			// PEGIPRT
			switch( this->combPEGIPRT->SelectedIndex )
			{
				case 0: rating =  4; break;
				case 1: rating =  6; break;
				case 2: rating = 12; break;
				case 3: rating = 16; break;
				case 4: rating = 18; break;
				default:
					rating = 0;
				break;
			}
			this->hSrl->hArrayParentalRating[OS_TWL_PCTL_OGN_PEGI_PRT] = gcnew System::Byte( rating );
			this->hSrl->hArrayParentalEffect[OS_TWL_PCTL_OGN_PEGI_PRT] = gcnew System::Boolean( this->cboxPEGIPRT->Checked );
			this->hSrl->hArrayParentalAlways[OS_TWL_PCTL_OGN_PEGI_PRT] = gcnew System::Boolean( this->cboxAlwaysPEGIPRT->Checked );

			// PEGIBBFC
			switch( this->combPEGIBBFC->SelectedIndex )
			{
				case 0: rating =  3; break;
				case 1: rating =  4; break;
				case 2: rating =  7; break;
				case 3: rating =  8; break;
				case 4: rating = 12; break;
				case 5: rating = 15; break;
				case 6: rating = 16; break;
				case 7: rating = 18; break;
				default:
					rating = 0;
				break;
			}
			this->hSrl->hArrayParentalRating[OS_TWL_PCTL_OGN_PEGI_BBFC] = gcnew System::Byte( rating );
			this->hSrl->hArrayParentalEffect[OS_TWL_PCTL_OGN_PEGI_BBFC] = gcnew System::Boolean( this->cboxPEGIBBFC->Checked );
			this->hSrl->hArrayParentalAlways[OS_TWL_PCTL_OGN_PEGI_BBFC] = gcnew System::Boolean( this->cboxAlwaysPEGIBBFC->Checked );

			// OFLC
			switch( this->combOFLC->SelectedIndex )
			{
				case 0: rating =  0; break;
				case 1: rating =  7; break;
				case 2: rating = 14; break;
				case 3: rating = 15; break;
				default:
					rating = 0;
				break;
			}
			this->hSrl->hArrayParentalRating[OS_TWL_PCTL_OGN_OFLC] = gcnew System::Byte( rating );
			this->hSrl->hArrayParentalEffect[OS_TWL_PCTL_OGN_OFLC] = gcnew System::Boolean( this->cboxOFLC->Checked );
			this->hSrl->hArrayParentalAlways[OS_TWL_PCTL_OGN_OFLC] = gcnew System::Boolean( this->cboxAlwaysOFLC->Checked );
		}

		// �y�A�����^���R���g���[���֘A�̃t�H�[�����͂����������������ݑO�`�F�b�N
		System::Boolean checkParentalForms( System::Windows::Forms::ComboBox ^comb, System::Windows::Forms::CheckBox ^enable, System::String ^msg )
		{
			// ���[�e�B���O���ݒ肳��Ă���̂ɗL���t���O�������Ă��Ȃ�
			if( (comb->SelectedIndex != (comb->Items->Count - 1)) && !(enable->Checked) )
			{
				this->parentalMsg( 1, msg );
				return false;
			}
			// ���R���Ȃ̂ɗL���t���O�������Ă���
			if( (comb->SelectedIndex == (comb->Items->Count - 1)) && enable->Checked )
			{
				this->parentalMsg( 2, msg );
				return false;
			}
			return true;
		}

		// SRL���̃y�A�����^���R���g���[�����𔲂��o���ăt�H�[���ɔ��f������
		void setParentalForms(void)
		{
			System::Int32  index;

			// CERO
			switch( *(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_CERO ]) )
			{
				case 0:
					if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_CERO ]) )	// �S�N����R��������
						index = 0;
					else
						index = 5;
				break;
				case 12: index = 1; break;
				case 15: index = 2; break;
				case 17: index = 3; break;
				case 18: index = 4; break;
				default:
					index = 5;
					this->parentalMsg( 0, this->labCERO->Text );
				break;
			}
			this->combCERO->SelectedIndex = index;
			this->cboxCERO->Checked       = *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_CERO ]);
			this->cboxAlwaysCERO->Checked = *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_CERO ]);
			// ESRB
			switch( *(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_ESRB ]) )
			{
				case 3:  index = 0; break;
				case 6:  index = 1; break;
				case 10: index = 2; break;
				case 13: index = 3; break;
				case 17: index = 4; break;
				case 0:  index = 5; break; // 0�̓f�t�H���g�l�Ȃ̂ŃG���[���b�Z�[�W���o���Ȃ�(�����I�ɖ��R������)
				default:
					index = 5;
					this->parentalMsg( 0, this->labESRB->Text );
				break;
			}
			this->combESRB->SelectedIndex = index;
			this->cboxESRB->Checked       = *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_ESRB ]);
			this->cboxAlwaysESRB->Checked = *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_ESRB ]);
			// USK
			switch( *(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_USK ]) )
			{
				case 0:
					if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_USK ]) )
						index = 0;
					else
						index = 5;
				break;
				case 6:  index = 1; break;
				case 12: index = 2; break;
				case 16: index = 3; break;
				case 18: index = 4; break;
				default:
					index = 5;
					this->parentalMsg( 0, this->labUSK->Text );
				break;
			}
			this->combUSK->SelectedIndex = index;
			this->cboxUSK->Checked       = *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_USK ]);
			this->cboxAlwaysUSK->Checked = *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_USK ]);
			// PEGI_GEN
			switch( *(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_GEN ]) )
			{
				case 3:  index = 0; break;
				case 7:  index = 1; break;
				case 12: index = 2; break;
				case 16: index = 3; break;
				case 18: index = 4; break;
				case 0:  index = 5; break;
				default:
					index = 5;
					this->parentalMsg( 0, this->labPEGI->Text );
				break;
			}
			this->combPEGI->SelectedIndex = index;
			this->cboxPEGI->Checked       = *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_GEN ]);
			this->cboxAlwaysPEGI->Checked = *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_PEGI_GEN ]);
			// PEGI_PRT
			switch( *(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_PRT ]) )
			{
				case 4:  index = 0; break;
				case 6:  index = 1; break;
				case 12: index = 2; break;
				case 16: index = 3; break;
				case 18: index = 4; break;
				case 0:  index = 5; break;
				default:
					index = 5;
					this->parentalMsg( 0, this->labPEGIPRT->Text );
				break;
			}
			this->combPEGIPRT->SelectedIndex = index;
			this->cboxPEGIPRT->Checked       = *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_PRT ]);
			this->cboxAlwaysPEGIPRT->Checked = *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_PEGI_PRT ]);
			// PEGI_BBFC
			switch( *(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_BBFC ]) )
			{
				case 3:  index = 0; break;
				case 4:  index = 1; break;
				case 7:  index = 2; break;
				case 8:  index = 3; break;
				case 12: index = 4; break;
				case 15: index = 5; break;
				case 16: index = 6; break;
				case 18: index = 7; break;
				case 0:  index = 8; break;
				default:
					index = 8;
					this->parentalMsg( 0, this->labPEGIBBFC->Text + this->labPEGIBBFC2->Text );
				break;
			}
			this->combPEGIBBFC->SelectedIndex = index;
			this->cboxPEGIBBFC->Checked       = *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_BBFC ]);
			this->cboxAlwaysPEGIBBFC->Checked = *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_PEGI_BBFC ]);
			// OFLC
			switch( *(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_OFLC ]) )
			{
				case 0:
					if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_OFLC ]) )
						index = 0;
					else
						index = 4;
				break;
				case 7:  index = 1; break;
				case 14: index = 2; break;
				case 15: index = 3; break;
				default:
					index = 4; break;
					this->parentalMsg( 0, this->labOFLC->Text );
				break;
			}
			this->combOFLC->SelectedIndex = index;
			this->cboxOFLC->Checked       = *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_OFLC ]);
			this->cboxAlwaysOFLC->Checked = *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_OFLC ]);
		}

		// �y�A�����^���R���g���[���֘A����p�̃_�C�A���O���b�Z�[�W
		void parentalMsg( const System::Int32 type, const System::String ^msg )
		{
			System::String ^prefix;
			System::String ^suffix;

			if( this->rSelectJ->Checked == true )
			{
				prefix = gcnew System::String( "�y�A�����^���R���g���[������ " );
			}
			else
			{
				prefix = gcnew System::String( "Parental Control Info. " );
			}

			// 0: �ǂݍ��ݎ��`�F�b�N
			// 1: �������ݎ��`�F�b�N(���[�e�B���O���ݒ肳��Ă���̂Ƀt���O���L���ɂȂ��Ă��Ȃ�)
			// 2: �������ݎ��`�F�b�N(���[�e�B���O�����R���Ȃ̂Ƀt���O���L���ɂȂ��Ă���)
			if( type == 0 )
			{
				if( this->rSelectJ->Checked == true )
					suffix = gcnew System::String( " �̃��[�e�B���O�l���s���ł��B�ЂƂ܂��u���R���v�ɂ��܂��̂ŁA�������l��ݒ肵�Ă��������B" );
				else
					suffix = gcnew System::String( " : Illegal Rating. The value is set to \"Unexamined\" temporarily. Please set the value." );
			}
			else if( type == 1 )
			{
				if( this->rSelectJ->Checked == true )
					suffix = gcnew System::String( " �̃��[�e�B���O�l���ݒ肳��Ă���̂ɗL���t���O�������Ă��܂���B�ݒ���m�F���Ă��������B" );
				else
					suffix = gcnew System::String( " : The rating control flag is NOT enabled, but rating is set. Please retry settings." );
			}
			else if( type == 2 )
			{
				if( this->rSelectJ->Checked == true )
					suffix = gcnew System::String( " �̃��[�e�B���O�����R���Ȃ̂ɗL���t���O�������Ă��܂��B�ݒ���m�F���Ă��������B" );
				else
					suffix = gcnew System::String( " : The rating control flag is enabled, but rating is NOT set. Please retry settings." );
			}
			this->errMsg( prefix + msg + suffix );
		}

		// �}�X�^���ޏ����t�H�[������擾���ď��ނɔ��f������
		void setDeliverablePropaties(void)
		{
			// ��o���
			this->hDeliv->hProductName    = this->tboxProductName->Text;
			this->hDeliv->hProductCode1   = this->tboxProductCode1->Text;
			this->hDeliv->hProductCode2   = this->tboxProductCode2->Text;
			this->hDeliv->hReleaseForeign = gcnew System::Boolean( this->cboxReleaseForeign->Checked );
			if( this->cboxReleaseForeign->Checked == true )
			{
				this->hDeliv->hProductNameForeign  = this->tboxProductNameForeign->Text;
				this->hDeliv->hProductCode1Foreign = this->tboxProductCode1Foreign->Text;
				this->hDeliv->hProductCode2Foreign = gcnew System::String("");
				this->hDeliv->hProductCode2Foreign = this->tboxProductCode2Foreign1->Text;
				if( !System::String::IsNullOrEmpty( this->tboxProductCode2Foreign2->Text ) )
				{
					this->hDeliv->hProductCode2Foreign += ("/" + this->tboxProductCode2Foreign2->Text);
				}
				if( !System::String::IsNullOrEmpty( this->tboxProductCode2Foreign2->Text ) )
				{
					this->hDeliv->hProductCode2Foreign += ("/" + this->tboxProductCode2Foreign2->Text);
				}
			}
			else
			{
				this->hDeliv->hProductNameForeign  = nullptr;
				this->hDeliv->hProductCode1Foreign = nullptr;
				this->hDeliv->hProductCode2Foreign = nullptr;
			}
			this->hDeliv->hReleaseYear   = gcnew System::Int32( this->dateRelease->Value.Year  );
			this->hDeliv->hReleaseMonth  = gcnew System::Int32( this->dateRelease->Value.Month );
			this->hDeliv->hReleaseDay    = gcnew System::Int32( this->dateRelease->Value.Day   );
			this->hDeliv->hSubmitYear    = gcnew System::Int32( this->dateSubmit->Value.Year   );
			this->hDeliv->hSubmitMonth   = gcnew System::Int32( this->dateSubmit->Value.Month  );
			this->hDeliv->hSubmitDay     = gcnew System::Int32( this->dateSubmit->Value.Day    );
			this->hDeliv->hSubmitVersion = gcnew System::Int32( System::Decimal::ToInt32( this->numSubmitVersion->Value ) );
			this->hDeliv->hSDK           = this->tboxSDK->Text;
			// ��o���@
			if( this->rSubmitPost->Checked == true )
			{
				this->hDeliv->hSubmitWay = this->rSubmitPost->Text;
			}
			else
			{
				this->hDeliv->hSubmitWay = this->rSubmitHand->Text;
			}
			// �p�r
			if( this->rUsageSale->Checked == true )
			{
				this->hDeliv->hUsage = this->rUsageSale->Text;
				this->hDeliv->hUsageOther = nullptr;
			}
			else if( this->rUsageSample->Checked == true )
			{
				this->hDeliv->hUsage = this->rUsageSample->Text;
				this->hDeliv->hUsageOther = nullptr;
			}
			else if( this->rUsageDst->Checked == true )
			{
				this->hDeliv->hUsage = this->rUsageDst->Text;
				this->hDeliv->hUsageOther = nullptr;
			}
			else if( this->rUsageOther->Checked == true )
			{
				this->hDeliv->hUsage = this->rUsageOther->Text;
				this->hDeliv->hUsageOther = this->tboxUsageOther->Text;
			}

			// ��Џ��
			this->hDeliv->hCompany1    = this->tboxCompany1->Text;
			this->hDeliv->hDepart1     = this->tboxDepart1->Text;
			this->hDeliv->hPerson1     = this->tboxPerson1->Text;
			if( this->rSelectJ->Checked == true )
			{
				this->hDeliv->hFurigana1 = this->tboxFurigana1->Text;
			}
			else
			{
				this->hDeliv->hFurigana1 = nullptr;
			}
			this->hDeliv->hTel1        = this->tboxTel1->Text;
			this->hDeliv->hFax1        = this->tboxFax1->Text;
			this->hDeliv->hMail1       = this->tboxMail1->Text;
			this->hDeliv->hNTSC1       = this->tboxNTSC1->Text;
			this->hDeliv->hIsPerson2   = gcnew System::Boolean( this->cboxIsInputPerson2->Checked );
			if( this->cboxIsInputPerson2->Checked == true )
			{
				this->hDeliv->hCompany2    = this->tboxCompany2->Text;
				this->hDeliv->hDepart2	   = this->tboxDepart2->Text;
				this->hDeliv->hPerson2     = this->tboxPerson2->Text;
				if( this->rSelectJ->Checked == true )
				{
					this->hDeliv->hFurigana2 = this->tboxFurigana2->Text;
				}
				else
				{
					this->hDeliv->hFurigana2 = nullptr;
				}
				this->hDeliv->hTel2        = this->tboxTel2->Text;
				this->hDeliv->hFax2        = this->tboxFax2->Text;
				this->hDeliv->hMail2       = this->tboxMail2->Text;
				this->hDeliv->hNTSC2       = this->tboxNTSC2->Text;
			}
			else
			{
				this->hDeliv->hCompany2    = nullptr;
				this->hDeliv->hDepart2	   = nullptr;
				this->hDeliv->hPerson2     = nullptr;
				this->hDeliv->hFurigana2   = nullptr;
				this->hDeliv->hTel2        = nullptr;
				this->hDeliv->hFax2        = nullptr;
				this->hDeliv->hMail2       = nullptr;
				this->hDeliv->hNTSC2       = nullptr;
			}

			// ���l
			System::String ^tmp = this->tboxCaption->Text->Replace( " ", "" );
			if( this->tboxCaption->Text->Equals("") || tmp->Equals("") )	// �X�y�[�X�݂̂̕�����͊܂߂Ȃ�
			{
				this->hDeliv->hCaption = nullptr;
			}
			else
			{
				this->hDeliv->hCaption = this->tboxCaption->Text;
			}
			// ���l���ɓ���Ȑݒ��ǋL
			tmp = this->tboxCaptionEx->Text->Replace( " ", "" );
			if( !(this->tboxCaptionEx->Text->Equals("")) && !(tmp->Equals("")) )
			{
				this->hDeliv->hCaption += (" " + this->tboxCaptionEx->Text);
			}

			// �ꕔ��ROM����o�^
			if( this->combBackup->SelectedIndex != (this->combBackup->Items->Count - 1) )
			{
				this->hDeliv->hBackupMemory = this->combBackup->SelectedItem->ToString();
			}
			else
			{
				this->hDeliv->hBackupMemory = this->tboxBackupOther->Text;
			}
		}
		void setDeliverableForms(void)
		{
		}

		// �t�H�[���̓��͂��`�F�b�N����
		System::Boolean checkDeliverableForms(void)
		{
			// �s���ȏꍇ�̓_�C�A���O�Œ��ӂ���return

			// ��o���
			if( this->checkTextForm( this->tboxProductName->Text, this->labProductName->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxProductCode1->Text, this->labProductCode->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxProductCode2->Text, this->labProductCode->Text ) == false )
				return false;
			if( this->cboxReleaseForeign->Checked == true )
			{
				if( this->checkTextForm( this->tboxProductNameForeign->Text, this->labProductNameForeign->Text ) == false )
					return false;
				if( this->checkTextForm( this->tboxProductCode1Foreign->Text, this->labProductCodeForeign->Text ) == false )
					return false;
				if( this->checkTextForm( this->tboxProductCode2Foreign1->Text, this->labProductCodeForeign->Text ) == false )
					return false;
			}
			if( this->rUsageOther->Checked == true )
			{
				if( this->checkTextForm( this->tboxUsageOther->Text, 
					this->gboxUsage->Text + "(" + this->rUsageOther->Text + ")" ) == false )
					return false;
			}
			if( this->checkTextForm( this->tboxSDK->Text, this->labSDK->Text ) == false )
				return false;

			// ��Џ��
			if( this->checkTextForm( this->tboxPerson1->Text, this->labPerson1->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxCompany1->Text, this->labCompany1->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxDepart1->Text, this->labDepart1->Text ) == false )
				return false;
			if( this->rSelectJ->Checked == true )
			{
				if( this->checkTextForm( this->tboxFurigana1->Text, this->labFurigana1->Text ) == false )
					return false;
			}
			if( this->checkTextForm( this->tboxTel1->Text, this->labTel1->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxFax1->Text, this->labFax1->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxMail1->Text, this->labMail1->Text ) == false )
				return false;
			if( this->checkTextForm( this->tboxNTSC1->Text, this->labNTSC1Pre->Text + " " + this->labNTSC1Sur->Text ) == false )
				return false;

			if( this->cboxIsInputPerson2->Checked == true )
			{
				if( this->checkTextForm( this->tboxPerson2->Text, this->labPerson2->Text ) == false )
					return false;
				if( this->checkTextForm( this->tboxCompany2->Text, this->labCompany2->Text ) == false )
					return false;
				if( this->checkTextForm( this->tboxDepart2->Text, this->labDepart2->Text ) == false )
					return false;
				if( this->rSelectJ->Checked == true )
				{
					if( this->checkTextForm( this->tboxFurigana2->Text, this->labFurigana2->Text ) == false )
						return false;
				}
				if( this->checkTextForm( this->tboxTel2->Text, this->labTel2->Text ) == false )
					return false;
				if( this->checkTextForm( this->tboxFax2->Text, this->labFax2->Text ) == false )
					return false;
				if( this->checkTextForm( this->tboxMail2->Text, this->labMail2->Text ) == false )
					return false;
				if( this->checkTextForm( this->tboxNTSC2->Text, this->labNTSC2Pre->Text + " " + this->labNTSC2Sur->Text ) == false )
					return false;
			}

			// �ꕔ��ROM���(SRL�o�C�i���ɔ��f����Ȃ����)�������Ń`�F�b�N����
			if( this->checkBoxIndex( this->combBackup, this->labBackup->Text ) == false )
				return false;
			if( this->combBackup->SelectedIndex == (this->combBackup->Items->Count - 1) )
			{
				if( this->checkTextForm( this->tboxBackupOther->Text, this->labBackup->Text ) == false )
					return false;
			}

			return true;
		}

		// �e�L�X�g���͂�����Ă��邩�`�F�b�N
		System::Boolean checkTextForm( System::String ^formtext, System::String ^label )
		{
			System::String ^msg;
			
			if( this->rSelectJ->Checked == true )
				msg = gcnew System::String( "�����͂���Ă��܂���B��蒼���Ă��������B" );
			else
				msg = gcnew System::String( " is not set. Please retry setting." );

			if( formtext == nullptr )
			{
				this->errMsg( label + msg );
				return false;
			}
			System::String ^tmp = formtext->Replace( " ", "" );		// �X�y�[�X�݂̂̕�������G���[
			if( formtext->Equals("") || tmp->Equals("") )
			{
				this->errMsg( label + msg );
				return false;
			}
			return true;
		}

		// ���l���͂����킩�ǂ����`�F�b�N
		System::Boolean checkNumRange( System::Int32 val, System::Int32 min, System::Int32 max, System::String ^label )
		{
			System::String ^msg;
			
			if( this->rSelectJ->Checked == true )
				msg = gcnew System::String( "�̒l�͈̔͂��s���ł��B��蒼���Ă��������B" );
			else
				msg = gcnew System::String( ": Invalidate range of value. Please retry." );

			if( (val < min) || (max < val) )
			{
				this->errMsg( label + msg );
				return false;
			}
			return true;
		}
		System::Boolean checkNumRange( System::String ^strval, System::Int32 min, System::Int32 max, System::String ^label )
		{
			try
			{
				System::Int32  i = System::Int32::Parse(strval);
				return (this->checkNumRange( i, min, max, label ));
			}
			catch ( System::FormatException ^ex )
			{
				(void)ex;
				return (this->checkNumRange( max+1, min, max, label ));		// �K����O�𔭐�������
			}
		}

		// �R���{�{�b�N�X���`�F�b�N
		System::Boolean checkBoxIndex( System::Windows::Forms::ComboBox ^box, System::String ^label )
		{
			System::String ^msg;
			
			if( this->rSelectJ->Checked == true )
			{
				msg = gcnew System::String( "���I������Ă��܂���B��蒼���Ă��������B" );
			}
			else
			{
				msg = gcnew System::String( " is not selected. Please retry." );
			}

			if( box->SelectedIndex < 0 )
			{
				this->errMsg( label + msg );
				return false;
			}
			return true;
		}

		// �G���[���b�Z�[�W���o��
		void errMsg( System::String ^str )
		{
			MessageBox::Show( str, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error );
		}

	// ����؂�ւ�
	private:
		void changeJapanese(void)
		{
			System::Int32 index;

			// �t�����g�p�l���㕔
			this->gboxSrl->Text       = gcnew System::String( "ROM�f�[�^���" ); 
			this->labTitleName->Text  = gcnew System::String( "�\�t�g�^�C�g��" );
			this->labGameCode->Text   = gcnew System::String( "�C�j�V�����R�[�h" );
			this->labMakerCode->Text  = gcnew System::String( "���[�J�R�[�h" );
			this->labPlatform->Text   = gcnew System::String( "�v���b�g�t�H�[��" );
			this->labRomType->Text    = gcnew System::String( "ROM�^�C�v�ݒ�" );
			this->labRomSize->Text    = gcnew System::String( "ROM�e��" );
			this->labRemasterVer->Text = gcnew System::String( "���}�X�^�[�o�[�W����" );
			this->cboxRemasterVerE->Text = gcnew System::String( "E(������)" );
			this->labBackup->Text     = gcnew System::String( "�o�b�N�A�b�v������" );
			this->labHeaderCRC->Text  = gcnew System::String( "�w�b�_CRC" );
			this->labRomCRC->Text     = gcnew System::String( "�S�̂�CRC" );
			index = this->combBackup->SelectedIndex;
			this->combBackup->Items->Clear();
			this->combBackup->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"4Kbit EEPROM", L"64Kbit EEPROM", L"512Kbit EEPROM", 
				L"256Kbit FRAM", L"2Mbit FLASH", L"4Mbit FLASH", L"8Mbit FLASH", L"�Ȃ�", L"���̑�"});
			this->combBackup->SelectedIndex = index;

			// �t�����g�p�l������
			this->gboxFileOpen->Text  = gcnew System::String( "ROM�f�[�^�t�@�C���̓��o��" );
			this->butOpen->Text       = gcnew System::String( "ROM�f�[�^���J��" );
			this->butSaveAs->Text     = gcnew System::String( "���͏��𔽉f�����ĕۑ�" );
			this->butMakeMaster->Text = gcnew System::String( "�}�X�^��o���ނ��쐬" );

			// ��o���^�u
			this->tabSubmitInfo->Text  = gcnew System::String( "��o���" );
			this->labProductName->Text = gcnew System::String( "���i��" );
			this->labProductCode->Text = gcnew System::String( "���i�R�[�h" );
			this->labReleaseDate->Text = gcnew System::String( "�����\���" );
			this->labSubmiteDate->Text = gcnew System::String( "���ޒ�o��" );
			this->gboxSubmitWay->Text  = gcnew System::String( "��o���@" );
			this->rSubmitPost->Text    = gcnew System::String( "�X��" );
			this->rSubmitHand->Text    = gcnew System::String( "��n��" );
			this->gboxUsage->Text      = gcnew System::String( "�p�r" );
			this->rUsageSale->Text     = gcnew System::String( "��ʔ̔��p" );
			this->rUsageSample->Text   = gcnew System::String( "���V��p" );
			this->rUsageDst->Text      = gcnew System::String( "�f�[�^�z�M�p" );
			this->rUsageOther->Text    = gcnew System::String( "���̑�" );
			this->labSubmitVer->Text     = gcnew System::String( "��o�o�[�W����" );
			this->labCapSubmitVer->Text  = gcnew System::String( "* ���}�X�^�[�o�[�W�������オ��ƍĂ�0����J�E���g" );
			// �C�O�Ń^�u
			this->tabForeignInfo->Text = gcnew System::String( "�C�O��" );
			this->labProductNameForeign->Text = gcnew System::String( "���i��" );
			this->labProductCodeForeign->Text = gcnew System::String( "���i�R�[�h" );
			this->cboxReleaseForeign->Text    = gcnew System::String( "�C�O�ł𔭔�����(�\��)" );
			this->labMultiForeign1->Text      = gcnew System::String( "* ��������ꍇ�̂�" );
			this->labMultiForeign2->Text      = gcnew System::String( "* ��������ꍇ�̂�" );
			// ��Џ��^�u
			this->tabCompanyInfo1->Text = gcnew System::String( "��Џ��1" );
			this->tabCompanyInfo2->Text = gcnew System::String( "��Џ��2" );
			this->gboxPerson1->Text    = gcnew System::String( "�S����1" );
			this->gboxPerson2->Text    = gcnew System::String( "�S����2" );
			this->cboxIsInputPerson2->Text = gcnew System::String( "�S����2����͂���" );
			this->labCompany1->Text    = gcnew System::String( "�M�Ж�" );
			this->labDepart1->Text     = gcnew System::String( "������" );
			this->labPerson1->Text     = gcnew System::String( "������" );
			this->labCompany2->Text    = gcnew System::String( "�M�Ж�" );
			this->labDepart2->Text     = gcnew System::String( "������" );
			this->labPerson2->Text     = gcnew System::String( "������" );
			// �ӂ肪�ȏ���L���ɂ���
			this->tboxFurigana1->Enabled = true;
			this->labFurigana1->Text = gcnew System::String("�ӂ肪��");
			this->tboxFurigana2->Enabled = true;
			this->labFurigana2->Text = gcnew System::String("�ӂ肪��");
			// ���l�^�u
			this->tabCaption->Text    = gcnew System::String( "���l" );
			this->labCaption->Text    = gcnew System::String( "���̑��A������������΂��L����������" );

			// �^�u�̒��ӏ���
			this->labCautionInput->Text = gcnew System::String( "* �����̍��ڂ͏��ޒ�o�ɕK�v�ȏ��ł��B���͂��Ă��������B" );
			this->labCautionCheck->Text = gcnew System::String( "* �����̍��ڂ�ROM�f�[�^�̊m�F�p�ł��B���͂̕K�v�͂���܂���B" );

			// TWL�d�l
			this->tabTWLSpec1->Text           = gcnew System::String( "TWL�d�l1" );
			this->tabTWLSpec2->Text           = gcnew System::String( "TWL�d�l2" );
			this->tabTWLSpec3->Text           = gcnew System::String( "TWL�d�l3" );
			this->tabTWLSpec4->Text           = gcnew System::String( "TWL�d�l4" );
			this->tabSDK->Text                = gcnew System::String( "SDK" );
			this->gboxTWLExInfo->Text         = gcnew System::String( "TWL�g�����" );
			this->labNormalRomOffset->Text    = gcnew System::String( "TWL�m�[�}���̈�ROM�I�t�Z�b�g" );
			this->labKeyTableRomOffset->Text  = gcnew System::String( "TWL��p�̈�ROM�I�t�Z�b�g" );
			this->cboxIsNormalJump->Text      = gcnew System::String( "�m�[�}���W�����v����" );
			this->cboxIsTmpJump->Text         = gcnew System::String( "tmp�W�����v����" );
			this->cboxIsSubBanner->Text       = gcnew System::String( "�T�u�o�i�[�t�@�C���L��" );
			this->cboxIsWL->Text              = gcnew System::String( "NTR�z���C�g���X�g�����L��" );
			this->gboxAccess->Text            = gcnew System::String( "�A�N�Z�X�R���g���[�����" );
			this->cboxIsSD->Text              = gcnew System::String( "SD�J�[�h" );
			this->cboxIsNAND->Text            = gcnew System::String( "NAND�t���b�V��������" );
			this->labIsGameCardOn->Text       = gcnew System::String( "�Q�[���J�[�h�d��" );
			this->labAccessOther->Text        = gcnew System::String( "���̑�" );
			this->gboxShared2Size->Text       = gcnew System::String( "Shared2�t�@�C���T�C�Y" );
			this->cboxIsShared2->Text         = gcnew System::String( "Shared2�t�@�C���g�p" );
			this->labSDK->Text                = gcnew System::String( "SDK�o�[�W����" );
			this->labLib->Text                = gcnew System::String( "�g�p���C�u����" );
			this->labCaptionEx->Text          = gcnew System::String( "���L����" );

			// ROM�f�[�^�ҏW�\���
			this->gboxTWLInfoWritable->Text   = gcnew System::String( "ROM�ҏW�\���(�K�v�ł���ΕύX���Ă�������)" );
			this->labEULA->Text               = gcnew System::String( "EULA���Ӄo�[�W����" );
			this->cboxIsEULA->Text            = gcnew System::String( "EULA����" );
			this->cboxIsWirelessIcon->Text    = gcnew System::String( "DS���C�����X�ʐM�A�C�R���\��" );
			this->cboxIsWiFiIcon->Text        = gcnew System::String( "Wi-Fi�ʐM�A�C�R���\��" );
			this->labRegion->Text             = gcnew System::String( "�J�[�h���[�W����" );
			this->combRegion->Items->Clear();
			this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5)
				{L"���{�̂�", L"�č��̂�", L"���B�̂�", L"���B�̂�", L"���B����э��B"});

			// �y�A�����^���R���g���[��
			this->labParentalRating->Text      = gcnew System::String( "���[�e�B���O" );
			this->labParentalEnable->Text      = gcnew System::String( "������L��" );
			this->labParentalForceEnable->Text = gcnew System::String( "Rating Pending" );

			index = this->combCERO->SelectedIndex;	// ��������clear����ƌ��݂�index�ɈӖ����Ȃ��Ȃ�̂őޔ�
			this->combCERO->Items->Clear();
			this->combCERO->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"A (�S�N��)", L"B (12�Έȏ�)", L"C (15�Έȏ�)", L"D (17�Έȏ�)", 
				L"Z (18�Έȏ�)", L"���R��"});
			this->combCERO->SelectedIndex = index;

			index = this->combESRB->SelectedIndex;
			this->combESRB->Items->Clear();
			this->combESRB->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"EC (3�Έȏ�)", L"E (6�Έȏ�)", L"E10+ (10�Έȏ�)", L"T (13�Έȏ�)", 
				L"M (17�Έȏ�)", L"���R��"});
			this->combESRB->SelectedIndex = index;

			index = this->combUSK->SelectedIndex;
			this->combUSK->Items->Clear();
			this->combUSK->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"�N����Ȃ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"���N�ɂ͕s�K��", 
				L"���R��"});
			this->combUSK->SelectedIndex = index;

			index = this->combPEGI->SelectedIndex;
			this->combPEGI->Items->Clear();
			this->combPEGI->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"3�Έȏ�", L"7�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", 
				L"���R��"});
			this->combPEGI->SelectedIndex = index;

			index = this->combPEGIPRT->SelectedIndex;
			this->combPEGIPRT->Items->Clear();
			this->combPEGIPRT->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"4�Έȏ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", 
				L"���R��"});
			this->combPEGIPRT->SelectedIndex = index;

			index = this->combPEGIBBFC->SelectedIndex;
			this->combPEGIBBFC->Items->Clear();
			this->combPEGIBBFC->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"3�Έȏ�", L"4�Έȏ㐄��", L"7�Έȏ�", L"8�Έȏ㐄��", L"12�Έȏ�", 
				L"15�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"���R��"});
			this->combPEGIBBFC->SelectedIndex = index;

			index = this->combOFLC->SelectedIndex;
			this->combOFLC->Items->Clear();
			this->combOFLC->Items->AddRange(gcnew cli::array< System::Object^  >(5) {L"G", L"PG", L"M", L"MA15+", L"���R��"});
			this->combOFLC->SelectedIndex = index;

			// ����Ȑݒ�p�̃e�L�X�g�{�b�N�X�̕\�L��ύX
			this->setSrlFormsCaptionEx();
		}

	private:
		void changeEnglish(void)
		{
			System::Int32 index;

			// �t�����g�p�l���㕔
			this->gboxSrl->Text       = gcnew System::String( "ROM Info." ); 
			this->labTitleName->Text  = gcnew System::String( "Game title" );
			this->labGameCode->Text   = gcnew System::String( "Game code" );
			this->labMakerCode->Text  = gcnew System::String( "Maker code" );
			this->labPlatform->Text   = gcnew System::String( "Platform" );
			this->labRomType->Text    = gcnew System::String( "ROM type" );
			this->labRomSize->Text    = gcnew System::String( "ROM size" );
			this->labRemasterVer->Text = gcnew System::String( "Release ver." );
			this->cboxRemasterVerE->Text = gcnew System::String( "E(Preliminary ver.)" );
			this->labBackup->Text     = gcnew System::String( "Backup memory" );
			this->labHeaderCRC->Text  = gcnew System::String( "Header CRC" );
			this->labRomCRC->Text     = gcnew System::String( "ROM CRC" );
			index = this->combBackup->SelectedIndex;
			this->combBackup->Items->Clear();
			this->combBackup->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"4Kbit EEPROM", L"64Kbit EEPROM", L"512Kbit EEPROM", 
				L"256Kbit FRAM", L"2Mbit FLASH", L"4Mbit FLASH", L"8Mbit FLASH", L"Nothing", L"Other"});
			this->combBackup->SelectedIndex = index;

			// �t�����g�p�l������
			this->gboxFileOpen->Text  = gcnew System::String( "ROM file I/O" );
			this->butOpen->Text       = gcnew System::String( "Open a ROM file" );
			this->butSaveAs->Text     = gcnew System::String( "Save a ROM file" );
			this->butMakeMaster->Text = gcnew System::String( "Make a submission sheet" );

			// ��o���^�u
			this->tabSubmitInfo->Text  = gcnew System::String( "Submission Info." );
			this->labProductName->Text = gcnew System::String( "Product name" );
			this->labProductCode->Text = gcnew System::String( "Product code" );
			this->labReleaseDate->Text = gcnew System::String( "Release date" );
			this->labSubmiteDate->Text = gcnew System::String( "Submission date" );
			this->gboxSubmitWay->Text  = gcnew System::String( "How to submit" );
			this->rSubmitPost->Text    = gcnew System::String( "Mail" );
			this->rSubmitHand->Text    = gcnew System::String( "Handover" );
			this->gboxUsage->Text      = gcnew System::String( "Purpose" );
			this->rUsageSale->Text     = gcnew System::String( "For Sale" );
			this->rUsageSample->Text   = gcnew System::String( "For Trial" );
			this->rUsageDst->Text      = gcnew System::String( "For Network distribution" );
			this->rUsageOther->Text    = gcnew System::String( "Other" );
			this->labSubmitVer->Text     = gcnew System::String( "Submission ver." );
			this->labCapSubmitVer->Text  = gcnew System::String( "* return to 0 when release ver. is updated." );
			// �C�O�Ń^�u
			this->tabForeignInfo->Text = gcnew System::String( "Foreign ver." );
			this->labProductNameForeign->Text = gcnew System::String( "Product name" );
			this->labProductCodeForeign->Text = gcnew System::String( "Product code" );
			this->cboxReleaseForeign->Text    = gcnew System::String( "Foreign ver. is scheduled to go on sale" );
			this->labMultiForeign1->Text      = gcnew System::String( "* Only release multi ver." );
			this->labMultiForeign2->Text      = gcnew System::String( "* Only release multi ver." );
			// ��Џ��^�u
			this->tabCompanyInfo1->Text = gcnew System::String( "Campany1" );
			this->tabCompanyInfo2->Text = gcnew System::String( "Campany2" );
			this->gboxPerson1->Text    = gcnew System::String( "Account1" );
			this->gboxPerson2->Text    = gcnew System::String( "Account2" );
			this->cboxIsInputPerson2->Text = gcnew System::String( "Input Account2" );
			this->labCompany1->Text    = gcnew System::String( "Company" );
			this->labDepart1->Text     = gcnew System::String( "Dept." );
			this->labPerson1->Text     = gcnew System::String( "Name" );
			this->labCompany2->Text    = gcnew System::String( "Company" );
			this->labDepart2->Text     = gcnew System::String( "Dept." );
			this->labPerson2->Text     = gcnew System::String( "Name" );
			// �ӂ肪�ȏ����폜
			this->tboxFurigana1->Clear();
			this->tboxFurigana1->Enabled = false;
			this->labFurigana1->Text = gcnew System::String("");
			this->tboxFurigana2->Clear();
			this->tboxFurigana2->Enabled = false;
			this->labFurigana2->Text = gcnew System::String("");
			// ���l�^�u
			this->tabCaption->Text    = gcnew System::String( "Remarks" );
			this->labCaption->Text    = gcnew System::String( "Please write further information." );

			// �^�u�̒��ӏ���
			this->labCautionInput->Text = gcnew System::String( "* These items are necesarry for submission. Please input." );
			this->labCautionCheck->Text = gcnew System::String( "* These items are used for checking ROM data. They are read only." );

			// TWL�d�l
			this->tabTWLSpec1->Text           = gcnew System::String( "TWL Spec.1" );
			this->tabTWLSpec2->Text           = gcnew System::String( "TWL Spec.2" );
			this->tabTWLSpec3->Text           = gcnew System::String( "TWL Spec.3" );
			this->tabTWLSpec4->Text           = gcnew System::String( "TWL Spec.4" );
			this->tabSDK->Text                = gcnew System::String( "SDK" );
			this->gboxTWLExInfo->Text         = gcnew System::String( "TWL Extended Info" );
			this->labNormalRomOffset->Text    = gcnew System::String( "TWL normal area ROM offset" );
			this->labKeyTableRomOffset->Text  = gcnew System::String( "TWL secure area ROM offset" );
			this->cboxIsNormalJump->Text      = gcnew System::String( "Enable Normal app. jump" );
			this->cboxIsTmpJump->Text         = gcnew System::String( "Enable Temp. app. jump" );
			this->cboxIsSubBanner->Text       = gcnew System::String( "Enable SubBanner file" );
			this->cboxIsWL->Text              = gcnew System::String( "Enable NTR WhiteList signature" );
			this->gboxAccess->Text            = gcnew System::String( "Access Control" );
			this->cboxIsSD->Text              = gcnew System::String( "SD Card" );
			this->cboxIsNAND->Text            = gcnew System::String( "NAND Flash Memory" );
			this->labIsGameCardOn->Text       = gcnew System::String( "Card Power" );
			this->labAccessOther->Text        = gcnew System::String( "Others" );
			this->gboxShared2Size->Text       = gcnew System::String( "Size of Shared2 Files" );
			this->cboxIsShared2->Text         = gcnew System::String( "Use Shared2 files" );
			this->labSDK->Text                = gcnew System::String( "SDK ver." );
			this->labLib->Text                = gcnew System::String( "Libraries used by the program" );
			this->labCaptionEx->Text          = gcnew System::String( "Special note" );

			// ROM�f�[�^�ҏW�\���
			this->gboxTWLInfoWritable->Text   = gcnew System::String( "ROM Writable Info.(Please Change if necesarry.)" );
			this->labEULA->Text               = gcnew System::String( "EULA agreement ver." );
			this->cboxIsEULA->Text            = gcnew System::String( "Agree EULA" );
			this->cboxIsWirelessIcon->Text    = gcnew System::String( "Display an Icon of DS Wireless" );
			this->cboxIsWiFiIcon->Text        = gcnew System::String( "Display an Icon of Wi-Fi connection" );
			this->labRegion->Text             = gcnew System::String( "Card Region" );
			this->combRegion->Items->Clear();
			this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5)
				{L"Japan Only", L"USA Only", L"Europe Only", L"Australia only", L"Europe and Australia"});

			// �y�A�����^���R���g���[��
			this->labParentalRating->Text      = gcnew System::String( "Rating" );
			this->labParentalEnable->Text      = gcnew System::String( "Enable Control" );
			this->labParentalForceEnable->Text = gcnew System::String( "Rating Pending" );

			index = this->combCERO->SelectedIndex;	// ��������clear����ƌ��݂�index�ɈӖ����Ȃ��Ȃ�̂őޔ�
			this->combCERO->Items->Clear();
			this->combCERO->Items->AddRange(gcnew cli::array< System::Object^  >(6)
				{ L"A (All ages)", L"B (aged 12 or older)", L"C (aged 15 or older)", L"D (aged 17 or older)", L"Z (aged 18 or older)", L"Unexamined"});
			this->combCERO->SelectedIndex = index;

			index = this->combESRB->SelectedIndex;
			this->combESRB->Items->Clear();
			this->combESRB->Items->AddRange(gcnew cli::array< System::Object^  >(6)
				{L"EC (aged 3 or older)", L"E (aged 6 or older)", L"E10+ (aged 10 or older)", L"T (aged 13 or older)",	L"M (aged 17 or older)", L"Unexamined"});
			this->combESRB->SelectedIndex = index;

			index = this->combUSK->SelectedIndex;
			this->combUSK->Items->Clear();
			this->combUSK->Items->AddRange(gcnew cli::array< System::Object^  >(6)
				{L"All ages", L"aged 6 or older", L"aged 12 or older", L"aged 16 or older", L"Inadequent for young", L"Unexamined"});
			this->combUSK->SelectedIndex = index;

			index = this->combPEGI->SelectedIndex;
			this->combPEGI->Items->Clear();
			this->combPEGI->Items->AddRange(gcnew cli::array< System::Object^  >(6)
				{L"aged 3 or older", L"aged 7 or older", L"aged 12 or older", L"aged 16 or older", L"aged 18 or older", L"Unexamined"});
			this->combPEGI->SelectedIndex = index;

			index = this->combPEGIPRT->SelectedIndex;
			this->combPEGIPRT->Items->Clear();
			this->combPEGIPRT->Items->AddRange(gcnew cli::array< System::Object^  >(6)
				{L"aged 4 or older", L"aged 6 or older", L"aged 12 or older", L"aged 16 or older", L"aged 18 or older", L"Unexamined"});
			this->combPEGIPRT->SelectedIndex = index;

			index = this->combPEGIBBFC->SelectedIndex;
			this->combPEGIBBFC->Items->Clear();
			this->combPEGIBBFC->Items->AddRange(gcnew cli::array< System::Object^  >(9)
				{L"aged 3 or older", L"aged 4 or older recommended", L"aged 7 or older", L"aged 8 or older recommended",
				 L"aged 12 or older", L"aged 15 or older", L"aged 16 or older", L"aged 18 or older", L"Unexamined"});
			this->combPEGIBBFC->SelectedIndex = index;

			index = this->combOFLC->SelectedIndex;
			this->combOFLC->Items->Clear();
			this->combOFLC->Items->AddRange(gcnew cli::array< System::Object^  >(5) {L"G", L"PG", L"M", L"MA15+", L"Unexamined"});
			this->combOFLC->SelectedIndex = index;

			// ����Ȑݒ�p�̃e�L�X�g�{�b�N�X�̕\�L��ύX
			this->setSrlFormsCaptionEx();
		}

	private:
		// SRL�̃I�[�v��
		System::Void loadSrl( System::String ^filename )
		{
			if( this->hSrl->readFromFile( filename ) != ECSrlResult::NOERROR )
			{
				if( this->rSelectJ->Checked == true )
					this->errMsg( "���[�h�Ɏ��s���܂����B" );
				else
					this->errMsg( "Reading the file failed." );
				return;							// �O�̃t�@�C��������ł���ۏ؂Ȃ��Ȃ̂őO�̃t�@�C�����㏑���ۑ��ł��Ȃ��悤�ɂ���
			}
			this->tboxFile->Text = filename;

			// GUI��ROM�����i�[
			this->setSrlForms();

			// �S�̂�CRC���Z�o
			u16  crc;
			if( !getWholeCRCInFile( filename, &crc ) )
			{
				if( this->rSelectJ->Checked == true )
					this->errMsg( "CRC�̌v�Z�Ɏ��s���܂����B" );
				else
					this->errMsg( "Calc CRC failed." );
				return;
			}
			System::UInt16 ^hcrc = gcnew System::UInt16( crc );
			this->tboxWholeCRC->Clear();
			this->tboxWholeCRC->AppendText( "0x" );
			this->tboxWholeCRC->AppendText( hcrc->ToString("X") );

			if( this->rSelectJ->Checked == true )
				this->tboxMsg->Text  = "�t�@�C���I�[�v���ɐ������܂����B";
			else
				this->tboxMsg->Text  = "Opening the file succeeded.";
		} // openSrl

		// SRL�̕ۑ�
		System::Void saveSrl( System::String ^filename )
		{
			// SRL�֘A�t�H�[�����͂��`�F�b�N����
			if( this->checkSrlForms() == false )
			{
				return;
			}

			// ROM�����t�H�[������擾����SRL�o�C�i���ɔ��f������
			this->setSrlPropaties();
			// �}�X�^���ޏ����t�H�[������擾���ď��ނɔ��f������ -> �K�v�Ȃ�
			//this->setDeliverablePropaties();

			// �t�@�C�����R�s�[
			if( !(filename->Equals( this->tboxFile->Text )) )
			{
				System::IO::File::Copy( this->tboxFile->Text, filename, true );
			}

			// �R�s�[�����t�@�C����ROM�w�b�_���㏑��
			if( this->hSrl->writeToFile( filename ) != ECSrlResult::NOERROR )
			{
				if( this->rSelectJ->Checked == true )
					this->errMsg( "�ۑ��Ɏ��s���܂����B" );
				else
					this->errMsg( "Saving the file failed." );
				return;
			}
			if( this->rSelectJ->Checked == true )
				this->tboxMsg->Text = "�ۑ����������܂����B";
			else
				this->tboxMsg->Text = "Saving the file succeeded.";
			this->tboxFile->Text = filename;

			// �ă��[�h
			this->loadSrl( filename );
		}

	/////////////////////////////////////////////
	// �t�H�[�����상�\�b�h
	/////////////////////////////////////////////

	// �J���{�^��
	private: 
		System::Void butOpen_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String^ filename;

			// �h���b�O�A���h�h���b�v�ȊO�ł̓_�C�A���O������͂���
			{
				System::Windows::Forms::OpenFileDialog ^dlg = gcnew (OpenFileDialog);

				dlg->InitialDirectory = "c:\\";
				dlg->Filter      = (this->rSelectJ->Checked == true)?"srl�`�� (*.srl)|*.srl|All files (*.*)|*.*"
																	:"srl format (*.srl)|*.srl|All files (*.*)|*.*";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					if( this->rSelectJ->Checked == true )
						this->errMsg( "�t�@�C���I�[�v�����L�����Z������܂����B" );
					else
						this->errMsg( "Opening the file is canceled." );
					return;
				}
				filename = dlg->FileName;
			}
			this->loadSrl( filename );
		} // end of butOpen_Click()

	// ���O�����ĕۑ�
	private: 
		System::Void butSaveAs_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String^ filename;

			// �_�C�A���O�Ō��߂��t�@�C����SRL��ۑ�
			{
				System::Windows::Forms::SaveFileDialog ^dlg = gcnew (SaveFileDialog);

				dlg->InitialDirectory = "c:\\";
				dlg->Filter      = (this->rSelectJ->Checked == true)?"srl�`�� (*.srl)|*.srl"
																	:"srl format (*.srl)|*.srl";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					if( this->rSelectJ->Checked == true )
						this->errMsg( "�ۑ����L�����Z������܂����̂�ROM�t�@�C���f�[�^�͍쐬(�X�V)����܂���B" );
					else
						this->errMsg( "ROM file is not updated since saving the file is canceled." );
					return;
				}
				filename = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".srl" )) )
				{
					filename += ".srl";
				}
			}
			this->saveSrl( filename );
		} // end of butSaveAs_Click()

	// �t�@�C���p�X�\���p�e�L�X�g�{�b�N�X
	private:
		// �h���b�O����Ă܂��}�E�X�̃{�^����������Ă��Ȃ��Ƃ�
		System::Void tboxFile_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e)
		{
			if( e->Data->GetDataPresent( DataFormats::FileDrop ) )
			{
				e->Effect = DragDropEffects::All;
			}
		}
		// �h���b�O���ꂽ���ƃ}�E�X�̃{�^���������ꂽ�Ƃ�
		System::Void tboxFile_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e)
		{
			array<String^> ^files = dynamic_cast< array<String^> ^>(e->Data->GetData( DataFormats::FileDrop ) );
			String ^filename = files[0];

			if( System::IO::File::Exists(filename) == false )
			{
				if( this->rSelectJ->Checked == true )
					this->errMsg( "�t�@�C�������݂��܂���̂ŊJ�����Ƃ��ł��܂���B" );
				else
					this->errMsg( "The file is not found, therefore the file can not be opened." );
				return;
			}
			this->loadSrl( filename );			// �h���b�O�A���h�h���b�v�̎��_�Ń{�^���������Ȃ��Ă��t�@�C�����J��
			this->tboxFile->Text = filename;
		}

	// �}�X�^��o���ނ��쐬�{�^��
	private:
		System::Void butMakeMaster_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String ^delivfile;
			ECDeliverableResult  result;
			System::String ^srlfile;
			System::UInt16 ^hcrc;
			cli::array<System::String^> ^paths;

			// SRL�Ə��ނ̗����̃t�H�[�����͂��`�F�b�N����
			if( this->checkSrlForms() == false )
			{
				return;
			}
			if( this->checkDeliverableForms() == false )
			{
				return;
			}

			// ���ӏ��� 
			{
				System::String ^stmp;
				if( this->rSelectJ->Checked == true )
					stmp = "Step1/2: ROM�f�[�^�t�@�C��(SRL)�ƒ�o���ނ̏�����v�����邽�߁A�܂��A���͏��𔽉f������SRL���쐬���܂��B\n(�L�����Z�����ꂽ�Ƃ��ASRL����ђ�o���ނ͍쐬����܂���B)";
				else
					stmp = "Step1/2: Firstly, We save ROM file(SRL) because several information in a submission sheet are match those in the SRL.\n(When it is canceled, both the SRL and a submission sheet are not made.)";
				MessageBox::Show( stmp, "Caution", MessageBoxButtons::OK, MessageBoxIcon::Information );
			}
			// �_�C�A���O����Srl�����擾����
			{
				System::Windows::Forms::SaveFileDialog ^dlg = gcnew (SaveFileDialog);

				dlg->InitialDirectory = "c:\\";
				dlg->Filter      = "srl�`�� (*.srl)|*.srl";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					if( this->rSelectJ->Checked == true )
						this->errMsg( "SRL�̕ۑ����L�����Z������܂����̂Œ�o���ނ͍쐬����܂���B");
					else
						this->errMsg( "A submission sheet can not be made, since saving SRL is canceled." );
					return;
				}
				srlfile = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".srl" )) )
				{
					srlfile += ".srl";
				}
			}

			// �}�X�^��o���ނɕK�v�ȏ����t�H�[������擾���čX�V
			this->setSrlPropaties();	// ���Srl���X�V���Ă���
			this->setDeliverablePropaties();

			// ���ӏ��� 
			{
				System::String ^stmp;
				if( this->rSelectJ->Checked == true )
					stmp = "Step2/2: �����Ē�o���ނ��쐬���܂��B\n�����ŃL�����Z�����ꂽ�Ƃ��A��o���ނ͂��Ƃ��SRL���쐬(�X�V)����܂���̂ł����ӂ��������B";
				else
					stmp = "Step2/2: Secondly, We should make a submission sheet. \n(CAUTION: When it is canceled, not only a submission sheet is not made, but also the SRL is selected previously.)";
				MessageBox::Show( stmp, "Caution", MessageBoxButtons::OK, MessageBoxIcon::Information );
			}
			// �_�C�A���O�Ńt�@�C���p�X������
			{
				System::Windows::Forms::SaveFileDialog ^dlg = gcnew (SaveFileDialog);

				dlg->InitialDirectory = "c:\\";
				dlg->Filter      = "xml�`�� (*.xml)|*.xml";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					if( this->rSelectJ->Checked == true )
						this->errMsg( "��o���ނ̍쐬���L�����Z������܂����B" );
					else
						this->errMsg( "Making a submission sheet is canceled." );
					return;
				}
				delivfile = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".xml" )) )
				{
					delivfile += ".xml";
				}
			}

			// SRL���X�V
			this->saveSrl( srlfile );
			u16  crc;			// SRL�S�̂�CRC���v�Z����(���ނɋL�q���邽��)
			if( !getWholeCRCInFile( srlfile, &crc ) )
			{
				if( this->rSelectJ->Checked == true )
					this->errMsg( "CRC�̌v�Z�Ɏ��s���܂����B��o���ނ̍쐬���L�����Z�����܂��B" );
				else
					this->errMsg( "Calc CRC is failed. Therefore, Making a submission sheet is canceled." );
				return;
			}
			hcrc = gcnew System::UInt16( crc );
			this->tboxWholeCRC->Clear();
			this->tboxWholeCRC->AppendText( "0x" );
			this->tboxWholeCRC->AppendText( hcrc->ToString("X") );

			// ���ލ쐬
			paths = srlfile->Split(L'\\');			// �]���ȃp�X���폜
			srlfile = paths[ paths->Length - 1 ];
			//result = this->hDeliv->write( delivfile, this->hSrl, hcrc, srlfile, !(this->rSelectJ->Checked) );
			result = this->hDeliv->writeSpreadsheet( delivfile, this->hSrl, hcrc, srlfile, !(this->rSelectJ->Checked) );
			if( result != ECDeliverableResult::NOERROR )
			{
				if( this->rSelectJ->Checked == true )
					this->errMsg( "���ނ̍쐬�Ɏ��s���܂����B" );
				else
					this->errMsg( "Making a submission sheet is failed." );
				return;
			}
			this->tboxMsg->Text = "���ނ̍쐬�ɐ������܂����B";

		} // end of butMakeMaster_Click

	// �`�F�b�N�{�^�����������Ƃ��ɑ��̃t�H�[����L���ɂ���
	private:
		System::Void cboxIsInputPerson2_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{
			this->gboxPerson2->Enabled = this->cboxIsInputPerson2->Checked;
			if( this->cboxIsInputPerson2->Checked == true )
			{
				if( (this->tboxCompany1->Text != nullptr) && !(this->tboxCompany1->Text->Equals("")) )
				{
					this->tboxCompany2->Text = gcnew System::String( this->tboxCompany1->Text );
				}
				if( (this->tboxDepart1->Text != nullptr) && !(this->tboxDepart1->Text->Equals("")) )
				{
					this->tboxDepart2->Text  = gcnew System::String( this->tboxDepart1->Text );
				}
			}
			else
			{
				this->tboxCompany2->Clear();
				this->tboxDepart2->Clear();
				this->tboxPerson2->Clear();
				this->tboxFurigana2->Clear();
				this->tboxTel2->Clear();
				this->tboxFax2->Clear();
				this->tboxMail2->Clear();
				this->tboxNTSC2->Clear();
			}
		}
	private:
		System::Void rUsageOther_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{
			this->tboxUsageOther->Enabled = this->rUsageOther->Checked;
			if( this->rUsageOther->Checked == false )
			{
				this->tboxUsageOther->Clear();
			}
		}
	private:
		System::Void cboxRemasterVerE_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{
			//this->numRemasterVer->Enabled = !(this->cboxRemasterVerE->Checked);
			//if( this->cboxRemasterVerE->Checked == false )
			//{
			//	this->numRemasterVer->Value = 0;
			//}
		}
	private:
		System::Void combBackup_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
		{
			if( this->combBackup->SelectedIndex == (this->combBackup->Items->Count - 1) )
			{
				this->tboxBackupOther->Enabled = true;
			}
			else
			{
				this->tboxBackupOther->Enabled = false;
				this->tboxBackupOther->Clear();
			}
		}
	private:
		// �y�A�����^���R���g���[�������N���A����
		void clearParental( System::Windows::Forms::ComboBox ^comb, 
			                System::Windows::Forms::CheckBox ^enable, 
							System::Windows::Forms::CheckBox ^rp )
		{
			comb->SelectedIndex = comb->Items->Count - 1;	// �u���R���v�ɂ���
			enable->Checked = false;
			rp->Checked = false;
		}
	private:
		// �y�A�����^���R���g���[������ҏW�ł���悤�ɂ���
		void enableParental( System::Windows::Forms::ComboBox ^comb, 
	                         System::Windows::Forms::CheckBox ^enable, 
					         System::Windows::Forms::CheckBox ^rp )
		{
			comb->Enabled = true;
			enable->Enabled = true;
			rp->Enabled = true;
		}
		// �y�A�����^���R���g���[������ҏW�ł��Ȃ�����
		void disableParental( System::Windows::Forms::ComboBox ^comb, 
	                          System::Windows::Forms::CheckBox ^enable, 
					          System::Windows::Forms::CheckBox ^rp )
		{
			this->clearParental( comb, enable, rp );
			comb->Enabled = false;
			enable->Enabled = false;
			rp->Enabled = false;
		}
	private:
		System::Void combRegion_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
		{
			switch( this->combRegion->SelectedIndex )
			{
				case 0:
					// ���{
					this->enableParental( this->combCERO, this->cboxCERO, this->cboxAlwaysCERO );

					this->disableParental( this->combESRB, this->cboxESRB, this->cboxAlwaysESRB );
					this->disableParental( this->combUSK, this->cboxUSK, this->cboxAlwaysUSK );
					this->disableParental( this->combPEGI, this->cboxPEGI, this->cboxAlwaysPEGI );
					this->disableParental( this->combPEGIPRT, this->cboxPEGIPRT, this->cboxAlwaysPEGIPRT );
					this->disableParental( this->combPEGIBBFC, this->cboxPEGIBBFC, this->cboxAlwaysPEGIBBFC );
					this->disableParental( this->combOFLC, this->cboxOFLC, this->cboxAlwaysOFLC );
				break;

				case 1:
					// �č�
					this->disableParental( this->combCERO, this->cboxCERO, this->cboxAlwaysCERO );
					this->enableParental( this->combESRB, this->cboxESRB, this->cboxAlwaysESRB );
					this->disableParental( this->combUSK, this->cboxUSK, this->cboxAlwaysUSK );
					this->disableParental( this->combPEGI, this->cboxPEGI, this->cboxAlwaysPEGI );
					this->disableParental( this->combPEGIPRT, this->cboxPEGIPRT, this->cboxAlwaysPEGIPRT );
					this->disableParental( this->combPEGIBBFC, this->cboxPEGIBBFC, this->cboxAlwaysPEGIBBFC );
					this->disableParental( this->combOFLC, this->cboxOFLC, this->cboxAlwaysOFLC );
				break;

				case 2:
					// ���B
					this->disableParental( this->combCERO, this->cboxCERO, this->cboxAlwaysCERO );
					this->disableParental( this->combESRB, this->cboxESRB, this->cboxAlwaysESRB );
					this->enableParental( this->combUSK, this->cboxUSK, this->cboxAlwaysUSK );
					this->enableParental( this->combPEGI, this->cboxPEGI, this->cboxAlwaysPEGI );
					this->enableParental( this->combPEGIPRT, this->cboxPEGIPRT, this->cboxAlwaysPEGIPRT );
					this->enableParental( this->combPEGIBBFC, this->cboxPEGIBBFC, this->cboxAlwaysPEGIBBFC );
					this->disableParental( this->combOFLC, this->cboxOFLC, this->cboxAlwaysOFLC );
				break;

				case 3:
					// ���B
					this->disableParental( this->combCERO, this->cboxCERO, this->cboxAlwaysCERO );
					this->disableParental( this->combESRB, this->cboxESRB, this->cboxAlwaysESRB );
					this->disableParental( this->combUSK, this->cboxUSK, this->cboxAlwaysUSK );
					this->disableParental( this->combPEGI, this->cboxPEGI, this->cboxAlwaysPEGI );
					this->disableParental( this->combPEGIPRT, this->cboxPEGIPRT, this->cboxAlwaysPEGIPRT );
					this->disableParental( this->combPEGIBBFC, this->cboxPEGIBBFC, this->cboxAlwaysPEGIBBFC );
					this->enableParental( this->combOFLC, this->cboxOFLC, this->cboxAlwaysOFLC );
				break;

				case 4:
					// ���B�ƍ��B
					this->disableParental( this->combCERO, this->cboxCERO, this->cboxAlwaysCERO );
					this->disableParental( this->combESRB, this->cboxESRB, this->cboxAlwaysESRB );
					this->enableParental( this->combUSK, this->cboxUSK, this->cboxAlwaysUSK );
					this->enableParental( this->combPEGI, this->cboxPEGI, this->cboxAlwaysPEGI );
					this->enableParental( this->combPEGIPRT, this->cboxPEGIPRT, this->cboxAlwaysPEGIPRT );
					this->enableParental( this->combPEGIBBFC, this->cboxPEGIBBFC, this->cboxAlwaysPEGIBBFC );
					this->enableParental( this->combOFLC, this->cboxOFLC, this->cboxAlwaysOFLC );
				break;

				default:
				break;
			}
		}

	private:
		// ���{��łւ̐؂�ւ�
		System::Void rSelectJ_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{
			this->changeJapanese();
		}

	private:
		// �p��łւ̐؂�ւ�
		System::Void rSelectE_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{
			this->changeEnglish();
		}

	private:
		System::Void cboxReleaseForeign_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{
			this->tboxProductNameForeign->Enabled   = this->cboxReleaseForeign->Checked;
			this->tboxProductCode1Foreign->Enabled  = this->cboxReleaseForeign->Checked;
			this->tboxProductCode2Foreign1->Enabled = this->cboxReleaseForeign->Checked;
			this->tboxProductCode2Foreign2->Enabled = this->cboxReleaseForeign->Checked;
			this->tboxProductCode2Foreign3->Enabled = this->cboxReleaseForeign->Checked;
			if( this->cboxReleaseForeign->Checked == false )
			{
				this->tboxProductNameForeign->Clear();
				this->tboxProductCode1Foreign->Clear();
				this->tboxProductCode2Foreign1->Clear();
				this->tboxProductCode2Foreign2->Clear();
				this->tboxProductCode2Foreign3->Clear();
			}
		}

}; // enf of ref class Form1

} // end of namespace MasterEditorTWL

