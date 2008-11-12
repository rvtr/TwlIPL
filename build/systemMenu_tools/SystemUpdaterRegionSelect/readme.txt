
**** SDK側は UIGブランチを使用してください ****


以下のディレクトリ構成でsystemMenuを用意して
ARM9/Makefileの UPDATER_HOST_ROOT_DIR変数で指定してください。

xxxxx
  +common
  l    +$
  l    +america/*
  l    +australia/*
  l    +europe/*
  l    +japan/*
  +debugger
  l    +america/#
  l    +australia/#
  l    +europe/#
  l    +japan/#
  l
  +standalone
       +america/#
       +australia/#
       +europe/#
       +japan/#


配置図:
 # -> ランチャーと本体設定のtad (開発機用とデバッガ用で内容が異なるもの）
 * -> 上記以外でALLリージョンでないtad
 $ -> 上記以外のtad 及び nandファーム 及び フォントデータ

