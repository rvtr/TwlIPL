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

#include <twl.h>
#include <twl/fatfs.h>
#include <twl/os/common/format_rom.h>
#include <twl/nam.h>
#include <es.h>
#include <estypes.h>
#include <twl/aes.h>
#include <twl/os/common/banner.h>
#include <sysmenu/namut.h>
#include <nitro/nvram.h>
#include <twl/lcfg/common/TWLSettings.h>
#include <twl/lcfg/common/api.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

// eTicketType
typedef enum ETicketType {
	ETICKET_TYPE_COMMON = 0,
	ETICKET_TYPE_PERSONALIZED = 1
}ETicketType;

// �{�̏�����(NAND������)�ŏ������Ȃ��^�C�g����
// TitleProperty (TitleID 32bit�j�̃r�b�g�Ŏw�肵�܂��B
// �ǂꂩ1�ł��r�b�g�������Ă���Ώ����̑Ώۂ���O���܂��B
#define PROTECT_TITLE_PROPERTY  (TITLE_ID_APP_TYPE_MASK)
#define CLEAR_DATA_SIZE        16384  // �t�@�C�������f�[�^�p�i512�̔{���ő傫���قǏ����������j

// �{�̏�����(NAND������)���s����
// �w��f�B���N�g���ȉ��͑S�ď�������܂��B
// �w��f�B���N�g�����͎̂c��܂��B
static const char* sDeleteDirectoryList[] =
{
    "nand:/import",
    "nand:/progress",
    "nand2:/photo",
    "nand:/tmp"
};

// �{�̏�����(NAND������)���s����
// �w��t�@�C���̓����_���f�[�^��Fill����܂��B
static const char* sFillFileList[] =
{
    "nand:/shared2/launcher/wrap.bin"
};

#define VOLUME_INITIAL_VALUE     5      // �{�̏��������ɐݒ肷��{�̃{�����[���l
#define BACKLIGHT_INITIAL_VALUE  3      // �{�̏��������ɐݒ肷��o�b�N���C�g�P�x
#define NAMUT_SHARE_ARCHIVE_MAX  6      // share�f�[�^��

#define SHOP_TITLE_ID         		0x00030015484E4600	// �V���b�v��TitleID�i���[�W���������j
#define TITLE_ID_MASK_EXCEPT_REGION 0xFFFFFFFFFFFFFF00	// TitleID�̃}�X�N�i���[�W���������j

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static NAMUTAlloc spAllocFunc;
static NAMUTFree  spFreeFunc;
static FSDirectoryEntryInfo sEntryInfo;
static char sCurrentFullPath[FS_ENTRY_LONGNAME_MAX];
static u32 sNCFGAddr;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL NAMUTi_DeleteNonprotectedTitle( BOOL isForceEraseCommonETicket );
static BOOL NAMUTi_ClearSavedataAll(void);
static BOOL NAMUTi_InitShareData(void);
static BOOL NAMUTi_MountAndFormatOtherTitleSaveData(u64 titleID, const char *arcname);
static BOOL NAMUTi_RandClearFile(const char* path);
static BOOL NAMUTi_CheckExistenceFile(const char* path);
static BOOL NAMUTi_ClearWiFiSettings( void );
static BOOL NAMUTi_DeleteShopAccount( void );
static void* NAMUT_Alloc(u32 size);
static void NAMUT_Free(void* buffer);
static s32 GetETicketType(NAMTitleId titleId, ETicketType *pETicketType );
static s32 GetTicketViews(ESTicketView** pptv, u32* pNumTicket, NAMTitleId titleId);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Init

  Description:  NAMUT ���C�u�����̏��������s���܂��B

  Arguments:    allocFunc:  �������m�ۊ֐��ւ̃|�C���^�B(�v�F32byte�A���C�����g�j
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

  Description:  �{�̏�����(NAND������)���s���܂��B
               �i�V�X�e���n�̕K�v�ȃt�@�C���݂̂��c�������������܂�
                 ���[�U�[�A�v���́Acommon, personalized�Ɋւ�炸�S�ď������܂��j

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format( void )
{
	return NAMUT_FormatCore( TRUE, TRUE );
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_FormatCore

  Description:  �{�̏�����(NAND������)���s���܂��B
               �i�V�X�e���n�̕K�v�ȃt�@�C���݂̂��c�������������܂�
                 ���[�U�[�A�v���� common, personalized�Ɋւ�炸�S�ď������邩�A
	�@�@�@�@�@�@ personalized�̂ݏ������邩�������őI���ł��܂��B

  Arguments:    isForceEraseCommonETicket: TRUE �̎��́Acommon, personalized�Ɋւ�炸���[�U�[�A�v����S����
                                           FALSE�̎��́AcommonETicket���c���i�A�v�����g�͏����j
	            isDeleteWifiSettings: WiFi�ݒ���폜���邩�H�iTRUE�ō폜�j

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_FormatCore( BOOL isForceEraseCommonETicket, BOOL isDeleteWiFiSettings )
{
    int i;
    BOOL ret = TRUE;

    // �v���e�N�g����Ă��Ȃ��^�C�g���̍폜���s���܂�
    if (!NAMUTi_DeleteNonprotectedTitle( isForceEraseCommonETicket ))
    {
        ret = FALSE;
        OS_TWarning("Fail! NAMUTi_DeleteNonprotectedTitle()\n");
    }

	// �V���b�v�A�J�E���g�����폜���܂�
	if (!NAMUTi_DeleteShopAccount())
	{
		ret = FALSE;
        OS_TWarning("Fail! NAMUTi_DeleteShopAccount()\n");
	}

    // �v���e�N�g�^�C�g���̃Z�[�u�f�[�^�����������܂�
    if (!NAMUTi_ClearSavedataAll())
    {
        ret = FALSE;
        OS_TWarning("Fail! NAMUTi_ClearSavedataAll()\n");
    }

    // share�f�[�^�����������܂�
    if (!NAMUTi_InitShareData())
    {
        ret = FALSE;
        OS_TWarning("Fail! NAMUTi_InitShareData()\n");
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
	if (isDeleteWiFiSettings &&
		!NAMUTi_ClearWiFiSettings())
	{
		ret = FALSE;
        OS_TWarning("Fail! NAMUTi_ClearWiFiSettings()\n");
	}
	
    // �{�̐ݒ�f�[�^�̃N���A
	if (!NAMUT_ClearTWLSettings( TRUE ))
	{
		ret = FALSE;
        OS_TWarning("Fail! NAMUT_ClearTWLSettings()\n");
	}

    // RTC�̃N���A�͕K�v�Ȃ��B2008.06.25 �����m�F�B
    // �{�̏�������̏���N���V�[�P���X�N������RTC���N���A���邽�߁B
    
    // �{�̃{�����[���ݒ�
    if (SNDEX_SetVolume(VOLUME_INITIAL_VALUE) != SNDEX_RESULT_SUCCESS)
    {
        ret = FALSE;
    }

    // �o�b�N���C�g�P�x�ݒ�
    if (PM_SendUtilityCommand( PMi_UTIL_SET_BACKLIGHT_BRIGHTNESS, (u16)BACKLIGHT_INITIAL_VALUE, NULL ) != PM_SUCCESS)
    {
        ret = FALSE;
    }

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
  Name:         NAMUT_DeleteNandTmpDirectory

  Description:  "nand:/tmp" �f�B���N�g���ȉ����������܂��B
                ��O�Ƃ��āA"nand:/tmp/es" �ȉ����c���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_DeleteNandTmpDirectory(void)
{
    FSFile  dir;
    FSDirectoryEntryInfo entryInfo;
    BOOL ret = TRUE;
    const char* path = "nand:/tmp";

    FS_InitFile(&dir);

    // �f�B���N�g�� "nand:/tmp" ���J��
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
        
        // es �f�B���N�g���ȉ��͗�O�Ƃ��Ďc��
        if (STD_CompareString(entryInfo.longname, "es") == 0 &&
            (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY))
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

  Arguments:    isForceEraseCommonETicket : TRUE�̎��́Acommon, personalized�Ɋւ�炸���[�U�[�A�v����S����
	                                        FALSE�̎��́AcommonETicket���c���i�A�v�����g�͏����j

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_DeleteNonprotectedTitle( BOOL isForceEraseCommonETicket )
{
    char dirPath[NAM_PATH_LEN];
    u32 title_num;              // NAND �ɃC���X�g�[������Ă���A�v���̐�
    u32 title_num_installed;    // NAND�ɃC���X�g�[�����ꂽ���Ƃ�����A�v���̐�
    u32 title_num_all;          // ��2���}�[�W�����A�v���̐�
    NAMTitleId* pTitleIdArray;
    NAMTitleId* pTitleIdArrayInstalled;
    s32 result = TRUE;
    s32 i,j;

    // �^�C�g�����擾
    title_num = (u32)NAM_GetNumTitles();
    title_num_installed = (u32)NAM_GetNumInstalledTitles();

    // �^�C�g��ID�z��p�������m��
    pTitleIdArray          = NAMUT_Alloc(sizeof(NAMTitleId)*(title_num + title_num_installed));
    pTitleIdArrayInstalled = NAMUT_Alloc(sizeof(NAMTitleId)*title_num_installed);

    if (pTitleIdArray == NULL || pTitleIdArrayInstalled == NULL)
    {
        OS_TWarning("Allocation failed in %s\n", __func__);
        NAMUT_Free(pTitleIdArray);
        NAMUT_Free(pTitleIdArrayInstalled); 
        return FALSE;
    }

    // �^�C�g�����X�g�擾
    if (NAM_GetTitleList(pTitleIdArray, title_num) != NAM_OK ||
        NAM_GetInstalledTitleList(pTitleIdArrayInstalled, title_num_installed) != NAM_OK)
    {
        OS_TWarning("Fail! NAM_Get*TitleList() in %s\n", __func__);
        NAMUT_Free(pTitleIdArray);
        NAMUT_Free(pTitleIdArrayInstalled); 
        return FALSE;
    }

    // NAM_GetTitleList�ł͍폜����Ă��邪eTicket�̂ݑ��݂���^�C�g�������X�g�A�b�v���ꂸ
    // NAM_GetInstalledTitleList�ł�SRL�͂��邪eTicket���Ȃ��^�C�g�������X�g�A�b�v����Ȃ��B
    // ���̂��ߗ��҂��}�[�W����
    title_num_all = title_num;
    for (i=0;i<title_num_installed;i++)
    {
        BOOL find = FALSE;
        for (j=0;j<title_num;j++)
        {
            if (pTitleIdArrayInstalled[i] == pTitleIdArray[j]) 
            {
                find = TRUE;
                break;
            }
        }
        if (find == FALSE)
        {
            pTitleIdArray[title_num_all] = pTitleIdArrayInstalled[i];
            title_num_all++;
        }
    }

    for (i=0;i<title_num_all;i++)
    {
        // �v���e�N�g�ΏۈȊO�ł����titleId_Hi�f�B���N�g�����Ə�������
        if (!(pTitleIdArray[i] & PROTECT_TITLE_PROPERTY))
        {
            // nand:/title/titleID_Hi/ �ȉ�������
            STD_TSNPrintf(dirPath, NAM_PATH_LEN, "nand:/title/%08x", NAM_GetTitleIdHi(pTitleIdArray[i]) );
            if ( !FS_DeleteDirectoryAuto( dirPath ) )
            {
                result = FALSE;
            }

       	    // nand:/ticket/titleID_Hi/ �ȉ�������
			{
				ETicketType eTicketType = ETICKET_TYPE_PERSONALIZED; // default
				
				if( isForceEraseCommonETicket ) {
					// ���� CommonETicket �����t���O���L���ȏꍇ�́A�S�`�P�b�g������
	    	       	STD_TSNPrintf(dirPath, NAM_PATH_LEN, "nand:/ticket/%08x", NAM_GetTitleIdHi(pTitleIdArray[i]) );
	        	   	if ( !FS_DeleteDirectoryAuto( dirPath ) )
		            {
	    	       		result = FALSE;
	        	   	}
				}else {
					// �����łȂ��ꍇ�́ACommonETicket �ȊO�̃^�C�g���� eTicket �̂ݏ���
					if( GetETicketType( pTitleIdArray[i], &eTicketType ) == NAM_OK )
					{
						if( eTicketType != ETICKET_TYPE_COMMON )
						{
			    	       	STD_TSNPrintf(dirPath, NAM_PATH_LEN, "nand:/ticket/%08x/%08x.tik",
										  NAM_GetTitleIdHi(pTitleIdArray[i]),
										  NAM_GetTitleIdLo(pTitleIdArray[i]) );
			        	   	if ( !FS_DeleteFile( dirPath ) &&
								 FS_GetArchiveResultCode( dirPath ) != FS_RESULT_ALREADY_DONE )
							{
								result = FALSE;
	        			   	}
						}
					}
					else
					{
						result = FALSE;
					}
				}
			}
        }
    }

    NAMUT_Free(pTitleIdArray);
    NAMUT_Free(pTitleIdArrayInstalled); 

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
    u32 title_num;
    NAMTitleId* pTitleIdArray;
    NAMTitleInfo namTitleInfo;
    char subBannerPath[ FS_ENTRY_LONGNAME_MAX ];
    BOOL ret = TRUE;
    s32 i;

    // �^�C�g�����擾
    title_num = (u32)NAM_GetNumTitles();

    // �^�C�g��ID�z��p�������m��
    pTitleIdArray = NAMUT_Alloc(sizeof(NAMTitleId)*title_num);
    if (pTitleIdArray == NULL)
    {
        OS_TWarning("Allocation failed in %s\n", __func__);
        return FALSE;
    }

    // �^�C�g�����X�g�擾
    if (NAM_GetTitleList(pTitleIdArray, title_num) != NAM_OK)
    {
        OS_TWarning("Fail! NAM_GetTitleList() in %s\n", __func__);
        NAMUT_Free(pTitleIdArray);
        return FALSE;
    }

    for (i=0;i<title_num;i++)
    {
        // �^�C�g�����擾
        if( NAM_ReadTitleInfo(&namTitleInfo, pTitleIdArray[i]) == NAM_OK )
        {
            // publicSaveSize��0�ȏ�Ȃ�t�H�[�}�b�g
            if (namTitleInfo.publicSaveSize > 0)
            {
                ret &= NAMUTi_MountAndFormatOtherTitleSaveData(namTitleInfo.titleId, "otherPub");
            }
            // privateSaveSize��0�ȏ�Ȃ�t�H�[�}�b�g
            if (namTitleInfo.privateSaveSize > 0)
            {
                ret &= NAMUTi_MountAndFormatOtherTitleSaveData(namTitleInfo.titleId, "otherPrv");
            }

            // �T�u�o�i�[�t�@�C���p�X�擾
            if (NAM_GetTitleBannerFilePath( subBannerPath, namTitleInfo.titleId) == NAM_OK)
            {
                NAMUTi_DestroySubBanner( subBannerPath );
            }
            else { ret = FALSE; }
        }
        else { ret = FALSE; }
    }

    NAMUT_Free(pTitleIdArray);

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

    pBanner = NAMUT_Alloc( sizeof(TWLSubBannerFile) );
    if (!pBanner)
    {
        OS_TWarning("Allocation failed. (%d)\n");
        return FALSE;
    }

    // RWL�����ŊJ��
    FS_InitFile(file);
    if ( !FS_OpenFileEx(file, path, FS_FILEMODE_RWL) )
    {
        OS_Warning("banner file open failed.\n");
        NAMUT_Free( pBanner );
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
    
    NAMUT_Free( pBanner );
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
    FSFATFSArchiveWork* pWork;
    FSResult    result;

    pWork = NAMUT_Alloc( sizeof(FSFATFSArchiveWork) );
    if (!pWork)
    {
        OS_TWarning("Allocation failed. (%d)\n");
        return FALSE;
    }

    // �}�E���g���s�B
    result = FSi_MountSpecialArchive(titleID, arcname, pWork);
    if (result != FS_RESULT_SUCCESS)
    {
        OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
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
        // �A���}�E���g�B
        (void)FSi_MountSpecialArchive(titleID, NULL, pWork);
    }

    NAMUT_Free ( pWork );

    return succeeded;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_InitShareData

  Description:  �SShare�f�[�^�t�@�C�����t�H�[�}�b�g�B
                ���̊֐����s�O��Share:/���}�E���g����Ă��Ă͂����Ȃ��B

  Arguments:    None

  Returns:      ���������TRUE
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_InitShareData(void)
{
    BOOL    succeeded = TRUE;
    FSFATFSArchiveWork* pWork;
    char path[NAM_PATH_LEN];
    FSResult    result;
    int i;

    pWork = NAMUT_Alloc( sizeof(FSFATFSArchiveWork) );
    if (!pWork)
    {
        OS_TWarning("Allocation failed. (%d)\n");
        return FALSE;
    }

    for (i=0;i<NAMUT_SHARE_ARCHIVE_MAX;i++)
    {
        // �t�@�C���̑��݂��m�F
        STD_TSNPrintf(path, NAM_PATH_LEN, "nand:/shared2/000%d", i);
        if (NAMUTi_CheckExistenceFile(path) == FALSE)
        {
            // �t�@�C�������݂��Ȃ����̂Ƃ݂Ȃ�
            OS_TPrintf("%s is not exist\n", path);
            continue;
        }

        // �}�E���g
        result = FSi_MountSpecialArchive((OSTitleId)i, "share", pWork);
        if (result != FS_RESULT_SUCCESS)
        {
            succeeded = FALSE;
            OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
            continue;
        }

        // �ăt�H�[�}�b�g
        result = FSi_FormatSpecialArchive("share:/");
        if (result != FS_RESULT_SUCCESS)
        {
            succeeded = FALSE;
            OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
        }

        // �A���}�E���g
        result = FSi_MountSpecialArchive((OSTitleId)i, NULL, pWork);
        if (result != FS_RESULT_SUCCESS)
        {
            succeeded = FALSE;
            OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
        }
    }

    NAMUT_Free( pWork );

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
        u32 filesize;
        u8* pClearData = NAMUT_Alloc( CLEAR_DATA_SIZE );
        if (!pClearData)
        {
            OS_TWarning("Allocation failed. (%d)\n");
            FS_CloseFile(&file);
            return FALSE;
        }

        for (filesize = FS_GetFileLength(&file); filesize > CLEAR_DATA_SIZE; filesize -= CLEAR_DATA_SIZE)
        {
            if( AES_Rand(pClearData, CLEAR_DATA_SIZE) != AES_RESULT_SUCCESS )
            {
                FS_CloseFile(&file);
                NAMUT_Free( pClearData );
                return FALSE;
            }
            FS_WriteFile(&file, pClearData, CLEAR_DATA_SIZE);
        }

        if (filesize > 0)
        {
            if( AES_Rand(pClearData, filesize) != AES_RESULT_SUCCESS )
            {
                FS_CloseFile(&file);
                NAMUT_Free( pClearData );
                return FALSE;
            }
            FS_WriteFile(&file, pClearData, (s32)filesize);
        }
        FS_CloseFile(&file);
        NAMUT_Free( pClearData );
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_CheckExistenceFile

  Description:  �w�肵���t�@�C�������݂��邩�ǂ����𔻒肵�܂��B

  Arguments:    path

  Returns:      ���݂���Ȃ�TRUE
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_CheckExistenceFile(const char* path)
{
    FSFile file;

    FS_InitFile(&file);

    if (!FS_OpenFileEx(&file, path, FS_FILEMODE_R))
    {
		return FALSE;
 	}

    FS_CloseFile(&file);
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
    u32 installedSoftBoxCount;

    // installedSoftBoxCount�̎擾
    if ( NAM_OK != NAM_GetInstalledSoftBoxCount( &installedSoftBoxCount ) ) {
        return FALSE;
    }

    // installed count
    *installed = (u8)installedSoftBoxCount;

    // free count
    *free = (u8)(LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - installedSoftBoxCount);

    return TRUE;
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
    BOOL retval = TRUE;

    // InstalledSoftBoxCount, FreeSoftBoxCount �𐔂��Ȃ���
    if (!NAMUT_GetSoftBoxCount(&installedSoftBoxCount, &freeSoftBoxCount))
    {
        return FALSE;
    }

    // LCFG���C�u�����̐ÓI�ϐ��ɑ΂���X�V
    LCFG_TSD_SetInstalledSoftBoxCount( installedSoftBoxCount ); 
    LCFG_TSD_SetFreeSoftBoxCount( freeSoftBoxCount );

    // LCFG���C�u�����̐ÓI�ϐ��̒l��NAND�ɔ��f
    pBuffer = NAMUT_Alloc( LCFG_WRITE_TEMP );
    if (!pBuffer) { return FALSE; }
    // �~���[�����O�f�[�^�̗����ɏ������݂��s���B
    retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
    retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
    NAMUT_Free( pBuffer );

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_ClearTWLSettings

  Description:  TWL�{�̐ݒ�f�[�^�̃N���A���s���܂��B

  Arguments:    None.

  Returns:      �����Ȃ�TRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_ClearTWLSettings( BOOL doWriteback )
{
    BOOL retval = TRUE;
    
    // �{�̐ݒ�f�[�^�̃N���A�BTP�L�����u���[�V�����f�[�^�͎c���B
    {
        LCFGTWLTPCalibData calib;
        u32 installedSoftBoxCount;

        // installedSoftBoxCount�̎擾
        if ( NAM_OK != NAM_GetInstalledSoftBoxCount( &installedSoftBoxCount ) ) {
            return FALSE;
        }

        LCFG_TSD_GetTPCalibration( &calib );
        LCFG_ClearTWLSettings( (u8)installedSoftBoxCount );
        LCFG_TSD_SetTPCalibration( &calib );
    }

    // LCFG���C�u�����̐ÓI�ϐ��̒l��NAND�ɔ��f
    if( doWriteback ) {
        u8 *pBuffer = NAMUT_Alloc( LCFG_WRITE_TEMP );
        if (!pBuffer) { return FALSE; }
        // �~���[�����O�f�[�^�̗����ɏ������݂��s���B
        retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
        retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
        NAMUT_Free( pBuffer );
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearWiFiSettings

  Description:  NVRAM��WiFi�ݒ�f�[�^���N���A����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_ClearWiFiSettings( void )
{
#define NCFG_ADDR			0x20
#define NTR_WIFI_DATA_SIZE	0x400
#define TWL_WIFI_DATA_SIZE	0x600
	const s32 RetryCount = 5;
	s32 i;
	BOOL readSuccess  = FALSE;
	BOOL writeSuccess = FALSE;
	u32 total_size = ( NTR_WIFI_DATA_SIZE + TWL_WIFI_DATA_SIZE );
	u8* pClearData = NAMUT_Alloc( total_size );

	if (!pClearData)
	{
        OS_TWarning("Allocation failed. (%d)\n");
		return FALSE;
	}

    if (!NVRAMi_IsInitialized()) {
        NVRAMi_Init();
    }
    DC_FlushRange( &sNCFGAddr, 2 );
	sNCFGAddr = 0;
	for (i=0;i<RetryCount;i++)
	{
		if (NVRAMi_Read( NCFG_ADDR, 2, (u8 *)&sNCFGAddr) == NVRAM_RESULT_SUCCESS)
		{
			readSuccess = TRUE;
			break;
		}
	}

	if (!readSuccess)
	{
		NAMUT_Free( pClearData );
		return FALSE;
	}

	sNCFGAddr = (u32)( ( sNCFGAddr << 3 ) - ( NTR_WIFI_DATA_SIZE + TWL_WIFI_DATA_SIZE ) );
	
	MI_CpuFillFast( pClearData, 0xffffffff, total_size);
    DC_FlushRange( pClearData, total_size );

	for (i=0;i<RetryCount;i++)
	{
    	if (NVRAMi_Write( sNCFGAddr, total_size , pClearData ) == NVRAM_RESULT_SUCCESS)
		{
			writeSuccess = TRUE;
			break;
		}
	}

	NAMUT_Free( pClearData );

	if (!writeSuccess)
	{
		return FALSE;
	}

	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteShopAccount

  Description:  �V���b�v�̃A�J�E���g�����������܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_DeleteShopAccount( void )
{
    u32 title_num;
    NAMTitleId* pTitleIdArray;
	char path[ FS_ENTRY_LONGNAME_MAX ];
    BOOL ret = TRUE;
    s32 i;

    // �^�C�g�����擾
    title_num = (u32)NAM_GetNumTitles();

    // �^�C�g��ID�z��p�������m��
    pTitleIdArray = NAMUT_Alloc(sizeof(NAMTitleId)*title_num);
    if (pTitleIdArray == NULL)
    {
        OS_TWarning("Allocation failed in %s\n", __func__);
        return FALSE;
    }

    // �^�C�g�����X�g�擾
    if (NAM_GetTitleList(pTitleIdArray, title_num) != NAM_OK)
    {
        OS_TWarning("Fail! NAM_GetTitleList() in %s\n", __func__);
        NAMUT_Free(pTitleIdArray);
        return FALSE;
    }

	// �V���b�v����������
    for (i=0;i<title_num;i++)
    {
		if ((pTitleIdArray[i] & TITLE_ID_MASK_EXCEPT_REGION) == SHOP_TITLE_ID)
		{
            STD_TSNPrintf(path, FS_ENTRY_LONGNAME_MAX, "nand:/title/%08x/%08x/data/ec.cfg", 
				NAM_GetTitleIdHi(pTitleIdArray[i]), NAM_GetTitleIdLo(pTitleIdArray[i]) );
			
            if ( !FS_DeleteFile( path ) )
			{
				FSResult fsResult = FS_GetArchiveResultCode(path);

				if ((fsResult != FS_RESULT_ALREADY_DONE) && (fsResult != FS_RESULT_NO_ENTRY))
				{
					ret = FALSE;
				}
			}
		}
    }

    NAMUT_Free(pTitleIdArray);
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Alloc

  Description:  �������̊m�ۂ��s���܂�

  Arguments:    size: �T�C�Y�o�C�g

  Returns:      �m�ۂ����������ւ̃|�C���^
 *---------------------------------------------------------------------------*/
static void* NAMUT_Alloc(u32 size)
{
    const u32 allocSize = MATH_ROUNDUP32(size);
    SDK_ASSERTMSG( spAllocFunc != NULL, "NAMUT_Init should be called previously.\n");
    return spAllocFunc(allocSize);
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Free

  Description:  �������̉�����s���܂�

  Arguments:    buffer: ����o�b�t�@

  Returns:      None
 *---------------------------------------------------------------------------*/
static void NAMUT_Free(void* buffer)
{
    SDK_ASSERTMSG( spFreeFunc != NULL, "NAMUT_Init should be called previously.\n");
    if (buffer)
    {
        spFreeFunc(buffer);
    }
}



/*---------------------------------------------------------------------------*
  Name:         GetETicketType

  Description:  �w�肳�ꂽ titleID �� eTicket �^�C�v���擾����

  Arguments:    titleID: common eTicket ���ǂ����𒲂ׂ����^�C�g���� titleID
	            pETicketType : ���ʂ��i�[����ESETicketType�|�C���^

  Returns:      NAM_OK  : �擾����
	            ����ȊO: �擾���s
 *---------------------------------------------------------------------------*/
static s32 GetETicketType(NAMTitleId titleId, ETicketType *pETicketType )
{
    s32 result;
	ESTicketView* ptv;
    u32 numTicket;
	
	*pETicketType = ETICKET_TYPE_PERSONALIZED;
	
    result = GetTicketViews(&ptv, &numTicket, titleId);
	
    if( result == NAM_OK )
    {
        if( numTicket > 0 )
        {
			// �擪 eTicket �� deviceId �� 0x00000000 �Ȃ�Acommon eTicket �Ɣ��f�B
			// ���S�Ă� eTicket ���r�߂�K�v�͂Ȃ��H
			if( ptv->deviceId == 0x00000000 ) {
				*pETicketType = ETICKET_TYPE_COMMON;
			}
		}
        NAMUT_Free(ptv);
	}
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         GetTicketViews

  Description:  �w�肳�ꂽ�^�C�g���� eTicket ���擾
�@�@�@�@�@�@�@�@��nam_title.c �� GetTicketViews �֐����R�s�y

  Arguments:    pptv       : �擾�������� eTicket ���X�g�̃|�C���^���i�[����|�C���^
	            pNumTicket : �擾�������� eTicket �����i�[����|�C���^
                titleID    : eTicket ���擾�������^�C�g���� titleID

  Returns:      NAM_OK     : �擾����
	            ����ȊO   : �擾���s
 *---------------------------------------------------------------------------*/
static s32 GetTicketViews(ESTicketView** pptv, u32* pNumTicket, NAMTitleId titleId)
{
    s32 result;
    u32 numTicket;
    ESTicketView* ptv = NULL;

    result = ES_GetTicketViews(titleId, NULL, &numTicket);
//  PRINT_RESULT(result);

    if( result != ES_ERR_OK )
    {
        return result;
    }

    if( numTicket != 0 )
    {
        ptv = NAMUT_Alloc(sizeof(ESTicketView) * numTicket);

        if( ptv == NULL )
        {
            return NAM_NO_MEMORY;
        }

        result = ES_GetTicketViews(titleId, ptv, &numTicket);
//      PRINT_RESULT(result);
    }

    if( result == ES_ERR_OK )
    {
        *pptv = ptv;
        *pNumTicket = numTicket;
    }
    else
    {
        NAMUT_Free(ptv);
    }

    return result;
}
