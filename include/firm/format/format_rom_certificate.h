/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom.TWL
  File:     format_rom_certificate.h

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

#ifndef FORMAT_ROM_CERTIFICATE_H_
#define FORMAT_ROM_CERTIFICATE_H_

//#include "misc.h"

/*===========================================================================*
 *  ROM CERTIFICATE FORMAT
 *===========================================================================*/

#define TWL_ROM_CERT_MAGIC_NUMBER       0x54524543      // "CERT"の文字列

#define RSA_KEY_LENGTH                  ( 1024 / 8 )
#define RSA_PUBKEY_1024_DER_MAX         0xc0            // 少しマージンをとってこの値
#define RSA_PRVKEY_1024_DER_MAX         0x280
#define ROM_CERT_SIGN_OFFSET            256

#define ROM_HEADER_SIGN_TARGET_SIZE     0xe00

// ROM証明書ヘッダ
typedef struct RomCertificateHeader {
    u32     magicNumber;
    u32     version;
    u32     serialNo;
    u32     gameCode;
}RomCertificateHeader;

// ROM証明書
typedef struct RomCertificate {
    RomCertificateHeader    header;
    u8                      reserved[ ( ROM_CERT_SIGN_OFFSET - sizeof(RomCertificateHeader) - RSA_PUBKEY_1024_DER_MAX ) ];
    u8                      pubKey[ RSA_PUBKEY_1024_DER_MAX ];
    u8                      sign[ RSA_KEY_LENGTH ];
}RomCertificate;

#endif //FORMAT_ROM_CERTIFICATE_H_
