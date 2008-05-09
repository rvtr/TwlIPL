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
#include <firm/fs.h>
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
    void *const code_buf = (void*)OS_BOOT_CODE_BUF;   // 0x023fee00
    void *const stack_top = (void*)OS_BOOT_STACK_TOP; // (HW_DTCM_END - HW_DTCM_SYSRV_SIZE - HW_SVC_STACK_SIZE)
#else
    void *entry = rom_header->s.sub_entry_address;
    void *const code_buf = (void*)OS_BOOT_CODE_BUF;   // 0x03fff600
    void *const stack_top = (void*)OS_BOOT_STACK_TOP; // (HW_WRAM_AREA_END - HW_PRV_WRAM_SYSRV_SIZE - HW_SVC_STACK_SIZE)
#endif
    void *const wram_reg = rom_header->s.main_wram_config_data;
    REBOOTTarget target = rom_header->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK ? REBOOT_TARGET_TWL_SECURE : (rom_header->s.titleID_Hi & TITLE_ID_HI_APP_TYPE_MASK ? REBOOT_TARGET_TWL_SYSTEM : REBOOT_TARGET_TWL_APP);
    BOOL scfg = TRUE;          // no touch
    BOOL set_jtag = FALSE;     // no touch
    BOOL forbid_jtag = FALSE;  // no touch
    BOOL psram_4mb = FALSE;    // no touch
    static u32  mem_list[32];
    int i = 0;

    // pre clear
    /* 自身の static & bss のクリア */
    mem_list[i++] = (u32)SDK_STATIC_START;
    mem_list[i++] = (u32)SDK_STATIC_BSS_END-(u32)SDK_STATIC_START;
#ifdef SDK_ARM9
    /* ITCM全クリア (FromFrimを除く) */
    mem_list[i++] = HW_ITCM;
    mem_list[i++] = HW_FIRM_FROM_FIRM_BUF - HW_ITCM;
    mem_list[i++] = HW_FIRM_FROM_FIRM_BUF_END;
    mem_list[i++] = HW_ITCM_END - HW_FIRM_FROM_FIRM_BUF_END;
    /* FS/FATFSバッファのクリア */
    mem_list[i++] = (u32)HW_FIRM_FATFS_COMMAND_BUFFER;  // 0x02ff7800 - 0x02ffbfff
    mem_list[i++] = (u32)HW_FIRM_FS_TEMP_BUFFER_END - (u32)HW_FIRM_FATFS_COMMAND_BUFFER;
    /* 一部鍵バッファのクリア (鍵管理.xls参照) */
    mem_list[i++] = (u32)OSi_GetFromFirmAddr()->rsa_pubkey[0];
    mem_list[i++] = ACS_PUBKEY_LEN;
#else   // SDK_ARM7
    /* FS_Loader用バッファのクリア */
    mem_list[i++] = HW_FIRM_LOAD_BUFFER_BASE;
    mem_list[i++] = HW_FIRM_LOAD_BUFFER_UNIT_SIZE * HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
    {   /* REBOOT_ExecuteのCODEとSTACKの隙間をクリア */
        u32 stack_bottom = (u32)stack_top - OS_BOOT_STACK_SIZE_MIN - sizeof(mem_list);
        u32 code_buf_end = OS_BOOT_CODE_BUF + OS_BOOT_CODE_SIZE;
        SDK_ASSERT( stack_bottom > code_buf_end );
        mem_list[i++] = code_buf_end;
        mem_list[i++] = stack_bottom - code_buf_end;
    }
    /* 一部鍵バッファのクリア (鍵管理.xls参照) */
    mem_list[i++] = (u32)OSi_GetFromFirmAddr()->aes_key[2];
    mem_list[i++] = ACS_AES_LEN;
#endif  // SDK_ARM7
    mem_list[i++] = NULL;
    // copy forward
#ifdef SDK_ARM7
    // MountInfo (移動する？)
    mem_list[i++] = HW_TWL_FS_MOUNT_INFO_BUF;
    mem_list[i++] = (u32)rom_header->s.sub_mount_info_ram_address;
    mem_list[i++] = HW_FIRM_FS_MOUNT_INFO_BUF_SIZE;
    // srlファイル名
    mem_list[i++] = HW_TWL_FS_BOOT_SRL_PATH_BUF;
    mem_list[i++] = (u32)rom_header->s.sub_mount_info_ram_address + HW_FIRM_FS_MOUNT_INFO_BUF_SIZE;
    mem_list[i++] = HW_FIRM_FS_BOOT_SRL_PATH_BUF_SIZE;
#endif
    mem_list[i++] = NULL;
    // copy backward
    mem_list[i++] = NULL;
    // post clear
#ifdef SDK_ARM7
    // MountInfo (移動する？)
    mem_list[i++] = HW_TWL_FS_MOUNT_INFO_BUF;
    mem_list[i++] = HW_FIRM_FS_MOUNT_INFO_BUF_SIZE;
    // srlファイル名
    mem_list[i++] = HW_TWL_FS_BOOT_SRL_PATH_BUF;
    mem_list[i++] = HW_FIRM_FS_BOOT_SRL_PATH_BUF_SIZE;
#endif
    mem_list[i++] = NULL;
    SDK_ASSERT(i <= sizeof(mem_list)/sizeof(mem_list[0]));
#ifdef FIRM_USE_TWLSDK_KEYS
    // TwlSDK内の鍵を使っている時は量産用CPUではブートしない
#ifdef SDK_ARM9
    if ( ! ((*(u8*)OS_CHIPTYPE_DEBUGGER_ADDR & OS_CHIPTYPE_DEBUGGER_MASK)) )
#else   // SDK_ARM7
    if ( ! ((*(u8*)HWi_WSYS08_ADDR & HWi_WSYS08_OP_OPT_MASK)) )
#endif  // SDK_ARM7
    {
        OS_Terminate();
    }
#endif // FIRM_USE_SDK_KEYS
    REBOOT_Execute(entry, wram_reg, mem_list, code_buf, stack_top, target, scfg, set_jtag, forbid_jtag, psram_4mb);
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
    OSFromBromBuf* const fromBromBuf = OSi_GetFromBromAddr();
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
#ifdef SDK_ARM7
    MI_CpuCopyFast( &fromBromBuf->SDNandContext, (void*)HW_SD_NAND_CONTEXT_BUF, sizeof(SDPortContextData) );
    MI_CpuClearFast( &fromBromBuf->SDNandContext, sizeof(SDPortContextData) );
#endif
    return result;
}
