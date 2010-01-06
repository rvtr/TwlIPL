/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     main.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
*---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "common.h"

/*
 * クローンブートを実現するアプリケーションのサンプル.
 *
 * MBライブラリのサンプルはマルチブート機能を使うため、
 * 同じ通信環境(無線あるいは有線)どうしの開発機材を複数必要とします。
 * $TwlSDK/bin/ARM9-TS/Rom/ ディレクトリにある
 * mb_child_NITRO.srl, mb_child_TWL.srl プログラムが最終実機におけるマルチブート子機と同等の
 * 機能を提供するサンプルになっていますので、
 * このバイナリをサンプルプログラムと同じ方法で他の機材に読み込み、
 * 一緒に実行してください。
 *
 */

/*
 * このデモは無線通信に WH ライブラリを使用していますが、
 * （特に子機側は）無線の終了処理を行いません。
 * WH ライブラリを用いた無線の終了処理に関しては、
 * WH ライブラリのソースコード冒頭のコメントや
 * wm/dataShare-Model デモ等をご参照ください。
 */

/******************************************************************************/



//============================================================================
//   関数定義
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         NitroMain / TwlMain

  Description:  メインルーチン

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    /*
     * このサンプルでは, multiboot-Model サンプルデモをそのまま使用し
     * 自身がDSダウンロードプレイ子機プログラムかどうかを判断して
     * 単純に処理を分けています.
     *
     * クローンブートで親機と子機との間にある主な環境相違点としては
     *  1. 子機は CARD アクセスができない
     *  2. 8kB 以内の親機専用コードを含まなければならない
     *  3. ワイヤレス通信の手順が親機/子機で異なる
     * が挙げられます.
     *
     * これらの点に注意してアプリケーション設計に合うラッパー処理で対応すれば
     * ほとんどの内容を親子で共通化しつつ一人用と対戦プレイをサポートし
     * CARD-ROM 容量も節約された効率的なプログラムを作成することが出来ます.
     * 逆に, DSダウンロードプレイの親機と子機の間に全く共通点が無ければ
     * 上に挙げたような利点は得られないでしょう.
     */
    if (!MB_IsMultiBootChild())
    {
        ParentMain();
    }
    else
    {
        ChildMain();
    }

    /* 制御はここへ及びません */
}
