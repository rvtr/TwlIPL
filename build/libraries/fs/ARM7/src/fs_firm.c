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

//#define WORKAROUND_NAND_2KB_BUG

#define FS_HEADER_AUTH_SIZE 0xe00

#define MODULE_ALIGNMENT    0x10    // 16バイト単位で読み込む
//#define MODULE_ALIGNMENT  0x200   // 512バイト単位で読み込む
#define RoundUpModuleSize(value)    (((value) + MODULE_ALIGNMENT - 1) & -MODULE_ALIGNMENT)

static ROM_Header* const rh= (ROM_Header*)HW_TWL_ROM_HEADER_BUF;

static BOOL         aesFlag;
static AESCounter   aesCounter;
static u8           aesBuffer[HW_FIRM_LOAD_BUFFER_UNIT_SIZE] ATTRIBUTE_ALIGN(32);

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
        for (info = OS_GetMountInfo(); *info->drive; ++info)
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
  Name:         FS_OpenSrl

  Description:  open srl file named in HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    None

  Returns:      file discriptor
 *---------------------------------------------------------------------------*/
int FS_OpenSrl( void )
{
    const char *fspath = (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF;
    u16 fatpath[OS_MOUNT_PATH_LEN];
    ConvertPath(fatpath, fspath, OS_MOUNT_PATH_LEN);
    return FATFSi_rtfs_po_open((u8*)fatpath, 0, 0);
}
