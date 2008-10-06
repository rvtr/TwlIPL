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
	System::Int32 index;

	// 入力文字数制限を変更する
	this->changeMaxLength( this->tboxCompany1, 25 );
	this->changeMaxLength( this->tboxDepart1,  25 );
	this->changeMaxLength( this->tboxPerson1,  15 );

	// タイトルバー
	this->stripFile->Text          = gcnew System::String( "ファイル" );
	this->stripItemOpenRom->Text   = gcnew System::String( "ROMデータを開く" );
	this->stripItemSaveTemp->Text  = gcnew System::String( "提出情報を一時保存する" );
	this->stripItemLoadTemp->Text  = gcnew System::String( "一時保存した提出情報を読み込む" );
	this->stripMaster->Text        = gcnew System::String( "マスター" );
	this->stripItemSheet->Text     = gcnew System::String( "提出データ一式を作成する" );
	this->stripItemMasterRom->Text = gcnew System::String( "マスターROMのみを作成する" );
	this->stripItemMiddlewareXml->Text  = gcnew System::String( "ミドルウェアリストを作成する(XML形式)" );
	this->stripItemMiddlewareHtml->Text = gcnew System::String( "ミドルウェアリストを作成する(HTML形式)" );

	// 入力ファイル
	this->labFile->Text = gcnew System::String( "ROMデータファイル" );

	// タブ
	this->tabRomInfo->Text     = gcnew System::String( "ROM基本情報(確認用)" );
	this->tabTWLInfo->Text     = gcnew System::String( "TWL拡張情報(確認用)" );
	this->tabRomEditInfo->Text = gcnew System::String( "ROM登録情報(編集可)" );
	this->tabSubmitInfo->Text  = gcnew System::String( "提出情報(編集可)" );
	this->tabCompanyInfo->Text = gcnew System::String( "会社情報(編集可)" );
	this->tabErrorInfo->Text   = gcnew System::String( "エラー情報(確認用)" );

	// ガイド
	this->tboxGuideRomInfo->Text = gcnew System::String( "このタブの情報は編集不可です。データに誤りがある場合にはROMデータの作成時の設定を見直してください。" );
	this->tboxGuideTWLInfo->Text = gcnew System::String( "このタブの情報は編集不可です。データに誤りがある場合にはROMデータの作成時の設定を見直してください。" );
	this->tboxGuideRomEditInfo->Text  = gcnew System::String( "" );
	this->tboxGuideRomEditInfo->Text += "このタブの情報は提出確認書およびマスターROMの作成に必要です。編集してください。";
	this->tboxGuideRomEditInfo->Text += "\r\n(マスターROMの作成をするまでROMデータの中には登録されません。)";
	this->tboxGuideSubmitInfo->Text  = gcnew System::String( "このタブの情報は提出確認書の作成に必要です。入力してください。" );
	this->tboxGuideCompanyInfo->Text = gcnew System::String( "このタブの情報は提出確認書の作成に必要です。入力してください。" );
	this->tboxGuideErrorInfo->Text   = gcnew System::String( "" );
	this->tboxGuideErrorInfo->Text  += "このタブには読み込んだROMデータの問題と本プログラムでの入力ミスが列挙されます。";
	this->tboxGuideErrorInfo->Text  += "\r\n赤文字の項目は、本プログラムで修正不可です。ROMデータ作成時の設定をご確認ください。";
	this->tboxGuideErrorInfo->Text  += "\r\n青文字の項目は、本プログラムで修正できますが、修正がマスターROMに反映されます。";
	this->tboxGuideErrorInfo->Text  += "\r\n黒文字の項目は、提出確認書にのみ反映され、マスターROMには反映されません。";

	// SRL情報
	this->gboxSrl->Text       = gcnew System::String( "ROMデータ情報" ); 
	this->labTitleName->Text  = gcnew System::String( "ソフトタイトル" );
	this->labGameCode->Text   = gcnew System::String( "イニシャルコード" );
	this->labMakerCode->Text  = gcnew System::String( "メーカコード" );
	this->labPlatform->Text   = gcnew System::String( "プラットフォーム" );
	this->labRomType->Text    = gcnew System::String( "ROMタイプ設定" );
	this->labRomSize->Text    = gcnew System::String( "ROM容量" );
	this->labRemasterVer->Text   = gcnew System::String( "リマスターバージョン" );
	this->cboxRemasterVerE->Text = gcnew System::String( "E(準備版)" );
	this->labHeaderCRC->Text  = gcnew System::String( "ヘッダCRC" );
	this->labRomCRC->Text     = gcnew System::String( "全体のCRC" );
	index = this->combBackup->SelectedIndex;

	// バックアップメモリ
	this->gboxProd->Text	= gcnew System::String( "ROM生産情報(必ず入力してください)" );
	this->labBackup->Text   = gcnew System::String( LANG_BACKUP_J );
	this->combBackup->Items->Clear();
	this->combBackup->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"4Kbit EEPROM", L"64Kbit EEPROM", L"512Kbit EEPROM", 
		L"256Kbit FRAM", L"2Mbit FLASH", L"4Mbit FLASH", L"8Mbit FLASH", L"なし", L"その他"});
	this->combBackup->SelectedIndex = index;

	// 提出情報
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

	// 会社情報
	this->gboxPerson1->Text    = gcnew System::String( LANG_PERSON_1_J );
	this->gboxPerson2->Text    = gcnew System::String( LANG_PERSON_2_J );
	this->cboxIsInputPerson2->Text = gcnew System::String( LANG_INPUT_PERSON_2_J );
	this->labCompany1->Text    = gcnew System::String( LANG_COMPANY_J );
	this->labDepart1->Text     = gcnew System::String( LANG_DEPART_J );
	this->labPerson1->Text     = gcnew System::String( LANG_PERSON_J );
	this->labCompany2->Text    = gcnew System::String( LANG_COMPANY_J );
	this->labDepart2->Text     = gcnew System::String( LANG_DEPART_J );
	this->labPerson2->Text     = gcnew System::String( LANG_PERSON_J );
	this->labArbit1->Text      = gcnew System::String( "(任意)" );
	this->labArbit2->Text      = gcnew System::String( "(任意)" );
	this->labArbit3->Text      = gcnew System::String( "(任意)" );
	this->labArbit4->Text      = gcnew System::String( "(任意)" );
	// ふりがな情報を有効にする
	this->tboxFurigana1->Enabled = true;
	this->labFurigana1->Text = gcnew System::String( LANG_FURIGANA_J );
	this->tboxFurigana2->Enabled = true;
	this->labFurigana2->Text = gcnew System::String( LANG_FURIGANA_J );
	// NTSC-UserIDも日本語版のみ
	this->tboxNTSC1->Enabled = true;
	this->tboxNTSC2->Enabled = true;
	this->labNTSC1Pre->Text  = gcnew System::String( LANG_NTSC_1_J );
	this->labNTSC1Sur->Text  = gcnew System::String( LANG_NTSC_2_J );
	this->labNTSC2Pre->Text  = gcnew System::String( LANG_NTSC_1_J );
	this->labNTSC2Sur->Text  = gcnew System::String( LANG_NTSC_2_J );

	// TWL仕様
	this->gboxTWLExInfo->Text         = gcnew System::String( "TWL拡張情報" );
	this->labNormalRomOffset->Text    = gcnew System::String( "TWLノーマル領域ROMオフセット" );
	this->labKeyTableRomOffset->Text  = gcnew System::String( "TWL専用領域ROMオフセット" );
	this->cboxIsNormalJump->Text      = gcnew System::String( "ノーマルジャンプ許可" );
	this->cboxIsTmpJump->Text         = gcnew System::String( "tmpジャンプ許可" );
	this->cboxIsSubBanner->Text       = gcnew System::String( "サブバナーファイル有効" );
	this->cboxIsWL->Text              = gcnew System::String( "NTRホワイトリスト署名有効" );
	this->gboxAccess->Text            = gcnew System::String( "アクセスコントロール情報" );
	this->cboxIsSD->Text              = gcnew System::String( "SDカード" );
	this->cboxIsNAND->Text            = gcnew System::String( "NANDフラッシュメモリ" );
	this->labIsGameCardOn->Text       = gcnew System::String( "ゲームカード電源" );
	this->labAccessOther->Text        = gcnew System::String( "その他" );
	this->gboxShared2Size->Text       = gcnew System::String( "Shared2ファイルサイズ" );
	this->cboxIsShared2->Text         = gcnew System::String( "Shared2ファイル使用" );
	this->labSDK->Text                = gcnew System::String( "SDKバージョン" );
	this->labLib->Text                = gcnew System::String( "使用ライブラリ" );
	this->labCaptionEx->Text          = gcnew System::String( "特記事項" );

	// SRL編集可能情報
	this->gboxEULA->Text         = gcnew System::String( LANG_BOX_EULA_J );
	this->cboxIsEULA->Text       = gcnew System::String( LANG_EULA_J );
	this->gboxIcon->Text         = gcnew System::String( LANG_ICON_J );
	this->rIsWirelessIcon->Text  = gcnew System::String( LANG_WIRELESS_ICON_J );
	this->rIsWiFiIcon->Text      = gcnew System::String( LANG_WIFI_ICON_J );
	this->rIsNoIcon->Text        = gcnew System::String( LANG_NO_ICON_J );
	this->labRegion->Text        = gcnew System::String( LANG_REGION_J );

	// リージョン
	index = this->combRegion->SelectedIndex;
	this->combRegion->Items->Clear();
	this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5)
		{L"日本のみ", L"米国のみ", L"欧州のみ", L"豪州のみ", L"欧州および豪州"});
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	this->combRegion->Items->Add( gcnew System::String( L"全リージョン" ) );
#endif
	this->combRegion->SelectedIndex = index;

	//// ペアレンタルコントロール
	this->gboxParental->Text           = gcnew System::String( LANG_REGION_PCTL_J );
	this->labParentalRating->Text      = gcnew System::String( LANG_PCTL_RATING_J );

	index = this->combCERO->SelectedIndex;	// いったんclearすると現在のindexに意味がなくなるので退避
	this->combCERO->Items->Clear();
	this->combCERO->Items->AddRange(gcnew cli::array< System::Object^  >(6)
		{L"A (全年齢)", L"B (12歳以上)", L"C (15歳以上)", L"D (17歳以上)", L"Z (18歳以上)", L"審査中"});
	this->combCERO->SelectedIndex = index;

	index = this->combESRB->SelectedIndex;
	this->combESRB->Items->Clear();
	this->combESRB->Items->AddRange(gcnew cli::array< System::Object^  >(7) 
		{L"年齢制限なし(全年齢)", L"EC (3歳以上)", L"E (6歳以上)", L"E10+ (10歳以上)", L"T (13歳以上)", L"M (17歳以上)", L"審査中"});
	this->combESRB->SelectedIndex = index;

	index = this->combUSK->SelectedIndex;
	this->combUSK->Items->Clear();
	this->combUSK->Items->AddRange(gcnew cli::array< System::Object^  >(6)
		{L"年齢制限なし", L"6歳以上", L"12歳以上", L"16歳以上", L"青少年には不適切", L"審査中"});
	this->combUSK->SelectedIndex = index;

	index = this->combPEGI->SelectedIndex;
	this->combPEGI->Items->Clear();
	this->combPEGI->Items->AddRange(gcnew cli::array< System::Object^  >(7)
		{L"年齢制限なし(全年齢)", L"3歳以上", L"7歳以上", L"12歳以上", L"16歳以上", L"18歳以上", L"審査中"});
	this->combPEGI->SelectedIndex = index;

	index = this->combPEGI_PRT->SelectedIndex;
	this->combPEGI_PRT->Items->Clear();
	this->combPEGI_PRT->Items->AddRange(gcnew cli::array< System::Object^  >(7)
		{L"年齢制限なし(全年齢)", L"4歳以上", L"6歳以上", L"12歳以上", L"16歳以上", L"18歳以上", L"審査中"});
	this->combPEGI_PRT->SelectedIndex = index;

	index = this->combPEGI_BBFC->SelectedIndex;
	this->combPEGI_BBFC->Items->Clear();
	this->combPEGI_BBFC->Items->AddRange(gcnew cli::array< System::Object^  >(10)
		{L"年齢制限なし(全年齢)", L"3歳以上", L"4歳以上推奨", L"7歳以上", L"8歳以上推奨", L"12歳以上", L"15歳以上", L"16歳以上", L"18歳以上", L"審査中"});
	this->combPEGI_BBFC->SelectedIndex = index;

	index = this->combOFLC->SelectedIndex;
	this->combOFLC->Items->Clear();
	this->combOFLC->Items->AddRange(gcnew cli::array< System::Object^  >(5)
		{L"G", L"PG", L"M", L"MA15+", L"審査中"});
	this->combOFLC->SelectedIndex = index;

	// エラー情報
	this->labError->Text = gcnew System::String( "エラー(必ず修正してください。)" );
	this->colErrorName->HeaderText  = gcnew System::String( "項目名" );
	this->colErrorBegin->HeaderText = gcnew System::String( "開始" );
	this->colErrorEnd->HeaderText   = gcnew System::String( "終了" );
	this->colErrorCause->HeaderText = gcnew System::String( "要因" );

	this->labWarn->Text  = gcnew System::String( "警告(修正は必須ではありませんが情報に誤りがないかご確認ください。)" );
	this->colWarnName->HeaderText  = gcnew System::String( "項目名" );
	this->colWarnBegin->HeaderText = gcnew System::String( "開始" );
	this->colWarnEnd->HeaderText   = gcnew System::String( "終了" );
	this->colWarnCause->HeaderText = gcnew System::String( "要因" );

	this->gboxErrorTiming->Text = gcnew System::String( "いつの情報を表示するか" );
	this->rErrorReading->Text   = gcnew System::String( "ROMデータ読み込み時" );
	this->rErrorCurrent->Text   = gcnew System::String( "現在の入力を反映" );

	// 特殊な設定用のテキストボックスの表記を変更
	this->setSrlFormsCaptionEx();
}

// 英語版への切り替え
void  Form1::changeEnglish(void)
{
	System::Int32 index;

	// 入力文字数制限を変更する
	this->changeMaxLength( this->tboxCompany1, 40 );
	this->changeMaxLength( this->tboxDepart1,  40 );
	this->changeMaxLength( this->tboxPerson1,  30 );

	// タイトルバー
	this->stripFile->Text          = gcnew System::String( "File" );
	this->stripItemOpenRom->Text   = gcnew System::String( "Open a ROM data file" );
	this->stripItemSaveTemp->Text  = gcnew System::String( "Save a temporary info." );
	this->stripItemLoadTemp->Text  = gcnew System::String( "Load a temporary info. saved previously" );
	this->stripMaster->Text        = gcnew System::String( "Master" );
	this->stripItemSheet->Text     = gcnew System::String( "Make a set of submission data" );
	this->stripItemMasterRom->Text = gcnew System::String( "Make a master ROM data file only" );
	this->stripItemMiddlewareXml->Text  = gcnew System::String( "Make a middleware list(XML format)" );
	this->stripItemMiddlewareHtml->Text = gcnew System::String( "Make a middleware list(HTML format)" );

	// 入力ファイル
	this->labFile->Text = gcnew System::String( "ROM Data File" );

	// タブ
	this->tabRomInfo->Text     = gcnew System::String( "ROM Info.(Read Only)" );
	this->tabTWLInfo->Text     = gcnew System::String( "TWL Info.(Read Only)" );
	this->tabRomEditInfo->Text = gcnew System::String( "ROM Settings(Editable)" );
	this->tabSubmitInfo->Text  = gcnew System::String( "Submission Info.(Editable)" );
	this->tabCompanyInfo->Text = gcnew System::String( "Company Info.(Editable)" );
	this->tabErrorInfo->Text   = gcnew System::String( "Error(Read Only)" );

	// ガイド
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

	// SRL情報
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
	// バックアップメモリ
	this->gboxProd->Text   = gcnew System::String( "ROM Production Info." );
	this->labBackup->Text  = gcnew System::String( LANG_BACKUP_E );
	this->combBackup->Items->Clear();
	this->combBackup->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"4Kbit EEPROM", L"64Kbit EEPROM", L"512Kbit EEPROM", 
		L"256Kbit FRAM", L"2Mbit FLASH", L"4Mbit FLASH", L"8Mbit FLASH", L"Nothing", L"Other"});
	this->combBackup->SelectedIndex = index;

	// 提出情報
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

	// 会社情報
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
	// ふりがな情報を削除
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

	// TWL仕様
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

	// SRL編集可能情報
	this->gboxEULA->Text         = gcnew System::String( LANG_BOX_EULA_E );
	this->cboxIsEULA->Text       = gcnew System::String( LANG_EULA_E );
	this->gboxIcon->Text         = gcnew System::String( LANG_ICON_E );
	this->rIsWirelessIcon->Text  = gcnew System::String( LANG_WIRELESS_ICON_E );
	this->rIsWiFiIcon->Text      = gcnew System::String( LANG_WIFI_ICON_E );
	this->rIsNoIcon->Text        = gcnew System::String( LANG_NO_ICON_E );
	this->labRegion->Text        = gcnew System::String( LANG_REGION_E );

	// リージョン
	index = this->combRegion->SelectedIndex;
	this->combRegion->Items->Clear();
	this->combRegion->Items->AddRange(gcnew cli::array< System::Object^  >(5)
		{L"Japan Only", L"USA Only", L"Europe Only", L"Australia Only", L"Europe and Australia"});
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	this->combRegion->Items->Add( gcnew System::String( L"All Region" ) );
#endif
	this->combRegion->SelectedIndex = index;

	//// ペアレンタルコントロール
	this->gboxParental->Text           = gcnew System::String( LANG_REGION_PCTL_E );
	this->labParentalRating->Text      = gcnew System::String( LANG_PCTL_RATING_E );

	index = this->combCERO->SelectedIndex;	// いったんclearすると現在のindexに意味がなくなるので退避
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

	// エラー情報
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

	// 特殊な設定用のテキストボックスの表記を変更
	this->setSrlFormsCaptionEx();
}

// end of file