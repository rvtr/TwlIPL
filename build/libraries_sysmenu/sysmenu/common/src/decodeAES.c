/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     decodeAES.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: #$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------

#ifdef SDK_ARM9

void SYSM_StartDecodeAESRegion( ROM_Header_Short *hs )
{
	void *region_addr[2];
	u32 region_size[2];
	u32 region_offset[2];
	u32 module_offset[4];
	u32 module_size[4];
	void *module_addr[4];
	int l, m;
	
	// AES有効？
	if( !hs->enable_aes )
	{
		return;
	}
	
	module_offset[0] = hs->main_rom_offset;
	module_offset[1] = hs->sub_rom_offset;
	module_offset[2] = hs->main_ltd_rom_offset;
	module_offset[3] = hs->sub_ltd_rom_offset;
	module_size[0] = hs->main_size;
	module_size[1] = hs->sub_size;
	module_size[2] = hs->main_ltd_size;
	module_size[3] = hs->sub_ltd_size;
	module_addr[0] = hs->main_ram_address;
	module_addr[1] = hs->sub_ram_address;
	module_addr[2] = hs->main_ltd_ram_address;
	module_addr[3] = hs->sub_ltd_ram_address;
	
	// 再配置情報があれば、モジュールは再配置情報のsrcのアドレスに格納されている
	for( l=0; l<RELOCATE_INFO_NUM ; l++ )
	{
		if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL )
		{
			module_addr[l] = (void *)SYSMi_GetWork()->romRelocateInfo[l].src;
		}
	}
	
	// デコード情報算出＆格納処理
	region_offset[0] = hs->aes_target_rom_offset;
	region_offset[1] = hs->aes_target2_rom_offset;
	region_size[0] = hs->aes_target_size;
	region_size[1] = hs->aes_target2_size;
	SYSMi_GetWork()->addr_AESregion[0] = NULL;
	SYSMi_GetWork()->addr_AESregion[1] = NULL;
	for( m=0; m<2; m++ )
	{
		// デコードする暗号化領域の判定とオフセット計算（ARM9flx, ARM9ltd, ARM7flx, ARM7ltdのどれか）
		region_addr[m] = NULL;
		if( region_size[m] == 0 )
		{
			continue;
		}
		for( l=0; l<4; l++ )
		{
			if( module_offset[l] <= region_offset[m] &&
				region_offset[m] < module_offset[l] + module_size[l] )
			{
				region_addr[m] = (void *)( (u32)module_addr[l] + (region_offset[m] - module_offset[l]) );
				break;
			}
		}
		if( region_addr[m] == NULL )
		{
			continue;
		}
		
		// 領域のキャッシュをフラッシュ
		DC_FlushRange( region_addr[m], region_size[m] );
		
		// Workに暗号化領域情報を格納
		SYSMi_GetWork()->addr_AESregion[m] = region_addr[m];
		SYSMi_GetWork()->size_AESregion[m] = region_size[m];
	}
	
	if(region_addr[0] == NULL && region_addr[1] == NULL)
	{
		// ターゲット１も２も存在しないor設定オフセットがおかしい
		return;
	}

	// Workに開発/製品情報を格納
	SYSMi_GetWork()->isDeveloperAESMode = ( hs->developer_encrypt ? TRUE : FALSE );
	
	// [TODO:]ARM7に開始通知
	// [TODO:]ARM7からの完了通知を受け取って完了
}

#else //SDK_ARM7

void SYSM_WaitDecodeAESRegion( void )
{
	// [TODO:]ARM9からの通知を受け取ったら開始
	// [TODO:]開発なら鍵セット、製品ならシードとIDセット
	// [TODO:]鍵ロードして暗号化領域の復号開始
	// [TODO:]一応、暗号化領域のキャッシュをフラッシュ
	// [TODO:]ARM9に完了通知
}

#endif //ifdef SDK_ARM9
