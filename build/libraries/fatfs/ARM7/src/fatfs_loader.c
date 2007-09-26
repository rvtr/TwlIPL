/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fatfs
  File:     fatfs_loader.c

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

#include <symbols.h>

#include <firm/fatfs.h>
#include <firm/sdmc.h>
#include <firm/format/format_rom.h>
#include <twl/aes.h>
#include <twl/aes/ARM7/lo.h>
#include <rtfs.h>

#define PXI_FIFO_TAG_DATA   PXI_FIFO_TAG_USER_0

static ROM_Header* const rh= (ROM_Header*)(HW_MAIN_MEM_SYSTEM_END - 0x2000);
static int menu_fd = -1;

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenRecentMenu

  Description:  open recent menu file

  Arguments:    driveno     drive number ('A' is 0)

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenRecentMenu( int driveno )
{
    char *menufile = (char*)L"A:\\ipl\\menu.srl";
    if (driveno < 0 || driveno >= 26)
    {
        return FALSE;
    }
    menufile[0] = (char)('A' + driveno);
    menu_fd = po_open((u8*)menufile, PO_BINARY, 0);
    if (menu_fd < 0)
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenSpecifiedMenu

  Description:  open specified menu file

  Arguments:    menufile    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenSpecifiedMenu( const char* menufile )
{
    menu_fd = po_open((u8*)menufile, PO_BINARY, 0);
    if (menu_fd < 0)
    {
        return FALSE;
    }
    return TRUE;
}

#define HEADER_SIZE 0x1000
#define AUTH_SIZE   0xe00

#define SLOT_SIZE   0x8000

static BOOL FATFS_LoadBuffer(u32 offset, u32 size)
{
    u8* base = (u8*)MI_GetWramMapStart_B();
    static int count = 0;

    // seek first
    if (po_lseek(menu_fd, (s32)offset, PSEEK_SET) < 0)
    {
        return FALSE;
    }
    // loading loop
    while (size > 0)
    {
        u8* dest = base + count * SLOT_SIZE;                    // target buffer address
        u32 unit = size < SLOT_SIZE ? size : SLOT_SIZE;         // size
        while (MI_GetWramBankMaster_B(count) != MI_WRAM_ARM7)   // waiting to be master
        {
        }
        if (po_read(menu_fd, (u8*)dest, (int)unit) < 0)            // reading
        {
            return FALSE;
        }
        PXI_NotifyID( FIRM_PXI_ID_LOAD_PIRIOD );
        count = (count + 1) & 0x7;
        size -= unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadHeader

  Description:  load menu header

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadHeader( void )
{
    // open the file in FATFS_InitFIRM()
    if (menu_fd < 0)
    {
        return FALSE;
    }

    // load header without AES
    PXI_NotifyID( FIRM_PXI_ID_LOAD_HEADER );
    FATFS_DisableAES();
    if (!FATFS_LoadBuffer(0, AUTH_SIZE) ||
        !FATFS_LoadBuffer(AUTH_SIZE, HEADER_SIZE - AUTH_SIZE) ||
        PXI_RecvID() != FIRM_PXI_ID_AUTH_HEADER )
    {
        return FALSE;
    }

    // set id depends on game_code and seed to use (or all?)
    {
        AESKeySeed seed;
        AESi_InitGameKeys((u8*)rh->s.game_code);
//        PXI_RecvDataByFifo( PXI_FIFO_TAG_DATA, &seed, AES_BLOCK_SIZE );
        AESi_SetKeySeedA(&seed);    // APP
        //AESi_SetKeySeedB(&seed);    // APP & HARD
        //AESi_SetKeySeedC(&seed);    //
        //AESi_SetKeySeedD(&seed);    // HARD
    }

    return TRUE;
}

static void FATFSi_AddCounter(AESCounter* pCounter, u32 nums)
{
    u32 data = 0;
    int i;
    for (i = 15; i >= 0; i--)
    {
        data += pCounter->bytes[i] + (nums & 0xFF);
        pCounter->bytes[i] = (u8)(data & 0xFF);
        data >>= 8;
        nums >>= 8;
        if ( !data && !nums )
        {
            break;
        }
    }
}

static AESCounter* FATFSi_GetCounter( u32 offset )
{
    static AESCounter counter;
    MI_CpuCopy8(rh->s.main_static_digest, &counter, 12);
    counter.words[3] = 0;
    FATFSi_AddCounter(&counter, offset - SECURE_AREA_START);
    return &counter;
}


/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadMenu

  Description:  load menu binary

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadMenu( void )
{
    // load ARM9 static region without AES
    if ( rh->s.main_size > 0 )
    {
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM9_STATIC );
        FATFS_DisableAES();
        if ( !FATFS_LoadBuffer( rh->s.main_rom_offset, rh->s.main_size ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM9_STATIC )
        {
            return FALSE;
        }
    }
    // load ARM7 static region without AES
    if ( rh->s.sub_size > 0 )
    {
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM7_STATIC );
        FATFS_DisableAES();
        if ( !FATFS_LoadBuffer( rh->s.sub_rom_offset, rh->s.sub_size ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM7_STATIC )
        {
            return FALSE;
        }
    }
    // load ARM9 extended static region with AES
    if ( rh->s.main_ex_size > 0 )
    {
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM9_STATIC_EX );
        if ( !rh->s.enable_aes || !rh->s.enable_signature )
        {
            FATFS_DisableAES();
        }
        else
        {
            FATFS_EnableAES( AES_KEY_SLOT_A, FATFSi_GetCounter( rh->s.main_ex_rom_offset ) );
        }
        if ( !FATFS_LoadBuffer( rh->s.main_ex_rom_offset, rh->s.main_ex_size ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM9_STATIC_EX )
        {
            return FALSE;
        }
    }
    // load ARM7 extended static region with AES
    if ( rh->s.sub_ex_size > 0 )
    {
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM7_STATIC_EX );
        if ( !rh->s.enable_aes || !rh->s.enable_signature )
        {
            FATFS_DisableAES();
        }
        else
        {
            FATFS_EnableAES( AES_KEY_SLOT_A, FATFSi_GetCounter( rh->s.sub_ex_rom_offset ) );
        }
        if ( !FATFS_LoadBuffer( rh->s.sub_ex_rom_offset, rh->s.sub_ex_size ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM7_STATIC_EX )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_BootMenu

  Description:  boot menu

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_BootMenu( void )
{
    OSi_Boot( rh->s.sub_entry_address, (MIHeader_WramRegs*)rh->s.main_wram_config_data );
}
