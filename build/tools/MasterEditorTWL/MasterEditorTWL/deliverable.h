#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include "srl.h"

namespace MasterEditorTWL
{
	// -------------------------------------------------------------------
	// Type : enum class
	// Name : ECDeliverableResult
	//
	// Description : RCDeliverable クラスの操作でのエラーを宣言
	// -------------------------------------------------------------------
	enum class ECDeliverableResult
	{
		NOERROR   = 0,
		// エラー特定しなくても原因がわかるときの返り値
		// (エラーが生じる可能性のある箇所が1つ etc.)
		ERROR,
		// ファイル操作でのエラー
		ERROR_FILE_OPEN,
		ERROR_FILE_READ,
		ERROR_FILE_WRITE,
	};

	// -------------------------------------------------------------------
	// Type : ref class
	// Name : RCDeliverable
	//
	// Description : 提出書類クラス
	// 
	// Role : 提出情報の入出力
	// -------------------------------------------------------------------
	ref class RCDeliverable
	{
		// field
	public:
		// 提出情報
		property System::String ^hProductName;		// 製品名
		property System::String ^hProductCode1;		// 製品コード
		property System::String ^hProductCode2;		// 製品コード
		property System::Int32  ^hReleaseYear;		// 発売予定日
		property System::Int32  ^hReleaseMonth;
		property System::Int32  ^hReleaseDay;
		property System::Int32  ^hSubmitYear;		// 提出日
		property System::Int32  ^hSubmitMonth;
		property System::Int32  ^hSubmitDay;
		property System::String ^hSubmitWay;		// 提出方法
		property System::String ^hUsage;			// 用途
		property System::String ^hUsageOther;		// その他の用途
		property System::Int32  ^hSubmitVersion;	// 提出バージョン
		property System::String ^hSDK;				// SDKバージョン
		property System::Boolean ^hReleaseForeign;	// 海外版の予定
		property System::String  ^hProductNameForeign;
		property System::String  ^hProductCode1Foreign;
		property System::String  ^hProductCode2Foreign;

		// 会社情報

		// 担当者(1人目)
		property System::String  ^hCompany1;		// 会社名
		property System::String  ^hDepart1;			// 部署名
		property System::String  ^hPerson1;			// 名前
		property System::String  ^hFurigana1;		// ふりがな
		property System::String  ^hTel1;			// 電話番号
		property System::String  ^hFax1;			// FAX番号
		property System::String  ^hMail1;			// メアド
		property System::String  ^hNTSC1;			// NTSC User ID
		// 担当者(2人目)
		property System::Boolean ^hIsPerson2;		// 2人目情報を入力したか
		property System::String  ^hCompany2;
		property System::String  ^hDepart2;
		property System::String  ^hPerson2;
		property System::String  ^hFurigana2;
		property System::String  ^hTel2;
		property System::String  ^hFax2;
		property System::String  ^hMail2;
		property System::String  ^hNTSC2;

		// プログラム自己申告仕様
		property System::Boolean ^hIsWireless;			// ワイヤレス通信対応
		property System::Boolean ^hIsTouch;				// タッチスクリーン対応
		property System::Boolean ^hIsMic;				// マイク対応
		property System::Boolean ^hIsWiFi;				// Wi-Fi対応
		property System::Boolean ^hIsGBACartridge;		// GBAカートリッジ対応
		property System::Boolean ^hIsDSCartridge;			// DSカード対応
		property System::Boolean ^hIsSoftReset;			// ソフトリセット機能あり
		property System::Boolean ^hIsPictoChatSearch;	// ピクトチャットサーチあり
		property System::Boolean ^hIsClock;				// 時計機能使用
		property System::Boolean ^hIsAutoBackLightOff;	// 自動バックライトOFF機能使用
		property System::Int32   ^hTimeAutoBackLightOff;// ...................する時間
		property System::Boolean ^hIsAutoLcdOff;		// 自動LCDOFF機能を使用
		property System::Int32   ^hTimeAutoLcdOff;		// ................する時間
		property System::Boolean ^hIsSleepMode;			// スリープモード対応
		property System::Boolean ^hIsNotSleepClose;		// 本体を閉じてもスリープモードに移行しない場合あり
		property System::Int32   ^hTimeSleepClose;		// ........................................時間
		property System::Boolean ^hIsSleepAlarm;		// RTCアラームで復帰する場合あり
		property System::String  ^hProcSleepAlarm;		// .........................の処理内容
		property System::Boolean ^hIsIPLUserComment;	// IPLのユーザ名およびコメント使用
		property System::String  ^hSceneIPLUserComment;	// 上記を使用している場面(文字列で入力)
		property System::Boolean ^hIsAllIPLFonts;		// IPLで設定可能なフォントをすべて表示できる

		// プログラム自己申告仕様2
		property System::Boolean ^hIsLangJ;				// ゲーム内での使用言語
		property System::Boolean ^hIsLangE;
		property System::Boolean ^hIsLangF;
		property System::Boolean ^hIsLangG;
		property System::Boolean ^hIsLangI;
		property System::Boolean ^hIsLangS;
		property System::Boolean ^hIsLangC;
		property System::Boolean ^hIsLangK;
		property System::Boolean ^hIsLangOther;
		property System::String  ^hLangOther;
		property System::Boolean ^hIsIPLLang;			// IPLの言語設定

		// 使用ライセンス
		property System::Boolean ^hUseLcFont;			// LCフォント(SHARP)
		property System::Boolean ^hUseVx;				// VX Middleware(Actimagine)
		property System::Boolean ^hUseAtok;				// ATOK(JUSTSYSTEM)
		property System::Boolean ^hUseVoiceChat;		// VoiceChat(Abiosso)
		property System::Boolean ^hUseWiFiLib;			// WiFiライブラリ(NINTENDO)
		property System::Boolean ^hUseVoiceRecog;		// 音声認識(松下)
		property System::Boolean ^hUseCharRecog;		// 文字認識(Zi)
		property System::Boolean ^hUseVoiceCombine;		// 音声合成(SHARP)
		property System::Boolean ^hUseNetFront;			// NetFront Browser(ACCESS)
		property System::String  ^hUseOthers;			// その他(文字列で入力)

		// 備考
		property System::String  ^hCaption;

		// ROMヘッダ不記載のROMバイナリ(SRL)固有情報
		property System::String  ^hBackupMemory;		// バックアップメモリの種別

		// constructor and destructor
	public:

		// method
	public:

		//
		// 書類出力
		//
		// @arg [out] 出力ファイル名
		// @arg [in]  ROMバイナリ(SRL)固有情報
		// @arg [in]  ファイル全体のCRC
		// @arg [in]  SRLのファイル名(書類に記述するために使用)
		// @arg [in]  英語フラグ
		//
		ECDeliverableResult writeSpreadsheet( 
			System::String ^hFilename, RCSrl ^hSrl, System::UInt16 ^hCRC, System::String ^hSrlFilename, System::Boolean english );

	}; // end of ref class RCDeliverable

} // end of namespace MasterEditorTWL
