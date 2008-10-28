// ----------------------------------------------
// 日英両対応
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

// 日本語版と英語版でテキストボックスの文字列制限が変わる
void Form1::changeMaxLength( System::Windows::Forms::TextBox ^tbox, System::Int32 maxlen )
{
	if( tbox->Text->Length > maxlen )
		tbox->Text = "";

	tbox->MaxLength = maxlen;
}

// 日本語版への切り替え
void Form1::changeJapanese(void)
{
	// 入力文字数制限を変更する
	this->changeMaxLength( this->tboxCompany1, 25 );
	this->changeMaxLength( this->tboxDepart1,  25 );
	this->changeMaxLength( this->tboxPerson1,  15 );

	this->changeLanguage( "ja" );

	// 特殊な設定用のテキストボックスの表記を変更
	this->setSrlFormsCaptionEx();

	// 複数行表示の改行を挿入
	this->tboxGuideRomEditInfo->Text = this->tboxGuideRomEditInfo->Text->Replace( "<newline>", "\r\n" );
	this->tboxGuideErrorInfo->Text   = this->tboxGuideErrorInfo->Text->Replace( "<newline>", "\r\n" );
}

// 英語版への切り替え
void  Form1::changeEnglish(void)
{
	this->changeMaxLength( this->tboxCompany1, 40 );
	this->changeMaxLength( this->tboxDepart1,  40 );
	this->changeMaxLength( this->tboxPerson1,  30 );

	this->changeLanguage( "en" );

	this->setSrlFormsCaptionEx();

	this->tboxGuideRomEditInfo->Text = this->tboxGuideRomEditInfo->Text->Replace( "<newline>", "\r\n" );
	this->tboxGuideErrorInfo->Text   = this->tboxGuideErrorInfo->Text->Replace( "<newline>", "\r\n" );
}

// 言語リソース切り替え
void MasterEditorTWL::Form1::changeLanguage( System::String ^langname )
{
	int  index;

	System::Threading::Thread::CurrentThread->CurrentUICulture = gcnew System::Globalization::CultureInfo(langname,true);
	System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));

	resources->ApplyResources(this->tboxFile, L"tboxFile");
	resources->ApplyResources(this->gboxSrl, L"gboxSrl");
	resources->ApplyResources(this->tboxRemasterVer, L"tboxRemasterVer");
	resources->ApplyResources(this->tboxRomSize, L"tboxRomSize");
	resources->ApplyResources(this->tboxPlatform, L"tboxPlatform");
	resources->ApplyResources(this->labPlatform, L"labPlatform");
	resources->ApplyResources(this->tboxRomLatency, L"tboxRomLatency");
	resources->ApplyResources(this->labRomSize, L"labRomSize");
	resources->ApplyResources(this->labRomType, L"labRomType");
	resources->ApplyResources(this->tboxMakerCode, L"tboxMakerCode");
	resources->ApplyResources(this->cboxRemasterVerE, L"cboxRemasterVerE");
	resources->ApplyResources(this->labMakerCode, L"labMakerCode");
	resources->ApplyResources(this->labGameCode, L"labGameCode");
	resources->ApplyResources(this->tboxGameCode, L"tboxGameCode");
	resources->ApplyResources(this->labTitleName, L"labTitleName");
	resources->ApplyResources(this->labRemasterVer, L"labRemasterVer");
	resources->ApplyResources(this->tboxTitleName, L"tboxTitleName");
	resources->ApplyResources(this->labBackup, L"labBackup");

	index = this->combBackup->SelectedIndex;
	this->combBackup->Items->Clear();
	resources->ApplyResources(this->combBackup, L"combBackup");
	this->combBackup->Items->AddRange(gcnew cli::array< System::Object^  >(9) {resources->GetString(L"combBackup.Items"), resources->GetString(L"combBackup.Items1"), 
		resources->GetString(L"combBackup.Items2"), resources->GetString(L"combBackup.Items3"), resources->GetString(L"combBackup.Items4"), 
		resources->GetString(L"combBackup.Items5"), resources->GetString(L"combBackup.Items6"), resources->GetString(L"combBackup.Items7"), 
		resources->GetString(L"combBackup.Items8")});
	this->combBackup->SelectedIndex = index;

	resources->ApplyResources(this->tboxBackupOther, L"tboxBackupOther");
	resources->ApplyResources(this->gboxCRC, L"gboxCRC");
	resources->ApplyResources(this->labRomCRC, L"labRomCRC");
	resources->ApplyResources(this->labHeaderCRC, L"labHeaderCRC");
	resources->ApplyResources(this->tboxHeaderCRC, L"tboxHeaderCRC");
	resources->ApplyResources(this->tboxWholeCRC, L"tboxWholeCRC");
	resources->ApplyResources(this->labCaption, L"labCaption");
	resources->ApplyResources(this->tboxCaption, L"tboxCaption");
	resources->ApplyResources(this->labOFLC, L"labOFLC");
	resources->ApplyResources(this->labPEGI_BBFC, L"labPEGI_BBFC");
	resources->ApplyResources(this->labPEGI_PRT, L"labPEGI_PRT");
	resources->ApplyResources(this->labPEGI, L"labPEGI");
	resources->ApplyResources(this->labUSK, L"labUSK");
	resources->ApplyResources(this->labESRB, L"labESRB");
	resources->ApplyResources(this->labCERO, L"labCERO");

	resources->ApplyResources(this->combOFLC, L"combOFLC");
	index = this->combOFLC->SelectedIndex;
	this->combOFLC->Items->Clear();
	this->combOFLC->Items->AddRange(gcnew cli::array< System::Object^  >(5) {resources->GetString(L"combOFLC.Items"), resources->GetString(L"combOFLC.Items1"), 
		resources->GetString(L"combOFLC.Items2"), resources->GetString(L"combOFLC.Items3"), resources->GetString(L"combOFLC.Items4")});
	this->combOFLC->SelectedIndex = index;

	resources->ApplyResources(this->combPEGI_BBFC, L"combPEGI_BBFC");
	index = this->combPEGI_BBFC->SelectedIndex;
	this->combPEGI_BBFC->Items->Clear();
	this->combPEGI_BBFC->Items->AddRange(gcnew cli::array< System::Object^  >(10) {resources->GetString(L"combPEGI_BBFC.Items"), 
		resources->GetString(L"combPEGI_BBFC.Items1"), resources->GetString(L"combPEGI_BBFC.Items2"), resources->GetString(L"combPEGI_BBFC.Items3"), 
		resources->GetString(L"combPEGI_BBFC.Items4"), resources->GetString(L"combPEGI_BBFC.Items5"), resources->GetString(L"combPEGI_BBFC.Items6"), 
		resources->GetString(L"combPEGI_BBFC.Items7"), resources->GetString(L"combPEGI_BBFC.Items8"), resources->GetString(L"combPEGI_BBFC.Items9")});
	this->combPEGI_BBFC->SelectedIndex = index;

	resources->ApplyResources(this->combPEGI_PRT, L"combPEGI_PRT");
	index = this->combPEGI_PRT->SelectedIndex;
	this->combPEGI_PRT->Items->Clear();
	this->combPEGI_PRT->Items->AddRange(gcnew cli::array< System::Object^  >(7) {resources->GetString(L"combPEGI_PRT.Items"), 
		resources->GetString(L"combPEGI_PRT.Items1"), resources->GetString(L"combPEGI_PRT.Items2"), resources->GetString(L"combPEGI_PRT.Items3"), 
		resources->GetString(L"combPEGI_PRT.Items4"), resources->GetString(L"combPEGI_PRT.Items5"), resources->GetString(L"combPEGI_PRT.Items6")});
	this->combPEGI_PRT->SelectedIndex = index;

	resources->ApplyResources(this->combPEGI, L"combPEGI");
	index = this->combPEGI->SelectedIndex;
	this->combPEGI->Items->Clear();
	this->combPEGI->Items->AddRange(gcnew cli::array< System::Object^  >(7) {resources->GetString(L"combPEGI.Items"), resources->GetString(L"combPEGI.Items1"), 
		resources->GetString(L"combPEGI.Items2"), resources->GetString(L"combPEGI.Items3"), resources->GetString(L"combPEGI.Items4"), 
		resources->GetString(L"combPEGI.Items5"), resources->GetString(L"combPEGI.Items6")});
	this->combPEGI->SelectedIndex = index;

	resources->ApplyResources(this->combUSK, L"combUSK");
	index = this->combUSK->SelectedIndex;
	this->combUSK->Items->Clear();
	this->combUSK->Items->AddRange(gcnew cli::array< System::Object^  >(6) {resources->GetString(L"combUSK.Items"), resources->GetString(L"combUSK.Items1"), 
		resources->GetString(L"combUSK.Items2"), resources->GetString(L"combUSK.Items3"), resources->GetString(L"combUSK.Items4"), resources->GetString(L"combUSK.Items5")});
	this->combUSK->SelectedIndex = index;

	resources->ApplyResources(this->combESRB, L"combESRB");
	index = this->combESRB->SelectedIndex;
	this->combESRB->Items->Clear();
	this->combESRB->Items->AddRange(gcnew cli::array< System::Object^  >(7) {resources->GetString(L"combESRB.Items"), resources->GetString(L"combESRB.Items1"), 
		resources->GetString(L"combESRB.Items2"), resources->GetString(L"combESRB.Items3"), resources->GetString(L"combESRB.Items4"), 
		resources->GetString(L"combESRB.Items5"), resources->GetString(L"combESRB.Items6")});
	this->combESRB->SelectedIndex = index;

	resources->ApplyResources(this->combCERO, L"combCERO");
	index = this->combCERO->SelectedIndex;
	this->combCERO->Items->Clear();
	this->combCERO->Items->AddRange(gcnew cli::array< System::Object^  >(6) {resources->GetString(L"combCERO.Items"), resources->GetString(L"combCERO.Items1"), 
		resources->GetString(L"combCERO.Items2"), resources->GetString(L"combCERO.Items3"), resources->GetString(L"combCERO.Items4"), 
		resources->GetString(L"combCERO.Items5")});
	this->combCERO->SelectedIndex = index;

	resources->ApplyResources(this->labParentalRating, L"labParentalRating");
	resources->ApplyResources(this->labRegion, L"labRegion");
	resources->ApplyResources(this->cboxIsEULA, L"cboxIsEULA");

	resources->ApplyResources(this->combRegion, L"combRegion");
	index = this->combRegion->SelectedIndex;
	this->combRegion->Items->Clear();
	this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5) {resources->GetString(L"combRegion.Items"), resources->GetString(L"combRegion.Items1"), 
		resources->GetString(L"combRegion.Items2"), resources->GetString(L"combRegion.Items3"), resources->GetString(L"combRegion.Items4")});
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	if( langname->Equals( "ja" ) )
	{
		this->combRegion->Items->Add( gcnew System::String( L"全リージョン" ) );
	}
	else
	{
		this->combRegion->Items->Add( gcnew System::String( L"All Region" ) );
	}
#endif
	this->combRegion->SelectedIndex = index;

	resources->ApplyResources(this->cboxIsInputPerson2, L"cboxIsInputPerson2");
	resources->ApplyResources(this->gboxPerson2, L"gboxPerson2");
	resources->ApplyResources(this->labArbit4, L"labArbit4");
	resources->ApplyResources(this->labArbit3, L"labArbit3");
	resources->ApplyResources(this->labNTSC2Sur, L"labNTSC2Sur");
	resources->ApplyResources(this->tboxNTSC2, L"tboxNTSC2");
	resources->ApplyResources(this->labFax2, L"labFax2");
	resources->ApplyResources(this->labNTSC2Pre, L"labNTSC2Pre");
	resources->ApplyResources(this->tboxFax2, L"tboxFax2");
	resources->ApplyResources(this->tboxMail2, L"tboxMail2");
	resources->ApplyResources(this->tboxTel2, L"tboxTel2");
	resources->ApplyResources(this->tboxFurigana2, L"tboxFurigana2");
	resources->ApplyResources(this->tboxPerson2, L"tboxPerson2");
	resources->ApplyResources(this->tboxDepart2, L"tboxDepart2");
	resources->ApplyResources(this->labDepart2, L"labDepart2");
	resources->ApplyResources(this->tboxCompany2, L"tboxCompany2");
	resources->ApplyResources(this->labMail2, L"labMail2");
	resources->ApplyResources(this->labTel2, L"labTel2");
	resources->ApplyResources(this->labFurigana2, L"labFurigana2");
	resources->ApplyResources(this->labPerson2, L"labPerson2");
	resources->ApplyResources(this->labCompany2, L"labCompany2");
	resources->ApplyResources(this->gboxPerson1, L"gboxPerson1");
	resources->ApplyResources(this->labArbit2, L"labArbit2");
	resources->ApplyResources(this->labArbit1, L"labArbit1");
	resources->ApplyResources(this->labNTSC1Sur, L"labNTSC1Sur");
	resources->ApplyResources(this->labFax1, L"labFax1");
	resources->ApplyResources(this->labNTSC1Pre, L"labNTSC1Pre");
	resources->ApplyResources(this->tboxNTSC1, L"tboxNTSC1");
	resources->ApplyResources(this->tboxFax1, L"tboxFax1");
	resources->ApplyResources(this->tboxMail1, L"tboxMail1");
	resources->ApplyResources(this->tboxTel1, L"tboxTel1");
	resources->ApplyResources(this->tboxFurigana1, L"tboxFurigana1");
	resources->ApplyResources(this->tboxPerson1, L"tboxPerson1");
	resources->ApplyResources(this->tboxDepart1, L"tboxDepart1");
	resources->ApplyResources(this->labDepart1, L"labDepart1");
	resources->ApplyResources(this->tboxCompany1, L"tboxCompany1");
	resources->ApplyResources(this->labMail1, L"labMail1");
	resources->ApplyResources(this->labTel1, L"labTel1");
	resources->ApplyResources(this->labFurigana1, L"labFurigana1");
	resources->ApplyResources(this->labPerson1, L"labPerson1");
	resources->ApplyResources(this->labCompany1, L"labCompany1");
	resources->ApplyResources(this->tboxProductCode2, L"tboxProductCode2");
	resources->ApplyResources(this->tboxProductCode1, L"tboxProductCode1");
	resources->ApplyResources(this->tboxProductName, L"tboxProductName");
	resources->ApplyResources(this->labProductCode2, L"labProductCode2");
	resources->ApplyResources(this->labProductCode1, L"labProductCode1");
	resources->ApplyResources(this->dateSubmit, L"dateSubmit");
	resources->ApplyResources(this->dateRelease, L"dateRelease");
	resources->ApplyResources(this->gboxUsage, L"gboxUsage");
	resources->ApplyResources(this->tboxUsageOther, L"tboxUsageOther");
	resources->ApplyResources(this->rUsageOther, L"rUsageOther");
	resources->ApplyResources(this->rUsageDst, L"rUsageDst");
	resources->ApplyResources(this->rUsageSample, L"rUsageSample");
	resources->ApplyResources(this->rUsageSale, L"rUsageSale");
	resources->ApplyResources(this->gboxSubmitWay, L"gboxSubmitWay");
	resources->ApplyResources(this->rSubmitHand, L"rSubmitHand");
	resources->ApplyResources(this->rSubmitPost, L"rSubmitPost");
	resources->ApplyResources(this->labSubmiteDate, L"labSubmiteDate");
	resources->ApplyResources(this->labReleaseDate, L"labReleaseDate");
	resources->ApplyResources(this->labProductCode, L"labProductCode");
	resources->ApplyResources(this->labProductName, L"labProductName");
	resources->ApplyResources(this->labCapSubmitVer, L"labCapSubmitVer");
	resources->ApplyResources(this->numSubmitVersion, L"numSubmitVersion");
	resources->ApplyResources(this->labSubmitVer, L"labSubmitVer");
	resources->ApplyResources(this->labMultiForeign1, L"labMultiForeign1");
	resources->ApplyResources(this->tboxProductCode2Foreign3, L"tboxProductCode2Foreign3");
	resources->ApplyResources(this->tboxProductCode2Foreign2, L"tboxProductCode2Foreign2");
	resources->ApplyResources(this->labProductCode2Foreign, L"labProductCode2Foreign");
	resources->ApplyResources(this->cboxReleaseForeign, L"cboxReleaseForeign");
	resources->ApplyResources(this->labProductNameForeign, L"labProductNameForeign");
	resources->ApplyResources(this->tboxProductNameForeign, L"tboxProductNameForeign");
	resources->ApplyResources(this->labProductCode1Foreign, L"labProductCode1Foreign");
	resources->ApplyResources(this->tboxProductCode1Foreign, L"tboxProductCode1Foreign");
	resources->ApplyResources(this->labProductCodeForeign, L"labProductCodeForeign");
	resources->ApplyResources(this->tboxProductCode2Foreign1, L"tboxProductCode2Foreign1");
	resources->ApplyResources(this->gboxShared2Size, L"gboxShared2Size");
	resources->ApplyResources(this->labShared2Size5, L"labShared2Size5");
	resources->ApplyResources(this->labShared2Size4, L"labShared2Size4");
	resources->ApplyResources(this->labShared2Size3, L"labShared2Size3");
	resources->ApplyResources(this->labShared2Size2, L"labShared2Size2");
	resources->ApplyResources(this->labShared2Size1, L"labShared2Size1");
	resources->ApplyResources(this->labShared2Size0, L"labShared2Size0");
	resources->ApplyResources(this->tboxShared2Size5, L"tboxShared2Size5");
	resources->ApplyResources(this->tboxShared2Size4, L"tboxShared2Size4");
	resources->ApplyResources(this->tboxShared2Size3, L"tboxShared2Size3");
	resources->ApplyResources(this->tboxShared2Size2, L"tboxShared2Size2");
	resources->ApplyResources(this->tboxShared2Size1, L"tboxShared2Size1");
	resources->ApplyResources(this->tboxShared2Size0, L"tboxShared2Size0");
	resources->ApplyResources(this->cboxIsShared2, L"cboxIsShared2");
	resources->ApplyResources(this->labLib, L"labLib");
	resources->ApplyResources(this->tboxSDK, L"tboxSDK");
	resources->ApplyResources(this->labSDK, L"labSDK");
	resources->ApplyResources(this->gboxTWLExInfo, L"gboxTWLExInfo");
	resources->ApplyResources(this->labByte1, L"labByte1");
	resources->ApplyResources(this->labHex4, L"labHex4");
	resources->ApplyResources(this->labHex3, L"labHex3");
	resources->ApplyResources(this->tboxIsCodec, L"tboxIsCodec");
	resources->ApplyResources(this->labIsCodec, L"labIsCodec");
	resources->ApplyResources(this->labNormalRomOffset, L"labNormalRomOffset");
	resources->ApplyResources(this->tboxNormalRomOffset, L"tboxNormalRomOffset");
	resources->ApplyResources(this->labKeyTableRomOffset, L"labKeyTableRomOffset");
	resources->ApplyResources(this->tboxPrivateSize, L"tboxPrivateSize");
	resources->ApplyResources(this->labPrivateSize, L"labPrivateSize");
	resources->ApplyResources(this->tboxKeyTableRomOffset, L"tboxKeyTableRomOffset");
	resources->ApplyResources(this->labPublicSize, L"labPublicSize");
	resources->ApplyResources(this->tboxPublicSize, L"tboxPublicSize");
	resources->ApplyResources(this->cboxIsSubBanner, L"cboxIsSubBanner");
	resources->ApplyResources(this->cboxIsWL, L"cboxIsWL");
	resources->ApplyResources(this->cboxIsNormalJump, L"cboxIsNormalJump");
	resources->ApplyResources(this->cboxIsTmpJump, L"cboxIsTmpJump");
	resources->ApplyResources(this->gboxAccess, L"gboxAccess");
	resources->ApplyResources(this->labAccessOther, L"labAccessOther");
	resources->ApplyResources(this->tboxAccessOther, L"tboxAccessOther");
	resources->ApplyResources(this->tboxIsGameCardOn, L"tboxIsGameCardOn");
	resources->ApplyResources(this->labIsGameCardOn, L"labIsGameCardOn");
	resources->ApplyResources(this->cboxIsNAND, L"cboxIsNAND");
	resources->ApplyResources(this->cboxIsSD, L"cboxIsSD");
	resources->ApplyResources(this->gboxTitleID, L"gboxTitleID");
	resources->ApplyResources(this->labAppType, L"labAppType");
	resources->ApplyResources(this->labMedia, L"labMedia");
	resources->ApplyResources(this->tboxAppType, L"tboxAppType");
	resources->ApplyResources(this->tboxMedia, L"tboxMedia");
	resources->ApplyResources(this->labHex2, L"labHex2");
	resources->ApplyResources(this->tboxTitleIDLo, L"tboxTitleIDLo");
	resources->ApplyResources(this->labTitleIDLo, L"labTitleIDLo");
	resources->ApplyResources(this->labTitleIDHi, L"labTitleIDHi");
	resources->ApplyResources(this->tboxTitleIDHi, L"tboxTitleIDHi");
	resources->ApplyResources(this->tboxAppTypeOther, L"tboxAppTypeOther");
	resources->ApplyResources(this->labAppTypeOther, L"labAppTypeOther");
	resources->ApplyResources(this->labCaptionEx, L"labCaptionEx");
	resources->ApplyResources(this->tboxCaptionEx, L"tboxCaptionEx");
	resources->ApplyResources(this->gboxProd, L"gboxProd");
	resources->ApplyResources(this->menuStripAbove, L"menuStripAbove");
	resources->ApplyResources(this->stripFile, L"stripFile");
	resources->ApplyResources(this->stripItemOpenRom, L"stripItemOpenRom");
	resources->ApplyResources(this->stripItemSepFile1, L"stripItemSepFile1");
	resources->ApplyResources(this->stripItemSaveTemp, L"stripItemSaveTemp");
	resources->ApplyResources(this->stripItemLoadTemp, L"stripItemLoadTemp");
	resources->ApplyResources(this->stripMaster, L"stripMaster");
	resources->ApplyResources(this->stripItemSheet, L"stripItemSheet");
	resources->ApplyResources(this->stripItemSepMaster1, L"stripItemSepMaster1");
	resources->ApplyResources(this->stripItemMasterRom, L"stripItemMasterRom");
	resources->ApplyResources(this->stripItemMiddlewareXml, L"stripItemMiddlewareXml");
	resources->ApplyResources(this->stripItemMiddlewareHtml, L"stripItemMiddlewareHtml");
	resources->ApplyResources(this->stripLang, L"stripLang");
	resources->ApplyResources(this->stripItemEnglish, L"stripItemEnglish");
	resources->ApplyResources(this->stripItemJapanese, L"stripItemJapanese");
	resources->ApplyResources(this->tabMain, L"tabMain");
	resources->ApplyResources(this->tabRomInfo, L"tabRomInfo");
	resources->ApplyResources(this->gridLibrary, L"gridLibrary");
	resources->ApplyResources(this->colLibPublisher, L"colLibPublisher");
	resources->ApplyResources(this->colLibName, L"colLibName");
	resources->ApplyResources(this->tboxGuideRomInfo, L"tboxGuideRomInfo");
	resources->ApplyResources(this->tabTWLInfo, L"tabTWLInfo");
	resources->ApplyResources(this->tboxGuideTWLInfo, L"tboxGuideTWLInfo");
	resources->ApplyResources(this->gboxExFlags, L"gboxExFlags");
	resources->ApplyResources(this->tabRomEditInfo, L"tabRomEditInfo");
	resources->ApplyResources(this->gboxOtherSpec, L"gboxOtherSpec");
	resources->ApplyResources(this->cboxIsUGC, L"cboxIsUGC");
	resources->ApplyResources(this->cboxIsPhotoEx, L"cboxIsPhotoEx");
	resources->ApplyResources(this->butSetBack, L"butSetBack");
	resources->ApplyResources(this->tboxGuideRomEditInfo, L"tboxGuideRomEditInfo");
	resources->ApplyResources(this->gboxParental, L"gboxParental");
	resources->ApplyResources(this->gboxIcon, L"gboxIcon");
	resources->ApplyResources(this->rIsNoIcon, L"rIsNoIcon");
	resources->ApplyResources(this->rIsWiFiIcon, L"rIsWiFiIcon");
	resources->ApplyResources(this->rIsWirelessIcon, L"rIsWirelessIcon");
	resources->ApplyResources(this->gboxEULA, L"gboxEULA");
	resources->ApplyResources(this->tabSubmitInfo, L"tabSubmitInfo");
	resources->ApplyResources(this->labProductNameLimit, L"labProductNameLimit");
	resources->ApplyResources(this->tboxGuideSubmitInfo, L"tboxGuideSubmitInfo");
	resources->ApplyResources(this->gboxForeign, L"gboxForeign");
	resources->ApplyResources(this->labProductNameLimitForeign, L"labProductNameLimitForeign");
	resources->ApplyResources(this->labMultiForeign2, L"labMultiForeign2");
	resources->ApplyResources(this->tabCompanyInfo, L"tabCompanyInfo");
	resources->ApplyResources(this->tboxGuideCompanyInfo, L"tboxGuideCompanyInfo");
	resources->ApplyResources(this->tabErrorInfo, L"tabErrorInfo");
	resources->ApplyResources(this->tboxGuideErrorInfo, L"tboxGuideErrorInfo");
	resources->ApplyResources(this->gboxErrorTiming, L"gboxErrorTiming");
	resources->ApplyResources(this->rErrorCurrent, L"rErrorCurrent");
	resources->ApplyResources(this->rErrorReading, L"rErrorReading");
	resources->ApplyResources(this->labWarn, L"labWarn");
	resources->ApplyResources(this->labError, L"labError");
	resources->ApplyResources(this->gridWarn, L"gridWarn");
	resources->ApplyResources(this->gridError, L"gridError");
	resources->ApplyResources(this->labFile, L"labFile");
	resources->ApplyResources(this->labAssemblyVersion, L"labAssemblyVersion");
	resources->ApplyResources(this->colErrorName, L"colErrorName");
	resources->ApplyResources(this->colErrorBegin, L"colErrorBegin");
	resources->ApplyResources(this->colErrorEnd, L"colErrorEnd");
	resources->ApplyResources(this->colErrorCause, L"colErrorCause");
	resources->ApplyResources(this->colWarnName, L"colWarnName");
	resources->ApplyResources(this->colWarnBegin, L"colWarnBegin");
	resources->ApplyResources(this->colWarnEnd, L"colWarnEnd");
	resources->ApplyResources(this->colWarnCause, L"colWarnCause");
	resources->ApplyResources(this, L"$this");
}

// end of file