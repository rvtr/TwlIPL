#pragma once

// 共用ライブラリ・クラスの宣言

#include <twl/types.h>

namespace MasterEditorTWL
{

	// -------------------------------------------------------------------
	// functions
	// -------------------------------------------------------------------

	//
	// String を char 配列に格納
	//
	// @arg [out] 格納先
	// @arg [in]  格納元
	// @arg [in]  文字列長
	// @arg [in]  余りを埋める padding
	//
	void setStringToChars( char *pDst, System::String ^hSrc, const System::Int32 nMax, const System::SByte pad  );

	//
	// ROMヘッダ中のROM容量設定バイトからROM容量を表す文字列を取得
	//
	// @arg [in]  ROM容量設定バイト
	//
	// @ret 文字列(単位つき)
	//
	System::String^ transRomSizeToString( System::Byte ^byte );

	//
	// ペアレンタルコントロールのレーティング値から文字列を取得
	//
	// @arg [in] 団体
	// @arg [in] 有効フラグ(0歳以上か未審査かを判定するために使用する)
	// @arg [in] レーティング値
	// @arg [in] 英語フラグ
	//
	// @ret レーティングの文字列(団体がおかしいときnullptr)
	//
	System::String^ transRatingToString( System::Byte ogn, System::Boolean enable, System::Byte rating, System::Boolean english ); 

} // end of namespace MasterEditorTWL
