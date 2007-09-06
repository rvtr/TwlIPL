/*---------------------------------------------------------------------------*
  Project:  NitroSDK - tools - makebanner
  File:     path.h

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: path.h,v $
  Revision 1.3  2006/01/18 02:11:20  kitase_hirotake
  do-indent

  Revision 1.2  2005/02/28 05:26:13  yosizaki
  do-indent.

  Revision 1.1  2004/08/30 08:41:14  yasu
  makebanner moves into CVS tree

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef	PATH_H_
#define	PATH_H_

#include  <sys/stat.h>                 // struct tat
#include  "misc.h"

#define DIRLEVEL_MAX		256
#ifndef	FILENAME_MAX
#define	FILENAME_MAX		1024
#endif

typedef enum
{
    FILESTATUS_ERROR = -1,
    FILESTATUS_FILE = 0,
    FILESTATUS_DIR = 1
}
tFileStatus;


// Item Reject Control

typedef struct tWildCard
{
    struct tWildCard *next;
    char   *name;

}
tWildCard;


// CallBacks

typedef BOOL (*tCallBack) (char *, void *);


// Prototypes

tFileStatus GetFileStatus(struct stat *s, const char *filename);
BOOL    ForeachEntry(const char *pathName, tWildCard * reject, tCallBack callBack, void *param);
BOOL    ForeachFile(const char *baseName, const char *fileName, tWildCard * reject,
                    tCallBack callBack, void *param);
BOOL    ForeachPathGlobbing(const char *pathName, tCallBack callBack, void *param);
BOOL    ForeachDirList(const char *dirName, tCallBack callBack, void *param);
int     PathCmp(const char *path, const char *cmp);
char   *PathCpy(char *dest, const char *src);
int     PathLen(const char *path);
char   *PathDup(const char *src);
char   *PathGetDirLevelDown(const char *path);
char   *GetBaseName(const char *path);
char   *GetFileName(const char *path);
BOOL    WildCardCmp(const char *wildcard, const char *path);
BOOL    isPathWildCard(const char *path);
char   *PathNormalize(const char *pathName, BOOL isTreatDotDot);
char   *PathDenormalize(char *path);
char   *GetSrcPath(const char *base, const char *file);
char   *GetDestPath(const char *base, const char *file);
char   *GetWin32Path(char *cygpath);
char   *ChangeWin32Path(char *cygpath);
char   *ChangeSuffix(const char *file, const char *suffix);
char   *GetCurrentDirectory(void);
BOOL    isAbsolutePath(const char *path);
const char *SkipDriveName(const char *path);
void    InitAppName(const char *path);
char   *GetAppName(void);
char   *GetAppBaseName(void);
char   *GetAppFileName(void);

#endif //PATH_H_
