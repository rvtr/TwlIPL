/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     crt1.c

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

#include        <twl/code32.h>
#include        <twl.h>
#include		<sysmenu/sysmenu_work.h>							// SYSM changed.

#define			SDK_NOCOMPRESS										// SYSM changed.


extern  void            NitroMain( void );
extern  void            OS_IrqHandler( void );
static  void            do_autoload( void );
static  void            init_cp15( void );
void                    _start( void );
extern  void* const     _start_ModuleParams[];
void                    _start_AutoloadDoneCallback( void* argv[] );

extern  void            __call_static_initializers( void );
extern  void            _fp_init( void );

// from LCF
extern  u32     SDK_IRQ_STACKSIZE[];
extern  void    SDK_AUTOLOAD_START( void );    // autoload data will start from here
extern  void    SDK_AUTOLOAD_LIST( void );     // start pointer to autoload information
extern  void    SDK_AUTOLOAD_LIST_END( void ); // end pointer to autoload information
extern  void    SDK_STATIC_BSS_START( void );  // static bss start address
extern  void    SDK_STATIC_BSS_END( void );    // static bss end address


/*---------------------------------------------------------------------------*
  Name:         _start

  Description:  Start up

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define INITi_HW_DTCM   SDK_AUTOLOAD_DTCM_START

__declspec ( weak ) asm void _start( void )
{
        //---- set IME = 0
        //     ( use that LSB of HW_REG_BASE equal to 0 )
        mov             r12, #HW_REG_BASE
        str             r12, [r12, #REG_IME_OFFSET]

        //---- initialize cp15
        bl              init_cp15

        //---- initialize stack pointer
        // SVC mode
        mov             r0, #HW_PSR_SVC_MODE
        msr             cpsr_c, r0
        ldr             r0, =INITi_HW_DTCM
        add             r0, r0, #0x3fc0
        mov             sp, r0

        // IRQ mode
        mov             r0, #HW_PSR_IRQ_MODE
        msr             cpsr_c, r0
        ldr             r0, =INITi_HW_DTCM
        add             r0, r0, #0x3fc0
        sub             r0, r0, #HW_SVC_STACK_SIZE
        mov             sp, r0

        // System mode
        ldr             r1, =SDK_IRQ_STACKSIZE
        sub             r1, r0, r1
        mov             r0, #HW_PSR_SYS_MODE
        msr             cpsr_csfx, r0
        sub             sp, r1, #4 // 4byte for stack check code

        //---- load autoload block and initialize bss
#ifndef SDK_NOCOMPRESS
        ldr             r1, =_start_ModuleParams
        ldr             r0, [r1, #20]   // r0 = bottom of compressed data
        bl              MIi_UncompressBackward
#endif
        bl              do_autoload
        
        //---- clear memory
        // DTCM (16KB)
        mov             r0, #0
        ldr             r1, =INITi_HW_DTCM
        mov             r2, #HW_DTCM_SIZE
        bl              MIi_CpuClear32

/*        // BG/OBJ palette (1KB)									// SYSM changed.
        mov             r0, #0
        ldr             r1, =HW_PLTT
        mov             r2, #HW_PLTT_SIZE
        bl              MIi_CpuClear32

        // OAM (1KB)
        mov             r0, #0x0200
        ldr             r1, =HW_OAM
        mov             r2, #HW_OAM_SIZE
        bl              MIi_CpuClear32
*/
        //---- set interrupt vector
        ldr             r1, =INITi_HW_DTCM
        add             r1, r1, #0x3fc0
        add             r1, r1, #HW_DTCM_SYSRV_OFS_INTR_VECTOR
        ldr             r0, =OS_IrqHandler
        str             r0, [r1, #0]

#ifndef SDK_NOINIT
        //---- for C++
        bl             _fp_init
        bl             NitroStartUp
        bl             __call_static_initializers
#endif
        //---- start (to 16bit code)
        ldr             r1, =NitroMain
//        ldr             lr, =HW_RESET_VECTOR
		ldr				lr, =RETURN_FROM_MAIN_ARM9_FUNCP			// SYSM changed.

        bx              r1
}

/*---------------------------------------------------------------------------*
  Name:         _start_ModuleParams

  Description:  autoload/compress/arguments data block

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void* const _start_ModuleParams[] =
{
    (void*)SDK_AUTOLOAD_LIST,
    (void*)SDK_AUTOLOAD_LIST_END,
    (void*)SDK_AUTOLOAD_START,
    (void*)SDK_STATIC_BSS_START,
    (void*)SDK_STATIC_BSS_END,
#ifndef  SDK_NOCOMPRESS
    (void*)0,           // CompressedStaticEnd
#endif
};

/*---------------------------------------------------------------------------*
  Name:         MIi_UncompressBackward

  Description:  Uncompress special archive for module compression

  Arguments:    bottom      = Bottom adrs of packed archive + 1
                bottom[-12] = offset for top    of compressed data
                                 inp_top = bottom + bottom[-12]
                bottom[ -8] = offset for bottom of compressed data
                                 inp     = bottom + bottom[ -8]
                bottom[ -4] = offset for bottom of original data
                                 outp    = bottom + bottom[ -4]
  
                typedef struct
                {
                    int         bufferTop;
                    int         compressBottom;
                    int         originalBottom;
                }   CompFooter;

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void  MIi_UncompressBackward( register void* bottom )
{
#define data            r0
#define inp_top         r1
#define outp            r2
#define inp             r3
#define outp_save       r4
#define flag            r5
#define count8          r6
#define index           r7
#define len             r12
                cmp     bottom, #0
                beq     @exit
                stmfd   sp!,    {r4-r7}
                ldmdb   bottom, {r1-r2}
                add     outp,    bottom,  outp
                sub     inp,     bottom,  inp_top, LSR #24
                bic     inp_top, inp_top, #0xff000000
                sub     inp_top, bottom,  inp_top
                mov     outp_save, outp
@loop:
                cmp     inp, inp_top            // exit if inp==inp_top
                ble     @end_loop
                ldrb    flag, [inp, #-1]!       // r4 = compress_flag = *--inp
                mov     count8, #8
@loop8:
                subs    count8, count8, #1
                blt     @loop
                tst     flag, #0x80
                bne     @blockcopy
@bytecopy:
                ldrb    data, [inp, #-1]!
#ifdef  SDK_TEG
                sub     outp, outp, #1
                swpb    data, data, [outp]
#else
                strb    data, [outp, #-1]!      // Copy 1 byte
#endif
                b       @joinhere
@blockcopy:
                ldrb    len,   [inp, #-1]!
                ldrb    index, [inp, #-1]!
                orr     index, index, len, LSL #8
                bic     index, index, #0xf000
                add     index, index, #0x0002
                add     len,   len,   #0x0020
@patterncopy:
                ldrb    data,  [outp, index]
#ifdef  SDK_TEG
                sub     outp, outp, #1
                swpb    data, data, [outp]
#else
                strb    data,  [outp, #-1]!
#endif
                subs    len,   len,   #0x0010
                bge     @patterncopy

@joinhere:
                cmp     inp, inp_top
                mov     flag, flag, LSL #1
                bgt     @loop8
@end_loop:
    
                // DC_FlushRange & IC_InvalidateRange
                bic     inp,  inp_top, #HW_CACHE_LINE_SIZE - 1
@cacheflush:
                mcr     p15, 0, inp, c7,  c5, 1         // ICache
                mcr     p15, 0, inp, c7, c14, 1         // DCache
                add     inp, inp, #HW_CACHE_LINE_SIZE
                cmp     inp, outp_save
                blt     @cacheflush
                
                ldmfd   sp!, {r4-r7}
@exit           bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         do_autoload

  Description:  put autoload data block according to autoload information,
                and clear static bss by filling with 0.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
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

@2:
        cmp     infop, infop_end                // reach to end?
        beq     @fill_bss

        ldr     dest,      [infop], #4          // dest
        ldr     dest_size, [infop], #4          // size
        add     dest_end, dest, dest_size       // dest_end
@1:
        cmp     dest, dest_end
        ldrmi   tmp, [src],  #4                 // [dest++] <- [src++]
        strmi   tmp, [dest], #4
        bmi     @1

        //---- fill bss with 0
        ldr     dest_size, [infop], #4          // size
        add     dest_end, dest, dest_size       // bss end
        mov     tmp, #0
@4:
        cmp     dest, dest_end
        strcc   tmp, [dest], #4
        bcc     @4

        beq     @2

        //---- fill static static bss with 0
@fill_bss:
        ldr     dest,      [ptable, #12]   // BSS segment start
        ldr     dest_end,  [ptable, #16]   // BSS segment end
        mov     tmp, #0
@3:
        cmp     dest, dest_end
        strcc   tmp, [dest], #4
        bcc     @3

        // r0 = _start_ModuleParams
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
__declspec ( weak ) asm void _start_AutoloadDoneCallback( void* argv[] )
{
        bx      lr
}

//-----------------------------------------------------------------------
//                   システム制御コプロセッサ 初期化
//-----------------------------------------------------------------------
static asm void init_cp15(void)
{
        // プロテクションユニット/キャッシュ/TCM ディセーブル

        mrc     p15, 0, r0, c1, c0, 0
        ldr     r1, =HW_C1_ICACHE_ENABLE  | HW_C1_DCACHE_ENABLE  \
                   | HW_C1_ITCM_ENABLE    | HW_C1_DTCM_ENABLE    \
                   | HW_C1_ITCM_LOAD_MODE | HW_C1_DTCM_LOAD_MODE \
                   | HW_C1_LD_INTERWORK_DISABLE                  \
                   | HW_C1_PROTECT_UNIT_ENABLE
        bic     r0, r0, r1
        mcr     p15, 0, r0, c1, c0, 0

        // キャッシュ無効化
        mov     r0, #0
        mcr     p15, 0, r0, c7, c5, 0       // 命令キャッシュ
        mcr     p15, 0, r0, c7, c6, 0       // データキャッシュ

        // ライトバッファ エンプティ待ち
        mcr     p15, 0, r0, c7, c10, 4

/*
; Region G:    BACK_GROUND: Base = 0x0,        Size = 4GB,   I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 0:    IO_VRAM:     Base = 0x04000000, Size = 64MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 1Rel: MAIN_MEM:    Base = 0x02000000, Size = 4MB,   I:Cach Buf / D:Cach Buf,  I:RW / D:RW
; Region 1Dbg: MAIN_MEM:    Base = 0x02000000, Size = 8MB,   I:Cach Buf / D:Cach Buf,  I:RW / D:RW
; Region 2Rel: SOUND_DATA:  Base = 0x02380000, Size = 512KB, I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 2D4M: SOUND_DATA:  Base = 0x02300000, Size = 1MB,   I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 2D8M: SOUND_DATA:  Base = 0x02600000, Size = 2MB,   I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 3:    CARTRIDGE:   Base = 0x08000000, Size = 128MB, I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 4:    DTCM:        Base = SOUND_DATA, Size = 16KB,  I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 5:    ITCM:        Base = 0x01000000, Size = 16MB,  I:NC NB    / D:NC NB,     I:RW / D:RW

; Region 6:    BIOS:        Base = 0xffff0000, Size = 32KB,  I:Cach NB  / D:Cach NB,   I:RO / D:RO
; Region 7:    SHARE_WORK:  Base = 0x027ff000, Size = 4KB,   I:NC NB    / D:NC NB,     I:NA / D:RW
;(Region 7:    DBG_RESERVE: Base = 0x02700000, Size = 1MB,   I:NC NB    / D:NC NB,     I:RW / D:RW)
*/
#define SET_PROTECTION_A( id, adr, siz )        ldr r0, =(adr|HW_C6_PR_##siz|HW_C6_PR_ENABLE)
#define SET_PROTECTION_B( id, adr, siz )        mcr     p15, 0, r0, c6, id, 0
#define REGION_BIT(a,b,c,d,e,f,g,h)     (((a)<<0)|((b)<<1)|((c)<<2)|((d)<<3)|((e)<<4)|((f)<<5)|((g)<<6)|((h)<<7))
#define REGION_ACC(a,b,c,d,e,f,g,h)     (((a)<<0)|((b)<<4)|((c)<<8)|((d)<<12)|((e)<<16)|((f)<<20)|((g)<<24)|((h)<<28))
#define NA      0
#define RW      1
#define RO      5


        //
        // メモリリージョン初期化
        //
        //---- I/O レジスタ & VRAM 等
        SET_PROTECTION_A( c0, HW_IOREG, 64MB )
        SET_PROTECTION_B( c0, HW_IOREG, 64MB )

        //---- メインメモリ
        SET_PROTECTION_A( c1, HW_MAIN_MEM_MAIN, 4MB )
        SET_PROTECTION_B( c1, HW_MAIN_MEM_MAIN, 4MB )

/*        //---- サウンドデータ領域									// SYSM changed.
#if     HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x1000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 4KB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 4KB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x2000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 8KB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 8KB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x4000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 16KB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 16KB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x8000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 32KB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 32KB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x10000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 64KB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 64KB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x20000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 128KB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 128KB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x40000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 256KB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 256KB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x80000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 512KB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 512KB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x100000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 1MB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 1MB )
#elif   HW_MAIN_MEM_SUB_SIZE+HW_MAIN_MEM_SHARED_SIZE == 0x200000
        SET_PROTECTION_A( c2, HW_MAIN_MEM_SUB, 2MB )
        SET_PROTECTION_B( c2, HW_MAIN_MEM_SUB, 2MB )
#else
#pragma message(ERROR: Size unmatch HW_MAIN_MEM_SUB_SIZE)
#endif
*/
        //---- カートリッジ又は他の用途
        //      CPU 内部ワーク RAM 等
        SET_PROTECTION_A( c3, HW_CTRDG_ROM, 128MB )
        SET_PROTECTION_B( c3, HW_CTRDG_ROM, 128MB )

        //---- データ TCM
        //      + CPU 内部ワーク RAM の場合あり
//#if   (HW_DTCM & 0x3FFF) != 0
//#pragma message(ERROR: HW_DTCM need to be aligned 16KB!)
//#endif

//        SET_PROTECTION_A( c4, HW_DTCM, 16KB )
        ldr     r0, =SDK_AUTOLOAD_DTCM_START
        orr     r0, r0, #HW_C6_PR_16KB
        orr     r0, r0, #HW_C6_PR_ENABLE
        SET_PROTECTION_B( c4, HW_DTCM, 16KB )

        //---- 命令 TCM
        //      データ TCM より優先が高い、メインメモリ領域までのイメージ
        SET_PROTECTION_A( c5, HW_ITCM_IMAGE, 16MB )
        SET_PROTECTION_B( c5, HW_ITCM_IMAGE, 16MB )

        //---- BIOS
        SET_PROTECTION_A( c6, HW_BIOS, 32KB )
        SET_PROTECTION_B( c6, HW_BIOS, 32KB )

        //---- SHARED  CPU 間通信ワーク領域
        SET_PROTECTION_A( c7, HW_MAIN_MEM_SHARED, 4KB )
        SET_PROTECTION_B( c7, HW_MAIN_MEM_SHARED, 4KB )
            
#if     HW_MAIN_MEM_SHARED_SIZE != 0x1000
#pragma message(ERROR: Size unmatch HW_MAIN_MEM_SHARED_SIZE)
#endif
       
        //
        // 命令ＴＣＭ 設定
        //
        mov     r0, #HW_C9_TCMR_32MB
        mcr     p15, 0, r0, c9, c1, 1

        //
        // データＴＣＭ 設定
        //
        ldr     r0, =INITi_HW_DTCM
        orr     r0, r0, #HW_C9_TCMR_16KB
        mcr     p15, 0, r0, c9, c1, 0

        //
        // 命令キャッシュ イネーブル (リージョン設定)
        //      1: MAIN_MEM
        //      6: BIOS
        //
        mov     r0, #REGION_BIT(0,1,0,0,0,0,1,0)
        mcr     p15, 0, r0, c2, c0, 1

        //
        // データキャッシュ イネーブル (リージョン設定)
        //      1: MAIN_MEM
        //      6: BIOS
        //
        mov     r0, #REGION_BIT(0,1,0,0,0,0,1,0)
        mcr     p15, 0, r0, c2, c0, 0

        //
        // ライトバッファ イネーブル(リージョン設定)
        //      1: MAIN_MEM
        //
        mov     r0, #REGION_BIT(0,1,0,0,0,0,0,0)
        mcr     p15, 0, r0, c3, c0, 0

        //
        // 命令アクセス許可 (リージョン設定)
        //  IO_VRAM       : RW
        //  MAIN_MEM_MAIN : RW
        //  MAIN_MEM_SUB  : NA
        //  CTRDG         : NA
        //  DTCM          : NA
        //  ITCM          : RW
        //  BIOS          : RO
        //  SHARED        : NA
        //
        ldr     r0, =REGION_ACC(RW,RW,NA,NA,NA,RW,RO,NA)
        mcr     p15, 0, r0, c5, c0, 3

        //
        // データアクセス許可（リージョン設定）
        //  IO_VRAM       : RW
        //  MAIN_MEM_MAIN : RW
        //  MAIN_MEM_SUB  : NA
        //  CTRDG         : RW
        //  DTCM          : RW
        //  ITCM          : RW
        //  BIOS          : RO
        //  SHARED        : RW
        //
        ldr     r0, =REGION_ACC(RW,RW,NA,RW,RW,RW,RO,RW)
        mcr     p15, 0, r0, c5, c0, 2 

        //
        // システム制御コプロセッサ マスター設定
        //
        mrc     p15, 0, r0, c1, c0, 0
        ldr     r1,=HW_C1_ICACHE_ENABLE | HW_C1_DCACHE_ENABLE | HW_C1_CACHE_ROUND_ROBIN \
                  | HW_C1_ITCM_ENABLE   | HW_C1_DTCM_ENABLE                             \
                  | HW_C1_SB1_BITSET    | HW_C1_EXCEPT_VEC_UPPER                        \
                  | HW_C1_PROTECT_UNIT_ENABLE
        orr     r0, r0, r1
        mcr     p15, 0, r0, c1, c0, 0

        bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         NitroStartUp

  Description:  hook for user start up

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
__declspec ( weak ) void NitroStartUp( void )
{
}
