/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     

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

/*---------------------------------------------------------------------------*/
#include <nitro.h>
#include <nitro/fs.h>

#include "acsign.h"
#include "acmemory.h"

/*---------------------------------------------------------------------------*/
unsigned long binarray[ 1024 * 1024 * 2 / sizeof (unsigned long) ];
unsigned long keyarray[ 1024 / sizeof (unsigned long) ];
unsigned long sgnarray[ 1024 / sizeof (unsigned long) ];
unsigned long bufferA[ ACSIGN_BUFFER / sizeof (unsigned long) ];
unsigned long bufferB[ ACSIGN_BUFFER / sizeof (unsigned long) ];



/*--------------------------------------------------------------------------*/
// ROMヘッダ
//----------------------------------------------------------------------
typedef struct {
    //
    // 0x000 System Reserved
    //
    char        title_name[12];             // Soft title name
    u32     game_code;                  // Game code
    
    u16     maker_code;                 // Maker code
    u8      machine_code;               // Machine code
    u8      rom_type;                   // Rom type
    u8      rom_size;                   // Rom size
    
    u8      reserved_A[9];              // System Reserved A ( Set ALL 0 )
    u8      soft_version;               // Soft version

    u8      comp_arm9_boot_area:1;      // Compress arm9 boot area
    u8      comp_arm7_boot_area:1;      // Compress arm7 boot area
    u8      inspectCard:1;              // 検査カードフラグ
    u8      disableClearMemoryPad:1;    // IPL2メモリパッドクリア・ディセーブルフラグ
    u8      :0;

    
    //
    // 0x020 for Static modules (Section:B)
    //
    //  ARM9
    u32     main_rom_offset;            // ROM offset
    void*       main_entry_address;         // Entry point
    void*       main_ram_address;           // RAM address
    u32     main_size;              // Module size
    
    //  ARM7
    u32     sub_rom_offset;             // ROM offset
    void*       sub_entry_address;          // Entry point
    void*       sub_ram_address;            // RAM address
    u32     sub_size;               // Module size
    
    //
    // 0x040 for File Name Table[FNT] (Section:C)
    //
    u32     fnt_offset;             // ROM offset
    u32     fnt_size;               // Table size
    
    //
    // 0x048 for File Allocation Table[FAT] (Section:E)
    //
    u32     fat_offset;             // ROM offset
    u32     fat_size;               // Table size
    
    //
    // 0x050 for Overlay Tables[OVT] (Section:D)
    //
    //  ARM9
    u32     main_ovt_offset;            // ROM offset
    u32     main_ovt_size;              // Table size
    
    //  ARM7
    u32     sub_ovt_offset;             // ROM offset
    u32     sub_ovt_size;               // Table size
    
    // 0x060 for ROM control parameter
    u8      reserved_A2[32];
    
    // 0x080 - 0x0C0 System Reserved
    u8      reserved_B[64];             // System Reserved B (Set 0)
    
    // 0x0C0 for NINTENDO logo data
    u8      nintendo_logo[0x9c];        // NINTENDO logo data
    u16         nintendo_logo_crc16;        //            CRC-16
    
    // 0x15E ROM header CRC-16
    u16     header_crc16;               // ROM header CRC-16
} RomHeader;


/* V-blank callback */
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}





/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void*           tempLo;
    OSHeapHandle    hh;

    // OS_Initは呼ばれているという前提
    tempLo = OS_InitAlloc(
        OS_ARENA_MAIN ,
        OS_GetMainArenaLo() ,
        OS_GetMainArenaHi() ,
        1
    );
    OS_SetArenaLo(
        OS_ARENA_MAIN ,
        tempLo
    );
    hh = OS_CreateHeap(
        OS_ARENA_MAIN , 
        OS_GetMainArenaLo() ,
        OS_GetMainArenaHi()
    );
    if( hh < 0 )
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(
        OS_ARENA_MAIN ,
        hh
    );
}


//
static
void    TestAC(char* iplfile, char* srlfile, char* sgnfile, FSFile* pfile )
{
    RomHeader*  pHeader;
    const char* path;
    BOOL        fresult;
    void*       srl_ptr = 0;
    long        srl_len = 0;
    void*       key_ptr = 0;
    long        key_len = 0;
    void*       sgn_ptr = 0;
    long        sgn_len = 0;


    if ( !pfile )   return ;
    if ( !iplfile ) return ;
    if ( !srlfile ) return ;
    if ( !sgnfile ) return ;

    path = iplfile;
    if ( (fresult = FS_OpenFile(pfile, path)) )
    {
        key_len = FS_ReadFile( pfile, keyarray, sizeof keyarray );
        key_ptr = keyarray;
        fresult = FS_CloseFile( pfile );
    }
    if ( !fresult )
        OS_Printf("file read error! %s\n", path );

    path = srlfile;
    if ( (fresult = FS_OpenFile(pfile, path)) )
    {
        srl_len = FS_ReadFile( pfile, binarray, sizeof binarray );
        srl_ptr = binarray;
        fresult = FS_CloseFile( pfile );
    }
    if ( !fresult )
        OS_Printf("file read error! %s\n", path );

    path = sgnfile; //"/data/sgn0.bin";    //  sgn
    if ( (fresult = FS_OpenFile(pfile, path)) )
    {
        sgn_len = FS_ReadFile( pfile, sgnarray, sizeof sgnarray );
        sgn_ptr = sgnarray;
        fresult = FS_CloseFile( pfile );
    }
    if ( !fresult )
        OS_Printf("file read error! %s\n", path );

    //  認証
    if ( srl_ptr && srl_len && key_ptr && key_len && sgn_ptr && sgn_len )
    {
        long    nSerial = 0;
        pHeader = (RomHeader*)binarray;

        MI_CpuFill8( bufferA, 0, sizeof bufferA );
        MI_CpuFill8( bufferB, 0, sizeof bufferB );
        #if 0
        HMAC/Digtal Signature
        >char seg_id[2];        //->"ac"に固定
        >u16 version;           //-> 1に固定
        >u8 auth_code[20]/digital_sign[128]
        >long serial_number
        >char title_name[12];  // RomHeader.title_name
        >u32 game_code;       // RomHeader.game_code
        >u16 maker_code;      // RomHeader.make_code
        >u8 machine_code;    // RomHeader.machine_code
        #endif
        ((unsigned char*)&nSerial)[0] = ((unsigned char*)sgn_ptr + 4 + 128)[0];
        ((unsigned char*)&nSerial)[1] = ((unsigned char*)sgn_ptr + 4 + 128)[1];
        ((unsigned char*)&nSerial)[2] = ((unsigned char*)sgn_ptr + 4 + 128)[2];
        ((unsigned char*)&nSerial)[3] = ((unsigned char*)sgn_ptr + 4 + 128)[3];

        (void)ACSign_Decrypto( bufferA, 
                       (char*)sgn_ptr + 4,      // ファイル内の暗号化部分はへのオフセットをプラス
                       (char*)key_ptr + 16 );   //  PC側でMODのみのファイルに対応すれば+16が不要

        (void)ACSign_Digest( bufferB,
                       srl_ptr,
                       (char*)srl_ptr + pHeader->main_rom_offset, 
                       pHeader->main_size,
                       (char*)srl_ptr + pHeader->sub_rom_offset,
                       pHeader->sub_size,
                       nSerial );

        if ( ACSign_Compare( bufferA, bufferB ) )
        {
            OS_Printf( "Authentication_Code test : success! [%12s %12s] \n", srlfile, sgnfile );
        }
        else
        {
            OS_Printf( "Authentication_Code test : failure! [%12s %12s]\n", srlfile, sgnfile );
        }

    }
    else
    {
        OS_Printf( "no test\n" );
    }

    OS_PrintServer();
}


void    NitroMain(void)
{
    FSFile      file;


    OS_InitPrintServer();
    OS_Init();
    OS_InitThread();
    InitializeAllocateSystem();

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();

    if ( sizeof (int ) < sizeof ( long ) )
        OS_Printf( "sizeof (int ) != sizeof ( long )\n" );
    if ( sizeof ( RomHeader ) != 0x0160 )
        OS_Printf( "sizeof ( RomHeader ) != 0x0160\n" );

    /* initialize file-system */
    FS_Init( (u32)MI_DMA_MAX_NUM );     /* use DMA-3 for FS */

    /* always preload FS table for faster directory access. */
    {
        u32 need_size = FS_GetTableSize();
        void    *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

    //  ファイル読み込み
    FS_InitFile(&file);
    //TestAC( "/data/iplpub.bin", "/data/main.srl", "/data/sgn.bin", &file );

    TestAC( "/data/iplpub.bin", "/data/main0.srl", "/data/sgn0.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main1.srl", "/data/sgn1.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main2.srl", "/data/sgn2.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main3.srl", "/data/sgn3.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main4.srl", "/data/sgn4.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main5.srl", "/data/sgn5.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main6.srl", "/data/sgn6.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main7.srl", "/data/sgn7.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main0.srl", "/data/sgn7.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main3.srl", "/data/sgn1.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main5.srl", "/data/sgn5.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main1.srl", "/data/sgn0.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main0.srl", "/data/sgn1.bin", &file );
    TestAC( "/data/iplpub.bin", "/data/main0.srl", "/data/sgn0.bin", &file );

    OS_Terminate();
}

