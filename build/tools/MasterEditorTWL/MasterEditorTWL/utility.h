#pragma once

// ���p���C�u�����E�N���X�̐錾

#include <twl/types.h>
#include <cstring>
#include <cstdio>

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

	//
	// �o�C�g��ɓ���̃p�^�[�����܂܂�邩�ǂ����}�b�`���O����
	//
	// @arg [in] �e�L�X�g
	//      [in] �e�L�X�g�̒���
	//      [in] �p�^�[��
	//      [in] �p�^�[���̒���
	//      [in] �e�L�X�g�̏I�[�܂Œ��ׂ����_�Ńe�L�X�g���r���܂Ń}�b�`���Ă����ꍇ�𐬌��Ƃ݂Ȃ���
	//
	// @ret �}�b�`�����e�L�X�g�̃C���f�b�N�X�����X�g�ŕԂ��B
	//
	System::Collections::Generic::List<u32>^ patternMatch( 
		const u8 *text, const u32 textLen, const u8 *pattern, const u32 patternLen, const System::Boolean enableLast );

	//
	// �t�@�C���Ƀo�C�g��̃p�^�[�����܂܂�邩�ǂ����}�b�`���O����
	//
	// @arg [in] �}�b�`�ΏۂƂȂ�t�@�C���|�C���^
	// @arg [in] �p�^�[��
	// @arg [in] �p�^�[���̒���
	//
	// @ret �}�b�`�����e�L�X�g�̃C���f�b�N�X�����X�g�ŕԂ��B
	//      �Ō�܂Ń}�b�`�����ꍇ�̂ݐ��������Ƃ݂Ȃ��B
	//
	System::Collections::Generic::List<u32>^ patternMatch( FILE *fp, const u8 *pattern, const u32 patternLen );

	void debugPatternMatch( System::String ^filename );

} // end of namespace MasterEditorTWL
