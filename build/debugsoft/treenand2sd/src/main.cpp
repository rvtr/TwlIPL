/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - treenand
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

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
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include <sd_log.h>
#include "text.h"
#include "MyStrings.h"

inline void* operator new(size_t size)      { return OS_Alloc(size); }
inline void* operator new[](size_t size)    { return OS_Alloc(size); }
inline void operator delete(void* ptr)      { OS_Free(ptr); }
inline void operator delete[](void* ptr)    { OS_Free(ptr); }

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

    void
    MyInit(void)
    {
        OS_Init();
        FX_Init();

        GX_Init();

        (void)OS_EnableIrqMask( OS_IE_V_BLANK );
        (void)OS_EnableIrq();

        FS_Init(MI_DMA_MAX_NUM);

        GX_DispOff();
        GXS_DispOff();

        (void)GX_VBlankIntr(TRUE);         // to generate VBlank interrupt request
        //---------------------------------------------------------------------------
        // All VRAM banks to LCDC
        //---------------------------------------------------------------------------
        GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

        //---------------------------------------------------------------------------
        // Clear all LCDC space
        //---------------------------------------------------------------------------
        MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

        //---------------------------------------------------------------------------
        // Disable the banks on LCDC
        //---------------------------------------------------------------------------
        (void)GX_DisableBankForLCDC();

        MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);      // clear OAM
        MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);        // clear the standard palette

        MI_CpuFillFast((void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE); // clear OAM
        MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);  // clear the standard palette

        GX_DispOn();
        GXS_DispOn();

        *(vu16 *) REG_IME_ADDR = 1;                            // IME セット

        wInitTextVram();

        //SDの存在チェック
        {
            FSPathInfo info;

            if(FS_GetPathInfo( "sdmc:/", &info ) )
            {
                if( (info.attributes&FS_ATTRIBUTE_IS_PROTECTED)==FS_ATTRIBUTE_IS_PROTECTED )
                {
                    wSetPalette(COLOR_RED);
                    wPrintf("\n\n  SD CARD IS PROTECTED!!");
                    OS_Terminate();
                }
                else
                {
                    return;
                }
            }
            else
            {
                wSetPalette(COLOR_RED);
                wPrintf("\n\n  SD CARD NOTHING!!");
                OS_Terminate();
            }
        }

    }
}

namespace
{
    class DirEntry;
    typedef std::vector<DirEntry> EntryList;
    class DirEntry
    {
    private:
        FSDirectoryEntryInfo  m_dei;
        std::string           m_path;
        EntryList*            m_pEntryList;

    public:
        DirEntry(std::string parent, const FSDirectoryEntryInfo& dei)
            : m_path(parent + "/" + dei.longname)
            , m_dei(dei)
            , m_pEntryList(NULL)
        {}
        ~DirEntry(){ delete m_pEntryList; }
        bool operator <(const DirEntry& rhs) const
        {
            return this->m_path < rhs.m_path;
        }

        std::string GetPath()      const { return m_path; }
        std::string GetName()      const { return m_dei.longname; }
        std::string GetShortName() const { return m_dei.shortname; }
        u32 GetSize()              const { return m_dei.filesize; }

        bool IsDirectory() const { return (m_dei.attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0; }
        bool IsReadOnly()  const { return (m_dei.attributes & FS_ATTRIBUTE_DOS_READONLY) != 0; }
        bool IsHidden()    const { return (m_dei.attributes & FS_ATTRIBUTE_DOS_HIDDEN)   != 0; }
        bool IsSystem()    const { return (m_dei.attributes & FS_ATTRIBUTE_DOS_SYSTEM)   != 0; }
        bool IsVolume()    const { return (m_dei.attributes & FS_ATTRIBUTE_DOS_VOLUME)   != 0; }
        bool IsArchive()   const { return (m_dei.attributes & FS_ATTRIBUTE_DOS_ARCHIVE)  != 0; }

        void SetEntryList(EntryList* pEntryList)
        {
            delete m_pEntryList;
            m_pEntryList = pEntryList;
        }
        const EntryList& GetEntryList() const
        {
            return *m_pEntryList;
        }
        std::string GetDateTimeString() const
        {
            std::ostringstream oss;

            oss << std::setw(4) << std::setfill('0') << m_dei.mtime.year   << "/";
            oss << std::setw(2) << std::setfill('0') << m_dei.mtime.month  << "/";
            oss << std::setw(2) << std::setfill('0') << m_dei.mtime.day    << " ";
            oss << std::setw(2) << std::setfill('0') << m_dei.mtime.hour   << ":";
            oss << std::setw(2) << std::setfill('0') << m_dei.mtime.minute << ":";
            oss << std::setw(2) << std::setfill('0') << m_dei.mtime.second;

            return oss.str();
        }
    };

    typedef std::vector<DirEntry> EntryList;

    void
    GetDirEntry(EntryList* pList, std::string path)
    {
        BOOL bSuccess;
        FSFile dir;
        FSDirectoryEntryInfo dei;

        pList->clear();

        FS_InitFile(&dir);
        bSuccess = FS_OpenDirectory(&dir, path.c_str(), FS_FILEMODE_R);
        if( bSuccess )
        {
            while( FS_ReadDirectory(&dir, &dei) )
            {
                std::string name = dei.longname;

                if( (name == ".") || (name == "..") )
                {
                    continue;
                }

                pList->push_back( DirEntry(path, dei) );
            }

            FS_CloseDirectory(&dir);
        }

        std::sort(pList->begin(), pList->end());
    }

    void
    GetDirEntryRecursive(EntryList* pList, std::string path)
    {
        EntryList::iterator i;

        GetDirEntry(pList, path);

        for( i = pList->begin(); i != pList->end(); ++i )
        {
            if( i->IsDirectory() )
            {
                EntryList* pSubEntryList = new EntryList;
                GetDirEntryRecursive(pSubEntryList, i->GetPath());
                i->SetEntryList(pSubEntryList);
            }
        }
    }

    void
    DrawEntries(std::string prevPart, std::string prevPart2, const EntryList& elist)
    {
        EntryList::const_iterator i;

        for( i = elist.begin(); i != elist.end(); ++i )
        {
            std::ostringstream oss, oss2;
            const bool isLast = (i + 1 == elist.end());

            oss << prevPart;
            oss2<< prevPart2;
            oss << (isLast  ? "└": "├" );
            oss2<< "+";
            oss << " " << i->GetName();
            oss2<< " " << i->GetName();
            if( i->IsDirectory() )
            {
                oss << "/";
                oss2<< "/";

            }else
            {
                char str[0x100]="";
                char size[20]="";
                int  offset,j;

                offset = 60-STD_StrLen(oss.str().c_str());
                for(j=0;j<offset;j++) str[j]=' ';
                STD_TSPrintf(size,"(%d)",i->GetSize());
                STD_StrCat(str,size);

                oss << " " << str;
                oss2<< " " << size;
            }
            oss << std::endl;
            oss2<< std::endl;

            OS_PutString(oss.str().c_str());
            SDLOG_Printf(oss.str().c_str());
            wPrintf(oss2.str().c_str());
            OS_Sleep(100);

            if( i->IsDirectory() )
            {
                DrawEntries(prevPart + (isLast ? "   ": "｜ "), (prevPart2 + "  "), i->GetEntryList());
            }
        }
    }

    void
    TreeNand(const char* root)
    {
        EntryList elist;
        GetDirEntryRecursive(&elist, root);

        OS_TPrintf("%s\n", root);
        DrawEntries("","", elist);
    }
}


void
TwlStartUp()
{
    OS_Init();
    InitAlloc();
}

// 文字表示
static void PutString( char *format, ... )
{
    u16             *dest = (u16*)G2_GetBG1ScrPtr();
    char            temp[32+1];
    int             i;
    va_list         va;

    va_start(va, format);
    (void)OS_VSNPrintf(temp, sizeof(temp), format, va);
    va_end(va);

    for (i = 0; i < 32 && temp[i]; i++)
    {
        dest[i] = (u16)((u8)temp[i] | (1 << 12));
    }
}

#define BUFFER_SIZE 0x900
#define PARTITION_SIZE 0x300
char logBuffer[ (BUFFER_SIZE + 1) ];
void
TwlMain(void)
{

    MyInit();

    if( !SDLOG_Init("sdmc:/treenand2sd/", logBuffer, BUFFER_SIZE, PARTITION_SIZE, SDLOG_WRITE_TYPE_CONTINUANCE) )
    {
        OS_Panic("Error");
    }

    OS_TPrintf("treenand start.\n");
    OS_TPrintf("---------------------------------------------------------------\n");
    wPrintf("\fnand:\n");
    wPrintf("-------------------------------\n");
    TreeNand("nand:");

    wPrintf("\n\nnand2:\n");
    wPrintf("-------------------------------\n");
    TreeNand("nand2:");
    OS_TPrintf("---------------------------------------------------------------\n");
    OS_TPrintf("treenand completed.\n");

    SDLOG_Flush();
    SDLOG_Finish();
    OS_Printf("Program is finished!\n");

    wSetPalette(COLOR_BLUE);
    wPrintf("\n      !Program is finished!\n");

    OS_Terminate();
}

