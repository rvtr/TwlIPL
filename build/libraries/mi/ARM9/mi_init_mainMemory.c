/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - mi
  File:     mi_init_mainMemory.c

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
#include        <firm/mi.h>

void MIi_InitMainMemCRCore( void );

/*---------------------------------------------------------------------------*
  Name:         MIi_IsMainMemoryInitialized

  Description:

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL MIi_IsMainMemoryInitialized( void )
{
    return  (BOOL)((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_CE2_MASK) >> REG_MI_EXMEMCNT_CE2_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MIi_WaitMainMemoryInitialize

  Description:

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_WaitMainMemoryInitialize( void )
{
    while( MIi_IsMainMemoryInitialized() == FALSE )
    {
    }
}


/*---------------------------------------------------------------------------*
  Name:         MIi_InitMainMemCR

  Description:  change mainmem into the burst mode

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#include        <nitro/code32.h>

asm void  MIi_InitMainMemCR( BOOL setCR )
{
        mov     r12, lr

        mov     r2, r0

        mov     r0, #0x8000        // low period 0.97ms
        bl      OS_SpinWaitCpuCycles

        ldr     r3, =REG_EXMEMCNT_ADDR
        mov     r1, #REG_MI_EXMEMCNT_CE2_MASK
        ldrh    r0, [r3]
        tst     r0, r1
        bxne    r12

        strh    r1, [r3]

        mov     r0, #0x8000        // high period 0.97ms
        bl      OS_SpinWaitCpuCycles

        cmp     r2, #FALSE
        beq     @10

        // メインメモリが同期モード時（ハードリセット時）に
        // 非同期モード（CLK固定）でコマンド発行しても大丈夫
        ldr     r3,  =HW_WRAM_AREA - 2
        bl      MIi_InitMainMemCRCore
#ifdef TWL_PLATFORM_TS
        ldr     r3,  =HW_MAIN_MEM_HI_EX_END - 2
        bl      MIi_InitMainMemCRCore
#endif // TWL_PLATFORM_TS

@10:
        ldr     r3, =REG_EXMEMCNT_ADDR
        mov     r1, #REG_MI_EXMEMCNT_IFM_MASK | REG_MI_EXMEMCNT_CE2_MASK
        strh    r1, [r3]

        bx      r12
}

asm void MIi_InitMainMemCRCore( void )
{
//        stmfd   sp!, { lr }

#ifdef TWL_PLATFORM_BB
        mov     r2, lr
        ldr     r0,  =MMEM_DCR0_BURST_MODE | MMEM_DCR0_BURST_CONTINUOUS \
                    | MMEM_DCR0_PARTIAL_REFRESH_NONE | MMEM_DCR0_SB1
        ldr     r1,  =MMEM_DCR1_1ST_R4_W3  | MMEM_DCR1_BURST_WRITE | MMEM_DCR1_BURST_LINER \
                    | MMEM_DCR1_CLOCK_TRIGGER_UP | MMEM_DCR1_SB1
        ldrh    lr, [r3]
        strh    lr, [r3]
        strh    lr, [r3]
        mov     lr, r2
        ldr     r2, =HW_MAIN_MEM | MMEM_DCR2_CLOCK_TRIGGER_UP \
                    | MMEM_DCR2_BURST_MODE | MMEM_DCR2_BURST_CONTINUOUS \
                    | MMEM_DCR2_1ST_R4_W3  | MMEM_DCR2_BURST_WRITE | MMEM_DCR2_BURST_LINER \
                    | MMEM_DCR2_PARTIAL_REFRESH_NONE | MMEM_DCR2_SB1
        strh    r0, [r3]
        strh    r1, [r3]
        ldrh    r3, [r2]
#else // TWL_PLATFORM_TS
        mov     r2, lr
        ldr     r0,  =MMEM_TCR0
        ldr     r1,  =MMEM_TCR1
        ldrh    lr, [r3]
        strh    lr, [r3]
        strh    lr, [r3]
        mov     lr, r2
        ldr     r2,  =MMEM_TCR2
        strh    r0, [r3]
        strh    r1, [r3]
        strh    r2, [r3]
#endif // TWL_PLATFORM_TS

//        ldmfd   sp!, { lr }
        bx      lr
}

asm void MIi_GetMainMemCR( MIMmemCR* dest )
{
#ifdef TWL_PLATFORM_TS
        ldr     r3,  =HW_WRAM_AREA - 2
        ldr     r1,  =MMEM_TCR0_R
        ldrh    r2, [r3]
        strh    r2, [r3]
        strh    r2, [r3]
        strh    r1, [r3]
        ldrh    r1, [r3]
        ldrh    r2, [r3]
        strh    r2, [r0, #4]
        strh    r1, [r0, #2]
        ldr     r1,  =MMEM_TCR0_R
        strh    r1, [r0, #0]
#endif // TWL_PLATFORM_TS

        bx      lr
}