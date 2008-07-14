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
// レイヤー的にはNAMと重なりますのでNAMに吸収しても良いと思います。

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
    定数定義
 *---------------------------------------------------------------------------*/

// 本体初期化(NAND初期化)で消去しないタイトルを
// TitleProperty (TitleID 32bit）のビットで指定します。
// どれか1つでもビットが立っていれば消去の対象から外します。
#define PROTECT_TITLE_PROPERTY  (TITLE_ID_APP_TYPE_MASK)

#define DIRECTORY_DEPTH_MAX      16  // ディレクトリの深さの最大（NANDの正規構成としては最大6）
#define TITLE_ID_HI_SIZE          8
#define TITLE_LIST_MAX          256
#define CLEAR_DATA_SIZE        4096  // ファイル消去データ用（512の倍数で大きいほど処理が早い）

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
	"nand:/shared1/TWLCFG0.dat",
	"nand:/shared1/TWLCFG1.dat"
};

#define VOLUME_INITIAL_VALUE     5		// 本体初期化時に設定する本体ボリューム値
#define BACKLIGHT_INITIAL_VALUE  3		// 本体初期化時に設定するバックライト輝度
#define NAMUT_SHARE_ARCHIVE_MAX  6      // shareデータ個数

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static NAMUTAlloc spAllocFunc;
static NAMUTFree  spFreeFunc;
static FSDirectoryEntryInfo sEntryInfo;
static NAMTitleId sTitleIdArray[TITLE_LIST_MAX];
static char sCurrentFullPath[FS_ENTRY_LONGNAME_MAX];
static u32 sNCFGAddr;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static BOOL NAMUTi_DeleteNonprotectedTitle(void);
static BOOL NAMUTi_DeleteNonprotectedTitleEntity(const char* path);
static BOOL NAMUTi_ClearSavedataAll(void);
static BOOL NAMUTi_InitShareData(void);
static BOOL NAMUTi_MountAndFormatOtherTitleSaveData(u64 titleID, const char *arcname);
static void NAMUTi_DrawNandTree(s32 depth, const char *path);
static BOOL NAMUTi_RandClearFile(const char* path);
static void NAMUTi_ClearWiFiSettings( void );
static void PrintDirectory(s32 depth, const char* path);
static void PrintFile(s32 depth, const char* path);

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
               （システム系の必要なファイルのみを残し他を消去します）

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format(void)
{
	int i;
	BOOL ret = TRUE;

	// プロテクトされていないタイトルの削除を行います
	if (!NAMUTi_DeleteNonprotectedTitle())
	{
		ret = FALSE;
		OS_TWarning("Fail! NAMUTi_DeleteNonprotectedTitle()\n");
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
	NAMUTi_ClearWiFiSettings();

	// 本体設定データのクリア
	ret &= NAMUT_ClearTWLSettings( TRUE );

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
  Name:         NAMUTi_DeleteNonprotectedTitle

  Description:  User App タイトルの削除を行います。

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

	// タイトルリスト取得
	if (NAM_GetTitleList(sTitleIdArray, TITLE_LIST_MAX) != NAM_OK)
	{
		OS_TWarning("Fail! NAM_GetTitleList() in %s\n", __func__);
		return FALSE;
	}
	
	// タイトル数取得
	title_num = NAM_GetNumTitles();

	for (i=0;i<title_num;i++)
	{
		// タイトル情報取得
	    if( NAM_ReadTitleInfo(&namTitleInfo, sTitleIdArray[i]) == NAM_OK )
	    {
			// プロテクト対象以外であればtitleId_Hiディレクトリごと消去する
			if (!(namTitleInfo.titleId & PROTECT_TITLE_PROPERTY))
			{
				// nand:/title/titleID_Hi/ 以下を消去
		    	STD_TSNPrintf(dirPath, NAM_PATH_LEN, "nand:/title/%08x", NAM_GetTitleIdHi(namTitleInfo.titleId) );
				if ( !FS_DeleteDirectoryAuto( dirPath ) )
				{
					result = FALSE;
				}
				// nand:/ticket/titleID_Hi/ 以下を消去
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

  Description:  全セーブデータの乱数クリア＆フォーマットを行います

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

	// タイトルリスト取得
	if (NAM_GetTitleList(sTitleIdArray, TITLE_LIST_MAX) != NAM_OK)
	{
		return FALSE;
	}
	
	// タイトル数取得
	title_num = NAM_GetNumTitles();

	for (i=0;i<title_num;i++)
	{
		// タイトル情報取得
	    if( NAM_ReadTitleInfo(&namTitleInfo, sTitleIdArray[i]) == NAM_OK )
	    {
			// セーブファイルパス取得
			if (NAM_GetTitleSaveFilePath(savePublicPath, savePrivatePath, sTitleIdArray[i]) == NAM_OK)
			{
				// publicSaveSizeが0以上なら乱数クリア＆フォーマット
				if (namTitleInfo.publicSaveSize > 0)
				{
					ret &= NAMUTi_ClearSavedataPublic(savePublicPath, namTitleInfo.titleId);
				}
				// privateSaveSizeが0以上なら乱数クリア＆フォーマット
				if (namTitleInfo.privateSaveSize > 0)
				{
					ret &= NAMUTi_ClearSavedataPrivate(savePrivatePath, namTitleInfo.titleId);
				}
			}
			else { ret = FALSE; }

			// サブバナーファイルパス取得
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

	if ( !spAllocFunc || !spFreeFunc )
	{
		OS_Warning("NAMUT_Init should be called previously.");		
		return FALSE;
	}

	pBanner = spAllocFunc( sizeof(TWLSubBannerFile) );
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
		spFreeFunc( pBanner );
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
	
	spFreeFunc( pBanner );
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

	if (!spAllocFunc || !spFreeFunc) 
	{
		return FALSE;
	}

	pWork = spAllocFunc( sizeof(FSFATFSArchiveWork) );
	if (!pWork)
	{
        OS_TWarning("Allocation failed. (%d)\n");
		return FALSE;
	}
	MI_CpuClear8( pWork, sizeof(FSFATFSArchiveWork) );

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
        // ドライブ情報をダンプ。
//      DumpArchiveResource(path);
        // アンマウント。
        (void)FSi_MountSpecialArchive(titleID, NULL, pWork);
    }

    spFreeFunc ( pWork );

    return succeeded;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_InitShareData

  Description:  全Shareデータファイルを乱数クリア＆フォーマット。
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

	if (!spAllocFunc || !spFreeFunc) 
	{
		return FALSE;
	}

	pWork = spAllocFunc( sizeof(FSFATFSArchiveWork) );
	if (!pWork)
	{
        OS_TWarning("Allocation failed. (%d)\n");
		return FALSE;
	}
	MI_CpuClear8( pWork, sizeof(FSFATFSArchiveWork) );

	for (i=0;i<NAMUT_SHARE_ARCHIVE_MAX;i++)
	{
		// 乱数クリア
		STD_TSNPrintf(path, NAM_PATH_LEN, "nand:/shared2/000%d", i);
		if (NAMUTi_RandClearFile(path) == FALSE)
		{
			// ファイルが存在しないものとみなし終了
	        OS_TPrintf("%s is not exist\n", path);
			break;
		}

	    // マウント
		result = FSi_MountSpecialArchive((OSTitleId)i, "share", pWork);
	    if (result != FS_RESULT_SUCCESS)
	    {
			succeeded = FALSE;
	        OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
			continue;
	    }

        // Mediaフォーマット
        if (!FATFSi_FormatMedia("share:/"))
        {
			succeeded = FALSE;
            OS_TWarning("FATFSi_FormatMedia failed. (%d)\n", FATFS_GetLastError());
			FSi_MountSpecialArchive(0, NULL, pWork);
			continue;
        }

		// Driveフォーマット
        if (!FATFS_FormatDrive("share:/"))
        {
			succeeded = FALSE;
            OS_TWarning("FATFS_FormatDrive failed. (%d)\n", FATFS_GetLastError());
			FSi_MountSpecialArchive(0, NULL, pWork);
			continue;
        }

		// アンマウント
		result = FSi_MountSpecialArchive((OSTitleId)i, NULL, pWork);
        if (result != FS_RESULT_SUCCESS)
		{
			succeeded = FALSE;
            OS_TWarning("FSi_MountSpecialArchive failed. (%d)\n", result);
			continue;
		}
	}

    spFreeFunc ( pWork );

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
		u32 filesize = FS_GetFileLength(&file);
        u8* pClearData = spAllocFunc( CLEAR_DATA_SIZE );
		if (!pClearData)
		{
	        OS_TWarning("Allocation failed. (%d)\n");
			FS_CloseFile(&file);
			return FALSE;
		}

		for (; filesize > CLEAR_DATA_SIZE; filesize -= CLEAR_DATA_SIZE)
		{
	        if( AES_Rand(pClearData, CLEAR_DATA_SIZE) != AES_RESULT_SUCCESS )
	        {
                spFreeFunc( pClearData );
	            return FALSE;
	        }
			FS_WriteFile(&file, pClearData, CLEAR_DATA_SIZE);
		}
		FS_WriteFile(&file, pClearData, (s32)filesize);
		FS_CloseFile(&file);
        spFreeFunc( pClearData );
	}
	else
	{
		return FALSE;
	}
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
    pBuffer = spAllocFunc( LCFG_WRITE_TEMP );
	if (!pBuffer) { return FALSE; }
	// ミラーリングデータの両方に書き込みを行う。
	retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
	retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
    spFreeFunc( pBuffer );

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
		u8 *pBuffer = spAllocFunc( LCFG_WRITE_TEMP );
		if (!pBuffer) { return FALSE; }
		// ミラーリングデータの両方に書き込みを行う。
	    retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
	    retval &= LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
    	spFreeFunc( pBuffer );
	}

	return retval;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DrawNandTree

  Description:  NANDのツリー情報をプリント出力します

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NAMUT_DrawNandTree(void)
{
	MI_CpuClear8( sCurrentFullPath, sizeof(sCurrentFullPath) );
	NAMUTi_DrawNandTree(0, "nand:");
	NAMUTi_DrawNandTree(0, "nand2:");
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DrawNandTree

  Description:  指定パスのツリー情報をプリント出力します（再帰関数）

  Arguments:    path : 絶対パス指定(スラッシュを含めない）

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

	// 深さ制限
	if (depth > DIRECTORY_DEPTH_MAX)
	{
		OS_Warning("Fail! Depth is too deep.\n");
		return;
	}

	// カレントパスを設定    
	STD_CopyLString( sCurrentFullPath, path, FS_ENTRY_LONGNAME_MAX );

	FS_InitFile(&dir);

	// 引数で指定されたディレクトリを開く
	if (!FS_OpenDirectory(&dir, sCurrentFullPath, (FS_FILEMODE_R)))
	{
		OS_Warning("%d Fail! FS_OpenDirectory(%s)\n", __LINE__, sCurrentFullPath);
		return;
	}

	// ディレクトリの中身を読む
	while (FS_ReadDirectory(&dir, &sEntryInfo))
	{
        if (STD_CompareString(sEntryInfo.longname, ".")  == 0 ||
            STD_CompareString(sEntryInfo.longname, "..") == 0)
        {
            continue;
        }

		// ディレクトリであれば再帰呼び出し
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

	// カレントパスを削る
	if (pSlash != NULL)
	{
		*pSlash = '\0';
	}

	FS_CloseDirectory(&dir);
}


/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearWiFiSettings

  Description:  NVRAMのWiFi設定データをクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NAMUTi_ClearWiFiSettings( void )
{
#define NCFG_ADDR			0x20
#define NTR_WIFI_DATA_SIZE	0x400
#define TWL_WIFI_DATA_SIZE	0x600
	u32 total_size = ( NTR_WIFI_DATA_SIZE + TWL_WIFI_DATA_SIZE );
	u8* pClearData = spAllocFunc( total_size );
	if (!pClearData)
	{
        OS_TWarning("Allocation failed. (%d)\n");
		return;
	}

    if (!NVRAMi_IsInitialized()) {
        NVRAMi_Init();
    }
    DC_FlushRange( &sNCFGAddr, 2 );
	sNCFGAddr = 0;
    NVRAMi_Read( NCFG_ADDR, 2, (u8 *)&sNCFGAddr);
	sNCFGAddr = (u32)( ( sNCFGAddr << 3 ) - ( NTR_WIFI_DATA_SIZE + TWL_WIFI_DATA_SIZE ) );
	
	MI_CpuFillFast( pClearData, 0xffffffff, total_size);
    DC_FlushRange( pClearData, total_size );
    NVRAMi_Write( sNCFGAddr, total_size , pClearData );

	spFreeFunc( pClearData );
}


/*---------------------------------------------------------------------------*
  Name:         PrintDirectory

  Description:  ツリー情報をプリント出力します（ディレクトリ用）

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintDirectory(s32 depth, const char* path)
{
#ifdef SDK_FINALROM
#pragma unused(path)
#endif
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

  Description:  ツリー情報をプリント出力します（ファイル用）

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintFile(s32 depth, const char* filename)
{
#ifdef SDK_FINALROM
#pragma unused(filename)
#endif
	int i;

	for (i=0; i<depth+1; i++)
	{
		OS_TPrintf("   ");
	}

	OS_TPrintf("l-- ");

	OS_TPrintf("%s\n", filename);
}
