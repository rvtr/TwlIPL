/*---------------------------------------------------------------------------*
  Project:  TwlFirm - OS
  File:     os_boot.c

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

#include <firm/os.h>
#include "reboot.h"

extern void SDK_STATIC_START(void);     // static and bss start address
extern void SDK_STATIC_BSS_END(void);   // static and bss end address

/*---------------------------------------------------------------------------*
  Name:         OS_BootWithRomHeaderFromFIRM

  Description:  boot with ROM header

  Arguments:    rom_header  :  ROM header

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_BootWithRomHeaderFromFIRM( ROM_Header* rom_header )
{
#ifdef SDK_ARM9
    void *entry = rom_header->s.main_entry_address;
    void *code_buf = (void*)OS_BOOT_CODE_BUF;   // 0x023fee00
    void *stack_top = (void*)OS_BOOT_STACK_TOP; // (HW_DTCM_END - HW_DTCM_SYSRV_SIZE - HW_SVC_STACK_SIZE)
#else
    void *entry = rom_header->s.sub_entry_address;
    void *code_buf = (void*)OS_BOOT_CODE_BUF;   // 0x03fff600
    void *stack_top = (void*)OS_BOOT_STACK_TOP; // (HW_WRAM_AREA_END - HW_PRV_WRAM_SYSRV_SIZE - HW_SVC_STACK_SIZE)
#endif
    void *wram_reg = rom_header->s.main_wram_config_data;
    REBOOTTarget target = REBOOT_TARGET_NAND_MENU;
    BOOL scfg = TRUE;   // no touch
    BOOL jtag = FALSE;  // no touch
    static u32  clr_list[32];
    int i = 0;

    /* 自身の static & bss のクリア */
    clr_list[i++] = (u32)SDK_STATIC_START;
    clr_list[i++] = (u32)SDK_STATIC_BSS_END-(u32)SDK_STATIC_START;
#ifdef SDK_ARM9
    /* ITCM全クリア */
    clr_list[i++] = (u32)HW_ITCM;
    clr_list[i++] = (u32)HW_ITCM_SIZE;
    /* PSEG1 Reserved領域のクリア (暫定) */
    clr_list[i++] = (u32)HW_MAIN_MEM_SHARED;        // 0x02fff000 - 0x02fff7ff
    clr_list[i++] = (u32)HW_PSEG1_RESERVED_0_END - (u32)HW_MAIN_MEM_SHARED;
    clr_list[i++] = (u32)HW_PSEG1_RESERVED_1;        // 0x02fffa00 - 0x02fffa7f
    clr_list[i++] = (u32)HW_PSEG1_RESERVED_1_END - (u32)HW_PSEG1_RESERVED_1;
    /* System Shared領域のクリア (暫定) */
    clr_list[i++] = (u32)HW_BOOT_CHECK_INFO_BUF;    // 0x02fffc00 - 0x02fffc1f
    clr_list[i++] = (u32)HW_BOOT_CHECK_INFO_BUF_END - (u32)HW_BOOT_CHECK_INFO_BUF;
    clr_list[i++] = (u32)HW_BOOT_SHAKEHAND_9;       // 0x02fffc24 - 0x02fffd7f
    clr_list[i++] = (u32)HW_NVRAM_USER_INFO_END - (u32)HW_BOOT_SHAKEHAND_9;
    clr_list[i++] = (u32)HW_ARENA_INFO_BUF;         // 0x02fffda0 - 0x02fffdff
    clr_list[i++] = (u32)HW_ROM_HEADER_BUF - (u32)HW_ARENA_INFO_BUF;
    clr_list[i++] = (u32)HW_PXI_SIGNAL_PARAM_ARM9;  // 0x02ffff80 - 0x02fffffd
    clr_list[i++] = (u32)HW_CMD_AREA - (u32)HW_PXI_SIGNAL_PARAM_ARM9;
#else   // SDK_ARM7
    {   /* REBOOT_ExecuteのCODEとSTACKの隙間をクリア */
        u32 stack_bottom = (u32)stack_top - OS_BOOT_STACK_SIZE_MIN - sizeof(clr_list);
        u32 code_buf_end = OS_BOOT_CODE_BUF + OS_BOOT_CODE_SIZE;
        SDK_ASSERT( stack_bottom > code_buf_end );
        clr_list[i++] = code_buf_end;
        clr_list[i++] = stack_bottom - code_buf_end;
    }
#endif  // SDK_ARM7
    clr_list[i++] = NULL;
    REBOOT_Execute(entry, wram_reg, clr_list, code_buf, stack_top, target, scfg, jtag);
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         OSi_FromBromToMenu

  Description:  convert OSFromBromBuf to OSFromFirmBuf

  Arguments:    None

  Returns:      FALSE if FromBrom is broken
 *---------------------------------------------------------------------------*/
BOOL OSi_FromBromToMenu( void )
{
    OSFromBromBuf* fromBromBuf = OSi_GetFromBromAddr();
    BOOL result = TRUE;
    int i;
    // check offset (why not to omit by compiler?)
    if ( OSi_GetFromFirmAddr()->rsa_pubkey != fromBromBuf->rsa_pubkey ) // same area without header
    {
        result = FALSE;
    }
    // check unused signature area
    for (i = 0; i < sizeof(fromBromBuf->hash_table_hash); i++)  // check all values are same
    {
        if (fromBromBuf->hash_table_hash[i] != 0x00)
        {
            result = FALSE;
        }
    }
    // clear out of OSFromFirmBuf area
    MI_CpuClearFast( fromBromBuf->header.max, sizeof(fromBromBuf->header.max) );
    return result;
}
