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
#include <twl/aes/common/aes.h>

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

#define DMA_SEND         2
#define DMA_RECV         3
static void CopyWithAes( const void* src, void* dest, u32 size )
{
    AESi_Reset();
    AESi_Reset();
    AESi_DmaSend( DMA_SEND, src,  size, NULL, NULL );
    AESi_DmaRecv( DMA_RECV, dest, size, NULL, NULL );
    AESi_SetCounter( &aesCounter );
    AESi_Run( AES_MODE_CTR, 0, size / AES_BLOCK_SIZE, NULL, NULL );
    AES_AddToCounter( &aesCounter, size / AES_BLOCK_SIZE );
    MI_WaitNDma( DMA_RECV );
}

static void EnableAes( u32 offset )
{
    aesFlag = TRUE;
    MI_CpuCopy8( rh->s.main_static_digest, &aesCounter, AES_BLOCK_SIZE );
    AES_AddToCounter( &aesCounter, (offset - rh->s.aes_target_rom_offset) / AES_BLOCK_SIZE );
}
static void DisableAes( void )
{
    aesFlag = FALSE;
}

static u32 GetTransferSize( u32 offset, u32 size )
{
    u32 aes_offset = rh->s.aes_target_rom_offset;
    u32 aes_end = aes_offset + RoundUpModuleSize(rh->s.aes_target_size);
    u32 end = offset + RoundUpModuleSize(size);
    if ( rh->s.enable_aes )
    {
        if ( offset >= aes_offset && offset < aes_end )
        {
            if ( end > aes_end )
            {
                size = aes_end - offset;
            }
            AESi_LoadKey( AES_KEY_SLOT_A );
            EnableAes( offset );
        }
        else
        {
            if ( offset < aes_offset && offset + size > aes_offset )
            {
                size = aes_offset - offset;
            }
            DisableAes();
        }
    }
    else
    {
        DisableAes();
    }
    return size;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadBuffer

  Description:  load data in file and pass to ARM9 via WRAM-B

  Arguments:    fd              file discriptor to read
                offset          offset to start to read in bytes
                size            total length to read in bytes

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadBuffer( int fd, u32 offset, u32 size )
{
    static int count = 0;

    if ( fd < 0 )
    {
        return FALSE;
    }
    // seek
    if ( FATFSi_rtfs_po_lseek( fd, (long)offset, PSEEK_SET ) != (long)offset )
    {
            OS_TPrintf("Failed to seek file. (offset=0x%X)\n", offset);
        return FALSE;
    }
    while ( size > 0 )
    {
        u8* dest = aesFlag ? aesBuffer :
                    (u8*)HW_FIRM_LOAD_BUFFER_BASE + count * HW_FIRM_LOAD_BUFFER_UNIT_SIZE;
        u32 unit = size < HW_FIRM_LOAD_BUFFER_UNIT_SIZE ? size : HW_FIRM_LOAD_BUFFER_UNIT_SIZE;
        while ( MI_GetWramBankMaster_B( count ) != MI_WRAM_ARM7 ) // wait to be ready
        {
        }
#ifdef WORKAROUND_NAND_2KB_BUG
        {
            u32 done;
            for ( done = 0; done < unit; done += 2048 )
            {
                u8* d = dest + done;
                u32 u = unit - done < 2048 ? unit - done : 2048;
                if ( FATFSi_rtfs_po_read( fd, (u8*)d, (int)u ) != (int)u )
                {
                    OS_TPrintf("Failed to read file. (dest=%p, size=0x%X)\n", d, u);
                    return FALSE;
                }
            }
        }
#else
        if ( FATFSi_rtfs_po_read( fd, (u8*)dest, (int)unit ) != (int)unit )
        {
            OS_TPrintf("Failed to read file. (dest=%p, size=0x%X)\n", dest, unit);
            return FALSE;
        }
#endif
        if ( aesFlag )
        {
            CopyWithAes( dest, (u8*)HW_FIRM_LOAD_BUFFER_BASE + count * HW_FIRM_LOAD_BUFFER_UNIT_SIZE, unit );
        }
        PXI_NotifyID( FIRM_PXI_ID_LOAD_PIRIOD );
        count = ( count + 1 ) % HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
        size -= unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadModule

  Description:  load data in file and pass to ARM9 via WRAM-B in view of AES
                settings in the ROM header in HW_TWL_ROM_HEADER_BUF

  Arguments:    fd              file discriptor to read
                offset          offset to start to read in bytes
                length          total length to read in bytes

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadModule( int fd, u32 offset, u32 size )
{
    size = RoundUpModuleSize( size );
    while ( size > 0 )
    {
        u32 unit = GetTransferSize( offset, size );
        if ( !FS_LoadBuffer( fd, offset, unit ) )
        {
            return FALSE;
        }
        offset += unit;
        size -= unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadHeader

  Description:  load ROM header in the head of file and pass to ARM9 via WRAM-B

  Arguments:    fd              file discriptor to read

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadHeader( int fd )
{
    if (fd < 0)
    {
        return FALSE;
    }
    DisableAes();
    if ( !FS_LoadBuffer(fd, 0, FS_HEADER_AUTH_SIZE) ||
         !FS_LoadBuffer(fd, FS_HEADER_AUTH_SIZE, HW_TWL_ROM_HEADER_BUF_SIZE - FS_HEADER_AUTH_SIZE) )
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadStatic

  Description:  load static regions in file and pass to ARM9 via WRAM-B
                specified by ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    fd              file discriptor to read

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadStatic( int fd )
{
    if ( rh->s.main_size > 0 )
    {
        if ( !FS_LoadModule( fd, rh->s.main_rom_offset, rh->s.main_size ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.sub_size > 0 )
    {
        if ( !FS_LoadModule( fd, rh->s.sub_rom_offset, rh->s.sub_size ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.main_ltd_size > 0 )
    {
        if ( !FS_LoadModule( fd, rh->s.main_ltd_rom_offset, rh->s.main_ltd_size ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.sub_ltd_size > 0 )
    {
        if ( !FS_LoadModule( fd, rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size ) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

