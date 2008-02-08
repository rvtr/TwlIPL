/*---------------------------------------------------------------------------*
  Project:  TwlIPL - NAMUT
  File:     namut.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

// Nand Application Management UTility ���C�u�����A������NAMUT���C�u�����ł��B
// ���C���[�I�ɂ�NAM�Əd�Ȃ�܂��̂�NAM�ɋz�����Ă��ǂ��Ǝv���܂��B

#include <twl.h>
#include <twl/fatfs.h>
#include <twl/os/common/format_rom.h>
#include <twl/nam.h>
#include <sysmenu/namut.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

// �N���[���A�b�v���s����
// �w��f�B���N�g���ȉ��͑S�ď�������܂��B
// �w��f�B���N�g�����͎̂c��܂�
static const char* sDeleteDirectoryList[] =
{
	"nand:/tmp"
};

// �N���[���A�b�v���s����
// �w��t�@�C����0xff��Fill����܂��B
static const char* sFillFileList[] =
{
	"nand:/shared1/TWLCFG0.dat",
	"nand:/shared1/TWLCFG1.dat"
};

// �N���[���A�b�v�ŏ������Ȃ��^�C�g����
// TitleProperty (TitleID Hi�̉���16bit�j�̃r�b�g�Ŏw�肵�܂��B
// �ǂꂩ1�ł��r�b�g�������Ă���Ώ����̑Ώۂ���O���܂��B
#define PROTECT_TITLE_PROPERTY  (TITLE_ID_HI_APP_TYPE_MASK)

#define DIRECTORY_DEPTH_MAX      16  // �f�B���N�g���̐[���̍ő�iNAND�̐��K�\���Ƃ��Ă͍ő�6�j
#define TITLE_PATH              "nand:/title"
#define TICKET_PATH             "nand:/ticket"
#define IMPORT_PATH             "nand:/import"
#define TITLE_ID_HI_SIZE          8
#define TITLE_LIST_MAX          256
#define CLEAR_DATA_SIZE         256

static const u8 sClearData[CLEAR_DATA_SIZE] = 
{ 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static FSDirectoryEntryInfo sEntryInfo;
static NAMTitleId sTitleIdArray[TITLE_LIST_MAX];

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL NAMUTi_DeleteNonprotectedTitle(void);
static BOOL NAMUTi_DeleteNonprotectedTitleEntity(const char* path);
static BOOL NAMUTi_ClearSavedataAll(BOOL fill);
static BOOL NAMUTi_ClearSavedata(const char* path, BOOL fill);
static BOOL NAMUTi_DeleteNandDirectory(const char *path);
static BOOL NAMUTi_FillFile(const char* path);
static void NAMUTi_DrawNandTree(s32 depth, const char *path);
static void PrintDirectory(s32 depth, const char* path);
static void PrintFile(s32 depth, const char* path);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Format

  Description:  NAND�̃N���[���A�b�v
               �i�V�X�e���n�̕K�v�ȃt�@�C���݂̂��c�������������܂��j

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format(void)
{
	int i;
	BOOL ret = TRUE;

	// �v���e�N�g����Ă��Ȃ��^�C�g���̍폜���s���܂�
	ret &= NAMUTi_DeleteNonprotectedTitle();

	// �v���e�N�g�^�C�g���̃Z�[�u�f�[�^���t�H�[�}�b�g���܂�
	ret &= NAMUTi_ClearSavedataAll(TRUE);

	// �w��t�@�C����0xff�ŃN���A���܂�
	for (i=0; i<sizeof(sFillFileList)/sizeof(char*); i++)
	{
		ret &= NAMUTi_FillFile(sFillFileList[i]);
	}

	// temp�ȉ����������܂�
	// NAM�֐���temp���쐬&�g�p�����\�������邽�ߍŌ�Ɏ��s���܂�
	for (i=0; i<sizeof(sDeleteDirectoryList)/sizeof(char*); i++)
	{
		ret &= NAMUTi_DeleteNandDirectory(sDeleteDirectoryList[i]);
	}

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteNandDirectory

  Description:  �w��f�B���N�g���ȉ����������܂��B
                �w��f�B���N�g�����͎̂c��܂��B

  Arguments:    path : ��΃p�X

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_DeleteNandDirectory(const char *path)
{
    FSFile  dir;
	FSDirectoryEntryInfo entryInfo;
	BOOL ret = TRUE;

	FS_InitFile(&dir);

	// �����Ŏw�肳�ꂽ�f�B���N�g�����J��
	if (!FS_OpenDirectory(&dir, path, FS_FILEMODE_R))
	{
		SDK_ASSERTMSG(0, "Fail! FS_OpenDirectory(%s) in %s\n", path, __FUNC__);
		return FALSE;
	}

	// �J�����g�f�B���N�g����ݒ肷��
	if (!FS_SetCurrentDirectory(path))
	{
		SDK_ASSERTMSG(0, "Fail! FS_SetCurrentDirectory(%s) in %s\n", path, __FUNC__);
		FS_CloseDirectory(&dir);
		return FALSE;
	}

	// �f�B���N�g���̒��g��ǂ�
	while (FS_ReadDirectory(&dir, &entryInfo))
	{
        if (STD_CompareString(entryInfo.longname, ".")  == 0 ||
            STD_CompareString(entryInfo.longname, "..") == 0)
        {
            continue;
        }

		// �f�B���N�g��
		if (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
		{
			ret &= FS_DeleteDirectoryAuto(entryInfo.longname);
			SDK_ASSERTMSG(ret, "Fail! FS_DeleteDirectoryAuto(%s) in %s\n", entryInfo.longname, __FUNC__);
		}
		// �t�@�C��
		else
		{
			ret &= FS_DeleteFileAuto(entryInfo.longname);
			SDK_ASSERTMSG(ret, "Fail! FS_DeleteFileAuto(%s) in %s\n", entryInfo.longname, __FUNC__);
		}
	}

	// �J�����g�f�B���N�g�������ɖ߂��܂�
	ret &= FS_SetCurrentDirectory("..");
	SDK_ASSERTMSG(ret, "Fail! FS_SetCurrentDirectory(..) in %s\n", __FUNC__);

	// �f�B���N�g�������
	FS_CloseDirectory(&dir);

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteNonprotectedTitle

  Description:  User App �^�C�g���̍폜���s���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_DeleteNonprotectedTitle(void)
{
	BOOL ret = TRUE;

	ret &= NAMUTi_DeleteNonprotectedTitleEntity(TITLE_PATH);
	ret &= NAMUTi_DeleteNonprotectedTitleEntity(TICKET_PATH);
	ret &= NAMUTi_DeleteNonprotectedTitleEntity(IMPORT_PATH);

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteNonprotectedTitleEntity

  Description:  User App �^�C�g���̍폜���s���܂��B

  Arguments:    path

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_DeleteNonprotectedTitleEntity(const char* path)
{
    FSFile  dir;
	FSDirectoryEntryInfo entryInfo;
	BOOL ret = TRUE;

	FS_InitFile(&dir);

	// �����Ŏw�肳�ꂽ�f�B���N�g�����J��
	if (!FS_OpenDirectory(&dir, path, FS_FILEMODE_R))
	{
		SDK_ASSERTMSG(0, "Fail! FS_OpenDirectory(%s) in %s\n", path, __FUNC__);
		return FALSE;
	}

	// �J�����g�f�B���N�g����ݒ肷��
	if (!FS_SetCurrentDirectory(path))
	{
		SDK_ASSERTMSG(0, "Fail! FS_SetCurrentDirectory(%s) in %s\n", path, __FUNC__);
		FS_CloseDirectory(&dir);
		return FALSE;
	}

	// �f�B���N�g���̒��g��ǂ�
	while (FS_ReadDirectory(&dir, &entryInfo))
	{
        if (STD_CompareString(entryInfo.longname, ".")  == 0 ||
            STD_CompareString(entryInfo.longname, "..") == 0)
        {
            continue;
        }

		// �f�B���N�g���̏ꍇ
		if (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
		{
			s8 titlePropety = (s8)(entryInfo.longname[TITLE_ID_HI_SIZE-1] - '0');

			// �v���e�N�g�ΏۂłȂ��ꍇ�f�B���N�g�����Ə�������
			if (!(titlePropety & PROTECT_TITLE_PROPERTY))
			{
				ret &= FS_DeleteDirectoryAuto(entryInfo.longname);
				SDK_ASSERTMSG(ret, "Fail! FS_DeleteDirectoryAuto(%s) in %s\n", entryInfo.longname, __FUNC__);
			}
		}
	}

	// �J�����g�f�B���N�g�������ɖ߂��܂�
	ret &= FS_SetCurrentDirectory("..");
	SDK_ASSERTMSG(ret, "Fail! FS_SetCurrentDirectory(..) in %s\n", __FUNC__);

	// �f�B���N�g�������
	ret &= FS_CloseDirectory(&dir);
	SDK_ASSERTMSG(ret, "Fail! FS_CloseDirectory() in %s\n", __FUNC__);

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataAll

  Description:  �S�Z�[�u�f�[�^�̂e�e�N���A���t�H�[�}�b�g���s���܂�

  Arguments:    fill : FF�N���A���s���Ȃ�TRUE���w�肵�܂�

  Returns:      None
 *---------------------------------------------------------------------------*/

static BOOL NAMUTi_ClearSavedataAll(BOOL fill)
{
	s32 title_num;	
	NAMTitleInfo namTitleInfo;
	char savePublicPath[ FS_ENTRY_LONGNAME_MAX ];
	char savePrivatePath[ FS_ENTRY_LONGNAME_MAX ];
	BOOL ret = TRUE;
	s32 i;

	// �^�C�g�����X�g�擾
	if (NAM_GetTitleList(sTitleIdArray, TITLE_LIST_MAX) != NAM_OK)
	{
		return FALSE;
	}
	
	// �^�C�g�����擾
	title_num = NAM_GetNumTitles();

	for (i=0;i<title_num;i++)
	{
		// �^�C�g�����擾
	    if( NAM_ReadTitleInfo(&namTitleInfo, sTitleIdArray[i]) == NAM_OK )
	    {
			// �Z�[�u�t�@�C���p�X�擾
			if (NAM_GetTitleSaveFilePath(savePublicPath, savePrivatePath, sTitleIdArray[i]) == NAM_OK)
			{
				// publicSaveSize��0�ȏ�Ȃ�0xFF�N���A���t�H�[�}�b�g
				if (namTitleInfo.publicSaveSize > 0)
				{
					ret &= NAMUTi_ClearSavedata(savePublicPath, fill);
				}
				// privateSaveSize��0�ȏ�Ȃ�0xFF�N���A���t�H�[�}�b�g
				if (namTitleInfo.privateSaveSize > 0)
				{
					ret &= NAMUTi_ClearSavedata(savePrivatePath, fill);
				}
			}
			else { ret = FALSE; }
		}
		else { ret = FALSE; }
	}

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedata

  Description:  �w�肵���Z�[�u�f�[�^�t�@�C���ɑ΂���
				�e�e�N���A���t�H�[�}�b�g���s���܂��B

  Arguments:    fill : FF�N���A���s���Ȃ�TRUE���w�肵�܂�

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_ClearSavedata(const char* path, BOOL fill)
{
	char drive[2] = { 'Z', '\0' };
	FSFile file;
	BOOL ret = FALSE;

	// �t�@�C���\���̏�����
    FS_InitFile(&file);

	// �Z�[�u�t�@�C���I�[�v��
	if (!FS_OpenFileEx(&file, path, (FS_FILEMODE_R|FS_FILEMODE_W)))
	{
		return FALSE;
	}

	// �Z�[�u�t�@�C����0xFF�ŃN���A
	if (fill)
	{
		u32 filesize = FS_GetFileLength(&file);
		for (; filesize > CLEAR_DATA_SIZE; filesize -= CLEAR_DATA_SIZE)
		{
			FS_WriteFile(&file, sClearData, CLEAR_DATA_SIZE);
		}
		FS_WriteFile(&file, sClearData, (s32)filesize);
	}

	// �Z�[�u�t�@�C���ɑ΂��ă}�E���g
	for (drive[0]='Z'; drive[0]>='A'; drive[0]--)
	{
		if (FATFS_MountDrive(drive, FATFS_MEDIA_TYPE_SUBNAND, (FATFSFileHandle)file.userdata))
		{
			break;
		}
	}
	
	// �}�E���g�����Ȃ�t�H�[�}�b�g����
	if (drive[0] >= 'A')
	{
		char drive_colon[3];
		drive_colon[0] = drive[0];
		drive_colon[1] = ':';
		drive_colon[2] = '\0';

		// ���f�B�A�t�H�[�}�b�g
		if (FATFSi_FormatMedia(drive_colon))
		{
			// �h���C�u�t�H�[�}�b�g
			if (FATFS_FormatDrive(drive_colon))
			{
				ret = TRUE;
			}
		}

		// �A���}�E���g
		FATFS_UnmountDrive(drive);
	}

	// �t�@�C���N���[�Y
	FS_CloseFile(&file);

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_FillFile

  Description:  �w�肵���t�@�C����0xFF�Ŗ��߂܂��B

  Arguments:    path

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_FillFile(const char* path)
{
	FSFile file;

	// �t�@�C���\���̏�����
    FS_InitFile(&file);

	// �t�@�C���I�[�v��
	if (FS_OpenFileEx(&file, path, (FS_FILEMODE_R|FS_FILEMODE_W)))
	{
		// �t�@�C����0xFF�ŃN���A
		u32 filesize = FS_GetFileLength(&file);
		for (; filesize > CLEAR_DATA_SIZE; filesize -= CLEAR_DATA_SIZE)
		{
			FS_WriteFile(&file, sClearData, CLEAR_DATA_SIZE);
		}
		FS_WriteFile(&file, sClearData, (s32)filesize);
		FS_CloseFile(&file);
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DrawNandTree

  Description:  NAND�̃c���[�����v�����g�o�͂��܂�

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/

void NAMUT_DrawNandTree(void)
{
	NAMUTi_DrawNandTree(0, "nand:/");
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DrawNandTree

  Description:  �w��p�X�̃c���[�����v�����g�o�͂��܂��i�ċA�֐��j

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void NAMUTi_DrawNandTree(s32 depth, const char *path)
{
    FSFile  dir;
	
	PrintDirectory(depth, path);

	// �[������
	if (depth > DIRECTORY_DEPTH_MAX)
	{
		OS_TPrintf("Fail! Depth is too deep.\n");
		return;
	}

	FS_InitFile(&dir);

	// �����Ŏw�肳�ꂽ�f�B���N�g�����J��
	if (!FS_OpenDirectory(&dir, path, (FS_FILEMODE_R|FS_FILEMODE_W)))
	{
		OS_TPrintf("%d Fail! FS_OpenDirectory(%s)\n", __LINE__, path);
		return;
	}

	// �J�����g�f�B���N�g����ݒ肷��
	if (!FS_SetCurrentDirectory(path))
	{
		OS_TPrintf("Fail! FS_SetCurrentDirectory(%s)\n", path);
		FS_CloseDirectory(&dir);
		return;
	}

	// �f�B���N�g���̒��g��ǂ�
	while (FS_ReadDirectory(&dir, &sEntryInfo))
	{
        if (STD_CompareString(sEntryInfo.longname, ".")  == 0 ||
            STD_CompareString(sEntryInfo.longname, "..") == 0)
        {
            continue;
        }

		// �f�B���N�g���ł���΍ċA�Ăяo��
		if (!(sEntryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY))
		{
//			PrintFile(depth, sEntryInfo.longname);
		}
		else
		{
			NAMUTi_DrawNandTree(depth + 1, sEntryInfo.longname);
		}
	}

	// �J�����g�f�B���N�g�������ɖ߂��܂�
	if (!FS_SetCurrentDirectory(".."))
	{
		OS_TPrintf("Fail! FS_SetCurrentDirectory(..)\n");
	}	

	FS_CloseDirectory(&dir);
}

/*---------------------------------------------------------------------------*
  Name:         PrintDirectory

  Description:  �c���[�����v�����g�o�͂��܂��i�f�B���N�g���p�j

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintDirectory(s32 depth, const char* path)
{
	int i;

	for (i=0; i<depth; i++)
	{
		OS_TPrintf("   ");
	}
	OS_TPrintf("+");
	OS_TPrintf("%s/\n", path);
}

/*---------------------------------------------------------------------------*
  Name:         PrintFile

  Description:  �c���[�����v�����g�o�͂��܂��i�t�@�C���p�j

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintFile(s32 depth, const char* filename)
{
	int i;

	for (i=0; i<depth+1; i++)
	{
		OS_TPrintf("   ");
	}

	OS_TPrintf("l-- ");

	OS_TPrintf("%s\n", filename);
}
