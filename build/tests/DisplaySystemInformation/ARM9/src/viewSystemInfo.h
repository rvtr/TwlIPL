/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     viewSystemInfo.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef	__LOAD_VIEW_INFO__
#define	__LOAD_VIEW_INFO__

#include <twl.h>
#include "drawFunc.h"
#include "address.h"

#define DISPINFO_BUFSIZE 64

#define MODE_CHANGE_MASK	0x1
#define MODE_EXECUTE_MASK	0x2
#define MODE_RESULT_DISPLAY_MASK	0x4
#define MODE_RESULT_MASK	0x8
#define MODE_RESULT_SHIFT	3

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////

typedef enum ChangeFuncArg{
	ARG_BOOL,
	ARG_INT,
	ARG_OTHER
} ChangeFuncArg;

typedef struct DispInfoEntry
{
	BOOL	isNumData;	// 表示するときは数値データか文字列データか
	BOOL	isSjis;		// UTF16で描画するデータだけFALSE isNumData=TRUEなら未定義
	BOOL	isAligned;	// 一列に表示できるならTRUE、字下げが必要ならFALSE
	BOOL	fromLCFG;	// LCFGから取得したデータは色つけて表示
	int 	numLines;	// 項目名、項目内容を表示するのに必要な行数
	
	const char	*kind;		// 項目名
	
	union {
		char*	sjis;
		u16*	utf;
	} str;
	
	int		iValue;		// データの数値型表現(インデクスとか)
	
	BOOL	changable;	// その値が変更可能か否か
	
	// ここから先はchangableがtrueのエントリのみ設定される
	ChangeFuncArg	argType; // 値を変更するための関数の引数型
	
	// 値を変更するための関数
	union {
		void	(*cBool)(bool);
		void	(*cInt)(int);
	} changeFunc;
	
	char 		**kindNameList;	// 項目名一覧の先頭へのポインタ
	int			numKindName;	// 項目名一覧の長さ
	
} DispInfoEntry;


////////////////////////////////

// 各種本体、ユーザ情報
extern DispInfoEntry*	gAllInfo[ROOTMENU_SIZE];

extern u8 gArm7SCFGReg[DISPINFO_SHARED_SCFG_REG_SIZE];		// ARM7からのデータ取得用バッファ
extern u8 gArm7SCFGShared[DISPINFO_SHARED_SCFG_WRAM_SIZE];	// ARM7からのデータ取得用バッファ

extern BOOL gLCFGAccessible;
extern u8* gBufLCFG;

////////////////////////////////

void displayInfoMain( void );
void displayInfoInit( void );

////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif  // __LOAD_VIEW_INFO__
