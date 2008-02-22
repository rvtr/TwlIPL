/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fs
  File:     fs_loader.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef FIRM_FS_FS_LOADER_H_
#define FIRM_FS_FS_LOADER_H_

#include <twl/types.h>
#include <twl/aes/common/types.h>
#include <twl/os/common/systemCall.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FS_LoadBuffer

  Description:  receive data from ARM7 via WRAM-B and store in destination address,
                calculate SHA1 in parallel if ctx is specified

  Arguments:    dest            destination address to read
                size            total length to read in bytes
                ctx             pointer to SHA1 context or NULL

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadBuffer( u8* dest, u32 size, SVCSHA1Context *ctx );

/*---------------------------------------------------------------------------*
  Name:         FS_LoadModule

  Description:  receive data from ARM7 via WRAM-B and store in destination address
                in view of AES settings in the ROM header at HW_TWL_ROM_HEADER_BUF,
                then verify the digest

  Arguments:    dest            destination address to read
                offset          file offset to start to read in bytes
                size            total length to read in bytes
                digest          digest to verify

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadModule( u8* dest, u32 offset, u32 size, const u8 digest[SVC_SHA1_BLOCK_SIZE] );

/*---------------------------------------------------------------------------*
  Name:         FS_LoadHeader

  Description:  receive ROM header, store to HW_TWL_ROM_HEADER_BUF,
                and verify signature

  Arguments:    pool            heap context to call SVC_DecryptSign
                rsa_key1        public key to verify the signature
                rsa_key2        public key to verify the signature
                                for system applications

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadHeader( SVCSignHeapContext* pool, const void* rsa_key1, const void* rsa_key2 );

/*---------------------------------------------------------------------------*
  Name:         FS_LoadStatic

  Description:  receive static regions from ARM6 via WRAM-B and store them
                specified by ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    digestKey       pointer to key for HMAC-SHA1
                                if NULL, use default key

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadStatic( const u8* digestKey );


#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FS_FS_LOADER_H_ */
#endif
