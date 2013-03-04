/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checker
  File:     util.cpp

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

#include "util/util.h"
#include "util/canvas.h"
#include "util/memory.h"
#include <nnsys/gfd.h>
#include <nnsys/g2d/g2d_Font.h>

extern u8 font_NFTR_begin[];
extern u8 font_NFTR_end[];

namespace util
{
  GamePad System_GamePadState;
  int System_BaseWidth;
  int System_LineHeight;

  namespace
    {
      const int REPEAT_START_FRAME        = 15;
      const int REPEAT_INTERVAL           = 30;
      const u32 TASK_ARRAY_NUM            = 4;
      const int CANVAS_CHARACTER_OFFSET   = 1;

      GXRgb COLOR_PALETTE[18] =
        {
          GX_RGB(26, 26, 26),
          GX_RGB(31, 31, 31),
          GX_RGB( 0,  0,  0),

          GX_RGB(31,  0,  3),
          GX_RGB(30, 28,  0),
          GX_RGB(21, 31,  0),
          GX_RGB( 6, 23, 30),
          GX_RGB( 0, 11, 30),
          GX_RGB(25, 23,  1),
          GX_RGB(22, 22, 22),
          GX_RGB(16, 16, 16),

          GX_RGB(29, 31, 31)
          };
      NNSG2dFont  sFont;
      CCanvas     sCanvasMain1;
      CCanvas     sCanvasMain3;
      CCanvas     sCanvasSub1;
      CCanvas     sCanvasSub3;

      NNSGfdVramTransferTask  sTaskArray[TASK_ARRAY_NUM];

      void ClearVram( void )
        {
          //---------------------------------------------------------------------------
          // All VRAM banks to LCDC
          //---------------------------------------------------------------------------
          GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

          //---------------------------------------------------------------------------
          // Clear all LCDC space
          //---------------------------------------------------------------------------
          MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

          //---------------------------------------------------------------------------
          // Disable the banks on LCDC
          //---------------------------------------------------------------------------
          (void)GX_DisableBankForLCDC();

          MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);      // clear OAM
          MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);        // clear the standard palette

          MI_CpuFillFast((void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE); // clear OAM
          MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);  // clear the standard palette
        }

      void InitFont(void* pNFTR)
        {
          NNS_G2dFontInitUTF16(&sFont, pNFTR);

          System_BaseWidth  = NNS_G2dFontGetCellWidth(&sFont) / 2;
          System_LineHeight = NNS_G2dFontGetLineFeed(&sFont);
        }

      void VBlankIntr(void)
        {
          OS_SetIrqCheckFlag( OS_IE_V_BLANK );
        }


    } // namespace

  void WaitVBlankIntr(void)
    {
      SVC_WaitVBlankIntr();
    }

  void dispOn(void)
    {
      GX_DispOn();
      GXS_DispOn();
    }

  CCanvas* GetMain1Canvas() { return &sCanvasMain1; }
  CCanvas* GetMain3Canvas() { return &sCanvasMain3; }
  CCanvas* GetSub1Canvas()  { return &sCanvasSub1;  }
  CCanvas* GetSub3Canvas()  { return &sCanvasSub3;  }

  void UpdateGamePad(void)
  {
    u16 status = PAD_Read();

    System_GamePadState.repeatTrigger = false;

    if( status != 0 )
      {
        if( System_GamePadState.button == 0 )
          {
            System_GamePadState.beginPress = 0;
          }
        else
          {
            System_GamePadState.beginPress++;
            const int lapse = System_GamePadState.beginPress - REPEAT_START_FRAME;

            if( lapse >= 0 )
              {
                if( (lapse % REPEAT_INTERVAL) == 0 )
                  {
                    System_GamePadState.repeatTrigger = true;
                  }
              }
          }
      }

    System_GamePadState.trigger = (u16)(status                     & (status ^ System_GamePadState.button));
    System_GamePadState.release = (u16)(System_GamePadState.button & (status ^ System_GamePadState.button));
    System_GamePadState.button  = status;
  }

  void UpdateDisplay(void)
  {
    sCanvasMain1.ResetTransferTask();
    sCanvasMain3.ResetTransferTask();
    sCanvasSub1 .ResetTransferTask();
    sCanvasSub3 .ResetTransferTask();
    NNS_GfdDoVramTransfer();
  }
  
  void Init()
    {
      OS_Init();
      FX_Init();
      GX_Init();

      OS_InitTick();
      OS_InitAlarm();

      GX_DispOff();
      GXS_DispOff();

      RTC_Init();

      OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
      (void)OS_EnableIrqMask(OS_IE_V_BLANK);
      (void)GX_VBlankIntr(TRUE);
      (void)OS_EnableIrq();
      (void)OS_EnableInterrupts();

      ClearVram();

      InitMemory();

      // assign vram banks
      GX_SetBankForBG(GX_VRAM_BG_128_A);
      GX_SetBankForOBJ(GX_VRAM_OBJ_16_F);
      GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
      GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_16_I);
      GX_SetBankForLCDC(GX_VRAM_LCDC_B);

      // init screen
      // BG 1 を設定
      G2_SetBG1Control(
        GX_BG_SCRSIZE_TEXT_256x256,     // スクリーンサイズ 256x256
        GX_BG_COLORMODE_16,             // カラーモード     16色
        GX_BG_SCRBASE_0x0000,           // スクリーンベース
        GX_BG_CHARBASE_0x04000,         // キャラクタベース
        GX_BG_EXTPLTT_01 );             // 拡張パレットスロット

      G2S_SetBG1Control(
        GX_BG_SCRSIZE_TEXT_256x256,     // スクリーンサイズ 256x256
        GX_BG_COLORMODE_16,             // カラーモード     16色
        GX_BG_SCRBASE_0x0000,           // スクリーンベース
        GX_BG_CHARBASE_0x04000,         // キャラクタベース
        GX_BG_EXTPLTT_01 );             // 拡張パレットスロット

      // BG 2 を設定
      G2_SetBG2ControlText(
        GX_BG_SCRSIZE_TEXT_256x256,     // スクリーンサイズ 256x256
        GX_BG_COLORMODE_16,             // カラーモード     16色
        GX_BG_SCRBASE_0x0800,           // スクリーンベース
        GX_BG_CHARBASE_0x0c000 );       // キャラクタベース

      G2S_SetBG2ControlText(
        GX_BG_SCRSIZE_TEXT_256x256,     // スクリーンサイズ 256x256
        GX_BG_COLORMODE_16,             // カラーモード     16色
        GX_BG_SCRBASE_0x0800,           // スクリーンベース
        GX_BG_CHARBASE_0x0c000 );       // キャラクタベース

      // BG 3 を設定
      G2_SetBG3ControlText(
        GX_BG_SCRSIZE_TEXT_256x256,     // スクリーンサイズ 256x256
        GX_BG_COLORMODE_16,             // カラーモード     16色
        GX_BG_SCRBASE_0x1000,           // スクリーンベース
        GX_BG_CHARBASE_0x14000 );       // キャラクタベース

      G2S_SetBG3ControlText(
        GX_BG_SCRSIZE_TEXT_256x256,     // スクリーンサイズ 256x256
        GX_BG_COLORMODE_16,             // カラーモード     16色
        GX_BG_SCRBASE_0x1000,           // スクリーンベース
        GX_BG_CHARBASE_0x14000 );       // キャラクタベース

      // BG1/3 を可視に
      GX_SetVisiblePlane ( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3 );
      GXS_SetVisiblePlane( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3 );

      // BG1/2/3 の優先順位設定
      G2_SetBG1Priority(3);
      G2_SetBG2Priority(2);
      G2_SetBG3Priority(1);
      G2S_SetBG1Priority(3);
      G2S_SetBG2Priority(2);
      G2S_SetBG3Priority(1);

      GX_LoadBGPltt(COLOR_PALETTE, 0, sizeof(COLOR_PALETTE));
      GXS_LoadBGPltt(COLOR_PALETTE, 0, sizeof(COLOR_PALETTE));

      GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
      GXS_SetGraphicsMode(GX_BGMODE_0);
      GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);
      GX_SetBGCharOffset(GX_BGCHAROFFSET_0x00000);

      InitFont(font_NFTR_begin);

      {
        const u32 chrOffset = CANVAS_CHARACTER_OFFSET * sizeof(GXCharFmt16);
        sCanvasMain1.Init(NNS_GFD_DST_2D_BG1_CHAR_MAIN, chrOffset, G2_GetBG1ScrPtr(),  &sFont);
        sCanvasMain3.Init(NNS_GFD_DST_2D_BG3_CHAR_MAIN, chrOffset, G2_GetBG3ScrPtr(),  &sFont);
        sCanvasSub1 .Init(NNS_GFD_DST_2D_BG1_CHAR_SUB,  chrOffset, G2S_GetBG1ScrPtr(), &sFont);
        sCanvasSub3 .Init(NNS_GFD_DST_2D_BG3_CHAR_SUB,  chrOffset, G2S_GetBG3ScrPtr(), &sFont);

        NNS_GfdInitVramTransferManager(sTaskArray, TASK_ARRAY_NUM);
      }

      PAD_Read();
    }

    u16 GetPadTrigger()
    {
        return static_cast<u16>(System_GamePadState.button);
    }

} // namespace util

