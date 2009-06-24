このパッケージについて
======================

ARM7コンポーネントを、racoonからjackalにすると、カメラ動作時の
ARM9の処理落ちが多くなることを調べるためのサンプルです。

コンポーネントの切り替えは、Makefileの

ifdef TWL_IPL_RED_ROOT
#コンポーネントを切り替えます。
#コメントアウトするとracoonを、有効化するとjackalを使います。
#下の行を変更したらmake cleanしてください。
TWL_IPL_USE_RED_IPL			:= TRUE
endif

上記の行をコメントアウトするか有効化するかで行ってください。
変更したらmake cleanしてください。

処理落ちが発生すると、デバッグ出力に

mConvBusy! 0, nextp empty
mConvBusy! 1, nextp empty
  :

と表示されます。数字は回数です。この頻度が、racoonとjackalで異な
ります。

画面上部の '*' の動きのスムーズさや、カメラに向かって手を振ったり
すると、カクカクすることでも処理落ちにお気づき頂けると思います。

以上です。



以下は、このパッケージに含まれる関数をSDKのDSP JPEGライブラリに追
加して頂くために提供したときに作成したドキュメントです。

↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
======================================================================

このパッケージについて
======================

UIGランチャーのカメラ撮影部分(カメラマネージャ)の実装のため、いろ
いろ実験するためのパッケージです。直接カメラやJPEGには関係しない
ファイルが含まれていますがご了承ください。

src/menuCameraManager.cppは、基本的に現状のUIGランチャーと同じも
のです。ここでの実験の成果をUIGランチャーへ反映しています。

YUV422->YUV420変換関数以外は、未完成の部分もあり、雑に作っている
部分もありますので、公開なさらないようにお願いします。

SDカードアクセス部は、UIGランチャー正式仕様ではありません。ただし
現状のUIGランチャーにもこのまま使われています。シャッター音もまだ
鳴りません。

実行方法は、普通にmakeしてmain.tlfをロードして頂くと、上画面にア
ウトカメラの画像が出ます。Aボタンでシャッターを切ります。SDカード
が入っていれば/menu/menu####.jpgを連番で生成します。(SDKのデモを
そのまま頂いています)


YUV422->YUV420変換関数の使用方法
================================

menuCameraManager.cppのConvert8bitYuv422To16bitYuv420()で、
YUV422->YUV420変換を行います。この関数を呼び出している同ファイル
の1074行目から1102行目あたり

  frame_y  = mpTestEncodeWork->y;
  frame_cb = mpTestEncodeWork->cbcr;
  frame_cr = frame_cb + (mShotWdt * mShotHgt / 4);
  tick = OS_GetTick();
  Convert8bitYuv422To16bitYuv420(reinterpret_cast<const u32*>(pBuf),
                                 frame_y,
                                 mShotWdt,
                                 mShotHgt,
                                 mpDtcmWork->mYuvBuf);
  // store YUV420 and invalidate encode result
  // DC_StoreAll(), DC_FlushAll() is NG?
  DC_FlushAll();

  IPL_PRINT("procShot: yuv422to420 time %d usec\n", (u32)OS_TicksToMicroSeconds(OS_GetTick() - tick));
  tick = OS_GetTick();

  // 出力サイズはいかほど必要？
  DSP_SetJpegEncoderBuffer(mpTestEncodeWork->result, sizeof(mpTestEncodeWork->result));
  // explicit initialization
  DSP_SetJpegEncoderThumbnailBuffer(NULL, NULL, NULL);
  DSP_SetJpegEncoderAllocator( dspAlloc_, dspFree_ );
  u32 size = DSP_StartJpegEncoder( static_cast< s16 > ( mShotWdt ) ,
                                   static_cast< s16 > ( mShotHgt ) ,
                                   DSP_JPEG_THUMBNAIL ,
                                   4 );

  // need here too ? sometimes picture breaks...
  // DC_StoreAll(), DC_FlushAll() is NG?
  DC_InvalidateRange(mpTestEncodeWork->result, size);

について説明します。

- 1078行目の変換関数の引数pBufは、カメラがVGA、YUV422でキャプチャ
した結果を指しています。このバッファは、直前にprocConvert()を通っ
てきており、917行目の
        DC_FlushAll();
で、データキャッシュのInvalidateが済んでいます。
変換元データはワード(4バイト)リードされるため、
reinterpret_cast<const u32*>しています。

- 引数frame_yは、SDKのJPEGエンコーダのグローバル変数です。今は11
行目から直接
extern "C" {
// build/libraries/dsp/ARM9.TWL/src/dsp_jpeg_enc.c
extern u16 *frame_y; 
extern u16 *frame_cb; 
extern u16 *frame_cr;
} /* extern "C" */
で参照していますが、SDKのグローバル変数名としては適切ではないため、
変数名を整えてSet関数を追加して頂いた方が良いと思います。

- SDKのJPEGエンコーダには、frame_cbとframe_crも与える必要がありま
す。frame_cbとframe_crを与えているのは1074行目からの
        frame_y  = mpTestEncodeWork->y;
        frame_cb = mpTestEncodeWork->cbcr;
        frame_cr = frame_cb + (mShotWdt * mShotHgt / 4);
になります。

変換関数は、frame_yに隙間無く続けてframe_cb、frame_crを出力します
ので、変換関数の引数はframe_yだけです。

- mShotWdtは640、mShotHgtは480です。

- mpDtcmWork->mYuvBufで、処理高速化用のDTCMワークを与えています。
menuCameraManager.hの43行目にある、12ダブルワード(96バイト)
    u64 mYuvBuf[12];
の領域が必要です。strd命令を使用しているので8バイトアラインメント
してください。(その上のmShrinkBufは、縮小用なので変換関数には関係
ありません)

DTCMを使うことについては、SDKとして公開する場合に検討が必要だと思
います。メインメモリやVRAMを割り当てても動作するはずですが、試し
ていません。staticなDTCMワークでなくても、メインスレッドのスタッ
クフレームなら通常はDTCMになるはずですが、そういうことを意識させ
ると敷居が高くなります。

- 変換後の1085行目
        DC_FlushAll();
は、JPEGエンコード時に画像が崩れる症状に悩まされ、念のためデータ
キャッシュの内容を実メモリへ吐き出しました。画像が崩れなくなれば、
不要かもしれません。

- 1094行目の
        DSP_SetJpegEncoderAllocator( dspAlloc_, dspFree_ );
で指定しているアロケータですが、実体はmainy.cppの1667行目にありま
す。

dspAlloc_()は、実は何もしていません。何もしていないどころか、呼ば
れたらアサートで停止させます。

dspFree_()は、SDKから呼ばれてしまうので、停止はしませんがダミー関
数にしてあります。

このアロケータの使われ方(SDKのdsp_jpeg_enc.c)については、いろいろ
思うところはあるのですが、簡単に言うと

a. 最大で何バイト必要になるのか、できればコンパイル時に知りたいが
その方法が提供されていない

b. pTmp2を最初に確保しているため、pYbuf、pCBbuf、pCRbufを確保した
後に
    DSPFree(pTmp2);
するのはフラグメントを生じる可能性がある

c. extra_bytesに関する処理や
    Yuv_image_size=(u32)(image_width+32)*(image_height+32);
の+32が意図不明

というところで不安になったのが、私が変換関数を自作したもう一つの
理由です。

- JPEGエンコード後の1102行目
        DC_InvalidateRange(mpTestEncodeWork->result, size);
も、JPEGエンコード時の画像崩れ対策テストです。
もしかして、SDへ書き込むFSライブラリに問題があるかもと思い、FSラ
イブラリ用にデータキャッシュをInvalidateしたものです。崩れなくな
れば、これも不要になるでしょう。


YUV422->YUV420変換関数の補足説明
================================

- 793行目から821行目まで、
#if 0
    // even line U
  :
#else
    // even column ave. uv (V0 U0)
  :
#endif

ですが、これはYUV422->YUV420に伴って失われるU、V成分を2ラインの
平均値とするか、Wikipediaを鵜呑みした

| YUV420 : 2×2ピクセルのうち、青の色差信号を最初のラインから取り、
| 赤の色差信号を次のラインからとる方式。輝度信号は1ピクセルごとにと
| る。 

の方式にするかを試したものです。平均値にした方がノイズが低減され
ているように感じたため、今の#if 0としてあります。#if 1にするとシ
ャープな感じになります。

しかし、そもそもY成分はそのままでU、Vだけ平均化したらおかしい絵に
ならないのか、とはいえ上のラインのVを下のラインから持ってくるのも
おかしな話では？という思いはあります。

JPEGエンコーダの入力データ仕様を詳しく調べていないので、どちらが
良いのか分かっていません。

- 変換関数の引数の制限について、714行目からのコメントを補足します。

// pYCbCrBufも最低4バイトアラインメントですが、各デバイスとの連携に
// 備えて32バイトアラインメントしてください。

↑これは、変換結果がDSPの入力データになるので、32バイトアラインメ
ントしておけばキャッシュまわりの心配が減るからです。

