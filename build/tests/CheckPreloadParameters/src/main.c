/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - CheckPreloadParameters
  File:     main.c

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

void VBlankIntr(void);

OSOwnerInfoEx s_owner;
u8 s_cameraInfo[ OS_TWL_HWINFO_CAMERA_LEN ];
static char *s_strCountry[ 256 ];
static char *s_strRegion[ OS_TWL_REGION_MAX ];
static char *s_strUserColor[ OS_FAVORITE_COLOR_MAX ];
static char *s_strLanguage[ OS_LANGUAGE_CODE_MAX ];


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
	OS_Init();

	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);

	OS_GetOwnerInfoEx( &s_owner );
	OS_TPrintf( "Language  : %02x %s\n", s_owner.language, s_strLanguage[ s_owner.language ] );
	OS_TPrintf( "UserColor : %02x %s\n", s_owner.favoriteColor, s_strUserColor[ s_owner.favoriteColor ] );
	OS_TPrintf( "Birthday  : %02d/%02d\n", s_owner.birthday.month, s_owner.birthday.day );
	{
		char string[ 256 ];
		int srcLen, dstLen;
		MI_CpuClear8( string, sizeof(string) );
		srcLen = (int)s_owner.nickNameLength;
		dstLen = sizeof(string);
		STD_ConvertStringUnicodeToSjis( string, &dstLen, s_owner.nickName, &srcLen, NULL );
		OS_TPrintf( "Nickname : %s\n", string );
		
		MI_CpuClear8( string, sizeof(string) );
		srcLen = (int)s_owner.commentLength;
		dstLen = sizeof(string);
		STD_ConvertStringUnicodeToSjis( string, &dstLen, s_owner.comment, &srcLen, NULL );
		OS_TPrintf( "Comment  : %s\n", string );
	}
	OS_TPrintf( "Country  : %s\n", s_strCountry[ s_owner.country ] );
	
	OS_TPrintf( "AvailableWireless : %s\n", OS_IsAvailableWireless() ? "TRUE" : "FALSE" );
	PMi_SetWirelessLED( OS_IsAvailableWireless() ? PM_WIRELESS_LED_ON : PM_WIRELESS_LED_OFF );
	{
		int i;
		OS_TPrintf( "CameraInfo :" );
		OS_GetCameraInfo( s_cameraInfo );
		for( i = 0; i < OS_TWL_HWINFO_CAMERA_LEN; i++ ) {
			if( ( i & 0x0f ) == 0 ) {
				OS_TPrintf( "\n" );
			}
			OS_TPrintf( " %02x,", s_cameraInfo[ i ] );
		}
		OS_TPrintf( "\n" );
	}
	OS_TPrintf( "ForceDisableWireless : %s\n", OS_IsForceDisableWireless() ? "TRUE" : "FALSE" );
	OS_TPrintf( "Region : %02x %s\n", OS_GetRegion(), ( OS_GetRegion() == 0xff ) ? "Invalid" : s_strRegion[ OS_GetRegion() ] );
	{
		u8 string[ 16 ];
		OS_GetSerialNo( string );
		OS_TPrintf( "SerialNo : %s\n", string );
	}
	
	OS_TPrintf("***End of demo\n");
	OS_Terminate();
}



/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}



static char *s_strRegion[] = {
	"JAPAN",
	"AMERICA",
	"EUROPA",
	"AUSTRALIA",
	"CHINA",
	"KOREA",
};

static char *s_strUserColor[] = {
	"GRAY      ",
	"BROWN     ",
	"RED       ",
	"PINK      ",
	"ORANGE    ",
	"YELLOW    ",
	"LIME_GREEN",
	"GREEN     ",
	"DARK_GREEN",
	"SEA_GREEN ",
	"TURQUOISE ",
	"BLUE      ",
	"DARK_BLUE ",
	"PURPLE    ",
	"VIOLET    ",
	"MAGENTA   ",
};

static char *s_strLanguage[] = {
	"JAPANESE",
	"ENGLISH",
	"FRENCH",
	"GERMAN",
	"ITALIAN",
	"SPANISH",
	"CHINESE",
	"KOREAN",
};

static char *s_strCountry[] = {
    	"UNDEFINED  ",        // ���ݒ�
    	"JAPAN      ",        // ���{
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"Anguilla   ",        // �A���M��
    	"ANTIGUA_AND_BARBUDA",   // �A���e�B�O�A�E�o�[�u�[�_
    	"ARGENTINA   ",      // �A���[���`��
    	"ARUBA",                 // �A���o
    	"BAHAMAS",               // �o�n�}
    	"BARBADOS",              // �o���o�h�X
    	"BELIZE",                // �x���[�Y
    	"BOLIVIA",               // �{���r�A
    	"BRAZIL",                // �u���W��
    	"BRITISH_VIRGIN_ISLANDS",    // �p�̃��@�[�W������
    	"CANADA",                // �J�i�_
    	"CAYMAN_ISLANDS",        // �P�C�}������
    	"CHILE       ",      // �`��
    	"COLOMBIA",              // �R�����r�A
    	"COSTA_RICA",            // �R�X�^���J
    	"DOMINICA",              // �h�~�j�J��
    	"DOMINICAN_REPUBLIC",    // �h�~�j�J���a��
    	"ECUADOR",               // �G�N�A�h��
    	"EL_SALVADOR",           // �G���T���o�h��
    	"FRENCH_GUIANA",         // �t�����X�̃M�A�i
    	"GRENADA",               // �O���i�_
    	"GUADELOUPE",            // �O�A�h���[�v
    	"GUATEMALA   ",      // �O�A�e�}��
    	"GUYANA",                // �K�C�A�i
    	"HAITI",                 // �n�C�`
    	"HONDURAS",              // �z���W�����X
    	"JAMAICA",               // �W���}�C�J
    	"MARTINIQUE",            // �}���e�B�j�[�N
    	"MEXICO",                // ���L�V�R
    	"MONTSERRAT",            // �����g�Z���g
    	"NETHERLANDS_ANTILLES",  // �I�����_�̃A���e�B��
    	"NICARAGUA",             // �j�J���O�A
    	"PANAMA      ",      // �p�i�}
    	"PARAGUAY",              // �p���O�A�C
    	"PERU",                  // �y���[
    	"ST_KITTS_AND_NEVIS",    // �Z���g�L�b�c�E�l�C�r�X
    	"ST_LUCIA",              // �Z���g���V�A
    	"ST_VINCENT_AND_THE_GRENADINES", // �Z���g�r���Z���g�E�O���i�f�B�[��
    	"SURINAME",              // �X���i��
    	"TRINIDAD_AND_TOBAGO",   // �g���j�_�[�h�E�g�o�S
    	"TURKS_AND_CAICOS_ISLANDS",  // �^�[�N�X�E�J�C�R�X����
    	"UNITED_STATES",         // �A�����J
    	"URUGUAY     ",      // �E���O�A�C
    	"US_VIRGIN_ISLANDS",     // �ė̃o�[�W������
    	"VENEZUELA",             // �x�l�Y�G��
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"ALBANIA     ",      // �A���o�j�A
    	"AUSTRALIA",             // �I�[�X�g�����A
    	"AUSTRIA",               // �I�[�X�g���A
    	"BELGIUM",               // �x���M�[
    	"BOSNIA_AND_HERZEGOVINA",    // �{�X�j�A�E�w���c�F�S�r�i
    	"BOTSWANA",              // �{�c���i
    	"BULGARIA    ",      // �u���K���A
    	"CROATIA",               // �N���A�`�A
    	"CYPRUS",                // �L�v���X
    	"CZECH_REPUBLIC",        // �`�F�R
    	"DENMARK",               // �f���}�[�N
    	"ESTONIA",               // �G�X�g�j�A
    	"FINLAND",               // �t�B�������h
    	"FRANCE",                // �t�����X
    	"GERMANY",               // �h�C�c
    	"GREECE",                // �M���V��
    	"HUNGARY     ",      // �n���K���[
    	"ICELAND",               // �A�C�X�����h
    	"IRELAND",               // �A�C�������h
    	"ITALY",                 // �C�^���A
    	"LATVIA",                // ���g�r�A
    	"LESOTHO",               // ���\�g
    	"LIECHTENSTEIN",         // ���q�e���V���^�C��
    	"LITHUANIA",             // ���g�A�j�A
    	"LUXEMBOURG",            // ���N�Z���u���N
    	"MACEDONIA",             // �}�P�h�j�A
    	"MALTA       ",      // �}���^
    	"MONTENEGRO",            // �����e�l�O��
    	"MOZAMBIQUE",            // ���U���r�[�N
    	"NAMIBIA",               // �i�~�r�A
    	"NETHERLANDS",           // �I�����_
    	"NEW_ZEALAND",           // �j���[�W�[�����h
    	"NORWAY",                // �m���E�F�[
    	"POLAND",                // �|�[�����h
    	"PORTUGAL",              // �|���g�K��
    	"ROMANIA",               // ���[�}�j�A
    	"RUSSIA      ",     // ���V�A
    	"SERBIA",                // �Z���r�A
    	"SLOVAKIA",              // �X���o�L�A
    	"SLOVENIA",              // �X���x�j�A
    	"SOUTH_AFRICA",          // ��A�t���J
    	"SPAIN",                 // �X�y�C��
    	"SWAZILAND",             // �X���W�����h
    	"SWEDEN",                // �X�E�F�[�f��
    	"SWITZERLAND",           // �X�C�X
    	"TURKEY",                // �g���R
    	"UNITED_KINGDOM ",  // �C�M���X
    	"ZAMBIA",                // �U���r�A
    	"ZIMBABWE",              // �W���o�u�G
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"TAIWAN      ",     // ��p
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"SOUTH_KOREA ",     // �؍�
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"HONG_KONG   ",     // �z���R��
    	"MACAU",                 // �}�J�I
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"INDONESIA   ",     // �C���h�l�V�A
    	"SINGAPORE   ",     // �V���K�|�[��
    	"THAILAND    ",     // �^�C
    	"PHILIPPINES",           // �t�B���s��
    	"MALAYSIA",              // �}���[�V�A
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"CHINA       ",     // ����
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"UAE         ",     // �A���u�񒷍��A�M
    	"INDIA       ",     // �C���h
    	"EGYPT       ",     // �G�W�v�g
    	"OMAN",                  // �I�}�[��
    	"QATAR",                 // �J�^�[��
    	"KUWAIT",                // �N�E�F�[�g
    	"SAUDI_ARABIA",          // �T�E�W�A���r�A
    	"SYRIA",                 // �V���A
    	"BAHRAIN",               // �o�[���[��
    	"JORDAN",                // �����_��
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"OTHERS      ",
    	"UNKNOWN     ",
};

