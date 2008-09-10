/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - mi
  File:     mi_loader.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef FIRM_MI_LOADER_H_
#define FIRM_MI_LOADER_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         MIi_LoadBuffer

  Description:  receive data from ARM7 and store(move) via WRAM[B]

                LoadBufferメカニズムで、ファイルの内容をARM7から受け取ります。
                引数でSVCSHA1Contextを指定していた場合、コピーのついでにSHA1の
                計算も行います。

                [LoadBufferメカニズム]
                WRAM[B]を利用して、ARM7,ARM9間のデータ転送を行います。
                WRAM[B]の各スロットをバケツリレー方式で渡します。
                1スロット分のデータまたは全データが格納されたとき、ARM7から
                FIRM_PXI_ID_LOAD_PIRIODを受信します。
                ARM9は受信後にそのスロットの使用権をARM9に変更してデータを
                取り出し、完了後にメモリをクリアして(セキュリティ)、使用権を
                ARM7に戻します。

                [使用条件]
                WRAM[B]をロックせず、初期状態としてARM7側に倒しておくこと。

                [注意点]
                offsetとsizeはARM7から通知されません。別の経路で同期を取ってください。
                SRLファイルを読み込む場合は、互いにROMヘッダを参照できれば十分です。
                (ROMヘッダ部分は元から知っているはず)

                補足:
                ここでは、あるライブラリ内でARM7/ARM9側で歩調を合わせられることを
                前提にしているが、汎用的にするには(独立ライブラリ化するなら)、
                送受信でスロットを半分ずつとし、それぞれに受信側のPXIコールバック
                ＆スレッドを用意し、送信側APIがデータをWRAMに格納した後、他方に
                destとsizeを通知するという形でOKではないか？
                (で完了したら返事を返す)

  Arguments:    dest        destination address for received data
                size        size to load
                ctx         context for SHA1 if execute SVC_SHA1Update

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadBuffer(u8* dest, u32 size, SVCSHA1Context *ctx);

/*---------------------------------------------------------------------------*
  Name:         MI_LoadHeader

  Description:  load header

  Arguments:    pool        pointer to the pool info for SVCSignHeapContext
                rsa_key     key address

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadHeader( SVCSignHeapContext* pool, const void* rsa_key );

/*---------------------------------------------------------------------------*
  Name:         MI_LoadStatic

  Description:  load static binary

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadStatic( void );

/*---------------------------------------------------------------------------*
  Name:         MI_Boot

  Description:  boot

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_Boot( void );

#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_MI_LOADER_H_ */
#endif
