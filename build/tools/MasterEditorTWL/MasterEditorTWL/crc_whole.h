#pragma once

// �����p�̌��f�[�^�̐錾

#include <twl/types.h>
#include <cstdio>

namespace MasterEditorTWL
{
	/*---------------------------------------------------------------------------*

	 Name:        getWholeCRCInFp

	 Description: srl�t�@�C���S�̂�CRC���Z�o

	 Arguments:   [in]  srl�t�@�C���̃t�@�C���|�C���^
				  [out] CRC�i�[��

	 Return:      �����Ȃ�TRUE.

	 *---------------------------------------------------------------------------*/
	BOOL getWholeCRCInFp( FILE *fp, u16 *pCRC );

	/*---------------------------------------------------------------------------*

	 Name:        getWholeCRCInFile

	 Description: srl�t�@�C���S�̂�CRC���Z�o

	 Arguments:   [in]  srl�t�@�C����(�Ăяo���O�ɕ��Ă�������)
				  [out] CRC�i�[��

	 Return:      None.

	 *---------------------------------------------------------------------------*/
	BOOL getWholeCRCInFile( System::String ^filename, u16 *pCRC );

} // end of namespace MasterEditorTWL