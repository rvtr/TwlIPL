 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - appjumpTest
  File:     common.h

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
#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <twl.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/* TitleID */
#define CARDAPP_TITLEID		(u64)(0x0003000034333041)	// 430A
#define CARDAPP_FAIL_TITLEID	(u64)(0x0003000034363041)	// 460A
#define CARDAPP_ANO_TITLEID	(u64)(0x0003000034363141)	// 461A

#define NANDAPP1_TITLEID	(u64)(0x0003000434333141)	// 431A
#define NANDAPP2_TITLEID	(u64)(0x0003000434333241)	// 432A

#define KEY_REPEAT_START    25  // キーリピート開始までのフレーム数
#define KEY_REPEAT_SPAN     10  // キーリピートの間隔フレーム数

/* アプリ間パラメータ関連 */
#define APPJUMP_STRING_LENGTH		24	// 引数一つ分として受け渡しされる文字列の長さ制限

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

typedef enum JumpTypeForB
{
	JUMPTYPE_RETURN = 0,
	JUMPTYPE_ANOTHER_CARD,
	JUMPTYPE_SYSMENU,
	JUMPTYPE_FAIL_CARD,
	
	JUMPTYPE_NUM
} JumpTypeForB;

// キー入力情報
typedef struct KeyInfo
{
    u16 cnt;    // 未加工入力値
    u16 trg;    // 押しトリガ入力
    u16 up;     // 離しトリガ入力
    u16 rep;    // 押し維持リピート入力
} KeyInfo;

// アプリ間でバイナリデータとして引き渡す構造体
typedef struct AppParam
{
	u32 jumpCount;		// アプリジャンプの実行回数
	u8  isAutoJump;		// 一定間隔で自動的にアプリジャンプを実行するかどうかのフラグ
	u8  rsv[3];			// 4バイトアラインメントのため
} AppParam;

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
void InitCommon(void);

void ReadKey(KeyInfo* pKey);

void VBlankIntr(void);

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* COMMON_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
