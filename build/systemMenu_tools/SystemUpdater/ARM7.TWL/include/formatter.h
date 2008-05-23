/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     formatter.h

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
#ifndef NAND_INITIALIZER_FORMATTER_H_
#define NAND_INITIALIZER_FORMATTER_H_

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

typedef enum {
	FORMAT_MODE_QUICK,	// Quick�t�H�[�}�b�g
	FORMAT_MODE_FULL	// Full�t�H�[�}�b�g(�e�p�[�e�B�V��������0xFF�Ŗ��߂�j
} FormatMode;


#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

BOOL ExeFormat(FormatMode format_mode);

/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* NAND_INITIALIZER_FORMATTER_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
