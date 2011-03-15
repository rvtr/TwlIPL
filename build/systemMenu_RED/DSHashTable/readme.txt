========================
ホワイトリストの作成方法
========================

●概要

既存の .srl を特定フォルダ以下に格納しておき、それに対してツールを使うことで
各種 .bin ファイルを生成します。

本フォルダでは、準備済みの .bin ファイルと、バージョン識別用の revision.bin を
すべて連結したのち、 .tad に変換しているだけです。

.bin ファイルの更新時に、このフォルダに配置、コミット後に全体を(上書き)
チェックアウトし直してからビルドしてください。

原則、ホワイトリストは trunk のみで管理します。

ブランチでビルドしたい場合は、trunkを更新したのち、該当ブランチにマージして
ご利用ください。


●変換ツール

TwlIPL_private以下にソースコードがありますので、ビルドしてお使いください。

・MakeDSHashTable.exe [.srlフォルダ]

  旧マスタリングそのものであり、ヘッダおよびARM9/ARM7スタティック領域、
  およびオーバーレイのある程度を対象としたデータベースを作成します。

  指定フォルダ以下を再帰的に走査し、DSHashTable.bin を作成します。
  旧マスタリングすらされていない .srl が対象です。


・MakeDSHashTableEx.exe [.srlフォルダ]

  新マスタリングで追加されたバナー領域を対象としたデータベースを作成します。

  指定フォルダ以下を再帰的に走査し、DSHashTableEx.bin を作成します。
  新マスタリングされていない .srl が対象です。


・MakeDSHashTableAdHoc.exe [.srlフォルダ]

  タイトル別に範囲を指定したハッシュ値を格納したデータベースを作成します。

  同一フォルダに存在している、 MakeDSHashTableAdHoc.ini を元に、
  必要な .srl を指定フォルダから探し出して DSHashTableAdHoc.bin を
  作成します。

  MakeDSHashTableAdHoc.ini は、現在以下の場所で管理しています。
  (No.1のUSBを参照してください)

    $TwlIPL_private/build/tools/MakeDSHashTableAdHoc

  ※ 面倒ならTwlIPLに移しても構いません。


●MakeDSHashTableAdHoc.iniの書式

Windows INIファイルの書式で、設定できる項目は次の通りです。
なお、数値は、十進数でも十六進数でも構いません。(0xが付くと十六進数と判断される)

(例)
  ; セミコロンの後ろはコメントです
  [Tak The Greet Juju Challenge]; 他のものとダブらなければ日本語でも何でもOKです
  game_code	= A3TE		; イニシャルコードです
  rom_version	= 0		; リマスターバージョンです

  ; 空行追加もお好きにどうぞ

  offset0	= 0x0013a400	; ハッシュ対象オフセット (0x200の倍数) No.0
  length0	= 0x200		; ハッシュ対象サイズ (0x200の倍数) No.0

  ; 以下は省略可 (offsetとlengthは必ずセットで、通し番号は欠番無いように)
  offset1	= 0x00141a00	; ハッシュ対象オフセット (0x200の倍数) No.1
  length1	= 0x200		; ハッシュ対象サイズ (0x200の倍数) No.1
  offset2	= 0x00145800	; ハッシュ対象オフセット (0x200の倍数) No.2
  length2	= 0x200		; ハッシュ対象サイズ (0x200の倍数) No.2
  offset3	= 0x01a5ac00	; ハッシュ対象オフセット (0x200の倍数) No.3
  length3	= 0x200		; ハッシュ対象サイズ (0x200の倍数) No.3
  offset4	= 0x01a5d400	; ハッシュ対象オフセット (0x200の倍数) No.4
  length4	= 0x5600	; ハッシュ対象サイズ (0x200の倍数) No.4
  offset5	= 0x0005c400	; ハッシュ対象オフセット (0x200の倍数) No.5
  length5	= 0x200		; ハッシュ対象サイズ (0x200の倍数) No.5
  offset6	= 0x00142000	; ハッシュ対象オフセット (0x200の倍数) No.6
  length6	= 0x200		; ハッシュ対象サイズ (0x200の倍数) No.6
  offset7	= 0x00126400	; ハッシュ対象オフセット (0x200の倍数) No.7
  length7	= 0x5C00	; ハッシュ対象サイズ (0x200の倍数) No.7
  ; ～7が最大です

●TWLの更新時の注意

commondefs.DSHashTable内の DS_HASH_TABLE_MAJOR_VERSION または
DS_HASH_TABLE_MINOR_VERSION を更新するのを忘れないでください。


●CTR対応

作成された HNHA-XXXX-YYYY.bin を以下のフォルダにコピーしてください。

  $Horizon/resources/shareddata/twl/DSHashTable/

ファイル名が変更された場合は、以下のファイルの内容を修正してください。
(DSHashTableAdHoc.bin のみの修正では変化しないはず)

  $Horizon/resources/shareddata/twl/OMakefile
  $Horizon/sources/firmware/CTR-Kernel/updater1st/UpdaterContents/Contents/ProgramPathes.om

以上
