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
    REBOOTTarget target = REBOOT_TARGET_TWL_SECURE_SYSTEM;
    BOOL scfg = TRUE;   // no touch
    BOOL jtag = FALSE;  // no touch
    static u32  mem_list[32];
    int i = 0;

    // pre clear
    /* ���g�� static & bss �̃N���A */
    mem_list[i++] = (u32)SDK_STATIC_START;
    mem_list[i++] = (u32)SDK_STATIC_BSS_END-(u32)SDK_STATIC_START;
#ifdef SDK_ARM9
    /* ITCM�S�N���A */
    mem_list[i++] = (u32)HW_ITCM;
    mem_list[i++] = (u32)HW_ITCM_SIZE;
    /* FS/FATFS�o�b�t�@�̃N���A */
    mem_list[i++] = (u32)HW_FIRM_FATFS_COMMAND_BUFFER;  // 0x02ff7800 - 0x02ffbfff
    mem_list[i++] = (u32)HW_FIRM_FS_TEMP_BUFFER_END - (u32)HW_FIRM_FATFS_COMMAND_BUFFER;
    /* �ꕔ���o�b�t�@�̃N���A (���Ǘ�.xls�Q��) */
    mem_list[i++] = (u32)OSi_GetFromFirmAddr()->rsa_pubkey[0];
    mem_list[i++] = ACS_PUBKEY_LEN;
    mem_list[i++] = (u32)OSi_GetFromFirmAddr()->rsa_pubkey[2];
    mem_list[i++] = ACS_PUBKEY_LEN;
#else   // SDK_ARM7
    {   /* REBOOT_Execute��CODE��STACK�̌��Ԃ��N���A */
        u32 stack_bottom = (u32)stack_top - OS_BOOT_STACK_SIZE_MIN - sizeof(mem_list);
        u32 code_buf_end = OS_BOOT_CODE_BUF + OS_BOOT_CODE_SIZE;
        SDK_ASSERT( stack_bottom > code_buf_end );
        mem_list[i++] = code_buf_end;
        mem_list[i++] = stack_bottom - code_buf_end;
    }
    /* �ꕔ���o�b�t�@�̃N���A (���Ǘ�.xls�Q��) */
    // �Y������
#endif  // SDK_ARM7
    mem_list[i++] = NULL;
    // copy forward
    mem_list[i++] = NULL;
    // copy backward
    mem_list[i++] = NULL;
    // post clear
    mem_list[i++] = NULL;
    SDK_ASSERT(i <= sizeof(mem_list)/sizeof(mem_list[0]));

#ifdef FIRM_USE_TWLSDK_KEYS
    // TwlSDK���̌����g���Ă��鎞�͗ʎY�pCPU�ł̓u�[�g���Ȃ�
#ifdef SDK_ARM9
    if ( ! ((*(u8*)OS_CHIPTYPE_DEBUGGER_ADDR & OS_CHIPTYPE_DEBUGGER_MASK)) )
#else   // SDK_ARM7
    if ( ! ((*(u8*)HWi_WSYS08_ADDR & HWi_WSYS08_OP_OPT_MASK)) )
#endif  // SDK_ARM7
    {
        OS_Terminate();
    }
#endif // FIRM_USE_SDK_KEYS

    REBOOT_Execute(entry, wram_reg, mem_list, code_buf, stack_top, target, scfg, jtag);
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
    return result;
}
