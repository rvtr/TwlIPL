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
    void *stack_top = (void*)(HW_DTCM_END - HW_DTCM_SYSRV_SIZE - HW_SVC_STACK_SIZE - (u32)SDK_IRQ_STACKSIZE);
#else
    void *entry = rom_header->s.sub_entry_address;
    void *code_buf = (void*)OS_BOOT_CODE_BUF;   // 0x03fff600
    void *stack_top = (void*)(HW_WRAM_AREA_END - HW_PRV_WRAM_SYSRV_SIZE - HW_SVC_STACK_SIZE - (u32)SDK_IRQ_STACKSIZE);
#endif
    void *wram_reg = rom_header->s.main_wram_config_data;
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
#else
    /* CODEとSTACKの隙間をクリア */
    if ((u32)stack_top > (u32)OS_BOOT_CODE_BUF - OS_BOOT_CODE_SIZE - OS_BOOT_STACK_SIZE_MIN - sizeof(clr_list))
    {
        clr_list[i++] = (u32)OS_BOOT_CODE_BUF + (u32)OS_BOOT_CODE_SIZE;
        clr_list[i++] = (u32)stack_top - (u32)OS_BOOT_CODE_BUF - OS_BOOT_CODE_SIZE - OS_BOOT_STACK_SIZE_MIN - sizeof(clr_list);
    }
#endif
    clr_list[i++] = NULL;
    REBOOT_Execute(entry, wram_reg, clr_list, code_buf, stack_top, scfg, jtag);
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
