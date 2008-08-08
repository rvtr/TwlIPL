#pragma once

// ���p���C�u�����E�N���X�̐錾

#include <twl/types.h>

namespace MasterEditorTWL
{

	// -------------------------------------------------------------------
	// functions
	// -------------------------------------------------------------------

	//
	// String �� char �z��Ɋi�[
	//
	// @arg [out] �i�[��
	// @arg [in]  �i�[��
	// @arg [in]  ������
	// @arg [in]  �]��𖄂߂� padding
	//
	void setStringToChars( char *pDst, System::String ^hSrc, const System::Int32 nMax, const System::SByte pad  );

	//
	// ROM�w�b�_����ROM�e�ʐݒ�o�C�g����ROM�e�ʂ�\����������擾
	//
	// @arg [in]  ROM�e�ʐݒ�o�C�g
	//
	// @ret ������(�P�ʂ�)
	//
	System::String^ transRomSizeToString( System::Byte ^byte );

	//
	// �y�A�����^���R���g���[���̃��[�e�B���O�l���當������擾
	//
	// @arg [in] �c��
	// @arg [in] �L���t���O(0�Έȏォ���R�����𔻒肷�邽�߂Ɏg�p����)
	// @arg [in] ���[�e�B���O�l
	// @arg [in] �p��t���O
	//
	// @ret ���[�e�B���O�̕�����(�c�̂����������Ƃ�nullptr)
	//
	System::String^ transRatingToString( System::Byte ogn, System::Boolean enable, System::Byte rating, System::Boolean english ); 

} // end of namespace MasterEditorTWL
