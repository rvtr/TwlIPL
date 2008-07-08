rem
rem  ***** SD起動ファイル作成ツール *****
rem
rem sdmc_launcher_writer.gcd が起動ファイルとして menu.srl しか受け付けないので
rem 引数として与えたファイルを menu.srl としてコピーする
rem 

if "%1" equ "" (
	echo 失敗しました。
    echo ディレクトリごとドラッグアンドドロップしてください。
	echo:
	goto end
)

set progdir=%~dp0

del  %progdir%\menu.srl
copy %1 %progdir%\menu.srl
