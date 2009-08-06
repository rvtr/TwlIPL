 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - appjumpTest
  File:     common.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-08-30#$
  $Rev: 8166 $
  $Author: nishimoto_takashi $
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

#define KEY_REPEAT_START    25  // キーリピート開始までのフレーム数
#define KEY_REPEAT_SPAN     10  // キーリピートの間隔フレーム数

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

// キー入力情報
typedef struct KeyInfo
{
    u16 cnt;    // 未加工入力値
    u16 trg;    // 押しトリガ入力
    u16 up;     // 離しトリガ入力
    u16 rep;    // 押し維持リピート入力
} KeyInfo;

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
