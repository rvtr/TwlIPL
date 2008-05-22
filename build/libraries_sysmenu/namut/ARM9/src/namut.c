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
#include <twl/aes.h>
#include <twl/os/common/banner.h>
#include <sysmenu/namut.h>
#include <nitro/nvram.h>
#include <twl/lcfg/common/TWLSettings.h>
#include <twl/lcfg/common/api.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

// �[���t�H�[�}�b�g�ŏ������Ȃ��^�C�g����
// TitleProperty (TitleID Hi�̉���16bit�j�̃r�b�g�Ŏw�肵�܂��B
// �ǂꂩ1�ł��r�b�g�������Ă���Ώ����̑Ώۂ���O���܂��B
#define PROTECT_TITLE_PROPERTY  (TITLE_ID_APP_TYPE_MASK)

#define DIRECTORY_DEPTH_MAX      16  // �f�B���N�g���̐[���̍ő�iNAND�̐��K�\���Ƃ��Ă͍ő�6�j
#define TITLE_ID_HI_SIZE          8
#define TITLE_LIST_MAX          256
#define CLEAR_DATA_SIZE         256

// �[���t�H�[�}�b�g���s����
// �w��f�B���N�g���ȉ��͑S�ď�������܂��B
// �w��f�B���N�g�����͎̂c��܂��B
static const char* sDeleteDirectoryList[] =
{
	"nand:/shared2",
	"nand2:/photo",
	"nand:/tmp"
};

// �[���t�H�[�}�b�g���s����
// �w��t�@�C���̓����_���f�[�^��Fill����܂��B
static const char* sFillFileList[] =
{
	"nand:/shared1/TWLCFG0.dat",
	"nand:/shared1/TWLCFG1.dat"
};

static u8  sClearData[CLEAR_DATA_SIZE] ATTRIBUTE_ALIGN(32);
static u32 sNCFGAddr;

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static NAMUTAlloc spAllocFunc;
static NAMUTFree  spFreeFunc;
static FSDirectoryEntryInfo sEntryInfo;
static NAMTitleId sTitleIdArray[TITLE_LIST_MAX];
static char sCurrentFullPath[FS_ENTRY_LONGNAME_MAX];

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL NAMUTi_DeleteNonprotectedTitle(void);
static BOOL NAMUTi_DeleteNonprotectedTitleEntity(const char* path);
static BOOL NAMUTi_ClearSavedataAll(void);
static BOOL NAMUTi_MountAndFormatOtherTitleSaveData(u64 titleID, const char *arcname);
static void NAMUTi_DrawNandTree(s32 depth, const char *path);
static BOOL NAMUTi_RandClearFile(const char* path);
static void NAMUTi_ClearWiFiSettings( void );
static void PrintDirectory(s32 depth, const char* path);
static void PrintFile(s32 depth, const char* path);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Init

  Description:  NAMUT ���C�u�����̏��������s���܂��B

  Arguments:    allocFunc:  �������m�ۊ֐��ւ̃|�C���^�B
                freeFunc:   ����������֐��ւ̃|�C���^�B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
void NAMUT_Init(NAMUTAlloc allocFunc, NAMUTFree freeFunc)
{
    SDK_ASSERT( spAllocFunc == NULL );
    SDK_ASSERT( spFreeFunc  == NULL );
    SDK_POINTER_ASSERT(allocFunc);
    SDK_POINTER_ASSERT(freeFunc);

    spAllocFunc        = allocFunc;
    spFreeFunc         = freeFunc;
}

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

	// �w��t�@�C���𗐐��ŃN���A���܂�
	for (i=0; i<sizeof(sFillFileList)/sizeof(sFillFileList[0]); i++)
	{
		if (!NAMUTi_RandClearFile(sFillFileList[i]))
		{
			ret = FALSE;
			OS_TWarning("Fail! NAMUTi_RandClearFile(%s)\n", sFillFileList[i]);
		}
	}

	// temp�ȉ����������܂�
	// NAM�֐���temp���쐬&�g�p�����\�������邽�ߍŌ�Ɏ��s���܂�
	for (i=0; i<sizeof(sDeleteDirectoryList)/sizeof(char*); i++)
	{
		if (!NAMUT_DeleteNandDirectory(sDeleteDirectoryList[i]))
		{
			ret = FALSE;
			OS_TWarning("NAMUT_DeleteNandDirectory(%s)\n", sDeleteDirectoryList[i]);
		}
	}

	// WiFi�ݒ�f�[�^���N���A���܂�
	NAMUTi_ClearWiFiSettings();

	// SoftBoxCount�̍X�V���s���܂�
	NAMUT_UpdateSoftBoxCount();

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DeleteNandDirectory

  Description:  �w��f�B���N�g���ȉ����������܂��B
                �w��f�B���N�g�����͎̂c��܂��B

  Arguments:    path : ��΃p�X�i�X���b�V�����܂߂Ȃ��j

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_DeleteNandDirectory(const char *path)
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
    char dirPath[NAM_PATH_LEN];
	s32 title_num;	
	NAMTitleInfo namTitleInfo;
    s32 result = TRUE;
	s32 i;

	// �^�C�g�����X�g�擾
	if (NAM_GetTitleList(sTitleIdArray, TITLE_LIST_MAX) != NAM_OK)
	{
		OS_TWarning("Fail! NAM_GetTitleList() in %s\n", __func__);
		return FALSE;
	}
	
	// �^�C�g�����擾
	title_num = NAM_GetNumTitles();

	for (i=0;i<title_num;i++)
	{
		// �^�C�g�����擾
	    if( NAM_ReadTitleInfo(&namTitleInfo, sTitleIdArray[i]) == NAM_OK )
	    {
			// �v���e�N�g�ΏۈȊO�ł����titleId_Hi�f�B���N�g�����Ə�������
			if (!(namTitleInfo.titleId & PROTECT_TITLE_PROPERTY))
			{
				// nand:/title/titleID_Hi/ �ȉ�������
		    	STD_TSNPrintf(dirPath, NAM_PATH_LEN, "nand:/title/%08x", NAM_GetTitleIdHi(namTitleInfo.titleId) );
				if ( !FS_DeleteDirectoryAuto( dirPath ) )
				{
					result = FALSE;
				}
				// nand:/ticket/titleID_Hi/ �ȉ�������
		    	STD_TSNPrintf(dirPath, NAM_PATH_LEN, "nand:/ticket/%08x", NAM_GetTitleIdHi(namTitleInfo.titleId) );
				if ( !FS_DeleteDirectoryAuto( dirPath ) )
				{
					result = FALSE;
				}
			}
		}
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataAll

  Description:  �S�Z�[�u�f�[�^�̗����N���A���t�H�[�}�b�g���s���܂�

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/

static BOOL NAMUTi_ClearSavedataAll( void )
{
	s32 title_num;	
	NAMTitleInfo namTitleInfo;
	char savePublicPath[ FS_ENTRY_LONGNAME_MAX ];
	char savePrivatePath[ FS_ENTRY_LONGNAME_MAX ];
	char subBannerPath[ FS_ENTRY_LONGNAME_MAX ];
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
				// publicSaveSize��0�ȏ�Ȃ痐���N���A���t�H�[�}�b�g
				if (namTitleInfo.publicSaveSize > 0)
				{
					ret &= NAMUTi_ClearSavedataPublic(savePublicPath, namTitleInfo.titleId);
				}
				// privateSaveSize��0�ȏ�Ȃ痐���N���A���t�H�[�}�b�g
				if (namTitleInfo.privateSaveSize > 0)
				{
					ret &= NAMUTi_ClearSavedataPrivate(savePrivatePath, namTitleInfo.titleId);
				}
			}
			else { ret = FALSE; }

			// �T�u�o�i�[�t�@�C���p�X�擾
			if (NAM_GetTitleBannerFilePath( subBannerPath, namTitleInfo.titleId) == NAM_OK)
			{
				NAMUTi_DestroySubBanner( subBannerPath );
			}
			else { ret = FALSE; }
		}
		else { ret = FALSE; }
	}

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataPublic

  Description:  �w�肵���Z�[�u�f�[�^�t�@�C���ɑ΂���
				�����N���A���t�H�[�}�b�g���s���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_ClearSavedataPublic(const char* path, u64 titleID)
{
	//----- �����N���A
	if (NAMUTi_RandClearFile(path) == FALSE)
	{
		OS_Warning(" Fail NAMUTi_RandClearFile");
		return FALSE;
	}

	//----- NAND�A�v���̃Z�[�u�f�[�^�t�@�C�����}�E���g���t�H�[�}�b�g
	return NAMUTi_MountAndFormatOtherTitleSaveData(titleID, "otherPub");
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataPrivate

  Description:  �w�肵���Z�[�u�f�[�^�t�@�C���ɑ΂���
				�����N���A���t�H�[�}�b�g���s���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_ClearSavedataPrivate(const char* path, u64 titleID)
{
	//----- �����N���A
	if (NAMUTi_RandClearFile(path) == FALSE)
	{
		OS_Warning(" Fail NAMUTi_RandClearFile");
		return FALSE;
	}

	//----- NAND�A�v���̃Z�[�u�f�[�^�t�@�C�����}�E���g���t�H�[�}�b�g
	return NAMUTi_MountAndFormatOtherTitleSaveData(titleID, "otherPrv");
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DestroySubBanner

  Description:  �w�肵���T�u�o�i�[��CRC�j������݂܂��B
				�w�肵���T�u�o�i�[�����݂��Ȃ��\��������܂���
				���̏ꍇ�ł�TRUE��Ԃ��܂��B�i�R�[�h��OS_DeleteSubBannerFile�̃p�N���j

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_DestroySubBanner(const char* path)
{
	TWLSubBannerFile* pBanner;
	u16 crc, solt;
	FSFile file[1];
	BOOL ret = FALSE;

	// R�����Ńt�@�C�����I�[�v�������݂ăt�@�C���̑��ݗL�����m�F����
	// ���݂��Ȃ��ꍇ��TRUE�ŕԂ�
	FS_InitFile(file);
	if ( !FS_OpenFileEx(file, path, FS_FILEMODE_R) )
	{
		return TRUE;
	}

	if ( !spAllocFunc || !spFreeFunc )
	{
		OS_Warning("NAMUT_Init should be called previously.");		
		return FALSE;
	}

	pBanner = spAllocFunc( sizeof(TWLSubBannerFile) );

	if (!pBanner)
	{
		return FALSE;
	}

	// RWL�����ŊJ��
	FS_InitFile(file);
	if ( !FS_OpenFileEx(file, path, FS_FILEMODE_RWL) )
	{
		OS_Warning("banner file open failed.\n");
		spFreeFunc( pBanner );
		return FALSE;
	}

	// CRC����₂��ď����߂�
	if( FS_ReadFile( file, pBanner, sizeof(TWLSubBannerFile) ) != -1 )
	{
		crc = SVC_GetCRC16( 0xffff, &pBanner->anime, sizeof(BannerAnime) );
		solt = 1;
		crc += solt;
		pBanner->h.crc16_anime = crc;
		FS_SeekFile( file, 0, FS_SEEK_SET );
		if( sizeof(BannerHeader) == FS_WriteFile(file, &pBanner->h, sizeof(BannerHeader)) )
		{
			ret = TRUE;
		}else
		{
			OS_Warning("banner file write failed.\n");
		}
	}else
	{
		OS_Warning("banner file read failed.\n");
	}
	FS_CloseFile(file);
	
	spFreeFunc( pBanner );
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
  Name:         NAMUTi_RandClearFile

  Description:  �w�肵���t�@�C���𗐐��Ŗ��߂܂��B

  Arguments:    path

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_RandClearFile(const char* path)
{
	FSFile file;

	// �t�@�C���\���̏�����
    FS_InitFile(&file);

	// �t�@�C���I�[�v��
	if (FS_OpenFileEx(&file, path, (FS_FILEMODE_RWL)))
	{
		// �t�@�C���������_���f�[�^�ŃN���A
		u32 filesize = FS_GetFileLength(&file);
		for (; filesize > CLEAR_DATA_SIZE; filesize -= CLEAR_DATA_SIZE)
		{
	        if( AES_Rand(sClearData, CLEAR_DATA_SIZE) != AES_RESULT_SUCCESS )
	        {
	            return FALSE;
	        }
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
  Name:         NAMUT_GetSoftBoxCount

  Description:  NAND�� InstalledSoftBoxCount�AFreeSoftBoxCount�𒲂ׂ�
                �w�肳�ꂽ�ϐ��Ɋi�[���܂��B

  Arguments:    installed : installed�J�E���g�i�[�ϐ�
				free      : free�J�E���g�i�[�ϐ�

  Returns:      �����Ȃ�TRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_GetSoftBoxCount( u8* installed, u8* free )
{
	s32 title_num;	
	NAMTitleInfo namTitleInfo;
	u8 count = 0;
	BOOL result = TRUE;
	s32 i;

	// �^�C�g�����X�g�擾
	if (NAM_GetTitleList(sTitleIdArray, TITLE_LIST_MAX) != NAM_OK)
	{
		OS_TWarning("Fail! NAM_GetTitleList() in %s\n", __func__);
		return FALSE;
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
		else
		{
			result = FALSE;
		}
	}

    // installed count
    *installed = count;

    // free count
    *free = (u8)(LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - count);

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_UpdateSoftBoxCount

  Description:  InstalledSoftBoxCount, FreeSoftBoxCount �̒l��
				���݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B

  Arguments:    None.

  Returns:      �����Ȃ�TRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_UpdateSoftBoxCount( void )
{
	u8 installedSoftBoxCount;
	u8 freeSoftBoxCount;
	u8 *pBuffer;

	// InstalledSoftBoxCount, FreeSoftBoxCount �𐔂��Ȃ���
	if (!NAMUT_GetSoftBoxCount(&installedSoftBoxCount, &freeSoftBoxCount))
	{
		return FALSE;
	}

//	OS_Printf("installedSoftBoxCount = %d\n", installedSoftBoxCount);
//	OS_Printf("freeSoftBoxCount      = %d\n", freeSoftBoxCount);

	// LCFG���C�u�����̐ÓI�ϐ��ɑ΂���X�V
    LCFG_TSD_SetInstalledSoftBoxCount( installedSoftBoxCount );	
    LCFG_TSD_SetFreeSoftBoxCount( freeSoftBoxCount );

	// LCFG���C�u�����̐ÓI�ϐ��̒l��NAND�ɔ��f
    pBuffer = spAllocFunc( LCFG_WRITE_TEMP );
	if (!pBuffer) { return FALSE; }
    (void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
    spFreeFunc( pBuffer );

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
		OS_Warning("Fail! Depth is too deep.\n");
		return;
	}

	// �J�����g�p�X��ݒ�    
	STD_CopyLString( sCurrentFullPath, path, FS_ENTRY_LONGNAME_MAX );

	FS_InitFile(&dir);

	// �����Ŏw�肳�ꂽ�f�B���N�g�����J��
	if (!FS_OpenDirectory(&dir, sCurrentFullPath, (FS_FILEMODE_R)))
	{
		OS_Warning("%d Fail! FS_OpenDirectory(%s)\n", __LINE__, sCurrentFullPath);
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
  Name:         NAMUTi_ClearWiFiSettings

  Description:  NVRAM��WiFi�ݒ�f�[�^���N���A����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NAMUTi_ClearWiFiSettings( void )
{
#define NCFG_ADDR			0x20
#define NTR_WIFI_DATA_SIZE	0x400
#define TWL_WIFI_DATA_SIZE	0x600
	int len = ( NTR_WIFI_DATA_SIZE + TWL_WIFI_DATA_SIZE );
	
    if (!NVRAMi_IsInitialized()) {
        NVRAMi_Init();
    }
    DC_FlushRange( &sNCFGAddr, 2 );
	sNCFGAddr = 0;
    NVRAMi_Read( NCFG_ADDR, 2, (u8 *)&sNCFGAddr);
	sNCFGAddr = (u32)( ( sNCFGAddr << 3 ) - len );
	
	MI_CpuFillFast( sClearData, 0xffffffff, CLEAR_DATA_SIZE);
    DC_FlushRange( sClearData, CLEAR_DATA_SIZE );
	while( len > 0 ) {
    	NVRAMi_Write( sNCFGAddr, CLEAR_DATA_SIZE , sClearData );
		sNCFGAddr += CLEAR_DATA_SIZE;
		len -= CLEAR_DATA_SIZE;
	}
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
