/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     common.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
*---------------------------------------------------------------------------*/
#ifndef MB_DEMO_COMMON_H_
#define MB_DEMO_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>

#define CARD_SECURE_AREA_OFFSET       0x4000 // 16KB
#define CARD_GAME_AREA_OFFSET         0x8000 // 32KB


//============================================================================
//  関数宣言
//============================================================================


/*
 * この関数は multiboot-Model の親機側で NitroMain() だったものです.
 * 本サンプルでは MB_IsMultiBootChild() == FALSE 判定で呼び出されます.
 */
void    ParentMain(void);

/*
 * この関数は multiboot-Model の子機側で NitroMain() だったものです.
 * 本サンプルでは MB_IsMultiBootChild() == TRUE 判定で呼び出されます.
 */
void    ChildMain(void);

/*
 * この関数は親機専用領域 .parent セクションに配置されています.
 * 内容は, 単に ParentMain() を呼び出すだけです.
 */
void    ParentIdentifier(void);

/* それ以外は全て multiboot-Model と同じです */

void    CommonInit();
void    ReadKey(void);
u16     GetPressKey(void);
u16     GetTrigKey(void);
void    InitAllocateSystem(void);

/*---------------------------------------------------------------------------*
  Name:         IS_PAD_PRESS

  Description:  キー判定

  Arguments:    判定するキーフラグ

  Returns:      指定したキーが押下されている場合は TRUE
                されていない場合は FALSE
 *---------------------------------------------------------------------------*/
static inline BOOL IS_PAD_PRESS(u16 flag)
{
    return (GetPressKey() & flag) == flag;
}

/*---------------------------------------------------------------------------*
  Name:         IS_PAD_TRIGGER

  Description:  キートリガ判定

  Arguments:    判定するキーフラグ

  Returns:      指定したキーのトリガが立っている場合は TRUE
                立っていない場合は FALSE
 *---------------------------------------------------------------------------*/
static inline BOOL IS_PAD_TRIGGER(u16 flag)
{
    return (GetTrigKey() & flag) == flag;
}

/*---------------------------------------------------------------------------*
  Name:         MpSendCallback

  Description:  MPデータ送信後に呼び出される関数。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MpSendCallback(BOOL result);

/*---------------------------------------------------------------------------*
  Name:         MpReceiveCallback

  Description:  MPデータ受信時に呼び出される関数。

  Arguments:    aid     - 送信元子機の aid( 0 の場合は親機からのデータ )
                data    - 受信データへのポインタ (NULL で切断通知)
                length  - 受信データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MpReceiveCallback(u16 aid, u16 *data, u16 length);

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  親機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeParent(void);

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  子機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeChild(void);

/*---------------------------------------------------------------------------*
  Name:         BlockTransferMain

  Description:  ブロック転送状態通知関数。

  Arguments:    arg     - 通知元 WM 関数のコールバックポインタ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BlockTransferMain(void);

/*---------------------------------------------------------------------------*
  Name:         BlockTransferCallback

  Description:  ブロック転送状態通知関数。

  Arguments:    arg     - 通知元 WM 関数のコールバックポインタ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BlockTransferCallback(void *arg);

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // MB_DEMO_COMMON_H_
