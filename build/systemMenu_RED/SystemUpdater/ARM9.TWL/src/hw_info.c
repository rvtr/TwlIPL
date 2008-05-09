/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SystemUpdater
  File:     hw_info.c

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
#include "hwi.h"
#include "hw_info.h"
//
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
//
/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

BOOL WriteHWNormalInfoFile( void );
BOOL WriteHWSecureInfoFile( u8 region );
BOOL DeleteHWInfoFile( void );

/*---------------------------------------------------------------------------*
  Name:         HW���S�̂̃��C�g

  Description:  

  Arguments:    region : 

  Returns:      None.
 *---------------------------------------------------------------------------*/

BOOL WriteHWInfoFile( u8 region, BOOL wirelessForceOff )
{
	static const char *pMsgNormalWriting  	= "Writing Normal File...";
	static const char *pMsgSecureWriting  	= "Writing Secure File...";
	static const char *pMsgSignWriting  	= "Writing Sign   File...";
	static const char *pMsgSucceeded 		= "Success!\n";
	static const char *pMsgFailed 			= "Failed!\n";
	u32 installedSoftBoxCount = 0;
	BOOL result = TRUE;

	// �m�[�}���t�@�C���̃��C�g
	if( HWI_WriteHWNormalInfoFile() ) {
//		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}

	// �Z�L���A�t�@�C���̃��C�g
	if( HWI_WriteHWSecureInfoFile( region, NULL, wirelessForceOff ) ) {	// �Ƃ肠���������͗L���ŁB
//		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
	}else {
		kamiFontPrintfConsoleEx(CONSOLE_RED, pMsgFailed );
		result = FALSE;
	}
	
	// HWID�����t�@�C���̃��C�g
	if( HWI_WriteHWIDSignFile() ) {
//		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, pMsgSucceeded );
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

	// InstalledSoftBoxCount, FreeSoftBoxCount �̒l�����݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B
	UpdateNandBoxCount();
	
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         HWInfo�t�@�C���̍폜

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

BOOL DeleteHWInfoFile( void )
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
  Name:         UpdateNandBoxCount

  Description:  InstalledSoftBoxCount, FreeSoftBoxCount �̒l��
				���݂�NAND�̏�Ԃɍ��킹�čX�V���܂��B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void UpdateNandBoxCount( void )
{
	u32 installedSoftBoxCount;
	u32 freeSoftBoxCount;

	// InstalledSoftBoxCount, FreeSoftBoxCount �𐔂��Ȃ���
	installedSoftBoxCount = NAMUT_SearchInstalledSoftBoxCount();
	freeSoftBoxCount = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - installedSoftBoxCount;

//	OS_Printf("installedSoftBoxCount = %d\n", installedSoftBoxCount);
//	OS_Printf("freeSoftBoxCount      = %d\n", freeSoftBoxCount);

	// LCFG���C�u�����̐ÓI�ϐ��ɑ΂���X�V
    LCFG_TSD_SetInstalledSoftBoxCount( (u8)installedSoftBoxCount );	
    LCFG_TSD_SetFreeSoftBoxCount( (u8)freeSoftBoxCount );

	// LCFG���C�u�����̐ÓI�ϐ��̒l��NAND�ɔ��f
    {
        u8 *pBuffer = OS_Alloc( LCFG_WRITE_TEMP );
        if( pBuffer ) {
            (void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
            OS_Free( pBuffer );
        }
    }
}

