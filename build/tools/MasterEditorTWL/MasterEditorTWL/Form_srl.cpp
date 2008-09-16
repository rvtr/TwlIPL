// ----------------------------------------------
// フォームとSRL内情報とのデータのやりとり
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

// ROM情報をフォームから取得してSRLクラスのプロパティに反映させる
// (ROMヘッダへの反映やCRCと署名の再計算をしない)
void Form1::setSrlProperties(void)
{
	// ROMヘッダの[0,0x160)の領域はRead Onlyで変更しない

	// TWL拡張領域のいくつかの情報をROMヘッダに反映させる
	this->hSrl->hIsEULA         = this->cboxIsEULA->Checked;
	this->hSrl->hIsWiFiIcon     = this->rIsWiFiIcon->Checked;
	this->hSrl->hIsWirelessIcon = this->rIsWirelessIcon->Checked;

	// リージョン
	this->setRegionSrlPropaties();

	// ペアレンタルコントロール
	this->setParentalSrlProperties();
} //setSrlProperties()

// SRLのROM情報をフォームに反映させる(ファイルが読み込まれていることが前提)
void Form1::setSrlForms(void)
{
	// NTR互換情報
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
		this->errMsg( "プラットホーム指定が不正です。ROMデータのビルド設定を見直してください。",
					  "Illegal Platform: Please check build settings of the ROM data.");
	}

	// TWL拡張情報
	this->tboxTitleIDLo->Text = this->hSrl->hTitleIDLo;
	this->tboxTitleIDHi->Text = this->hSrl->hTitleIDHi->ToString("X8");
	this->tboxNormalRomOffset->Text   = this->hSrl->hNormalRomOffset->ToString("X8");
	this->tboxKeyTableRomOffset->Text = this->hSrl->hKeyTableRomOffset->ToString("X8");
	this->tboxPublicSize->Text  = MasterEditorTWL::transSizeToString( this->hSrl->hPublicSize );
	this->tboxPrivateSize->Text = MasterEditorTWL::transSizeToString( this->hSrl->hPrivateSize );
	this->cboxIsNormalJump->Checked = *(this->hSrl->hIsNormalJump);
	this->cboxIsTmpJump->Checked    = *(this->hSrl->hIsTmpJump);
	this->cboxIsSubBanner->Checked  = *(this->hSrl->hIsSubBanner);
	this->cboxIsWL->Checked         = *(this->hSrl->hIsWL);
	if( *(this->hSrl->hIsCodecTWL) == true )
	{
		this->tboxIsCodec->Text = gcnew System::String( "TWL" );
	}
	else
	{
		this->tboxIsCodec->Text = gcnew System::String( "NTR" );
	}
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
	this->tboxShared2Size0->Text = MasterEditorTWL::transSizeToString( this->hSrl->hShared2SizeArray[0] );
	this->tboxShared2Size1->Text = MasterEditorTWL::transSizeToString( this->hSrl->hShared2SizeArray[1] );
	this->tboxShared2Size2->Text = MasterEditorTWL::transSizeToString( this->hSrl->hShared2SizeArray[2] );
	this->tboxShared2Size3->Text = MasterEditorTWL::transSizeToString( this->hSrl->hShared2SizeArray[3] );
	this->tboxShared2Size4->Text = MasterEditorTWL::transSizeToString( this->hSrl->hShared2SizeArray[4] );
	this->tboxShared2Size5->Text = MasterEditorTWL::transSizeToString( this->hSrl->hShared2SizeArray[5] );

	// アプリ種別
	if( *this->hSrl->hIsMediaNand )
	{
		this->tboxMedia->Text = gcnew System::String( "NAND" );
	}
	else
	{
		this->tboxMedia->Text = gcnew System::String( "Game Card" );
	}


	if( *(this->hSrl->hIsAppLauncher) == true )
	{
		this->tboxAppType->Text = gcnew System::String( "Launcher" );
	}
	else if( *(this->hSrl->hIsAppSecure) == true )
	{
		this->tboxAppType->Text = gcnew System::String( "Secure" );
	}
	else if( *(this->hSrl->hIsAppSystem) == true )
	{
		this->tboxAppType->Text = gcnew System::String( "System" );
	}
	else if( *(this->hSrl->hIsAppUser) == true )
	{
		this->tboxAppType->Text = gcnew System::String( "User" );
	}
	else
	{
		this->tboxAppType->Text = gcnew System::String( "UNKNOWN" );
	}

	// アクセスコントロール その他
	System::String ^acc = gcnew System::String("");
	if( *(this->hSrl->hIsCommonClientKey) == true )
	{
		acc += "commonClientKey.\r\n";
	}
	if( *(this->hSrl->hIsAesSlotBForES) == true )
	{
		acc += "AES-SlotB(ES).\r\n";
	}
	if( *(this->hSrl->hIsAesSlotCForNAM) == true )
	{
		acc += "AES-SlotC(NAM).\r\n";
	}
	if( *(this->hSrl->hIsAesSlotBForJpegEnc) == true )
	{
		acc += "AES-SlotB(JpegEnc.).\r\n";
	}
	if( *(this->hSrl->hIsAesSlotAForSSL) == true )
	{
		acc += "AES-SlotA(SSL).\r\n";
	}
	if( *(this->hSrl->hIsCommonClientKeyForDebugger) == true )
	{
		acc += "commonClientKey(Debug.).\r\n";
	}
	this->tboxAccessOther->Text = acc;

	// 特殊な設定をテキストボックスに反映
	this->setSrlFormsCaptionEx();

	// SDKバージョンとライブラリ
	this->tboxSDK->Clear();
	if( this->hSrl->hSDKList != nullptr )
	{
		for each( RCSDKVersion ^ver in this->hSrl->hSDKList )
		{
			if( ver->IsStatic )
				this->tboxSDK->Text += ver->Version + " (main static)\r\n";
			else
				this->tboxSDK->Text += ver->Version + "\r\n";
		}
	}
	this->gridLibrary->Rows->Clear();
	if( this->hSrl->hLicenseList != nullptr )
	{
		for each( RCLicense ^lic in this->hSrl->hLicenseList )
		{
			this->gridLibrary->Rows->Add( gcnew cli::array<System::Object^>{lic->Publisher, lic->Name} );
		}
	}

	// 編集可能情報
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
	this->setParentalForms();			// ペアレンタルコントロール関連
} // setSrlForms()

// SRLの特殊な設定をフォームにセットする(言語切り替えで表示を変えたいので独立させる)
void Form1::setSrlFormsCaptionEx()
{
	if( System::String::IsNullOrEmpty( this->tboxFile->Text ) )
	{
		return;
	}
	System::String ^appother = gcnew System::String("");
	if( *(this->hSrl->hIsLaunch) == false )
	{
		if( this->stripItemJapanese->Checked == true )
			appother += "ランチャー非表示.\r\n";
		else
			appother += "Not Display On the Launcher.\r\n";
	}
	if( *(this->hSrl->hIsDataOnly) == true )
	{
		if( this->stripItemJapanese->Checked == true )
			appother += "データ専用.\r\n";
		else
			appother += "Data Only.\r\n";
	}
	this->tboxAppTypeOther->Text = appother;

	this->tboxCaptionEx->Clear();
	if( (this->hSrl->hHasDSDLPlaySign != nullptr) && (*(this->hSrl->hHasDSDLPlaySign) == true) )
	{
		if( this->stripItemJapanese->Checked == true )
			this->tboxCaptionEx->Text += gcnew System::String( "DSクローンブート対応.\r\n" );
		else
			this->tboxCaptionEx->Text += gcnew System::String( "DS Clone Boot.\r\n" );
	}
	if( (this->hSrl->hIsSCFGAccess != nullptr) && (*(this->hSrl->hIsSCFGAccess) == true) )
	{
		if( this->stripItemJapanese->Checked == true )
			this->tboxCaptionEx->Text += gcnew System::String( "SCFGレジスタアクセス可能.\r\n" );
		else
			this->tboxCaptionEx->Text += gcnew System::String( "SDFC Register Accessible.\r\n" );
	}
} // setSrlFormsCaptionEx()

// フォームの入力をチェックする
System::Boolean Form1::checkSrlForms(void)
{
	// リージョン
	if( this->checkBoxIndex( this->combRegion, LANG_REGION_J, LANG_REGION_E, true ) == false )
		return false;

	// リージョンを決める
	System::Boolean bJapan     = false;
	System::Boolean bAmerica   = false;
	System::Boolean bEurope    = false;
	System::Boolean bAustralia = false;
	switch( this->combRegion->SelectedIndex )
	{
		case 0:
			bJapan = true;
		break;
		case 1:
			bAmerica = true;
		break;
		case 2:
			bEurope = true;
		break;
		case 3:
			bAustralia = true;
		break;
		case 4:
			bEurope    = true;
			bAustralia = true;
		break;

#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
		case 5:
			bJapan = true;
			bAmerica = true;
			bEurope = true;
			bAustralia = true;
		break;
#endif
		default:
		break;
	}

	// ペアレンタルコントロール
	this->checkParentalForms( bJapan, this->combCERO, this->labCERO->Text );
	this->checkParentalForms( bAmerica, this->combESRB, this->labESRB->Text );
	this->checkParentalForms( bEurope, this->combUSK, this->labUSK->Text );
	this->checkParentalForms( bEurope, this->combPEGI, this->labPEGI->Text );
	this->checkParentalForms( bEurope, this->combPEGI_PRT, this->labPEGI_PRT->Text );
	this->checkParentalForms( bEurope, this->combPEGI_BBFC, this->labPEGI_BBFC->Text );
	this->checkParentalForms( bAustralia, this->combOFLC, this->labOFLC->Text );

	// ひととおりエラー登録をした後で
	// SRLバイナリに影響を与えるエラーが存在するかチェック
#ifdef METWL_LIGHT_CHECK
	return this->isValidAffectRomModified();
#else
	return this->isValidAffectRom();
#endif
} // checkSrlForms()

// end of file