■ SDLOG対応版ランチャーについて

実機等でのでデバッグ用に、SDカードにデバッグプリントを出力するランチャーです。

TwlIPL/build/tests/Launcher_sdlogに移動してビルドすると、
同じディレクトリにHNAA.tadが作成されますので、NandInitializer等でImportして下さい。

●ARM9のデバッグプリントのみを出力

$ make SDLOG=ARM9

●ARM9/ARM7のデバッグプリントを出力

$ make SDLOG=ALL


■ SDLOG(改造版)ライブラリ超適当リファレンス

// SDLOGシステムの開始。開始以降、OS_Printf系を乗っ取り、SDログバッファに蓄積→SDカード書き出しを行います。
//
// path:           出力するファイルを置くSDカード上のディレクトリ(例:sdmc:/log)
// filename:       出力するファイル名
// buffer:         ログの一時書込先。大量のログを残す場合は 1KB 以上を推奨
// partitionSize:  SD カードに書き込むサイズ。 bufferSize の 1/n を推奨
// writeType:      上書き or 追記の指定。ファイルが存在しない場合はどちらも新規作成となる

BOOL SDLOG_InitEx(const char* path, const char* filename, char* buffer, u16 bufferSize, u16 partitionSize, SDLOGWriteType writeType);

// OS_Printf系を乗っ取らない版
// path:           保存先。 sdmc:/sample と指定すると sdmc:/sample/Log0.log に
//                 ログが保存される(ファイル名は固定です。)
BOOL SDLOG_Init(const char* path, char* buffer, u16 bufferSize, u16 partitionSize, SDLOGWriteType writeType);


// SDLOGにデバッグ情報を書き出す。
void SDLOG_Printf(const char *fmt, ...);

// バッファに貯められたログを強制的に SD カードに書き込む
// ここでは確実にログを書き出したいという場所に追加することを推奨。
void SDLOG_Flush(void);

// まだ書き出していないログを SD カードに書き込んでファイルをクローズ
void SDLOG_FinishEx(void); // OS_Printf乗っ取り版
void SDLOG_Finish(void);   // OS_Printf乗っ取らない版

// ARM7のデバッグログを出力
void SDLOG_PrintServer(void);

