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

#ifndef SDK_FINALROM
#define PROFILE_PXI_SEND    1000000000
#define PROFILE_PXI_RECV    2000000000
extern u32 profile[];
extern u32 pf_cnt;
#endif

static BOOL FATFS_LoadBuffer(u32 offset, u32 size)
{
    u8* base = (u8*)MI_GetWramMapStart_B();
    static int count = 0;

    // seek first
    if (po_lseek(menu_fd, (s32)offset, PSEEK_SET) < 0)
    {
        return FALSE;
    }
#ifndef SDK_FINALROM
    // x2: after Seek
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    // loading loop
    while (size > 0)
    {
        u8* dest = base + count * SLOT_SIZE;                    // target buffer address
        u32 unit = size < SLOT_SIZE ? size : SLOT_SIZE;         // size
        while (MI_GetWramBankMaster_B(count) != MI_WRAM_ARM7)   // waiting to be master
        {
        }
#ifndef SDK_FINALROM
        // x3...: after to wait ARM9
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if (po_read(menu_fd, (u8*)dest, (int)unit) < 0)            // reading
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // x4...: before PXI
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_PIRIOD;    // checkpoint
#endif
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

#ifndef SDK_FINALROM
    // 10: before PXI
    pf_cnt = 10;
    profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_HEADER;    // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    // load header without AES
    PXI_NotifyID( FIRM_PXI_ID_LOAD_HEADER );
    FATFS_DisableAES();
    if (!FATFS_LoadBuffer(0, AUTH_SIZE) ||
#ifndef SDK_FINALROM
        // 12: after to load half
        ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
        !FATFS_LoadBuffer(AUTH_SIZE, HEADER_SIZE - AUTH_SIZE) ||
#ifndef SDK_FINALROM
        // 1x: after to load remain
        ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
        PXI_RecvID() != FIRM_PXI_ID_AUTH_HEADER )
    {
        return FALSE;
    }
#ifndef SDK_FINALROM
    // 1x: after PXI
    profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_HEADER;    // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    // set id depends on game_code and seed to use (or all?)
    {
        AESKeySeed seed;
        AESi_InitGameKeys((u8*)rh->s.game_code);
        PXI_RecvDataByFifo( PXI_FIFO_TAG_DATA, &seed, AES_BLOCK_SIZE );
        AESi_SetKeySeedA(&seed);    // APP
        //AESi_SetKeySeedB(&seed);    // APP & HARD
        //AESi_SetKeySeedC(&seed);    //
        //AESi_SetKeySeedD(&seed);    // HARD
        AESi_SetKeyC(&seed);        // Direct
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFSi_GetCounter

  Description:  get counter

  Arguments:    offset  offset from head of ROM_Header

  Returns:      counter
 *---------------------------------------------------------------------------*/
static AESCounter* FATFSi_GetCounter( u32 offset )
{
    static AESCounter counter;
    MI_CpuCopy8(rh->s.main_static_digest, &counter, 12);
    counter.words[3] = 0;
    AESi_AddCounter(&counter, offset - offsetof(ROM_Header, s.main_ltd_rom_offset));
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
#ifndef SDK_FINALROM
        // 30: before PXI
        pf_cnt = 30;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_ARM9_STATIC;   // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM9_STATIC );
        FATFS_DisableAES();
        if ( !FATFS_LoadBuffer( rh->s.main_rom_offset, rh->s.main_size ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM9_STATIC )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // 3x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_ARM9_STATIC;   // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    }
    // load ARM7 static region without AES
    if ( rh->s.sub_size > 0 )
    {
#ifndef SDK_FINALROM
        // 50: before PXI
        pf_cnt = 50;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_ARM7_STATIC;   // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM7_STATIC );
        FATFS_DisableAES();
        if ( !FATFS_LoadBuffer( rh->s.sub_rom_offset, rh->s.sub_size ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM7_STATIC )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // 5x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_ARM7_STATIC;   // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    }
    // load ARM9 extended static region with AES
    if ( rh->s.main_ltd_size > 0 )
    {
#ifndef SDK_FINALROM
        // 70: before PXI
        pf_cnt = 70;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_ARM9_LTD_STATIC;    // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM9_LTD_STATIC );
        if ( !rh->s.enable_aes )
        {
            FATFS_DisableAES();
        }
        else
        {
            AESi_WaitKey();
            AESi_LoadKey( AES_KEY_SLOT_A );
            FATFS_EnableAES( FATFSi_GetCounter( rh->s.main_ltd_rom_offset ) );
        }
        if ( !FATFS_LoadBuffer( rh->s.main_ltd_rom_offset, rh->s.main_ltd_size ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM9_LTD_STATIC )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // 7x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_ARM9_LTD_STATIC;    // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    }
    // load ARM7 extended static region with AES
    if ( rh->s.sub_ltd_size > 0 )
    {
#ifndef SDK_FINALROM
        // 90: before PXI
        pf_cnt = 90;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_ARM7_LTD_STATIC;    // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM7_LTD_STATIC );
        if ( !rh->s.enable_aes )
        {
            FATFS_DisableAES();
        }
        else
        {
            AESi_WaitKey();
            AESi_LoadKey( AES_KEY_SLOT_A );
            FATFS_EnableAES( FATFSi_GetCounter( rh->s.sub_ltd_rom_offset ) );
        }
        if ( !FATFS_LoadBuffer( rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM7_LTD_STATIC )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // 9x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_ARM7_LTD_STATIC;    // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
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
