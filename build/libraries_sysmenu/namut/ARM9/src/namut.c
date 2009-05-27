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

// Nand Application Management UTility ライブラリ、略してNAMUTライブラリです。

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
    定数定義
 *---------------------------------------------------------------------------*/

// eTicketType
typedef enum ETicketType {
	ETICKET_TYPE_COMMON = 0,
	ETICKET_TYPE_PERSONALIZED = 1
}ETicketType;

// 本体初期化(NAND初期化)で消去しないタイトルを
// TitleProperty (TitleID 32bit）のビットで指定します。
// どれか1つでもビットが立っていれば消去の対象から外します。
#define PROTECT_TITLE_PROPERTY  (TITLE_ID_APP_TYPE_MASK)
#define CLEAR_DATA_SIZE        16384  // ファイル消去データ用（512の倍数で大きいほど処理が早い）

// 本体初期化(NAND初期化)実行時に
// 指定ディレクトリ以下は全て消去されます。
// 指定ディレクトリ自体は残ります。
static const char* sDeleteDirectoryList[] =
{
    "nand:/import",
    "nand:/progress",
    "nand2:/photo",
    "nand:/tmp"
};

// 本体初期化(NAND初期化)実行時に
// 指定ファイルはランダムデータでFillされます。
static const char* sFillFileList[] =
{
    "nand:/shared2/launcher/wrap.bin"
};

#define VOLUME_INITIAL_VALUE     5      // 本体初期化時に設定する本体ボリューム値
#define BACKLIGHT_INITIAL_VALUE  3      // 本体初期化時に設定するバックライト輝度
#define NAMUT_SHARE_ARCHIVE_MAX  6      // shareデータ個数

#define SHOP_TITLE_ID         		0x00030015484E4600	// ショップのTitleID（リージョン除く）
#define TITLE_ID_MASK_EXCEPT_REGION 0xFFFFFFFFFFFFFF00	// TitleIDのマスク（リージョン除く）

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static NAMUTAlloc spAllocFunc;
static NAMUTFree  spFreeFunc;
static FSDirectoryEntryInfo sEntryInfo;
static char sCurrentFullPath[FS_ENTRY_LONGNAME_MAX];
static u32 sNCFGAddr;

/*---------------------------------------------------------------------------*
    内部関数宣言
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

  Description:  NAMUT ライブラリの初期化を行います。

  Arguments:    allocFunc:  メモリ確保関数へのポインタ。(要：32byteアライメント）
                freeFunc:   メモリ解放関数へのポインタ。

  Returns:      なし。
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

  Description:  本体初期化(NAND初期化)を行います。
               （システム系の必要なファイルのみを残し他を消去します
                 ユーザーアプリは、common, personalizedに関わらず全て消去します）

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format( void )
{
	return NAMUT_FormatCore( TRUE, TRUE );
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_FormatCore

  Description:  本体初期化(NAND初期化)を行います。
               （システム系の必要なファイルのみを残し他を消去します
                 ユーザーアプリを common, personalizedに関わらず全て消去するか、
	　　　　　　 personalizedのみ消去するかを引数で選択できます。

  Arguments:    isForceEraseCommonETicket: TRUE の時は、common, personalizedに関わらずユーザーアプリを全消去
                                           FALSEの時は、commonETicketを残す（アプリ自身は消去）
	            isDeleteWifiSettings: WiFi設定を削除するか？（TRUEで削除）

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_FormatCore( BOOL isForceEraseCommonETicket, BOOL isDeleteWiFiSettings )
{
    int i;
    BOOL ret = TRUE;

    // プロテクトされていないタイトルの削除を行います
    if (!NAMUTi_DeleteNonprotectedTitle( isForceEraseCommonETicket ))
    {
        ret = FALSE;
        OS_TWarning("Fail! NAMUTi_DeleteNonprotectedTitle()\n");
    }

	// ショップアカウント情報を削除します
	if (!NAMUTi_DeleteShopAccount())
	{
		ret = FALSE;
        OS_TWarning("Fail! NAMUTi_DeleteShopAccount()\n");
	}

    // プロテクトタイトルのセーブデータを初期化します
    if (!NAMUTi_ClearSavedataAll())
    {
        ret = FALSE;
        OS_TWarning("Fail! NAMUTi_ClearSavedataAll()\n");
    }

    // shareデータを初期化します
    if (!NAMUTi_InitShareData())
    {
        ret = FALSE;
        OS_TWarning("Fail! NAMUTi_InitShareData()\n");
    }

    // 指定ファイルを乱数でクリアします
    for (i=0; i<sizeof(sFillFileList)/sizeof(sFillFileList[0]); i++)
    {
        if (!NAMUTi_RandClearFile(sFillFileList[i]))
        {
            ret = FALSE;
            OS_TWarning("Fail! NAMUTi_RandClearFile(%s)\n", sFillFileList[i]);
        }
    }

    // temp以下を消去します
    // NAM関数でtempが作成&使用される可能性があるため最後に実行します
    for (i=0; i<sizeof(sDeleteDirectoryList)/sizeof(char*); i++)
    {
        if (!NAMUT_DeleteNandDirectory(sDeleteDirectoryList[i]))
        {
            ret = FALSE;
            OS_TWarning("NAMUT_DeleteNandDirectory(%s)\n", sDeleteDirectoryList[i]);
        }
    }
	
	// WiFi設定データをクリアします
	if (isDeleteWiFiSettings &&
		!NAMUTi_ClearWiFiSettings())
	{
		ret = FALSE;
        OS_TWarning("Fail! NAMUTi_ClearWiFiSettings()\n");
	}
	
    // 本体設定データのクリア
	if (!NAMUT_ClearTWLSettings( TRUE ))
	{
		ret = FALSE;
        OS_TWarning("Fail! NAMUT_ClearTWLSettings()\n");
	}

    // RTCのクリアは必要ない。2008.06.25 小野沢確認。
    // 本体初期化後の初回起動シーケンス起動時にRTCをクリアするため。
    
    // 本体ボリューム設定
    if (SNDEX_SetVolume(VOLUME_INITIAL_VALUE) != SNDEX_RESULT_SUCCESS)
    {
        ret = FALSE;
    }

    // バックライト輝度設定
    if (PM_SendUtilityCommand( PMi_UTIL_SET_BACKLIGHT_BRIGHTNESS, (u16)BACKLIGHT_INITIAL_VALUE, NULL ) != PM_SUCCESS)
    {
        ret = FALSE;
    }

    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DeleteNandDirectory

  Description:  指定ディレクトリ以下を消去します。
                指定ディレクトリ自体は残ります。

  Arguments:    path : 絶対パス（スラッシュを含めない）

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_DeleteNandDirectory(const char *path)
{
    FSFile  dir;
    FSDirectoryEntryInfo entryInfo;
    BOOL ret = TRUE;

    FS_InitFile(&dir);

    // 引数で指定されたディレクトリを開く
    if (!FS_OpenDirectory(&dir, path, FS_FILEMODE_R))
    {
        OS_TWarning("Fail! FS_OpenDirectory(%s) in %s\n", path, __func__);
        return FALSE;
    }

    // ディレクトリの中身を読む
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

        // ディレクトリ
        if (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
        {
            if (!FS_DeleteDirectoryAuto(sCurrentFullPath))
            {
                ret = FALSE;
                OS_TWarning("Fail! FS_DeleteDirectoryAuto(%s) in %s\n", sCurrentFullPath, __func__);
            }
        }
        // ファイル
        else
        {
            if (!FS_DeleteFileAuto(sCurrentFullPath))
            {
                ret = FALSE;
                OS_TWarning("Fail! FS_DeleteFileAuto(%s) in %s\n", sCurrentFullPath, __func__);
            }
        }
    }

    // ディレクトリを閉じる
    FS_CloseDirectory(&dir);

    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DeleteNandTmpDirectory

  Description:  "nand:/tmp" ディレクトリ以下を消去します。
                例外として、"nand:/tmp/es" 以下を残します。

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

    // ディレクトリ "nand:/tmp" を開く
    if (!FS_OpenDirectory(&dir, path, FS_FILEMODE_R))
    {
        OS_TWarning("Fail! FS_OpenDirectory(%s) in %s\n", path, __func__);
        return FALSE;
    }

    // ディレクトリの中身を読む
    while (FS_ReadDirectory(&dir, &entryInfo))
    {
        if (STD_CompareString(entryInfo.longname, ".")  == 0 ||
            STD_CompareString(entryInfo.longname, "..") == 0)
        {
            continue;
        }
        
        // es ディレクトリ以下は例外として残す
        if (STD_CompareString(entryInfo.longname, "es") == 0 &&
            (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY))
        {
            continue;
        }

        STD_CopyLString( sCurrentFullPath, path, FS_ENTRY_LONGNAME_MAX );
        STD_ConcatenateLString(sCurrentFullPath, "/", FS_ENTRY_LONGNAME_MAX);
        STD_ConcatenateLString(sCurrentFullPath, entryInfo.longname, FS_ENTRY_LONGNAME_MAX);

        // ディレクトリ
        if (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
        {
            if (!FS_DeleteDirectoryAuto(sCurrentFullPath))
            {
                ret = FALSE;
                OS_TWarning("Fail! FS_DeleteDirectoryAuto(%s) in %s\n", sCurrentFullPath, __func__);
            }
        }
        // ファイル
        else
        {
            if (!FS_DeleteFileAuto(sCurrentFullPath))
            {
                ret = FALSE;
                OS_TWarning("Fail! FS_DeleteFileAuto(%s) in %s\n", sCurrentFullPath, __func__);
            }
        }
    }

    // ディレクトリを閉じる
    FS_CloseDirectory(&dir);

    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteNonprotectedTitle

  Description:  User App タイトルの削除を行います。

  Arguments:    isForceEraseCommonETicket : TRUEの時は、common, personalizedに関わらずユーザーアプリを全消去
	                                        FALSEの時は、commonETicketを残す（アプリ自身は消去）

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_DeleteNonprotectedTitle( BOOL isForceEraseCommonETicket )
{
    char dirPath[NAM_PATH_LEN];
    u32 title_num;              // NAND にインストールされているアプリの数
    u32 title_num_installed;    // NANDにインストールされたことがあるアプリの数
    u32 title_num_all;          // 上2つをマージしたアプリの数
    NAMTitleId* pTitleIdArray;
    NAMTitleId* pTitleIdArrayInstalled;
    s32 result = TRUE;
    s32 i,j;

    // タイトル数取得
    title_num = (u32)NAM_GetNumTitles();
    title_num_installed = (u32)NAM_GetNumInstalledTitles();

    // タイトルID配列用メモリ確保
    pTitleIdArray          = NAMUT_Alloc(sizeof(NAMTitleId)*(title_num + title_num_installed));
    pTitleIdArrayInstalled = NAMUT_Alloc(sizeof(NAMTitleId)*title_num_installed);

    if (pTitleIdArray == NULL || pTitleIdArrayInstalled == NULL)
    {
        OS_TWarning("Allocation failed in %s\n", __func__);
        NAMUT_Free(pTitleIdArray);
        NAMUT_Free(pTitleIdArrayInstalled); 
        return FALSE;
    }

    // タイトルリスト取得
    if (NAM_GetTitleList(pTitleIdArray, title_num) != NAM_OK ||
        NAM_GetInstalledTitleList(pTitleIdArrayInstalled, title_num_installed) != NAM_OK)
    {
        OS_TWarning("Fail! NAM_Get*TitleList() in %s\n", __func__);
        NAMUT_Free(pTitleIdArray);
        NAMUT_Free(pTitleIdArrayInstalled); 
        return FALSE;
    }

    // NAM_GetTitleListでは削除されているがeTicketのみ存在するタイトルがリストアップされず
    // NAM_GetInstalledTitleListではSRLはあるがeTicketがないタイトルがリストアップされない。
    // そのため両者をマージする
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
        // プロテクト対象以外であればtitleId_Hiディレクトリごと消去する
        if (!(pTitleIdArray[i] & PROTECT_TITLE_PROPERTY))
        {
            // nand:/title/titleID_Hi/ 以下を消去
            STD_TSNPrintf(dirPath, NAM_PATH_LEN, "nand:/title/%08x", NAM_GetTitleIdHi(pTitleIdArray[i]) );
            if ( !FS_DeleteDirectoryAuto( dirPath ) )
            {
                result = FALSE;
            }

       	    // nand:/ticket/titleID_Hi/ 以下を消去
			{
				ETicketType eTicketType = ETICKET_TYPE_PERSONALIZED; // default
				
				if( isForceEraseCommonETicket ) {
					// 強制 CommonETicket 消去フラグが有効な場合は、全チケットを消去
	    	       	STD_TSNPrintf(dirPath, NAM_PATH_LEN, "nand:/ticket/%08x", NAM_GetTitleIdHi(pTitleIdArray[i]) );
	        	   	if ( !FS_DeleteDirectoryAuto( dirPath ) )
		            {
	    	       		result = FALSE;
	        	   	}
				}else {
					// そうでない場合は、CommonETicket 以外のタイトルの eTicket のみ消去
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

  Description:  全セーブデータの乱数クリア＆フォーマットを行います

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

    // タイトル数取得
    title_num = (u32)NAM_GetNumTitles();

    // タイトルID配列用メモリ確保
    pTitleIdArray = NAMUT_Alloc(sizeof(NAMTitleId)*title_num);
    if (pTitleIdArray == NULL)
    {
        OS_TWarning("Allocation failed in %s\n", __func__);
        return FALSE;
    }

    // タイトルリスト取得
    if (NAM_GetTitleList(pTitleIdArray, title_num) != NAM_OK)
    {
        OS_TWarning("Fail! NAM_GetTitleList() in %s\n", __func__);
        NAMUT_Free(pTitleIdArray);
        return FALSE;
    }

    for (i=0;i<title_num;i++)
    {
        // タイトル情報取得
        if( NAM_ReadTitleInfo(&namTitleInfo, pTitleIdArray[i]) == NAM_OK )
        {
            // publicSaveSizeが0以上ならフォーマット
            if (namTitleInfo.publicSaveSize > 0)
            {
                ret &= NAMUTi_MountAndFormatOtherTitleSaveData(namTitleInfo.titleId, "otherPub");
            }
            // privateSaveSizeが0以上ならフォーマット
            if (namTitleInfo.privateSaveSize > 0)
            {
                ret &= NAMUTi_MountAndFormatOtherTitleSaveData(namTitleInfo.titleId, "otherPrv");
            }

            // サブバナーファイルパス取得
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

  Description:  指定したセーブデータファイルに対して
                乱数クリア＆フォーマットを行います。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_ClearSavedataPublic(const char* path, u64 titleID)
{
    //----- 乱数クリア
    if (NAMUTi_RandClearFile(path) == FALSE)
    {
        OS_Warning(" Fail NAMUTi_RandClearFile");
        return FALSE;
    }

    //----- NANDアプリのセーブデータファイルをマウントかつフォーマット
    return NAMUTi_MountAndFormatOtherTitleSaveData(titleID, "otherPub");
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataPrivate

  Description:  指定したセーブデータファイルに対して
                乱数クリア＆フォーマットを行います。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_ClearSavedataPrivate(const char* path, u64 titleID)
{
    //----- 乱数クリア
    if (NAMUTi_RandClearFile(path) == FALSE)
    {
        OS_Warning(" Fail NAMUTi_RandClearFile");
        return FALSE;
    }

    //----- NANDアプリのセーブデータファイルをマウントかつフォーマット
    return NAMUTi_MountAndFormatOtherTitleSaveData(titleID, "otherPrv");
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DestroySubBanner

  Description:  指定したサブバナーのCRC破壊を試みます。
                指定したサブバナーが存在しない可能性もありますが
                その場合でもTRUEを返します。（コードはOS_DeleteSubBannerFileのパクリ）

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUTi_DestroySubBanner(const char* path)
{
    TWLSubBannerFile* pBanner;
    u16 crc, solt;
    FSFile file[1];
    BOOL ret = FALSE;

    // R属性でファイルをオープンを試みてファイルの存在有無を確認する
    // 存在しない場合はTRUEで返す
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

    // RWL属性で開く
    FS_InitFile(file);
    if ( !FS_OpenFileEx(file, path, FS_FILEMODE_RWL) )
    {
        OS_Warning("banner file open failed.\n");
        NAMUT_Free( pBanner );
        return FALSE;
    }

    // CRCを改竄して書き戻す
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

  Description:  指定NANDアプリのセーブデータファイルをマウントかつフォーマット。

  Arguments:    titleID : タイトルID
                arcname : "otherPub"、"otherPriv"、またはNULL

  Returns:      成功すればTRUE
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

    // マウント試行。
    result = FSi_MountSpecialArchive(titleID, arcname, pWork);
    if (result != FS_RESULT_SUCCESS)
    {
        OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
    }
    else
    {
        // 成功したらフォーマット試行。
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
        // アンマウント。
        (void)FSi_MountSpecialArchive(titleID, NULL, pWork);
    }

    NAMUT_Free ( pWork );

    return succeeded;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_InitShareData

  Description:  全Shareデータファイルをフォーマット。
                この関数実行前にShare:/がマウントされていてはいけない。

  Arguments:    None

  Returns:      成功すればTRUE
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
        // ファイルの存在を確認
        STD_TSNPrintf(path, NAM_PATH_LEN, "nand:/shared2/000%d", i);
        if (NAMUTi_CheckExistenceFile(path) == FALSE)
        {
            // ファイルが存在しないものとみなす
            OS_TPrintf("%s is not exist\n", path);
            continue;
        }

        // マウント
        result = FSi_MountSpecialArchive((OSTitleId)i, "share", pWork);
        if (result != FS_RESULT_SUCCESS)
        {
            succeeded = FALSE;
            OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
            continue;
        }

        // 再フォーマット
        result = FSi_FormatSpecialArchive("share:/");
        if (result != FS_RESULT_SUCCESS)
        {
            succeeded = FALSE;
            OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
        }

        // アンマウント
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

  Description:  指定したファイルを乱数で埋めます。

  Arguments:    path

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_RandClearFile(const char* path)
{
    FSFile file;

    // ファイル構造体初期化
    FS_InitFile(&file);

    // ファイルオープン
    if (FS_OpenFileEx(&file, path, (FS_FILEMODE_RWL)))
    {
        // ファイルをランダムデータでクリア
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

  Description:  指定したファイルが存在するかどうかを判定します。

  Arguments:    path

  Returns:      存在するならTRUE
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

  Description:  NANDの InstalledSoftBoxCount、FreeSoftBoxCountを調べて
                指定された変数に格納します。

  Arguments:    installed : installedカウント格納変数
                free      : freeカウント格納変数

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_GetSoftBoxCount( u8* installed, u8* free )
{
    u32 installedSoftBoxCount;

    // installedSoftBoxCountの取得
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

  Description:  InstalledSoftBoxCount, FreeSoftBoxCount の値を
                現在のNANDの状態に合わせて更新します。

  Arguments:    None.

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_UpdateSoftBoxCount( void )
{
    u8 installedSoftBoxCount;
    u8 freeSoftBoxCount;
    u8 *pBuffer;
    BOOL retval = TRUE;

    // InstalledSoftBoxCount, FreeSoftBoxCount を数えなおす
    if (!NAMUT_GetSoftBoxCount(&installedSoftBoxCount, &freeSoftBoxCount))
    {
        return FALSE;
    }

    // LCFGライブラリの静的変数に対する更新
    LCFG_TSD_SetInstalledSoftBoxCount( installedSoftBoxCount ); 
    LCFG_TSD_SetFreeSoftBoxCount( freeSoftBoxCount );

    // LCFGライブラリの静的変数の値をNANDに反映
    pBuffer = NAMUT_Alloc( LCFG_WRITE_TEMP );
    if (!pBuffer) { return FALSE; }
    // ミラーリングデータの両方に書き込みを行う。
    retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
    retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
    NAMUT_Free( pBuffer );

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_ClearTWLSettings

  Description:  TWL本体設定データのクリアを行います。

  Arguments:    None.

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_ClearTWLSettings( BOOL doWriteback )
{
    BOOL retval = TRUE;
    
    // 本体設定データのクリア。TPキャリブレーションデータは残す。
    {
        LCFGTWLTPCalibData calib;
        u32 installedSoftBoxCount;

        // installedSoftBoxCountの取得
        if ( NAM_OK != NAM_GetInstalledSoftBoxCount( &installedSoftBoxCount ) ) {
            return FALSE;
        }

        LCFG_TSD_GetTPCalibration( &calib );
        LCFG_ClearTWLSettings( (u8)installedSoftBoxCount );
        LCFG_TSD_SetTPCalibration( &calib );
    }

    // LCFGライブラリの静的変数の値をNANDに反映
    if( doWriteback ) {
        u8 *pBuffer = NAMUT_Alloc( LCFG_WRITE_TEMP );
        if (!pBuffer) { return FALSE; }
        // ミラーリングデータの両方に書き込みを行う。
        retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
        retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
        NAMUT_Free( pBuffer );
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearWiFiSettings

  Description:  NVRAMのWiFi設定データをクリアする。

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

  Description:  ショップのアカウント情報を消去します。

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

    // タイトル数取得
    title_num = (u32)NAM_GetNumTitles();

    // タイトルID配列用メモリ確保
    pTitleIdArray = NAMUT_Alloc(sizeof(NAMTitleId)*title_num);
    if (pTitleIdArray == NULL)
    {
        OS_TWarning("Allocation failed in %s\n", __func__);
        return FALSE;
    }

    // タイトルリスト取得
    if (NAM_GetTitleList(pTitleIdArray, title_num) != NAM_OK)
    {
        OS_TWarning("Fail! NAM_GetTitleList() in %s\n", __func__);
        NAMUT_Free(pTitleIdArray);
        return FALSE;
    }

	// ショップを検索する
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

  Description:  メモリの確保を行います

  Arguments:    size: サイズバイト

  Returns:      確保したメモリへのポインタ
 *---------------------------------------------------------------------------*/
static void* NAMUT_Alloc(u32 size)
{
    const u32 allocSize = MATH_ROUNDUP32(size);
    SDK_ASSERTMSG( spAllocFunc != NULL, "NAMUT_Init should be called previously.\n");
    return spAllocFunc(allocSize);
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Free

  Description:  メモリの解放を行います

  Arguments:    buffer: 解放バッファ

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

  Description:  指定された titleID の eTicket タイプを取得する

  Arguments:    titleID: common eTicket かどうかを調べたいタイトルの titleID
	            pETicketType : 結果を格納するESETicketTypeポインタ

  Returns:      NAM_OK  : 取得成功
	            それ以外: 取得失敗
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
			// 先頭 eTicket の deviceId が 0x00000000 なら、common eTicket と判断。
			// ※全ての eTicket を舐める必要はない？
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

  Description:  指定されたタイトルの eTicket を取得
　　　　　　　　※nam_title.c の GetTicketViews 関数をコピペ

  Arguments:    pptv       : 取得成功時に eTicket リストのポインタを格納するポインタ
	            pNumTicket : 取得成功時に eTicket 数を格納するポインタ
                titleID    : eTicket を取得したいタイトルの titleID

  Returns:      NAM_OK     : 取得成功
	            それ以外   : 取得失敗
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
