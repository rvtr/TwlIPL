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
    STD_CopyString( "nand:/sys/HWINFO_S.dat", (void*)HW_FIRM_BOOT_SRL_PATH_BUF );   // dummy for firm
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetMountInfoForSrl

  Description:  set mount info for srl file

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FS_SetMountInfoForSrl( void )
{
    const OSMountInfo defaultMountList[] ATTRIBUTE_ALIGN(4) = {
    //  drive  device                target  pertitionIdx  resource           userPermission                rsvA  B  archive    path
        { 'A', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, 0, 0, 0, "nand",    "/" },    // ユーザーアプリはこのアーカイブではR/W不可
        { 'B', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, 0, 0, 0, "nand2",   "/" },    // ユーザーアプリはこのアーカイブではR/W不可
        { 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R), 0, 0, "content", NULL },            // Write不可
        { 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R), 0, 0, "shared1", "nand:/shared1" }, // Write不可
        { 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand:/shared2" },
        { 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  1, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
        { 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL }, // NANDにセーブデータがないアプリの場合は、マウントされない。
        { 'H', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL }, // NANDにセーブデータがないアプリの場合は、マウントされない。
        { 'I', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
    };
#define NAND_MOUNT_INDEX            0
#define NAND2_MOUNT_INDEX           1
#define CONTENT_MOUNT_INDEX         2
    const ROM_Header_Short* const rhs = HW_TWL_ROM_HEADER_BUF;
    OSMountInfo* const dest = (OSMountInfo*)HW_FIRM_MOUNT_INFO_BUF;

    MI_CpuCopy8(defaultMountList, dest, sizeof(defaultMountList));
    // nandアプリならパスを追加
    if ( *(char*)HW_FIRM_BOOT_SRL_PATH_BUF )
    {
        const char* lastpath = STD_SearchCharReverse( (char*)HW_FIRM_BOOT_SRL_PATH_BUF, '/' );
        if (lastpath && *(char*)(lastpath-1) != ':')
        {
            u32 len = (u32)(lastpath - (char*)HW_FIRM_BOOT_SRL_PATH_BUF);
            if (len > OS_MOUNT_PATH_LEN)
            {
                len = OS_MOUNT_PATH_LEN;
            }
            MI_CpuCopy8((char*)HW_FIRM_BOOT_SRL_PATH_BUF, dest[CONTENT_MOUNT_INDEX].path, len);
            if (len != OS_MOUNT_PATH_LEN)
            {
                ((char*)HW_FIRM_BOOT_SRL_PATH_BUF)[len] = 0;
            }
        }
    }
    // secureアプリならアクセス許可
    if ( rhs->titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK )
    {
        dest[NAND_MOUNT_INDEX].userPermission = dest[NAND2_MOUNT_INDEX].userPermission = (OS_MOUNT_USR_R|OS_MOUNT_USR_W);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenSrl

  Description:  open srl file named via HW_FIRM_TEMP_SRL_PATH_BUF

  Arguments:    None

  Returns:      file discriptor
 *---------------------------------------------------------------------------*/
int FS_OpenSrl( void )
{
    u16 fatpath[HW_FIRM_BOOT_SRL_PATH_BUF_SIZE];
    MI_CpuCopyFast((void*)HW_FIRM_TEMP_SRL_PATH_BUF, (void*)HW_FIRM_BOOT_SRL_PATH_BUF, HW_FIRM_BOOT_SRL_PATH_BUF_SIZE);
    ConvertPath(fatpath, (void*)HW_FIRM_BOOT_SRL_PATH_BUF, HW_FIRM_BOOT_SRL_PATH_BUF_SIZE);
    return FATFSi_rtfs_po_open((u8*)fatpath, 0, 0);
}
