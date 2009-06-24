/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SystemUpdaterRegionSelect
  File:     sort_title.c

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

#include "sort_title.h"

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/

// �\�[�g�p Compare �֐�
static s32 TitleCompareFunc(void *elem1, void *elem2)
{
	TitleSortSet *ipp1 = (TitleSortSet *)elem1;
	TitleSortSet *ipp2 = (TitleSortSet *)elem2;
	u32 titleID_lo1 = (u32)(0xffffffff & ipp1->titleID);
	u32 titleID_lo2 = (u32)(0xffffffff & ipp2->titleID);
	BOOL isSystem1 = (u32)( 0x1 & ( ipp1->titleID >> 32 ) ) ? TRUE : FALSE;
	BOOL isSystem2 = (u32)( 0x1 & ( ipp2->titleID >> 32 ) ) ? TRUE : FALSE;
	BOOL isH1 = ( (u8)( 0xff & ( titleID_lo1 >> 24 ) ) == 'H' ) ? TRUE : FALSE;
	BOOL isH2 = ( (u8)( 0xff & ( titleID_lo2 >> 24 ) ) == 'H' ) ? TRUE : FALSE;
	
	if( isSystem1 && !isSystem2 )
	{
		// �v�f�P�� System �ł���A�v�f�Q�� System �łȂ��ꍇ�A�v�f�P���O (-1)
		return -1;
	}else if( !isSystem1 && isSystem2 )
	{
		// �v�f�P�� System �łȂ��A�v�f�Q�� System �ł���ꍇ�A�v�f�P���� (1)
		return 1;
	}else if( isH1 && !isH2 )
	{
		// �v�f�P�� "H***" �ł���A�v�f�Q�� "H***" �łȂ��ꍇ�A�v�f�P���O (-1)
		return -1;
	}else if( !isH1 && isH2 )
	{
		// �v�f�P�� "H***" �łȂ��A�v�f�Q�� "H***" �ł���ꍇ�A�v�f�P���� (1)
		return 1;
	}else
	{
		// ���̑��̏ꍇ�AtitleID_lo�̏������ق����O
		return ( titleID_lo1 > titleID_lo2 ) ?
				1 :
				( (titleID_lo1 < titleID_lo2) ? -1 : 0 );
	}
	
}

// TitleSortSet �̔z�������@���ɂ��������ă\�[�g���܂��B
// ������ MATH_QSort ���g�p���Ă���̂ŁAMATH_QSortStackSize() �֐��Ŏ擾�ł���
// �T�C�Y�̍�ƃo�b�t�@�� buf �ɗ^����K�v������܂��B
// ���̃T�C�Y�� (log2(num)+1) * 8 byte �ƂȂ��Ă��܂��B
// ��ƃo�b�t�@��n���Ȃ��ꍇ�ɂ́A�X�^�b�N���炱�̃T�C�Y�̍�Ɨ̈悪�m�ۂ���܂��B
// 
// info        �\�[�g���� TitleSortSet �z��̐擪�A�h���X
// num         �\�[�g����z��̗v�f��
// buf         ��ƃo�b�t�@
void SortTitle( TitleSortSet *info, u32 num, void *buf )
{
	MATH_QSort( info, num, sizeof(TitleSortSet), TitleCompareFunc, buf );
}