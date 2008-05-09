#ifndef	MISC_H_
#define	MISC_H_
#include	<time.h>

typedef enum
{
    FALSE = 0,
    TRUE = 1
}
BOOL;

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;
typedef signed long long s64;

#endif //MISC_H_



#if 0
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
    char    platform_code;              // Platform code	bit0: not support NTR,  bit1: support TWL ( NTR_only=0x00, NTR/TWL=0x03, TWL_only=0x02 )
    u8      rom_type;                  // Rom type
    u8      rom_size;                  // Rom size (2のrom_size乗 Mbit: ex. 128Mbitのときrom_size = 7)

    u8      reserved_A[7];             // System Reserved A ( Set ALL 0 )

    u8		enable_signature:1;        // enable ROM Header signature
	u8		enable_aes:1;              // enable AES encryption
	u8      developer_encrypt:1;       // 開発用セキュリティがかかっている場合に"1"。製品版では"0"
    u8:     5;

    u8:     6;
    u8      for_korea:1;               // For Korea
    u8      for_china:1;               // For China

    u8      rom_version;               // Rom version

    u8      comp_arm9_boot_area:1;     // Compress arm9 boot area
    u8      comp_arm7_boot_area:1;     // Compress arm7 boot area
	u8		inspect_card:1;       		// Show inspect card
	u8      disable_clear_memory_pad:1; // for Debugger
	u8		enable_twl_rom_cache_read:1; // Enable TWL ROM cacheRead command
	u8:     1;
    u8      warning_no_spec_rom_speed:1;// Warning not to specify rom speed
	u8		disable_detect_pull_out:1;  // 

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
    u32     main_module_param_offset;  // Offset for table of ARM9 module parameters
    u32     sub_module_param_offset;   // Offset for table of ARM7 module parameters

    // 0x090 - 0x0C0 System Reserved
	u16		twl_card_normal_area_rom_offset;		// undeveloped
	u16		twl_card_keytable_area_rom_offset;		// undeveloped
	u16		nand_card_dl_area_rom_offset;			// undeveloped
	u16		nand_card_bk_area_rom_offset;			// undeveloped
	u8		nand_card_flag;							// undeveloped
	u8		reserved_B[39];
	
    // 0x0C0 for NINTENDO logo data
    u8      nintendo_logo[ NINTENDO_LOGO_DATA_LENGTH ];  // NINTENDO logo data
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
	u32		main_wram_config_data[8];		// developing...

	// 0x1A0 - 0x1B0 TWL-WRAM A/B/C ARM7 configuration data
	u32		sub_wram_config_data[4];		// developing...

	// 0x1B0 - reserved.
	u8		reserved_ltd_A[ 8 ];
	
	// 0x1B8 - ARM7-SCFG
	u32     arm7_scfg_ext;                  // SCFG-EXT
	u8      arm7_scfg_clk;                  // SCFG-CLK

	// padding(2byte)
	u8		reserved_ltd_A2[ 2 ];

	// 0x1BF - TWL expansion flags
	u8		codec_mode:1;					// 0:NTR mode, 1:TWL mode		// undeveloped
	u8		otherParentalControls:1;		// 1: Enable OtherParentalControls
	u8		saveBannerFile:1;				// 1: Enable SaveBannerFile
	u8		wifiConnection:1;				// 1: Use WiFiConnection
	u8		rsv:4;
	
    // 0x1C0 for EX Static modules
    //
    //  ARM9
    u32     main_ltd_rom_offset;           // ROM offset			// undeveloped
    u8		reserved_ltd_B[ 4 ];
    void   *main_ltd_ram_address;          // RAM address		// undeveloped	//
    u32     main_ltd_size;                 // Module size		// undeveloped	//

    //  ARM7
    u32     sub_ltd_rom_offset;            // ROM offset			// undeveloped
    u8		reserved_ltd_C[ 4 ];
    void   *sub_ltd_ram_address;           // RAM address		// undeveloped	//
    u32     sub_ltd_size;                  // Module size		// undeveloped	//
	/* 注意： os_reset.c / crt0.HYB.c / crt0.LTD.c 内で 0x01c0 ～ 0x01e0 のアドレスを
	          ハードコーディングしています。 これら８つのメンバのオフセットを変更しないで下さい。 */

	// 0x01E0 - 0x01E8 for NITRO digest area offset & size
	u32		nitro_digest_area_rom_offset;
	u32		nitro_digest_area_size;
	
	// 0x01E8 - 0x01F0 for TWL   digest area offset & size
	u32		twl_digest_area_rom_offset;
	u32		twl_digest_area_size;

	// 0x01F0 - 0x01F8 for FS digest table1 offset & size
	u32		digest1_table_offset;
	u32		digest1_table_size;

	// 0x01F8 - 0x0200 for FS digest table1 offset
	u32		digest2_table_offset;
    u32		digest2_table_size;
	
	// 0x0200 - 0x0208 for FS digest config parameters
	u32		digest1_block_size;
	u32		digest2_covered_digest1_num;
	
	// 0x0208 - 0x0210 for Banner for TWL
	u32		banner_twl_offset;
	u32		banner_twl_size;

	// 0x0210 - 0x0218 for TWL rom valid size
    u32     twl_rom_valid_size;            // ROM Original Size
	u8		reserved_ltd_D[ 4 ];
	
	// 0x0218 - 0x0220 for TWL ltd module param offset
    u32     main_ltd_module_param_offset;  // Offset for table of ARM9 ltd module parameters
    u32     sub_ltd_module_param_offset;   // Offset for table of ARM7 ltd module parameters
	
	// 0x0220 - 0x0230 for AES target offset & size
	u32		aes_target_rom_offset;
	u32		aes_target_size;
	u32		aes_target2_rom_offset;			// 予約
	u32		aes_target2_size;				// 予約

	// 0x0230 - 0x0238 for TitleID
	union {
		u64 titleID;
		struct {
			u8		titleID_Lo[ 4 ];
			u32		titleID_Hi;
		};
	};
	/* 注意： os_reset.c / crt0.HYB.c / crt0.LTD.c 内で 0x0234 をハードコーディングしています。
	          titleID_Hi のオフセットを変更しないで下さい。 */
	
	// 0x0238 - 0x0240 for Public & Private Save Data Size
	u32		public_save_data_size;
	u32		private_save_data_size;
	
	// 0x0240 - 0x02f0 reserved.
	u8		reserved_ltd_F[ 0x2f0 - 0x240 ];
	
	// 0x02f0 - 0x0300 Parental Controls Rating Info
	u8		ParentalControlsRatingInfo[ 0x10 ];
	
	// 0x0300 - 0x0378 Rom Segment Digest
	u8		main_static_digest[ DIGEST_SIZE_SHA1 ];
	u8		sub_static_digest[ DIGEST_SIZE_SHA1 ];
	u8		digest2_table_digest[ DIGEST_SIZE_SHA1 ];
	u8		banner_twl_digest[ DIGEST_SIZE_SHA1 ];
	u8		main_ltd_static_digest[ DIGEST_SIZE_SHA1 ];
	u8		sub_ltd_static_digest[ DIGEST_SIZE_SHA1 ];

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
	u8		signature[ RSA_KEY_LENGTH ];		// PKCS#1_v1.5の形式でフォーマットされたSignatureDataが、RSA暗号化された状態

    //
    // 0x01000 - 0x04000 System Reserved
    //
    u8      reserved_E[12 * 1024];     // Reserved E

}
ROM_Header;                            // 16KB

#endif
