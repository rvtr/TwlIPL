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
// 再配置情報生成
//
// ============================================================================

// ROMのロード先領域をチェックし、再配置の必要があればロード先アドレスを変更し、再配置情報を*infoにセットする。
// ロード先領域が被ってはいけない領域のリストcheck_destは{開始, 終了, 開始２, 終了２, ……, NULL}の形式。
// 再配置の有り無しに関わらずロード可能ならばTRUE、ロード不可能ならばFALSEを返す
// 再配置が必要ない場合、再配置情報のsrc,dest,lengthにはそれぞれNULLが代入される。
static BOOL SYSMi_CheckLoadRegionAndSetRelocateInfoEx
( u32 *dest, u32 length, RomSegmentRange default_region, u32 *check_dest, Relocate_Info *info )
{
	u32 ori_len = length;
	length = MATH_ROUNDUP( length, SYSM_ALIGNMENT_LOAD_MODULE );// AES暗号化領域の関係で、再配置必要性のチェックに使うlengthは32バイトアライメントに補正
	MI_CpuClearFast( info, sizeof(Relocate_Info) );
	if( default_region.end - default_region.start < length ) return FALSE;// サイズオーバー
	if( !( default_region.start <= *dest && *dest + length <= default_region.end ) )
	{
		// 再配置の必要あり
		while( *check_dest != NULL )
		{
			if( check_dest[0] < *dest + length && *dest < check_dest[1] ) return FALSE;// チェック領域に被ったらNG
			check_dest += 2;
		}
		
		// ここから先はlengthでなくori_lenを使用
		
		// ここまで来ていれば再配置可
		// 後方コピーフラグOFF
		info->rev = FALSE;
		if( default_region.start < *dest + ori_len && *dest + ori_len <= default_region.end )
		{
			// デフォルト配置領域の先頭部に、再配置先の後部が被っている
			// ポストクリア情報
			info->post_clear_addr = *dest + ori_len;
			info->post_clear_length = default_region.end - (*dest + ori_len);
		}
		else if( default_region.start <= *dest && *dest < default_region.end )
		{
			// デフォルト配置領域の後部に、再配置先の先頭部が被っている
			// ポストクリア情報
			info->post_clear_addr = default_region.start;
			info->post_clear_length = *dest - default_region.start;
			if( *dest < default_region.start + ori_len )
			{
				// 更に、デフォルト配置領域にロードしたデータの最後尾と再配置先の先頭部が被っている
				// 後方コピーフラグON
				info->rev = TRUE;
			}
		}else
		{
			// まったく被っていない
			// ポストクリア情報
			info->post_clear_addr = default_region.start;
			info->post_clear_length = default_region.end - default_region.start;
		}
		info->src = default_region.start;
		info->dest = *dest;
		info->length = ori_len;
		*dest = default_region.start;
	}else
	{
		// 再配置の必要なし
	}
	return TRUE;
}

// SYSMi_CheckLoadRegionAndSetRelocateInfoExのラッパー関数
BOOL SYSM_CheckLoadRegionAndSetRelocateInfo( RomSegmentName seg, u32 *dest, u32 length, Relocate_Info *info, BOOL isTwlApp)
{
	RomSegmentRange *rsr;
    if( isTwlApp )
    {
		rsr = romSegmentRange;
	}else
	{
		//NTR専用
		rsr = romSegmentRangeNitro;
	}
	return SYSMi_CheckLoadRegionAndSetRelocateInfoEx(dest, length, rsr[seg], load_region_check_list[seg], info);
}
