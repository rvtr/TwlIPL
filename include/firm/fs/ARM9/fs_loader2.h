/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fs
  File:     fs_loader2.h

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

#ifndef FIRM_FS_FS_LOADER2_H_
#define FIRM_FS_FS_LOADER2_H_

#include <twl/types.h>
#include <twl/aes/common/types.h>
#include <twl/os/common/systemCall.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FS2_LoadModule

  Description:  receive data from ARM7 via WRAM-B and store in destination address
                in view of AES settings in the ROM header at HW_TWL_ROM_HEADER_BUF,
                then verify the digest

  Arguments:    pFile           pointer to FSFile streucture
                dest            destination address to read
                offset          file offset to start to read in bytes
                size            total length to read in bytes
                digest          digest to verify

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS2_LoadModule( FSFile *pFile, u8* dest, u32 offset, u32 size, const u8 digest[SVC_SHA1_DIGEST_SIZE] );

/*---------------------------------------------------------------------------*
  Name:         FS2_OpenSrl

  Description:  open srl file named at HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    pFile   pointer to FSFile streucture

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS2_OpenSrl( FSFile *pFile );

/*---------------------------------------------------------------------------*
  Name:         FS2_LoadHeader

  Description:  load ROM header to HW_TWL_ROM_HEADER_BUF using normal FS,
                and verify signature

  Arguments:    pFile           pointer to FSFile streucture
                pool            heap context to call SVC_DecryptSign
                rsa_key1        public key to verify the signature
                rsa_key2        public key to verify the signature
                                for system applications

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS2_LoadHeader( FSFile *pFile, SVCSignHeapContext* pool, const void* rsa_key1, const void* rsa_key2 );

/*---------------------------------------------------------------------------*
  Name:         FS2_LoadStatic

  Description:  receive static regions from ARM6 via WRAM-B and store them
                specified by ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    pFile           pointer to FSFile streucture
                digestKey       pointer to key for HMAC-SHA1
                                if NULL, use default key

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS2_LoadStatic( FSFile *pFile, const u8* digestKey );


#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FS_FS_LOADER2_H_ */
#endif
