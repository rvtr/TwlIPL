// deliverable.h のクラス実装

#include "stdafx.h"
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
	doc->Load( "../resource/sheet_templete.xml" );
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
	if( *(hSrl->hIsAppUser) == true )
	{
		apptype += "Type:User. ";
	}
	if( *(hSrl->hIsAppSystem) == true )
	{
		apptype += "Type:System. ";
	}
	if( *(hSrl->hIsAppLauncher) == true )
	{
		apptype += "Type:Launcher. ";
	}
	if( *(hSrl->hIsAppSecure) == true )
	{
		apptype += "Type:Secure. ";
	}
	if( *(hSrl->hIsMediaNand) == true )
	{
		apptype += "Media:NAND. ";
	}
	else
	{
		apptype += "Media:Card. ";
	}
	if( *(hSrl->hIsLaunch) == true )
	{
		apptype += "Launch. ";
	}
	else
	{
		apptype += "Not-Launch. ";
	}
	if( *(hSrl->hIsDataOnly) == true )
	{
		apptype += "DataOnly. ";
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
					node->FirstChild->Value = hSrl->hRomVersion->ToString();
				else
					node->FirstChild->Value = gcnew System::String( "E" );
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
			if( node->FirstChild->Value->Equals( "TagEULAVersion" ) )
			{
				node->FirstChild->Value = hSrl->hEULAVersion->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagTitleIDLo" ) )
			{
				node->FirstChild->Value = "0x" + hSrl->hTitleIDLo->ToString("X8");
			}
			if( node->FirstChild->Value->Equals( "TagTitleIDHi" ) )
			{
				node->FirstChild->Value = "0x" + hSrl->hTitleIDHi->ToString("X8");
			}
			if( node->FirstChild->Value->Equals( "TagAppType" ) )
			{
				node->FirstChild->Value = apptype;
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
				node->FirstChild->Value = hSrl->hPublicSize->ToString() + "Byte";
			}
			if( node->FirstChild->Value->Equals( "TagPrivateSize" ) )
			{
				node->FirstChild->Value = hSrl->hPrivateSize->ToString() + "Byte";
			}
			if( node->FirstChild->Value->Equals( "TagIsRegionJapan" ) )
			{
				if( *(hSrl->hIsRegionJapan) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsRegionAmerica" ) )
			{
				if( *(hSrl->hIsRegionAmerica) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsRegionEurope" ) )
			{
				if( *(hSrl->hIsRegionEurope) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagIsRegionAustralia" ) )
			{
				if( *(hSrl->hIsRegionAustralia) == true )
					node->FirstChild->Value = gcnew System::String("○");
				else
					node->FirstChild->Value = nullptr;
			}
			//if( node->FirstChild->Value->Equals( "TagIsRegionChina" ) )
			//{
			//	if( *(hSrl->hIsRegionChina) == true )
			//		node->FirstChild->Value = gcnew System::String("○");
			//	else
			//		node->FirstChild->Value = nullptr;
			//}
			//if( node->FirstChild->Value->Equals( "TagIsRegionKorea" ) )
			//{
			//	if( *(hSrl->hIsRegionKorea) == true )
			//		node->FirstChild->Value = gcnew System::String("○");
			//	else
			//		node->FirstChild->Value = nullptr;
			//}
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
				node->FirstChild->Value = hSrl->hShared2SizeArray[0]->ToString() + "KB";
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size1" ) )
			{
				node->FirstChild->Value = hSrl->hShared2SizeArray[1]->ToString() + "KB";
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size2" ) )
			{
				node->FirstChild->Value = hSrl->hShared2SizeArray[2]->ToString() + "KB";
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size3" ) )
			{
				node->FirstChild->Value = hSrl->hShared2SizeArray[3]->ToString() + "KB";
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size4" ) )
			{
				node->FirstChild->Value = hSrl->hShared2SizeArray[4]->ToString() + "KB";
			}
			if( node->FirstChild->Value->Equals( "TagShared2Size5" ) )
			{
				node->FirstChild->Value = hSrl->hShared2SizeArray[5]->ToString() + "KB";
			}

			// 会社情報
			if( node->FirstChild->Value->Equals( "TagCompany1" ) )
			{
				node->FirstChild->Value = this->hCompany1;
			}
			if( node->FirstChild->Value->Equals( "TagDepart1" ) )
			{
				node->FirstChild->Value = this->hDepart1;
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
			if( node->FirstChild->Value->Equals( "TagDepart2" ) )
			{
				node->FirstChild->Value = this->hDepart2;
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

			// ペアレンタルコントロール
			if( node->FirstChild->Value->Equals( "TagRatingCERO" ) )
			{
				node->FirstChild->Value = hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_CERO ]->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagRatingCEROStr" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transRatingToString( 
											OS_TWL_PCTL_OGN_CERO, 
											*(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_CERO ]),
											*(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_CERO ]),
											english );
			}
			if( node->FirstChild->Value->Equals( "TagEnableCERO" ) )
			{
				if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_CERO ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagAlwaysCERO" ) )
			{
				if( *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_CERO ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}

			if( node->FirstChild->Value->Equals( "TagRatingESRB" ) )
			{
				node->FirstChild->Value = hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_ESRB ]->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagRatingESRBStr" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transRatingToString( 
											OS_TWL_PCTL_OGN_ESRB, 
											*(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_ESRB ]),
											*(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_ESRB ]),
											english );
			}
			if( node->FirstChild->Value->Equals( "TagEnableESRB" ) )
			{
				if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_ESRB ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagAlwaysESRB" ) )
			{
				if( *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_ESRB ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}

			if( node->FirstChild->Value->Equals( "TagRatingUSK" ) )
			{
				node->FirstChild->Value = hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_USK ]->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagRatingUSKStr" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transRatingToString( 
											OS_TWL_PCTL_OGN_USK, 
											*(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_USK ]),
											*(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_USK ]),
											english );
			}
			if( node->FirstChild->Value->Equals( "TagEnableUSK" ) )
			{
				if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_USK ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagAlwaysUSK" ) )
			{
				if( *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_USK ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}

			if( node->FirstChild->Value->Equals( "TagRatingPEGI" ) )
			{
				node->FirstChild->Value = hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_GEN ]->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagRatingPEGIStr" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transRatingToString( 
											OS_TWL_PCTL_OGN_PEGI_GEN, 
											*(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_GEN ]),
											*(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_GEN ]),
											english );
			}
			if( node->FirstChild->Value->Equals( "TagEnablePEGI" ) )
			{
				if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_GEN ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagAlwaysPEGI" ) )
			{
				if( *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_PEGI_GEN ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}

			if( node->FirstChild->Value->Equals( "TagRatingPEGIPRT" ) )
			{
				node->FirstChild->Value = hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_PRT ]->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagRatingPEGIPRTStr" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transRatingToString( 
											OS_TWL_PCTL_OGN_PEGI_PRT, 
											*(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_PRT ]),
											*(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_PRT ]),
											english );
			}
			if( node->FirstChild->Value->Equals( "TagEnablePEGIPRT" ) )
			{
				if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_PRT ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagAlwaysPEGIPRT" ) )
			{
				if( *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_PEGI_PRT ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}

			if( node->FirstChild->Value->Equals( "TagRatingPEGIBBFC" ) )
			{
				node->FirstChild->Value = hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_BBFC ]->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagRatingPEGIBBFCStr" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transRatingToString( 
											OS_TWL_PCTL_OGN_PEGI_BBFC, 
											*(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_BBFC ]),
											*(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_PEGI_BBFC ]),
											english );
			}
			if( node->FirstChild->Value->Equals( "TagEnablePEGIBBFC" ) )
			{
				if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_PEGI_BBFC ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagAlwaysPEGIBBFC" ) )
			{
				if( *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_PEGI_BBFC ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}

			if( node->FirstChild->Value->Equals( "TagRatingOFLC" ) )
			{
				node->FirstChild->Value = hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_OFLC ]->ToString();
			}
			if( node->FirstChild->Value->Equals( "TagRatingOFLCStr" ) )
			{
				node->FirstChild->Value = MasterEditorTWL::transRatingToString( 
											OS_TWL_PCTL_OGN_OFLC, 
											*(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_OFLC ]),
											*(hSrl->hArrayParentalRating[ OS_TWL_PCTL_OGN_OFLC ]),
											english );
			}
			if( node->FirstChild->Value->Equals( "TagEnableOFLC" ) )
			{
				if( *(hSrl->hArrayParentalEffect[ OS_TWL_PCTL_OGN_OFLC ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
				else
					node->FirstChild->Value = nullptr;
			}
			if( node->FirstChild->Value->Equals( "TagAlwaysOFLC" ) )
			{
				if( *(hSrl->hArrayParentalAlways[ OS_TWL_PCTL_OGN_OFLC ]) )
					node->FirstChild->Value = gcnew System::String( "○" );
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
			for( byte=0; byte < GAME_CODE_MAX; byte++ )
			{
				if( node->FirstChild->Value->Equals( "TagGameCode" + byte.ToString() ) )
				{
					System::String ^bstr = hSrl->hGameCode[byte].ToString();
					if( bstr == nullptr )
						node->FirstChild->Value = gcnew System::String( "null" );
					else if( bstr->Equals( "\0" ) )
						node->FirstChild->Value = gcnew System::String( "\\0" );
					else if( bstr->Equals( " " ) )
						node->FirstChild->Value = gcnew System::String( "\\s" );
					else
						node->FirstChild->Value = gcnew System::String( bstr );
					//node->FirstChild->Value = gcnew System::String( hSrl->hGameCode[byte].ToString() );
				}
				else if( node->FirstChild->Value->Equals( "TagGameCodeHex" + byte.ToString() ) )
				{
					node->FirstChild->Value = game_code[byte].ToString("X2");
				}
			}
			for( byte=0; byte < MAKER_CODE_MAX; byte++ )
			{
				if( node->FirstChild->Value->Equals( "TagMakerCode" + byte.ToString() ) )
				{
					System::String ^bstr = hSrl->hMakerCode[byte].ToString();
					if( bstr == nullptr )
						node->FirstChild->Value = gcnew System::String( "null" );
					else if( bstr->Equals( "\0" ) )
						node->FirstChild->Value = gcnew System::String( "\\0" );
					else if( bstr->Equals( " " ) )
						node->FirstChild->Value = gcnew System::String( "\\s" );
					else
						node->FirstChild->Value = gcnew System::String( bstr );
					//node->FirstChild->Value = gcnew System::String( hSrl->hMakerCode[byte].ToString() );
				}
				else if( node->FirstChild->Value->Equals( "TagMakerCodeHex" + byte.ToString() ) )
				{
					node->FirstChild->Value = maker_code[byte].ToString("X2");
				}
			}

		} // if( (node->FirstChild != nullptr) && (node->FirstChild->Value != nullptr) )
	} // for( i=0; i < list->Count; i++ )

	doc->Save( hFilename );

	return ECDeliverableResult::NOERROR;
} // ECDeliverableResult RCDeliverable::writeSpreadsheet(System::String ^hFilename, MasterEditorTWL::RCSrl ^hSrl)

