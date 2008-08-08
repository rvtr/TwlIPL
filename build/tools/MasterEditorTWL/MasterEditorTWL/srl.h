#pragma once

// ROMデータ(SRL)クラスと関連クラスの宣言

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
		ERROR_SIGN_ENCRYPT,
		ERROR_SIGN_DECRYPT,
		// CRC算出でのエラー
		ERROR_SIGN_CRC,
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
		//property System::Byte    ^hForKorea;
		//property System::Byte    ^hForChina;
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
		property System::Byte    ^hEULAVersion;		// 編集可能
		property System::UInt32  ^hTitleIDLo;
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
		property System::UInt32  ^hShared2Size0;
		property System::UInt32  ^hShared2Size1;
		property System::UInt32  ^hShared2Size2;
		property System::UInt32  ^hShared2Size3;
		property System::UInt32  ^hShared2Size4;
		property System::UInt32  ^hShared2Size5;

		// カードリージョン Read Only
		property System::Boolean ^hIsRegionJapan;
		property System::Boolean ^hIsRegionAmerica;
		property System::Boolean ^hIsRegionEurope;
		property System::Boolean ^hIsRegionAustralia;
		//property System::Boolean ^hRegionChina;
		//property System::Boolean ^hRegionKorea;

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

		// SRLバイナリから特殊な設定を調べる
		ECSrlResult hasDSDLPlaySign( FILE *fp );
				// DSダウンロード署名がSRLに格納されているか調べる
				// @arg [in]  入力ファイルのFP (->SRL読み込み時に実行されるべき)

	}; // end of ref class RCSrl

} // end of namespace MasterEditorTWL
