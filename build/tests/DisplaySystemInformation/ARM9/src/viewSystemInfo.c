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
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <wchar.h>
#include <stdlib.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include <twl/os/common/sharedFont.h>
#include <twl/na.h>
#include "misc.h"
#include "viewSystemInfo.h"
#include "strResource.h"
#include "control.h"
#include "myIoreg_SCFG.h"



#define DISPINFO_BUFSIZE 64
#define WL_TITLEID 0x0003000F484E4341

#define FS_VERSION_FILE					"verdata:/version.bin"
#define FS_EULA_URL_FILE				"verdata:/eula_url.bin"
#define FS_NUP_HOSTNAME_FILE			"verdata:/nup_host.bin"
#define FS_TIMESTAMP_FILE				"verdata:/time_stamp.bin"
#define TWL_SYSMENU_VER_STR_LEN			28				// �V�X�e�����j���[�o�[�W����������MAX bytes
#define TWL_EULA_URL_LEN				128
#define TWL_NUP_HOSTNAME_LEN			64


typedef struct SystemMenuVersion {
	u16		major;
	u16		minor;
	u16		str[ TWL_SYSMENU_VER_STR_LEN / sizeof(u16) ];
}SystemMenuVersion;

/* function prototype  ----------------------------- */
void getAllInfo( void );
void initInfo( void );
void infoAlloc( DispInfoEntry *p, u8 index, u8 size, BOOL isSjis );
void printAllInfo ( void );
void getOwnerInfo( void );
void getParentalInfo( void );
void getOtherInfo();
void getNormalHWInfo( void );
void getSecureHWInfo( void );
void getSCFGARM9Info( void );
void getSCFGARM7InfoReg( void );
void getSCFGARM7InfoShared( void );
void getSysmenuInfo( void );
void getVersions( void );
void getWirelessVersion( void );
void getContentsVersion( void );
void getSharedFontVersion( void );


/* global variables ----------------------------- */

DispInfoEntry* gAllInfo[ROOTMENU_SIZE]; // ���ꂼ��̃��j���[���Ƃɍ��ڂ̒��g��"�������"���z��

// �R���e���c�^�C�g���֌W
OSTitleId *gContentsTitle;		// �R���e���c�^�C�g���̈ꗗ
s32 gNumContents;				// gContentsTitle�Ɋ܂܂��^�C�g���̐�
u16 *gContentsVersion;			// gContentsTitle���ꂼ��̃o�[�W�������

// ARM7����SCFG�f�[�^��ێ�
u8 gArm7SCFGReg[DISPINFO_SHARED_SCFG_REG_SIZE];
u8 gArm7SCFGShared[DISPINFO_SHARED_SCFG_WRAM_SIZE];

// LCFG�f�[�^��ǂݍ��ނ��߂̃o�b�t�@
u8 *bufLCFG;
/* static data ---------------------------------- */


/* function definition -------------------------- */

void displayInfoMain( void )
{
	static int menu = MENU_ROOT;
	static int line = 0; 		// ���ݑI�����Ă���y�[�W�i���o�ƍs
	static int changeLine = 0;	// changeMode�p
	static BOOL firstCall = TRUE;	// �ŏ��̌Ăяo�����ۂ�
	static BOOL isChangeMode = FALSE;
	
	BOOL repaintFlag = FALSE;
	ChangeCotnrolResult ccResult = CHANGE_NOTHING;
	
	// ����̌Ăяo�����͑S�Ă̏����擾���Ȃ���
	if( firstCall )
	{
		initInfo();
		getAllInfo();
	}
	
	if( isChangeMode )
	{
		 ccResult = changeControl( &menu, &line, &changeLine, &isChangeMode );
	}
	else
	{
		repaintFlag = control( &menu, &line, &changeLine, &isChangeMode );
	}

	if( ccResult == CHANGE_VALUE_CHANGED )
	{
		// �l���X�V���ꂽ�Ƃ��͑S���擾���Ȃ���
		LCFG_WriteTWLSettings( (u8 (*) [LCFG_WRITE_TEMP] ) bufLCFG );
		getAllInfo();
	}
	
	// �p�b�h���őI��ł�ꏊ�Ƃ����Ƃ��X�V
	if(	ccResult == CHANGE_VALUE_CHANGED || ccResult == CHANGE_CONTROL || repaintFlag || firstCall )
	{
		// �������삪�������Ƃ��̓L�����o�X�N���A���ĕ`�悵�Ȃ���
	    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
   	    NNS_G2dCharCanvasClear( &gCanvasSub, TXT_COLOR_WHITE );
		// ���ꗗ��`�悷��
		drawMenu( menu, line, changeLine, isChangeMode);

		firstCall = FALSE;
	}
	
}

void initInfo( void )
// �Œ蕶����őΉ��ł��Ȃ����ڂɕ�����̈�����蓖�Ă�
{
	OS_TPrintf("buffer initialize\n");
	

	infoAlloc( gAllInfo[MENU_OWNER], OWNER_BIRTHDAY, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_OTHER], OTHER_LCFG_LASTBOOT_ID, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_NORMAL_HW], NORMAL_HW_UNIQUE_ID, OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_SERIAL , OS_TWL_HWINFO_SERIALNO_LEN_MAX + 1, TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_LANGUAGE, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_FUSE, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SECURE_HW], SECURE_HW_TITLEID_LO, DISPINFO_BUFSIZE, TRUE );
	infoAlloc( gAllInfo[MENU_SCFG_ARM7], SCFG_ARM7_MI_CC, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SCFG_ARM7], SCFG_ARM7_MI_CA, DISPINFO_BUFSIZE , TRUE );
	infoAlloc( gAllInfo[MENU_SYSMENU], SYSMENU_EULA_URL, TWL_EULA_URL_LEN + 1, FALSE );
	infoAlloc( gAllInfo[MENU_SYSMENU], SYSMENU_NUP_HOST, TWL_NUP_HOSTNAME_LEN + 1, FALSE );

	infoAlloc( gAllInfo[MENU_OWNER], OWNER_NICKNAME, OS_OWNERINFO_NICKNAME_MAX + 1 , FALSE );
	infoAlloc( gAllInfo[MENU_OWNER], OWNER_COMMENT, OS_OWNERINFO_COMMENT_MAX + 1 , FALSE );
	infoAlloc( gAllInfo[MENU_PARENTAL], PARENTAL_ANSWER, OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1 , FALSE );
	infoAlloc( gAllInfo[MENU_SYSMENU], SYSMENU_VERSION_STR, TWL_SYSMENU_VER_STR_LEN + 1, FALSE );

	OS_TPrintf( "information alloc succeeded\n" );
}

	
void infoAlloc( DispInfoEntry *p, u8 index, u8 size, BOOL isSjis )
// initInfo�̍�Ɨp�֐��BisSjis=true����char���Afalse�Ȃ�u16�����蓖�Ă�
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
	getOtherInfo();
	getNormalHWInfo();
	getSecureHWInfo();
	getSCFGARM7InfoReg();
	getSCFGARM7InfoShared();
	getSCFGARM9Info();
	getSysmenuInfo();
	getVersions();

	
	printAllInfo();
	OS_TPrintf("reflesh information finished\n");
}


void displayInfoInit( void )
{
	int loop1, loop2;
	
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	// �S�̏������z����Z�b�g
	// �\�Ȃ��̂͗p�ӂ���static������ւ̃|�C���^�őΉ�
	// �K�v�Ȃ��̂������Ƃ�malloc����
	for(loop1=0; loop1<ROOTMENU_SIZE; loop1++)
	{
		// ARM7���j���[�̂Ƃ������o�b�t�@���{�����
		int numBuf = loop1 == MENU_SCFG_ARM7 ? SCFG_ARM7_MENU_SIZE*2 : s_numMenu[loop1];
		
		gAllInfo[loop1] = (DispInfoEntry*)Alloc(sizeof(DispInfoEntry) * numBuf);
				
		SDK_ASSERT( gAllInfo[loop1] );
				
		for(loop2 = 0; loop2 < numBuf; loop2++)
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
			gAllInfo[loop1][loop2].fromLCFG = FALSE;
		}
	}
	
	bufLCFG = (u8*) Alloc ( LCFG_READ_TEMP );
	SDK_ASSERT( bufLCFG );
	LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ]) bufLCFG );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

void printAllInfo ( void )
{
	int loop1, loop2;
	OS_TPrintf(" size version: %d\n", s_numMenu[MENU_VERSION]);
	
	for(loop1=0; loop1 < ROOTMENU_SIZE; loop1++ )
	{
		for(loop2=0; loop2 < s_numMenu[loop1]; loop2++ )
		{
			DispInfoEntry *entry;
			
			if( loop1 == MENU_VERSION && loop2 >= MENU_OTHER )
			{
				int idx = loop2 - MENU_OTHER;
				OS_TPrintf("%d %d : %x %x\n", loop1, loop2, gContentsTitle[idx], gContentsVersion[idx] );
				continue;
			}
			
			entry = &gAllInfo[loop1][loop2];

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
			
		}
	}
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

void getOtherInfo( void ) 
{
	int value;
	
	value = OS_IsAgreeEULA();
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA].str.sjis = s_strBool[value];
		
	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION].iValue = OS_GetAgreedEULAVersion();
	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION].isNumData = TRUE;
	
	value = OS_IsAvailableWireless();
	gAllInfo[MENU_OTHER][OTHER_WIRELESS].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_WIRELESS].str.sjis = s_strEnable[value];

	value = LCFG_TSD_IsFinishedInitialSetting();
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].str.sjis = s_strBool[ value ];
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].changable = TRUE;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].changeFunc.cBool = LCFG_TSD_SetFlagFinishedInitialSetting;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].argType = ARG_BOOL;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].kindNameList = s_strBool;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].numKindName = 2;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_SETTINGS].fromLCFG = TRUE;
	
	value = LCFG_TSD_IsFinishedInitialSetting_Launcher();
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].str.sjis = s_strBool[ value ];
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].changable = TRUE;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].changeFunc.cBool = LCFG_TSD_SetFlagFinishedInitialSetting_Launcher;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].argType = ARG_BOOL;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].kindNameList = s_strBool;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].numKindName = 2;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_INITIAL_LAUNCHER].fromLCFG = TRUE;
	
	value = LCFG_TSD_IsFinishedBrokenTWLSettings();
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].str.sjis = s_strBool[ value ];
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].changable = TRUE;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].changeFunc.cBool = LCFG_TSD_SetFlagFinishedBrokenTWLSettings;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].argType = ARG_BOOL;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].kindNameList = s_strBool;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].numKindName = 2;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_BROKEN_SETTINGS].fromLCFG = TRUE;

	value = LCFG_TSD_GetInstalledSoftBoxCount();
	gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_INSTALLED_SOFTBOX].fromLCFG = TRUE;
	
	
	value = LCFG_TSD_GetFreeSoftBoxCount();
	gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_FREE_SOFTBOX].fromLCFG = TRUE;
	
	value = LCFG_TSD_GetLastTimeBootSoftIndex();
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_IDX].fromLCFG = TRUE;
	
	value = LCFG_TSD_GetLastTimeBootSoftPlatform();
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].iValue = value;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].isNumData = TRUE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].isAligned = FALSE;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].numLines = 2;
	gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_PLATFORM].fromLCFG = TRUE;
	

	{
		u64 buf = LCFG_TSD_GetLastTimeBootSoftTitleID();
		
		gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].isAligned = FALSE;
		gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].numLines = 2;
		gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].fromLCFG = TRUE;
		
		MI_CpuCopy( &buf, gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].str.sjis, 8 );
		gAllInfo[MENU_OTHER][OTHER_LCFG_LASTBOOT_ID].str.sjis[8] = '\0';
	}
}

void getNormalHWInfo( void )
{
	int value;

	value = (int) OS_GetOwnerRtcOffset();
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_RTC_OFFSET].iValue = (int) value;
	gAllInfo[MENU_NORMAL_HW][NORMAL_HW_RTC_OFFSET].isNumData = TRUE;

	{
		int i;
		char ascii[] = "0123456789abcdef";
		const u8 *unq = OS_GetMovableUniqueIDPtr();
		// 16�i��1�o�C�g���l�߂Ă���
		// �o�b�t�@��������3�{���Ȃ̂́A�f�[�^��"%02x-%02x-%02x..."�ɒu�����邽��
		for(i=0; i < OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3; i += 3, unq++ )
		{	
			gAllInfo[MENU_NORMAL_HW][NORMAL_HW_UNIQUE_ID].str.sjis[i] = ascii[(*unq>>4) & 0x0f];
			gAllInfo[MENU_NORMAL_HW][NORMAL_HW_UNIQUE_ID].str.sjis[i+1] = ascii[*unq & 0x0f];
			gAllInfo[MENU_NORMAL_HW][NORMAL_HW_UNIQUE_ID].str.sjis[i+2] = 
				(i+2) == (OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 - 1) ? (char)'\0' : (char)'-' ;
			OS_TPrintf("uniqid: %d\n", *unq);
		}
	}

}

void getSecureHWInfo( void )
{
	int value;
	
	value = OS_IsForceDisableWireless();
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].iValue = value;
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].str.sjis = s_strBool[ value ];
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].isAligned = FALSE;
	gAllInfo[MENU_SECURE_HW][SECURE_HW_FORCE_DISABLE].numLines = 2;
	
	value = OS_GetRegion();
	gAllInfo[MENU_SECURE_HW][SECURE_HW_REGION].iValue = value;
	gAllInfo[MENU_SECURE_HW][SECURE_HW_REGION].str.sjis = s_strRegion[ value ];

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
		gAllInfo[MENU_SECURE_HW][SECURE_HW_FUSE].numLines = 2;

	}
	
	// �����`���̃^�C�g��IDLo�̎擾
	if( LCFG_ReadHWSecureInfo() )
	{
		const u8 *titleIDLo = LCFG_THW_GetLauncherTitleID_LoPtr();
		value = ( int )(	titleIDLo[0] << 8*3 |
							titleIDLo[1] << 8*2 |
							titleIDLo[2] << 8*1 |
							titleIDLo[3] << 8*0 );

		gAllInfo[MENU_SECURE_HW][SECURE_HW_TITLEID_LO].iValue = value;
		gAllInfo[MENU_SECURE_HW][SECURE_HW_TITLEID_LO].isNumData = TRUE;;
		gAllInfo[MENU_SECURE_HW][SECURE_HW_TITLEID_LO].isAligned = FALSE;
		gAllInfo[MENU_SECURE_HW][SECURE_HW_TITLEID_LO].numLines = 2;
		gAllInfo[MENU_SECURE_HW][SECURE_HW_TITLEID_LO].fromLCFG = TRUE;	
	}

}

void getSCFGARM9Info( void )
// ARM9���Ŏ擾�ł���SCFG�����擾����
// ARM9SCFGAPI�̓��W�X�^�𒼐ڎQ�Ƃ��Ă���̂ŁAAPI���g���Ă����W�X�^�𒼐ڌ��Ă������l
{
	int value;

	// ROM���䃌�W�X�^	
	
	// IsSecureRomAccessible�̕Ԃ�l�̓��W�X�^�r�b�g�����]
	value = ! SCFG_IsSecureRomAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_SEC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_SEC].str.sjis = s_strJoint[ value ];
	
	value = SCFG_GetSystemRomType() == SCFG_SYSTEM_ROM_FOR_NITRO;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_STATE].iValue = 	value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_STATE].str.sjis = s_strRomMode[ value ];
	
	// �N���b�N���䃌�W�X�^
	value = SCFG_GetCpuSpeed();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].str.sjis = s_strCpuSpeed[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].changeFunc.cBool = SCFG_SetCpuSpeed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].kindNameList = s_strCpuSpeed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].numKindName = 2;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].changable = TRUE;

	value = SCFG_IsClockSuppliedToDSP();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].str.sjis = s_strSupply[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].changeFunc.cBool = SCFG_SupplyClockToDSP;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].kindNameList = s_strSupply;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].numKindName = 2;

	value = SCFG_IsClockSuppliedToCamera();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].str.sjis = s_strSupply[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].changeFunc.cBool = SCFG_SupplyClockToCamera;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].kindNameList = s_strSupply;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].numKindName = 2;

	value = SCFG_IsClockSuppliedToWram();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_WRAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_WRAM].str.sjis = s_strSupply[ value ];	

	value = SCFG_IsCameraCKIClockEnable();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].changeFunc.cBool = SCFG_SetCameraCKIClock;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].numKindName = 2;

	// �V�K�u���b�N���䃌�W�X�^
	value = SCFG_IsDSPReset();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].str.sjis = s_strBool[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].argType = ARG_OTHER;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].kindNameList = s_strBool;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].numKindName = 2;

	// �g���@�\���䃌�W�X�^

	value =  SCFG_IsDmacFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].changeFunc.cBool = SCFG_SetDmacFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].numKindName = 2;

	value =  SCFG_IsGeometryFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].changeFunc.cBool = SCFG_SetGeometryFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].numKindName = 2;

	value =  SCFG_IsRendererFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].changeFunc.cBool = SCFG_SetRendererFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].numKindName = 2;

	value =  SCFG_Is2DEngineFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].changeFunc.cBool = SCFG_Set2DEngineFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].numKindName = 2;

	value =  SCFG_IsDividerFixed();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].changeFunc.cBool = SCFG_SetDividerFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].numKindName = 2;
	
	value =  SCFG_IsCardFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].changeFunc.cBool = SCFG_SetCardFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].numKindName = 2;

	value = SCFG_IsIntcExpanded();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].changeFunc.cBool = SCFG_SetIntcExpanded;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].numKindName = 2;
	
	value = SCFG_IsLCDCExpanded();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].changeFunc.cBool = SCFG_SetLCDCExpanded;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].numKindName = 2;
	
	value = SCFG_IsVRAMExpanded();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].changeFunc.cBool = SCFG_SetVRAMExpanded;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].numKindName = 2;

	{
		u8 idx;
		value = SCFG_GetPsramdBoundary();
		
		if( value  == SCFG_PSRAM_BOUNDARY_4MB )
		{
			idx = 0;
		}
		else if ( value == SCFG_PSRAM_BOUNDARY_16MB )
		{
			idx = 1;
		}
		else if ( value == SCFG_PSRAM_BOUNDARY_32MB )
		{
			idx = 2;
		}
		
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].iValue = idx;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].str.sjis = s_strPSRAM[ idx ];
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].changable = TRUE;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].argType = ARG_OTHER;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].kindNameList = s_strPSRAM;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].numKindName = 3;
	}
	
	value = SCFG_IsNDmaAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].str.sjis = s_strAccess[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].changeFunc.cBool = SCFG_SetNDmaAccessible;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].kindNameList = s_strAccess;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].numKindName = 2;
	
	value = SCFG_IsCameraAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].str.sjis = s_strAccess[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].changeFunc.cBool = SCFG_SetCameraAccessible;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].kindNameList = s_strAccess;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].numKindName = 2;
	
	value = SCFG_IsDSPAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].str.sjis = s_strAccess[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].changeFunc.cBool = SCFG_SetDSPAccessible;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].kindNameList = s_strAccess;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].numKindName = 2;
	
	value = (reg_SCFG_EXT & REG_SCFG_EXT_MC_B_MASK) || 0;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MCB].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MCB].str.sjis = s_strAccess[ value ];
	
	value = SCFG_IsWRAMAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_WRAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_WRAM].str.sjis = s_strAccess[ value ];

	value = SCFG_IsConfigBlockAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].str.sjis = s_strAccess[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].argType = ARG_OTHER;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].kindNameList = s_strAccess;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].numKindName = 2;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].isAligned = FALSE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].numLines = 2;
}

void getSCFGARM7InfoReg( void )
{
	// ���W�X�^�ɒ��ڊi�[����Ă���ق���SCFG�f�[�^���擾
	

	int value;
	
	// ROM���䃌�W�X�^(L)�A(H)
	{
		// SEC�t���O��TRUE = �؂藣��(�A�N�Z�X�s��),  FALSE = �ڑ�(�A�N�Z�X��)
		value = ( gArm7SCFGReg[DISP_REG_A9ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A9ROM_SEC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_SEC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_SEC].str.sjis = s_strJoint[ value ];
			
		value = ( gArm7SCFGReg[DISP_REG_A9ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A9ROM_RSEL_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_RSEL].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_RSEL].str.sjis = s_strRomMode[ value ];

		// SEC�t���O��TRUE = �؂藣��(�A�N�Z�X�s��),  FALSE = �ڑ�(�A�N�Z�X��)
		value = ( gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_SEC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_SEC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_SEC].str.sjis = s_strJoint[ value ];
			
		value = ( gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_RSEL_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_RSEL].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_RSEL].str.sjis = s_strRomMode[ value ];
			
		// FuseROM�t���O��TRUE = �؂藣��(�A�N�Z�X�s��),  FALSE = �ڑ�(�A�N�Z�X��)
		value = ( gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_FUSE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_FUSE].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_FUSE].str.sjis = s_strJoint[ value ];
			
		value = ( gArm7SCFGReg[DISP_REG_ROMWE_OFFSET - 0x4000] & DISP_REG_SCFG_ROMWE_WE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_WE].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_WE].str.sjis = s_strEnable[ value ];
	}
		
	// �V�K�u���b�N�N���b�N���䃌�W�X�^
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_CLK_OFFSET - 0x4000] );
		
		value = ( flag & DISP_REG_SCFG_CLK_SD1HCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD1].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD1].str.sjis = s_strSupply[ value ];
			
		value = ( flag & DISP_REG_SCFG_CLK_SD2HCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD2].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD2].str.sjis = s_strSupply[ value ];
		
		value = ( flag & DISP_REG_SCFG_CLK_AESHCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_AES].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_AES].str.sjis = s_strSupply[ value ];
			
		value = ( flag & DISP_REG_SCFG_CLK_WRAMHCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_WRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_WRAM].str.sjis = s_strSupply[ value ];
			
		value = ( flag & DISP_REG_SCFG_CLK_SNDMCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SND].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SND].str.sjis = s_strSupply[ value ];
	}
	
	// JTAG���䃌�W�X�^
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_JTAG_OFFSET - 0x4000] );
		
		value = ( flag & DISP_REG_SCFG_JTAG_ARM7SEL_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].numLines = 2;

		value = ( flag & DISP_REG_SCFG_JTAG_CPUJE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_CPU].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_CPU].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_JTAG_DSPJE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_DSP].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_DSP].str.sjis = s_strEnable[ value ];
	}	
	
	// �g���@�\���䃌�W�X�^
	{
		u32 flag = MI_LoadLE32( &gArm7SCFGReg[DISP_REG_EXT_OFFSET - 0x4000] );
		
		value = ( flag & DISP_REG_SCFG_EXT_DMA_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMA].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMA].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_SDMA_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SDMA].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SDMA].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_SND_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SND].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SND].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_MC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].numLines = 2;

		value = ( flag & DISP_REG_SCFG_EXT_INTC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].numLines = 2;

		value = ( flag & DISP_REG_SCFG_EXT_SPI_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SPI].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SPI].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_DSEL_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].numLines = 2;

		value = ( flag & DISP_REG_SCFG_EXT_SIO_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SIO].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SIO].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_LCDC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_LCDC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_LCDC].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_VRAM_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_VRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_VRAM].str.sjis = s_strEnable[ value ];

		{
			u8 idx = 0;
			value = (int) ( (flag & DISP_REG_SCFG_EXT_PSRAM_MASK) >> DISP_REG_SCFG_EXT_PSRAM_SHIFT );

			
			if( value <= 1 )
			{
				idx = 0;
			}
			else if ( value == 2 )
			{
				idx = 1;
			}
			else if ( value == 3 )
			{
				idx = 2;
			}

			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PS].iValue = value;
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PS].str.sjis = s_strPSRAM[idx];
		}
		
		value = ( flag & DISP_REG_SCFG_EXT_DMAC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMAC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMAC].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_AES_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_AES].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_AES].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_SD1_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD1].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD1].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_SD2_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD2].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD2].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_MIC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MIC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MIC].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_I2S_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2S].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2S].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_I2C_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2C].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2C].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_GPIO_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_GPIO].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_GPIO].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_MC_B_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MCB].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MCB].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_WRAM_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_WRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_WRAM].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_PUENABLE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PU].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PU].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_CFG_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_CFG].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_CFG].str.sjis = s_strEnable[ value ];
	
	}
	
	// �������J�[�h I/F ���䃌�W�X�^
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MC_OFFSET - 0x4000] );
		
		value = ( flag & DISP_REG_MI_MC_SL1_CDET_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_CDET].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_CDET].str.sjis = s_strBool[ value ];

		value = ( flag & DISP_REG_MI_MC_SL2_CDET_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_CDET].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_CDET].str.sjis = s_strBool[ value ];
			
		value = ( flag & DISP_REG_MI_MC_SWP_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SWP].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SWP].str.sjis = s_strBool[ value ];

		value = (flag & DISP_REG_MI_MC_SL1_MODE_MASK) >> DISP_REG_MI_MC_SL1_MODE_SHIFT ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_MODE].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_MODE].str.sjis = s_strMCMode[value];

		value = (flag & DISP_REG_MI_MC_SL2_MODE_MASK) >> DISP_REG_MI_MC_SL2_MODE_SHIFT;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_MODE].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_MODE].str.sjis = s_strMCMode[value];
		
		flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MCCHAT_OFFSET - 0x4000] );
		value = (flag & DISP_REG_MI_MCCHAT_CC_MASK) >> DISP_REG_MI_MCCHAT_CC_SHIFT;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CC].iValue = value;
		snprintf( gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CC].str.sjis , DISPINFO_BUFSIZE-1, "%04x", value);
		
		flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MC2_OFFSET - 0x4000] );
		value = (flag & DISP_REG_MI_MC2_CA_MASK ) >> DISP_REG_MI_MC2_CA_SHIFT;
		;gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CA].iValue = value;
		snprintf( gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CA].str.sjis , DISPINFO_BUFSIZE-1, "%04x", value );
		
	}
	
	// ����������M���䃌�W�X�^
	{
		u8 flag =  gArm7SCFGReg[DISP_REG_WL_OFFSET - 0x4000];

		value = ( flag & DISP_REG_SCFG_WL_OFFB_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_WL_OFFB].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_WL_OFFB].str.sjis = s_strEnable[ value ];

	}
	
	
	// �I�v�V�����[�q�ǂݏo�����W�X�^
	{
		
		u8 flag = gArm7SCFGReg[DISP_REG_OP_OFFSET - 0x4000];
		value =  (flag & DISP_REG_SCFG_OP_OPT_MASK) >> DISP_REG_SCFG_OP_OPT_SHIFT;
				
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_FORM].iValue = (value & 0x2) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_FORM].str.sjis = value == 3 ? s_strRomForm[1] : s_strRomForm[0];
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_APP].iValue = (value & 0x1) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_APP].str.sjis = s_strRomApp[ value ];
	}

}

void getSCFGARM7InfoShared( void )
{
	// ���L�̈�ɑޔ����ꂽ�ق���SCFG�f�[�^���擾����
	int value;
	
	// �g���@�\���䃌�W�X�^ (4byte)
	{
		u32 extData = MI_LoadLE32( &gArm7SCFGShared[HWi_WSYS04_OFFSET] );
		
		value = (extData & DISP_REG_SCFG_EXT_DMA_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DMA].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DMA].str.sjis = s_strEnable[ value ];
		
		value = (extData & DISP_REG_SCFG_EXT_SDMA_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SDMA].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SDMA].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_SND_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SND].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SND].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_MC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MC].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MC].numLines = 2;

		value = (extData & DISP_REG_SCFG_EXT_INTC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_INTC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_INTC].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_INTC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_INTC].numLines = 2;

		value = (extData & DISP_REG_SCFG_EXT_SPI_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SPI].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SPI].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_DSEL_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DSEL].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DSEL].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DSEL].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DSEL].numLines = 2;

		value = (extData & DISP_REG_SCFG_EXT_LCDC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_LCDC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_LCDC].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_VRAM_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_VRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_VRAM].str.sjis = s_strEnable[ value ];

		{
			u8 idx = 0;
			value = (int) ( (extData & HWi_WSYS04_EXT_PSRAM_MASK ) >> HWi_WSYS04_EXT_PSRAM_SHIFT );
			
			if( value <= 1 )
			{
				idx = 0;
			}
			else if ( value == 2 )
			{
				idx = 1;
			}
			else if ( value == 3 )
			{
				idx = 2;
			}

			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_PS].iValue = value;
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_PS].str.sjis = s_strPSRAM[idx];
		}

		value = (extData & DISP_REG_SCFG_EXT_DMAC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DMAC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DMAC].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_AES_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_AES].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_AES].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_SD1_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SD1].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SD1].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_SD2_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SD2].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SD2].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_MIC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MIC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MIC].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_I2S_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_I2S].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_I2S].str.sjis = s_strEnable[ value ];
				
		value = (extData & DISP_REG_SCFG_EXT_I2C_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_I2C].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_I2C].str.sjis = s_strEnable[ value ];
		
		value = (extData & DISP_REG_SCFG_EXT_GPIO_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_GPIO].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_GPIO].str.sjis = s_strEnable[ value ];
		
		value = (extData & DISP_REG_SCFG_EXT_MC_B_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MCB].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MCB].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_WRAM_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_WRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_WRAM].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_PUENABLE_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_PU].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_PU].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_CFG_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_CFG].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_CFG].str.sjis = s_strEnable[ value ];
	}
	
	// �{���f�B���O�Arom��ԁA���������W�X�^ ( 1byte )
	{
		u8 regData = gArm7SCFGShared[HWi_WSYS08_OFFSET];
		
		value = (regData & HWi_WSYS08_OP_OPT_MASK) >> HWi_WSYS08_OP_OPT_SHIFT ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_OP_FORM].iValue = (value & 0x2) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_OP_FORM].str.sjis = value == 3 ? s_strRomForm[1] : s_strRomForm[0];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_OP_APP].iValue = (value & 0x1) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_OP_APP].str.sjis = s_strRomApp[ value ];
		
		// rom����
		value = ( regData & HWi_WSYS08_ROM_ARM9SEC_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM9_SEC ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM9_SEC ].str.sjis = s_strAccess[ !value ] ;
		
		value = ( regData & HWi_WSYS08_ROM_ARM9RSEL_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM9_RSEL ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM9_RSEL ].str.sjis = s_strRomMode[value] ;
		
		value = ( regData & HWi_WSYS08_ROM_ARM7RSEL_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM7_RSEL ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM7_RSEL ].str.sjis = s_strRomMode[value] ;
		
		value = ( regData & HWi_WSYS08_ROM_ARM7FUSE_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM7_FUSE ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM7_FUSE ].str.sjis = s_strAccess[ !value ] ;
		
		// 
		value = ( regData & HWi_WSYS08_WL_OFFB_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_WL_OFFB ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_WL_OFFB ].str.sjis = s_strEnable[value] ;
	
	}
	
	// jtag�Aclk���W�X�^ ( 1byte )
	{
		u8 regData = gArm7SCFGShared[HWi_WSYS09_OFFSET];
		
		value = ( regData & HWi_WSYS09_JTAG_ARM7SEL_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_A7 ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_A7 ].str.sjis = s_strEnable[value] ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_A7 ].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_A7 ].numLines = 2;
		
		value = ( regData & HWi_WSYS09_JTAG_CPUJE_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_CPU ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_CPU ].str.sjis = s_strEnable[value] ;
		
		value = ( regData & HWi_WSYS09_JTAG_DSPJE_MASK  ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_DSP ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_DSP ].str.sjis = s_strEnable[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_SD1HCLK_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SD1 ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SD1 ].str.sjis = s_strSupply[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_SD2HCLK_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SD2 ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SD2 ].str.sjis = s_strSupply[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_AESHCLK_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_AES ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_AES ].str.sjis = s_strSupply[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_WRAMHCLK_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_WRAM ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_WRAM].str.sjis = s_strSupply[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_SNDMCLK_MASK  ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SND ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SND ].str.sjis = s_strSupply[value] ;
	}
	
	
}

void getSysmenuInfo( void )
{
	u8 *pBuffer = (u8*) Alloc (NA_VERSION_DATA_WORK_SIZE);
	
	if( !NA_LoadVersionDataArchive( pBuffer, NA_VERSION_DATA_WORK_SIZE ) ) {
		return ;
	}
	
	// �o�[�W�����̓ǂݏo��
    {
        FSFile file;
        SystemMenuVersion bufVersion;
        s32 len;
		
        FS_InitFile(&file);
		
        if (!FS_OpenFileEx(&file, FS_VERSION_FILE, FS_FILEMODE_R))
        {
            return ;
        }
		
        len = FS_ReadFile(&file, &bufVersion, sizeof(SystemMenuVersion));
        FS_CloseFile(&file);
		        
        gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_NUM].iValue = (int)( bufVersion.major << 16 | bufVersion.minor );
        gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_NUM].isNumData = TRUE;
        
		wcsncpy( gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_STR].str.utf, bufVersion.str, TWL_SYSMENU_VER_STR_LEN );
		gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_STR].isSjis = FALSE;
		
    }
	
	// EULA URL�̓ǂݏo��
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if (!FS_OpenFileEx(&file, FS_EULA_URL_FILE, FS_FILEMODE_R)) {
            return;
        }
		
        len = FS_ReadFile(&file, gAllInfo[MENU_SYSMENU][SYSMENU_EULA_URL].str.sjis , TWL_EULA_URL_LEN) ;
        FS_CloseFile(&file);
		
    }
	
	// NUP HOST NAME �̓ǂݏo��
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if (!FS_OpenFileEx(&file, FS_NUP_HOSTNAME_FILE, FS_FILEMODE_R)) {
    		return;
        }
		
        len = FS_ReadFile(&file, gAllInfo[MENU_SYSMENU][SYSMENU_NUP_HOST].str.sjis, TWL_NUP_HOSTNAME_LEN);
        FS_CloseFile(&file);
		
    }
	
	// �^�C���X�^���v �̓ǂݏo��
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if (!FS_OpenFileEx(&file, FS_TIMESTAMP_FILE, FS_FILEMODE_R)) {
            return ;
        }
		
        len = FS_ReadFile(&file, &gAllInfo[MENU_SYSMENU][SYSMENU_TIMESTAMP].iValue, sizeof(u32) );
        FS_CloseFile(&file);
        
		gAllInfo[MENU_SYSMENU][SYSMENU_TIMESTAMP].isNumData = TRUE;
    }
	
	// SystemMenuVersion�̃A���}�E���g
	if( !NA_UnloadVersionDataArchive() ) {
		return;
	}
	
	Free(pBuffer);

}

void getVersions( void )
{
	
#ifdef SEA_ENABLE
	getWirelessVersion();
	getContentsVersion();
#endif

	getSharedFontVersion();

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
	OS_TPrintf("wireless firm path: %s\n", filePath ) ;
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
	u32 time;
	
	OS_InitSharedFont();
	time = OS_GetSharedFontTimestamp();
	OS_TPrintf("SharedFont Time Stamp %08lx\n", time );
	gAllInfo[MENU_VERSION][VERSION_FONT].iValue = (int) time;
	gAllInfo[MENU_VERSION][VERSION_FONT].isNumData = TRUE;
}

void getContentsVersion( void )
// �R���e���c���X�g�����ƂɊe�R���e���c�̃^�C�g��ID�ƃo�[�W�������擾
{
	NAMTitleInfo info;
	int i;

	gNumContents = NAM_GetNumTitles();
	OS_TPrintf(" numContents: %d\n", gNumContents);	
	
	if( gContentsTitle == NULL )
	{
		// ���񏈗��̎��̓o�b�t�@���m��
		gContentsTitle = (NAMTitleId*) Alloc( sizeof(NAMTitleId) * gNumContents );
		gContentsVersion = (u16*) Alloc( sizeof(u16) * (u32)gNumContents);
		SDK_ASSERT( gContentsTitle );
		SDK_ASSERT( gContentsVersion );
	}
			
	NAM_GetTitleList( gContentsTitle, (u32)gNumContents);
	SDK_POINTER_ASSERT( gContentsTitle );

	for( i=0; i<gNumContents; i++ )
	{
		NAM_ReadTitleInfo( &info, gContentsTitle[i] );
//		gContentsTitle[i] = info.titleId;
		gContentsVersion[i] = info.version;
	}
	
	s_numMenu[MENU_VERSION] += gNumContents;	
}


