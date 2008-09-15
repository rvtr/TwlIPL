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
		this->sucMsg( "設定ファイルを開くことができませんでした。", "Setting file can't be opened." );
		return;
	}

	// <init>タグ : ルート
	System::Xml::XmlElement ^root = doc->DocumentElement;

	// <rw>タグ
	System::Boolean bReadOnly = MasterEditorTWL::isXmlEqual( root, "rw", "r" );
	if( bReadOnly )
	{
		this->readOnly();
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
				u8 size = System::UInt32::Parse( MasterEditorTWL::getXPathText( root, "/init/shared2/size" + i.ToString() ) );
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
		System::String ^msgJ = gcnew System::String("動作モード:");
		System::String ^msgE = gcnew System::String("Processing Mode:");
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
System::Void Form1::saveSrl( System::String ^filename )
{
	// コピーしたファイルにROMヘッダを上書き
	if( !this->saveSrlCore( filename ) )
	{
		this->errMsg( "ROMデータの保存に失敗しました。", "Saving the ROM data file failed." );
		return;
	}
	this->sucMsg( "ROMデータの保存が成功しました。", "Saving the ROM data file succeeded." );
	this->tboxFile->Text = filename;

	// 再リード
	this->loadSrl( filename );
} // saveSrl()

// SRLの一時保存
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