#pragma once

// ROMデータ(SRL)クラスと関連クラスの宣言

#include <apptype.h>
#include "common.h"
#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/ownerInfoEx.h>

namespace MasterEditorTWL
{
	// -------------------------------------------------------------------
	// Type : enum class
	// Name : ECSrlResult
	//
	// Description : RCSrlクラスの操作でのエラーを宣言
	// -------------------------------------------------------------------
	enum class ECSrlResult
	{
		NOERROR   = 0,
		// エラー特定しなくても原因がわかるときの返り値
		// (エラーが生じる可能性のある箇所が1つ etc.)
		ERROR,
		// ファイル操作でのエラー
		ERROR_FILE_OPEN,
		ERROR_FILE_READ,
		ERROR_FILE_WRITE,
		// 署名でのエラー
		ERROR_SIGN_ENCRYPT,	// 署名できない
		ERROR_SIGN_DECRYPT,	// 署名解除できない
		ERROR_SIGN_VERIFY,	// 署名解除した後のダイジェストが合わない
		// CRC算出でのエラー
		ERROR_SIGN_CRC,
		// SDKバージョン取得でのエラー
		ERROR_SDK,
		// プラットフォームがNTR専用 or 不正バイナリ
		ERROR_PLATFORM,
	};

	// -------------------------------------------------------------------
	// Type : ref class
	// Name : RCSDKVersion
	//
	// Description : RCSrlクラスに持たせるSDKバージョン情報クラス
	// 
	// Role : 構造体としてデータをまとめておく
	// -------------------------------------------------------------------
	ref class RCSDKVersion
	{
	private:
		System::String  ^hVersion;
		System::UInt32  ^hCode;
		System::Boolean ^hIsStatic;
	private:
		RCSDKVersion(){}	// 封じる
	public:
		RCSDKVersion( System::String ^ver, System::UInt32 code, System::Boolean isStatic )	// 生成時にのみフィールドを設定可能
		{
			if( ver == nullptr )
				this->hVersion = gcnew System::String("");	// NULL参照バグを避ける
			else
				this->hVersion  = ver;
			this->hCode     = gcnew System::UInt32( code );
			this->hIsStatic = gcnew System::Boolean( isStatic );
		}
	public:
		property System::String ^Version	// 生成後にはフィールドはRead Only
		{
			System::String^ get(){ return this->hVersion; }
		}
	public:
		property System::Boolean IsStatic
		{
			System::Boolean get(){ return *(this->hIsStatic); }
		}
	public:
		property System::UInt32 Code
		{
			System::UInt32 get(){ return *(this->hCode); }
		}
	};

	// -------------------------------------------------------------------
	// Type : ref class
	// Name : RCLicense
	//
	// Description : RCSrlクラスに持たせるライセンス情報クラス
	// 
	// Role : 構造体としてデータをまとめておく
	// -------------------------------------------------------------------
	ref class RCLicense
	{
	private:
		System::String ^hPublisher;
		System::String ^hName;
	private:
		RCLicense(){}	// 封じる
	public:
		RCLicense( System::String ^pub, System::String ^name )	// 生成時にのみフィールドを設定可能
		{
			if( pub == nullptr )
				this->hPublisher = gcnew System::String("");
			else
				this->hPublisher = pub;

			if( name == nullptr )
				this->hName = gcnew System::String("");
			else
				this->hName = name;
		}
	public:
		property System::String ^Name	// 生成後にはフィールドはRead Only
		{
			System::String^ get(){ return this->hName; }
		}
	public:
		property System::String ^Publisher
		{
			System::String^ get(){ return this->hPublisher; }
		}
	};

	// -------------------------------------------------------------------
	// Type : ref class
	// Name : RCMrcError
	//
	// Description : RCSrlクラスに持たせるMRCエラー情報クラス
	// 
	// Role : 構造体としてデータをまとめておく
	// -------------------------------------------------------------------
	public ref class RCMrcError
	{
	private:
		System::String  ^hName;		// 項目名
		System::UInt32  ^hBegin;	// 開始アドレス
		System::UInt32  ^hEnd;		// 終了アドレス
		System::String  ^hMsg;		// エラーメッセージ
		System::String  ^hNameE;	// 英語版
		System::String  ^hMsgE;
		System::Boolean ^hEnableModify;	// マスタエディタで修正可能かどうか
		System::Boolean ^hAffectRom;	// 変更するとSRL(ROMバイナリ)が変更されるか
	private:
		RCMrcError(){}		// 封じる
	public:
		RCMrcError( 
			System::String ^name,  System::UInt32 beg,   System::UInt32 end, System::String ^msg, 
			System::String ^nameE, System::String ^msgE, System::Boolean enableModify, System::Boolean affectRom )
		{
			if( name == nullptr )
				this->hName = gcnew System::String("");
			else
				this->hName = name;

			if( nameE == nullptr )
				this->hNameE = gcnew System::String("");
			else
				this->hNameE = nameE;

			this->hBegin = gcnew System::UInt32( beg );
			this->hEnd   = gcnew System::UInt32( end );

			if( msg == nullptr )
				this->hMsg = gcnew System::String("");
			else
				this->hMsg = msg;

			if( msgE == nullptr )
				this->hMsgE = gcnew System::String("");
			else
				this->hMsgE = msgE;

			this->hEnableModify = gcnew System::Boolean( enableModify );
			this->hAffectRom    = gcnew System::Boolean( affectRom );
		}
	public:
		property System::Boolean EnableModify
		{
			System::Boolean get(){ return *(this->hEnableModify); }		// Read Only
		}
		property System::Boolean AffectRom
		{
			System::Boolean get(){ return *(this->hAffectRom); }
		}
	public:
		// gridViewの表示形式にあわせる
		cli::array<System::Object^>^ getAll( System::Boolean isJapanese )
		{
			if( (*this->hBegin == METWL_ERRLIST_NORANGE) && (*this->hEnd == METWL_ERRLIST_NORANGE) )
			{
				if( isJapanese )
					return (gcnew array<System::Object^>{this->hName,  "-", "-", this->hMsg});
				else
					return (gcnew array<System::Object^>{this->hNameE,  "-", "-", this->hMsgE});
			}

			if( isJapanese )
				return (gcnew array<System::Object^>{this->hName,  this->hBegin->ToString("X04")+"h", this->hEnd->ToString("X04")+"h", this->hMsg});
			else
				return (gcnew array<System::Object^>{this->hNameE, this->hBegin->ToString("X04")+"h", this->hEnd->ToString("X04")+"h", this->hMsgE});
		}
	};

	// -------------------------------------------------------------------
	// Type : ref class
	// Name : RCMrcSpecialList
	//
	// Description : MRCの追加エラー項目クラス
	// 
	// Role : 構造体としてデータをまとめておく
	// -------------------------------------------------------------------
	ref class RCMrcSpecialList
	{
	public:
		property System::Boolean ^hIsCheck;
		property System::UInt32  ^hSDKVer;
		property System::Byte    ^hEULAVer;
		property cli::array<System::UInt32^> ^hShared2SizeArray;
	public:
		RCMrcSpecialList()
		{
			this->hIsCheck = gcnew System::Boolean( false );
			this->hSDKVer  = gcnew System::UInt32( 0 );
			this->hEULAVer = gcnew System::Byte( 0 );
			this->hShared2SizeArray = gcnew cli::array<System::UInt32^>(METWL_NUMOF_SHARED2FILES);	// ファイルサイズの数に合わせる
			System::Int32 i;
			for( i=0; i < METWL_NUMOF_SHARED2FILES; i++ )
			{
				this->hShared2SizeArray[i] = gcnew System::UInt32( 0 );
			}
		}
	};

	// -------------------------------------------------------------------
	// Type : ref class
	// Name : RCSrl
	//
	// Description : ROMデータ(SRL)の設定情報クラス
	// 
	// Role : ROMデータのファイル入出力・内部情報の更新
	// -------------------------------------------------------------------
	ref class RCSrl
	{
		// field
	private:
		// ROMヘッダ
		ROM_Header *pRomHeader;

	public:
		// (GUIに表示される)ROMヘッダ固有情報

		// NTR互換情報 ReadOnly
		property System::String  ^hTitleName;
		property System::String  ^hGameCode;
		property System::String  ^hMakerCode;
		property System::String  ^hPlatform;
		property System::String  ^hRomSize;
		property System::Byte    ^hRomVersion;
		property System::UInt16  ^hHeaderCRC;
		property System::String  ^hLatency;		// MROM/1TROM/Illegal

		// ペアレンタルコントロール
		property cli::array<System::Byte^>    ^hArrayParentalRating;	// 各団体での制限年齢
		property cli::array<System::Boolean^> ^hArrayParentalEffect;	// 制限有効フラグ
		property cli::array<System::Boolean^> ^hArrayParentalAlways;	// 制限強制有効フラグ

		// TWL専用情報 一部編集可能
		property System::UInt32  ^hNormalRomOffset;
		property System::UInt32  ^hKeyTableRomOffset;
		property System::String  ^hTitleIDLo;
		property System::UInt32  ^hTitleIDHi;
		property System::Boolean ^hIsAppLauncher;	// TitleIDLoからわかるアプリ種別
		property System::Boolean ^hIsAppUser;		// TitleIDHiからわかるアプリ種別
		property System::Boolean ^hIsAppSystem;		//
		property System::Boolean ^hIsAppSecure;		//
		property System::Boolean ^hIsLaunch;		//
		property System::Boolean ^hIsMediaNand;		//
		property System::Boolean ^hIsDataOnly;		//
		property System::UInt16  ^hPublisherCode;	//
		property System::UInt32  ^hPublicSize;
		property System::UInt32  ^hPrivateSize;
		property System::Boolean ^hIsNormalJump;
		property System::Boolean ^hIsTmpJump;
		property System::Boolean ^hHasDSDLPlaySign;	// ROMヘッダ外のSRLからわかる署名の有無
		property System::Boolean ^hIsOldDevEncrypt;	// 旧開発用暗号フラグが立っている
		property System::Boolean ^hIsSCFGAccess;		// SCFGレジスタをロックしている

		// TWL拡張フラグ 一部編集可能
		property System::Boolean ^hIsCodecTWL;
		property System::Boolean ^hIsEULA;			// 編集可能
		property System::Boolean ^hIsSubBanner;
		property System::Boolean ^hIsWiFiIcon;		// 編集可能
		property System::Boolean ^hIsWirelessIcon;	// 編集可能
		property System::Boolean ^hIsWL;

		// TWLアクセスコントロール Read Only
		property System::Boolean ^hIsCommonClientKey;
		property System::Boolean ^hIsAesSlotBForES;
		property System::Boolean ^hIsAesSlotCForNAM;
		property System::Boolean ^hIsSD;
		property System::Boolean ^hIsNAND;
		property System::Boolean ^hIsGameCardOn;
		property System::Boolean ^hIsShared2;
		property System::Boolean ^hIsAesSlotBForJpegEnc;
		property System::Boolean ^hIsGameCardNitro;
		property System::Boolean ^hIsAesSlotAForSSL;
		property System::Boolean ^hIsCommonClientKeyForDebugger;

		// Shared2ファイルサイズ Read Only
		property cli::array<System::UInt32^> ^hShared2SizeArray;

		// カードリージョン Read Only
		property System::Boolean ^hIsRegionJapan;
		property System::Boolean ^hIsRegionAmerica;
		property System::Boolean ^hIsRegionEurope;
		property System::Boolean ^hIsRegionAustralia;

		// SDKバージョンと使用ライブラリのリスト
		property System::Collections::Generic::List<RCSDKVersion^> ^hSDKList;
		property System::Collections::Generic::List<RCLicense^> ^hLicenseList;

		// MRC機能でチェックされたエラー情報のリスト
		property System::Collections::Generic::List<RCMrcError^> ^hErrorList;
		property System::Collections::Generic::List<RCMrcError^> ^hWarnList;

		// MRC追加項目
		property RCMrcSpecialList ^hMrcSpecialList;

		// constructor and destructor
	public:
		RCSrl();
		~RCSrl();

		// method
	public:

		//
		// ROMヘッダのファイル入出力
		//
		// @arg [in/out] 入出力ファイル名
		//
		ECSrlResult readFromFile ( System::String ^filename );
		ECSrlResult writeToFile( System::String ^filename );

		// internal method
	private:
		// ROM固有情報とROMヘッダの設定
		ECSrlResult setRomInfo(void);		// ROMヘッダから取得したROM固有情報をフィールドに反映させる
		ECSrlResult setRomHeader(void);		// ROMヘッダにROM固有情報フィールドの値を反映させる

		// ROMヘッダの更新
		ECSrlResult calcRomHeaderCRC(void);	// ROMヘッダのCRCを再計算
		ECSrlResult signRomHeader(void);	// ROMヘッダ更新後の再署名
		ECSrlResult decryptRomHeader(void);	// ROMヘッダの署名を外す

		// SRLバイナリから特殊な設定を調べる
		ECSrlResult hasDSDLPlaySign( FILE *fp );
				// DSダウンロード署名がSRLに格納されているか調べる
				// @arg [in]  入力ファイルのFP (->SRL読み込み時に実行されるべき)]
		ECSrlResult searchSDKVersion( FILE *fp );		// SDKバージョンを取得する
		ECSrlResult searchLicenses( FILE *fp );			// 使用ライセンスを取得する

		// MRC(Master ROM Checker)機能
		ECSrlResult mrc( FILE *fp );
		ECSrlResult mrcNTR( FILE *fp );
		ECSrlResult mrcTWL( FILE *fp );
		ECSrlResult mrcTWLParentalControl(void);
		System::Boolean mrcRegion( System::UInt32 region );		// @ret リージョンが正しいかどうか
		void mrcRegionOrganization( System::UInt32 region );
		void mrcRating( System::Byte ogn );
		void mrcPadding( FILE *fp );
		void mrcBanner( FILE *fp );

		// utility
		void clearParentalControl( System::Byte ogn );

	}; // end of ref class RCSrl

} // end of namespace MasterEditorTWL
