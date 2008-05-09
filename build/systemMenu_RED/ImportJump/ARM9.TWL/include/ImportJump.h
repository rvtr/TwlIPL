/*---------------------------------------------------------------------------*
  Project:  ImportJump
  File:     import.h

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

#ifndef IMPORT_JUMP_H_
#define IMPORT_JUMP_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

typedef struct _ImportJumpSetting
{
	u32	 magicCode;		             	// = TWLD
	u32	 clearPublicSaveData :1;	    // publicセーブデータをクリアする（デフォルトOFF）
	u32	 clearPrivateSaveData :1;	 	// privareセーブデータをクリアする（デフォルトOFF）
	u32	 clearSaveBannerFile:1;	     	// セーブバナーファイルをクリアする（デフォルトOFF）
	u32  importTad:1;                  	// パスで指定されたTADファイルをインポートするか（TADの更新有無に依存）
	u32	 rsv :28;                       // 予約
	u32  tadRomOffset;					// TADをロードしたエミュレーションROMオフセット
	u32	 tadLength;		                // TADファイルの長さ
} ImportJump;

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

inline ImportJump* GetImportJumpSetting(void)
{
	// リセット後は各PSRAMの先頭8MBしか保証されない
	return (ImportJump *)HW_TWL_MAIN_MEM_EX;
}

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* IMPORT_JUMP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
