/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     font.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "font.h"
#include "main.h"

// define data----------------------------------
#define SEARCH_ENTRY_MAX_NUM			128			// 処理が重くなるのを防ぐための、エントリのサーチ回数上限
#define NITRO_CHAR_SIZE					0x20		// NITRO標準キャラクタサイズ(4bitカラー）
#define CHAR_CODE_OFFSET				(SJIS_CHAR_VRAM_OFFSET / NITRO_CHAR_SIZE)	// キャラナンバーをスクリーンに転送する際のオフセット（BG_CHAR_VRAM+0x4000から始めているので、オフセットは0x200）
#define DRAW_CHAR_NUM_LIMIT				(SJIS_CHAR_VRAM_SIZE   / NITRO_CHAR_SIZE)	// １つの文字列で表示できるキャラクタ数の限界

#define HANDLE_INDEX					0x40000000

// フォント構成データ
typedef struct FontComponent {
	u16					need_tate_dot;				// 文字を表示するのに必要なキャラ単位のドット数
	u16					tate_dot;					// 文字キャラの縦ドット数
	u16					yoko_dot;					// 文字キャラの横ドット数
	u16					org_char_size;				// 文字キャラのbyteサイズ
	u32 				org_row_size;				// 文字キャラ
	u32					start_adr;					// キャラクタバイナリのスタートアドレス
	u32					end_adr;					// キャラクタバイナリのエンドアドレス
}FontComponent;


// VRAMキャラデータのメモリブロック動的確保リスト
typedef struct CharAllocListTag CharAllocList;
struct CharAllocListTag {
	CharAllocList		*next;						// 次のメモリブロック確保構造体へのポインタ
	u16					memTop;						// キャラ割り当てテーブルcharAllocTblの占有ブロックの先頭ポイント
	u16					memNum;						// 確保したメモリブロックの数
}CharAllocListTag;


// SJIS文字列表示のためのVRAMエントリデータ
typedef struct StrEntry {
	const FontComponent	*font;						// 表示するフォントの構成要素
	u32					dataAddr;					// データ元のアドレス（IDとして使用）
	u32					*charMemp;					// 動的に確保したVRAMメモリブロックへのポインタ
	u16					charNum;					// トータルのキャラ数
	u16					drawPos;					// 表示位置（BGスクリーンバッファの先頭からの2byteオフセット）
}StrEntry;


// フォントの文字間隔詰め展開用データ
typedef struct FontPropotion {
	u16					*leftp;						// 現在の左側フォントデータへのポインタ
	u16					*rightp;					// 現在の右側フォントデータへのポインタ
	u16					h_offset;					// 16x16ドット単位のキャラクタデータ内の現在の横オフセット
	u8					pad[2];
	u32 				buff16x16[2][16/2];
}FontPropotion;

// インクリメント／インクリメントなしフラグ（SetScrDataで使用）
typedef enum IncFlag {
	NO_INC,											// インクリメントなし
	INC_1											// １インクリメント
}IncFlag;

// extern data----------------------------
extern void _binary_f12han_dat(void);
extern void _binary_f12han_dat_end(void);


// function's prototype declaration-------------
static void		SetScrData(u16 code_top, StrEntry *sEntryp, IncFlag inc_flag);
static u16		MeasureStringCharWidth(const u8 *str);
static void 	SpreadStringCloser(u8 *str, StrEntry *sEntryp, u16 v_offset);
static void 	SetFirstFont(u8 **strpp, u16 v_offset,  FontPropotion *fontProp);
static u8 * 	CloseFontChar(u8 **strpp, u16 v_offset,  FontPropotion *fontProp);
static u32  	CalcCharCodeAddress(u8 **strpp, const FontComponent **font);
static void 	SpreadFontZen(u16 bit0_color, u16 bit1_color, u8 *fromAdr, u32 *toAdr);
static void 	InitStrEntryTable( StrEntry **sEntrypp );
static StrEntry *GetStrEntry( u16 handle );
static StrEntry **GetStrEntryAddress( u16 handle );
static int		InsertStrEntry(u16 *handle, u16 charNum);
static int		InsertStrEntry2(u16 handle, u16 charNum);
static u16  	DeleteStrEntry( u16 handle);
static BOOL		GetEmptyStrEntry(u32 dataAddr, u16 *handlep);
static u16		SearchStrEntry(u32 dataAddr);
static u8 *     HexToDecimalString(const void *hexp, u8 *strp, u16 figure, u16 size);
static u8 *     HexToString(const void *hexp, u8 *strp, u16 figure);
static void		InitAllocSystemSJIS( TargetScreen target );
static void		*AllocMemoryFromVram( u32 size );
static void		FreeMemoryToVram( void *p );

static u16		DrawStringSJIS_Core( int x, int y, u16 color, const void *strp, u32 handleIndex);
static u32		CalcHandleIndex(u32 dataAddr, u32 index);

// global variables-----------------------------


// static variables-----------------------------
static int					font_init		= 0;
static TargetScreen			targetScreen	= BOTTOM_SCREEN;
static const FontComponent	*nowFontHan;
static StrEntry 			*entryTblS[ STR_ENTRY_MAX_NUM ];
static StrEntry 			*entryTblM[ STR_ENTRY_MAX_NUM ];
static StrEntry				**entryTblp;
static u16					*bgBakp;

// const data-----------------------------------
const u16 tbl_ASCII_to_SJIS[0x100];
const FontComponent fontTypeTableHan[] = {
	{ 16, 12,  6, 12, 14*0xbc, (u32)&_binary_f12han_dat, (u32)&_binary_f12han_dat_end},
};


// functions description------------------------

//----------------------------------------------
// API関数
//----------------------------------------------

// フォントAPIの初期化
void InitFont( TargetScreen target)
{
	if( font_init & (0x01 << target ) ) {							// ターゲットが初期すみならリターン
		return;
	}
	
	InitAllocSystemSJIS( target );									// ターゲットのVRAMアリーナ初期化
	
	if( target == TOP_SCREEN ) {									// ターゲットのエントリを初期化
		InitStrEntryTable( &entryTblM[0] );
	}else {
		InitStrEntryTable( &entryTblS[0] );
	}
	
	if( font_init == 0 ) {											// まだ何も初期化されていないなら、スクリーンを初期設定に。
		SetTargetScreenSJIS( target );
	}
	
	SetFont( (FontType)0 );
	font_init |= 0x01 << target;
}


// 表示フォントの切り替え
void SetFont(FontType font)
{
	if(font >= FONT_TYPE_MAX) {
		font = (FontType)0;
	}
	nowFontHan = &fontTypeTableHan[font];
}


// 表示ターゲットスクリーンの切り替え
void SetTargetScreenSJIS( TargetScreen target )
{
	targetScreen	= target;
	if( target == TOP_SCREEN ) {
		entryTblp		= &entryTblM[0];
		bgBakp			= bgBakM;
	}else {
		entryTblp		= &entryTblS[0];
		bgBakp			= bgBakS;
	}
}


// 指定されたSJIS文字列を表示する。（ハンドル指定なしでハンドル算出加算値付き）
static u16 DrawStringSJIS_Core( int x, int y, u16 color, const void *strp, u32 handleIndex)
{
	StrEntry *sEntryp;
	int		 newEntry;
	u16		 handle;
	u16		 charNum;
	u16		 code;
	u16		 v_offset = 1;		// ※v_offsetは引数にすることもできる。
	
	InitFont( targetScreen );
	
	// 空文字列チェック
	if(*(u8 *)strp == 0x00) {
		return 0;
	}
	
	// 文字列表示に必要なキャラクタ数の測定
	charNum = MeasureStringCharWidth((const u8 *)strp);
	if(charNum > DRAW_CHAR_NUM_LIMIT) {								// 必要キャラクタ数が５１２以内ならメモリ確保。
		OS_Printf("SJIS-draw: str length max over.\n");
		return DSJIS_ERR_STR_LENGTH_TOO_LONG;
	}
	
	// ハンドル取得
	if( !GetEmptyStrEntry( handleIndex, &handle ) ) {
		OS_Printf("SJIS-draw: error StrEntry get failed.\n");
		return DSJIS_ERR_ENTRY_GET_FAILED;
	}
	
	// 文字列エントリの登録
	newEntry = InsertStrEntry2( handle, charNum );
	if(newEntry < 0) {
		OS_Printf("SJIS-draw: error StrEntry alloc failed.\n");
		return DSJIS_ERR_ENTRY_ALLOC_FAILED;
	}
	sEntryp				= GetStrEntry(handle);						// エントリへのポインタを格納
	sEntryp->dataAddr	= handleIndex;
	
	// VRAMキャラメモリの動的確保
	if(newEntry > 0) {
		sEntryp->charMemp = AllocMemoryFromVram( (u32)( charNum * NITRO_CHAR_SIZE * 2 ) );
																	// ※縦も２キャラ分必要なので、NITRO_CHAR_SIZE*2
//		OS_Printf("alloc charMemp = %x\n",sEntryp->charMemp);
		if( sEntryp->charMemp == NULL ) {							// メモリ確保に失敗したら、エントリのメモリを解放してリターン
			OS_Printf("VRAM char memory alloc failed.\n");
			OS_Free(sEntryp);
			return DSJIS_ERR_CHAR_ALLOC_FAILED;
		}
	}
	
	// 文字列エントリの登録
	sEntryp->font    = nowFontHan;									// 現在のフォントタイプを格納
	sEntryp->drawPos = (u16)(x + y * 0x20);							// 表示場所を格納。
	sEntryp->charNum = charNum;
	
	// 縦配置位置指定v_offsetの矯正
	if(v_offset) {
		if(((nowFontHan->tate_dot & 0x07)== 0)) {					// 縦8dot or 16dotの場合はv_offsetは0
			v_offset = 0;
		}else {
			if( (16 - nowFontHan->tate_dot) < v_offset ) {			// それ以外の時は、縦16dotに収まる範囲に矯正
				v_offset = (u16)(16 - nowFontHan->tate_dot);
			}
		}
	}
	// 対象文字列のフォントデータをビット展開＆キャラデータ詰めをおこなって、確保したVRAMキャラメモリに転送
	SpreadStringCloser((u8 *)strp, sEntryp, v_offset);
	
	// スクリーンデータに今回の文字列のキャラナンバーを書き込み
	code = (u16)( (((u32)sEntryp->charMemp - (u32)G2_GetBG1CharPtr()) / NITRO_CHAR_SIZE) | color );
	SetScrData(code, sEntryp, INC_1);
	
	return handle;
}

// ハンドルインデックスの算出
static u32 CalcHandleIndex(u32 dataAddr, u32 index)
{
	if(index) {
		index += (u32)( HANDLE_INDEX + dataAddr );					// インデックスを加算した場合は、他のデータのアドレスと被らないよう、NITROメモリマップ外になるようオフセットを加算。
	}else {
		index  = dataAddr;
	}
	return index;
}


// 指定されたSJIS文字列を表示する。（ハンドル指定なし）
u16 DrawStringSJIS( int x, int y, u16 color, const void *strp)
{
	return DrawStringSJIS_Core( x, y, color, strp, (u32)strp );			// index = strp でCoreをコール
}


// 指定された値を16進数で表示する。（ハンドル指定なし版）
u16 DrawHexSJIS( int x, int y, u16 color, const void *hexp, u16 figure)
{
	u8 buff[16];
	u8 *strp = HexToString(hexp, buff, figure);
	return DrawStringSJIS_Core(x, y, color, strp, (u32)hexp );
}


// 指定された値を10進数で表示する。（ハンドル指定なし版）
u16 DrawDecimalSJIS( int x, int y, u16 color, const void *hexp, u16 figure, u16 size)
{
	u8 buff[16];
	u8 *strp = HexToDecimalString(hexp, buff, figure, size);
	return DrawStringSJIS_Core(x, y, color, strp, (u32)hexp );
}


// 指定されたSJIS文字列を表示する。（ハンドル指定なしでハンドル算出加算値付き）
u16 DrawStringSJISEx( int x, int y, u16 color, const void *strp, u32 index)
{
	index = CalcHandleIndex( (u32)strp, index);
	return DrawStringSJIS_Core( x, y, color, strp, index );				// index を加算してCoreをコール
}


// 指定された値を16進数で表示する。（ハンドル指定なし版）
u16 DrawHexSJISEx( int x, int y, u16 color, const void *hexp, u16 figure, u32 index)
{
	u8 buff[16];
	u8 *strp = HexToString(hexp, buff, figure);
	index = CalcHandleIndex( (u32)hexp, index);
	return DrawStringSJIS_Core( x, y, color, strp, index );
}


// 指定された値を10進数で表示する。（ハンドル指定なし版）
u16 DrawDecimalSJISEx( int x, int y, u16 color, const void *hexp, u16 figure, u16 size, u32 index)
{
	u8 buff[16];
	u8 *strp = HexToDecimalString(hexp, buff, figure, size);
	index = CalcHandleIndex( (u32)hexp, index);
	return DrawStringSJIS_Core(x, y, color, strp, index);
}


// 指定コードから始まるスクリーンデータのセット
static void SetScrData(u16 code_top, StrEntry *sEntryp, IncFlag inc_flag)
{
	u16 i;
	u16	lastPos;
	u16	pos_l;
	u16	*buffp_l;
	u16	*buffp_h;
	
	lastPos = 0x800 >> 1;											// バッファ最終位置 == BGスクリーンサイズ(0x800)
	pos_l   = (u16)(sEntryp->drawPos + 0x0020);						// 下8dotキャラクタ表示位置
	buffp_h = bgBakp + sEntryp->drawPos;
	buffp_l = buffp_h + 0x0020;
	
	for(i = 0; i < sEntryp->charNum; i++) {
		*buffp_h++ = code_top;
		code_top  += inc_flag;
		if(nowFontHan->need_tate_dot == 16)	{ 						// 必要縦ドットが16の場合（8ドット文字以外全部）、もう１キャラセット
			*buffp_l++ = code_top;
		}
		code_top  += inc_flag;
		pos_l++;
		if(pos_l == lastPos) {
			break;
		}
	}
}


// 指定されたハンドルのSJIS文字列をクリアする。
void ClearStringSJIS_handle(u16 handle)
{
	StrEntry *sEntryp;
	
	InitFont( targetScreen );
	
	sEntryp = GetStrEntry(handle);
	if(sEntryp == NULL) {
		return;
	}
	
	// スクリーンデータのクリア
	SetScrData(0x0000, sEntryp, NO_INC);
	
	// キャラクタデータのクリア
//	MI_CpuClearFast(sEntryp->charMemp, sEntryp->charNum * NITRO_CHAR_SIZE * 2);
																	// 動的に確保したVRAMキャラデータのクリア
	FreeMemoryToVram( sEntryp->charMemp );							// このエントリが確保していたキャラクタメモリの解放。
	(void)DeleteStrEntry(handle);
}


// 指定されたデータアドレスで表示しているSJIS文字列をクリアする。
void ClearStringSJIS(void *datap)
{
	u16 handle = SearchStrEntry( (u32)datap );
	
	if( handle > STR_ENTRY_MAX_NUM ) {
		return;
	}
	ClearStringSJIS_handle(handle);
}


// 指定されたデータアドレスで表示しているSJIS文字列をクリアする。Ex系で描画したものを消す場合。
void ClearStringSJISEx(void *datap, u32 handleIndex)
{
	u16 handle;
	
	handleIndex = CalcHandleIndex( (u32)datap, handleIndex);
	
	handle = SearchStrEntry( (u32)handleIndex );
	
	if( handle > STR_ENTRY_MAX_NUM ) {
		return;
	}
	ClearStringSJIS_handle(handle);
}



// 現在登録されている全てのSJIS文字列をクリアする（動的メモリも全て解放）
void ClearAllStringSJIS(void)
{
	u16 handle;
	
	InitFont( targetScreen );
	
	for(handle = 0; handle < STR_ENTRY_MAX_NUM; handle++) {
		if( GetStrEntry( handle ) != NULL) {
			ClearStringSJIS_handle(handle);
		}
	}
}


// 指定されたハンドルで表示されている文字列のカラーを変更。
u16 ChangeColorSJIS(u16 handle, u16 new_color)
{
	int      i;
	u16	     *buffp_l;
	u16	     *buffp_h;
	StrEntry *sEntryp = GetStrEntry( handle );						// エントリへのポインタを格納
	
	if((handle > STR_ENTRY_MAX_NUM) || (sEntryp == NULL)) {
		return 0;
	}
	
	buffp_h = bgBakp + sEntryp->drawPos;
	buffp_l = buffp_h + 0x20;
	for(i = 0; i < sEntryp->charNum; i++) {
		*buffp_h = (u16)((*buffp_h & 0x0fff) | new_color);
		*buffp_l = (u16)((*buffp_l & 0x0fff) | new_color);
		buffp_h++;
		buffp_l++;
	}
	return 1;
}

//----------------------------------------------
// 文字列エントリテーブルの処理
//----------------------------------------------

// エントリテーブルの初期化
static void InitStrEntryTable( StrEntry **sEntrypp )
{
	u16 i;
	for(i = 0; i < STR_ENTRY_MAX_NUM; i++) {
		*sEntrypp++ = NULL;
	}
}


// 指定されたハンドルのStrEntryポインタを返す
static StrEntry * GetStrEntry( u16 handle )
{
	if ( handle >= STR_ENTRY_MAX_NUM ) {
		return NULL;
	}else {
//		return entryTblS[ handle ];
		return entryTblp[ handle ];
	}
}


// 指定されたハンドルのStrEntryポインタへのポインタを返す
static StrEntry ** GetStrEntryAddress( u16 handle )
{
	if ( handle >= STR_ENTRY_MAX_NUM ) {
		return NULL;
	}else {
//		return &entryTblS[ handle ];
		return &entryTblp[ handle ];
	}
}


// StrEntryの登録
static int InsertStrEntry(u16 *handle, u16 charNum)
{
	StrEntry **sEntrypp = GetStrEntryAddress( *handle );
	
	if(*handle >= STR_ENTRY_MAX_NUM) {
		return -1;
	}else if(*handle > 0) {
		if( *sEntrypp != NULL) {									// 指定ハンドルに既にエントリがある場合は、確保メモリ量を新規メモリ量と比べて、
			if( charNum > (*sEntrypp)->charNum ) {					// 再確保が必要な場合は、メモリ解放→再確保を行う。
//				OS_Printf("SJIS: same entry free to charMemory.\n");
//				OS_Printf("free charMemp = %x\n",(*sEntrypp)->charMemp);
				SetScrData( 0x0000, *sEntrypp, NO_INC );			// スクリーンデータのクリア
				FreeMemoryToVram( (*sEntrypp)->charMemp );			// このエントリが確保していたキャラクタメモリの解放。
				return 1;
			}else {
				if( charNum < (*sEntrypp)->charNum ) {
					SetScrData( 0x0000, *sEntrypp, NO_INC );		// スクリーンデータのクリア
				}
				return 0;											// このエントリでは、キャラクタメモリを継続して使用
			}
		}
	}
	
	// *handle == 0													// 指定ハンドルが０の時は新しいハンドルを確保
	*handle		= 0;
	sEntrypp	= GetStrEntryAddress( *handle );
	
	for( ; *handle < STR_ENTRY_MAX_NUM; (*handle)++ ) {
		if( *sEntrypp++ == NULL) {									// NULLエントリを探す
			break;
		}
	}
	if(*handle == STR_ENTRY_MAX_NUM) {
		return -1;													// NULLエントリが見つからなかったら、エラー終了。
	}
	
	// StrEntryのメモリ確保
	*sEntrypp = (StrEntry *)OS_Alloc( sizeof(StrEntry) );
	if( *sEntrypp == NULL) {
		OS_Printf("StrEntry allocate failed.\n");
		return -1;
	}else {
		return 1;
	}
}


// エントリ挿入処理２
static int InsertStrEntry2(u16 handle, u16 charNum)
{
	StrEntry **sEntrypp = GetStrEntryAddress( handle );
	
	if( *sEntrypp != NULL) {									// 指定ハンドルに既にエントリがある場合は、確保メモリ量を新規メモリ量と比べて、
		if( charNum > (*sEntrypp)->charNum ) {					// 再確保が必要な場合は、メモリ解放→再確保を行う。
//			OS_Printf("SJIS: same entry free to charMemory.\n");
//			OS_Printf("free charMemp = %x\n",(*sEntrypp)->charMemp);
			SetScrData( 0x0000, *sEntrypp, NO_INC );			// スクリーンデータのクリア
			FreeMemoryToVram( (*sEntrypp)->charMemp );			// このエントリが確保していたキャラクタメモリの解放。
			return 1;
		}else {
			if( charNum < (*sEntrypp)->charNum ) {
				SetScrData( 0x0000, *sEntrypp, NO_INC );		// スクリーンデータのクリア
			}
			return 0;											// このエントリでは、キャラクタメモリを継続して使用
		}
	}
	
	// StrEntryのメモリを新規確保
	*sEntrypp = (StrEntry *)OS_Alloc( sizeof(StrEntry) );
	if( *sEntrypp == NULL) {
		OS_Printf("StrEntry allocate failed.\n");
		return -1;
	}else {
		return 1;
	}
}


// 指定されたハンドルのStrEntry要素をエントリテーブルから削除
static u16 DeleteStrEntry(u16 handle)
{
	StrEntry **sEntrypp = GetStrEntryAddress( handle );
	
	if( *sEntrypp == NULL) {
		return NULL;
	}
	
	OS_Free( *sEntrypp );
	*sEntrypp = NULL;
	
	return handle;
}


// 空きエントリを探す（あんま効率よくない）
static BOOL GetEmptyStrEntry(u32 dataAddr, u16 *handlep)
{
	int i		= SEARCH_ENTRY_MAX_NUM;								// 処理が重くなるのを防ぐため、サーチ回数に制限を加える。
	u16 handle	= (u16)( (dataAddr >> 2) & (STR_ENTRY_MAX_NUM - 1) );
	
	while(i--) {
		StrEntry *sEntryp = GetStrEntry( handle );
		if( sEntryp == NULL ) {
			*handlep = handle;
			return TRUE;											// 新規ハンドル位置確保
		}else {
			if( sEntryp->dataAddr == dataAddr ) {
				*handlep = handle;
				return TRUE;										// 既存ハンドル位置発見
			}
			handle = (u16)( (handle + 1) & (STR_ENTRY_MAX_NUM - 1) );
		}
	}
	return FALSE;
}


// 指定データのハンドルを探す（あんま効率よくない）
static u16 SearchStrEntry(u32 dataAddr)
{
	int i		= SEARCH_ENTRY_MAX_NUM;								// 処理が重くなるのを防ぐため、サーチ回数に制限を加える。
	u16 handle	= (u16)( (dataAddr >> 2) & (STR_ENTRY_MAX_NUM - 1) );
	
	while(i--) {
		StrEntry *sEntryp = GetStrEntry( handle );
		if(sEntryp != NULL) {
			if( sEntryp->dataAddr == dataAddr ) {
				return handle;										// 既存ハンドル位置発見
			}
		}
		handle = (u16)( (handle + 1) & (STR_ENTRY_MAX_NUM - 1) );
	}
	return 0xffff;
}


//----------------------------------------------
// キャラクタVRAMの動的確保処理
//----------------------------------------------

// 文字列表示に必要なキャラクタ数の測定
static u16 MeasureStringCharWidth(const u8 *str)
{
	u16 totalWidth = 0;												// 文字列のトータルの横ドットサイズ
	u16 charNum    = 0;												// 文字列表示に必要なキャラクタ数
	
	// 文字列のトータルの横ドットサイズを算出
	while(*str) {													// 各文字は　SJIS　か　ASCII　か？
		if( ((*str >= SJIS_HIGHER_CODE1_MIN) && (*str <= SJIS_HIGHER_CODE1_MAX))
		 || ((*str >= SJIS_HIGHER_CODE2_MIN) && (*str <= SJIS_HIGHER_CODE2_MAX)) ) {
			str += 2;												// SJIS文字
			totalWidth += nowFontHan->yoko_dot;
		}else {
			str++;													// ASCII文字
			totalWidth += nowFontHan->yoko_dot;
		}
	}
	// トータル横ドット数から、文字列の表示に必要なキャラクタ数を算出
	charNum = (u16)(totalWidth >> 3);
	if(totalWidth & 0x0007) {
		charNum = (u16)(charNum + 1);
	}
	
	return charNum;
}


//----------------------------------------------
//  フォントデータ→NITROキャラデータへの変換関数群
//----------------------------------------------

// 指定された文字列を指定メモリブロックに展開しながら転送（使用フォントによって文字詰めも行う）
static void SpreadStringCloser(u8 *strp, StrEntry *sEntryp, u16 v_offset)
{
	u32 charBuff[(NITRO_CHAR_SIZE * 4) / 4];						// 16x16キャラ２文字で４キャラ分のバッファ。（２文字展開して、文字詰めを行うため）
	u8  *charMemp;
	u8  *fromAdr;
	u16 i;
	u16 charNum;
	FontPropotion fontPro;
	
	// Spread target string char data to WRAM pool
	charNum          = sEntryp->charNum;
	charMemp         = (u8 *)sEntryp->charMemp;
	fontPro.h_offset = 0;
	fontPro.leftp    = (u16 *)0x0;
	fontPro.rightp   = (u16 *)0x0;
	
	while(1) {
		// 隣接する2つの1bitフォントデータを16x16ドットキャラクタ配列にロード
		if(fontPro.h_offset == 0) {
			SetFirstFont(&strp, v_offset, &fontPro);				// 表示文字列を16x16ドット単位に区切った時に先頭にくるキャラクタをロード
		}
		fromAdr = CloseFontChar(&strp, v_offset, &fontPro);			// 上記キャラクタに隣接するキャラクタをロード
		
		// ロードした1bitフォントデータを4bitNITROキャラデータに展開
		SpreadFontZen(3, 1, fromAdr, (u32 *)charBuff);				// キャラクタデータを4bitに展開。
		
		// 展開したキャラデータをVRAMにコピー
		fromAdr = (u8 *)charBuff;
		for(i = 0; i < 2; i++) {									// 生成した16x16ドット（４キャラ）分のデータを転送
			MI_CpuCopyFast(fromAdr, charMemp, (NITRO_CHAR_SIZE * 2));
			fromAdr += NITRO_CHAR_SIZE * 2;
			charMemp+= NITRO_CHAR_SIZE * 2;
			if(--charNum == 0) {
				return;
			}
		}
	}
}


// 最初のフォントデータをバッファにセット
static void SetFirstFont(u8 **strpp, u16 v_offset, FontPropotion *fontProp)
{
	u8					tate;
	u8					*fontAdr;
	u16					*firstp;
	const FontComponent	*fontComp;
	
	if(**strpp == 0x00) {
		return;
	}
	
	fontAdr = (u8 *)CalcCharCodeAddress(strpp, &fontComp);			// 今回の文字のコードから、フォントデータ格納アドレスを算出。
	
	MI_CpuClearFast(fontProp->buff16x16[0], 64);					// 16x16ドット単位の一時展開バッファのクリア
	
	firstp = (u16 *)fontProp->buff16x16[0] + v_offset;
	for(tate = 0; tate < fontComp->tate_dot - 1; tate++) {
		*firstp = (u16)(*fontAdr++ << 8);
		if(fontComp->yoko_dot > 8) {
			*firstp |= *fontAdr++;
		}
		firstp++;
	}
	fontProp->h_offset = fontComp->yoko_dot;
	fontProp->leftp    = (u16 *)fontProp->buff16x16[0];
	fontProp->rightp   = (u16 *)fontProp->buff16x16[1];
}


// 隣接するフォントデータをバッファにセット
static u8 * CloseFontChar(u8 **strpp, u16 v_offset,  FontPropotion *fontProp)
{
	u8 					tate;
	u8 					*fontAdr;
	u16					*temp;
	u16					*leftp_tmp;
	u16					*rightp_tmp;
	const FontComponent	*fontComp;
	
	while(fontProp->h_offset < 16) {
		if(**strpp == 0x00) {
			break;
		}
		
		fontAdr = (u8 *)CalcCharCodeAddress(strpp, &fontComp);		// 今回の文字のコードから、フォントデータ格納アドレスを算出。
		
		leftp_tmp  = fontProp->leftp  + v_offset;					// フォントデータを横ドット詰めしながらロード
		rightp_tmp = fontProp->rightp + v_offset;
		for(tate=0; tate < fontComp->tate_dot - 1; tate++) {
			*rightp_tmp = (u16)(*fontAdr++ << 8);
			if(fontComp->yoko_dot > 8) {
				*rightp_tmp |= *fontAdr++;
			}
			
			*leftp_tmp   |= *rightp_tmp >> fontProp->h_offset;
			*rightp_tmp <<= 16 - fontProp->h_offset;
			leftp_tmp++;
			rightp_tmp++;
		}
		fontProp->h_offset += fontComp->yoko_dot;
	}
	fontProp->h_offset -= 16;
	temp                = fontProp->leftp;
	fontProp->leftp     = fontProp->rightp;
	fontProp->rightp    = temp;
	return (u8 *)temp;
}


// 文字コードに対応したフォントデータ格納アドレスを算出する。
static u32 CalcCharCodeAddress(u8 **strpp, const FontComponent **font)
{
	u8  higher_code;
	u8  lower_code;
	u16 sjis_code;
	u32 adr;
	
	// 対象文字が　SJIS or ASCII　かを判定
	higher_code = *(*strpp)++;
	if( ((higher_code >= SJIS_HIGHER_CODE1_MIN) && (higher_code <= SJIS_HIGHER_CODE1_MAX))
	  ||((higher_code >= SJIS_HIGHER_CODE2_MIN) && (higher_code <= SJIS_HIGHER_CODE2_MAX)) ) {	// SJISか？
		lower_code  = *(*strpp)++;
		*font       = nowFontHan;
	}else {	// ASCII
		sjis_code   = tbl_ASCII_to_SJIS[higher_code];
		higher_code = (u8)(sjis_code >> 8);
		lower_code  = (u8)(sjis_code & 0x00ff);
		*font       = nowFontHan;
	}
	
	// 文字コードからフォントデータアドレスを算出
	if(higher_code <= SJIS_HIGHER_CODE1_MIN + 3) {
  		higher_code -= 0x81;
	}else if(higher_code <= SJIS_HIGHER_CODE1_MIN + 6) {
  		higher_code -= 0x83;
	}else if(higher_code <= SJIS_HIGHER_CODE1_MAX) {
  		higher_code -= 0x84;
	}else if(higher_code >= SJIS_HIGHER_CODE2_MIN) {
 		higher_code -= 0xc4;
	}
	
	if(lower_code < 0x80) {
		lower_code -= 0x40;
	}else {
		lower_code -= 0x41;
	}
	
	adr = (*font)->start_adr
		+((*font)->org_row_size)    * higher_code
		+((*font)->org_char_size+2) * lower_code + 2;
	
	return adr;
}


// 指定バッファの1bitフォントデータを4bitNITROキャラデータに展開。
static void SpreadFontZen(u16 bit0_color, u16 bit1_color, u8 *fromAdr, u32 *toAdr)
{
	int i;
	u8  tate;
	u8  yoko;
	u8	rd_data;
	u32 wr_data;
	u32 b0c;
	u32 b1c;
	u32 *toAdr_left;
	u32 *toAdr_right;
	
	toAdr_left  = toAdr;
	toAdr_right = toAdr + 16;										// toAdr + (16 * 4byte)
	
	for(tate = 0; tate < nowFontHan->need_tate_dot; tate++) {		// キャラクタの縦ドット数だけ展開
		// 左右それぞれの8dotキャラを展開
		for(yoko = 0; yoko < 2; yoko++) {
			wr_data     = 0x00000000;								// ライトデータ初期化
			
			if(tate < nowFontHan->tate_dot) {
				rd_data     = *fromAdr++;							// 左右8dotキャラの読み出し
				b0c  = (u32)bit0_color;								// bit0 のカラー
				b1c  = (u32)bit1_color;								// bit1 のカラー
				
				for(i = 0; i < 8; i++) {							// 8dot分の各1bitの判定
					if(rd_data & 0x80) {
						wr_data |= b1c;								// "1"をカラー１へ
					}else {
						wr_data |= b0c;								// "0"をカラー０へ
					}
					rd_data    <<= 1;
					b0c <<= 4;
					b1c <<= 4;
        		}
			}
			
			if(yoko==0) {
	 			*toAdr_right++ = wr_data;							// 32bit書きこみ
			}else {
	 			*toAdr_left++  = wr_data;
			}
 		}
	}
}


//----------------------------------------------
//  MALLOCシステムの初期化
//----------------------------------------------

// VRAM上にヒープを作成。（アリーナにVRAMはないので、WRAM_MAIN, WRAM_SUBのアリーナで代用）
static void InitAllocSystemSJIS( TargetScreen target )
{
	void*			tempLo;
	OSHeapHandle	hh;
	
	// VRAM上にヒープを作成
	
	if( target == TOP_SCREEN ) {
		// メインLCD
		OS_SetArenaLo( OS_ARENA_WRAM_MAIN, (void *)VRAM_M_ARENA_LO );
		OS_SetArenaHi( OS_ARENA_WRAM_MAIN, (void *)VRAM_M_ARENA_HI );
		tempLo	= OS_InitAlloc( OS_ARENA_WRAM_MAIN, OS_GetWramMainArenaLo(), OS_GetWramMainArenaHi(), 1 );
		OS_SetArenaLo( OS_ARENA_WRAM_MAIN, tempLo );
		
		hh = OS_CreateHeap( OS_ARENA_WRAM_MAIN, OS_GetWramMainArenaLo(), OS_GetWramMainArenaHi() );
		if(hh < 0) OS_Panic( "ARM9-MAIN LCD: Fail to create heap...\n" );
		OS_Printf( " arena lo = %x\n", OS_GetWramMainArenaLo() );
	    OS_Printf( "WRAM arena hi = %x\n", OS_GetWramMainArenaHi() );
		hh		= OS_SetCurrentHeap( OS_ARENA_WRAM_MAIN, hh );
	}else {		// BOTTOM_SCREEN
		// サブLCD
		OS_SetArenaLo( OS_ARENA_WRAM_SUB, (void *)VRAM_S_ARENA_LO );
		OS_SetArenaHi( OS_ARENA_WRAM_SUB, (void *)VRAM_S_ARENA_HI );
		tempLo	= OS_InitAlloc( OS_ARENA_WRAM_SUB, OS_GetWramSubArenaLo(), OS_GetWramSubArenaHi(), 1 );
		OS_SetArenaLo( OS_ARENA_WRAM_SUB, tempLo );
		
		hh = OS_CreateHeap( OS_ARENA_WRAM_SUB, OS_GetWramSubArenaLo(), OS_GetWramSubArenaHi() );
		if(hh < 0) OS_Panic( "ARM9-SUB  LCD: Fail to create heap...\n" );
		OS_Printf( "WRAM arena lo = %x\n", OS_GetWramSubArenaLo() );
	    OS_Printf( "WRAM arena hi = %x\n", OS_GetWramSubArenaHi() );
		hh		= OS_SetCurrentHeap( OS_ARENA_WRAM_SUB, hh );
	}
}


// VRAMへのキャラメモリ確保
static void *AllocMemoryFromVram( u32 size )
{
	if ( targetScreen == TOP_SCREEN ) {
		return OS_AllocFromWramMain( size );						// ※縦も２キャラ分必要なので、NITRO_CHAR_SIZE*2
	}else {
		return OS_AllocFromWramSub ( size );						// ※縦も２キャラ分必要なので、NITRO_CHAR_SIZE*2
	}
}


// VRAMへのキャラメモリ解放
static void FreeMemoryToVram( void *p )
{
	if ( targetScreen == TOP_SCREEN ) {
		OS_FreeToWramMain( p );
	}else {
		OS_FreeToWramSub( p );
	}
}


//----------------------------------------------
//  16進データを文字列に変換
//----------------------------------------------

// メインメモリからでも大丈夫な形式での1byteのデータ読み出し。
__inline static u16 ReadByteHWBus(const void *srcp)
{
	if( (u32)srcp & 0x00000001 ) return (u16)(*(u16 *)( (u32)srcp ^ 0x00000001 ) >> 8 );
	else						 return (u16)(*(u16 *)( srcp ) & 0x00ff );
}

// 16進データを10進文字列に変換
static u8 *HexToDecimalString(const void *hexp, u8 *strp, u16 figure, u16 size)
{
	u16 i;
	u32 target = 0;
	
	for(i = 0; i < size; i++) {
		target |= ReadByteHWBus( ((u8 *)hexp)++ ) << (8 * i);
	}
	
	strp   += figure;
	*strp-- = 0x00;
	for(i = 0; i < figure; i++) {
		CP_SetDiv32_32(target, 10);
		*strp--	 = (u8 )(CP_GetDivRemainder32() + 0x30);
		target   = (u32) CP_GetDivResult32();
	}
	return (strp + 1);
}


// 16進データを16進文字列に変換
static u8 *HexToString(const void *hexp, u8 *strp, u16 figure)
{
	u16 i;
	u8  hex;
	
	strp   += figure;
	*strp-- = 0x00;
	
	for(i = 0; i < figure; i++){
		hex = (u8)ReadByteHWBus(hexp);
		if(i & 0x01){
			hex = (u8)( (hex >> 4) & 0x0f);
			((u8 *)hexp)++;
		}else{
			hex = (u8)( hex & 0x0f);
		}
		if(hex < 0x000a)	hex += 0x30;
		else				hex += 0x41 - 0x0a;
		*strp-- = hex;
	}
	return (strp + 1);
}


//----------------------------------------------
//  ASCII -> SJIS 変換テーブル
//----------------------------------------------

const u16 tbl_ASCII_to_SJIS[0x100] = {
																	// 00
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// 10
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// 20
	0x8140,0x8149,0x8168,0x81f2,0x8190,0x8193,0x8195,0x8166,
	0x8169,0x816a,0x8196,0x817b,0x8143,0x817c,0x8144,0x815e,
																	// 30
	0x824f,0x8250,0x8251,0x8252,0x8253,0x8254,0x8255,0x8256,
	0x8257,0x8258,0x8146,0x8147,0x8183,0x8181,0x8184,0x8148,
																	// 40
	0x8197,0x8260,0x8261,0x8262,0x8263,0x8264,0x8265,0x8266,
	0x8267,0x8268,0x8269,0x826a,0x826b,0x826c,0x826d,0x826e,
																	// 50
	0x826f,0x8270,0x8271,0x8272,0x8273,0x8274,0x8275,0x8276,
	0x8277,0x8278,0x8279,0x816d,0x818f,0x816e,0x814f,0x8151,
																	// 60
	0x8165,0x8281,0x8282,0x8283,0x8284,0x8285,0x8286,0x8287,
	0x8288,0x8289,0x828a,0x828b,0x828c,0x828d,0x828e,0x828f,
																	// 70
	0x8290,0x8291,0x8292,0x8293,0x8294,0x8295,0x8296,0x8297,
	0x8298,0x8299,0x829a,0x816f,0x8162,0x8170,0x8160,0x81fc,

																	// 80
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// 90
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// a0
	0x81fc,0x8142,0x8175,0x8176,0x8141,0x8145,0x8192,0x8340,
	0x8342,0x8344,0x8346,0x8348,0x8383,0x8385,0x8387,0x8362,
																	// b0
	0x815b,0x8341,0x8343,0x8345,0x8347,0x8349,0x834a,0x834c,
	0x834e,0x8350,0x8352,0x8354,0x8356,0x8358,0x835a,0x835c,
																	// c0
	0x835e,0x8360,0x8363,0x8365,0x8367,0x8369,0x836a,0x836b,
	0x836c,0x836d,0x836e,0x8371,0x8374,0x8377,0x837a,0x837d,
																	// d0
	0x837e,0x837f,0x838a,0x8382,0x8384,0x8386,0x8388,0x8389,
	0x838a,0x838b,0x838c,0x838d,0x838f,0x8393,0x814a,0x814b,

																	// e0
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// f0
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
};

