/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fs
  File:     fs_firm_ex.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <firm.h>

#if 0
    すべて確実ではないコード
    本来ならメインメモリ辺りにデータを置き、REBOOT内部でコピーする必要がある
#endif

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
    const char* const srl = HW_TWL_FS_BOOT_SRL_PATH_BUF;
    OSMountInfo* dest = (OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;

    MI_CpuCopy8(defaultMountList, dest, sizeof(defaultMountList));
    // nandアプリならパスを追加
    if ( *srl )
    {
        const char* lastpath = STD_SearchCharReverse( srl, '/' );
        u32 len = (u32)(lastpath - srl);
        if (len > OS_MOUNT_PATH_LEN)
        {
            len = OS_MOUNT_PATH_LEN;
        }
        MI_CpuCopy8(srl, dest[CONTENT_MOUNT_INDEX].path, len);
        if (len != OS_MOUNT_PATH_LEN)
        {
            dest[CONTENT_MOUNT_INDEX].path[len] = 0;
        }
    }
    // secureアプリならアクセス許可
    if ( rhs->titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK )
    {
        dest[NAND_MOUNT_INDEX].userPermission = dest[NAND2_MOUNT_INDEX].userPermission = (OS_MOUNT_USR_R|OS_MOUNT_USR_W);
    }
}
