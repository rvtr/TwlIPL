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
	// サイズから単位つきの文字列を取得
	//
	// @arg [in] バイト数
	//
	// @ret 文字列(単位つき)
	//
	System::String^ transSizeToString( System::UInt32 ^size );

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
	// ペアレンタルコントロールの各団体の設定可能年齢を取得する
	//
	// @arg [in] 団体
	//
	cli::array<System::Byte>^ getOgnRatingAges( int ogn );

	//
	// ペアレンタルコントロールの各団体の団体名を取得する
	//
	// @arg [in] 団体
	//
	System::String^ getOgnName( int ogn );

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

	//
	// XMLのルートノードから指定した名前のタグを検索して返す
	//
	// @arg [in] XMLのルートノード
	// @arg [in] タグ名
	//
	// @ret 検索でマッチしたときノードを返す。ないときは nullptr。
	//      ただし、最初にマッチしたもののみ返す
	//
	System::Xml::XmlNode^ searchXmlNode( System::Xml::XmlElement ^root, System::String ^tag );

	//
	// タグを検索してそのテキストが指定したテキストと一致するか調べる
	//
	// @arg [in] XMLのルートノード
	// @arg [in] タグ名
	// @arg [in] 値
	//
	// @ret 一致するときtrue。一致しないとき、タグが存在しないときはfalse。
	//
	System::Boolean isXmlEqual( System::Xml::XmlElement ^root, System::String ^tag, System::String ^val );

	//
	// タグを検索してそのテキストを返す
	//
	// @arg [in] XMLのルートノード
	// @arg [in] タグのXPath
	//
	// @ret テキストが存在するときそのテキストを返す。存在しないときnullptr。
	//
	System::String^ getXPathText( System::Xml::XmlElement ^root, System::String ^xpath );

	//
	// XMLにタグを追加する
	//
	// @arg [in] XMLドキュメント
	// @arg [in] 親ノード
	// @arg [in] タグ名
	// @arg [in] テキスト
	//
	void appendXmlTag( System::Xml::XmlDocument ^doc, System::Xml::XmlElement ^parent,
					   System::String ^tag, System::String ^text );

} // end of namespace MasterEditorTWL
