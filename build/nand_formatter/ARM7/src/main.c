/*---------------------------------------------------------------------------*
  Project:  TwlIPL
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

#include	<nitro/types.h>
#include	<twl/init/crt0.h>
#include	<twl/memorymap_sp.h>
#include	<twl/os.h>
#include	<twl/spi.h>
#include	<twl/fatfs.h>
#include	<nitro/pad.h>
#include	<nitro/std.h>
#include	<nitro/card.h>


extern void		Dummy_FLXWRAM(void);
extern void		Dummy_FLXMAIN(void);
#ifdef	SDK_WIRELESS_IN_VRAM
extern void		Dummy_EXTWRAM(void);
#endif
extern void		Dummy_LTDWRAM(void);
extern void		Dummy_LTDMAIN(void);

static void		Pragma_LTDWRAM(void);
static void		Pragma_LTDMAIN(void);

static void CreateDirectory( const char *pDrive, const char **ppDirList );
static void CheckDirectory( const char *pDrive, const char **ppDirList );

// const data--------------------------------------------------------
static const char *s_dirList0[] = {
	(const char *)"sys",
	(const char *)"title",
	(const char *)"ticket",
	(const char *)"shared1",
	(const char *)"import",
	(const char *)"tmp",
	NULL,
	};

static const char *s_dirList1[] = {
	(const char *)"data",
	(const char *)"photo",
	(const char *)"shared2",
	(const char *)"tmp",
	NULL,
	};

/*---------------------------------------------------------------------------*/
#include	<twl/ltdwram_begin.h>
static void
Pragma_LTDWRAM(void)
{
	OS_Printf("Printed from limited wram area by pragma. [%p]\n", Pragma_LTDWRAM);
}
#include	<twl/ltdwram_end.h>

/*---------------------------------------------------------------------------*/
#include	<twl/ltdmain_begin.h>
static void
Pragma_LTDMAIN(void)
{
	OS_Printf("Printed from limited main memory area by pragma. [%p]\n", Pragma_LTDMAIN);
}
#include	<twl/ltdmain_end.h>


/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  メモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    OS_TPrintf("OS_GetWramSubPrivArenaLo() = %p\n", OS_GetWramSubPrivArenaLo());
    OS_TPrintf("OS_GetWramSubPrivArenaHi() = %p\n", OS_GetWramSubPrivArenaHi());
    OS_TPrintf("OS_GetWramSubArenaLo() = %p\n", OS_GetWramSubArenaLo());
    OS_TPrintf("OS_GetWramSubArenaHi() = %p\n", OS_GetWramSubArenaHi());
    OS_TPrintf("OS_GetSubPrivArenaLo() = %p\n", OS_GetSubPrivArenaLo());
    OS_TPrintf("OS_GetSubPrivArenaHi() = %p\n", OS_GetSubPrivArenaHi());

    OS_TPrintf("call OS_SetWramSubPrivArenaHi(0x0380f980); to fix arena.\n");
    OS_SetWramSubPrivArenaHi((void*)0x0380f980);

    // メモリ割当て初期化
    tempLo = OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV,
                          OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi(), 1);

    // アリーナを0クリア
    MI_CpuClear8(tempLo, (u32)OS_GetWramSubPrivArenaHi() - (u32)tempLo);

    // アリーナ下位アドレスを設定
    OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, tempLo);
    // ヒープ作成
    hh = OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV,
                       OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi());

    if (hh < 0)
    {
        OS_Panic("ARM7: Fail to create heap.\n");
    }

    // カレントヒープに設定
    (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);
}

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain
  Description:  起動ベクタ。
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
TwlSpMain(void)
{
    int     nand_fat_partition_num;
  
    OS_Init();
    SPI_Init(2);
    OS_InitTick();
    OS_InitAlarm();
    OS_InitThread();
    (void)PAD_InitXYButton();
    InitializeAllocateSystem();

    // 割り込みの有効化
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // FATFSライブラリ用ヒープを生成 (応急的にARM9側の空白部分を間借り)
    {
        OSHeapHandle hh;
        u8     *lo = (u8*)0x02A00000; // OS_GetSubPrivArenaLo()
        u8     *hi = (u8*)0x02B00000; // OS_GetSubPrivArenaHi()
        OS_SetSubPrivArenaLo(OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1));
        hh = OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, lo + 32, hi - 32);
        OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
    }

    // FATFSライブラリのSDIOとFATパーサを初期化
    if(!FATFS_Init(FATFS_DMA_NOT_USE, 8))
    {
        OS_TPanic("FATFS_Init() failed.\n");
    }

    // NANDをフォーマット
    {
#define FATAL_ERROR() do {} while (TRUE)
	// ここを編集したら、SDKのFATFS_Init内の同パラメータも修正する必要がある。
#define NAND_SIZE				245			// 256MB mobiNANDでの使用可能サイズ（iNANDでは違う値になる。未定。）
#define PARTITION_RAW_SIZE		4
#define PARTITION_0_SIZE		190
#define PARTITION_1_SIZE		( NAND_SIZE - PARTITION_RAW_SIZE - PARTITION_0_SIZE )
#define NAND_FAT_PARTITION_NUM	2			// FATパーティション数（RAWパーティションを除く）
		
        int     i;

        /* パーティションサイズをプロンプトから設定 */
        u32     partition_MB_size[5];
        partition_MB_size[0] = PARTITION_RAW_SIZE;		// RAW領域
        partition_MB_size[1] = PARTITION_0_SIZE;		// FAT0領域
        partition_MB_size[2] = PARTITION_1_SIZE;		// FAT1領域
        nand_fat_partition_num = NAND_FAT_PARTITION_NUM;
      
        for( i=0; i<nand_fat_partition_num; i++) {
            char    drive[4];
            STD_TSPrintf( drive, "%c:", 'F'+i);
            FATFS_UnmountDrive( drive);
        }
      
        if (FATFSi_SetNANDPartitions(partition_MB_size, nand_fat_partition_num))
        {
            // メディア全体をフォーマット
            if (FATFS_MountDrive("F", FATFS_MEDIA_TYPE_NAND, 0))
            {
                const char     *path = "F:";		// "F:"がFAT0パーティションになる。
                if (!FATFS_SetDefaultDrive(path))
                {
                    FATAL_ERROR();
                }
                else if(!FATFS_FormatMedia(path))
                {
                    FATAL_ERROR();
                }
                else if(!FATFS_FormatDrive(path))
                {
                    FATAL_ERROR();
                }
                else
                {
                    for (i = 1; i < nand_fat_partition_num; ++i)
                    {
                        char    drive[4];
                        STD_TSPrintf(drive, "%c:", 'F' + i);
                        if(!FATFS_MountDrive(drive, FATFS_MEDIA_TYPE_NAND, (u32)i))
                        {
                            FATAL_ERROR();
                            break;
                        }
                    }
                    {
                        for (i = 1; i < nand_fat_partition_num; ++i)
                        {
                            char    drive[4];
                            STD_TSPrintf(drive, "%c:", 'F' + i);
                            if (!FATFS_FormatDrive(drive))
                            {
                                FATAL_ERROR();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

	// ディレクトリ生成＆チェック
	CreateDirectory( "F:", s_dirList0 );
	CheckDirectory ( "F:", s_dirList0 );
	CreateDirectory( "G:", s_dirList1 );
	CheckDirectory ( "G:", s_dirList1 );
	
	// ドライブアンマウント
    {
        int i;
        for( i=0; i<nand_fat_partition_num; i++) {
            char    drive[4];
            STD_TSPrintf( drive, "%c:", 'F'+i);
            FATFS_UnmountDrive( drive);
        }
    }
  
    // ARM9に完了通知
    {
        const CARDRomHeader    *header = (const CARDRomHeader *)HW_ROM_HEADER_BUF;;
        *(u32 *)header->main_ram_address = 0x00000000;
    }

	while (TRUE)
	{
		OS_Halt();
	}
}


// ディレクトリ作成
static void CreateDirectory( const char *pDrive, const char **ppDirList )
{
	// デフォルトドライブの指定
	OS_TPrintf( "\nCreate directory : %s\n", pDrive );
	if( !FATFS_SetDefaultDrive( pDrive ) ) {
        FATAL_ERROR();
	}
	// 指定されたディレクトリをルートに作成
	while( *ppDirList ) {
		OS_TPrintf( "  %s...", *ppDirList );
		if( !FATFS_CreateDirectory( *ppDirList, "rwxrwxrwx") ) {
			OS_TPrintf( "ng.\n" );
			FATAL_ERROR();
		}
		OS_TPrintf( "ok.\n" );
		ppDirList++;
	}
}


// ディレクトリ存在チェック
static void CheckDirectory( const char *pDrive, const char **ppDirList )
{
	// デフォルトドライブの指定
	OS_TPrintf( "\nCheck directory : %s\n", pDrive );
	if( !FATFS_SetDefaultDrive( pDrive ) ) {
        FATAL_ERROR();
	}
	
	// 指定されたディレクトリをチェック
	while( *ppDirList ) {
	    FATFSDirectoryHandle dir = FATFS_OpenDirectory( *ppDirList, "rw");
		OS_TPrintf( "  %s...", *ppDirList );
        if (dir != NULL)
	    {
			OS_TPrintf( "ok.\n" );
    	    (void)FATFS_CloseDirectory( dir );
		}else {
			OS_TPrintf( "ng.\n" );
			FATAL_ERROR();
		}
		ppDirList++;
	}
}
