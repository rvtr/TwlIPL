/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - makenorfirm
  File:     compress.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef COMPRESS_H_
#define COMPRESS_H_

#include    "misc.h"


/*---------------------------------------------------------------------------*
  Name:         MI_CompressLZ

  Description:  LZ77���k���s�Ȃ��֐�

  Arguments:    srcp            ���k���f�[�^�ւ̃|�C���^
                size            ���k���f�[�^�T�C�Y
                dstp            ���k��f�[�^�ւ̃|�C���^
                                ���k���f�[�^�����傫���T�C�Y�̃o�b�t�@���K�v�ł��B

  Returns:      ���k��̃f�[�^�T�C�Y�B
 *---------------------------------------------------------------------------*/
u32 LZCompWrite(u8 *srcp, u32 size, u8 *dstp, int boundary);


#endif //COMPRESS_H_
