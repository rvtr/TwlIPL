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


#include <nitro/mb.h>	// twl/mb.h���Ȃ��B
#include <mb_private.h>
#include <sysmenu/mb_loader.h>

// --------------------------------------------------------------------------

// Download���̃T�C�Y

#define MB_OVT_MAX_SIZE					MB_COMM_BLOCK_SIZE			// OVT�̍ő�T�C�Y�iBlock�T�C�Y���ő�T�C�Y�Ƃ���j

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

  Description:  ���[�_�[�pCpuCopy

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

  Description:  ARM9,ARM7�̊e�Z�O�����g��K�v�ɉ����čĔz�u����B

  Arguments:    mdfi

  Returns:      void
 *---------------------------------------------------------------------------*/

static void	LOADERi_LocateAllSegments( MBDownloadFileInfo *mdfi )
{
	MBi_SearchAndLocateSegmentInfo(mdfi, MI_PROCESSOR_ARM9);		// ARM9�Z�O�����g�ɂ��Ă̔z�u����
	MBi_SearchAndLocateSegmentInfo(mdfi, MI_PROCESSOR_ARM7);		// ARM7�Z�O�����g�ɂ��Ă̔z�u����
}


/* �w��̃Z�O�����g��������, �Ĕz�u���� */
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
#ifndef SDK_SMALL_BUILD												// ��IPL2�̏ꍇ�́A���̃������N���A��IPL2�ōs���B
					MIm_CpuClear32( 0, (void*)seg_info->recv_addr, seg_info->size );
#endif
				}
			}
		}
	}
}


/*---------------------------------------------------------------------------*
  Name:         LOADERi_Jump

  Description:  ���[�_�[���N�� (ARM7/9 ����)

  Arguments:    ����

  Returns:      void
 *---------------------------------------------------------------------------*/

static void	LOADERi_Jump(void)
{
#if	defined(SDK_ARM7)
	
	MBDownloadFileInfo *mdfi	= (MBDownloadFileInfo*)MB_DOWNLOAD_FILEINFO_ADDRESS;
	MBParam	*p_param			= (MBParam*)HW_WM_BOOT_BUF;
	
	if( p_param->boot_type != MB_TYPE_MULTIBOOT ) {					// �u�[�g�^�C�v���}���`�u�[�g�łȂ��ꍇ�́A����������TRUE���^�[���B
		return;
	}
	
	LOADERi_LocateAllSegments( mdfi );								// �u�[�g�v���O�����̍Ĕz�u���s���B
	
#endif
}


/*---------------------------------------------------------------------------*
  Name:         MBi_fifo_callback_arm7

  Description:  ��p PXI �^�O�o�R�Ń��[�_�[�N��

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

  Description:  ���[�_�[�̃X�^�[�g

  Arguments:    None.

  Returns:      TRUE - success FALSE - failed
 *---------------------------------------------------------------------------*/


void LOADER_Start(void)
{

#if	defined(SDK_ARM9)
	int result;
	MBDownloadFileInfo  *mdfi    = (MBDownloadFileInfo*)MB_DOWNLOAD_FILEINFO_ADDRESS;
	MBParam				*p_param = (MBParam*)HW_WM_BOOT_BUF;
	
	// �}���`�u�[�g�̎��̓f�o�b�K�G���g���ɔ�΂Ȃ��悤�N���A����B
#ifdef SDK_SMALL_BUILD
	GetMovedInfoFromIPL1Addr()->isOnDebugger = 0;	// USG-WW-3rd & USG-China-2nd�ł́AGetSharedWorkAddr()->isOnDebugger�̓p�b�`�̈�ƃ����o�b�e�B���O���Ă��邪�A���̃��[�`����ARM9�̃}���`�u�[�g���[�`���ŌĂ΂ꂽ��ŁAARM7�Ńp�b�`�}�����[�`�����Ă΂��̂ŁA���v�B
													// �������A�f�o�b�K�Ńr���h���ɂ́A�Q�Ƃ��鑤��isOnDebugger�t���O���N���A����Ă��Ȃ������̂ŁA�C������B
#endif // SDK_SMALL_BUILD
	
	// �V�X�e���̈�փ}���`�u�[�g�t���O����������
	p_param->boot_type = MB_TYPE_MULTIBOOT;
	// �e�@�����V�X�e���̈�֏�������
	MI_CpuCopy8((void*)MB_BSSDESC_ADDRESS, &p_param->parent_bss_desc, MB_BSSDESC_SIZE);
	
	// ARM7���փ��[�_�[�N����ʒm
	result = PXI_SendWordByFifo( PXI_FIFO_TAG_MB, (u32)MB_TRIGGER_SIGNAL_TO_ARM7, FALSE );
	SDK_ASSERTMSG((result >= 0), "ARM9:FIFO SEND ERROR!\n");
	
	return;
	
#else /* defined(SDK_ARM9) */
	
	// ���[�_�[���R�[��
	LOADERi_Jump();
	
#endif
}


/*---------------------------------------------------------------------------*
  Name:         LOADER_Init

  Description:  ���[�_�[�̏�����

  Arguments:    callback - �u�[�g�̏������o�����Ƃ��ɕԂ��R�[���o�b�N

  Returns:      None.
 *---------------------------------------------------------------------------*/

void LOADER_Init(MB_LoaderCallback callback)
{
	MBParam	*p_param = (MBParam*)HW_WM_BOOT_BUF;
	
	PXI_Init();	// ����������Ă��Ȃ�������A�������������s��
	
#if	defined(SDK_ARM7)
	
	loader_precallback = callback;
	
	/* �u�[�g�t���O�̕␳(�}���`�u�[�g�t���O���w�肳��Ă��Ȃ��ꍇ��ROM�Ƃ݂Ȃ��B) */
	if (p_param->boot_type != MB_TYPE_MULTIBOOT) {
		p_param->boot_type = MB_TYPE_NORMAL;
	}
	/* ��̏�����, �K�� MB_TYPE_MULTIBOOT �� MB_TYPE_NORMAL �ɂȂ� */
	
	// �}���`�u�[�g�Ď�FIFO�R�[���o�b�N���Z�b�g
	PXI_SetFifoRecvCallback( PXI_FIFO_TAG_MB, MBi_fifo_callback_arm7 );
	
#else	/* defined(SDK_ARM7) */
	
	#pragma unused(callback)
	
#endif
}



/*----------------------------------------------------------------------------*
/* ���� �s�g�p
 *----------------------------------------------------------------------------*/

#if	defined(LOADER_USE_OVT_BUF)

//----------------------------------------------------------------------
// �I�[�o�[���C�e�[�u��
//----------------------------------------------------------------------
typedef struct {
	u32			id;							// �I�[�o�[���C ID 
	void		*ram_address;				// ���[�h�擪�ʒu 
	u32			ram_size;					// ���[�h�T�C�Y 
	u32			bss_size;					// bss �̈�T�C�Y 
	void		*sinit_init;				// static initializer �擪�A�h���X 
	void		*sinit_init_end;			// static initializer �ŏI�A�h���X
	u32			file_id;					// �I�[�o�[���C�t�@�C��ID
	u32			rsv;						// �\��B
} ROM_OVT;


// OVT�̍ő�T�C�Y�i����ɂ��Ă͍čl�̗]�n����j
#define MB_OVT_MAX_SIZE		MB_COMM_BLOCKSIZE

// Overlay Table Buffer
// �}���`�u�[�g����v���O������ŁA�X�^�e�B�b�N�C�j�V�����C�U���N������̂ɕK�v
// �I�[�o�[���C�e�[�u�����ŗe�ʂ��ς���Ă���B
// IPL2�ɂ�����d�l���ł܂�܂ŁA�����ɒu���Ă���
static u32 mb_ovt_buf[MB_OVT_MAX_SIZE/sizeof(u32)];

static void MB_SetOverlayTable(ROM_OVT *srcp, u16 sec_num)
{
	if (srcp && sec_num)
	{
		MI_CpuCopy8((void*)srcp, (void*)mb_ovt_buf, sec_num*sizeof(ROM_OVT));
	}
}

#endif
