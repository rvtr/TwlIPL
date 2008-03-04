/*---------------------------------------------------------------------------*
  Project:  TwlIPL - HOTSW - include
  File:     command.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: $
  $Rev: $
  $Author: $
 *---------------------------------------------------------------------------*/
#ifndef __HOTSW_COMMAND_H__
#define __HOTSW_COMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

// NORMAL command

#define HSWOP_N_OP_MASK               0xff00000000000000ULL
#define HSWOP_N_OP_SIZE               8

#define HSWOP_N_OP_RD_ID              0x9000000000000000ULL
#define HSWOP_N_OP_RD_PAGE            0x0000000000000000ULL
#define HSWOP_N_OP_WR_PAGE            0x8000000000000000ULL
#define HSWOP_N_OP_LD_TABLE           0x9f00000000000000ULL
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
#define HSWOP_G_OP_RD_CACHE_START     0x5800000000000000ULL
#define HSWOP_G_OP_RD_CACHE           0x6000000000000000ULL
#define HSWOP_G_OP_RD_CACHE_LAST      0x6800000000000000ULL
#define HSWOP_G_OP_WR_PAGE            0x8000000000000000ULL

#define HSWOP_G_RD_ID_PAD             0x00ffffffffffffffULL
#define HSWOP_G_RD_UID_PAD            0x00ffffffffffffffULL
#define HSWOP_G_RD_PAGE_PAD           0x00f0000000ffffffULL

#define HSWOP_G_RD_PAGE_ADDR_SHIFT    33
#define HSWOP_G_RD_PAGE_ADDR_SIZE     23
#define HSWOP_G_RD_PAGE_ADDR_MASK     0x000ffffe00000000ULL


// ROM ID

#define HOTSW_ROMID_1TROM_MASK        0x80000000UL
#define HOTSW_ROMID_TWLROM_MASK       0x40000000UL
#define HOTSW_ROMID_BADBLK_MASK       0x20000000UL
#define HOTSW_ROMID_SIZE_MASK         0x0000ff00UL


#ifdef __cplusplus
} /* extern "C" */

#endif

/* __HOTSW_COMMAND_H__ */
#endif
