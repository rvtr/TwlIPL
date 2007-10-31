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


typedef struct FileProperty {
	u32			size;
	const char *path;
}FileProperty;

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
static void CheckDirectory ( const char *pDrive, const char **ppDirList );
static void CreateFile( const FileProperty *pFileList );
static void CheckFile ( const FileProperty *pFileList );

// const data--------------------------------------------------------

// FATFS�̃N���X�^�T�C�Y��16KB
static const FileProperty s_fileList[] = {
	{   128, "F:/sys/ID.sgn"           },	// ����A�S���T�C�Y�͓K���B���g����B
	{  4096, "F:/sys/HWINFO.dat"       },
	{  4096, "F:/shared1/TWLCFG0.dat"  },
	{  4096, "F:/shared1/TWLCFG1.dat"  },	// �~���[
	{  4096, "F:/shared1/WIFICFG0.dat" },
	{  4096, "F:/shared1/WIFICFG1.dat" },	// �~���[
	{     0, NULL },
};

static const char *s_pDirList0[] = {
	(const char *)"sys",
	(const char *)"title",
	(const char *)"ticket",
	(const char *)"shared1",
	(const char *)"import",
	(const char *)"tmp",
	NULL,
	};

static const char *s_pDirList1[] = {
	(const char *)"photo",
	(const char *)"shared2",
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

  Description:  �����������ăV�X�e��������������B

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

    // �����������ď�����
    tempLo = OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV,
                          OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi(), 1);

    // �A���[�i��0�N���A
    MI_CpuClear8(tempLo, (u32)OS_GetWramSubPrivArenaHi() - (u32)tempLo);

    // �A���[�i���ʃA�h���X��ݒ�
    OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, tempLo);
    // �q�[�v�쐬
    hh = OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV,
                       OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi());

    if (hh < 0)
    {
        OS_Panic("ARM7: Fail to create heap.\n");
    }

    // �J�����g�q�[�v�ɐݒ�
    (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);
}

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain
  Description:  �N���x�N�^�B
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

    // ���荞�݂̗L����
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // FATFS���C�u�����p�q�[�v�𐶐� (���}�I��ARM9���̋󔒕������Ԏ؂�)
    {
        OSHeapHandle hh;
        u8     *lo = (u8*)0x02A00000; // OS_GetSubPrivArenaLo()
        u8     *hi = (u8*)0x02B00000; // OS_GetSubPrivArenaHi()
        OS_SetSubPrivArenaLo(OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1));
        hh = OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, lo + 32, hi - 32);
        OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
    }

    // FATFS���C�u������SDIO��FAT�p�[�T��������
    if(!FATFS_Init(FATFS_DMA_NOT_USE, 8))
    {
        OS_TPanic("FATFS_Init() failed.\n");
    }

    // NAND���t�H�[�}�b�g
    {
#define FATAL_ERROR() do {} while (TRUE)
	// ������ҏW������ASDK��FATFS_Init���̓��p�����[�^���C������K�v������B
#define NAND_SIZE				245			// 256MB mobiNAND�ł̎g�p�\�T�C�Y�iiNAND�ł͈Ⴄ�l�ɂȂ�B����B�j
#define PARTITION_RAW_SIZE		4
#define PARTITION_0_SIZE		213
#define PARTITION_1_SIZE		( NAND_SIZE - PARTITION_RAW_SIZE - PARTITION_0_SIZE )
#define NAND_FAT_PARTITION_NUM	2			// FAT�p�[�e�B�V�������iRAW�p�[�e�B�V�����������j
		
        int     i;

        /* �p�[�e�B�V�����T�C�Y���v�����v�g����ݒ� */
        u32     partition_MB_size[5];
        partition_MB_size[0] = PARTITION_RAW_SIZE;		// RAW�̈�
        partition_MB_size[1] = PARTITION_0_SIZE;		// FAT0�̈�
        partition_MB_size[2] = PARTITION_1_SIZE;		// FAT1�̈�
        nand_fat_partition_num = NAND_FAT_PARTITION_NUM;
      
        for( i=0; i<nand_fat_partition_num; i++) {
            char    drive[4];
            STD_TSPrintf( drive, "%c:", 'F'+i);
            FATFS_UnmountDrive( drive);
        }
      
        if (FATFSi_SetNANDPartitions(partition_MB_size, nand_fat_partition_num))
        {
            // ���f�B�A�S�̂��t�H�[�}�b�g
            if (FATFS_MountDrive("F", FATFS_MEDIA_TYPE_NAND, 0))
            {
                const char     *path = "F:";		// "F:"��FAT0�p�[�e�B�V�����ɂȂ�B
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

	// �f�B���N�g���������`�F�b�N
	CreateDirectory( "F:", s_pDirList0 );
	CheckDirectory ( "F:", s_pDirList0 );
	CreateDirectory( "G:", s_pDirList1 );
	CheckDirectory ( "G:", s_pDirList1 );
	
	// �t�@�C���������`�F�b�N
	CreateFile( &s_fileList[0] );
	CheckFile ( &s_fileList[0] );
	
	// �h���C�u�A���}�E���g
    {
        int i;
        for( i=0; i<nand_fat_partition_num; i++) {
            char    drive[4];
            STD_TSPrintf( drive, "%c:", 'F'+i);
            FATFS_UnmountDrive( drive);
        }
    }
  
    // ARM9�Ɋ����ʒm
    {
        const CARDRomHeader    *header = (const CARDRomHeader *)HW_ROM_HEADER_BUF;;
        *(u32 *)header->main_ram_address = 0x00000000;
    }

	while (TRUE)
	{
		OS_Halt();
	}
}


// �f�B���N�g���쐬
static void CreateDirectory( const char *pDrive, const char **ppDirList )
{
	// �f�t�H���g�h���C�u�̎w��
	OS_TPrintf( "\nCreate directory : %s\n", pDrive );
	if( !FATFS_SetDefaultDrive( pDrive ) ) {
        FATAL_ERROR();
	}
	// �w�肳�ꂽ�f�B���N�g�������[�g�ɍ쐬
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


// �f�B���N�g�����݃`�F�b�N
static void CheckDirectory( const char *pDrive, const char **ppDirList )
{
	// �f�t�H���g�h���C�u�̎w��
	OS_TPrintf( "\nCheck directory : %s\n", pDrive );
	if( !FATFS_SetDefaultDrive( pDrive ) ) {
        FATAL_ERROR();
	}
	
	// �w�肳�ꂽ�f�B���N�g�����`�F�b�N
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


// �t�@�C���쐬
static void CreateFile( const FileProperty *pFileList )
{
	// �f�t�H���g�h���C�u�̎w��
	OS_TPrintf( "\nCreate File :\n" );
	
	// �w�肳�ꂽ�f�B���N�g�������[�g�ɍ쐬
	while( pFileList->path ) {
		FATFSFileHandle file;
		
		OS_TPrintf( "  %s, %dbytes...", pFileList->path, pFileList->size );
		if( !FATFS_CreateFile( pFileList->path, TRUE, "rwxrwxrwx" ) ) {
			OS_TPrintf( "ng.\n" );
			FATAL_ERROR();
		}
		file = FATFS_OpenFile( pFileList->path, "w" );
		if( !file ) {
			OS_TPrintf( "ng.\n" );
			FATAL_ERROR();
		}
		if( !FATFS_SetFileLength( file, (int)pFileList->size ) ) {
			OS_TPrintf( "ng.\n" );
			FATAL_ERROR();
		}
		(void)FATFS_CloseFile( file );
		OS_TPrintf( "ok.\n" );
		pFileList++;
	}
}


// �t�@�C���`�F�b�N
static void CheckFile( const FileProperty *pFileList )
{
	// �f�t�H���g�h���C�u�̎w��
	OS_TPrintf( "\nCheck File :\n" );
	
	// �w�肳�ꂽ�f�B���N�g�������[�g�ɍ쐬
	while( pFileList->path ) {
		FATFSFileHandle file;
		
		OS_TPrintf( "  %s, %dbytes...", pFileList->path, pFileList->size );
		file = FATFS_OpenFile( pFileList->path, "r" );
		if( !file ) {
			OS_TPrintf( "ng.\n" );
			FATAL_ERROR();
		}
		if( FATFS_GetFileLength( file ) != pFileList->size ) {
			OS_TPrintf( "ng. size = %d\n", FATFS_GetFileLength( file )  );
			FATAL_ERROR();
		}
		(void)FATFS_CloseFile( file );
		OS_TPrintf( "ok.\n" );
		pFileList++;
	}
}

