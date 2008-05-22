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

// 擬似フォーマットで消去しないタイトルを
// TitleProperty (TitleID Hiの下位16bit）のビットで指定します。
// どれか1つでもビットが立っていれば消去の対象から外します。
#define PROTECT_TITLE_PROPERTY  (TITLE_ID_APP_TYPE_MASK)

#define DIRECTORY_DEPTH_MAX      16  // ディレクトリの深さの最大（NANDの正規構成としては最大6）
#define TITLE_ID_HI_SIZE          8
#define TITLE_LIST_MAX          256
#define CLEAR_DATA_SIZE         256

// 擬似フォーマット実行時に
// 指定ディレクトリ以下は全て消去されます。
// 指定ディレクトリ自体は残ります。
static const char* sDeleteDirectoryList[] =
{
	"nand:/shared2",
	"nand2:/photo",
	"nand:/tmp"
};

// 擬似フォーマット実行時に
// 指定ファイルはランダムデータでFillされます。
static const char* sFillFileList[] =
{
	"nand:/shared1/TWLCFG0.dat",
	"nand:/shared1/TWLCFG1.dat"
};

static u8  sClearData[CLEAR_DATA_SIZE] ATTRIBUTE_ALIGN(32);
static u32 sNCFGAddr;

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static NAMUTAlloc spAllocFunc;
static NAMUTFree  spFreeFunc;
static FSDirectoryEntryInfo sEntryInfo;
static NAMTitleId sTitleIdArray[TITLE_LIST_MAX];
static char sCurrentFullPath[FS_ENTRY_LONGNAME_MAX];

/*---------------------------------------------------------------------------*
    内部関数宣言
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

  Description:  NAMUT ライブラリの初期化を行います。

  Arguments:    allocFunc:  メモリ確保関数へのポインタ。
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

  Description:  NANDの擬似フォーマット
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

	// プロテクトタイトルのセーブデータをフォーマットします
	if (!NAMUTi_ClearSavedataAll())
	{
		ret = FALSE;
		OS_TWarning("Fail! NAMUTi_ClearSavedataAll()\n");
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

	// SoftBoxCountの更新を行います
	NAMUT_UpdateSoftBoxCount();

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
    // マウント試行。
    FSResult    result = FS_MountOtherTitleArchive(titleID, arcname);
    if (result != FS_RESULT_SUCCESS)
    {
        OS_TWarning("FS_MountOtherTitleArchive failed. (%d)\n", result);
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
        (void)FS_MountOtherTitleArchive(titleID, NULL);
    }
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

  Description:  NANDの InstalledSoftBoxCount、FreeSoftBoxCountを調べて
                指定された変数に格納します。

  Arguments:    installed : installedカウント格納変数
				free      : freeカウント格納変数

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL NAMUT_GetSoftBoxCount( u8* installed, u8* free )
{
	s32 title_num;	
	NAMTitleInfo namTitleInfo;
	u8 count = 0;
	BOOL result = TRUE;
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
			// NOT_LAUNCH_FLAG または DATA_ONLY_FLAG が立っているタイトルはカウントしない
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

	// InstalledSoftBoxCount, FreeSoftBoxCount を数えなおす
	if (!NAMUT_GetSoftBoxCount(&installedSoftBoxCount, &freeSoftBoxCount))
	{
		return FALSE;
	}

//	OS_Printf("installedSoftBoxCount = %d\n", installedSoftBoxCount);
//	OS_Printf("freeSoftBoxCount      = %d\n", freeSoftBoxCount);

	// LCFGライブラリの静的変数に対する更新
    LCFG_TSD_SetInstalledSoftBoxCount( installedSoftBoxCount );	
    LCFG_TSD_SetFreeSoftBoxCount( freeSoftBoxCount );

	// LCFGライブラリの静的変数の値をNANDに反映
    pBuffer = spAllocFunc( LCFG_WRITE_TEMP );
	if (!pBuffer) { return FALSE; }
    (void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
    spFreeFunc( pBuffer );

	return TRUE;
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

  Description:  ツリー情報をプリント出力します（ディレクトリ用）

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

  Description:  ツリー情報をプリント出力します（ファイル用）

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
