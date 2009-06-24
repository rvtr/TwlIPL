/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     sort_title.h

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

/*

【つかいかた】

TitleID と パス文字列の先頭アドレス を TitleSortSet 構造体にセットして
TitleSortSet 構造体を SortTitle 関数に引き渡します。

パス文字列の実体は、適当にどこかに格納しておいて弄らないようにしてください。
次に使用例を示します。


【擬似コード】

#define FILE_NUM_MAX         256         // 手抜き
#define QSORT_BUF_SIZE       ((8+1) * 8) // サイズは(log2(num)+1) * 8 bytes 

	char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
	TitleSortSet sTitleSortSet[FILE_NUM_MAX];
	FSDirectoryEntryInfo   info[1];
	NAMTadInfo tadInfo;
	char qsortBuf[QSORT_BUF_SIZE];

	while (ファイル全部舐めるまで)
	{
		// ～ info にファイル情報読み込み～
		// ～ tadInfo に tad 情報取得～
		
		STD_CopyString( sFilePath[ counter ], info->longname );
		sTitleSortSet[ counter ].titleID = tadInfo.titleInfo.titleId;
		sTitleSortSet[ counter ].path = sFilePath[ counter ];
		
		counter++;
	}
	
	SortTitle( sTitleSortSet, counter, qsortBuf );
	
	// ソート完了

*/

#ifndef TWL_SORT_TITLE_H_
#define TWL_SORT_TITLE_H_

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#ifdef  __cplusplus
extern "C" {
#endif

#include <twl.h>

/*===========================================================================*/

typedef struct {
	OSTitleId titleID;
	char *path;
} TitleSortSet;

// TitleSortSet の配列をある法則にしたがってソートします。
// 内部で MATH_QSort を使用しているので、MATH_QSortStackSize() 関数で取得できる
// サイズの作業バッファを buf に与える必要があります。
// このサイズは (log2(num)+1) * 8 byte となっています。
// 作業バッファを渡さない場合には、スタックからこのサイズの作業領域が確保されます。
// 
// info        ソートする TitleSortSet 配列の先頭アドレス
// num         ソートする配列の要素数
// buf         作業バッファ
void SortTitle( TitleSortSet *info, u32 num, void *buf );

/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_SORT_TITLE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
