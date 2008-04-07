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
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static BOOL s_finished = FALSE;
// const data------------------------------------------------------------------

#ifdef SDK_ARM9

void SYSM_StartDecryptAESRegion( ROM_Header_Short *hs )
{
	void *region_addr[2];
	u32 region_size[2];
	u32 region_offset[2];
	u32 module_offset[4];
	u32 module_size[4];
	void *module_addr[4];
	int l, m;
	
	// AES�L���H
	if( !hs->enable_aes )
	{
		OS_TPrintf( "SYSM_StartDecryptAESRegion:AES disabled.\n" );
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
	
	// �Ĕz�u��񂪂���΁A���W���[���͍Ĕz�u����src�̃A�h���X�Ɋi�[����Ă���
	for( l=0; l<RELOCATE_INFO_NUM ; l++ )
	{
		if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL )
		{
			module_addr[l] = (void *)SYSMi_GetWork()->romRelocateInfo[l].src;
		}
	}
	
	// �f�N���v�g���Z�o���i�[����
	region_offset[0] = hs->aes_target_rom_offset;
	region_offset[1] = hs->aes_target2_rom_offset;
	region_size[0] = hs->aes_target_size;
	region_size[1] = hs->aes_target2_size;
	SYSMi_GetWork()->addr_AESregion[0] = NULL;
	SYSMi_GetWork()->addr_AESregion[1] = NULL;
	for( m=0; m<2; m++ )
	{
		// �f�N���v�g����Í����̈�̔���ƃI�t�Z�b�g�v�Z�iARM9flx, ARM9ltd, ARM7flx, ARM7ltd�̂ǂꂩ�j
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
		
		// �̈�̃L���b�V�����t���b�V��
		DC_FlushRange( region_addr[m], region_size[m] );
		
		// Work�ɈÍ����̈�����i�[
		SYSMi_GetWork()->addr_AESregion[m] = region_addr[m];
		SYSMi_GetWork()->size_AESregion[m] = region_size[m];
	}
	
	if(region_addr[0] == NULL && region_addr[1] == NULL)
	{
		// �^�[�Q�b�g�P���Q�����݂��Ȃ�or�ݒ�I�t�Z�b�g����������
		OS_TPrintf( "SYSM_StartDecryptAESRegion:No targets.\n" );
		return;
	}

	// Work�ɊJ��/���i�����i�[
	SYSMi_GetWork()->isDeveloperAESMode = ( hs->developer_encrypt ? TRUE : FALSE );
	
	// Work�Ɂu���vor�u�V�[�h�ƃQ�[���R�[�h�v���Z�b�g
	if( hs->developer_encrypt )
	{
		MI_CpuCopy8( hs->title_name, SYSMi_GetWork()->keyAES, AES_KEY_SIZE );
	}else
	{
		MI_CpuCopy8( hs->main_ltd_static_digest, SYSMi_GetWork()->seedAES, AES_KEY_SIZE );
		MI_CpuCopy8( hs->game_code, SYSMi_GetWork()->idAES, GAME_CODE_MAX );
	}
	
	// ARM7�ɊJ�n�ʒm
	s_finished = FALSE;
	while( PXI_SendWordByFifo(PXI_FIFO_TAG_DECRYPTAES, 0, FALSE) != PXI_FIFO_SUCCESS )
    {
    	OS_TPrintf( "SYSM_StartDecryptAESRegion:ARM9 PXI send error.\n" );
    }
    
	// ARM7����̊����ʒm���󂯎���Ċ���
	while( !s_finished )
	{
		OS_WaitAnyIrq();
	}
	
	OS_TPrintf( "SYSM_StartDecryptAESRegion:AES decryption succeed.\n" );
}

#else //SDK_ARM7

#include <twl/aes/ARM7/hi.h>
#include <twl/aes/ARM7/lo.h>
#include <firm/aes/ARM7/aes_init.h>

static BOOL         aesFlag;
static AESCounter   aesCounter;

#define DMA_SEND         2
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

#endif //ifdef SDK_ARM9

static void SYSMi_CallbackDecryptAESRegion(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(data)
#pragma unused(err)

#ifdef SDK_ARM9
	s_finished = TRUE;
#else //SDK_ARM7
	int l;
	
	// ���i�Ȃ�ID�Z�b�g
	if ( !SYSMi_GetWork()->isDeveloperAESMode )
	{
		AESi_InitKeysForApp( SYSMi_GetWork()->idAES );
	}
	
	// �J���Ȃ献�Z�b�g�A���i�Ȃ�V�[�h�Z�b�g
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
    
    for( l=0; l<2; l++ )
    {
		if( SYSMi_GetWork()->addr_AESregion[l]==NULL )
		{
    		OS_TPrintf( "SYSM_StartDecryptAESRegion:Region %d skip.\n", l );
			continue;
		}
		// �����[�h���ĈÍ����̈�̕����J�n
		ReplaceWithAes( SYSMi_GetWork()->addr_AESregion[l], SYSMi_GetWork()->size_AESregion[l] );
		// DMA�]���Ȃ̂ŃL���b�V���P�A�͕s�v�̂͂��c�cAES_DmaSend��Recv�̎d�l��v�m�F
	}
	
	// ARM9�Ɋ����ʒm
	while( PXI_SendWordByFifo(PXI_FIFO_TAG_DECRYPTAES, 0, FALSE) != PXI_FIFO_SUCCESS )
    {
    	OS_TPrintf( "SYSM_StartDecryptAESRegion:ARM7 PXI send error.\n" );
    }
	
#endif //ifdef SDK_ARM9
}

void SYSM_InitDecryptAESPXICallback( void )
{
	PXI_SetFifoRecvCallback(PXI_FIFO_TAG_DECRYPTAES, SYSMi_CallbackDecryptAESRegion);
}