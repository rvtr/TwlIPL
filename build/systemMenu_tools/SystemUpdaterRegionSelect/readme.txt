
/*-----------------------------------------------------*/
/*                   対応SDKについて                   */
/*-----------------------------------------------------*/

**** SDK側は UIGブランチを使用してください ****


/*-----------------------------------------------------*/
/*           データフォルダの構成について              */
/*-----------------------------------------------------*/

以下のディレクトリ構成でsystemMenuを用意して
ARM9/Makefileの UPDATER_HOST_ROOT_DIR変数で指定してください。

xxxxx
  +common
  l    +$
  l    +america/*
  l    +australia/*
  l    +china/*
  l    +europe/*
  l    +japan/*
  l    +korea/*
  l
  +debugger
  l    +america/#
  l    +australia/#
  l    +china/#
  l    +europe/#
  l    +japan/#
  l    +korea/#
  l
  +standalone
       +america/#
       +australia/#
       +china/#
       +europe/#
       +japan/#
       +korea/#


配置図:
 # -> ランチャーと本体設定のtad (開発機用とデバッガ用で内容が異なるもの）
 * -> 上記以外でALLリージョンでないtad
 $ -> 上記以外のtad 及び nandファーム 及び フォントデータ


/*-----------------------------------------------------*/
/*           コンパイルスイッチについて                */
/*-----------------------------------------------------*/

 +-----------------------+--------+--------------------+----------------------------------------+
 l                       l  通常  l Lot Check Group 用 l                  説明                  l
 +-----------------------+--------+--------------------+----------------------------------------+
 l IGNORE_VERSION_CHECK  l  FALSE l        TRUE        l TRUEならバージョンダウン可能           l
 +-----------------------+--------+--------------------+----------------------------------------+
 l SKIP_WRITE_HWINFO     l  FALSE l        TRUE        l TRUEならハードウェア情報を更新しない   l
 +-----------------------+--------+--------------------+----------------------------------------+
 l USE_NORMAL_COMMON_KEY l  FALSE l        TRUE        l TRUEなら通常のcommon client key を使用 l
 +-----------------------+--------+--------------------+----------------------------------------+

※ [通常]は開発者へリリースする目的のもの
※ [Lot Check Group 用]はSystemMenuのロットチェックでネットワークアップデートのテストを
   行うために一旦SystemMenuを古いものに入れ替えるためのもの

 +-----------------------+----------------------------------------------------------------------+
 l                       l                            説明                                      l
 +-----------------------+----------------------------------------------------------------------+
 l JP_REGION_ONLY        l  書き込みリージョンの選択肢をJPに限定する場合はTRUEを選択します      l
 l                       l  この際、japanでないリージョン以下のデータは不要です
 +-----------------------+----------------------------------------------------------------------+

/*-----------------------------------------------------*/
/*                フォントデータついて                 */
/*-----------------------------------------------------*/

中国フォントのファイル名は     ***_CN_***.dat  としてください。
韓国フォントのファイル名は     ***_KR_***.dat  としてください。
日米欧豪フォントのファイル名には、 "_CN_" 及び "_KR_" を含めないでください。