/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - NandFiler
  File:     main.cpp

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
#include <twl.h>
#include <nitro/fs.h>
#include <nitro/std.h>

#include <string>
#include <list>
#include "suffix.h"

inline void* operator new(size_t size)      { return OS_Alloc(size); }
inline void* operator new[](size_t size)    { return OS_Alloc(size); }
inline void operator delete(void* ptr)      { OS_Free(ptr); }
inline void operator delete[](void* ptr)    { OS_Free(ptr); }

typedef std::list<std::string> PathList;

namespace
{
    void
    InitInterrupt(void)
    {
        OS_EnableIrq();
        OS_EnableInterrupts();
    }

    void
    InitAlloc(void)
    {
        OSHeapHandle hHeap;
        void* lo = OS_GetMainArenaLo();
        void* hi = OS_GetMainArenaHi();

        lo = OS_InitAlloc(OS_ARENA_MAIN, lo, hi, 1);
        OS_SetArenaLo(OS_ARENA_MAIN, lo);

        hHeap = OS_CreateHeap(OS_ARENA_MAIN, lo, hi);
        SDK_ASSERT( hHeap >= 0 );

        OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
    }
}



static void InitInteruptSystem();
static void InitFileSystem();

static void PickupTargetPath(const char* root, const char* suffix, PathList& out);
static void CopyTarget(const char* to, const char* header, PathList& out);
static void CopyFile(const char* src, const char* dest);

void
TwlStartUp()
{
    OS_Init();
    InitAlloc();
}


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  ���C���֐��ł��B 

  Arguments:    �Ȃ��B 

  Returns:      �Ȃ��B 
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    InitInteruptSystem();
    InitFileSystem();

    GX_DispOn();
    GXS_DispOn();
    
    // SD���������ĂȂ�������h����܂ő҂�
    {
        FSArchiveResource res;
        while(!FS_GetArchiveResource("sdmc:/", &res))
        {
            *(u16*)HW_PLTT  =   static_cast<u16>(0x001f);
            *(u16*)HW_DB_PLTT  =   static_cast<u16>(0x001f);
        }
    }
    
    PathList path;
    
    for(u32 i = 0; i < sizeof(SUFFIX) / sizeof(char*); ++i)
    {
        PickupTargetPath("sdmc:/", SUFFIX[i], path);
    }
    CopyTarget("nand2:/photo", "sdmc:", path);
    
    
    *(u16*)HW_PLTT  =   0x001f << 10;
    *(u16*)HW_DB_PLTT  =   0x001f << 10;
    // �����`���[�ɖ߂��悤�ɁA �I�����Ȃ� 
    for (;;)
    {
        // �t���[���X�V�B 
        {
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem 

  Description:  ���荞�݂����������܂��B 

  Arguments:    �Ȃ��B 

  Returns:      �Ȃ��B 
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // �ʊ��荞�݃t���O��S�ĕs���� 
    (void)OS_SetIrqMask(0);

    // �}�X�^�[���荞�݃t���O������ 
    (void)OS_EnableIrq();

    // IRQ ���荞�݂������܂� 
    (void)OS_EnableInterrupts();
	
	(void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);
}


/*---------------------------------------------------------------------------*
  Name:         InitFileSystem 

  Description:  FS ������������ rom �ɃA�N�Z�X�ł���悤�ɂ��܂��B 
                ���̊֐����Ăяo���O�� InitInteruptSystem() �� 
                �Ă΂�Ă���K�v������܂��B 

  Arguments:    �Ȃ��B 

  Returns:      �Ȃ��B 
 *---------------------------------------------------------------------------*/
static void InitFileSystem()
{
    // ARM7�Ƃ̒ʐMFIFO���荞�݋��� 
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);

    // �t�@�C���V�X�e�������� 
    FS_Init( FS_DMA_NOT_USE );
}


static void PickupTargetPath(const char* root, const char* suffix, PathList& out)
{
    FSFile  fp;
    FSDirectoryEntryInfo    fdei;
    BOOL    result;
    
    FS_InitFile(&fp);
    // �f�B���N�g���̃I�[�v��
    result = FS_OpenDirectory(&fp, root, NULL);
    if(!result)
    {
        OS_TPrintf("%s: Fail Open\nErrorCode:%d\n", root, FS_GetArchiveResultCode(root));
        return;
    }
    
    while(FS_ReadDirectory(&fp, &fdei))
    {
        if(STD_StrCmp(fdei.longname, ".") == 0 || STD_StrCmp(fdei.longname, "..") == 0)
            continue;
        
        OS_TPrintf("%s:\n", fdei.longname);
        // �f�B���N�g���Ȃ�q��ǂ�
        if(fdei.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
        {
            PickupTargetPath((std::string(root) + fdei.longname + "/").c_str(), suffix, out); 
        }
        else    // �t�@�C���Ȃ�out�ɒǉ�
        {
            // �g���q�𔻒�
            if(std::string(fdei.longname).rfind(suffix) + STD_StrLen(suffix) == STD_StrLen(fdei.longname))
            {
                out.push_back(std::string(root) + fdei.longname);
            }
        }
    }
    
    FS_CloseDirectory(&fp);
    
}

static void CopyTarget(const char* to, const char* header, PathList& list)
{
    PathList::iterator itr = list.begin(), end = list.end();
    u16 color = 0;
    for(; itr != end; ++itr)
    {
        std::string dest = std::string(to) + (*itr).substr(static_cast<u32>(STD_StrLen(header)));
        
        CopyFile((*itr).c_str(), dest.c_str());
        color = static_cast<u16>((color + 2) & 0x001f);
        *(u16*)HW_PLTT = static_cast<u16>(color << 5);
        *(u16*)HW_DB_PLTT = static_cast<u16>(color << 5);
    }
}

static void CopyFile(const char* src, const char* dest)
{
    FSFile sfp, dfp;
    BOOL    result;
    std::string tmp_name = dest;
    
    tmp_name += "___tmp";
    
    OS_TPrintf("Copying %s...", dest);

    // �e���|�����t�@�C���ɃR�s�[
    FS_DeleteFile(tmp_name.c_str());
    result = FS_CreateFileAuto(tmp_name.c_str(), FS_PERMIT_R | FS_PERMIT_W);
//    result = FS_CreateFileAuto(dest, FS_PERMIT_R | FS_PERMIT_W);
    if(!result)
    {
        OS_TPrintf("%s:Cant create\n", tmp_name.c_str());
        return;
    }
    
    result = FS_OpenFileEx(&sfp, src, FS_FILEMODE_R);
    if(!result)
    {
        OS_TPrintf("%s:Cant open\n", src);
        return;
    }

    result = FS_OpenFileEx(&dfp, tmp_name.c_str(), FS_FILEMODE_W);
    if(!result)
    {
        OS_TPrintf("%s:Cant open\n", src);
        FS_CloseFile(&sfp);
        return;
    }

    const u32 BUFSIZE = 4096;
    u8 buf[BUFSIZE];
    s32 count = 0;
    while((count = FS_ReadFile(&sfp, buf, BUFSIZE)) > 0)
    {
        s32 write = FS_WriteFile(&dfp, buf, count);
        if(write <= 0)
        {
            OS_TPrintf("Cant Write\n");
            FS_CloseFile(&sfp);
            FS_CloseFile(&dfp);
            FS_DeleteFile(tmp_name.c_str());
            return;
        }
    }
    
    FS_CloseFile(&sfp);
    FS_CloseFile(&dfp);

    // �e���|�����t�@�C�������l�[��
    // ���łɃt�@�C�������݂���Ȃ�폜
    FS_DeleteFileAuto(dest);
    FS_RenameFile(tmp_name.c_str(), dest);
    
    OS_TPrintf("...ok\n");
}

