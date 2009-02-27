/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_import.c

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
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include <twl/lcfg.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "hw_info.h"
#include "TWLHWInfo_api.h"
#include "graphics.h"
#include "kami_global.h"
#include "font.h"
#include "sort_title.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

#define DIR_NUM           3
#define FULLPATH_LEN      ( FS_ENTRY_LONGNAME_MAX+6 )

static const char* sDirectoryNameRegion[] =
{
	"japan",
	"america",
	"europe",
	"australia"
};

static const char* sDirectoryNameConsole[] =
{
	"debugger",	  // IS_TWL_DEBUGGER
	"standalone", // IS_TWL_CAPTURE
	"standalone", // TWL
	""			  // UNKNOWN
};

typedef struct {
	u8 dirNameIndex;
	char fileName[FS_ENTRY_LONGNAME_MAX];
} ImportFileInfo;

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static u32  sCurrentProgress;
static vu8 sNowImport = FALSE;
static vu8 sProgress  = FALSE;
static u8  sStack[THREAD_STACK_SIZE];

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static s32 kamiImportTad(const char* path, BOOL erase);
static void ProgressThread(void* arg);
static void Destructor(void* arg);
void ProgressDraw(f32 ratio);
BOOL ImportDirectoryTad(char* directory);

static u32 CountTadInDir( char (*dir_path)[FULLPATH_LEN], u32 dir_num );
static BOOL MakeList( char (*dir)[FULLPATH_LEN], u32 dir_max, ImportFileInfo *info, TitleSortSet *sortset );
static BOOL ImportTadFromList( char (*dir)[FULLPATH_LEN], ImportFileInfo *info, TitleSortSet *sortset, u32 import_max );

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessImport

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ProcessImport( void *(*alloc)(unsigned long), void (*free)(void *) )
{
	const s32 MAX_RETRY_COUNT = 2;
	BOOL result = TRUE;
	char directory[DIR_NUM][FULLPATH_LEN];
	s32 i=0;
	s32 j=0;
	u32 fileCount = 0;
	ImportFileInfo *importFileInfoList = NULL;
	TitleSortSet *titleSortSetList = NULL;
	void *sortBuf = NULL;

	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas,  TXT_COLOR_WHITE, 0,  30, 256, 100);
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
	OS_WaitVBlankIntr();
	NNS_G2dTextCanvasDrawText(&gTextCanvas, 40, 60,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)L"Now Import..");

	while(!FadeInTick())
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);
		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
	    OS_WaitVBlankIntr();
	}

	STD_TSNPrintf(directory[0], sizeof(directory[0]), "rom:/data/%s/%s/", sDirectoryNameConsole[GetConsole()], sDirectoryNameRegion[gRegion]);
	STD_TSNPrintf(directory[1], sizeof(directory[1]), "rom:/data/common/%s/", sDirectoryNameRegion[gRegion]);
	STD_TSNPrintf(directory[2], sizeof(directory[2]), "rom:/data/common/");
	
	// �f�B���N�g������ TAD �t�@�C�������J�E���g����
	fileCount = CountTadInDir(directory, DIR_NUM);

	// �t�@�C����+�f�B���N�g���C���f�b�N�X�z��ATitleSortInfo �z����m��
	importFileInfoList = alloc( sizeof(ImportFileInfo) * fileCount );
	titleSortSetList = alloc( sizeof(TitleSortSet) * fileCount );
	
	// ����ǂݍ���
	result &= MakeList( directory, DIR_NUM, importFileInfoList, titleSortSetList);
	
	// TitleSortInfo���\�[�g
	sortBuf = alloc( MATH_QSortStackSize( fileCount ) );
	SortTitle( titleSortSetList, fileCount, sortBuf );
	
	// �C���|�[�g
	result &= ImportTadFromList( directory, importFileInfoList, titleSortSetList, fileCount );
	
	// ���͂�K�v�Ȃ����X�g�̉��
	free( importFileInfoList );
	free( titleSortSetList );

	while (!FadeOutTick())
	{
	    OS_WaitVBlankIntr();
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         CountTadInDir

  Description:  ���X�g�Ŏw�肵���f�B���N�g���ɂ��� TAD �̑�����Ԃ��܂��B

  Arguments:    dir_list,dir_max

  Returns:      u32
 *---------------------------------------------------------------------------*/
static u32 CountTadInDir( char (*dir_list)[FULLPATH_LEN], u32 dir_max )
{
	int l;
	u32 count = 0;
	for( l=0; l<dir_max; l++ )
	{
		char *dirName = dir_list[ l ];
	    FSFile  dir;
	    FSDirectoryEntryInfo   info[1];

		FS_InitFile(&dir);
		if (!FS_OpenDirectory(&dir, dirName, FS_FILEMODE_R))
		{
			// ��f�B���N�g����Makerom���ɍ폜�����悤�Ȃ̂ł����ł͔�΂�
			continue;
		}
		
	    while (FS_ReadDirectory(&dir, info))
	    {
	        if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
	        {
				char* pExtension;
				// �g���q�̃`�F�b�N
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".tad") || !STD_CompareString( pExtension, ".TAD")  )
					{
						count++;
					}
				}
	        }
		}
		FS_CloseDirectory( &dir );
	}
	return count;
}

/*---------------------------------------------------------------------------*
  Name:         MakeList

  Description:  ���X�g�Ŏw�肵���f�B���N�g������ ImportTadFromList 
                ���s���̂ɕK�v�ȃ��X�g���쐬���܂��B

  Arguments:    dir_list, dir_max, info, sortset

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
static BOOL MakeList( char (*dir_list)[FULLPATH_LEN], u32 dir_max, ImportFileInfo *info, TitleSortSet *sortset )
{
	int l;
	u32 count = 0;
	BOOL result = TRUE;
	
	for( l=0; l<dir_max; l++ )
	{
		char *dirName = dir_list[ l ];
	    FSFile  dir;
	    FSDirectoryEntryInfo   fsinfo[1];

		FS_InitFile(&dir);
		if (!FS_OpenDirectory(&dir, dirName, FS_FILEMODE_R))
		{
			// ��f�B���N�g����Makerom���ɍ폜�����悤�Ȃ̂ł����ł͔�΂�
			continue;
		}
		
	    while (FS_ReadDirectory(&dir, fsinfo))
	    {
	        if ((fsinfo->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
	        {
				char* pExtension;
				// �g���q�̃`�F�b�N
				pExtension = STD_SearchCharReverse( fsinfo->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".tad") || !STD_CompareString( pExtension, ".TAD")  )
					{
						NAMTadInfo tadInfo;
						char fullPath[FULLPATH_LEN];
						
						// �t���p�X�쐬
						STD_TSNPrintf(fullPath, sizeof(fullPath), "%s/%s", dirName, fsinfo->longname);

						// TAD���擾
						// tad�t�@�C���̏��擾
						if (NAM_ReadTadInfo(&tadInfo, fullPath) != NAM_OK)
						{
							// ���s������G���[��\�����Ď��̃t�@�C���ցA���ʂ�False
							kamiFontPrintfConsole(CONSOLE_RED, "Error NAM_ReadTadInfo()\n");
							kamiFontPrintfConsole(CONSOLE_RED, "file : %s\n",fsinfo->longname);
							result = FALSE;
							continue;
						}

						// ImportFileInfo �� TitleSortSet �ɏ���]��
						info[count].dirNameIndex = (u8)l;
						STD_TSNPrintf(info[count].fileName, FS_ENTRY_LONGNAME_MAX, fsinfo->longname);
						sortset[count].index = count;
						sortset[count].titleID = tadInfo.titleInfo.titleId;

						count++;
					}
				}
	        }
		}
	}
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         ImportTadFromList

  Description:  MakeList �œ������X�g�̏��ɏ]���� TAD ���C���|�[�g���܂��B

  Arguments:    dir_list, info, sortset, import_max

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
static BOOL ImportTadFromList( char (*dir_list)[FULLPATH_LEN], ImportFileInfo *info, TitleSortSet *sortset, u32 import_max )
{
	int l;
	int j;
	BOOL result = TRUE;
	s32 listNo=0;
	
	for( l=0; l<import_max; l++ )
	{
		char *longName = info[ sortset[ l ].index ].fileName;
		char *dirName = dir_list[ info[ sortset[ l ].index ].dirNameIndex ];
		char fullPath[FULLPATH_LEN];
		char string1[256];
		u16  string2[256];
		const s32 MAX_RETRY_COUNT = 2;
		s32  nam_result;
		char *tlo = (char *)( &sortset[ l ].titleID );
		
		// �t���p�X�쐬
		STD_TSNPrintf(fullPath, sizeof(fullPath), "%s/%s", dirName, longName);
		
		// �C���|�[�g
		STD_TSPrintf(string1, "List %d ", ++listNo);
		MI_CpuClear8(string2, sizeof(string2));
		STD_ConvertStringSjisToUnicode(string2, NULL, string1, NULL, NULL);

		NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_WHITE, 0, 60, 256, 20);
		NNS_G2dTextCanvasDrawText(&gTextCanvas, 40, 60,
			TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)L"Now Import..");
		NNS_G2dTextCanvasDrawText(&gTextCanvas, 135, 60,
			TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)string2);

		// MAX_RETRY_COUNT�܂Ń��g���C����
		for (j=0; j<MAX_RETRY_COUNT; j++)
		{	
			nam_result = kamiImportTad(fullPath, j);
			if (nam_result == NAM_OK)
			{
				break;
			}
			else
			{
				kamiFontPrintfConsole(CONSOLE_GREEN, "Import %d Retry!\n", listNo);
			}
		}

		if ( nam_result == NAM_OK)
		{
			// kamiFontPrintfConsole(FONT_COLOR_GREEN, "List : %d(%c%c%c%c) Import Success.\n", listNo, tlo[3], tlo[2], tlo[1], tlo[0] );
			kamiFontPrintfConsole(FONT_COLOR_GREEN, "List : %d Import Success.\n", listNo );
		}
		else
		{
			// kamiFontPrintfConsole(FONT_COLOR_RED, "Error: %d(%c%c%c%c) : RetCode = %d\n", listNo, tlo[3], tlo[2], tlo[1], tlo[0], nam_result );
			kamiFontPrintfConsole(FONT_COLOR_RED, "Error: %d : RetCode = %d\n", listNo, nam_result );
			result = FALSE;
		}

		kamiFontLoadScreenData();

	}
	return result;
}


/*---------------------------------------------------------------------------*
  Name:         ImportDirectoryTad

  Description:  �w�肵���f�B���N�g���ɂ���TAD�𖳏�����Import���܂��B

  Arguments:    path

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ImportDirectoryTad(char* directory)
{
    FSFile  dir;
    FSDirectoryEntryInfo   info[1];
	const s32 MAX_RETRY_COUNT = 2;
	BOOL result = TRUE;
	char full_path[FULLPATH_LEN];
	static s32 listNo=0;
	s32 j=0;

	FS_InitFile(&dir);
	if (!FS_OpenDirectory(&dir, directory, FS_FILEMODE_R))
	{
		// ��f�B���N�g����Makerom���ɍ폜�����悤�Ȃ̂ł����ł�TRUE��Ԃ�
//    	kamiFontPrintfConsole(CONSOLE_GREEN, "%s can not Open.\n", directory);
		return TRUE;
	}

	// tad�t�@�C�����������ăC���|�[�g
	// [TODO:]���full_path�̃��X�g������āANAM_ReadTadInfo�Ŏ���TitleID_lo�̒l�Ń\�[�g���Ă���
	// ���ԂɃC���|�[�g����悤�ɕύX����B
	// ���̊֐��𕪂���C���[�W�ŁA�e�t�H���_�̖��̃��X�g���ɍ쐬����
	// 
    while (FS_ReadDirectory(&dir, info))
    {
		s32  nam_result;
		char string1[256];
		u16  string2[256];

		MI_CpuClear8(string2, sizeof(string2));

        if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
        {
			char* pExtension;

			// �g���q�̃`�F�b�N
			pExtension = STD_SearchCharReverse( info->longname, '.');
			if (pExtension)
			{
				if (!STD_CompareString( pExtension, ".tad") || !STD_CompareString( pExtension, ".TAD")  )
				{

					STD_TSPrintf(string1, "List %d ", ++listNo);
					STD_ConvertStringSjisToUnicode(string2, NULL, string1, NULL, NULL);

					NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_WHITE, 0, 60, 256, 20);
					NNS_G2dTextCanvasDrawText(&gTextCanvas, 40, 60,
						TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)L"Now Import..");
					NNS_G2dTextCanvasDrawText(&gTextCanvas, 135, 60,
						TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)string2);

					STD_TSNPrintf(full_path, sizeof(full_path), "%s/%s", directory, info->longname);
//            		kamiFontPrintfConsole(CONSOLE_GREEN, "  %s\n", full_path);

					// MAX_RETRY_COUNT�܂Ń��g���C����
					for (j=0; j<MAX_RETRY_COUNT; j++)
					{	
						nam_result = kamiImportTad(full_path, j);
						if (nam_result == NAM_OK)
						{
							break;
						}
						else
						{
							kamiFontPrintfConsole(CONSOLE_GREEN, "Import %d Retry!\n", listNo);
						}
					}

					if ( nam_result == NAM_OK)
					{
						kamiFontPrintfConsole(FONT_COLOR_GREEN, "List : %d Import Success.\n", listNo);			
					}
					else
					{
						kamiFontPrintfConsole(FONT_COLOR_RED, "Error: %d : RetCode = %d\n", listNo, nam_result );
						result = FALSE;
					}

					kamiFontLoadScreenData();
				}
			}
        }
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         kamiImportTad

  Description:  .tad �t�@�C���C���|�[�g

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static s32 kamiImportTad(const char* path, BOOL erase)
{
	NAMTadInfo tadInfo;
	OSThread thread;
	s32  nam_result;

	// tad�t�@�C���̏��擾
	nam_result = NAM_ReadTadInfo(&tadInfo, path);
	if ( nam_result != NAM_OK )
	{
		return nam_result;
	}

	// ES�̎d�l�ŌÂ� e-ticket ������ƐV���� e-ticket ���g�����C���|�[�g���ł��Ȃ�
	// �b��Ή��Ƃ��ĊY���^�C�g�������S�폜���Ă���C���|�[�g����
	if (erase)
	{
		nam_result = NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
		if ( nam_result != NAM_OK )
		{
			kamiFontPrintfConsole(CONSOLE_RED, "Fail! RetCode=%x\n", nam_result);
			return FALSE;
		}
	}

	// �C���|�[�g�J�n�t���O�𗧂Ă�
	sNowImport = TRUE;

    // �i���X���b�h�쐬
	MI_CpuClear8(sStack, THREAD_STACK_SIZE);
    OS_CreateThread(&thread, ProgressThread, NULL,
        (void*)((u32)sStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_GetCurrentThread()->priority - 1);
    OS_WakeupThreadDirect(&thread);

	// Import�J�n
	nam_result = NAM_ImportTad( path );

	// �C���|�[�g�J�n�t���O��������
	sNowImport = FALSE;

	// �i���X���b�h�̎��͏I����҂�
	while (sProgress){};

	// ������ƕ\������
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
    OS_WaitVBlankIntr();

	// InstalledSoftBoxCount, FreeSoftBoxCount �̒l�����݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B
	(void)NAMUT_UpdateSoftBoxCount();

	return nam_result;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressThread

  Description:  .tad �t�@�C���C���|�[�g�̐i����\������X���b�h�B
				�i����100%�ɒB����Ə����𔲂���B

  Arguments:    arg -   �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void ProgressThread(void* /*arg*/)
{
    u32  currentSize;
    u32  totalSize   = 0;
	u32  totalSizeBk = 0;

	sProgress = TRUE;

    while (sNowImport)
    {
        NAM_GetProgress(&currentSize, &totalSize);

		if ((totalSize > 0 && totalSize == currentSize) || totalSizeBk > totalSize)
		{
			// ���ɃC���|�[�g���I��
			ProgressDraw((f32)1.0);
			break;	
		}
		else if (totalSize > 0)
		{
			ProgressDraw((f32)currentSize/totalSize);
		}

		totalSizeBk = totalSize;

		// V�u�����N�҂�
		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
        OS_WaitVBlankIntr();

		// 3D������
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);
    }

	sProgress = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressDraw

  Description:  �C���|�[�g�̐i����\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/

void ProgressDraw(f32 ratio)
{
	s16 x = (s16)(30 + (226 - 30)*ratio);

	// �O���[���o�[
	DrawQuadWithColors( 30,  86,   x,  95, GX_RGB(22, 31, 22), GX_RGB(12, 25, 12));

	// �O���[�o�[
	DrawQuad( 30,  86, 226,  95, GX_RGB(28, 28, 28));
}
