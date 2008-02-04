/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     CooperationC.c

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
#include "misc_simple.h"
#include "CooperationC.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

// ソフトウェアキーボードLCD領域
#define CLIST_LT_X							2
#define CLIST_LT_Y							6

#define CLIST_MARGIN						2
#define CLIST_KEY_PER_SEGMENT				5
#define CLIST_SEGMENT_INTERVAL				1

#define COPA_MENU_ELEMENT_NUM			3						// メニューの項目数
#define CHAR_LIST_CHAR_NUM					120
#define CHAR_LIST_MODE_NUM					1
// 特殊キーコード
#define EOM_			(char)0xf8
#define CODE_BUTTON_TOP_	(char)0xf9
#define DEL_BUTTON_		(char)0xf9
#define SPACE_BUTTON_	(char)0xfa
#define VAR_BUTTON1_	(char)0xfb
#define VAR_BUTTON2_	(char)0xfc
#define OK_BUTTON_		(char)0xfd
#define CANCEL_BUTTON_	(char)0xfe
#define CODE_BUTTON_BOTTOM_	(char)0xff

#define CHAR_USCORE		'_'
#define KEY_PER_LINE	11

#define KEY_START		98	//ソフトウェアキーのカーソルデフォルト位置はキャンセルキー

#define PARAM_LENGTH	10

typedef struct CopA_Work
{
	u16 csr;
	int char_mode;
	u16 key_csr;
	char parameter[ PARAM_LENGTH + 1 ];
	char temp_parameter[ PARAM_LENGTH + 1 ];
	u8  temp_parameter_length;
	void(*pNowProcess)(void);
	int starx,stary;
} CopA_Work;

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------

// static variable -------------------------------------

static CopA_Work s_work = (CopA_Work){0,0,0,"","",0,0,0};

// const data  -----------------------------------------
static const char char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM];

static const char *s_pStrMenu[ COPA_MENU_ELEMENT_NUM ] = 
{
	"Set Parameter",
	"Launch CooperationB",
	"Return to Launcher",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  3,   6 },
	{ TRUE,  3,   8 },
	{ TRUE,  3,  10 },
	{ TRUE, 3,  12 },
};

static const MenuParam s_menuParam = {
	COPA_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const char **)&s_pStrMenu,
};

static const char *str_button_char[CHAR_LIST_MODE_NUM] = {
									"eisuu",
									};

//static u16 next_char_mode[CHAR_LIST_MODE_NUM-1];

static const char  str_button_del[] = "DEL";
static const char  str_button_space[] = "SPACE";
static const char  str_button_ok[] = "OK";
static const char  str_button_cancel[] = "CANCEL";

static const char *str_button[] = {
									(const char *)str_button_del,
									(const char *)str_button_space,
									NULL,
									NULL,
									(const char *)str_button_ok,
									(const char *)str_button_cancel,
									};
									
//======================================================
// アプリ連携テストプログラムA
//======================================================

static void SetSoftKeyboardButton(int mode)
{
/*
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
*/
	s_work.char_mode = mode;
}


// ソフトウェアキー関係
// キーの表示
static void DrawCharKeys( void )
{
	int l;
	char code;

	for( l=0; l<CHAR_LIST_CHAR_NUM; l++ )
	{
		u8 color=TXT_COLOR_BLACK;
		code = char_tbl[s_work.char_mode][l];
		if (s_work.key_csr == l) color = TXT_COLOR_GREEN;
		if(code != EOM_)
		{
			if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
			{
				int x = code - CODE_BUTTON_TOP_;
				myDp_Printf( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, MAIN_SCREEN, str_button[x] );
			}
			else
			{
				char s[2];
				s[0] = code;
				s[1] = 0;
				myDp_Printf( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, MAIN_SCREEN, s );
			}
		}
	}
}

// 一文字削除
static void DeleteACharacter( void )
{
	char *buf;
	u8 *length;
		buf = s_work.temp_parameter;
		length = &s_work.temp_parameter_length;
	
	if(*length > 0) buf[--(*length)] = CHAR_USCORE;
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
	
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "CooperationC");
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_work.pNowProcess = MenuScene;
	
	GX_DispOn();
	GXS_DispOn();
}

// 選択中文字キー・特殊キーで決定した時の挙動
static void PushKeys( char code )
{
	char *buf;
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
//				SetSoftKeyboardButton(next_char_mode[code - VAR_BUTTON1_]);
				break;
			case DEL_BUTTON_:
				DeleteACharacter();
				break;
			case SPACE_BUTTON_:
				if(*length < max_length) buf[(*length)++] = ' ';
				break;
			case OK_BUTTON_:
				MI_CpuClear8(buf + *length, (u32)(max_length - *length));// ゼロクリア
				MI_CpuCopy8(buf, s_work.parameter, (PARAM_LENGTH+1));
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

// ソフトウェアキー上でのキーパッド及びタッチパッド処理
// 先にReadTPしておくこと。
static void PadDetectOnKey( void )
{
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( MYPAD_IS_TRIG(PAD_KEY_RIGHT) ){									// カーソルの移動
		do
		{
			if(s_work.key_csr%KEY_PER_LINE != KEY_PER_LINE-1) s_work.key_csr++;
			else s_work.key_csr -= KEY_PER_LINE-1;
			if( s_work.key_csr == CHAR_LIST_CHAR_NUM ) s_work.key_csr -= s_work.key_csr%KEY_PER_LINE;
		}
		while(char_tbl[s_work.char_mode][s_work.key_csr]==EOM_);
	}
	if( MYPAD_IS_TRIG(PAD_KEY_LEFT) ){
		do
		{
			if(s_work.key_csr%KEY_PER_LINE != 0) s_work.key_csr--;
			else s_work.key_csr += KEY_PER_LINE-1;
			if( s_work.key_csr & 0x8000 ) s_work.key_csr = KEY_PER_LINE-1;
		}
		while(char_tbl[s_work.char_mode][s_work.key_csr]==EOM_);
	}
	if( MYPAD_IS_TRIG(PAD_KEY_DOWN) ){									// カーソルの移動
		do
		{
			s_work.key_csr += KEY_PER_LINE;
			if( s_work.key_csr >= CHAR_LIST_CHAR_NUM ) s_work.key_csr -= KEY_PER_LINE*(s_work.key_csr/KEY_PER_LINE);
		}
		while(char_tbl[s_work.char_mode][s_work.key_csr]==EOM_);
	}
	if( MYPAD_IS_TRIG(PAD_KEY_UP) ){
		do
		{
			if( s_work.key_csr < KEY_PER_LINE ) s_work.key_csr += (CHAR_LIST_CHAR_NUM/KEY_PER_LINE)*KEY_PER_LINE;
			else s_work.key_csr -= KEY_PER_LINE;
			if( s_work.key_csr >= CHAR_LIST_CHAR_NUM ) s_work.key_csr -= KEY_PER_LINE;
		}
		while(char_tbl[s_work.char_mode][s_work.key_csr]==EOM_);
	}
	
	
	if( MYPAD_IS_TRIG(PAD_BUTTON_A)  ) {				// キーが押された
		PushKeys( char_tbl[s_work.char_mode][s_work.key_csr] );
	}else if( MYPAD_IS_TRIG(PAD_BUTTON_B) ) {
		DeleteACharacter();
	}
}

// パラメータ編集画面の描画処理
static void DrawSetParameterScene( void )
{
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "PARAMETER" );
	myDp_Printf( 11, 3, TXT_COLOR_BLACK, MAIN_SCREEN, s_work.temp_parameter );
	DrawCharKeys();
}

// パラメータ編集の初期化
static void SetParameterInit( void )
{
	SetSoftKeyboardButton(0);
	s_work.key_csr = KEY_START;
	
	// パラメータ用テンポラリバッファの初期化
	MI_CpuCopy8(s_work.parameter, s_work.temp_parameter, (PARAM_LENGTH+1));
	s_work.temp_parameter_length = (u8)STD_GetStringLength( s_work.temp_parameter );
	if( s_work.temp_parameter_length < PARAM_LENGTH ) {
		MI_CpuFill8( &s_work.temp_parameter[ s_work.temp_parameter_length ], CHAR_USCORE, (u32)(PARAM_LENGTH - s_work.temp_parameter_length) );
	}
	
	DrawSetParameterScene();
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// パラメータ編集メイン
static void SetParameterMain( void )
{
	PadDetectOnKey();
	
	// 描画処理
	DrawSetParameterScene();
}

static void DrawMenuScene( void )
{
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "CooperationC");
	myDp_Printf( 19 , 6, TXT_COLOR_DARKLIGHTBLUE, MAIN_SCREEN, s_work.parameter );
    // メニュー項目
	myDp_DrawMenu( s_work.csr, MAIN_SCREEN, &s_menuParam );
	myDp_Printf( 1, 20, TXT_COLOR_BLACK, MAIN_SCREEN, "'$' pos = (%d,%d)", s_work.starx, s_work.stary);
	myDp_Printf( s_work.starx, s_work.stary, TXT_COLOR_RED, MAIN_SCREEN, "$");
}

static void MenuScene(void)
{
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_NAND, TRUE, FALSE, FALSE, FALSE, 0};

	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( MYPAD_IS_TRIG(PAD_KEY_DOWN) ){									// カーソルの移動
		if( ++s_work.csr == COPA_MENU_ELEMENT_NUM ) {
			s_work.csr=0;
		}
		if( ++s_work.stary == 24)
		{
			s_work.stary = 0;
		}
	}
	if( MYPAD_IS_TRIG(PAD_KEY_UP) ){
		if( --s_work.csr & 0x80 ) {
			s_work.csr=COPA_MENU_ELEMENT_NUM - 1;
		}
		if( --s_work.stary == -1)
		{
			s_work.stary = 23;
		}
	}
	if( MYPAD_IS_TRIG(PAD_KEY_LEFT) ){									// カーソルの移動
		if( --s_work.starx == -1)
		{
			s_work.starx = 31;
		}
	}
	if( MYPAD_IS_TRIG(PAD_KEY_RIGHT) ){
		if( ++s_work.starx == 32)
		{
			s_work.starx = 0;
		}
	}
	
    DrawMenuScene();
	
	if( MYPAD_IS_TRIG(PAD_BUTTON_A)) {				// メニュー項目への分岐
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
						if(!FS_SetCurrentDirectory("dataPrv:/")) {MI_CpuCopy8("W Error 1",s_work.parameter,10); break;}
						FS_CreateFile("test.dat", FS_PERMIT_R | FS_PERMIT_W );
						if(!FS_OpenFileEx(&f, "test.dat", FS_FILEMODE_W)) {MI_CpuCopy8("W Error 2",s_work.parameter,10); break;}
						if(-1 == FS_WriteFile(&f, &s_work, sizeof(s_work))) {MI_CpuCopy8("W Error 3",s_work.parameter,10); break;}
						if(!FS_CloseFile( &f )) {MI_CpuCopy8("W Error 4",s_work.parameter,10); break;}
					}
					// アプリ間パラメータをセット
					{
						u16 *maker_code_src_addr = (u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
						u32 *game_code_src_addr = (u32 *)(HW_TWL_ROM_HEADER_BUF + 0xc);
						// アプリ間パラメータの初期化
						OS_InitArgBufferForDelivery( OS_DELIVER_ARG_BUFFER_SIZE );
						// validフラグを立てる
						OS_SetValidDeliveryArgumentInfo( TRUE );
						// メーカーコードとゲームコードのセット
						OS_SetMakerCodeToDeliveryArgumentInfo( *maker_code_src_addr );
						OS_SetGameCodeToDeliveryArgumentInfo( *game_code_src_addr );
						OS_SetTitleIdToDeliveryArgumentInfo( 0x00030004434f5043 );
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
void CooperationCInit( void )
{
	s_work.parameter[0] = 0;
	MenuInit();
	FS_Init(3);

	{
		if( OS_IsValidDeliveryArgumentInfo() )
		{
			OS_DecodeDeliveryBuffer();
			if(STD_CompareNString((const char *)OS_GetArgv(1), "-r", 3) == 0)
			{
				// セーブしたデータから復帰
				FSFile f;
				FS_InitFile(&f);
				if(!FS_SetCurrentDirectory("dataPrv:/")) {MI_CpuCopy8("R Error 1",s_work.parameter,10); return;}
				if(!FS_OpenFileEx(&f, "test.dat", FS_FILEMODE_R)) {return;}
				if(-1 == FS_ReadFile(&f, &s_work, sizeof(s_work))) {MI_CpuCopy8("R Error 2",s_work.parameter,10); return;}
				if(!FS_CloseFile( &f )) {MI_CpuCopy8("R Error 3",s_work.parameter,10); return;}
			}
		}
	}
}

// メインループ
void CooperationCMain(void)
{
	s_work.pNowProcess();
}


//======================================================
// ソフトウェアキーボード用キャラテーブル
//======================================================

static const char char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM] = {
	{	// 英数
		'A',	'B',	'C',	'D',	'E',
		'F',	'G',	'H',	'I',	'J',	DEL_BUTTON_,
		'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	SPACE_BUTTON_,
		'U',	'V',	'W',	'X',	'Y',
		'Z',	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		
		'a',	'b',	'c',	'd',	'e',
		'f',	'g',	'h',	'i',	'j',	EOM_,
		'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	EOM_,
		'u',	'v',	'w',	'x',	'y',
		'z',	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		
		'0',	'1',	'2',	'3',	'4',
		'5',	'6',	'7',	'8',	'9',	EOM_,
		'!',	EOM_,	'&',	EOM_,	'/',
		',',	EOM_,	'.',	EOM_,	'-',	OK_BUTTON_,
		'\'',	EOM_,	'"',	EOM_,	EOM_,
		'@',	EOM_,	'(',	EOM_,	')',	CANCEL_BUTTON_,
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
};