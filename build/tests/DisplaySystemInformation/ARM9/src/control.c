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
void resetUserData( int idx );
void breakUserData( int idx );
static void TSDi_ClearSettingsDirect( LCFGTWLSettingsData *pTSD );
static BOOL LCFGi_TSD_WriteSettingsDirectForRecovery( const LCFGTWLSettingsData *pSrcInfo, int index );
BOOL LCFGi_TSF_WriteFile( char *pPath, TSFHeader *pHeader, const void *pSrcBody, u8 *pSaveCount );

ChangeCotnrolResult changeControl( int *menu, int *line, int *changeLine, int *changeMode )
{
	int linemax = gAllInfo[*menu][*line].numKindName;
	BOOL controlFlag = FALSE;

	if( !gAllInfo[*menu][*line].changable )
	{
		*changeMode = FALSE;
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
		*changeMode = FALSE;
	}
	
	return controlFlag ? CHANGE_CONTROL : CHANGE_NOTHING ;
}


BOOL control( int *menu, int *line, int *changeLine, int *changeMode )
{
	int linemax = s_numMenu[*menu]; // 選択中ページの項目数
	BOOL controlFlag = FALSE;				// 何か操作があったらTRUEになる

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
				case MENU_OTHER:
				case MENU_NORMAL_HW:
				case MENU_SECURE_HW:
				case MENU_SCFG_ARM7:
				case MENU_SCFG_ARM9:
				case MENU_SYSMENU:
				case MENU_VERSION:
					// 今の画面の選択位置を記録
					selectLine[ROOTMENU_SIZE] = *line;

					// 次のメニュー画面を開く
					*menu = *line;
					*line = selectLine[*menu];
					break;
				
				case MENU_RESET_INFO:
					resetUserData(0);
					resetUserData(1);
					break;
					
				case MENU_BREAK_DATA:
					breakUserData(0);
					breakUserData(1);
					break;
					
			}
		}
		else if( gAllInfo[*menu][*line].changable )
		{
			controlFlag = TRUE;

			// 変更可能な項目は変更画面を開く
			*changeMode = TRUE;
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

void resetUserData( int idx )
// idx(0 or 1)番目のユーザデータをリセットする
{
	u8 *dataBuf = (u8*) Alloc (LCFG_READ_TEMP);
	
	LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ])dataBuf );
	TSDi_ClearSettingsDirect( (LCFGTWLSettingsData *)(&dataBuf[ LCFG_TEMP_BUFFER_SIZE*idx ]) );
	LCFGi_TSD_WriteSettingsDirectForRecovery( (LCFGTWLSettingsData *)&dataBuf[ LCFG_TEMP_BUFFER_SIZE*idx ], idx );
}

void breakUserData( int idx )
{
	// LCFG APIを使わずに、FSレベルでファイルを読んで、データを破壊してから書き戻す
	FSFile file;
	u8 *fileBuf = (u8*) Alloc ( LCFG_TEMP_BUFFER_SIZE );
	
	FS_InitFile( &file );

	if( !FS_OpenFileEx( &file, s_TSDPath[idx], FS_FILEMODE_R | FS_FILEMODE_W ) )
	{
		OS_TPrintf("OpenFile failed. result: %d path: %s\n", FS_GetArchiveResultCode(&file), s_TSDPath[idx]);
		return;
	}
/*	
	if( FS_ReadFile( &file, fileBuf, LCFG_TEMP_BUFFER_SIZE ) == -1 )
	{
		OS_TPrintf("readFile failed. path: %s\n", s_TSDPath[idx]);
		return;
	}
*/	
	// 適当にデータを壊す
	MI_CpuFill8( fileBuf, 0xFF, LCFG_TEMP_BUFFER_SIZE );
	
	// データの書き戻し
	FS_SeekFileToBegin( &file );
	
	if( FS_WriteFile( &file, fileBuf, LCFG_TEMP_BUFFER_SIZE ) == -1 )
	{
		OS_TPrintf("writeFile failed. path: %s\n", s_TSDPath[idx]);
		return;
	}
	
	/*
	// 念のため中身を確認
	MI_CpuClear8( fileBuf, LCFG_TEMP_BUFFER_SIZE );
	FS_SeekFileToBegin( &file );
	if( FS_ReadFile( &file, fileBuf, LCFG_TEMP_BUFFER_SIZE ) == -1 )
	{
		OS_TPrintf("readFile failed. path: %s\n", s_TSDPath[idx]);
		return;
	}
	
	{
		int i;	
		for( i = 0; i < LCFG_TEMP_BUFFER_SIZE; i++ )
		{
			if( i % 16  == 0 )
			{
				OS_TPrintf("\n");
			}
			
			OS_TPrintf("%x ",fileBuf[i] );
		}
	}
	*/
	
	FS_CloseFile( &file );
	
	

	OS_TPrintf("Breaking UserData Succeeded. path: %s\n", s_TSDPath[idx]);
}

// TWL設定データの直接クリア
static void TSDi_ClearSettingsDirect( LCFGTWLSettingsData *pTSD )
{
    int i;
    MI_CpuClearFast( pTSD, sizeof(LCFGTWLSettingsData) );
    // 初期値が"0"以外のもの
    pTSD->owner.userColor = OS_FAVORITE_COLOR_MAGENTA;	// 2008.06.23 UIG松島さんの要望により
    pTSD->owner.birthday.month  = 1;
    pTSD->owner.birthday.day    = 1;
	pTSD->flags.isAvailableWireless = 1;
	pTSD->launcherStatus.InstalledSoftBoxCount = 0;
	pTSD->launcherStatus.freeSoftBoxCount      = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX;
	pTSD->agreeEulaVersion[ 0 ] = 1;
    // 言語コードはHW情報の言語ビットマップから算出
    for( i = 0; i < LCFG_TWL_LANG_CODE_MAX; i++ ) {
        if( OS_GetValidLanguageBitmap() & ( 0x0001 << i ) ) {	// ValidLanguageBitmap情報は、ランチャーがMMEMにロードしたものを使用
            pTSD->language = (LCFGTWLLangCode)i;
            break;
        }
    }
}




// 指定データの値をファイルに直接ライト(リカバリ用にs_indexTSDの変更をライト後に行う）
static BOOL LCFGi_TSD_WriteSettingsDirectForRecovery( const LCFGTWLSettingsData *pSrcInfo, int index )
{
	u8 saveCount = 0;
    // ヘッダの作成
    TSFHeader header;
    MI_CpuClear8( &header, sizeof(TSFHeader) );
    header.version = LCFG_TWL_SETTINGS_DATA_VERSION;
    header.bodyLength = sizeof(LCFGTWLSettingsData);
    SVC_CalcSHA1( header.digest.sha1, pSrcInfo, sizeof(LCFGTWLSettingsData) );
	
    // ファイルにライト
    if( !LCFGi_TSF_WriteFile( (char *)s_TSDPath[ index ],
                        &header,
                        (const void *)pSrcInfo,
                        &saveCount ) ) {
        return FALSE;
    }

    return TRUE;
}

// TWLファイルのライト
BOOL LCFGi_TSF_WriteFile( char *pPath, TSFHeader *pHeader, const void *pSrcBody, u8 *pSaveCount )
{
    BOOL retval = FALSE;
    FSFile file;
    FS_InitFile( &file );
    
    if( pSaveCount ) {
        *pSaveCount = (u8)( ( *pSaveCount + 1 ) & SAVE_COUNT_MASK );
        pHeader->saveCount = *pSaveCount;
    }else {
        pHeader->saveCount = 0;
    }
    
    OS_TPrintf( "Write > %s : %d\n", pPath, pHeader->saveCount );
    
    // ファイルオープン
    if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R | FS_FILEMODE_W ) ) {       // R|Wモードで開くと、既存ファイルを残したまま更新。
        OS_TPrintf( "Write : file open error. %s\n", pPath );
        return FALSE;
    }
    
    // ライト
    if( FS_WriteFile( &file, pHeader, sizeof(TSFHeader) ) < sizeof(TSFHeader) ) {
        OS_TPrintf( "Write : file header write error. %s\n", pPath );
        goto END;
    }
    if( FS_WriteFile( &file, pSrcBody, (long)pHeader->bodyLength ) < pHeader->bodyLength ) {
        OS_TPrintf( "Write : file body write error. %s\n", pPath );
        goto END;
    }
    
    retval = TRUE;
END:
    // ファイルクローズ
    (void)FS_CloseFile( &file );
    
    return retval;
}
