// utility.h の実装

#include "stdafx.h"
#include "common.h"
#include "utility.h"
#include <cstring>
#include <cstdio>
#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/ownerInfoEx.h>

//
// String を char 配列に格納
//
// @arg [out] 格納先
// @arg [in]  格納元
// @arg [in]  文字列長
// @arg [in]  余りを埋める padding
//
void MasterEditorTWL::setStringToChars( char *pDst, System::String ^hSrc, 
									    const System::Int32 nMax, const System::SByte pad )
{
	System::Int32 i;

	memset( pDst, pad, nMax );
	for( i=0; (i < hSrc->Length) && (i < nMax); i++ )
	{
		pDst[i] = (char)hSrc[i];
	}
}

//
// ROMヘッダ中のROM容量設定バイトからROM容量を表す文字列を取得
//
// @arg [in]  ROM容量設定バイト
//
// @ret 文字列(単位つき)
//
System::String^ MasterEditorTWL::transRomSizeToString( System::Byte ^byte )
{
	System::UInt32  size;
	System::String  ^unit;

	// RomSize = 2^(byte) [Mbit] 
	size = 1 << *(byte);

	if(size >= 1024)
	{
		unit = gcnew System::String("Gbit");
		size = size / 1024;
	}
	else
	{
		unit = gcnew System::String("Mbit");
	}
	return (size.ToString() + unit);
}

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
System::String^ MasterEditorTWL::transRatingToString( System::Byte ogn, System::Boolean enable, System::Byte rating, System::Boolean english )
{
	cli::array< System::String^ > ^list;
	System::Int32  index;
	System::String ^str;

	if( ogn == OS_TWL_PCTL_OGN_CERO )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{ L"A (All ages)", L"B (aged 12 or older)", L"C (aged 15 or older)", 
			                                            L"D (aged 17 or older)", L"Z (aged 18 or older)", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"A (全年齢)", L"B (12歳以上)", L"C (15歳以上)", L"D (17歳以上)", L"Z (18歳以上)", L"未審査"};
		}

		switch( rating )
		{
			case 0:
				if( enable )	// 全年齢か未審査か判定
					index = 0;
				else
					index = 5;
			break;
			case 12: index = 1; break;
			case 15: index = 2; break;
			case 17: index = 3; break;
			case 18: index = 4; break;
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_ESRB )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"EC (aged 3 or older)", L"E (aged 6 or older)", L"E10+ (aged 10 or older)", 
			                                           L"T (aged 13 or older)",	L"M (aged 17 or older)", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"EC (3歳以上)", L"E (6歳以上)", L"E10+ (10歳以上)", L"T (13歳以上)", L"M (17歳以上)", L"未審査"};
		}

		switch( rating )
		{
			case 3:  index = 0; break;
			case 6:  index = 1; break;
			case 10: index = 2; break;
			case 13: index = 3; break;
			case 17: index = 4; break;
			case 0:  index = 5; break; // 0はデフォルト値なのでエラーメッセージを出さない(自動的に未審査扱い)
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_USK )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"All ages", L"aged 6 or older", L"aged 12 or older", L"aged 16 or older", 
			                                           L"Inadequent for young", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"年齢制限なし", L"6歳以上", L"12歳以上", L"16歳以上", L"青少年には不適切", L"未審査"};
		}

		switch( rating )
		{
			case 0:
				if( enable )
					index = 0;
				else
					index = 5;
			break;
			case 6:  index = 1; break;
			case 12: index = 2; break;
			case 16: index = 3; break;
			case 18: index = 4; break;
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_PEGI_GEN )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"aged 3 or older", L"aged 7 or older", L"aged 12 or older", L"aged 16 or older", 
			                                           L"aged 18 or older", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"3歳以上", L"7歳以上", L"12歳以上", L"16歳以上", L"18歳以上", L"未審査"};
		}

		switch( rating )
		{
			case 3:  index = 0; break;
			case 7:  index = 1; break;
			case 12: index = 2; break;
			case 16: index = 3; break;
			case 18: index = 4; break;
			case 0:  index = 5; break;
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_PEGI_PRT )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"aged 4 or older", L"aged 6 or older", L"aged 12 or older", L"aged 16 or older", 
			                                           L"aged 18 or older", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"4歳以上", L"6歳以上", L"12歳以上", L"16歳以上", L"18歳以上", L"未審査"};
		}

		switch( rating )
		{
			case 4:  index = 0; break;
			case 6:  index = 1; break;
			case 12: index = 2; break;
			case 16: index = 3; break;
			case 18: index = 4; break;
			case 0:  index = 5; break;
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_PEGI_BBFC )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"aged 3 or older", L"aged 4 or older recommended", L"aged 7 or older", L"aged 8 or older recommended",
													   L"aged 12 or older", L"aged 15 or older", L"aged 16 or older", L"aged 18 or older", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"3歳以上", L"4歳以上推奨", L"7歳以上", L"8歳以上推奨", 
			                                           L"12歳以上", L"15歳以上", L"16歳以上", L"18歳以上", L"未審査"};
		}
		switch( rating )
		{
			case 3:  index = 0; break;
			case 4:  index = 1; break;
			case 7:  index = 2; break;
			case 8:  index = 3; break;
			case 12: index = 4; break;
			case 15: index = 5; break;
			case 16: index = 6; break;
			case 18: index = 7; break;
			case 0:  index = 8; break;
			default:
				index = 8;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_OFLC )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"G", L"PG", L"M", L"MA15+", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"G", L"PG", L"M", L"MA15+", L"未審査"};
		}

		switch( rating )
		{
			case 0:
				if( enable )
					index = 0;
				else
					index = 4;
			break;
			case 7:  index = 1; break;
			case 14: index = 2; break;
			case 15: index = 3; break;
			default:
				index = 4; break;
			break;
		}
		str = list[ index ];
	}

	else
	{
		str = nullptr;
	}

	return str;
} // MasterEditorTWL::transRatingToString

//
// バイト列に特定のパターンが含まれるかどうかマッチングする
//
// @arg [in] テキスト
//      [in] テキストの長さ
//      [in] パターン
//      [in] パターンの長さ
//      [in] テキストの終端まで調べた時点でテキストが途中までマッチしていた場合を成功とみなすか
//
// @ret マッチしたテキストのオフセットをリストで返す。
//
System::Collections::Generic::List<u32>^ MasterEditorTWL::patternMatch( 
	const u8 *text, const u32 textLen, const u8 *pattern, const u32 patternLen, const System::Boolean enableLast )
{
	// ひとまずシンプルな方法で実装する
	u32  first;
	u32  len;
	System::Collections::Generic::List<u32> ^list = gcnew System::Collections::Generic::List<u32>();
	list->Clear();

	// パターンを1文字ずつずらしながらマッチング
	for( first=0; first < textLen; first++ )
	{
		len = (patternLen < (textLen-first))?patternLen:(textLen-first);	// 最後のほうは途中までしかマッチングしない
		if( *(text+first) == pattern[0] )		// 高速化のため最初の文字が一致したときのみ全文マッチング
		{
			if( memcmp( text+first, pattern, len ) == 0 )
			{
				if( (enableLast == true) )
				{
					list->Add( first );
				}
				else if( len == patternLen )	// 完全一致しないとダメ
				{
					list->Add( first );
				}
			}
		}
	}
	return list;
} // MasterEditorTWL::patternMatch

//
// ファイルにバイト列のパターンが含まれるかどうかマッチングする
//
// @arg [in] マッチ対象となるファイルポインタ
// @arg [in] パターン
// @arg [in] パターンの長さ(PATTERN_MATCH_LEN_MAX以下でなければならない)
//
// @ret マッチしたテキストのオフセットをリストで返す。
//      最後までマッチした場合のみ成功したとみなす。
//
#define PATTERN_MATCH_LEN_MAX	(10*1024)
System::Collections::Generic::List<u32>^ MasterEditorTWL::patternMatch( FILE *fp, const u8 *pattern, const u32 patternLen )
{
	u8  text[ 2 * PATTERN_MATCH_LEN_MAX ];		// バッファの切れ目を探索するため多めに読み込むので最大でパターンの2倍だけバッファが必要になる
	u32 cur;
	u32 filesize;
	u32 len;
	u32 extra;
	System::Collections::Generic::List<u32> ^tmplist;
	System::Collections::Generic::List<u32> ^list = gcnew System::Collections::Generic::List<u32>;
	list->Clear();

	if( patternLen > PATTERN_MATCH_LEN_MAX )
		return nullptr;

	fseek( fp, 0, SEEK_END );
	filesize = ftell( fp );

	cur = 0;
	while( cur < filesize )
	{
		// バッファの切れ目を調べたいため実際には(パターンの長さ-1)だけ多めにリードする
		len   = ((filesize - cur) < PATTERN_MATCH_LEN_MAX)?(filesize - cur):PATTERN_MATCH_LEN_MAX;
		extra = (len < PATTERN_MATCH_LEN_MAX)?0:(patternLen-1);
		fseek( fp, cur, SEEK_SET );
		if( (len + extra) != fread( text, 1, len + extra, fp ) )
		{
			return nullptr;
		}

		// テキスト終端に途中までマッチングしたときは失敗とみなす
		tmplist = MasterEditorTWL::patternMatch( text, len+extra, pattern, patternLen, false );
		if( tmplist != nullptr )
		{
			for each( u32 tmpval in tmplist )
			{
				list->Add( tmpval + cur );	// 実際のオフセットはファイルオフセットを加えた値
			}
		}

		// 次のSEEK位置:多めにリードしたはずらす
		cur += len;
	}
	return list;
}
