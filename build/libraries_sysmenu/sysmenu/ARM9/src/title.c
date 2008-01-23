/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.c

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
#include <sysmenu.h>
#include <es.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
#define CARD_BANNER_INDEX			( LAUNCHER_TITLE_LIST_NUM - 1 )

// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
static s32  ReadFile( FSFile* pf, void* buffer, s32 size );
static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle );
static void SYSMi_Relocate( void );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static OSThread			s_thread;
static TWLBannerFile	s_bannerBuf[ LAUNCHER_TITLE_LIST_NUM ] ATTRIBUTE_ALIGN(32);

// const data------------------------------------------------------------------
static const OSBootType s_launcherToOSBootType[ LAUNCHER_BOOTTYPE_MAX ] = {
    OS_BOOTTYPE_ILLEGAL,	// ILLEGAL
    OS_BOOTTYPE_ROM,		// ROM
    OS_BOOTTYPE_NAND,		// TEMP
    OS_BOOTTYPE_NAND,		// NAND
    OS_BOOTTYPE_MEMORY,		// MEMORY
};

// ============================================================================
//
// ���擾
//
// ============================================================================

// �J�[�h�^�C�g���̎擾
BOOL SYSM_GetCardTitleList( TitleProperty *pTitleList_Card )
{
	BOOL retval = FALSE;
	
	if( SYSMi_GetWork()->isCardStateChanged ) {
		
		MI_CpuClear32( pTitleList_Card, sizeof(TitleProperty) );
		
		// ROM�w�b�_�o�b�t�@�̃R�s�[
		if( SYSM_IsExistCard() ) {
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );		// ARM7�Ɣr�����䂷��
			DC_InvalidateRange( (void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );	// �L���b�V���P�A
			MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BAK, (void *)SYSM_CARD_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );	// ROM�w�b�_�R�s�[
			SYSMi_GetWork()->cardHeaderCrc16 = SYSMi_GetWork()->cardHeaderCrc16;	// ROM�w�b�_CRC�R�s�[
			SYSMi_GetWork()->isCardStateChanged = FALSE;							// �J�[�h���X�V�t���O�𗎂Ƃ�
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );	// ARM7�Ɣr�����䂷��
			OS_ReleaseLockID( id );
			
			pTitleList_Card->flags.isValid = TRUE;
			pTitleList_Card->flags.isAppLoadCompleted = TRUE;
			pTitleList_Card->flags.isAppRelocate = TRUE;
			pTitleList_Card->pBanner = NULL;
			
			// �o�i�[�f�[�^�̃��[�h
			if( SYSM_GetCardRomHeader()->banner_offset &&
				SYSMi_ReadCardBannerFile( SYSM_GetCardRomHeader()->banner_offset, &s_bannerBuf[ CARD_BANNER_INDEX ] ) ) {
				pTitleList_Card->pBanner = &s_bannerBuf[ CARD_BANNER_INDEX ];
			}else {
				MI_CpuClearFast( &s_bannerBuf[ CARD_BANNER_INDEX ], sizeof(TWLBannerFile) );
			}
		}
		
		retval = TRUE;
	}
	
	// �^�C�g�����t���O�̃Z�b�g
	pTitleList_Card->flags.bootType = LAUNCHER_BOOTTYPE_ROM;
	pTitleList_Card->titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
	
	return retval;
}


// NAND�^�C�g�����X�g�̎擾
int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int listNum )
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, ���̏������w�肵�ă^�C�g�����X�g���擾����B
	// �Ƃ肠����ALL
	OSTick start;
	int l;
	int gotten;
	NAMTitleId titleIdArray[ LAUNCHER_TITLE_LIST_NUM ];
	
	if( listNum > LAUNCHER_TITLE_LIST_NUM ) {
		OS_TPrintf( "Warning: TitleList_Nand num over LAUNCHER_TITLE_LIST_NUM(%d)\n", LAUNCHER_TITLE_LIST_NUM );
	}

	start = OS_GetTick();
	gotten = NAM_GetTitleList( &titleIdArray[ 0 ], LAUNCHER_TITLE_LIST_NUM - 1 );
	OS_TPrintf( "NAM_GetTitleList : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	start = OS_GetTick();
	gotten = NAM_GetNumTitles();	// [TODO:]�{����������K�v�Ȃ����A���݂�NAM_GetTitleList���A�v�����������ƕԂ��Ă���Ȃ��̂ŁB
	OS_TPrintf( "NAM_GetNumTitles : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	
	for(l=0;l<gotten;l++)
	{
		//�w�b�_����o�i�[��ǂݍ���
		FSFile  file[1];
		BOOL bSuccess;
		static const int PATH_LENGTH=1024;
		char path[PATH_LENGTH];
		static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
		s32 readLen;
		s32 offset;
		
		start = OS_GetTick();
		readLen = NAM_GetTitleBootContentPathFast(path, titleIdArray[l]);
		OS_TPrintf( "NAM_GetTitleBootContentPath : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
		
		if(readLen != NAM_OK){
			OS_TPrintf("NAM_GetTitleBootContentPath failed %d,%lld,%d\n",l,titleIdArray[l],readLen);
		}
		
		bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);

		if( ! bSuccess )
		{
		OS_TPrintf("SYSM_GetNandTitleList failed: cant open file %s\n",path);
		    return -1;
		}
		
		// �o�i�[�f�[�^�I�t�Z�b�g��ǂݍ���
		bSuccess = FS_SeekFile(file, 0x68, FS_SEEK_SET);
		if( ! bSuccess )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant seek file(0)\n");
			FS_CloseFile(file);
		    return -1;
		}
		readLen = FS_ReadFile(file, &offset, sizeof(offset));
		if( readLen != sizeof(offset) )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant read file\n");
			FS_CloseFile(file);
		    return -1;
		}
		
		bSuccess = FS_SeekFile(file, offset, FS_SEEK_SET);
		if( ! bSuccess )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant seek file(offset)\n");
			FS_CloseFile(file);
		    return -1;
		}
		readLen = ReadFile(file, &s_bannerBuf[l], (s32)sizeof(TWLBannerFile));
		if( readLen != (s32)sizeof(TWLBannerFile) )
		{
			OS_TPrintf("SYSM_GetNandTitleList failed: cant read file2\n");
			FS_CloseFile(file);
		    return -1;
		}
		
		FS_CloseFile(file);
	}
	for(l=gotten;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		// �O�̂���0�ɃN���A
		titleIdArray[l] = 0;
	}

	// �J�[�h�A�v�����������������X�g�N���A
	MI_CpuClearFast( &pTitleList_Nand[ 1 ], sizeof(TitleProperty) * ( listNum - 1 ) );
	
	listNum = (gotten<listNum) ? gotten : listNum;
	
	for(l=0;l<listNum;l++)
	{
		pTitleList_Nand[l+1].titleID = titleIdArray[l];
		pTitleList_Nand[l+1].pBanner = &s_bannerBuf[l];
		if( titleIdArray[l] ) {
			pTitleList_Nand[l+1].flags.isValid = TRUE;
			pTitleList_Nand[l+1].flags.bootType = LAUNCHER_BOOTTYPE_NAND;
		}
	}
	// return : *TitleProperty Array
	return listNum;
}


// �w��t�@�C�����[�h
static s32 ReadFile(FSFile* pf, void* buffer, s32 size)
{
    u8* p = (u8*)buffer;
    s32 remain = size;

    while( remain > 0 )
    {
        const s32 len = MATH_IMin(1024, remain);
        const s32 readLen = FS_ReadFile(pf, p, len);

        if( readLen < 0 )
        {
            return readLen;
        }
        if( readLen != len )
        {
            return size - remain + readLen;
        }

        remain -= readLen;
        p      += readLen;
    }

    return size;
}


// ============================================================================
//
// �A�v���N��
//
// ============================================================================

static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle )
{
	enum
	{
	    region_header = 0,
	    region_arm9_ntr,
	    region_arm7_ntr,
	    region_arm9_twl,
	    region_arm7_twl,
	    region_max
	};
	// ���C���������̃N���A
	// DS�_�E�����[�h�v���C�̎��́AROM�w�b�_��ޔ�����
	// �A�v�����[�h
	// �A�v���F��
	
	
	// ���[�h
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    BOOL isTwlApp = TRUE;
	
	switch( pBootTitle->flags.bootType )
	{
	case LAUNCHER_BOOTTYPE_NAND:
		// NAND
    	NAM_GetTitleBootContentPathFast(path, pBootTitle->titleID);
		break;
	case LAUNCHER_BOOTTYPE_ROM:
		// TODO:CARD���ǂ̏ꍇ�̏���
		break;
	case LAUNCHER_BOOTTYPE_TEMP:
		// tmp�t�H���_
		STD_TSNPrintf( path, 31, "nand:/tmp/%.16llx.srl", pBootTitle->titleID );
		break;
	default:
		// unknown
		return;
	}

    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);

    if( ! bSuccess )
    {
OS_TPrintf("RebootSystem failed: cant open file\n");
        return;
    }

    {
        int     i;
        u32     source[region_max];
        u32     length[region_max];
        u32     destaddr[region_max];
        static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
        s32 readLen;

        // �܂�ROM�w�b�_��ǂݍ���
        // (�{���Ȃ炱����SRL�̐���������)
        bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);

        if( ! bSuccess )
        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
            FS_CloseFile(file);
            return;
        }

        readLen = ReadFile(file, header, (s32)sizeof(header));

        if( readLen != (s32)sizeof(header) )
        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", header, 0, sizeof(header), readLen);
            FS_CloseFile(file);
            return;
        }

        if( header[0x15C] != 0x56 || header[0x15D] != 0xCF )
        {
int i, j;
for( i = 0; i < 0x20; ++i )
{
for( j = 0; j < 0x10; ++j )
{
OS_TPrintf("%02X ", header[i * 0x10 + j]);
}
OS_TPrintf("\n");
}
OS_TPrintf("RebootSystem failed: logo CRC error\n");
            FS_CloseFile(file);
            return;
        }
        
        if( header[0x12] && 0x03 == 0 )
        {
			//NTR��pROM
			isTwlApp = FALSE;
		}
		
        // �e�̈��ǂݍ���
        source  [region_header  ] = 0x00000000;
        length  [region_header  ] = HW_TWL_ROM_HEADER_BUF_SIZE;
        destaddr[region_header  ] = HW_TWL_ROM_HEADER_BUF;
		
        source  [region_arm9_ntr] = *(const u32*)&header[0x020];
        length  [region_arm9_ntr] = *(const u32*)&header[0x02C];
        destaddr[region_arm9_ntr] = *(const u32*)&header[0x028];
		
        source  [region_arm7_ntr] = *(const u32*)&header[0x030];
        length  [region_arm7_ntr] = *(const u32*)&header[0x03C];
        destaddr[region_arm7_ntr] = *(const u32*)&header[0x038];
		
		if( isTwlApp )
		{
	        source  [region_arm9_twl] = *(const u32*)&header[0x1C0];
	        length  [region_arm9_twl] = *(const u32*)&header[0x1CC];
	        destaddr[region_arm9_twl] = *(const u32*)&header[0x1C8];
			
	        source  [region_arm7_twl] = *(const u32*)&header[0x1D0];
	        length  [region_arm7_twl] = *(const u32*)&header[0x1DC];
	        destaddr[region_arm7_twl] = *(const u32*)&header[0x1D8];
        }
        
        // �̈�ǂݍ��ݐ�̃`�F�b�N�y�эĔz�u���f�[�^�̍쐬
        // �Q�[���J�[�h�̍Ĕz�u��񂪏������܂�Ă���̂ŁAnand�A�v�����[�h�O�Ɉ�U�N���A
        MI_CpuClearFast(SYSMi_GetWork()->romRelocateInfo, sizeof(Relocate_Info) * RELOCATE_INFO_NUM);
		for( i=0; i<RELOCATE_INFO_NUM; i++ )
		{
			if ( !isTwlApp && i >= ARM9_LTD_STATIC ) continue;// nitro�ł͓ǂݍ��܂Ȃ��̈�
			if ( !SYSM_CheckLoadRegionAndSetRelocateInfo( (RomSegmentName)i, &(destaddr[i+region_arm9_ntr]), length[i+region_arm9_ntr],
				 &(SYSMi_GetWork()->romRelocateInfo[i]), isTwlApp ) )
			{
	OS_TPrintf("RebootSystem failed: ROM Load Region error\n");
	            FS_CloseFile(file);
				return;
			}
		}

        for (i = region_header; i < region_max; ++i)
        {
            u32 len = length[i];
            
            if ( !isTwlApp && i >= region_arm9_twl ) continue;// nitro�ł͓ǂݍ��܂Ȃ��̈�

            bSuccess = FS_SeekFile(file, (s32)source[i], FS_SEEK_SET);

            if( ! bSuccess )
            {
OS_TPrintf("RebootSystem failed: cant seek file(%d)\n", source[i]);
                FS_CloseFile(file);
                return;
            }

            readLen = ReadFile(file, (void *)destaddr[i], (s32)len);

            if( readLen != (s32)len )
            {
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
                FS_CloseFile(file);
                return;
            }
        }

        (void)FS_CloseFile(file);

		if(pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP)
		{
			// tmp�A�v���̏ꍇ�̓t�@�C���폜
			// TODO:���̑��ǂݍ��ݓ��̎��s���ɂ�Delete����K�v����H
			(void)FS_DeleteFile(path);
		}
    }

	// ROM�w�b�_�o�b�t�@���R�s�[
	MI_CpuCopy32( (void *)HW_TWL_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	
	SYSMi_GetWork()->isLoadSucceeded = TRUE;
}


// �w��^�C�g����ʃX���b�h�Ń��[�h�J�n����
void SYSM_StartLoadTitle( TitleProperty *pBootTitle )
{
#define THREAD_PRIO 17
#define STACK_SIZE 0xc00
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	
	// �A�v�������[�h��ԂȂ�A���[�h�J�n
	if( !pBootTitle->flags.isAppLoadCompleted ) {
		SYSMi_GetWork()->isLoadSucceeded = FALSE;
		OS_InitThread();
		OS_CreateThread( &s_thread, (void (*)(void *))SYSMi_LoadTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
		OS_WakeupThreadDirect( &s_thread );
	}else if( pBootTitle->flags.isAppRelocate ) {
	// �A�v�����[�h�ς݂ŁA�Ĕz�u�v������Ȃ�A�Ĕz�u�i�J�[�h�̂ݑΉ��j
		SYSMi_Relocate();
		SYSMi_GetWork()->isLoadSucceeded = TRUE;
	}else
	{
		// �A�v�����[�h�ς݂ŁA�Ĕz�u�v���Ȃ�
		SYSMi_GetWork()->isLoadSucceeded = TRUE;
	}
	
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM ) {
		SYSMi_GetWork()->isCardBoot = TRUE;
	}else if(pBootTitle->flags.isAppLoadCompleted)
	{
		// �J�[�h�u�[�g�łȂ��A���[�h�ς݂̏ꍇ�A�Ĕz�u���������`���[�p�����^����ǂݍ���
		MI_CpuCopy8( SYSM_GetLauncherParamBody()->v1.relocInfo, SYSMi_GetWork()->romRelocateInfo, sizeof(Relocate_Info)*RELOCATE_INFO_NUM );
		// �X�Ƀw�b�_���Ĕz�u
		if( ((ROM_Header_Short *)(OS_TWL_HEADER_PRELOAD_MMEM))->platform_code & PLATFORM_CODE_FLAG_TWL ) {
			// TWL-ROM�w�b�_���̍Ĕz�u
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		}else {
			// TWL-ROM�w�b�_���̍Ĕz�u
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
			MI_CpuCopyFast( (void *)(OS_TWL_HEADER_PRELOAD_MMEM), (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
			// NTR-ROM�w�b�_���̍Ĕz�u�́Areboot���C�u�����ōs���B
		}
	}
}


// �J�[�h�A�v���P�[�V�����̍Ĕz�u
static void SYSMi_Relocate( void )
{
	u32 size;
	u32 *dest = SYSM_GetCardRomHeader()->main_ram_address;
	// NTR�Z�L���A�̈�̍Ĕz�u
	DC_InvalidateRange( (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );	// �L���b�V���P�A
	size = ( SYSM_GetCardRomHeader()->main_size < SECURE_AREA_SIZE ) ?
			 SYSM_GetCardRomHeader()->main_size : SECURE_AREA_SIZE;
	// rom�̍Ĕz�u�����Q�Ƃ��āA�Z�L���A�̈�̍Ĕz�u���ύX����K�v�����������ׂ�
	if( SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC].src != NULL )
	{
		dest = (u32 *)SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC].src;
	}
	MI_CpuCopyFast( (void *)SYSM_CARD_NTR_SECURE_BUF, dest, size );
	
	if( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL ) {
		// TWL�Z�L���A�̈�̍Ĕz�u
		dest = SYSM_GetCardRomHeader()->main_ltd_ram_address;
		DC_InvalidateRange( (void *)SYSM_CARD_TWL_SECURE_BUF, SECURE_AREA_SIZE );	// �L���b�V���P�A
		size = ( SYSM_GetCardRomHeader()->main_ltd_size < SECURE_AREA_SIZE ) ?
				 SYSM_GetCardRomHeader()->main_ltd_size : SECURE_AREA_SIZE;
		// rom�̍Ĕz�u�����Q�Ƃ��āA�Z�L���A�̈�̍Ĕz�u���ύX����K�v�����������ׂ�
		if( SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC].src != NULL )
		{
			dest = (u32 *)SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC].src;
		}
		MI_CpuCopyFast( (void *)SYSM_CARD_TWL_SECURE_BUF, dest, size );
		// TWL-ROM�w�b�_���̍Ĕz�u
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	}else {
		// TWL-ROM�w�b�_���̍Ĕz�u
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	}
}


// �A�v�����[�h�ς݁H
BOOL SYSM_IsLoadTitleFinished( TitleProperty *pBootTitle )
{
	if( pBootTitle->flags.isAppLoadCompleted ) {
		return TRUE;
	}
	return OS_IsThreadTerminated( &s_thread );
}


// ���[�h�ς݂̎w��^�C�g���̔F�؂ƃu�[�g���s��
AuthResult SYSM_AuthenticateTitle( TitleProperty *pBootTitle )
{
	// ���[�h��
	if( !SYSM_IsLoadTitleFinished( pBootTitle ) ) {
		return AUTH_RESULT_PROCESSING;
	}
	// ���[�h�����H
	if( SYSMi_GetWork()->isLoadSucceeded == FALSE )
	{
		return AUTH_RESULT_TITLE_LOAD_FAILED;
	}
	// �p�����[�^�`�F�b�N
	if( !SYSMi_CheckTitlePointer( pBootTitle ) ) {
		return AUTH_RESULT_TITLE_POINTER_ERROR;
	}
#if 0
	// �G���g���A�h���X�̐��������`�F�b�N
	if( !SYSMi_CheckEntryAddress() ) {
		return AUTH_RESULT_ENTRY_ADDRESS_ERROR;
	}
#endif
	
	// BOOTTYPE_MEMORY��NTR���[�h��FS����Ńu�[�g����ƁA��NitroSDK�Ńr���h���ꂽ�A�v���̏ꍇ�A
	// ROM�A�[�J�C�u�ɃJ�[�h�����蓖�Ă��āAFS�Ŋ֌W�Ȃ��J�[�h�ɃA�N�Z�X�ɂ����Ă��܂��̂ŁA�����h�~����B
	if( ( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_MEMORY ) &&
		( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->platform_code ) == 0 ) &&
		( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->fat_size ) > 0 )
		) {
		return AUTH_RESULT_TITLE_BOOTTYPE_ERROR;
	}
	
	// ��ROM�w�b�_�F��
	
	
	// �}�E���g���̓o�^
	SYSMi_SetBootAppMountInfo( pBootTitle );
	
	// HW_WM_BOOT_BUF�ւ̃u�[�g���Z�b�g
	( (OSBootInfo *)OS_GetBootInfo() )->boot_type = s_launcherToOSBootType[ pBootTitle->flags.bootType ];
	
	BOOT_Ready();	// never return.
	
	return AUTH_RESULT_SUCCEEDED;
}


#if 0
// �w��^�C�g���̔F�؁����[�h�@���P�t���[������I����B
// ����������Ǝg��Ȃ�����
void SYSM_LoadAndAuthenticateTitleThread( TitleProperty *pBootTitle )
{
	SYSMi_LoadTitleThreadFunc( pBootTitle );
	OS_JoinThread(&s_thread);
	
	// �F��
	return SYSM_AuthenticateTitle( pBootTitle );
}
#endif


// �w��^�C�g�����u�[�g�\�ȃ|�C���^���`�F�b�N
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle )
{
#pragma unused( pBootTitle )
	
	return TRUE;
}


