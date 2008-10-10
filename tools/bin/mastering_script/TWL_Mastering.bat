@echo off

rem
rem mastering & comparing sample batch file
rem
rem    Simple usage: SRL�t�@�C�����h���b�O�����%output_path%�Ƀ}�X�^�����O
rem                  ���ʂ��o�͂��A�������ł��������؂��s��
rem
rem    �v�m�F�ϐ�: mastering_tool, comparing_tool, output_path
rem    �⑫: %~d0%~p0�̓o�b�`�t�@�C���̃p�X�ł�
rem    �⑫: %~d1%~p1�͊e���̓t�@�C���̃p�X�ł�
rem

rem �����`�F�b�N
set usage=USAGE: %~nx0 INPUT_FILE...
if "%1" equ "" (
	echo %usage%
	goto end
)

rem �p�X�ݒ�
set mastering_tool=%~d0%~p0mastering.TWL.exe
set mastering_ini=%~d0%~p0mastering.TWL.ini

rem ���[�v�J�n (%0���g���̂͂����܂ŁA%1���g���̂͂�������A%2�ȍ~�͎g��Ȃ�)
:begin

rem �o�̓p�X�ݒ�
rem (�����ł͓��͂�<path>\<file.srl>�Ȃ�o�͂�<path>\m\mastered_<file.srl>�Ƃ�����)
set output_path=%~d1%~p1
set output_srl=%output_path%\%~n1.master%~x1
set output_pro=%output_path%\%~n1.master.property

rem ���̑��̕ϐ�
set input_srl=%1

rem �v���O�����m�F
if not exist "%mastering_tool%" (
	echo mastering tool "%mastering_tool%" is not found.
	goto end
)

rem ���̓t�@�C���m�F
if not exist %input_srl% (
	echo %input_srl% is not found.
	goto end
)


rem �}�X�^�����O����
echo.
echo %~nx1���}�X�^�����O���܂��B
echo.
%mastering_tool% %input_srl% %output_srl% -i %mastering_ini% -p %output_pro%

if not %ERRORLEVEL% == 0 goto end

rem ���̓t�@�C�����c���Ă���Ȃ�begin�ɖ߂�
shift
if "%1" neq "" goto begin

rem �I�� (�L�[���͑҂�)
:end
echo.
echo ���ׂẴ}�X�^�����O���I�����܂����B
pause
