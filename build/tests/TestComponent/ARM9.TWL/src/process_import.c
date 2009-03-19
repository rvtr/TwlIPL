/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
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

#include "sort_title.h"

#include <twl.h>
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include <twl/lcfg.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_import.h"
#include "process_hw_info.h"
#include "process_eticket.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "common_utility.h"
#include "TWLHWInfo_api.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

typedef enum {
	TAD_WRITE_OPTION_OVERWRITE,		// �����㏑��
	TAD_WRITE_OPTION_NONEXISTENT,	// NAND�ɓ��v���O���������݂��Ȃ��ꍇ�Ɍ��菑������
	TAD_WRITE_OPTION_USER			// ���[�U�[�ɑI��������
} TadWriteOption;

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define NUM_OF_MENU_SELECT               4
#define NUM_OF_MENU_SELECT_INDIVIDUALLY 17

#define DOT_OF_MENU_SPACE               16
#define DOT_OF_MENU_SPACE_INDIVIDUALLY   8

#define CHAR_OF_MENU_SPACE               2
#define CHAR_OF_MENU_SPACE_INDIVIDUALLY  1

#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      40


// �\�����C���|�[�g�ł���.TAD�t�@�C���͍ő�16�܂�
// �������r�c�J�[�h�̃��[�g�ɑ��݂���t�@�C���݂̂Ƃ�������y����
#define FILE_NUM_MAX         256
#define QSORT_BUF_SIZE       ((8+1) * 8) // �T�C�Y��(Log2(FILE_NUM_MAX)+1) * 8 bytes �K�v ���I�m�ۂł���Ȃ炻�����̕����y

#define VIEW_LINES_MAX        16

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s32 sMenuSelectNo;
static s32 sMenuSelectNoIndividually;

static LCFGReadResult (*s_pReadSecureInfoFunc)( void );

static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
static TitleSortSet sTitleSortSet[FILE_NUM_MAX];

static u8 sFileNum;

static void* spStack;

static u32  sCurrentProgress;

static vu8 sNowImport = FALSE;

static s32 sTadListViewOffset;

static s32 	sLines;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL ImportTad(char* file_name, TadWriteOption option);
static void ProgressThread(void* arg);
static void Destructor(void* arg);
static void ShowTadList(void);
static void DumpTadInfo(void);
static void ShowTitleinfoDifference( NAMTitleInfo* titleInfoNand, NAMTitleInfo* titleInfoSd);
void ProgessInit(void);
void ProgressDraw(f32 ratio);
static void* ImportProcessReturn1(void);

static void* ImportIndividuallyProcess0(void);
static void* ImportIndividuallyProcess1(void);
static void* ImportIndividuallyProcess2(void);
static void* ImportIndividuallyProcess3(void);

static void* ImportAllOverwriteProcess0(void);
static void* ImportAllOverwriteProcess1(void);
static void* ImportAllOverwriteProcess2(void);
static void* ImportAllOverwriteProcess3(void);

static void* ImportAllNonexistentProcess0(void);
static void* ImportAllNonexistentProcess1(void);
static void* ImportAllNonexistentProcess2(void);
static void* ImportAllNonexistentProcess3(void);

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Import �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcess0(void)
{
    FSFile    dir;
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import TAD from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  5, FONT_COLOR_BLACK, "l  OVERWRITE ALL     l    l");
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l  WRITE NONEXISTENT l    l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l  SELECT FILE >>    l    l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l  RETURN            l    l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+--------------------+----+");

	// �z��N���A
	MI_CpuClear8( sFilePath, sizeof(sFilePath) );
	MI_CpuClear8( sTitleSortSet, sizeof(sTitleSortSet) );

	// �t�@�C����������
	sFileNum = 0;

	// �\���I�t�Z�b�g������
	sTadListViewOffset = 0;

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 1, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 2, BG_COLOR_PINK, BG_COLOR_TRANS );
	
	FS_InitFile(&dir);

    // SD�J�[�h�̃��[�g�f�B���N�g��������
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
        kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory(sdmc:/)\n");
    }
    else
    {
		int l;
		char qsortBuf[QSORT_BUF_SIZE];
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ tad file List -----\n");

		// .dat .nand .nor ��T���ăt�@�C������ۑ����Ă���
        while (FS_ReadDirectory(&dir, info))
        {
            OS_Printf("  %s", info->longname);
            if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) != 0)
            {
                OS_Printf("/\n");
            }
            else
            {
				char* pExtension;
              OS_Printf(" (%d BYTEs)\n", info->filesize);

				// �g���q�̃`�F�b�N
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".tad") || !STD_CompareString( pExtension, ".TAD")  )
					{
						NAMTadInfo tadInfo;
						char full_path[FS_ENTRY_LONGNAME_MAX+6];

						// �t���p�X���쐬
						MakeFullPathForSD(info->longname, full_path);

						STD_CopyString( sFilePath[sFileNum], info->longname );
						// kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);
						
						// tad�t�@�C���̏��擾
						if (NAM_ReadTadInfo(&tadInfo, full_path) != NAM_OK)
						{
							// ���s������G���[��\�����Č��݂̃t�@�C�����΂��Đ�֐i��
							kamiFontPrintfConsole(CONSOLE_RED, "Error NAM_ReadTadInfo()\n");
							continue;
						}
						sTitleSortSet[sFileNum].titleID = tadInfo.titleInfo.titleId;
						sTitleSortSet[sFileNum].path = sFilePath[sFileNum];

						if (++sFileNum >= FILE_NUM_MAX)
						{
							break;
						}
					}
				}
            }
        }
        (void)FS_CloseDirectory(&dir);
        
        // �t�@�C���p�X��TitleID_lo���Ƀ\�[�g����
        SortTitle( sTitleSortSet, sFileNum, qsortBuf );
        
        for( l=0; l<sFileNum; l++ )
        {
			kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", l, sTitleSortSet[l].path);
		}
        
		kamiFontPrintfConsole(CONSOLE_ORANGE, "--------------------------\n");

//		DumpTadInfo();
    }

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( ImportProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         Import �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcess1(void)
{
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return ImportProcess2;
	}

	// �I�����j���[�̕ύX
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = NUM_OF_MENU_SELECT -1;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo >= NUM_OF_MENU_SELECT) sMenuSelectNo = 0;
	}

	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// ����
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return ImportProcess2;
	}
	// �g�b�v���j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return ImportProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Import �v���Z�X�Q

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcess2(void)
{
	switch( sMenuSelectNo )
	{
	case 0:
		return ImportAllOverwriteProcess0;
		break;
	case 1:
		return ImportAllNonexistentProcess0;
		break;
	case 2:
		FADE_OUT_RETURN( ImportIndividuallyProcess0 );
	case 3:
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return ImportProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         ImportProcessReturn1

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcessReturn1(void)
{
	while(1)
	{
		kamiPadRead();
    	if (kamiPadIsTrigger(PAD_BUTTON_B)) { break; }
	}

	FADE_OUT_RETURN( TopmenuProcess0 );
}

/*---------------------------------------------------------------------------*
     �S�t�@�C���C���|�[�g�v���Z�X�i�㏑���j
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         �S�t�@�C��Import �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

static void* ImportAllOverwriteProcess0(void)
{
	int i;
	BOOL result = TRUE;

	kamiFontPrintf(25,  5, FONT_COLOR_BLACK, "WAIT");

	for (i=0;i<sFileNum;i++)
	{
		// �����㏑��
		if (ImportTad(sTitleSortSet[i].path, TAD_WRITE_OPTION_OVERWRITE) == FALSE)
		{
			result = FALSE;
		}
	}

	if (result) { kamiFontPrintf(25,  5, FONT_COLOR_GREEN, " OK "); }
	else		{ kamiFontPrintf(25,  5, FONT_COLOR_RED,   " NG "); }

	// Auto�p
	if (gAutoFlag)
	{
		if (result && sFileNum > 0) 
		{
			gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_TAD] = AUTO_PROCESS_RESULT_SUCCESS;  
			FADE_OUT_RETURN( AutoProcess1 ); 
		}
		else 
		{ 
			gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_TAD] = AUTO_PROCESS_RESULT_FAILURE;  
			FADE_OUT_RETURN( AutoProcess2 ); 
		}
	}

	return ImportProcess1;
}

/*---------------------------------------------------------------------------*
    �㏑���C���|�[�g�v���Z�X�i�����t�@�C���͏㏑�����Ȃ��j
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         �d�����Ȃ��t�@�C��Import �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

static void* ImportAllNonexistentProcess0(void)
{
	int i;
	BOOL result = TRUE;

	kamiFontPrintf(25,  7, FONT_COLOR_BLACK, "WAIT");

	for (i=0;i<sFileNum;i++)
	{
		// �������݃`�������W
		if (ImportTad(sTitleSortSet[i].path, TAD_WRITE_OPTION_NONEXISTENT) == FALSE)
		{
			result = FALSE;
		}
	}

	if (result) { kamiFontPrintf(25,  7, FONT_COLOR_GREEN, " OK "); }
	else		{ kamiFontPrintf(25,  7, FONT_COLOR_RED,   " NG "); }

	return ImportProcess1;
}

/*---------------------------------------------------------------------------*
     �ʃC���|�[�g�v���Z�X
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ��Import �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/
static void* ImportIndividuallyProcess0(void)
{
	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import from SD Card ");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// TAD���X�g�\��
	ShowTadList();

	DumpTadInfo();

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( ImportIndividuallyProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         �� Import �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/
void* ImportIndividuallyProcess1(void)
{
	// �I�����j���[�̕ύX
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNoIndividually < 0) 
		{
			sMenuSelectNoIndividually = sFileNum - 1;
			if (sFileNum > VIEW_LINES_MAX)
			{
				sTadListViewOffset = sFileNum - VIEW_LINES_MAX;
			}
			else
			{
				sTadListViewOffset = 0;
			}
		}
		if (sMenuSelectNoIndividually < sTadListViewOffset)
		{
			sTadListViewOffset--;
		}

		DumpTadInfo();
		ShowTadList();
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNoIndividually > sFileNum - 1) 
		{
			sMenuSelectNoIndividually = 0;
			sTadListViewOffset = 0;
		}
		if ((sMenuSelectNoIndividually - sTadListViewOffset) > VIEW_LINES_MAX - 1)
		{
			sTadListViewOffset++;
		}

		DumpTadInfo();
		ShowTadList();
	}

	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + (sMenuSelectNoIndividually - sTadListViewOffset) * DOT_OF_MENU_SPACE_INDIVIDUALLY));

	// ����
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return ImportIndividuallyProcess2;
	}
	// �ЂƂO�̃��j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( ImportProcess0 );
	}

	return ImportIndividuallyProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         ��Import �v���Z�X�Q

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

void* ImportIndividuallyProcess2(void)
{
	BOOL ret;

	if (STD_GetStringLength(sTitleSortSet[sMenuSelectNoIndividually].path))
	{
		// �ʃC���|�[�g
		ret = ImportTad(sTitleSortSet[sMenuSelectNoIndividually].path, TAD_WRITE_OPTION_USER);
	}
	else
	{
		// ���^�[��
		return ImportProcess0;
	}

	// ����̌��ʂ�\��
	if ( ret == TRUE )
	{
		kamiFontPrintf(26, (s16)(5+sMenuSelectNoIndividually*CHAR_OF_MENU_SPACE_INDIVIDUALLY), FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf(26, (s16)(5+sMenuSelectNoIndividually*CHAR_OF_MENU_SPACE_INDIVIDUALLY), FONT_COLOR_RED, "NG");
	}

	return ImportIndividuallyProcess1;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ImportTad

  Description:  .tad �t�@�C���C���|�[�g

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL ImportTad(char* file_name, TadWriteOption option)
{
	NAMTadInfo tadInfo;
	NAMTitleInfo titleInfoTmp;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];
	OSThread thread;
	BOOL ret = FALSE;
	s32  nam_result;
	BOOL overwrite = FALSE;

	// �t���p�X���쐬
	MakeFullPathForSD(file_name, full_path);

	// tad�t�@�C���̏��擾
	if (NAM_ReadTadInfo(&tadInfo, full_path) != NAM_OK)
	{
		return FALSE;
	}

	// NAND�̏����擾
	if ( option != TAD_WRITE_OPTION_OVERWRITE && NAM_ReadTitleInfo(&titleInfoTmp, tadInfo.titleInfo.titleId) == NAM_OK )
	{
		// NAND�Ɋ��ɃC���X�g�[������Ă��邩�ǂ����m�F����
		if (tadInfo.titleInfo.titleId == titleInfoTmp.titleId)
		{
			overwrite = TRUE;

			switch (option)
			{
			case TAD_WRITE_OPTION_NONEXISTENT:
				return TRUE;
			case TAD_WRITE_OPTION_USER:
				ShowTitleinfoDifference(&titleInfoTmp, &tadInfo.titleInfo);

				kamiFontPrintfConsole(1, "The program has already existed.");
				kamiFontPrintfConsole(1, "Do you overwrite ?\n");
				kamiFontPrintfConsole(1, " <Yes: Push A>   <No: Push B>\n");

				// �t�H���g�X�N���[���f�[�^���[�h
				kamiFontLoadScreenData();

				while(1)
				{
					kamiPadRead();

				    if (kamiPadIsTrigger(PAD_BUTTON_A))
			    	{
						break;
					}
				    else if (kamiPadIsTrigger(PAD_BUTTON_B))
			    	{
						kamiFontPrintfConsole(CONSOLE_ORANGE, "Import was canceled.\n");
						return TRUE;
					}
				}
			}
		}
	}

	// NOT_LAUNCH_FLAG �܂��� DATA_ONLY_FLAG �������Ă��Ȃ��^�C�g���̏ꍇ
	// freeSoftBoxCount�ɋ󂫂��Ȃ���΃C���|�[�g���Ȃ�
	if (!(tadInfo.titleInfo.titleId & (TITLE_ID_NOT_LAUNCH_FLAG_MASK | TITLE_ID_DATA_ONLY_FLAG_MASK)))
	{
		// �㏑���C���|�[�g�̏ꍇ��freeSoftBoxCount�̓`�F�b�N���Ȃ�
		if (!overwrite)
		{
			u8 installed, free;
			if (!NAMUT_GetSoftBoxCount( &installed, &free ))
			{
				return FALSE;
			}

			if (free == 0)
			{
				kamiFontPrintfConsole(1, "NAND FreeSoftBoxCount == 0");
				return FALSE;
			}
		}
	}

	// ES�̎d�l�ŌÂ� e-ticket ������ƐV���� e-ticket ���g�����C���|�[�g���ł��Ȃ�
	// �b��Ή��Ƃ��ĊY���^�C�g�������S�폜���Ă���C���|�[�g����
	nam_result = NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
	if ( nam_result != NAM_OK )
	{
		kamiFontPrintfConsole(CONSOLE_RED, "Fail! RetCode=%x\n", nam_result);
		return FALSE;
	}

	// �C���|�[�g�J�n�t���O�𗧂Ă�
	sNowImport = TRUE;

    // �i���X���b�h�쐬
	spStack = OS_Alloc(THREAD_STACK_SIZE);
	MI_CpuClear8(spStack, THREAD_STACK_SIZE);
    OS_CreateThread(&thread, ProgressThread, NULL,
        (void*)((u32)spStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_GetCurrentThread()->priority - 1);
	// �f�X�g���N�^�Z�b�g
	OS_SetThreadDestructor( &thread, Destructor );
    OS_WakeupThreadDirect(&thread);

	// Import�J�n
	OS_Printf( "Import %s Start.\n", full_path );
	kamiFontPrintfConsole(CONSOLE_ORANGE, "Import %s Start.\n", file_name );

	nam_result = NAM_ImportTad( full_path );

	// �i���X���b�h�̎��͏I����҂�
	while (sNowImport){};

	if ( nam_result == NAM_OK )
	{
		kamiFontPrintfConsole(CONSOLE_ORANGE, "Success!\n");
		ret = TRUE;
	}
	else
	{
		kamiFontPrintfConsole(CONSOLE_RED, "Fail! RetCode=%d\n", nam_result);
	}

	// InstalledSoftBoxCount, FreeSoftBoxCount �̒l�����݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B
	NAMUT_UpdateSoftBoxCount();

	return ret;
}

static void Destructor(void* /*arg*/)
{
	OS_Free(spStack);
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

	ProgressInit();

    while (TRUE)
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
        OS_WaitVBlankIntr();
    }

	sNowImport = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ShowTadList

  Description:  .tad �̃��X�g��\������

  Arguments:    arg -   �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShowTadList(void)
{
	int i;

	// ���j���[�ꗗ
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	if (sFileNum > 15)  { sLines = VIEW_LINES_MAX; }
	else                { sLines = sFileNum; }
	for (i=0;i<sLines;i++)
	{
		kamiFontPrintf(3, (s16)(5+i), FONT_COLOR_BLACK, "l                    l    l");
	}
	kamiFontPrintf(3, (s16)(5+sLines), FONT_COLOR_BLACK, "+--------------------+----+");

	// tad �t�@�C�����X�g��\��
	for (i=0;i<sLines; i++)
	{
		// �t�@�C�����ǉ�
		kamiFontPrintf(3,  (s16)(5+CHAR_OF_MENU_SPACE_INDIVIDUALLY*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sTitleSortSet[sTadListViewOffset+i].path);
	}
}

/*---------------------------------------------------------------------------*
  Name:         DumpTadInfo

  Description:  .tad �t�@�C���̏���\������

  Arguments:    arg -   �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void DumpTadInfo(void)
{
	NAMTadInfo info;

	// �t�@�C�����̗L�����m�F
	if (STD_GetStringLength(sTitleSortSet[sMenuSelectNoIndividually].path))
	{
		char full_path[FS_ENTRY_LONGNAME_MAX+6];

		// �t���p�X���쐬
		MakeFullPathForSD( sTitleSortSet[sMenuSelectNoIndividually].path, full_path );

		// TAD�t�@�C���̏��擾
		if (NAM_ReadTadInfo(&info, full_path) == NAM_OK)
		{
			char temp[100];
			u16 companyCode = MI_SwapEndian16(info.titleInfo.companyCode);
			u32 gameCode = (u32)(info.titleInfo.titleId & 0xffffffff);
			gameCode = MI_SwapEndian32(gameCode);

			kamiFontPrintfConsole(CONSOLE_ORANGE, "------ tad profile -----\n");

			// File Name
			kamiFontPrintfConsole(CONSOLE_ORANGE, "%s\n", sTitleSortSet[sMenuSelectNoIndividually].path);

			// File Size
			kamiFontPrintfConsole(CONSOLE_ORANGE, "fileSize      = %d Byte\n", info.fileSize);

			// Company Code
			MI_CpuCopy8( &companyCode, temp, sizeof(companyCode) );
			temp[sizeof(companyCode)] = NULL;
			kamiFontPrintfConsole(CONSOLE_ORANGE, "Company Code  = %s\n", temp);

			// Game Code
			MI_CpuCopy8( &gameCode, temp, sizeof(gameCode) );
			temp[sizeof(gameCode)] = NULL;
			kamiFontPrintfConsole(CONSOLE_ORANGE, "GameCode Code = %s\n", temp);

			// Game Version
			kamiFontPrintfConsole(CONSOLE_ORANGE, "GameVersion   = %d\n", info.titleInfo.version);

			// Public Save Size
			kamiFontPrintfConsole(CONSOLE_ORANGE, "PublicSaveSize = %d\n", info.titleInfo.publicSaveSize);

			// Private Save Size
			kamiFontPrintfConsole(CONSOLE_ORANGE, "PrivateSaveSize = %d\n", info.titleInfo.privateSaveSize);

			kamiFontPrintfConsole(CONSOLE_ORANGE, "--------------------------\n");
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         ShowTitleinfoDifference

  Description:  ����NAND�v���O������SD�v���O�����̔�r����\������

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShowTitleinfoDifference( NAMTitleInfo* titleInfoNand, NAMTitleInfo* titleInfoSd)
{
	char tempOld[100];
	char tempNew[100];
	u16 companyCode;
	u32 gameCode;

	kamiFontPrintfConsole(CONSOLE_ORANGE, "+--------------+-------+-------+");
	kamiFontPrintfConsole(CONSOLE_ORANGE, "l              l NAND  l  SD   l");
	kamiFontPrintfConsole(CONSOLE_ORANGE, "+--------------+-------+-------+");
	
	// Company Code (nand)
	companyCode = MI_SwapEndian16(titleInfoNand->companyCode);
	MI_CpuCopy8( &companyCode, tempOld, sizeof(companyCode) );
	tempOld[sizeof(companyCode)] = NULL;

	// Company Code (sd)
	companyCode = MI_SwapEndian16(titleInfoSd->companyCode);
	MI_CpuCopy8( &companyCode, tempNew, sizeof(companyCode) );
	tempNew[sizeof(companyCode)] = NULL;
	kamiFontPrintfConsole(CONSOLE_ORANGE, "l Company Code l  %-2.2s   l  %-2.2s   l", tempOld, tempNew);

	// Game Code (nand)
	gameCode = (u32)(titleInfoNand->titleId & 0xffffffff);
	gameCode = MI_SwapEndian32(gameCode);
	MI_CpuCopy8( &gameCode, tempOld, sizeof(gameCode) );
	tempOld[sizeof(gameCode)] = NULL;

	// Game Code (sd)
	gameCode = (u32)(titleInfoSd->titleId & 0xffffffff);
	gameCode = MI_SwapEndian32(gameCode);
	MI_CpuCopy8( &gameCode, tempNew, sizeof(gameCode) );
	tempNew[sizeof(gameCode)] = NULL;
	kamiFontPrintfConsole(CONSOLE_ORANGE, "l Game Code    l  %-4.4s l  %-4.4s l", tempOld, tempNew);

	// Game Version
	kamiFontPrintfConsole(CONSOLE_ORANGE, "l Game Version l  %-4.4d l  %-4.4d l", titleInfoNand->version, titleInfoSd->version );
	kamiFontPrintfConsole(CONSOLE_ORANGE, "+--------------+-------+-------+");
}

/*---------------------------------------------------------------------------*
  Name:         ProgressInit

  Description:  �C���|�[�g�̐i����\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressInit(void)
{
	sCurrentProgress = 0;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressDraw

  Description:  �C���|�[�g�̐i����\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressDraw(f32 ratio)
{
	char square[2] = { 0x01, 0x00 };
	u32 temp;
	s32 i;

	temp = (u32)(32 * ratio);
	if (temp > sCurrentProgress)
	{
		s32 diff = (s32)(temp - sCurrentProgress);
		for (i=0;i<diff;i++)
		{
			kamiFontPrintfConsole(2, square);
		}
	}
	sCurrentProgress = temp;

	// �t�H���g�X�N���[���f�[�^���[�h
	kamiFontLoadScreenData();
}

