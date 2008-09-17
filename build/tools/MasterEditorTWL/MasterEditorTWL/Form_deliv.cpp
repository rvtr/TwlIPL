// ----------------------------------------------
// マスタ書類情報(SRL影響なし)をフォームから取得
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

void Form1::setDeliverableProperties(void)
{
	// 提出情報
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
		if( !System::String::IsNullOrEmpty( this->tboxProductCode2Foreign3->Text ) )
		{
			this->hDeliv->hProductCode2Foreign += ("/" + this->tboxProductCode2Foreign3->Text);
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
	// SDK
	this->hDeliv->hSDK = nullptr;
	if( this->hSrl->hSDKList )
	{
		for each( RCSDKVersion ^sdk in this->hSrl->hSDKList )	// 書類には ARM9 static のバージョン情報を記入する
		{
			if( sdk->IsStatic )
				this->hDeliv->hSDK = sdk->Version;
		}
	}
	if( this->hDeliv->hSDK == nullptr )
	{
		this->hDeliv->hSDK = gcnew System::String( "" );
	}
	// 提出方法
	if( this->rSubmitPost->Checked == true )
	{
		this->hDeliv->hSubmitWay = this->rSubmitPost->Text;
	}
	else
	{
		this->hDeliv->hSubmitWay = this->rSubmitHand->Text;
	}
	// 用途
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

	// 会社情報
	this->hDeliv->hCompany1    = this->tboxCompany1->Text + " " + this->tboxDepart1->Text;
	this->hDeliv->hPerson1     = this->tboxPerson1->Text;
	if( this->stripItemJapanese->Checked == true )
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
	if( this->cboxIsInputPerson2->Checked == true )
	{
		this->hDeliv->hCompany2    = this->tboxCompany2->Text + " " + this->tboxDepart2->Text;
		this->hDeliv->hPerson2     = this->tboxPerson2->Text;
		if( this->stripItemJapanese->Checked == true )
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
		this->hDeliv->hPerson2     = nullptr;
		this->hDeliv->hFurigana2   = nullptr;
		this->hDeliv->hTel2        = nullptr;
		this->hDeliv->hFax2        = nullptr;
		this->hDeliv->hMail2       = nullptr;
		this->hDeliv->hNTSC2       = nullptr;
	}

	// 備考
	System::String ^tmp = this->tboxCaption->Text->Replace( " ", "" );
	if( this->tboxCaption->Text->Equals("") || tmp->Equals("") )	// スペースのみの文字列は含めない
	{
		this->hDeliv->hCaption = nullptr;
	}
	else
	{
		this->hDeliv->hCaption = this->tboxCaption->Text;
	}
	// 備考欄に特殊な設定を追記
	tmp = this->tboxCaptionEx->Text->Replace( " ", "" );
	if( !(this->tboxCaptionEx->Text->Equals("")) && !(tmp->Equals("")) )
	{
		this->hDeliv->hCaption += ("(" + this->tboxCaptionEx->Text + ")");
	}

	// 特殊なアプリ種別
	this->hDeliv->hAppTypeOther = this->tboxAppTypeOther->Text;
	if( this->hDeliv->hAppTypeOther != nullptr )
	{
		this->hDeliv->hAppTypeOther->Replace("\r\n","");
	}

	// 一部のROM情報を登録
	if( this->combBackup->SelectedIndex != (this->combBackup->Items->Count - 1) )
	{
		if( this->combBackup->SelectedIndex > 0 )
		{
			this->hDeliv->hBackupMemory = this->combBackup->SelectedItem->ToString();
		}
		else
		{
			this->hDeliv->hBackupMemory = gcnew System::String("");
		}
	}
	else
	{
		this->hDeliv->hBackupMemory = this->tboxBackupOther->Text;
	}

	// SRL情報を文字列で登録
	if( this->combCERO->SelectedIndex < 0 )
	{
		if( this->stripItemJapanese->Checked == true )
			this->hDeliv->hCERO = gcnew System::String("不可");
		else
			this->hDeliv->hCERO = gcnew System::String("Undefined");
	}
	else
	{
		this->hDeliv->hCERO = dynamic_cast<System::String^>(this->combCERO->SelectedItem);
	}
	if( this->combESRB->SelectedIndex < 0 )
	{
		if( this->stripItemJapanese->Checked == true )
			this->hDeliv->hESRB = gcnew System::String("不可");
		else
			this->hDeliv->hESRB = gcnew System::String("Undefined");
	}
	else
	{
		this->hDeliv->hESRB = dynamic_cast<System::String^>(this->combESRB->SelectedItem);
	}
	if( this->combUSK->SelectedIndex < 0 )
	{
		if( this->stripItemJapanese->Checked == true )
			this->hDeliv->hUSK = gcnew System::String("不可");
		else
			this->hDeliv->hUSK = gcnew System::String("Undefined");
	}
	else
	{
		this->hDeliv->hUSK = dynamic_cast<System::String^>(this->combUSK->SelectedItem);
	}
	if( this->combPEGI->SelectedIndex < 0 )
	{
		if( this->stripItemJapanese->Checked == true )
			this->hDeliv->hPEGI = gcnew System::String("不可");
		else
			this->hDeliv->hPEGI = gcnew System::String("Undefined");
	}
	else
	{
		this->hDeliv->hPEGI = dynamic_cast<System::String^>(this->combPEGI->SelectedItem);
	}
	if( this->combPEGI_PRT->SelectedIndex < 0 )
	{
		if( this->stripItemJapanese->Checked == true )
			this->hDeliv->hPEGI_PRT = gcnew System::String("不可");
		else
			this->hDeliv->hPEGI_PRT = gcnew System::String("Undefined");
	}
	else
	{
		this->hDeliv->hPEGI_PRT = dynamic_cast<System::String^>(this->combPEGI_PRT->SelectedItem);
	}
	if( this->combPEGI_BBFC->SelectedIndex < 0 )
	{
		if( this->stripItemJapanese->Checked == true )
			this->hDeliv->hPEGI_BBFC = gcnew System::String("不可");
		else
			this->hDeliv->hPEGI_BBFC = gcnew System::String("Undefined");
	}
	else
	{
		this->hDeliv->hPEGI_BBFC = dynamic_cast<System::String^>(this->combPEGI_BBFC->SelectedItem);
	}
	if( this->combOFLC->SelectedIndex < 0 )
	{
		if( this->stripItemJapanese->Checked == true )
			this->hDeliv->hOFLC = gcnew System::String("不可");
		else
			this->hDeliv->hOFLC = gcnew System::String("Undefined");
	}
	else
	{
		this->hDeliv->hOFLC = dynamic_cast<System::String^>(this->combOFLC->SelectedItem);
	}
}

// ----------------------------------------------
// マスタ書類情報(SRL影響なし)のフォームチェック
// ----------------------------------------------

System::Boolean Form1::checkDeliverableForms(void)
{
	// 不正な場合はダイアログで注意してreturn

	// 提出情報
	this->checkTextForm( this->tboxProductName->Text, LANG_PRODUCT_NAME_J, LANG_PRODUCT_NAME_E, false );	// SRL作成には問題のないエラー
	this->checkTextForm( this->tboxProductCode1->Text, LANG_PRODUCT_CODE_J, LANG_PRODUCT_CODE_E, false );
	this->checkTextForm( this->tboxProductCode2->Text, LANG_PRODUCT_CODE_J, LANG_PRODUCT_CODE_E, false );
	if( this->cboxReleaseForeign->Checked == true )
	{
		this->checkTextForm( this->tboxProductNameForeign->Text, LANG_PRODUCT_NAME_F_J, LANG_PRODUCT_NAME_F_E, false );
		this->checkTextForm( this->tboxProductCode1Foreign->Text, LANG_PRODUCT_CODE_F_J, LANG_PRODUCT_CODE_F_E, false );
		this->checkTextForm( this->tboxProductCode2Foreign1->Text, LANG_PRODUCT_CODE_F_J, LANG_PRODUCT_CODE_F_E, false );
	}
	if( this->rUsageOther->Checked == true )
	{
		this->checkTextForm( this->tboxUsageOther->Text, LANG_USAGE_J, LANG_USAGE_E, false );
	}

	// 会社情報
	this->checkTextForm( this->tboxPerson1->Text, LANG_PERSON_J, LANG_PERSON_E, false );
	this->checkTextForm( this->tboxCompany1->Text, LANG_COMPANY_J, LANG_COMPANY_E, false );
	this->checkTextForm( this->tboxDepart1->Text, LANG_DEPART_J, LANG_DEPART_E, false );
	if( this->stripItemJapanese->Checked == true )
	{
		this->checkTextForm( this->tboxFurigana1->Text, LANG_FURIGANA_J, LANG_FURIGANA_J, false );
	}
	this->checkTextForm( this->tboxTel1->Text, LANG_TEL_J, LANG_TEL_E, false );
	//this->checkTextForm( this->tboxFax1->Text, LANG_FAX_J, LANG_FAX_E, false );
	this->checkTextForm( this->tboxMail1->Text, LANG_MAIL_J, LANG_MAIL_E, false );
	//if( this->stripItemJapanese->Checked == true )
	//{
	//	this->checkTextForm( this->tboxNTSC1->Text, LANG_NTSC_1_J + " " + LANG_NTSC_2_J, LANG_NTSC_1_J + " " + LANG_NTSC_2_J, false );
	//}

	if( this->cboxIsInputPerson2->Checked == true )
	{
		this->checkTextForm( this->tboxPerson2->Text, LANG_PERSON_J, LANG_PERSON_E, false );
		this->checkTextForm( this->tboxCompany2->Text, LANG_COMPANY_J, LANG_COMPANY_E, false );
		this->checkTextForm( this->tboxDepart2->Text, LANG_DEPART_J, LANG_DEPART_E, false );
		if( this->stripItemJapanese->Checked == true )
		{
			this->checkTextForm( this->tboxFurigana2->Text, LANG_FURIGANA_J, LANG_FURIGANA_J, false );
		}
		this->checkTextForm( this->tboxTel2->Text, LANG_TEL_J, LANG_TEL_E, false );
		//this->checkTextForm( this->tboxFax2->Text, LANG_FAX_J, LANG_FAX_E, false );
		this->checkTextForm( this->tboxMail2->Text, LANG_MAIL_J, LANG_MAIL_E, false );
		//if( this->stripItemJapanese->Checked == true )
		//{
		//	this->checkTextForm( this->tboxNTSC2->Text, LANG_NTSC_1_J + " " + LANG_NTSC_2_J, LANG_NTSC_1_J + " " + LANG_NTSC_2_J, false );
		//}
	}

	// 一部のROM情報(SRLバイナリに反映されない情報)をここでチェックする
	this->checkBoxIndex( this->combBackup, LANG_BACKUP_J, LANG_BACKUP_E, false );
	if( this->combBackup->SelectedIndex == (this->combBackup->Items->Count - 1) )
	{
		this->checkTextForm( this->tboxBackupOther->Text, LANG_BACKUP_J, LANG_BACKUP_E, false );
	}

	// ひととおりエラー登録をした後で
	// 書類上のエラー(SRLバイナリには影響しない)が存在するかチェック
	return this->isValidOnlyDeliverable();
}

// end of file