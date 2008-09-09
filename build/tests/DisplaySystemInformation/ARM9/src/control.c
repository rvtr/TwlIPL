/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     control.c

  Copyright **** Nintendo.  All rights reserved.

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
#include <twl/lcfg.h>
#include "misc.h"
#include "drawFunc.h"
#include "control.h"
#include "strResource.h"
#include "viewSystemInfo.h"

#define SAVE_COUNT_MASK                 0x7f        // saveCountの値の範囲をマスクする。(0x00-0x7f）


// TSFヘッダ
typedef struct TSFHeader{
    union digest {
        u8              sha1[ SVC_SHA1_DIGEST_SIZE ];   // SHA-1ダイジェスト
        u8              rsa[ RSA_KEY_LENGTH ];          // RSA署名
        u8              dst[ RSA_KEY_LENGTH ];          // 転送用の最大サイズ要素
    }digest;
    u8                  version;                        // データver.
    u8                  saveCount;                      // セーブカウント（ミラーリングしないファイルは使用しない）
    u8                  rsv[2];                         // 予約
    u32                 bodyLength;                     // データ長
}TSFHeader; // 134bytes

static const char *s_TSDPath[] = {
    (const char *)"nand:/shared1/TWLCFG0.dat",
    (const char *)"nand:/shared1/TWLCFG1.dat",
};


static int selectLine[ROOTMENU_SIZE+1];
BOOL resetUserData( void );
BOOL breakUserData( u8 idx );
BOOL resetRTC( void );
static void TSDi_ClearSettingsDirect( LCFGTWLSettingsData *pTSD );
static BOOL LCFGi_TSD_WriteSettingsDirectForRecovery( const LCFGTWLSettingsData *pSrcInfo, int index );
static BOOL LCFGi_TSF_WriteFile( char *pPath, TSFHeader *pHeader, const void *pSrcBody, u8 *pSaveCount );



BOOL executeControl( int *line, int *changeLine, u8 *mode )
{
	BOOL controlFlag = FALSE;
	
	// 上下で項目変更
	if( ( pad.trg & PAD_KEY_UP ) || ( pad.trg & PAD_KEY_DOWN ) )
	{
		controlFlag = TRUE;
		*changeLine = !*changeLine;
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) && *changeLine  )
	{
		BOOL result = TRUE;
		controlFlag = TRUE;

		*mode |= MODE_RESULT_DISPLAY_MASK;

		switch( *line )
		{
			case MENU_RESET_INFO:
				result = resetUserData();
				break;

			case MENU_BREAK_DATA:
				result &= breakUserData(0);
				result &= breakUserData(1);
				break;

			case MENU_RTC_RESET:
				result = resetRTC();
				break;
		}
		
		*mode = (u8) ((*mode & ~MODE_RESULT_MASK ) | ( result << MODE_RESULT_SHIFT) );
	}

	// Bでキャンセルして戻る
	if( (pad.trg & PAD_BUTTON_B) || 
		( ( pad.trg & PAD_BUTTON_A ) && !*changeLine) )
	{
		controlFlag = TRUE;
		*mode = 0;
	}
	
	return controlFlag;
}

ChangeCotnrolResult changeControl( int *menu, int *line, int *changeLine, u8 *mode )
{
	int linemax = gAllInfo[*menu][*line].numKindName;
	BOOL controlFlag = FALSE;

	if( !gAllInfo[*menu][*line].changable )
	{
		// 何かの間違い
		*mode = 0;
		return CHANGE_CONTROL;
	}
		
	// 上下で項目変更
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		if( --(*changeLine) < 0 )
		{
			// ラインをデクリメントした結果マイナスになったら一番最後へ
			*changeLine = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		if( ++(*changeLine) >= linemax )
		{
			// ラインをインクリメントした結果、maxlineを超えたら最初へ
			*changeLine = 0;
		}
	}

	if( pad.trg & PAD_BUTTON_A )
	{
		switch( gAllInfo[*menu][*line].argType )
		{
			case ARG_INT:
				gAllInfo[*menu][*line].changeFunc.cInt(*changeLine);
				break;
			
			case ARG_BOOL:
				gAllInfo[*menu][*line].changeFunc.cBool(*changeLine);
				break;
				
			case ARG_OTHER:
				// 論理値でもintでも渡せない関数は残念な対応をする
				if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_RST_DSP )
				{
					*changeLine == 0 ? SCFG_ReleaseResetDSP(): SCFG_ResetDSP();
				}
				else if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_EXT_PS )
				{
					SCFGPsramBoundary idx = SCFG_PSRAM_BOUNDARY_4MB;
					
					switch(*changeLine)
					{
						case 0:
							idx = SCFG_PSRAM_BOUNDARY_4MB;
							break;
						case 1:
							idx = SCFG_PSRAM_BOUNDARY_16MB;
							break;
						case 2:
							idx = SCFG_PSRAM_BOUNDARY_32MB;
							break;
					}
					
					SCFG_SetPsramBoundary( idx );
					
				}
				
				else if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_EXT_CFG )
				{
					if( *changeLine == 0 )
					{
						SCFG_SetConfigBlockInaccessible();
					}
				}
				
				break;
		}
		
		return CHANGE_VALUE_CHANGED;
	}

	// Bでキャンセルして戻る
	if( pad.trg & PAD_BUTTON_B )
	{
		controlFlag = TRUE;
		*mode = 0;
	}
	
	return controlFlag ? CHANGE_CONTROL : CHANGE_NOTHING ;
}


BOOL control( int *menu, int *line, int *changeLine, u8 *mode )
{
	int linemax = s_numMenu[*menu]; // 選択中ページの項目数
	BOOL controlFlag = FALSE;		// 何か操作があったらTRUEになる

	// 上下で項目変更
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		if( --(*line) < 0 )
		{
			// ラインをデクリメントした結果マイナスになったら一番最後へ
			*line = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		if( ++(*line) >= linemax )
		{
			// ラインをインクリメントした結果、maxlineを超えたら最初へ
			*line = 0;
		}
	}

	// 左右でページ送り
	if( pad.trg & PAD_KEY_RIGHT )
	{
		controlFlag = TRUE;
		*line += DISP_NUM_LINES - 2;
		
		if( *line >= linemax )
		{
			*line = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_LEFT )
	{
		controlFlag = TRUE;
		*line -= DISP_NUM_LINES - 2;
		
		if( *line < 0 )
		{
			*line = 0;
		}
	}

	// Aボタン
	if( pad.trg & PAD_BUTTON_A )
	{
		if(*menu == MENU_ROOT)
		{
			controlFlag = TRUE;
			
			switch( *line )
			{
				case MENU_ROOT :
				case MENU_OWNER:
				case MENU_PARENTAL:
				case MENU_SECURE_USER:
				case MENU_OTHER:
				case MENU_NORMAL_HW:
				case MENU_SECURE_HW:
				case MENU_SCFG_ARM7:
				case MENU_SCFG_ARM9:
				case MENU_SYSMENU:
				case MENU_FONT:
				case MENU_WL:
				case MENU_WHITE:
				case MENU_VERSION:
					// 今の画面の選択位置を記録
					selectLine[ROOTMENU_SIZE] = *line;

					// 次のメニュー画面を開く
					*menu = *line;
					*line = selectLine[*menu];
					break;
				
				case MENU_RESET_INFO:
				case MENU_BREAK_DATA:
				case MENU_RTC_RESET:
					*mode |= MODE_EXECUTE_MASK;
					*changeLine = 0;	// デフォルト位置をcancelにしておく
					break;
			}
		}
		else if( *menu != MENU_VERSION && *menu != MENU_FONT && gAllInfo[*menu][*line].changable )
		{
			controlFlag = TRUE;

			// 変更可能な項目は変更画面を開く
			*mode |= MODE_CHANGE_MASK;
			*changeLine = gAllInfo[*menu][*line].iValue;
		}
		
	}
	
	if( pad.trg & PAD_BUTTON_B )
	{
		if( *menu != MENU_ROOT )
		{
			controlFlag = TRUE;

			// 設定値表示画面のときはルートに戻る
			selectLine[*menu] = *line;
			*menu = MENU_ROOT;
			*line = selectLine[ROOTMENU_SIZE];
		}
	}

	if( ( pad.trg & PAD_BUTTON_SELECT ) && *menu == MENU_SCFG_ARM7 )
	{
		controlFlag = TRUE;
		
		// ARM7SCFGの表示データを切り替える
		switchViewMode();
	}
		
	return controlFlag;
}

BOOL resetUserData( void )
// idx(0 or 1)番目のユーザデータをリセットする
{
	LCFGTWLTPCalibData calib;
	// DisplayInitで一回Readしてるからいらんかった
//	u8 *dataBuf = (u8*) Alloc (LCFG_READ_TEMP);
	
//	LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ])dataBuf );

	// バックアップをつぶすために二回書き込む
	LCFG_TSD_GetTPCalibration( &calib );					// TPキャリブレーションは残す。
	LCFG_ClearTWLSettings( LCFG_TSD_GetInstalledSoftBoxCount() );
	LCFG_TSD_SetTPCalibration( &calib );
	
	if( ! LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ]) gBufLCFG ) )
	{
		OS_TPrintf( "Write Setting failed.\n" );
		return FALSE;
	}

	if( ! LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ]) gBufLCFG ) )
	{
		OS_TPrintf( "Write Setting failed.\n" );
		return FALSE;
	}
	
	OS_TPrintf("Clear Setting Succeeded.\n" );

//	TSDi_ClearSettingsDirect( (LCFGTWLSettingsData *)(&dataBuf[ LCFG_TEMP_BUFFER_SIZE*idx ]) );
//	LCFGi_TSD_WriteSettingsDirectForRecovery( (LCFGTWLSettingsData *)&dataBuf[ LCFG_TEMP_BUFFER_SIZE*idx ], idx );
	
	return TRUE;
}

BOOL breakUserData( u8 idx )
{
	// LCFG APIを使わずに、FSレベルでファイルを読んで、データを破壊してから書き戻す
	FSFile file;
	u8 *fileBuf = (u8*) Alloc ( LCFG_TEMP_BUFFER_SIZE );
	
	FS_InitFile( &file );

	if( !FS_OpenFileEx( &file, s_TSDPath[idx], FS_FILEMODE_R | FS_FILEMODE_W ) )
	{
		OS_TPrintf("OpenFile failed. result: %d path: %s\n", FS_GetArchiveResultCode(&file), s_TSDPath[idx]);
		return FALSE;
	}

	// 適当にデータを壊す
	MI_CpuFill8( fileBuf, 0xFF, LCFG_TEMP_BUFFER_SIZE );
	
	// データの書き戻し
	FS_SeekFileToBegin( &file );
	
	if( FS_WriteFile( &file, fileBuf, LCFG_TEMP_BUFFER_SIZE ) == -1 )
	{
		OS_TPrintf("writeFile failed. path: %s\n", s_TSDPath[idx]);
		return FALSE;
	}
	
	FS_CloseFile( &file );

	OS_TPrintf("Breaking UserData Succeeded. path: %s\n", s_TSDPath[idx]);
	return TRUE;
}


BOOL resetRTC( void )
{
	RTCResult result;
	RTCDate date;
	RTCTime time;
	
	date.year = 0;
	date.month = 1;
	date.day = 1;
	date.week = RTC_WEEK_SUNDAY;
	
	time.hour = 0;
	time.minute = 0;
	time.second = 0;
	
	// resultは成功したらゼロが返る
	result = RTC_SetDate( &date );
	if( result != RTC_RESULT_SUCCESS )
	{
		OS_TPrintf("RTC Date Reset Failed. err: %d", result );
		return FALSE;
	}
	
	result |= RTC_SetTime( &time );
	if( result != RTC_RESULT_SUCCESS )
	{
		OS_TPrintf("RTC Time Reset Failed. err: %d", result );
		return FALSE;
	}

	return TRUE;
}