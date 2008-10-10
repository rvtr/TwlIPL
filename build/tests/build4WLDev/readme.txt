* 無線開発用SystemUpdater

1. 各種systemMenuファイルの用意

TwlIPL/build/tests/build4WLDev の直下に以下のディレクトリ構成で
systemMenu関連ファイルを用意してください。(HNA*.tadは除いてください。)

※Regionは、JapanとAmericaのみをサポート、開発実機には対応していません。

data_sysmenu
  +debugger
  l    +america/*
  l    +japan/*

上記の*には、
HNA*.tad以外の全tadとnandファームとフォントデータを置いてください。


2.SystemUpdaterのビルド

TwlIPL/build/tests/build4WLDev でmakeを実行してください。
※TwlIPL_privateが必要です。

最終的に、以下のような日付時間タグつきのSystemUpdaterが同じディレクトリに作成されます。

SystemUpdater_WL_yymmdd_hhmm.srl
