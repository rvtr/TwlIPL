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

// Nand Application Management UTility ライブラリ、略してNAMUTライブラリです。
// レイヤー的にはNAMと重なりますのでNAMに吸収しても良いと思います。

#include <twl.h>
#include <twl/fatfs.h>
#include <twl/os/common/format_rom.h>
#include <twl/nam.h>
#include <sysmenu/namut.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

// クリーンアップ実行時に
// 指定ディレクトリ以下は全て消去されます。
// 指定ディレクトリ自体は残ります
static const char* sDeleteDirectoryList[] =
{
	"nand:/tmp"
};

// クリーンアップ実行時に
// 指定ファイルは0xffでFillされます。
static const char* sFillFileList[] =
{
	"nand:/shared1/TWLCFG0.dat",
	"nand:/shared1/TWLCFG1.dat"
};

// クリーンアップで消去しないタイトルを
// TitleProperty (TitleID Hiの下位16bit）のビットで指定します。
// どれか1つでもビットが立っていれば消去の対象から外します。
#define PROTECT_TITLE_PROPERTY  (TITLE_ID_HI_APP_TYPE_MASK)

#define DIRECTORY_DEPTH_MAX      16  // ディレクトリの深さの最大（NANDの正規構成としては最大6）
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
    内部変数定義
 *---------------------------------------------------------------------------*/

static FSDirectoryEntryInfo sEntryInfo;
static NAMTitleId sTitleIdArray[TITLE_LIST_MAX];

/*---------------------------------------------------------------------------*
    内部関数宣言
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

  Description:  NANDのクリーンアップ
               （システム系の必要なファイルのみを残し他を消去します）

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format(void)
{
	int i;
	BOOL ret = TRUE;

	// プロテクトされていないタイトルの削除を行います
	ret &= NAMUTi_DeleteNonprotectedTitle();

	// プロテクトタイトルのセーブデータをフォーマットします
	ret &= NAMUTi_ClearSavedataAll(TRUE);

	// 指定ファイルを0xffでクリアします
	for (i=0; i<sizeof(sFillFileList)/sizeof(char*); i++)
	{
		ret &= NAMUTi_FillFile(sFillFileList[i]);
	}

	// temp以下を消去します
	// NAM関数でtempが作成&使用される可能性があるため最後に実行します
	for (i=0; i<sizeof(sDeleteDirectoryList)/sizeof(char*); i++)
	{
		ret &= NAMUTi_DeleteNandDirectory(sDeleteDirectoryList[i]);
	}

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteNandDirectory

  Description:  指定ディレクトリ以下を消去します。
                指定ディレクトリ自体は残ります。

  Arguments:    path : 絶対パス

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_DeleteNandDirectory(const char *path)
{
    FSFile  dir;
	FSDirectoryEntryInfo entryInfo;
	BOOL ret = TRUE;

	FS_InitFile(&dir);

	// 引数で指定されたディレクトリを開く
	if (!FS_OpenDirectory(&dir, path, FS_FILEMODE_R))
	{
		SDK_ASSERTMSG(0, "Fail! FS_OpenDirectory(%s) in %s\n", path, __FUNC__);
		return FALSE;
	}

	// カレントディレクトリを設定する
	if (!FS_SetCurrentDirectory(path))
	{
		SDK_ASSERTMSG(0, "Fail! FS_SetCurrentDirectory(%s) in %s\n", path, __FUNC__);
		FS_CloseDirectory(&dir);
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

		// ディレクトリ
		if (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
		{
			ret &= FS_DeleteDirectoryAuto(entryInfo.longname);
			SDK_ASSERTMSG(ret, "Fail! FS_DeleteDirectoryAuto(%s) in %s\n", entryInfo.longname, __FUNC__);
		}
		// ファイル
		else
		{
			ret &= FS_DeleteFileAuto(entryInfo.longname);
			SDK_ASSERTMSG(ret, "Fail! FS_DeleteFileAuto(%s) in %s\n", entryInfo.longname, __FUNC__);
		}
	}

	// カレントディレクトリを元に戻します
	ret &= FS_SetCurrentDirectory("..");
	SDK_ASSERTMSG(ret, "Fail! FS_SetCurrentDirectory(..) in %s\n", __FUNC__);

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
	BOOL ret = TRUE;

	ret &= NAMUTi_DeleteNonprotectedTitleEntity(TITLE_PATH);
	ret &= NAMUTi_DeleteNonprotectedTitleEntity(TICKET_PATH);
	ret &= NAMUTi_DeleteNonprotectedTitleEntity(IMPORT_PATH);

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DeleteNonprotectedTitleEntity

  Description:  User App タイトルの削除を行います。

  Arguments:    path

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_DeleteNonprotectedTitleEntity(const char* path)
{
    FSFile  dir;
	FSDirectoryEntryInfo entryInfo;
	BOOL ret = TRUE;

	FS_InitFile(&dir);

	// 引数で指定されたディレクトリを開く
	if (!FS_OpenDirectory(&dir, path, FS_FILEMODE_R))
	{
		SDK_ASSERTMSG(0, "Fail! FS_OpenDirectory(%s) in %s\n", path, __FUNC__);
		return FALSE;
	}

	// カレントディレクトリを設定する
	if (!FS_SetCurrentDirectory(path))
	{
		SDK_ASSERTMSG(0, "Fail! FS_SetCurrentDirectory(%s) in %s\n", path, __FUNC__);
		FS_CloseDirectory(&dir);
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

		// ディレクトリの場合
		if (entryInfo.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
		{
			s8 titlePropety = (s8)(entryInfo.longname[TITLE_ID_HI_SIZE-1] - '0');

			// プロテクト対象でない場合ディレクトリごと消去する
			if (!(titlePropety & PROTECT_TITLE_PROPERTY))
			{
				ret &= FS_DeleteDirectoryAuto(entryInfo.longname);
				SDK_ASSERTMSG(ret, "Fail! FS_DeleteDirectoryAuto(%s) in %s\n", entryInfo.longname, __FUNC__);
			}
		}
	}

	// カレントディレクトリを元に戻します
	ret &= FS_SetCurrentDirectory("..");
	SDK_ASSERTMSG(ret, "Fail! FS_SetCurrentDirectory(..) in %s\n", __FUNC__);

	// ディレクトリを閉じる
	ret &= FS_CloseDirectory(&dir);
	SDK_ASSERTMSG(ret, "Fail! FS_CloseDirectory() in %s\n", __FUNC__);

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_ClearSavedataAll

  Description:  全セーブデータのＦＦクリア＆フォーマットを行います

  Arguments:    fill : FFクリアを行うならTRUEを指定します

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
				// publicSaveSizeが0以上なら0xFFクリア＆フォーマット
				if (namTitleInfo.publicSaveSize > 0)
				{
					ret &= NAMUTi_ClearSavedata(savePublicPath, fill);
				}
				// privateSaveSizeが0以上なら0xFFクリア＆フォーマット
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

  Description:  指定したセーブデータファイルに対して
				ＦＦクリア＆フォーマットを行います。

  Arguments:    fill : FFクリアを行うならTRUEを指定します

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_ClearSavedata(const char* path, BOOL fill)
{
	char drive[2] = { 'Z', '\0' };
	FSFile file;
	BOOL ret = FALSE;

	// ファイル構造体初期化
    FS_InitFile(&file);

	// セーブファイルオープン
	if (!FS_OpenFileEx(&file, path, (FS_FILEMODE_R|FS_FILEMODE_W)))
	{
		return FALSE;
	}

	// セーブファイルを0xFFでクリア
	if (fill)
	{
		u32 filesize = FS_GetFileLength(&file);
		for (; filesize > CLEAR_DATA_SIZE; filesize -= CLEAR_DATA_SIZE)
		{
			FS_WriteFile(&file, sClearData, CLEAR_DATA_SIZE);
		}
		FS_WriteFile(&file, sClearData, (s32)filesize);
	}

	// セーブファイルに対してマウント
	for (drive[0]='Z'; drive[0]>='A'; drive[0]--)
	{
		if (FATFS_MountDrive(drive, FATFS_MEDIA_TYPE_SUBNAND, (FATFSFileHandle)file.userdata))
		{
			break;
		}
	}
	
	// マウント成功ならフォーマットする
	if (drive[0] >= 'A')
	{
		char drive_colon[3];
		drive_colon[0] = drive[0];
		drive_colon[1] = ':';
		drive_colon[2] = '\0';

		// メディアフォーマット
		if (FATFSi_FormatMedia(drive_colon))
		{
			// ドライブフォーマット
			if (FATFS_FormatDrive(drive_colon))
			{
				ret = TRUE;
			}
		}

		// アンマウント
		FATFS_UnmountDrive(drive);
	}

	// ファイルクローズ
	FS_CloseFile(&file);

	return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_FillFile

  Description:  指定したファイルを0xFFで埋めます。

  Arguments:    path

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL NAMUTi_FillFile(const char* path)
{
	FSFile file;

	// ファイル構造体初期化
    FS_InitFile(&file);

	// ファイルオープン
	if (FS_OpenFileEx(&file, path, (FS_FILEMODE_R|FS_FILEMODE_W)))
	{
		// ファイルを0xFFでクリア
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

  Description:  NANDのツリー情報をプリント出力します

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/

void NAMUT_DrawNandTree(void)
{
	NAMUTi_DrawNandTree(0, "nand:/");
}

/*---------------------------------------------------------------------------*
  Name:         NAMUTi_DrawNandTree

  Description:  指定パスのツリー情報をプリント出力します（再帰関数）

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void NAMUTi_DrawNandTree(s32 depth, const char *path)
{
    FSFile  dir;
	
	PrintDirectory(depth, path);

	// 深さ制限
	if (depth > DIRECTORY_DEPTH_MAX)
	{
		OS_TPrintf("Fail! Depth is too deep.\n");
		return;
	}

	FS_InitFile(&dir);

	// 引数で指定されたディレクトリを開く
	if (!FS_OpenDirectory(&dir, path, (FS_FILEMODE_R|FS_FILEMODE_W)))
	{
		OS_TPrintf("%d Fail! FS_OpenDirectory(%s)\n", __LINE__, path);
		return;
	}

	// カレントディレクトリを設定する
	if (!FS_SetCurrentDirectory(path))
	{
		OS_TPrintf("Fail! FS_SetCurrentDirectory(%s)\n", path);
		FS_CloseDirectory(&dir);
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
//			PrintFile(depth, sEntryInfo.longname);
		}
		else
		{
			NAMUTi_DrawNandTree(depth + 1, sEntryInfo.longname);
		}
	}

	// カレントディレクトリを元に戻します
	if (!FS_SetCurrentDirectory(".."))
	{
		OS_TPrintf("Fail! FS_SetCurrentDirectory(..)\n");
	}	

	FS_CloseDirectory(&dir);
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
