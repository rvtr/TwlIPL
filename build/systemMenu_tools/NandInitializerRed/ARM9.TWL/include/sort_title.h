/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     sort_title.h

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

/*

�y���������z

TitleID �� �p�X������̐擪�A�h���X �� TitleSortSet �\���̂ɃZ�b�g����
TitleSortSet �\���̂� SortTitle �֐��Ɉ����n���܂��B

�p�X������̎��̂́A�K���ɂǂ����Ɋi�[���Ă����ĘM��Ȃ��悤�ɂ��Ă��������B
���Ɏg�p��������܂��B


�y�[���R�[�h�z

#define FILE_NUM_MAX         256         // �蔲��
#define QSORT_BUF_SIZE       ((8+1) * 8) // �T�C�Y��(log2(num)+1) * 8 bytes 

	char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
	TitleSortSet sTitleSortSet[FILE_NUM_MAX];
	FSDirectoryEntryInfo   info[1];
	NAMTadInfo tadInfo;
	char qsortBuf[QSORT_BUF_SIZE];

	while (�t�@�C���S���r�߂�܂�)
	{
		// �` info �Ƀt�@�C�����ǂݍ��݁`
		// �` tadInfo �� tad ���擾�`
		
		STD_CopyString( sFilePath[ counter ], info->longname );
		sTitleSortSet[ counter ].titleID = tadInfo.titleInfo.titleId;
		sTitleSortSet[ counter ].path = sFilePath[ counter ];
		
		counter++;
	}
	
	SortTitle( sTitleSortSet, counter, qsortBuf );
	
	// �\�[�g����

*/

#ifndef TWL_SORT_TITLE_H_
#define TWL_SORT_TITLE_H_

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#ifdef  __cplusplus
extern "C" {
#endif

#include <twl.h>

/*===========================================================================*/

typedef struct {
	OSTitleId titleID;
	char *path;
} TitleSortSet;

// TitleSortSet �̔z�������@���ɂ��������ă\�[�g���܂��B
// ������ MATH_QSort ���g�p���Ă���̂ŁAMATH_QSortStackSize() �֐��Ŏ擾�ł���
// �T�C�Y�̍�ƃo�b�t�@�� buf �ɗ^����K�v������܂��B
// ���̃T�C�Y�� (log2(num)+1) * 8 byte �ƂȂ��Ă��܂��B
// ��ƃo�b�t�@��n���Ȃ��ꍇ�ɂ́A�X�^�b�N���炱�̃T�C�Y�̍�Ɨ̈悪�m�ۂ���܂��B
// 
// info        �\�[�g���� TitleSortSet �z��̐擪�A�h���X
// num         �\�[�g����z��̗v�f��
// buf         ��ƃo�b�t�@
void SortTitle( TitleSortSet *info, u32 num, void *buf );

/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_SORT_TITLE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
