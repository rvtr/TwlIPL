/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - fs
  File:     fs_wram.h

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

#ifndef TWL_FS_WRAM_H_
#define TWL_FS_WRAM_H_

#ifdef SDK_TWL
#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    FSWramCallback
        Read/Write中のWRAMにデータがある状態で呼び出されるAPI
        改ざんした場合は、改ざん後のデータが有効となることに注意
        addr        データがあるアドレス
        orig_addr   本来のRead格納先/Write元アドレス
        len         有効なデータサイズ
        wram        使用しているWRAM
        slot        使用しているスロット
        arg         APIに渡した引数
*/
typedef void (*FSWramCallback)(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg);

/*
    FS_InitWramTransfer
        一度だけ呼び出しておく必要がある
        priority        立ち上げるスレッドの優先順位
*/
BOOL FS_InitWramTransfer( u32 priority );
/*
    FS_ReadFileViaWram
        FS/FATFSに対して同期Readを行う
        完了するまで返ってこない点に注意
        p_file      FSでオープンしたファイル (シーク済み)
        dst         読み出し先
        len         読み出しサイズ
        wram        供出するWRAM (B or C)
        slot        供出する先頭スロット (だれも確保していない＆ARM7/ARM9両方で確保可能であること)
        size        供出するサイズ
        callback    Read中にWRAMにデータがある段階で随時呼び出されるコールバック
        arg         コールバックに渡される引数
*/
BOOL FS_ReadFileViaWram( FSFile *p_file, void *dst, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg );
/*
    FS_WriteFileViaWram
        FS/FATFSに対して同期Writeを行う
        完了するまで返ってこない点に注意
        p_file      FSでオープンしたファイル (シーク済み)
        src         書き込み元
        len         書き込みサイズ
        wram        供出するWRAM (B or C)
        slot        供出する先頭スロット (だれも確保していない＆ARM7/ARM9両方で確保可能であること)
        size        供出するサイズ
        callback    Write中にWRAMにデータがある段階で随時呼び出されるコールバック
        arg         コールバックに渡される引数
*/
BOOL FS_WriteFileViaWram( FSFile *p_file, const void *src, s32 len, MIWramPos wram, s32 slot, MIWramSize size, FSWramCallback callback, void* arg );
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_FS_WRAM_H_ */
