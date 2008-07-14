/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     viewSystemInfo.c

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include <wchar.h>
#include <stdlib.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include <twl/os/common/sharedFont.h>
#include "misc.h"
#include "viewSystemInfo.h"
#include "strResource.h"
#include "control.h"
#include "myIoreg_SCFG.h"

#define DISPINFO_BUFSIZE 64
#define WL_TITLEID 0x0003000F484E4341

/* function prototype  ----------------------------- */
void getAllInfo( void );
void initInfo( void );
void infoAlloc( DispInfoEntry *p, u8 index, u8 size, BOOL isSjis );
void printAllInfo ( void );
void getOwnerInfo( void );
void getParentalInfo( void );
void getNormalHWInfo( void );
BOOL getSecureHWInfo( void );
void getSCFGARM9Info( void );
void getSCFGARM7Info( void );
void getVersions( void );
void getWirelessVersion( void );
void getContentsVersion( void );
void getSharedFontVersion( void );


/* global variables ----------------------------- */

DispInfoEntry* gAllInfo[ROOTMENU_SIZE]; // ���ꂼ��̃��j���[���Ƃɍ��ڂ̒��g��"�������"���z��
//u16 gUserName[OS_OWNERINFO_NICKNAME_MAX + 1];					// ���[�U��
//u16 gUserComment[OS_OWNERINFO_COMMENT_MAX + 1];					// ���[�U�R�����g
//u16 gSecretAnswer[OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1];	// �閧�̓���

// �R���e���c�^�C�g���֌W
OSTitleId *gContentsTitle;		// �R���e���c�^�C�g���̈ꗗ
s32 gNumContents;				// gContentsTitle�Ɋ܂܂��^�C�g���̐�
u16 *gContentsVersion;			// gContentsTitle���ꂼ��̃o�[�W�������

// ARM7����SCFG�f�[�^��ێ�
u8 gArm7SCFGReg[DISPINFO_SHARED_SCFG_REG_SIZE];
u8 gArm7SCFGWram[DISPINFO_SHARED_SCFG_WRAM_SIZE];

/*
static OSOwnerInfoEx 		gOwnerInfo;						// �I�[�i�[���
static BOOL 				gbWirelessAvailable;			// �������L�����ǂ���
static BOOL 				gbBatteryExtensionAvailable;	// �o�b�e���[�G�N�X�e���V�������[�h���L����
static BOOL 				gbAgreeEula;					// Eula���Ӎς݂�
static u8 					gEulaVersion;					// ����Eula�̃o�[�W����
static const OSTWLParentalControl	*gpPC;		// �y�A�����^���R���g���[�����
static u8 					gUniqueID[OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN];	// ���j�[�NId
static BOOL 				gbForceDisableWireless;			// �����̋����������t���O
static OSTWLRegion			gRegion; 						// ���[�W�����ԍ�
static u8 					gSerialNo[OS_TWL_HWINFO_SERIALNO_LEN_MAX]; // �V���A���ԍ�
*/

/* static data ---------------------------------- */


/* function definition -------------------------- */

void displayInfoMain( void )
{
	static int menu = MENU_ROOT;
	static int line = 0; // ���ݑI�����Ă���y�[�W�i���o�ƍs

	static BOOL firstCall = TRUE;	// �ŏ��̌Ăяo�����ۂ�
	
	// ����̌Ăяo�����͑S�Ă̏����擾���Ȃ���
	if( firstCall )
	{
		initInfo();
		getAllInfo();
	}
	
	// �p�b�h���őI��ł�ꏊ�Ƃ����Ƃ��X�V
	if(	control( &menu, &line ) || firstCall )
	{
		// �������삪�������Ƃ��̓L�����o�X�N���A���ĕ`�悵�Ȃ���
	    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
   	    NNS_G2dCharCanvasClear( &gCanvasSub, TXT_COLOR_WHITE );
		
		firstCall = FALSE;
	}
	
	drawMenu( menu, line); // ���ꗗ��`�悷��
	drawHeader( menu, line );
}

void initInfo( void )
{
	OS_TPrintf("buffer initialize\n");
	
	// �Œ蕶����őΉ��ł��Ȃ����ڂɕ�����̈�����蓖�Ă�
	infoAlloc( gAllInfo[MENU_OWNER], OWNER_BIRTHDAY, DISPINFO_BUFSIZE , TRUE );
//	infoAlloc( gAllInfo[MENU_PARENTAL], PARENTAL_AGE, DISPINFO_BUFSIZE , TRUE );
//	infoAlloc( gAllInfo[MENU_PARENTAL], PARENTAL_QUESTION_ID, DISPINFO_BUFSIZE, TRUE );
//	infoAlloc( gAllInfo[MENU_PARENTAL], PARENTAL_PASSWORD, OS_TWL_PCTL_PASSWORD_LENGTH+1, TRUE );
//	infoAlloc( gAllInfo[MENU_NORMAL_HW], NORMAL_HW_EULA_VERSION , DISPINFO_BUFSIZE , TRUE );
//	infoAlloc( gAllInfo[MENU_NORMAL_HW], NORMAL_HW_RTC_OFFSET,  DISPINFO_BUFSIZE, TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_UNIQUE_ID, OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_SERIAL , OS_TWL_HWINFO_SERIALNO_LEN_MAX + 1, TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_LANGUAGE, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_FUSE, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SCFG_ARM7], SCFG_ARM7_MI_CC, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SCFG_ARM7], SCFG_ARM7_MI_CA, DISPINFO_BUFSIZE , TRUE );

	infoAlloc( gAllInfo[MENU_OWNER], OWNER_NICKNAME, OS_OWNERINFO_NICKNAME_MAX + 1 , FALSE );
	infoAlloc( gAllInfo[MENU_OWNER], OWNER_COMMENT, OS_OWNERINFO_COMMENT_MAX + 1 , FALSE );
	infoAlloc( gAllInfo[MENU_PARENTAL], PARENTAL_ANSWER, OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1 , FALSE );

	OS_TPrintf( "information alloc succeeded\n" );


}
	
void infoAlloc( DispInfoEntry *p, u8 index, u8 size, BOOL isSjis )
{
	if( isSjis )
	{
		p[index].str.sjis = (char*) Alloc (sizeof(char) * size);
		SDK_ASSERT( p->str.sjis );
		p[index].str.sjis[size] = '\0';
	}
	else
	{
		p[index].str.utf = (u16*) Alloc (sizeof(u16) * size);
		SDK_ASSERT( p->str.utf );
		p[index].str.utf[size-1] = '\0';
	}

}


void getAllInfo( void )
// 
{
	BOOL SCFGAccessable = FALSE;
	BOOL fuseRomAccessable = FALSE;

	OS_TPrintf("reflesh Information\n");
		
	getOwnerInfo();
	getParentalInfo();
	getNormalHWInfo();
	getSecureHWInfo();
	getSCFGARM7Info();
	getSCFGARM9Info();
	getVersions();
	
	
	/*
	if( fuseRomAccessable )
	{
		
	}*/
	
	printAllInfo();
	OS_TPrintf("reflesh information finished\n");
}

void getOwnerInfo( void )
// �I�[�i�[��������擾����
{
	OSOwnerInfoEx ownerInfo;
	
	OS_TPrintf( "...Owner Information\n");
	// �{�̐ݒ�܂��
	OS_GetOwnerInfoEx( &ownerInfo );
	
	// �S�̏��ɂ߂Ă���
	// �D�L����������Ȃ̂ł��Ƃōl����
	gAllInfo[MENU_OWNER][OWNER_LANGUAGE].str.sjis = s_strLanguage[ ownerInfo.language ];
	gAllInfo[MENU_OWNER][OWNER_LANGUAGE].iValue = ownerInfo.language;
	
	gAllInfo[MENU_OWNER][OWNER_COLOR].str.sjis = s_strUserColor[ ownerInfo.favoriteColor ];
	gAllInfo[MENU_OWNER][OWNER_COLOR].iValue = ownerInfo.favoriteColor;

	snprintf( gAllInfo[MENU_OWNER][OWNER_BIRTHDAY].str.sjis, DISPINFO_BUFSIZE-1, "%02d/%02d",  ownerInfo.birthday.month, ownerInfo.birthday.day);
	gAllInfo[MENU_OWNER][OWNER_BIRTHDAY].iValue = ownerInfo.birthday.month * 100 + ownerInfo.birthday.day;
	
	gAllInfo[MENU_OWNER][OWNER_COUNTRY].str.sjis = s_strCountry[ownerInfo.country];
	gAllInfo[MENU_OWNER][OWNER_COUNTRY].iValue = ownerInfo.country;
	
	wcsncpy( gAllInfo[MENU_OWNER][OWNER_NICKNAME].str.utf , ownerInfo.nickName, OS_OWNERINFO_NICKNAME_MAX + 1);
	gAllInfo[MENU_OWNER][OWNER_NICKNAME].isSjis = FALSE;	
	wcsncpy( gAllInfo[MENU_OWNER][OWNER_COMMENT].str.utf , ownerInfo.comment, OS_OWNERINFO_COMMENT_MAX + 1 );
	gAllInfo[MENU_OWNER][OWNER_COMMENT].isSjis = FALSE;
}

void getParentalInfo( void )
// �y�A�����^���R���g���[�������擾����
{
	// �y�A�����^���R���g���[���܂��
	const LCFGTWLParentalControl *pPC = (const LCFGTWLParentalControl *) OS_GetParentalControlInfoPtr();

	OS_TPrintf( "...Parental Control Information\n" );

	gAllInfo[MENU_PARENTAL][PARENTAL_FLAG].str.sjis = s_strBool[ pPC->flags.isSetParentalControl ];	
	gAllInfo[MENU_PARENTAL][PARENTAL_FLAG].iValue = (int) pPC->flags.isSetParentalControl;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_PICTOCHAT].str.sjis = s_strBool[ pPC->flags.pictoChat ];
	gAllInfo[MENU_PARENTAL][PARENTAL_PICTOCHAT].iValue = (int) pPC->flags.pictoChat;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_DOWNLOAD].str.sjis = s_strBool[ pPC->flags.dsDownload ];
	gAllInfo[MENU_PARENTAL][PARENTAL_DOWNLOAD].iValue = (int) pPC->flags.dsDownload;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_BROWSER].str.sjis = s_strBool[ pPC->flags.browser ];
	gAllInfo[MENU_PARENTAL][PARENTAL_BROWSER].iValue = (int) pPC->flags.browser;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_WIIPOINT].str.sjis = s_strBool[ pPC->flags.wiiPoint ];
	gAllInfo[MENU_PARENTAL][PARENTAL_WIIPOINT].iValue = (int) pPC->flags.wiiPoint;

	gAllInfo[MENU_PARENTAL][PARENTAL_PHOTO_EXCHANGE].str.sjis = s_strBool[ pPC->flags.photoExchange ];
	gAllInfo[MENU_PARENTAL][PARENTAL_PHOTO_EXCHANGE].iValue = (int) pPC->flags.photoExchange;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_UGC].str.sjis = s_strBool[ pPC->flags.ugc ];
	gAllInfo[MENU_PARENTAL][PARENTAL_UGC].iValue = (int) pPC->flags.ugc;	
	
	gAllInfo[MENU_PARENTAL][PARENTAL_ORGANIZATION].str.sjis = s_strRatingOrg[ pPC->ogn ];
	gAllInfo[MENU_PARENTAL][PARENTAL_ORGANIZATION].iValue = (int) pPC->ogn;

	gAllInfo[MENU_PARENTAL][PARENTAL_AGE].iValue = pPC->ratingAge;
	gAllInfo[MENU_PARENTAL][PARENTAL_AGE].isNumData = TRUE;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_QUESTION_ID].iValue = pPC->secretQuestionID;
	gAllInfo[MENU_PARENTAL][PARENTAL_QUESTION_ID].isNumData = TRUE;
	
	gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD].iValue = atoi( pPC->password ) ;
	gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD].isNumData = TRUE;
	
	wcsncpy( gAllInfo[MENU_PARENTAL][PARENTAL_ANSWER].str.utf , pPC->secretAnswer, OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1);
	gAllInfo[MENU_PARENTAL][PARENTAL_ANSWER].isSjis = FALSE;
}

void getNormalHWInfo( void )
{
	int val;

	val = OS_IsAvailableWireless();
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_WIRELESS].iValue = val;
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_WIRELESS].str.sjis = s_strEnable[ val ];

	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_RTC_OFFSET].iValue = (int) OS_GetOwnerRtcOffset();
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_RTC_OFFSET].isNumData = TRUE;

	val = OS_IsAgreeEULA();
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_AGREE_EULA].iValue = val;
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_AGREE_EULA].str.sjis = s_strBool[ val ];
	
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_EULA_VERSION].iValue = OS_GetAgreedEULAVersion();
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_EULA_VERSION].isNumData = TRUE;
}

BOOL getSecureHWInfo( void )
{
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].str.sjis = s_strBool[ OS_IsForceDisableWireless() ];
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].isAligned = FALSE;
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].numLines = 2;
		
	gAllInfo[MENU_SECURE_HW][SECURE_HW_REGION].str.sjis = s_strRegion[ OS_GetRegion() ];
	
	{
		int i;
		char ascii[] = "0123456789abcdef";
		const u8 *unq = OS_GetMovableUniqueIDPtr();
		// 16�i��1�o�C�g���l�߂Ă���
		// �o�b�t�@��������3�{���Ȃ̂́A�f�[�^��"%02x-%02x-%02x..."�ɒu�����邽��
		for(i=0; i < OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3; i += 3, unq++ )
		{	
			gAllInfo[MENU_SECURE_HW][SECURE_HW_UNIQUE_ID].str.sjis[i] = ascii[(*unq>>4) & 0x0f];
			gAllInfo[MENU_SECURE_HW][SECURE_HW_UNIQUE_ID].str.sjis[i+1] = ascii[*unq & 0x0f];
			gAllInfo[MENU_SECURE_HW][SECURE_HW_UNIQUE_ID].str.sjis[i+2] = 
				(i+2) == (OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 - 1) ? (char)'\0' : (char)'-' ;
			OS_TPrintf("uniqid: %d\n", *unq);
		}
	}
	
	{
		u8 serialBuf[OS_TWL_HWINFO_SERIALNO_LEN_MAX];
		OS_GetSerialNo( serialBuf );
		snprintf(  gAllInfo[MENU_SECURE_HW][SECURE_HW_SERIAL].str.sjis , OS_TWL_HWINFO_SERIALNO_LEN_MAX, "%s", serialBuf);
	}
	
	{
		u64	buf;
		buf = OS_GetValidLanguageBitmap();
		OS_TPrintf("language bitmap : %lx\n", buf );
		snprintf( gAllInfo[MENU_SECURE_HW][SECURE_HW_LANGUAGE].str.sjis ,
				DISPINFO_BUFSIZE-1, "%08lx", OS_GetValidLanguageBitmap() );
		
		// fuseRom�f�[�^�̓ǂݏo��
		// secure�ȃA�v���ȊO�̓n�[�h�I�ɐ؂藣�����̂Ń[���ɂȂ�
		buf = SCFG_ReadFuseData();
		OS_TPrintf("fuse data : %llx\n", buf);
		snprintf( gAllInfo[MENU_SECURE_HW][SECURE_HW_FUSE].str.sjis ,
				DISPINFO_BUFSIZE-1, "%016llx", SCFG_ReadFuseData() );

		// �Ԃ�l�ŃZ�L���A�A�v�����ǂ�������ł���悤��
		if( buf )
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

void getSCFGARM9Info( void )
// SCFG�����擾����
{
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_STATE].str.sjis = 
		SCFG_GetSystemRomType() == SCFG_SYSTEM_ROM_FOR_NITRO ? s_strRomMode[1] : s_strRomMode[0];
	
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CAMERA_CKI].str.sjis = s_strEnable[ SCFG_IsCameraCKIClockEnable() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_WRAM_CLOCK].str.sjis = s_strSupply[ SCFG_IsClockSuppliedToWram() ];	
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CAMERA_CLOCK].str.sjis = s_strSupply[ SCFG_IsClockSuppliedToCamera() ];	
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_DSP_CLOCK].str.sjis = s_strSupply[ SCFG_IsClockSuppliedToDSP() ];	
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CPU_SPEED].str.sjis = 
		SCFG_GetCpuSpeed() == SCFG_CPU_SPEED_1X ? s_strCpuSpeed[0] : s_strCpuSpeed[1];
		
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_DSP_RESET].str.sjis = s_strBool[ SCFG_IsDSPReset() ];

	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CFG_ACCESSIBLE].str.sjis = s_strEnable[ SCFG_IsConfigBlockAccessible() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CFG_ACCESSIBLE].isAligned = FALSE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CFG_ACCESSIBLE].numLines = 2;
	
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_WRAM_ACCESSIBLE].str.sjis = s_strEnable[ SCFG_IsWramAccessible() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_DSP_ACCESSIBLE].str.sjis = s_strEnable[ SCFG_IsDSPAccessible() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CAMERA_ACCESSIBLE].str.sjis = s_strEnable[ SCFG_IsCameraAccessible() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_NDMA_ACCESSIBLE].str.sjis = s_strEnable[ SCFG_IsNDmaAccessible() ];
	
	{
		u8 idx;
		
		if( SCFG_GetPsramBoundary() == SCFG_PSRAM_BOUNDARY_4MB )
		{
			idx = 0;
		}
		else if ( SCFG_GetPsramBoundary() == SCFG_PSRAM_BOUNDARY_16MB )
		{
			idx = 1;
		}
		else if ( SCFG_GetPsramBoundary() == SCFG_PSRAM_BOUNDARY_32MB )
		{
			idx = 2;
		}
		
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_PSRAM_BOUNDARY].str.sjis = s_strPSRAM[ idx ];
	}
	
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_INTC_EXPANSION].str.sjis = s_strEnable[ SCFG_IsIntcExpanded() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_LCDC_EXPANSION].str.sjis = s_strEnable[ SCFG_IsLCDCExpanded() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_VRAM_EXPANSION].str.sjis = s_strEnable[ SCFG_IsVramExpanded() ];
	
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_FIX_CARD].str.sjis = s_strEnable[ SCFG_IsCardFixed() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_FIX_DIVIDER].str.sjis = s_strEnable[ SCFG_IsDividerFixed() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_FIX_2DENGINE].str.sjis = s_strEnable[ SCFG_Is2DEngineFixed() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_FIX_RENDERER].str.sjis = s_strEnable[ SCFG_IsRendererFixed() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_FIX_GEOMETRY].str.sjis = s_strEnable[ SCFG_IsGeometryFixed() ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_FIX_DMA].str.sjis = s_strEnable[ SCFG_IsDmacFixed() ];
	
}

void getSCFGARM7Info( void )
{
	// �K�v�ȃf�[�^�͑S��gARM7SCFGReg, gARM7SCFGWram�Ŋm�ۍ�
	
	// ROM���䃌�W�X�^(L)�A(H)
	{
		// SEC�t���O��TRUE = �؂藣��(�A�N�Z�X�s��),  FALSE = �ڑ�(�A�N�Z�X��)
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_SEC].str.sjis = 
			gArm7SCFGReg[DISP_REG_A9ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A9ROM_SEC_MASK ? s_strAccess[0]: s_strAccess[1];
			
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_RSEL].str.sjis = 
			gArm7SCFGReg[DISP_REG_A9ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A9ROM_RSEL_MASK ? s_strRomMode[1]: s_strRomMode[0];

		// SEC�t���O��TRUE = �؂藣��(�A�N�Z�X�s��),  FALSE = �ڑ�(�A�N�Z�X��)
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_SEC].str.sjis = 
			gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_SEC_MASK ? s_strAccess[0]: s_strAccess[1];
			
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_RSEL].str.sjis = 
			gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_RSEL_MASK ? s_strRomMode[1]: s_strRomMode[0];
			
		// FuseROM�t���O��TRUE = �؂藣��(�A�N�Z�X�s��),  FALSE = �ڑ�(�A�N�Z�X��)
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_FUSE].str.sjis = 
			gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_FUSE_MASK ? s_strAccess[0]: s_strAccess[1];
			
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_WE].str.sjis = 
			gArm7SCFGReg[DISP_REG_ROMWE_OFFSET - 0x4000] & DISP_REG_SCFG_ROMWE_WE_MASK ? s_strEnable[1]: s_strEnable[0];
	}
		
	// �V�K�u���b�N�N���b�N���䃌�W�X�^
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_CLK_OFFSET - 0x4000] );
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD1].str.sjis = 
			flag & DISP_REG_SCFG_CLK_SD1HCLK_MASK ? s_strSupply[1]: s_strSupply[0];
			
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD2].str.sjis = 
			flag & DISP_REG_SCFG_CLK_SD2HCLK_MASK ? s_strSupply[1]: s_strSupply[0];
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_AES].str.sjis = 
			flag & DISP_REG_SCFG_CLK_AESHCLK_MASK ? s_strSupply[1]: s_strSupply[0];
			
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_WRAM].str.sjis = 
			flag & DISP_REG_SCFG_CLK_WRAMHCLK_MASK ? s_strSupply[1]: s_strSupply[0];
			
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SND].str.sjis = 
			flag & DISP_REG_SCFG_CLK_SNDMCLK_MASK ? s_strSupply[1]: s_strSupply[0];
	}
	
	// JTAG���䃌�W�X�^
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_JTAG_OFFSET - 0x4000] );
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].str.sjis = 
			flag & DISP_REG_SCFG_JTAG_ARM7SEL_MASK ? s_strEnable[1]: s_strEnable[0];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].numLines = 2;

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_CPU].str.sjis = 
			flag & DISP_REG_SCFG_JTAG_CPUJE_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_DSP].str.sjis = 
			flag & DISP_REG_SCFG_JTAG_DSPJE_MASK ? s_strEnable[1]: s_strEnable[0];
	}	
	
	// �g���@�\���䃌�W�X�^
	{
		u32 flag = MI_LoadLE32( &gArm7SCFGReg[DISP_REG_EXT_OFFSET - 0x4000] );
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMA].str.sjis = 
			flag & DISP_REG_SCFG_EXT_DMA_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SDMA].str.sjis = 
			flag & DISP_REG_SCFG_EXT_SDMA_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SND].str.sjis = 
			flag & DISP_REG_SCFG_EXT_SND_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].str.sjis = 
			flag & DISP_REG_SCFG_EXT_MC_MASK ? s_strEnable[1]: s_strEnable[0];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].numLines = 2;

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].str.sjis = 
			flag & DISP_REG_SCFG_EXT_INTC_MASK ? s_strEnable[1]: s_strEnable[0];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].numLines = 2;

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SPI].str.sjis = 
			flag & DISP_REG_SCFG_EXT_SPI_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].str.sjis = 
			flag & DISP_REG_SCFG_EXT_DSEL_MASK ? s_strEnable[1]: s_strEnable[0];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].numLines = 2;

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SIO].str.sjis = 
			flag & DISP_REG_SCFG_EXT_SIO_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_LCDC].str.sjis = 
			flag & DISP_REG_SCFG_EXT_LCDC_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_VRAM].str.sjis = 
			flag & DISP_REG_SCFG_EXT_VRAM_MASK ? s_strEnable[1]: s_strEnable[0];

		{
			u32 psFlag = (flag & DISP_REG_SCFG_EXT_PSRAM_MASK) >> DISP_REG_SCFG_EXT_PSRAM_SHIFT;
			u8 idx = 0;
			
			if( psFlag <= 1 )
			{
				idx = 0;
			}
			else if ( psFlag == 2 )
			{
				idx = 1;
			}
			else if ( psFlag == 3 )
			{
				idx = 2;
			}

			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PS].str.sjis = s_strPSRAM[idx];
		}
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMAC].str.sjis = 
			flag & DISP_REG_SCFG_EXT_DMAC_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_AES].str.sjis = 
			flag & DISP_REG_SCFG_EXT_AES_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD1].str.sjis = 
			flag & DISP_REG_SCFG_EXT_SD1_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD2].str.sjis = 
			flag & DISP_REG_SCFG_EXT_SD2_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MIC].str.sjis = 
			flag & DISP_REG_SCFG_EXT_MIC_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2S].str.sjis = 
			flag & DISP_REG_SCFG_EXT_I2S_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2C].str.sjis = 
			flag & DISP_REG_SCFG_EXT_I2C_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_GPIO].str.sjis = 
			flag & DISP_REG_SCFG_EXT_GPIO_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MCB].str.sjis = 
			flag & DISP_REG_SCFG_EXT_MC_B_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_WRAM].str.sjis = 
			flag & DISP_REG_SCFG_EXT_WRAM_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PU].str.sjis = 
			flag & DISP_REG_SCFG_EXT_PUENABLE_MASK ? s_strEnable[1]: s_strEnable[0];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_CFG].str.sjis = 
			flag & DISP_REG_SCFG_EXT_CFG_MASK ? s_strEnable[1]: s_strEnable[0];
	
	}
	
	// �������J�[�h I/F ���䃌�W�X�^
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MC_OFFSET - 0x4000] );
		u8 idx;
		
		OS_TPrintf("mc flag: %04x\n", flag );
		OS_TPrintf("flag & SC1 cdet mask: %d\n", flag & DISP_REG_MI_MC_SL1_CDET_MASK );
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_CDET].str.sjis = 
			flag & DISP_REG_MI_MC_SL1_CDET_MASK ? s_strBool[1]: s_strBool[0];

		OS_TPrintf("mi SC1 cdet: %s\n", gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_CDET].str.sjis );

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_CDET].str.sjis = 
			flag & DISP_REG_MI_MC_SL2_CDET_MASK ? s_strBool[1]: s_strBool[0];
			
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SWP].str.sjis = 
			flag & DISP_REG_MI_MC_SWP_MASK ? s_strBool[1]: s_strBool[0];

		idx = (u8) ( (flag & DISP_REG_MI_MC_SL1_MODE_MASK) >> DISP_REG_MI_MC_SL1_MODE_SHIFT );
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_MODE].str.sjis = s_strMCMode[idx];

		idx = (u8) ( (flag & DISP_REG_MI_MC_SL2_MODE_MASK) >> DISP_REG_MI_MC_SL2_MODE_SHIFT );
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_MODE].str.sjis = s_strMCMode[idx];
		
		flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MCCHAT_OFFSET - 0x4000] );
		snprintf( gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CC].str.sjis , DISPINFO_BUFSIZE-1, "%04x", 
					flag & DISP_REG_MI_MCCHAT_CC_MASK);
		
		flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MC2_OFFSET - 0x4000] );
		snprintf( gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CA].str.sjis , DISPINFO_BUFSIZE-1, "%04x", 
					flag & DISP_REG_MI_MC2_CA_MASK );
		
	}
	
	// ����������M���䃌�W�X�^
	{
		u8 flag =  gArm7SCFGReg[DISP_REG_WL_OFFSET - 0x4000];

		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_WL_OFFB].str.sjis = 
			flag & DISP_REG_SCFG_WL_OFFB_MASK ? s_strEnable[1]: s_strEnable[0];

	}
	
	
	// �I�v�V�����[�q�ǂݏo�����W�X�^
	{
		
		u8 flag = gArm7SCFGReg[DISP_REG_OP_OFFSET - 0x4000];
		u8 idx =  (u8)(flag & DISP_REG_SCFG_OP_OPT_MASK);
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_FORM].str.sjis = idx == 3 ? s_strRomForm[1] : s_strRomForm[0];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_APP].str.sjis = s_strRomApp[idx];
	}

}


void getVersions( void )
{
	getWirelessVersion();
	getSharedFontVersion();
	getContentsVersion();
}

void getWirelessVersion( void )
// �����t�@�[���E�F�A�̃o�[�W�������擾����
{
	FSFile file;
	char filePath[NAM_PATH_LEN+1];
	char filebuf[2];	// FW�o�[�W������Major, Minor���ꂼ��1�o�C�g����
	int res;

	FS_InitFile( &file );	
	NAM_GetTitleBootContentPath( filePath , WL_TITLEID); // �����t�@�[���̃t�@�C���p�X���擾
	
	res = FS_OpenFileEx( &file, filePath, FS_FILEMODE_R );
	
	// �o�[�W�������̓ǂݎ��
	FS_SeekFile( &file, 0xA0, FS_SEEK_SET ); // �t�@�C����0xA0����2�o�C�g���o�[�W�������
	res = FS_ReadFile( &file, filebuf, 2 );
	SDK_ASSERT( res == 2 );
	
	gAllInfo[MENU_VERSION][VERSION_WIRELESS].iValue = filebuf[0] *100 + filebuf[1];
	gAllInfo[MENU_VERSION][VERSION_WIRELESS].isNumData = TRUE;
}

void getSharedFontVersion( void )
{
	u32 time = OS_GetSharedFontTimestamp();
	gAllInfo[MENU_VERSION][VERSION_FONT].iValue = (int) time;
	gAllInfo[MENU_VERSION][VERSION_FONT].isNumData = TRUE;
}

void getContentsVersion( void )
// �R���e���c���X�g�����ƂɊe�R���e���c�̃^�C�g��ID�ƃo�[�W�������擾
{
	NAMTitleInfo info;
	int i;

	gNumContents = NAM_GetNumTitles();
	
	if( gContentsTitle == NULL )
	{
		// ���񏈗��̎��̓o�b�t�@���m��
		gContentsTitle = (NAMTitleId*) Alloc( sizeof(NAMTitleId) * gNumContents );
		gContentsVersion = (u16*) Alloc( sizeof(u16) * (u32)gNumContents);
	}
			
	NAM_GetTitleList( gContentsTitle, (u32)gNumContents);
	SDK_POINTER_ASSERT( gContentsTitle );

	for( i=0; i<gNumContents; i++ )
	{
		NAM_ReadTitleInfo( &info, gContentsTitle[i] );
		gContentsTitle[i] = info.titleId;
		gContentsVersion[i] = info.version;
	}

}

void displayInfoInit( void )
{
	int loop1, loop2;
	
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"RomTypeTest");
	PutStringUTF16( 4 * 8, 8 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push A To Start Test.");
	PutStringUTF16( 4 * 8, 10 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push X To Start Test Quietly.");

	// �S�̏������z����Z�b�g
	// �S���ڂɕ�����o�b�t�@����������ƃ������������݂������Ȃ�̂�
	// �\�Ȃ��̂͗p�ӂ���static������ւ̃|�C���^�őΉ�
	// �K�v�Ȃ��̂������Ƃ�malloc����
	for(loop1=0; loop1<ROOTMENU_SIZE; loop1++)
	{
		gAllInfo[loop1] = (DispInfoEntry*)Alloc(sizeof(DispInfoEntry) * s_numMenu[loop1]);
				
		SDK_ASSERT( gAllInfo[loop1] );
		
		for(loop2=0; loop2<s_numMenu[loop1]; loop2++)
		{
			// �f�t�H���g�l�̐ݒ�
			gAllInfo[loop1][loop2].kind = s_strMetaMenu[loop1][loop2];
			gAllInfo[loop1][loop2].iValue = 0;
			gAllInfo[loop1][loop2].isNumData = FALSE;
			gAllInfo[loop1][loop2].numLines	= 1;
			gAllInfo[loop1][loop2].isSjis = TRUE;
			gAllInfo[loop1][loop2].isAligned = TRUE;
			gAllInfo[loop1][loop2].str.sjis = s_strNA;
			gAllInfo[loop1][loop2].changable = FALSE;
		}
	}

	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

void printAllInfo ( void )
{
	int loop1, loop2;
	
	for(loop1=0; loop1 < ROOTMENU_SIZE; loop1++ )
	{
		for(loop2=0; loop2 < s_numMenu[loop1]; loop2++ )
		{
			DispInfoEntry *entry = &gAllInfo[loop1][loop2];

			if( entry->isNumData )
			{
				OS_TPrintf("%d %d : %d\n", loop1, loop2, entry->iValue);
			}
			else if( entry->isSjis )
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, entry->str.sjis);
			}
			else
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, entry->str.utf); 
			}
			
			/*
			// utf16�ŕ`�悷����̂���������
			if(loop1 == MENU_OWNER && loop2 == OWNER_NICKNAME)
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, gUserName); 
			}
			else if(loop1 == MENU_OWNER && loop2 == OWNER_COMMENT)
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, gUserComment); 
			}
			else if(loop1 == MENU_PARENTAL && loop2 == PARENTAL_ANSWER)
			{			
				OS_TPrintf("%d %d : %s\n", loop1, loop2, gSecretAnswer); 
			}
			else
			{
				OS_TPrintf("%d %d : %s\n", loop1, loop2, gAllInfo[loop1][loop2]); 
			}
			*/
		}
	}
}

