/*---------------------------------------------------------------------------*
  Project:  NitroSDK - tools - makebanner
  File:     path.c

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: path.c,v $
  Revision 1.3  2006/01/18 02:11:20  kitase_hirotake
  do-indent

  Revision 1.2  2005/02/28 05:26:13  yosizaki
  do-indent.

  Revision 1.1  2004/08/30 08:41:14  yasu
  makebanner moves into CVS tree

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<stdlib.h>             // free()
#include	<string.h>             // strcasecmp()
#include	<sys/stat.h>           // stat()
#include	<dirent.h>             // opendir()/readdir()/closedir()
#include	<unistd.h>             // getcwd()
#ifdef	__CYGWIN__
#include	<sys/cygwin.h>         // cygwin_conv_to_win32_path()
#endif
#include	"path.h"

//---------------------------------------------------------------------------
//  Get File Statue
//---------------------------------------------------------------------------

tFileStatus GetFileStatus(struct stat *s, const char *filename)
{
    // Get file status
    if (stat(filename, s))
    {
        error("Can't get status %s", filename);
        return FILESTATUS_ERROR;
    }

    if (S_ISREG(s->st_mode))
    {
        return FILESTATUS_FILE;
    }
    else if (S_ISDIR(s->st_mode))
    {
        return FILESTATUS_DIR;
    }

    error("Unknown file type %s", filename);
    return FILESTATUS_ERROR;
}


//---------------------------------------------------------------------------
//  File Globbing & Dir Listing
//---------------------------------------------------------------------------

typedef struct
{
    tCallBack callBack;
    void   *param;
    tWildCard *accept;
    tWildCard *reject;

}
tForeachEntryParam;


static BOOL isAcceptEntryName(char *pathName, tWildCard * accept, tWildCard * reject)
{
    char   *p = pathName;

    while (*p)
    {
        if (*p == '/')
            pathName = p + 1;
        p++;
    }

    if (accept)
    {
        while (accept)
        {
            if (WildCardCmp(accept->name, pathName))
            {
                goto accepted;
            }
            accept = accept->next;
        }
        return FALSE;
    }
  accepted:

    while (reject)
    {
        if (WildCardCmp(reject->name, pathName))
        {
            return FALSE;
        }
        reject = reject->next;
    }
    return TRUE;
}


static BOOL ForeachEntry_CallBack(char *pathName, void *param)
{
    tForeachEntryParam *t = (tForeachEntryParam *) param;
    struct stat fstat;

    if (!isAcceptEntryName(pathName, t->accept, t->reject))
    {
        // Reject!!! ignored
        return TRUE;
    }

    switch (GetFileStatus(&fstat, pathName))
    {
    case FILESTATUS_FILE:
        return t->callBack(pathName, t->param);

    case FILESTATUS_DIR:
        return ForeachDirList(pathName, ForeachEntry_CallBack, param);

    default:
        break;
    }
    return FALSE;
}


BOOL ForeachEntry(const char *pathName, tWildCard * reject, tCallBack callBack, void *param)
{
    tForeachEntryParam t;

    t.callBack = callBack;
    t.param = param;
    t.accept = NULL;
    t.reject = reject;

    return ForeachPathGlobbing(pathName, ForeachEntry_CallBack, &t);
}


typedef struct
{
    tCallBack callBack;
    void   *param;
    char   *baseName;

}
tForeachFileParam;


static BOOL ForeachFile_CallBack(char *pathName, void *param)
{
    tForeachFileParam *t = (tForeachFileParam *) param;

    int     len = strlen(t->baseName);

    debug_printf("        ForeachFile_CallBack path[%s] base[%s]\n", pathName, t->baseName);

    if (strncmp(pathName, t->baseName, len))
    {
        error("Wildcard in Root is not supported");
        return FALSE;
    }

    return t->callBack(pathName + len, t->param);
}


BOOL ForeachFile(const char *baseName, const char *fileName, tWildCard * reject, tCallBack callBack,
                 void *param)
{
    char   *cBaseName;
    char   *cPathName;
    BOOL    state;
    tForeachFileParam t;

    debug_printf("ForeachFile :  baseName[%s]  fileName[%s]\n", baseName, fileName);

    cBaseName = GetSrcPath(baseName, "");
    cPathName = GetSrcPath(baseName, fileName);

    debug_printf("ForeachFile : cBaseName[%s] cPathName[%s]\n", cBaseName, cPathName);

    t.callBack = callBack;
    t.param = param;
    t.baseName = cBaseName;

    state = ForeachEntry(cPathName, reject, ForeachFile_CallBack, &t);

    free(cBaseName);
    free(cPathName);

    return state;
}


//---------------------------------------------------------------------------
//  FilePath Globbing
//---------------------------------------------------------------------------

typedef struct
{
    char   *baseName;
    char   *pathName;
    tCallBack callBack;
    void   *param;

}
tGlobParam;


static int CountFile;
static BOOL ForeachPathGlobbing_Entry(tGlobParam * pg);
static BOOL ForeachPathGlobbing_WildCard(char *pathName, void *param);

BOOL ForeachPathGlobbing(const char *pathName, tCallBack callBack, void *param)
{
    tGlobParam g;
    BOOL    ret;

    g.baseName = NULL;
    g.pathName = PathNormalize(pathName, TRUE);
    g.callBack = callBack;
    g.param = param;
    CountFile = 0;

    debug_printf("PathGlobbing : Name [%s]->[%s]\n", pathName, g.pathName);

    ret = ForeachPathGlobbing_Entry(&g);

    free(g.pathName);

    if (ret && CountFile == 0)
    {
        error("No file or directory matched %s", pathName);
        return FALSE;
    }
    return ret;
}


static BOOL ForeachPathGlobbing_Entry(tGlobParam * pg)
{
    tGlobParam g;
    char   *entryName;
    struct stat s;
    BOOL    state;

    if (pg->pathName)
    {
        entryName = PathDup(pg->pathName);

        g = *pg;
        g.pathName = PathGetDirLevelDown(pg->pathName);

        if (pg->baseName)
        {
            g.baseName = Alloc(strlen(pg->baseName) + strlen(entryName) + 2);
            sprintf(g.baseName, "%s/%s", pg->baseName, entryName);
        }
        else
        {
            g.baseName = strdup(entryName);
        }

        // Check if wildcard ?
        if (isPathWildCard(entryName))
        {
            state = ForeachDirList(pg->baseName, ForeachPathGlobbing_WildCard, &g);
        }
        else
        {
            state = ForeachPathGlobbing_Entry(&g);
        }

        Free(&entryName);
        Free(&g.baseName);
    }
    else
    {
        // Check if file exists
        if (!stat(pg->baseName, &s))
        {
            debug_printf("        File Found [%s]\n", pg->baseName);

            // Globbing done, exec callback

            state = pg->callBack(pg->baseName, pg->param);
            CountFile++;
        }
        else
        {
            debug_printf("        File Not Found [%s]\n", pg->baseName);
            state = TRUE;              // Ignored
        }
    }
    return state;
}


static BOOL ForeachPathGlobbing_WildCard(char *pathName, void *param)
{
    tGlobParam g;
    tGlobParam *pg = (tGlobParam *) param;

    debug_printf("        WildCardCmp: [%s] [%s]\n", pg->baseName, pathName);

    if (WildCardCmp(pg->baseName, pathName))
    {
        g = *pg;
        g.baseName = pathName;
        return ForeachPathGlobbing_Entry(&g);
    }

    return TRUE;                       // Ignored
}


//---------------------------------------------------------------------------
//  Directory Listing
//              Listing directory & Exec CallBack
//---------------------------------------------------------------------------

BOOL ForeachDirList(const char *dirName, tCallBack callBack, void *param)
{
    DIR    *dir;
    struct dirent *entry;
    char   *pathName;
    BOOL    state = TRUE;

    if (!dirName)
    {
        dirName = ".";
    }

    debug_printf("DirectoryList: Name [%s]\n", dirName);

    // Open directory
    if (NULL == (dir = opendir(dirName)))
    {
        error("Can't read directory %s", dirName);
        return FALSE;
    }

    // Store new files
    while (NULL != (entry = readdir(dir)))
    {
        pathName = entry->d_name;

        if (!strcmp(pathName, ".") || !strcmp(pathName, ".."))
        {
            continue;
        }

        debug_printf(" :%s\n", pathName);
        pathName = GetSrcPath(dirName, pathName);
        state = callBack(pathName, param);
        free(pathName);

        if (!state)
            break;
    }

    closedir(dir);
    return state;
}


//---------------------------------------------------------------------------
//
//  PathName Utilities
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  StrCmp/StrCpy entry name terminated / or \0
//---------------------------------------------------------------------------

int PathCmp(const char *path, const char *cmp)
{
    char    c;

    do
    {
        c = *path;
        if (c == '/')
            c = '\0';                  // end of string if '/'
        if (c != *cmp)
            return 1;
        path++;
        cmp++;
    }
    while (c);

    return 0;
}


char   *PathCpy(char *dest, const char *src)
{
    while (*src != '\0' && *src != '/')
    {
        *dest++ = *src++;
    }
    return dest;                       // Don't set '\0'
}


int PathLen(const char *path)
{
    int     n = 0;

    while (*path != '\0' && *path != '/')
    {
        n++;
        path++;
    }
    return n;
}


char   *PathDup(const char *src)
{
    int     n = PathLen(src);
    char   *dest = Alloc(n + 2);

    PathCpy(dest, src);
    dest[n] = '\0';

    return dest;
}


BOOL WildCardCmp(const char *wildcard, const char *path)
{
    if (*wildcard == '*')
    {
        if (*path != '\0' && WildCardCmp(wildcard, path + 1))
            return TRUE;
        if (WildCardCmp(wildcard + 1, path))
            return TRUE;
    }

    else if (*wildcard == '?')
    {
        return *path != '\0' && WildCardCmp(wildcard + 1, path + 1);
    }

    else if (*wildcard == *path)
    {
        return *path == '\0' || WildCardCmp(wildcard + 1, path + 1);
    }

    return FALSE;
}


BOOL isPathWildCard(const char *path)
{
    while (*path != '\0' && *path != '/')
    {
        if (*path == '*' || *path == '?')
        {
            return TRUE;
        }
        path++;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
//  Go up/down directory level
//---------------------------------------------------------------------------

char   *PathGetDirLevelDown(const char *path)
{
    while (*path)
    {
        if (*path == '/')
            return (char *)path + 1;
        path++;
    }
    return NULL;
}


//---------------------------------------------------------------------------
//  Get Basename
//---------------------------------------------------------------------------

char   *GetBaseName(const char *path)
{
    int     i;
    char   *new_path;

    for (i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            new_path = strdup(path);
            new_path[i] = '\0';
            return new_path;
        }
        if (path[i] == ':')
        {
            new_path = Alloc(i + 3);
            strncpy(new_path, path, i);
            strcpy(new_path + i, ":.");
            return new_path;
        }
    }

    new_path = strdup(".");
    return new_path;
}


//---------------------------------------------------------------------------
//  Get Filename
//---------------------------------------------------------------------------

char   *GetFileName(const char *path)
{
    int     i;
    char   *new_file;

    for (i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/' || path[i] == ':')
        {
            new_file = strdup(path + i + 1);
            return new_file;
        }
    }
    new_file = strdup(path);
    return new_file;
}


//---------------------------------------------------------------------------
//  Reconstruct path name
//
//      - Resolve '.' or '..' in path name
//      - Work around . and / to translate regular form
//
//      Regular form of path:
//              Absolute Path    [Drive:]/.[/Entry...]
//              Relative Path    [Drive:].[/Entry]...
//
//              ex)
//                      abc/def -> ./abc/def
//                      /aaa    -> /./aaa
//                      D:/aaa  -> D:/./aaa
//                      /       -> /.
//                      .       -> .
//                      ../aa   -> ./../aa
//---------------------------------------------------------------------------

char   *PathNormalize(const char *pathName, BOOL isTreatDotDot)
{
    int     i, level, level_root, n;
    BOOL    isAbsolute;
    const char *entry[DIRLEVEL_MAX];

    char   *pathNormal = Alloc(strlen(pathName) + 4);
    const char *p_org;
    char   *p_new;

    //
    // Check if drive letter C: D: E:
    // Check if absolute path
    //
    p_new = pathNormal;
    p_org = SkipDriveName(pathName);
    n = (int)p_org - (int)pathName;

    if (n > 0)
    {
        strncpy(p_new, pathName, n);
        p_new += n;
    }
    isAbsolute = isAbsolutePath(p_org);

    //
    // Resolve '.' and '..'
    //
    //   Slice the path at point of / , put them into entry[]
    //

    level = level_root = 0;

    for (; p_org; p_org = PathGetDirLevelDown(p_org))
    {
        if (!PathCmp(p_org, "") || !PathCmp(p_org, "."))
        {
            // skip it
            continue;
        }
        else if (!PathCmp(p_org, "..") && isTreatDotDot)
        {
            if (level > level_root)
            {
                // Back to parent dir
                level--;
                continue;
            }

            // if pathname starts with '/', no directory to go up
            if (isAbsolute)
            {
                error("Can't go up directory, '..' Ignored. %s", pathName);
                continue;
            }

            // keep '..'
            level_root = level + 1;
        }

        // name entry
        entry[level] = p_org;
        level++;
    }

    // Reconstruct pathname
    if (isAbsolute)
    {
        *p_new++ = '/';
    }
    *p_new++ = '.';

    for (i = 0; i < level; i++)
    {
        *p_new++ = '/';
        p_new = PathCpy(p_new, entry[i]);
    }
    *p_new = '\0';

#if 0
    if (strcmp(pathNormal, pathName))
    {
        debug_printf("        PathNormal: [%s] -> [%s]\n", pathName, pathNormal);
    }
#endif
    return pathNormal;
}


//---------------------------------------------------------------------------
//  Get Src Path
//      Normalize BASENAME
//      Normalize FILENAME
//      Concat both
//---------------------------------------------------------------------------

char   *GetSrcPath(const char *baseName, const char *fileName)
{
    char   *base;
    char   *file;
    char   *t;
    char   *path;

    base = PathNormalize(baseName, TRUE);
    file = PathNormalize(fileName, TRUE);
    t = Alloc(strlen(base) + strlen(file) + 2);

    // Concat base + '/' + file
    sprintf(t, "%s/%s", base, file);
    path = PathNormalize(t, FALSE);

    free(base);
    free(file);
    free(t);

    debug_printf("        GetSrcPath: [%s]\n", path);
    return path;
}


//---------------------------------------------------------------------------
//  Get Dest Path
//      Concat BASENAME + FILENAME
//      Normalize it
//---------------------------------------------------------------------------

char   *GetDestPath(const char *baseName, const char *fileName)
{
    char   *t;
    char   *path;

    t = Alloc(strlen(baseName) + strlen(fileName) + 2);

    // Concat base + '/' + file
    sprintf(t, "%s/%s", baseName, fileName);
    path = PathNormalize(t, TRUE);

    free(t);

    debug_printf("        GetDestPath: [%s]\n", path);
    return path;
}


//---------------------------------------------------------------------------
//  Remake the path into familier shape
//      Delete ./
//---------------------------------------------------------------------------

char   *PathDenormalize(char *path)
{
    char   *p;

    p = (char *)SkipDriveName(path);
    if (*p == '/')
    {
        p++;
    }

    // Cut './'
    if (*p == '.' && *(p + 1) == '/')
    {
        while ('\0' != (*p = *(p + 2)))
        {
            p++;
        }

        if (p == path)
        {

        }
    }



    return path;
}


//---------------------------------------------------------------------------
//  Get PC Path
//---------------------------------------------------------------------------

char   *GetWin32Path(char *cygpath)
{
    static char buffer[FILENAME_MAX];

#ifdef	__CYGWIN__
    if (*cygpath == '/')
    {
        cygwin_conv_to_win32_path(cygpath, buffer);
    }
    else
#endif
    {
        strcpy(buffer, cygpath);
    }

    return ChangeBackSlash(buffer);
}

char   *ChangeWin32Path(char *cygpath)
{
    char   *win32path = strdup(GetWin32Path(cygpath));

    free(cygpath);
    return win32path;
}

//---------------------------------------------------------------------------
//  Change suffix
//---------------------------------------------------------------------------
char   *ChangeSuffix(const char *file, const char *suffix)
{
    int     i, n;
    char   *path;

    n = strlen(file);

    for (i = n; file[i] != '.'; i--)
    {
        if (file[i] == '/' || i == 0)
        {
            i = n;
            break;
        }
    }

    path = Alloc(i + strlen(suffix) + 1);
    strncpy(path, file, i);
    strcpy(path + i, suffix);

    return path;
}


//---------------------------------------------------------------------------
//  Get Current Dir
//---------------------------------------------------------------------------

char   *GetCurrentDirectory(void)
{
    static char buffer[FILENAME_MAX];
    char   *cwd;

    cwd = getcwd(buffer, FILENAME_MAX);
    if (!cwd)
    {
        error("Can't access current directory");
        exit(10);
    }
    return cwd;
}


//---------------------------------------------------------------------------
//  Check if absolute path
//
//      Return True in case of ...
//
//              /dirA/dirB/fileC
//              D:/dirA/dirB/fileC
//
//      Return False in case of ...
//
//              dirX/dirY/fileZ
//              D:dirX/dirY/fileZ
//---------------------------------------------------------------------------

BOOL isAbsolutePath(const char *path)
{
    const char *p = path;

    while (*p != '\0')
    {
        if (*p == '/' || *p == '\\')
        {
            if (p == path || p[-1] == ':')
            {
                return TRUE;
            }
        }
        p++;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
//  Check if drive name
//
//      Return next character of ':' if drive name
//      Return head of path if no drive name
//---------------------------------------------------------------------------

const char *SkipDriveName(const char *path)
{
    const char *p = path;

    while (*p != '\0' && *p != '/' && *p != '\\')
    {
        if (*p == ':')
        {
            return p + 1;
        }
        p++;
    }
    return path;
}


//---------------------------------------------------------------------------
//  App Name Utilities
//---------------------------------------------------------------------------
static char *appName;
static char *appBaseName;
static char *appFileName;

void InitAppName(const char *path)
{
    char   *slash_path = ChangeBackSlash(strdup(path));

    appBaseName = GetBaseName(slash_path);
    appFileName = GetFileName(slash_path);
    appName = ChangeSuffix(appFileName, "");

    free(slash_path);
}

char   *GetAppName(void)
{
    return appName;
}

char   *GetAppBaseName(void)
{
    return appBaseName;
}

char   *GetAppFileName(void)
{
    return appFileName;
}


#ifdef	TEST
int main(int argc, char *argv[])
{
    int     i;
    char   *s;

    for (i = 1; i < argc; i++)
    {
        s = PathNormalize(argv[i], TRUE);
        printf("[%s] -> [%s]\n", argv[i], s);
        free(s);
    }
    return 0;
}
#endif
