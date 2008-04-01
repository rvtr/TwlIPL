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
  $Rev$
  $Author$
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

// �[���t�H�[�}�b�g�ŏ������Ȃ��^�C�g����
// TitleProperty (TitleID Hi�̉���16bit�j�̃r�b�g�Ŏw�肵�܂��B
// �ǂꂩ1�ł��r�b�g�������Ă���Ώ����̑Ώۂ���O���܂��B
#define PROTECT_TITLE_PROPERTY  (TITLE_ID_HI_APP_TYPE_MASK)

#define DIRECTORY_DEPTH_MAX      16  // �f�B���N�g���̐[���̍ő�iNAND�̐��K�\���Ƃ��Ă͍ő�6�j
#define TITLE_ID_HI_SIZE          8
#define TITLE_LIST_MAX          256
#define CLEAR_DATA_SIZE         256

// �[���t�H�[�}�b�g���s����
// �w��f�B���N�g���ȉ��͑S�ď�������܂��B
// �w��f�B���N�g�����͎̂c��܂��B
static const char* sDeleteDirectoryList[] =
{
	"nand:/tmp"
};

// �[���t�H�[�}�b�g���s����
// �w��f�B���N�g���ȉ���NonProtected�ȃ^�C�g���͑S�ď�������܂��B
// �w��f�B���N�g�����͎̂c��܂��B
static const char* sDeleteNonProtecedDirectoryList[] =
{
	"nand:/title",
	"nand:/ticket",
	"nand:/import"
};

// �[���t�H�[�}�b�g���s����
// �w��t�@�C����0xff��Fill����܂��B
static const char* sFillFileList[] =
{
	"nand:/shared1/TWLCFG0.dat",
	"nand:/shared1/TWLCFG1.dat"
};

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
static char sCurrentFullPath[FS_ENTRY_LONGNAME_MAX];

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL NAMUTi_DeleteNonprotectedTitle(void);
static BOOL NAMUTi_DeleteNonprotectedTitleEntity(const char* path);
static BOOL NAMUTi_ClearSavedataAll(void);
static BOOL NAMUTi_ClearSavedata(const char* path, u64 titleID, BOOL private);
static BOOL NAMUTi_DeleteNandDirectory(const char *path);
static BOOL NAMUTi_FillFile(const char* path);
static BOOL NAMUTi_MountAndFormatOtherTitleSaveData(u64 titleID, const char *arcname);
static void NAMUTi_DrawNandTree(s32 depth, const char *path);
static void PrintDirectory(s32 depth, const char* path);
static void PrintFile(s32 depth, const char* path);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Format

  Description:  NAND�̋[���t�H�[�}�b�g
               �i�V�X�e���n�̕K�v�ȃt�@�C���݂̂��c�������������܂��j

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format(void)
{
	int i;
	BOOL ret = TRUE;

	// �v���e�N�g����Ă��Ȃ��^�C�g���̍폜���s���܂�
	if (!NAMUTi_DeleteNonprotectedTitle())
	{
		ret = FALSE;
		OS_TWarning("Fail! NAMUTi_DeleteNonprotectedTitle()\n");
	}

	// �v���e�N�g�^�C�g���̃Z�[�u�f�[�^���t�H�[�}�b�g���܂�
	if (!NAMUTi_ClearSavedataAll())
	{
		ret = FALSE;
		OS_TWarning("Fail! NAMUTi_ClearSavedataAll()\n");
	}

	// �w��t�@�C����0xff�ŃN���A���܂�
	for (i=0; i<sizeof(sFillFileList)/sizeof(sFillFileList[0]); i++)
	{
		if (!NAMUTi_FillFile(sFillFileList[i]))
		{
			ret = FALSE;
			OS_TWarning("Fail! NAMUTi_FillFile(%s)\n", sFillFileList[i]);
		}
	}

	// temp�ȉ����������܂�
	// NAM�֐���temp���쐬&�g�p�����\�������邽�ߍŌ�Ɏ��s���܂�
	for (i=0; i<sizeof(sDeleteDirectoryList)/sizeof(char*); i++)
	{
		if (!NAMUTi_DeleteNandDirectory(sDeleteDirectoryList[i]))
		{
			ret = FALSE;
			OS_TWarning("NAMUTi_DeleteNandDirectory(%s)\n", sDeleteDirectoryList[i]);
		}
	}

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteNandDirectory

  Description:  �w��f�B���N�g���ȉ����������܂��B
                �w��f�B���N�g�����͎̂c��܂��B

  Arguments:    path : ��΃p�X�i�X���b�V�����܂߂Ȃ��j

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
		OS_TWarning("Fail! FS_OpenDirectory(%s) in %s\n", path, __func__);
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

		STD_CopyLString( sCurrentFullPath, path, FS_ENTRY_LONGNAME_MAX );
		STD_ConcatenateLString(sCurrentFullPath, "/", FS_ENTRY_LONGNAME_MAX);
		STD_ConcatenateLString(sCurrentFullPath, entryInfo.longname, FS_ENTRY_LONGNAME_MAX);

		// �f�B���N�g��
		if (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
		{
			if (!FS_DeleteDirectoryAuto(sCurrentFullPath))
			{
				ret = FALSE;
				OS_TWarning("Fail! FS_DeleteDirectoryAuto(%s) in %s\n", sCurrentFullPath, __func__);
			}
		}
		// �t�@�C��
		else
		{
			if (!FS_DeleteFileAuto(sCurrentFullPath))
			{
				ret = FALSE;
				OS_TWarning("Fail! FS_DeleteFileAuto(%s) in %s\n", sCurrentFullPath, __func__);
			}
		}
	}

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
	int i;

	for (i=0; i<sizeof(sDeleteNonProtecedDirectoryList)/sizeof(sDeleteNonProtecedDirectoryList[0]); i++)
	{
		ret &= NAMUTi_DeleteNonprotectedTitleEntity(sDeleteNonProtecedDirectoryList[i]);
	}

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteNonprotectedTitleEntity

  Description:  User App �^�C�g���̍폜���s���܂��B

  Arguments:    path : ��΃p�X�i�X���b�V�����܂܂Ȃ��j

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
		OS_TWarning("Fail! FS_OpenDirectory(%s) in %s\n", path, __func__);
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
				STD_CopyLString( sCurrentFullPath, path, FS_ENTRY_LONGNAME_MAX );
				STD_ConcatenateLString(sCurrentFullPath, "/", FS_ENTRY_LONGNAME_MAX);
				STD_ConcatenateLString(sCurrentFullPath, entryInfo.longname, FS_ENTRY_LONGNAME_MAX);

				if (!FS_DeleteDirectoryAuto(sCurrentFullPath))
				{
					ret = FALSE;
					OS_TWarning("Fail! FS_DeleteDirectoryAuto(%s) in %s\n", sCurrentFullPath, __func__);
				}
			}
		}
	}

	// �f�B���N�g�������
	if (!FS_CloseDirectory(&dir))
	{
		ret = FALSE;
		OS_TWarning("Fail! FS_CloseDirectory() in %s\n", __func__);
	}

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataAll

  Description:  �S�Z�[�u�f�[�^�̂e�e�N���A���t�H�[�}�b�g���s���܂�

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/

static BOOL NAMUTi_ClearSavedataAll( void )
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
					ret &= NAMUTi_ClearSavedata(savePublicPath, namTitleInfo.titleId, FALSE);
				}
				// privateSaveSize��0�ȏ�Ȃ�0xFF�N���A���t�H�[�}�b�g
				if (namTitleInfo.privateSaveSize > 0)
				{
					ret &= NAMUTi_ClearSavedata(savePrivatePath, namTitleInfo.titleId, TRUE);
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

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_ClearSavedata(const char* path, u64 titleID, BOOL private)
{
	FSFile file;
	u32 filesize;
	BOOL ret;

	// �t�@�C���\���̏�����
    FS_InitFile(&file);

	// �Z�[�u�t�@�C���I�[�v��
	if (!FS_OpenFileEx(&file, path, (FS_FILEMODE_R|FS_FILEMODE_W)))
	{
		return FALSE;
	}

	//----- �Z�[�u�t�@�C����0xFF�ŃN���A
	filesize = FS_GetFileLength(&file);
	for (; filesize > CLEAR_DATA_SIZE; filesize -= CLEAR_DATA_SIZE)
	{
		FS_WriteFile(&file, sClearData, CLEAR_DATA_SIZE);
	}
	FS_WriteFile(&file, sClearData, (s32)filesize);

	// �t�@�C���N���[�Y
	FS_CloseFile(&file);

	//----- NAND�A�v���̃Z�[�u�f�[�^�t�@�C�����}�E���g���t�H�[�}�b�g

	// private
	if (private)
	{
		ret = NAMUTi_MountAndFormatOtherTitleSaveData(titleID, "otherPrv");
	}
	// public
	else
	{
		ret = NAMUTi_MountAndFormatOtherTitleSaveData(titleID, "otherPub");
	}

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_MountAndFormatOtherTitleSaveData

  Description:  �w��NAND�A�v���̃Z�[�u�f�[�^�t�@�C�����}�E���g���t�H�[�}�b�g�B

  Arguments:    titleID : �^�C�g��ID
                arcname : "otherPub"�A"otherPriv"�A�܂���NULL

  Returns:      ���������TRUE
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_MountAndFormatOtherTitleSaveData(u64 titleID, const char *arcname)
{
    BOOL    succeeded = FALSE;
    // �}�E���g���s�B
    FSResult    result = FS_MountOtherTitleArchive(titleID, arcname);
    if (result != FS_RESULT_SUCCESS)
    {
        OS_TWarning("FS_MountOtherTitleArchive failed. (%d)\n", result);
    }
    else
    {
        // ����������t�H�[�}�b�g���s�B
        char    path[FS_ENTRY_LONGNAME_MAX];
        (void)STD_TSPrintf(path, "%s:/", arcname);
        if (!FATFSi_FormatMedia(path))
        {
            OS_TWarning("FATFSi_FormatMedia failed. (%d)\n", FATFS_GetLastError());
        }
        else if (!FATFS_FormatDrive(path))
        {
            OS_TWarning("FATFS_FormatDrive failed. (%d)\n", FATFS_GetLastError());
        }
        else
        {
            succeeded = TRUE;
        }
        // �h���C�u�����_���v�B
//      DumpArchiveResource(path);
        // �A���}�E���g�B
        (void)FS_MountOtherTitleArchive(titleID, NULL);
    }
    return succeeded;
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
		OS_TWarning("Fail! FS_OpenFileEx(%s)\n", path);
		return FALSE;
	}
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_SearchInstalledSoftBoxCount

  Description:  InstalledSoftBoxCount�̐��𒲂ׂĕԂ��܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
u32 NAMUT_SearchInstalledSoftBoxCount( void )
{
	s32 title_num;	
	NAMTitleInfo namTitleInfo;
	u32 count = 0;
	s32 i;

	// �^�C�g�����X�g�擾
	if (NAM_GetTitleList(sTitleIdArray, TITLE_LIST_MAX) != NAM_OK)
	{
		OS_TWarning("Fail! NAM_GetTitleList() in %s\n", __func__);
		return 0;
	}
	
	// �^�C�g�����擾
	title_num = NAM_GetNumTitles();

	for (i=0;i<title_num;i++)
	{
		// �^�C�g�����擾
	    if( NAM_ReadTitleInfo(&namTitleInfo, sTitleIdArray[i]) == NAM_OK )
	    {
			// NOT_LAUNCH_FLAG �܂��� DATA_ONLY_FLAG �������Ă���^�C�g���̓J�E���g���Ȃ�
			if (!(namTitleInfo.titleId & (TITLE_ID_NOT_LAUNCH_FLAG_MASK | TITLE_ID_DATA_ONLY_FLAG_MASK)))
			{
				count++;
			}
		}
	}

	return count;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DrawNandTree

  Description:  NAND�̃c���[�����v�����g�o�͂��܂�

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NAMUT_DrawNandTree(void)
{
	MI_CpuClear8( sCurrentFullPath, sizeof(sCurrentFullPath) );
	NAMUTi_DrawNandTree(0, "nand:");
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DrawNandTree

  Description:  �w��p�X�̃c���[�����v�����g�o�͂��܂��i�ċA�֐��j

  Arguments:    path : ��΃p�X�w��(�X���b�V�����܂߂Ȃ��j

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void NAMUTi_DrawNandTree(s32 depth, const char *path)
{
    FSFile  dir;
	char* pSlash = STD_SearchCharReverse( sCurrentFullPath, '/' );

	if (pSlash != NULL)
	{
		PrintDirectory(depth, pSlash);
	}
	else
	{
		PrintDirectory(depth, path);		
	}

	// �[������
	if (depth > DIRECTORY_DEPTH_MAX)
	{
		OS_TPrintf("Fail! Depth is too deep.\n");
		return;
	}

	// �J�����g�p�X��ݒ�    
	STD_CopyLString( sCurrentFullPath, path, FS_ENTRY_LONGNAME_MAX );

	FS_InitFile(&dir);

	// �����Ŏw�肳�ꂽ�f�B���N�g�����J��
	if (!FS_OpenDirectory(&dir, sCurrentFullPath, (FS_FILEMODE_R|FS_FILEMODE_W)))
	{
		OS_TPrintf("%d Fail! FS_OpenDirectory(%s)\n", __LINE__, sCurrentFullPath);
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
			PrintFile(depth, sEntryInfo.longname);
		}
		else
		{
			STD_ConcatenateLString(sCurrentFullPath, "/", FS_ENTRY_LONGNAME_MAX);
			STD_ConcatenateLString(sCurrentFullPath, sEntryInfo.longname, FS_ENTRY_LONGNAME_MAX);
			NAMUTi_DrawNandTree(depth + 1, sCurrentFullPath);
		}
	}

	// �J�����g�p�X�����
	if (pSlash != NULL)
	{
		*pSlash = '\0';
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
