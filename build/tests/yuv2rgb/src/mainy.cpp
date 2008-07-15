/*---------------------------------------------------------------------------*
  Project:  TwlSDK - YUV2RGB
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/camera.h>
#include <twl/dsp.h>
#include <twl/dsp/ARM9/dsp_jpeg_enc.h>
#include <twl/dsp/ARM9/dsp_jpeg_dec.h>
#include <twl/dsp/ARM9/dsp_util.h>
//#include <twl/dsp/common/shutter.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/systemWork.h>
#include <twl/mi.h>
#include <nitro/std.h>
#include <twl/lcfg.h>
#include <sysmenu.h>
#include <sysmenu/hotsw.h>
// #include <twl/dsp.h>
// #include <twl/dsp/common/pipe.h>
#include "DEMOBitmap.h"
#include "yuv2rgb.h"
#include "menuIrqHandler.h"
#include "menuCameraManager.h"

#include "snd_data.h"

// テストグローバルアクセサ
cmn::sys::cmr2::Manager* mpCmrMgr2;

//extern u16 b_data[];

extern const u8 _binary_camera_shutter_dat[], _binary_camera_shutter_dat_end[];
extern const u8 _binary_camera_shutter_MONO_dat[], _binary_camera_shutter_MONO_dat_end[];
extern const u8 _binary_wheel_dat[], _binary_wheel_dat_end[];


#include <twl/dtcm_begin.h>
// 実験に必要なu32個数はmax(208*3, 256*2)
// とりあえず256*4で。
static cmn::sys::cmr2::dtcmWork sDtcmWork;
#include <twl/dtcm_end.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define CAMERA_NEW_DMA_NO      1    // CAMERAで使用するNew DMA番号
#define WIDTH 640
#define HEIGHT 480

#define LINES_SPLIT     1
#define LINES_AT_ONCE   (CAMERA_GET_MAX_LINES(WIDTH)/LINES_SPLIT)     // 一回の転送ライン数
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // 一ラインの転送バイト数

  enum
  {
    eStt_Deactive = 0 ,
    eStt_Capture ,
    eStt_WaitShooting ,
    eStt_Shoot ,
    eStt_FinishShooting ,
    eStt_Stop ,
    eStt_Max
  } mState, mLastState;

BOOL mError;

    enum
    {
#if 1
        cPictureOffsetX = 0,
        cPictureOffsetY = 0,
#else
        cPictureOffsetX = 24,
        cPictureOffsetY = 24,
#endif
        cPictureWidth   = cScreenWidth  - (cPictureOffsetX * 2),
        cPictureHeight  = cScreenHeight - (cPictureOffsetY * 2),
        cShootWidth     = 640,
        cShootHeight    = 480
    };

/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/

static void VBlankIntr(void);
static void CameraIntr(void);
//static void CameraIntr2(void);
//static void CameraIntr3(void);
static void NDmaIntr(void);
static void NDmaDummyIntr(void);
static void PutString( char *format, ... );
static void DrawFPS(void);

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/

static BOOL startRequest = FALSE;

// [0],[1]はcameraからのinput。[2]は捨てられるcamera input用？
static u16 buffer[3][WIDTH*HEIGHT] ATTRIBUTE_ALIGN(32);
//static vu8 we;
//static vu8 re;
//static vu8 reading;
//static vu8 tp;
//static vu8 wp;                  // カメラからデータを取り込み中のバッファ
//static vu8 rp;                  // 最後のVRAMにコピーしたバッファ
//static BOOL wp_pending;         // 取り込みを中断した (再び同じバッファに取り込む)
static CAMERASelect current = CAMERA_SELECT_IN;
static MenuIrqHandlerLogBuffer logBuf[0x10000];

static u16 text_buffer[ 32 ] ATTRIBUTE_ALIGN(32);

static vu8 sNowConvert = FALSE; // YUV->RGB変換中フラグ

static vu8 sLongDmaFlag;

static u32 gWords[0x2000];
static u32 gSum;

#define GRAD_WIDTH 256
static u16 grad0[GRAD_WIDTH] ATTRIBUTE_ALIGN(32);
static u16 grad1[GRAD_WIDTH] ATTRIBUTE_ALIGN(32);
static void MakeGrad(void)
{
    static u16 table[] = {
         0, 8, 0, 1, 0, 2, 0, 3,
         0, 4, 0, 5, 0, 6, 0, 7,
         1, 8, 1, 9, 2,10, 3,11,
         4,12, 5,13, 6,14, 7,15,
         8,16, 9,17,10,18,11,19,
        12,20,13,21,14,22,15,23,
        16,24,17,25,18,26,19,27,
        20,28,21,29,22,30,23,31,
    };
    u32 i;
    u32 color;
    u32 shift = GX_RGBA_G_SHIFT;

    for (i = 0; i < GRAD_WIDTH; i++) {
        color = i >> 2; // 0-63
        grad0[i] = (u16)((table[color] << shift) | GX_RGBA_A_MASK);
        grad1[i] = (u16)(((color >> 1) << shift) | GX_RGBA_A_MASK);
    }

    DC_StoreAll();
}

static inline int r_diff( u8 v )
{
//    return (1402 * ( v - 128 )) / 1000;
    return (11760828/*.416*/ * ( v - 128)) >> 23;
}

static inline int g_diff( u8 u, u8 v )
{
//    return (-344 * ( u - 128 ) -714 * ( v - 128 )) / 1000;
    return (-2885681/*.152*/ * ( u  - 128 ) -5989466/*.112*/ * ( v - 128 )) >> 23;
}
static inline int b_diff( u8 u )
{
//    return (1772 * ( u - 128 )) / 1000;
    return (14864613/*.376*/ * ( u - 128 )) >> 23;
}

static inline int trim( int e )
{
    return e < 0 ? 0 : ( e > 255 ? 255 : e);
}
#if 0
static u32 CpuYuv2Rgb666( const void* src, void* dest, u32 pixels )
{
    return CpuYuv2Rgb(src, dest, pixels);
}
#elif 0
static u32 CpuYuv2Rgb666( const void* src, void* dest, u32 pixels )
{
    MakeGrad();
    return 0;
}
#elif 0
// little endian only
static asm u32 CpuYuv2Rgb666( register const void* src, register void* dst, register u32 pixels )
{
    // x * 6/7
    ldr     r0, =0x000003fd
    ldr     r1, =(6*0x10000)/7
    smulwb  r0, r1, r0
//    mov     r0, #0x16c << 16
    bx      lr
}
#elif 0
// little endian only
static asm u32 CpuYuv2Rgb666( register const void* src, register void* dst, register u32 pixels )
{
    // カメラ起動中、AHBはどれくらいストールする？


// 0009 0000 0000 0000 0000 0000 0000 0000 ret=00000001
// 0002 0000 0000 0000 0000 0000 0000 0000 ret=00000001
// 0003 0000 0000 0000 0000 0000 0000 0000 ret=00000001
    // 9とか酷いな。sysclk/64なので...cpuclkにすると4倍、
    // タイマーがカウントアップ直前だったとしても
    // つまり8*64*4=2048 cpu cyclesの間、ストールしているってこと？
    // これは何が原因？
    // カメラのラインバッファって16MHzで動いてるの？
    // もしそうだとすると、256ドット=64 Words、このDMAには
    // sysclkにして8倍、64*8=512 cpu cycles...そんなに早くない？
    //
    // あーっと！！CAMERA_GetBytesAtOnce(WIDTH)=2048だ！
    // つまり 512 Wordsなので512*8 = 4096 cycles...そんな計算にはならんな。
    // カメラのラインバッファがsysclk動作ならcpuclkにすると4倍、
    // 512*4=2048 cpu cycles...つじつま合う？
    // バイト数と同じcpu cyclesになるとすると、2048はアカンやろ(笑)
    // 割り込みルーチンの中でもそんなにストールするリスクがある。
    // サウンドDMAだってあるのに。
    // まあでも、他の要因によってストールする可能性があるから
    // カメラのラインバッファを4ライン分としているのかな。
    // こまったなあ。うーん、カメラのラインバッファからメインメモリ
    // へ転送時の、DMA終了割り込みを使わなければ、DMAが追いつかなくて
    // 破綻するとかは、あまり心配しなくて良さそう。
    //
    // 000e 0000 0000 0000 0000 0000 0000 0000 ret=00000001
    // なんじゃこりゃ！？割り込み入った？
    //
    // mi_ndma.cに
    // #define CAMERA_DMA_BLOCK_SIZE      MI_NDMA_BWORD_16
    // #define CAMERA_DMA_INTERVAL        2
    // #define CAMERA_DMA_PRESCALER       MI_NDMA_INTERVAL_PS_1
    // とあるので、16ワード単位の分割転送になっているのかな。
    // うん、なってる。プリスケーラ=1倍(sysclk)、インターバル=2
    // 特に悪い設定とは思えないが、ラインバッファがオーバーフローするのかな…
    // しかもストールが長いのは、カメラNDMAのせいでもなさそうだし。



// keep stack frame 8-Bytes aligned
#define SAVE_REGS r3,r4-r11,lr

#define t0 r6
#define t1 r7
#define tm0l_addr r11
#define vcount_addr lr

#define tm0    r4
#define vcount r5

#define COUNT_MAX 8

    // keep 8-Bytes aligned
    stmfd   sp!, {SAVE_REGS}

    ldr     tm0l_addr, =REG_TM0CNT_L_ADDR
    ldr     vcount_addr, =REG_VCOUNT_ADDR
    mov     pixels, #COUNT_MAX // 最高計測個数

// VCOUNT値は0xc4とか0x60とかなので、繰り上がりは非対応。
    ldrh    vcount, [vcount_addr]
    add     vcount, vcount, #0x30
    mov     vcount, #192
@10:
    ldrsh   tm0, [tm0l_addr]
@20:
    ldrsh   t0, [tm0l_addr]
    sub     t1, t0, tm0
    movs    t0, t1, lsr #1
    beq     @50

    strh    t1, [dst], #2
    subs    pixels, pixels, #1
    beq     @150

@50:
    ldrh    t0, [vcount_addr]
    cmp     t0, vcount
    blt     @10
    b       @100

@100:
    mov     t1, pixels
    mov     t0, #0
@110:
    strh    t0, [dst], #2
    subs    t1, t1, #1
    bne     @110

@150:
    rsb     r0, pixels, #COUNT_MAX

    // keep 8-Bytes aligned
    ldmfd   sp!, {SAVE_REGS}
    bx      lr
#undef SAVE_REGS
#undef t0
#undef t1
#undef tm0l_addr
#undef vcount_addr
#undef tm0
#undef vcount
#undef COUNT_MAX
}
#elif 1
// dstダミーフェッチ無し
// dstはMAINではない(VRAM)と想定しているので、DTCM非使用。
// 666じゃなくて単なる555。
// little endian only
static asm u32 CpuYuv2Rgb666( register const void* src, register void* dst, register u32 pixels )
{

// keep stack frame 8-Bytes aligned
#define SAVE_REGS r3,r4-r11,lr
#define t0   r3
#define t1   r12
#define y0   r9
#define y1   r10
#define uv   r11
#define rgba lr

#define CONST_REGS r4-r8
#define const_0x80008000      r4
#define const_0xff00ff00      r5
#define const_0x7f800000      r6
#define const_H1_402_L1_772   r7
#define const_Hm0_344_Lm0_714 r8

    // omit odd and zero
    eor     t0, pixels, #1
    movs    t0, t0, lsr #1
    bxls    lr // LS: C clear or Z set

    // keep 8-Bytes aligned
    stmfd   sp!, {SAVE_REGS}

    ldr     t0, =@constants
    ldmia   t0, {CONST_REGS}

    ldr     t0, [src], #4
@10:
    and     uv, t0, const_0xff00ff00
    eor     uv, uv, const_0x80008000
    and     y0, const_0x7f800000, t0, lsl #23    // 0x000000ff -> 0x7f800000
    and     y1, const_0x7f800000, t0, lsl #23-16 // 0x00ff0000 -> 0x7f800000

    // Green
    // -0.344*(U-128)
    smultb  t0, const_Hm0_344_Lm0_714, uv
    // -0.714*(V-128)
    smulbt  t1, const_Hm0_344_Lm0_714, uv
    mov     rgba, const_0x80008000
    add     t1, t0, t1
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #21 // 0x7c000000 -> 0x000003e0
    ands    t0, t1, #0xfc000000

    // Red
    // 1.402*(V-128)
    smultt  t1, const_H1_402_L1_772, uv
    orrpl   rgba, rgba, t0, lsr #21-16 // 0x7c000000 -> 0x03e00000
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #26 // 0x7c000000 -> 0x0000001f
    ands    t0, t1, #0xfc000000

    // Blue
    // 1.772*(U-128)
    smulbb  t1, const_H1_402_L1_772, uv
    orrpl   rgba, rgba, t0, lsr #26-16 // 0x7c000000 -> 0x001f0000
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #16 // 0x7c000000 -> 0x00007c00
    ands    t0, t1, #0xfc000000
    orrpl   rgba, rgba, t0 // 0x7c000000 

    str     rgba, [dst], #4
    subs    pixels, pixels, #2
    ldrne   t0, [src], #4
    bne     @10

    // keep 8-Bytes aligned
    ldmfd   sp!, {SAVE_REGS}
    bx      lr

@constants:
    dcd     0x80008000
    dcd     0xff00ff00
    dcd     0x7f800000
    dcd     0x59ba7168 // ((1.402 * 0x4000) << 16) | ((1.772 * 0x4000) & 0xffff)
    dcd     0xe9fcd24e // ((-0.344 * 0x4000) << 16) | ((-0.714 * 0x4000) & 0xffff)

#undef SAVE_REGS
#undef t0
#undef t1
#undef y0
#undef y1
#undef uv
#undef rgba
#undef CONST_REGS
#undef const_0x80008000
#undef const_0xff00ff00
#undef const_0x7f800000
#undef const_H1_402_L1_772
#undef const_Hm0_344_Lm0_714
}

#elif 0
// dstダミーフェッチあり…めちゃ遅くなる。残念。
// little endian only
static asm u32 CpuYuv2Rgb666( register const void* src, register void* dst, register u32 pixels )
{

// keep stack frame 8-Bytes aligned
#define SAVE_REGS r3,r4-r11,lr
#define t0   r3
#define t1   r12
#define y0   r9
#define y1   r10
#define uv   r11
#define rgba lr

#define CONST_REGS r4-r8
#define const_0x80008000      r4
#define const_0xff00ff00      r5
#define const_0x7f800000      r6
#define const_H1_402_L1_772   r7
#define const_Hm0_344_Lm0_714 r8

    // omit odd and zero
    eor     t0, pixels, #1
    movs    t0, t0, lsr #1
    bxls    lr // LS: C clear or Z set

    // keep 8-Bytes aligned
    stmfd   sp!, {SAVE_REGS}

    ldr     t0, =@constants
    ldmia   t0, {CONST_REGS}

    ldr     t0, [src], #4
@10:
    and     uv, t0, const_0xff00ff00
    eor     uv, uv, const_0x80008000
    and     y0, const_0x7f800000, t0, lsl #23    // 0x000000ff -> 0x7f800000
    and     y1, const_0x7f800000, t0, lsl #23-16 // 0x00ff0000 -> 0x7f800000

    // dst dummy-fetch
    ldr     rgba, [dst]

    // Green
    // -0.344*(U-128)
    smultb  t0, const_Hm0_344_Lm0_714, uv
    // -0.714*(V-128)
    smulbt  t1, const_Hm0_344_Lm0_714, uv
    mov     rgba, const_0x80008000
    add     t1, t0, t1
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #21 // 0x7c000000 -> 0x000003e0
    ands    t0, t1, #0xfc000000

    // Red
    // 1.402*(V-128)
    smultt  t1, const_H1_402_L1_772, uv
    orrpl   rgba, rgba, t0, lsr #21-16 // 0x7c000000 -> 0x03e00000
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #26 // 0x7c000000 -> 0x0000001f
    ands    t0, t1, #0xfc000000

    // Blue
    // 1.772*(U-128)
    smulbb  t1, const_H1_402_L1_772, uv
    orrpl   rgba, rgba, t0, lsr #26-16 // 0x7c000000 -> 0x001f0000
    // +Y
    qdadd   t0, y0, t1
    qdadd   t1, y1, t1
    ands    t0, t0, #0xfc000000
    orrpl   rgba, rgba, t0, lsr #16 // 0x7c000000 -> 0x00007c00
    ands    t0, t1, #0xfc000000
    orrpl   rgba, rgba, t0 // 0x7c000000 

    str     rgba, [dst], #4
    subs    pixels, pixels, #2
    ldrne   t0, [src], #4
    bne     @10

    // keep 8-Bytes aligned
    ldmfd   sp!, {SAVE_REGS}
    bx      lr

@constants:
    dcd 0x80008000
    dcd 0xff00ff00
    dcd 0x7f800000
    dcd 0x59ba7168 // ((1.402 * 0x4000) << 16) | ((1.772 * 0x4000) & 0xffff)
    dcd 0xe9fcd24e // ((-0.344 * 0x4000) << 16) | ((-0.714 * 0x4000) & 0xffff)

#undef SAVE_REGS
#undef t0
#undef t1
#undef y0
#undef y1
#undef uv
#undef rgba
#undef CONST_REGS
#undef const_0x80008000
#undef const_0xff00ff00
#undef const_0x7f800000
#undef const_H1_402_L1_772
#undef const_Hm0_344_Lm0_714
}

#elif 0
// little endian only
// 最小値、最大値を調べる。
static asm u32 CpuYuv2Rgb666( register const void* src, register void* dst, register u32 pixels )
{
#define SAVE_REGS0 src,dst,pixels
#define SAVE_REGS1 r4-r11,lr
#define t0   r3
#define minVal r4
#define maxVal r5

    // keep 8-Bytes aligned
    stmfd   sp!, {SAVE_REGS0, SAVE_REGS1}

    mvn     minVal, #0x80000000
    mov     maxVal, #0x80000000

@10:
    ldrb    t0, [src], #4
    ldrb    t0, [src, #1-4]
    cmp     minVal, t0
    movgt   minVal, t0
    cmp     maxVal, t0
    movlt   maxVal, t0
    
#if 0
    cmp     minVal, t0
    movgt   minVal, t0
    cmp     maxVal, t0
    movlt   maxVal, t0
    ldrb    t0, [src, #2-4]
    cmp     minVal, t0
    movgt   minVal, t0
    cmp     maxVal, t0
    movlt   maxVal, t0
#endif

    subs    pixels, pixels, #2
    bne     @10

    mov     minVal, minVal, lsl #16
    mov     maxVal, maxVal, lsl #16
    orr     t0, maxVal, minVal, lsr #16

    // keep 8-Bytes aligned
    ldmfd   sp!, {SAVE_REGS0, SAVE_REGS1}
    mov     r0, t0
    bx      lr

#undef SAVE_REGS0
#undef SAVE_REGS1
#undef t0
#undef minVal
#undef maxVal
}

#else
// little endian only
// ベタに作ったらこうなる。Cと変わらん。
static asm u32 CpuYuv2Rgb666( register const void* src, register void* dst, register u32 pixels )
{
// registers
// keep stack frame 8-Bytes aligned
#define SAVE_REGS0 src,dst,pixels
#define SAVE_REGS1 r4-r11,lr
#define t0   r3
#define rgba r12
#define u    src
#define v    dst
#define y0   r11
#define y1   lr

#define tablep  r9
#define data0   r10
#define data1   pixels

#define CONST_REGS r4-r8
#define const_0x80008000      r4
#define const_minus_0_344     r5
#define const_minus_0_714     r6
#define const_1_402           r7
#define const_1_772           r8

    cmp     pixels, #2
    bxlt    lr

    // keep 8-Bytes aligned
    stmfd   sp!, {SAVE_REGS0, SAVE_REGS1}

    ldr     t0, =@constants
    ldmia   t0, {CONST_REGS}

    ldr     tablep, =@table

@10:
    ldrb    y0, [src], #4
    ldrb    t0, [src, #1-4]
    ldrb    y1, [src, #2-4]
    ldrb    rgba,  [src, #3-4]
    // do not update sp here! keep 8-Bytes aligned
    stmia   sp, {SAVE_REGS0}
    sub     u, t0, #0x80
    sub     v, rgba, #0x80

    // Green
    // -0.344*(U-128)
    mul     t0, const_minus_0_344, u
    // -0.714*(V-128)
    mul     rgba, const_minus_0_714, v
    mov     t0, t0, asr #23
    add     t0, t0, rgba, asr #23
    // +Y
    adds    rgba, t0, y0
    movmi   rgba, #0
    cmp     rgba, #0xff
    movgt   rgba, #0xff
    ldrb    data1, [tablep, rgba, lsr #2]
    and     rgba, rgba, #0xf8
    mov     rgba, rgba, lsl #2
    adds    t0, t0, y1
    movmi   t0, #0
    cmp     t0, #0xff
    movgt   t0, #0xff
    ldrb    data0, [tablep, t0, lsr #2]
    and     t0, t0, #0xf8
    orr     rgba, rgba, t0, lsl #16+2
    mov     data1, data1, lsl #5
    orr     data0, data1, data0, lsl #16+5

    // Red
    // 1.402*(V-128)
    mul     t0, const_1_402, v
    mov     t0, t0, asr #23
    // +Y
    // now v is free
    adds    v, t0, y0
    movmi   v, #0
    cmp     v, #0xff
    movgt   v, #0xff
    ldrb    data1, [tablep, v, lsr #2]
    orr     rgba, rgba, v, lsr #3
    adds    t0, t0, y1
    movmi   t0, #0
    cmp     t0, #0xff
    movgt   t0, #0xff
    ldrb    v, [tablep, t0, lsr #2]
    and     t0, t0, #0xf8
    orr     rgba, rgba, t0, lsl #16-3
    orr     data0, data0, data1
    orr     data0, data0, v, lsl #16

    // Blue
    // 1.772*(U-128)
    mul     t0, const_1_772, u
    mov     t0, t0, asr #23
    // +Y
    // now u,v is free
    adds    v, t0, y0
    movmi   v, #0
    cmp     v, #0xff
    movgt   v, #0xff
    ldrb    data1, [tablep, v, lsr #2]
    and     v, v, #0xf8
    orr     rgba, rgba, v, lsl #7
    adds    t0, t0, y1
    movmi   t0, #0
    cmp     t0, #0xff
    movgt   t0, #0xff
    and     t0, t0, #0xf8
    ldrb    v, [tablep, t0, lsr #2]
    orr     rgba, rgba, t0, lsl #16+7
    orr     data0, data0, data1, lsl #10
    orr     data0, data0, v, lsl #16+10

    // do not update sp here! keep 8-Bytes aligned
    ldmia   sp, {SAVE_REGS0}
    orr     data0, data0, const_0x80008000
    str     data0, [dst, const_0x80008000, lsl #1]
    orr     rgba, rgba, const_0x80008000
    str     rgba, [dst], #4
    subs    pixels, pixels, #2
    bne     @10

    // keep 8-Bytes aligned
    ldmfd   sp!, {SAVE_REGS0, SAVE_REGS1}
    bx      lr

@table:
    dcd 0x01000800
    dcd 0x03000200
    dcd 0x05000400
    dcd 0x07000600
    dcd 0x09010801
    dcd 0x0b030a02
    dcd 0x0d050c04
    dcd 0x0f070e06
    dcd 0x11091008
    dcd 0x130b120a
    dcd 0x150d140c
    dcd 0x170f160e
    dcd 0x19111810
    dcd 0x1b131a12
    dcd 0x1d151c14
    dcd 0x1f171e16

@constants:
/* const_0x80008000      r4 */  dcd 0x80008000
/* const_minus_0_344     r5 */  dcd (-2885681)
/* const_minus_0_714     r6 */  dcd (-5989466)
/* const_1_402           r7 */  dcd 11760828
/* const_1_772           r8 */  dcd 14864613

    
//     OSTick begin;
//     u8* yuyv = (u8*)src;
//     u32* rgb = (u32*)dest;
// 
//     u32 limit = pixels >> 1;
//     u32 i;
// 
// begin = OS_GetTick();
// 
//     for (i = 0; i < limit; i++)
//     {
//         u8 y1 = *yuyv++;
//         u8 u  = *yuyv++;
//         u8 y2 = *yuyv++;
//         u8 v  = *yuyv++;
//         *rgb++ = (u32)( 0x80008000 |
//                         ( ( trim( y2 + b_diff( u    ) ) & 0xF8 ) << 23) |
//                         ( ( trim( y2 + g_diff( u, v ) ) & 0xF8 ) << 18) |
//                         ( ( trim( y2 + r_diff( v    ) ) & 0xF8 ) << 13) |
//                         ( ( trim( y1 + b_diff( u    ) ) & 0xF8 ) <<  7) |
//                         ( ( trim( y1 + g_diff( u, v ) ) & 0xF8 ) <<  2) |
//                         ( ( trim( y1 + r_diff( v    ) )        ) >>  3) );
//     }
// 
// OS_Printf("total(666) = %d us\n", (u32)OS_TicksToMicroSeconds(OS_GetTick() - begin));
// 
//     return pixels * sizeof(RGBX16);

#undef SAVE_REGS0
#undef SAVE_REGS1
#undef t0   
#undef rgba 
#undef u    
#undef v    
#undef y0   
#undef y1   

#undef tablep  
#undef data0   
#undef data1   

#undef CONST_REGS 
#undef const_0x80008000      
#undef const_minus_0_344     
#undef const_minus_0_714     
#undef const_1_402           
#undef const_1_772           
}
#endif


static void drawTexture(u8* vramAddr, BOOL shot)
{
#pragma unused(vramAddr)
#pragma unused(shot)
    s32 x0, y0;
    s32 x1, y1;
    s32 s0, s1;
    s32 t0, t1;

    x0 = cPictureOffsetX;
    x1 = cPictureOffsetX + cPictureWidth;
    y0 = cPictureOffsetY;
    y1 = cPictureOffsetY + cPictureHeight;

    s0 = 0;
    s1 = cPictureWidth;
    t0 = 0;
    t1 = cPictureHeight;

        G3X_Reset();

    // Ortho matrix
    {
        G3_Ortho(0,
                 -GX_LCD_SIZE_Y,
                 0,
                 GX_LCD_SIZE_X,
                 0 * FX32_ONE,
                 2 * FX32_ONE,
                 NULL);
        G3_StoreMtx(0);
    }

        {
            G3_MtxMode(GX_MTXMODE_TEXTURE);
            G3_Identity();
            // Use an identity matrix for the texture matrix for simplicity
            G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
        }

        //---------------------------------------------------------------------------
        // Set up a camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE };   // Eye position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);
        }

#if 0
    GX_BeginLoadTex();                 // map the texture image slots onto LCDC address space
    {
        GX_LoadTex((void *)&grad0[0],
                   myTexGrad0Addr,
                   256 * sizeof(u16)
                   );
        GX_LoadTex((void *)&grad1[0],
                   myTexGrad1Addr,
                   256 * sizeof(u16)
                   );

    }
    GX_EndLoadTex();                   // restore the texture image slots
#endif

        G3_PolygonAttr(GX_LIGHTMASK_NONE, // Light #0 is on
                       GX_POLYGONMODE_DECAL, // modulation mode
                       GX_CULL_BACK,   // cull none
                       1,              // polygon ID(0 - 63)
                       30,             // alpha(0 - 31)
                       GX_POLYGON_ATTR_MISC_XLU_DEPTH_UPDATE              // OR of GXPolygonAttrMisc's value
                       );
        G3_TexImageParam(GX_TEXFMT_DIRECT,
                         GX_TEXGEN_TEXCOORD,    // use texcoord
                         GX_TEXSIZE_S256,
                         GX_TEXSIZE_T256,
                         GX_TEXREPEAT_NONE,     // no repeat
                         GX_TEXFLIP_NONE,       // no flip
                         GX_TEXPLTTCOLOR0_USE,  // use color 0 of the palette
                         0x00000                // the offset of the texture image
                         );
        G3_Begin(GX_BEGIN_QUADS);
        {
            G3_TexCoord(s1 << FX32_SHIFT, t1 << FX32_SHIFT);
            G3_Vtx((s16)x1, (s16)(-y1), 0);
            G3_TexCoord(s1 << FX32_SHIFT, t0 << FX32_SHIFT);
            G3_Vtx((s16)x1, (s16)(-y0), 0);
            G3_TexCoord(s0 << FX32_SHIFT, t0 << FX32_SHIFT);
            G3_Vtx((s16)x0, (s16)(-y0), 0);
            G3_TexCoord(s0 << FX32_SHIFT, t1 << FX32_SHIFT);
            G3_Vtx((s16)x0, (s16)(-y1), 0);
        }
        G3_End();

//         G3_PolygonAttr(GX_LIGHTMASK_NONE, // Light #0 is on
//                        GX_POLYGONMODE_DECAL, // modulation mode
//                        GX_CULL_BACK,   // cull none
//                        2,              // polygon ID(0 - 63)
//                        1,             // alpha(0 - 31)
//                        GX_POLYGON_ATTR_MISC_DEPTHTEST_DECAL              // OR of GXPolygonAttrMisc's value
//                        );
//         G3_TexImageParam(GX_TEXFMT_DIRECT,
//                          GX_TEXGEN_TEXCOORD,    // use texcoord
//                          GX_TEXSIZE_S256,
//                          GX_TEXSIZE_T256,
//                          GX_TEXREPEAT_NONE,     // no repeat
//                          GX_TEXFLIP_NONE,       // no flip
//                          GX_TEXPLTTCOLOR0_USE,  // use color 0 of the palette
//                          0x10000                // the offset of the texture image
//                          );
//         G3_Begin(GX_BEGIN_QUADS);
//         {
//             G3_TexCoord(256 * FX32_ONE, (HEIGHT/2) * FX32_ONE);
//             G3_Vtx((s16)x1, (s16)y1, 0);
//             G3_TexCoord(256 * FX32_ONE, 0 * FX32_ONE);
//             G3_Vtx((s16)x1, (s16)y0, 0);
//             G3_TexCoord(0 * FX32_ONE, 0 * FX32_ONE);
//             G3_Vtx((s16)x0, (s16)y0, 0);
//             G3_TexCoord(0 * FX32_ONE, (HEIGHT/2) * FX32_ONE);
//             G3_Vtx((s16)x0, (s16)y1, 0);
//         }
//         G3_End();

            {
                OSIntrMode enabled = OS_DisableInterrupts();

                // swapping the polygon list RAM, the vertex RAM, etc.
                G3_SwapBuffers(GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z);
                mpCmrMgr2->setTexSwap();

                OS_RestoreInterrupts(enabled);
            }
    
    DrawFPS();
}

static void dumpWramBanks(u32 line)
{
    u32 i;

    OS_TPrintf("line %d: WramB=", line);
    for (i = 0; i < 8; i++) {
        OS_TPrintf(" %d", MI_GetWramBankEnable_B((s32)i));
    }
    OS_TPrintf(", WramC=");
    for (i = 0; i < 8; i++) {
        OS_TPrintf(" %d", MI_GetWramBankEnable_C((s32)i));
    }
    OS_TPrintf("\n");
}

#if SDK_VERSION_DATE < 20080624
static void proc_camera_intr_() {
#else
static void cameraVsyncCallback(CAMERAResult result) {
#pragma unused( result )
#endif
#if 1
    // 自家製のみ呼ぶ。
    mpCmrMgr2->procCameraIntr();
#else
    MenuSys()->procCameraIntr();
#endif

  OS_SetIrqCheckFlag( OS_IE_CAMERA );
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
//    CAMERAResult result;

#ifdef TWL_IPL_USE_RED_IPL
	HOTSW_Init();
#endif
	
	// 初期化
    // in new.cpp: TwlStartUp()

    MenuIrqHandlerStart(logBuf, sizeof(logBuf));

    // DEMOInitCommon(), DEMOInitVRAM(), DEMOInitDisplay3D()を呼ばない
    FX_Init();
    G3X_Init();                        // initialize the 3D graphics states
    G3X_InitMtxStack();                // initialize the matrix stack

    // GXでDMAを使わない (旧DMAはカメラDMAの邪魔になる)
    GX_SetDefaultDMA(GX_DMA_NOT_USE);

    // VRAMクリア
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);
    GX_SetBankForLCDC(GX_VRAM_LCDC_B);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_A, 128 * 1024);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_B, 128 * 1024);

    GX_SetBankForLCDC(GX_VRAM_LCDC_C);
    GX_SetBankForLCDC(GX_VRAM_LCDC_D);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_C, HW_VRAM_C_SIZE);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_D, HW_VRAM_D_SIZE);
//    GX_SetBankForTex(GX_VRAM_TEX_0_C);


    // ダイレクトビットマップ表示モード＆文字表示
    GX_SetBankForBG(GX_VRAM_BG_256_AB);         // VRAM-A,BバンクをＢＧに割り当る
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_4, GX_BG0_AS_3D);

    G2_SetBG0Priority(2);
    G3X_AntiAlias(TRUE);               // enable antialias(without additional computing costs)
    G3X_AlphaBlend(TRUE);              // enable alpha blending
    //---------------------------------------------------------------------------
    // After you start the geometry engine and the rendering one
    // (by GX_SetPower etc.), G3_SwapBuffers must be called once before use
    //---------------------------------------------------------------------------
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
    G3X_SetClearColor(GX_RGB(0, 0, 0), // clear color
                      0,               // clear alpha
                      0x7fff,          // clear depth
                      63,              // clear polygon ID
                      FALSE);          // fog
    G3_ViewPort(0, 0, 255, 191);       // Viewport

    GX_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3);

    G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GX_BG_EXTPLTT_01);
    G2_SetBG1Priority(1);
    G2_BG1Mosaic(FALSE);

    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x20000);
    G2_SetBG3Priority(3);
    G2_BG3Mosaic(FALSE);

    // 文字の読み込み
    {
        static const GXRgb pal[16] = { GX_RGB(0, 0, 0), GX_RGB(31, 31, 31), };
        GX_LoadBG1Char(DEMOAsciiChr, 0x00000, sizeof(DEMOAsciiChr));
        GX_LoadBGPltt(pal, 0x0000, sizeof(pal));
    }
//     we = TRUE;
//     re = FALSE;
//     wp = 0;
//     rp = 1;
//     tp = 0;
//     wp_pending = TRUE;

//    MakeGrad();
    
    // VBlank割り込みを許可するより前にカメラマネージャを生成しておく。
// #if 0
//     pDtcm = OS_AllocFromArenaLo(OS_ARENA_DTCM, 256*4*sizeof(u32), HW_CACHE_LINE_SIZE);
// #else
//     pDtcm = (u32 *)myArena;
// #endif
    OS_TPrintf("sDtcmWork=0x%x\n", &sDtcmWork);
    OS_DumpArenaInfo(OS_ARENA_DTCM, TRUE);

#if SDK_VERSION_DATE < 20080624
  OS_SetIrqFunction( OS_IE_CAMERA  , proc_camera_intr_ );
#else
    // CAMERAライブラリからのコールバックに変更されました。
//    OS_SetIrqFunction(OS_IE_CAMERA, proc_camera_intr_);
    CAMERA_SetVsyncCallback(cameraVsyncCallback);
#endif

    // new()って使えないよね…new.cpp追加で使えた！
    mpCmrMgr2 = new cmn::sys::cmr2::Manager();

#if 0
    // old

    // 割り込みを許可しておかないとダメ？
    mpCmrMgr2->init(cmn::sys::cmr2::eMode_Tex, current,
                   /*GX_VRAM_A, GX_VRAM_B,*/GX_VRAM_C, GX_VRAM_D, &sDtcmWork, drawTexture);
    mpCmrMgr2->activate();

    mpCmrMgr2->setShutterSnd(_binary_camera_shutter_dat,
                            (u32)(_binary_camera_shutter_dat_end - _binary_camera_shutter_dat),
                            TRUE);
#endif

    mpCmrMgr2->preInit();
//    mpCmrMgr2->setCaptureRgbFlag(TRUE);

//     // stereo
//     mpCmrMgr2->setShutterSnd(_binary_camera_shutter_dat,
//                              (u32)(_binary_camera_shutter_dat_end - _binary_camera_shutter_dat),
//                              TRUE);

//     // mono
//     mpCmrMgr2->setShutterSnd(_binary_camera_shutter_MONO_dat,
//                              (u32)(_binary_camera_shutter_MONO_dat_end - _binary_camera_shutter_MONO_dat),
//                              FALSE);

    if (1) {
//        mpCmrMgr2->setOneShotFlag(TRUE);
    }




    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableInterrupts();

    // SD初期化
    FS_Init( FS_DMA_NOT_USE );

#if 1
    // demos.TWL/snd/extraFunc
    /* サウンド初期化 */
    SND_Init();
    SND_AssignWaveArc((SNDBankData*)sound_bank_data, 0, (SNDWaveArc*)sound_wave_data);
    SND_StartSeq(0, sound_seq_data, 0, (SNDBankData*)sound_bank_data);
#endif

        /* サウンド拡張機能初期化 */
        SNDEX_Init();

    OS_WaitIrq(TRUE, OS_IE_V_BLANK);
    GX_DispOn();

//     OS_TPrintf("sizeof testEncodeWork=%d\n", sizeof(cmn::sys::cmr2::Manager::testEncodeWork));
//     OS_TPrintf("ticks/1000usec = %lld\n", (u64)OS_MicroSecondsToTicks(1000));
// 
//     {
//         OSTick tick;
//         BOOL flag = FALSE;
// 
//         tick = OS_GetTick();
//         mpCmrMgr2->preInit();
// 
//         // 1秒あればpreInitは終わるとして
//         OS_Sleep(1000);
// 
//         tick = OS_GetTick();
//         // 本initにかかる時間は？
//         mpCmrMgr2->init(cmn::sys::cmr2::eMode_Tex, current, GX_VRAM_C, GX_VRAM_D, &sDtcmWork, drawTexture);
// 
//         while (1) {
//             OS_Sleep(5000);
//             if (!flag) {
//                 flag = TRUE;
//                 OS_TPrintf("main loop ok, tick before init=%lld\n", (u64)tick);
//             }
//         }
//     }


#ifdef TWL_IPL_MENU_CAMERA_MANAGER_USE_DSP
    MI_FreeWram_B  ( MI_WRAM_ARM9 );
    MI_CancelWram_B( MI_WRAM_ARM9 );
    MI_FreeWram_C  ( MI_WRAM_ARM9 );
    MI_CancelWram_C( MI_WRAM_ARM9 );

    // for DSP_PlaySound() ?
    // DSP_InitPipe();
    /* プログラムを起動する */
#if 1
    // JPEG DECと併用できない！
    FSFile file[1];
    DSP_OpenStaticComponentJpegEncoder(file);
    if(!DSP_LoadJpegEncoder(file, 0xff, 0xff))
    {
        // WramB= 1 1 1 0 0 0 0 0, WramC= 1 1 1 1 0 0 0 0
        OS_TPanic("failed to load JpegEncoder DSP-component! (lack of WRAM-B/C)");
    }
#endif
    dumpWramBanks(__LINE__);
#if 0
    // JPEG ENCと併用できない！
    if(!DSP_LoadJpegDecoder())
    {
        // WramB= 1 1 0 0 0 0 0 0, WramC= 1 1 1 1 0 0 0 0
        OS_TPanic("failed to load JpegDecoder DSP-component! (lack of WRAM-B/C)");
    }
    dumpWramBanks(__LINE__);
#endif
#if 0
    // JPEGと併用できない！
    if (!DSP_LoadShutter()) {
        // WramB= 1 0 0 0 0 0 0 0, WramC= 1 0 1 1 0 0 0 0
        OS_TPanic("failed to load Shutter DSP-component! (lack of WRAM-B/C)");
    }
#endif
#else
    dumpWramBanks(__LINE__);
#endif



        OS_WaitVBlankIntr();

    while (1)
    {
        u16 pad;
        u16 trg;
        static u16 old = 0xffff;
        static BOOL refresh = FALSE;

//        static const u32     myTexGrad0Addr = 0x2000;
//        static const u32     myTexGrad1Addr = 0x3000;


            /* サウンドメイン処理 */
            while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
            {
            }
            (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

#if 1
//        // more than 1 frame
//        OS_WaitIrq(TRUE, OS_IE_V_BLANK);
        OS_WaitVBlankIntr();
#else
        // keep AHB busy
        OS_ClearIrqCheckFlag(OS_IE_V_BLANK);
        while (!(OS_GetIrqCheckFlag() & OS_IE_V_BLANK)) {
            u32 wordIdx;
            wordIdx = (wordIdx + 0x11) & 0x1fff;
            gWords[wordIdx]++;
        }
#endif
        GX_LoadBG1Scr(text_buffer, 0, sizeof(text_buffer));
        
        
        pad = PAD_Read();
        trg = (u16)(pad & ~old);
        old = pad;

        {
            static BOOL initialized;
            static BOOL destroying;

            if (!initialized && (trg || 1)) {
                initialized = TRUE;
                mpCmrMgr2->init(cmn::sys::cmr2::eMode_Tex, current, GX_VRAM_C, GX_VRAM_D, &sDtcmWork, drawTexture);

                mpCmrMgr2->startCapture(current);
            }

            if (!destroying) {
                if (trg & PAD_BUTTON_START) {
                    destroying = TRUE;
                }
            } else {
                if (!mpCmrMgr2->tryDestroy()) {
                    IPL_PRINT("waiting for destroy...\n");
                } else {
                    IPL_PRINT("destroy done!\n");
                    destroying = FALSE;
                    initialized = FALSE;
                }
            }
            if (!initialized || destroying) {
                // この後のボタンチェック(撮影やカメラ切り替え等)を行わない。
                continue;
            }
        }
        


        if (PAD_DetectFold()) {
            // キャプチャを停止して、停止を確認してから
            // スリープモードへ移行するようにしないと、
            // cameraIntr: status error
            // cameraIntr: status error
            // と、2回エラーになるようだ。

            if (mpCmrMgr2->tryPrepareToSleep()) {
                PM_GoSleepMode(PM_TRIGGER_COVER_OPEN, PM_PAD_LOGIC_AND, 0);
            } else {
                IPL_PRINT("waiting for ready to sleep...\n");
            }
        }

//         if (trg & PAD_BUTTON_B) {
//             // test
//             OS_TPrintf("force stop dma\n");
//             MI_StopNDma(CAMERA_NEW_DMA_NO);
//         }
//         if (trg & PAD_BUTTON_Y) {
//             // test
//             OS_TPrintf("force long dma\n");
//             sLongDmaFlag = TRUE;
//         }

        if (!mpCmrMgr2->isCameraChanging()) {
            if (trg & PAD_BUTTON_X) {
                mpCmrMgr2->changeCamera();
            } else if (pad & PAD_BUTTON_SELECT) {
                static u8 r;

                r += 1;
                if (!(r & 3)) {
                    mpCmrMgr2->changeCamera();
                }
            }
        }

        
        if (trg & (PAD_BUTTON_A | PAD_BUTTON_B)) {
#ifndef TWL_IPL_FINAL
            mpCmrMgr2->mDebugPort = __LINE__;
#endif
//      SNDEX_SetDSPMixRate( 0 );
//      SNDEX_ForceOut( TRUE );
// 
//             if (trg & PAD_BUTTON_A) {
//             DSP_PlaySound(const_cast<u8*>(_binary_camera_shutter_dat),
//                           (u32)(_binary_camera_shutter_dat_end - _binary_camera_shutter_dat) / 2,
//                           TRUE);
//             } else {
//             DSP_PlaySound(const_cast<u8*>(_binary_wheel_dat),
//                           (u32)(_binary_wheel_dat_end - _binary_wheel_dat) / 2,
//                           TRUE);
//             }

// #if 0
//             // test
//             CAMERA_SetLED(TRUE);
// #endif

            
            IPL_PRINT("shoot()=%d\n", mpCmrMgr2->shoot());
        }
        

//         if (re) {
//             OSTick begin = OS_GetTick();
//             // 今はy方向の間引きが出来ないので、480 linesじゃなくて156*3 linesとしている。
//             // ちょっとこのへん適当。
//             rp = (u8)(wp ^ 1);
//             Shrink640to208(buffer[rp ? 0 : 1], 156*3/*480*/, pDtcm, (u16*)(rp ? HW_LCDC_VRAM_D : HW_LCDC_VRAM_C) /*buffer[2]*/);
//             we = TRUE;
// //            CpuYuv2Rgb666(buffer[2], (u16*)(wp ? HW_LCDC_VRAM_D : HW_LCDC_VRAM_C), (u32)(HW_LCD_WIDTH*HW_LCD_HEIGHT) / 1);
//             OS_Printf("total(exp) = %d us\n", (u32)OS_TicksToMicroSeconds(OS_GetTick() - begin));
//             DrawFPS();
//         }

//         
//         if (refresh) {
// //            GX_LoadBG3Scr( buffer[rp], 0, BYTES_PER_LINE * HEIGHT);
//             GX_DisableBankForTex();
//             GX_SetBankForTex(rp ? GX_VRAM_TEX_0_D : GX_VRAM_TEX_0_C);
//             GX_SetBankForLCDC(rp ? GX_VRAM_LCDC_C : GX_VRAM_LCDC_D);
//             refresh = FALSE;
//         }
//             
//         if (!we && !sNowConvert)
//         {
//             // 変なことになりそうだがとりあえず動かす。
//             // CPUによるYUV->RGB変換実行
// //            DC_InvalidateRange( buffer[wp], BYTES_PER_LINE * HEIGHT);
//             DC_FlushAll();
// 
// // RGB666実験は一旦封印。
// //             if (mode || 1) {
// //                 OSTick begin = OS_GetTick();
// //                 u32 ret;
// //                 // テクスチャイメージスロットの半分、つまり画面の上半分しか書けない。
// //                 GX_SetBankForLCDC((wp) ? GX_VRAM_LCDC_D : GX_VRAM_LCDC_C);
// // #if 1
// //                 ret = CpuYuv2Rgb666(buffer[wp],  (void *)((wp) ? HW_LCDC_VRAM_D : HW_LCDC_VRAM_C), (u32)(HW_LCD_WIDTH*HW_LCD_HEIGHT) / 2);
// // //                OS_Printf("total(666) = %d us, ret=0x%08x\n", (u32)OS_TicksToMicroSeconds(OS_GetTick() - begin), ret);
// // #else
// //                 ret = CpuYuv2Rgb666(buffer[wp],  buffer[wp] + (WIDTH*HEIGHT) / 2, (u32)(HW_LCD_WIDTH*HW_LCD_HEIGHT) / 2);
// //                 OS_Printf("%04x %04x %04x %04x %04x %04x %04x %04x ret=%08x\n",
// //                           buffer[wp][(WIDTH*HEIGHT) / 2 + 0],
// //                           buffer[wp][(WIDTH*HEIGHT) / 2 + 1],
// //                           buffer[wp][(WIDTH*HEIGHT) / 2 + 2],
// //                           buffer[wp][(WIDTH*HEIGHT) / 2 + 3],
// //                           buffer[wp][(WIDTH*HEIGHT) / 2 + 4],
// //                           buffer[wp][(WIDTH*HEIGHT) / 2 + 5],
// //                           buffer[wp][(WIDTH*HEIGHT) / 2 + 6],
// //                           buffer[wp][(WIDTH*HEIGHT) / 2 + 7],
// //                           ret);
// // #endif
// //             }
//             /*else*/
//             {
//                 
// #if 1
// #if 0
//                 // 条件を合わせて下半分だけに。
//                 CpuYuv2Rgb( buffer[wp],  buffer[wp] + (WIDTH*HEIGHT) / 2, (u32)(HW_LCD_WIDTH*HW_LCD_HEIGHT) / 2);
//                 MI_CpuFillFast((void *)buffer[wp], 0, sizeof(u16) * (u32)(HW_LCD_WIDTH*HW_LCD_HEIGHT) / 2);
// #else
//                 OSTick begin = OS_GetTick();
// //                CpuYuv2Rgb666(buffer[wp], buffer[wp] + (WIDTH*HEIGHT) / 2, (u32)(HW_LCD_WIDTH*HW_LCD_HEIGHT) / 2);
// 
//                 // 固定データ実験
//                 Shrink640to208(buffer[wp]/* b_dat*/, (u16*)(wp ? HW_LCDC_VRAM_D : HW_LCDC_VRAM_C) /*buffer[wp]*/, 96 * 3, pDtcm);
// //                Shrink640to208(buffer[wp]/* b_dat*/, buffer[2], 96 * 3, pDtcm);
// //                CpuYuv2Rgb666(buffer[2], (u16*)(wp ? HW_LCDC_VRAM_D : HW_LCDC_VRAM_C), (u32)(HW_LCD_WIDTH*HW_LCD_HEIGHT) / 2);
// 
//                 OS_Printf("total(exp) = %d us\n", (u32)OS_TicksToMicroSeconds(OS_GetTick() - begin));
// #endif
// #endif
//             }
// 
// //            DC_FlushRange( buffer[wp], BYTES_PER_LINE * HEIGHT);
// //            DC_FlushRange((void *)((wp) ? HW_LCDC_VRAM_D : HW_LCDC_VRAM_C), BYTES_PER_LINE * HEIGHT);
// //            DC_StoreAll();
// 
//             rp = wp;
//             refresh = TRUE;
//             we = TRUE;
//         }
// 



        if (mpCmrMgr2->isShotDone()) {
// #if 1
//             // test
//             CAMERA_SetLED(FALSE);
// #endif
            OS_TPrintf("SHOT DONE!\n");
            // check error mShotErr

            // anything to process ?
            mpCmrMgr2->clearShotDone();

            if (!mpCmrMgr2->getOneShotFlag()) {
                mpCmrMgr2->startCapture(current);
            }

//    SNDEX_SetDSPMixRate( 8 );
//    SNDEX_ForceOut( FALSE );

        } else {
            // ここで(trg)にするとカメラ切り替えとかできなくなる。
            if (mpCmrMgr2->getOneShotFlag() && (trg & PAD_BUTTON_Y)) {
                mpCmrMgr2->startCapture(current);
            }
        }

//         {
// 
//             if (re) {
//                 re = FALSE;
//                 tp = rp;
//             }
// 
//         }

    }
}

/*---------------------------------------------------------------------------*
  Name:         Ｖブランク割り込み処理

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    mpCmrMgr2->procVBlankIntr();

    
//     if (tp == rp) {
//         GX_DisableBankForTex();
//         GX_SetBankForTex( tp ? GX_VRAM_TEX_0_D : GX_VRAM_TEX_0_C);
//         GX_SetBankForLCDC(tp ? GX_VRAM_LCDC_C  : GX_VRAM_LCDC_D);
//     }

    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

// static void CameraIntr2(void)
// {
//     u32 nwp = wp;
// 
//     // ちゃんとエラー処理してないけど…
//     if (CAMERA_GetErrorStatus()) {
//         OS_TPrintf("Error!\n");
//         CAMERA_StopCapture();
//         CAMERA_ClearBuffer();
//         MI_StopNDma(CAMERA_NEW_DMA_NO);
//         CAMERA_StartCapture();
//     } else {
//         if (MI_IsNDmaBusy(CAMERA_NEW_DMA_NO)) {
//             static u32 count;
//             OS_TPrintf("DMA was not done until VBlank.%d\n", count++);
//             MI_StopNDma(CAMERA_NEW_DMA_NO);
//         } else {
//             if (reading) {
//                 re = TRUE;
//                 if (we) {
//                     we = FALSE;
//                     wp ^= 1;
//                 }
//             }
//         }
//     }
// 
//     if (sLongDmaFlag) {
//         sLongDmaFlag = FALSE;
//         CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[nwp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT + 1), /*(MINDmaCallback)NDmaDummyIntr*/NULL, NULL);
//     } else {
//         CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[nwp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), /*(MINDmaCallback)NDmaDummyIntr*/NULL, NULL);
//     }
//     reading = TRUE;
// 
// 
// //     OS_SetIrqCheckFlag(OS_IE_CAMERA); // checking camera interrupt
// // 
// //     if (mError) {
// //         CAMERA_ClearBuffer();
// //         mError = FALSE;
// //     } else {
// //         if (CAMERA_GetErrorStatus()
// //             /*||
// //             (OS_GetIrqMask() & (OS_IE_NDMA0 << CAMERA_NEW_DMA_NO))*/) {
// //             // error
// //         OS_TPrintf("line %d %x\n", __LINE__, (OS_GetIrqMask() & (OS_IE_NDMA0 << CAMERA_NEW_DMA_NO)));
// // 
// //             CAMERA_StopCapture();
// //             MI_StopNDma(CAMERA_NEW_DMA_NO);
// // //            OS_DisableIrqMask(OS_IE_NDMA0 << CAMERA_NEW_DMA_NO);
// //             mError = TRUE;
// //             return;
// //         }
// // 
// //         if (mLastState == eStt_Capture) {
// // //            OS_EnableIrqMask(OS_IE_NDMA0 << CAMERA_NEW_DMA_NO);
// //             return;
// //         }
// //     }
// // 
// //     switch (mState) {
// //     case eStt_Capture:
// //         CAMERA_StartCapture();
// // //        reg_OS_IF = OS_IE_NDMA0 << CAMERA_NEW_DMA_NO;
// //         // callbackを指定しないと、mi_ndma.cで
// // // 		//---- set interrupt enable 
// // // 		if ( callback )
// // // 		{
// // // 			contData |= MI_NDMA_IF_ENABLE;
// // // 		}
// // //       となっているので割り込みがかからない。
// // 
// // //        CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[0], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), (MINDmaCallback)NDmaDummyIntr, NULL);
// //         CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[0], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), (MINDmaCallback)NDmaIntr, NULL);
// //         break;
// // 
// //     default:
// //         break;
// //     }
// // 
// //     mLastState = mState;
// 
// //    DrawFPS();
// }
// 
// // static void NDmaIntr(void)
// // {
// //     OS_SetIrqCheckFlag(OS_IE_NDMA0 << CAMERA_NEW_DMA_NO);
// // 
// //     if (mError) {
// //         // captured data is broken
// //     } else {
// //         // 変換スレッドにメッセージを送る。
// // 
// //         mLastState = mState;
// //         switch (mState) {
// //         case eStt_Capture:
// //             // 表示用キャプチャを継続。
// //             CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[0], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), (MINDmaCallback)NDmaIntr, NULL);
// //             break;
// // 
// //         default:
// //             // とりあえずそれ以外はキャプチャ停止。
// //             CAMERA_StopCapture();
// //             break;
// //         }
// //         OS_TPrintf("Send message!\n");
// //         
// //     }
// // 
// // //    OS_DisableIrqMask(OS_IE_NDMA0 << CAMERA_NEW_DMA_NO);
// // }

static void NDmaDummyIntr(void)
{
//     // camera-2/main.c: CameraDmaIntr()で、これが行われている。
//     // これを抜いたのがうまく動かない原因ではなさそう。
//     // OS_SetIrqCheckFlag(OS_IE_NDMA0 << CAMERA_NEW_DMA_NO);
// 
//     // camera-2/main.c: CameraDmaIntr()で、これが行われている。
//     // …おい！ここでセットする関数は、引数無しだぞ？どうなっとるん？
//     // あ、CAMERA_DmaRecv*()のcallbackではないから、別物なのか！？
//     // callbackは毎line来るんだよね？
//     OS_SetIrqFunction(OS_IE_NDMA0 << CAMERA_NEW_DMA_NO, NDmaDummyIntr);   // for next interrupt
    
}


// /*---------------------------------------------------------------------------*
//   Name:         カメラ割り込み処理 (エラー時とVsync時の両方で発生)
// 
//   Description:  
// 
//   Arguments:    None
// 
//   Returns:      None
//  *---------------------------------------------------------------------------*/
// void CameraIntr(void)
// {
//     int nwp = 2;
//     OS_SetIrqCheckFlag(OS_IE_CAMERA); // checking camera interrupt
// 
//     if (CAMERA_GetErrorStatus()) 
//     {
//         OS_TPrintf("Error was occurred.\n");
//         // 停止処理
//         CAMERA_StopCapture();           // カメラ停止
//         CAMERA_ClearBuffer();           // クリア
//         MI_StopNDma(CAMERA_NEW_DMA_NO); // DMA停止
//         wp_pending = TRUE;              // 次回も同じフレームを使用する
//         startRequest = TRUE;            // カメラ再開要求
//         return;                         // waiting next frame (skip current frame)
//     }
// 
//     // 以降はVsync時の処理
// 
//     if (startRequest)
//     {
//         CAMERA_ClearBuffer();
//         CAMERA_StartCapture();
//         startRequest = FALSE;
//     }
// 
//     if (CAMERA_IsBusy() == FALSE)   // done to execute stop command?
//     {
//         //OS_TPrintf("while stopping the capture or just finished\n");
//     }
//     else
//     {
//         if (MI_IsNDmaBusy(CAMERA_NEW_DMA_NO))    // NOT done to capture last frame?
//         {
//             OS_TPrintf("DMA was not done until VBlank.\n");
//             MI_StopNDma(CAMERA_NEW_DMA_NO);  // DMA停止
// //          return; // waiting next frame (skip current frame)
//         }
//         // start to capture for next frame
//         if (wp_pending)
//         {
//             wp_pending = FALSE;
//         }
//         else
//         {
//             // 更新バッファ変更
//         if (we) {
//             nwp = wp;
//             wp ^= 1;
//             we = FALSE;
//         }
// 
// // #ifdef CONVERT_WITH_DSP
// //             // DSPによるYUV->RGB変換実行
// //             sNowConvert = TRUE;
// //             DSP_Yuv2RgbConvertAsync(buffer[rp ^ 1], buffer[rp ^ 1], (WIDTH*HEIGHT*2), Yuv2rgbCallback);
// // #else
// //             // CPUによるYUV->RGB変換実行
// //             DC_InvalidateRange( buffer[rp ^ 1], BYTES_PER_LINE * HEIGHT);
// //             GX_SetBankForLCDC((rp ^ 1) ? GX_VRAM_LCDC_D : GX_VRAM_LCDC_C);
// //             CpuYuv2Rgb666(buffer[rp ^ 1],  (void *)((rp ^ 1) ? HW_LCDC_VRAM_D : HW_LCDC_VRAM_C), (u32)(WIDTH*HEIGHT) );
// //             CpuYuv2Rgb( buffer[rp ^ 1],  buffer[rp ^ 1], (u32)(WIDTH*HEIGHT) );
// // #endif
//         }
// 
// //        CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[nwp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), NULL, NULL);
//         // なんで
//         // CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[nwp], CAMERA_GetBytesAtOnce(WIDTH)/LINES_SPLIT, CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), NULL, NULL);
//         // じゃないの？
// //        CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[nwp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), NULL, NULL);
//         CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, buffer[nwp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), (MINDmaCallback)CameraIntr, NULL);
//     }
// 
//     // frame rate print
//     // 割り込みでは遠慮しよう。
// //    DrawFPS();
// }

/*---------------------------------------------------------------------------*
  Name:         PutString

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PutString( char *format, ... )
{
    u16             *dest = text_buffer;
    char            temp[32+1];
    int             i;
    va_list         va;

    va_start(va, format);
    (void)OS_VSNPrintf(temp, sizeof(temp), format, va);
    va_end(va);

    MI_CpuClearFast(text_buffer, sizeof(text_buffer));
    for (i = 0; i < 32 && temp[i]; i++)
    {
        dest[i] = (u16)((u8)temp[i] | (0 << 12));
    }
    DC_StoreRange(text_buffer, sizeof(text_buffer));
}

/*---------------------------------------------------------------------------*
  Name:         DrawFPS

  Description:  FPSを表示します。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void DrawFPS(void)
{
#define FPS_SAMPLES 4
    static OSTick begin = 0;
    static int uspf[FPS_SAMPLES] = { 0 };
    static int count = 0;
    //                        0123456789abcdef
    static u8 star[16 + 1] = "                ";
    static u8 starIdx;
    int i;
    int sum = 0;
    OSTick end = OS_GetTick();
    if (begin)  // 初回を除く
    {
        uspf[count] = (int)OS_TicksToMicroSeconds(end - begin);
        count = (count + 1) % FPS_SAMPLES;
    }
    begin = end;
    // 平均値の算定
    for (i = 0; i < FPS_SAMPLES; i++)
    {
        if (uspf[i] == 0)  break;
        sum +=  uspf[i];
    }
    star[starIdx] = ' ';
    star[starIdx ^ 8] = ' ';
    starIdx = (u8)((starIdx + 1) & 0xf);
    star[starIdx] = '*';
    star[starIdx ^ 8] = '*';
    star[16] = '\0';
    if (1 || sum)
    {
        int mfps = (int)(1000000000LL * i / sum);
        PutString("%2d.%03d fps %s", mfps / 1000, mfps % 1000, star);
    }
}

// /*---------------------------------------------------------------------------*
//   Name:         Yuv2rgbCallback
// 
//   Description:  YUV->RGB変換完了後に呼び出されるコールバック
// 
//   Arguments:    None
// 
//   Returns:      None
//  *---------------------------------------------------------------------------*/
// static void Yuv2rgbCallback(void)
// {
//     sNowConvert = FALSE;
// }
// 
// 

// namespace cmn { namespace sys { namespace cmr2
// {
// void* Manager::dspAlloc_( u32 size ) {
// #pragma unused(size)
//     // this function must not be called by SDK
//     OS_TPanic("dspAlloc_\n");
//     return NULL;
// }
// 
// void Manager::dspFree_( void* p_ptr ) {
// #pragma unused(p_ptr)
//     // this function is always called by SDK
//     return;
// }
// 
// }}}
