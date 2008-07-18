/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     control.c

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

#include <twl.h>
#include <twl/lcfg.h>
#include "misc.h"
#include "drawFunc.h"
#include "control.h"
#include "strResource.h"
#include "viewSystemInfo.h"

#define SAVE_COUNT_MASK                 0x7f        // saveCount�̒l�͈̔͂��}�X�N����B(0x00-0x7f�j


// TSF�w�b�_
typedef struct TSFHeader{
    union digest {
        u8              sha1[ SVC_SHA1_DIGEST_SIZE ];   // SHA-1�_�C�W�F�X�g
        u8              rsa[ RSA_KEY_LENGTH ];          // RSA����
        u8              dst[ RSA_KEY_LENGTH ];          // �]���p�̍ő�T�C�Y�v�f
    }digest;
    u8                  version;                        // �f�[�^ver.
    u8                  saveCount;                      // �Z�[�u�J�E���g�i�~���[�����O���Ȃ��t�@�C���͎g�p���Ȃ��j
    u8                  rsv[2];                         // �\��
    u32                 bodyLength;                     // �f�[�^��
}TSFHeader; // 134bytes

static const char *s_TSDPath[] = {
    (const char *)"nand:/shared1/TWLCFG0.dat",
    (const char *)"nand:/shared1/TWLCFG1.dat",
};


static int selectLine[ROOTMENU_SIZE+1];
void resetUserData( int idx );
void breakUserData( int idx );
static void TSDi_ClearSettingsDirect( LCFGTWLSettingsData *pTSD );
static BOOL LCFGi_TSD_WriteSettingsDirectForRecovery( const LCFGTWLSettingsData *pSrcInfo, int index );
BOOL LCFGi_TSF_WriteFile( char *pPath, TSFHeader *pHeader, const void *pSrcBody, u8 *pSaveCount );

ChangeCotnrolResult changeControl( int *menu, int *line, int *changeLine, int *changeMode )
{
	int linemax = gAllInfo[*menu][*line].numKindName;
	BOOL controlFlag = FALSE;

	if( !gAllInfo[*menu][*line].changable )
	{
		*changeMode = FALSE;
		return CHANGE_CONTROL;
	}
		
	// �㉺�ō��ڕύX
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		if( --(*changeLine) < 0 )
		{
			// ���C�����f�N�������g�������ʃ}�C�i�X�ɂȂ������ԍŌ��
			*changeLine = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		if( ++(*changeLine) >= linemax )
		{
			// ���C�����C���N�������g�������ʁAmaxline�𒴂�����ŏ���
			*changeLine = 0;
		}
	}

	if( pad.trg & PAD_BUTTON_A )
	{
		switch( gAllInfo[*menu][*line].argType )
		{
			case ARG_INT:
				gAllInfo[*menu][*line].changeFunc.cInt(*changeLine);
				break;
			
			case ARG_BOOL:
				gAllInfo[*menu][*line].changeFunc.cBool(*changeLine);
				break;
				
			case ARG_OTHER:
				// �_���l�ł�int�ł��n���Ȃ��֐��͎c�O�ȑΉ�������
				if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_RST_DSP )
				{
					*changeLine == 0 ? SCFG_ReleaseResetDSP(): SCFG_ResetDSP();
				}
				else if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_EXT_PS )
				{
					SCFGPsramBoundary idx = SCFG_PSRAM_BOUNDARY_4MB;
					
					switch(*changeLine)
					{
						case 0:
							idx = SCFG_PSRAM_BOUNDARY_4MB;
							break;
						case 1:
							idx = SCFG_PSRAM_BOUNDARY_16MB;
							break;
						case 2:
							idx = SCFG_PSRAM_BOUNDARY_32MB;
							break;
					}
					
					SCFG_SetPsramBoundary( idx );
					
				}
				
				else if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_EXT_CFG )
				{
					if( *changeLine == 0 )
					{
						SCFG_SetConfigBlockInaccessible();
					}
				}
				
				break;
		}
		
		return CHANGE_VALUE_CHANGED;
	}

	// B�ŃL�����Z�����Ė߂�
	if( pad.trg & PAD_BUTTON_B )
	{
		controlFlag = TRUE;
		*changeMode = FALSE;
	}
	
	return controlFlag ? CHANGE_CONTROL : CHANGE_NOTHING ;
}


BOOL control( int *menu, int *line, int *changeLine, int *changeMode )
{
	int linemax = s_numMenu[*menu]; // �I�𒆃y�[�W�̍��ڐ�
	BOOL controlFlag = FALSE;				// �������삪��������TRUE�ɂȂ�

	// �㉺�ō��ڕύX
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		if( --(*line) < 0 )
		{
			// ���C�����f�N�������g�������ʃ}�C�i�X�ɂȂ������ԍŌ��
			*line = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		if( ++(*line) >= linemax )
		{
			// ���C�����C���N�������g�������ʁAmaxline�𒴂�����ŏ���
			*line = 0;
		}
	}

	// ���E�Ńy�[�W����
	if( pad.trg & PAD_KEY_RIGHT )
	{
		controlFlag = TRUE;
		*line += DISP_NUM_LINES - 2;
		
		if( *line >= linemax )
		{
			*line = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_LEFT )
	{
		controlFlag = TRUE;
		*line -= DISP_NUM_LINES - 2;
		
		if( *line < 0 )
		{
			*line = 0;
		}
	}

	// A�{�^��
	if( pad.trg & PAD_BUTTON_A )
	{
		if(*menu == MENU_ROOT)
		{
			controlFlag = TRUE;
			
			switch( *line )
			{
				case MENU_ROOT :
				case MENU_OWNER:
				case MENU_PARENTAL:
				case MENU_OTHER:
				case MENU_NORMAL_HW:
				case MENU_SECURE_HW:
				case MENU_SCFG_ARM7:
				case MENU_SCFG_ARM9:
				case MENU_SYSMENU:
				case MENU_VERSION:
					// ���̉�ʂ̑I���ʒu���L�^
					selectLine[ROOTMENU_SIZE] = *line;

					// ���̃��j���[��ʂ��J��
					*menu = *line;
					*line = selectLine[*menu];
					break;
				
				case MENU_RESET_INFO:
					resetUserData(0);
					resetUserData(1);
					break;
					
				case MENU_BREAK_DATA:
					breakUserData(0);
					breakUserData(1);
					break;
					
			}
		}
		else if( gAllInfo[*menu][*line].changable )
		{
			controlFlag = TRUE;

			// �ύX�\�ȍ��ڂ͕ύX��ʂ��J��
			*changeMode = TRUE;
			*changeLine = gAllInfo[*menu][*line].iValue;
		}
		
	}
	
	if( pad.trg & PAD_BUTTON_B )
	{
		if( *menu != MENU_ROOT )
		{
			controlFlag = TRUE;

			// �ݒ�l�\����ʂ̂Ƃ��̓��[�g�ɖ߂�
			selectLine[*menu] = *line;
			*menu = MENU_ROOT;
			*line = selectLine[ROOTMENU_SIZE];
		}
	}

	if( ( pad.trg & PAD_BUTTON_SELECT ) && *menu == MENU_SCFG_ARM7 )
	{
		controlFlag = TRUE;
		
		// ARM7SCFG�̕\���f�[�^��؂�ւ���
		switchViewMode();
	}
		
	return controlFlag;
}

void resetUserData( int idx )
// idx(0 or 1)�Ԗڂ̃��[�U�f�[�^�����Z�b�g����
{
	u8 *dataBuf = (u8*) Alloc (LCFG_READ_TEMP);
	
	LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ])dataBuf );
	TSDi_ClearSettingsDirect( (LCFGTWLSettingsData *)(&dataBuf[ LCFG_TEMP_BUFFER_SIZE*idx ]) );
	LCFGi_TSD_WriteSettingsDirectForRecovery( (LCFGTWLSettingsData *)&dataBuf[ LCFG_TEMP_BUFFER_SIZE*idx ], idx );
}

void breakUserData( int idx )
{
	// LCFG API���g�킸�ɁAFS���x���Ńt�@�C����ǂ�ŁA�f�[�^��j�󂵂Ă��珑���߂�
	FSFile file;
	u8 *fileBuf = (u8*) Alloc ( LCFG_TEMP_BUFFER_SIZE );
	
	FS_InitFile( &file );

	if( !FS_OpenFileEx( &file, s_TSDPath[idx], FS_FILEMODE_R | FS_FILEMODE_W ) )
	{
		OS_TPrintf("OpenFile failed. result: %d path: %s\n", FS_GetArchiveResultCode(&file), s_TSDPath[idx]);
		return;
	}
/*	
	if( FS_ReadFile( &file, fileBuf, LCFG_TEMP_BUFFER_SIZE ) == -1 )
	{
		OS_TPrintf("readFile failed. path: %s\n", s_TSDPath[idx]);
		return;
	}
*/	
	// �K���Ƀf�[�^����
	MI_CpuFill8( fileBuf, 0xFF, LCFG_TEMP_BUFFER_SIZE );
	
	// �f�[�^�̏����߂�
	FS_SeekFileToBegin( &file );
	
	if( FS_WriteFile( &file, fileBuf, LCFG_TEMP_BUFFER_SIZE ) == -1 )
	{
		OS_TPrintf("writeFile failed. path: %s\n", s_TSDPath[idx]);
		return;
	}
	
	/*
	// �O�̂��ߒ��g���m�F
	MI_CpuClear8( fileBuf, LCFG_TEMP_BUFFER_SIZE );
	FS_SeekFileToBegin( &file );
	if( FS_ReadFile( &file, fileBuf, LCFG_TEMP_BUFFER_SIZE ) == -1 )
	{
		OS_TPrintf("readFile failed. path: %s\n", s_TSDPath[idx]);
		return;
	}
	
	{
		int i;	
		for( i = 0; i < LCFG_TEMP_BUFFER_SIZE; i++ )
		{
			if( i % 16  == 0 )
			{
				OS_TPrintf("\n");
			}
			
			OS_TPrintf("%x ",fileBuf[i] );
		}
	}
	*/
	
	FS_CloseFile( &file );
	
	

	OS_TPrintf("Breaking UserData Succeeded. path: %s\n", s_TSDPath[idx]);
}

// TWL�ݒ�f�[�^�̒��ڃN���A
static void TSDi_ClearSettingsDirect( LCFGTWLSettingsData *pTSD )
{
    int i;
    MI_CpuClearFast( pTSD, sizeof(LCFGTWLSettingsData) );
    // �����l��"0"�ȊO�̂���
    pTSD->owner.userColor = OS_FAVORITE_COLOR_MAGENTA;	// 2008.06.23 UIG��������̗v�]�ɂ��
    pTSD->owner.birthday.month  = 1;
    pTSD->owner.birthday.day    = 1;
	pTSD->flags.isAvailableWireless = 1;
	pTSD->launcherStatus.InstalledSoftBoxCount = 0;
	pTSD->launcherStatus.freeSoftBoxCount      = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX;
	pTSD->agreeEulaVersion[ 0 ] = 1;
    // ����R�[�h��HW���̌���r�b�g�}�b�v����Z�o
    for( i = 0; i < LCFG_TWL_LANG_CODE_MAX; i++ ) {
        if( OS_GetValidLanguageBitmap() & ( 0x0001 << i ) ) {	// ValidLanguageBitmap���́A�����`���[��MMEM�Ƀ��[�h�������̂��g�p
            pTSD->language = (LCFGTWLLangCode)i;
            break;
        }
    }
}




// �w��f�[�^�̒l���t�@�C���ɒ��ڃ��C�g(���J�o���p��s_indexTSD�̕ύX�����C�g��ɍs���j
static BOOL LCFGi_TSD_WriteSettingsDirectForRecovery( const LCFGTWLSettingsData *pSrcInfo, int index )
{
	u8 saveCount = 0;
    // �w�b�_�̍쐬
    TSFHeader header;
    MI_CpuClear8( &header, sizeof(TSFHeader) );
    header.version = LCFG_TWL_SETTINGS_DATA_VERSION;
    header.bodyLength = sizeof(LCFGTWLSettingsData);
    SVC_CalcSHA1( header.digest.sha1, pSrcInfo, sizeof(LCFGTWLSettingsData) );
	
    // �t�@�C���Ƀ��C�g
    if( !LCFGi_TSF_WriteFile( (char *)s_TSDPath[ index ],
                        &header,
                        (const void *)pSrcInfo,
                        &saveCount ) ) {
        return FALSE;
    }

    return TRUE;
}

// TWL�t�@�C���̃��C�g
BOOL LCFGi_TSF_WriteFile( char *pPath, TSFHeader *pHeader, const void *pSrcBody, u8 *pSaveCount )
{
    BOOL retval = FALSE;
    FSFile file;
    FS_InitFile( &file );
    
    if( pSaveCount ) {
        *pSaveCount = (u8)( ( *pSaveCount + 1 ) & SAVE_COUNT_MASK );
        pHeader->saveCount = *pSaveCount;
    }else {
        pHeader->saveCount = 0;
    }
    
    OS_TPrintf( "Write > %s : %d\n", pPath, pHeader->saveCount );
    
    // �t�@�C���I�[�v��
    if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R | FS_FILEMODE_W ) ) {       // R|W���[�h�ŊJ���ƁA�����t�@�C�����c�����܂܍X�V�B
        OS_TPrintf( "Write : file open error. %s\n", pPath );
        return FALSE;
    }
    
    // ���C�g
    if( FS_WriteFile( &file, pHeader, sizeof(TSFHeader) ) < sizeof(TSFHeader) ) {
        OS_TPrintf( "Write : file header write error. %s\n", pPath );
        goto END;
    }
    if( FS_WriteFile( &file, pSrcBody, (long)pHeader->bodyLength ) < pHeader->bodyLength ) {
        OS_TPrintf( "Write : file body write error. %s\n", pPath );
        goto END;
    }
    
    retval = TRUE;
END:
    // �t�@�C���N���[�Y
    (void)FS_CloseFile( &file );
    
    return retval;
}
