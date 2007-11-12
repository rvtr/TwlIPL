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
#include <twl/nam.h>
#include <sysmenu.h>
#include <sysmenu/boot/common/boot.h>
#include "sysmenu_define.h"
#include "sysmenu_card.h"
#include "spi.h"
#include "mb_child.h"

// define data-----------------------------------------------------------------

typedef struct BannerCheckParam {
	u8		*srcp;
	u32		size;
}BannerCheckParam;

// extern data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static BOOL SYSMi_IsDebuggerBannerViewMode( void );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );

static void SYSMi_WriteAdjustRTC( void );
static int  SYSMi_ExistCard( void );
static void SYSMi_ReadCardBannerFile( void );
static BOOL SYSMi_CheckEntryAddress( void );
static void SYSMi_CheckCardCloneBoot( void );
static void SYSMi_CheckRTC( void );

// global variable-------------------------------------------------------------
void *(*SYSM_Alloc)( u32 size  );
void  (*SYSM_Free )( void *ptr );

#ifdef __SYSM_DEBUG
SharedWork		*swp;												// �f�o�b�K�ł�IPL1SharedWork�̃E�H�b�`�p
SYSM_work		*pSysm;											// �f�o�b�K�ł�SYSM���[�N�̃E�H�b�`�p
NitroConfigData *ncdp;												// �f�o�b�K�ł�NC�f�[�^�@�̃E�H�b�`�p
#endif

// static variable-------------------------------------------------------------
static BOOL			s_isBanner = FALSE;
static BannerFile	s_bannerBuf;
static NAMTitleId	old_titleIdArray[ LAUNCHER_TITLE_LIST_NUM ];

// const data------------------------------------------------------------------

static BannerCheckParam s_bannerCheckList[ BNR_VER_MAX ] = {
	{ (u8 *)&s_bannerBuf.v1, sizeof( BannerFileV1 ) },
	{ (u8 *)&s_bannerBuf.v2, sizeof( BannerFileV2 ) },
	{ (u8 *)&s_bannerBuf.v3, sizeof( BannerFileV3 ) },
};



// ============================================================================
// function's description
// ============================================================================

static void * AllocForNAM(unsigned long size)
{
	return OS_AllocFromHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE, size );
}

static void FreeForNAM(void *p)
{
	OS_FreeToHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE, p);
}


// �V�X�e�����j���[���C�u�����p�������A���P�[�^�̐ݒ�
void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
	SYSM_Alloc = pAlloc;
	SYSM_Free  = pFree;
}


// SystemMenu�̏�����
void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
#ifdef __SYSM_DEBUG
	pSysm = GetSYSMWork();
	ncdp  = GetTSD();
#endif /* __SYSM_DEBUG */
	
	TP_Init();
	RTC_Init();
	
	SYSM_SetAllocFunc( pAlloc, pFree );
	
	// WRAM�ݒ�͂���H
//	MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
//	MI_SetWramBank(MI_WRAM_ARM7_ALL);
	
	SVC_CpuClearFast(0x0000, (u16 *)GetSYSMWork(), sizeof(SYSM_work));	// SYSM���[�N�̃N���A
	
	
	reg_OS_PAUSE |= REG_OS_PAUSE_CHK_MASK;							// PAUSE���W�X�^�̃`�F�b�N�t���O�̃Z�b�g
	
//	SYSM_ReadHWInfo();												// NAND����HW�������[�h
	SYSM_ReadTWLSettingsFile();										// NAND����TWL�{�̐ݒ�f�[�^�����[�h
	
	SYSM_SetBackLightBrightness();									// �ǂݏo����TWL�{�̐ݒ�f�[�^�����ƂɃo�b�N���C�g�P�x�ݒ�
	SYSM_CaribrateTP();												// �ǂݏo����TWL�{�̐ݒ�f�[�^�����Ƃ�TP�L�����u���[�V�����B
	SYSMi_WriteAdjustRTC();											// �ǂݏo����TWL�{�̐ݒ�f�[�^�����Ƃ�RTC�N���b�N�␳�l���Z�b�g�B
	SYSMi_CheckRTC();
	
	SYSM_VerifyAndRecoveryNTRSettings();							// NTR�ݒ�f�[�^��ǂݏo���āATWL�ݒ�f�[�^�ƃx���t�@�C���A�K�v�Ȃ烊�J�o��
	
//	SYSMi_CheckCardCloneBoot();										// �J�[�h���N���[���u�[�g���`�F�b�N
//	SYSMi_ReadCardBannerFile();										// �J�[�h�o�i�[�t�@�C���̓ǂݏo���B
	
	//NAM�̏�����
	//NAM_Init(AllocForNAM,FreeForNAM);
	
	MI_CpuClearFast(old_titleIdArray, sizeof(old_titleIdArray) );
}


// �J�[�h�^�C�g���̎擾
int SYSM_GetCardTitleList( TitleProperty *pTitleList_Card )
{
#pragma unused( pTitleList_Card )
	return 0;
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

#include <twl/ese.h>
ESTitleMeta dst[1];

// NAND�^�C�g�����X�g�̎擾
int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size)
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, ���̏������w�肵�ă^�C�g�����X�g���擾����B
	// �Ƃ肠����ALL
	int l;
	int gotten;
	NAMTitleId titleIdArray[ LAUNCHER_TITLE_LIST_NUM ];
	static BannerFile bannerBuf[ LAUNCHER_TITLE_LIST_NUM ];
	gotten = NAM_GetTitleList(titleIdArray, LAUNCHER_TITLE_LIST_NUM );
	
	// �o�i�[�̓ǂݍ��݁c�c�ʂ̊֐��Ɉڂ��ׂ������B
	// ���t���[���ω�������K�v������B
	// �O�̃t���[����NAMTitleId�̔z����c���Ă����A��r�B
	// ID���ω����Ă�����ⓚ���p�Ńo�i�[��ǂݍ��ށB
	for(l=0;l<gotten;l++)
	{
		if(titleIdArray[l] != old_titleIdArray[l])
		{
			//�w�b�_����o�i�[��ǂݍ���
			FSFile  file[1];
			BOOL bSuccess;
			static const int PATH_LENGTH=1024;
			char path[PATH_LENGTH];
			static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
			s32 readLen;
			s32 offset;
			
			readLen = NAM_GetTitleBootContentPath(path, titleIdArray[l]);
			
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
			readLen = ReadFile(file, &bannerBuf[l], (s32)sizeof(BannerFile));
			if( readLen != (s32)sizeof(BannerFile) )
			{
				OS_TPrintf("SYSM_GetNandTitleList failed: cant read file2\n");
				FS_CloseFile(file);
			    return -1;
			}
			
			FS_CloseFile(file);
			
		}
	}
	for(l=gotten;l<LAUNCHER_TITLE_LIST_NUM;l++)
	{
		// �O�̂���0�ɃN���A
		titleIdArray[l] = 0;
	}
	MI_CpuCopyFast(titleIdArray,old_titleIdArray,sizeof(old_titleIdArray));
	
	for(l=0;l<size;l++)
	{
		pTitleList_Nand[l].titleID = 0;
		pTitleList_Nand[l].pBanner = 0;
	}

	size = (gotten<size) ? gotten : size;
	
	for(l=0;l<size;l++)
	{
		pTitleList_Nand[l].titleID = titleIdArray[l];
		pTitleList_Nand[l].pBanner = &bannerBuf[l];
	}
	// return : *TitleProperty Array
	return size;
}


// �w��^�C�g�����u�[�g�\�ȃ|�C���^���`�F�b�N
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle )
{
#pragma unused( pBootTitle )
	
	return TRUE;
}

enum
{
    region_header,
    region_arm9_ntr,
    region_arm7_ntr,
    region_arm9_twl,
    region_arm7_twl,
    region_max
};
// �w��^�C�g���̔F�؁����[�h�@���P�t���[������I����B
AuthResult SYSM_LoadAndAuthenticateTitle( TitleProperty *pBootTitle )
{
	// ���C���������̃N���A
	// DS�_�E�����[�h�v���C�̎��́AROM�w�b�_��ޔ�����
	// �A�v�����[�h
	// �A�v���F��
	
	// �����p�Bnam���������Ă���B���[�h���邾���B
	//NAM_LaunchTitle(pBootTitle->titleID);
	
	// ���[�h
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    NAM_GetTitleBootContentPath(path, pBootTitle->titleID);

    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);

    if( ! bSuccess )
    {
OS_TPrintf("RebootSystem failed: cant open file\n");
        return AUTH_RESULT_TITLE_POINTER_ERROR;
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
            return AUTH_RESULT_TITLE_POINTER_ERROR;
        }

        readLen = ReadFile(file, header, (s32)sizeof(header));

        if( readLen != (s32)sizeof(header) )
        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", header, 0, sizeof(header), readLen);
            FS_CloseFile(file);
            return AUTH_RESULT_TITLE_POINTER_ERROR;
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
            return AUTH_RESULT_TITLE_POINTER_ERROR;
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
		
        source  [region_arm9_twl] = *(const u32*)&header[0x1C0];
        length  [region_arm9_twl] = *(const u32*)&header[0x1CC];
        destaddr[region_arm9_twl] = *(const u32*)&header[0x1C8];
		
        source  [region_arm7_twl] = *(const u32*)&header[0x1D0];
        length  [region_arm7_twl] = *(const u32*)&header[0x1DC];
        destaddr[region_arm7_twl] = *(const u32*)&header[0x1D8];

        for (i = region_header; i < region_max; ++i)
        {
            u32 len = length[i];

            bSuccess = FS_SeekFile(file, (s32)source[i], FS_SEEK_SET);

            if( ! bSuccess )
            {
OS_TPrintf("RebootSystem failed: cant seek file(%d)\n", source[i]);
                FS_CloseFile(file);
                return AUTH_RESULT_TITLE_POINTER_ERROR;
            }

            readLen = ReadFile(file, (void *)destaddr[i], (s32)len);

            if( readLen != (s32)len )
            {
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
                FS_CloseFile(file);
                return AUTH_RESULT_TITLE_POINTER_ERROR;
            }
        }

        (void)FS_CloseFile(file);
    }

	// ROM�w�b�_�o�b�t�@���R�s�[
	MI_CpuCopy32( (void *)HW_TWL_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	
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
	
	// �N���B
	BOOT_Ready();	// never return;
	
	return AUTH_RESULT_SUCCEEDED;
}


#if 0
// TP���[�h�\���ǂ����𒲂ׂ�B
BOOL SYSM_IsTPReadable( void )
{
	if( SYSM_GetBootFlag() & BFLG_BOOT_DECIDED )	return FALSE;
	else											return TRUE;
}
#endif


// ============================================================================
// �T�u���[�`��
// ============================================================================

// ���S�f���X�L�b�v���H
BOOL SYSM_IsLogoDemoSkip( void )
{
	// ���V�X�e���A�v������̃n�[�h���Z�b�g�ɂ�郍�S�f����΂�������ɓ����B
	
	return SYSMi_IsDebuggerBannerViewMode();
}

// IS�f�o�b�K�̃o�i�[�r���[���[�h�N�����ǂ����H
static BOOL SYSMi_IsDebuggerBannerViewMode( void )
{
#ifdef __IS_DEBUGGER_BUILD
	return ( GetSYSMWork()->isOnDebugger &&
			 SYSMi_ExistCard() &&
			 GetRomHeaderAddr()->dbgRomSize == 0 ) ? TRUE : FALSE;
#else
	return FALSE;
#endif	// __IS_DEBUGGER_BUILD
}


// �o�i�[�t�@�C���̓ǂݍ��݂̎���
static void SYSMi_ReadCardBannerFile( void )
{
	s32 lockCardID;
	BannerFile *pBanner = &s_bannerBuf;
	
	if( ( !SYSMi_ExistCard() ) || ( *(void** )BANNER_ROM_OFFSET == NULL ) ) {
		s_isBanner = FALSE;
		return;
	}
	
	// ROM�J�[�h����̃o�i�[�f�[�^�̃��[�h
	if ( ( lockCardID = OS_GetLockID() ) > 0 ) {
		( void )OS_LockCard( (u16 )lockCardID );
		DC_FlushRange( pBanner, sizeof(BannerFile) );
		SYSM_ReadCard(*(void** )BANNER_ROM_OFFSET, pBanner, sizeof(BannerFile) );
		( void )OS_UnLockCard( (u16 )lockCardID );
		OS_ReleaseLockID( (u16 )lockCardID );
	}
	
	// �o�i�[�f�[�^�̐���`�F�b�N
	{
		int i;
		u16 calc_crc = 0xffff;
		u16 *hd_crcp = (u16 *)&pBanner->h.crc16_v1;
		BannerCheckParam *chkp = &s_bannerCheckList[ 0 ];
		
		s_isBanner  = TRUE;
		
		for( i = 0; i < BNR_VER_MAX; i++ ) {
			if( i < pBanner->h.version ) {
			    calc_crc = SVC_GetCRC16( calc_crc, chkp->srcp, chkp->size );
				if( calc_crc != *hd_crcp++ ) {
					s_isBanner =  FALSE;
					break;
				}
			}else {
				MI_CpuClear16( chkp->srcp, chkp->size );
			}
			chkp++;
		}
		if( !s_isBanner ) {
			MI_CpuClear16( &s_bannerBuf, sizeof(BannerFile) );
		}
	}
}


// �^�b�`�p�l���L�����u���[�V����
void SYSM_CaribrateTP( void )
{
#ifndef __TP_OFF
	TPCalibrateParam calibrate;
	
	( void )TP_CalcCalibrateParam( &calibrate,							// �^�b�`�p�l��������
			GetTSD()->tp.data.raw_x1, GetTSD()->tp.data.raw_y1, (u16)GetTSD()->tp.data.dx1, (u16)GetTSD()->tp.data.dy1,
			GetTSD()->tp.data.raw_x2, GetTSD()->tp.data.raw_y2, (u16)GetTSD()->tp.data.dx2, (u16)GetTSD()->tp.data.dy2 );
	TP_SetCalibrateParam( &calibrate );
	OS_Printf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			GetTSD()->tp.data.raw_x1, GetTSD()->tp.data.raw_y1, (u16)GetTSD()->tp.data.dx1, (u16)GetTSD()->tp.data.dy1,
			GetTSD()->tp.data.raw_x2, GetTSD()->tp.data.raw_y2, (u16)GetTSD()->tp.data.dx2, (u16)GetTSD()->tp.data.dy2 );
#endif
}


// RTC�N���b�N�␳�l���Z�b�g
static void SYSMi_WriteAdjustRTC( void )
{
	// ��TWL�̎��́ANAND��"/sys/HWINFO.dat"�t�@�C������Y����������擾����B
#if 0
	FS_OpenFile( "/sys/HWINFO.dat" );
	FS_ReadFile( xxxx );
	raw = xxxx.rtcRaw;
	( void )RTCi_SetRegAdjust( &raw );
#endif
	
#ifndef __IS_DEBUGGER_BUILD											// �f�o�b�K�p�r���h���͕␳���Ȃ��B
	RTCRawAdjust raw;
	raw.adjust = 0;
//	raw.adjust = GetTSD()->rtcClockAdjust;							// ncd_invalid���ɂ�rtcClockAdjust��
																	// 0�N���A����Ă��邽�ߕ␳�@�\�͎g�p����Ȃ�
	( void )RTCi_SetRegAdjust( &raw );
#endif /* __IS_DEBUGGER_BUILD */
}


// NTR,TWL�J�[�h���݃`�F�b�N 		�u���^�[���@1�F�J�[�h�F���@0�F�J�[�h�Ȃ��v
static int SYSMi_ExistCard( void )
{
	if( ( GetRomHeaderAddr()->nintendo_logo_crc16 == 0xcf56 ) &&
	    ( GetRomHeaderAddr()->header_crc16 == GetSYSMWork()->cardHeaderCrc16) ) {
		return TRUE;												// NTR,TWL�J�[�h����iNintendo���SCRC�A�J�[�h�w�b�_CRC���������ꍇ�j
																	// ��Nintendo���S�f�[�^�̃`�F�b�N�́A�����̓s����A���S�\�����[�`���N����ɍs���܂��B
	}else {
		return FALSE;												// NTR,TWL�J�[�h�Ȃ�
	}
}


// �X���[�v���[�h�ւ̑J��
void SYSM_GoSleepMode( void )
{
#ifndef __IS_DEBUGGER_BUILD											// �f�o�b�K�p�r���h���̓X���[�v���Ȃ��B
	PM_GoSleepMode( (PMWakeUpTrigger)( (PAD_DetectFold() ? PM_TRIGGER_COVER_OPEN : 0) | PM_TRIGGER_RTC_ALARM ),
					0,
					0 );
#endif /* __IS_DEBUGGER_BUILD */
}


// �o�b�N���C�g�P�x����
void SYSM_SetBackLightBrightness( void )
{
	( void )PMi_WriteRegister( 4, (u16)TSD_GetBacklightBrightness() );
	( void )PM_SetBackLight( PM_LCD_ALL, PM_BACKLIGHT_ON );
}




//======================================================================
//  �e��`�F�b�N
//======================================================================

// Nintendo���S�`�F�b�N			�u���^�[���@1:Nintendo���S�F�������@0�F���s�v
BOOL SYSM_CheckNintendoLogo(u16 *logo_cardp)
{
	u16 *logo_orgp	= (u16 *)SYSROM9_NINLOGO_ADR;					// ARM9�̃V�X�e��ROM�̃��S�f�[�^�ƃJ�[�g���b�W���̂��̂��r
	u16 length		= NINTENDO_LOGO_LENGTH >> 1;
	
	while(length--) {
		if(*logo_orgp++ != *logo_cardp++) return FALSE;
	}
	return TRUE;
}


// �G���g���A�h���X�̐������`�F�b�N
static BOOL SYSMi_CheckEntryAddress( void )
{
	// �G���g���A�h���X��ROM���o�^�G���A��AGB�J�[�g���b�W�G���A�Ȃ�A�������[�v�ɓ���B
	if(   !(   ( (u32)GetRomHeaderAddr()->main_entry_address >= HW_MAIN_MEM              )
			&& ( (u32)GetRomHeaderAddr()->main_entry_address <  SYSM_ARM9_MMEM_ENTRY_ADDR_LIMIT ) )
	   || !(    (   ( (u32)GetRomHeaderAddr()->sub_entry_address  >= HW_MAIN_MEM      )
			     && ( (u32)GetRomHeaderAddr()->sub_entry_address  <  SYSM_ARM7_LOAD_MMEM_LAST_ADDR ) )
			 || (   ( (u32)GetRomHeaderAddr()->sub_entry_address  >= HW_WRAM    )
				 && ( (u32)GetRomHeaderAddr()->sub_entry_address  <  SYSM_ARM7_LOAD_WRAM_LAST_ADDR ) ) ) )
	{
		OS_TPrintf("entry address invalid.\n");
#ifdef __DEBUG_SECURITY_CODE
		DispSingleColorScreen( SCREEN_YELLOW );
#endif
		return FALSE;
	}
	OS_TPrintf("entry address valid.\n");
	return TRUE;
}


// �N���[���u�[�g����
static void SYSMi_CheckCardCloneBoot( void )
{
	s32	lockCardID;
	u8 	*buffp         = (u8 *)&s_bannerBuf;		// �o�i�[�p�o�b�t�@���e���|�����Ƃ��Ďg�p
	u32 total_rom_size = GetRomHeaderAddr()->total_rom_size ? GetRomHeaderAddr()->total_rom_size : 0x01000000;
	u32 file_offset    = total_rom_size & 0xFFFFFE00;
	
	if( !SYSMi_ExistCard() ) {
		return;
	}
	
	if ( ( lockCardID = OS_GetLockID() ) > 0 ) {
		( void )OS_LockCard( (u16 )lockCardID );
		DC_FlushRange( buffp, BNR_IMAGE_SIZE );
		SYSM_ReadCard( (void *)file_offset, buffp, BNR_IMAGE_SIZE );
		( void )OS_UnLockCard( (u16 )lockCardID );
		OS_ReleaseLockID( (u16 )lockCardID );
	}
	
	buffp += total_rom_size & 0x000001FF;
	if( *buffp++ == 'a' && *buffp == 'c' ) {
		GetSYSMWork()->clone_boot_mode = CLONE_BOOT_MODE;
	}else {
		GetSYSMWork()->clone_boot_mode = OTHER_BOOT_MODE;
	}
}


// �N������RTC�`�F�b�N
static void SYSMi_CheckRTC( void )
{
	RTCDate date;
	RTCTime	time;
	
	// RTC�̃��Z�b�g or ���������l�����o�����ꍇ�͏���N���V�[�P���X�ցB
	( void )RTC_GetDateTime( &date, &time );
	if( !SYSM_CheckRTCDate( &date ) ||
	    !SYSM_CheckRTCTime( &time )
#ifndef __IS_DEBUGGER_BUILD											// �f�o�b�K�ł�RTC�̓d�r���Ȃ��̂ŁA���񂱂��ɂЂ��������Đݒ�f�[�^���Е��N���A����Ă��܂��B�����h���X�C�b�`�B
		|| ( GetSYSMWork()->rtcStatus & 0x01 )
#endif
		) {							// RTC�ُ̈�����o������Artc���̓t���O��rtcOffset��0�ɂ���NVRAM�ɏ������݁B
		OS_TPrintf("\"RTC reset\" or \"Illegal RTC data\" detect!\n");
		GetTSD()->flags.isSetDateTime	= 0;
		GetTSD()->rtcOffset				= 0;
		GetTSD()->rtcLastSetYear		= 0;
		// �������C�g����H
		SYSM_WriteTWLSettingsFile();
	}
}


//======================================================================
//  �f�o�b�O
//======================================================================
