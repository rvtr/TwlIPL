/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - compstatic
  File:     file.c

  Copyright 2003 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <malloc.h>                    // calloc()
#include <stdlib.h>                    // free(), exit()
#include <sys/stat.h>                  // stat()
#include <string.h>                    // strlen/strdup/strcpy
#include <stdarg.h>                    // va_start(),va_end()
#include <unistd.h>                    // unlink()
#include "file.h"


/*---------------------------------------------------------------------------*
  Name:         ReadFile

  Description:  ファイルの読み込み バッファの確保
 *---------------------------------------------------------------------------*/
int ReadFile(const char *filename, u8 **buffer)
{
    FILE   *fp;
    struct stat filestat;
    int     filesize;

    fp = NULL;
    *buffer = NULL;

    if (filename == NULL)
    {
        ErrorPrintf("Not specified filename\n");
        goto error;
    }

    /* Open file */
    if (stat(filename, &filestat) || !S_ISREG(filestat.st_mode) ||
        NULL == (fp = fopen(filename, "rb")))
    {
        ErrorPrintf("Cannot open file '%s'\n", filename);
        goto error;
    }

    /* Read file */
    filesize = filestat.st_size;
    if (NULL == (*buffer = malloc(filesize)))
    {
        ErrorPrintf("Cannot allocate memory size=%d\n", filesize);
        goto error;
    }

    if (filesize != fread(*buffer, sizeof(u8), filesize, fp))
    {
        ErrorPrintf("Cannot read file '%s'\n", filename);
        goto error;
    }

    DebugPrintf("%p %8d bytes  ReadFile \'%s\'\n", *buffer, filesize, filename);

    /* Close file */
    fclose(fp);
    return filesize;

  error:
    if (*buffer)
        free(*buffer);
    if (fp)
        fclose(fp);
    return -1;
}


/*---------------------------------------------------------------------------*
  Name:         WriteFile

  Description:  ファイルの作成書き込み
 *---------------------------------------------------------------------------*/
int WriteFile(const char *filename, u8 *buffer, int size)
{
    FILE   *fp;

    DebugPrintf("%p %8d bytes  WriteFile \'%s\'\n", buffer, size, filename);

    /* Open file */
    if (NULL == (fp = fopen(filename, "wb")))
    {
        ErrorPrintf("Cannot open file '%s'\n", filename);
        return -1;
    }

    /* Write file */
    if (size != fwrite(buffer, sizeof(u8), size, fp))
    {
        ErrorPrintf("Cannot write file '%s'\n", filename);
        (void)fclose(fp);
        (void)unlink(filename);
        return -1;
    }

    /* Close file */
    if (0 > fclose(fp))
    {
        ErrorPrintf("Cannot close file '%s'\n", filename);
        (void)unlink(filename);
        return -1;
    }
    return size;
}

/*---------------------------------------------------------------------------*
  Name:         CopyBuffer

  Description:  バッファのコピー
 *---------------------------------------------------------------------------*/
void CopyBuffer(const u8 *src, u8 *dst, int size)
{
    int     i;

    if ((unsigned int)src > (unsigned int)dst)
    {
        for (i = 0; i < size; i++)
        {
            dst[i] = src[i];
        }
    }
    else
    {
        for (i = size - 1; i >= 0; i--)
        {
            dst[i] = src[i];
        }
    }
    return;
}

/*---------------------------------------------------------------------------*
  Name:         GetDirName

  Description:  ファイル名のディレクトリ名を取得する
 *---------------------------------------------------------------------------*/
char   *GetDirName(const char *path)
{
    int     i;
    char   *new_path;

    for (i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            if (NULL != (new_path = strdup(path)))
            {
                new_path[i] = '\0';
            }
            return new_path;
        }
        if (path[i] == ':')
        {
            if (NULL != (new_path = malloc(i + 3)))
            {
                strncpy(new_path, path, i);
                strcpy(new_path + i, ":.");
            }
            return new_path;
        }
    }

    return strdup(".");
}

/*---------------------------------------------------------------------------*
  Name:         DebugPrintf

  Description:  Debug 出力用 Printf
 *---------------------------------------------------------------------------*/
BOOL    bDebugMode = FALSE;

void DebugPrintf(const char *fmt, ...)
{
    va_list va;

    if (bDebugMode)
    {
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ConsolePrintf

  Description:  通常コンソール出力用 Printf
 *---------------------------------------------------------------------------*/
void ConsolePrintf(const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
    vfprintf(stdout, fmt, va);
    va_end(va);
}

/*---------------------------------------------------------------------------*
  Name:         ErrorPrintf

  Description:  Error 出力用 Printf
 *---------------------------------------------------------------------------*/
void ErrorPrintf(const char *fmt, ...)
{
    va_list va;

    fprintf(stderr, "Error: ");
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}


/*---------------------------------------------------------------------------*
  Name:         StrDup

  Description:  エラーハンドリング＆古い値の free 付き strdup 関数
 *---------------------------------------------------------------------------*/
char   *StrDup(char *old, char *new)
{
    if (old)
    {
        free(old);
    }
    if (new && NULL == (new = strdup(new)))
    {
        ErrorPrintf("Cannot allocate memory\n");
        exit(1);
    }
    return new;
}


/*---------------------------------------------------------------------------*
  Name:         StrCat

  Description:  文字列の連結
 *---------------------------------------------------------------------------*/
char   *StrCat(int num, ...)
{
    va_list va;
    int     i;
    int     size;
    char   *result;

    va_start(va, num);
    size = 0;
    for (i = 0; i < num; i++)
    {
        size += strlen(va_arg(va, char *));
    }
    va_end(va);

    if (NULL == (result = malloc(size + 1)))
    {
        ErrorPrintf("Cannot allocate memory\n");
        exit(1);
    }

    va_start(va, num);
    result[0] = '\0';
    for (i = 0; i < num; i++)
    {
        (void)strcat(result, va_arg(va, char *));
    }
    va_end(va);

    return result;
}

//---------------------------------------------------------------------------
// パス文字列からファイル名部分のポインタを取得し、拡張子を削る
// @param path パス
// @return ファイル名のポインタ
//---------------------------------------------------------------------------
char *StrCutFname(char *path)
{
    char   *search_tmp;

    if (path == NULL)
    {
        return NULL;
    }

    if ((search_tmp = strrchr(path, '/')) != NULL)
    {
        path = (search_tmp + 1);
    }
    if ((search_tmp = strrchr(path, '\\')) != NULL)
    {
        path = (search_tmp + 1);
    }
    if ((search_tmp = strrchr(path, '.')) != NULL)
    {
        *search_tmp = '\0';
    }
    return path;
}
