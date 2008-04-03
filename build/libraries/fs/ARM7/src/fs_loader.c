/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fs
  File:     fs_loader.c

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

#define MODULE_ALIGNMENT    0x20    // 16*2バイト単位で読み込む
//#define MODULE_ALIGNMENT  0x200   // 512バイト単位で読み込む
#define RoundUpModuleSize(value)    (((value) + MODULE_ALIGNMENT - 1) & -MODULE_ALIGNMENT)

static ROM_Header* const rh= (ROM_Header*)HW_TWL_ROM_HEADER_BUF;

static BOOL         aesFlag;
static AESCounter   aesCounter;
static u8           aesBuffer[HW_FIRM_LOAD_BUFFER_UNIT_SIZE] ATTRIBUTE_ALIGN(32);

#define DMA_SEND         2
#define DMA_RECV         3
static void CopyWithAes( const void* src, void* dest, u32 size )
{
    AES_Lock();
    AES_Reset();
    AES_Reset();
    AES_WaitKey();
    AES_LoadKey( AES_KEY_SLOT_A );
    AES_WaitKey();
    AES_DmaSend( DMA_SEND, src,  size, NULL, NULL );
    AES_DmaRecv( DMA_RECV, dest, size, NULL, NULL );
    AES_SetCounter( &aesCounter );
    AES_Run( AES_MODE_CTR, 0, size / AES_BLOCK_SIZE, NULL, NULL );
    AES_AddToCounter( &aesCounter, size / AES_BLOCK_SIZE );
    MI_WaitNDma( DMA_RECV );
    AES_Unlock();
}

static void EnableAes( u32 offset )
{
    aesFlag = TRUE;
    MI_CpuCopy8( rh->s.main_static_digest, &aesCounter, AES_BLOCK_SIZE );
    AES_AddToCounter( &aesCounter, (offset - rh->s.aes_target_rom_offset) / AES_BLOCK_SIZE );
}
static void EnableAes2( u32 offset )
{
    aesFlag = TRUE;
    MI_CpuCopy8( rh->s.sub_static_digest, &aesCounter, AES_BLOCK_SIZE );
    AES_AddToCounter( &aesCounter, (offset - rh->s.aes_target2_rom_offset) / AES_BLOCK_SIZE );
}
static void DisableAes( void )
{
    aesFlag = FALSE;
}

static u32 GetTransferSize( u32 offset, u32 size )
{
    if ( rh->s.enable_aes )
    {
        u32 end = offset + RoundUpModuleSize(size);
        u32 aes_offset = rh->s.aes_target_rom_offset;
        u32 aes_end = aes_offset + RoundUpModuleSize(rh->s.aes_target_size);
        u32 aes_offset2 = rh->s.aes_target2_rom_offset;
        u32 aes_end2 = aes_offset2 + RoundUpModuleSize(rh->s.aes_target2_size);

        if ( offset >= aes_offset && offset < aes_end )
        {
            if ( end > aes_end )
            {
                size = aes_end - offset;
            }
            EnableAes( offset );
        }
        else if ( offset >= aes_offset2 && offset < aes_end2 )
        {
            if ( end > aes_end2 )
            {
                size = aes_end2 - offset;
            }
            EnableAes2( offset );
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
        PXI_AcquireLoadBufferSemaphore(); // wait to be ready
        if ( MI_GetWramBankMaster_B( count ) != MI_WRAM_ARM7 )
        {
            OS_TPanic("PROGRAM ERROR!");
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
        PXI_ReleaseLoadBufferSemaphore();
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

