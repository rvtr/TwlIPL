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

#include <twl.h>
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_import.h"
#include "process_eticket.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"

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
#define FILE_NUM_MAX         16

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s32 sMenuSelectNo;
static s32 sMenuSelectNoIndividually;

static LCFGReadResult (*s_pReadSecureInfoFunc)( void );

static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];

static u8 sFileNum;

static void* spStack;

static u32  sCurrentProgress;

static vu8 sNowImport = FALSE;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL ImportTad(char* file_name, TadWriteOption option);
static void ProgressThread(void* arg);
static void Destructor(void* arg);
static void DumpTadInfo(void);
static void MakeFullPathForSD(char* file_name, char* full_path);
static void ShowTitleinfoDifference( NAMTitleInfo* titleInfoNand, NAMTitleInfo* titleInfoSd);
void ProgessInit(void);
void ProgressDraw(f32 ratio);
static void* ImportProcessReturn0(void);
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
    FATFSFileHandle fat_handle;
    FSFile    dir;
	int i;

	// F:sys/cert.sys�����݂��Ȃ��Ȃ�o�����Ă��炤
    fat_handle = FATFS_OpenFile(E_TICKET_FILE_PATH_IN_NAND, "r");
    if (!fat_handle)
    {
		FATFS_CloseFile(fat_handle);
		return ImportProcessReturn0;
    }
	FATFS_CloseFile(fat_handle);

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

	// �t�@�C����������
	sFileNum = 0;

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 1, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 2, BG_COLOR_PINK, BG_COLOR_TRANS );

    // SD�J�[�h�̃��[�g�f�B���N�g��������
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
        kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory(sdmc:/)\n");
    }
    else
    {
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
					if (!STD_CompareString( pExtension, ".tad") )
					{
						char full_path[FS_ENTRY_LONGNAME_MAX+6];

						// �t���p�X���쐬
						MakeFullPathForSD(info->longname, full_path);

						STD_CopyString( sFilePath[sFileNum], info->longname );
						kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);

						// �ő�16�ŏI��
						if (++sFileNum >= FILE_NUM_MAX)
						{
							break;
						}
					}
				}
            }
        }
        (void)FS_CloseDirectory(&dir);

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
  Name:         ImportProcessReturn0

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcessReturn0(void)
{
	int i;

	// ������S�N���A
	kamiFontClear();
	kamiFontPrintf(2,  10, FONT_COLOR_RED, "%s is not exist", E_TICKET_FILE_PATH_IN_NAND);
	kamiFontPrintf(2,  11, FONT_COLOR_RED, "You should write e-ticket", E_TICKET_FILE_PATH_IN_NAND);
	kamiFontPrintf(2,  12, FONT_COLOR_RED, "beforehand.", E_TICKET_FILE_PATH_IN_NAND);
	kamiFontPrintf(2,  22, FONT_COLOR_BLACK, "B Button : return to menu");

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import TAD from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 1, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 2, BG_COLOR_PINK, BG_COLOR_TRANS );

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	// �t�H���g�X�N���[���f�[�^���[�h
	kamiFontLoadScreenData();

	FADE_IN_RETURN( ImportProcessReturn1 );
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
		if (ImportTad(sFilePath[i], TAD_WRITE_OPTION_OVERWRITE) == FALSE)
		{
			result = FALSE;
		}
	}

	if (result) { kamiFontPrintf(25,  5, FONT_COLOR_GREEN, " OK "); }
	else		{ kamiFontPrintf(25,  5, FONT_COLOR_RED,   " NG "); }

	// Auto�p
	if (gAutoFlag)
	{
		if (result && sFileNum > 0) { FADE_OUT_RETURN( AutoProcess1 ); }
		else { FADE_OUT_RETURN( AutoProcess2 ); }
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
		if (ImportTad(sFilePath[i], TAD_WRITE_OPTION_NONEXISTENT) == FALSE)
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
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import from SD Card ");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	for (i=0;i<sFileNum+1;i++)
	{
		kamiFontPrintf(3, (s16)(5+i), FONT_COLOR_BLACK, "l                    l    l");
	}
	kamiFontPrintf(3, (s16)(5+i), FONT_COLOR_BLACK, "+--------------------+----+");

	// tad �t�@�C�����X�g��\��
	for (i=0;i<sFileNum; i++)
	{
		// �t�@�C�����ǉ�
		kamiFontPrintf(3,  (s16)(5+CHAR_OF_MENU_SPACE_INDIVIDUALLY*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[i]);
	}

	// �Ō�Ƀ��^�[����ǉ�
	kamiFontPrintf(3, (s16)(5+CHAR_OF_MENU_SPACE_INDIVIDUALLY*sFileNum), FONT_COLOR_BLACK, "l   RETURN           l    l");

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
		if (--sMenuSelectNoIndividually < 0) sMenuSelectNoIndividually = sFileNum;
		DumpTadInfo();
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNoIndividually > sFileNum) sMenuSelectNoIndividually = 0;
		DumpTadInfo();
	}

	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNoIndividually * DOT_OF_MENU_SPACE_INDIVIDUALLY));

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

	if (STD_GetStringLength(sFilePath[sMenuSelectNoIndividually]))
	{
		// �ʃC���|�[�g
		ret = ImportTad(sFilePath[sMenuSelectNoIndividually], TAD_WRITE_OPTION_USER);
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
	NAMTitleInfo titleInfo;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];
	OSThread thread;
	BOOL ret = FALSE;
	s32  nam_result;

	// �t���p�X���쐬
	MakeFullPathForSD(file_name, full_path);

	// tad�t�@�C���̏��擾
	if (NAM_ReadTadInfo(&tadInfo, full_path) != NAM_OK)
	{
		return FALSE;
	}

	// NAND�̏����擾
	if ( option != TAD_WRITE_OPTION_OVERWRITE && NAM_ReadTitleInfo(&titleInfo, tadInfo.titleInfo.titleId) == NAM_OK)
	{
		// NAND�Ɋ��ɃC���X�g�[������Ă��邩�ǂ����m�F����
		if (tadInfo.titleInfo.titleId == titleInfo.titleId)
		{
			switch (option)
			{
			case TAD_WRITE_OPTION_NONEXISTENT:
				return TRUE;
			case TAD_WRITE_OPTION_USER:
				ShowTitleinfoDifference(&titleInfo, &tadInfo.titleInfo);

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

	// �C���|�[�g�J�n�t���O�𗧂Ă�
	sNowImport = TRUE;

    // �i���X���b�h�쐬
	spStack = OS_Alloc(THREAD_STACK_SIZE);
	MI_CpuClear8(spStack, THREAD_STACK_SIZE);
    OS_CreateThread(&thread, ProgressThread, NULL,
        (void*)((u32)spStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_THREAD_PRIORITY_MAX);
	// �f�X�g���N�^�Z�b�g
	OS_SetThreadDestructor( &thread, Destructor );
    OS_WakeupThreadDirect(&thread);

	// Import�J�n
	OS_Printf( "Import %s Start.\n", full_path );
	kamiFontPrintfConsole(CONSOLE_ORANGE, "Import %s Start.\n", file_name );

	nam_result = NAM_ImportTad( full_path );

	// �i���X���b�h�̎��͏I����҂�
	while (sNowImport){ OS_Sleep(1); };

	if ( nam_result == NAM_OK )
	{
		kamiFontPrintfConsole(CONSOLE_ORANGE, "Import %s Sucess.\n", file_name );
		ret = TRUE;
	}
	else
	{
		kamiFontPrintfConsole(1, "Import %s Fail.\n", file_name );
	}

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
    }

	sNowImport = FALSE;
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
	if (STD_GetStringLength(sFilePath[sMenuSelectNoIndividually]))
	{
		char full_path[FS_ENTRY_LONGNAME_MAX+6];

		// �t���p�X���쐬
		MakeFullPathForSD( sFilePath[sMenuSelectNoIndividually], full_path );

		// TAD�t�@�C���̏��擾
		if (NAM_ReadTadInfo(&info, full_path) == NAM_OK)
		{
			char temp[100];
			u16 companyCode = MI_SwapEndian16(info.titleInfo.companyCode);
			u32 gameCode = (u32)(info.titleInfo.titleId & 0xffffffff);
			gameCode = MI_SwapEndian32(gameCode);

			kamiFontPrintfConsole(CONSOLE_ORANGE, "------ tad profile -----\n");

			// File Name
			kamiFontPrintfConsole(CONSOLE_ORANGE, "%s\n", sFilePath[sMenuSelectNoIndividually]);

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

			kamiFontPrintfConsole(CONSOLE_ORANGE, "--------------------------\n");
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         MakeFullPathForSD

  Description:  

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MakeFullPathForSD(char* file_name, char* full_path)
{
	// �t���p�X���쐬
	STD_CopyString( full_path, "sdmc:/" );
	STD_ConcatenateString( full_path, file_name );
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

