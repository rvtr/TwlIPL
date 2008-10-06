// deliverable.h のクラス実装

#include "stdafx.h"
#include <apptype.h>
#include "common.h"
#include "srl.h"
#include "deliverable.h"
#include "utility.h"
#include <cstring>
#include <cstdio>

using namespace MasterEditorTWL;

//
// RCDeliverable クラス
//

//
// 書類出力
//
// @arg [out] 出力ファイル名
// @arg [in]  ROMバイナリ(SRL)固有情報
// @arg [in]  ファイル全体のCRC
// @arg [in]  SRLのファイル名(書類に記述するために使用)
// @arg [in]  英語フラグ
//
ECDeliverableResult RCDeliverable::writeSpreadsheet(
	System::String ^hFilename, MasterEditorTWL::RCSrl ^hSrl, System::UInt16 ^hCRC, System::String ^hSrlFilename, System::Boolean english )
{
	// テンプレートを読み込む
	System::Xml::XmlDocument ^doc = gcnew System::Xml::XmlDocument();
	try
	{
		doc->Load( "../resource/sheet_templete.xml" );
	}
	catch ( System::Exception ^ex )
	{
		(void)ex;
		return ECDeliverableResult::ERROR_FILE_OPEN;
	}
	System::Xml::XmlElement ^root = doc->DocumentElement;

	// ソフトタイトルetc.は1文字ずつ入れる
	char title_name[ TITLE_NAME_MAX ];
	char game_code[  GAME_CODE_MAX ];
	char maker_code[ MAKER_CODE_MAX ];
	MasterEditorTWL::setStringToChars( title_name, hSrl->hTitleName, TITLE_NAME_MAX, 0 );
	MasterEditorTWL::setStringToChars( game_code,  hSrl->hGameCode,  GAME_CODE_MAX,  0 );
	MasterEditorTWL::setStringToChars( maker_code, hSrl->hMakerCode, MAKER_CODE_MAX, 0 );
	//System::String ^str = gcnew System::String( hSrl->hTitleName[0], 1 );
	//System::Diagnostics::Debug::WriteLine( str );
	//System::Diagnostics::Debug::WriteLine( "hex 0x: " + title_name[0].ToString("X") );

	// アプリ種別
	System::String ^apptype = gcnew System::String("");
	if( *hSrl->hIsAppLauncher )
	{
		apptype = gcnew System::String( "Launcher" );
	}
	else if( *hSrl->hIsAppSecure )
	{
		apptype = gcnew System::String( "Secure" );
	}
	else if( *hSrl->hIsAppSystem )
	{
		apptype = gcnew System::String( "System" );
	}
	else if( *hSrl->hIsAppUser )
	{
		apptype = gcnew System::String( "User" );
	}
	System::String ^media = gcnew System::String("");
	if( *hSrl->hIsMediaNand )
	{
		media = gcnew System::String( "NAND" );
	}
	else
	{
		media = gcnew System::String( "Game Card" );
	}
	System::String ^appother = gcnew System::String("");
	if( *(hSrl->hIsLaunch) == false )
	{
		appother += "ランチャー非表示.";
	}
	if( *(hSrl->hIsDataOnly) == true )
	{
		apptype += "データ専用.";
	}

	// アクセスコントロール その他
	System::String ^access = gcnew System::String("");
	if( *(hSrl->hIsCommonClientKey) == true )
	{
		access += "commonClientKey. ";
	}
	if( *(hSrl->hIsAesSlotBForES) == true )
	{
		access += "AES-SlotB(ES). ";
	}
	if( *(hSrl->hIsAesSlotCForNAM) == true )
	{
		access += "AES-SlotC(NAM). ";
	}
	if( *(hSrl->hIsAesSlotBForJpegEnc) == true )
	{
		access += "AES-SlotB(JpegEnc.). ";
	}
	if( *(hSrl->hIsAesSlotAForSSL) == true )
	{
		access += "AES-SlotA(SSL). ";
	}
	if( *(hSrl->hIsCommonClientKeyForDebugger) == true )
	{
		access += "commonClientKey(Debug). ";
	}

	// 使用ライセンス
	System::String ^lib = gcnew System::String("");
	if( hSrl->hLicenseList != nullptr )
	{
		for each( RCLicense ^lic in hSrl->hLicenseList )
		{
			lib += lic->Publisher + " " + lic->Name + ". ";
		}
	}

	// 書類テンプレートの各タグを入力情報に置き換え
	System::Xml::XmlNodeList ^list;
	list = root->GetElementsByTagName( "Data" );
	System::Int32 i;
	for( i=0; i < list->Count; i++ )
	{
		System::Xml::XmlNode ^node = list->Item(i);
		if( (node->FirstChild != nullptr) && (node->FirstChild->Value != nullptr) )
		{
			// 提出情報
			if( node->FirstChild->Value->Equals( "TagProductName" ) )
			{
				node->FirstChild->Value = this->hProductName;
			}
			if( node->FirstChild->Value->Equals( "TagProductCode1" ) )
			{
				node->FirstChild->Value = this->hProductCode1;
			}
			if( node->FirstChild->Value->Equals( "TagProductCode2" ) )
			{
				node->FirstChild->Value = this->hProductCode2;
			}
			if( node->FirstChild->Value->Equals( "TagReleaseForeign" ) )
			{
				if( *(this->hReleaseForeign) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagProductNameForeign" ) )
			{
				node->FirstChild->Value = this->hProductNameForeign;
			}
			if( node->FirstChild->Value->Equals( "TagProductCode1Foreign" ) )
			{
				node->FirstChild->Value = this->hProductCode1Foreign;
			}
			if( node->FirstChild->Value->Equals( "TagProductCode2Foreign" ) )
			{
				node->FirstChild->Value = this->hProductCode2Foreign;
			}
			if( node->FirstChild->Value->Equals( "TagSubmitYear" ) )
			{
				node->FirstChild->Value = this->hSubmitYear->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagSubmitMonth" ) )
			{
				node->FirstChild->Value = this->hSubmitMonth->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagSubmitDay" ) )
			{
				node->FirstChild->Value = this->hSubmitDay->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagReleaseYear" ) )
			{
				node->FirstChild->Value = this->hReleaseYear->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagReleaseMonth" ) )
			{
				node->FirstChild->Value = this->hReleaseMonth->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagReleaseDay" ) )
			{
				node->FirstChild->Value = this->hReleaseDay->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagSubmitWay" ) )
			{
				node->FirstChild->Value = this->hSubmitWay;
			}
			if( node->FirstChild->Value->Equals( "TagUsage" ) )
			{
				node->FirstChild->Value = this->hUsage;
			}
			if( node->FirstChild->Value->Equals( "TagUsageOther" ) )
			{
				node->FirstChild->Value = this->hUsageOther;	// nullptr のときはセルが空になるので好都合
			}
			if( node->FirstChild->Value->Equals( "TagRomVersion" ) )
			{
				if( *(hSrl->hRomVersion) != 0xE0 )
				{
					node->FirstChild->Value = hSrl->hRomVersion->ToString();
				}
				else
				{
					if( english )
						node->FirstChild->Value = gcnew System::String( "E (Preliminary ver.)" );
					else
						node->FirstChild->Value = gcnew System::String( "E (事前版)" );
				}
			}
			if( node->FirstChild->Value->Equals( "TagSubmitVersion" ) )
			{
				node->FirstChild->Value = this->hSubmitVersion->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagSrlFilename" ) )
			{
				node->FirstChild->Value = hSrlFilename;
			}
			if( node->FirstChild->Value->Equals( "TagCRC" ) )
			{
				node->FirstChild->Value = "0x" + hCRC->ToString("X");
			}
			// ROM情報
			if( node->FirstChild->Value->Equals( "TagLatency" ) )
			{
				node->FirstChild->Value = hSrl->hLatency;
			}
			if( node->FirstChild->Value->Equals( "TagPlatform" ) )
			{
				node->FirstChild->Value = hSrl->hPlatform;
			}
			if( node->FirstChild->Value->Equals( "TagRomSize" ) )
			{
				node->FirstChild->Value = hSrl->hRomSize;
			}
			if( node->FirstChild->Value->Equals( "TagBackupMemory" ) )
			{
				node->FirstChild->Value = this->hBackupMemory;
			}
			if( node->FirstChild->Value->Equals( "TagSDK" ) )
			{
				node->FirstChild->Value = this->hSDK;
			}
			if( node->FirstChild->Value->Equals( "TagLibrary" ) )
			{
				node->FirstChild->Value = lib;
			}
			// ROM情報 (TWL拡張情報)
			if( node->FirstChild->Value->Equals( "TagTitleIDLo" ) )
			{
				node->FirstChild->Value = hSrl->hTitleIDLo;
			}
			if( node->FirstChild->Value->Equals( "TagTitleIDHi" ) )
			{
				node->FirstChild->Value = "0x" + hSrl->hTitleIDHi->ToString("X8");
			}
			if( node->FirstChild->Value->Equals( "TagAppType" ) )
			{
				node->FirstChild->Value = apptype;
			}
			if( node->FirstChild->Value->Equals( "TagMedia" ) )
			{
				node->FirstChild->Value = media;
			}
			if( node->FirstChild->Value->Equals( "TagAppTypeOther" ) )
			{
				node->FirstChild->Value = appother;
			}
			if( node->FirstChild->Value->Equals( "TagIsNormalJump" ) )
			{
				if( *(hSrl->hIsNormalJump) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsTmpJump" ) )
			{
				if( *(hSrl->hIsTmpJump) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagNormalRomOffset" ) )
			{
				node->FirstChild->Value = "0x" + hSrl->hNormalRomOffset->ToString("X8");
			}
			if( node->FirstChild->Value->Equals( "TagKeyTableRomOffset" ) )
			{
				node->FirstChild->Value = "0x" + hSrl->hKeyTableRomOffset->ToString("X8");
			}
			if( node->FirstChild->Value->Equals( "TagPublicSize" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transSizeToString( hSrl->hPublicSize );
			}
			if( node->FirstChild->Value->Equals( "TagPrivateSize" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transSizeToString( hSrl->hPrivateSize );
			}
			if( node->FirstChild->Value->Equals( "TagIsCodec" ) )
			{
				if( *(hSrl->hIsCodecTWL) == true )
					node->FirstChild->Value = gcnew System::String("TWL");
				else
					node->FirstChild->Value = gcnew System::String("NTR");
			}
			if( node->FirstChild->Value->Equals( "TagIsEULA" ) )
			{
				if( *(hSrl->hIsEULA) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsSubBanner" ) )
			{
				if( *(hSrl->hIsSubBanner) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsWiFiIcon" ) )
			{
				if( *(hSrl->hIsWiFiIcon) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsWirelessIcon" ) )
			{
				if( *(hSrl->hIsWirelessIcon) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsWL" ) )
			{
				if( *(hSrl->hIsWL) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsSD" ) )
			{
				if( *(hSrl->hIsSD) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsNAND" ) )
			{
				if( *(hSrl->hIsNAND) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsShared2" ) )
			{
				if( *(hSrl->hIsShared2) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsGameCardOn" ) )
			{
				if( *(hSrl->hIsGameCardNitro) == true )
					node->FirstChild->Value = gcnew System::String("ON(NTR)");
				else if( *(hSrl->hIsGameCardOn) == true )
					node->FirstChild->Value = gcnew System::String("ON(normal)");
				else
					node->FirstChild->Value = gcnew System::String("OFF");;
			}
			if( node->FirstChild->Value->Equals( "TagAccessOther" ) )
			{
				node->FirstChild->Value = access;
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size0" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transSizeToString( hSrl->hShared2SizeArray[0] );
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size1" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transSizeToString( hSrl->hShared2SizeArray[1] );
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size2" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transSizeToString( hSrl->hShared2SizeArray[2] );
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size3" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transSizeToString( hSrl->hShared2SizeArray[3] );
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size4" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transSizeToString( hSrl->hShared2SizeArray[4] );
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size5" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transSizeToString( hSrl->hShared2SizeArray[5] );
			}

			// 会社情報
			if( node->FirstChild->Value->Equals( "TagCompany1" ) )
			{
				node->FirstChild->Value = this->hCompany1;
			}
			if( node->FirstChild->Value->Equals( "TagPerson1" ) )
			{
				node->FirstChild->Value = this->hPerson1;
			}
			if( node->FirstChild->Value->Equals( "TagFurigana1" ) )
			{
				node->FirstChild->Value = this->hFurigana1;
			}
			if( node->FirstChild->Value->Equals( "TagTel1" ) )
			{
				node->FirstChild->Value = this->hTel1;
			}
			if( node->FirstChild->Value->Equals( "TagFax1" ) )
			{
				node->FirstChild->Value = this->hFax1;
			}
			if( node->FirstChild->Value->Equals( "TagMail1" ) )
			{
				node->FirstChild->Value = this->hMail1;
			}
			if( node->FirstChild->Value->Equals( "TagNTSC1" ) )
			{
				node->FirstChild->Value = this->hNTSC1;
			}
			if( node->FirstChild->Value->Equals( "TagCompany2" ) )
			{
				node->FirstChild->Value = this->hCompany2;
			}
			if( node->FirstChild->Value->Equals( "TagPerson2" ) )
			{
				node->FirstChild->Value = this->hPerson2;
			}
			if( node->FirstChild->Value->Equals( "TagFurigana2" ) )
			{
				node->FirstChild->Value = this->hFurigana2;
			}
			if( node->FirstChild->Value->Equals( "TagTel2" ) )
			{
				node->FirstChild->Value = this->hTel2;
			}
			if( node->FirstChild->Value->Equals( "TagFax2" ) )
			{
				node->FirstChild->Value = this->hFax2;
			}
			if( node->FirstChild->Value->Equals( "TagMail2" ) )
			{
				node->FirstChild->Value = this->hMail2;
			}
			if( node->FirstChild->Value->Equals( "TagNTSC2" ) )
			{
				node->FirstChild->Value = this->hNTSC2;
			}

			// 備考
			if( node->FirstChild->Value->Equals( "TagCaption" ) )
			{
				node->FirstChild->Value = this->hCaption;
			}

			// リージョン
			if( node->FirstChild->Value->Equals( "TagRegion" ) )
			{
				node->FirstChild->Value = this->hRegion;
			}

			// ペアレンタルコントロール
			if( node->FirstChild->Value->Equals( "TagRatingCERO" ) )
			{
				node->FirstChild->Value = this->hCERO;
			}
			if( node->FirstChild->Value->Equals( "TagRatingESRB" ) )
			{
				node->FirstChild->Value = this->hESRB;
			}
			if( node->FirstChild->Value->Equals( "TagRatingUSK" ) )
			{
				node->FirstChild->Value = this->hUSK;
			}
			if( node->FirstChild->Value->Equals( "TagRatingPEGI" ) )
			{
				node->FirstChild->Value = this->hPEGI;
			}
			if( node->FirstChild->Value->Equals( "TagRatingPEGI_PRT" ) )
			{
				node->FirstChild->Value = this->hPEGI_PRT;
			}
			if( node->FirstChild->Value->Equals( "TagRatingPEGI_BBFC" ) )
			{
				node->FirstChild->Value = this->hPEGI_BBFC;
			}
			if( node->FirstChild->Value->Equals( "TagRatingOFLC" ) )
			{
				node->FirstChild->Value = this->hOFLC;
			}

			// SRLに登録されないROM仕様
			if( node->FirstChild->Value->Equals( "TagIsUGC" ) )
			{
				if( *(this->hIsUGC) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsPhotoEx" ) )
			{
				if( *(this->hIsPhotoEx) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}

			// ROM内登録データを1バイトずつ表に書き込む
			if( node->FirstChild->Value->Equals( "TagRomVersionHex" ) )
			{
				node->FirstChild->Value = hSrl->hRomVersion->ToString("X2");
			}
			System::Int32  byte;
			for( byte=0; byte < TITLE_NAME_MAX; byte++ )
			{
				if( node->FirstChild->Value->Equals( "TagTitleName" + byte.ToString() ) )
				{
					System::String ^bstr = hSrl->hTitleName[byte].ToString();
					if( bstr == nullptr )
						node->FirstChild->Value = gcnew System::String( "null" );
					else if( bstr->Equals( "\0" ) )
						node->FirstChild->Value = gcnew System::String( "\\0" );
					else if( bstr->Equals( " " ) )
						node->FirstChild->Value = gcnew System::String( "\\s" );
					else
						node->FirstChild->Value = gcnew System::String( bstr );
					//node->FirstChild->Value = gcnew System::String( hSrl->hTitleName[byte].ToString() );
				}
				else if( node->FirstChild->Value->Equals( "TagTitleNameHex" + byte.ToString() ) )
				{
					node->FirstChild->Value = title_name[byte].ToString("X2");
				}
			}
			if( node->FirstChild->Value->Equals( "TagGameCode" ) )
			{
				node->FirstChild->Value = hSrl->hGameCode;
			}
			if( node->FirstChild->Value->Equals( "TagMakerCode" ) )
			{
				node->FirstChild->Value = hSrl->hMakerCode;
			}

		} // if( (node->FirstChild != nullptr) && (node->FirstChild->Value != nullptr) )
	} // for( i=0; i < list->Count; i++ )

	try
	{
		doc->Save( hFilename );
	}
	catch ( System::Exception ^ex )
	{
		(void)ex;
		return ECDeliverableResult::ERROR_FILE_WRITE;
	}

	return ECDeliverableResult::NOERROR;
} // ECDeliverableResult RCDeliverable::writeSpreadsheet(System::String ^hFilename, MasterEditorTWL::RCSrl ^hSrl)

