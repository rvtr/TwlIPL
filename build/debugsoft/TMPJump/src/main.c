/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - tmpjumpTest
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

#include <twl.h>
#include "common.h"
#include "screen.h"

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/
#define DMA_NO_FS	1
#define APP_NUM		5		// TMP ジャンプ先ロム数

/*---------------------------------------------------------------------------*
    変数 定義
 *---------------------------------------------------------------------------*/
// キー入力
static KeyInfo  gKey;

// tmp へコピーする srl
static u8 gSrlNumber = 0;

// 各 srl の TitleID
static u64 gRomTitleIDList[APP_NUM] = 
{
	0x0003000434333341,		// NTR		GameCode 433A
	0x0003000434333441,		// TWL-HYB	GameCode 434A
	0x0003000434333541,		// TWL-LTD	GameCode 435A
	0x0003000434353841,		// NTR (don't permit TMP jump) GameCode 458A
	0x0003000434353941		// TWL (don't permit TMP jump) GameCode 459A
};

// 各 srl の場所
static const char gRomPath[APP_NUM][32] = 
{
	"rom:/TestApp.srl", "rom:/TestApp_HYBRID.srl", "rom:/TestApp_LIMITED.srl",
	"rom:/Test_NITROfail.srl", "rom:/Test_TWLfail.srl"
};

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
static void DrawMainScene(void);
void VBlankIntr(void);

void TwlMain(void)
{
	InitCommon();
	InitScreen();
	
	FS_Init(DMA_NO_FS);
	
	GX_DispOn();
	GXS_DispOn();
	
	ClearScreen();
	
	// 一度空読み
	ReadKey(&gKey);
	
	while(TRUE)
	{
		ReadKey(&gKey);
		
		if (gKey.trg & PAD_BUTTON_A)
		{
			BOOL success = TRUE;
			FSFile src, dest;
			void *buf;
			s32 len = 0;
			
			PutSubScreen(1, 8, 0xff, "now writing tmp app...");
			OS_WaitVBlankIntr();
			
			// srl を NAND 上の OS_TMP_APP_PATH へコピー
			FS_DeleteFile(OS_TMP_APP_PATH);
			FS_CreateFile(OS_TMP_APP_PATH, FS_PERMIT_R | FS_PERMIT_W);
			FS_InitFile( &src );
			FS_InitFile( &dest );
			if ( !FS_OpenFileEx( &src, gRomPath[gSrlNumber], FS_FILEMODE_R ) ) success = FALSE;
			len = (s32)FS_GetFileLength( &src );
			
			buf = OS_AllocFromMain((u32)len);
			if (buf == NULL)
			{
				success = FALSE;
				break;
			}
			
			if ( -1 == FS_ReadFile( &src, buf, len ) )
			{
				success = FALSE;
				break;
			}
			
			if ( !FS_CloseFile( &src ) ) success = FALSE;
			
			if ( !FS_OpenFileEx( &dest, OS_TMP_APP_PATH, FS_FILEMODE_W ) ) success = FALSE;
			if ( -1 == FS_WriteFile( &dest, buf, len ) ) success = FALSE;
			if ( !FS_CloseFile( &dest ) ) success = FALSE;
			
			if ( !success ) break;
			
			// アプリジャンプ
			OS_DoApplicationJump( gRomTitleIDList[gSrlNumber], OS_APP_JUMP_TMP );
			// 失敗時にはエラーを下画面に出力して終了
			PutSubScreen(1, 10, 0xf1, "ERROR: TMP Jump failed!");
			break;
		}
		
		if (gKey.trg & PAD_BUTTON_B)
		{
			// リターンジャンプを試みる
			if(!OS_ReturnToPrevApplication())
			{
				// 失敗時にはメッセージを表示
				PutSubScreen(1, 10, 0xf1, "ERROR: Return Jump failed!");
				break;
			}
		}
		
		// 方向キー上下で、OS_TMP_APP_PATH へコピーする srl を選択
		if (gKey.trg & PAD_KEY_DOWN)
		{
			gSrlNumber++;
			if (gSrlNumber > APP_NUM - 1)
				gSrlNumber = 0;
		}
		else if (gKey.trg & PAD_KEY_UP)
		{
			if (gSrlNumber == 0)
				gSrlNumber = APP_NUM -1;
			else
				gSrlNumber--;
		}
		
		ClearScreen();
		DrawMainScene();
		
		OS_WaitVBlankIntr();
	}
	
	OS_WaitVBlankIntr();
	OS_Terminate();
}


static void DrawMainScene(void)
{
	PutMainScreen(1,  1, 0xf2, "Application (Main)");
	PutMainScreen(2,  4, 0xff, "B BUTTON: try return jump");
	
	PutMainScreen(3, 12, 0xff, "Tmp jump to NITRO   rom");
	PutMainScreen(3, 14, 0xff, "Tmp jump to TWL-HYB rom");
	PutMainScreen(3, 16, 0xff, "Tmp jump to TWL-LTD rom");
	
	PutMainScreen(3, 18, 0xfe, "Tmp jump to NITRO x rom");
	PutMainScreen(3, 20, 0xfe, "Tmp jump to TWL   x rom");
	
	// 選択中を示すカーソル描画
	PutMainScreen(1, gSrlNumber * 2 + 12, 0xf4, "*");
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

    // IRQ チェックフラグをセット
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
