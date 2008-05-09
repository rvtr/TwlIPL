/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mb_loader.c

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

#include <twl.h>

#if	defined(SDK_ARM7)
#include <nitro_wl/ARM7/WlLib.h>
#else
#ifdef SDK_SMALL_BUILD
#include "SYSM_work.h"
#endif
#endif


#include <nitro/mb.h>	// twl/mb.hがない。
#include <mb_private.h>
#include <sysmenu/mb_loader.h>

// --------------------------------------------------------------------------

// Download情報のサイズ

#define MB_OVT_MAX_SIZE					MB_COMM_BLOCK_SIZE			// OVTの最大サイズ（Blockサイズを最大サイズとする）

/*----------------------------------------------------------------------------*/
#define MB_TRIGGER_SIGNAL_TO_ARM7		(0x00000001)

/*----------------------------------------------------------------------------*/
static void MIm_CpuClear32( register u32 data, register void *destp, register u32 size );
static void MIm_CpuClear32( register u32 data, register void *destp, register u32 size );
static void	LOADERi_LocateAllSegments( MBDownloadFileInfo *mdfi );
static void MBi_SearchAndLocateSegmentInfo( MBDownloadFileInfo *mdfi, u16 processor );
static void	LOADERi_Jump(void);
static void MBi_fifo_callback_arm7(PXIFifoTag tag, u32 msg_adr, BOOL err);

/*----------------------------------------------------------------------------*/
static MB_LoaderCallback loader_precallback = NULL;

/*----------------------------------------------------------------------------*/


#if	defined(SDK_ARM7)

/*---------------------------------------------------------------------------*
  Name:         MIm_CpuCopy32

  Description:  ローダー用CpuCopy

  Arguments:    srcp, destp, size

  Returns:      void
 *---------------------------------------------------------------------------*/

#include <nitro/code32.h>

asm void MIm_CpuCopy32( register const void *srcp, register void *destp, register u32 size )
{
        add     r12, r1, r2             // r12: destEndp = destp + size

@30:
        cmp     r1, r12                 // while (destp < destEndp)
        ldmltia r0!, {r2}               // *((vu32 *)(destp)++) = *((vu32 *)(srcp)++)
        stmltia r1!, {r2}
        blt     @30

        bx      lr
}

#ifndef SDK_SMALL_BUILD
static asm void MIm_CpuClear32( register u32 data, register void *destp, register u32 size )
{
        add     r12, r1, r2             // r12: destEndp = destp + size

@20:
        cmp     r1, r12                 // while (destp < destEndp)
        stmltia r1!, {r0}               // *((vu32 *)(destp++)) = data
        blt     @20

        bx      lr
}
#endif

#include <nitro/codereset.h>


/*---------------------------------------------------------------------------*
  Name:         LOADERi_LocateAllSegments

  Description:  ARM9,ARM7の各セグメントを必要に応じて再配置する。

  Arguments:    mdfi

  Returns:      void
 *---------------------------------------------------------------------------*/

static void	LOADERi_LocateAllSegments( MBDownloadFileInfo *mdfi )
{
	MBi_SearchAndLocateSegmentInfo(mdfi, MI_PROCESSOR_ARM9);		// ARM9セグメントについての配置処理
	MBi_SearchAndLocateSegmentInfo(mdfi, MI_PROCESSOR_ARM7);		// ARM7セグメントについての配置処理
}


/* 指定のセグメントを検索し, 再配置する */
static void MBi_SearchAndLocateSegmentInfo( MBDownloadFileInfo *mdfi, u16 processor )
{
	int	i;
	MbSegmentInfo *seg_info;
	
	if( mdfi ) {
		for( i = 0 ; i < MB_DL_SEGMENT_NUM ; ++i ) {
			seg_info = &mdfi->seg[i];
			if ( seg_info->target == processor ) {
				if ( seg_info->recv_addr != seg_info->load_addr ) {
					MIm_CpuCopy32( (void*)seg_info->recv_addr, (void*)seg_info->load_addr, seg_info->size );
#ifndef SDK_SMALL_BUILD												// ※IPL2の場合は、このメモリクリアはIPL2で行う。
					MIm_CpuClear32( 0, (void*)seg_info->recv_addr, seg_info->size );
#endif
				}
			}
		}
	}
}


/*---------------------------------------------------------------------------*
  Name:         LOADERi_Jump

  Description:  ローダーを起動 (ARM7/9 共通)

  Arguments:    無し

  Returns:      void
 *---------------------------------------------------------------------------*/

static void	LOADERi_Jump(void)
{
#if	defined(SDK_ARM7)
	
	MBDownloadFileInfo *mdfi	= (MBDownloadFileInfo*)MB_DOWNLOAD_FILEINFO_ADDRESS;
	MBParam	*p_param			= (MBParam*)HW_WM_BOOT_BUF;
	
	if( p_param->boot_type != MB_TYPE_MULTIBOOT ) {					// ブートタイプがマルチブートでない場合は、何もせずにTRUEリターン。
		return;
	}
	
	LOADERi_LocateAllSegments( mdfi );								// ブートプログラムの再配置を行う。
	
#endif
}


/*---------------------------------------------------------------------------*
  Name:         MBi_fifo_callback_arm7

  Description:  専用 PXI タグ経由でローダー起動

  Arguments:    

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void MBi_fifo_callback_arm7(PXIFifoTag tag, u32 msg_adr, BOOL err)
{
#pragma unused( err )
	
	if (tag == PXI_FIFO_TAG_MB && msg_adr == (u32)MB_TRIGGER_SIGNAL_TO_ARM7)
	{
		if ( loader_precallback ) {
			(*loader_precallback)();
		}
	}
}

#endif	/* defined(SDK_ARM7) */


/*---------------------------------------------------------------------------*
  Name:         LOADER_Start

  Description:  ローダーのスタート

  Arguments:    None.

  Returns:      TRUE - success FALSE - failed
 *---------------------------------------------------------------------------*/


void LOADER_Start(void)
{

#if	defined(SDK_ARM9)
	int result;
	MBDownloadFileInfo  *mdfi    = (MBDownloadFileInfo*)MB_DOWNLOAD_FILEINFO_ADDRESS;
	MBParam				*p_param = (MBParam*)HW_WM_BOOT_BUF;
	
	// マルチブートの時はデバッガエントリに飛ばないようクリアする。
#ifdef SDK_SMALL_BUILD
	GetMovedInfoFromIPL1Addr()->isOnDebugger = 0;	// USG-WW-3rd & USG-China-2ndでは、GetSharedWorkAddr()->isOnDebuggerはパッチ領域とモロバッティングしているが、このルーチンがARM9のマルチブートルーチンで呼ばれた後で、ARM7でパッチ挿入ルーチンが呼ばれるので、大丈夫。
													// しかし、デバッガ版ビルド時には、参照する側のisOnDebuggerフラグがクリアされていなかったので、修正する。
#endif // SDK_SMALL_BUILD
	
	// システム領域へマルチブートフラグを書き込み
	p_param->boot_type = MB_TYPE_MULTIBOOT;
	// 親機情報をシステム領域へ書き込み
	MI_CpuCopy8((void*)MB_BSSDESC_ADDRESS, &p_param->parent_bss_desc, MB_BSSDESC_SIZE);
	
	// ARM7側へローダー起動を通知
	result = PXI_SendWordByFifo( PXI_FIFO_TAG_MB, (u32)MB_TRIGGER_SIGNAL_TO_ARM7, FALSE );
	SDK_ASSERTMSG((result >= 0), "ARM9:FIFO SEND ERROR!\n");
	
	return;
	
#else /* defined(SDK_ARM9) */
	
	// ローダーをコール
	LOADERi_Jump();
	
#endif
}


/*---------------------------------------------------------------------------*
  Name:         LOADER_Init

  Description:  ローダーの初期化

  Arguments:    callback - ブートの準備が出来たときに返すコールバック

  Returns:      None.
 *---------------------------------------------------------------------------*/

void LOADER_Init(MB_LoaderCallback callback)
{
	MBParam	*p_param = (MBParam*)HW_WM_BOOT_BUF;
	
	PXI_Init();	// 初期化されていなかったら、初期化処理を行う
	
#if	defined(SDK_ARM7)
	
	loader_precallback = callback;
	
	/* ブートフラグの補正(マルチブートフラグが指定されていない場合はROMとみなす。) */
	if (p_param->boot_type != MB_TYPE_MULTIBOOT) {
		p_param->boot_type = MB_TYPE_NORMAL;
	}
	/* 上の処理で, 必ず MB_TYPE_MULTIBOOT か MB_TYPE_NORMAL になる */
	
	// マルチブート監視FIFOコールバックをセット
	PXI_SetFifoRecvCallback( PXI_FIFO_TAG_MB, MBi_fifo_callback_arm7 );
	
#else	/* defined(SDK_ARM7) */
	
	#pragma unused(callback)
	
#endif
}



/*----------------------------------------------------------------------------*
/* 現状 不使用
 *----------------------------------------------------------------------------*/

#if	defined(LOADER_USE_OVT_BUF)

//----------------------------------------------------------------------
// オーバーレイテーブル
//----------------------------------------------------------------------
typedef struct {
	u32			id;							// オーバーレイ ID 
	void		*ram_address;				// ロード先頭位置 
	u32			ram_size;					// ロードサイズ 
	u32			bss_size;					// bss 領域サイズ 
	void		*sinit_init;				// static initializer 先頭アドレス 
	void		*sinit_init_end;			// static initializer 最終アドレス
	u32			file_id;					// オーバーレイファイルID
	u32			rsv;						// 予約。
} ROM_OVT;


// OVTの最大サイズ（これについては再考の余地あり）
#define MB_OVT_MAX_SIZE		MB_COMM_BLOCKSIZE

// Overlay Table Buffer
// マルチブートするプログラム上で、スタティックイニシャライザを起動するのに必要
// オーバーレイテーブル数で容量が変わってくる。
// IPL2における仕様が固まるまで、ここに置いておく
static u32 mb_ovt_buf[MB_OVT_MAX_SIZE/sizeof(u32)];

static void MB_SetOverlayTable(ROM_OVT *srcp, u16 sec_num)
{
	if (srcp && sec_num)
	{
		MI_CpuCopy8((void*)srcp, (void*)mb_ovt_buf, sec_num*sizeof(ROM_OVT));
	}
}

#endif
