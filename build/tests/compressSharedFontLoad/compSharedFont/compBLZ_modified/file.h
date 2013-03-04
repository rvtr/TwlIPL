/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - compstatic
  File:     file.h

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
#ifndef FILE_H__
#define FILE_H__

#include "common.h"

int     ReadFile(const char *filename, u8 **buffer);
int     WriteFile(const char *filename, u8 *buffer, int size);
void    CopyBuffer(const u8 *src, u8 *dst, int size);
char   *GetDirName(const char *filename);
void    DebugPrintf(const char *fmt, ...);
void    ErrorPrintf(const char *fmt, ...);
void    ConsolePrintf(const char *fmt, ...);
char   *StrDup(char *old, char *new);
char   *StrCat(int num, ...);
char   *StrCutFname(char *path);

extern BOOL bDebugMode;

#endif //FILE_H__
