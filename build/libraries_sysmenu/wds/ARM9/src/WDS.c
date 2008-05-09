/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     WDS.c

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

//**********************************************************************
/**
//	@file		WDS.c
//	@brief		�X���e�@��MP���M����ڑ���AP������M
//
//	@author		M.Okuno
//	@date		2008/02/05
//	@version	01.00
//
***********************************************************************/
#include <sysmenu/WDS.h>
#include <nitro/crypto/rc4.h>

//-----------------------------------------------------
//	Macros
//-----------------------------------------------------
/**
	@brief	�X�L�����o�b�t�@�T�C�Y
*/
#define	WDS_SCAN_BUF_SIZE		1024

/**
	@brief	�e�@AP���GGID
*/
#define	WDS_APINFO_GGID			0x00000857

/**
	@brief	�X�e�[�^�X
*/
enum 
{
	WDS_STATUS_INIT ,			///< ��������
	WDS_STATUS_STARTSCAN ,		///< �X�L�����J�n
	WDS_STATUS_ENDSCAN ,		///< �X�L�����I��
	WDS_STATUS_END				///< �I��
};

//-----------------------------------------------------
//	Structs
//-----------------------------------------------------
/**
	@brief	���[�N�̈�
*/
typedef struct WDSWork
{
	u8				wmwork[ WM_SYSTEM_BUF_SIZE ];	///< WM���C�u�����p�o�b�t�@
	
	u8				scanbuf[ WDS_SCAN_BUF_SIZE ] ATTRIBUTE_ALIGN(32);	///< �X�L�����o�b�t�@
	WMScanExParam	scanparam;						///< �X�L�����p�����[�^
	WDSCallbackFunc	scancb;							///< �X�L�����p�R�[���o�b�N
	WDSApInfo		apinfo[ WDS_APINFO_MAX ];		///< �X�L�����o����AP���
	u16				linklevel[ WDS_APINFO_MAX ];	///< �d�g���x
	u16				tgid[ WDS_APINFO_MAX ];			///< �����̃r�[�R�����󂯎�����ۂɎ��ʗp�Ɏg��TGID
	s32				apnum;							///< �X�L�����o����AP���
	u32				status;							///< �X�e�[�^�X
	
	MATHCRC32Context	crcContext;
	MATHCRC32Table		crcTable;
} WDSWork;

//-----------------------------------------------------
//	Variables
//-----------------------------------------------------
static WDSWork	*gWdsWork	= NULL;

//--------------------------------------------------------------------------------
/**	UTF8 �� UCS2 �֕ϊ����܂�
		@param	<1> UTF8 ����
		@param	<2> �o�C�g����
		@return	UCS2 ����
	@note
		browse_unicode.cpp ����ڐA�B
*///------------------------------------------------------------------------------
static u16	bu_UTF8_To_UCS2( const u8 *c, s32 *bytes_used )
{
	u16		ret;
	u8		b1, b2, b3;
	
	if( (*c & 0x80) == 0 ) {
		// 1�o�C�g�AASCII
		ret			= *c;
		*bytes_used	= 1;
	}
	else if( (*c & 0xe0) == 0xc0 ) {
		// 2�o�C�g�A���B�����Ƃ��H
		b1	= (u8)(c[0] & 0x1f);
		b2	= (u8)(c[1] & 0x3f);
		ret	= (u16)(( b1 << 6 ) | b2 );
		*bytes_used	= 2;
	}
	else if( (*c & 0xf0) == 0xe0 ) {
		// 3�o�C�g�A���{��͂�������
		b1	= (u8)(c[0] & 0x0f);
		b2	= (u8)(c[1] & 0x3f);
		b3	= (u8)(c[2] & 0x3f);
		ret	= (u16)(( b1 << 12 ) | ( b2 << 6 ) | b3 );
		*bytes_used	= 3;
	}
	else {
		// �z��O�̒l�̏ꍇ�AASCII�̃n�e�i��Ԃ��Ƃ�
		ret			= 0x3F;
		*bytes_used	= 1;
	}
	return ret;
}

//--------------------------------------------------------------------------------
/**	WM_StartScanEx() �p�R�[���o�b�N
		@param	<1> �p�����[�^
*///------------------------------------------------------------------------------
static void	WDSScanCallback( void *arg )
{
	WMStartScanExCallback	*pParam	= (WMStartScanExCallback*)arg;
	u32						i, j;
	
	// �X�L�����̌��ʓ���ꂽ�f�[�^��ARM7����DMA�o�R�Ń��C���������ɒ��ڏ������܂��
	// ���̂���ARM9�̃f�[�^�L���b�V�����������񖳌������Ȃ���΃f�[�^�𐳏�ɓǂ߂Ȃ����Ƃɒ���
	DC_InvalidateRange(gWdsWork->scanbuf, WDS_SCAN_BUF_SIZE);
	
	// �e�@�𔭌�����������
	if( pParam->errcode == WM_ERRCODE_SUCCESS && pParam->state == WM_STATECODE_PARENT_FOUND ) {
		// ���������e�@�̒��ɓX���p�e�@�����邩����
		for( i = 0 ; i < pParam->bssDescCount ; i++ ) {
			// GGID ����
			if( WM_IsValidGameInfo( &pParam->bssDesc[i]->gameInfo, sizeof(WMGameInfo) ) && pParam->bssDesc[i]->gameInfo.ggid == WDS_APINFO_GGID ) {
				// AP ��񔭌�
				CRYPTORC4FastContext	rc4context;
				u8						rc4key[ 8 ];
				BOOL					duplicated;
				const u32				magic	= 'WDS!';
				
				// �r�[�R���̏d���`�F�b�N
				duplicated = FALSE;
				for( j = 0 ; j < gWdsWork->apnum ; j++ )
				{
					if( gWdsWork->tgid[j] == pParam->bssDesc[i]->gameInfo.tgid )
					{
						duplicated = TRUE;
						break;
					}
				}
				if( duplicated == TRUE )
					continue;
				
				// �f�o�b�O�\��
				OS_TPrintf( "Found AP GGID : %08x TGID : %04x\n", pParam->bssDesc[i]->gameInfo.ggid, pParam->bssDesc[i]->gameInfo.tgid );
				
				// �r�[�R�����e���R�s�[���A�Í�������
				MI_CpuCopy8( pParam->bssDesc[i]->gameInfo.userGameInfo, &gWdsWork->apinfo[gWdsWork->apnum], sizeof(WDSApInfo) );
				MI_CpuCopy8( &magic, &rc4key[0], 4 );
				MI_CpuCopy8( &pParam->bssDesc[i]->bssid[2], &rc4key[4], 4 );
				CRYPTO_RC4FastInit( &rc4context, rc4key, 8 );
				CRYPTO_RC4FastEncrypt( &rc4context, &gWdsWork->apinfo[gWdsWork->apnum], sizeof(WDSApInfo), &gWdsWork->apinfo[gWdsWork->apnum] );
				
				// ���̑��̃f�[�^��ݒ�
				gWdsWork->linklevel[gWdsWork->apnum]	= pParam->linkLevel[i];
				gWdsWork->tgid[gWdsWork->apnum]			= pParam->bssDesc[i]->gameInfo.tgid;
				gWdsWork->apnum = ( gWdsWork->apnum + 1 ) % WDS_APINFO_MAX;
			}
		}
	}
	
	// �X�L�����I��
	gWdsWork->status	= WDS_STATUS_ENDSCAN;
	
	// �R�[���o�b�N���s
	if( gWdsWork->scancb )
		gWdsWork->scancb( arg );
}

//--------------------------------------------------------------------------------
/**	WDS���C�u�������g�p���郏�[�N�G���A�̃T�C�Y�𓾂܂�
		@return 0 �ȊO : ���[�N�G���A�̃T�C�Y
	@note
		���C�u�������������Ɏg�p���܂�
*///------------------------------------------------------------------------------
size_t WDS_GetWorkAreaSize( void )
{
	return sizeof( WDSWork );
}

//--------------------------------------------------------------------------------
/**	WDS�����������܂�
		@param	<1> �Ăяo�����ɂ���Ċm�ۂ��ꂽ�o�b�t�@�ւ̃|�C���^���w�肵�܂�
		@param	<2> ���������������ۂɌĂяo�����R�[���o�b�N�֐��ւ̃|�C���^
		@param	<3> WM_Initialize��dmaNo�ɏ��������
		@return 0      : ���� ( �R�[���o�b�N��҂��� )
				0 �ȊO : ���s
	@note
		<1> �̃T�C�Y��WDS_SYSTEM_BUF_SIZE�����K�v�ł���A
		���� 32�o�C�g�A���C������Ă���K�v������܂��B
		<2> �̃R�[���o�b�N�͊��荞�݃n���h��������Ăяo����܂��B
		WDS �֘A�̃R�[���o�b�N�֐����ł͌Ăяo���܂���B
		�֐�������WM_Initialize()�����s����ׁAWM��READY�X�e�[�g����IDLE�X�e�[�g�Ɉڍs���܂��B
*///------------------------------------------------------------------------------
int	WDS_Initialize( void *wdsWork, WDSCallbackFunc callback, u16 dmaNo )
{
	WMErrCode	errcode;
	
	SDK_ASSERT( wdsWork );
	
	if( callback == NULL )
		return -1;
	
	// ���[�N�p�o�b�t�@�ݒ�
	gWdsWork	= (WDSWork*)wdsWork;
	
	// ������������
	MI_CpuClear8( gWdsWork, WDS_GetWorkAreaSize() );
	gWdsWork->status	= WDS_STATUS_INIT;
	
	// WM ���C�u����������
	errcode	= WM_Initialize( gWdsWork->wmwork, callback, dmaNo );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	return 0;
}


//--------------------------------------------------------------------------------
/**	WDS�����������܂�(�w�肵���X�L�������ʂ�����I�ɕێ�������Ԃł̏�����)
		@param	<1> �Ăяo�����ɂ���Ċm�ۂ��ꂽ�o�b�t�@�ւ̃|�C���^���w�肵�܂�
		@param	<2> ���������������ۂɌĂяo�����R�[���o�b�N�֐��ւ̃|�C���^
		@param	<3> WM_Initialize��dmaNo�ɏ��������
		@param	<4> ���O��WDS���g�p���Ď擾����WDSBriefInfo�̔z��
		@return 0      : ���� ( �R�[���o�b�N��҂��� )
				0 �ȊO : ���s
	@note
		<1> �̃T�C�Y��WDS_SYSTEM_BUF_SIZE�����K�v�ł���A
		���� 32�o�C�g�A���C������Ă���K�v������܂��B
		<2> �̃R�[���o�b�N�͊��荞�݃n���h��������Ăяo����܂��B
		WDS �֘A�̃R�[���o�b�N�֐����ł͌Ăяo���܂���B
		�֐�������WM_Initialize()�����s����ׁAWM��READY�X�e�[�g����IDLE�X�e�[�g�Ɉڍs���܂��B
*///------------------------------------------------------------------------------
int	WDS_InitializeEx( void *wdsWork, WDSCallbackFunc callback, u16 dmaNo, WDSBriefApInfo *apinfo )
{
	WMErrCode	errcode;
	int			i;
	
	SDK_ASSERT( wdsWork );
	
	if( callback == NULL )
		return -1;
	
	// ���[�N�p�o�b�t�@�ݒ�
	gWdsWork	= (WDSWork*)wdsWork;
	
	// ������������
	MI_CpuClear8( gWdsWork, WDS_GetWorkAreaSize() );
	gWdsWork->status	= WDS_STATUS_INIT;
	
	// �����l���R�s�[
	for( i = 0 ; i < WDS_APINFO_MAX ; i++ )
	{
		if( apinfo[i].isvalid == TRUE )
		{
			gWdsWork->apnum++;
			gWdsWork->apinfo[i]		= apinfo[i].apinfo;
			gWdsWork->linklevel[i]	= apinfo[i].rssi;
		}
	}
	
	// WM ���C�u����������
	errcode	= WM_Initialize( gWdsWork->wmwork, callback, dmaNo );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	
	// �����l����ꂽ�֌W��A�X�e�[�^�X���X�L���������ɂ���
	gWdsWork->status = WDS_STATUS_ENDSCAN;
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	WDS���I�����܂�
		@param	<1> ���������������ۂɌĂяo�����R�[���o�b�N�֐��ւ̃|�C���^
		@return 0      : ���� ( �R�[���o�b�N��҂��� )
				0 �ȊO : ���s
	@note
		<1> �̃R�[���o�b�N�͊��荞�݃n���h��������Ăяo����܂��B
		WDS �֘A�̃R�[���o�b�N�֐����ł͌Ăяo���܂���B
*///------------------------------------------------------------------------------
int	WDS_End( WDSCallbackFunc callback )
{
	WMErrCode	errcode;
	
	if( callback == NULL )
		return -1;
	
	// WDS �I��
	gWdsWork->status	= WDS_STATUS_END;
	gWdsWork			= NULL;
	
	// WM �I��
	errcode	= WM_End( callback );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	return 0;
}

//--------------------------------------------------------------------------------
/**	DS�z�M�V�X�e���e�@�r�[�R�����X�L�������J�n���܂�
		@param	<1> ���������������ۂɌĂяo�����R�[���o�b�N�֐��ւ̃|�C���^
		@return 0      : ���� ( �R�[���o�b�N��҂��� )
				0 �ȊO : ���s
	@note
		<1> �̃R�[���o�b�N�͊��荞�݃n���h��������Ăяo����܂��B
		WDS �֘A�̃R�[���o�b�N�֐����ł͌Ăяo���܂���B
		�֐�������WM_StartScane()�����s����ׁAWM��IDLE�X�e�[�g�̏ꍇ��SCAN�X�e�[�g�Ɉڍs���܂��B
*///------------------------------------------------------------------------------
int	WDS_StartScan( WDSCallbackFunc callback )
{
	WMErrCode	errcode;
	
	SDK_ASSERT( gWdsWork );
	
	if( callback == NULL )
		return -1;
	
	// �X�L�����p�����[�^�ݒ�
	// �X�L�����`�����l���� 1, 7, 13
	// 300ms�`600ms �X�L�������s��
	gWdsWork->scanparam.scanBuf			= (WMBssDesc*)&gWdsWork->scanbuf;
	gWdsWork->scanparam.scanBufSize		= WDS_SCAN_BUF_SIZE;
	gWdsWork->scanparam.channelList		= WM_GetAllowedChannel();
	gWdsWork->scanparam.maxChannelTime	= WM_GetDispersionScanPeriod();
	gWdsWork->scanparam.scanType		= WM_SCANTYPE_PASSIVE;
	MI_CpuFill8( gWdsWork->scanparam.bssid, 0xFF, WDS_MACADDR_BUF_SIZE );
	
	// �R�[���o�b�N�ۑ�
	gWdsWork->scancb	= callback;
	
	// �X�L�����J�n
	gWdsWork->status	= WDS_STATUS_STARTSCAN;
	errcode	= WM_StartScanEx( WDSScanCallback, &gWdsWork->scanparam );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	return 0;
}

//--------------------------------------------------------------------------------
/**	DS�z�M�V�X�e���e�@�r�[�R�����X�L�������I�����܂�
		@param	<1> ���������������ۂɌĂяo�����R�[���o�b�N�֐��ւ̃|�C���^
		@return 0      : ���� ( �R�[���o�b�N��҂��� )
				0 �ȊO : ���s
	@note
		<1> �̃R�[���o�b�N�͊��荞�݃n���h��������Ăяo����܂��B
		WDS �֘A�̃R�[���o�b�N�֐����ł͌Ăяo���܂���B
		�֐�������WM_EndScan()�����s����ׁAWM��IDLE�X�e�[�g�Ɉڍs���܂��B
*///------------------------------------------------------------------------------
int	WDS_EndScan( WDSCallbackFunc callback )
{
	WMErrCode	errcode;
	
	SDK_ASSERT( gWdsWork );
	
	if( callback == NULL )
		return -1;
	
	// �X�L�����I��
	gWdsWork->status	= WDS_STATUS_ENDSCAN;
	errcode	= WM_EndScan( callback );
	if( errcode != WM_ERRCODE_OPERATING )
		return errcode;
	return 0;
}

//--------------------------------------------------------------------------------
/**	AP�r�[�R�����̐����擾���܂�
		@return 0 �ȏ� : ����
				0 ���� : ���s ( �X�L������ )
	@note
		�X�L�������Ɏ��s�����ꍇ�̓G���[�l��Ԃ��܂��B
*///------------------------------------------------------------------------------
int	WDS_GetApInfoNum( void )
{
	SDK_ASSERT( gWdsWork );
	
	// �X�L����������
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	return gWdsWork->apnum;
}

//--------------------------------------------------------------------------------
/**	AP�r�[�R�������C���f�b�N�X���w�肵�Ď擾���܂�
		@param	<1> �擾����AP�r�[�R�����̃C���f�b�N�X�l(0�`15)
		@param	<2> �擾����AP�r�[�R�������������ޗ̈�ւ̃|�C���^
		@return 0      : ����
				0 �ȊO : ���s ( �X�L�������A���̓C���f�b�N�X�̎����ʒu�Ƀf�[�^���Ȃ� )
	@note
		�X�L�������Ɏ��s�����ꍇ�̓G���[�l��Ԃ��܂��B
*///------------------------------------------------------------------------------
int	WDS_GetApInfoByIndex( int index, WDSBriefApInfo *briefapinfo )
{
	SDK_ASSERT( gWdsWork );
	
	// �X�L����������
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	
	// �C���f�b�N�X�G���[����
	if( index < 0 || index >= gWdsWork->apnum )
		return -1;
	
	// ������������
	MI_CpuClear8( briefapinfo, sizeof(WDSBriefApInfo) );
	
	// ���̑��ݒ�
	briefapinfo->isvalid		= TRUE;
	briefapinfo->rssi			= gWdsWork->linklevel[index];
	MI_CpuCopy8(&gWdsWork->apinfo[index], &briefapinfo->apinfo, sizeof(WDSApInfo));
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	AP�r�[�R������S�Ď擾���܂�
		@param	<1> �擾����AP�r�[�R�������������ޔz��̐擪�ւ̃|�C���^(WDS_APINFO_MAX�̗v�f���K�v)
		@return 0      : ����
				0 �ȊO : ���s ( �X�L�������A���̓C���f�b�N�X�̎����ʒu�Ƀf�[�^���Ȃ� )
	@note
		�X�L�������Ɏ��s�����ꍇ�̓G���[�l��Ԃ��܂��B
*///------------------------------------------------------------------------------
int	WDS_GetApInfoAll( WDSBriefApInfo *briefapinfo )
{
	int index;
	
	SDK_ASSERT( gWdsWork );
	
	// �X�L����������
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	
	// ������������
	MI_CpuClear8( briefapinfo, sizeof(WDSBriefApInfo)*WDS_APINFO_MAX );
	for( index = 0 ; index < WDS_APINFO_MAX ; index++ )
	{
		briefapinfo[index].isvalid = FALSE;
	}
	
	// �w��C���f�b�N�X��AP�r�[�R�������R�s�[
	// �z�b�g�X�|�b�g����ݒ� ( UTF-8 �̏ꍇ�� UTF-16 �ɕϊ� )
	// �S�Ă�AP�r�[�R�������R�s�[
	for( index = 0 ; index < gWdsWork->apnum ; index++ ) {
		if( WDS_GetApInfoByIndex( index, briefapinfo+index ) == -1 )
			break;
	}
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	AP�r�[�R������DWC�̎����ڑ���Ƃ��Đݒ肵�܂�
		@param	<1> �����ڑ���Ƃ��Đݒ肷��AP�r�[�R�����̃C���f�b�N�X�l(0�`15)
		@return 0      : ����
				0 �ȊO : ���s ( �X�L�������A���̓C���f�b�N�X�̎����ʒu�Ƀf�[�^���Ȃ� )
	@note
		�X�L�������Ɏ��s�����ꍇ�̓G���[�l��Ԃ��܂��B
*///------------------------------------------------------------------------------
int	WDS_SetConnectTargetByIndex( int index )
{
	SDK_ASSERT( gWdsWork );
	
	// �X�L����������
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	
	// �C���f�b�N�X�G���[����
	if( index < 0 || index >= gWdsWork->apnum )
		return -1;
	
	// �w�� SSID �֐ڑ�
	//DWC_AC_SetSpecifyAp2( gWdsWork->apinfo[index].ssid, (u16)((int)gWdsWork->apinfo[index].channel-1), gWdsWork->apinfo[index].wepkey, gWdsWork->apinfo[index].encryptflag );
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	AP�r�[�R������DWC�̎����ڑ���Ƃ��Đݒ肵�܂�
		@param	<1> �����ڑ���Ƃ��Đݒ肷��AP�r�[�R�����
		@return 0      : ����
				0 �ȊO : ���s ( �X�L�������A���̓C���f�b�N�X�̎����ʒu�Ƀf�[�^���Ȃ� )
	@note
		�X�L�������Ɏ��s�����ꍇ�̓G���[�l��Ԃ��܂��B
*///------------------------------------------------------------------------------
int	WDS_SetConnectTargetByBriefApInfo( WDSBriefApInfo *briefapinfo )
{
#pragma unused(briefapinfo)
	
	SDK_ASSERT( gWdsWork );
	
	// �X�L����������
	if( gWdsWork->status != WDS_STATUS_ENDSCAN )
		return -1;
	
	// �w�� SSID �֐ڑ�
	//DWC_AC_SetSpecifyAp2( briefapinfo->apinfo.ssid, (u16)((int)briefapinfo->apinfo.channel-1), briefapinfo->apinfo.wepkey, briefapinfo->apinfo.encryptflag );
	
	return 0;
}

//--------------------------------------------------------------------------------
/**	AP�r�[�R������AP��������UTF-16�œ��܂�
		@param	<1> AP���������擾����Ώۂ�AP�r�[�R�����
				<2> AP���������������ރo�b�t�@�ւ̃|�C���^(WDS_HOTSPOTNAME_UTF16_BUF_SIZE�o�C�g)
		@return 0      : ����
				0 �ȊO : ���s ( �X�L�������A���̓C���f�b�N�X�̎����ʒu�Ƀf�[�^���Ȃ� )
	@note
		�X�L�������Ɏ��s�����ꍇ�̓G���[�l��Ԃ��܂��B
*///------------------------------------------------------------------------------
int	WDS_GetApDescriptionUTF16( WDSBriefApInfo *briefapinfo, void *outbuf )
{
	SDK_ASSERT( briefapinfo->isvalid );
	
	// �o�b�t�@���N���A
	MI_CpuClear8( outbuf, WDS_HOTSPOTNAME_UTF16_BUF_SIZE );
	
	// �w��C���f�b�N�X��AP�r�[�R�������R�s�[
	// �z�b�g�X�|�b�g����ݒ� ( UTF-8 �̏ꍇ�� UTF-16 �ɕϊ� )
	if( (briefapinfo->apinfo.hotspotid & WDS_HOTSPOT_ENCODE_MASK) == WDS_HOTSPOT_ENCODE_UTF8 ) {
		// UTF-8
		u8		*pStr		= briefapinfo->apinfo.hotspotname;
		u8		*pEndStr	= briefapinfo->apinfo.hotspotname + WDS_HOTSPOTNAME_BUF_SIZE;
		u16		*pUcs2		= (u16*)outbuf;
		s32		temp;
		
		// UTF-16�ɕϊ�
		while( pStr != pEndStr && *pStr != '\0' ) {
			*pUcs2	= bu_UTF8_To_UCS2( pStr, &temp );
			pStr	+= temp;
			pUcs2	+= 1;
		}
	}
	else {
		// UTF-16
		MI_CpuCopy8( briefapinfo->apinfo.hotspotname, outbuf, WDS_HOTSPOTNAME_BUF_SIZE );
	}
	
	return 0;
}
