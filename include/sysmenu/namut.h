/*---------------------------------------------------------------------------*
  Project:  TwlIPL - NAMUT
  File:     namut.h

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

#ifndef NAM_UTILITY_H_
#define NAM_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9

typedef void* (*NAMUTAlloc)(u32 size);
typedef void  (*NAMUTFree)(void* ptr);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Init

  Description:  NAMUT ライブラリの初期化を行います。

  Arguments:    allocFunc:  メモリ確保関数へのポインタ。(要：32byteアライメント）
                freeFunc:   メモリ解放関数へのポインタ。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void NAMUT_Init(NAMUTAlloc allocFunc, NAMUTFree freeFunc);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Format

  Description:  NANDの擬似フォーマット
               （システム系の必要なファイルのみを残し他を消去します）

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format(void);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_FormatCore

  Description:  本体初期化(NAND初期化)を行います。
               （システム系の必要なファイルのみを残し他を消去します
                 ユーザーアプリを common, personalizedに関わらず全て消去するか、
	　　　　　　 personalizedのみ消去するかを引数で選択できます。

  Arguments:    isForceEraseCommonETicket: TRUE の時は、common, personalizedに関わらずユーザーアプリを全消去
                                           FALSEの時は、commonETicketを残す（アプリ自身は消去）
	            isDeleteWifiSettings: WiFi設定を削除するか？（TRUEで削除）
  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_FormatCore( BOOL isForceEraseCommonETicket, BOOL isDeleteWiFiSettings );

/*---------------------------------------------------------------------------*
  Name:         NAMUT_GetSoftBoxCount

  Description:  NANDの installedカウント、freeカウントを調べて
                指定された変数に格納します。

  Arguments:    installed : installedカウント格納変数
                free      : freeカウント格納変数

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_GetSoftBoxCount( u8* installed, u8* free );

/*---------------------------------------------------------------------------*
  Name:         NAMUT_UpdateSoftBoxCount

  Description:  InstalledSoftBoxCount, FreeSoftBoxCount の値を
                現在のNANDの状態に合わせて更新します。

  Arguments:    None.

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_UpdateSoftBoxCount( void );

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataPublic

  Description:  指定したセーブデータファイルに対して
                ＦＦクリア＆フォーマットを行います。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_ClearSavedataPublic(const char* path, u64 titleID);

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataPrivate

  Description:  指定したセーブデータファイルに対して
                ＦＦクリア＆フォーマットを行います。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_ClearSavedataPrivate(const char* path, u64 titleID);

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DestroySubBanner

  Description:  指定したサブバナーのCRC破壊を試みます。
                指定したサブバナーが存在しない可能性もありますが
                その場合でもTRUEを返します。（コードはOS_DeleteSubBannerFileのパクリ）

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_DestroySubBanner(const char* path);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DeleteNandDirectory

  Description:  指定ディレクトリ以下を消去します。
                指定ディレクトリ自体は残ります。

  Arguments:    path : 絶対パス（スラッシュを含めない）

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_DeleteNandDirectory(const char *path);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DeleteNandTmpDirectory

  Description:  "nand:/tmp" ディレクトリ以下を消去します。
                例外として、"nand:/tmp/es" 以下を残します。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_DeleteNandTmpDirectory(void);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_ClearTWLSettings

  Description:  TWL本体設定データのクリアを行います。

  Arguments:    TRUE : クリアした値をNANDにライトバックします。
                FALSE: NANDへのライトバックは行いません。

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_ClearTWLSettings( BOOL doWriteback );


#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif  /* NAM_UTILITY_H_ */
