rem
rem  ***** SD�N���t�@�C���쐬�c�[�� *****
rem
rem sdmc_launcher_writer.gcd ���N���t�@�C���Ƃ��� menu.srl �����󂯕t���Ȃ��̂�
rem �����Ƃ��ė^�����t�@�C���� menu.srl �Ƃ��ăR�s�[����
rem 

if "%1" equ "" (
	echo ���s���܂����B
    echo �f�B���N�g�����ƃh���b�O�A���h�h���b�v���Ă��������B
	echo:
	goto end
)

set progdir=%~dp0

del  %progdir%\menu.srl
copy %1 %progdir%\menu.srl
