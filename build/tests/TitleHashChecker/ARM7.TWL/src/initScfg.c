/*---------------------------------------------------------------------------*
  Project:  TwlSDK - components - mongoose.TWL
  File:     initScfg.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-01#$
  $Rev: 5238 $
  $Author: nakasima $
 *---------------------------------------------------------------------------*/

#include	<twl/memorymap_sp.h>
#include	<twl/hw/common/mmap_wramEnv.h>

extern void		INIT_InitializeScfg(void);

#include	<twl/code32.h>
/*---------------------------------------------------------------------------*
  Name:         INIT_InitializeScfg

  Description:  ローダーが行う SCFG ブロックへの設定処理を代行する為の関数。
                crt0 内の弱シンボルを上書きする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void
INIT_InitializeScfg(void)
{
		// SCFG enable?
		ldr			r2, =REG_EXT_ADDR
		ldr			r0, [r2]
		tst			r0, #REG_SCFG_EXT_CFG_MASK
		beq			@end

		/* NITRO 互換無線の送受信機能を動作 */
		ldr			r1, =REG_WL_ADDR
		ldrh		r0, [r1]
		orr			r0, r0, #REG_SCFG_WL_OFFB_MASK
		strh		r0, [r1]

		/* 新規サウンドブロックへのクロック供給 */
		ldr			r1, =REG_CLK_ADDR
		ldrh		r0, [r1]
		orr			r0, r0, #REG_SCFG_CLK_SNDMCLK_MASK
		strh		r0, [r1]

		/* ARM7 側の DMA には新 DMA 回路を採用 */
		ldr			r1, =REG_EXT_ADDR
		ldr			r0, [r1]
		orr			r0, r0, #REG_SCFG_EXT_DMAC_MASK
		str			r0, [r1]

		ldr			r2, =HW_PRV_WRAM_SYSRV
		//EXT(extentions)
		ldr			r3, =REG_EXT_ADDR
		ldr			r0, [r3]
		str			r0, [r2, #HWi_WSYS04_WRAMOFFSET]
		//OPT(bonding option)
		ldr			r3, =REG_OP_ADDR
		ldrb		r0, [r3]
		//A9ROM(ARM9 ROM)
		ldr			r3, =REG_A9ROM_ADDR
		ldrb		r1, [r3]
		and			r12,r1, #(REG_SCFG_A9ROM_RSEL_MASK | REG_SCFG_A9ROM_SEC_MASK)
		orr			r0, r0, r12, LSL #(HWi_WSYS08_ROM_ARM9RSEL_SHIFT - REG_SCFG_A9ROM_RSEL_SHIFT)
		//A7ROM(ARM7 ROM)
		ldr			r3, =REG_A7ROM_ADDR
		ldrb		r1, [r3]
		and			r12,r1, #(REG_SCFG_A7ROM_RSEL_MASK | REG_SCFG_A7ROM_FUSE_MASK)
		orr			r0, r0, r12, LSL #(HWi_WSYS08_ROM_ARM7RSEL_SHIFT - REG_SCFG_A7ROM_RSEL_SHIFT)
		//DS-WL(DS wireless)
		ldr			r3, =REG_WL_ADDR
		ldrb		r1, [r3]
		and			r12,r1, #REG_SCFG_WL_OFFB_MASK
		orr			r0, r0, r12, LSL #(HWi_WSYS08_WL_OFFB_SHIFT - REG_SCFG_WL_OFFB_SHIFT)
		strb		r0, [r2, #HWi_WSYS08_WRAMOFFSET]
		//OPT(JTAG info)
		ldr			r3, =REG_JTAG_ADDR
		ldrh		r0, [r3]
		and			r12,r0, #REG_SCFG_JTAG_DSPJE_MASK
		orr			r0, r0, r12, LSR #(REG_SCFG_JTAG_DSPJE_SHIFT - HWi_WSYS09_JTAG_DSPJE_SHIFT)
		//CLK(only wram clock)
		ldr			r3, =REG_CLK_ADDR
		ldrh		r1, [r3]
		and			r12,r1, #(REG_SCFG_CLK_AESHCLK_MASK | REG_SCFG_CLK_SD2HCLK_MASK | REG_SCFG_CLK_SD1HCLK_MASK)
		orr			r0, r0, r12, LSL #(HWi_WSYS09_CLK_SD1HCLK_SHIFT - REG_SCFG_CLK_SD1HCLK_SHIFT)
		and			r12,r1, #(REG_SCFG_CLK_SNDMCLK_MASK | REG_SCFG_CLK_WRAMHCLK_MASK)
		orr			r0, r0, r12, LSR #(REG_SCFG_CLK_WRAMHCLK_SHIFT - HWi_WSYS09_CLK_WRAMHCLK_SHIFT)
		strb		r0, [r2, #HWi_WSYS09_WRAMOFFSET]

		/* A7-SCFG ブロックへのアクセスを無効化 */
		ldr			r1, =REG_EXT_ADDR
		ldr			r0, [r1]
		bic			r0, r0, #REG_SCFG_EXT_CFG_MASK
		str			r0, [r1]

@end:
		bx		lr
}
#include	<twl/codereset.h>
