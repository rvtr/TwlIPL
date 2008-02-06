/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fs
  File:     fs_firm.h

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

#ifndef FIRM_FS_FS_FIRM_H_
#define FIRM_FS_FS_FIRM_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FS_InitFIRM

  Description:  initialize FS/FATFS for firm

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FS_InitFIRM( void );

/*---------------------------------------------------------------------------*
  Name:         FS_GetTitleBootContentPathFast

  Description:  NAND にインストールされているアプリの実行ファイルのパスを
                取得します。
                取得する情報の正当性を検証しないため高速ですが、
                情報が改竄されている可能性があることに注意しなければなりません。

  Arguments:    buf:        パスを格納するバッファへのポインタ。
                            FS_ENTRY_LONGNAME_MAX 以上のサイズが必要です。
                titleId:    パスを取得するアプリの Title ID。

  Returns:      正常に処理が行われたなら TRUE を返します。
 *---------------------------------------------------------------------------*/
BOOL FS_GetTitleBootContentPathFast(char* buf, u64 titleId);

/*---------------------------------------------------------------------------*
  Name:         FS_ResolveSrl

  Description:  resolve srl filename and store to HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    titleId         title id for srl file

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_ResolveSrl( u64 titleId );

/*---------------------------------------------------------------------------*
  Name:         FS_ResolveSrlUnsecured

  Description:  resolve srl filename and store to HW_TWL_FS_BOOT_SRL_PATH_BUF
                without almost security check

  Arguments:    titleId         title id for srl file

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_ResolveSrlUnsecured( u64 titleId );

#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FS_FS_FIRM_H_ */
#endif
