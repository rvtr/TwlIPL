/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mb_loader.h

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

#if !defined(_LOADER_H_)
#define _LOADER_H_

#include <twl.h>
#include <nitro/mb.h>	// twl/mb.h���Ȃ��B

#ifdef __cplusplus
extern "C" {
#endif

#define PXI_FIFO_TAG_MB					PXI_FIFO_TAG_USER_0
/*
	�����̍ہA����FIFO_TAG�Ƃ�������Ȃ�TAG�ԍ��Ɏw�肵�Ă��������B
*/

/*---------------------------------------------------------------------------*
  Type definition
 *---------------------------------------------------------------------------*/

typedef	void (*MB_LoaderCallback)(void);

/*---------------------------------------------------------------------------*
  functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         LOADER_Init

  Description:  ���[�_�[�̏�����

  Arguments:    callback - �u�[�g�̏������o�����Ƃ��ɕԂ��R�[���o�b�N

  Returns:      None.
 *---------------------------------------------------------------------------*/

void	LOADER_Init(MB_LoaderCallback callback);


/*---------------------------------------------------------------------------*
  Name:         LOADER_Start

  Description:  ���[�_�[�̃X�^�[�g

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void	LOADER_Start( void );


/*---------------------------------------------------------------------------*
  Name:         MIm_CpuCopy32

  Description:  ���[�_�[�p��CpuCopy32�֐�

  Arguments:    

  Returns:      TRUE - success FALSE - failed
 *---------------------------------------------------------------------------*/

void	MIm_CpuCopy32( register const void *srcp, register void *destp, register u32 size );


#ifdef __cplusplus
}
#endif

#endif /*	_LOADER_H_	*/
