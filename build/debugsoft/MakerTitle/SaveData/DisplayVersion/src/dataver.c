/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - yasuda - MenuTest
  File:     dataver.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include "dataver.h"
#include <nitro/fs.h>
#include <nitro/os.h>
#include <twl/nam.h>
#include <twl/na.h>

//#define ACCESS_TEST_1
//#define ACCESS_TEST_2
//#define ACCESS_TEST_3
//#define ACCESS_TEST_4
//#define ACCESS_TEST_5

#define PUBDATA_VERSION_PATH	"dataPub:/version"
#define PRVDATA_VERSION_PATH	"dataPrv:/version"
#define OTHER_PUBDATA_VERSION_PATH	"otherPub:/version"
#define OTHER_PRVDATA_VERSION_PATH	"otherPrv:/version"
#define VERSION_LEN             3
#define OUR_COMPANY_CODE        0x3031		// "01" : Nintendo
#define SELF_INITIAL_CODE       0x34564341	//"4VCA"

static BOOL getDataInfoStrings(char *path, char *verstr, char *timestr, char *datestr);
static BOOL setDataTimeStamp(char *path);
static BOOL setOurNandAppDataTimeStamp(s32 no, BOOL pub);

FSResult g_fs_result;
static s32        g_title_num;
static NAMTitleId g_title[40];

#ifdef USER_MODE
static const char* TARGET_APP = "4VDA";
#endif

void GetAppVersionString(char *str)
{
	u8 *pHeader, version;

	pHeader = (u8*)HW_TWL_ROM_HEADER_BUF;
	version = *(pHeader + 0x1e);

	str[0] = (char)(version / 100 + 0x30);
	str[1] = (char)((version % 100) / 10 + 0x30);
	str[2] = (char)(version % 10 + 0x30);
	str[3] = '\0';
}

BOOL GetPublicDataInfoStrings(char *verstr, char *timestr, char *datestr)
{
	return getDataInfoStrings(PUBDATA_VERSION_PATH, verstr, timestr, datestr);
}

static BOOL getDataInfoStrings(char *path, char *verstr, char *timestr, char *datestr)
{
	FSFile      file;
	FSPathInfo	info;
	FSResult    fresult;

	FS_InitFile( &file );

	if( !FS_GetPathInfo(path, &info) )
	{
		fresult = FS_GetArchiveResultCode(path);
		return FALSE;
	}

	STD_TSPrintf( timestr, "%04d/%02d/%02d",
				 info.mtime.year, info.mtime.month, info.mtime.day );
	STD_TSPrintf( datestr, "%02d:%02d:%02d",
				 info.mtime.hour, info.mtime.minute, info.mtime.second );

	if( FS_OpenFileEx( &file, path, FS_FILEMODE_R ) == FALSE )
	{
		return FALSE;
	}

	if( FS_ReadFile( &file, verstr, VERSION_LEN ) != VERSION_LEN )
	{
		return FALSE;
	}
	verstr[VERSION_LEN] = '\0';

	if( FS_CloseFile( &file ) == FALSE )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL GetPrivateDataInfoStrings(char *verstr, char *timestr, char *datestr)
{
	return getDataInfoStrings(PRVDATA_VERSION_PATH, verstr, timestr, datestr);
}

BOOL SetPublicDataTimeStamp()
{
	return setDataTimeStamp(PUBDATA_VERSION_PATH);
}

static BOOL setDataTimeStamp(char *path)
{
	FSFile      file;
	//FSPathInfo	info;
	char        verstr[8];
	FSResult    fresult;

	GetAppVersionString( verstr );

	FS_InitFile( &file );

	if( FS_OpenFileEx( &file, path, FS_FILEMODE_W ) == FALSE )
	{
		if( !FS_CreateFile(path, FS_PERMIT_R | FS_PERMIT_W) )
		{
			fresult = FS_GetArchiveResultCode(path);
			g_fs_result = fresult;
			return FALSE;
		}
		if( FS_OpenFileEx( &file, path, FS_FILEMODE_W ) == FALSE )
		{
			return FALSE;
		}
	}

	if( FS_WriteFile( &file, verstr, VERSION_LEN ) != VERSION_LEN )
	{
		return FALSE;
	}

	if( FS_CloseFile( &file ) == FALSE )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL SetPrivateDataTimeStamp()
{
	return setDataTimeStamp(PRVDATA_VERSION_PATH);
}

s32 GetOutNandAppInfo(void)
{
	s32           i, j;
	s32           appnum, ret;
	NAMTitleInfo  info;
	NAMTitleId    tmptitle[40];

	NAM_Init(OS_AllocFromMain, OS_FreeToMain);

	appnum = NAM_GetNumTitles();
	if( appnum < 0 )
		return -1;
	ret = NAM_GetTitleList( tmptitle, 40 );
	if( ret != NAM_OK )
		return -2;
	for( i=0, j=0; i<appnum; i++ )
	{
		if( !NAM_IsExecTitle( tmptitle[i] ) )
			continue;
		if( !NAM_IsVisibleTitle( tmptitle[i] ) )
			continue;
		ret = NAM_ReadTitleInfo( &info, tmptitle[i] );
		if( ret != NAM_OK )
			return -3;
		if( info.companyCode != OUR_COMPANY_CODE )
			continue;
		if( (NAM_GetTitleIdLo(tmptitle[i]) == SELF_INITIAL_CODE) )
			continue;
		g_title[j] = tmptitle[i];
		j++;
	}
	g_title_num = j;

	return j;
}


s32 GetOurNandAppNum(void)
{
	return g_title_num;
}

BOOL GetOurNandAppVersionString(s32 no, char *verstr, char *inistr)
{
	NAMTitleInfo  info;
	u32           ini;

	if( no >= g_title_num )
		return FALSE;

	// 該当NANDアプリのバージョンを得る
	if( NAM_ReadTitleInfo( &info, g_title[no] ) != NAM_OK )
		return FALSE;

	// str に返す
	//OS_TPrintf("app ver = 0x%x\n", info.version);
	verstr[0] = (char)(info.version / 100 + 0x30);
	verstr[1] = (char)((info.version % 100) / 10 + 0x30);
	verstr[2] = (char)(info.version % 10 + 0x30);
	verstr[3] = '\0';

	ini = NAM_GetTitleIdLo(g_title[no]);
	*(u32*)inistr = MI_SwapEndian32(ini);
	inistr[4] = '\0';

	return TRUE;
}

static BOOL getOtherNandAppDataInfoStrings(s32 no, BOOL pub, char *verstr, char *timestr, char *datestr)
{
	BOOL            ret = FALSE;
	FSFile          file;
	FSPathInfo      info;
	FSResult        fresult;
	u32             ini;
	NATitleArchive  arc;
	char            *path;
	const char      pubpath[] = OTHER_PUBDATA_VERSION_PATH;
	const char      prvpath[] = OTHER_PRVDATA_VERSION_PATH;

	ini = MI_SwapEndian32( NAM_GetTitleIdLo(g_title[no]) );
	arc = pub ? NA_TITLE_ARCHIVE_DATAPUB : NA_TITLE_ARCHIVE_DATAPRV;
#ifdef USER_MODE
	if( NA_LoadOtherTitleArchive( TARGET_APP, arc ) != FS_RESULT_SUCCESS )
		return FALSE;
#else
	if( NA_LoadOtherTitleArchive( (char*)&ini, arc ) != FS_RESULT_SUCCESS )
		return FALSE;
#endif
	path = pub ? (char*)pubpath : (char*)prvpath;

	FS_InitFile( &file );

	if( !FS_GetPathInfo(path, &info) )
	{
		fresult = FS_GetArchiveResultCode(path);
		goto quit;
	}

	STD_TSPrintf( timestr, "%04d/%02d/%02d",
				 info.mtime.year, info.mtime.month, info.mtime.day );
	STD_TSPrintf( datestr, "%02d:%02d:%02d",
				 info.mtime.hour, info.mtime.minute, info.mtime.second );

	if( FS_OpenFileEx( &file, path, FS_FILEMODE_R ) == FALSE )
	{
		goto quit;
	}

	if( FS_ReadFile( &file, verstr, VERSION_LEN ) != VERSION_LEN )
	{
		goto quit;
	}
	verstr[VERSION_LEN] = '\0';

	if( FS_CloseFile( &file ) == FALSE )
	{
		goto quit;
	}

	ret = TRUE;

quit:
	NA_UnloadOtherTitleArchive();

	return ret;
}

BOOL GetOurNandAppPublicDataInfoStrings(s32 no, char *verstr, char *timestr, char *datestr)
{
	return getOtherNandAppDataInfoStrings(no, TRUE, verstr, timestr, datestr );
}

BOOL GetOurNandAppPrivateDataInfoStrings(s32 no, char *verstr, char *timestr, char *datestr)
{
	return getOtherNandAppDataInfoStrings(no, FALSE, verstr, timestr, datestr );
}


static BOOL setOurNandAppDataTimeStamp(s32 no, BOOL pub)
{
	BOOL            ret = FALSE;
	FSFile          file;
	char            verstr[8];
	//char            inistr[8];
	FSResult        fresult;
	u32             ini;
	NATitleArchive  arc;
	char            *path;
	const char      pubpath[] = OTHER_PUBDATA_VERSION_PATH;
	const char      prvpath[] = OTHER_PRVDATA_VERSION_PATH;

	//GetOurNandAppVersionString( no, verstr, inistr );
	GetAppVersionString( verstr );
	ini = MI_SwapEndian32( NAM_GetTitleIdLo(g_title[no]) );

	arc = pub ? NA_TITLE_ARCHIVE_DATAPUB : NA_TITLE_ARCHIVE_DATAPRV;
#ifdef USER_MODE
	if( NA_LoadOtherTitleArchive( TARGET_APP, arc ) != FS_RESULT_SUCCESS )
		goto quit;
#else
	if( NA_LoadOtherTitleArchive( (char*)&ini, arc ) != FS_RESULT_SUCCESS )
		goto quit;
#endif
	path = pub ? (char*)pubpath : (char*)prvpath;

	FS_InitFile( &file );

	if( FS_OpenFileEx( &file, path, FS_FILEMODE_W ) == FALSE )
	{
		if( !FS_CreateFile(path, FS_PERMIT_R | FS_PERMIT_W) )
		{
			fresult = FS_GetArchiveResultCode(path);
			g_fs_result = fresult;
			goto quit;
		}
		if( FS_OpenFileEx( &file, path, FS_FILEMODE_W ) == FALSE )
		{
			goto quit;
		}
	}

	if( FS_WriteFile( &file, verstr, VERSION_LEN ) != VERSION_LEN )
	{
		return FALSE;
	}

	if( FS_CloseFile( &file ) == FALSE )
	{
		goto quit;
	}

	ret = TRUE;

quit:
	NA_UnloadOtherTitleArchive();

	return ret;
}
BOOL SetOurNandAppPublicDataTimeStamp(s32 no)
{
	return setOurNandAppDataTimeStamp( no, TRUE );
}

BOOL SetOurNandAppPrivateDataTimeStamp(s32 no)
{
	return setOurNandAppDataTimeStamp( no, FALSE );
}

#if 0
/* *INDENT-OFF* */
static const char  * const fs_result_strings[] =
{
    "FS_RESULT_SUCCESS",
    "FS_RESULT_FAILURE",
    "FS_RESULT_BUSY",
    "FS_RESULT_CANCELED",
    "FS_RESULT_UNSUPPORTED",
    "FS_RESULT_ERROR",
    "FS_RESULT_INVALID_PARAMETER",
    "FS_RESULT_NO_MORE_RESOUCE",
    "FS_RESULT_ALREADY_DONE",
    "FS_RESULT_PERMISSION_DENIED",
    "FS_RESULT_MEDIA_FATAL",
    "FS_RESULT_NO_ENTRY",
    "FS_RESULT_MEDIA_NOTHING",
    "FS_RESULT_MEDIA_UNKNOWN",
    "FS_RESULT_BAD_FORMAT",
};
static const size_t fs_result_string_max = sizeof(fs_result_strings) / sizeof(*fs_result_strings);
SDK_COMPILER_ASSERT(fs_result_string_max == FS_RESULT_MAX);
/* *INDENT-ON* */

static void ReportLastErrorPath(const char *path)
{
    FSResult    result = FS_GetArchiveResultCode(path);

    SDK_ASSERT((result >= 0) && (result < fs_result_string_max));
    OS_TPrintf("FS error:\n    \"%s\"\n    %s\n",
               path, fs_result_strings[result]);
}

static void ReportLastErrorFile(const FSFile *file)
{
    FSArchive  *arc = FS_GetAttachedArchive(file);
    FSResult    result = FS_GetResultCode(file);

    SDK_ASSERT((result >= 0) && (result < fs_result_string_max));
    OS_TPrintf("FS error:\n    (archive \"%s:\")\n    %s\n",
               FS_GetArchiveName(arc), fs_result_strings[result]);
}

BOOL PubDataAccessTest(void)
{
	BOOL   ret;
	FSFile file, dir;
	FSResult res;
	FSArchiveResource resource;
	static const char *testdir = "dataPub:/testdir";
	static const char *testfile = "dataPub:/testdir/test.txt";

#if defined(ACCESS_TEST_1)
	FS_InitFile( &file );
	FS_InitFile( &dir );

#if 1
	if( FS_OpenDirectory( &dir, testdir, FS_FILEMODE_R | FS_FILEMODE_W ) )
	{
		OS_TPrintf("%s already exists\n", testdir);
		FS_CloseDirectory( &dir );
	}
	else if( FS_CreateDirectory( testdir, FS_PERMIT_R | FS_PERMIT_W ) == FALSE )
	{
		res = FS_GetArchiveResultCode( testdir );
		return FALSE;
	}
#endif

#if 1
	if( FS_CreateFile( testfile, FS_PERMIT_R | FS_PERMIT_W ) == FALSE )
	{
		OS_TPrintf("dataPub:/testdir/test.txt already exists\n");
		//return FALSE;
	}

	// dataPub:/testdir/test.txt を開いてみる
	if( FS_OpenFileEx( &file, testfile, FS_FILEMODE_W ) == FALSE )
	{
		return FALSE;
	}
	if( FS_CloseFile( &file ) == FALSE )
	{
		return FALSE;
	}
#endif
#elif defined(ACCESS_TEST_2)
	if( FS_GetArchiveResource("rom:/", &resource) == FALSE )
	{
		res = FS_GetArchiveResultCode( "dataPub:/" );
		return FALSE;
	}
#elif defined(ACCESS_TEST_3)
	{
		FSArchive* FSi_GetArchiveChain(void);
        FSArchive  *arc;

		for (arc = FSi_GetArchiveChain(); arc ; arc = arc->next)
        {
            FSPathInfo  info[1];
            char        path[FS_ENTRY_LONGNAME_MAX];

			OS_Printf("archive - %s\n", FS_GetArchiveName(arc));
        }
	}
#elif defined(ACCESS_TEST_4)
    {
        static const char      *basedir = "dataPub:/test";
        static const char      *basedir2 = "dataPub:";
        char                    tmppath[FS_ENTRY_LONGNAME_MAX];
		FSArchiveResource       resource;
		static FSResult         fresult;

        // at first, initialize FSFile structures. (equal to C++ constructor)
        FSFile                  sfile[1];
        FSFile                  sdir[1];
        FS_InitFile(sfile);
        FS_InitFile(sdir);

		if( !FS_GetArchiveResource(basedir2, &resource) )
		{
			fresult = FS_GetArchiveResultCode(basedir2);
			return FALSE;
		}

        // DIRECTORY CREATION TEST
        {
            if (FS_OpenDirectory(sdir, basedir, FS_FILEMODE_R | FS_FILEMODE_W))
            {
                OS_TPrintf("\"%s\" already exists.\n", basedir);
                (void)FS_CloseDirectory(sdir);
            }
            else if (!FS_CreateDirectory(basedir, FS_PERMIT_R | FS_PERMIT_W))
            {
                ReportLastErrorPath(basedir);
                OS_TPanic("FS_CreateDirectory(\"%s\") failed.\n", basedir);
            }
            else
            {
                int     i;
                OS_TPrintf("FS_CreateDirectory(\"%s\") succeeded.\n", basedir);
                for (i = 1; i <= 3; ++i)
                {
                    (void)STD_TSPrintf(tmppath, "%s/dir%d", basedir, i);
                    if (!FS_CreateDirectory(tmppath, FS_PERMIT_R | FS_PERMIT_W))
                    {
                        ReportLastErrorPath(tmppath);
                        OS_TPanic("FS_CreateDirectory(%s) failed.\n", tmppath);
                    }
                    else
                    {
                        OS_TPrintf("FS_CreateDirectory(%s) succeeded.\n", tmppath);
                    }
                }
            }
        }
        // DIRECTORY DELETION TEST
        {
            STD_TSPrintf(tmppath, "%s/tmp/", basedir);
            if (!FS_CreateDirectory(tmppath, FS_PERMIT_R | FS_PERMIT_W))
            {
                ReportLastErrorPath(tmppath);
                OS_TPrintf("FS_CreateDirectory(\"%s\") failed.\n", tmppath);
            }
            if (!FS_DeleteDirectory(tmppath))
            {
                ReportLastErrorPath(tmppath);
                OS_TPanic("FS_DeleteDirectory(\"%s\") failed.\n", tmppath);
            }
        }
		// DIRECTORY DELETE
        if (!FS_DeleteDirectoryAuto(basedir))
        {
            ReportLastErrorPath(basedir);
            OS_TPanic("FS_DeleteDirectoryAuto(\"%s\") failed.\n", basedir);
        }
	}
#elif defined(ACCESS_TEST_5)
	{
        static const char      *basedir = "dataPub:/test";
        char                    tmppath[FS_ENTRY_LONGNAME_MAX];

        // at first, initialize FSFile structures. (equal to C++ constructor)
        FSFile                  sfile[1];
        FSFile                  sdir[1];
        FS_InitFile(sfile);
        FS_InitFile(sdir);

		// FILE CREATION TEST
        {
            enum { file_max = 3 };
            static const char *(message[file_max]) =
            {
                "hello, world!",
                "foobar",
                "hogehoge",
            };
            int     i;
            for (i = 0; i < file_max; ++i)
            {
                STD_TSPrintf(tmppath, "%s/arm9%d.txt", basedir, i);
                if (!FS_CreateFile(tmppath, FS_PERMIT_R | FS_PERMIT_W))
                {
                    ReportLastErrorPath(tmppath);
                }
                else
                {
                    OS_TPrintf("FS_CreateFile(%s) succeeded.\n", tmppath);
                    if (!FS_OpenFileEx(sfile, tmppath, FS_FILEMODE_W))
                    {
                        ReportLastErrorPath(tmppath);
                        OS_TPanic("FS_OpenFileEx(%s) failed.\n", tmppath);
                    }
                    else
                    {
                        if (FS_WriteFile(sfile, message[i], STD_GetStringLength(message[i])) < 0)
                        {
                            ReportLastErrorFile(sfile);
                            OS_TPanic("FS_WritFile() failed.\n");
                        }
                        else
                        {
                            OS_TPrintf("FS_WritFile() succeeded.\n");
                        }
                    }
                    (void)FS_CloseFile(sfile);
                }
            }
        }
	}
#endif

	return TRUE;
}
#endif
