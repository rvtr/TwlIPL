/*---------------------------------------------------------------------------*
  Project:  TwlIPL - HOTSW - include
  File:     romSpec.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_ROMSPEC_H__
#define __HOTSW_ROMSPEC_H__

#ifdef __cplusplus
extern "C" {
#endif


// ROM memory map

#define HOTSW_PAGE_SIZE               0x200  // 512B
#define HOTSW_BOOT_SEG_SIZE           0x1000 // 4KB
#define HOTSW_LOAD_TABLE_SIZE         0x2000 // 8KB
#define HOTSW_KEYTABLE_AREA_SIZE	  0x3000 // 12KB
#define HOTSW_KEYTABLE2_AREA_SIZE	  0x3000 // 12KB
#define HOTSW_SECURE_AREA_SIZE        0x4000 // 16KB
#define HOTSW_SECURE2_AREA_SIZE       0x4000 // 16KB
#define HOTSW_ROMEMU_INFO_SIZE        0x20

#define HOTSW_BOOTSEGMENT_AREA_OFS	  0x0000 // 0KB
#define HOTSW_KEYTABLE_AREA_OFS		  0x1000 // 4KB
#define HOTSW_SECURE_AREA_OFS         0x4000 // 16KB
#define HOTSW_GAME_AREA_OFS           0x8000 // 32KB
#define	HOTSW_SECURE2_AREA_OFS        0x3000 // 12KB
#define	HOTSW_GAME2_AREA_OFS          0x7000 // 28KB
#define	HOTSW_ROMEMU_INFO_OFS         (HOTSW_SECURE_AREA_OFS - HOTSW_PAGE_SIZE + 0x160)


// ROM ID

#define HOTSW_ROMID_1TROM_MASK        0x80000000UL
#define HOTSW_ROMID_TWLROM_MASK       0x40000000UL
#define HOTSW_ROMID_RFSSUP_MASK       0x20000000UL
#define HOTSW_ROMID_SIZE_MASK         0x0000ff00UL

// ROM STATUS

#define HOTSW_ROMST_RFS_WARN_L1_MASK  0x00000004UL
#define HOTSW_ROMST_RFS_WARN_L2_MASK  0x00000008UL
#define HOTSW_ROMST_RFS_READY_MASK    0x00000020UL


// NORMAL command

#define HSWOP_N_OP_MASK               0xff00000000000000ULL
#define HSWOP_N_OP_SIZE               8

#define HSWOP_N_OP_RD_ID              0x9000000000000000ULL
#define HSWOP_N_OP_RD_PAGE            0x0000000000000000ULL
#define HSWOP_N_OP_WR_PAGE            0x8000000000000000ULL
#define HSWOP_N_OP_LD_TABLE           0x9f00000000000000ULL
#define HSWOP_N_OP_RD_CACHE_START     0x5800000000000000ULL
#define HSWOP_N_OP_RD_CACHE           0x6000000000000000ULL
#define HSWOP_N_OP_RD_CACHE_LAST      0x6800000000000000ULL
#define HSWOP_N_OP_RD_STAT            0xd600000000000000ULL
#define HSWOP_N_OP_RFS_BLK            0xb500000000000000ULL
#define HSWOP_N_OP_CHG_MODE           0x3c00000000000000ULL
#define HSWOP_N_OP_CHG2_MODE          0x3d00000000000000ULL

#define HSWOP_N_RD_ID_PAD             0x00ffffffffffffffULL
#define HSWOP_N_RD_PAGE_PAD           0x00fffffeff00ffffULL
#define HSWOP_N_CHG_MODE_PAD          0x00000000f00000ffULL
#define HSWOP_N_CHG2_MODE_PAD         HSWOP_N_CHG_MODE_PAD

#define HSWOP_N_RD_PAGE_ADDR_SHIFT    33
#define HSWOP_N_RD_PAGE_ADDR_SIZE     23
#define HSWOP_N_RD_PAGE_ADDR_MASK     0x00fffffe00000000ULL

#define HSWOP_N_VAE_SHIFT             32
#define HSWOP_N_VAE_SIZE              24
#define HSWOP_N_VAE_MASK              0x00ffffff00000000ULL

#define HSWOP_N_VBI_SHIFT             8
#define HSWOP_N_VBI_SIZE              20
#define HSWOP_N_VBI_MASK              0x000000000fffff00ULL


// SECURE command

#define HSWOP_S_OP_MASK               0xf000000000000000ULL
#define HSWOP_S_OP_SIZE               4

#define HSWOP_S_OP_RD_ID              0x1000000000000000ULL
#define HSWOP_S_OP_RD_SEG             0x2000000000000000ULL
#define HSWOP_S_OP_PNG_ON             0x4000000000000000ULL
#define HSWOP_S_OP_PNG_OFF            0x6000000000000000ULL
#define HSWOP_S_OP_CHG_MODE           0xa000000000000000ULL

#define HSWOP_S_VA_SHIFT              HSWOP_S_VB_SIZE
#define HSWOP_S_VA_SIZE               24
#define HSWOP_S_VA_MASK               0x00000ffffff00000ULL

#define HSWOP_S_VB_SHIFT              0
#define HSWOP_S_VB_SIZE               20
#define HSWOP_S_VB_MASK               0x00000000000fffffULL

#define HSWOP_S_VC_SHIFT              (HSWOP_S_VA_SIZE + HSWOP_S_VB_SIZE)
#define HSWOP_S_VC_SIZE               16
#define HSWOP_S_VC_MASK               0x0ffff00000000000ULL

#define HSWOP_S_VD_SHIFT              HSWOP_S_VA_SHIFT
#define HSWOP_S_VD_SIZE               HSWOP_S_VA_SIZE
#define HSWOP_S_VD_MASK               HSWOP_S_VA_MASK


// GAME command

#define HSWOP_G_OP_MASK               0xff00000000000000ULL
#define HSWOP_G_OP_SIZE               8

#define HSWOP_G_OP_RD_ID              0xb800000000000000ULL
#define HSWOP_G_OP_RD_UID             0xb900000000000000ULL
#define HSWOP_G_OP_RD_PAGE            0xb700000000000000ULL
#define HSWOP_G_OP_WR_PAGE            0x8000000000000000ULL
#define HSWOP_G_OP_RD_CACHE_START     HSWOP_N_OP_RD_CACHE_START
#define HSWOP_G_OP_RD_CACHE           HSWOP_N_OP_RD_CACHE
#define HSWOP_G_OP_RD_CACHE_LAST      HSWOP_N_OP_RD_CACHE_LAST
#define HSWOP_G_OP_RD_STAT            HSWOP_N_OP_RD_STAT
#define HSWOP_G_OP_RFS_BLK            HSWOP_N_OP_RFS_BLK

#define HSWOP_G_RD_ID_PAD             0x00ffffffffffffffULL
#define HSWOP_G_RD_UID_PAD            0x00ffffffffffffffULL
#define HSWOP_G_RD_PAGE_PAD           0x00f0000000ffffffULL

#define HSWOP_G_RD_PAGE_ADDR_SHIFT    33
#define HSWOP_G_RD_PAGE_ADDR_SIZE     23
#define HSWOP_G_RD_PAGE_ADDR_MASK     0x000ffffe00000000ULL


#ifdef __cplusplus
} /* extern "C" */

#endif

/* __HOTSW_ROMSPEC_H__ */
#endif
