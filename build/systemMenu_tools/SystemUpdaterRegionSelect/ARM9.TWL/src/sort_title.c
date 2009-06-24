/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SystemUpdaterRegionSelect
  File:     sort_title.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include "sort_title.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

// ソート用 Compare 関数
static s32 TitleCompareFunc(void *elem1, void *elem2)
{
	TitleSortSet *ipp1 = (TitleSortSet *)elem1;
	TitleSortSet *ipp2 = (TitleSortSet *)elem2;
	u32 titleID_lo1 = (u32)(0xffffffff & ipp1->titleID);
	u32 titleID_lo2 = (u32)(0xffffffff & ipp2->titleID);
	BOOL isSystem1 = (u32)( 0x1 & ( ipp1->titleID >> 32 ) ) ? TRUE : FALSE;
	BOOL isSystem2 = (u32)( 0x1 & ( ipp2->titleID >> 32 ) ) ? TRUE : FALSE;
	BOOL isH1 = ( (u8)( 0xff & ( titleID_lo1 >> 24 ) ) == 'H' ) ? TRUE : FALSE;
	BOOL isH2 = ( (u8)( 0xff & ( titleID_lo2 >> 24 ) ) == 'H' ) ? TRUE : FALSE;
	
	if( isSystem1 && !isSystem2 )
	{
		// 要素１が System であり、要素２が System でない場合、要素１が前 (-1)
		return -1;
	}else if( !isSystem1 && isSystem2 )
	{
		// 要素１が System でなく、要素２が System である場合、要素１が後 (1)
		return 1;
	}else if( isH1 && !isH2 )
	{
		// 要素１が "H***" であり、要素２が "H***" でない場合、要素１が前 (-1)
		return -1;
	}else if( !isH1 && isH2 )
	{
		// 要素１が "H***" でなく、要素２が "H***" である場合、要素１が後 (1)
		return 1;
	}else
	{
		// その他の場合、titleID_loの小さいほうが前
		return ( titleID_lo1 > titleID_lo2 ) ?
				1 :
				( (titleID_lo1 < titleID_lo2) ? -1 : 0 );
	}
	
}

// TitleSortSet の配列をある法則にしたがってソートします。
// 内部で MATH_QSort を使用しているので、MATH_QSortStackSize() 関数で取得できる
// サイズの作業バッファを buf に与える必要があります。
// このサイズは (log2(num)+1) * 8 byte となっています。
// 作業バッファを渡さない場合には、スタックからこのサイズの作業領域が確保されます。
// 
// info        ソートする TitleSortSet 配列の先頭アドレス
// num         ソートする配列の要素数
// buf         作業バッファ
void SortTitle( TitleSortSet *info, u32 num, void *buf )
{
	MATH_QSort( info, num, sizeof(TitleSortSet), TitleCompareFunc, buf );
}