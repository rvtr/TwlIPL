// ---------------------------------------------------------------------
// リージョンとペアレンタルコントロール設定
// ---------------------------------------------------------------------

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

// ---------------------------------------------------------------------
// リージョン設定は複雑なので別に切り出す
// ---------------------------------------------------------------------

// フォーム入力をSRLに反映させる
void Form1::setRegionSrlPropaties(void)
{
	this->hSrl->hIsRegionJapan     = gcnew System::Boolean(false);
	this->hSrl->hIsRegionAmerica   = gcnew System::Boolean(false);
	this->hSrl->hIsRegionEurope    = gcnew System::Boolean(false);
	this->hSrl->hIsRegionAustralia = gcnew System::Boolean(false);
	switch( this->combRegion->SelectedIndex )
	{
		case 0:
			this->hSrl->hIsRegionJapan = gcnew System::Boolean(true);
		break;

		case 1:
			this->hSrl->hIsRegionAmerica = gcnew System::Boolean(true);
		break;

		case 2:
			this->hSrl->hIsRegionEurope = gcnew System::Boolean(true);
		break;

		case 3:
			this->hSrl->hIsRegionAustralia = gcnew System::Boolean(true);
		break;

		case 4:
			this->hSrl->hIsRegionEurope    = gcnew System::Boolean(true);
			this->hSrl->hIsRegionAustralia = gcnew System::Boolean(true);
		break;

#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
		case 5:
			this->hSrl->hIsRegionJapan     = gcnew System::Boolean(true);
			this->hSrl->hIsRegionAmerica   = gcnew System::Boolean(true);
			this->hSrl->hIsRegionEurope    = gcnew System::Boolean(true);
			this->hSrl->hIsRegionAustralia = gcnew System::Boolean(true);
		break;
#endif //defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
		default:
		break;
	}
}

// ---------------------------------------------------------------------
// ペアレンタルコントロール設定は複雑なので別に切り出す
// ---------------------------------------------------------------------

// SRL情報をフォームに反映させる
void Form1::setRegionForms(void)
{
	System::Boolean isJapan   = *(this->hSrl->hIsRegionJapan);			// リージョン
	System::Boolean isAmerica = *(this->hSrl->hIsRegionAmerica);
	System::Boolean isEurope  = *(this->hSrl->hIsRegionEurope);
	System::Boolean isAustralia = *(this->hSrl->hIsRegionAustralia);
	System::Int32  index;
	if( isJapan && !isAmerica && !isEurope && !isAustralia )
		index = 0;
	else if( !isJapan && isAmerica && !isEurope && !isAustralia )
		index = 1;
	else if( !isJapan && !isAmerica && isEurope && !isAustralia )
		index = 2;
	else if( !isJapan && !isAmerica && !isEurope && isAustralia )
		index = 3;
	else if( !isJapan && !isAmerica && isEurope && isAustralia )
		index = 4;
	else
		index = -1;	// 不正
#if defined(METWL_VER_APPTYPE_SYSTEM) || defined(METWL_VER_APPTYPE_SECURE) || defined(METWL_VER_APPTYPE_LAUNCHER)
	if( isJapan && isAmerica && isEurope && isAustralia )
	index = 5;
#endif
	this->combRegion->SelectedIndex = index;
	this->maskParentalForms();		// ペアレンタルコントロール用フォームの表示/非表示切り替え
}


// フォーム入力をSRLに反映させる
void Form1::setParentalSrlProperties(void)
{
	// 各団体のフォーム入力を反映
	this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_CERO ] = this->combCERO->SelectedIndex;
	this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_ESRB ] = this->combESRB->SelectedIndex;
	this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_USK ]  = this->combUSK->SelectedIndex;
	this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_PEGI_GEN ]  = this->combPEGI->SelectedIndex;
	this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_PEGI_PRT ]  = this->combPEGI_PRT->SelectedIndex;
	this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_PEGI_BBFC ] = this->combPEGI_BBFC->SelectedIndex;
	this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_OFLC ] = this->combOFLC->SelectedIndex;
} //setParentalSrlProperties()

// SRL内のペアレンタルコントロール情報を抜き出してフォームに反映させる
void Form1::setParentalForms(void)
{
	// 各団体のコンボボックスのインデックスを設定
	this->combCERO->SelectedIndex = this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_CERO ];
	this->combESRB->SelectedIndex = this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_ESRB ];
	this->combUSK->SelectedIndex  = this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_USK ];
	this->combPEGI->SelectedIndex = this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_PEGI_GEN ];
	this->combPEGI_PRT->SelectedIndex  = this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_PEGI_PRT ];
	this->combPEGI_BBFC->SelectedIndex = this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_PEGI_BBFC ];
	this->combOFLC->SelectedIndex = this->hSrl->hArrayParentalIndex[ OS_TWL_PCTL_OGN_OFLC ];
} //setParentalForms()

// リージョン情報からペアレンタルコントロールの編集可能団体をマスクする
void Form1::maskParentalForms(void)
{
	this->enableParental( this->combCERO, this->labCERO, nullptr );
	this->enableParental( this->combESRB, this->labESRB, nullptr );
	this->enableParental( this->combUSK,  this->labUSK,  nullptr );
	this->enableParental( this->combPEGI, this->labPEGI, nullptr );
	this->enableParental( this->combPEGI_PRT,  this->labPEGI_PRT,  nullptr );
	this->enableParental( this->combPEGI_BBFC, this->labPEGI_BBFC, nullptr );
	this->enableParental( this->combOFLC, this->labOFLC, nullptr );
	switch( this->combRegion->SelectedIndex )
	{
		case 0:
			// 日本
			this->enableParental( this->combCERO, this->labCERO, nullptr );
			this->disableParental( this->combESRB, this->labESRB, nullptr );
			this->disableParental( this->combUSK,  this->labUSK,  nullptr );
			this->disableParental( this->combPEGI, this->labPEGI, nullptr );
			this->disableParental( this->combPEGI_PRT,  this->labPEGI_PRT,  nullptr );
			this->disableParental( this->combPEGI_BBFC, this->labPEGI_BBFC, nullptr );
			this->disableParental( this->combOFLC, this->labOFLC, nullptr );
		break;

		case 1:
			// 米国
			this->disableParental( this->combCERO, this->labCERO, nullptr );
			this->enableParental( this->combESRB,  this->labESRB, nullptr );
			this->disableParental( this->combUSK,  this->labUSK,  nullptr );
			this->disableParental( this->combPEGI, this->labPEGI, nullptr );
			this->disableParental( this->combPEGI_PRT,  this->labPEGI_PRT,  nullptr );
			this->disableParental( this->combPEGI_BBFC, this->labPEGI_BBFC, nullptr );
			this->disableParental( this->combOFLC, this->labOFLC, nullptr );
		break;

		case 2:
			// 欧州
			this->disableParental( this->combCERO, this->labCERO, nullptr );
			this->disableParental( this->combESRB, this->labESRB, nullptr );
			this->enableParental( this->combUSK,   this->labUSK,  nullptr );
			this->enableParental( this->combPEGI,  this->labPEGI, nullptr );
			this->enableParental( this->combPEGI_PRT,  this->labPEGI_PRT,  nullptr );
			this->enableParental( this->combPEGI_BBFC, this->labPEGI_BBFC, nullptr );
			this->disableParental( this->combOFLC, this->labOFLC, nullptr );
		break;

		case 3:
			// 豪州
			this->disableParental( this->combCERO, this->labCERO, nullptr );
			this->disableParental( this->combESRB, this->labESRB, nullptr );
			this->disableParental( this->combUSK,  this->labUSK,  nullptr );
			this->disableParental( this->combPEGI, this->labPEGI, nullptr );
			this->disableParental( this->combPEGI_PRT,  this->labPEGI_PRT,  nullptr );
			this->disableParental( this->combPEGI_BBFC, this->labPEGI_BBFC, nullptr );
			this->enableParental( this->combOFLC,  this->labOFLC, nullptr );
		break;

		case 4:
			// 欧州と豪州
			this->disableParental( this->combCERO, this->labCERO, nullptr );
			this->disableParental( this->combESRB, this->labESRB, nullptr );
			this->enableParental( this->combUSK,   this->labUSK,  nullptr );
			this->enableParental( this->combPEGI,  this->labPEGI, nullptr );
			this->enableParental( this->combPEGI_PRT,  this->labPEGI_PRT,  nullptr );
			this->enableParental( this->combPEGI_BBFC, this->labPEGI_BBFC, nullptr );
			this->enableParental( this->combOFLC,  this->labOFLC, nullptr );
		break;

		// 全リージョンのときは何もdisableにしない
		default:
		break;
	}
} //maskParentalForms()

// ペアレンタルコントロール関連のフォーム入力が正しいか書き込み前チェック
void Form1::checkParentalForms( System::Boolean inRegion, System::Windows::Forms::ComboBox ^comb, System::String ^msg )
{
	// リージョンに含まれていないとき: 0クリアが保証されるのでチェック必要なし
	if( !inRegion )
		return;

	// 設定されていないときエラー
	if( (comb->SelectedIndex < 0) || (comb->SelectedIndex >= comb->Items->Count)  )
	{
		this->hErrorList->Add( gcnew RCMrcError( 
			"ペアレンタルコントロール情報", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
			msg + ": レーティングを選択してください。",
			"Parental Control", 
			msg + ": Rating Pending is setting. When rating age is examined, Please submit again.", true, true ) );
	}

	// 審査中のとき警告
	if( comb->SelectedIndex == (comb->Items->Count - 1) )
	{
		this->hWarnList->Add( gcnew RCMrcError( 
			"ペアレンタルコントロール情報", METWL_ERRLIST_NORANGE, METWL_ERRLIST_NORANGE,
			msg + ": 審査中指定がされています。審査が決まりしだい、再提出してください。",
			"Parental Control", msg + ": Save ROM data as Game soft which needs rating examinination.", true, true ) );
	}
} //checkParentalForms()


// ペアレンタルコントロール情報をクリアする
void Form1::clearParental( System::Windows::Forms::ComboBox ^comb )
{
	comb->SelectedIndex = -1;	// 空白にする
}


// ペアレンタルコントロール情報を編集できるようにする
void Form1::enableParental( System::Windows::Forms::ComboBox ^comb, 
							System::Windows::Forms::Label    ^lab1, 
							System::Windows::Forms::Label    ^lab2 )
{
	comb->Enabled   = true;
	comb->Visible   = true;
	lab1->Visible   = true;
	if( lab2 != nullptr )
	{
		lab2->Visible   = true;
	}
}

// ペアレンタルコントロール情報を編集できなくする
void Form1::disableParental( System::Windows::Forms::ComboBox ^comb, 
							 System::Windows::Forms::Label    ^lab1, 
							 System::Windows::Forms::Label    ^lab2 )
{
	this->clearParental( comb );
	comb->Enabled   = false;
	comb->Visible   = false;
	lab1->Visible   = false;
	if( lab2 != nullptr )
	{
		lab2->Visible   = false;
	}
}

// end of file