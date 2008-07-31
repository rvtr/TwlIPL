NCFGの修復仕様まとめ


接続設定のCRCをチェックし、エラーがある場合はその設定を初期化します。
（IPアドレスの設定や、接続タイプ等が全てクリアされる状態）

Nitroのときのように、接続設定１と２を異常にすると、３も道連れになりクリアされるというようなことはありません。


より、詳細な仕様は影舞に記されています。
http://phoenix.boy.nintendo.co.jp/~twl-wifi/kagemai/html/user.cgi?project=twl-wifi&action=view_report&id=84
http://phoenix.boy.nintendo.co.jp/~twl-wifi/kagemai/html/user.cgi?project=twl-wifi&action=view_report&id=83

一部抜粋
- CRC エラーの接続先は、個別に消去します。
- 全領域 CRC エラーだった場合は、初回起動だと見なし、消去のみを行い、
  エラーを返しません。
- 設定がされていた領域で消去が発生した場合は、エラーを通知します。
- それ以外の場合は、エラーを返しません。
  （元々設定されていなかった領域で消去が発生した場合も含む）

