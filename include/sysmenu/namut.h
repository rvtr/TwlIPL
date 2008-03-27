/*---------------------------------------------------------------------------*
  Project:  TwlIPL - NAMUT
  File:     namut.h

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

#ifndef NAM_UTILITY_H_
#define NAM_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         NAMUT_Format

  Description:  NANDの擬似フォーマット
               （システム系の必要なファイルのみを残し他を消去します）

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL NAMUT_Format(void);

/*---------------------------------------------------------------------------*
  Name:         NAMUT_SearchInstalledSoftBoxCount

  Description:  InstalledSoftBoxCountの数を調べて返します。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
u32 NAMUT_SearchInstalledSoftBoxCount( void );

/*---------------------------------------------------------------------------*
  Name:         NAMUT_DrawNandTree

  Description:  NANDのツリー情報をプリント出力します

  Arguments:    ...

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NAMUT_DrawNandTree(void);


#endif // SDK_ARM9

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif	/* NAM_UTILITY_H_ */
