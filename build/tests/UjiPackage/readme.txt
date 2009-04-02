製品技術部検査プログラム用TwlIPLパッケージ作成Makefile

■必要なTwlSDKパッケージ

 TWLSDK-5_1-20090303-jp.zip 
 TWLSDK-5_1-20090303-private-jp.zip 
 TWLSDK-5_1-20090303-private-Secure7-jp.zip
 TWLSDK-5_1-20090303-private-SysMenu-jp.zip
 TwlSDK-5_1-20090303-private-TwlWireless-jp.zip


■作り方
1. 各種変数を設定

$ export TWLSDK_PLATFORM=TWL
$ export TWLSYSTEM_ROOT=(TwlSystemのインストールディレクトリ)
※TWLSYSTEMは使用しませんが、TwlIPLのcommondefsがきちんと解釈されないため、
　設定してください。

2. packageターゲットを指定してmake

$ make package

TwlIPL-uji-(日付)-(時間).zip がこのディレクトリに生成されます。
