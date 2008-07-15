/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     myIoreg_SCFG.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/

// ARM9側で受け取ったARM7のSCFGを参照するために
// ARM9側なのにARM7依存のオフセット情報が必要...


// マクロ再定義を防ぐための苦肉の策

#ifndef __MY_IOREG_SCFG_H__
#define __MY_IOREG_SCFG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Definition of Register offsets, addresses and variables.
 */


/* ROM */

#define DISP_REG_ROM_OFFSET                                     0x4000
#define DISP_REG_ROM_ADDR                                       (HW_REG_BASE + DISP_REG_ROM_OFFSET)

/* A9ROM */

#define DISP_REG_A9ROM_OFFSET                                   0x4000
#define DISP_REG_A9ROM_ADDR                                     (HW_REG_BASE + DISP_REG_A9ROM_OFFSET)

/* A7ROM */

#define DISP_REG_A7ROM_OFFSET                                   0x4001
#define DISP_REG_A7ROM_ADDR                                     (HW_REG_BASE + DISP_REG_A7ROM_OFFSET)

/* ROMWE */

#define DISP_REG_ROMWE_OFFSET                                   0x4002
#define DISP_REG_ROMWE_ADDR                                     (HW_REG_BASE + DISP_REG_ROMWE_OFFSET)

/* CLK */

#define DISP_REG_CLK_OFFSET                                     0x4004
#define DISP_REG_CLK_ADDR                                       (HW_REG_BASE + DISP_REG_CLK_OFFSET)

/* JTAG */

#define DISP_REG_JTAG_OFFSET                                    0x4006
#define DISP_REG_JTAG_ADDR                                      (HW_REG_BASE + DISP_REG_JTAG_OFFSET)

/* EXT */

#define DISP_REG_EXT_OFFSET                                     0x4008
#define DISP_REG_EXT_ADDR                                       (HW_REG_BASE + DISP_REG_EXT_OFFSET)

/* MC */

#define DISP_REG_MC_OFFSET                                      0x4010
#define DISP_REG_MC_ADDR                                        (HW_REG_BASE + DISP_REG_MC_OFFSET)

/* MCCHAT */

#define DISP_REG_MCCHAT_OFFSET                                  0x4012
#define DISP_REG_MCCHAT_ADDR                                    (HW_REG_BASE + DISP_REG_MCCHAT_OFFSET)

/* MC2 */

#define DISP_REG_MC2_OFFSET                                     0x4014
#define DISP_REG_MC2_ADDR                                       (HW_REG_BASE + DISP_REG_MC2_OFFSET)


/* WL */

#define DISP_REG_WL_OFFSET                                      0x4020
#define DISP_REG_WL_ADDR                                        (HW_REG_BASE + DISP_REG_WL_OFFSET)

/* OP */

#define DISP_REG_OP_OFFSET                                      0x4024
#define DISP_REG_OP_ADDR                                        (HW_REG_BASE + DISP_REG_OP_OFFSET)


/*
 * Definitions of Register fields
 */


/* ROM */

#define DISP_REG_SCFG_ROM_ROMWE_SHIFT                           16
#define DISP_REG_SCFG_ROM_ROMWE_SIZE                            1
#define DISP_REG_SCFG_ROM_ROMWE_MASK                            0x00010000

#define DISP_REG_SCFG_ROM_ARM7FUSE_SHIFT                        10
#define DISP_REG_SCFG_ROM_ARM7FUSE_SIZE                         1
#define DISP_REG_SCFG_ROM_ARM7FUSE_MASK                         0x00000400

#define DISP_REG_SCFG_ROM_ARM7RSEL_SHIFT                        9
#define DISP_REG_SCFG_ROM_ARM7RSEL_SIZE                         1
#define DISP_REG_SCFG_ROM_ARM7RSEL_MASK                         0x00000200

#define DISP_REG_SCFG_ROM_ARM7SEL_SHIFT                         8
#define DISP_REG_SCFG_ROM_ARM7SEL_SIZE                          1
#define DISP_REG_SCFG_ROM_ARM7SEL_MASK                          0x00000100

#define DISP_REG_SCFG_ROM_ARM9RSEL_SHIFT                        1
#define DISP_REG_SCFG_ROM_ARM9RSEL_SIZE                         1
#define DISP_REG_SCFG_ROM_ARM9RSEL_MASK                         0x00000002

#define DISP_REG_SCFG_ROM_ARM9SEL_SHIFT                         0
#define DISP_REG_SCFG_ROM_ARM9SEL_SIZE                          1
#define DISP_REG_SCFG_ROM_ARM9SEL_MASK                          0x00000001

#ifndef SDK_ASM
#define DISP_REG_SCFG_ROM_FIELD( romwe, arm7fuse, arm7rsel, arm7sel, arm9rsel, arm9sel ) \
    (u32)( \
    ((u32)(romwe) << DISP_REG_SCFG_ROM_ROMWE_SHIFT) | \
    ((u32)(arm7fuse) << DISP_REG_SCFG_ROM_ARM7FUSE_SHIFT) | \
    ((u32)(arm7rsel) << DISP_REG_SCFG_ROM_ARM7RSEL_SHIFT) | \
    ((u32)(arm7sel) << DISP_REG_SCFG_ROM_ARM7SEL_SHIFT) | \
    ((u32)(arm9rsel) << DISP_REG_SCFG_ROM_ARM9RSEL_SHIFT) | \
    ((u32)(arm9sel) << DISP_REG_SCFG_ROM_ARM9SEL_SHIFT))
#endif


/* A9ROM */

#define DISP_REG_SCFG_A9ROM_RSEL_SHIFT                          1
#define DISP_REG_SCFG_A9ROM_RSEL_SIZE                           1
#define DISP_REG_SCFG_A9ROM_RSEL_MASK                           0x02

#define DISP_REG_SCFG_A9ROM_SEC_SHIFT                           0
#define DISP_REG_SCFG_A9ROM_SEC_SIZE                            1
#define DISP_REG_SCFG_A9ROM_SEC_MASK                            0x01

#ifndef SDK_ASM
#define DISP_REG_SCFG_A9ROM_FIELD( rsel, sec ) \
    (u8)( \
    ((u32)(rsel) << DISP_REG_SCFG_A9ROM_RSEL_SHIFT) | \
    ((u32)(sec) << DISP_REG_SCFG_A9ROM_SEC_SHIFT))
#endif


/* A7ROM */

#define DISP_REG_SCFG_A7ROM_FUSE_SHIFT                          2
#define DISP_REG_SCFG_A7ROM_FUSE_SIZE                           1
#define DISP_REG_SCFG_A7ROM_FUSE_MASK                           0x04

#define DISP_REG_SCFG_A7ROM_RSEL_SHIFT                          1
#define DISP_REG_SCFG_A7ROM_RSEL_SIZE                           1
#define DISP_REG_SCFG_A7ROM_RSEL_MASK                           0x02

#define DISP_REG_SCFG_A7ROM_SEC_SHIFT                           0
#define DISP_REG_SCFG_A7ROM_SEC_SIZE                            1
#define DISP_REG_SCFG_A7ROM_SEC_MASK                            0x01

#ifndef SDK_ASM
#define DISP_REG_SCFG_A7ROM_FIELD( fuse, rsel, sec ) \
    (u8)( \
    ((u32)(fuse) << DISP_REG_SCFG_A7ROM_FUSE_SHIFT) | \
    ((u32)(rsel) << DISP_REG_SCFG_A7ROM_RSEL_SHIFT) | \
    ((u32)(sec) << DISP_REG_SCFG_A7ROM_SEC_SHIFT))
#endif


/* ROMWE */

#define DISP_REG_SCFG_ROMWE_WE_SHIFT                            0
#define DISP_REG_SCFG_ROMWE_WE_SIZE                             1
#define DISP_REG_SCFG_ROMWE_WE_MASK                             0x0001

#ifndef SDK_ASM
#define DISP_REG_SCFG_ROMWE_FIELD( we ) \
    (u16)( \
    ((u32)(we) << DISP_REG_SCFG_ROMWE_WE_SHIFT))
#endif


/* CLK */

#define DISP_REG_SCFG_CLK_SNDMCLK_SHIFT                         8
#define DISP_REG_SCFG_CLK_SNDMCLK_SIZE                          1
#define DISP_REG_SCFG_CLK_SNDMCLK_MASK                          0x0100

#define DISP_REG_SCFG_CLK_WRAMHCLK_SHIFT                        7
#define DISP_REG_SCFG_CLK_WRAMHCLK_SIZE                         1
#define DISP_REG_SCFG_CLK_WRAMHCLK_MASK                         0x0080

#define DISP_REG_SCFG_CLK_AESHCLK_SHIFT                         2
#define DISP_REG_SCFG_CLK_AESHCLK_SIZE                          1
#define DISP_REG_SCFG_CLK_AESHCLK_MASK                          0x0004

#define DISP_REG_SCFG_CLK_SD2HCLK_SHIFT                         1
#define DISP_REG_SCFG_CLK_SD2HCLK_SIZE                          1
#define DISP_REG_SCFG_CLK_SD2HCLK_MASK                          0x0002

#define DISP_REG_SCFG_CLK_SD1HCLK_SHIFT                         0
#define DISP_REG_SCFG_CLK_SD1HCLK_SIZE                          1
#define DISP_REG_SCFG_CLK_SD1HCLK_MASK                          0x0001

#ifndef SDK_ASM
#define DISP_REG_SCFG_CLK_FIELD( sndmclk, wramhclk, aeshclk, sd2hclk, sd1hclk ) \
    (u16)( \
    ((u32)(sndmclk) << DISP_REG_SCFG_CLK_SNDMCLK_SHIFT) | \
    ((u32)(wramhclk) << DISP_REG_SCFG_CLK_WRAMHCLK_SHIFT) | \
    ((u32)(aeshclk) << DISP_REG_SCFG_CLK_AESHCLK_SHIFT) | \
    ((u32)(sd2hclk) << DISP_REG_SCFG_CLK_SD2HCLK_SHIFT) | \
    ((u32)(sd1hclk) << DISP_REG_SCFG_CLK_SD1HCLK_SHIFT))
#endif


/* JTAG */

#define DISP_REG_SCFG_JTAG_DSPJE_SHIFT                          8
#define DISP_REG_SCFG_JTAG_DSPJE_SIZE                           1
#define DISP_REG_SCFG_JTAG_DSPJE_MASK                           0x0100

#define DISP_REG_SCFG_JTAG_CPUJE_SHIFT                          1
#define DISP_REG_SCFG_JTAG_CPUJE_SIZE                           1
#define DISP_REG_SCFG_JTAG_CPUJE_MASK                           0x0002

#define DISP_REG_SCFG_JTAG_ARM7SEL_SHIFT                        0
#define DISP_REG_SCFG_JTAG_ARM7SEL_SIZE                         1
#define DISP_REG_SCFG_JTAG_ARM7SEL_MASK                         0x0001

#ifndef SDK_ASM
#define DISP_REG_SCFG_JTAG_FIELD( dspje, cpuje, arm7sel ) \
    (u16)( \
    ((u32)(dspje) << DISP_REG_SCFG_JTAG_DSPJE_SHIFT) | \
    ((u32)(cpuje) << DISP_REG_SCFG_JTAG_CPUJE_SHIFT) | \
    ((u32)(arm7sel) << DISP_REG_SCFG_JTAG_ARM7SEL_SHIFT))
#endif


/* EXT */

#define DISP_REG_SCFG_EXT_CFG_SHIFT                             31
#define DISP_REG_SCFG_EXT_CFG_SIZE                              1
#define DISP_REG_SCFG_EXT_CFG_MASK                              0x80000000

#define DISP_REG_SCFG_EXT_PUENABLE_SHIFT                        28
#define DISP_REG_SCFG_EXT_PUENABLE_SIZE                         1
#define DISP_REG_SCFG_EXT_PUENABLE_MASK                         0x10000000

#define DISP_REG_SCFG_EXT_SD20_SHIFT                            28
#define DISP_REG_SCFG_EXT_SD20_SIZE                             1
#define DISP_REG_SCFG_EXT_SD20_MASK                             0x10000000

#define DISP_REG_SCFG_EXT_NEWB_ACCESS_E_SHIFT                   16
#define DISP_REG_SCFG_EXT_NEWB_ACCESS_E_SIZE                    10
#define DISP_REG_SCFG_EXT_NEWB_ACCESS_E_MASK                    0x03ff0000

#define DISP_REG_SCFG_EXT_WRAM_SHIFT                            25
#define DISP_REG_SCFG_EXT_WRAM_SIZE                             1
#define DISP_REG_SCFG_EXT_WRAM_MASK                             0x02000000

#define DISP_REG_SCFG_EXT_MC_B_SHIFT                            24
#define DISP_REG_SCFG_EXT_MC_B_SIZE                             1
#define DISP_REG_SCFG_EXT_MC_B_MASK                             0x01000000

#define DISP_REG_SCFG_EXT_GPIO_SHIFT                            23
#define DISP_REG_SCFG_EXT_GPIO_SIZE                             1
#define DISP_REG_SCFG_EXT_GPIO_MASK                             0x00800000

#define DISP_REG_SCFG_EXT_I2C_SHIFT                             22
#define DISP_REG_SCFG_EXT_I2C_SIZE                              1
#define DISP_REG_SCFG_EXT_I2C_MASK                              0x00400000

#define DISP_REG_SCFG_EXT_I2S_SHIFT                             21
#define DISP_REG_SCFG_EXT_I2S_SIZE                              1
#define DISP_REG_SCFG_EXT_I2S_MASK                              0x00200000

#define DISP_REG_SCFG_EXT_MIC_SHIFT                             20
#define DISP_REG_SCFG_EXT_MIC_SIZE                              1
#define DISP_REG_SCFG_EXT_MIC_MASK                              0x00100000

#define DISP_REG_SCFG_EXT_SD2_SHIFT                             19
#define DISP_REG_SCFG_EXT_SD2_SIZE                              1
#define DISP_REG_SCFG_EXT_SD2_MASK                              0x00080000

#define DISP_REG_SCFG_EXT_SD1_SHIFT                             18
#define DISP_REG_SCFG_EXT_SD1_SIZE                              1
#define DISP_REG_SCFG_EXT_SD1_MASK                              0x00040000

#define DISP_REG_SCFG_EXT_AES_SHIFT                             17
#define DISP_REG_SCFG_EXT_AES_SIZE                              1
#define DISP_REG_SCFG_EXT_AES_MASK                              0x00020000

#define DISP_REG_SCFG_EXT_DMAC_SHIFT                            16
#define DISP_REG_SCFG_EXT_DMAC_SIZE                             1
#define DISP_REG_SCFG_EXT_DMAC_MASK                             0x00010000

#define DISP_REG_SCFG_EXT_NITROB_EX_E_SHIFT                     8
#define DISP_REG_SCFG_EXT_NITROB_EX_E_SIZE                      8
#define DISP_REG_SCFG_EXT_NITROB_EX_E_MASK                      0x0000ff00

#define DISP_REG_SCFG_EXT_PSRAM_SHIFT                           14
#define DISP_REG_SCFG_EXT_PSRAM_SIZE                            2
#define DISP_REG_SCFG_EXT_PSRAM_MASK                            0x0000c000

#define DISP_REG_SCFG_EXT_VRAM_SHIFT                            13
#define DISP_REG_SCFG_EXT_VRAM_SIZE                             1
#define DISP_REG_SCFG_EXT_VRAM_MASK                             0x00002000

#define DISP_REG_SCFG_EXT_LCDC_SHIFT                            12
#define DISP_REG_SCFG_EXT_LCDC_SIZE                             1
#define DISP_REG_SCFG_EXT_LCDC_MASK                             0x00001000

#define DISP_REG_SCFG_EXT_SIO_SHIFT                             11
#define DISP_REG_SCFG_EXT_SIO_SIZE                              1
#define DISP_REG_SCFG_EXT_SIO_MASK                              0x00000800

#define DISP_REG_SCFG_EXT_DSEL_SHIFT                            10
#define DISP_REG_SCFG_EXT_DSEL_SIZE                             1
#define DISP_REG_SCFG_EXT_DSEL_MASK                             0x00000400

#define DISP_REG_SCFG_EXT_SPI_SHIFT                             9
#define DISP_REG_SCFG_EXT_SPI_SIZE                              1
#define DISP_REG_SCFG_EXT_SPI_MASK                              0x00000200

#define DISP_REG_SCFG_EXT_INTC_SHIFT                            8
#define DISP_REG_SCFG_EXT_INTC_SIZE                             1
#define DISP_REG_SCFG_EXT_INTC_MASK                             0x00000100

#define DISP_REG_SCFG_EXT_NITROB_MOD_E_SHIFT                    0
#define DISP_REG_SCFG_EXT_NITROB_MOD_E_SIZE                     8
#define DISP_REG_SCFG_EXT_NITROB_MOD_E_MASK                     0x000000ff

#define DISP_REG_SCFG_EXT_MC_SHIFT                              7
#define DISP_REG_SCFG_EXT_MC_SIZE                               1
#define DISP_REG_SCFG_EXT_MC_MASK                               0x00000080

#define DISP_REG_SCFG_EXT_SND_SHIFT                             2
#define DISP_REG_SCFG_EXT_SND_SIZE                              1
#define DISP_REG_SCFG_EXT_SND_MASK                              0x00000004

#define DISP_REG_SCFG_EXT_SDMA_SHIFT                            1
#define DISP_REG_SCFG_EXT_SDMA_SIZE                             1
#define DISP_REG_SCFG_EXT_SDMA_MASK                             0x00000002

#define DISP_REG_SCFG_EXT_DMA_SHIFT                             0
#define DISP_REG_SCFG_EXT_DMA_SIZE                              1
#define DISP_REG_SCFG_EXT_DMA_MASK                              0x00000001

#ifndef SDK_ASM
#define DISP_REG_SCFG_EXT_FIELD( cfg, puenable, sd20, newb_access_e, wram, mc_b, gpio, i2c, i2s, mic, sd2, sd1, aes, dmac, nitrob_ex_e, psram, vram, lcdc, sio, dsel, spi, intc, nitrob_mod_e, mc, snd, sdma, dma ) \
    (u32)( \
    ((u32)(cfg) << DISP_REG_SCFG_EXT_CFG_SHIFT) | \
    ((u32)(puenable) << DISP_REG_SCFG_EXT_PUENABLE_SHIFT) | \
    ((u32)(sd20) << DISP_REG_SCFG_EXT_SD20_SHIFT) | \
    ((u32)(newb_access_e) << DISP_REG_SCFG_EXT_NEWB_ACCESS_E_SHIFT) | \
    ((u32)(wram) << DISP_REG_SCFG_EXT_WRAM_SHIFT) | \
    ((u32)(mc_b) << DISP_REG_SCFG_EXT_MC_B_SHIFT) | \
    ((u32)(gpio) << DISP_REG_SCFG_EXT_GPIO_SHIFT) | \
    ((u32)(i2c) << DISP_REG_SCFG_EXT_I2C_SHIFT) | \
    ((u32)(i2s) << DISP_REG_SCFG_EXT_I2S_SHIFT) | \
    ((u32)(mic) << DISP_REG_SCFG_EXT_MIC_SHIFT) | \
    ((u32)(sd2) << DISP_REG_SCFG_EXT_SD2_SHIFT) | \
    ((u32)(sd1) << DISP_REG_SCFG_EXT_SD1_SHIFT) | \
    ((u32)(aes) << DISP_REG_SCFG_EXT_AES_SHIFT) | \
    ((u32)(dmac) << DISP_REG_SCFG_EXT_DMAC_SHIFT) | \
    ((u32)(nitrob_ex_e) << DISP_REG_SCFG_EXT_NITROB_EX_E_SHIFT) | \
    ((u32)(psram) << DISP_REG_SCFG_EXT_PSRAM_SHIFT) | \
    ((u32)(vram) << DISP_REG_SCFG_EXT_VRAM_SHIFT) | \
    ((u32)(lcdc) << DISP_REG_SCFG_EXT_LCDC_SHIFT) | \
    ((u32)(sio) << DISP_REG_SCFG_EXT_SIO_SHIFT) | \
    ((u32)(dsel) << DISP_REG_SCFG_EXT_DSEL_SHIFT) | \
    ((u32)(spi) << DISP_REG_SCFG_EXT_SPI_SHIFT) | \
    ((u32)(intc) << DISP_REG_SCFG_EXT_INTC_SHIFT) | \
    ((u32)(nitrob_mod_e) << DISP_REG_SCFG_EXT_NITROB_MOD_E_SHIFT) | \
    ((u32)(mc) << DISP_REG_SCFG_EXT_MC_SHIFT) | \
    ((u32)(snd) << DISP_REG_SCFG_EXT_SND_SHIFT) | \
    ((u32)(sdma) << DISP_REG_SCFG_EXT_SDMA_SHIFT) | \
    ((u32)(dma) << DISP_REG_SCFG_EXT_DMA_SHIFT))
#endif

/* MC1 */

#define DISP_REG_MI_MC1_CC_SHIFT                                16
#define DISP_REG_MI_MC1_CC_SIZE                                 16
#define DISP_REG_MI_MC1_CC_MASK                                 0xffff0000

#define DISP_REG_MI_MC1_SWP_SHIFT                               15
#define DISP_REG_MI_MC1_SWP_SIZE                                1
#define DISP_REG_MI_MC1_SWP_MASK                                0x00008000

#define DISP_REG_MI_MC1_SL2_STATUS_SHIFT                        4
#define DISP_REG_MI_MC1_SL2_STATUS_SIZE                         4
#define DISP_REG_MI_MC1_SL2_STATUS_MASK                         0x000000f0

#define DISP_REG_MI_MC1_SL2_M1_SHIFT                            7
#define DISP_REG_MI_MC1_SL2_M1_SIZE                             1
#define DISP_REG_MI_MC1_SL2_M1_MASK                             0x00000080

#define DISP_REG_MI_MC1_SL2_M0_SHIFT                            6
#define DISP_REG_MI_MC1_SL2_M0_SIZE                             1
#define DISP_REG_MI_MC1_SL2_M0_MASK                             0x00000040

#define DISP_REG_MI_MC1_SL2_CDET_SHIFT                          4
#define DISP_REG_MI_MC1_SL2_CDET_SIZE                           1
#define DISP_REG_MI_MC1_SL2_CDET_MASK                           0x00000010

#define DISP_REG_MI_MC1_SL1_STATUS_SHIFT                        0
#define DISP_REG_MI_MC1_SL1_STATUS_SIZE                         4
#define DISP_REG_MI_MC1_SL1_STATUS_MASK                         0x0000000f

#define DISP_REG_MI_MC1_SL1_M1_SHIFT                            3
#define DISP_REG_MI_MC1_SL1_M1_SIZE                             1
#define DISP_REG_MI_MC1_SL1_M1_MASK                             0x00000008

#define DISP_REG_MI_MC1_SL1_M0_SHIFT                            2
#define DISP_REG_MI_MC1_SL1_M0_SIZE                             1
#define DISP_REG_MI_MC1_SL1_M0_MASK                             0x00000004

#define DISP_REG_MI_MC1_SL1_CDET_SHIFT                          0
#define DISP_REG_MI_MC1_SL1_CDET_SIZE                           1
#define DISP_REG_MI_MC1_SL1_CDET_MASK                           0x00000001


/* MC */

#define DISP_REG_MI_MC_SWP_SHIFT                                15
#define DISP_REG_MI_MC_SWP_SIZE                                 1
#define DISP_REG_MI_MC_SWP_MASK                                 0x8000

#define DISP_REG_MI_MC_SL2_MODE_SHIFT                           6
#define DISP_REG_MI_MC_SL2_MODE_SIZE                            2
#define DISP_REG_MI_MC_SL2_MODE_MASK                            0x00c0

#define DISP_REG_MI_MC_SL2_CDET_SHIFT                           4
#define DISP_REG_MI_MC_SL2_CDET_SIZE                            1
#define DISP_REG_MI_MC_SL2_CDET_MASK                            0x0010

#define DISP_REG_MI_MC_SL1_MODE_SHIFT                           2
#define DISP_REG_MI_MC_SL1_MODE_SIZE                            2
#define DISP_REG_MI_MC_SL1_MODE_MASK                            0x000c

#define DISP_REG_MI_MC_SL1_CDET_SHIFT                           0
#define DISP_REG_MI_MC_SL1_CDET_SIZE                            1
#define DISP_REG_MI_MC_SL1_CDET_MASK                            0x0001

#ifndef SDK_ASM
#define DISP_REG_MI_MC_FIELD( swp, sl2_mode, sl2_cdet, sl1_mode, sl1_cdet ) \
    (u16)( \
    ((u32)(swp) << DISP_REG_MI_MC_SWP_SHIFT) | \
    ((u32)(sl2_mode) << DISP_REG_MI_MC_SL2_MODE_SHIFT) | \
    ((u32)(sl2_cdet) << DISP_REG_MI_MC_SL2_CDET_SHIFT) | \
    ((u32)(sl1_mode) << DISP_REG_MI_MC_SL1_MODE_SHIFT) | \
    ((u32)(sl1_cdet) << DISP_REG_MI_MC_SL1_CDET_SHIFT))
#endif

/* MCCHAT */

#define DISP_REG_MI_MCCHAT_CC_SHIFT                             0
#define DISP_REG_MI_MCCHAT_CC_SIZE                              16
#define DISP_REG_MI_MCCHAT_CC_MASK                              0xffff

#ifndef SDK_ASM
#define DISP_REG_MI_MCCHAT_FIELD( cc ) \
    (u16)( \
    ((u32)(cc) << DISP_REG_MI_MCCHAT_CC_SHIFT))
#endif


/* MC2 */

#define DISP_REG_MI_MC2_CA_SHIFT                                0
#define DISP_REG_MI_MC2_CA_SIZE                                 16
#define DISP_REG_MI_MC2_CA_MASK                                 0xffff

#ifndef SDK_ASM
#define DISP_REG_MI_MC2_FIELD( ca ) \
    (u16)( \
    ((u32)(ca) << DISP_REG_MI_MC2_CA_SHIFT))
#endif


/* WL */

#define DISP_REG_SCFG_WL_OFFB_SHIFT                             0
#define DISP_REG_SCFG_WL_OFFB_SIZE                              1
#define DISP_REG_SCFG_WL_OFFB_MASK                              0x0001

#ifndef SDK_ASM
#define DISP_REG_SCFG_WL_FIELD( offb ) \
    (u16)( \
    ((u32)(offb) << DISP_REG_SCFG_WL_OFFB_SHIFT))
#endif


/* OP */

#define DISP_REG_SCFG_OP_OP1_SHIFT                              1
#define DISP_REG_SCFG_OP_OP1_SIZE                               1
#define DISP_REG_SCFG_OP_OP1_MASK                               0x0002

#define DISP_REG_SCFG_OP_OP0_SHIFT                              0
#define DISP_REG_SCFG_OP_OP0_SIZE                               1
#define DISP_REG_SCFG_OP_OP0_MASK                               0x0001

#define DISP_REG_SCFG_OP_OPT_SHIFT                              0
#define DISP_REG_SCFG_OP_OPT_SIZE                               2
#define DISP_REG_SCFG_OP_OPT_MASK                               0x0003

#ifndef SDK_ASM

#define DISP_REG_SCFG_OP_FIELD( op1, op0, opt ) \
    (u16)( \
    ((u32)(op1) << DISP_REG_SCFG_OP_OP1_SHIFT) | \
    ((u32)(op0) << DISP_REG_SCFG_OP_OP0_SHIFT) | \
    ((u32)(opt) << DISP_REG_SCFG_OP_OPT_SHIFT))
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_IOREG_SCFG_H_ */
#endif
