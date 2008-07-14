/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInfo
  File:     viewSystemInfo.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include <twl/os/common/ownerInfoEx_private.h>
#include "misc.h"
#include "viewSystemInfo.h"
#include "strResource.h"

#define ID_BUFSIZE 2
#define AGE_BUFSIZE 4
#define BIRTHDAY_BUFSIZE 6

/* function prototype  ----------------------------- */
void getAllInfo( void );
void initInfo( void );
void infoAlloc( char* p, int size, const char *name);
void control( u8 *menu, u8 *page, u8 *line );
void printAllInfo ( void );


/* global variables ----------------------------- */
RTCDrawProperty g_rtcDraw = {
	TRUE, RTC_DATE_TOP_X, RTC_DATE_TOP_Y, RTC_TIME_TOP_X, RTC_TIME_TOP_Y
};

char** gAllInfo[ROOTMENU_KSIZE]; // ���ꂼ��̃��j���[���Ƃɍ��ڂ̒��g��"�������"���z��
u16 gUserName[OS_OWNERINFO_NICKNAME_MAX + 1];
u16 gUserComment[OS_OWNERINFO_COMMENT_MAX + 1];
u16 gSecretAnswer[OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1];


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


/* static data ---------------------------------- */

// ���j���[���ڐ�
static const u8 s_numMenu[11]; //�b��

// �����񃊃\�[�X
/*
static const u16 *s_strRootMenu[ROOTMENU_SIZE];
static const char *s_strMenuName[ROOTMENU_SIZE];
static const u16 *s_strOwnerMenu[OWNERMENU_SIZE];
static const u16 *s_strParentalMenu[PARENTALMENU_SIZE];
static char *s_strCountry[ 256 ];
static char *s_strRegion[ OS_TWL_REGION_MAX ];
static char *s_strUserColor[ OS_FAVORITE_COLOR_MAX ];
static char *s_strLanguage[ OS_LANGUAGE_CODE_MAX ];
static char *s_strBool[2];
static char *s_strRatingOrg[OS_TWL_PCTL_OGN_MAX];
*/

/* function definition -------------------------- */

void displayInfoMain( void )
{
	static u8 menu = MENU_ROOT;
	static u8 page = 0, line = 0; // ���ݑI�����Ă���y�[�W�i���o�ƍs

	static BOOL firstCall = TRUE;	// �ŏ��̌Ăяo�����ۂ�
	
	// ����̌Ăяo�����͑S�Ă̏����擾���Ȃ���
	if( firstCall )
	{
		firstCall = FALSE;
		initInfo();
		getAllInfo();
	}
	
	// �p�b�h���őI��ł�ꏊ�Ƃ����Ƃ��X�V
	control( &menu, &page, &line );
		
	
	// �ύX�\�ȍ��ڂł`�{�^����������ύX���[�h�Ɉڂ�Ƃ�
	// �a�{�^���ŃW�������ɖ߂�Ƃ�����Ȃ���
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
    
	drawHeader(menu, page, line);
	drawMenu(menu, page, line); // ���ꗗ��`�悷��
	GetAndDrawRTCData( &g_rtcDraw, FALSE ); // RTC�X�V

}

void initInfo( void )
{
	OS_TPrintf("buffer initialize\n");
	
	// �Œ蕶����őΉ��ł��Ȃ����ڂɕ�����̈�����蓖�Ă�
	gAllInfo[MENU_OWNER][OWNER_BIRTHDAY] = (char*) Alloc (sizeof(char) * BIRTHDAY_BUFSIZE);
	SDK_ASSERT( gAllInfo[MENU_OWNER][OWNER_BIRTHDAY] );

	gAllInfo[MENU_PARENTAL][PARENTAL_AGE] = (char*) Alloc (sizeof(char) * AGE_BUFSIZE);
	SDK_ASSERT( gAllInfo[MENU_PARENTAL][PARENTAL_AGE] );
	
	gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD] = (char*) Alloc (sizeof(char) * OS_TWL_PCTL_PASSWORD_LENGTH+1);
	SDK_ASSERT( gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD] );

	gAllInfo[MENU_OTHER][OTHER_EULA_VERSION] = (char*) Alloc (sizeof(char) * AGE_BUFSIZE);
	SDK_ASSERT( gAllInfo[MENU_OTHER][OTHER_EULA_VERSION] );

	gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID] = (char*) Alloc (sizeof(char) * OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3);
	SDK_ASSERT( gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID] );

	gAllInfo[MENU_OTHER][OTHER_SERIAL] = (char*) Alloc (sizeof(char) * OS_TWL_HWINFO_SERIALNO_LEN_MAX);
	SDK_ASSERT( gAllInfo[MENU_OTHER][OTHER_SERIAL] );
	
	OS_TPrintf( "information alloc succeeded\n" );
}
	

void getAllInfo( void )
// 
{
	BOOL SCFGAccessable = FALSE;
	BOOL fuseRomAccessable = FALSE;
	u8 uniqIDBuf[OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN];
	
	
	OS_TPrintf("reflesh Information\n");
		
	OS_TPrintf( "...Owner Information\n");
	// �{�̐ݒ�܂��
	OS_GetOwnerInfoEx( &gOwnerInfo );
	
	// �S�̏��ɂ߂Ă���
	// �D�L����������Ȃ̂ł��Ƃōl����
	gAllInfo[MENU_OWNER][OWNER_LANGUAGE] = s_strLanguage[ gOwnerInfo.language ];
	gAllInfo[MENU_OWNER][OWNER_COLOR] = s_strUserColor[ gOwnerInfo.favoriteColor ];
	snprintf( gAllInfo[MENU_OWNER][OWNER_BIRTHDAY], BIRTHDAY_BUFSIZE, "%02d/%02d",  gOwnerInfo.birthday.month, gOwnerInfo.birthday.day);
			OS_TPrintf( "language:%s\n", gAllInfo[MENU_OWNER][OWNER_LANGUAGE]);
	gAllInfo[MENU_OWNER][OWNER_COUNTRY] = s_strCountry[gOwnerInfo.country];
	wcsncpy( gUserName, gOwnerInfo.nickName, OS_OWNERINFO_NICKNAME_MAX + 1);
	wcsncpy( gUserComment, gOwnerInfo.comment, OS_OWNERINFO_COMMENT_MAX + 1 ); // !!! �Ȃ����R�����g���擾�ł��Ȃ� �v����
//	wcsncpy( gUserComment, L"���ׂ�ׂ�[", OS_OWNERINFO_COMMENT_MAX + 1 );

	OS_TPrintf( "...Parental Control Information\n" );
	// �y�A�����^���R���g���[���܂��
	gpPC = OS_GetParentalControlInfoPtr();
	
	gAllInfo[MENU_PARENTAL][PARENTAL_FLAG] = s_strBool[ gpPC->flags.isSetParentalControl ];
	gAllInfo[MENU_PARENTAL][PARENTAL_ORGANIZATION] = s_strRatingOrg[ gpPC->ogn ];
	snprintf( gAllInfo[MENU_PARENTAL][PARENTAL_AGE], AGE_BUFSIZE, "%d", gpPC->ratingAge );
	snprintf( gAllInfo[MENU_PARENTAL][PARENTAL_QUESTION_ID], ID_BUFSIZE, "%d", gpPC->secretQuestionID );
	wcsncpy( gUserComment, gpPC->secretAnswer, OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1);
	snprintf( gAllInfo[MENU_PARENTAL][PARENTAL_PASSWORD], OS_TWL_PCTL_PASSWORD_LENGTH + 1, "%d", gpPC->password );
	
	OS_TPrintf( "...Other Machine Setting Information\n" );
	// ���̂ق��̖{�̏��܂��	
	//gbBatteryExtensionAvailable = OS_IsAvailableBatteryExtension();
	gAllInfo[MENU_OTHER][OTHER_WIRELESS] = s_strEnable[ OS_IsAvailableWireless() ];
	gAllInfo[MENU_OTHER][OTHER_FORCE_DISABLE] = s_strBool[ OS_IsForceDisableWireless() ];
	gAllInfo[MENU_OTHER][OTHER_AGREE_EULA] = s_strBool[ OS_IsAgreeEULA() ];
	snprintf( gAllInfo[MENU_OTHER][OTHER_EULA_VERSION], AGE_BUFSIZE, "%d", OS_GetAgreedEULAVersion()) ;
	gAllInfo[MENU_OTHER][OTHER_REGION] = s_strRegion[ OS_GetRegion() ];
	
	{
		int i;
		char ascii[] = "0123456789abcdef";
		const u8 *unq = OS_GetMovableUniqueIDPtr();
		// 16�i��1�o�C�g���l�߂Ă���
		// �o�b�t�@��������3�{���Ȃ̂́A�f�[�^��"%02x-%02x-%02x..."�ɒu�����邽��
		for(i=0; i < OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3; i += 3, unq++ )
		{	
			gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID][i] = ascii[(*unq>>4) & 0x0f];
			gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID][i+1] = ascii[*unq & 0x0f];
			gAllInfo[MENU_OTHER][OTHER_UNIQUE_ID][i+2] = 
				(i+2) == (OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*3 - 1) ? '\0' : '-' ;
			OS_TPrintf("uniqid: %d\n", *unq);
		}
	}
	{
		u8 serialBuf[OS_TWL_HWINFO_SERIALNO_LEN_MAX];
		OS_GetSerialNo( serialBuf );
		snprintf(  gAllInfo[MENU_OTHER][OTHER_SERIAL], OS_TWL_HWINFO_SERIALNO_LEN_MAX, "%s", serialBuf);
	}

	// �w�b�_�����`�F�b�N
	
	// Secure�A�v���Ȃ�fuseROM������
	// Secure�A�v���������I�Ɏw�肵���A�v���Ȃ�SCFG���W�X�^������
	
	/*
	if( SCFGAccessable && SCFG_IsSecureRomAccessible())
	{
		
	}
	
	if( fuseRomAccessable )
	{
		
	}*/
	
	printAllInfo();
	
	OS_TPrintf("reflesh information finished\n");
}

void control( u8 *menu, u8 *page, u8 *line )
{
	u8 pagemax = ( s_numMenuK[*menu] / NUM_LINES ) + 1;	// �I�𒆃��j���[�̃y�[�W��
	u8 linemax = ( s_numMenuK[*menu] - (*page * NUM_LINES)) > NUM_LINES ?
					NUM_LINES : s_numMenuK[*menu] - (*page * NUM_LINES); // �I�𒆃y�[�W�̍��ڐ�
								
	if( pad.trg & PAD_KEY_UP )
	{
		// ���ږ��̂Ȃ��Ƃ���̓J�[�\����΂�
		do
		{
			*line = (*line + linemax - 1) % linemax;
		}
		while( !wcscmp( s_strMetaMenu[*menu][*line], L"") );

	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		do
		{
			*line = (*line+1) % linemax;
		}
		while( !wcscmp( s_strMetaMenu[*menu][*line], L"") );
	}

	if( pad.trg & PAD_KEY_LEFT )
	{
		// �y�[�W����
		*page = (*page + pagemax - 1) % pagemax;
		*line = 0;
	}
	else if( pad.trg & PAD_KEY_RIGHT )
	{
		*page = (*page+1) % pagemax;
		*line = 0;
	}
	
	if( pad.trg & PAD_BUTTON_A )
	{
		if(*menu == MENU_ROOT)
		{
			// ���̃��j���[��ʂ��J��
			*menu = *line;
			*line = 0;
			*page = 0;
		}
		else
		{
			// !!! �ݒ�\�ȍ��ڂ�������ݒ�ύX���
		}
		
	}
	
	if( pad.trg & PAD_BUTTON_B )
	{
		// !!! �Ƃ肠�������̓��[�g�ɖ߂�
		// �l�ݒ��ʂ̎��̓L�����Z�����邾���ɂ���
		*menu = MENU_ROOT;
		*page = 0;
		*line = 0;
	}
}

void displayInfoInit( void )
{
	int loop1,loop2;
	
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"RomTypeTest");
	PutStringUTF16( 4 * 8, 8 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push A To Start Test.");
	PutStringUTF16( 4 * 8, 10 * 8, TXT_COLOR_BLACK, (const u16 *)L"Push X To Start Test Quietly.");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );

	// �S�̏������z����Z�b�g
	// �S���ڂɕ�����o�b�t�@����������ƃ������������݂������Ȃ�̂�
	// �\�Ȃ��̂�static������ւ̃|�C���^�őΉ�
	// �K�v�Ȃ��̂������Ƃ�malloc����
	for(loop1=0; loop1<ROOTMENU_KSIZE; loop1++)
	{
		int loop2;
		gAllInfo[loop1] = (char**)Alloc(sizeof(char*) * s_numMenuV[loop1]);
				
		if(gAllInfo[loop1] == NULL ){
			OS_TPrintf( "AllInfo malloc failed\n");
			return;
		}
		
		// �Ƃ肠�����S��N/A�Ŗ��߂Ă���
		for(loop2=0; loop2<s_numMenuV[loop1]; loop2++)
		{
			gAllInfo[loop1][loop2] = s_strNA;
		}
	}

	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}

void printAllInfo ( void )
{
	int loop1, loop2;
	
	for(loop1=0; loop1 < ROOTMENU_KSIZE; loop1++ )
	{
		for(loop2=0; loop2 < s_numMenuV[loop1]; loop2++ )
		{
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
		}
	}
}

