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

#define IMPORT_TAD_ROM_OFS       0x00800000
#define IMPORT_JUMP_SETTING_OFS  (IMPORT_TAD_ROM_OFS - CARD_ROM_PAGE_SIZE)

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

ImportJump* GetImportJumpSetting( void );

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* IMPORT_JUMP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
