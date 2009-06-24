/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fs
  File:     fs_firm.c

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
#include <firm.h>
#include <rtfs.h>

static void ConvertPath( u16* dest, const char* src, u32 max)
{
    dest[0] = 0;
    {
        const OSMountInfo *info;
        int len;
        // デバイス部分を取得
        for (len = 0; src[len] && src[len] != ':'; len++)
        {
            if (len >= max) // もっと手前で止めても良い？
            {
                OS_TPrintf("%s: Cannot detect ':' in %d charactors.\n", __func__, len);
                return;
            }
        }

        // ドライブ名の解決
        for (info = (void*)HW_FIRM_MOUNT_INFO_BUF; *info->drive; ++info)
        {
            if ((STD_CompareNString(src, info->archiveName, len) == 0) &&
                (info->archiveName[len] == 0))
            {
                if (info->target != OS_MOUNT_TGT_ROOT)  // 多重マウント未対応
                {
                    return;
                }
                dest[0] = (u16)*info->drive;
                break;
            }
        }
        max --;
        dest++;
        src += len;
    }
    {
        int len;
        // 残りunicode化 (ASCIIのみ)
        for (len = 0; len < max && src[len]; len++)
        {
            if (src[len] == '/')
            {
                dest[len] = L'\\';
            }
#ifndef SDK_FINALROM
            else if (src[len] & 0x80)
            {
                OS_TPrintf("%s: Multi-byte charactor was detected (0x%02X).\n", __func__, src[len]);
                dest[len] = (u16)src[len];  // ignore but maybe broken
            }
#endif
            else
            {
                dest[len] = (u16)src[len];
            }
        }
        if (len < max)
        {
            dest[len] = 0;
        }
        else    // l type (ensure to terminate zero)
        {
            dest[max-1] = 0;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_InitMountInfo

  Description:  set mount info for firm

  Arguments:    nand    TRUE if set nand entry
                sdmc    TRUE if set sdmc entry

  Returns:      None
 *---------------------------------------------------------------------------*/
void FS_InitMountInfo( BOOL nand, BOOL sdmc )
{
    const OSMountInfo nandSetting = { 'A', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand", "/" };
    const OSMountInfo sdmcSetting = { 'B', OS_MOUNT_DEVICE_SD, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc", "/" };
    OSMountInfo* dest = (OSMountInfo*)HW_FIRM_MOUNT_INFO_BUF;

    if ( nand )
    {
        MI_CpuCopy8(&nandSetting, dest++, sizeof(OSMountInfo));
    }
    if ( sdmc )
    {
        MI_CpuCopy8(&sdmcSetting, dest++, sizeof(OSMountInfo));
    }
    if ( dest < (void*)HW_FIRM_BOOT_SRL_PATH_BUF )
    {
        *(u32*)dest = 0;
    }
    *(u8*)HW_TWL_RED_LAUNCHER_VER = 1;
    *(char*)HW_FIRM_BOOT_SRL_PATH_BUF = 0; // no rom archive
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenSrl

  Description:  open srl file named via HW_FIRM_TEMP_SRL_PATH_BUF

  Arguments:    None

  Returns:      file discriptor
 *---------------------------------------------------------------------------*/
int FS_OpenSrl( void )
{
    u16 fatpath[HW_FIRM_FS_BOOT_SRL_PATH_BUF_SIZE];
    ConvertPath(fatpath, (void*)HW_TWL_FS_BOOT_SRL_PATH_BUF, HW_FIRM_FS_BOOT_SRL_PATH_BUF_SIZE);
    return FATFSi_rtfs_po_open((u8*)fatpath, 0, 0);
}
