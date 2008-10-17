// ----------------------------------------------
// ファイルのR/W
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

// 設定ファイルの読み込み
void Form1::loadInit(void)
{
	System::Xml::XmlDocument ^doc = gcnew System::Xml::XmlDocument();

	// xmlファイルの読み込み
	try
	{
		doc->Load( "../resource/ini.xml" );
	}
	catch( System::IO::FileNotFoundException ^s )
	{
		(void)s;
		this->errMsg( "設定ファイルが見つかりません。", "Setting file is not found." );
		return;
	}
	catch( System::Exception ^s )
	{
		(void)s;
		this->errMsg( "設定ファイルを開くことができませんでした。", "Setting file can't be opened." );
		return;
	}

	// <init>タグ : ルート
	System::Xml::XmlElement ^root = doc->DocumentElement;

	// <rw>タグ
	System::Boolean bReadOnly = MasterEditorTWL::isXmlEqual( root, "rw", "r" );
	this->hIsReadOnly = System::Boolean( bReadOnly );
	if( bReadOnly )
	{
		this->readOnly();
	}

	// <lang>タグ
	if( MasterEditorTWL::isXmlEqual( root, "lang", "E" ) )
	{
		this->stripItemEnglish->Checked  = true;
		this->stripItemJapanese->Checked = false;
		this->changeEnglish();
	}

	// <output>タグ
	System::Boolean bXML = MasterEditorTWL::isXmlEqual( root, "output", "XML" );

	// <spcheck>タグ
	System::Boolean bCheck = MasterEditorTWL::isXmlEqual( root, "spcheck", "ON" );

	if( bCheck )	// チェックするときのみ追加チェック項目を設定
	{
		// チェックするかどうか
		this->hSrl->hMrcSpecialList->hIsCheck = gcnew System::Boolean( true );

		// SDK
		try
		{
			u32 major   = System::UInt32::Parse( MasterEditorTWL::getXPathText( root, "/init/sdk/major" ) );
			u32 minor   = System::UInt32::Parse( MasterEditorTWL::getXPathText( root, "/init/sdk/minor" ) );
			u32 relstep = System::UInt32::Parse( MasterEditorTWL::getXPathText( root, "/init/sdk/relstep" ) );
			u32 sdkver  = (major << 24) | (minor << 16) | (relstep & 0xFFFF);
			this->hSrl->hMrcSpecialList->hSDKVer = gcnew System::UInt32( sdkver );
		}
		catch ( System::Exception ^ex )
		{
			(void)ex;
			this->errMsg( "設定ファイル中のSDKバージョンが読み込めませんでした。バージョンは0とみなされます。", 
				          "SDK ver. can't be read from setting file. Therefore it is set by 0." );
			this->hSrl->hMrcSpecialList->hSDKVer = gcnew System::UInt32( 0 );
		}

		// Shared2File
		try
		{
			System::Int32 i;
			for( i=0; i < METWL_NUMOF_SHARED2FILES; i++ )
			{
				u32 size = System::UInt32::Parse( MasterEditorTWL::getXPathText( root, "/init/shared2/size" + i.ToString() ) );
				this->hSrl->hMrcSpecialList->hShared2SizeArray[i] = gcnew System::UInt32( size );
			}
		}
		catch ( System::Exception ^ex )
		{
			(void)ex;
			this->errMsg( "設定ファイル中のShared2ファイルサイズが読み込めませんでした。サイズはすべて0とみなされます。", 
				          "One of shared2 file sizes can't be read from setting file. Therefore they are set by 0." );
			System::Int32 i;
			for( i=0; i < METWL_NUMOF_SHARED2FILES; i++ )
			{
				this->hSrl->hMrcSpecialList->hShared2SizeArray[i] = gcnew System::UInt32( 0 );
			}
		}
	} //if( bCheck )

	if( bReadOnly || bXML | bCheck )
	{
		System::String ^msgJ = gcnew System::String("[動作モード]");
		System::String ^msgE = gcnew System::String("[Processing Mode]");
		if( bReadOnly )
		{
			msgJ += "\nリードオンリーモード";
			msgE += "\nRead Only Mode";
		}
		if( bXML )
		{
			msgJ += "\nXML出力モード";
			msgE += "\nXML Output Mode";
		}
		if( bCheck )
		{
			msgJ += "\n追加チェックモード";
			msgE += "\nExtra Check Mode";
		}
		this->sucMsg( msgJ, msgE );
	}
} // loadInit()

// SRLのオープン
System::Void Form1::loadSrl( System::String ^filename )
{
	ECSrlResult result = this->hSrl->readFromFile( filename );
	if( result != ECSrlResult::NOERROR )
	{
		switch( result )
		{
			case ECSrlResult::ERROR_PLATFORM:
				this->errMsg( "本ツールはTWL対応ROM専用です。NTR専用ROMなどのTWL非対応ROMを読み込むことはできません。",
							  "This tool can only read TWL ROM. This can't read an other data e.g. NTR limited ROM." );
			break;

			case ECSrlResult::ERROR_SIGN_DECRYPT:
			case ECSrlResult::ERROR_SIGN_VERIFY:
				this->errMsg( "不正なROMデータです。TWL対応ROMでないかROMデータが改ざんされている可能性があります。",
							  "Illegal ROM data. It is not for TWL ROM, or is altered illegally." );
			break;

			default:
				this->errMsg( "ROMデータファイルの読み込みに失敗しました。\n再度「ROMデータを開く」を選択してROMデータを読み出してください。", 
					          "Reading the ROM data file failed. \nPlease read a ROM data file again, with \"Open a ROM data file\"" );
			break;
		}
		return;
	}
	this->tboxFile->Text = filename;

	// GUIにROM情報を格納
	this->setSrlForms();
	if( *(this->hIsReadOnly) )
	{
		this->readOnly();	// リードオンリーモードのときフォームをEnableにする
	}

	// SRLに登録されないROM仕様のフォーム情報も戻るボタンで読み込み時の状態に戻したいが
	// 読み込み時の状態をSRLに保存しておけないので退避しておく
	this->saveOtherForms();

	// 全体のCRCを算出
	u16  crc;
	if( !getWholeCRCInFile( filename, &crc ) )
	{
		this->errMsg( "ROMデータのCRC計算に失敗しました。ROMデータの読み込みはキャンセルされました。",
			          "Calculating CRC of the ROM data failed. Therefore reading ROM data is canceled." );
		return;
	}
	System::UInt16 ^hcrc = gcnew System::UInt16( crc );
	this->tboxWholeCRC->Clear();
	this->tboxWholeCRC->AppendText( "0x" );
	this->tboxWholeCRC->AppendText( hcrc->ToString("X") );

	// 読み込み時エラーを登録する
	this->rErrorReading->Checked = true;
	this->setGridError();
	this->setGridWarn();
	//if( this->hSrl->hErrorList->Count > 0 )
	//{
	//	this->errMsg( "ROMデータにエラーがあります。「エラー情報」タブをご確認ください。",
	//				  "ROM data include error. Please look the tab \"Setting Error\"." );
	//	return;
	//}

	return;
} // loadSrl()

// SRLの保存
System::Boolean Form1::saveSrl( System::String ^filename )
{
	// コピーしたファイルにROMヘッダを上書き
	if( !this->saveSrlCore( filename ) )
	{
		return false;
	}

	// 再リード
	this->loadSrl( filename );
	return true;
} // saveSrl()

// SRLの保存
System::Boolean Form1::saveSrlCore( System::String ^filename )
{
	// ROM情報をフォームから取得してSRLバイナリに反映させる
	this->setSrlProperties();

	// ファイルをコピー
	if( !(filename->Equals( this->tboxFile->Text )) )
	{
		System::IO::File::Copy( this->tboxFile->Text, filename, true );
	}

	// コピーしたファイルにROMヘッダを上書き
	if( this->hSrl->writeToFile( filename ) != ECSrlResult::NOERROR )
	{
		return false;
	}
	return true;
}

// ミドルウェアリストの作成
System::Void Form1::makeMiddlewareListXml(System::Xml::XmlDocument^ doc)
{
	System::Xml::XmlElement ^root = doc->CreateElement( "twl-master-editor" );
	System::Reflection::Assembly ^ass = System::Reflection::Assembly::GetEntryAssembly();
	root->SetAttribute( "version", this->getVersion() );
	doc->AppendChild( root );

	// ゲーム情報
	System::Xml::XmlElement ^game = doc->CreateElement( "game" );
	root->AppendChild( game );
	if( System::String::IsNullOrEmpty( this->tboxProductName->Text ) )
	{
		MasterEditorTWL::appendXmlTag( doc, game, "product-name", this->tboxTitleName->Text );	// 製品名が未入力のときはソフトタイトルで代用
	}
	else
	{
		MasterEditorTWL::appendXmlTag( doc, game, "product-name", this->tboxProductName->Text );
	}
	MasterEditorTWL::appendXmlTag( doc, game, "title-name",   this->tboxTitleName->Text );
	MasterEditorTWL::appendXmlTag( doc, game, "game-code",    this->tboxGameCode->Text );
	MasterEditorTWL::appendXmlTag( doc, game, "rom-version",  this->tboxRemasterVer->Text );
	MasterEditorTWL::appendXmlTag( doc, game, "submit-version", System::Decimal::ToByte(this->numSubmitVersion->Value).ToString("X") );

	// ミドルウェアリスト
	System::Xml::XmlElement ^midlist = doc->CreateElement( "middleware-list" );
	root->AppendChild( midlist );
	if( this->hSrl->hLicenseList != nullptr )
	{
		for each( RCLicense ^lic in this->hSrl->hLicenseList )
		{
			System::Xml::XmlElement ^mid = doc->CreateElement( "middleware" );
			MasterEditorTWL::appendXmlTag( doc, mid, "publisher", lic->Publisher );
			MasterEditorTWL::appendXmlTag( doc, mid, "name", lic->Name );
			midlist->AppendChild( mid );
		}
	}
}

// ミドルウェアリストの保存
System::Boolean Form1::saveMiddlewareListXml( System::String ^filename )
{
	System::Xml::XmlDocument ^doc = gcnew System::Xml::XmlDocument();
	doc->AppendChild( doc->CreateXmlDeclaration("1.0","UTF-8",nullptr) );

	this->makeMiddlewareListXml( doc );

	try
	{
		doc->Save( filename );
	}
	catch ( System::Exception ^ex )
	{
		(void)ex;
		return false;
	}
	return true;
}

// ミドルウェアリストの保存
System::Boolean Form1::saveMiddlewareListXmlEmbeddedXsl( System::String ^filename )
{
	System::Xml::XmlDocument ^doc = gcnew System::Xml::XmlDocument();
	doc->AppendChild( doc->CreateXmlDeclaration("1.0","UTF-8",nullptr) );

	// XSLを埋め込むための宣言たち
	System::Xml::XmlProcessingInstruction ^proc = doc->CreateProcessingInstruction( "xml-stylesheet", "type='text/xml' href='#stylesheet'" );
	doc->AppendChild( proc );
	System::Xml::XmlDocumentType ^doctype = doc->CreateDocumentType( "twl-master-editor", nullptr, nullptr, "<!ATTLIST xsl:stylesheet id ID #REQUIRED>" );
	doc->AppendChild( doctype );

	this->makeMiddlewareListXml( doc );

	// 埋め込みスタイルシート
	System::Xml::XmlDocument ^xsl  = gcnew System::Xml::XmlDocument();
	System::Xml::XmlElement  ^root = doc->DocumentElement;
	try
	{
		xsl->Load("../resource/middleware_e.xsl");
		root->AppendChild( doc->ImportNode(xsl->DocumentElement, true) );
	}
	catch ( System::Exception ^ex )
	{
		(void)ex;
		return false;
	}

	try
	{
		doc->Save( filename );
	}
	catch ( System::Exception ^ex )
	{
		(void)ex;
		return false;
	}
	return true;
}

// ミドルウェアリストの保存(XML->HTML変換)
System::Boolean Form1::saveMiddlewareListHtml( System::String ^filename )
{
	System::Xml::Xsl::XslCompiledTransform ^xslt = gcnew System::Xml::Xsl::XslCompiledTransform;
	System::String ^tmpxml = gcnew System::String( "middleware-tmp.xml" );

	if( !saveMiddlewareListXml(tmpxml) )
	{
		return false;
	}

	try
	{
		xslt->Load("../resource/middleware.xsl");
		xslt->Transform( tmpxml, filename );
	}
	catch (System::Exception ^s)
	{
		(void)s;
		System::IO::File::Delete( tmpxml );
		return false;
	}
	System::IO::File::Delete( tmpxml );
	return true;
}
