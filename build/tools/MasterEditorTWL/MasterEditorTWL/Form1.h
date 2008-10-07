#pragma once

#include <apptype.h>
#include "common.h"
#include "srl.h"
#include "deliverable.h"
#include "crc_whole.h"
#include "utility.h"
#include "lang.h"
#include "FormError.h"

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

		// ���̓G���[���
		System::Collections::Generic::List<RCMrcError ^> ^hErrorList;
		System::Collections::Generic::List<RCMrcError ^> ^hWarnList;

		// SRL�ɓo�^����Ȃ�ROM�d�l��ǂݍ��ݎ��̏�Ԃɖ߂���d�g��
		System::Boolean ^hIsCheckedUGC;			// �ǂݍ��ݎ��Ƀ`�F�b�N����Ă�����
		System::Boolean ^hIsCheckedPhotoEx;

	// VC�����ǉ��t�B�[���h
	private: System::Windows::Forms::GroupBox^  gboxCRC;
	private: System::Windows::Forms::TextBox^  tboxWholeCRC;

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



	private: System::Windows::Forms::TextBox^  tboxRomSize;





























































	private: System::Windows::Forms::Label^  labOFLC;
	private: System::Windows::Forms::Label^  labPEGI_BBFC;
	private: System::Windows::Forms::Label^  labPEGI_PRT;
	private: System::Windows::Forms::Label^  labPEGI;
	private: System::Windows::Forms::Label^  labUSK;
	private: System::Windows::Forms::Label^  labESRB;
	private: System::Windows::Forms::Label^  labCERO;







	private: System::Windows::Forms::ComboBox^  combOFLC;
	private: System::Windows::Forms::ComboBox^  combPEGI_BBFC;
	private: System::Windows::Forms::ComboBox^  combPEGI_PRT;
	private: System::Windows::Forms::ComboBox^  combPEGI;
	private: System::Windows::Forms::ComboBox^  combUSK;
	private: System::Windows::Forms::ComboBox^  combESRB;
	private: System::Windows::Forms::ComboBox^  combCERO;








	private: System::Windows::Forms::Label^  labParentalRating;




































	private: System::Windows::Forms::CheckBox^  cboxIsEULA;



























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









private: System::Windows::Forms::Label^  labProductCode2Foreign;
private: System::Windows::Forms::CheckBox^  cboxReleaseForeign;
private: System::Windows::Forms::Label^  labProductNameForeign;
private: System::Windows::Forms::TextBox^  tboxProductNameForeign;
private: System::Windows::Forms::Label^  labProductCode1Foreign;
private: System::Windows::Forms::TextBox^  tboxProductCode1Foreign;
private: System::Windows::Forms::Label^  labProductCodeForeign;
private: System::Windows::Forms::TextBox^  tboxProductCode2Foreign1;














private: System::Windows::Forms::GroupBox^  gboxShared2Size;
private: System::Windows::Forms::Label^  labShared2Size5;
private: System::Windows::Forms::Label^  labShared2Size4;
private: System::Windows::Forms::Label^  labShared2Size3;
private: System::Windows::Forms::Label^  labShared2Size2;
private: System::Windows::Forms::Label^  labShared2Size1;
private: System::Windows::Forms::Label^  labShared2Size0;






private: System::Windows::Forms::TextBox^  tboxShared2Size5;
private: System::Windows::Forms::TextBox^  tboxShared2Size4;
private: System::Windows::Forms::TextBox^  tboxShared2Size3;
private: System::Windows::Forms::TextBox^  tboxShared2Size2;
private: System::Windows::Forms::TextBox^  tboxShared2Size1;
private: System::Windows::Forms::TextBox^  tboxShared2Size0;
private: System::Windows::Forms::CheckBox^  cboxIsShared2;

private: System::Windows::Forms::Label^  labLib;
private: System::Windows::Forms::TextBox^  tboxSDK;
private: System::Windows::Forms::Label^  labSDK;

private: System::Windows::Forms::GroupBox^  gboxTWLExInfo;



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


private: System::Windows::Forms::GroupBox^  gboxAccess;
private: System::Windows::Forms::Label^  labAccessOther;
private: System::Windows::Forms::TextBox^  tboxAccessOther;
private: System::Windows::Forms::TextBox^  tboxIsGameCardOn;
private: System::Windows::Forms::Label^  labIsGameCardOn;
private: System::Windows::Forms::CheckBox^  cboxIsNAND;
private: System::Windows::Forms::CheckBox^  cboxIsSD;



private: System::Windows::Forms::GroupBox^  gboxTitleID;
private: System::Windows::Forms::Label^  labHex2;

private: System::Windows::Forms::TextBox^  tboxTitleIDLo;
private: System::Windows::Forms::Label^  labTitleIDLo;
private: System::Windows::Forms::Label^  labTitleIDHi;
private: System::Windows::Forms::TextBox^  tboxTitleIDHi;
private: System::Windows::Forms::TextBox^  tboxAppTypeOther;

private: System::Windows::Forms::Label^  labAppTypeOther;

private: System::Windows::Forms::Label^  labCaptionEx;
private: System::Windows::Forms::TextBox^  tboxCaptionEx;



private: System::Windows::Forms::TextBox^  tboxProductCode2Foreign3;

private: System::Windows::Forms::TextBox^  tboxProductCode2Foreign2;
private: System::Windows::Forms::Label^  labMultiForeign1;



private: System::Windows::Forms::GroupBox^  gboxProd;



private: System::Windows::Forms::MenuStrip^  menuStripAbove;

private: System::Windows::Forms::ToolStripMenuItem^  stripFile;
private: System::Windows::Forms::ToolStripMenuItem^  stripItemOpenRom;

private: System::Windows::Forms::ToolStripMenuItem^  stripItemSaveTemp;
private: System::Windows::Forms::ToolStripMenuItem^  stripItemLoadTemp;
private: System::Windows::Forms::ToolStripMenuItem^  stripMaster;
private: System::Windows::Forms::ToolStripMenuItem^  stripItemMasterRom;
private: System::Windows::Forms::ToolStripMenuItem^  stripItemSheet;
private: System::Windows::Forms::ToolStripMenuItem^  stripLang;
private: System::Windows::Forms::ToolStripMenuItem^  stripItemEnglish;
private: System::Windows::Forms::ToolStripMenuItem^  stripItemJapanese;
private: System::Windows::Forms::TabControl^  tabMain;
private: System::Windows::Forms::TabPage^  tabRomInfo;


private: System::Windows::Forms::TabPage^  tabTWLInfo;
private: System::Windows::Forms::TabPage^  tabRomEditInfo;
private: System::Windows::Forms::TabPage^  tabSubmitInfo;







private: System::Windows::Forms::TabPage^  tabCompanyInfo;

private: System::Windows::Forms::GroupBox^  gboxForeign;
private: System::Windows::Forms::GroupBox^  gboxExFlags;



private: System::Windows::Forms::GroupBox^  gboxEULA;
private: System::Windows::Forms::GroupBox^  gboxIcon;
private: System::Windows::Forms::GroupBox^  gboxParental;
private: System::Windows::Forms::TextBox^  tboxGuideRomInfo;
private: System::Windows::Forms::TextBox^  tboxGuideTWLInfo;
private: System::Windows::Forms::TextBox^  tboxGuideRomEditInfo;
private: System::Windows::Forms::TextBox^  tboxGuideSubmitInfo;
private: System::Windows::Forms::TextBox^  tboxGuideCompanyInfo;










private: System::Windows::Forms::Label^  labFile;
public: System::Windows::Forms::DataGridView^  gridError;
private: System::Windows::Forms::TabPage^  tabErrorInfo;
public: 
private: 

public: 




public: System::Windows::Forms::DataGridView^  gridWarn;
private: System::Windows::Forms::TextBox^  tboxGuideErrorInfo;
public: 
private: 

public: 




private: System::Windows::Forms::GroupBox^  gboxErrorTiming;
private: System::Windows::Forms::Label^  labWarn;
private: System::Windows::Forms::Label^  labError;
private: System::Windows::Forms::RadioButton^  rErrorCurrent;

private: System::Windows::Forms::RadioButton^  rErrorReading;








private: System::Windows::Forms::Label^  labMultiForeign2;
private: System::Windows::Forms::DataGridView^  gridLibrary;










private: System::Windows::Forms::Label^  labAssemblyVersion;
private: System::Windows::Forms::Label^  labArbit4;
private: System::Windows::Forms::Label^  labArbit3;
private: System::Windows::Forms::Label^  labArbit2;
private: System::Windows::Forms::Label^  labArbit1;
private: System::Windows::Forms::Button^  butSetBack;
private: System::Windows::Forms::Label^  labAppType;

private: System::Windows::Forms::Label^  labMedia;
private: System::Windows::Forms::TextBox^  tboxAppType;

private: System::Windows::Forms::TextBox^  tboxMedia;
private: System::Windows::Forms::Label^  labProductNameLimit;
private: System::Windows::Forms::Label^  labProductNameLimitForeign;
private: System::Windows::Forms::RadioButton^  rIsWiFiIcon;



private: System::Windows::Forms::RadioButton^  rIsWirelessIcon;
private: System::Windows::Forms::RadioButton^  rIsNoIcon;









private: System::Windows::Forms::DataGridViewTextBoxColumn^  colWarnName;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colWarnBegin;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colWarnEnd;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colWarnCause;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colErrorName;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colErrorBegin;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colErrorEnd;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colErrorCause;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colLibPublisher;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colLibName;
private: System::Windows::Forms::ToolStripMenuItem^  stripItemMiddlewareXml;
private: System::Windows::Forms::ToolStripMenuItem^  stripItemMiddlewareHtml;
private: System::Windows::Forms::ToolStripSeparator^  stripItemSepFile1;
private: System::Windows::Forms::ToolStripSeparator^  stripItemSepMaster1;
private: System::Windows::Forms::GroupBox^  gboxOtherSpec;
private: System::Windows::Forms::CheckBox^  cboxIsUGC;
private: System::Windows::Forms::CheckBox^  cboxIsPhotoEx;










































































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
			this->hErrorList = gcnew System::Collections::Generic::List<RCMrcError^>();
			this->hErrorList->Clear();
			this->hWarnList = gcnew System::Collections::Generic::List<RCMrcError^>();
			this->hWarnList->Clear();
			this->hIsCheckedUGC     = gcnew System::Boolean(false);
			this->hIsCheckedPhotoEx = gcnew System::Boolean(false);

			// �o�[�W��������\��
			//this->labAssemblyVersion->Text = System::Windows::Forms::Application::ProductVersion;
			System::Reflection::Assembly ^ass = System::Reflection::Assembly::GetEntryAssembly();
			this->labAssemblyVersion->Text = "ver." + this->getVersion();

			// �f�t�H���g�l
			this->hIsSpreadSheet = gcnew System::Boolean( true );
			this->dateRelease->Value = System::DateTime::Now;
			this->dateSubmit->Value  = System::DateTime::Now;
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
			this->combRegion->Items->Add( gcnew System::String( L"�S���[�W����" ) );
#endif

			// �A�v����ʂ�����
			System::String ^appstr = nullptr;
#ifdef METWL_VER_APPTYPE_LAUNCHER
			appstr += "Launcher/";
#endif
#ifdef METWL_VER_APPTYPE_SECURE
			appstr += "Secure/";
#endif
#ifdef METWL_VER_APPTYPE_SYSTEM
			appstr += "System/";
#endif
			if( appstr != nullptr)
			{
				this->Text += " [ Supported App: " + appstr + "User ]";
			}

			// �����s�\���������������l�Őݒ�ł��Ȃ��̂ł����Őݒ�
			this->tboxGuideRomEditInfo->Text  = "���̃^�u�̊e���ڂւ̓��͂͒�o�m�F������у}�X�^�[ROM�̍쐬�̂��߂ɕK�v�ł��B";
			this->tboxGuideRomEditInfo->Text += "\r\n�����̏��̓}�X�^�[ROM�̍쐬����ROM���o�^�f�[�^�Ƃ��ēo�^����܂�(�u���̑�ROM���v�������B)";

			this->tboxGuideErrorInfo->Text  = "���̃^�u�ɂ͓ǂݍ���ROM�f�[�^�̖��Ɩ{�v���O�����ł̓��̓~�X���񋓂���܂��B";
			this->tboxGuideErrorInfo->Text += "\r\n�ԕ����̍��ڂ́A�{�v���O�����ŏC���s�ł��BROM�f�[�^�쐬���̐ݒ�����m�F���������B";
			this->tboxGuideErrorInfo->Text += "\r\n�����̍��ڂ́A�{�v���O�����ŏC���ł��܂����A�C�����}�X�^�[ROM�ɔ��f����܂��B";
			this->tboxGuideErrorInfo->Text += "\r\n�������̍��ڂ́A��o�m�F���ɂ̂ݔ��f����A�}�X�^�[ROM�ɂ͔��f����܂���B";

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


	private: System::Windows::Forms::GroupBox^  gboxSrl;
	private: System::Windows::Forms::TextBox^  tboxTitleName;



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
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle13 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle14 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle15 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle16 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			this->tboxFile = (gcnew System::Windows::Forms::TextBox());
			this->gboxSrl = (gcnew System::Windows::Forms::GroupBox());
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
			this->labBackup = (gcnew System::Windows::Forms::Label());
			this->combBackup = (gcnew System::Windows::Forms::ComboBox());
			this->tboxBackupOther = (gcnew System::Windows::Forms::TextBox());
			this->gboxCRC = (gcnew System::Windows::Forms::GroupBox());
			this->labRomCRC = (gcnew System::Windows::Forms::Label());
			this->labHeaderCRC = (gcnew System::Windows::Forms::Label());
			this->tboxHeaderCRC = (gcnew System::Windows::Forms::TextBox());
			this->tboxWholeCRC = (gcnew System::Windows::Forms::TextBox());
			this->labCaption = (gcnew System::Windows::Forms::Label());
			this->tboxCaption = (gcnew System::Windows::Forms::TextBox());
			this->labOFLC = (gcnew System::Windows::Forms::Label());
			this->labPEGI_BBFC = (gcnew System::Windows::Forms::Label());
			this->labPEGI_PRT = (gcnew System::Windows::Forms::Label());
			this->labPEGI = (gcnew System::Windows::Forms::Label());
			this->labUSK = (gcnew System::Windows::Forms::Label());
			this->labESRB = (gcnew System::Windows::Forms::Label());
			this->labCERO = (gcnew System::Windows::Forms::Label());
			this->combOFLC = (gcnew System::Windows::Forms::ComboBox());
			this->combPEGI_BBFC = (gcnew System::Windows::Forms::ComboBox());
			this->combPEGI_PRT = (gcnew System::Windows::Forms::ComboBox());
			this->combPEGI = (gcnew System::Windows::Forms::ComboBox());
			this->combUSK = (gcnew System::Windows::Forms::ComboBox());
			this->combESRB = (gcnew System::Windows::Forms::ComboBox());
			this->combCERO = (gcnew System::Windows::Forms::ComboBox());
			this->labParentalRating = (gcnew System::Windows::Forms::Label());
			this->labRegion = (gcnew System::Windows::Forms::Label());
			this->cboxIsEULA = (gcnew System::Windows::Forms::CheckBox());
			this->combRegion = (gcnew System::Windows::Forms::ComboBox());
			this->cboxIsInputPerson2 = (gcnew System::Windows::Forms::CheckBox());
			this->gboxPerson2 = (gcnew System::Windows::Forms::GroupBox());
			this->labArbit4 = (gcnew System::Windows::Forms::Label());
			this->labArbit3 = (gcnew System::Windows::Forms::Label());
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
			this->labArbit2 = (gcnew System::Windows::Forms::Label());
			this->labArbit1 = (gcnew System::Windows::Forms::Label());
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
			this->gboxShared2Size = (gcnew System::Windows::Forms::GroupBox());
			this->labShared2Size5 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size4 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size3 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size2 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size1 = (gcnew System::Windows::Forms::Label());
			this->labShared2Size0 = (gcnew System::Windows::Forms::Label());
			this->tboxShared2Size5 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size4 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size3 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size2 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size1 = (gcnew System::Windows::Forms::TextBox());
			this->tboxShared2Size0 = (gcnew System::Windows::Forms::TextBox());
			this->cboxIsShared2 = (gcnew System::Windows::Forms::CheckBox());
			this->labLib = (gcnew System::Windows::Forms::Label());
			this->tboxSDK = (gcnew System::Windows::Forms::TextBox());
			this->labSDK = (gcnew System::Windows::Forms::Label());
			this->gboxTWLExInfo = (gcnew System::Windows::Forms::GroupBox());
			this->labByte1 = (gcnew System::Windows::Forms::Label());
			this->labHex4 = (gcnew System::Windows::Forms::Label());
			this->labHex3 = (gcnew System::Windows::Forms::Label());
			this->tboxIsCodec = (gcnew System::Windows::Forms::TextBox());
			this->labIsCodec = (gcnew System::Windows::Forms::Label());
			this->labNormalRomOffset = (gcnew System::Windows::Forms::Label());
			this->tboxNormalRomOffset = (gcnew System::Windows::Forms::TextBox());
			this->labKeyTableRomOffset = (gcnew System::Windows::Forms::Label());
			this->tboxPrivateSize = (gcnew System::Windows::Forms::TextBox());
			this->labPrivateSize = (gcnew System::Windows::Forms::Label());
			this->tboxKeyTableRomOffset = (gcnew System::Windows::Forms::TextBox());
			this->labPublicSize = (gcnew System::Windows::Forms::Label());
			this->tboxPublicSize = (gcnew System::Windows::Forms::TextBox());
			this->cboxIsSubBanner = (gcnew System::Windows::Forms::CheckBox());
			this->cboxIsWL = (gcnew System::Windows::Forms::CheckBox());
			this->cboxIsNormalJump = (gcnew System::Windows::Forms::CheckBox());
			this->cboxIsTmpJump = (gcnew System::Windows::Forms::CheckBox());
			this->gboxAccess = (gcnew System::Windows::Forms::GroupBox());
			this->labAccessOther = (gcnew System::Windows::Forms::Label());
			this->tboxAccessOther = (gcnew System::Windows::Forms::TextBox());
			this->tboxIsGameCardOn = (gcnew System::Windows::Forms::TextBox());
			this->labIsGameCardOn = (gcnew System::Windows::Forms::Label());
			this->cboxIsNAND = (gcnew System::Windows::Forms::CheckBox());
			this->cboxIsSD = (gcnew System::Windows::Forms::CheckBox());
			this->gboxTitleID = (gcnew System::Windows::Forms::GroupBox());
			this->labAppType = (gcnew System::Windows::Forms::Label());
			this->labMedia = (gcnew System::Windows::Forms::Label());
			this->tboxAppType = (gcnew System::Windows::Forms::TextBox());
			this->tboxMedia = (gcnew System::Windows::Forms::TextBox());
			this->labHex2 = (gcnew System::Windows::Forms::Label());
			this->tboxTitleIDLo = (gcnew System::Windows::Forms::TextBox());
			this->labTitleIDLo = (gcnew System::Windows::Forms::Label());
			this->labTitleIDHi = (gcnew System::Windows::Forms::Label());
			this->tboxTitleIDHi = (gcnew System::Windows::Forms::TextBox());
			this->tboxAppTypeOther = (gcnew System::Windows::Forms::TextBox());
			this->labAppTypeOther = (gcnew System::Windows::Forms::Label());
			this->labCaptionEx = (gcnew System::Windows::Forms::Label());
			this->tboxCaptionEx = (gcnew System::Windows::Forms::TextBox());
			this->gboxProd = (gcnew System::Windows::Forms::GroupBox());
			this->menuStripAbove = (gcnew System::Windows::Forms::MenuStrip());
			this->stripFile = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemOpenRom = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemSepFile1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->stripItemSaveTemp = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemLoadTemp = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripMaster = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemSheet = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemSepMaster1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->stripItemMasterRom = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemMiddlewareXml = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemMiddlewareHtml = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripLang = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemEnglish = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stripItemJapanese = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tabMain = (gcnew System::Windows::Forms::TabControl());
			this->tabRomInfo = (gcnew System::Windows::Forms::TabPage());
			this->gridLibrary = (gcnew System::Windows::Forms::DataGridView());
			this->colLibPublisher = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colLibName = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->tboxGuideRomInfo = (gcnew System::Windows::Forms::TextBox());
			this->tabTWLInfo = (gcnew System::Windows::Forms::TabPage());
			this->tboxGuideTWLInfo = (gcnew System::Windows::Forms::TextBox());
			this->gboxExFlags = (gcnew System::Windows::Forms::GroupBox());
			this->tabRomEditInfo = (gcnew System::Windows::Forms::TabPage());
			this->gboxOtherSpec = (gcnew System::Windows::Forms::GroupBox());
			this->cboxIsUGC = (gcnew System::Windows::Forms::CheckBox());
			this->cboxIsPhotoEx = (gcnew System::Windows::Forms::CheckBox());
			this->butSetBack = (gcnew System::Windows::Forms::Button());
			this->tboxGuideRomEditInfo = (gcnew System::Windows::Forms::TextBox());
			this->gboxParental = (gcnew System::Windows::Forms::GroupBox());
			this->gboxIcon = (gcnew System::Windows::Forms::GroupBox());
			this->rIsNoIcon = (gcnew System::Windows::Forms::RadioButton());
			this->rIsWiFiIcon = (gcnew System::Windows::Forms::RadioButton());
			this->rIsWirelessIcon = (gcnew System::Windows::Forms::RadioButton());
			this->gboxEULA = (gcnew System::Windows::Forms::GroupBox());
			this->tabSubmitInfo = (gcnew System::Windows::Forms::TabPage());
			this->labProductNameLimit = (gcnew System::Windows::Forms::Label());
			this->tboxGuideSubmitInfo = (gcnew System::Windows::Forms::TextBox());
			this->gboxForeign = (gcnew System::Windows::Forms::GroupBox());
			this->labProductNameLimitForeign = (gcnew System::Windows::Forms::Label());
			this->labMultiForeign2 = (gcnew System::Windows::Forms::Label());
			this->tabCompanyInfo = (gcnew System::Windows::Forms::TabPage());
			this->tboxGuideCompanyInfo = (gcnew System::Windows::Forms::TextBox());
			this->tabErrorInfo = (gcnew System::Windows::Forms::TabPage());
			this->tboxGuideErrorInfo = (gcnew System::Windows::Forms::TextBox());
			this->gboxErrorTiming = (gcnew System::Windows::Forms::GroupBox());
			this->rErrorCurrent = (gcnew System::Windows::Forms::RadioButton());
			this->rErrorReading = (gcnew System::Windows::Forms::RadioButton());
			this->labWarn = (gcnew System::Windows::Forms::Label());
			this->labError = (gcnew System::Windows::Forms::Label());
			this->gridWarn = (gcnew System::Windows::Forms::DataGridView());
			this->colWarnName = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colWarnBegin = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colWarnEnd = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colWarnCause = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->gridError = (gcnew System::Windows::Forms::DataGridView());
			this->colErrorName = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colErrorBegin = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colErrorEnd = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colErrorCause = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->labFile = (gcnew System::Windows::Forms::Label());
			this->labAssemblyVersion = (gcnew System::Windows::Forms::Label());
			this->gboxSrl->SuspendLayout();
			this->gboxCRC->SuspendLayout();
			this->gboxPerson2->SuspendLayout();
			this->gboxPerson1->SuspendLayout();
			this->gboxUsage->SuspendLayout();
			this->gboxSubmitWay->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numSubmitVersion))->BeginInit();
			this->gboxShared2Size->SuspendLayout();
			this->gboxTWLExInfo->SuspendLayout();
			this->gboxAccess->SuspendLayout();
			this->gboxTitleID->SuspendLayout();
			this->gboxProd->SuspendLayout();
			this->menuStripAbove->SuspendLayout();
			this->tabMain->SuspendLayout();
			this->tabRomInfo->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridLibrary))->BeginInit();
			this->tabTWLInfo->SuspendLayout();
			this->gboxExFlags->SuspendLayout();
			this->tabRomEditInfo->SuspendLayout();
			this->gboxOtherSpec->SuspendLayout();
			this->gboxParental->SuspendLayout();
			this->gboxIcon->SuspendLayout();
			this->gboxEULA->SuspendLayout();
			this->tabSubmitInfo->SuspendLayout();
			this->gboxForeign->SuspendLayout();
			this->tabCompanyInfo->SuspendLayout();
			this->tabErrorInfo->SuspendLayout();
			this->gboxErrorTiming->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridWarn))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridError))->BeginInit();
			this->SuspendLayout();
			// 
			// tboxFile
			// 
			this->tboxFile->AllowDrop = true;
			this->tboxFile->Location = System::Drawing::Point(120, 50);
			this->tboxFile->Name = L"tboxFile";
			this->tboxFile->ReadOnly = true;
			this->tboxFile->Size = System::Drawing::Size(607, 19);
			this->tboxFile->TabIndex = 0;
			this->tboxFile->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &Form1::tboxFile_DragDrop);
			this->tboxFile->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &Form1::tboxFile_DragEnter);
			// 
			// gboxSrl
			// 
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
			this->gboxSrl->Location = System::Drawing::Point(26, 52);
			this->gboxSrl->Name = L"gboxSrl";
			this->gboxSrl->Size = System::Drawing::Size(285, 227);
			this->gboxSrl->TabIndex = 3;
			this->gboxSrl->TabStop = false;
			this->gboxSrl->Text = L"ROM�f�[�^���";
			// 
			// tboxRemasterVer
			// 
			this->tboxRemasterVer->Location = System::Drawing::Point(109, 190);
			this->tboxRemasterVer->Name = L"tboxRemasterVer";
			this->tboxRemasterVer->ReadOnly = true;
			this->tboxRemasterVer->Size = System::Drawing::Size(42, 19);
			this->tboxRemasterVer->TabIndex = 7;
			// 
			// tboxRomSize
			// 
			this->tboxRomSize->Location = System::Drawing::Point(109, 162);
			this->tboxRomSize->Name = L"tboxRomSize";
			this->tboxRomSize->ReadOnly = true;
			this->tboxRomSize->Size = System::Drawing::Size(100, 19);
			this->tboxRomSize->TabIndex = 15;
			// 
			// tboxPlatform
			// 
			this->tboxPlatform->Location = System::Drawing::Point(109, 106);
			this->tboxPlatform->Name = L"tboxPlatform";
			this->tboxPlatform->ReadOnly = true;
			this->tboxPlatform->Size = System::Drawing::Size(100, 19);
			this->tboxPlatform->TabIndex = 14;
			// 
			// labPlatform
			// 
			this->labPlatform->AutoSize = true;
			this->labPlatform->Location = System::Drawing::Point(22, 109);
			this->labPlatform->Name = L"labPlatform";
			this->labPlatform->Size = System::Drawing::Size(73, 12);
			this->labPlatform->TabIndex = 13;
			this->labPlatform->Text = L"�v���b�g�t�H�[��";
			// 
			// tboxRomLatency
			// 
			this->tboxRomLatency->Location = System::Drawing::Point(109, 134);
			this->tboxRomLatency->Name = L"tboxRomLatency";
			this->tboxRomLatency->ReadOnly = true;
			this->tboxRomLatency->Size = System::Drawing::Size(100, 19);
			this->tboxRomLatency->TabIndex = 11;
			// 
			// labRomSize
			// 
			this->labRomSize->AutoSize = true;
			this->labRomSize->Location = System::Drawing::Point(22, 165);
			this->labRomSize->Name = L"labRomSize";
			this->labRomSize->Size = System::Drawing::Size(54, 12);
			this->labRomSize->TabIndex = 9;
			this->labRomSize->Text = L"ROM�e��";
			// 
			// labRomType
			// 
			this->labRomType->AutoSize = true;
			this->labRomType->Location = System::Drawing::Point(22, 137);
			this->labRomType->Name = L"labRomType";
			this->labRomType->Size = System::Drawing::Size(80, 12);
			this->labRomType->TabIndex = 7;
			this->labRomType->Text = L"ROM�^�C�v�ݒ�";
			// 
			// tboxMakerCode
			// 
			this->tboxMakerCode->Location = System::Drawing::Point(109, 78);
			this->tboxMakerCode->MaxLength = 2;
			this->tboxMakerCode->Name = L"tboxMakerCode";
			this->tboxMakerCode->ReadOnly = true;
			this->tboxMakerCode->Size = System::Drawing::Size(100, 19);
			this->tboxMakerCode->TabIndex = 2;
			// 
			// cboxRemasterVerE
			// 
			this->cboxRemasterVerE->AutoSize = true;
			this->cboxRemasterVerE->Enabled = false;
			this->cboxRemasterVerE->Location = System::Drawing::Point(157, 192);
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
			this->labMakerCode->Location = System::Drawing::Point(22, 81);
			this->labMakerCode->Name = L"labMakerCode";
			this->labMakerCode->Size = System::Drawing::Size(59, 12);
			this->labMakerCode->TabIndex = 4;
			this->labMakerCode->Text = L"���[�J�R�[�h";
			// 
			// labGameCode
			// 
			this->labGameCode->AutoSize = true;
			this->labGameCode->Location = System::Drawing::Point(22, 53);
			this->labGameCode->Name = L"labGameCode";
			this->labGameCode->Size = System::Drawing::Size(78, 12);
			this->labGameCode->TabIndex = 3;
			this->labGameCode->Text = L"�C�j�V�����R�[�h";
			// 
			// tboxGameCode
			// 
			this->tboxGameCode->Location = System::Drawing::Point(109, 50);
			this->tboxGameCode->MaxLength = 4;
			this->tboxGameCode->Name = L"tboxGameCode";
			this->tboxGameCode->ReadOnly = true;
			this->tboxGameCode->Size = System::Drawing::Size(100, 19);
			this->tboxGameCode->TabIndex = 1;
			// 
			// labTitleName
			// 
			this->labTitleName->AutoSize = true;
			this->labTitleName->Location = System::Drawing::Point(22, 24);
			this->labTitleName->Name = L"labTitleName";
			this->labTitleName->Size = System::Drawing::Size(65, 12);
			this->labTitleName->TabIndex = 1;
			this->labTitleName->Text = L"�\�t�g�^�C�g��";
			// 
			// labRemasterVer
			// 
			this->labRemasterVer->AutoSize = true;
			this->labRemasterVer->Location = System::Drawing::Point(13, 193);
			this->labRemasterVer->Name = L"labRemasterVer";
			this->labRemasterVer->Size = System::Drawing::Size(93, 12);
			this->labRemasterVer->TabIndex = 22;
			this->labRemasterVer->Text = L"���}�X�^�[�o�[�W����";
			// 
			// tboxTitleName
			// 
			this->tboxTitleName->ImeMode = System::Windows::Forms::ImeMode::NoControl;
			this->tboxTitleName->Location = System::Drawing::Point(109, 21);
			this->tboxTitleName->MaxLength = 12;
			this->tboxTitleName->Name = L"tboxTitleName";
			this->tboxTitleName->ReadOnly = true;
			this->tboxTitleName->Size = System::Drawing::Size(100, 19);
			this->tboxTitleName->TabIndex = 0;
			// 
			// labBackup
			// 
			this->labBackup->AutoSize = true;
			this->labBackup->Location = System::Drawing::Point(6, 21);
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
			this->combBackup->Location = System::Drawing::Point(105, 17);
			this->combBackup->MaxDropDownItems = 9;
			this->combBackup->Name = L"combBackup";
			this->combBackup->Size = System::Drawing::Size(113, 20);
			this->combBackup->TabIndex = 5;
			this->combBackup->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::combBackup_SelectedIndexChanged);
			// 
			// tboxBackupOther
			// 
			this->tboxBackupOther->Enabled = false;
			this->tboxBackupOther->Location = System::Drawing::Point(224, 17);
			this->tboxBackupOther->MaxLength = 20;
			this->tboxBackupOther->Name = L"tboxBackupOther";
			this->tboxBackupOther->Size = System::Drawing::Size(125, 19);
			this->tboxBackupOther->TabIndex = 6;
			// 
			// gboxCRC
			// 
			this->gboxCRC->Controls->Add(this->labRomCRC);
			this->gboxCRC->Controls->Add(this->labHeaderCRC);
			this->gboxCRC->Controls->Add(this->tboxHeaderCRC);
			this->gboxCRC->Controls->Add(this->tboxWholeCRC);
			this->gboxCRC->Location = System::Drawing::Point(26, 285);
			this->gboxCRC->Name = L"gboxCRC";
			this->gboxCRC->Size = System::Drawing::Size(285, 49);
			this->gboxCRC->TabIndex = 5;
			this->gboxCRC->TabStop = false;
			this->gboxCRC->Text = L"CRC";
			// 
			// labRomCRC
			// 
			this->labRomCRC->AutoSize = true;
			this->labRomCRC->Location = System::Drawing::Point(146, 20);
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
			this->tboxHeaderCRC->Location = System::Drawing::Point(78, 17);
			this->tboxHeaderCRC->Name = L"tboxHeaderCRC";
			this->tboxHeaderCRC->ReadOnly = true;
			this->tboxHeaderCRC->Size = System::Drawing::Size(55, 19);
			this->tboxHeaderCRC->TabIndex = 1;
			// 
			// tboxWholeCRC
			// 
			this->tboxWholeCRC->Location = System::Drawing::Point(215, 17);
			this->tboxWholeCRC->Name = L"tboxWholeCRC";
			this->tboxWholeCRC->ReadOnly = true;
			this->tboxWholeCRC->Size = System::Drawing::Size(55, 19);
			this->tboxWholeCRC->TabIndex = 0;
			// 
			// labCaption
			// 
			this->labCaption->AutoSize = true;
			this->labCaption->Location = System::Drawing::Point(379, 245);
			this->labCaption->Name = L"labCaption";
			this->labCaption->Size = System::Drawing::Size(241, 12);
			this->labCaption->TabIndex = 8;
			this->labCaption->Text = L"���l�� - ���̑��A������������΂��L����������";
			// 
			// tboxCaption
			// 
			this->tboxCaption->Location = System::Drawing::Point(378, 263);
			this->tboxCaption->MaxLength = 300;
			this->tboxCaption->Multiline = true;
			this->tboxCaption->Name = L"tboxCaption";
			this->tboxCaption->Size = System::Drawing::Size(349, 74);
			this->tboxCaption->TabIndex = 8;
			// 
			// labOFLC
			// 
			this->labOFLC->AutoSize = true;
			this->labOFLC->Location = System::Drawing::Point(101, 234);
			this->labOFLC->Name = L"labOFLC";
			this->labOFLC->Size = System::Drawing::Size(34, 12);
			this->labOFLC->TabIndex = 33;
			this->labOFLC->Text = L"OFLC";
			// 
			// labPEGI_BBFC
			// 
			this->labPEGI_BBFC->AutoSize = true;
			this->labPEGI_BBFC->Location = System::Drawing::Point(12, 204);
			this->labPEGI_BBFC->Name = L"labPEGI_BBFC";
			this->labPEGI_BBFC->Size = System::Drawing::Size(134, 12);
			this->labPEGI_BBFC->TabIndex = 32;
			this->labPEGI_BBFC->Text = L"PEGI(General) and BBFC";
			// 
			// labPEGI_PRT
			// 
			this->labPEGI_PRT->AutoSize = true;
			this->labPEGI_PRT->Location = System::Drawing::Point(69, 182);
			this->labPEGI_PRT->Name = L"labPEGI_PRT";
			this->labPEGI_PRT->Size = System::Drawing::Size(76, 12);
			this->labPEGI_PRT->TabIndex = 31;
			this->labPEGI_PRT->Text = L"PEGI Portugal";
			// 
			// labPEGI
			// 
			this->labPEGI->AutoSize = true;
			this->labPEGI->Location = System::Drawing::Point(69, 156);
			this->labPEGI->Name = L"labPEGI";
			this->labPEGI->Size = System::Drawing::Size(77, 12);
			this->labPEGI->TabIndex = 30;
			this->labPEGI->Text = L"PEGI(General)";
			// 
			// labUSK
			// 
			this->labUSK->AutoSize = true;
			this->labUSK->Location = System::Drawing::Point(102, 130);
			this->labUSK->Name = L"labUSK";
			this->labUSK->Size = System::Drawing::Size(27, 12);
			this->labUSK->TabIndex = 29;
			this->labUSK->Text = L"USK";
			// 
			// labESRB
			// 
			this->labESRB->AutoSize = true;
			this->labESRB->Location = System::Drawing::Point(102, 104);
			this->labESRB->Name = L"labESRB";
			this->labESRB->Size = System::Drawing::Size(35, 12);
			this->labESRB->TabIndex = 28;
			this->labESRB->Text = L"ESRB";
			// 
			// labCERO
			// 
			this->labCERO->AutoSize = true;
			this->labCERO->Location = System::Drawing::Point(101, 78);
			this->labCERO->Name = L"labCERO";
			this->labCERO->Size = System::Drawing::Size(36, 12);
			this->labCERO->TabIndex = 27;
			this->labCERO->Text = L"CERO";
			// 
			// combOFLC
			// 
			this->combOFLC->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combOFLC->FormattingEnabled = true;
			this->combOFLC->Items->AddRange(gcnew cli::array< System::Object^  >(5) {L"G", L"PG", L"M", L"MA15+", L"�R����"});
			this->combOFLC->Location = System::Drawing::Point(152, 232);
			this->combOFLC->Name = L"combOFLC";
			this->combOFLC->Size = System::Drawing::Size(204, 20);
			this->combOFLC->TabIndex = 18;
			// 
			// combPEGI_BBFC
			// 
			this->combPEGI_BBFC->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combPEGI_BBFC->FormattingEnabled = true;
			this->combPEGI_BBFC->Items->AddRange(gcnew cli::array< System::Object^  >(10) {L"�N����Ȃ�(�S�N��)", L"3�Έȏ�", L"4�Έȏ㐄��", L"7�Έȏ�", 
				L"8�Έȏ㐄��", L"12�Έȏ�", L"15�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"�R����"});
			this->combPEGI_BBFC->Location = System::Drawing::Point(152, 205);
			this->combPEGI_BBFC->MaxDropDownItems = 10;
			this->combPEGI_BBFC->Name = L"combPEGI_BBFC";
			this->combPEGI_BBFC->Size = System::Drawing::Size(204, 20);
			this->combPEGI_BBFC->TabIndex = 15;
			// 
			// combPEGI_PRT
			// 
			this->combPEGI_PRT->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combPEGI_PRT->FormattingEnabled = true;
			this->combPEGI_PRT->Items->AddRange(gcnew cli::array< System::Object^  >(7) {L"�N����Ȃ�(�S�N��)", L"4�Έȏ�", L"6�Έȏ�", L"12�Έȏ�", 
				L"16�Έȏ�", L"18�Έȏ�", L"�R����"});
			this->combPEGI_PRT->Location = System::Drawing::Point(152, 179);
			this->combPEGI_PRT->Name = L"combPEGI_PRT";
			this->combPEGI_PRT->Size = System::Drawing::Size(204, 20);
			this->combPEGI_PRT->TabIndex = 12;
			// 
			// combPEGI
			// 
			this->combPEGI->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combPEGI->FormattingEnabled = true;
			this->combPEGI->Items->AddRange(gcnew cli::array< System::Object^  >(7) {L"�N����Ȃ�(�S�N��)", L"3�Έȏ�", L"7�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", 
				L"18�Έȏ�", L"�R����"});
			this->combPEGI->Location = System::Drawing::Point(152, 153);
			this->combPEGI->Name = L"combPEGI";
			this->combPEGI->Size = System::Drawing::Size(204, 20);
			this->combPEGI->TabIndex = 9;
			// 
			// combUSK
			// 
			this->combUSK->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combUSK->FormattingEnabled = true;
			this->combUSK->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"�N����Ȃ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"���N�ɂ͕s�K��", 
				L"�R����"});
			this->combUSK->Location = System::Drawing::Point(152, 127);
			this->combUSK->Name = L"combUSK";
			this->combUSK->Size = System::Drawing::Size(204, 20);
			this->combUSK->TabIndex = 6;
			// 
			// combESRB
			// 
			this->combESRB->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combESRB->FormattingEnabled = true;
			this->combESRB->Items->AddRange(gcnew cli::array< System::Object^  >(7) {L"�N����Ȃ�(�S�N��)", L"EC (3�Έȏ�)", L"E (6�Έȏ�)", L"E10+ (10�Έȏ�)", 
				L"T (13�Έȏ�)", L"M (17�Έȏ�)", L"�R����"});
			this->combESRB->Location = System::Drawing::Point(152, 101);
			this->combESRB->Name = L"combESRB";
			this->combESRB->Size = System::Drawing::Size(204, 20);
			this->combESRB->TabIndex = 3;
			// 
			// combCERO
			// 
			this->combCERO->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combCERO->FormattingEnabled = true;
			this->combCERO->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"A (�S�N��)", L"B (12�Έȏ�)", L"C (15�Έȏ�)", L"D (17�Έȏ�)", 
				L"Z (18�Έȏ�)", L"�R����"});
			this->combCERO->Location = System::Drawing::Point(152, 75);
			this->combCERO->Name = L"combCERO";
			this->combCERO->Size = System::Drawing::Size(204, 20);
			this->combCERO->TabIndex = 0;
			// 
			// labParentalRating
			// 
			this->labParentalRating->AutoSize = true;
			this->labParentalRating->Location = System::Drawing::Point(201, 60);
			this->labParentalRating->Name = L"labParentalRating";
			this->labParentalRating->Size = System::Drawing::Size(82, 12);
			this->labParentalRating->TabIndex = 2;
			this->labParentalRating->Text = L"���[�e�B���O���";
			// 
			// labRegion
			// 
			this->labRegion->AutoSize = true;
			this->labRegion->Location = System::Drawing::Point(98, 24);
			this->labRegion->Name = L"labRegion";
			this->labRegion->Size = System::Drawing::Size(47, 12);
			this->labRegion->TabIndex = 37;
			this->labRegion->Text = L"���[�W����";
			// 
			// cboxIsEULA
			// 
			this->cboxIsEULA->AutoSize = true;
			this->cboxIsEULA->Location = System::Drawing::Point(8, 20);
			this->cboxIsEULA->Name = L"cboxIsEULA";
			this->cboxIsEULA->Size = System::Drawing::Size(157, 16);
			this->cboxIsEULA->TabIndex = 0;
			this->cboxIsEULA->Text = L"EULA�ւ̓��ӂ�K�v�Ƃ���";
			this->cboxIsEULA->UseVisualStyleBackColor = true;
			// 
			// combRegion
			// 
			this->combRegion->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->combRegion->FormattingEnabled = true;
			this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5) {L"���{�̂�", L"�č��̂�", L"���B�̂�", L"���B�̂�", L"���B����э��B"});
			this->combRegion->Location = System::Drawing::Point(152, 21);
			this->combRegion->Name = L"combRegion";
			this->combRegion->Size = System::Drawing::Size(204, 20);
			this->combRegion->TabIndex = 36;
			this->combRegion->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::combRegion_SelectedIndexChanged);
			// 
			// cboxIsInputPerson2
			// 
			this->cboxIsInputPerson2->AutoSize = true;
			this->cboxIsInputPerson2->Location = System::Drawing::Point(392, 66);
			this->cboxIsInputPerson2->Name = L"cboxIsInputPerson2";
			this->cboxIsInputPerson2->Size = System::Drawing::Size(150, 16);
			this->cboxIsInputPerson2->TabIndex = 7;
			this->cboxIsInputPerson2->Text = L"�S����2����͂���(�C��)";
			this->cboxIsInputPerson2->UseVisualStyleBackColor = true;
			this->cboxIsInputPerson2->CheckedChanged += gcnew System::EventHandler(this, &Form1::cboxIsInputPerson2_CheckedChanged);
			// 
			// gboxPerson2
			// 
			this->gboxPerson2->Controls->Add(this->labArbit4);
			this->gboxPerson2->Controls->Add(this->labArbit3);
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
			this->gboxPerson2->Location = System::Drawing::Point(377, 88);
			this->gboxPerson2->Name = L"gboxPerson2";
			this->gboxPerson2->Size = System::Drawing::Size(347, 234);
			this->gboxPerson2->TabIndex = 14;
			this->gboxPerson2->TabStop = false;
			this->gboxPerson2->Text = L"�S����2";
			// 
			// labArbit4
			// 
			this->labArbit4->AutoSize = true;
			this->labArbit4->Location = System::Drawing::Point(230, 207);
			this->labArbit4->Name = L"labArbit4";
			this->labArbit4->Size = System::Drawing::Size(37, 12);
			this->labArbit4->TabIndex = 15;
			this->labArbit4->Text = L"(�C��)";
			// 
			// labArbit3
			// 
			this->labArbit3->AutoSize = true;
			this->labArbit3->Location = System::Drawing::Point(230, 156);
			this->labArbit3->Name = L"labArbit3";
			this->labArbit3->Size = System::Drawing::Size(37, 12);
			this->labArbit3->TabIndex = 13;
			this->labArbit3->Text = L"(�C��)";
			// 
			// labNTSC2Sur
			// 
			this->labNTSC2Sur->AutoSize = true;
			this->labNTSC2Sur->Location = System::Drawing::Point(18, 214);
			this->labNTSC2Sur->Name = L"labNTSC2Sur";
			this->labNTSC2Sur->Size = System::Drawing::Size(44, 12);
			this->labNTSC2Sur->TabIndex = 12;
			this->labNTSC2Sur->Text = L"User ID";
			// 
			// tboxNTSC2
			// 
			this->tboxNTSC2->Location = System::Drawing::Point(66, 200);
			this->tboxNTSC2->MaxLength = 50;
			this->tboxNTSC2->Name = L"tboxNTSC2";
			this->tboxNTSC2->Size = System::Drawing::Size(158, 19);
			this->tboxNTSC2->TabIndex = 11;
			// 
			// labFax2
			// 
			this->labFax2->AutoSize = true;
			this->labFax2->Location = System::Drawing::Point(23, 152);
			this->labFax2->Name = L"labFax2";
			this->labFax2->Size = System::Drawing::Size(27, 12);
			this->labFax2->TabIndex = 4;
			this->labFax2->Text = L"FAX";
			// 
			// labNTSC2Pre
			// 
			this->labNTSC2Pre->AutoSize = true;
			this->labNTSC2Pre->Location = System::Drawing::Point(18, 200);
			this->labNTSC2Pre->Name = L"labNTSC2Pre";
			this->labNTSC2Pre->Size = System::Drawing::Size(35, 12);
			this->labNTSC2Pre->TabIndex = 11;
			this->labNTSC2Pre->Text = L"NTSC";
			// 
			// tboxFax2
			// 
			this->tboxFax2->Location = System::Drawing::Point(66, 149);
			this->tboxFax2->MaxLength = 20;
			this->tboxFax2->Name = L"tboxFax2";
			this->tboxFax2->Size = System::Drawing::Size(158, 19);
			this->tboxFax2->TabIndex = 13;
			// 
			// tboxMail2
			// 
			this->tboxMail2->Location = System::Drawing::Point(66, 175);
			this->tboxMail2->MaxLength = 50;
			this->tboxMail2->Name = L"tboxMail2";
			this->tboxMail2->Size = System::Drawing::Size(268, 19);
			this->tboxMail2->TabIndex = 14;
			// 
			// tboxTel2
			// 
			this->tboxTel2->Location = System::Drawing::Point(66, 125);
			this->tboxTel2->MaxLength = 20;
			this->tboxTel2->Name = L"tboxTel2";
			this->tboxTel2->Size = System::Drawing::Size(158, 19);
			this->tboxTel2->TabIndex = 12;
			// 
			// tboxFurigana2
			// 
			this->tboxFurigana2->Location = System::Drawing::Point(66, 100);
			this->tboxFurigana2->MaxLength = 50;
			this->tboxFurigana2->Name = L"tboxFurigana2";
			this->tboxFurigana2->Size = System::Drawing::Size(268, 19);
			this->tboxFurigana2->TabIndex = 11;
			// 
			// tboxPerson2
			// 
			this->tboxPerson2->Location = System::Drawing::Point(66, 75);
			this->tboxPerson2->MaxLength = 50;
			this->tboxPerson2->Name = L"tboxPerson2";
			this->tboxPerson2->Size = System::Drawing::Size(268, 19);
			this->tboxPerson2->TabIndex = 10;
			// 
			// tboxDepart2
			// 
			this->tboxDepart2->Location = System::Drawing::Point(66, 50);
			this->tboxDepart2->MaxLength = 50;
			this->tboxDepart2->Name = L"tboxDepart2";
			this->tboxDepart2->Size = System::Drawing::Size(268, 19);
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
			this->tboxCompany2->MaxLength = 50;
			this->tboxCompany2->Name = L"tboxCompany2";
			this->tboxCompany2->Size = System::Drawing::Size(268, 19);
			this->tboxCompany2->TabIndex = 8;
			// 
			// labMail2
			// 
			this->labMail2->AutoSize = true;
			this->labMail2->Location = System::Drawing::Point(18, 178);
			this->labMail2->Name = L"labMail2";
			this->labMail2->Size = System::Drawing::Size(38, 12);
			this->labMail2->TabIndex = 5;
			this->labMail2->Text = L"e-mail";
			// 
			// labTel2
			// 
			this->labTel2->AutoSize = true;
			this->labTel2->Location = System::Drawing::Point(23, 128);
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
			this->gboxPerson1->Controls->Add(this->labArbit2);
			this->gboxPerson1->Controls->Add(this->labArbit1);
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
			this->gboxPerson1->Location = System::Drawing::Point(17, 88);
			this->gboxPerson1->Name = L"gboxPerson1";
			this->gboxPerson1->Size = System::Drawing::Size(344, 234);
			this->gboxPerson1->TabIndex = 8;
			this->gboxPerson1->TabStop = false;
			this->gboxPerson1->Text = L"�S����1";
			// 
			// labArbit2
			// 
			this->labArbit2->AutoSize = true;
			this->labArbit2->Location = System::Drawing::Point(219, 207);
			this->labArbit2->Name = L"labArbit2";
			this->labArbit2->Size = System::Drawing::Size(37, 12);
			this->labArbit2->TabIndex = 12;
			this->labArbit2->Text = L"(�C��)";
			// 
			// labArbit1
			// 
			this->labArbit1->AutoSize = true;
			this->labArbit1->Location = System::Drawing::Point(219, 156);
			this->labArbit1->Name = L"labArbit1";
			this->labArbit1->Size = System::Drawing::Size(37, 12);
			this->labArbit1->TabIndex = 11;
			this->labArbit1->Text = L"(�C��)";
			// 
			// labNTSC1Sur
			// 
			this->labNTSC1Sur->AutoSize = true;
			this->labNTSC1Sur->Location = System::Drawing::Point(18, 214);
			this->labNTSC1Sur->Name = L"labNTSC1Sur";
			this->labNTSC1Sur->Size = System::Drawing::Size(44, 12);
			this->labNTSC1Sur->TabIndex = 10;
			this->labNTSC1Sur->Text = L"User ID";
			// 
			// labFax1
			// 
			this->labFax1->AutoSize = true;
			this->labFax1->Location = System::Drawing::Point(23, 152);
			this->labFax1->Name = L"labFax1";
			this->labFax1->Size = System::Drawing::Size(27, 12);
			this->labFax1->TabIndex = 4;
			this->labFax1->Text = L"FAX";
			// 
			// labNTSC1Pre
			// 
			this->labNTSC1Pre->AutoSize = true;
			this->labNTSC1Pre->Location = System::Drawing::Point(18, 200);
			this->labNTSC1Pre->Name = L"labNTSC1Pre";
			this->labNTSC1Pre->Size = System::Drawing::Size(35, 12);
			this->labNTSC1Pre->TabIndex = 9;
			this->labNTSC1Pre->Text = L"NTSC";
			// 
			// tboxNTSC1
			// 
			this->tboxNTSC1->Location = System::Drawing::Point(66, 200);
			this->tboxNTSC1->MaxLength = 30;
			this->tboxNTSC1->Name = L"tboxNTSC1";
			this->tboxNTSC1->Size = System::Drawing::Size(147, 19);
			this->tboxNTSC1->TabIndex = 8;
			// 
			// tboxFax1
			// 
			this->tboxFax1->Location = System::Drawing::Point(66, 149);
			this->tboxFax1->MaxLength = 15;
			this->tboxFax1->Name = L"tboxFax1";
			this->tboxFax1->Size = System::Drawing::Size(147, 19);
			this->tboxFax1->TabIndex = 5;
			// 
			// tboxMail1
			// 
			this->tboxMail1->Location = System::Drawing::Point(66, 175);
			this->tboxMail1->MaxLength = 50;
			this->tboxMail1->Name = L"tboxMail1";
			this->tboxMail1->Size = System::Drawing::Size(261, 19);
			this->tboxMail1->TabIndex = 6;
			// 
			// tboxTel1
			// 
			this->tboxTel1->Location = System::Drawing::Point(66, 125);
			this->tboxTel1->MaxLength = 15;
			this->tboxTel1->Name = L"tboxTel1";
			this->tboxTel1->Size = System::Drawing::Size(147, 19);
			this->tboxTel1->TabIndex = 4;
			// 
			// tboxFurigana1
			// 
			this->tboxFurigana1->Location = System::Drawing::Point(66, 100);
			this->tboxFurigana1->MaxLength = 15;
			this->tboxFurigana1->Name = L"tboxFurigana1";
			this->tboxFurigana1->Size = System::Drawing::Size(261, 19);
			this->tboxFurigana1->TabIndex = 3;
			// 
			// tboxPerson1
			// 
			this->tboxPerson1->Location = System::Drawing::Point(66, 75);
			this->tboxPerson1->MaxLength = 15;
			this->tboxPerson1->Name = L"tboxPerson1";
			this->tboxPerson1->Size = System::Drawing::Size(261, 19);
			this->tboxPerson1->TabIndex = 2;
			// 
			// tboxDepart1
			// 
			this->tboxDepart1->Location = System::Drawing::Point(66, 50);
			this->tboxDepart1->MaxLength = 25;
			this->tboxDepart1->Name = L"tboxDepart1";
			this->tboxDepart1->Size = System::Drawing::Size(261, 19);
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
			this->tboxCompany1->MaxLength = 25;
			this->tboxCompany1->Name = L"tboxCompany1";
			this->tboxCompany1->Size = System::Drawing::Size(261, 19);
			this->tboxCompany1->TabIndex = 0;
			// 
			// labMail1
			// 
			this->labMail1->AutoSize = true;
			this->labMail1->Location = System::Drawing::Point(18, 178);
			this->labMail1->Name = L"labMail1";
			this->labMail1->Size = System::Drawing::Size(38, 12);
			this->labMail1->TabIndex = 5;
			this->labMail1->Text = L"e-mail";
			// 
			// labTel1
			// 
			this->labTel1->AutoSize = true;
			this->labTel1->Location = System::Drawing::Point(23, 128);
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
			this->tboxProductCode2->Location = System::Drawing::Point(175, 81);
			this->tboxProductCode2->MaxLength = 4;
			this->tboxProductCode2->Name = L"tboxProductCode2";
			this->tboxProductCode2->Size = System::Drawing::Size(45, 19);
			this->tboxProductCode2->TabIndex = 2;
			// 
			// tboxProductCode1
			// 
			this->tboxProductCode1->Location = System::Drawing::Point(140, 81);
			this->tboxProductCode1->MaxLength = 1;
			this->tboxProductCode1->Name = L"tboxProductCode1";
			this->tboxProductCode1->Size = System::Drawing::Size(18, 19);
			this->tboxProductCode1->TabIndex = 1;
			// 
			// tboxProductName
			// 
			this->tboxProductName->ImeMode = System::Windows::Forms::ImeMode::NoControl;
			this->tboxProductName->Location = System::Drawing::Point(106, 53);
			this->tboxProductName->MaxLength = 30;
			this->tboxProductName->Name = L"tboxProductName";
			this->tboxProductName->Size = System::Drawing::Size(256, 19);
			this->tboxProductName->TabIndex = 0;
			// 
			// labProductCode2
			// 
			this->labProductCode2->AutoSize = true;
			this->labProductCode2->Location = System::Drawing::Point(164, 84);
			this->labProductCode2->Name = L"labProductCode2";
			this->labProductCode2->Size = System::Drawing::Size(11, 12);
			this->labProductCode2->TabIndex = 33;
			this->labProductCode2->Text = L"-";
			// 
			// labProductCode1
			// 
			this->labProductCode1->AutoSize = true;
			this->labProductCode1->Location = System::Drawing::Point(104, 84);
			this->labProductCode1->Name = L"labProductCode1";
			this->labProductCode1->Size = System::Drawing::Size(37, 12);
			this->labProductCode1->TabIndex = 32;
			this->labProductCode1->Text = L"TWL -";
			// 
			// dateSubmit
			// 
			this->dateSubmit->Format = System::Windows::Forms::DateTimePickerFormat::Short;
			this->dateSubmit->Location = System::Drawing::Point(106, 140);
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
			this->dateRelease->Location = System::Drawing::Point(106, 111);
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
			this->gboxUsage->Location = System::Drawing::Point(16, 258);
			this->gboxUsage->Name = L"gboxUsage";
			this->gboxUsage->Size = System::Drawing::Size(346, 79);
			this->gboxUsage->TabIndex = 6;
			this->gboxUsage->TabStop = false;
			this->gboxUsage->Text = L"�p�r";
			// 
			// tboxUsageOther
			// 
			this->tboxUsageOther->Enabled = false;
			this->tboxUsageOther->Location = System::Drawing::Point(71, 47);
			this->tboxUsageOther->MaxLength = 25;
			this->tboxUsageOther->Name = L"tboxUsageOther";
			this->tboxUsageOther->Size = System::Drawing::Size(250, 19);
			this->tboxUsageOther->TabIndex = 4;
			// 
			// rUsageOther
			// 
			this->rUsageOther->AutoSize = true;
			this->rUsageOther->Location = System::Drawing::Point(6, 48);
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
			this->rUsageDst->Location = System::Drawing::Point(198, 18);
			this->rUsageDst->Name = L"rUsageDst";
			this->rUsageDst->Size = System::Drawing::Size(87, 16);
			this->rUsageDst->TabIndex = 2;
			this->rUsageDst->Text = L"�f�[�^�z�M�p";
			this->rUsageDst->UseVisualStyleBackColor = true;
			// 
			// rUsageSample
			// 
			this->rUsageSample->AutoSize = true;
			this->rUsageSample->Location = System::Drawing::Point(104, 18);
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
			this->rUsageSale->Location = System::Drawing::Point(6, 18);
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
			this->gboxSubmitWay->Location = System::Drawing::Point(16, 207);
			this->gboxSubmitWay->Name = L"gboxSubmitWay";
			this->gboxSubmitWay->Size = System::Drawing::Size(155, 45);
			this->gboxSubmitWay->TabIndex = 5;
			this->gboxSubmitWay->TabStop = false;
			this->gboxSubmitWay->Text = L"��o���@";
			// 
			// rSubmitHand
			// 
			this->rSubmitHand->AutoSize = true;
			this->rSubmitHand->Location = System::Drawing::Point(82, 18);
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
			this->labSubmiteDate->Location = System::Drawing::Point(14, 143);
			this->labSubmiteDate->Name = L"labSubmiteDate";
			this->labSubmiteDate->Size = System::Drawing::Size(65, 12);
			this->labSubmiteDate->TabIndex = 11;
			this->labSubmiteDate->Text = L"���ޒ�o��";
			// 
			// labReleaseDate
			// 
			this->labReleaseDate->AutoSize = true;
			this->labReleaseDate->Location = System::Drawing::Point(15, 114);
			this->labReleaseDate->Name = L"labReleaseDate";
			this->labReleaseDate->Size = System::Drawing::Size(65, 12);
			this->labReleaseDate->TabIndex = 7;
			this->labReleaseDate->Text = L"�����\���";
			// 
			// labProductCode
			// 
			this->labProductCode->AutoSize = true;
			this->labProductCode->Location = System::Drawing::Point(22, 84);
			this->labProductCode->Name = L"labProductCode";
			this->labProductCode->Size = System::Drawing::Size(56, 12);
			this->labProductCode->TabIndex = 6;
			this->labProductCode->Text = L"���i�R�[�h";
			// 
			// labProductName
			// 
			this->labProductName->AutoSize = true;
			this->labProductName->Location = System::Drawing::Point(22, 56);
			this->labProductName->Name = L"labProductName";
			this->labProductName->Size = System::Drawing::Size(41, 12);
			this->labProductName->TabIndex = 5;
			this->labProductName->Text = L"���i��";
			// 
			// labCapSubmitVer
			// 
			this->labCapSubmitVer->AutoSize = true;
			this->labCapSubmitVer->Location = System::Drawing::Point(104, 194);
			this->labCapSubmitVer->Name = L"labCapSubmitVer";
			this->labCapSubmitVer->Size = System::Drawing::Size(233, 12);
			this->labCapSubmitVer->TabIndex = 26;
			this->labCapSubmitVer->Text = L"* ���}�X�^�[�o�[�W�������オ��ƍĂ�0����J�E���g";
			// 
			// numSubmitVersion
			// 
			this->numSubmitVersion->Location = System::Drawing::Point(106, 172);
			this->numSubmitVersion->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			this->numSubmitVersion->Name = L"numSubmitVersion";
			this->numSubmitVersion->Size = System::Drawing::Size(38, 19);
			this->numSubmitVersion->TabIndex = 9;
			// 
			// labSubmitVer
			// 
			this->labSubmitVer->AutoSize = true;
			this->labSubmitVer->Location = System::Drawing::Point(15, 174);
			this->labSubmitVer->Name = L"labSubmitVer";
			this->labSubmitVer->Size = System::Drawing::Size(74, 12);
			this->labSubmitVer->TabIndex = 24;
			this->labSubmitVer->Text = L"��o�o�[�W����";
			// 
			// labMultiForeign1
			// 
			this->labMultiForeign1->AutoSize = true;
			this->labMultiForeign1->Location = System::Drawing::Point(217, 96);
			this->labMultiForeign1->Name = L"labMultiForeign1";
			this->labMultiForeign1->Size = System::Drawing::Size(101, 12);
			this->labMultiForeign1->TabIndex = 44;
			this->labMultiForeign1->Text = L"(��������ꍇ�̂�)";
			// 
			// tboxProductCode2Foreign3
			// 
			this->tboxProductCode2Foreign3->Enabled = false;
			this->tboxProductCode2Foreign3->Location = System::Drawing::Point(165, 118);
			this->tboxProductCode2Foreign3->MaxLength = 4;
			this->tboxProductCode2Foreign3->Name = L"tboxProductCode2Foreign3";
			this->tboxProductCode2Foreign3->Size = System::Drawing::Size(46, 19);
			this->tboxProductCode2Foreign3->TabIndex = 43;
			// 
			// tboxProductCode2Foreign2
			// 
			this->tboxProductCode2Foreign2->Enabled = false;
			this->tboxProductCode2Foreign2->Location = System::Drawing::Point(165, 93);
			this->tboxProductCode2Foreign2->MaxLength = 4;
			this->tboxProductCode2Foreign2->Name = L"tboxProductCode2Foreign2";
			this->tboxProductCode2Foreign2->Size = System::Drawing::Size(46, 19);
			this->tboxProductCode2Foreign2->TabIndex = 42;
			// 
			// labProductCode2Foreign
			// 
			this->labProductCode2Foreign->AutoSize = true;
			this->labProductCode2Foreign->Location = System::Drawing::Point(152, 74);
			this->labProductCode2Foreign->Name = L"labProductCode2Foreign";
			this->labProductCode2Foreign->Size = System::Drawing::Size(11, 12);
			this->labProductCode2Foreign->TabIndex = 41;
			this->labProductCode2Foreign->Text = L"-";
			// 
			// cboxReleaseForeign
			// 
			this->cboxReleaseForeign->AutoSize = true;
			this->cboxReleaseForeign->Location = System::Drawing::Point(17, 18);
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
			this->labProductNameForeign->Location = System::Drawing::Point(15, 43);
			this->labProductNameForeign->Name = L"labProductNameForeign";
			this->labProductNameForeign->Size = System::Drawing::Size(41, 12);
			this->labProductNameForeign->TabIndex = 37;
			this->labProductNameForeign->Text = L"���i��";
			// 
			// tboxProductNameForeign
			// 
			this->tboxProductNameForeign->Enabled = false;
			this->tboxProductNameForeign->Location = System::Drawing::Point(92, 40);
			this->tboxProductNameForeign->MaxLength = 30;
			this->tboxProductNameForeign->Name = L"tboxProductNameForeign";
			this->tboxProductNameForeign->Size = System::Drawing::Size(257, 19);
			this->tboxProductNameForeign->TabIndex = 12;
			// 
			// labProductCode1Foreign
			// 
			this->labProductCode1Foreign->AutoSize = true;
			this->labProductCode1Foreign->Location = System::Drawing::Point(90, 74);
			this->labProductCode1Foreign->Name = L"labProductCode1Foreign";
			this->labProductCode1Foreign->Size = System::Drawing::Size(37, 12);
			this->labProductCode1Foreign->TabIndex = 40;
			this->labProductCode1Foreign->Text = L"TWL -";
			// 
			// tboxProductCode1Foreign
			// 
			this->tboxProductCode1Foreign->Enabled = false;
			this->tboxProductCode1Foreign->Location = System::Drawing::Point(129, 69);
			this->tboxProductCode1Foreign->MaxLength = 1;
			this->tboxProductCode1Foreign->Name = L"tboxProductCode1Foreign";
			this->tboxProductCode1Foreign->Size = System::Drawing::Size(18, 19);
			this->tboxProductCode1Foreign->TabIndex = 13;
			// 
			// labProductCodeForeign
			// 
			this->labProductCodeForeign->AutoSize = true;
			this->labProductCodeForeign->Location = System::Drawing::Point(15, 74);
			this->labProductCodeForeign->Name = L"labProductCodeForeign";
			this->labProductCodeForeign->Size = System::Drawing::Size(56, 12);
			this->labProductCodeForeign->TabIndex = 38;
			this->labProductCodeForeign->Text = L"���i�R�[�h";
			// 
			// tboxProductCode2Foreign1
			// 
			this->tboxProductCode2Foreign1->Enabled = false;
			this->tboxProductCode2Foreign1->Location = System::Drawing::Point(165, 68);
			this->tboxProductCode2Foreign1->MaxLength = 4;
			this->tboxProductCode2Foreign1->Name = L"tboxProductCode2Foreign1";
			this->tboxProductCode2Foreign1->Size = System::Drawing::Size(46, 19);
			this->tboxProductCode2Foreign1->TabIndex = 14;
			// 
			// gboxShared2Size
			// 
			this->gboxShared2Size->Controls->Add(this->labShared2Size5);
			this->gboxShared2Size->Controls->Add(this->labShared2Size4);
			this->gboxShared2Size->Controls->Add(this->labShared2Size3);
			this->gboxShared2Size->Controls->Add(this->labShared2Size2);
			this->gboxShared2Size->Controls->Add(this->labShared2Size1);
			this->gboxShared2Size->Controls->Add(this->labShared2Size0);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size5);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size4);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size3);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size2);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size1);
			this->gboxShared2Size->Controls->Add(this->tboxShared2Size0);
			this->gboxShared2Size->Controls->Add(this->cboxIsShared2);
			this->gboxShared2Size->Location = System::Drawing::Point(547, 54);
			this->gboxShared2Size->Name = L"gboxShared2Size";
			this->gboxShared2Size->Size = System::Drawing::Size(164, 220);
			this->gboxShared2Size->TabIndex = 1;
			this->gboxShared2Size->TabStop = false;
			this->gboxShared2Size->Text = L"Shared2�t�@�C���T�C�Y";
			// 
			// labShared2Size5
			// 
			this->labShared2Size5->AutoSize = true;
			this->labShared2Size5->Location = System::Drawing::Point(7, 180);
			this->labShared2Size5->Name = L"labShared2Size5";
			this->labShared2Size5->Size = System::Drawing::Size(34, 12);
			this->labShared2Size5->TabIndex = 20;
			this->labShared2Size5->Text = L"File 5";
			// 
			// labShared2Size4
			// 
			this->labShared2Size4->AutoSize = true;
			this->labShared2Size4->Location = System::Drawing::Point(6, 155);
			this->labShared2Size4->Name = L"labShared2Size4";
			this->labShared2Size4->Size = System::Drawing::Size(34, 12);
			this->labShared2Size4->TabIndex = 19;
			this->labShared2Size4->Text = L"File 4";
			// 
			// labShared2Size3
			// 
			this->labShared2Size3->AutoSize = true;
			this->labShared2Size3->Location = System::Drawing::Point(7, 130);
			this->labShared2Size3->Name = L"labShared2Size3";
			this->labShared2Size3->Size = System::Drawing::Size(34, 12);
			this->labShared2Size3->TabIndex = 18;
			this->labShared2Size3->Text = L"File 3";
			// 
			// labShared2Size2
			// 
			this->labShared2Size2->AutoSize = true;
			this->labShared2Size2->Location = System::Drawing::Point(7, 105);
			this->labShared2Size2->Name = L"labShared2Size2";
			this->labShared2Size2->Size = System::Drawing::Size(34, 12);
			this->labShared2Size2->TabIndex = 17;
			this->labShared2Size2->Text = L"File 2";
			// 
			// labShared2Size1
			// 
			this->labShared2Size1->AutoSize = true;
			this->labShared2Size1->Location = System::Drawing::Point(7, 80);
			this->labShared2Size1->Name = L"labShared2Size1";
			this->labShared2Size1->Size = System::Drawing::Size(34, 12);
			this->labShared2Size1->TabIndex = 16;
			this->labShared2Size1->Text = L"File 1";
			// 
			// labShared2Size0
			// 
			this->labShared2Size0->AutoSize = true;
			this->labShared2Size0->Location = System::Drawing::Point(7, 55);
			this->labShared2Size0->Name = L"labShared2Size0";
			this->labShared2Size0->Size = System::Drawing::Size(34, 12);
			this->labShared2Size0->TabIndex = 15;
			this->labShared2Size0->Text = L"File 0";
			// 
			// tboxShared2Size5
			// 
			this->tboxShared2Size5->Location = System::Drawing::Point(46, 177);
			this->tboxShared2Size5->Name = L"tboxShared2Size5";
			this->tboxShared2Size5->ReadOnly = true;
			this->tboxShared2Size5->Size = System::Drawing::Size(90, 19);
			this->tboxShared2Size5->TabIndex = 8;
			this->tboxShared2Size5->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size4
			// 
			this->tboxShared2Size4->Location = System::Drawing::Point(47, 152);
			this->tboxShared2Size4->Name = L"tboxShared2Size4";
			this->tboxShared2Size4->ReadOnly = true;
			this->tboxShared2Size4->Size = System::Drawing::Size(89, 19);
			this->tboxShared2Size4->TabIndex = 7;
			this->tboxShared2Size4->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size3
			// 
			this->tboxShared2Size3->Location = System::Drawing::Point(47, 127);
			this->tboxShared2Size3->Name = L"tboxShared2Size3";
			this->tboxShared2Size3->ReadOnly = true;
			this->tboxShared2Size3->Size = System::Drawing::Size(89, 19);
			this->tboxShared2Size3->TabIndex = 6;
			this->tboxShared2Size3->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size2
			// 
			this->tboxShared2Size2->Location = System::Drawing::Point(47, 102);
			this->tboxShared2Size2->Name = L"tboxShared2Size2";
			this->tboxShared2Size2->ReadOnly = true;
			this->tboxShared2Size2->Size = System::Drawing::Size(89, 19);
			this->tboxShared2Size2->TabIndex = 5;
			this->tboxShared2Size2->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size1
			// 
			this->tboxShared2Size1->Location = System::Drawing::Point(47, 77);
			this->tboxShared2Size1->Name = L"tboxShared2Size1";
			this->tboxShared2Size1->ReadOnly = true;
			this->tboxShared2Size1->Size = System::Drawing::Size(89, 19);
			this->tboxShared2Size1->TabIndex = 4;
			this->tboxShared2Size1->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxShared2Size0
			// 
			this->tboxShared2Size0->Location = System::Drawing::Point(47, 52);
			this->tboxShared2Size0->Name = L"tboxShared2Size0";
			this->tboxShared2Size0->ReadOnly = true;
			this->tboxShared2Size0->Size = System::Drawing::Size(89, 19);
			this->tboxShared2Size0->TabIndex = 3;
			this->tboxShared2Size0->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// cboxIsShared2
			// 
			this->cboxIsShared2->AutoSize = true;
			this->cboxIsShared2->Enabled = false;
			this->cboxIsShared2->Location = System::Drawing::Point(8, 25);
			this->cboxIsShared2->Name = L"cboxIsShared2";
			this->cboxIsShared2->Size = System::Drawing::Size(123, 16);
			this->cboxIsShared2->TabIndex = 2;
			this->cboxIsShared2->Text = L"Shared2�t�@�C���g�p";
			this->cboxIsShared2->UseVisualStyleBackColor = true;
			// 
			// labLib
			// 
			this->labLib->AutoSize = true;
			this->labLib->Location = System::Drawing::Point(335, 174);
			this->labLib->Name = L"labLib";
			this->labLib->Size = System::Drawing::Size(70, 12);
			this->labLib->TabIndex = 31;
			this->labLib->Text = L"�g�p���C�u����";
			// 
			// tboxSDK
			// 
			this->tboxSDK->Location = System::Drawing::Point(337, 67);
			this->tboxSDK->Multiline = true;
			this->tboxSDK->Name = L"tboxSDK";
			this->tboxSDK->ReadOnly = true;
			this->tboxSDK->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->tboxSDK->Size = System::Drawing::Size(186, 88);
			this->tboxSDK->TabIndex = 10;
			// 
			// labSDK
			// 
			this->labSDK->AutoSize = true;
			this->labSDK->Location = System::Drawing::Point(335, 52);
			this->labSDK->Name = L"labSDK";
			this->labSDK->Size = System::Drawing::Size(72, 12);
			this->labSDK->TabIndex = 30;
			this->labSDK->Text = L"SDK�o�[�W����";
			// 
			// gboxTWLExInfo
			// 
			this->gboxTWLExInfo->Controls->Add(this->labByte1);
			this->gboxTWLExInfo->Controls->Add(this->labHex4);
			this->gboxTWLExInfo->Controls->Add(this->labHex3);
			this->gboxTWLExInfo->Controls->Add(this->tboxIsCodec);
			this->gboxTWLExInfo->Controls->Add(this->labIsCodec);
			this->gboxTWLExInfo->Controls->Add(this->labNormalRomOffset);
			this->gboxTWLExInfo->Controls->Add(this->tboxNormalRomOffset);
			this->gboxTWLExInfo->Controls->Add(this->labKeyTableRomOffset);
			this->gboxTWLExInfo->Controls->Add(this->tboxPrivateSize);
			this->gboxTWLExInfo->Controls->Add(this->labPrivateSize);
			this->gboxTWLExInfo->Controls->Add(this->tboxKeyTableRomOffset);
			this->gboxTWLExInfo->Controls->Add(this->labPublicSize);
			this->gboxTWLExInfo->Controls->Add(this->tboxPublicSize);
			this->gboxTWLExInfo->Location = System::Drawing::Point(231, 54);
			this->gboxTWLExInfo->Name = L"gboxTWLExInfo";
			this->gboxTWLExInfo->Size = System::Drawing::Size(300, 146);
			this->gboxTWLExInfo->TabIndex = 24;
			this->gboxTWLExInfo->TabStop = false;
			this->gboxTWLExInfo->Text = L"TWL�g�����";
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
			this->labHex4->Location = System::Drawing::Point(261, 46);
			this->labHex4->Name = L"labHex4";
			this->labHex4->Size = System::Drawing::Size(11, 12);
			this->labHex4->TabIndex = 29;
			this->labHex4->Text = L"h";
			// 
			// labHex3
			// 
			this->labHex3->AutoSize = true;
			this->labHex3->Location = System::Drawing::Point(261, 21);
			this->labHex3->Name = L"labHex3";
			this->labHex3->Size = System::Drawing::Size(11, 12);
			this->labHex3->TabIndex = 8;
			this->labHex3->Text = L"h";
			// 
			// tboxIsCodec
			// 
			this->tboxIsCodec->Location = System::Drawing::Point(172, 118);
			this->tboxIsCodec->Name = L"tboxIsCodec";
			this->tboxIsCodec->ReadOnly = true;
			this->tboxIsCodec->Size = System::Drawing::Size(87, 19);
			this->tboxIsCodec->TabIndex = 28;
			this->tboxIsCodec->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// labIsCodec
			// 
			this->labIsCodec->AutoSize = true;
			this->labIsCodec->Location = System::Drawing::Point(55, 121);
			this->labIsCodec->Name = L"labIsCodec";
			this->labIsCodec->Size = System::Drawing::Size(75, 12);
			this->labIsCodec->TabIndex = 27;
			this->labIsCodec->Text = L"CODEC Mode";
			// 
			// labNormalRomOffset
			// 
			this->labNormalRomOffset->AutoSize = true;
			this->labNormalRomOffset->Location = System::Drawing::Point(7, 21);
			this->labNormalRomOffset->Name = L"labNormalRomOffset";
			this->labNormalRomOffset->Size = System::Drawing::Size(155, 12);
			this->labNormalRomOffset->TabIndex = 9;
			this->labNormalRomOffset->Text = L"TWL�m�[�}���̈�ROM�I�t�Z�b�g";
			// 
			// tboxNormalRomOffset
			// 
			this->tboxNormalRomOffset->Location = System::Drawing::Point(172, 18);
			this->tboxNormalRomOffset->Name = L"tboxNormalRomOffset";
			this->tboxNormalRomOffset->ReadOnly = true;
			this->tboxNormalRomOffset->Size = System::Drawing::Size(87, 19);
			this->tboxNormalRomOffset->TabIndex = 8;
			this->tboxNormalRomOffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// labKeyTableRomOffset
			// 
			this->labKeyTableRomOffset->AutoSize = true;
			this->labKeyTableRomOffset->Location = System::Drawing::Point(7, 46);
			this->labKeyTableRomOffset->Name = L"labKeyTableRomOffset";
			this->labKeyTableRomOffset->Size = System::Drawing::Size(142, 12);
			this->labKeyTableRomOffset->TabIndex = 11;
			this->labKeyTableRomOffset->Text = L"TWL��p�̈�ROM�I�t�Z�b�g";
			// 
			// tboxPrivateSize
			// 
			this->tboxPrivateSize->Location = System::Drawing::Point(172, 93);
			this->tboxPrivateSize->Name = L"tboxPrivateSize";
			this->tboxPrivateSize->ReadOnly = true;
			this->tboxPrivateSize->Size = System::Drawing::Size(87, 19);
			this->tboxPrivateSize->TabIndex = 13;
			this->tboxPrivateSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// labPrivateSize
			// 
			this->labPrivateSize->AutoSize = true;
			this->labPrivateSize->Location = System::Drawing::Point(31, 96);
			this->labPrivateSize->Name = L"labPrivateSize";
			this->labPrivateSize->Size = System::Drawing::Size(123, 12);
			this->labPrivateSize->TabIndex = 15;
			this->labPrivateSize->Text = L"Private Save Data Size";
			// 
			// tboxKeyTableRomOffset
			// 
			this->tboxKeyTableRomOffset->Location = System::Drawing::Point(172, 43);
			this->tboxKeyTableRomOffset->Name = L"tboxKeyTableRomOffset";
			this->tboxKeyTableRomOffset->ReadOnly = true;
			this->tboxKeyTableRomOffset->Size = System::Drawing::Size(87, 19);
			this->tboxKeyTableRomOffset->TabIndex = 10;
			this->tboxKeyTableRomOffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// labPublicSize
			// 
			this->labPublicSize->AutoSize = true;
			this->labPublicSize->Location = System::Drawing::Point(31, 71);
			this->labPublicSize->Name = L"labPublicSize";
			this->labPublicSize->Size = System::Drawing::Size(118, 12);
			this->labPublicSize->TabIndex = 14;
			this->labPublicSize->Text = L"Public Save Data Size";
			// 
			// tboxPublicSize
			// 
			this->tboxPublicSize->Location = System::Drawing::Point(172, 68);
			this->tboxPublicSize->Name = L"tboxPublicSize";
			this->tboxPublicSize->ReadOnly = true;
			this->tboxPublicSize->Size = System::Drawing::Size(87, 19);
			this->tboxPublicSize->TabIndex = 12;
			this->tboxPublicSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// cboxIsSubBanner
			// 
			this->cboxIsSubBanner->AutoSize = true;
			this->cboxIsSubBanner->Enabled = false;
			this->cboxIsSubBanner->Location = System::Drawing::Point(14, 62);
			this->cboxIsSubBanner->Name = L"cboxIsSubBanner";
			this->cboxIsSubBanner->Size = System::Drawing::Size(131, 16);
			this->cboxIsSubBanner->TabIndex = 26;
			this->cboxIsSubBanner->Text = L"�T�u�o�i�[�t�@�C���L��";
			this->cboxIsSubBanner->UseVisualStyleBackColor = true;
			// 
			// cboxIsWL
			// 
			this->cboxIsWL->AutoSize = true;
			this->cboxIsWL->Enabled = false;
			this->cboxIsWL->Location = System::Drawing::Point(14, 84);
			this->cboxIsWL->Name = L"cboxIsWL";
			this->cboxIsWL->Size = System::Drawing::Size(155, 16);
			this->cboxIsWL->TabIndex = 25;
			this->cboxIsWL->Text = L"NTR�z���C�g���X�g�����L��";
			this->cboxIsWL->UseVisualStyleBackColor = true;
			// 
			// cboxIsNormalJump
			// 
			this->cboxIsNormalJump->AutoSize = true;
			this->cboxIsNormalJump->Enabled = false;
			this->cboxIsNormalJump->Location = System::Drawing::Point(14, 18);
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
			this->cboxIsTmpJump->Location = System::Drawing::Point(14, 40);
			this->cboxIsTmpJump->Name = L"cboxIsTmpJump";
			this->cboxIsTmpJump->Size = System::Drawing::Size(103, 16);
			this->cboxIsTmpJump->TabIndex = 17;
			this->cboxIsTmpJump->Text = L"tmp�W�����v����";
			this->cboxIsTmpJump->UseVisualStyleBackColor = true;
			// 
			// gboxAccess
			// 
			this->gboxAccess->Controls->Add(this->labAccessOther);
			this->gboxAccess->Controls->Add(this->tboxAccessOther);
			this->gboxAccess->Controls->Add(this->tboxIsGameCardOn);
			this->gboxAccess->Controls->Add(this->labIsGameCardOn);
			this->gboxAccess->Controls->Add(this->cboxIsNAND);
			this->gboxAccess->Controls->Add(this->cboxIsSD);
			this->gboxAccess->Location = System::Drawing::Point(231, 211);
			this->gboxAccess->Name = L"gboxAccess";
			this->gboxAccess->Size = System::Drawing::Size(300, 133);
			this->gboxAccess->TabIndex = 0;
			this->gboxAccess->TabStop = false;
			this->gboxAccess->Text = L"�A�N�Z�X�R���g���[�����";
			// 
			// labAccessOther
			// 
			this->labAccessOther->AutoSize = true;
			this->labAccessOther->Location = System::Drawing::Point(139, 20);
			this->labAccessOther->Name = L"labAccessOther";
			this->labAccessOther->Size = System::Drawing::Size(36, 12);
			this->labAccessOther->TabIndex = 5;
			this->labAccessOther->Text = L"���̑�";
			// 
			// tboxAccessOther
			// 
			this->tboxAccessOther->Location = System::Drawing::Point(141, 36);
			this->tboxAccessOther->Multiline = true;
			this->tboxAccessOther->Name = L"tboxAccessOther";
			this->tboxAccessOther->ReadOnly = true;
			this->tboxAccessOther->Size = System::Drawing::Size(142, 85);
			this->tboxAccessOther->TabIndex = 4;
			// 
			// tboxIsGameCardOn
			// 
			this->tboxIsGameCardOn->Location = System::Drawing::Point(8, 102);
			this->tboxIsGameCardOn->Name = L"tboxIsGameCardOn";
			this->tboxIsGameCardOn->ReadOnly = true;
			this->tboxIsGameCardOn->Size = System::Drawing::Size(122, 19);
			this->tboxIsGameCardOn->TabIndex = 3;
			this->tboxIsGameCardOn->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// labIsGameCardOn
			// 
			this->labIsGameCardOn->AutoSize = true;
			this->labIsGameCardOn->Location = System::Drawing::Point(7, 87);
			this->labIsGameCardOn->Name = L"labIsGameCardOn";
			this->labIsGameCardOn->Size = System::Drawing::Size(87, 12);
			this->labIsGameCardOn->TabIndex = 2;
			this->labIsGameCardOn->Text = L"�Q�[���J�[�h�d��";
			// 
			// cboxIsNAND
			// 
			this->cboxIsNAND->AutoSize = true;
			this->cboxIsNAND->Enabled = false;
			this->cboxIsNAND->Location = System::Drawing::Point(9, 47);
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
			this->cboxIsSD->Location = System::Drawing::Point(9, 24);
			this->cboxIsSD->Name = L"cboxIsSD";
			this->cboxIsSD->Size = System::Drawing::Size(67, 16);
			this->cboxIsSD->TabIndex = 0;
			this->cboxIsSD->Text = L"SD�J�[�h";
			this->cboxIsSD->UseVisualStyleBackColor = true;
			// 
			// gboxTitleID
			// 
			this->gboxTitleID->Controls->Add(this->labAppType);
			this->gboxTitleID->Controls->Add(this->labMedia);
			this->gboxTitleID->Controls->Add(this->tboxAppType);
			this->gboxTitleID->Controls->Add(this->tboxMedia);
			this->gboxTitleID->Controls->Add(this->labHex2);
			this->gboxTitleID->Controls->Add(this->tboxTitleIDLo);
			this->gboxTitleID->Controls->Add(this->labTitleIDLo);
			this->gboxTitleID->Controls->Add(this->labTitleIDHi);
			this->gboxTitleID->Controls->Add(this->tboxTitleIDHi);
			this->gboxTitleID->Controls->Add(this->tboxAppTypeOther);
			this->gboxTitleID->Controls->Add(this->labAppTypeOther);
			this->gboxTitleID->Location = System::Drawing::Point(12, 54);
			this->gboxTitleID->Name = L"gboxTitleID";
			this->gboxTitleID->Size = System::Drawing::Size(198, 174);
			this->gboxTitleID->TabIndex = 23;
			this->gboxTitleID->TabStop = false;
			this->gboxTitleID->Text = L"TitleID";
			// 
			// labAppType
			// 
			this->labAppType->AutoSize = true;
			this->labAppType->Location = System::Drawing::Point(21, 98);
			this->labAppType->Name = L"labAppType";
			this->labAppType->Size = System::Drawing::Size(56, 12);
			this->labAppType->TabIndex = 11;
			this->labAppType->Text = L"App. Type";
			// 
			// labMedia
			// 
			this->labMedia->AutoSize = true;
			this->labMedia->Location = System::Drawing::Point(39, 73);
			this->labMedia->Name = L"labMedia";
			this->labMedia->Size = System::Drawing::Size(35, 12);
			this->labMedia->TabIndex = 10;
			this->labMedia->Text = L"Media";
			// 
			// tboxAppType
			// 
			this->tboxAppType->Location = System::Drawing::Point(83, 95);
			this->tboxAppType->Name = L"tboxAppType";
			this->tboxAppType->ReadOnly = true;
			this->tboxAppType->Size = System::Drawing::Size(71, 19);
			this->tboxAppType->TabIndex = 9;
			this->tboxAppType->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxMedia
			// 
			this->tboxMedia->Location = System::Drawing::Point(83, 70);
			this->tboxMedia->Name = L"tboxMedia";
			this->tboxMedia->ReadOnly = true;
			this->tboxMedia->Size = System::Drawing::Size(71, 19);
			this->tboxMedia->TabIndex = 8;
			this->tboxMedia->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// labHex2
			// 
			this->labHex2->AutoSize = true;
			this->labHex2->Location = System::Drawing::Point(157, 48);
			this->labHex2->Name = L"labHex2";
			this->labHex2->Size = System::Drawing::Size(11, 12);
			this->labHex2->TabIndex = 7;
			this->labHex2->Text = L"h";
			// 
			// tboxTitleIDLo
			// 
			this->tboxTitleIDLo->Location = System::Drawing::Point(83, 20);
			this->tboxTitleIDLo->Name = L"tboxTitleIDLo";
			this->tboxTitleIDLo->ReadOnly = true;
			this->tboxTitleIDLo->Size = System::Drawing::Size(71, 19);
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
			this->labTitleIDHi->Location = System::Drawing::Point(11, 48);
			this->labTitleIDHi->Name = L"labTitleIDHi";
			this->labTitleIDHi->Size = System::Drawing::Size(66, 12);
			this->labTitleIDHi->TabIndex = 2;
			this->labTitleIDHi->Text = L"TitleID High";
			// 
			// tboxTitleIDHi
			// 
			this->tboxTitleIDHi->Location = System::Drawing::Point(83, 45);
			this->tboxTitleIDHi->Name = L"tboxTitleIDHi";
			this->tboxTitleIDHi->ReadOnly = true;
			this->tboxTitleIDHi->Size = System::Drawing::Size(71, 19);
			this->tboxTitleIDHi->TabIndex = 3;
			this->tboxTitleIDHi->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// tboxAppTypeOther
			// 
			this->tboxAppTypeOther->Location = System::Drawing::Point(14, 134);
			this->tboxAppTypeOther->Multiline = true;
			this->tboxAppTypeOther->Name = L"tboxAppTypeOther";
			this->tboxAppTypeOther->ReadOnly = true;
			this->tboxAppTypeOther->Size = System::Drawing::Size(170, 34);
			this->tboxAppTypeOther->TabIndex = 4;
			// 
			// labAppTypeOther
			// 
			this->labAppTypeOther->AutoSize = true;
			this->labAppTypeOther->Location = System::Drawing::Point(12, 122);
			this->labAppTypeOther->Name = L"labAppTypeOther";
			this->labAppTypeOther->Size = System::Drawing::Size(58, 12);
			this->labAppTypeOther->TabIndex = 5;
			this->labAppTypeOther->Text = L"Other Info.";
			// 
			// labCaptionEx
			// 
			this->labCaptionEx->AutoSize = true;
			this->labCaptionEx->Location = System::Drawing::Point(538, 52);
			this->labCaptionEx->Name = L"labCaptionEx";
			this->labCaptionEx->Size = System::Drawing::Size(53, 12);
			this->labCaptionEx->TabIndex = 11;
			this->labCaptionEx->Text = L"���L����";
			// 
			// tboxCaptionEx
			// 
			this->tboxCaptionEx->Location = System::Drawing::Point(540, 67);
			this->tboxCaptionEx->Multiline = true;
			this->tboxCaptionEx->Name = L"tboxCaptionEx";
			this->tboxCaptionEx->ReadOnly = true;
			this->tboxCaptionEx->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->tboxCaptionEx->Size = System::Drawing::Size(171, 88);
			this->tboxCaptionEx->TabIndex = 10;
			// 
			// gboxProd
			// 
			this->gboxProd->Controls->Add(this->combBackup);
			this->gboxProd->Controls->Add(this->labBackup);
			this->gboxProd->Controls->Add(this->tboxBackupOther);
			this->gboxProd->Location = System::Drawing::Point(378, 36);
			this->gboxProd->Name = L"gboxProd";
			this->gboxProd->Size = System::Drawing::Size(356, 50);
			this->gboxProd->TabIndex = 13;
			this->gboxProd->TabStop = false;
			this->gboxProd->Text = L"ROM���Y���";
			// 
			// menuStripAbove
			// 
			this->menuStripAbove->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->stripFile, this->stripMaster, 
				this->stripLang});
			this->menuStripAbove->Location = System::Drawing::Point(0, 0);
			this->menuStripAbove->Name = L"menuStripAbove";
			this->menuStripAbove->Size = System::Drawing::Size(777, 24);
			this->menuStripAbove->TabIndex = 33;
			this->menuStripAbove->Text = L"menuStrip1";
			// 
			// stripFile
			// 
			this->stripFile->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->stripItemOpenRom, 
				this->stripItemSepFile1, this->stripItemSaveTemp, this->stripItemLoadTemp});
			this->stripFile->Name = L"stripFile";
			this->stripFile->Size = System::Drawing::Size(51, 20);
			this->stripFile->Text = L"�t�@�C��";
			// 
			// stripItemOpenRom
			// 
			this->stripItemOpenRom->Name = L"stripItemOpenRom";
			this->stripItemOpenRom->Size = System::Drawing::Size(211, 22);
			this->stripItemOpenRom->Text = L"ROM�f�[�^���J��";
			this->stripItemOpenRom->Click += gcnew System::EventHandler(this, &Form1::stripItemOpenRom_Click);
			// 
			// stripItemSepFile1
			// 
			this->stripItemSepFile1->Name = L"stripItemSepFile1";
			this->stripItemSepFile1->Size = System::Drawing::Size(208, 6);
			// 
			// stripItemSaveTemp
			// 
			this->stripItemSaveTemp->Name = L"stripItemSaveTemp";
			this->stripItemSaveTemp->Size = System::Drawing::Size(211, 22);
			this->stripItemSaveTemp->Text = L"��o�����ꎞ�ۑ�����";
			this->stripItemSaveTemp->Click += gcnew System::EventHandler(this, &Form1::stripItemSaveTemp_Click);
			// 
			// stripItemLoadTemp
			// 
			this->stripItemLoadTemp->Name = L"stripItemLoadTemp";
			this->stripItemLoadTemp->Size = System::Drawing::Size(211, 22);
			this->stripItemLoadTemp->Text = L"�ꎞ�ۑ�������o�����J��";
			this->stripItemLoadTemp->Click += gcnew System::EventHandler(this, &Form1::stripItemLoadTemp_Click);
			// 
			// stripMaster
			// 
			this->stripMaster->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {this->stripItemSheet, 
				this->stripItemSepMaster1, this->stripItemMasterRom, this->stripItemMiddlewareXml, this->stripItemMiddlewareHtml});
			this->stripMaster->Name = L"stripMaster";
			this->stripMaster->Size = System::Drawing::Size(53, 20);
			this->stripMaster->Text = L"�}�X�^�[";
			// 
			// stripItemSheet
			// 
			this->stripItemSheet->Name = L"stripItemSheet";
			this->stripItemSheet->Size = System::Drawing::Size(304, 22);
			this->stripItemSheet->Text = L"��o�f�[�^�ꎮ���쐬����";
			this->stripItemSheet->Click += gcnew System::EventHandler(this, &Form1::stripItemSheet_Click);
			// 
			// stripItemSepMaster1
			// 
			this->stripItemSepMaster1->Name = L"stripItemSepMaster1";
			this->stripItemSepMaster1->Size = System::Drawing::Size(301, 6);
			// 
			// stripItemMasterRom
			// 
			this->stripItemMasterRom->Name = L"stripItemMasterRom";
			this->stripItemMasterRom->Size = System::Drawing::Size(304, 22);
			this->stripItemMasterRom->Text = L"�}�X�^�[ROM�݂̂��쐬����";
			this->stripItemMasterRom->Click += gcnew System::EventHandler(this, &Form1::stripItemMasterRom_Click);
			// 
			// stripItemMiddlewareXml
			// 
			this->stripItemMiddlewareXml->Name = L"stripItemMiddlewareXml";
			this->stripItemMiddlewareXml->Size = System::Drawing::Size(304, 22);
			this->stripItemMiddlewareXml->Text = L"�g�p�~�h���E�F�A�ꗗ�݂̂��쐬����(XML�`��)";
			this->stripItemMiddlewareXml->Click += gcnew System::EventHandler(this, &Form1::stripItemMiddlewareXml_Click);
			// 
			// stripItemMiddlewareHtml
			// 
			this->stripItemMiddlewareHtml->Name = L"stripItemMiddlewareHtml";
			this->stripItemMiddlewareHtml->Size = System::Drawing::Size(304, 22);
			this->stripItemMiddlewareHtml->Text = L"�g�p�~�h���E�F�A�ꗗ�݂̂��쐬����(HTML�`��)";
			this->stripItemMiddlewareHtml->Click += gcnew System::EventHandler(this, &Form1::stripItemMiddlewareHtml_Click);
			// 
			// stripLang
			// 
			this->stripLang->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->stripItemEnglish, 
				this->stripItemJapanese});
			this->stripLang->Name = L"stripLang";
			this->stripLang->Size = System::Drawing::Size(65, 20);
			this->stripLang->Text = L"Language";
			// 
			// stripItemEnglish
			// 
			this->stripItemEnglish->Name = L"stripItemEnglish";
			this->stripItemEnglish->Size = System::Drawing::Size(119, 22);
			this->stripItemEnglish->Text = L"English";
			this->stripItemEnglish->Click += gcnew System::EventHandler(this, &Form1::stripItemEnglish_Click);
			// 
			// stripItemJapanese
			// 
			this->stripItemJapanese->Checked = true;
			this->stripItemJapanese->CheckState = System::Windows::Forms::CheckState::Checked;
			this->stripItemJapanese->Name = L"stripItemJapanese";
			this->stripItemJapanese->Size = System::Drawing::Size(119, 22);
			this->stripItemJapanese->Text = L"Japanese";
			this->stripItemJapanese->Click += gcnew System::EventHandler(this, &Form1::stripItemJapanese_Click);
			// 
			// tabMain
			// 
			this->tabMain->Controls->Add(this->tabRomInfo);
			this->tabMain->Controls->Add(this->tabTWLInfo);
			this->tabMain->Controls->Add(this->tabRomEditInfo);
			this->tabMain->Controls->Add(this->tabSubmitInfo);
			this->tabMain->Controls->Add(this->tabCompanyInfo);
			this->tabMain->Controls->Add(this->tabErrorInfo);
			this->tabMain->Location = System::Drawing::Point(12, 93);
			this->tabMain->Name = L"tabMain";
			this->tabMain->SelectedIndex = 0;
			this->tabMain->Size = System::Drawing::Size(753, 377);
			this->tabMain->TabIndex = 34;
			this->tabMain->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::tabMain_SelectedIndexChanged);
			// 
			// tabRomInfo
			// 
			this->tabRomInfo->Controls->Add(this->gridLibrary);
			this->tabRomInfo->Controls->Add(this->tboxGuideRomInfo);
			this->tabRomInfo->Controls->Add(this->gboxSrl);
			this->tabRomInfo->Controls->Add(this->labLib);
			this->tabRomInfo->Controls->Add(this->labCaptionEx);
			this->tabRomInfo->Controls->Add(this->tboxCaptionEx);
			this->tabRomInfo->Controls->Add(this->gboxCRC);
			this->tabRomInfo->Controls->Add(this->tboxSDK);
			this->tabRomInfo->Controls->Add(this->labSDK);
			this->tabRomInfo->Location = System::Drawing::Point(4, 21);
			this->tabRomInfo->Name = L"tabRomInfo";
			this->tabRomInfo->Padding = System::Windows::Forms::Padding(3);
			this->tabRomInfo->Size = System::Drawing::Size(745, 352);
			this->tabRomInfo->TabIndex = 0;
			this->tabRomInfo->Text = L"ROM��{���(�m�F�p)";
			this->tabRomInfo->UseVisualStyleBackColor = true;
			// 
			// gridLibrary
			// 
			this->gridLibrary->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::AllCellsExceptHeaders;
			this->gridLibrary->BackgroundColor = System::Drawing::SystemColors::Control;
			this->gridLibrary->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->gridLibrary->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) {this->colLibPublisher, 
				this->colLibName});
			dataGridViewCellStyle13->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
			dataGridViewCellStyle13->BackColor = System::Drawing::SystemColors::Window;
			dataGridViewCellStyle13->Font = (gcnew System::Drawing::Font(L"MS UI Gothic", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(128)));
			dataGridViewCellStyle13->ForeColor = System::Drawing::SystemColors::ControlText;
			dataGridViewCellStyle13->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle13->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle13->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->gridLibrary->DefaultCellStyle = dataGridViewCellStyle13;
			this->gridLibrary->Location = System::Drawing::Point(337, 189);
			this->gridLibrary->Name = L"gridLibrary";
			this->gridLibrary->ReadOnly = true;
			this->gridLibrary->RowHeadersVisible = false;
			this->gridLibrary->RowTemplate->Height = 21;
			this->gridLibrary->Size = System::Drawing::Size(374, 145);
			this->gridLibrary->TabIndex = 36;
			// 
			// colLibPublisher
			// 
			this->colLibPublisher->HeaderText = L"Publisher";
			this->colLibPublisher->Name = L"colLibPublisher";
			this->colLibPublisher->ReadOnly = true;
			// 
			// colLibName
			// 
			this->colLibName->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
			this->colLibName->HeaderText = L"Library Name";
			this->colLibName->Name = L"colLibName";
			this->colLibName->ReadOnly = true;
			// 
			// tboxGuideRomInfo
			// 
			this->tboxGuideRomInfo->BackColor = System::Drawing::SystemColors::Info;
			this->tboxGuideRomInfo->Location = System::Drawing::Point(24, 11);
			this->tboxGuideRomInfo->Name = L"tboxGuideRomInfo";
			this->tboxGuideRomInfo->ReadOnly = true;
			this->tboxGuideRomInfo->Size = System::Drawing::Size(687, 19);
			this->tboxGuideRomInfo->TabIndex = 35;
			this->tboxGuideRomInfo->Text = L"���̃^�u�̏��͕ҏW�s�ł��B�f�[�^�Ɍ�肪����ꍇ�ɂ�ROM�f�[�^�̍쐬���̐ݒ���������Ă��������B";
			// 
			// tabTWLInfo
			// 
			this->tabTWLInfo->Controls->Add(this->tboxGuideTWLInfo);
			this->tabTWLInfo->Controls->Add(this->gboxExFlags);
			this->tabTWLInfo->Controls->Add(this->gboxShared2Size);
			this->tabTWLInfo->Controls->Add(this->gboxTWLExInfo);
			this->tabTWLInfo->Controls->Add(this->gboxAccess);
			this->tabTWLInfo->Controls->Add(this->gboxTitleID);
			this->tabTWLInfo->Location = System::Drawing::Point(4, 21);
			this->tabTWLInfo->Name = L"tabTWLInfo";
			this->tabTWLInfo->Padding = System::Windows::Forms::Padding(3);
			this->tabTWLInfo->Size = System::Drawing::Size(745, 352);
			this->tabTWLInfo->TabIndex = 1;
			this->tabTWLInfo->Text = L"TWL�g�����(�m�F�p)";
			this->tabTWLInfo->UseVisualStyleBackColor = true;
			// 
			// tboxGuideTWLInfo
			// 
			this->tboxGuideTWLInfo->BackColor = System::Drawing::SystemColors::Info;
			this->tboxGuideTWLInfo->Location = System::Drawing::Point(24, 11);
			this->tboxGuideTWLInfo->Name = L"tboxGuideTWLInfo";
			this->tboxGuideTWLInfo->ReadOnly = true;
			this->tboxGuideTWLInfo->Size = System::Drawing::Size(687, 19);
			this->tboxGuideTWLInfo->TabIndex = 36;
			this->tboxGuideTWLInfo->Text = L"���̃^�u�̏��͕ҏW�s�ł��B�f�[�^�Ɍ�肪����ꍇ�ɂ�ROM�f�[�^�̍쐬���̐ݒ���������Ă��������B";
			// 
			// gboxExFlags
			// 
			this->gboxExFlags->Controls->Add(this->cboxIsNormalJump);
			this->gboxExFlags->Controls->Add(this->cboxIsTmpJump);
			this->gboxExFlags->Controls->Add(this->cboxIsWL);
			this->gboxExFlags->Controls->Add(this->cboxIsSubBanner);
			this->gboxExFlags->Location = System::Drawing::Point(12, 236);
			this->gboxExFlags->Name = L"gboxExFlags";
			this->gboxExFlags->Size = System::Drawing::Size(198, 108);
			this->gboxExFlags->TabIndex = 35;
			this->gboxExFlags->TabStop = false;
			this->gboxExFlags->Text = L"TWL�g�����t���O";
			// 
			// tabRomEditInfo
			// 
			this->tabRomEditInfo->Controls->Add(this->gboxOtherSpec);
			this->tabRomEditInfo->Controls->Add(this->butSetBack);
			this->tabRomEditInfo->Controls->Add(this->tboxGuideRomEditInfo);
			this->tabRomEditInfo->Controls->Add(this->gboxParental);
			this->tabRomEditInfo->Controls->Add(this->gboxIcon);
			this->tabRomEditInfo->Controls->Add(this->gboxEULA);
			this->tabRomEditInfo->Location = System::Drawing::Point(4, 21);
			this->tabRomEditInfo->Name = L"tabRomEditInfo";
			this->tabRomEditInfo->Size = System::Drawing::Size(745, 352);
			this->tabRomEditInfo->TabIndex = 2;
			this->tabRomEditInfo->Text = L"ROM�o�^���(�ҏW��)";
			this->tabRomEditInfo->UseVisualStyleBackColor = true;
			// 
			// gboxOtherSpec
			// 
			this->gboxOtherSpec->Controls->Add(this->cboxIsUGC);
			this->gboxOtherSpec->Controls->Add(this->cboxIsPhotoEx);
			this->gboxOtherSpec->Location = System::Drawing::Point(19, 225);
			this->gboxOtherSpec->Name = L"gboxOtherSpec";
			this->gboxOtherSpec->Size = System::Drawing::Size(266, 70);
			this->gboxOtherSpec->TabIndex = 41;
			this->gboxOtherSpec->TabStop = false;
			this->gboxOtherSpec->Text = L"���̑�ROM�d�l";
			// 
			// cboxIsUGC
			// 
			this->cboxIsUGC->AutoSize = true;
			this->cboxIsUGC->Location = System::Drawing::Point(8, 18);
			this->cboxIsUGC->Name = L"cboxIsUGC";
			this->cboxIsUGC->Size = System::Drawing::Size(210, 16);
			this->cboxIsUGC->TabIndex = 39;
			this->cboxIsUGC->Text = L"UGC(User Generated Contents)�Ή�";
			this->cboxIsUGC->UseVisualStyleBackColor = true;
			// 
			// cboxIsPhotoEx
			// 
			this->cboxIsPhotoEx->AutoSize = true;
			this->cboxIsPhotoEx->Location = System::Drawing::Point(8, 46);
			this->cboxIsPhotoEx->Name = L"cboxIsPhotoEx";
			this->cboxIsPhotoEx->Size = System::Drawing::Size(96, 16);
			this->cboxIsPhotoEx->TabIndex = 40;
			this->cboxIsPhotoEx->Text = L"�ʐ^�����Ή�";
			this->cboxIsPhotoEx->UseVisualStyleBackColor = true;
			// 
			// butSetBack
			// 
			this->butSetBack->Location = System::Drawing::Point(69, 309);
			this->butSetBack->Name = L"butSetBack";
			this->butSetBack->Size = System::Drawing::Size(151, 23);
			this->butSetBack->TabIndex = 38;
			this->butSetBack->Text = L"�ǂݍ��ݎ��̐ݒ�ɖ߂�";
			this->butSetBack->UseVisualStyleBackColor = true;
			this->butSetBack->Click += gcnew System::EventHandler(this, &Form1::butSetBack_Click);
			// 
			// tboxGuideRomEditInfo
			// 
			this->tboxGuideRomEditInfo->BackColor = System::Drawing::SystemColors::Info;
			this->tboxGuideRomEditInfo->Location = System::Drawing::Point(24, 11);
			this->tboxGuideRomEditInfo->Multiline = true;
			this->tboxGuideRomEditInfo->Name = L"tboxGuideRomEditInfo";
			this->tboxGuideRomEditInfo->ReadOnly = true;
			this->tboxGuideRomEditInfo->Size = System::Drawing::Size(687, 34);
			this->tboxGuideRomEditInfo->TabIndex = 37;
			// 
			// gboxParental
			// 
			this->gboxParental->Controls->Add(this->labRegion);
			this->gboxParental->Controls->Add(this->combRegion);
			this->gboxParental->Controls->Add(this->labParentalRating);
			this->gboxParental->Controls->Add(this->labOFLC);
			this->gboxParental->Controls->Add(this->labPEGI_BBFC);
			this->gboxParental->Controls->Add(this->combPEGI_BBFC);
			this->gboxParental->Controls->Add(this->combCERO);
			this->gboxParental->Controls->Add(this->labCERO);
			this->gboxParental->Controls->Add(this->combOFLC);
			this->gboxParental->Controls->Add(this->labPEGI_PRT);
			this->gboxParental->Controls->Add(this->combPEGI_PRT);
			this->gboxParental->Controls->Add(this->combESRB);
			this->gboxParental->Controls->Add(this->labESRB);
			this->gboxParental->Controls->Add(this->labPEGI);
			this->gboxParental->Controls->Add(this->combPEGI);
			this->gboxParental->Controls->Add(this->combUSK);
			this->gboxParental->Controls->Add(this->labUSK);
			this->gboxParental->Location = System::Drawing::Point(313, 60);
			this->gboxParental->Name = L"gboxParental";
			this->gboxParental->Size = System::Drawing::Size(398, 272);
			this->gboxParental->TabIndex = 33;
			this->gboxParental->TabStop = false;
			this->gboxParental->Text = L"���[�W�����ƃ��[�e�B���O���";
			// 
			// gboxIcon
			// 
			this->gboxIcon->Controls->Add(this->rIsNoIcon);
			this->gboxIcon->Controls->Add(this->rIsWiFiIcon);
			this->gboxIcon->Controls->Add(this->rIsWirelessIcon);
			this->gboxIcon->Location = System::Drawing::Point(19, 114);
			this->gboxIcon->Name = L"gboxIcon";
			this->gboxIcon->Size = System::Drawing::Size(266, 105);
			this->gboxIcon->TabIndex = 32;
			this->gboxIcon->TabStop = false;
			this->gboxIcon->Text = L"���j���[��ł̃A�C�R���\��";
			// 
			// rIsNoIcon
			// 
			this->rIsNoIcon->AutoSize = true;
			this->rIsNoIcon->Checked = true;
			this->rIsNoIcon->Location = System::Drawing::Point(8, 25);
			this->rIsNoIcon->Name = L"rIsNoIcon";
			this->rIsNoIcon->Size = System::Drawing::Size(120, 16);
			this->rIsNoIcon->TabIndex = 6;
			this->rIsNoIcon->TabStop = true;
			this->rIsNoIcon->Text = L"�A�C�R����\�����Ȃ�";
			this->rIsNoIcon->UseVisualStyleBackColor = true;
			// 
			// rIsWiFiIcon
			// 
			this->rIsWiFiIcon->AutoSize = true;
			this->rIsWiFiIcon->Location = System::Drawing::Point(8, 77);
			this->rIsWiFiIcon->Name = L"rIsWiFiIcon";
			this->rIsWiFiIcon->Size = System::Drawing::Size(134, 16);
			this->rIsWiFiIcon->TabIndex = 5;
			this->rIsWiFiIcon->Text = L"Wi-Fi�ʐM�A�C�R���\��";
			this->rIsWiFiIcon->UseVisualStyleBackColor = true;
			// 
			// rIsWirelessIcon
			// 
			this->rIsWirelessIcon->AutoSize = true;
			this->rIsWirelessIcon->Location = System::Drawing::Point(8, 51);
			this->rIsWirelessIcon->Name = L"rIsWirelessIcon";
			this->rIsWirelessIcon->Size = System::Drawing::Size(167, 16);
			this->rIsWirelessIcon->TabIndex = 4;
			this->rIsWirelessIcon->Text = L"DS���C�����X�ʐM�A�C�R���\��";
			this->rIsWirelessIcon->UseVisualStyleBackColor = true;
			// 
			// gboxEULA
			// 
			this->gboxEULA->Controls->Add(this->cboxIsEULA);
			this->gboxEULA->Location = System::Drawing::Point(19, 60);
			this->gboxEULA->Name = L"gboxEULA";
			this->gboxEULA->Size = System::Drawing::Size(266, 48);
			this->gboxEULA->TabIndex = 31;
			this->gboxEULA->TabStop = false;
			this->gboxEULA->Text = L"EULA(���p�K��)";
			// 
			// tabSubmitInfo
			// 
			this->tabSubmitInfo->Controls->Add(this->labProductNameLimit);
			this->tabSubmitInfo->Controls->Add(this->tboxGuideSubmitInfo);
			this->tabSubmitInfo->Controls->Add(this->gboxForeign);
			this->tabSubmitInfo->Controls->Add(this->tboxCaption);
			this->tabSubmitInfo->Controls->Add(this->labSubmitVer);
			this->tabSubmitInfo->Controls->Add(this->labCaption);
			this->tabSubmitInfo->Controls->Add(this->tboxProductCode2);
			this->tabSubmitInfo->Controls->Add(this->labCapSubmitVer);
			this->tabSubmitInfo->Controls->Add(this->gboxUsage);
			this->tabSubmitInfo->Controls->Add(this->numSubmitVersion);
			this->tabSubmitInfo->Controls->Add(this->gboxProd);
			this->tabSubmitInfo->Controls->Add(this->gboxSubmitWay);
			this->tabSubmitInfo->Controls->Add(this->tboxProductCode1);
			this->tabSubmitInfo->Controls->Add(this->labSubmiteDate);
			this->tabSubmitInfo->Controls->Add(this->dateRelease);
			this->tabSubmitInfo->Controls->Add(this->tboxProductName);
			this->tabSubmitInfo->Controls->Add(this->labReleaseDate);
			this->tabSubmitInfo->Controls->Add(this->dateSubmit);
			this->tabSubmitInfo->Controls->Add(this->labProductCode2);
			this->tabSubmitInfo->Controls->Add(this->labProductCode);
			this->tabSubmitInfo->Controls->Add(this->labProductName);
			this->tabSubmitInfo->Controls->Add(this->labProductCode1);
			this->tabSubmitInfo->Location = System::Drawing::Point(4, 21);
			this->tabSubmitInfo->Name = L"tabSubmitInfo";
			this->tabSubmitInfo->Size = System::Drawing::Size(745, 352);
			this->tabSubmitInfo->TabIndex = 3;
			this->tabSubmitInfo->Text = L"��o���(�ҏW��)";
			this->tabSubmitInfo->UseVisualStyleBackColor = true;
			// 
			// labProductNameLimit
			// 
			this->labProductNameLimit->Location = System::Drawing::Point(237, 75);
			this->labProductNameLimit->Name = L"labProductNameLimit";
			this->labProductNameLimit->Size = System::Drawing::Size(124, 11);
			this->labProductNameLimit->TabIndex = 39;
			this->labProductNameLimit->Text = L"(30�����ȓ�)";
			this->labProductNameLimit->TextAlign = System::Drawing::ContentAlignment::TopRight;
			// 
			// tboxGuideSubmitInfo
			// 
			this->tboxGuideSubmitInfo->BackColor = System::Drawing::SystemColors::Info;
			this->tboxGuideSubmitInfo->Location = System::Drawing::Point(24, 11);
			this->tboxGuideSubmitInfo->Name = L"tboxGuideSubmitInfo";
			this->tboxGuideSubmitInfo->ReadOnly = true;
			this->tboxGuideSubmitInfo->Size = System::Drawing::Size(687, 19);
			this->tboxGuideSubmitInfo->TabIndex = 38;
			this->tboxGuideSubmitInfo->Text = L"���̃^�u�̏��͒�o�m�F���̍쐬�ɕK�v�ł��B���͂��Ă��������B";
			// 
			// gboxForeign
			// 
			this->gboxForeign->Controls->Add(this->labProductNameLimitForeign);
			this->gboxForeign->Controls->Add(this->labMultiForeign2);
			this->gboxForeign->Controls->Add(this->cboxReleaseForeign);
			this->gboxForeign->Controls->Add(this->labMultiForeign1);
			this->gboxForeign->Controls->Add(this->tboxProductCode2Foreign1);
			this->gboxForeign->Controls->Add(this->tboxProductCode2Foreign3);
			this->gboxForeign->Controls->Add(this->labProductCodeForeign);
			this->gboxForeign->Controls->Add(this->tboxProductCode2Foreign2);
			this->gboxForeign->Controls->Add(this->tboxProductCode1Foreign);
			this->gboxForeign->Controls->Add(this->labProductCode2Foreign);
			this->gboxForeign->Controls->Add(this->labProductCode1Foreign);
			this->gboxForeign->Controls->Add(this->tboxProductNameForeign);
			this->gboxForeign->Controls->Add(this->labProductNameForeign);
			this->gboxForeign->Location = System::Drawing::Point(378, 91);
			this->gboxForeign->Name = L"gboxForeign";
			this->gboxForeign->Size = System::Drawing::Size(355, 144);
			this->gboxForeign->TabIndex = 35;
			this->gboxForeign->TabStop = false;
			this->gboxForeign->Text = L"�C�O��";
			// 
			// labProductNameLimitForeign
			// 
			this->labProductNameLimitForeign->Location = System::Drawing::Point(225, 62);
			this->labProductNameLimitForeign->Name = L"labProductNameLimitForeign";
			this->labProductNameLimitForeign->Size = System::Drawing::Size(124, 11);
			this->labProductNameLimitForeign->TabIndex = 40;
			this->labProductNameLimitForeign->Text = L"(30�����ȓ�)";
			this->labProductNameLimitForeign->TextAlign = System::Drawing::ContentAlignment::TopRight;
			// 
			// labMultiForeign2
			// 
			this->labMultiForeign2->AutoSize = true;
			this->labMultiForeign2->Location = System::Drawing::Point(217, 121);
			this->labMultiForeign2->Name = L"labMultiForeign2";
			this->labMultiForeign2->Size = System::Drawing::Size(101, 12);
			this->labMultiForeign2->TabIndex = 45;
			this->labMultiForeign2->Text = L"(��������ꍇ�̂�)";
			// 
			// tabCompanyInfo
			// 
			this->tabCompanyInfo->Controls->Add(this->tboxGuideCompanyInfo);
			this->tabCompanyInfo->Controls->Add(this->cboxIsInputPerson2);
			this->tabCompanyInfo->Controls->Add(this->gboxPerson1);
			this->tabCompanyInfo->Controls->Add(this->gboxPerson2);
			this->tabCompanyInfo->Location = System::Drawing::Point(4, 21);
			this->tabCompanyInfo->Name = L"tabCompanyInfo";
			this->tabCompanyInfo->Size = System::Drawing::Size(745, 352);
			this->tabCompanyInfo->TabIndex = 4;
			this->tabCompanyInfo->Text = L"��Џ��(�ҏW��)";
			this->tabCompanyInfo->UseVisualStyleBackColor = true;
			// 
			// tboxGuideCompanyInfo
			// 
			this->tboxGuideCompanyInfo->BackColor = System::Drawing::SystemColors::Info;
			this->tboxGuideCompanyInfo->Location = System::Drawing::Point(24, 11);
			this->tboxGuideCompanyInfo->Name = L"tboxGuideCompanyInfo";
			this->tboxGuideCompanyInfo->ReadOnly = true;
			this->tboxGuideCompanyInfo->Size = System::Drawing::Size(687, 19);
			this->tboxGuideCompanyInfo->TabIndex = 39;
			this->tboxGuideCompanyInfo->Text = L"���̃^�u�̏��͒�o�m�F���̍쐬�ɕK�v�ł��B���͂��Ă��������B";
			// 
			// tabErrorInfo
			// 
			this->tabErrorInfo->Controls->Add(this->tboxGuideErrorInfo);
			this->tabErrorInfo->Controls->Add(this->gboxErrorTiming);
			this->tabErrorInfo->Controls->Add(this->labWarn);
			this->tabErrorInfo->Controls->Add(this->labError);
			this->tabErrorInfo->Controls->Add(this->gridWarn);
			this->tabErrorInfo->Controls->Add(this->gridError);
			this->tabErrorInfo->Location = System::Drawing::Point(4, 21);
			this->tabErrorInfo->Name = L"tabErrorInfo";
			this->tabErrorInfo->Size = System::Drawing::Size(745, 352);
			this->tabErrorInfo->TabIndex = 5;
			this->tabErrorInfo->Text = L"�G���[���(�m�F�p)";
			this->tabErrorInfo->UseVisualStyleBackColor = true;
			// 
			// tboxGuideErrorInfo
			// 
			this->tboxGuideErrorInfo->BackColor = System::Drawing::SystemColors::Info;
			this->tboxGuideErrorInfo->Location = System::Drawing::Point(24, 11);
			this->tboxGuideErrorInfo->Multiline = true;
			this->tboxGuideErrorInfo->Name = L"tboxGuideErrorInfo";
			this->tboxGuideErrorInfo->ReadOnly = true;
			this->tboxGuideErrorInfo->Size = System::Drawing::Size(511, 61);
			this->tboxGuideErrorInfo->TabIndex = 40;
			// 
			// gboxErrorTiming
			// 
			this->gboxErrorTiming->Controls->Add(this->rErrorCurrent);
			this->gboxErrorTiming->Controls->Add(this->rErrorReading);
			this->gboxErrorTiming->Location = System::Drawing::Point(551, 11);
			this->gboxErrorTiming->Name = L"gboxErrorTiming";
			this->gboxErrorTiming->Size = System::Drawing::Size(172, 71);
			this->gboxErrorTiming->TabIndex = 41;
			this->gboxErrorTiming->TabStop = false;
			this->gboxErrorTiming->Text = L"���̏���\�����邩";
			// 
			// rErrorCurrent
			// 
			this->rErrorCurrent->AutoSize = true;
			this->rErrorCurrent->Location = System::Drawing::Point(6, 45);
			this->rErrorCurrent->Name = L"rErrorCurrent";
			this->rErrorCurrent->Size = System::Drawing::Size(114, 16);
			this->rErrorCurrent->TabIndex = 1;
			this->rErrorCurrent->TabStop = true;
			this->rErrorCurrent->Text = L"���݂̓��͂𔽉f";
			this->rErrorCurrent->UseVisualStyleBackColor = true;
			this->rErrorCurrent->CheckedChanged += gcnew System::EventHandler(this, &Form1::rErrorCurrent_CheckedChanged);
			// 
			// rErrorReading
			// 
			this->rErrorReading->AutoSize = true;
			this->rErrorReading->Checked = true;
			this->rErrorReading->Location = System::Drawing::Point(6, 18);
			this->rErrorReading->Name = L"rErrorReading";
			this->rErrorReading->Size = System::Drawing::Size(134, 16);
			this->rErrorReading->TabIndex = 0;
			this->rErrorReading->TabStop = true;
			this->rErrorReading->Text = L"ROM�f�[�^�ǂݍ��ݎ�";
			this->rErrorReading->UseVisualStyleBackColor = true;
			this->rErrorReading->CheckedChanged += gcnew System::EventHandler(this, &Form1::rErrorReading_CheckedChanged);
			// 
			// labWarn
			// 
			this->labWarn->AutoSize = true;
			this->labWarn->ForeColor = System::Drawing::SystemColors::ActiveCaption;
			this->labWarn->Location = System::Drawing::Point(22, 213);
			this->labWarn->Name = L"labWarn";
			this->labWarn->Size = System::Drawing::Size(339, 12);
			this->labWarn->TabIndex = 40;
			this->labWarn->Text = L"�x��(�C���͕K�{�ł͂���܂��񂪏��Ɍ�肪�Ȃ������m�F���������B)";
			// 
			// labError
			// 
			this->labError->AutoSize = true;
			this->labError->ForeColor = System::Drawing::SystemColors::ActiveCaption;
			this->labError->Location = System::Drawing::Point(22, 84);
			this->labError->Name = L"labError";
			this->labError->Size = System::Drawing::Size(145, 12);
			this->labError->TabIndex = 39;
			this->labError->Text = L"�G���[(�K���C�����Ă��������B)";
			// 
			// gridWarn
			// 
			this->gridWarn->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::AllCellsExceptHeaders;
			this->gridWarn->BackgroundColor = System::Drawing::SystemColors::Control;
			this->gridWarn->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->gridWarn->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(4) {this->colWarnName, 
				this->colWarnBegin, this->colWarnEnd, this->colWarnCause});
			dataGridViewCellStyle14->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
			dataGridViewCellStyle14->BackColor = System::Drawing::SystemColors::Window;
			dataGridViewCellStyle14->Font = (gcnew System::Drawing::Font(L"MS UI Gothic", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(128)));
			dataGridViewCellStyle14->ForeColor = System::Drawing::SystemColors::ControlText;
			dataGridViewCellStyle14->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle14->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle14->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->gridWarn->DefaultCellStyle = dataGridViewCellStyle14;
			this->gridWarn->GridColor = System::Drawing::SystemColors::Control;
			this->gridWarn->Location = System::Drawing::Point(24, 228);
			this->gridWarn->Name = L"gridWarn";
			this->gridWarn->ReadOnly = true;
			this->gridWarn->RowHeadersVisible = false;
			this->gridWarn->RowTemplate->Height = 21;
			this->gridWarn->Size = System::Drawing::Size(699, 106);
			this->gridWarn->TabIndex = 38;
			// 
			// colWarnName
			// 
			this->colWarnName->HeaderText = L"���ږ�";
			this->colWarnName->Name = L"colWarnName";
			this->colWarnName->ReadOnly = true;
			this->colWarnName->Width = 150;
			// 
			// colWarnBegin
			// 
			this->colWarnBegin->HeaderText = L"�J�n";
			this->colWarnBegin->Name = L"colWarnBegin";
			this->colWarnBegin->ReadOnly = true;
			this->colWarnBegin->Width = 60;
			// 
			// colWarnEnd
			// 
			this->colWarnEnd->HeaderText = L"�I��";
			this->colWarnEnd->Name = L"colWarnEnd";
			this->colWarnEnd->ReadOnly = true;
			this->colWarnEnd->Width = 60;
			// 
			// colWarnCause
			// 
			this->colWarnCause->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
			this->colWarnCause->HeaderText = L"�v��";
			this->colWarnCause->Name = L"colWarnCause";
			this->colWarnCause->ReadOnly = true;
			// 
			// gridError
			// 
			dataGridViewCellStyle15->BackColor = System::Drawing::Color::White;
			this->gridError->AlternatingRowsDefaultCellStyle = dataGridViewCellStyle15;
			this->gridError->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::AllCellsExceptHeaders;
			this->gridError->BackgroundColor = System::Drawing::SystemColors::Control;
			this->gridError->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->gridError->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(4) {this->colErrorName, 
				this->colErrorBegin, this->colErrorEnd, this->colErrorCause});
			dataGridViewCellStyle16->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
			dataGridViewCellStyle16->BackColor = System::Drawing::SystemColors::Window;
			dataGridViewCellStyle16->Font = (gcnew System::Drawing::Font(L"MS UI Gothic", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(128)));
			dataGridViewCellStyle16->ForeColor = System::Drawing::SystemColors::ControlText;
			dataGridViewCellStyle16->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle16->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle16->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->gridError->DefaultCellStyle = dataGridViewCellStyle16;
			this->gridError->GridColor = System::Drawing::SystemColors::Control;
			this->gridError->Location = System::Drawing::Point(24, 99);
			this->gridError->Name = L"gridError";
			this->gridError->ReadOnly = true;
			this->gridError->RowHeadersVisible = false;
			this->gridError->RowTemplate->Height = 21;
			this->gridError->Size = System::Drawing::Size(699, 103);
			this->gridError->TabIndex = 37;
			// 
			// colErrorName
			// 
			this->colErrorName->HeaderText = L"���ږ�";
			this->colErrorName->Name = L"colErrorName";
			this->colErrorName->ReadOnly = true;
			this->colErrorName->Width = 150;
			// 
			// colErrorBegin
			// 
			this->colErrorBegin->HeaderText = L"�J�n";
			this->colErrorBegin->Name = L"colErrorBegin";
			this->colErrorBegin->ReadOnly = true;
			this->colErrorBegin->Width = 60;
			// 
			// colErrorEnd
			// 
			this->colErrorEnd->HeaderText = L"�I��";
			this->colErrorEnd->Name = L"colErrorEnd";
			this->colErrorEnd->ReadOnly = true;
			this->colErrorEnd->Width = 60;
			// 
			// colErrorCause
			// 
			this->colErrorCause->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
			this->colErrorCause->HeaderText = L"�v��";
			this->colErrorCause->Name = L"colErrorCause";
			this->colErrorCause->ReadOnly = true;
			// 
			// labFile
			// 
			this->labFile->AutoSize = true;
			this->labFile->Location = System::Drawing::Point(22, 53);
			this->labFile->Name = L"labFile";
			this->labFile->Size = System::Drawing::Size(92, 12);
			this->labFile->TabIndex = 36;
			this->labFile->Text = L"ROM�f�[�^�t�@�C��";
			// 
			// labAssemblyVersion
			// 
			this->labAssemblyVersion->AutoSize = true;
			this->labAssemblyVersion->ForeColor = System::Drawing::SystemColors::ControlText;
			this->labAssemblyVersion->Location = System::Drawing::Point(696, 24);
			this->labAssemblyVersion->Name = L"labAssemblyVersion";
			this->labAssemblyVersion->Size = System::Drawing::Size(69, 12);
			this->labAssemblyVersion->TabIndex = 37;
			this->labAssemblyVersion->Text = L"ver.0.0.00000";
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(777, 482);
			this->Controls->Add(this->labAssemblyVersion);
			this->Controls->Add(this->labFile);
			this->Controls->Add(this->tabMain);
			this->Controls->Add(this->tboxFile);
			this->Controls->Add(this->menuStripAbove);
			this->MainMenuStrip = this->menuStripAbove;
			this->Name = L"Form1";
			this->Text = L"TWL MasterEditor";
			this->gboxSrl->ResumeLayout(false);
			this->gboxSrl->PerformLayout();
			this->gboxCRC->ResumeLayout(false);
			this->gboxCRC->PerformLayout();
			this->gboxPerson2->ResumeLayout(false);
			this->gboxPerson2->PerformLayout();
			this->gboxPerson1->ResumeLayout(false);
			this->gboxPerson1->PerformLayout();
			this->gboxUsage->ResumeLayout(false);
			this->gboxUsage->PerformLayout();
			this->gboxSubmitWay->ResumeLayout(false);
			this->gboxSubmitWay->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numSubmitVersion))->EndInit();
			this->gboxShared2Size->ResumeLayout(false);
			this->gboxShared2Size->PerformLayout();
			this->gboxTWLExInfo->ResumeLayout(false);
			this->gboxTWLExInfo->PerformLayout();
			this->gboxAccess->ResumeLayout(false);
			this->gboxAccess->PerformLayout();
			this->gboxTitleID->ResumeLayout(false);
			this->gboxTitleID->PerformLayout();
			this->gboxProd->ResumeLayout(false);
			this->gboxProd->PerformLayout();
			this->menuStripAbove->ResumeLayout(false);
			this->menuStripAbove->PerformLayout();
			this->tabMain->ResumeLayout(false);
			this->tabRomInfo->ResumeLayout(false);
			this->tabRomInfo->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridLibrary))->EndInit();
			this->tabTWLInfo->ResumeLayout(false);
			this->tabTWLInfo->PerformLayout();
			this->gboxExFlags->ResumeLayout(false);
			this->gboxExFlags->PerformLayout();
			this->tabRomEditInfo->ResumeLayout(false);
			this->tabRomEditInfo->PerformLayout();
			this->gboxOtherSpec->ResumeLayout(false);
			this->gboxOtherSpec->PerformLayout();
			this->gboxParental->ResumeLayout(false);
			this->gboxParental->PerformLayout();
			this->gboxIcon->ResumeLayout(false);
			this->gboxIcon->PerformLayout();
			this->gboxEULA->ResumeLayout(false);
			this->gboxEULA->PerformLayout();
			this->tabSubmitInfo->ResumeLayout(false);
			this->tabSubmitInfo->PerformLayout();
			this->gboxForeign->ResumeLayout(false);
			this->gboxForeign->PerformLayout();
			this->tabCompanyInfo->ResumeLayout(false);
			this->tabCompanyInfo->PerformLayout();
			this->tabErrorInfo->ResumeLayout(false);
			this->tabErrorInfo->PerformLayout();
			this->gboxErrorTiming->ResumeLayout(false);
			this->gboxErrorTiming->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridWarn))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridError))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	/////////////////////////////////////////////
	// �������\�b�h
	/////////////////////////////////////////////
	private:
		// ----------------------------------------------
		// �t�@�C����R/W
		// ----------------------------------------------

		// �ݒ�t�@�C���̓ǂݍ���
		void loadInit(void);

		// SRL�̃I�[�v��
		System::Void loadSrl( System::String ^filename );

		// SRL�̕ۑ��ƍēǂݏo��
		System::Boolean saveSrl( System::String ^filename );

		// SRL�̕ۑ��̂� @ret ����
		System::Boolean saveSrlCore( System::String ^filename );

		// �~�h���E�F�A���X�g�̍쐬(XML�`��)
		System::Void makeMiddlewareListXml(System::Xml::XmlDocument^ doc);

		// �~�h���E�F�A���X�g�̕ۑ�
		System::Boolean saveMiddlewareListXml( System::String ^filename );

		// �~�h���E�F�A���X�g�̕ۑ�(XML->HTML�ϊ�)
		System::Boolean saveMiddlewareListHtml( System::String ^filename );

		// �~�h���E�F�A���X�g�̕ۑ�(XSL���ߍ���)
		System::Boolean saveMiddlewareListXmlEmbeddedXsl( System::String ^filename );

	private:
		// ----------------------------------------------
		// �ꎞ�t�@�C���̎�舵��
		// ----------------------------------------------

		// �ꎞ�ۑ�
		System::Void saveTmp( System::String ^filename );

		// �ꎞ�t�@�C����ǂݏo��
		void loadTmp( System::String ^filename );

		// �ꎞ�ۑ������t�H�[�����ɕϊ�
		System::Boolean parseTmp( System::Xml::XmlElement ^root, System::String ^xpath, System::Windows::Forms::ComboBox ^comb );
		System::Boolean parseTmp( System::Xml::XmlElement ^root, System::String ^xpath, System::Windows::Forms::NumericUpDown ^num );
		System::Boolean parseTmp( System::Xml::XmlElement ^root, System::String ^xpath, 
								  cli::array<System::Windows::Forms::RadioButton^>^rbuts, cli::array<System::String ^>^textCands );
		System::Boolean parseTmp( System::Xml::XmlElement ^root, System::String ^xpath, System::Windows::Forms::CheckBox ^cbox );
		System::Boolean parseTmp( System::Xml::XmlElement ^root, System::String ^xpath, System::Windows::Forms::TextBox ^tbox );
		System::Boolean parseTmp( System::Xml::XmlElement ^root, System::String ^xpath, System::Windows::Forms::DateTimePicker ^date );

	private:
		// ----------------------------------------------
		// �t�H�[���̏����ݒ�
		// ----------------------------------------------

		// �ݒ�/�I���\�ȃt�H�[�������ׂ� disable �ɂ���
		void readOnly( void )
		{
			this->cboxIsEULA->Enabled = false;
			this->rIsWirelessIcon->Enabled = false;
			this->rIsWiFiIcon->Enabled     = false;
			this->rIsNoIcon->Enabled       = false;
			this->cboxIsUGC->Enabled       = false;
			this->cboxIsPhotoEx->Enabled   = false;

			this->combCERO->Enabled = false;
			this->combESRB->Enabled = false;
			this->combUSK->Enabled = false;
			this->combPEGI->Enabled = false;
			this->combPEGI_PRT->Enabled = false;
			this->combPEGI_BBFC->Enabled = false;
			this->combOFLC->Enabled = false;
		}

		// �o�[�W���������擾
		System::String^ getVersion( void )
		{
			System::Reflection::Assembly ^ass = System::Reflection::Assembly::GetEntryAssembly();
			System::Version ^ver =  ass->GetName()->Version;
			return ( ver->Major.ToString() + "." + ver->Minor.ToString() + "." + ver->Build.ToString() );
		}

		// SRL�ɓo�^����Ȃ�ROM�d�l�̃t�H�[�����͂�
		// �V�K�ǂݍ��݂̂Ƃ��ɂ̓N���A����
		// �ēǂݍ��݂̂Ƃ��ɂ͑O�̏�Ԃɖ߂�
		void clearOtherForms(void)
		{
			this->cboxIsUGC->Checked     = false;
			this->cboxIsPhotoEx->Checked = false;
			this->hIsCheckedUGC     = gcnew System::Boolean(false);
			this->hIsCheckedPhotoEx = gcnew System::Boolean(false);
		}
		void saveOtherForms(void)
		{
			this->hIsCheckedUGC     = gcnew System::Boolean(this->cboxIsUGC->Checked);
			this->hIsCheckedPhotoEx = gcnew System::Boolean(this->cboxIsPhotoEx->Checked);
		}
		void loadOtherForms(void)
		{
			this->cboxIsUGC->Checked     = *(this->hIsCheckedUGC);
			this->cboxIsPhotoEx->Checked = *(this->hIsCheckedPhotoEx);
		}

	private:
		// ----------------------------------------------
		// �t�H�[����SRL�����𖵏��Ȃ���v������
		// ----------------------------------------------

		// ROM�����t�H�[������擾����SRL�N���X�̃v���p�e�B�ɔ��f������
		// (ROM�w�b�_�ւ̔��f��CRC�Ə����̍Čv�Z�����Ȃ�)
		void setSrlProperties(void);

		// SRL��ROM�����t�H�[���ɔ��f������(�t�@�C�����ǂݍ��܂�Ă��邱�Ƃ��O��)
		void setSrlForms(void);

		// SRL�̓���Ȑݒ���t�H�[���ɃZ�b�g����(����؂�ւ��ŕ\����ς������̂œƗ�������)
		void setSrlFormsCaptionEx();

		// �t�H�[���̓��͂��`�F�b�N����
		System::Boolean checkSrlForms(void);

	private:
		// ---------------------------------------------------------------------
		// ���[�W�����ݒ�͕��G�Ȃ̂ŕʂɐ؂�o��
		// ---------------------------------------------------------------------
		
		// �t�H�[�����͂�SRL�ɔ��f������
		void setRegionSrlPropaties(void);

		// SRL�����t�H�[���ɔ��f������
		void setRegionForms(void);

	private:
		// ---------------------------------------------------------------------
		// �y�A�����^���R���g���[���ݒ�͕��G�Ȃ̂ŕʂɐ؂�o��
		// ---------------------------------------------------------------------

		// �t�H�[�����͂�SRL�ɔ��f������
		void setParentalSrlProperties(void);

		// SRL�����t�H�[���ɔ��f������
		void setParentalForms(void);

		// ���[�W������񂩂�y�A�����^���R���g���[���̕ҏW�\�c�̂��}�X�N����
		void maskParentalForms(void);

		// �t�H�[�����͂����������������ݑO�`�F�b�N
		void checkParentalForms( System::Boolean inRegion, System::Windows::Forms::ComboBox ^comb, System::String ^msg );

		// �N���A����
		void clearParental( System::Windows::Forms::ComboBox ^comb );

		// �ҏW�ł���悤�ɂ���
		void enableParental( System::Windows::Forms::ComboBox ^comb, 
							 System::Windows::Forms::Label    ^lab1, 
							 System::Windows::Forms::Label    ^lab2 );

		// �ҏW�ł��Ȃ�����
		void disableParental( System::Windows::Forms::ComboBox ^comb, 
							  System::Windows::Forms::Label    ^lab1, 
							  System::Windows::Forms::Label    ^lab2 );

		// ----------------------------------------------
		// �t�H�[���̃`�F�b�N
		// ----------------------------------------------

		// �e�L�X�g���͂�����Ă��邩�`�F�b�N
		System::Boolean checkTextForm( System::String ^formtext, System::String ^labelJ, System::String ^labelE, System::Boolean affectRom );

		// ���l���͂����킩�ǂ����`�F�b�N
		System::Boolean checkNumRange( 
			System::Int32 val, System::Int32 min, System::Int32 max, System::String ^labelJ, System::String ^labelE, System::Boolean affectRom );

		System::Boolean checkNumRange( System::String ^strval, System::Int32 min, System::Int32 max, 
									   System::String ^labelJ, System::String ^labelE, System::Boolean affectRom );
		// �R���{�{�b�N�X���`�F�b�N
		System::Boolean checkBoxIndex( System::Windows::Forms::ComboBox ^box, System::String ^labelJ, System::String ^labelE, System::Boolean affectRom );

		// -----------------------------------------------------------------
		// ��o���(SRL�ɉe�����Ȃ��ӏ��̂�)�ƃt�H�[���Ԃ̃f�[�^�̂��Ƃ�
		// -----------------------------------------------------------------

		// ��o�m�F���Ƀt�H�[���𔽉f
		void setDeliverableProperties(void);

		// ��o���̃t�H�[���`�F�b�N
		System::Boolean checkDeliverableForms(void);

		// ----------------------------------------------
		// �_�C�A���O
		// ----------------------------------------------

		void sucMsg( System::String ^msgJ, System::String ^msgE )
		{
			if( this->stripItemJapanese->Checked )
				MessageBox::Show( msgJ, "Information", MessageBoxButtons::OK, MessageBoxIcon::None );
			else
				MessageBox::Show( msgE, "Information", MessageBoxButtons::OK, MessageBoxIcon::None );
		}

		// �G���[���b�Z�[�W���o��
		void errMsg( System::String ^msgJ, System::String ^msgE )
		{
			if( this->stripItemJapanese->Checked )
				MessageBox::Show( msgJ, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error );
			else
				MessageBox::Show( msgE, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error );
		}

	private:
		// ----------------------------------------------
		// ���p���Ή�
		// ----------------------------------------------

		// ���{��łƉp��łŃe�L�X�g�{�b�N�X�̕����񐧌����ς��
		void changeMaxLength( System::Windows::Forms::TextBox ^tbox, System::Int32 maxlen );

		// ���{��łւ̐؂�ւ�
		void changeJapanese(void);

		// �p��łւ̐؂�ւ�
		void changeEnglish(void);

		// --------------------------------------------------------
		// �G���[���̓o�^
		// --------------------------------------------------------

		// �ǂݍ��ݎ��G���[�̓o�^
		void setGridError( void );
		void setGridWarn( void );

		// �ǂݍ��ݎ��Ɍ��o�����C���\�G���[�Ɍ��݂̓��͂𔽉f
		void overloadGridError( void );
		void overloadGridWarn( void );

		// �Z���̐F��ς���
		void colorGridError( RCMrcError ^err );
		void colorGridWarn( RCMrcError ^err );

		// �܂Ƃ߂čX�V
		void updateGrid(void);

		// ----------------------------------------------
		// �G���[����
		// ----------------------------------------------

		// SRL�ɂ͊֌W���Ȃ����ޏ�̃G���[���`�F�b�N
		System::Boolean isValidOnlyDeliverable(void);

		// SRL�̃o�C�i���ɉe�����鍀�ڂɃG���[�����邩�`�F�b�N
		System::Boolean isValidAffectRom(void);

		// SRL�̃o�C�i���ɉe�����鍀�ڂ̒��ŏC���\�ȃG���[�������`�F�b�N
		System::Boolean isValidAffectRomModified(void);

	/////////////////////////////////////////////
	// �^�C�g���o�[���상�\�b�h
	/////////////////////////////////////////////

	private:
		System::Void stripItemEnglish_Click(System::Object^  sender, System::EventArgs^  e)
		{
			this->stripItemEnglish->Checked  = true;
			this->stripItemJapanese->Checked = false;
			this->changeEnglish();
			this->updateGrid();
		}

	private:
		System::Void stripItemJapanese_Click(System::Object^  sender, System::EventArgs^  e)
		{
			this->stripItemEnglish->Checked  = false;
			this->stripItemJapanese->Checked = true;
			this->changeJapanese();
			this->updateGrid();
		}

	private:
		System::Void stripItemOpenRom_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String^ filename;

			// �h���b�O�A���h�h���b�v�ȊO�ł̓_�C�A���O������͂���
			{
				System::Windows::Forms::OpenFileDialog ^dlg = gcnew (OpenFileDialog);

				dlg->InitialDirectory = System::Environment::GetFolderPath( System::Environment::SpecialFolder::Desktop );//"c:\\";
				dlg->Filter      = "srl format (*.srl)|*.srl|All files (*.*)|*.*";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					//this->errMsg( "ROM�f�[�^�t�@�C���̃I�[�v�����L�����Z������܂����B", "Opening the ROM data file is canceled by user." );
					return;
				}
				filename = dlg->FileName;
			}
			this->loadSrl( filename );
			this->clearOtherForms();
			//this->sucMsg( "ROM�f�[�^�t�@�C���̃I�[�v���ɐ������܂����B", "The ROM data file is opened successfully." );
		} //stripItemOpenRom_Click()

	private:
		System::Void stripItemMasterRom_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String^ filename;

			// SRL���ǂݍ��܂�Ă��Ȃ��Ƃ��ɂ̓��[�h�����Ȃ�
			if( System::String::IsNullOrEmpty( this->tboxFile->Text ) )
			{
				this->errMsg( "ROM�f�[�^�t�@�C�����I�[�v������Ă��܂���̂ŁA�}�X�^�[ROM�̍쐬���ł��܂���B", 
							  "ROM data file has not opened yet. A master ROM data can't be made." );
				return;
			}

			// SRL�֘A�t�H�[�����͂��`�F�b�N����
			this->hErrorList->Clear();
			this->hWarnList->Clear();
			if( this->checkSrlForms() == false )
			{
				this->errMsg( "�s���Ȑݒ肪���邽�߃}�X�^�[ROM�̍쐬���ł��܂���B",
							  "Setting is illegal. A master ROM data can't be made." );
				return;
			}

			// SRL�����o�菇���ɏ]�킹��
			{
				filename = gcnew System::String("");

				if( this->cboxRemasterVerE->Checked == true )
				{
					filename = "T" + this->hSrl->hGameCode + "E" + this->numSubmitVersion->Value.ToString() + ".SRL";
				}
				else
				{
					filename = "T" + this->hSrl->hGameCode + this->hSrl->hRomVersion->ToString() + this->numSubmitVersion->Value.ToString() + ".SRL";
				}
			}

			// ���ӏ��� 
			{
				this->sucMsg( 
					"��o�菇���ɂ��������AROM�f�[�^�t�@�C������ \"" + filename + "\"�ƂȂ�܂��B\n" + "\nROM�f�[�^�t�@�C����ۑ�����t�H���_��I�����Ă��������B",
					"ROM data file name is \"" + filename + "\".\n" + "\nPlease select a folder in which the ROM data is saved."
				);
			}
			// �_�C�A���O����SRL��ۑ�����f�B���N�g�����擾����
			{
				System::Windows::Forms::FolderBrowserDialog ^dlg = gcnew (System::Windows::Forms::FolderBrowserDialog);

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					this->errMsg( "�t�H���_�̑I�����L�����Z������܂����̂Ń}�X�^�[ROM�͍쐬����܂���B", 
								  "A submission sheet can not be made, since selecting folder is canceled." );
					return;
				}
				else
				{
					if( !dlg->SelectedPath->EndsWith("\\") )
					{
						filename = dlg->SelectedPath + "\\" + filename;
					}
					else
					{
						filename = dlg->SelectedPath + filename;
					}
				}
				if( System::IO::File::Exists( filename ) )
				{
					System::String ^msg;
					if( this->stripItemJapanese->Checked )
						msg = gcnew System::String( filename + "�͂��łɑ��݂��܂��B�㏑�����܂���?" );
					else
						msg = gcnew System::String( filename + "already exists. Overwrite it?" );
					if( MessageBox::Show( msg, "Information", MessageBoxButtons::YesNo, MessageBoxIcon::None ) 
						== System::Windows::Forms::DialogResult::No )
					{
						this->errMsg( "�}�X�^�[ROM�̍쐬���L�����Z�����܂����B", 
									  "Making a master ROM is canceled." );
						return;
					}
				}
			}
			try
			{
				if( !this->saveSrl( filename ) )
				{
					this->errMsg( "�}�X�^�[ROM�̍쐬�Ɏ��s���܂����B",
								  "Making a master ROM failed." );
					return;
				}
				this->sucMsg( "�}�X�^�[ROM�̍쐬���������܂����B", "Making the ROM data file succeeded." );
				this->tboxFile->Text = filename;
			}
			catch( System::Exception ^ex )
			{
				(void)ex;
				this->errMsg( "�}�X�^�[ROM�̍쐬�Ɏ��s���܂����B",
							  "Making a master ROM failed." );
				return;
			}
		} //stripItemMasterRom_Click()

	private:
		System::Void stripItemSheet_Click(System::Object^  sender, System::EventArgs^  e)
		{
			ECDeliverableResult  result;

			// SRL���ǂݍ��܂�Ă��Ȃ��Ƃ��ɂ̓��[�h�����Ȃ�
			if( System::String::IsNullOrEmpty( this->tboxFile->Text ) )
			{
				this->errMsg( "ROM�f�[�^�t�@�C�����I�[�v������Ă��܂���B", "ROM file has not opened yet." );
				return;
			}

			// SRL�Ə��ނ̗����̃t�H�[�����͂��`�F�b�N����
			this->hErrorList->Clear();
			this->hWarnList->Clear();
			if( this->checkSrlForms() == false )
			{
				this->errMsg( "�s���Ȑݒ肪���邽�ߒ�o�f�[�^�̍쐬���ł��܂���B",
							  "Setting is illegal. Submission data can't be made." );
				return;
			}
			if( this->checkDeliverableForms() == false )
			{
				this->errMsg( "���͏��ɕs�������邽�ߒ�o�m�F�����쐬�ł��܂���B",
							  "Input is not enough. Submission data can't be made." );
				return;
			}

			// SRL�����o�菇���ɏ]�킹��
			System::String ^srlfile;
			{
				srlfile = gcnew System::String("");

				if( this->cboxRemasterVerE->Checked == true )
				{
					srlfile = "T" + this->hSrl->hGameCode + "E" + this->numSubmitVersion->Value.ToString() + ".SRL";
				}
				else
				{
					srlfile = "T" + this->hSrl->hGameCode + this->hSrl->hRomVersion->ToString() + this->numSubmitVersion->Value.ToString() + ".SRL";
				}
			}

			// ���ӏ��� 
			{
				this->sucMsg( 
					"Step1/3: ROM�f�[�^�t�@�C���ƒ�o�m�F���̏�����v�����邽�߁A�܂��A���͏��𔽉f�������}�X�^�[ROM�f�[�^�t�@�C�����쐬���܂��B\n(�L�����Z�����ꂽ�Ƃ��A��o�f�[�^�ꎮ�͍쐬����܂���B)\n"
					+ "\n  �}�X�^�[ROM�f�[�^�t�@�C������ \"" + srlfile + "\"�ƂȂ�܂��B\n" + "\nROM�f�[�^�t�@�C����ۑ�����t�H���_��I�����Ă��������B",
					"Step1/3: Firstly, We make a master ROM file because all information in a submission sheet are match those in the ROM data file.\n(When it is canceled, both A set of submission data is not made.)\n"
					+ "\n  The name of the master ROM data file is \"" + srlfile + "\".\n" + "\nPlease select a folder in which the ROM data is saved."
				);
			}

			// �_�C�A���O����SRL��ۑ�����f�B���N�g�����擾����
			System::String ^delivfile;
			{
				System::Windows::Forms::FolderBrowserDialog ^dlg = gcnew (System::Windows::Forms::FolderBrowserDialog);

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					this->errMsg( "�t�H���_�̑I�����L�����Z������܂����̂Œ�o�f�[�^�ꎮ�͍쐬����܂���B", 
								  "A set of submission data can not be made, since selecting folder is canceled." );
					return;
				}
				else
				{
					if( !dlg->SelectedPath->EndsWith("\\") )
					{
						srlfile = dlg->SelectedPath + "\\" + srlfile;
					}
					else
					{
						srlfile = dlg->SelectedPath + srlfile;
					}
				}
				if( System::IO::File::Exists( srlfile ) )
				{
					System::String ^msg;
					if( this->stripItemJapanese->Checked )
						msg = gcnew System::String( srlfile + "�͂��łɑ��݂��܂��B�㏑�����܂���?" );
					else
						msg = gcnew System::String( srlfile + "already exists. Overwrite it?" );
					if( MessageBox::Show( msg, "Information", MessageBoxButtons::YesNo, MessageBoxIcon::None ) 
						== System::Windows::Forms::DialogResult::No )
					{
						this->errMsg( "�t�@�C���̑I�����L�����Z������܂����̂Œ�o�f�[�^�ꎮ�͍쐬����܂���B", 
									  "Since selecting a file is canceled, a set of submission data can not be made." );
						return;
					}
				}

			}

			// ���ӏ��� 
			{
				this->sucMsg( 
					"Step2/3: �����Ďg�p����Ă���~�h���E�F�A�̃��X�g���쐬���܂��B\n�����ŃL�����Z�����ꂽ�Ƃ��A��o�f�[�^�ꎮ�͍쐬����܂���̂ł����ӂ��������B",
					"Step2/3: Secondly, We should make a list of middlewares used by the ROM. \n(CAUTION: When it is canceled, A set of submission data is not made.)"
				);
			}
			// �_�C�A���O�Ńt�@�C���p�X������
			System::String ^middlefile;
			{
				System::Windows::Forms::SaveFileDialog ^dlg = gcnew (SaveFileDialog);

				dlg->InitialDirectory = System::Environment::GetFolderPath( System::Environment::SpecialFolder::Desktop );//"c:\\";
				dlg->Filter      = "xml format (*.xml)|*.xml";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					this->errMsg( "�~�h���E�F�A���X�g�̍쐬���L�����Z������܂����B��o�f�[�^�ꎮ�͍쐬����܂���B", 
						          "Making a list of middlewares is canceled. A set of submission data is not made." );
					return;
				}
				middlefile = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".xml" )) )
				{
					middlefile += ".xml";
				}
			}

			// ���ӏ��� 
			{
				this->sucMsg( 
					"Step3/3: �����Ē�o�m�F�����쐬���܂��B\n�����ŃL�����Z�����ꂽ�Ƃ��A��o�f�[�^�ꎮ�͍쐬����܂���̂ł����ӂ��������B",
					"Step3/3: Finally, We should make a submission sheet. \n(CAUTION: When it is canceled, A set of submission data is not made, but also the master ROM data and a list of middleware are not made.)"
				);
			}
			// �_�C�A���O�Ńt�@�C���p�X������
			{
				System::Windows::Forms::SaveFileDialog ^dlg = gcnew (SaveFileDialog);

				dlg->InitialDirectory = System::Environment::GetFolderPath( System::Environment::SpecialFolder::Desktop );//"c:\\";
				dlg->Filter      = "xml format (*.xml)|*.xml";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					this->errMsg( "��o�m�F���̍쐬���L�����Z������܂����B��o�f�[�^�ꎮ�͍쐬����܂���B", 
						          "Making a submission sheet is canceled. A set of submission data is not made." );
					return;
				}
				delivfile = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".xml" )) )
				{
					delivfile += ".xml";
				}
			}

			// �}�X�^��o�m�F���ɕK�v�ȏ����t�H�[������擾���čX�V
			this->setSrlProperties();	// ���Srl���X�V���Ă���
			this->setDeliverableProperties();

			// �X�V���SRL��ʃt�@�C���ɍ쐬
			try
			{
				if( !this->saveSrl( srlfile ) )
				{
					this->errMsg( "�}�X�^�[ROM�̕ۑ��Ɏ��s���܂����B��o�m�F������у~�h���E�F�A���X�g�͍쐬����܂���B",
								  "Making a master ROM failed. And a submission sheet and a list of middlewares are not made." );
					return;
				}
				this->tboxFile->Text = srlfile;
			}
			catch( System::Exception ^ex )
			{
				(void)ex;
				this->errMsg( "�}�X�^�[ROM�̕ۑ��Ɏ��s���܂����B��o�m�F������у~�h���E�F�A���X�g�͍쐬����܂���B",
							  "Making a master ROM failed. And a submission sheet and a list of middlewares are not made." );
				return;
			}
			u16  crc;			// SRL�S�̂�CRC���v�Z����(���ނɋL�q���邽��)
			if( !getWholeCRCInFile( srlfile, &crc ) )
			{
				this->errMsg( "CRC�̌v�Z�Ɏ��s���܂����B��o�m�F������у~�h���E�F�A���X�g�͍쐬����܂���B", 
							  "Calc CRC is failed. Therefore, And a submission sheet and a list of middlewares are not made." );
				return;
			}
			System::UInt16 ^hcrc = gcnew System::UInt16( crc );
			this->tboxWholeCRC->Clear();
			this->tboxWholeCRC->AppendText( "0x" );
			this->tboxWholeCRC->AppendText( hcrc->ToString("X") );

			// �~�h���E�F�A�̃��X�g���쐬
			if( !this->saveMiddlewareListXmlEmbeddedXsl( middlefile ) )
			{
				this->errMsg( "�~�h���E�F�A�̃��X�g���쐬�ł��܂���ł����B��o�m�F���͍쐬����܂���B",
							  "Making a list of middleware failed. And a submission sheet is not made.");
				return;
			}

			// ���ލ쐬
			cli::array<System::String^> ^paths = srlfile->Split(L'\\');			// �]���ȃp�X���폜
			srlfile = paths[ paths->Length - 1 ];
			//result = this->hDeliv->write( delivfile, this->hSrl, hcrc, srlfile, !(this->stripItemJapanese->Checked) );
			result = this->hDeliv->writeSpreadsheet( delivfile, this->hSrl, hcrc, srlfile, !(this->stripItemJapanese->Checked) );
			if( result != ECDeliverableResult::NOERROR )
			{
				switch( result )
				{
					case ECDeliverableResult::ERROR_FILE_OPEN:
						this->errMsg( "��o�m�F���̃e���v���[�g���J���Ȃ��������߁A��o�m�F���̍쐬�Ɏ��s���܂����B", 
							          "Since a templete of the submission sheet can't be opened, making the sheet is failed." );
					break;

					case ECDeliverableResult::ERROR_FILE_WRITE:
						this->errMsg( "��o�m�F���Ƀf�[�^���������݂ł��܂���ł����B�����t�@�C�������łɊJ����Ă��Ȃ������m�F���������B", 
							          "Writing data into a submission sheet failed. Please check that the file has been opened already." );
					break;

					default:
						this->errMsg( "��o�m�F���̍쐬�Ɏ��s���܂����B", "Making the submission sheet is failed." );
					break;
				}
				return;
			}
			this->sucMsg( "��o�f�[�^�ꎮ�̍쐬�ɐ������܂����B", "The submission sheet is made successfully." );

		} //stripItemSheet_Click()

	private:
		System::Void stripItemSaveTemp_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String ^filename = gcnew System::String("");

			// �_�C�A���O�Ńt�@�C���p�X������
			{
				System::Windows::Forms::SaveFileDialog ^dlg = gcnew (SaveFileDialog);

				dlg->InitialDirectory = System::Environment::GetFolderPath( System::Environment::SpecialFolder::Desktop );//"c:\\";
				dlg->Filter      = "xml format (*.xml)|*.xml";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					return;
				}
				filename = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".xml" )) )
				{
					filename += ".xml";
				}
			}
			this->saveTmp( filename );
		} //stripItemSaveTemp_Click()

	private:
		System::Void stripItemLoadTemp_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String ^filename = gcnew System::String("");

			// �_�C�A���O�Ńt�@�C���p�X������
			{
				System::Windows::Forms::OpenFileDialog ^dlg = gcnew (OpenFileDialog);

				dlg->InitialDirectory = System::Environment::GetFolderPath( System::Environment::SpecialFolder::Desktop );//"c:\\";
				dlg->Filter      = "xml format (*.xml)|*.xml";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					return;
				}
				filename = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".xml" )) )
				{
					filename += ".xml";
				}
			}
			this->loadTmp( filename );
		} //stripItemLoadTemp_Click()

	private:
		System::Void stripItemMiddlewareXml_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String ^filename = gcnew System::String("");

			if( System::String::IsNullOrEmpty(this->tboxFile->Text) )
			{
				this->errMsg( "ROM�f�[�^�t�@�C�����I�[�v������Ă��܂���B", "ROM file has not opened yet." );
				return;
			}

			// �_�C�A���O�Ńt�@�C���p�X������
			{
				System::Windows::Forms::SaveFileDialog ^dlg = gcnew (SaveFileDialog);

				dlg->InitialDirectory = System::Environment::GetFolderPath( System::Environment::SpecialFolder::Desktop );//"c:\\";
				dlg->Filter      = "xml format (*.xml)|*.xml";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					return;
				}
				filename = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".xml" )) )
				{
					filename += ".xml";
				}
			}
			if( !this->saveMiddlewareListXmlEmbeddedXsl(filename) )
			{
				this->errMsg( "�~�h���E�F�A���X�g�̍쐬�Ɏ��s���܂����B","Making a middleware list failed." );
			}
		} //stripItemMiddlewareXml_Click()

	private:
		System::Void stripItemMiddlewareHtml_Click(System::Object^  sender, System::EventArgs^  e)
		{
			System::String ^filename = gcnew System::String("");

			if( System::String::IsNullOrEmpty(this->tboxFile->Text) )
			{
				this->errMsg( "ROM�f�[�^�t�@�C�����I�[�v������Ă��܂���B", "ROM file has not opened yet." );
				return;
			}

			// �_�C�A���O�Ńt�@�C���p�X������
			{
				System::Windows::Forms::SaveFileDialog ^dlg = gcnew (SaveFileDialog);

				dlg->InitialDirectory = System::Environment::GetFolderPath( System::Environment::SpecialFolder::Desktop );//"c:\\";
				dlg->Filter      = "html format (*.html)|*.html";
				dlg->FilterIndex = 1;
				dlg->RestoreDirectory = true;

				if( dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK )
				{
					return;
				}
				filename = dlg->FileName;
				if( !(dlg->FileName->EndsWith( ".html" )) )
				{
					filename += ".html";
				}
			}
			if( !this->saveMiddlewareListHtml(filename) )
			{
				this->errMsg( "�~�h���E�F�A���X�g�̍쐬�Ɏ��s���܂����B","Making a middleware list failed." );
			}
		} //stripItemMiddlewareHtml_Click


	/////////////////////////////////////////////
	// �t�H�[�����상�\�b�h
	/////////////////////////////////////////////

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
				this->errMsg( "ROM�f�[�^�t�@�C�������݂��܂���̂ŊJ�����Ƃ��ł��܂���B", 
							  "The ROM data file is not found. Therefore the file can not be opened." );
				return;
			}
			this->loadSrl( filename );			// �h���b�O�A���h�h���b�v�̎��_�Ń{�^���������Ȃ��Ă��t�@�C�����J��
			this->clearOtherForms();
			//this->sucMsg( "ROM�f�[�^�t�@�C���̃I�[�v���ɐ������܂����B", "The ROM data file is opened successfully." );
		}

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
		System::Void combRegion_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
		{
			this->maskParentalForms();
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

	private:
		System::Void rErrorReading_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{
			this->updateGrid();
		}

	private:
		System::Void rErrorCurrent_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{
			this->updateGrid();
		}

	private:
		System::Void tabMain_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
		{
			// �G���[�^�u���ŐV���ɍX�V
			if( tabMain->SelectedIndex == 5 )
			{
				this->updateGrid();
			}
		}

	private:
		System::Void butSetBack_Click(System::Object^  sender, System::EventArgs^  e)
		{
			if( System::String::IsNullOrEmpty( this->tboxFile->Text ) )
				return;

			// �ҏW�\����ǂݍ��ݎ��̐ݒ�ɖ߂�
			this->cboxIsEULA->Checked       = *(this->hSrl->hIsEULA);
			if( (  *this->hSrl->hIsWiFiIcon  &&   *this->hSrl->hIsWirelessIcon) ||
				(!(*this->hSrl->hIsWiFiIcon) && !(*this->hSrl->hIsWirelessIcon)) )
			{
				this->rIsNoIcon->Checked = true;
			}
			else if( *(this->hSrl->hIsWiFiIcon) && !*(this->hSrl->hIsWirelessIcon) )
			{
				this->rIsWiFiIcon->Checked = true;
			}
			else
			{
				this->rIsWirelessIcon->Checked = true;
			}
			this->setRegionForms();
			this->setParentalForms();
			this->loadOtherForms();		// SRL�ɓo�^����Ă��Ȃ�ROM�d�l�̃t�H�[�����߂�
		}

}; // enf of ref class Form1

} // end of namespace MasterEditorTWL

