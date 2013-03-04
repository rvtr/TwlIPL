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

// OS_Printf�̃t�b�N�֐�
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

  Description:  ���C���֐��ł��B

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
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

    // Shared2 �t�@�C���� NAND ��ɕۑ�����܂��B
    // ���̂��ߎg�p���� API ��  FS API �ƂȂ�A
    // �t�@�C���V�X�e���Ƃ��ăA�N�Z�X���邱�Ƃ��ł��܂��B
    {
        // �ushared2�v�� �A�v���Ԃŋ��L�ł���t�@�C���̃A�[�J�C�u���ł��B
        OS_Printf("Shared2 file %04x:\n", *(u16 *)( HW_TWL_ROM_HEADER_BUF + 0x020c ) );
        PrintAndAddBackup("shared2:");

        OS_Printf("\n");
    }

    OS_Printf("*** End of demo\n");

	
    // �����`���[�ɖ߂��悤�ɁA
    // �I�����Ȃ�
    for (;;)
    {
        // �t���[���X�V�B
        {
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         LoadFile

  Description:  �����Ń��������m�ۂ��t�@�C����ǂݍ��݂܂��B

  Arguments:    path:   �ǂݍ��ރt�@�C���̃p�X�B

  Returns:      �t�@�C�������݂���Ȃ�t�@�C���̓��e���ǂݍ��܂ꂽ
                �����Ŋm�ۂ����o�b�t�@�ւ̃|�C���^��Ԃ��܂��B
                ���̃|�C���^�� FS_Free �ŉ������K�v������܂��B
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

  Description:  �t�@�C�����쐬���f�[�^���������݂܂��B
                �r���̃f�B���N�g���͍쐬���܂���B

  Arguments:    path:   �쐬����t�@�C���̃p�X�B
                pData:  �������ރf�[�^�B
                size:   �������ރf�[�^�̃T�C�Y�B

  Returns:      ���������TRUE
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

  Description:  /counter.bin �ɕۑ�����Ă���J�E���^�l��ǂݍ��݂܂��B

  Arguments:    root:   counter.bin ���ۑ�����Ă���f�B���N�g���̃p�X�B

  Returns:      �J�E���^�l��Ԃ��܂��B
                /counter.bin �����݂��Ȃ��ꍇ�� 0 ��Ԃ��܂��B
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

  Description:  /counter.bin �ɃJ�E���^�l��ۑ����܂��B
                �t�@�C���������ꍇ�͍쐬���܂��B

  Arguments:    root:       counter.bin ��ۑ�����f�B���N�g���̃p�X�B
                counter:    �ۑ�����J�E���^�l�B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static void SetCounter(const char* root, u32 counter)
{
    char path[FS_FILE_NAME_MAX];

    STD_TSNPrintf(path, sizeof(path), "%s/counter.bin", root);
    (void)SaveFile(path, &counter, sizeof(counter));
}

/*---------------------------------------------------------------------------*
  Name:         PrintBackup

  Description:  �w�肳�ꂽ�f�B���N�g���̓��e�� print ���܂��B
                �T�u�f�B���N�g���̓��e�� print ���܂���B

  Arguments:    root:   �Ώۂ̃f�B���N�g���̃p�X�B

  Returns:      �Ȃ��B
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
            // ������e���w���G���g���̓X�L�b�v���܂�
            if( (STD_CompareString(entry.longname, ".")  == 0)
             || (STD_CompareString(entry.longname, "..") == 0) )
            {
                continue;
            }

            if( (entry.attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0 )
            {
                // �f�B���N�g��
                OS_Printf("  d %s\n", entry.longname);
            }
            else
            {
                // �t�@�C��

                char path[FS_FILE_NAME_MAX];
                char* text;
                char* found_cr;
                char* found_lf;

                // �ŏ��̉��s�܂ł̓��e��\�����܂��B
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

  Description:  �Ώۂ̃f�B���N�g���Ƀe�X�g�p�̃f�B���N�g���ƃt�@�C����
                �쐬���܂��B

  Arguments:    root:   �Ώۂ̃f�B���N�g���̃p�X�B

  Returns:      ���������TRUE
 *---------------------------------------------------------------------------*/
static BOOL AddBackup(const char* root)
{
    char path[FS_FILE_NAME_MAX];
    char text[256];
    u32 counter;
    BOOL bSuccess;
	u8 GameCode[ 5 ];
	
	// ROM�w�b�_����GameCode�擾
	{
		int i;
		u8 *p = (u8 *)(HW_TWL_ROM_HEADER_BUF + 0x0233 );
		MI_CpuClear8( GameCode, sizeof(GameCode) );
		for( i = 0; i < 4; i++ ) {
			GameCode[ i ] = *p--;
		}
	}
    // �t�@�C���ɏ������ޓ��e�𐶐����܂��B
    STD_TSNPrintf(text, sizeof(text), ": generated by %s", GameCode );

    // �t�@�C�����̐����Ɏg�p����J�E���^�l���擾/�X�V���܂��B
    counter = GetCounter(root);
    counter++;
    SetCounter(root, counter);

    // �f�B���N�g�����쐬
    STD_TSNPrintf(path, sizeof(path), "%s/dir_%d", root, counter);
    bSuccess = FS_CreateDirectory(path, (FS_PERMIT_R|FS_PERMIT_W));
    if (bSuccess == FALSE)
    {
        FSResult res = FS_GetArchiveResultCode(path);
        OS_Printf("Failed create directory:%d\n", res);
        return FALSE;
    }
    SDK_ASSERT( bSuccess );

    // �t�@�C�����쐬
    STD_TSNPrintf(path, sizeof(path), "%s/file_%d", root, counter);
    (void)SaveFile(path, text, (u32)STD_GetStringLength(text));
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         PrintAndAddBackup

  Description:  �Ώۂ̃f�B���N�g���̓��e��\��������A�e�X�g�p��
                �f�B���N�g���ƃt�@�C�����쐬���܂��B

  Arguments:    root:   �Ώۂ̃f�B���N�g���̃p�X�B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static void PrintAndAddBackup(const char* root)
{
    PrintBackup(root);
    (void)AddBackup(root);
}


/*---------------------------------------------------------------------------*
  Name:         PrintBootType

  Description:  BootType �� print ���܂��B

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
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

  Description:  �R���\�[���̉�ʏo�͗p�̕\���ݒ���s���܂��B

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
    // ��ʕ\���̏������B
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

  Description:  ���荞�݂����������܂��B

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // �ʊ��荞�݃t���O��S�ĕs����
    (void)OS_SetIrqMask(0);

    // �}�X�^�[���荞�݃t���O������
    (void)OS_EnableIrq();

    // IRQ ���荞�݂������܂�
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocSystem

  Description:  �q�[�v���쐬���� OS_Alloc ���g����悤�ɂ��܂��B

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static void InitAllocSystem()
{
    void* newArenaLo;
    OSHeapHandle hHeap;

    // ���C���A���[�i�̃A���P�[�g�V�X�e����������
    newArenaLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo(newArenaLo);

    // ���C���A���[�i��Ƀq�[�v���쐬
    hHeap = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}

/*---------------------------------------------------------------------------*
  Name:         InitFileSystem

  Description:  FS �����������܂��B
                ���̊֐����Ăяo���O�� InitInteruptSystem() ��
                �Ă΂�Ă���K�v������܂��B

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
static void InitFileSystem()
{
    // ARM7�Ƃ̒ʐMFIFO���荞�݋���
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);

    // �t�@�C���V�X�e��������
    FS_Init( FS_DMA_NOT_USE );
}
