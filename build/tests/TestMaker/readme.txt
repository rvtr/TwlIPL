#----------------------------------------------------------------------------
# [CLS テスト用] ROM 作成
#
# -- CLS テスト 用 ROM を作成します。
#    このドキュメントはほぼ覚書です。
#
#----------------------------------------------------------------------------

* 注意
- Python 2.5 の実行環境が必須です。すいません。
- PyYAML のインストールが必須です。すいません。
- pyExcelerator のインストールが一部必要です。すいません。
-- http://pyyaml.org/ を参照してください。

* 利用方法

- CLS テスト ROM 作成には make clstest -> make してください。
- FONT テスト ROM 作成には make fonttest -> make してください。
- PARENTAL テスト ROM 作成には make parentaltest -> make してください。
- EULA テスト ROM 作成には make eulatest -> make してください。
- 上記 make 作業後に、make forcls と打つと、~
  CLS 操作用ディレクトリ (forCLS) が生成されます。

* ROM 生成パラメータ
- romparam.yaml にパラメータが指定されています。
- CLS テスト用パラメータに関しては、
  ./docs/twl_cls_checksheet_20080508.xls を参照してください。
- FONT テスト用パラメータに関しては、
  ./docs/20080617_DS_fontcodeList_NOE_FIANL.xls を参照してください。
- PARENTAL テスト用パラメータに関しては、
  ./docs/twl_parentalcontrol_spec_20080704.xls を参照してください。
- EULA テスト用パラメータに関しては、
  今後検証内容の詳細をつめる必要があります。

* CLS テスト生成物
  HZ0A : System/NAND/セキュア      --> ShopApp / Menu
  HZ4A : System/NAND/データ        --> 写真帳
  HZ5A : System/非表示/NAND/データ --> 無線 Firm
  KZ2A : User/NAND                 --> Shop 販売 App