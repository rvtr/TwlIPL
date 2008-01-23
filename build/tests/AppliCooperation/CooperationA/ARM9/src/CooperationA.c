/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     CooperationA.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-31#$
  $Rev: 91 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "misc.h"
#include "CooperationA.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

// ソフトウェアキーボードLCD領域
#define CLIST_LT_X							23
#define CLIST_LT_Y							50

#define CLIST_MARGIN						14
#define CLIST_KEY_PER_SEGMENT				5
#define CLIST_SEGMENT_INTERVAL				7

#define COPA_MENU_ELEMENT_NUM			3						// メニューの項目数
#define CHAR_LIST_CHAR_NUM					120
#define CHAR_LIST_MODE_NUM					3
// 特殊キーコード
#define EOM_			(u16)0xe050
#define CODE_BUTTON_TOP_	(u16)0xe051
#define DEL_BUTTON_		(u16)0xe051
#define SPACE_BUTTON_	(u16)0xe052
#define VAR_BUTTON1_	(u16)0xe053
#define VAR_BUTTON2_	(u16)0xe054
#define OK_BUTTON_		(u16)0xe055
#define CANCEL_BUTTON_	(u16)0xe056
#define CODE_BUTTON_BOTTOM_	(u16)0xe057

#define CHAR_USCORE		L'＿'
#define KEY_PER_LINE	11

#define KEY_START		109	//ソフトウェアキーのカーソルデフォルト位置はキャンセルキー

#define KEY_OK			0xffff
#define KEY_CANCEL		0xfffe

#define PARAM_LENGTH	10

typedef struct CopA_Work
{
	u16 csr;
	int char_mode;
	u16 key_csr;
	u16 parameter[ PARAM_LENGTH + 1 ];
	u16 temp_parameter[ PARAM_LENGTH + 1 ];
	u8  temp_parameter_length;
	void(*pNowProcess)(void);
	int starx,stary;
} CopA_Work;

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------
extern RTCDrawProperty g_rtcDraw;

// static variable -------------------------------------

static CopA_Work s_work = (CopA_Work){0,0,0,L"",L"",0,0,0};

// const data  -----------------------------------------
static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM];

static const u16 *s_pStrMenu[ COPA_MENU_ELEMENT_NUM ] = 
{
	L"パラメータ設定",
	L"Bを起動",
	L"ランチャーに戻る",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE, 4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
};

static const MenuParam s_menuParam = {
	COPA_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const u16 **)&s_pStrMenu,
};

static const u16 *str_button_char[CHAR_LIST_MODE_NUM] = {
									L"かな",
									L"カナ",
									L"英数",
									};

static u16 next_char_mode[CHAR_LIST_MODE_NUM-1];

static const u16  str_button_del[] = L"DEL";
static const u16  str_button_space[] = L"SPACE";
static const u16  str_button_ok[] = L"OK";
static const u16  str_button_cancel[] = L"CANCEL";

static const u16 *str_button[] = {
									(const u16 *)str_button_del,
									(const u16 *)str_button_space,
									NULL,
									NULL,
									(const u16 *)str_button_ok,
									(const u16 *)str_button_cancel,
									};
									
//======================================================
// アプリ連携テストプログラムA
//======================================================

// UTF16の文字列長算出
static u8 MY_StrLen( const u16 *pStr )
{
	u8 len = 0;
	while( *pStr++ ) {
		++len;
		if( len == 255 ) {
			break;
		}
	}
	return len;
}

static void SetSoftKeyboardButton(int mode)
{
	int l;
	int count = 0;
	for(l=0; l<CHAR_LIST_MODE_NUM ;l++)
	{
		if(l != mode){
			str_button[2+count]=str_button_char[l];
			next_char_mode[count] = (u16)l;
			count++;
		}
	}
	s_work.char_mode = mode;
}


// ソフトウェアキー関係
// キーの表示
static void DrawCharKeys( void )
{
	int l;
	u16 code;

	for( l=0; l<CHAR_LIST_CHAR_NUM; l++ )
	{
		int color=TXT_COLOR_BLACK;
		code = char_tbl[s_work.char_mode][l];
		if (s_work.key_csr == l) color = TXT_COLOR_GREEN;
		if(code != EOM_)
		{
			if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
			{
				int x = code - CODE_BUTTON_TOP_;
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, str_button[x] );
			}
			else
			{
				u16 s[2];
				s[0] = code;
				s[1] = 0;
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, s );
			}
		}
	}
}

// 一文字削除
static void DeleteACharacter( void )
{
	u16 *buf;
	u8 *length;
		buf = s_work.temp_parameter;
		length = &s_work.temp_parameter_length;
	
	if(*length > 0) buf[--(*length)] = CHAR_USCORE;
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"CooperationA");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_work.pNowProcess = MenuScene;
	
	GX_DispOn();
	GXS_DispOn();
}

// 選択中文字キー・特殊キーで決定した時の挙動
static void PushKeys( u16 code )
{
	u16 *buf;
	u8 *length;
	u16 max_length;
		buf = s_work.temp_parameter;
		length = &s_work.temp_parameter_length;
		max_length = PARAM_LENGTH;
	
	if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
	{
		// 特殊キー
		switch(code)
		{
			case VAR_BUTTON1_:
			case VAR_BUTTON2_:
				SetSoftKeyboardButton(next_char_mode[code - VAR_BUTTON1_]);
				break;
			case DEL_BUTTON_:
				DeleteACharacter();
				break;
			case SPACE_BUTTON_:
				if(*length < max_length) buf[(*length)++] = L'　';
				break;
			case OK_BUTTON_:
				SVC_CpuClear(0, buf + *length, (max_length - *length) * 2, 16);// ゼロクリア
				MI_CpuCopy8(buf, s_work.parameter, 2*(PARAM_LENGTH+1));
				// セーブ後にキャンセル処理と合流
			case CANCEL_BUTTON_:
				MenuInit();
				break;
			default:// unknown code
				break;
		}
	}
	else
	{
		// 普通キー
		if(*length < max_length) buf[(*length)++] = code;
	}
}

// PadDetectOnKeyのSelectSomethingByTPで使うSelectSomethingFuncの実装
static BOOL SelectSoftwareKeyFunc( u16 *csr, TPData *tgt )
{
	// まずは候補となる座標（カーソル単位）を取得
	int csrx;
	int csry;
	int csrxy;
	int a;
	int b;
	NNSG2dTextRect rect;
	u16 code;
	BOOL ret;
	
	csrx = tgt->x - CLIST_LT_X;
	csrx = csrx - (CLIST_SEGMENT_INTERVAL*(csrx/(CLIST_MARGIN*CLIST_KEY_PER_SEGMENT+CLIST_SEGMENT_INTERVAL)));
	csrx = csrx / CLIST_MARGIN;
	csry = (tgt->y - CLIST_LT_Y) / CLIST_MARGIN;
	if(csrx < 0 ) return FALSE;

	if ( csrx >= KEY_PER_LINE ) csrx = KEY_PER_LINE - 1;
	csrxy = csrx + csry * KEY_PER_LINE;

	if ( csrxy < 0 || csrxy >= CHAR_LIST_CHAR_NUM) return FALSE;// 明らかにはみ出した

	// 候補座標のキーコード取得
	code = char_tbl[s_work.char_mode][csrxy];
	if(code == EOM_) return FALSE;
	
	// 候補座標の領域取得
	if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
	{
		int x = code - CODE_BUTTON_TOP_;
		rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, str_button[x] );
	}
	else
	{
		u16 s[2];
		s[0] = code;
		s[1] = 0;
		// rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, s );
		// 文字幅じゃかなり判定が厳しい……ギリギリまでとってみる
		rect.width = CLIST_MARGIN;
		rect.height = CLIST_MARGIN;
	}
	a = CLIST_LT_X + CLIST_MARGIN*(csrxy%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((csrxy%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT);
	b = CLIST_LT_Y + CLIST_MARGIN*(csrxy/KEY_PER_LINE);
	
	// 候補座標の領域にタッチ座標が含まれているかチェック
	ret = WithinRangeTP( a, b, a+rect.width, b+rect.height, tgt );
	
	if(ret)
	{
		*csr = (u16)csrxy;
	}
	return ret;
}

// ソフトウェアキー上でのキーパッド及びタッチパッド処理
// 先にReadTPしておくこと。
static void PadDetectOnKey( void )
{
	SelectSomethingFunc func[1];
	BOOL tp_select = FALSE;
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_RIGHT ){									// カーソルの移動
		do
		{
			if(s_work.key_csr%KEY_PER_LINE != KEY_PER_LINE-1) s_work.key_csr++;
			else s_work.key_csr -= KEY_PER_LINE-1;
			if( s_work.key_csr == CHAR_LIST_CHAR_NUM ) s_work.key_csr -= s_work.key_csr%KEY_PER_LINE;
		}
		while(char_tbl[s_work.char_mode][s_work.key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_LEFT ){
		do
		{
			if(s_work.key_csr%KEY_PER_LINE != 0) s_work.key_csr--;
			else s_work.key_csr += KEY_PER_LINE-1;
			if( s_work.key_csr & 0x8000 ) s_work.key_csr = KEY_PER_LINE-1;
		}
		while(char_tbl[s_work.char_mode][s_work.key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
		do
		{
			s_work.key_csr += KEY_PER_LINE;
			if( s_work.key_csr >= CHAR_LIST_CHAR_NUM ) s_work.key_csr -= KEY_PER_LINE*(s_work.key_csr/KEY_PER_LINE);
		}
		while(char_tbl[s_work.char_mode][s_work.key_csr]==EOM_);
	}
	if( pad.trg & PAD_KEY_UP ){
		do
		{
			if( s_work.key_csr < KEY_PER_LINE ) s_work.key_csr += (CHAR_LIST_CHAR_NUM/KEY_PER_LINE)*KEY_PER_LINE;
			else s_work.key_csr -= KEY_PER_LINE;
			if( s_work.key_csr >= CHAR_LIST_CHAR_NUM ) s_work.key_csr -= KEY_PER_LINE;
		}
		while(char_tbl[s_work.char_mode][s_work.key_csr]==EOM_);
	}
	
	func[0] = (SelectSomethingFunc)SelectSoftwareKeyFunc;
	tp_select = SelectSomethingByTP(&s_work.key_csr, func, 1 );
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// キーが押された
		PushKeys( char_tbl[s_work.char_mode][s_work.key_csr] );
	}else if( pad.trg & PAD_BUTTON_B ) {
		DeleteACharacter();
	}
}

// パラメータ編集画面の描画処理
static void DrawSetParameterScene( void )
{
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PARAMETER" );
	PutStringUTF16( 128-60 , 21 , TXT_UCOLOR_G0, s_work.temp_parameter );
	DrawCharKeys();
}

// パラメータ編集の初期化
static void SetParameterInit( void )
{
	SetSoftKeyboardButton(0);
	s_work.key_csr = KEY_START;
	
	// パラメータ用テンポラリバッファの初期化
	MI_CpuCopy8(s_work.parameter, s_work.temp_parameter, 2*(PARAM_LENGTH+1));
	s_work.temp_parameter_length = MY_StrLen( s_work.temp_parameter );
	if( s_work.temp_parameter_length < PARAM_LENGTH ) {
		SVC_CpuClear(CHAR_USCORE, &s_work.temp_parameter[ s_work.temp_parameter_length ],
					 ( PARAM_LENGTH - s_work.temp_parameter_length ) * 2, 16);
	}
	
	DrawSetParameterScene();
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// パラメータ編集メイン
static void SetParameterMain( void )
{
	ReadTP();
	
	PadDetectOnKey();
	
	// 描画処理
	DrawSetParameterScene();
}

static void DrawMenuScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"CooperationA");
	PutStringUTF16( s_work.starx-6, s_work.stary-6, TXT_UCOLOR_G0, (const u16 *)L"★");
	PutStringUTF16( 128 , 8*8, TXT_UCOLOR_G0, s_work.parameter );
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
    // メニュー項目
	DrawMenu( s_work.csr, &s_menuParam );
	PrintfSJIS( 1*8, 20*8, TXT_COLOR_BLACK, "★の位置：(%d,%d)",s_work.starx, s_work.stary);
}

static void MenuScene(void)
{
	BOOL tp_select = FALSE;
	static TPData tgt = (TPData){0,0,0,0};
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_NAND, TRUE, FALSE, FALSE, FALSE, 0};
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
		if( ++s_work.csr == COPA_MENU_ELEMENT_NUM ) {
			s_work.csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_work.csr & 0x80 ) {
			s_work.csr=COPA_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_work.csr, &s_menuParam );
	
	if(tpd.disp.touch)
	{
		tgt = tpd.disp;
		if(s_work.starx != tgt.x) s_work.starx += (((tgt.x - s_work.starx)>>3) + ((tgt.x > s_work.starx) ? 1 : -1) );
		if(s_work.stary != tgt.y) s_work.stary += (((tgt.y - s_work.stary)>>3) + ((tgt.y > s_work.stary) ? 1 : -1) );
	}
	
    DrawMenuScene();
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
		if( s_menuPos[ s_work.csr ].enable ) {
			switch( s_work.csr ) {
				case 0:
					SetParameterInit();
					s_work.pNowProcess = SetParameterMain;
					break;
				case 1:
					// 現在のアプリ状態をセーブ
					{
						FSFile f;
						FS_InitFile(&f);
						if(!FS_SetCurrentDirectory("dataPrv:/")) {MI_CpuCopy8(L"ライトエラー１",s_work.parameter,14); break;}
						FS_CreateFile("test.dat", FS_PERMIT_R | FS_PERMIT_W );
						if(!FS_OpenFileEx(&f, "test.dat", FS_FILEMODE_W)) {MI_CpuCopy8(L"ライトエラー２",s_work.parameter,14); break;}
						if(-1 == FS_WriteFile(&f, &s_work, sizeof(s_work))) {MI_CpuCopy8(L"ライトエラー３",s_work.parameter,14); break;}
						if(!FS_CloseFile( &f )) {MI_CpuCopy8(L"ライトエラー４",s_work.parameter,14); break;}
					}
					// アプリ間パラメータをセット
					{
						u16 *maker_code_src_addr = (u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
						u32 *game_code_src_addr = (u32 *)(HW_TWL_ROM_HEADER_BUF + 0xc);
						// アプリ間パラメータの初期化
						OS_InitArgBufferForDelivery( OS_DELIVER_ARG_BUFFER_SIZE );
						// validフラグを立てる
						OS_SetValidDeliveryArgumentInfo( TRUE );
						// メーカーコードとゲームコードのセット(Launcher側でやるべき？)
						OS_SetMakerCodeToDeliveryArgumentInfo( *maker_code_src_addr );
						OS_SetGameCodeToDeliveryArgumentInfo( *game_code_src_addr );
						OS_SetTitleIdToDeliveryArgumentInfo( 0x00030004434f5041 );
						// アプリ専用引数のセット
						OS_SetDeliveryArgments( (const char *)s_work.parameter );
					}
					//B起動
					OS_SetLauncherParamAndResetHardware( 0, 0x00030004434f5042, &tempflag );
					break;
				case 2:
					OS_SetLauncherParamAndResetHardware( 0, NULL, &tempflag );
					//再起動
					break;
			}
		}
	}
}

// 初期化
void CooperationAInit( void )
{
	ChangeUserColor( TSD_GetUserColor() );
	s_work.parameter[0] = 0;
	MenuInit();

	{
		if( OS_IsValidDeliveryArgumentInfo() )
		{
			OS_DecodeDeliveryBuffer();
			if(STD_CompareNString((const char *)OS_GetArgv(1), "-r", 3) == 0)
			{
				// セーブしたデータから復帰
				FSFile f;
				FS_InitFile(&f);
				if(!FS_SetCurrentDirectory("dataPrv:/")) {MI_CpuCopy8(L"リードエラー１",s_work.parameter,14); return;}
				if(!FS_OpenFileEx(&f, "test.dat", FS_FILEMODE_R)) {return;}
				if(-1 == FS_ReadFile(&f, &s_work, sizeof(s_work))) {MI_CpuCopy8(L"リードエラー２",s_work.parameter,14); return;}
				if(!FS_CloseFile( &f )) {MI_CpuCopy8(L"リードエラー３",s_work.parameter,14); return;}
			}
		}
	}
}

// メインループ
void CooperationAMain(void)
{
	s_work.pNowProcess();
}


//======================================================
// ソフトウェアキーボード用キャラテーブル
//======================================================

static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM] = {
	{	// ひらがな
		L'あ',	L'い',	L'う',	L'え',	L'お',
		L'か',	L'き',	L'く',	L'け',	L'こ',	DEL_BUTTON_,
		L'さ',	L'し',	L'す',	L'せ',	L'そ',
		L'た',	L'ち',	L'つ',	L'て',	L'と',	SPACE_BUTTON_,
		L'な',	L'に',	L'ぬ',	L'ね',	L'の',
		L'は',	L'ひ',	L'ふ',	L'へ',	L'ほ',	EOM_,
		
		L'ま',	L'み',	L'む',	L'め',	L'も',
		L'や',	EOM_,	L'ゆ',	EOM_,	L'よ',	VAR_BUTTON1_,
		L'ら',	L'り',	L'る',	L'れ',	L'ろ',
		L'わ',	L'ゐ',	L'ゑ',	L'を',	L'ん',	VAR_BUTTON2_,
		L'ぁ',	L'ぃ',	L'ぅ',	L'ぇ',	L'ぉ',
		L'ゃ',	EOM_,	L'ゅ',	EOM_,	L'ょ',	EOM_,
		
		L'が',	L'ぎ',	L'ぐ',	L'げ',	L'ご',
		L'ざ',	L'じ',	L'ず',	L'ぜ',	L'ぞ',	EOM_,
		L'だ',	L'ぢ',	L'づ',	L'で',	L'ど',
		L'ば',	L'び',	L'ぶ',	L'べ',	L'ぼ',	OK_BUTTON_,
		L'ぱ',	L'ぴ',	L'ぷ',	L'ぺ',	L'ぽ',
		L'っ',	L'、',	L'。',	L'！',	L'？',	EOM_,
		
		L'「',	L'」',	L'～',	L'・',	L'ー',
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// カタカナ
		L'ア',	L'イ',	L'ウ',	L'エ',	L'オ',
		L'カ',	L'キ',	L'ク',	L'ケ',	L'コ',	DEL_BUTTON_,
		L'サ',	L'シ',	L'ス',	L'セ',	L'ソ',
		L'タ',	L'チ',	L'ツ',	L'テ',	L'ト',	SPACE_BUTTON_,
		L'ナ',	L'ニ',	L'ヌ',	L'ネ',	L'ノ',
		L'ハ',	L'ヒ',	L'フ',	L'ヘ',	L'ホ',	EOM_,
		
		L'マ',	L'ミ',	L'ム',	L'メ',	L'モ',
		L'ヤ',	EOM_,	L'ユ',	EOM_,	L'ヨ',	VAR_BUTTON1_,
		L'ラ',	L'リ',	L'ル',	L'レ',	L'ロ',
		L'ワ',	EOM_,	L'ヲ',	EOM_,	L'ン',	VAR_BUTTON2_,
		L'ァ',	L'ィ',	L'ゥ',	L'ェ',	L'ォ',
		L'ャ',	EOM_,	L'ュ',	EOM_,	L'ョ',	EOM_,
		
		L'ガ',	L'ギ',	L'グ',	L'ゲ',	L'ゴ',
		L'ザ',	L'ジ',	L'ズ',	L'ゼ',	L'ゾ',	EOM_,
		L'ダ',	L'ヂ',	L'ヅ',	L'デ',	L'ド',
		L'バ',	L'ビ',	L'ブ',	L'ベ',	L'ボ',	OK_BUTTON_,
		L'パ',	L'ピ',	L'プ',	L'ペ',	L'ポ',
		L'ッ',	L'、',	L'。',	L'！',	L'ー',	EOM_,
		
		L'「',	L'」',	L'～',	L'・',	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// 英数
		L'Ａ',	L'Ｂ',	L'Ｃ',	L'Ｄ',	L'Ｅ',
		L'Ｆ',	L'Ｇ',	L'Ｈ',	L'Ｉ',	L'Ｊ',	DEL_BUTTON_,
		L'Ｋ',	L'Ｌ',	L'Ｍ',	L'Ｎ',	L'Ｏ',
		L'Ｐ',	L'Ｑ',	L'Ｒ',	L'Ｓ',	L'Ｔ',	SPACE_BUTTON_,
		L'Ｕ',	L'Ｖ',	L'Ｗ',	L'Ｘ',	L'Ｙ',
		L'Ｚ',	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'ａ',	L'ｂ',	L'ｃ',	L'ｄ',	L'ｅ',
		L'ｆ',	L'ｇ',	L'ｈ',	L'ｉ',	L'ｊ',	VAR_BUTTON1_,
		L'ｋ',	L'ｌ',	L'ｍ',	L'ｎ',	L'ｏ',
		L'ｐ',	L'ｑ',	L'ｒ',	L'ｓ',	L'ｔ',	VAR_BUTTON2_,
		L'ｕ',	L'ｖ',	L'ｗ',	L'ｘ',	L'ｙ',
		L'ｚ',	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		
		L'０',	L'１',	L'２',	L'３',	L'４',
		L'５',	L'６',	L'７',	L'８',	L'９',	EOM_,
		L'！',	EOM_,	L'＆',	EOM_,	L'／',
		L'，',	EOM_,	L'．',	EOM_,	L'－',	OK_BUTTON_,
		L'’',	EOM_,	L'”',	EOM_,	EOM_,
		L'＠',	EOM_,	L'（',	EOM_,	L'）',	EOM_,
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
};