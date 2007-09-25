/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - init
  File:     crt0.c

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
#include        <nitro/code32.h>
#include        <firm.h>

extern void TwlMain(void);
extern void OS_IrqHandler(void);
extern void *const _start_ModuleParams[];
static void do_autoload(void);
static void detect_main_memory_size(void);
void    _start(void);
void    _start_AutoloadDoneCallback(void *argv[]);

extern void __call_static_initializers(void);
extern void _fp_init(void);

// from LCF
extern unsigned long SDK_IRQ_STACKSIZE[];
extern void SDK_STATIC_BSS_START(void); // static bss start address
extern void SDK_STATIC_BSS_END(void);  // static bss start address
extern void SDK_AUTOLOAD_START(void);  // autoload data will start from here
extern void SDK_AUTOLOAD_LIST(void);   // start pointer to autoload information
extern void SDK_AUTOLOAD_LIST_END(void);        // end pointer to autoload information

// volatile parameters in IPL's work memory
#define IPL_PARAM_CARD_ROM_HEADER      0x023FE940
#define IPL_PARAM_DOWNLOAD_PARAMETER   0x023FE904

//---- IRQ+SVC stack size in boot (this area is not cleared)
#define INITi_Initial_Stack   0x100

/*---------------------------------------------------------------------------*
  Name:         _start

  Description:  Start up

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void _start( void )
{
        ldr     r1, =REG_JTAG_ADDR
        ldrh    r2, [r1]
        orr     r2, r2, #REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK
        strh    r2, [r1]

        //---- set IME = 0
        //     ( use that LSB of HW_REG_BASE equal to 0 )
        mov             r12, #HW_REG_BASE
        str             r12, [r12, #REG_IME_OFFSET]

        //---- initialize stack pointer
        // SVC mode
        mov             r0, #HW_PSR_SVC_MODE
        msr             cpsr_c, r0
        ldr             sp, =HW_PRV_WRAM_SVC_STACK_END

        // IRQ mode
        mov             r0, #HW_PSR_IRQ_MODE
        msr             cpsr_c, r0
        ldr             r0, =HW_PRV_WRAM_IRQ_STACK_END
        mov             sp, r0

        // System mode
        ldr             r1, =SDK_IRQ_STACKSIZE
        sub             r1, r0, r1
        mov             r0, #HW_PSR_SYS_MODE
        msr             cpsr_csfx, r0
        sub             sp, r1, #4 // 4byte for stack check code

#if 0
        // move parameters from IPL's work memory to shared area
        ldr             r0, =IPL_PARAM_CARD_ROM_HEADER
        ldr             r1, =HW_CARD_ROM_HEADER
        add             r2, r1, #HW_CARD_ROM_HEADER_SIZE
@1_1:
        ldr             r3, [r0], #4
        str             r3, [r1], #4
        cmp             r1, r2
        bmi             @1_1
        ldr             r0, =IPL_PARAM_DOWNLOAD_PARAMETER
        add             r2, r1, #HW_DOWNLOAD_PARAMETER_SIZE
@1_2:
        ldr             r3, [r0], #4
        str             r3, [r1], #4
        cmp             r1, r2
        bmi             @1_2
#endif

        //---- wait for main memory mode into burst mode
        ldr             r3, =REG_EXMEMCNT_L_ADDR
        mov             r1, #REG_MI_EXMEMCNT_L_ECE2_MASK
@1:
        ldrh            r2, [r3]
        tst             r2, r1
        beq             @1

        //---- load autoload block and initialize bss
        bl              do_autoload

        //---- fill static static bss with 0
        ldr             r0, =_start_ModuleParams
        ldr             r1,  [r0, #12]   // BSS segment start
        ldr             r2,  [r0, #16]   // BSS segment end
        mov             r0, #0
@2:     cmp             r1, r2
        strcc           r0, [r1], #4
        bcc             @2

        //---- detect main memory size
        bl              detect_main_memory_size

        //---- set interrupt vector
        ldr             r1, =HW_INTR_VECTOR_BUF
        ldr             r0, =OS_IrqHandler
        str             r0, [r1, #0]

#ifndef SDK_NOINIT
        //---- for C++
        bl              _fp_init
        bl              TwlSpStartUp
        bl              __call_static_initializers
#endif

        //---- start (to 16bit code)
        ldr             r1, =TwlSpMain
        ldr             lr, =HW_RESET_VECTOR

        bx              r1
}

/*---------------------------------------------------------------------------*
  Name:         do_autoload

  Description:  put autoload data block according to autoload information,
                and clear static bss by filling with 0.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void   *const _start_ModuleParams[] = {
    (void *)SDK_AUTOLOAD_LIST,
    (void *)SDK_AUTOLOAD_LIST_END,
    (void *)SDK_AUTOLOAD_START,
    (void *)SDK_STATIC_BSS_START,
    (void *)SDK_STATIC_BSS_END,
};

static asm void do_autoload( void )
{
#define ptable          r0
#define infop           r1
#define infop_end       r2
#define src             r3
#define dest            r4
#define dest_size       r5
#define dest_end        r6
#define tmp             r7

        ldr     ptable, =_start_ModuleParams
        ldr     infop,      [ptable, #0]   // r1 = start pointer to autoload_info
        ldr     infop_end,  [ptable, #4]   // r2 = end pointer to autoload_info
        ldr     src,        [ptable, #8]   // r3 = autoload block

        //---- put each blocks according to autoload information
@2:
        cmp     infop, infop_end                // reach to end?
        beq     @skipout

        ldr     dest,      [infop], #4          // dest
        ldr     dest_size, [infop], #4          // size
        add     dest_end, dest, dest_size       // dest_end
#if 1
        mov     dest, dest_end
#else
@1:
        cmp     dest, dest_end
        ldrmi   tmp, [src],  #4                 // [dest++] <- [src++]
        strmi   tmp, [dest], #4
        bmi     @1
#endif
        //---- fill bss with 0
        ldr     dest_size, [infop], #4          // size
        add     dest_end, dest, dest_size       // bss end
        mov     tmp, #0
@4:
        cmp     dest, dest_end
        strcc   tmp, [dest], #4
        bcc     @4
        beq     @2

@skipout:
        // r0 = autoload_params
        b       _start_AutoloadDoneCallback   // Jump into the callback
}

/*---------------------------------------------------------------------------*
  Name:         _start_AutoloadDoneCallback

  Description:  hook for end of autoload (This is dummy target for DEBUGGER)

  Arguments:    argv: pointer for autoload parameters
                     argv[0] = SDK_AUTOLOAD_LIST
                     argv[1] = SDK_AUTOLOAD_LIST_END
                     argv[2] = SDK_AUTOLOAD_START
                     argv[3] = SDK_STATIC_BSS_START
                     argv[4] = SDK_STATIC_BSS_END

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void _start_AutoloadDoneCallback( void* argv[] )
{
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         detect_main_memory_size

  Description:  detect main memory size.
                result is written into (u32*)HW_MMEMCHECKER_SUB.
                value is [OS_CONSOLE_SIZE_4MB|OS_CONSOLE_SIZE_8MB]

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define OSi_IMAGE_DIFFERENCE 0x400000

static asm void detect_main_memory_size( void )
{
        mov     r0, #OS_CONSOLE_SIZE_4MB
        mov     r1, #0

        ldr     r2, =HW_MMEMCHECKER_SUB
        sub     r3, r2, #OSi_IMAGE_DIFFERENCE
@1:
        strh    r1, [r2]
        ldrh    r12, [r3]
        cmp     r1, r12

        movne   r0, #OS_CONSOLE_SIZE_8MB
        bne     @2

        add     r1, r1, #1
        cmp     r1, #2 // check 2 loop
        bne     @1
@2:
        strh    r0, [r2]
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         TwlSpStartUp

  Description:  hook for user start up

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void TwlSpStartUp(void)
{
}
