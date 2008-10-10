@echo off

rem
rem mastering & comparing sample batch file
rem
rem    Simple usage: SRLファイルをドラッグすると%output_path%にマスタリング
rem                  結果を出力し、正しくできたか検証を行う
rem
rem    要確認変数: mastering_tool, comparing_tool, output_path
rem    補足: %~d0%~p0はバッチファイルのパスです
rem    補足: %~d1%~p1は各入力ファイルのパスです
rem

rem 引数チェック
set usage=USAGE: %~nx0 INPUT_FILE...
if "%1" equ "" (
	echo %usage%
	goto end
)

rem パス設定
set mastering_tool=%~d0%~p0mastering.TWL.exe
set mastering_ini=%~d0%~p0mastering.TWL.ini

rem ループ開始 (%0を使うのはここまで、%1を使うのはここから、%2以降は使わない)
:begin

rem 出力パス設定
rem (ここでは入力が<path>\<file.srl>なら出力は<path>\m\mastered_<file.srl>という例)
set output_path=%~d1%~p1
set output_srl=%output_path%\%~n1.master%~x1
set output_pro=%output_path%\%~n1.master.property

rem その他の変数
set input_srl=%1

rem プログラム確認
if not exist "%mastering_tool%" (
	echo mastering tool "%mastering_tool%" is not found.
	goto end
)

rem 入力ファイル確認
if not exist %input_srl% (
	echo %input_srl% is not found.
	goto end
)


rem マスタリング処理
echo.
echo %~nx1をマスタリングします。
echo.
%mastering_tool% %input_srl% %output_srl% -i %mastering_ini% -p %output_pro%

if not %ERRORLEVEL% == 0 goto end

rem 入力ファイルが残っているならbeginに戻る
shift
if "%1" neq "" goto begin

rem 終了 (キー入力待ち)
:end
echo.
echo すべてのマスタリングが終了しました。
pause
