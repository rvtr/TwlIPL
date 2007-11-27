/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fatfs
  File:     fatfs_loader.h

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

#ifndef FIRM_FATFS_FATFS_LOADER_H_
#define FIRM_FATFS_FATFS_LOADER_H_

#include <twl/types.h>
#include <twl/fatfs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenRecentMenu

  Description:  open recent menu file

  Arguments:    driveno     drive number ('A' is 0)

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenRecentMenu( int driveno );

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenSpecifiedSrl

  Description:  open specified menu file

  Arguments:    menufile    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenSpecifiedSrl( const char* menufile );

/*---------------------------------------------------------------------------*
  Name:         FATFS_SaveSrlFilename

  Description:  store filename to HW_TWL_FS_BOOT_SRL_PATH_BUF

                ファイル名をHW_TWL_FS_BOOT_SRL_PATH_BUFに書き込みます。

  Arguments:    media       media type
                filename    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_SaveSrlFilename( FATFSMediaType media, const char* filename );

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetSrlDescriptor

  Description:  open specified menu file

                任意のファイルをオープンし、ファイルIDをmenu_fdにセットします。

  Arguments:    None

  Returns:      int
 *---------------------------------------------------------------------------*/
int FATFS_GetSrlDescriptor( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetSrlDescriptor

  Description:  set current file descriptor that was opened outside

                オープン済みのファイルIDをmenu_fdにセットします。

  Arguments:    None

  Returns:      int
 *---------------------------------------------------------------------------*/
void FATFS_SetSrlDescriptor( int fd );

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadBuffer

  Description:  load data and pass to ARM9 via WRAM[B]

                LoadBufferメカニズムで、FAT中のファイルの内容をARM9に転送します。

                [LoadBufferメカニズム]
                WRAM[B]を利用して、ARM7,ARM9間のデータ転送を行います。
                WRAM[B]の各スロットをバケツリレー方式で渡します。
                1スロット分のデータまたは全データが格納できたとき、ARM9へ
                FIRM_PXI_ID_LOAD_PIRIODを送信します。
                データ残がある場合は次のスロットの処理に移ります。
                2回目以降の呼び出しでは、前回最後のスロットの続きから使用します。
                使用したいスロットがARM9側に割り当てられているときは、ARM7側に
                なるまでストールします。

                [使用条件]
                WRAM[B]をロックせず、初期状態としてARM7側に倒しておくこと。

                [注意点]
                offsetとsizeはARM9に通知されません。別の経路で同期を取ってください。
                SRLファイルを読み込む場合は、互いにROMヘッダを参照できれば十分です。
                (ROMヘッダ部分は元から知っているはず)

                補足:
                ここでは、あるライブラリ内でARM7/ARM9側で歩調を合わせられることを
                前提にしているが、汎用的にするには(独立ライブラリ化するなら)、
                送受信でスロットを半分ずつとし、それぞれに受信側のPXIコールバック
                ＆スレッドを用意し、送信側APIがデータをWRAMに格納した後、他方に
                destとsizeを通知するという形でOKではないか？
                (で完了したら返事を返す)

                補足２：
                一度に複数スロットを使えそうな場合は、一気にpo_readした方が速い。
                ARM9からデータ内容を触る予定がないなら、本APIを使わず、普通に
                直接メインメモリに転送した方が速い。

  Arguments:    offset      offset of the file to load (512 bytes alignment)
                size        size to load

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadBuffer(u32 offset, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadHeader

  Description:  load menu header

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadHeader( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadStatic

  Description:  load static binary

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadStatic( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_Boot

  Description:  boot

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_Boot( void );


#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FATFS_FATFS_LOADER_H_ */
#endif
