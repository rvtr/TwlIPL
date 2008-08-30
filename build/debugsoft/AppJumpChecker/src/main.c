/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - appjumpTest - Nand-2
  File:     main.c

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
#include <twl/nam.h>
#include <twl/os/common/format_rom.h>

#include "application_jump_private.h"
#include "common.h"
#include "screen.h"

#define DMA_NO_FS		1
#define TITLE_NUM_CUL  18
#define TITLE_NUM_PAGE (TITLE_NUM_CUL * 2)

/*---------------------------------------------------------------------------*
    変数 定義
 *---------------------------------------------------------------------------*/
// キー入力
static KeyInfo  gKey;

// インストールされている NAND アプリの数
static s32 gNandAppNum;

// カーソル位置
static s32 gCurPos = 0;

static BOOL gIsExistCard = FALSE;

// エラー表示用バッファ
static char gErrBuf[33];

// エラー表示時間計測用
static OSTick gErrTick = 0;

typedef struct DataStruct
{
	NAMTitleId	id;
	BOOL		normaljmp_flag;
} DataStruct;

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
static void PrintErrMsg(const char* msg);

static BOOL GetDataStruct(DataStruct* list);
static void DrawScene(DataStruct* list);

static void ConvertTitleIdLo(u8* code, u8* titleid_lo);
static void ConvertGameCode(u8* code, u32 game_code);
static void ConvertInitialCode(u8* code, u32 titleid_lo);

static void* AllocForNAM(u32 size);
static void FreeForNAM(void* ptr);

/*---------------------------------------------------------------------------*/

static void PrintErrMsg(const char* msg)
{
	if ( STD_StrLen(msg) > 32 )
	{
		OS_Warning("err msg is too long.");
		return;
	}
	
	STD_StrCpy( gErrBuf, msg );
	gErrTick = OS_GetTick();
}

void TwlMain(void)
{
	DataStruct dataList[TITLE_NUM_PAGE];

	InitCommon();
    InitScreen();

    GX_DispOn();
    GXS_DispOn();

	FS_Init(DMA_NO_FS);
    NAM_Init(AllocForNAM, FreeForNAM);

    ClearScreen();

    // NAND にインポートされているNAND アプリの数を取得する
    if ( (gNandAppNum = NAM_GetNumTitles()) < 0)
    {
		OS_Panic("NAM_GetNumTitles() failed.");
	}

	

    // キー入力情報取得の空呼び出し(IPL での A ボタン押下対策)
    ReadKey(&gKey);
    
   	// 情報の取得
	if ( !GetDataStruct(dataList) )
	{
		PrintErrMsg("Failed to get dataList.");
	}

    while(TRUE)
    {
        // キー入力情報取得
		ReadKey(&gKey);
        
        // カーソルの移動
        if (gKey.trg & PAD_KEY_DOWN)
        {
			gCurPos++;
			if ( gCurPos >= ( gNandAppNum < TITLE_NUM_PAGE ? gNandAppNum : TITLE_NUM_PAGE) )
			{
				gCurPos = 0;
			}
        }
        if (gKey.trg & PAD_KEY_UP)
        {
			if ( gCurPos == 0)
			{
				if ( gNandAppNum < TITLE_NUM_PAGE )
				{
					gCurPos = gNandAppNum - 1;
				}
				else
				{
					gCurPos = TITLE_NUM_PAGE - 1;
				}
			}
			else
			{
				gCurPos--;
			}
		}
		
		// 列の移動
		if ( (gKey.trg & PAD_KEY_RIGHT || gKey.trg & PAD_KEY_LEFT) && gNandAppNum >= TITLE_NUM_CUL )
		{
			if ( gCurPos > TITLE_NUM_CUL )
			{
				if ( gCurPos - TITLE_NUM_CUL < gNandAppNum )
				{
					gCurPos -= TITLE_NUM_CUL;
				}
			}
			else
			{
				if ( gCurPos + TITLE_NUM_CUL < gNandAppNum )
				{
					gCurPos += TITLE_NUM_CUL;
				}
			}
		}
		
		// アプリジャンプの実行
		if (gKey.trg & PAD_BUTTON_A)
		{
			OS_DoApplicationJump(dataList[gCurPos].id, OS_APP_JUMP_NORMAL);
			// 成功時はここ以降は実行されない
			PrintErrMsg("Failed to App Jump.");
		}
		// 内部関数を使用したアプリジャンプの実行
		if (gKey.trg & PAD_BUTTON_B)
		{
			LauncherBootFlags flag;
			
			flag.bootType = LAUNCHER_BOOTTYPE_NAND;
			flag.isValid = TRUE;
			flag.isLogoSkip = TRUE;
			flag.isInitialShortcutSkip = FALSE;
			flag.isAppLoadCompleted = FALSE;
			flag.isAppRelocate = FALSE;
			flag.rsv = 0;
			OS_SetLauncherParamAndResetHardware(dataList[gCurPos].id, &flag);
			// 成功時はここ以降は実行されない
			PrintErrMsg("Failed to App Jump(force).");
		}
		
		// カードアプリへのアプリジャンプ試行
		if (gKey.trg & PAD_BUTTON_Y)
		{
			CARDRomHeader* rh;
			u64 titleId;
			rh = (CARDRomHeader*)CARD_GetRomHeader();
			
			if ( rh->game_code != 0)
			{
				titleId = (0x00030000 << 32) | rh->game_code;
				OS_DoApplicationJump( titleId, OS_APP_JUMP_NORMAL);
			}

			PrintErrMsg("Failed to App Jump.");
		}
		
		// エラー表示判定
		if ( OS_TicksToSeconds(OS_GetTick() - gErrTick) > 3 )
		{
			PrintErrMsg(" ");
		}
		
		// 画面描画
		DrawScene(dataList);
        
        // Ｖブランク待ち
        OS_WaitVBlankIntr();
        
        // 画面クリア
        ClearScreen();
    }

	// Ｖブランク待ち 最後に画面を更新してから終了
    OS_WaitVBlankIntr();
    OS_Terminate();
}

static BOOL GetDataStruct(DataStruct* list)
{
	// 36個分のタイトルIDリストバッファ
	NAMTitleId titleIdList[TITLE_NUM_PAGE];
	s32 i;
	FSFile fp;
	
	if ( NAM_GetTitleList(titleIdList, TITLE_NUM_PAGE) != NAM_OK )
	{
		PrintErrMsg("NAM_GetTitleList failed.");
		return FALSE;
	}
	
	// データリストの作成（1ページ分）
	for ( i=0; i<TITLE_NUM_PAGE; i++, list++)
	{
		ROM_Header_Short rh;
		char pathbuf[FS_ENTRY_LONGNAME_MAX + 32];
		
		// そもそも NAND アプリの数が 1ページにも満たない場合は途中で終了する
		if ( i >= gNandAppNum )
		{
			break;
		}
		
	// TitleID の格納
		list->id = titleIdList[i];
		
	// ノーマルジャンプをされるのを許可するかを表すフラグの取得
		// TitleID を見て、タイトル種別がデータタイトルならば
		// そもそも ROMヘッダが存在しない、アプリジャンプをする必要もない
		if ( !NAM_IsDataTitle(list->id) )
		{
			// アプリ本体へのパスを取得
			if ( NAM_GetTitleBootContentPath(pathbuf, list->id) != NAM_OK )
			{
				PrintErrMsg("GetContentPath failed.");
				return FALSE;
			}
		
			// アプリのファイルオープン
			FS_InitFile(&fp);
			if ( !FS_OpenFileEx(&fp, pathbuf, FS_FILEMODE_R) )
			{
				// 失敗時はエラーコード出力で終了
				PrintErrMsg("FS_OpenFileEx failed.");
				return FALSE;
			}
			
			if ( -1 == FS_ReadFile(&fp, &rh, sizeof(ROM_Header_Short) ))
			{
				PrintErrMsg("FS_ReadFile failed.");
				return FALSE;
			}
			
			list->normaljmp_flag = rh.permit_landing_normal_jump;
			
			FS_CloseFile(&fp);
		}
		else
		{
			list->normaljmp_flag = FALSE;
		}
	}
	
	PrintErrMsg(" ");
	return TRUE;
}

static void DrawScene(DataStruct* list)
{
	s32 i;
	u8 init_code[5], titleid_lo[5];
	
	// 上画面
	PutMainScreen( 0,  1, 0xff, " ------ App Jump Checker ------ ");
	
	PutMainScreen( 0,  3, 0xff, "  total app : %d", gNandAppNum);
	
	PutMainScreen( 0,  5, 0xff, " ------------------------------ ");
	
	PutMainScreen( 0,  7, 0xff, "   A : try App Jump (to NAND)"   );
	PutMainScreen( 0,  8, 0xfe, "   B : try App Jump (force)  "   );
	PutMainScreen( 0,  9, 0xff, "   Y : try App Jump (to CARD)"   );
	PutMainScreen( 0, 11, 0xff, "   UP ,DOWN ,LEFT ,RIGHT KEY :"  );
	PutMainScreen( 0, 12, 0xff, "                move * (cursor) ");
	PutMainScreen( 0, 14, 0xff, " ------------------------------ ");
	
	PutMainScreen( 0, 16, 0xf1, "%s", gErrBuf);
	
	// 下画面
	PutSubScreen(  0, 0, 0xf4, " NAND ( max 36 )");
	PutSubScreen(  0, 1, 0xff, "--- ID ---------------- ID -------");
	
	
		// 下画面に1行ずつ、カーソル用スペース、イニシャルコード、ノーマルジャンプ可否フラグを表示
	for ( i=0; i < TITLE_NUM_PAGE; i++, list++)
	{
		// そもそも NAND アプリの数が 1ページにも満たない場合は途中で終了する
		if ( i >= gNandAppNum )
		{
			break;
		}
		
		ConvertInitialCode(init_code, NAM_GetTitleIdLo(list->id));
		
		if ( list->normaljmp_flag )
		{
			PutSubScreen( (i >= TITLE_NUM_CUL ? 18 : 0), 2 + (i >= TITLE_NUM_CUL ? i-18 : i), 0xf2, "   %s : o", init_code );
		}
		else
		{
			PutSubScreen( (i >= TITLE_NUM_CUL ? 18 : 0), 2 + (i >= TITLE_NUM_CUL ? i-18 : i), 0xff, "   %s : -", init_code );
		}
	}
	
		// カードアプリチェック
	PutSubScreen(  0, 2 + TITLE_NUM_CUL + 1, 0xf4, " CARD: Init. Code, TitleID_Lo");
	{
		CARDRomHeader* rh;	
		rh = (CARDRomHeader*)CARD_GetRomHeader();
		
		if ( rh->game_code != 0 )
		{
			// イニシャルコードの取得
			if ( rh->product_id & 0x03 || rh->product_id & 0x02 )	// 刺さっているカードのロムが TWLアプリ
			{
				ROM_Header* rh2;
				rh2 = (ROM_Header*)HW_TWL_CARD_ROM_HEADER_BUF;
				
				// ROM ヘッダの拡張領域まで TitleID_Lo を見に行く
				// ROM ヘッダ（共通部）のイニシャルコードは基本的に製品版に入れられる
//				ConvertTitleIdLo(titleid_lo, rh2->s.titleID_Lo);
				MI_CpuCopy8( rh2->s.titleID_Lo, titleid_lo, 4 );
				titleid_lo[4] = 0x00;
				MI_CpuCopy8( rh2->s.game_code, init_code, 4 );
			}
			else	// 刺さっているカードのロムが NTRアプリ
			{
				// 未マスタリング状態のロムでは、ROM ヘッダ（共通部）のイニシャルコードへ
				// 設定が反映されない
				
				// 特別に設定された NTRアプリなら TitleID_Lo が設定されている可能性があるので読み込んでみる
				MI_CpuClear8( titleid_lo, 5 );
				MI_CpuCopy8( ((ROM_Header*)rh)->s.titleID_Lo, titleid_lo, 4 );
				titleid_lo[4] = 0x00;
				ConvertGameCode(init_code, rh->game_code);
			}
			
			// ノーマルジャンプを許可しているかどうかを取得
			if ( rh->reserved_A[8] & 0x80 )
			{
				PutSubScreen( 0, 2 + TITLE_NUM_CUL + 2, 0xf2, "   %s, %s : o", init_code, titleid_lo );
			}
			else
			{
				PutSubScreen( 0, 2 + TITLE_NUM_CUL + 2, 0xff, "   %s, %s : -", init_code, titleid_lo );
			}
		}
		else
		{
			PutSubScreen( 0, 2 + TITLE_NUM_CUL + 2, 0xfe, "   not exist");
		}
		
		ConvertGameCode(init_code, rh->game_code);
	}
	
	
	// カーソル描画
	PutSubScreen( 1 + (gCurPos >= TITLE_NUM_CUL ? 18 : 0), 2 + (gCurPos >= TITLE_NUM_CUL ? gCurPos - 18 : gCurPos), 0xf4, "*");
}

static void ConvertTitleIdLo(u8* code, u8* titleid_lo)
{
	u8 tmp[5];
	s32 i;
	
//	for ( titleid_lo += 3, i=0 ; i<4; i++, code++, titleid_lo--)
	for ( i=3; i>=0; i--, titleid_lo++ )
	{
		tmp[i] = *titleid_lo;
		*code = tmp[i];
	}
	
//	*code = tmp;
	
	// NULL 終端
	*code = 0x00;
}

static void ConvertGameCode(u8* code, u32 game_code)
{
	u8 tmp[5];
	s32 i;
	
	ConvertInitialCode(tmp, game_code);
	
	for ( i=3; i>=0; i--, code++)
	{
		*code = tmp[i];
	}
	
	// NULL 文字終端
	*code = 0x00;
}

static void ConvertInitialCode(u8* code, u32 titleid_lo)
{
	s32 i;
	
	for ( i=0; i<4; i++, code++)
	{
		*code = (u8)(titleid_lo >> (8 * (3-i)));
	}
	
	// NULL文字終端
	*code = 0x00;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みハンドラ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    // テキスト表示を更新
    UpdateScreen();

    // IRQ チェックフラグ47をセット
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

static void* AllocForNAM(u32 size)
{
	void* ptr;
	ptr = OS_AllocFromMain(size);
	
	if (ptr == NULL)
	{
		OS_Panic("alloc failed.");
	}
	
	return ptr;
	
}

static void FreeForNAM(void* ptr)
{
	OS_FreeToMain(ptr);
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
