/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     viewSystemInfo.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef	__LOAD_VIEW_INFO__
#define	__LOAD_VIEW_INFO__

#include <twl.h>
#include "drawFunc.h"
#include "address.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////

typedef enum ChangeFuncArg{
	ARG_BOOL,
	ARG_INT,
	ARG_OTHER
} ChangeFuncArg;

typedef struct DispInfoEntry
{
	BOOL	isNumData;	// �\������Ƃ��͐��l�f�[�^��������f�[�^��
	BOOL	isSjis;		// UTF16�ŕ`�悷��f�[�^����FALSE isNumData=TRUE�Ȃ疢��`
	BOOL	isAligned;	// ���ɕ\���ł���Ȃ�TRUE�A���������K�v�Ȃ�FALSE
	int 	numLines;	// ���ږ��A���ړ��e��\������̂ɕK�v�ȍs��
	
	const char	*kind;		// ���ږ�
	
	union {
		char*	sjis;
		u16*	utf;
	} str;
	
	int		iValue;		// �f�[�^�̐��l�^�\��(�C���f�N�X�Ƃ�)
	
	BOOL	changable;	// ���̒l���ύX�\���ۂ�
	
	// ����������changable��true�̃G���g���̂ݐݒ肳���
	ChangeFuncArg	argType; // �l��ύX���邽�߂̊֐��̈����^
	
	// �l��ύX���邽�߂̊֐�
	union {
		void	(*cBool)(bool);
		void	(*cInt)(int);
	} changeFunc;
	
	char 		**kindNameList;	// ���ږ��ꗗ�̐擪�ւ̃|�C���^
	int			numKindName;	// ���ږ��ꗗ�̒���
	
} DispInfoEntry;


////////////////////////////////

// �e��{�́A���[�U���
extern DispInfoEntry*	gAllInfo[ROOTMENU_SIZE];

extern s32 gNumContents;
extern OSTitleId *gContentsTitle;
extern u16 *gContentsVersion;

extern u8 gArm7SCFGReg[DISPINFO_SHARED_SCFG_REG_SIZE];		// ARM7����̃f�[�^�擾�p�o�b�t�@
extern u8 gArm7SCFGShared[DISPINFO_SHARED_SCFG_WRAM_SIZE];	// ARM7����̃f�[�^�擾�p�o�b�t�@

////////////////////////////////

void displayInfoMain( void );
void displayInfoInit( void );

////////////////////////////////




#ifdef __cplusplus
}
#endif

#endif  // __LOAD_VIEW_INFO__
