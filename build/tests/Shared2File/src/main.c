/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - backup
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
#include <twl.h>
#include <DEMO.h>

static void PrintBootType();

static void InitDEMOSystem();
static void InitInteruptSystem();
static void InitAllocSystem();
static void InitFileSystem();

static char* LoadFile(const char* path);
static BOOL SaveFile(const char* path, void* pData, u32 size);
static u32 GetCounter(const char* root);
static void SetCounter(const char* root, u32 counter);
static void PrintBackup(const char* root);
static BOOL AddBackup(const char* root);
static void PrintAndAddBackup(const char* root);

// OS_Printfのフック関数
#ifndef SDK_FINALROM
void OS_Printf(const char *fmt, ...)
{
    char dst[256];
    
    int     ret;
    va_list va;
    va_start(va, fmt);
    ret = OS_VSPrintf(dst, fmt, va);
    va_end(va);

    OS_PutString(dst);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  メイン関数です。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    OS_Init();
    RTC_Init();
    InitInteruptSystem();
    InitFileSystem();
    InitAllocSystem();
    InitDEMOSystem();
    OS_Printf("*** start nandApp demo\n");

    OS_Printf("Build:%s %s\n", __DATE__, __TIME__);

    PrintBootType();

    // Shared2 ファイルは NAND 上に保存されます。
    // そのため使用する API は  FS API となり、
    // ファイルシステムとしてアクセスすることができます。
    {
        // 「shared2」は アプリ間で共有できるファイルのアーカイブ名です。
        OS_Printf("Shared2 file %04x:\n", *(u16 *)( HW_TWL_ROM_HEADER_BUF + 0x020c ) );
        PrintAndAddBackup("shared2:");

        OS_Printf("\n");
    }

    OS_Printf("*** End of demo\n");

	
    // ランチャーに戻れるように、
    // 終了しない
    for (;;)
    {
        // フレーム更新。
        {
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         LoadFile

  Description:  内部でメモリを確保しファイルを読み込みます。

  Arguments:    path:   読み込むファイルのパス。

  Returns:      ファイルが存在するならファイルの内容が読み込まれた
                内部で確保したバッファへのポインタを返します。
                このポインタは FS_Free で解放する必要があります。
 *---------------------------------------------------------------------------*/
static char* LoadFile(const char* path)
{
    FSFile f;
    BOOL bSuccess;
    char* pBuffer;
    u32 fileSize;
    s32 readSize;

    FS_InitFile(&f);

    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_R);
    if( ! bSuccess )
    {
        return NULL;
    }

    fileSize = FS_GetFileLength(&f);
    pBuffer = (char*)OS_Alloc(fileSize + 1);
    SDK_POINTER_ASSERT(pBuffer);

    readSize = FS_ReadFile(&f, pBuffer, (s32)fileSize);
    SDK_ASSERT( readSize == fileSize );

    bSuccess = FS_CloseFile(&f);
    SDK_ASSERT( bSuccess );

    pBuffer[fileSize] = '\0';
    return pBuffer;
}

/*---------------------------------------------------------------------------*
  Name:         SaveFile

  Description:  ファイルを作成しデータを書き込みます。
                途中のディレクトリは作成しません。

  Arguments:    path:   作成するファイルのパス。
                pData:  書き込むデータ。
                size:   書き込むデータのサイズ。

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
static BOOL SaveFile(const char* path, void* pData, u32 size)
{
    FSFile f;
    BOOL bSuccess;
    FSResult fsResult;
    s32 writtenSize;

    FS_InitFile(&f);

    FS_CreateFile(path, (FS_PERMIT_R|FS_PERMIT_W));
    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_W);
    if (bSuccess == FALSE)
    {
        FSResult res = FS_GetArchiveResultCode(path);
        OS_Printf("Failed create file:%d\n", res);
        return FALSE;
    }
    SDK_ASSERT( bSuccess );

    fsResult = FS_SetFileLength(&f, 0);
    SDK_ASSERT( fsResult == FS_RESULT_SUCCESS );

    writtenSize = FS_WriteFile(&f, pData, (s32)size);
    SDK_ASSERT( writtenSize == size );

    bSuccess = FS_CloseFile(&f);
    SDK_ASSERT( bSuccess );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         GetCounter

  Description:  /counter.bin に保存されているカウンタ値を読み込みます。

  Arguments:    root:   counter.bin が保存されているディレクトリのパス。

  Returns:      カウンタ値を返します。
                /counter.bin が存在しない場合は 0 を返します。
 *---------------------------------------------------------------------------*/
static u32 GetCounter(const char* root)
{
    char path[FS_FILE_NAME_MAX];
    char* pBuffer;
    u32 counter = 0;

    STD_TSNPrintf(path, sizeof(path), "%s/counter.bin", root);

    pBuffer = LoadFile(path);
    if( pBuffer != NULL )
    {
        counter = MI_LoadLE32(pBuffer);
        OS_Free(pBuffer);
    }

    OS_Printf("%s counter is %d.\n", root, counter);

    return counter;
}

/*---------------------------------------------------------------------------*
  Name:         SetCounter

  Description:  /counter.bin にカウンタ値を保存します。
                ファイルが無い場合は作成します。

  Arguments:    root:       counter.bin を保存するディレクトリのパス。
                counter:    保存するカウンタ値。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void SetCounter(const char* root, u32 counter)
{
    char path[FS_FILE_NAME_MAX];

    STD_TSNPrintf(path, sizeof(path), "%s/counter.bin", root);
    (void)SaveFile(path, &counter, sizeof(counter));
}

/*---------------------------------------------------------------------------*
  Name:         PrintBackup

  Description:  指定されたディレクトリの内容を print します。
                サブディレクトリの内容は print しません。

  Arguments:    root:   対象のディレクトリのパス。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void PrintBackup(const char* root)
{
    FSFile dir;
    FSDirectoryEntryInfo entry;
    BOOL bSuccess;
    int numEntry = 0;

    FS_InitFile(&dir);

    bSuccess = FS_OpenDirectory(&dir, root, FS_FILEMODE_R);
    if( bSuccess )
    {
        while( FS_ReadDirectory(&dir, &entry) )
        {
            // 自分や親を指すエントリはスキップします
            if( (STD_CompareString(entry.longname, ".")  == 0)
             || (STD_CompareString(entry.longname, "..") == 0) )
            {
                continue;
            }

            if( (entry.attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0 )
            {
                // ディレクトリ
                OS_Printf("  d %s\n", entry.longname);
            }
            else
            {
                // ファイル

                char path[FS_FILE_NAME_MAX];
                char* text;
                char* found_cr;
                char* found_lf;

                // 最初の改行までの内容を表示します。
                STD_TSNPrintf(path, sizeof(path), "%s/%s", root, entry.longname);
                text = LoadFile(path);
                SDK_POINTER_ASSERT(text);

                found_cr = STD_SearchString(text, "\r");
                if( found_cr != NULL )
                {
                    *found_cr = '\0';
                }
                found_lf = STD_SearchString(text, "\n");
                if( found_lf != NULL )
                {
                    *found_lf = '\0';
                }

                OS_Printf("  f %-6s %s\n", entry.longname, text);
                OS_Free(text);
            }

            numEntry++;
        }

        bSuccess = FS_CloseDirectory(&dir);
        SDK_ASSERT( bSuccess );
    }

    if( numEntry == 0 )
    {
        OS_TPrintf("  (no entry)\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         AddBackup

  Description:  対象のディレクトリにテスト用のディレクトリとファイルを
                作成します。

  Arguments:    root:   対象のディレクトリのパス。

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
static BOOL AddBackup(const char* root)
{
    char path[FS_FILE_NAME_MAX];
    char text[256];
    u32 counter;
    BOOL bSuccess;
	u8 GameCode[ 5 ];
	
	// ROMヘッダからGameCode取得
	{
		int i;
		u8 *p = (u8 *)(HW_TWL_ROM_HEADER_BUF + 0x0233 );
		MI_CpuClear8( GameCode, sizeof(GameCode) );
		for( i = 0; i < 4; i++ ) {
			GameCode[ i ] = *p--;
		}
	}
    // ファイルに書き込む内容を生成します。
    STD_TSNPrintf(text, sizeof(text), ": generated by %s", GameCode );

    // ファイル名の生成に使用するカウンタ値を取得/更新します。
    counter = GetCounter(root);
    counter++;
    SetCounter(root, counter);

    // ディレクトリを作成
    STD_TSNPrintf(path, sizeof(path), "%s/dir_%d", root, counter);
    bSuccess = FS_CreateDirectory(path, (FS_PERMIT_R|FS_PERMIT_W));
    if (bSuccess == FALSE)
    {
        FSResult res = FS_GetArchiveResultCode(path);
        OS_Printf("Failed create directory:%d\n", res);
        return FALSE;
    }
    SDK_ASSERT( bSuccess );

    // ファイルを作成
    STD_TSNPrintf(path, sizeof(path), "%s/file_%d", root, counter);
    (void)SaveFile(path, text, (u32)STD_GetStringLength(text));
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         PrintAndAddBackup

  Description:  対象のディレクトリの内容を表示した後、テスト用の
                ディレクトリとファイルを作成します。

  Arguments:    root:   対象のディレクトリのパス。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void PrintAndAddBackup(const char* root)
{
    PrintBackup(root);
    (void)AddBackup(root);
}


/*---------------------------------------------------------------------------*
  Name:         PrintBootType

  Description:  BootType を print します。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void PrintBootType()
{
    const OSBootType btype = OS_GetBootType();

    switch( btype )
    {
    case OS_BOOTTYPE_ROM:   OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_ROM\n"); break;
    case OS_BOOTTYPE_NAND:  OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_NAND\n"); break;
    default:
        {
            OS_Warning("unknown BootType(=%d)", btype);
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem

  Description:  コンソールの画面出力用の表示設定を行います。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
    // 画面表示の初期化。
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem

  Description:  割り込みを初期化します。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // 個別割り込みフラグを全て不許可に
    (void)OS_SetIrqMask(0);

    // マスター割り込みフラグを許可に
    (void)OS_EnableIrq();

    // IRQ 割り込みを許可します
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocSystem

  Description:  ヒープを作成して OS_Alloc が使えるようにします。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitAllocSystem()
{
    void* newArenaLo;
    OSHeapHandle hHeap;

    // メインアリーナのアロケートシステムを初期化
    newArenaLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo(newArenaLo);

    // メインアリーナ上にヒープを作成
    hHeap = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}

/*---------------------------------------------------------------------------*
  Name:         InitFileSystem

  Description:  FS を初期化します。
                この関数を呼び出す前に InitInteruptSystem() が
                呼ばれている必要があります。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitFileSystem()
{
    // ARM7との通信FIFO割り込み許可
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);

    // ファイルシステム初期化
    FS_Init( FS_DMA_NOT_USE );
}
