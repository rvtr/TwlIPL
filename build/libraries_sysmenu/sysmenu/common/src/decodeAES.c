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
#include <twl/aes/common/types.h>

// define data-----------------------------------------------------------------
#define PXI_FIFO_DATA_DECRYPTAES_W_INIT		0
#define PXI_FIFO_DATA_DECRYPTAES_W_TARGET1	1
#define PXI_FIFO_DATA_DECRYPTAES_W_TARGET2	2
#define PXI_FIFO_DATA_DECRYPTAES_NORMAL		3
#define SYSM_DECODE_AES_MESSAGE_ARRAY_MAX   8
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
#ifdef SDK_ARM9
static BOOL s_finished = FALSE;
static void *s_Addr_AESregion[2];
static u32 s_Size_AESregion[2];
static BOOL s_initialized = FALSE;
static u8 s_initCounterAES[2][AES_BLOCK_SIZE];
static OSMessageQueue  msgQ4arm9;
static OSMessage       msgArray4arm9[SYSM_DECODE_AES_MESSAGE_ARRAY_MAX];
#else
static OSMessageQueue  msgQ4arm7;
static OSMessage       msgArray4arm7[SYSM_DECODE_AES_MESSAGE_ARRAY_MAX];
#endif
// const data------------------------------------------------------------------

#ifdef SDK_ARM9
#include <twl/aes/ARM9/aes_internal.h>

// WRAM経由ファイル読み込みのコールバックで使うAESデクリプト処理の初期化
BOOL SYSM_InitDecryptAESRegion_W( ROM_Header_Short *hs )
{
	// AESデクリプト領域アドレスの算出と保存
	// ヘッダ情報から、AESデクリプトが必要な領域のアドレスとサイズ
	// （最終的な配置ではなく、ランチャーによってアプリがメモリにロードされた状態でのアドレス）
	// を算出、保存する
	// ここで保存したアドレスを使って、順次読み込んだ領域と比較しながらデクリプト処理を行う
	// カードにも対応する場合、セキュア領域の退避についても考慮する必要あり
	// ついでに7側に飛んで、鍵やらIDのセットも済ませておく
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
		OS_TPrintf( "SYSM_InitDecryptAESRegion(arm9):AES disabled.\n" );
		s_initialized = FALSE;
		return FALSE;
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
	
	// デクリプト情報算出＆格納処理
	region_offset[0] = hs->aes_target_rom_offset;
	region_offset[1] = hs->aes_target2_rom_offset;
	region_size[0] = hs->aes_target_size;
	region_size[1] = hs->aes_target2_size;
	s_Addr_AESregion[0] = NULL;
	s_Addr_AESregion[1] = NULL;
	for( m=0; m<2; m++ )
	{
		// デクリプトする暗号化領域の判定とオフセット計算（ARM9flx, ARM9ltd, ARM7flx, ARM7ltdのどれか）
		region_addr[m] = NULL;
		if( region_size[m] == 0 )
		{
			continue;
		}
		for( l=0; l<4; l++ )
		{
			// モジュール配置先の領域（サイズは32バイト単位に補正）に、復号指定領域が含まれているか？
			if( module_offset[l] <= region_offset[m] &&
				region_offset[m] + region_size[m] <= module_offset[l] + MATH_ROUNDUP( module_size[l], SYSM_ALIGNMENT_LOAD_MODULE ) )
			{
				region_addr[m] = (void *)( (u32)module_addr[l] + (region_offset[m] - module_offset[l]) );
				break;
			}
		}
		if( region_addr[m] == NULL )
		{
			continue;
		}
		
		// Workに暗号化領域情報を格納
		s_Addr_AESregion[m] = region_addr[m];
		s_Size_AESregion[m] = region_size[m];
		// [TODO:緊急]AES領域がカードのセキュア領域に被った場合の処理
	}
	
	if(region_addr[0] == NULL && region_addr[1] == NULL)
	{
		// ターゲット１も２も存在しないor設定オフセットがおかしい
		OS_TPrintf( "SYSM_InitDecryptAESRegion(arm9):No targets.\n" );
		s_initialized = FALSE;
		return FALSE;
	}

	// Workに開発/製品情報を格納
	SYSMi_GetWork()->isDeveloperAESMode = ( hs->developer_encrypt ? TRUE : FALSE );
	
	// カウンタの初期値記録
	MI_CpuCopy8( hs->main_static_digest, s_initCounterAES[0], AES_BLOCK_SIZE );	// 領域1初期値
	MI_CpuCopy8( hs->sub_static_digest, s_initCounterAES[1], AES_BLOCK_SIZE );	// 領域2初期値

	// Workに「鍵」or「シードとゲームコード」をセット
	if( hs->developer_encrypt )
	{
		MI_CpuCopy8( hs->title_name, SYSMi_GetWork()->keyAES, AES_KEY_SIZE );
	}else
	{
		MI_CpuCopy8( hs->main_ltd_static_digest, SYSMi_GetWork()->seedAES, AES_KEY_SIZE );
		MI_CpuCopy8( hs->game_code, SYSMi_GetWork()->idAES, GAME_CODE_MAX );
	}
	
	// ARM7に開始通知
	s_finished = FALSE;
	while( PXI_SendWordByFifo(PXI_FIFO_TAG_DECRYPTAES, PXI_FIFO_DATA_DECRYPTAES_W_INIT, FALSE) != PXI_FIFO_SUCCESS )
    {
    	OS_TPrintf( "SYSM_InitDecryptAESRegion(arm9):ARM9 PXI send error.\n" );
    }
    
	// ARM7からの完了通知を受け取って完了
	OS_ReceiveMessage(&msgQ4arm9, (OSMessage*)&s_finished, OS_MESSAGE_BLOCK);
	
	OS_TPrintf( "SYSM_InitDecryptAESRegion(arm9):Init finished.\n" );
	s_initialized = TRUE;
	return TRUE;
}

// WRAM経由ファイル読み込みのコールバックで使うAESデクリプト処理関数
// 注意：キャッシュケア済み、WRAMが7に倒れ済みである事を前提とする
void SYSM_StartDecryptAESRegion_W( const void *wram_addr, const void *orig_addr, u32 size )
{
	int l;
	if( !s_initialized )
	{
		return;
	}
	
	SYSMi_GetWork()->addr_AESregion[0] = NULL;
	SYSMi_GetWork()->addr_AESregion[1] = NULL;
	// target1と2について両方調べる
	for( l=0;l<2;l++ )
	{
		u32 start;
		u32 end;
		
		// AESデクリプト領域と、ファイルからWRAMに読み込んできた領域の目的地の比較
		if( ( ((u32)orig_addr + size) < (u32)s_Addr_AESregion[l] ) ||
		    ( ((u32)s_Addr_AESregion[l] + s_Size_AESregion[l]) < (u32)orig_addr )
		  )
		{
			continue;
		}
		
		// 上で比較した結果、共通部分があればデクリプトする領域なので括り出す
		start = ( orig_addr < s_Addr_AESregion[l] ) ? (u32)s_Addr_AESregion[l] : (u32)orig_addr;
		end = ( (u32)orig_addr + size < (u32)s_Addr_AESregion[l] + s_Size_AESregion[l] ) ?
				(u32)orig_addr + size : (u32)s_Addr_AESregion[l] + s_Size_AESregion[l];
				
		// デクリプトする領域のアドレスとサイズ（WRAMに読み込んだ状態でのアドレスとサイズ）をWORKに記録
		SYSMi_GetWork()->addr_AESregion[l] = (void *)( (s32)wram_addr + (start - (u32)orig_addr) );
		SYSMi_GetWork()->size_AESregion[l] = end - start;
		
		// 対象となる領域をデクリプトする際に必要なカウンタの値も算出してWORKに入れておく
		MI_CpuCopy8( s_initCounterAES[l], SYSMi_GetWork()->counterAES[l], AES_BLOCK_SIZE );
		AESi_AddToCounter( (AESCounter *)SYSMi_GetWork()->counterAES[l], (start - (u32)s_Addr_AESregion[l]) / AES_BLOCK_SIZE );
//		OS_TPrintf( "SYSM_StartDecryptAESRegion_W(arm9):wramaddr:0x%0.8x start:0x%0.8x end:0x%0.8x counter offset: %d.\n",wram_addr, start,end,(start - (u32)s_Addr_AESregion[l]) / AES_BLOCK_SIZE );

		// 7にデクリプトしてもらう（target1なのか2なのかFIFOで送る）
		s_finished = FALSE;
		while( PXI_SendWordByFifo(PXI_FIFO_TAG_DECRYPTAES, (u32)(PXI_FIFO_DATA_DECRYPTAES_W_TARGET1 + l), FALSE) != PXI_FIFO_SUCCESS )
	    {
	    	OS_TPrintf( "SYSM_StartDecryptAESRegion_W(arm9):ARM9 PXI send error.\n" );
	    }
		// ARM7からの完了通知を受け取って完了
		OS_ReceiveMessage(&msgQ4arm9, (OSMessage*)&s_finished, OS_MESSAGE_BLOCK);
	}
}

// べた書きAESデクリプト処理
void SYSM_StartDecryptAESRegion( ROM_Header_Short *hs )
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
		OS_TPrintf( "SYSM_StartDecryptAESRegion(arm9):AES disabled.\n" );
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
	
	// デクリプト情報算出＆格納処理
	region_offset[0] = hs->aes_target_rom_offset;
	region_offset[1] = hs->aes_target2_rom_offset;
	region_size[0] = hs->aes_target_size;
	region_size[1] = hs->aes_target2_size;
	SYSMi_GetWork()->addr_AESregion[0] = NULL;
	SYSMi_GetWork()->addr_AESregion[1] = NULL;
	for( m=0; m<2; m++ )
	{
		// デクリプトする暗号化領域の判定とオフセット計算（ARM9flx, ARM9ltd, ARM7flx, ARM7ltdのどれか）
		region_addr[m] = NULL;
		if( region_size[m] == 0 )
		{
			continue;
		}
		for( l=0; l<4; l++ )
		{
			// モジュール配置先の領域（サイズは32バイト単位に補正）に、復号指定領域が含まれているか？
			if( module_offset[l] <= region_offset[m] &&
				region_offset[m] + region_size[m] <= module_offset[l] + MATH_ROUNDUP( module_size[l], SYSM_ALIGNMENT_LOAD_MODULE ) )
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
		
		// [TODO:緊急]AES領域がカードのセキュア領域に被った場合の処理
	}
	
	if(region_addr[0] == NULL && region_addr[1] == NULL)
	{
		// ターゲット１も２も存在しないor設定オフセットがおかしい
		OS_TPrintf( "SYSM_StartDecryptAESRegion(arm9):No targets.\n" );
		return;
	}

	// Workに開発/製品情報を格納
	SYSMi_GetWork()->isDeveloperAESMode = ( hs->developer_encrypt ? TRUE : FALSE );

	// Workにカウンタの初期値セット
	MI_CpuCopy8( hs->main_static_digest, SYSMi_GetWork()->counterAES[0], AES_BLOCK_SIZE );	// 領域1初期値
	MI_CpuCopy8( hs->sub_static_digest, SYSMi_GetWork()->counterAES[1], AES_BLOCK_SIZE );	// 領域2初期値

	// Workに「鍵」or「シードとゲームコード」をセット
	if( hs->developer_encrypt )
	{
		MI_CpuCopy8( hs->title_name, SYSMi_GetWork()->keyAES, AES_KEY_SIZE );
	}else
	{
		MI_CpuCopy8( hs->main_ltd_static_digest, SYSMi_GetWork()->seedAES, AES_KEY_SIZE );
		MI_CpuCopy8( hs->game_code, SYSMi_GetWork()->idAES, GAME_CODE_MAX );
	}
	
	// ARM7に開始通知
	s_finished = FALSE;
	while( PXI_SendWordByFifo(PXI_FIFO_TAG_DECRYPTAES, PXI_FIFO_DATA_DECRYPTAES_NORMAL, FALSE) != PXI_FIFO_SUCCESS )
    {
    	OS_TPrintf( "SYSM_StartDecryptAESRegion(arm9):ARM9 PXI send error.\n" );
    }
    
	// ARM7からの完了通知を受け取って完了
	OS_ReceiveMessage(&msgQ4arm9, (OSMessage*)&s_finished, OS_MESSAGE_BLOCK);
	
	OS_TPrintf( "SYSM_StartDecryptAESRegion(arm9):AES decryption finished.\n" );
}

#else //SDK_ARM7

#include <twl/aes/ARM7/hi.h>
#include <twl/aes/ARM7/lo.h>
#include <firm/aes/ARM7/aes_init.h>

static AESCounter   aesCounter;

#define DMA_SEND         1
#define DMA_RECV         3
static void ReplaceWithAes( void* ptr, u32 size )
{
    AES_Lock();
    AES_Reset();
    AES_Reset();
    AES_WaitKey();
    AES_LoadKey( AES_KEY_SLOT_A );
    AES_WaitKey();
    AES_DmaSend( DMA_SEND, ptr,  size, NULL, NULL );
    AES_DmaRecv( DMA_RECV, ptr, size, NULL, NULL );
    AES_SetCounter( &aesCounter );
    AES_Run( AES_MODE_CTR, 0, size / AES_BLOCK_SIZE, NULL, NULL );
    AES_AddToCounter( &aesCounter, size / AES_BLOCK_SIZE );
    MI_WaitNDma( DMA_RECV );
    AES_Unlock();
}

static void SYSMi_SetKeys( void )
{
	if ( !SYSMi_GetWork()->isDeveloperAESMode )
	{
		AESi_InitKeysForApp( SYSMi_GetWork()->idAES );
	}
	
	// 開発なら鍵セット、製品ならシードセット
    AES_Lock();
    AES_WaitKey();
    if ( SYSMi_GetWork()->isDeveloperAESMode )
    {
        AES_SetKeyA( (AESKey*)SYSMi_GetWork()->keyAES );
    }
    else
    {
        AES_SetKeySeedA( (AESKeySeed*)SYSMi_GetWork()->seedAES );
    }
    AES_Unlock();
}

static void SYSMi_DecryptAESRegion_sub( int target )
{
	if( SYSMi_GetWork()->addr_AESregion[target]==NULL )
	{
		OS_TPrintf( "SYSMi_DecryptAESRegion_sub(arm7):Target%d Addr Error!\n",target+1 );
		return;
	}
	
	// カウンタの初期値セット
	MI_CpuCopy8( SYSMi_GetWork()->counterAES[target], &aesCounter, AES_BLOCK_SIZE );
	
	// 鍵ロードして暗号化領域の復号開始
	ReplaceWithAes( SYSMi_GetWork()->addr_AESregion[target], MATH_ROUNDUP( SYSMi_GetWork()->size_AESregion[target] ,32 ) );
//	OS_TPrintf( "SYSMi_DecryptAESRegion_sub(arm7):target:%d addr:0x%0.8x size:0x%x\n",target+1, SYSMi_GetWork()->addr_AESregion[target], SYSMi_GetWork()->size_AESregion[target] );
}

#endif //ifdef SDK_ARM9

static void SYSMi_CallbackDecryptAESRegion(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(data)
#pragma unused(err)

#ifdef SDK_ARM9
	if(!OS_SendMessage(&msgQ4arm9, (OSMessage)TRUE, OS_MESSAGE_NOBLOCK))
	{
		OS_TPrintf( "SYSMi_CallbackDecryptAESRegion(arm9):Message send error.\n" );
	}
#else //SDK_ARM7
	if(!OS_SendMessage(&msgQ4arm7, (OSMessage)data, OS_MESSAGE_NOBLOCK))
	{
		OS_TPrintf( "SYSMi_CallbackDecryptAESRegion(arm7):Message send error.\n" );
	}
#endif //ifdef SDK_ARM9
}

#ifdef SDK_ARM7
#define SYSM_AES_THREAD_STACK_SIZE   512
#define AES_THREAD_PRIORITY 5
static OSThread aes_thread;
static u64 aes_thread_stack[SYSM_AES_THREAD_STACK_SIZE/sizeof(u64)];

static void SYSMi_DecryptAESThread(void* arg)
{
#pragma unused(arg)
	int aes_start;
	while(1)
	{
		OS_ReceiveMessage(&msgQ4arm7, (OSMessage*)&aes_start, OS_MESSAGE_BLOCK);
		
		if(aes_start == PXI_FIFO_DATA_DECRYPTAES_W_INIT)
		{
			SYSMi_SetKeys();
		}else if(aes_start == PXI_FIFO_DATA_DECRYPTAES_W_TARGET1 || aes_start == PXI_FIFO_DATA_DECRYPTAES_W_TARGET2)
		{
			int target = (int)(aes_start - PXI_FIFO_DATA_DECRYPTAES_W_TARGET1);
			SYSMi_DecryptAESRegion_sub( target );
		}else if(aes_start == PXI_FIFO_DATA_DECRYPTAES_NORMAL)
		{
			int l;
			SYSMi_SetKeys();
		    for( l=0; l<2; l++ )
		    {
				SYSMi_DecryptAESRegion_sub( l );
			}
		}
		
		// ARM9に完了通知
		while( PXI_SendWordByFifo(PXI_FIFO_TAG_DECRYPTAES, 0, FALSE) != PXI_FIFO_SUCCESS )
	    {
	    	OS_TPrintf( "SYSM_StartDecryptAESRegion(arm7):ARM7 PXI send error.\n" );
	    }
	}
}
#endif

void SYSM_InitDecryptAESPXICallback( void )
{
	PXI_SetFifoRecvCallback(PXI_FIFO_TAG_DECRYPTAES, SYSMi_CallbackDecryptAESRegion);
#ifdef SDK_ARM9
	OS_InitMessageQueue(&msgQ4arm9, msgArray4arm9, SYSM_DECODE_AES_MESSAGE_ARRAY_MAX);
#else
	OS_InitMessageQueue(&msgQ4arm7, msgArray4arm7, SYSM_DECODE_AES_MESSAGE_ARRAY_MAX);
	// ARM7側の処理を行うためのスレッド開始
    OS_CreateThread(&aes_thread, SYSMi_DecryptAESThread, 0,
                    (void*)(aes_thread_stack + (SYSM_AES_THREAD_STACK_SIZE/sizeof(u64))),
                    SYSM_AES_THREAD_STACK_SIZE, AES_THREAD_PRIORITY);
    OS_WakeupThreadDirect(&aes_thread);
#endif
}