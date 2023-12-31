■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■                                                                          ■
■  TWLWDSライブラリ                                           2008/8/16版  ■
■                                                                          ■
■  ネットワーク開発部からのお知らせ                                        ■
■                                                                          ■
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

●動作確認バージョン
・SDK
(TwlSDK-5_00rc1-patch-plus-080805)
(TwlNADK-080725)
・コンパイラ
(CodeWarrior v2.0)
(20080215_cw_ds_2.0_sp2_Buildtools_Patch2.jpSP1)
・デバッグ環境
(IS-TWL-DEBUGGER Version 0.70.0807.2400)

●ビルドする際の注意事項
・ライブラリのビルドは
  %make TWLSDK_PLATFORM=TWL
・受信サンプルのビルドはWDSTestディレクトリ下で
  %make TWLSDK_PLATFORM=TWL
・WDSラッパーを使用した受信サンプルのビルドはWDSWrapperTestディレクトリ下で
  %make TWLSDK_PLATFORM=TWL

●変更履歴
2008/09/04 20080904版
・WDS_WrapperStopScan関数を呼び出した直後にWDS_WrapperCleanup関数を呼び出すと、
WDS_WrapperStopScanに関するコールバックが呼び出されず、WDS_WrapperCleanupに関する
コールバックのみが呼び出される問題を修正しました。

2008/08/16 20080816版
・WDSWrapperがビーコンの間欠受信を停止している状態、あるいは間欠受信中でも
次の受信開始を待っている状態のいずれかでWDS_WrapperStopScan関数を使用した場合、
間欠受信停止のコールバックが呼び出されない問題を修正しました。

2008/08/06 20080806版
・WDS_WrapperStopScan関数を使用してビーコン受信を中断した場合、
中断の直前に呼び出したWDS_WrapperCheckValidBeacon関数ではビーコンが
受信されていると通知されるにもかかわらず、中断後にはWDSWrapperライブラリの
状態が、ビーコンが受信されていない状態になる場合があるバグを修正しました。

2008/07/14 20080714版
・WDSWrapperTestサンプルにスリープモードに関連する処理を追加しました。

2008/07/03 20080703版
・WDSラッパー内部で行っていた排他制御に関連して発生していたフリーズ問題を修正しました。

2008/06/26 20080626版
・WDSライブラリを簡易に使用するためのWDSWrapperが追加されました。

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
・なお、WDSWrapperはWDS_INFOFLAG_NOTIFYに関連した処理を内部で自動的に行います。
WDS_WrapperCheckValidBeacon関数について
・20080703バージョン以降は、WDS_WrapperCheckValidBeacon関数は初期化時に指定した
　コールバック関数の中からのみ呼び出すことができます。
