// utility.h �̎���

#include "stdafx.h"
#include "utility.h"
#include <cstring>
#include <cstdio>
#include <twl/types.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/ownerInfoEx.h>

//
// String �� char �z��Ɋi�[
//
// @arg [out] �i�[��
// @arg [in]  �i�[��
// @arg [in]  ������
// @arg [in]  �]��𖄂߂� padding
//
void MasterEditorTWL::setStringToChars( char *pDst, System::String ^hSrc, 
									    const System::Int32 nMax, const System::SByte pad )
{
	System::Int32 i;

	memset( pDst, pad, nMax );
	for( i=0; (i < hSrc->Length) && (i < nMax); i++ )
	{
		pDst[i] = (char)hSrc[i];
	}
}

//
// ROM�w�b�_����ROM�e�ʐݒ�o�C�g����ROM�e�ʂ�\����������擾
//
// @arg [in]  ROM�e�ʐݒ�o�C�g
//
// @ret ������(�P�ʂ�)
//
System::String^ MasterEditorTWL::transRomSizeToString( System::Byte ^byte )
{
	System::UInt32  size;
	System::String  ^unit;

	// RomSize = 2^(byte) [Mbit] 
	size = 1 << *(byte);

	if(size >= 1024)
	{
		unit = gcnew System::String("Gbit");
		size = size / 1024;
	}
	else
	{
		unit = gcnew System::String("Mbit");
	}
	return (size.ToString() + unit);
}

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
System::String^ MasterEditorTWL::transRatingToString( System::Byte ogn, System::Boolean enable, System::Byte rating, System::Boolean english )
{
	cli::array< System::String^ > ^list;
	System::Int32  index;
	System::String ^str;

	if( ogn == OS_TWL_PCTL_OGN_CERO )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{ L"A (All ages)", L"B (aged 12 or older)", L"C (aged 15 or older)", 
			                                            L"D (aged 17 or older)", L"Z (aged 18 or older)", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"A (�S�N��)", L"B (12�Έȏ�)", L"C (15�Έȏ�)", L"D (17�Έȏ�)", L"Z (18�Έȏ�)", L"���R��"};
		}

		switch( rating )
		{
			case 0:
				if( enable )	// �S�N����R��������
					index = 0;
				else
					index = 5;
			break;
			case 12: index = 1; break;
			case 15: index = 2; break;
			case 17: index = 3; break;
			case 18: index = 4; break;
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_ESRB )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"EC (aged 3 or older)", L"E (aged 6 or older)", L"E10+ (aged 10 or older)", 
			                                           L"T (aged 13 or older)",	L"M (aged 17 or older)", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"EC (3�Έȏ�)", L"E (6�Έȏ�)", L"E10+ (10�Έȏ�)", L"T (13�Έȏ�)", L"M (17�Έȏ�)", L"���R��"};
		}

		switch( rating )
		{
			case 3:  index = 0; break;
			case 6:  index = 1; break;
			case 10: index = 2; break;
			case 13: index = 3; break;
			case 17: index = 4; break;
			case 0:  index = 5; break; // 0�̓f�t�H���g�l�Ȃ̂ŃG���[���b�Z�[�W���o���Ȃ�(�����I�ɖ��R������)
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_USK )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"All ages", L"aged 6 or older", L"aged 12 or older", L"aged 16 or older", 
			                                           L"Inadequent for young", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"�N����Ȃ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"���N�ɂ͕s�K��", L"���R��"};
		}

		switch( rating )
		{
			case 0:
				if( enable )
					index = 0;
				else
					index = 5;
			break;
			case 6:  index = 1; break;
			case 12: index = 2; break;
			case 16: index = 3; break;
			case 18: index = 4; break;
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_PEGI_GEN )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"aged 3 or older", L"aged 7 or older", L"aged 12 or older", L"aged 16 or older", 
			                                           L"aged 18 or older", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"3�Έȏ�", L"7�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"���R��"};
		}

		switch( rating )
		{
			case 3:  index = 0; break;
			case 7:  index = 1; break;
			case 12: index = 2; break;
			case 16: index = 3; break;
			case 18: index = 4; break;
			case 0:  index = 5; break;
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_PEGI_PRT )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"aged 4 or older", L"aged 6 or older", L"aged 12 or older", L"aged 16 or older", 
			                                           L"aged 18 or older", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"4�Έȏ�", L"6�Έȏ�", L"12�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"���R��"};
		}

		switch( rating )
		{
			case 4:  index = 0; break;
			case 6:  index = 1; break;
			case 12: index = 2; break;
			case 16: index = 3; break;
			case 18: index = 4; break;
			case 0:  index = 5; break;
			default:
				index = 5;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_PEGI_BBFC )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"aged 3 or older", L"aged 4 or older recommended", L"aged 7 or older", L"aged 8 or older recommended",
													   L"aged 12 or older", L"aged 15 or older", L"aged 16 or older", L"aged 18 or older", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"3�Έȏ�", L"4�Έȏ㐄��", L"7�Έȏ�", L"8�Έȏ㐄��", 
			                                           L"12�Έȏ�", L"15�Έȏ�", L"16�Έȏ�", L"18�Έȏ�", L"���R��"};
		}
		switch( rating )
		{
			case 3:  index = 0; break;
			case 4:  index = 1; break;
			case 7:  index = 2; break;
			case 8:  index = 3; break;
			case 12: index = 4; break;
			case 15: index = 5; break;
			case 16: index = 6; break;
			case 18: index = 7; break;
			case 0:  index = 8; break;
			default:
				index = 8;
			break;
		}
		str = list[ index ];
	}

	else if( ogn == OS_TWL_PCTL_OGN_OFLC )
	{
		if( english )
		{
			list = gcnew cli::array< System::String^ >{L"G", L"PG", L"M", L"MA15+", L"Unexamined"};
		}
		else
		{
			list = gcnew cli::array< System::String^ >{L"G", L"PG", L"M", L"MA15+", L"���R��"};
		}

		switch( rating )
		{
			case 0:
				if( enable )
					index = 0;
				else
					index = 4;
			break;
			case 7:  index = 1; break;
			case 14: index = 2; break;
			case 15: index = 3; break;
			default:
				index = 4; break;
			break;
		}
		str = list[ index ];
	}

	else
	{
		str = nullptr;
	}

	return str;
}
