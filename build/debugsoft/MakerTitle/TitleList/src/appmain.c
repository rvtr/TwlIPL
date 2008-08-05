/*---------------------------------------------------------------------------*
  Project:  TwlSDK_IPL - debugsoft - MakerTitle - TitleList
  File:     appmain.c

  Copyright 2003 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include "appmain.h"
#include "screen.h"
#include "mycode.h"
#include "title.h"
#include <twl/na.h>
#include <nitro/fs.h>
#include <nitro/std.h>
#include <nitro/mi.h>

BOOL TestWriting(const char* code, BOOL out_results[2]);
BOOL WriteFile(const char* arc_name, const char* file_name);
BOOL ReadFile(const char* arc_name, const char* file_name, char* out, int len);
void PrintResult(int x, int y, char* text);

const char* ARC_NAME[2] = {
    "otherPub:/",
    "otherPrv:/",
};


const char* SAV_NAME[2] = {
    "dataPub:/",
    "dataPrv:/",
};

const char* FILE_NAME = "save.dat";

BOOL TestWriting(const char* code, BOOL out_results[2])
{
    const NATitleArchive arc[2] = {
        NA_TITLE_ARCHIVE_DATAPUB,
        NA_TITLE_ARCHIVE_DATAPRV,
    };
    s32 i = 0;
    for(i = 0; i < 2; ++i)
    {
        FSResult res = NA_LoadOtherTitleArchive(code, arc[i]);
        out_results[i] = (res != FS_RESULT_SUCCESS) ? FALSE : TRUE;
        if(res != FS_RESULT_SUCCESS)
        {
            OS_TPrintf("Mount Failed:%s\n", code);
            OS_TPrintf("ErrorCode:%d\n", res);
            continue;
        }
        
        WriteFile(ARC_NAME[i], FILE_NAME);
        
        NA_UnloadOtherTitleArchive();
    }
    
    return out_results[0] | out_results[1];
}

BOOL WriteFile(const char* arc_name, const char* file_name)
{
    FSFile fp;
    char buf[256];
    s32 len = 0;
    
    STD_TSPrintf(buf, "%s%s", arc_name, file_name);
    
    FS_InitFile(&fp);
    FS_CreateFile(buf, FS_PERMIT_W | FS_PERMIT_R);
    
    if(!FS_OpenFileEx(&fp, buf, FS_FILEMODE_RW))
    {
        OS_TPrintf("Open Fail:%s\n", buf);
        return FALSE;
    }
    
    FS_SeekFileToEnd(&fp);
    len = STD_TSPrintf(buf, "Written by [%s]\n", MYNAME);
    FS_WriteFile(&fp, buf, len);
    FS_CloseFile(&fp);
    
    return TRUE;
}

void DrawScreen(void)
{
     PrintString(0, 0, 15, "GameCode:%s", MYCODE);
     PrintString(0, 2, 15, "A - Writing test to other apps");
     PrintString(0, 3, 15, "B - Show my save data");
     if(STD_StrCmp("4KAA", MYCODE) == 0)
     {
         PrintString(0, 4, 15, "X - Clear save datas");
     }
}

void DoWritingTest(void)
{
    BOOL    results[TITLE_COUNT][2];
    s16 i = 0;
    char*    result_char[3] = {"O", "X", "-"};

    MI_CpuClear8(results, sizeof(results));
    
    ClearSubScreen();
    PrintString(0, 24, 15, "APP  : PUB  PRV  WRITE");
    PrintString(0, 25, 15, "---------------------");
    for(i = 0; i < TITLE_COUNT; ++i)
    {
        char buf[256];
        char format[] = "ROM %c: %s    %s    %s";
        // 自分自身はチェックしない 
//        OS_TPrintf("%s %s\n", GAMECODE_LIST[i], MYCODE);
        if(STD_StrCmp(GAMECODE_LIST[i], MYCODE) == 0){
            STD_TSPrintf(buf, format, 'A' + i, "-", "-", "-");
        }
        else
        {
            s32 result_code[3] = {0, 0, 0};
            TestWriting(GAMECODE_LIST[i], results[i]);
            result_code[0] = (!DATA_EXIST[i][0]) ? 2 : (results[i][0] ? 0 : 1); 
            result_code[1] = (!DATA_EXIST[i][1]) ? 2 : (results[i][1] ? 0 : 1); 
            result_code[2] = (result_code[0] == 2 && result_code[1] == 2) ? 2 : (result_code[0] != 1 && result_code[1] != 1) ? 0 : 1;
            STD_TSPrintf(buf, format, 'A' + i, 
                result_char[result_code[0]],
                result_char[result_code[1]],
                result_char[result_code[2]]
            );
        }
        OS_TPrintf("%s\n", buf);
        PrintString(0, (s16)(26 + i), 15, buf);
    }
}

void DoReadingTest(void)
{
    char read_buf[2048];
    
    ClearSubScreen();

    {
        s32 i = 0;
        BOOL result[2] = {FALSE, FALSE};
        
        PrintString(0, 24, 15, "Public");
        PrintString(16, 24, 15, "Private");
        for(i = 0; i < 2; ++i)
        {
            MI_CpuClear8(read_buf, sizeof(read_buf));
            result[i] = ReadFile(SAV_NAME[i], FILE_NAME, read_buf, sizeof(read_buf));
/*            if(!result[i])
            {
                continue;
            }*/
            OS_TPrintf("%s%s\n", SAV_NAME[i], FILE_NAME);
            OS_TPrintf("%s\n", read_buf);
            OS_TPrintfEx("% 256.16b\n", read_buf);
            
            PrintResult(i * 16, 25, read_buf);
        }
    }
}

BOOL ReadFile(const char* arc_name, const char* file_name, char* out, int len)
{
    FSFile fp;
    char buf[256];
    
    STD_TSPrintf(buf, "%s%s", arc_name, file_name);
    
    FS_InitFile(&fp);
    
    if(!FS_OpenFileEx(&fp, buf, FS_FILEMODE_R))
    {
        FSResult res;
        
        res = FS_GetArchiveResultCode(buf);
        OS_TPrintf("Code:%d\n", res);
        OS_TPrintf("Open Fail:%s\n", buf);
        STD_TSPrintf(out,"Error:%d\n%s Open Failed\n", res, buf);
        return FALSE;
    }
    
    FS_ReadFile(&fp, out, len);
    FS_CloseFile(&fp);
    
    return TRUE;    
}

void PrintResult(int x, int y, char* text)
{
    int len = 0, i = 0;
    char *p = text, *next = NULL;
    char buf[2048];
    while(*p != '\0')
    {
        next = STD_StrChr(p, '\n');
        STD_CopyLStringZeroFill(buf, p, next - p + 1);
        OS_TPrintf("str:%s\n", buf);
        PrintString((short)x, (short)y, 15, buf);
        y++;
        p = next + 1;
    }
}

void DeleteSaveDatas(void)
{
    const NATitleArchive arc[2] = {
        NA_TITLE_ARCHIVE_DATAPUB,
        NA_TITLE_ARCHIVE_DATAPRV,
    };
    int i = 0, j = 0, k = 0;
    ClearSubScreen();
    for(i = 0; i < TITLE_COUNT; ++i)
    {
        for(j = 0; j < 2; ++j)
        {
            char    path[256];
            FSResult res = NA_LoadOtherTitleArchive(GAMECODE_LIST[i], arc[j]);
            if(res != FS_RESULT_SUCCESS)
            {
                OS_TPrintf("Mount Failed:%s\n", GAMECODE_LIST[i]);
                OS_TPrintf("ErrorCode:%d\n", res);
                continue;
            }
            
            STD_TSPrintf(path, "%s%s", ARC_NAME[j], FILE_NAME);
            if(!FS_DeleteFile(path))
            {
                OS_TPrintf("Delete Fail:%s\n", path);
            }
            else
            {
                OS_TPrintf("Deleted:%s %s\n", GAMECODE_LIST[i], path);
                PrintString(0,(short) (24 + k++), 15, "Deleted:%s %s", GAMECODE_LIST[i], path);
            }
            NA_UnloadOtherTitleArchive();
        }
    }
}
