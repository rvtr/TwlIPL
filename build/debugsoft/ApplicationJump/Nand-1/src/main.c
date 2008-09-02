/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - appjumpTest - Nand-1
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
#include "common.h"
#include "screen.h"

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/
// キー入力
static KeyInfo  gKey;

// B ボタンジャンプ先の切りかえフラグ
static u8 gJumpTypeForB = JUMPTYPE_RETURN;
// アプリ間パラメータ
static AppParam gAppParam;
// アプリ間パラメータとして文字列をセットするかどうか
static BOOL gIsSetDelArg = TRUE;
// アプリ間パラメータとして引き渡された文字列を格納するバッファ（6つまで）
static char gStrAppParam[6][APPJUMP_STRING_LENGTH + 1] ATTRIBUTE_ALIGN(32);
// アプリ間パラメータとして引き渡された文字列の個数
static int gArgc;

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
static void AddDeliverArg(OSDeliverArgInfo *argInfo, BOOL isReturn);

/*---------------------------------------------------------------------------*/

void TwlMain(void)
{
    OSDeliverArgInfo argInfo;
    
    int result;
    int argSize =sizeof(u32);

    InitCommon();
    InitScreen();

    GX_DispOn();
    GXS_DispOn();

    ClearScreen();

    // キー入力情報取得の空呼び出し(IPL での A ボタン押下対策)
    ReadKey(&gKey);

    OS_InitDeliverArgInfo(&argInfo, sizeof(AppParam));
	OS_DecodeDeliverArg();
	
	/* アプリ間パラメータ（バイナリデータの取得） */
    if ( OS_DELIVER_ARG_SUCCESS  != (result = OS_GetBinaryFromDeliverArg( &gAppParam, &argSize, sizeof(AppParam))) )
    {
        PutMainScreen(1, 16, 0xf1, "ERROR!: READ_ERROR (%d)", result);
        
        OS_WaitVBlankIntr();
        OS_Terminate();
    }
    
    /* アプリ間パラメータ（文字列）の取得 */
    if ((gArgc = OS_GetDeliverArgc()) > 0)
    {
		int i;
		
		OS_TPrintf("argc = %d\n", gArgc);
		
		for (i=0; i < gArgc-1 && i < 6; i++)
		{
			STD_StrLCpy(gStrAppParam[i], (const char*)OS_GetDeliverArgv(i+1), APPJUMP_STRING_LENGTH);
			gStrAppParam[i][APPJUMP_STRING_LENGTH] = '\0';
		}
	}

    while(TRUE)
    {
		int i;
		
		// 自動テスト中に、ユーザのキー入力による終了指示を受け付けるためにウエイトを入れる
		if (gAppParam.isAutoJump == 1)
		{
			OSTick tick = OS_GetTick();
			
			PutMainScreen(1,  9, 0xf8, "executing auto app jump...");
			PutMainScreen(1, 11, 0xff, "wait 2 seconds...");
			PutMainScreen(1, 13, 0xff, "START: quit auto app jump");
			do {
				ReadKey(&gKey);
				
				if (gKey.trg & PAD_BUTTON_START)
				{
					break;
				}
				
				OS_WaitVBlankIntr();
			} while (OS_TicksToSeconds(OS_GetTick() - tick) < 2);
		}
		else
		{
	        // キー入力情報取得
	        ReadKey(&gKey);
	    }

        // 画面クリア
        ClearScreen();
        
        // メイン画面描画
        PutMainScreen(0,  2, 0xf5, " ***** This APP is NAND-1 ***** ");
        PutMainScreen(0,  5, 0xff, " APP JUMP : %u times ", gAppParam.jumpCount);
        PutMainScreen(0,  7, 0xff, " DELIVERED PARAM (recent 6 app)");
        
        for (i=0; i<gArgc-1; i++)
        {
			if ( i==0 )
			{
				PutMainScreen(2, 8, 0xf2, "%d : %s", i+1, gStrAppParam[i]);	// 最新の履歴のみ色をつける
			}
			else
			{
		        PutMainScreen(2, 8+i, 0xff, "%d : %s", i+1, gStrAppParam[i]);
		    }
        }
        
        // サブ画面描画
        PutSubScreen(0,  0, 0xf5, " ------- APP JUMP TEST -------- ");

		if (gIsSetDelArg)
		{
			PutSubScreen(0, 2, 0xf8, " DELIVER ARG: ON");
		}
		else
		{
			PutSubScreen(0, 2, 0xff, " DELIVER ARG: OFF");
		}
        
		if (gAppParam.isAutoJump)
		{
			PutSubScreen(0, 4, 0xf8, " AUTO JUMP TEST: ON");
		}
		else
		{
			PutSubScreen(0, 4, 0xff, " AUTO JUMP TEST: OFF");
		}
        
        PutSubScreen(0, 13, 0xf5, " ------------------------------- ");
        PutSubScreen(0, 15, 0xff, "   A: JUMP TO CARD APP");
        PutSubScreen(0, 16, 0xff, "   Y: JUMP TO NAND-2 APP");
        PutSubScreen(0, 17, 0xff, "   X: JUMP TO SELF");
        
        switch (gJumpTypeForB)
        {
		case JUMPTYPE_RETURN:
	        PutSubScreen(0, 18, 0xff, "   B: RETURN JUMP");
	    break;
	    case JUMPTYPE_ANOTHER_CARD:
	    	PutSubScreen(0, 18, 0xfe, "   B: JUMP TO CARD APP");
	    break;
	    case JUMPTYPE_SYSMENU:
	    	PutSubScreen(0, 18, 0xff, "   B: JUMP TO SYSMENU");
	    break;
	    }

		PutSubScreen(0, 20, 0xff, " UP DOWN: SWITCH JUMP TYPE (B) ");
        PutSubScreen(0, 21, 0xff, " L R: SWITCH DELIVER ARG ON/OFF");
        PutSubScreen(0, 22, 0xff, " STR: SWITCH AUTO TEST ON/OFF");
        
        if (gKey.trg & PAD_KEY_DOWN)
        {
			if ( gJumpTypeForB == JUMPTYPE_RETURN )
			{
				gJumpTypeForB = JUMPTYPE_NUM - 1;
			}
			else
				gJumpTypeForB--;
        }
        else if (gKey.trg & PAD_KEY_UP)
        {
			if ( ++gJumpTypeForB >= JUMPTYPE_NUM )
			{
				gJumpTypeForB = JUMPTYPE_RETURN;
			}
        }
        
        if (gKey.trg & PAD_BUTTON_START)
        {
			// 自動テストフラグをスイッチ
			gAppParam.isAutoJump ^= 1;
		}
        
        if (gKey.trg & PAD_BUTTON_A)
        {
			AddDeliverArg(&argInfo, FALSE);
			// カードアプリへジャンプ
			if( !OS_DoApplicationJump( CARDAPP_TITLEID, OS_APP_JUMP_NORMAL ))
			{
				OS_TPrintf("Failed to Jump.\n");
				PutMainScreen(1, 16, 0xf1, "ERROR!: Failed to Jump.");
			}
            break;
        }
        
        if (gKey.trg & PAD_BUTTON_Y)
        {
			AddDeliverArg(&argInfo, FALSE);
			
			// もう一方の NAND アプリへジャンプ
			if ( !OS_DoApplicationJump( NANDAPP2_TITLEID, OS_APP_JUMP_NORMAL ))
			{
				OS_TPrintf("Failed to Jump.\n");
				PutMainScreen(1, 16, 0xf1, "ERROR!: Failed to Jump.");
			}
            break;
        }
        if (gKey.trg & PAD_BUTTON_X || gAppParam.isAutoJump == 1)
        {
			AddDeliverArg(&argInfo, FALSE);
			// 自分自身へジャンプ
			if( !OS_DoApplicationJump( OS_GetTitleId(), OS_APP_JUMP_NORMAL ))
			{
				OS_TPrintf("Failed to Jump.\n");
				PutMainScreen(1, 16, 0xf1, "ERROR!: Failed to Jump.");
			}
			break;
        }
        if (gKey.trg & PAD_BUTTON_B)
        {
			AddDeliverArg(&argInfo, TRUE);
			
			switch (gJumpTypeForB)
			{
			case JUMPTYPE_RETURN:
	            if ( !OS_ReturnToPrevApplication() )
	            {
					OS_TPrintf("Failed to Return Jump.\n");
					PutMainScreen(1, 16, 0xf1, "ERROR!: Failed to Return Jump.");
				}
	            break;
	        case JUMPTYPE_ANOTHER_CARD:
	        	if ( !OS_DoApplicationJump(CARDAPP_ANO_TITLEID, OS_APP_JUMP_NORMAL) )
	        	{
					OS_TPrintf("Failed to Jump.\n");
					PutMainScreen(1, 16, 0xf1, "ERROR!: Failed to Jump.");
				}
	        	break;
	        case JUMPTYPE_SYSMENU:
	        	OS_JumpToSystemMenu();
	        break;
	        }
        }
        
        if (gKey.trg & PAD_BUTTON_L || gKey.trg & PAD_BUTTON_R)
        {
			gIsSetDelArg = !gIsSetDelArg;
		}
        
        // Ｖブランク待ち
        OS_WaitVBlankIntr();
    }

	// Ｖブランク待ち 最後に画面を更新してから終了
    OS_WaitVBlankIntr();
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         AddDeliverArg

  Description:  アプリジャンプ先へ引き渡すパラメータを追加。

  Arguments:    argInfo  : 
  				isReturn : ジャンプ元への復帰ならば TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AddDeliverArg(OSDeliverArgInfo *argInfo, BOOL isReturn)
{
	int result;
	char argument[APPJUMP_STRING_LENGTH + 1];
	gAppParam.jumpCount++;
	
	OS_InitDeliverArgInfo(argInfo, sizeof(AppParam));
	
	// アプリジャンプ回数を +1 してセット
	result = OS_SetBinaryToDeliverArg( &(gAppParam), sizeof(AppParam) );

    if(result != OS_DELIVER_ARG_SUCCESS)
    {
        OS_Warning("Failed to Set DeliverArgument.");
    }
    
    if (gIsSetDelArg)
    {
		int i;
		
    	// 文字列をセット
    	MI_CpuClear8(argument, APPJUMP_STRING_LENGTH + 1);
    	if (isReturn)
    	{
			STD_TSPrintf(argument, "Returned from NAND-1");
		}
		else
		{
	    	STD_TSPrintf(argument, "Jumped   from NAND-1");
	    }
    	OS_SetStringToDeliverArg(argument);

		// 今まで引き渡されたパラメータ文字列を引き継がせる
		for (i=0; i <= 4 && i < gArgc; i++)
		{
			MI_CpuClear8(argument, APPJUMP_STRING_LENGTH + 1);
			STD_StrLCpy( argument, gStrAppParam[i], APPJUMP_STRING_LENGTH );
			result = OS_SetStringToDeliverArg(argument);
			OS_TPrintf("arg = %s : result = %d\n", argument, result);
		}
    }
    OS_EncodeDeliverArg();
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

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
