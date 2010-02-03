【card_copy操作手順】

1. TWL 開発機へ NMenu または NandInitializer で card_copy_*.tad をインポート（インポート済みなら不要）
2. TWL メニューから「カードコピーマルチブート」 NAND アプリを起動
3. NTR へ吸い出したい DS カードを差す
4. NTR の DS ダウンロードメニューを起動
5. NTR の DS ダウンロードメニューへ "CardCopyMultiBoot" リストが出たら選択
6. DS カード→無線→ SD カードの転送が開始する
7.「Sent/Received ROM size=....」という表示が出たら、SD カードの card_dump.sbin へ
   ゲーム領域の吸出しが完了

【TwlSDK変更箇所】

build/libraries/card/common/src/card_common.c

CARDAccessLevel CARDi_GetAccessLevel(void)
{
        .
        .
    else if (!OS_IsRunOnTwl())
    {
        level = CARD_ACCESS_LEVEL_FULL; // CARD_ACCESS_LEVEL_BACKUP から変更
    }
        .
        .
    return level;
}
