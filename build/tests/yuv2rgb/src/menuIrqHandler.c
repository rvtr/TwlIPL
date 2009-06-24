// This IRQ Handler is for ARM9 only.
#include "menuIrqHandler.h"
// <twl/hw/common/lcd.h> not supplied
#include <nitro/hw/common/lcd.h>

typedef struct MenuIrqHandlerLogControl {
    // keep order! {End, Cur, VBlankCount, Top} for ldmia/stmia
    MenuIrqHandlerLogBuffer* mpBufferEnd;
    MenuIrqHandlerLogBuffer* mpBufferCur;
    u32 mVBlankCount;
    MenuIrqHandlerLogBuffer* mpBufferTop;
} MenuIrqHandlerLogControl;

extern void MenuIrqHandler( void );

#include    <twl/dtcm_begin.h>
#ifndef TWL_IPL_FINAL
static MenuIrqHandlerLogControl MenuIrqHandlerLogCtrl;
#endif
#include    <twl/dtcm_end.h>

#include    <twl/os/common/interrupt.h>
#include    <twl/code32.h>
#include    <twl/itcm_begin.h>

#ifndef TWL_IPL_FINAL
// derived from OS_IrqHandler
/*---------------------------------------------------------------------------*
  Name:         MenuIrqHandler

  Description:  IRQ handler. call handler according to OS_InterruptTable

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void MenuIrqHandler( void )
{
        stmfd   sp!, { lr }                     // save LR

        // get IE address
        mov     r12,      #HW_REG_BASE
        add     r12, r12, #(REG_IE_OFFSET - 4)        // r12: REG_IE address - 4

        // get IME
        ldrh    r0, [ r12, #REG_IME_ADDR - (REG_IE_ADDR - 4) ]  // r0: IME

        // get IE&IF
        ldmib   r12!, { r1-r2 }                 // r1: IE, r2: IF

        tst     r0, #0x01
        andnes  r1, r1, r2                      // r1: IE & IF

        // if (IME==0) || (IE&IF==0) then return (without changing IF)
        ldmeqfd sp!, { pc }

        //--------------------------------------------------
        // IRQ HANDLING CODE for ARCHITECTURE VERSION 5
        //--------------------------------------------------

        ldr     lr, =MenuIrqHandlerLogCtrl

        // get lowest 1 bit
        rsb     r3, r1, #0
        and     r3, r1, r3

        // clear IF
        str     r3,  [ r12 ], #-REG_IF_OFFSET

        ldmia   lr!, {r0, r1, r2} // mpBufferEnd, mpBufferCur, mVBlankCount
        cmp     r1, r0
        bcs     @logSkip // log full or no buffer

@logWrap:
        ldrh    r0, [r12, #REG_VCOUNT_ADDR - HW_REG_BASE]
        add     r1, r1, #sizeof(MenuIrqHandlerLogBuffer)
        tst     r3, #OS_IE_V_BLANK
        addne   r2, r2, #1
        bic     r2, r2, #1 << 18  // lower 18bits only
        stmdb   lr, {r1, r2} // update mpBufferCur, mVBlankCount

        orr     r2, r2, r0, lsl #23 // mInVLineCount | mInVBlankCount
        // get jump vector
        ldr     r0, =OS_IRQTable + ((OS_IRQ_TABLE_MAX - 1) << 2)
        clz     r3, r3
        ldr     r0, [r0, -r3, lsl #2]
        orr     r2, r2, r3, lsl #18 // mIntr
        ldr     r3, [r12, #REG_TM0CNT_L_ADDR - HW_REG_BASE]
        stmfd   sp!, {r1, r2, r3} // mpBufferCur, word0, tick

        blx     r0

        mov     r12, #HW_REG_BASE
        ldr     lr, [r12, #REG_TM0CNT_L_ADDR - HW_REG_BASE]
        ldrh    r0, [r12, #REG_VCOUNT_ADDR - HW_REG_BASE]
        ldmfd   sp!, {r1, r2, r3}
        subs    r12, r0, r2, lsr #23 // mVLineCounts
        addmi   r12, r12, #HW_LCD_HEIGHT
        addmi   r12, r12, #HW_LCD_VBLANK
        sub     r3, lr, r3 // mTicks
        orr     r3, r12, r3, lsl #16
        stmdb   r1, {r2, r3} // word0, word1
        b       OS_IrqHandler_ThreadSwitch

@logSkip:
        cmp     r1, #0
        ldrne   r1, [lr] // mpBufferTop
        bne     @logWrap

        // get jump vector
        ldr     r0, =OS_IRQTable + ((OS_IRQ_TABLE_MAX - 1) << 2)
        clz     r3, r3
        ldr     r0, [r0, -r3, lsl #2]
        ldr     lr, =OS_IrqHandler_ThreadSwitch
        bx      r0      // set return address for thread rescheduling
}
#endif

#include    <twl/itcm_end.h>
#include    <twl/codereset.h>


void MenuIrqHandlerStart( MenuIrqHandlerLogBuffer* pBuffer, u32 size )
{
#ifndef TWL_IPL_FINAL
    u32 count = size / sizeof(MenuIrqHandlerLogBuffer);
    MenuIrqHandlerLogControl* pCtrl = &MenuIrqHandlerLogCtrl;
    BOOL enabled = OS_DisableIrq();

    *(u32 *)(HW_DTCM + HW_DTCM_SIZE - HW_DTCM_SYSRV_SIZE + HW_DTCM_SYSRV_OFS_INTR_VECTOR)
    = (u32)MenuIrqHandler;

    if ( pBuffer && count ) {
        pCtrl->mpBufferTop =
        pCtrl->mpBufferCur = pBuffer;
        pCtrl->mpBufferEnd = pBuffer + count;
    } else {
        pCtrl->mpBufferTop =
        pCtrl->mpBufferCur =
        pCtrl->mpBufferEnd = NULL;
    }
    pCtrl->mVBlankCount = 0;

    OS_RestoreIrq(enabled);
#else
#pragma unused(pBuffer)
#pragma unused(size)
#endif
}

void MenuIrqHandlerEnd( void )
{
#ifndef TWL_IPL_FINAL
    *(u32 *)(HW_DTCM + HW_DTCM_SIZE - HW_DTCM_SYSRV_SIZE + HW_DTCM_SYSRV_OFS_INTR_VECTOR)
    = (u32)OS_IrqHandler;
    // keep buffer
#endif
}

BOOL MenuIrqHandlerIsUsed( void )
{
#ifndef TWL_IPL_FINAL
    return (*(u32 *)(HW_DTCM + HW_DTCM_SIZE - HW_DTCM_SYSRV_SIZE + HW_DTCM_SYSRV_OFS_INTR_VECTOR)
            == (u32)MenuIrqHandler);
#else
    return FALSE;
#endif
}

