/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_hw_info.c

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

#include <twl.h>
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <twl/lcfg.h>
#include <nitro/card.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_hw_info.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "hwi.h"

//
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
//
/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

enum {
	MENU_REGION_JAPAN = 0,
	MENU_REGION_AMERICA,
	MENU_REGION_EUROPE,
	MENU_REGION_AUSTRALIA,
	MENU_REGION_CHINA,
	MENU_REGION_KOREA,
	MENU_RETURN,
	NUM_OF_MENU_SELECT
};

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE    16
#define CHAR_OF_MENU_SPACE    2
#define MENU_TOP_LINE         5
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      40

#define NANDINITIALIZER_SETTING_FILE_PATH_IN_SD  "sdmc:/nandinitializer.ini"

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s8   sMenuSelectNo;
static BOOL sWirelessForceOff;
static BOOL sLogoDemoSkipForce;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL WriteHWNormalInfoFile( void );
static BOOL WriteHWSecureInfoFile( u8 region );
//static BOOL DeleteHWInfoFile( void );

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         HWInfo �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* HWInfoProcess0(void)
{
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Write Hardware Information ");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  5, FONT_COLOR_BLACK, "l   REGION JAPAN     l    l");
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   REGION AMERICA   l    l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l   REGION EUROPE    l    l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l   REGION AUSTRALIA l    l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "l   REGION CHINA     l    l");
	kamiFontPrintf(3, 14, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "l   REGION KOREA     l    l");
	kamiFontPrintf(3, 16, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 17, FONT_COLOR_BLACK, "l   RETURN           l    l");
	kamiFontPrintf(3, 18, FONT_COLOR_BLACK, "+--------------------+----+");

	// ���݂̃��[�W������"now"�ƕ\��
	kamiFontPrintf(26, (s16)(MENU_TOP_LINE+OS_GetRegion()*CHAR_OF_MENU_SPACE), FONT_COLOR_BLACK, "now");

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_PURPLE, BG_COLOR_PURPLE );
	kamiFontFillChar( 1, BG_COLOR_PURPLE, BG_COLOR_PURPLE );
	kamiFontFillChar( 2, BG_COLOR_PURPLE, BG_COLOR_TRANS );

	// �J�[�\�����O
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( HWInfoProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         HWInfo �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* HWInfoProcess1(void)
{
#ifndef NAND_INITIALIZER_LIMITED_MODE
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		return HWInfoProcess2;
	}
#endif

	// �I�����j���[�̕ύX
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = NUM_OF_MENU_SELECT -1;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo >= NUM_OF_MENU_SELECT) sMenuSelectNo = 0;
	}

	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// ����
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return HWInfoProcess2;
	}
	// �g�b�v���j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return HWInfoProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         HWInfo �v���Z�X�Q

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* HWInfoProcess2(void)
{
	int i;
	BOOL result;

#ifndef NAND_INITIALIZER_LIMITED_MODE
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		// SD�J�[�h��nandinitializer.ini���ݒ���擾
		if (!GetNandInitializerSetting((u8 *)&sMenuSelectNo, (u8 *)&sWirelessForceOff, (u8 *)&sLogoDemoSkipForce))
		{
			// �ݒ�̎擾�Ɏ��s�����ꍇ�̓f�t�H���g�ݒ�(REGION_JAPAN/WIRELESS_ENABLE)
			sMenuSelectNo = 0;
			sWirelessForceOff = FALSE;
			sLogoDemoSkipForce = FALSE;
		}
	}
	else
#endif
	{
		sWirelessForceOff = LCFG_THW_IsForceDisableWireless();
	}

	switch( sMenuSelectNo )
	{
	case MENU_REGION_JAPAN:
	case MENU_REGION_AMERICA:
	case MENU_REGION_EUROPE:
	case MENU_REGION_AUSTRALIA:
	case MENU_REGION_CHINA:
	case MENU_REGION_KOREA:

		result = WriteHWInfoFile( (u8)sMenuSelectNo, sWirelessForceOff, sLogoDemoSkipForce );

		// �S���[�W�����̌��ʂ��N���A
		for (i=0;i<NUM_OF_MENU_SELECT;i++)
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+i*CHAR_OF_MENU_SPACE), FONT_COLOR_BLACK, "   ");
		}
		// ����̌��ʂ�\��
		if ( result == TRUE )
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK ");
		}
		else
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, "NG ");
		}
		break;

	case MENU_RETURN:
		FADE_OUT_RETURN( TopmenuProcess0 );
		break;
	}


#ifndef NAND_INITIALIZER_LIMITED_MODE
	// Auto�p
	if (gAutoFlag)
	{
		if (result) 
		{ 
			gAutoProcessResult[AUTO_PROCESS_MENU_HARDWARE_INFO] = AUTO_PROCESS_RESULT_SUCCESS; 
			FADE_OUT_RETURN( AutoProcess1 ); 
		}
		else 
		{
			gAutoProcessResult[AUTO_PROCESS_MENU_HARDWARE_INFO] = AUTO_PROCESS_RESULT_FAILURE;  
			FADE_OUT_RETURN( AutoProcess2 ); 
		}
	}
#endif

	return HWInfoProcess1;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         HW���S�̂̃��C�g

  Description:  

  Arguments:    region : 

  Returns:      None.
 *---------------------------------------------------------------------------*/

BOOL WriteHWInfoFile( u8 region, BOOL wirelessForceOff, BOOL logoDemoSkipForce )
{
	static const char *pMsgSecureWriting  	= "Writing Secure File...";
	static const char *pMsgNormalWriting  	= "Writing Normal File...";
	static const char *pMsgSignWriting  	= "Writing Sign   File...";
	static const char *pMsgSucceeded 		= "Success!\n";
	static const char *pMsgFailed 			= "Failed!\n";
	u32 installedSoftBoxCount = 0;
	BOOL result = TRUE;

	// HWID�����t�@�C���̃��C�g
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSignWriting );
	
	if( HWI_WriteHWIDSignFile() ) {
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}

	// �Z�L���A�t�@�C���̃��C�g
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSecureWriting );
	
	if( HWI_WriteHWSecureInfoFile( region, NULL, wirelessForceOff, logoDemoSkipForce ) ) {	
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// �m�[�}���t�@�C���̃��C�g(�ڍs�\�ȃ��j�[�NID�̂��߂ɃZ�L���A�t�@�C���̌�ŏ������ށj
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgNormalWriting );
	
	if( HWI_WriteHWNormalInfoFile() ) {
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// CFG�f�[�^�̏C��
	if (!HWI_ModifyLanguage( region ))
	{
		kamiFontPrintfConsoleEx(CONSOLE_RED, "Fail! Write TWLSettings\n" );	
		result = FALSE;
	}
	
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         HWInfo�t�@�C���̍폜

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static BOOL DeleteHWInfoFile( void )
{
	static const char *pMsgNormalDeleting  	= "Deleting Normal File...";
	static const char *pMsgSecureDeleting  	= "Deteting Secure File...";
	static const char *pMsgSucceeded 		= "Success!\n";
	static const char *pMsgFailed 			= "Failed!\n";
	BOOL result = TRUE;

	// �m�[�}���t�@�C��
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgNormalDeleting );
	if( HWI_DeleteHWNormalInfoFile() ) {
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// �Z�L���A�t�@�C��
	kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSecureDeleting );
	if( HWI_DeleteHWSecureInfoFile() ) {
		OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         GetNandInitializerSetting

  Description:  SD�J�[�h��nandinitializer.ini�̐ݒ���m�F���܂�

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL GetNandInitializerSetting(u8* region, u8* wireless, u8* logodemoskip)
{
    FSFile  file;	
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	void* pTempBuf;
	char* pStr;
	u8    temp_region;
	u8    temp_wireless;
	u8    temp_logodemoskip;
	u32 file_size;
	u32 alloc_size;

	// ROM�t�@�C���I�[�v��
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, NANDINITIALIZER_SETTING_FILE_PATH_IN_SD);
	if (!open_is_ok)
	{
    	OS_Printf("%s is not exist.\n", NANDINITIALIZER_SETTING_FILE_PATH_IN_SD);
		return FALSE;
	}

	// ROM�t�@�C�����[�h
	file_size  = FS_GetFileLength(&file) ;
	alloc_size = ROUND_UP(file_size, 32) ;
	pTempBuf = OS_Alloc( alloc_size );
	SDK_NULL_ASSERT(pTempBuf);
	DC_InvalidateRange(pTempBuf, alloc_size);
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	if (!read_is_ok)
	{
	    OS_Printf("%s could not be read.\n", NANDINITIALIZER_SETTING_FILE_PATH_IN_SD);
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// ROM�t�@�C���N���[�Y
	FS_CloseFile(&file);
	
	// �������S�f���X�L�b�v�ݒ��ǂݎ��
	pStr = STD_SearchString( pTempBuf, "LOGO_DEMO_SKIP_FORCE:");
	if (pStr != NULL)
	{

		pStr += STD_GetStringLength("LOGO_DEMO_SKIP_FORCE:");
		temp_logodemoskip = (u8)(*pStr - '0');

		if ( temp_logodemoskip == 1 )
		{ 
			*logodemoskip = temp_logodemoskip; 
		}
		else
		{
			*logodemoskip = 0; 
		}
	}else
	{
		*logodemoskip = 0;
	}

	// REGION: ��ǂݎ��
	pStr = STD_SearchString( pTempBuf, "REGION:");
	if (pStr == NULL)
	{
		OS_Free(pTempBuf);
		return FALSE;
	}

	pStr += STD_GetStringLength("REGION:");
	temp_region = (u8)(*pStr - '0');

	if (OS_TWL_REGION_JAPAN <= temp_region && temp_region < OS_TWL_REGION_MAX)
	{
		*region = temp_region;
	}
	else
	{
		OS_Free(pTempBuf);
		return FALSE;		
	}

	// �������C�����XOFF�ݒ��ǂݎ��
	pStr = STD_SearchString( pTempBuf, "WIRELESS_FORCE_OFF:");
	if (pStr == NULL)
	{
		OS_Free(pTempBuf);
		return FALSE;
	}

	pStr += STD_GetStringLength("WIRELESS_FORCE_OFF:");
	temp_wireless = (u8)(*pStr - '0');

	if (0 <= temp_wireless && temp_wireless <= 1)
	{ 
		*wireless = temp_wireless; 
	}
	else
	{
		OS_Free(pTempBuf);
		return FALSE;
	}

	OS_Free(pTempBuf);
	return TRUE;
}
