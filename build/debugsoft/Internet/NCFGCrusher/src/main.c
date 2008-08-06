/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NCFG - demos - ncfg-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    NVRAM 上のネットワーク設定を一定の値で初期化しながら、
    その結果をダンプするサンプルプログラムです。
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include <nitroWiFi.h>
#include <nitroWiFi/ncfg.h>
#include <DEMO.h>
#include <nitro/pad.h>

//#include <nitro/std.h>
//#include <nitro/fs.h>

static void MainLoop(void);
//static void VBlankIntr(void);

static void ReadNCFG(s32 index, u8* out);
static void WriteNCFG(s32 index);
static void InitDEMOSystem(void);

static u32 UpdateCursor(u32 val, u32 max, u16 key);

static const char* SLOT_NAME[] = {
    "SLOT_1", "SLOT_2", "SLOT_3", "SLOT_EX_1", "SLOT_EX_2", "SLOT_EX_3"
};
static const u32 SLOT_NUM = sizeof(SLOT_NAME) / sizeof(char*);

void NitroMain()
{
    OS_Init();
    FX_Init();
    GX_Init();

//    GX_DispOff();
//    GXS_DispOff();

    // V ブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    // 割り込み許可
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    
    InitDEMOSystem();

    (void)PAD_Read();


    
    OS_TPrintf("UP DOWN - SELECT NCFG SLOT\n");
    OS_TPrintf("A - OVERWRITE NCFG\n");
    
    
    MainLoop();

    OS_Terminate();
}

static u32 UpdateCursor(u32 val, u32 max, u16 key)
{
    if(key & PAD_KEY_DOWN)
    {
        val = (val + 1) % max;
    }
    if(key & PAD_KEY_UP)
    {
        val = (val + max - 1) % max;
    }
    return val;
}

static void MainLoop(void)
{
    u32 index = 0;
    s32 slot_index[] = 
    {
        NCFG_SLOT_1,
        NCFG_SLOT_2,
        NCFG_SLOT_3,
        NCFG_SLOT_EX_1,
        NCFG_SLOT_EX_2,
        NCFG_SLOT_EX_3,
    };
    u16 key = 0, old_key = 0, trig = 0;

    while(1)
    {
        s32 i = 0;
        
        old_key = key;
        key = PAD_Read();
        trig = (u16)(key & (key ^ old_key));
        
        index = UpdateCursor(index, SLOT_NUM, trig);
        if(trig & PAD_BUTTON_A)
        {
            WriteNCFG(slot_index[index]);
            OS_TPrintf("%s was modified\n", SLOT_NAME[index]);
        }
        
        for(i = 0; i < SLOT_NUM; ++i)
        {
            
            DEMODrawText(0, i * 8, ((index == i) ? "*" : " "));
            DEMODrawText(8, i * 8, "%s\n", SLOT_NAME[i]);
        }
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
}

static void ReadNCFG(s32 index, u8* out)
{
    static u8 buf[512];
    FSFile fp;
    s32 result;

    FS_InitFile(&fp);
    
    result = NCFG_ReadBackupMemory(buf, sizeof(buf), index);
    OS_TPrintf("NCFG_ReadBackupMemory(%d): %d\n", index, result);
    if ( result >= 0 )
    {
    //        OS_TPrintfEx("% *.16b", result, buf);
        OS_TPrintfEx("% 256.16b", buf);
        OS_TPrintf("\n");
    }
    
    MI_CpuCopy8(buf, out, sizeof(buf));
}

static void WriteNCFG(s32 index)
{
    static u8 buf[512];
    s32 result;

    ReadNCFG(index, buf);
    MI_CpuCopy("abcdefg", buf, 7);
    result = NCFG_WriteBackupMemory(index, buf, sizeof(buf));
    OS_TPrintf("NCFG_WriteBackupMemory(%d): %d\n", index, result);
    OS_TPrintfEx("% 256.16b", buf);
    OS_TPrintf("\n");
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem 

  Description:  コンソールの画面出力用の表示設定を行います。 

  Arguments:    なし。 

  Returns:      なし。 
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem(void)
{
    // 画面表示の初期化。 
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}
