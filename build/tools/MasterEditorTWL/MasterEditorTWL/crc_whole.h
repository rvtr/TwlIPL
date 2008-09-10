#pragma once

// 署名用の鍵データの宣言

#include <twl/types.h>
#include <cstdio>

namespace MasterEditorTWL
{
	/*---------------------------------------------------------------------------*

	 Name:        getSeg3CRCInFp

	 Description: Segment3(0x3000-0x3fff)のCRCを算出

	 Arguments:   [in]  srlファイルのファイルポインタ
				  [out] CRC格納先

	 Return:      成功ならTRUE.

	 *---------------------------------------------------------------------------*/
	BOOL getSeg3CRCInFp( FILE *fp, u16 *pCRC );

	/*---------------------------------------------------------------------------*

	 Name:        getWholeCRCInFp

	 Description: srlファイル全体のCRCを算出

	 Arguments:   [in]  srlファイルのファイルポインタ
				  [out] CRC格納先

	 Return:      成功ならTRUE.

	 *---------------------------------------------------------------------------*/
	BOOL getWholeCRCInFp( FILE *fp, u16 *pCRC );

	/*---------------------------------------------------------------------------*

	 Name:        getWholeCRCInFile

	 Description: srlファイル全体のCRCを算出

	 Arguments:   [in]  srlファイル名(呼び出し前に閉じておくこと)
				  [out] CRC格納先

	 Return:      None.

	 *---------------------------------------------------------------------------*/
	BOOL getWholeCRCInFile( System::String ^filename, u16 *pCRC );

} // end of namespace MasterEditorTWL