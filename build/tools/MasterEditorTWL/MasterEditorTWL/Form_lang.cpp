// ----------------------------------------------
// ���p���Ή�
// ----------------------------------------------

#include "stdafx.h"
#include <apptype.h>
#include "common.h"
#include "srl.h"
#include "deliverable.h"
#include "crc_whole.h"
#include "utility.h"
#include "lang.h"
#include "FormError.h"
#include "Form1.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace MasterEditorTWL;

// ���{��łƉp��łŃe�L�X�g�{�b�N�X�̕����񐧌����ς��
void Form1::changeMaxLength( System::Windows::Forms::TextBox ^tbox, System::Int32 maxlen )
{
	if( tbox->Text->Length > maxlen )
		tbox->Text = "";

	tbox->MaxLength = maxlen;
}

// ���{��łւ̐؂�ւ�
void Form1::changeJapanese(void)
{
	System::Int32 index;

	// ���͕�����������ύX����
	this->changeMaxLength( this->tboxCompany1, 25 );
	this->changeMaxLength( this->tboxDepart1,  25 );
	this->changeMaxLength( this->tboxPerson1,  15 );

	// �^�C�g���o�[
	this->stripFile->Text          = gcnew System::String( "�t�@�C��" );
	this->stripItemOpenRom->Text   = gcnew System::String( "ROM�f�[�^���J��" );
	this->stripItemSaveTemp->Text  = gcnew System::String( "��o�����ꎞ�ۑ�����" );
	this->stripItemLoadTemp->Text  = gcnew System::String( "�ꎞ�ۑ�������o����ǂݍ���" );
	this->stripMaster->Text        = gcnew System::String( "�}�X�^�[" );
	this->stripItemSheet->Text     = gcnew System::String( "��o�f�[�^�ꎮ���쐬����" );
	this->stripItemMasterRom->Text = gcnew System::String( "�}�X�^�[ROM�݂̂��쐬����" );
	this->stripItemMiddlewareXml->Text  = gcnew System::String( "�~�h���E�F�A���X�g���쐬����(XML�`��)" );
	this->stripItemMiddlewareHtml->Text = gcnew System::String( "�~�h���E�F�A���X�g���쐬����(HTML�`��)" );

	// ���̓t�@�C��
	this->labFile->Text = gcnew System::String( "ROM�f�[�^�t�@�C��" );

	// �^�u
	this->tabRomInfo->Text     = gcnew System::String( "ROM��{���(�m�F�p)" );
	this->tabTWLInfo->Text     = gcnew System::String( "TWL�g�����(�m�F�p)" );
	this->tabRomEditInfo->Text = gcnew System::String( "ROM�o�^���(�ҏW��)" );
	this->tabSubmitInfo->Text  = gcnew System::String( "��o���(�ҏW��)" );
	this->tabCompanyInfo->Text = gcnew System::String( "��Џ��(�ҏW��)" );
	this->tabErrorInfo->Text   = gcnew System::String( "�G���[���(�m�F�p)" );

	// �K�C�h
	this->tboxGuideRomInfo->Text = gcnew System::String( "���̃^�u�̏��͕ҏW�s�ł��B�f�[�^�Ɍ�肪����ꍇ�ɂ�ROM�f�[�^�̍쐬���̐ݒ���������Ă��������B" );
	this->tboxGuideTWLInfo->Text = gcnew System::String( "���̃^�u�̏��͕ҏW�s�ł��B�f�[�^�Ɍ�肪����ꍇ�ɂ�ROM�f�[�^�̍쐬���̐ݒ���������Ă��������B" );
	this->tboxGuideRomEditInfo->Text  = gcnew System::String( "" );
	this->tboxGuideRomEditInfo->Text += "���̃^�u�̏��͒�o�m�F������у}�X�^�[ROM�̍쐬�ɕK�v�ł��B�ҏW���Ă��������B";
	this->tboxGuideRomEditInfo->Text += "\r\n(�}�X�^�[ROM�̍쐬������܂�ROM�f�[�^�̒��ɂ͓o�^����܂���B)";
	this->tboxGuideSubmitInfo->Text  = gcnew System::String( "���̃^�u�̏��͒�o�m�F���̍쐬�ɕK�v�ł��B���͂��Ă��������B" );
	this->tboxGuideCompanyInfo->Text = gcnew System::String( "���̃^�u�̏��͒�o�m�F���̍쐬�ɕK�v�ł��B���͂��Ă��������B" );
	this->tboxGuideErrorInfo->Text   = gcnew System::String( "" );
	this->tboxGuideErrorInfo->Text  += "���̃^�u�ɂ͓ǂݍ���ROM�f�[�^�̖��Ɩ{�v���O�����ł̓��̓~�X���񋓂���܂��B";
	this->tboxGuideErrorInfo->Text  += "\r\n�ԕ����̍��ڂ́A�{�v���O�����ŏC���s�ł��BROM�f�[�^�쐬���̐ݒ�����m�F���������B";
	this->tboxGuideErrorInfo->Text  += "\r\n�����̍��ڂ́A�{�v���O�����ŏC���ł��܂����A�C�����}�X�^�[ROM�ɔ��f����܂��B";
	this->tboxGuideErrorInfo->Text  += "\r\n�������̍��ڂ́A��o�m�F���ɂ̂ݔ��f����A�}�X�^�[ROM�ɂ͔��f����܂���B";

	// SRL���
	this->gboxSrl->Text       = gcnew System::String( "ROM�f�[�^���" ); 
	this->labTitleName->Text  = gcnew System::String( "�\�t�g�^�C�g��" );
	this->labGameCode->Text   = gcnew System::String( "�C�j�V�����R�[�h" );
	this->labMakerCode->Text  = gcnew System::String( "���[�J�R�[�h" );
	this->labPlatform->Text   = gcnew System::String( "�v���b�g�t�H�[��" );
	this->labRomType->Text    = gcnew System::String( "ROM�^�C�v�ݒ�" );
	this->labRomSize->Text    = gcnew System::String( "ROM�e��" );
	this->labRemasterVer->Text   = gcnew System::String( "���}�X�^�[�o�[�W����" );
	this->cboxRemasterVerE->Text = gcnew System::String( "E(������)" );
	this->labHeaderCRC->Text  = gcnew System::String( "�w�b�_CRC" );
	this->labRomCRC->Text     = gcnew System::String( "�S�̂�CRC" );
	index = this->combBackup->SelectedIndex;

	// �o�b�N�A�b�v������
	this->gboxProd->Text	= gcnew System::String( "ROM���Y���(�K�����͂��Ă�������)" );
	this->labBackup->Text   = gcnew System::String( LANG_BACKUP_J );
	this->combBackup->Items->Clear();
	this->combBackup->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"4Kbit EEPROM", L"64Kbit EEPROM", L"512Kbit EEPROM", 
		L"256Kbit FRAM", L"2Mbit FLASH", L"4Mbit FLASH", L"8Mbit FLASH", L"�Ȃ�", L"���̑�"});
	this->combBackup->SelectedIndex = index;

	// ��o���
	this->labProductName->Text = gcnew System::String( LANG_PRODUCT_NAME_J );
	this->labProductCode->Text = gcnew System::String( LANG_PRODUCT_CODE_J );
	this->labReleaseDate->Text = gcnew System::String( LANG_RELEASE_DATE_J );
	this->labSubmiteDate->Text = gcnew System::String( LANG_SUBMIT_DATE_J );
	this->gboxSubmitWay->Text  = gcnew System::String( LANG_SUBMIT_WAY_J );
	this->rSubmitPost->Text    = gcnew System::String( LANG_SUBMIT_POST_J );
	this->rSubmitHand->Text    = gcnew System::String( LANG_SUBMIT_HAND_J );
	this->gboxUsage->Text      = gcnew System::String( LANG_USAGE_J );
	this->rUsageSale->Text     = gcnew System::String( LANG_USAGE_SALE_J );
	this->rUsageSample->Text   = gcnew System::String( LANG_USAGE_SAMPLE_J );
	this->rUsageDst->Text      = gcnew System::String( LANG_USAGE_DST_J );
	this->rUsageOther->Text    = gcnew System::String( LANG_USAGE_OTHER_J );
	this->labSubmitVer->Text     = gcnew System::String( LANG_SUBMIT_VER_J );
	this->labCapSubmitVer->Text  = gcnew System::String( LANG_SUBMIT_VER_CAP_J );
	this->gboxForeign->Text      = gcnew System::String( LANG_F_J );
	this->labProductNameForeign->Text = gcnew System::String( LANG_PRODUCT_NAME_F_J );
	this->labProductCodeForeign->Text = gcnew System::String( LANG_PRODUCT_CODE_F_J );
	this->cboxReleaseForeign->Text    = gcnew System::String( LANG_RELEASE_F_J );
	this->labMultiForeign1->Text      = gcnew System::String( LANG_MULTI_F_J );
	this->labMultiForeign2->Text      = gcnew System::String( LANG_MULTI_F_J );
	this->labCaption->Text    = gcnew System::String( LANG_CAPTION_J );
	this->labProductNameLimit->Text = gcnew System::String( LANG_PRODUCT_LIMIT_J );
	this->labProductNameLimitForeign->Text = gcnew System::String( LANG_PRODUCT_LIMIT_J );

	// ��Џ��
	this->gboxPerson1->Text    = gcnew System::String( LANG_PERSON_1_J );
	this->gboxPerson2->Text    = gcnew System::String( LANG_PERSON_2_J );
	this->cboxIsInputPerson2->Text = gcnew System::String( LANG_INPUT_PERSON_2_J );
	this->labCompany1->Text    = gcnew System::String( LANG_COMPANY_J );
	this->labDepart1->Text     = gcnew System::String( LANG_DEPART_J );
	this->labPerson1->Text     = gcnew System::String( LANG_PERSON_J );
	this->labCompany2->Text    = gcnew System::String( LANG_COMPANY_J );
	this->labDepart2->Text     = gcnew System::String( LANG_DEPART_J );
	this->labPerson2->Text     = gcnew System::String( LANG_PERSON_J );
	this->labArbit1->Text      = gcnew System::String( "(�C��)" );
	this->labArbit2->Text      = gcnew System::String( "(�C��)" );
	this->labArbit3->Text      = gcnew System::String( "(�C��)" );
	this->labArbit4->Text      = gcnew System::String( "(�C��)" );
	// �ӂ肪�ȏ���L���ɂ���
	this->tboxFurigana1->Enabled = true;
	this->labFurigana1->Text = gcnew System::String( LANG_FURIGANA_J );
	this->tboxFurigana2->Enabled = true;
	this->labFurigana2->Text = gcnew System::String( LANG_FURIGANA_J );
	// NTSC-UserID�����{��ł̂�
	this->tboxNTSC1->Enabled = true;
	this->tboxNTSC2->Enabled = true;
	this->labNTSC1Pre->Text  = gcnew System::String( LANG_NTSC_1_J );
	this->labNTSC1Sur->Text  = gcnew System::String( LANG_NTSC_2_J );
	this->labNTSC2Pre->Text  = gcnew System::String( LANG_NTSC_1_J );
	this->labNTSC2Sur->Text  = gcnew System::String( LANG_NTSC_2_J );

	// TWL�d�l
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

	// SRL�ҏW�\���
	this->gboxEULA->Text         = gcnew System::String( LANG_BOX_EULA_J );
	this->cboxIsEULA->Text       = gcnew System::String( LANG_EULA_J );
	this->gboxIcon->Text         = gcnew System::String( LANG_ICON_J );
	this->rIsWirelessIcon->Text  = gcnew System::String( LANG_WIRELESS_ICON_J );
	this->rIsWiFiIcon->Text      = gcnew System::String( LANG_WIFI_ICON_J );
	this->rIsNoIcon->Text        = gcnew System::String( LANG_NO_ICON_J );
	this->labRegion->Text        = gcnew System::String( LANG_REGION_J );

	// ���[�W����
	index = this->combRegion->SelectedIndex;
	this->combRegion->Items->Clear();
	this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5)
		{L"���{�̂�", L"�č��̂�", L"���B�̂�", L"���B�̂�", L"���B����э��B"});
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	this->combRegion->Items->Add( gcnew System::String( L"�S���[�W����" ) );
#endif
	this->combRegion->SelectedIndex = index;

	//// �y�A�����^���R���g���[��
	this->gboxParental->Text           = gcnew System::String( LANG_REGION_PCTL_J );
	this->labParentalRating->Text      = gcnew System::String( LANG_PCTL_RATING_J );

	index = this->combCERO->SelectedIndex;	// ��������clear����ƌ��݂�index�ɈӖ����Ȃ��Ȃ�̂őޔ�
	this->combCERO->Items->Clear();
	this->combCERO->Items->AddRange(gcnew cli::array< System::Object^  >(6)
		{L"A (�S�N��)", L"B (12�Έȏ�)", L"C (15�Έȏ�)", L"D (17�Έȏ�)", L"Z (18�Έȏ�)", L"�R����"});
	this->combCERO->SelectedIndex = index;

	index = this->combESRB->SelectedIndex;
	this->combESRB->Items->Clear();
	this->combESRB->Items->AddRange(gcnew cli::array< System::Object^  >(7) 
		{L"�N����Ȃ�(�S�N��)", L"EC (3�Έȏ�)", L"E (6�Έȏ�)", L"E10+ (10�Έȏ�)", L"T (13�Έȏ�)", L"M (17�Έȏ�)", L"�R����"});
	this->combESRB->SelectedIndex = index;

	index = this->combUSK->SelectedIndex;
	this->combUSK->Items->Clear();
	this->combUSK->Items->AddRange(gcnew cli::array< System::Object^  >(6)
		{L"�N����Ȃ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"���N�ɂ͕s�K��", L"�R����"});
	this->combUSK->SelectedIndex = index;

	index = this->combPEGI->SelectedIndex;
	this->combPEGI->Items->Clear();
	this->combPEGI->Items->AddRange(gcnew cli::array< System::Object^  >(7)
		{L"�N����Ȃ�(�S�N��)", L"3�Έȏ�", L"7�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"�R����"});
	this->combPEGI->SelectedIndex = index;

	index = this->combPEGI_PRT->SelectedIndex;
	this->combPEGI_PRT->Items->Clear();
	this->combPEGI_PRT->Items->AddRange(gcnew cli::array< System::Object^  >(7)
		{L"�N����Ȃ�(�S�N��)", L"4�Έȏ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"�R����"});
	this->combPEGI_PRT->SelectedIndex = index;

	index = this->combPEGI_BBFC->SelectedIndex;
	this->combPEGI_BBFC->Items->Clear();
	this->combPEGI_BBFC->Items->AddRange(gcnew cli::array< System::Object^  >(10)
		{L"�N����Ȃ�(�S�N��)", L"3�Έȏ�", L"4�Έȏ㐄��", L"7�Έȏ�", L"8�Έȏ㐄��", L"12�Έȏ�", L"15�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"�R����"});
	this->combPEGI_BBFC->SelectedIndex = index;

	index = this->combOFLC->SelectedIndex;
	this->combOFLC->Items->Clear();
	this->combOFLC->Items->AddRange(gcnew cli::array< System::Object^  >(5)
		{L"G", L"PG", L"M", L"MA15+", L"�R����"});
	this->combOFLC->SelectedIndex = index;

	// �G���[���
	this->labError->Text = gcnew System::String( "�G���[(�K���C�����Ă��������B)" );
	this->colErrorName->HeaderText  = gcnew System::String( "���ږ�" );
	this->colErrorBegin->HeaderText = gcnew System::String( "�J�n" );
	this->colErrorEnd->HeaderText   = gcnew System::String( "�I��" );
	this->colErrorCause->HeaderText = gcnew System::String( "�v��" );

	this->labWarn->Text  = gcnew System::String( "�x��(�C���͕K�{�ł͂���܂��񂪏��Ɍ�肪�Ȃ������m�F���������B)" );
	this->colWarnName->HeaderText  = gcnew System::String( "���ږ�" );
	this->colWarnBegin->HeaderText = gcnew System::String( "�J�n" );
	this->colWarnEnd->HeaderText   = gcnew System::String( "�I��" );
	this->colWarnCause->HeaderText = gcnew System::String( "�v��" );

	this->gboxErrorTiming->Text = gcnew System::String( "���̏���\�����邩" );
	this->rErrorReading->Text   = gcnew System::String( "ROM�f�[�^�ǂݍ��ݎ�" );
	this->rErrorCurrent->Text   = gcnew System::String( "���݂̓��͂𔽉f" );

	// ����Ȑݒ�p�̃e�L�X�g�{�b�N�X�̕\�L��ύX
	this->setSrlFormsCaptionEx();
}

// �p��łւ̐؂�ւ�
void  Form1::changeEnglish(void)
{
	System::Int32 index;

	// ���͕�����������ύX����
	this->changeMaxLength( this->tboxCompany1, 40 );
	this->changeMaxLength( this->tboxDepart1,  40 );
	this->changeMaxLength( this->tboxPerson1,  30 );

	// �^�C�g���o�[
	this->stripFile->Text          = gcnew System::String( "File" );
	this->stripItemOpenRom->Text   = gcnew System::String( "Open a ROM data file" );
	this->stripItemSaveTemp->Text  = gcnew System::String( "Save a temporary info." );
	this->stripItemLoadTemp->Text  = gcnew System::String( "Load a temporary info. saved previously" );
	this->stripMaster->Text        = gcnew System::String( "Master" );
	this->stripItemSheet->Text     = gcnew System::String( "Make a set of submission data" );
	this->stripItemMasterRom->Text = gcnew System::String( "Make a master ROM data file only" );
	this->stripItemMiddlewareXml->Text  = gcnew System::String( "Make a middleware list(XML format)" );
	this->stripItemMiddlewareHtml->Text = gcnew System::String( "Make a middleware list(HTML format)" );

	// ���̓t�@�C��
	this->labFile->Text = gcnew System::String( "ROM Data File" );

	// �^�u
	this->tabRomInfo->Text     = gcnew System::String( "ROM Info.(Read Only)" );
	this->tabTWLInfo->Text     = gcnew System::String( "TWL Info.(Read Only)" );
	this->tabRomEditInfo->Text = gcnew System::String( "ROM Settings(Editable)" );
	this->tabSubmitInfo->Text  = gcnew System::String( "Submission Info.(Editable)" );
	this->tabCompanyInfo->Text = gcnew System::String( "Company Info.(Editable)" );
	this->tabErrorInfo->Text   = gcnew System::String( "Error(Read Only)" );

	// �K�C�h
	this->tboxGuideRomInfo->Text = gcnew System::String( "This tab is for checking ROM data. When ROM data is illegal, please check settings of building ROM data" );
	this->tboxGuideTWLInfo->Text = gcnew System::String( "This tab is for checking ROM data. When ROM data is illegal, please check settings of building ROM data" );
	this->tboxGuideRomEditInfo->Text  = gcnew System::String( "" );
	this->tboxGuideRomEditInfo->Text += "These items is necessary not only to make a submission sheet and but also to make a master ROM data. Please edit certainly.";
	this->tboxGuideRomEditInfo->Text += "\r\n(In making a master ROM data, these info will be registered in it.)";
	this->tboxGuideSubmitInfo->Text  = gcnew System::String( "These items are necessary for making a submission sheet. Please input." );
	this->tboxGuideCompanyInfo->Text = gcnew System::String( "These items are necessary for making a submission sheet. Please input." );
	this->tboxGuideErrorInfo->Text   = gcnew System::String( "" );
	this->tboxGuideErrorInfo->Text  += "This tab discribes errors in the ROM data file and edit mistakes.";
	this->tboxGuideErrorInfo->Text  += "\r\nItems highlighted by Red can't be modified by this program. Please modify build settings.";
	this->tboxGuideErrorInfo->Text  += "\r\nItems highlighted by Blue can be modified by this program and will register in a master ROM.";
	this->tboxGuideErrorInfo->Text  += "\r\nItems highlighted by Black are discribed in a submission sheet and aren't affect a master ROM.";

	// SRL���
	this->gboxSrl->Text       = gcnew System::String( "ROM Info." ); 
	this->labTitleName->Text  = gcnew System::String( "Game Title" );
	this->labGameCode->Text   = gcnew System::String( "Game Code" );
	this->labMakerCode->Text  = gcnew System::String( "Maker Code" );
	this->labPlatform->Text   = gcnew System::String( "Platform" );
	this->labRomType->Text    = gcnew System::String( "ROM Type" );
	this->labRomSize->Text    = gcnew System::String( "ROM Size" );
	this->labRemasterVer->Text   = gcnew System::String( "Release Ver." );
	this->cboxRemasterVerE->Text = gcnew System::String( "E(Preliminary Ver.)" );
	this->labHeaderCRC->Text  = gcnew System::String( "Header CRC" );
	this->labRomCRC->Text     = gcnew System::String( "ROM CRC" );
	index = this->combBackup->SelectedIndex;
	// �o�b�N�A�b�v������
	this->gboxProd->Text   = gcnew System::String( "ROM Production Info." );
	this->labBackup->Text  = gcnew System::String( LANG_BACKUP_E );
	this->combBackup->Items->Clear();
	this->combBackup->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"4Kbit EEPROM", L"64Kbit EEPROM", L"512Kbit EEPROM", 
		L"256Kbit FRAM", L"2Mbit FLASH", L"4Mbit FLASH", L"8Mbit FLASH", L"Nothing", L"Other"});
	this->combBackup->SelectedIndex = index;

	// ��o���
	this->labProductName->Text = gcnew System::String( LANG_PRODUCT_NAME_E );
	this->labProductCode->Text = gcnew System::String( LANG_PRODUCT_CODE_E );
	this->labReleaseDate->Text = gcnew System::String( LANG_RELEASE_DATE_E );
	this->labSubmiteDate->Text = gcnew System::String( LANG_SUBMIT_DATE_E );
	this->gboxSubmitWay->Text  = gcnew System::String( LANG_SUBMIT_WAY_E );
	this->rSubmitPost->Text    = gcnew System::String( LANG_SUBMIT_POST_E );
	this->rSubmitHand->Text    = gcnew System::String( LANG_SUBMIT_HAND_E );
	this->gboxUsage->Text      = gcnew System::String( LANG_USAGE_E );
	this->rUsageSale->Text     = gcnew System::String( LANG_USAGE_SALE_E );
	this->rUsageSample->Text   = gcnew System::String( LANG_USAGE_SAMPLE_E );
	this->rUsageDst->Text      = gcnew System::String( LANG_USAGE_DST_E );
	this->rUsageOther->Text    = gcnew System::String( LANG_USAGE_OTHER_E );
	this->labSubmitVer->Text     = gcnew System::String( LANG_SUBMIT_VER_E );
	this->labCapSubmitVer->Text  = gcnew System::String( LANG_SUBMIT_VER_CAP_E );
	this->gboxForeign->Text      = gcnew System::String( LANG_F_E );
	this->labProductNameForeign->Text = gcnew System::String( LANG_PRODUCT_NAME_F_E );
	this->labProductCodeForeign->Text = gcnew System::String( LANG_PRODUCT_CODE_F_E );
	this->cboxReleaseForeign->Text    = gcnew System::String( LANG_RELEASE_F_E );
	this->labMultiForeign1->Text      = gcnew System::String( LANG_MULTI_F_E );
	this->labMultiForeign2->Text      = gcnew System::String( LANG_MULTI_F_E );
	this->labCaption->Text    = gcnew System::String( LANG_CAPTION_E );
	this->labProductNameLimit->Text = gcnew System::String( LANG_PRODUCT_LIMIT_E );
	this->labProductNameLimitForeign->Text = gcnew System::String( LANG_PRODUCT_LIMIT_E );

	// ��Џ��
	this->gboxPerson1->Text    = gcnew System::String( LANG_PERSON_1_E );
	this->gboxPerson2->Text    = gcnew System::String( LANG_PERSON_2_E );
	this->cboxIsInputPerson2->Text = gcnew System::String( LANG_INPUT_PERSON_2_E );
	this->labCompany1->Text    = gcnew System::String( LANG_COMPANY_E );
	this->labDepart1->Text     = gcnew System::String( LANG_DEPART_E );
	this->labPerson1->Text     = gcnew System::String( LANG_PERSON_E );
	this->labCompany2->Text    = gcnew System::String( LANG_COMPANY_E );
	this->labDepart2->Text     = gcnew System::String( LANG_DEPART_E );
	this->labPerson2->Text     = gcnew System::String( LANG_PERSON_E );
	this->labArbit1->Text      = gcnew System::String( "(Arbitrary)" );
	this->labArbit2->Text      = gcnew System::String( "(Arbitrary)" );
	this->labArbit3->Text      = gcnew System::String( "(Arbitrary)" );
	this->labArbit4->Text      = gcnew System::String( "(Arbitrary)" );
	// �ӂ肪�ȏ����폜
	this->tboxFurigana1->Clear();
	this->tboxFurigana1->Enabled = false;
	this->labFurigana1->Text = gcnew System::String( LANG_FURIGANA_E );
	this->tboxFurigana2->Clear();
	this->tboxFurigana2->Enabled = false;
	this->labFurigana2->Text = gcnew System::String( LANG_FURIGANA_E );
	this->tboxNTSC1->Enabled = false;
	this->tboxNTSC1->Text    = gcnew System::String("");
	this->tboxNTSC2->Enabled = false;
	this->tboxNTSC2->Text    = gcnew System::String("");
	this->labNTSC1Pre->Text  = gcnew System::String( LANG_NTSC_1_E );
	this->labNTSC1Sur->Text  = gcnew System::String( LANG_NTSC_2_E );
	this->labNTSC2Pre->Text  = gcnew System::String( LANG_NTSC_1_E );
	this->labNTSC2Sur->Text  = gcnew System::String( LANG_NTSC_2_E );

	// TWL�d�l
	this->gboxTWLExInfo->Text         = gcnew System::String( "TWL Extended Info" );
	this->labNormalRomOffset->Text    = gcnew System::String( "TWL Normal Area ROM Offset" );
	this->labKeyTableRomOffset->Text  = gcnew System::String( "TWL Secure Area ROM Offset" );
	this->cboxIsNormalJump->Text      = gcnew System::String( "Enable Normal App. Jump" );
	this->cboxIsTmpJump->Text         = gcnew System::String( "Enable Temp. App. Jump" );
	this->cboxIsSubBanner->Text       = gcnew System::String( "Enable SubBanner File" );
	this->cboxIsWL->Text              = gcnew System::String( "Enable NTR WhiteList Signature" );
	this->gboxAccess->Text            = gcnew System::String( "Access Control" );
	this->cboxIsSD->Text              = gcnew System::String( "SD Card" );
	this->cboxIsNAND->Text            = gcnew System::String( "NAND Flash Memory" );
	this->labIsGameCardOn->Text       = gcnew System::String( "Card Power" );
	this->labAccessOther->Text        = gcnew System::String( "Others" );
	this->gboxShared2Size->Text       = gcnew System::String( "Size of Shared2 Files" );
	this->cboxIsShared2->Text         = gcnew System::String( "Use Shared2 Files" );
	this->labSDK->Text                = gcnew System::String( "SDK Ver." );
	this->labLib->Text                = gcnew System::String( "Libraries used by the program" );
	this->labCaptionEx->Text          = gcnew System::String( "Special Note" );

	// SRL�ҏW�\���
	this->gboxEULA->Text         = gcnew System::String( LANG_BOX_EULA_E );
	this->cboxIsEULA->Text       = gcnew System::String( LANG_EULA_E );
	this->gboxIcon->Text         = gcnew System::String( LANG_ICON_E );
	this->rIsWirelessIcon->Text  = gcnew System::String( LANG_WIRELESS_ICON_E );
	this->rIsWiFiIcon->Text      = gcnew System::String( LANG_WIFI_ICON_E );
	this->rIsNoIcon->Text        = gcnew System::String( LANG_NO_ICON_E );
	this->labRegion->Text        = gcnew System::String( LANG_REGION_E );

	// ���[�W����
	index = this->combRegion->SelectedIndex;
	this->combRegion->Items->Clear();
	this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5)
		{L"Japan Only", L"USA Only", L"Europe Only", L"Australia Only", L"Europe and Australia"});
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	this->combRegion->Items->Add( gcnew System::String( L"All Region" ) );
#endif
	this->combRegion->SelectedIndex = index;

	//// �y�A�����^���R���g���[��
	this->gboxParental->Text           = gcnew System::String( LANG_REGION_PCTL_E );
	this->labParentalRating->Text      = gcnew System::String( LANG_PCTL_RATING_E );

	index = this->combCERO->SelectedIndex;	// ��������clear����ƌ��݂�index�ɈӖ����Ȃ��Ȃ�̂őޔ�
	this->combCERO->Items->Clear();
	this->combCERO->Items->AddRange(gcnew cli::array< System::Object^  >(6)
		{L"A (All ages)", L"B (aged 12 or older)", L"C (aged 15 or older)", L"D (aged 17 or older)", L"Z (aged 18 or older)", L"Rating Pending"});
	this->combCERO->SelectedIndex = index;

	index = this->combESRB->SelectedIndex;
	this->combESRB->Items->Clear();
	this->combESRB->Items->AddRange(gcnew cli::array< System::Object^  >(7) 
		{L"All ages", L"EC (aged 3 or older)", L"E (aged 6 or older)", L"E10+ (aged 10 or older)", L"T (aged 13 or older)",	L"M (aged 17 or older)", L"Rating Pending"});
	this->combESRB->SelectedIndex = index;

	index = this->combUSK->SelectedIndex;
	this->combUSK->Items->Clear();
	this->combUSK->Items->AddRange(gcnew cli::array< System::Object^  >(6)
		{L"All ages", L"aged 6 or older", L"aged 12 or older", L"aged 16 or older", L"Inadequent for young", L"Rating Pending"});
	this->combUSK->SelectedIndex = index;

	index = this->combPEGI->SelectedIndex;
	this->combPEGI->Items->Clear();
	this->combPEGI->Items->AddRange(gcnew cli::array< System::Object^  >(7)
		{L"All ages", L"aged 3 or older", L"aged 7 or older", L"aged 12 or older", L"aged 16 or older", L"aged 18 or older", L"Rating Pending"});
	this->combPEGI->SelectedIndex = index;

	index = this->combPEGI_PRT->SelectedIndex;
	this->combPEGI_PRT->Items->Clear();
	this->combPEGI_PRT->Items->AddRange(gcnew cli::array< System::Object^  >(7)
		{L"All ages", L"aged 4 or older", L"aged 6 or older", L"aged 12 or older", L"aged 16 or older", L"aged 18 or older", L"Rating Pending"});
	this->combPEGI_PRT->SelectedIndex = index;

	index = this->combPEGI_BBFC->SelectedIndex;
	this->combPEGI_BBFC->Items->Clear();
	this->combPEGI_BBFC->Items->AddRange(gcnew cli::array< System::Object^  >(10)
		{L"All ages", L"aged 3 or older", L"aged 4 or older recommended", L"aged 7 or older", L"aged 8 or older recommended",
		 L"aged 12 or older", L"aged 15 or older", L"aged 16 or older", L"aged 18 or older", L"Rating Pending"});
	this->combPEGI_BBFC->SelectedIndex = index;

	index = this->combOFLC->SelectedIndex;
	this->combOFLC->Items->Clear();
	this->combOFLC->Items->AddRange(gcnew cli::array< System::Object^  >(5)
		{L"G", L"PG", L"M", L"MA15+", L"Rating Pending"});
	this->combOFLC->SelectedIndex = index;

	// �G���[���
	this->labError->Text = gcnew System::String( "Error Information(Modification is necessary.)" );
	this->colErrorName->HeaderText  = gcnew System::String( "Name" );
	this->colErrorBegin->HeaderText = gcnew System::String( "Begin" );
	this->colErrorEnd->HeaderText   = gcnew System::String( "End" );
	this->colErrorCause->HeaderText = gcnew System::String( "Reason" );

	this->labWarn->Text  = gcnew System::String( "Warning(Modification is not necessary. Please check validity of these information.)" );
	this->colWarnName->HeaderText  = gcnew System::String( "Name" );
	this->colWarnBegin->HeaderText = gcnew System::String( "Begin" );
	this->colWarnEnd->HeaderText   = gcnew System::String( "End" );
	this->colWarnCause->HeaderText = gcnew System::String( "Reason" );

	this->gboxErrorTiming->Text = gcnew System::String( "Error Of Timing" );
	this->rErrorReading->Text   = gcnew System::String( "When ROM data was read" );
	this->rErrorCurrent->Text   = gcnew System::String( "In current settings" );

	// ����Ȑݒ�p�̃e�L�X�g�{�b�N�X�̕\�L��ύX
	this->setSrlFormsCaptionEx();
}

// end of file