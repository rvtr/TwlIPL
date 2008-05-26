/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     HWInfoWriterLib.c

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
#include <sysmenu/acsign.h>
#include <sysmenu/namut.h>
#include "TWLHWInfo_api.h"
#include "TWLSettings_api.h"
#include "hwi.h"

// define data------------------------------------------
#ifdef FIRM_USE_PRODUCT_KEYS                                                // ���I���X�C�b�`
#define HWINFO_PRIVKEY_PATH     "rom:key/private_HWInfo.der"                // ���i�p�閧��
#else
#define HWINFO_PRIVKEY_PATH     "rom:key/private_HWInfo_dev.der"            // �J���p�閧��
#endif

// extern data------------------------------------------
const LCFGTWLHWNormalInfo *LCFG_THW_GetDefaultNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetDefaultSecureInfo( void );
const LCFGTWLHWNormalInfo *LCFG_THW_GetNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetSecureInfo( void );

// function's prototype declaration---------------------
static HwiInitResult ReadPrivateKey( void );
static void VerifyHWInfo( void );
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len );
static BOOL ReadHWInfoFile( void );
static BOOL ReadTWLSettings( void );
static u8 HWIi_CalcCheckCode(u8 src[8]);

// global variable -------------------------------------

// static variable -------------------------------------
static u8 *s_pPrivKeyBuffer = NULL;
static void *(*spAlloc)( u32 length );
static void  (*spFree)( void *ptr );

// const data  -----------------------------------------

static const u32 s_langBitmapList[ OS_TWL_REGION_MAX ] = {
    LCFG_TWL_LANG_BITMAP_JAPAN,
    LCFG_TWL_LANG_BITMAP_AMERICA,
    LCFG_TWL_LANG_BITMAP_EUROPE,
    LCFG_TWL_LANG_BITMAP_AUSTRALIA,
    LCFG_TWL_LANG_BITMAP_CHINA,
    LCFG_TWL_LANG_BITMAP_KOREA,
};

static char *strLanguage[] = {
    (char *)"LANG_JAPANESE",
    (char *)"LANG_ENGLISH",
    (char *)"LANG_FRENCH",
    (char *)"LANG_GERMAN",
    (char *)"LANG_ITALIAN",
    (char *)"LANG_SPANISH",
    (char *)"LANG_CHINESE",
    (char *)"LANG_KOREAN",
};

static const char *strLauncherGameCode[] = {
    "HNAJ",
    "HNAE",
    "HNAP",
    "HNAU",
    "HNAC",
    "HNAK",
};

static const char regionAsciiForSerialNo[] = {
	'J',	// ���{
	'W',	// �č�
	'E',	// ���B
	'A',	// ���B
	'C',	// ����
	'K'		// �؍�
};

//======================================================
// HW��񃉃C�^�[
//======================================================

/*---------------------------------------------------------------------------*
  Name:         HWI_Init

  Description:  HW��񃉃C�^�[�̏�����

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/
HwiInitResult HWI_Init( void *(*pAlloc)( u32 ), void (*pFree)( void * ) )
{
    HwiInitResult result;

    spAlloc = pAlloc;
    spFree  = pFree;

    ACSign_SetAllocFunc( pAlloc, pFree );
    result = ReadPrivateKey();
    ReadHWInfoFile();
//  VerifyHWInfo();
    if( 1 ) {
        u8 sign[ RSA_KEY_LENGTH ];
        if( !LCFG_ReadHWID_Signature( sign ) ||
            !LCFG_CheckHWID_Signature( sign )
            ) {
            OS_TPrintf( "HWID Signature check failed.\n" );
        }else {
            OS_TPrintf( "HWID Signature check succeeded.\n" );
        }
    }
    // ��LanguageBitmap�𔻒�Ŏg�p����̂ŁA�K��ReadHWInfo�̌�Ŏ��s����K�v������B
    ReadTWLSettings();

    return result;
}


// TWL�ݒ�f�[�^�̃��[�h
static BOOL ReadTWLSettings( void )
{
    u8 *pBuffer = spAlloc( LCFG_READ_TEMP );
    BOOL result;
    if( pBuffer ) {
        result = LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ] )pBuffer );
		// Read�Ɏ��s�����ꍇ LCFG_ReadTWLSettings �����Ńt�@�C�������J�o����������邪
		// �Ԃ�l�� FALSE �ƂȂ邽�߂�����x���[�h�����݂�
		if (!result)
		{
			OS_TPrintf( "TSD read failed. Retry onece more.\n" );
	        result = LCFG_ReadTWLSettings( (u8 (*)[ LCFG_READ_TEMP ] )pBuffer );
		}
        spFree( pBuffer );
    }
    if( result ) {
        OS_TPrintf( "TSD read succeeded.\n" );
    }else {
        OS_TPrintf( "TSD read failed.\n" );
    }

	return result;
}

// �閧���̃��[�h
HwiInitResult ReadPrivateKey( void )
{
    BOOL result = FALSE;
    u32 keyLength;
    FSFile file;
    OSTick start = OS_GetTick();

    FS_InitFile( &file );
    if( !FS_OpenFileEx( &file, HWINFO_PRIVKEY_PATH, FS_FILEMODE_R ) )
    {
        OS_TPrintf( "PrivateKey read failed.\n" );
    }
    else
    {
        keyLength = FS_GetFileLength( &file );
        if( keyLength > 0 ) {
            s_pPrivKeyBuffer = spAlloc( keyLength );
            if( FS_ReadFile( &file, s_pPrivKeyBuffer, (s32)keyLength ) == keyLength ) {
                OS_TPrintf( "PrivateKey read succeeded.\n" );
                result = TRUE;
            }else {
                OS_TPrintf( "PrivateKey read failed.\n" );
            }
        }
        FS_CloseFile( &file );
    }

    if( !result && s_pPrivKeyBuffer ) {
        spFree( s_pPrivKeyBuffer );
        s_pPrivKeyBuffer = NULL;
    }
//  OS_TPrintf( "PrivKey read time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

    if (result) {
#ifdef FIRM_USE_PRODUCT_KEYS
        return HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE;
#else
        return HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE;
#endif
    }else {
        return HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE;
    }
}

// HW���S�̂̃��[�h
static BOOL ReadHWInfoFile( void )
{
    LCFGReadResult retval;
	BOOL result = TRUE;

    retval = LCFGi_THW_ReadSecureInfo();
    if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        OS_TPrintf( "HW Secure Info read succeeded.\n" );
    }else {
		result = FALSE;
        OS_TPrintf( "HW Secure Info read failed.\n" );
    }

    retval = LCFGi_THW_ReadNormalInfo();
    if( retval == LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        OS_TPrintf( "HW Normal Info read succeeded.\n" );
    }else {
		result = FALSE;
        OS_TPrintf( "HW Normal Info read failed.\n" );
    }

	return result;
}

// HWInfo�t�@�C���̃x���t�@�C
static void VerifyHWInfo( void )
{
    if( VerifyData( (const u8 *)LCFG_THW_GetSecureInfo(), (const u8 *)LCFG_THW_GetDefaultSecureInfo(), sizeof(LCFGTWLHWSecureInfo) ) ) {
        OS_TPrintf( "HW secure Info verify succeeded.\n" );
    }else {
        OS_TPrintf( "HW secure Info verify failed.\n" );
    }
    if( VerifyData( (const u8 *)LCFG_THW_GetNormalInfo(), (const u8 *)LCFG_THW_GetDefaultNormalInfo(), sizeof(LCFGTWLHWNormalInfo) ) ) {
        OS_TPrintf( "HW normal Info verify succeeded.\n" );
    }else {
        OS_TPrintf( "HW normal Info verify failed.\n" );
    }
}

// ��������̃f�[�^�x���t�@�C
static BOOL VerifyData( const u8 *pTgt, const u8 *pOrg, u32 len )
{
    while( len-- ) {
        if( *pTgt++ != *pOrg++ ) {
            return FALSE;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_ModifyLanguage

  Description:  ����R�[�h�����[�W�����l�ɍ��킹�ďC������B

  Arguments:

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_ModifyLanguage( u8 region )
{
#pragma unused( region )
    u32 langBitmap = LCFG_THW_GetValidLanguageBitmap();
    u8  nowLanguage = LCFG_TSD_GetLanguage();
	u8  installedSoftBoxCount;
	u8  freeSoftBoxCount;
	BOOL result = TRUE;

	if (!ReadTWLSettings())
	{
		result = FALSE;
        OS_TPrintf( "Read TWLSettings failed.\n" );
	}

    if( langBitmap & ( 0x0001 << nowLanguage ) ) {
        OS_TPrintf( "Language no change.\n" );
    }else {
        int i;
        for( i = 0; i < LCFG_TWL_LANG_CODE_MAX; i++ ) {
            if( langBitmap & ( 0x0001 << i ) ) {
                break;
            }
        }
        LCFG_TSD_SetLanguage( (LCFGTWLLangCode)i );
        OS_TPrintf( "Language Change \"%s\" -> \"%s\"\n",
                    strLanguage[ nowLanguage ], strLanguage[ LCFG_TSD_GetLanguage() ] );
    }

    // ���R�[�h���N���A���Ă����B
    LCFG_TSD_SetCountry( LCFG_TWL_COUNTRY_UNDEFINED );

    // �y�A�����^���R���g���[�������N���A���Ă���
    MI_CpuClearFast( (void *)LCFG_TSD_GetPCTLPtr(), sizeof(LCFGTWLParentalControl) );

	// �\�t�g�{�b�N�X�J�E���g���X�V
	if (!NAMUT_GetSoftBoxCount(&installedSoftBoxCount, &freeSoftBoxCount))
	{
		return FALSE;
	}

	// LCFG���C�u�����̐ÓI�ϐ��ɑ΂���X�V
    LCFG_TSD_SetInstalledSoftBoxCount( installedSoftBoxCount );	
    LCFG_TSD_SetFreeSoftBoxCount( freeSoftBoxCount );
	
    // region���ς�����ꍇ�́ALANGUAGE_BITMAP���K���ς��̂ŁA�����NTR���ɔ��f�����邽�߂ɕK��TWL�ݒ�f�[�^�̏������݂��s���B
    {
        u8 *pBuffer = spAlloc( LCFG_WRITE_TEMP );
        if( pBuffer ) {
            if (!LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer ))
			{
				result = FALSE;
				OS_TPrintf("Fail! LCFG_WriteTWLSettings()\n");
			}
            spFree( pBuffer );
        }
    }

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWNormalInfoFile

  Description:  HW�m�[�}��Info�t�@�C���̃��C�g

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/

const u8 s_serialNo_mask[] = {
	0x82, 0xb7, 0x0e, 0xf3, 0xad, 0x0a, 0x35, 0x85,
	0xaa, 0x84, 0xae, 0x14, 0xe7, 0x06, 0xd5, 0x00,
};

BOOL HWI_WriteHWNormalInfoFile( void )
{
    LCFGTWLHWNormalInfo Info;
    LCFGReadResult result;

    result = LCFGi_THW_ReadNormalInfo();
    if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        if( !LCFGi_THW_RecoveryNormalInfo( result ) ) {
            OS_TPrintf( "HW Normal Info Recovery failed.\n" );
            return FALSE;
        }
    }

    Info.rtcAdjust = LCFG_THW_GetRTCAdjust();
	{
		int i;
		u8 serialNo[ LCFG_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN ];
		
		MI_CpuClear8( serialNo, LCFG_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN );
		LCFG_THW_GetSerialNo( serialNo );
		
		for( i = 0; i < LCFG_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN; i++ ) {
			Info.movableUniqueID[ i ] = (u8)( serialNo[ i ] ^ s_serialNo_mask[ i ] );
		}
	}

    if (!LCFGi_THW_WriteNormalInfoDirect( &Info ))
    {
        OS_TPrintf( "HW Normal Info Write failed.\n" );
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWSecureInfoFile

  Description:  HW�Z�L���AInfo�t�@�C���̃��C�g

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL HWI_WriteHWSecureInfoFile( u8 region, const u8 *pSerialNo, BOOL isDisableWireless )
{
    BOOL isWrite = TRUE;
    LCFGReadResult result;
	u8 old_region;

    // �t�@�C���̃��[�h
    result = LCFGi_THW_ReadSecureInfo();

    // ���[�h�Ɏ��s�����烊�J�o��
    if( result != LCFG_TSF_READ_RESULT_SUCCEEDED ) {
        if( !LCFGi_THW_RecoverySecureInfo( result ) ) {
            OS_TPrintf( "HW Secure Info Recovery failed.\n" );
            isWrite = FALSE;
        }
    }

    LCFG_THW_SetFlagForceDisableWireless( isDisableWireless );

	// �����[�W������ۑ�
	old_region = LCFG_THW_GetRegion();

    // ���[�W�����̃Z�b�g
    LCFG_THW_SetRegion( region );

    // �Ή�����r�b�g�}�b�v�̃Z�b�g
    LCFG_THW_SetValidLanguageBitmap( s_langBitmapList[ region ] );

    // �V���A��No.�̃Z�b�g
    if( pSerialNo == NULL ) {
        // �ʎY�H���łȂ��ƃV���A��No.�͗p�ӂł��Ȃ��̂ŁA�����ł�MAC�A�h���X�����ƂɓK���Ȓl���Z�b�g����B
        u8 buffer[ 12 ] = "SERIAL";     // �K���ȕ������MAC�A�h���X�ƌ�������SHA1�����A��SerialNo�Ƃ���B
        u8 serialNoOld[ SVC_SHA1_DIGEST_SIZE ];
		u8 serialNoNew[ SVC_SHA1_DIGEST_SIZE ];
		u8 sha1_buffer[ SVC_SHA1_DIGEST_SIZE ];
        int i;
        int len;
		int offset;

		if ( region == OS_TWL_REGION_AMERICA ) 
		{
			len    = LCFG_TWL_HWINFO_SERIALNO_LEN_AMERICA;
			offset = LCFG_TWL_HWINFO_SERIALNO_UNIQUE_OFFSET_AMERICA;
		}
		else
		{
            len    = LCFG_TWL_HWINFO_SERIALNO_LEN_OTHERS;
			offset = LCFG_TWL_HWINFO_SERIALNO_UNIQUE_OFFSET_OTHERS;
		}

		// ���݂̃V���A��No�擾
		LCFG_THW_GetSerialNo( serialNoOld );

		// �V�����V���A��No���N���A���Ă���
		MI_CpuClear8( serialNoNew, sizeof(serialNoNew) );

		// �V���A��No�̐擪��'T'�łȂ���Εs���Ȃ̂ŉ��̃V���A��No.���쐬����
		if ( serialNoOld[0] != 'T')
		{
			// 1�o�C�g�ڂ�TWL��'T'
			serialNoNew[0] = 'T';
			// 2�o�C�g�ڂ̓��[�W������ASCII
			serialNoNew[1] = (u8)regionAsciiForSerialNo[region];			
			// �č����[�W�����ȊO��3�o�C�g�ڂ�EMS�i���V���A��No.�Ȃ̂ŔC�V����'N'�j
			if ( region != OS_TWL_REGION_AMERICA )
			{
				serialNoNew[2] = 'N';
			}

			// ����8��
	        OS_GetMacAddress( buffer + 6 );
	        SVC_CalcSHA1( sha1_buffer, buffer, sizeof(buffer) );
	        for( i = offset; i < len-1; i++ ) {
	            serialNoNew[ i ] = (u8)( ( sha1_buffer[ i ] % 10 ) + 0x30 );
	        }

			// �`�F�b�N�R�[�h�擾
			serialNoNew[len-1] = HWIi_CalcCheckCode(&serialNoNew[offset]);

			// ���V���A��No.�ł��邱�Ƃ̈�Ƃ���14�o�C�g�ڂ�'K'�Ƃ���
			serialNoNew[13] = 'K';
		}
		// �V���A��No�̐擪��'T'�ł���ꍇ���j�[�N�����W���͂��̂܂܂ő���ύX����
		else
		{
			// 1�o�C�g�ڂ�TWL��'T'
			serialNoNew[0] = 'T';
			// 2�o�C�g�ڂ̓��[�W������ASCII
			serialNoNew[1] = (u8)regionAsciiForSerialNo[region];
			// �č����[�W�����ȊO��3�o�C�g�ڂ�EMS
			if ( region != OS_TWL_REGION_AMERICA )
			{
				// EMS��񂪑��݂���Ȃ�����p��
				if ( old_region != OS_TWL_REGION_AMERICA )
				{
					serialNoNew[2] = serialNoOld[2];
				}
				// EMS��񂪑��݂��Ȃ����߉��ɔC�V����'N'�Ƃ���
				else
				{
					serialNoNew[2] = 'N';
				}
			}

			// ���j�[�N�������R�s�[
			if ( old_region == OS_TWL_REGION_AMERICA )
			{
				MI_CpuCopy( &serialNoOld[LCFG_TWL_HWINFO_SERIALNO_UNIQUE_OFFSET_AMERICA], &serialNoNew[offset], LCFG_TWL_HWINFO_SERIALNO_UNIQUE_LENGTH );
			}
			else
			{
				MI_CpuCopy( &serialNoOld[LCFG_TWL_HWINFO_SERIALNO_UNIQUE_OFFSET_OTHERS], &serialNoNew[offset], LCFG_TWL_HWINFO_SERIALNO_UNIQUE_LENGTH );
			}

			// �`�F�b�N�R�[�h�擾
			serialNoNew[len-1] = HWIi_CalcCheckCode(&serialNoNew[offset]);

			// 14�o�C�g�ڂ͋�����V�փR�s�[
			serialNoNew[13] = serialNoOld[13];
		}

     	OS_TPrintf( "serialNo : %s\n", serialNoNew );

        LCFG_THW_SetSerialNo( serialNoNew );
    }else {
        LCFG_THW_SetSerialNo( pSerialNo );
    }

    // �����`���[TitleID_Lo�̃Z�b�g
    {
        int i;
        u8 titleID_Lo[4];
        for( i = 0; i < 4; i++ ) titleID_Lo[ i ] = (u8)strLauncherGameCode[ region ][ 4 - i - 1 ];
        LCFG_THW_SetLauncherTitleID_Lo( (const u8 *)titleID_Lo );
    }

    // ���C�g
    if( isWrite &&
        !LCFGi_THW_WriteSecureInfo( s_pPrivKeyBuffer ) ) {
        isWrite = FALSE;
        OS_TPrintf( "HW Secure Info Write failed.\n" );
    }

    return isWrite;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_WriteHWIDSignFile

  Description:  HWID�����t�@�C���̃��C�g

  Arguments:

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL HWI_WriteHWIDSignFile( void )
{
    BOOL retval;

    (void)FS_DeleteFile( (char *)LCFG_TWL_HWID_SIGN_PATH );
    if( !FS_CreateFile( LCFG_TWL_HWID_SIGN_PATH, FS_PERMIT_R | FS_PERMIT_W ) ) {
        OS_TPrintf( "file create error. %s\n", LCFG_TWL_HWID_SIGN_PATH );
    }
    retval = LCFG_WriteHWID_Signature( s_pPrivKeyBuffer );
    if( !retval ) {
        OS_TPrintf( "HWID Signature Write failed.\n" );
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         HWI_DeleteHWNormalInfoFile

  Description:  HW�m�[�}��Info�t�@�C���̏���

  Arguments:    None

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_DeleteHWNormalInfoFile( void )
{
    if (FS_DeleteFile( (char *)LCFG_TWL_HWINFO_NORMAL_PATH ))
    {
        OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
        return TRUE;
    }
    else
    {
        OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_NORMAL_PATH );
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         HWI_DeleteHWSecureInfoFile

  Description:  HW�Z�L���AInfo�t�@�C���̏���

  Arguments:    None

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_DeleteHWSecureInfoFile( void )
{
    if (FS_DeleteFile( (char *)LCFG_TWL_HWINFO_SECURE_PATH ))
    {
        OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
        return TRUE;
    }
    else
    {
        OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWINFO_SECURE_PATH );
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         HWI_DeleteHWIDSignFile

  Description:  HWID�����t�@�C���̏���

  Arguments:    None

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
BOOL HWI_DeleteHWIDSignFile( void )
{
    if (FS_DeleteFile( (char *)LCFG_TWL_HWID_SIGN_PATH ))
    {
        OS_TPrintf( "%s delete succeeded.\n", (char *)LCFG_TWL_HWID_SIGN_PATH );
        return TRUE;
    }
    else
    {
        OS_TPrintf( "%s delete failed.\n", (char *)LCFG_TWL_HWID_SIGN_PATH );
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         HWIi_CalcCheckCode

  Description:  �V���A���ԍ��̐����W���̃`�F�b�N�R�[�h���v�Z���܂�

  Arguments:    Ascii ����������

  Returns:      Ascii ����
 *---------------------------------------------------------------------------*/
static u8 HWIi_CalcCheckCode(u8 src[8])
{
	// ���W�����X10 �E�F�C�g3�1�iM10W31�j
	int temp = (src[7] - '0')*3 +
               (src[6] - '0')*1 +
               (src[5] - '0')*3 +
               (src[4] - '0')*1 +
               (src[3] - '0')*3 +
               (src[2] - '0')*1 +
               (src[1] - '0')*3 +
               (src[0] - '0')*1;
	temp %= 10;
	temp = 10-temp;

	return (u8)('0' + temp);
}

