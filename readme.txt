1.ビルドに必要なもの
　NORファーム、NANDファーム
	・(TwlIPL_private)
	・TwlSDK
　システムメニュー
　　・(TwlIPL_private)
　　・TwlSDK
　　・NitroSystem 070314版(本ツリーのadd-ins内のTwlSDK対応パッチを当てたもの）

　TwlIPL_privateは一部の鍵を使用する場合にのみ必要


2.環境変数の設定
　TWL_IPL_RED_ROOT
	ex.) TWL_IPL_RED_ROOT=c:/TwlIPL
  TWL_IPL_RED_PRIVATE_ROOT（TwlIPL_private を利用する場合のみ）
	ex.) TWL_IPL_RED_PRIVATE_ROOT=c:/TwlIPL_private


3.開発実機向けのビルド例

    ① TwlIPLチェックアウト
       file://aqua/svn/TwlIPL/branches/20100611_RedLauncher_Remodel
       ビルド確認：r2923

    ② TwlSDK(UIG_plus)チェックアウト
        http://10.114.1.206:80/~twl-dev/svn/TwlSDK/branches/branch-5_0rcplus_UIG_plus
       ビルド確認: r10813

    ③ NItroSystem
       ①でチェックアウトしたTwlIPL\add-ins\NitroSystemを使用
        NitroSystem_forTWL-080408-jp.zip を解凍したものを、
        NitroSystem_forTWL-080408-SDK5_0FCplus5-patch-jp.zip を解凍したもので上書き

       フォルダ構成
       . ┬ TwlSDK
         ├ NitroSystem
         └ TwlIPL

    ④ 必ず、TwlSDKをビルドする前にTwlIPLの環境変数設定
       cd TwlIPL
       source ./setup
       ＃ TWL_IPL_RED_ROOTが定義されていないと、
       ＃ lcfgの署名の算出が実行されないTwlSDKになってしまう。

    ⑤ TwlSDKをビルド（同じCygwin上で続ける）
       ビルド確認：
         CWFOLDER_TWL C:\Program Files\Freescale\CW for NINTENDO DS V2.0

       cd ../TwlSDK
       source ./setup
       make TWLSDK_PLATFORM=TWL

    ⑥ ライブラリのマージ
       TwlSDK\add-ins\es\es-sdk-20080822\twl\lib\ARM9-TS.LTD.thumb\Release 以下の
       ファイルを TwlSDK\lib\ARM9-TS\Release にコピーする

    ⑦ NitroSystemをビルド（同じCygwin上で続ける）
       cd ../NitroSystem
       export NITROSYSTEM_ROOT="NitoroSystemのフォルダ"
       export TWLSYSTEM_ROOT="NitoroSystemのフォルダ"
       make TWLSDK_PLATFORM=TWL

    ⑧ 指紋認証ロッカーからTwlIPL_privateが保存されたUSBメモリを入手
       USBメモリのドライブ名を合わせて、同じCygwin上で、
       export TWL_IPL_RED_PRIVATE_ROOT=F:/TwlIPL_private

    ⑨ TwlIPLをビルド（同じCygwin上で続ける）
       cd ../TwlIPL
       make -f Makefile.full TWLSDK_PLATFORM=TWL 

    備考：
    ・過去にビルドした環境を使用する場合は、最初にmake cleanが必要


以上
