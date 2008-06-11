■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■                                                                          ■
■  TWLWDSライブラリ                                           2008/6/11版  ■
■                                                                          ■
■  ネットワーク開発部からのお知らせ                                        ■
■                                                                          ■
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

●動作確認バージョン
・SDK
(TwlSDK-5_00fc-080325-1027-branch-5_0fc-TS400_jp)
(TwlSDK-5_00fc-patch-plus4-080609_jp)
(TwlSDK-5_00fcplus2-080428-1417-private-crypto-branch-5_0fcplus2_jp)
・コンパイラ
(CodeWarrior v2.0)
(20080215_cw_ds_2.0_sp2_Buildtools_Patch2.jpSP1)
・デバッグ環境
(IS-TWL-DEBUGGER Version 0.63.0805.2900)

●ビルドする際の注意事項
・ライブラリのビルドは
  %make TWLSDK_PLATFORM=TWL
・受信サンプルのビルドはWDSTestディレクトリ下で
  %make TWLSDK_PLATFORM=TWL

●変更履歴

2008/06/11 初版
・動作確認用バージョンから、ビーコンに以下の情報が追加されました
  infoflag - 付加情報フラグが格納される領域です
  mtu      - そのAPが使用可能な最大のMTUを格納されます
・ビーコン受信時にビット誤りが発生した場合には、そのビーコンを無視する
  ようになりました。

2008/02/21 動作確認用バージョン
・動作確認用として開発技術部にリリース

●使用時の注意
infoflagの扱い
・ランチャーメニューでのニンテンドースポットビューアの強調表示は、
  infoflagのWDS_INFOFLAG_NOTIFYビットが1にセットされたビーコンを
  受信した場合のみ行ってください。
