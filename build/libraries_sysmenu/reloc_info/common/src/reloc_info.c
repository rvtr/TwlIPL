/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     reloc_info.c

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

#include <sysmenu/sysmenu_lib.h>

// const data------------------------------------------------------------------

typedef struct RomSegmentRange {
	u32		start;
	u32		end;
}RomSegmentRange;

static RomSegmentRange romSegmentRange[RELOCATE_INFO_NUM] = {
	{ SYSM_TWL_ARM9_LOAD_MMEM,     SYSM_TWL_ARM9_LOAD_MMEM_END },
	{ SYSM_TWL_ARM7_LOAD_MMEM,     SYSM_TWL_ARM7_LOAD_MMEM_END },
	{ SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END },
	{ SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END },
};

static RomSegmentRange romSegmentRangeNitro[RELOCATE_INFO_NUM] = {
	{ SYSM_NTR_ARM9_LOAD_MMEM,     SYSM_NTR_ARM9_LOAD_MMEM_END },
	{ SYSM_NTR_ARM7_LOAD_MMEM,     SYSM_NTR_ARM7_LOAD_MMEM_END },
	{ SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END },
	{ SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END },
};

static u32 load_region_check_list[RELOCATE_INFO_NUM][RELOCATE_INFO_NUM * 2 - 1] = 
{
	{SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END, 
	SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END, 
	SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	SYSM_TWL_ARM7_LTD_LOAD_MMEM, SYSM_TWL_ARM7_LTD_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, SYSM_TWL_ARM9_LTD_LOAD_MMEM_END, 
	NULL }, 
};

// ============================================================================
//
// �Ĕz�u��񐶐�
//
// ============================================================================

// ROM�̃��[�h��̈���`�F�b�N���A�Ĕz�u�̕K�v������΃��[�h��A�h���X��ύX���A�Ĕz�u����*info�ɃZ�b�g����B
// ���[�h��̈悪����Ă͂����Ȃ��̈�̃��X�gcheck_dest��{�J�n, �I��, �J�n�Q, �I���Q, �c�c, NULL}�̌`���B
// �Ĕz�u�̗L�薳���Ɋւ�炸���[�h�\�Ȃ��TRUE�A���[�h�s�\�Ȃ��FALSE��Ԃ�
// �Ĕz�u���K�v�Ȃ��ꍇ�A�Ĕz�u����src,dest,length�ɂ͂��ꂼ��NULL����������B
static BOOL SYSMi_CheckLoadRegionAndSetRelocateInfoEx
( u32 *dest, u32 length, RomSegmentRange default_region, u32 *check_dest, Relocate_Info *info )
{
	u32 ori_len = length;
	length = MATH_ROUNDUP( length, SYSM_ALIGNMENT_LOAD_MODULE );// AES�Í����̈�̊֌W�ŁA�Ĕz�u�K�v���̃`�F�b�N�Ɏg��length��32�o�C�g�A���C�����g�ɕ␳
	MI_CpuClearFast( info, sizeof(Relocate_Info) );
	if( default_region.end - default_region.start < length ) return FALSE;// �T�C�Y�I�[�o�[
	if( !( default_region.start <= *dest && *dest + length <= default_region.end ) )
	{
		// �Ĕz�u�̕K�v����
		while( *check_dest != NULL )
		{
			if( check_dest[0] < *dest + length && *dest < check_dest[1] ) return FALSE;// �`�F�b�N�̈�ɔ������NG
			check_dest += 2;
		}
		
		// ����������length�łȂ�ori_len���g�p
		
		// �����܂ŗ��Ă���΍Ĕz�u��
		// ����R�s�[�t���OOFF
		info->rev = FALSE;
		if( default_region.start < *dest + ori_len && *dest + ori_len <= default_region.end )
		{
			// �f�t�H���g�z�u�̈�̐擪���ɁA�Ĕz�u��̌㕔������Ă���
			// �|�X�g�N���A���
			info->post_clear_addr = *dest + ori_len;
			info->post_clear_length = default_region.end - (*dest + ori_len);
		}
		else if( default_region.start <= *dest && *dest < default_region.end )
		{
			// �f�t�H���g�z�u�̈�̌㕔�ɁA�Ĕz�u��̐擪��������Ă���
			// �|�X�g�N���A���
			info->post_clear_addr = default_region.start;
			info->post_clear_length = *dest - default_region.start;
			if( *dest < default_region.start + ori_len )
			{
				// �X�ɁA�f�t�H���g�z�u�̈�Ƀ��[�h�����f�[�^�̍Ō���ƍĔz�u��̐擪��������Ă���
				// ����R�s�[�t���OON
				info->rev = TRUE;
			}
		}else
		{
			// �܂���������Ă��Ȃ�
			// �|�X�g�N���A���
			info->post_clear_addr = default_region.start;
			info->post_clear_length = default_region.end - default_region.start;
		}
		info->src = default_region.start;
		info->dest = *dest;
		info->length = ori_len;
		*dest = default_region.start;
	}else
	{
		// �Ĕz�u�̕K�v�Ȃ�
	}
	return TRUE;
}

// SYSMi_CheckLoadRegionAndSetRelocateInfoEx�̃��b�p�[�֐�
BOOL SYSM_CheckLoadRegionAndSetRelocateInfo( RomSegmentName seg, u32 *dest, u32 length, Relocate_Info *info, BOOL isTwlApp)
{
	RomSegmentRange *rsr;
    if( isTwlApp )
    {
		rsr = romSegmentRange;
	}else
	{
		//NTR��p
		rsr = romSegmentRangeNitro;
	}
	return SYSMi_CheckLoadRegionAndSetRelocateInfoEx(dest, length, rsr[seg], load_region_check_list[seg], info);
}
