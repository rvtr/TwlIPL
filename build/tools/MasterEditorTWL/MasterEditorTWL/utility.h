#pragma once

// 共用ライブラリ・クラスの宣言

#include <twl/types.h>
#include <cstring>
#include <cstdio>

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

	//
	// バイト列に特定のパターンが含まれるかどうかマッチングする
	//
	// @arg [in] テキスト
	//      [in] テキストの長さ
	//      [in] パターン
	//      [in] パターンの長さ
	//      [in] テキストの終端まで調べた時点でテキストが途中までマッチしていた場合を成功とみなすか
	//
	// @ret マッチしたテキストのインデックスをリストで返す。
	//
	System::Collections::Generic::List<u32>^ patternMatch( 
		const u8 *text, const u32 textLen, const u8 *pattern, const u32 patternLen, const System::Boolean enableLast );

	//
	// ファイルにバイト列のパターンが含まれるかどうかマッチングする
	//
	// @arg [in] マッチ対象となるファイルポインタ
	// @arg [in] パターン
	// @arg [in] パターンの長さ
	//
	// @ret マッチしたテキストのインデックスをリストで返す。
	//      最後までマッチした場合のみ成功したとみなす。
	//
	System::Collections::Generic::List<u32>^ patternMatch( FILE *fp, const u8 *pattern, const u32 patternLen );

	void debugPatternMatch( System::String ^filename );

} // end of namespace MasterEditorTWL
