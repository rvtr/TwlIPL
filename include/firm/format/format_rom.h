/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom.TWL
  File:     format_rom.h

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

#ifndef FORMAT_ROM_H_
#define FORMAT_ROM_H_

//#include "misc.h"
//#include "format_rom_certificate.h"
//#include "acsign/include/acsign.h"
//#include "aes/include/aes2.h"
#include <twl/aes.h>
#include <firm/format/format_rom_certificate.h>


//#define ENABLE_OVERLAY_DIGEST_ANNEX           // この定義が有効なら、OverlayのダイジェストをcompstaticのMB用のものとは別途持つようになる。

#define ROM_KEYTABLE2_SIZE              0x3000      // TWL-ROMのTWL専用領域先頭にあるキーテーブル２サイズ。TWL専用領域のセキュア領域は、この後ろから開始。
#define ROM_TWL_LTD_ALIGN               0x80000     // TWL-ROMのTWL専用領域の設定単位　※仕様書上は512KB
#define ROM_TWL_LTD_ALIGN_SHIFT         19          // 上記の単位をビットシフトに換算
#define ROM_TWL_ALL_NORMAL_AREA_FLAG    0x8000

#define DIGEST_SIZE_SHA1        20

#define DEFAULT_ALIGN           512
#define DEFAULT_HOSTROOT        "."
#define DEFAULT_ROOT            "/"
#define DEFAULT_ROMHEADER_TEMPLATE  "rom_header.template.sbin"
#define DEFAULT_ROMHEADER_SURFFIX   "_header.sbin"
#define DEFAULT_FILENAME_SURFFIX    "_files.sbin"

#define DEFAULT_REJECT          { "CVS", "vssver.scc", ".?*", 0/*Terminater*/ }
#define DEFAULT_REJECT_CONSERVATIVE { "CVS", "vssver.scc", 0/*Terminater*/ }

#define FORMAT_VERSION          "1.1"

#define ENTRYNAME_MAX           127

#define FILE_ID_START           0x0000
#define FILE_ID_END         0xefff
#define DIR_ID_START            0xf000
#define DIR_ID_END          0xffff

#define DEFAULT_LISTFILE        "default.nlf"

#define DEFAULT_ROMFILE_SUFFIX      ".srl"
#define DEFAULT_LISTFILE_SUFFIX     ".nlf"
#define DEFAULT_LISTFILE_SUFFIX_TWL ".tlf"

#define ROM_SIZE_MIN            0x20000

#define CRC16_INIT_VALUE        0xffff
#define CALC_CRC16_SIZE         0x15e

#define SECURE_AREA_START       0x00004000
#define SECURE_AREA_END         0x00008000
#define SECURE_AREA_SIZE        (SECURE_AREA_END - SECURE_AREA_START)

#define CARD_LATENCY_MASK               0x083f1fff
#define CARD_MROM_GAME_LATENCY          0x00010017
#define CARD_MROM_SECURE_HW_LATENCY     0x001808f8
#define CARD_MROM_SECURE_SW_LATENCY     0x051e
#define CARD_1TROM_GAME_LATENCY         0x00010657
#define CARD_1TROM_SECURE_HW_LATENCY    0x081808f8
#define CARD_1TROM_SECURE_SW_LATENCY    0x0d7e

// for out_romheader.c size check
#define BOOTABLE_SIZE_ARM9      0x00280000      // 2.5M
#define BOOTABLE_SIZE_ARM7      0x00040000      // 256K

/*===========================================================================*
 *  ROM FORMAT
 *===========================================================================*/

//---------------------------------------------------------------------------
//  Section A   ROM HEADER
//---------------------------------------------------------------------------

#define ROMHEADER_CORP_ID       "NINTENDO    "
#define TITLE_NAME_MAX          12
#define GAME_CODE_MAX           4
#define MAKER_CODE_MAX          2

typedef struct ROM_Header_Short
{
    //==========================================================
    //
    // NTR/TWL common
    //
    //==========================================================

    //
    // 0x000 System Reserved
    //
    char    title_name[TITLE_NAME_MAX]; // Soft title name
    char    game_code[GAME_CODE_MAX];   // Game code
    char    maker_code[MAKER_CODE_MAX]; // Maker code
    char    platform_code;              // Platform code    bit0: not support NTR,  bit1: support TWL ( NTR_only=0x00, NTR/TWL=0x03, TWL_only=0x02 )
    u8      rom_type;                  // Rom type
    u8      rom_size;                  // Rom size (2のrom_size乗 Mbit: ex. 128Mbitのときrom_size = 7)

    u8      reserved_A[7];             // System Reserved A ( Set ALL 0 )

    u8      enable_signature:1;        // enable ROM Header signature
    u8      enable_aes:1;              // enable AES encryption
    u8      developer_encrypt:1;       // 開発用セキュリティがかかっている場合に"1"。製品版では"0"
    u8:     5;

    u8:     6;
    u8      for_korea:1;               // For Korea
    u8      for_china:1;               // For China

    u8      rom_version;               // Rom version

    u8      comp_arm9_boot_area:1;     // Compress arm9 boot area
    u8      comp_arm7_boot_area:1;     // Compress arm7 boot area
    u8      inspect_card:1;             // Show inspect card
    u8      disable_clear_memory_pad:1; // for Debugger
    u8      enable_twl_rom_cache_read:1; // Enable TWL ROM cacheRead command
    u8:     1;
    u8      warning_no_spec_rom_speed:1;// Warning not to specify rom speed
    u8      disable_detect_pull_out:1;  //

    //
    // 0x020 for Static modules (Section:B)
    //
    //  ARM9
    u32     main_rom_offset;           // ROM offset
    void   *main_entry_address;        // Entry point
    void   *main_ram_address;          // RAM address
    u32     main_size;                 // Module size

    //  ARM7
    u32     sub_rom_offset;            // ROM offset
    void   *sub_entry_address;         // Entry point
    void   *sub_ram_address;           // RAM address
    u32     sub_size;                  // Module size

    //
    // 0x040 for File Name Table[FNT] (Section:C)
    //
    struct ROM_FNT *fnt_offset;        // ROM offset
    u32     fnt_size;                  // Table size

    //
    // 0x048 for File Allocation Table[FAT] (Section:E)
    //
    struct ROM_FAT *fat_offset;        // ROM offset
    u32     fat_size;                  // Table size

    //
    // 0x050 for Overlay Tables[OVT] (Section:D)
    //
    //  ARM9
    struct ROM_OVT *main_ovt_offset;   // ROM offset
    u32     main_ovt_size;             // Table size

    //  ARM7
    struct ROM_OVT *sub_ovt_offset;    // ROM offset
    u32     sub_ovt_size;              // Table size

    // 0x060 for ROM control parameter (Section:F)
    u32     game_cmd_param;            // Game command parameter
    u32     secure_cmd_param;          // Secure command parameter

    u32     banner_offset;             // Banner ROM offset

    u16     secure_area_crc16;         // Secure area CRC-16
    u16     secure_cmd_latency;        // Secure command latency ((param+2)*256 system cycles)

    //                  since NITRO-SDK 2.0PR4
    void   *main_autoload_done;        // ARM9 autoload done callback address (debug purpose)
    void   *sub_autoload_done;         // ARM7 autoload done callback address (debug purpose)

    u8      ctrl_reserved_B[8];        // Ctrl Reserved B (Set 0)

    //                  since NITRO-SDK 2.0PR6
    u32     rom_valid_size;            // ROM Original Size
    u32     rom_header_size;           // ROM Header size
    u32     main_module_param;         // Offset for table of ARM9 module parameters
    u32     sub_module_param;          // Offset for table of ARM7 module parameters

    // 0x090 - 0x0C0 System Reserved
    u16     twl_card_normal_area_rom_offset;        // undeveloped
    u16     twl_card_keytable_area_rom_offset;      // undeveloped
    u16     nand_card_dl_area_rom_offset;           // undeveloped
    u16     nand_card_bk_area_rom_offset;           // undeveloped
    u8      nand_card_flag;                         // undeveloped
    u8      reserved_B[39];

    // 0x0C0 for NINTENDO logo data
    u8      nintendo_logo[0x9c];       // NINTENDO logo data
    u16     nintendo_logo_crc16;       //            CRC-16

    // 0x15E ROM header CRC-16
    u16     header_crc16;              // ROM header CRC-16

    // 0x160 - 0x180 Debugger Reserved
    u8      reserved_C[32];            // Debugger Reserved (Set ALL 0)

    //==========================================================
    //
    // TWL only
    //
    //==========================================================

    // 0x180 - 0x190 TWL-WRAM A/B/C ARM9 configuration data
    u32     main_wram_config_data[8];       // developing...

    // 0x1A0 - 0x1B0 TWL-WRAM A/B/C ARM7 configuration data
    u32     sub_wram_config_data[4];        // developing...

    // 0x1B0 - reserved.
    u8      reserved_ltd_A[ 15 ];

    // 0x1BF - TWL expansion flags
    u8      codec_mode:1;               // 0:NTR mode, 1:TWL mode       // undeveloped
    u8:     3;
    u8      enable_boot_nand:1;             // undeveloped
    u8      enable_boot_sd_card:1;              // undeveloped
    u8      enable_boot_game_card:1;            // undeveloped
    u8      enable_boot_download_play:1;        // undeveloped

    // 0x1C0 for EX Static modules
    //
    //  ARM9
    u32     main_ltd_rom_offset;           // ROM offset            // undeveloped
    u8      reserved_ltd_B[ 4 ];
    void   *main_ltd_ram_address;          // RAM address       // undeveloped
    u32     main_ltd_size;                 // Module size       // undeveloped

    //  ARM7
    u32     sub_ltd_rom_offset;            // ROM offset            // undeveloped
    u8      reserved_ltd_C[ 4 ];
    void   *sub_ltd_ram_address;           // RAM address       // undeveloped
    u32     sub_ltd_size;                  // Module size       // undeveloped

    // 0x01E0 - 0x01E8 for NITRO digest area offset & size
    u32     nitro_digest_area_rom_offset;
    u32     nitro_digest_area_size;

    // 0x01E8 - 0x01F0 for TWL   digest area offset & size
    u32     twl_digest_area_rom_offset;
    u32     twl_digest_area_size;

    // 0x01F0 - 0x01F8 for FS digest table1 offset & size
    u32     digest1_table_offset;
    u32     digest1_table_size;

    // 0x01F8 - 0x0200 for FS digest table1 offset
    u32     digest2_table_offset;
    u32     digest2_table_size;

    // 0x0200 - 0x0208 for FS digest config parameters
    u32     digest1_block_size;
    u32     digest2_covered_digest1_num;

    // 0x0208 - 0x0210 for Banner for TWL
    u32     banner_twl_offset;
    u32     banner_twl_size;

    // 0x0210 - 0x0220 for AES key/seed
    u8      aes_key[ 16 ];

    // 0x220 - 0x298 Rom Segment Digest
    u8      main_static_digest[ DIGEST_SIZE_SHA1 ];
    u8      sub_static_digest[ DIGEST_SIZE_SHA1 ];
    u8      digest2_table_digest[ DIGEST_SIZE_SHA1 ];
    u8      banner_twl_digest[ DIGEST_SIZE_SHA1 ];
    u8      main_ltd_static_digest[ DIGEST_SIZE_SHA1 ];
    u8      sub_ltd_static_digest[ DIGEST_SIZE_SHA1 ];


#ifdef ENABLE_OVERLAY_DIGEST_ANNEX
    // ※今は無効。
    // overlayのダイジェストは、compstaticでstaticモジュール内に付加される、
    // 既存のDSダウンロードプレイ用のオーバーレイダイジェストを使用するので、いらない。
    // また、overlay_exは、overlayと共通化する予定なので、OVTもダイジェストテーブルも作らない。
    u32     main_overlay_digest_table_offset;
    u32     sub_overlay_digest_table_offset;
    u8      main_overlay_digesttable_digest[ DIGEST_SIZE_SHA1 ];
    u8      sub_overlay_digesttable_digest[ DIGEST_SIZE_SHA1 ];

#endif // ENABLE_OVERLAY_DIGEST_ANNEX
}
ROM_Header_Short;

typedef struct
{
    struct ROM_Header_Short s;

    ////
    ////  EXTRA SYSTEM REGION
    ////

    //
    // 0x02f0 - 0x0e00 System Reserved
    //
    u8      reserved_D[4 * 1024 - sizeof(ROM_Header_Short) - sizeof(RomCertificate) - RSA_KEY_LENGTH ];       // Reserved D

    //
    // 0x0e00 - 0x0f80 ROM Certificate
    //
    RomCertificate certificate;

    //
    // 0x0f80 - 0x1000 ROM Header Signature
    //
    u8      signature[ RSA_KEY_LENGTH ];        // PKCS#1_v1.5の形式でフォーマットされたSignatureDataが、RSA暗号化された状態

    //
    // 0x01000 - 0x04000 System Reserved
    //
    u8      reserved_E[12 * 1024];     // Reserved E

}
ROM_Header;                            // 16KB



// 署名(ROM_Header.signature)内のデータ
typedef struct SignatureData
{
    u8  aes_key_seed[ AES_BLOCK_SIZE ];
    u8  digest[ DIGEST_SIZE_SHA1 ];
}SignatureData;



//---------------------------------------------------------------------------
//  Section C   FNT: FILE NAME TABLE
//                      - DIRECTORY TABLE
//                      - ENTRY TABLE
//---------------------------------------------------------------------------

typedef struct ROM_FNTDir
{
    u32     entry_start;               // offset for entry
    u16     entry_file_id;             // file id for 1st file
    u16     parent_id;                 // parent directory id

}
ROM_FNTDir;


/*  P-SEUDO CODE for YOUR INFO.
typedef struct ROM_FNTStrFile
{
    u8  entry_type       :1;    // 0 if file entry
    u8  entry_name_length:7;    // length of filename (0-127)
    char    entry_name[length]; // file name (surpress '\0')

} ROM_FNTStrFile;


typedef struct ROM_FNTStrDir
{
    u8  entry_type       :1;    // 1 if directory entry
    u8  entry_name_length:7;    // length of directory name (0-127)
    char    entry_name[length]; // directory name (surpress '\0')
    u8  dir_id_L;       // low  byte of directory id
    u8  dir_id_H;       // high byte of directory id
} ROM_FNTStrDir;
*/

//---------------------------------------------------------------------------
//  Section D   OVT: OVERLAY TABLE
//---------------------------------------------------------------------------

typedef struct ROM_OVT
{
    void   *id;                        // Overlay ID
    void   *ram_address;               // ram address
    u32     ram_size;                  // ram size
    u32     bss_size;                  // bss size
    void   *sinit_init;                // static initializer start
    void   *sinit_init_end;            // static initializer end
    u32     file_id;                   // file id in FAT
    u32     compressed:24;             // オーバーレイ圧縮後のサイズ
    u32     flag:8;                    // オーバーレイ情報フラグ

}
ROM_OVT;


//---------------------------------------------------------------------------
//  Section E   FAT: FILE ALLOCATION TABLE
//---------------------------------------------------------------------------

typedef struct ROM_FilePtr
{
    u32     offset;                    // offset

}
ROM_FilePtr;


typedef struct ROM_FAT
{
    ROM_FilePtr top;                   // file region start
    ROM_FilePtr bottom;                //             end

}
ROM_FAT;


//---------------------------------------------------------------------------
//  Section F   LT: ROM LATENCY
//---------------------------------------------------------------------------

typedef enum
{
    ONETROM = 0,
    MROM
}
tRomSpeedType;

typedef struct ROM_LT
{
    u32     game_latency;
    u32     secure_hw_latency;
    u16     secure_sw_latency;

}
ROM_LT;


//---------------------------------------------------------------------------
//  Section X   ONT: OVERLAY NAME TABLE  (not include in ROM) generated by LD
//                      - OverlayDefs
//---------------------------------------------------------------------------

typedef struct ROM_ONTHeader
{
    void   *static_ram_address;        // static module ram_address
    void   *static_entry_address;      //               entry address
    u32     static_size;               //               size
    void   *static_autoload_done;      // autoload done callback address

}
ROM_ONTHeader;

typedef struct ROM_ONT
{
    ROM_ONTHeader header;
    char    file_list[1];

}
ROM_ONT;

#define SIZE_OF_SIGN     136
#define NITROCODE_LE     0x2106c0de
#define NITROCODE_BE     0xdec00621

//---------------------------------------------------------------------------
//  Section Y   MDP: Module params
//---------------------------------------------------------------------------
typedef struct tModuleParam
{
    void   *autoload_list;
    void   *autoload_list_end;
    void   *autoload_start;
    void   *static_bss_start;
    void   *static_bss_end;
    void   *compressed_static_end;     // tCompFooter の 1要素分先を指す
    u32     version;
    u32     nitro_magic_be;
    u32     nitro_magic_le;

}
tModuleParam;

typedef struct
{
    u32     bufferTop:24;
    u32     compressBottom:8;
    u32     originalBottom;
}
tCompFooter;

typedef struct      // compstatic/component.hより抜粋
{
    u32     magicNumber;               // 認識子 ==MAGICNUMBER_FOOTER
    int     staticParamsOffset;        // StaticParams     へのオフセット
    int     digestParamsOffset;        // SDK_DIGEST_TABLE へのオフセット
}
StaticFooter;

#endif //FORMAT_ROM_H_
