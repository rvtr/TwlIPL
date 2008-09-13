// ---------------------------------------------------------------------
// ペアレンタルコントロール設定は複雑なので外部ファイルに切り出す
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