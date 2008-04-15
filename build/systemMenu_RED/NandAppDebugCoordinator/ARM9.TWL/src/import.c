/*---------------------------------------------------------------------------*
  Project:  NandAppDebugCoordinator
  File:     process_import.c

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

#include <twl.h>
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include <twl/lcfg.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "import.h"
#include "TWLHWInfo_api.h"


/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static void* spStack;
static u32  sCurrentProgress;
static vu8 sNowImport = FALSE;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static void ProgressThread(void* arg);
static void Destructor(void* arg);
void ProgressInit(void);
void ProgressDraw(f32 ratio);
static void UpdateNandBoxCount( void );

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         kamiImportTad

  Description:  .tad ファイルインポート

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/

BOOL kamiImportTad(char* path, NAMTitleId* pTitleId)
{
	NAMTadInfo tadInfo;
	OSThread thread;
	s32  nam_result;

	// tadファイルの情報取得
	if (NAM_ReadTadInfo(&tadInfo, path) != NAM_OK)
	{
		OS_Warning(" Fail! : NAM_ReadTadInfo\n");
		return FALSE;
	}

	// 後でアプリジャンプするTitleIdをここで読み取っておく
	*pTitleId = tadInfo.titleInfo.titleId;

	// Not Launch なら失敗
	if (tadInfo.titleInfo.titleId & TITLE_ID_NOT_LAUNCH_FLAG_MASK)
	{
		OS_Warning(" Fail! :  NOT_LAUNCH_FLAG is specified in rsf file\n");
		return FALSE;
	}

	// Data Only なら失敗
	if (tadInfo.titleInfo.titleId & TITLE_ID_DATA_ONLY_FLAG_MASK)
	{
		OS_Warning(" Fail! : DATA_ONLY_FLAG is specified in rsf file\n");
		return FALSE;
	}

	// NOT_LAUNCH_FLAG または DATA_ONLY_FLAG が立っていないタイトルの場合
	// freeSoftBoxCountに空きがなければインポートしない
	if (NAMUT_SearchInstalledSoftBoxCount() == LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX)
	{
		OS_Warning(" Fail! : NAND FreeSoftBoxCount == 0\n");
		return FALSE;
	}

	// インポート開始フラグを立てる
	sNowImport = TRUE;

    // 進捗スレッド作成
	spStack = OS_Alloc(THREAD_STACK_SIZE);
	MI_CpuClear8(spStack, THREAD_STACK_SIZE);
    OS_CreateThread(&thread, ProgressThread, NULL,
        (void*)((u32)spStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_GetCurrentThread()->priority - 1);
	OS_SetThreadDestructor( &thread, Destructor );
    OS_WakeupThreadDirect(&thread);

	// Import開始
	nam_result = NAM_ImportTad( path );

	// 進捗スレッドの自力終了を待つ
	while (sNowImport){};

	if ( nam_result == NAM_OK )
	{
		// InstalledSoftBoxCount, FreeSoftBoxCount の値を現在のNANDの状態に合わせて更新します。
		UpdateNandBoxCount();
		return TRUE;
	}
	else
	{
		OS_Warning(" Fail! : NAM Result Code = 0x%x\n", nam_result);
		return FALSE;
	}
}

static void Destructor(void* /*arg*/)
{
	OS_Free(spStack);
}

/*---------------------------------------------------------------------------*
  Name:         ProgressThread

  Description:  .tad ファイルインポートの進捗を表示するスレッド。
				進捗が100%に達すると処理を抜ける。

  Arguments:    arg -   使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void ProgressThread(void* /*arg*/)
{
    u32  currentSize;
    u32  totalSize   = 0;
	u32  totalSizeBk = 0;

	ProgressInit();

    while (TRUE)
    {
        NAM_GetProgress(&currentSize, &totalSize);

		if ((totalSize > 0 && totalSize == currentSize) || totalSizeBk > totalSize)
		{
			// 既にインポートが終了
			ProgressDraw((f32)1.0);
			break;	
		}
		else if (totalSize > 0)
		{
			ProgressDraw((f32)currentSize/totalSize);
		}

		totalSizeBk = totalSize;

		// Vブランク待ち
        OS_WaitVBlankIntr();
    }

	sNowImport = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressInit

  Description:  インポートの進捗を表示します

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressInit(void)
{
	sCurrentProgress = 0;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressDraw

  Description:  インポートの進捗を表示します

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressDraw(f32 ratio)
{
	char square[2] = { 0x01, 0x00 };
	u32 temp;
	s32 i;

	temp = (u32)(32 * ratio);
	if (temp > sCurrentProgress)
	{
		s32 diff = (s32)(temp - sCurrentProgress);
		for (i=0;i<diff;i++)
		{
			kamiFontPrintfConsole(2, square);
		}
	}
	sCurrentProgress = temp;

	// フォントスクリーンデータロード
	kamiFontLoadScreenData();
}

/*---------------------------------------------------------------------------*
  Name:         UpdateNandBoxCount

  Description:  InstalledSoftBoxCount, FreeSoftBoxCount の値を
				現在のNANDの状態に合わせて更新します。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void UpdateNandBoxCount( void )
{
	u32 installedSoftBoxCount;
	u32 freeSoftBoxCount;

	// InstalledSoftBoxCount, FreeSoftBoxCount を数えなおす
	installedSoftBoxCount = NAMUT_SearchInstalledSoftBoxCount();
	freeSoftBoxCount = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - installedSoftBoxCount;

	// LCFGライブラリの静的変数に対する更新
    LCFG_TSD_SetInstalledSoftBoxCount( (u8)installedSoftBoxCount );	
    LCFG_TSD_SetFreeSoftBoxCount( (u8)freeSoftBoxCount );

	// LCFGライブラリの静的変数の値をNANDに反映
    {
        u8 *pBuffer = OS_Alloc( LCFG_WRITE_TEMP );
        if( pBuffer ) {
            (void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
            OS_Free( pBuffer );
        }
    }
}

