/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ExecPreLoadedApp.c

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
#include "ExecPreLoadedApp.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

#define MLEP_MENU_ELEMENT_NUM			7						// メニューの項目数

#define PARAM_LENGTH	10

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------
extern RTCDrawProperty g_rtcDraw;

// static variable -------------------------------------
static u16 s_csr = 0;
static void(*s_pNowProcess)(void);

// const data  -----------------------------------------
static const u16 *s_pStrMenu[ MLEP_MENU_ELEMENT_NUM ] = 
{
	L"再配置チェッカ0（再配置無し）",
	L"再配置チェッカ1（エラー）",
	L"再配置チェッカ2（正順コピー1）",
	L"再配置チェッカ3（正順コピー2）",
	L"再配置チェッカ4（エラー）",
	L"再配置チェッカ5（WRAMへ配置）",
	L"ランチャーに戻る",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
	{ TRUE,  4 * 8,  20 * 8 },
};

static const MenuParam s_menuParam = {
	MLEP_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const u16 **)&s_pStrMenu,
};
									
//======================================================
// テストプログラム
//======================================================

typedef struct RomSegmentRange {
	u32		start;
	u32		end;
}RomSegmentRange;

#define EPLA_TWL_HEADER_LOAD_MMEM			(SYSM_TWL_ARM9_LTD_LOAD_MMEM_END - 0x4000)
#define EPLA_TWL_HEADER_LOAD_MMEM_END		SYSM_TWL_ARM9_LTD_LOAD_MMEM_END
#define EPLA_TWL_ARM7_LTD_LOAD_MMEM			(EPLA_TWL_HEADER_LOAD_MMEM - 0x108000)
#define EPLA_TWL_ARM7_LTD_LOAD_MMEM_END		EPLA_TWL_HEADER_LOAD_MMEM
#define EPLA_TWL_ARM9_LTD_LOAD_MMEM_END		EPLA_TWL_ARM7_LTD_LOAD_MMEM

static RomSegmentRange romSegmentRange[RELOCATE_INFO_NUM] = {
	{ SYSM_TWL_ARM9_LOAD_MMEM,     SYSM_TWL_ARM9_LOAD_MMEM_END },
	{ SYSM_TWL_ARM7_LOAD_MMEM,     SYSM_TWL_ARM7_LOAD_MMEM_END },
	{ SYSM_TWL_ARM9_LTD_LOAD_MMEM, EPLA_TWL_ARM9_LTD_LOAD_MMEM_END },
	{ EPLA_TWL_ARM7_LTD_LOAD_MMEM, EPLA_TWL_ARM7_LTD_LOAD_MMEM_END },
};

static RomSegmentRange romSegmentRangeNitro[RELOCATE_INFO_NUM] = {
	{ SYSM_NTR_ARM9_LOAD_MMEM,     SYSM_NTR_ARM9_LOAD_MMEM_END },
	{ SYSM_NTR_ARM7_LOAD_MMEM,     SYSM_NTR_ARM7_LOAD_MMEM_END },
	{ SYSM_TWL_ARM9_LTD_LOAD_MMEM, EPLA_TWL_ARM9_LTD_LOAD_MMEM_END },
	{ EPLA_TWL_ARM7_LTD_LOAD_MMEM, EPLA_TWL_ARM7_LTD_LOAD_MMEM_END },
};

static u32 load_region_check_list[RELOCATE_INFO_NUM][(RELOCATE_INFO_NUM+1) * 2 - 1] = 
{
	{SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, EPLA_TWL_ARM9_LTD_LOAD_MMEM_END, 
	EPLA_TWL_ARM7_LTD_LOAD_MMEM, EPLA_TWL_ARM7_LTD_LOAD_MMEM_END, 
	EPLA_TWL_HEADER_LOAD_MMEM, EPLA_TWL_HEADER_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, EPLA_TWL_ARM9_LTD_LOAD_MMEM_END, 
	EPLA_TWL_ARM7_LTD_LOAD_MMEM, EPLA_TWL_ARM7_LTD_LOAD_MMEM_END, 
	EPLA_TWL_HEADER_LOAD_MMEM, EPLA_TWL_HEADER_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	EPLA_TWL_ARM7_LTD_LOAD_MMEM, EPLA_TWL_ARM7_LTD_LOAD_MMEM_END, 
	EPLA_TWL_HEADER_LOAD_MMEM, EPLA_TWL_HEADER_LOAD_MMEM_END, 
	NULL }, 
	{SYSM_NTR_ARM9_LOAD_MMEM, SYSM_NTR_ARM9_LOAD_MMEM_END, 
	SYSM_NTR_ARM7_LOAD_MMEM, SYSM_NTR_ARM7_LOAD_MMEM_END, 
	SYSM_TWL_ARM9_LTD_LOAD_MMEM, EPLA_TWL_ARM9_LTD_LOAD_MMEM_END, 
	EPLA_TWL_HEADER_LOAD_MMEM, EPLA_TWL_HEADER_LOAD_MMEM_END, 
	NULL }, 
};

// ============================================================================
//
// アプリ起動
//
// ============================================================================

// ROMのロード先領域をチェックし、再配置の必要があればロード先アドレスを変更し、再配置情報を*infoにセットする。
// ロード先領域が被ってはいけない領域のリストcheck_destは{開始, 終了, 開始２, 終了２, ……, NULL}の形式。
// 再配置の有り無しに関わらずロード可能ならばTRUE、ロード不可能ならばFALSEを返す
// 再配置が必要ない場合、再配置情報のsrc,dest,lengthにはそれぞれNULLが代入される。
static BOOL EPLAi_CheckLoadRegionAndSetRelocateInfoEx
( u32 *dest, u32 length, RomSegmentRange default_region, u32 *check_dest, Relocate_Info *info )
{
	// 再配置情報が残っている可能性大なのでクリアしておく
	MI_CpuClearFast( info, sizeof(Relocate_Info) );
	
	if( default_region.end - default_region.start < length ) return FALSE;// サイズオーバー
	if( !( default_region.start <= *dest && *dest + length <= default_region.end ) )
	{
		// 再配置の必要あり
		while( *check_dest != NULL )
		{
			if( check_dest[0] < *dest + length && *dest < check_dest[1] ) return FALSE;// チェック領域に被ったらNG
			check_dest += 2;
		}
		
		// ここまで来ていれば再配置可能
		// 後方コピーフラグOFF
		info->rev = FALSE;
		if( default_region.start < *dest + length && *dest + length <= default_region.end )
		{
			// デフォルト配置領域の先頭部に、再配置先の後部が被っている
			// ポストクリア情報
			info->post_clear_addr = *dest + length;
			info->post_clear_length = default_region.end - (*dest + length);
		}
		else if( default_region.start <= *dest && *dest < default_region.end )
		{
			// デフォルト配置領域の後部に、再配置先の先頭部が被っている
			// ポストクリア情報
			info->post_clear_addr = default_region.start;
			info->post_clear_length = *dest - default_region.start;
			if( *dest < default_region.start + length )
			{
				// 更に、デフォルト配置領域にロードしたデータの最後尾と再配置先の先頭部が被っている
				// 後方コピーフラグON
				info->rev = TRUE;
			}
		}else
		{
			// まったく被っていない
			// ポストクリア情報
			info->post_clear_addr = default_region.start;
			info->post_clear_length = default_region.end - default_region.start;
		}
		info->src = default_region.start;
		info->dest = *dest;
		info->length = length;
		*dest = default_region.start;
	}else
	{
		// 再配置の必要なし
		info->src = NULL;
		info->dest = NULL;
		info->length = NULL;
		info->post_clear_addr = NULL;
		info->post_clear_length = NULL;
	}
	return TRUE;
}

// EPLAi_CheckLoadRegionAndSetRelocateInfoExのラッパー関数
static BOOL EPLAi_CheckLoadRegionAndSetRelocateInfo( RomSegmentName seg, u32 *dest, u32 length, Relocate_Info *info, BOOL isTwlApp)
{
	RomSegmentRange *rsr;
    if( isTwlApp )
    {
		rsr = romSegmentRange;
	}else
	{
		//NTR専用
		rsr = romSegmentRangeNitro;
	}
	return EPLAi_CheckLoadRegionAndSetRelocateInfoEx(dest, length, rsr[seg], load_region_check_list[seg], info);
}


static void DrawMenuScene( void )
{
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"ExecPreLoadedApp");
    // メニュー項目
	DrawMenu( s_csr, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"ExecPreLoadedApp");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_pNowProcess = MenuScene;

	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}

// 指定ファイルリード
static s32 ReadFile(FSFile* pf, void* buffer, s32 size)
{
    u8* p = (u8*)buffer;
    s32 remain = size;

    while( remain > 0 )
    {
        const s32 len = MATH_IMin(1024, remain);
        const s32 readLen = FS_ReadFile(pf, p, len);

        if( readLen < 0 )
        {
            return readLen;
        }
        if( readLen != len )
        {
            return size - remain + readLen;
        }

        remain -= readLen;
        p      += readLen;
    }

    return size;
}

#define EPLAi_GetLauncherParamAddr()				( (LauncherParam *)HW_PARAM_LAUNCH_PARAM )

static BOOL LoadTitle( NAMTitleId bootTitleID )
{
	enum
	{
	    region_header = 0,
	    region_arm9_ntr,
	    region_arm7_ntr,
	    region_arm9_twl,
	    region_arm7_twl,
	    region_max
	};
	// メインメモリのクリア
	// DSダウンロードプレイの時は、ROMヘッダを退避する
	// アプリロード
	// アプリ認証
	
	
	// ロード
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    BOOL isTwlApp = TRUE;
    NAM_GetTitleBootContentPath(path, bootTitleID);

    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);

    if( ! bSuccess )
    {
OS_TPrintf("RebootSystem failed: cant open file\n");
        return FALSE;
    }

    {
        int     i;
        u32     source[region_max];
        u32     length[region_max];
        u32     destaddr[region_max];
        static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
        s32 readLen;

        // まずROMヘッダを読み込む
        // (本来ならここでSRLの正当性判定)
        bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);

        if( ! bSuccess )
        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
            FS_CloseFile(file);
            return FALSE;
        }

        readLen = ReadFile(file, header, (s32)sizeof(header));

        if( readLen != (s32)sizeof(header) )
        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", header, 0, sizeof(header), readLen);
            FS_CloseFile(file);
            return FALSE;
        }

        if( header[0x15C] != 0x56 || header[0x15D] != 0xCF )
        {
int i, j;
for( i = 0; i < 0x20; ++i )
{
for( j = 0; j < 0x10; ++j )
{
OS_TPrintf("%02X ", header[i * 0x10 + j]);
}
OS_TPrintf("\n");
}
OS_TPrintf("RebootSystem failed: logo CRC error\n");
            FS_CloseFile(file);
            return FALSE;
        }
        
        if( header[0x12] && 0x03 == 0 )
        {
			//NTR専用ROM
			isTwlApp = FALSE;
		}
		
        // 各領域を読み込む
        source  [region_header  ] = 0x00000000;
        length  [region_header  ] = HW_TWL_ROM_HEADER_BUF_SIZE;
        destaddr[region_header  ] = HW_TWL_ROM_HEADER_BUF;
		
        source  [region_arm9_ntr] = *(const u32*)&header[0x020];
        length  [region_arm9_ntr] = *(const u32*)&header[0x02C];
        destaddr[region_arm9_ntr] = *(const u32*)&header[0x028];
		
        source  [region_arm7_ntr] = *(const u32*)&header[0x030];
        length  [region_arm7_ntr] = *(const u32*)&header[0x03C];
        destaddr[region_arm7_ntr] = *(const u32*)&header[0x038];
		
		if( isTwlApp )
		{
	        source  [region_arm9_twl] = *(const u32*)&header[0x1C0];
	        length  [region_arm9_twl] = *(const u32*)&header[0x1CC];
	        destaddr[region_arm9_twl] = *(const u32*)&header[0x1C8];
			
	        source  [region_arm7_twl] = *(const u32*)&header[0x1D0];
	        length  [region_arm7_twl] = *(const u32*)&header[0x1DC];
	        destaddr[region_arm7_twl] = *(const u32*)&header[0x1D8];
        }
        
        // 領域読み込み先のチェック及び再配置情報データの作成
		for( i=0; i<RELOCATE_INFO_NUM; i++ )
		{
			if ( !isTwlApp && i >= ARM9_LTD_STATIC ) continue;// nitroでは読み込まない領域
			// 再配置情報のランチャーパラメタへの受け渡し処理。暫定的な実装。SDKに機能として組み込んでしまうか検討中。
			if ( !EPLAi_CheckLoadRegionAndSetRelocateInfo( (RomSegmentName)i, &(destaddr[i+region_arm9_ntr]), length[i+region_arm9_ntr],
				 (Relocate_Info *)&(EPLAi_GetLauncherParamAddr()->body.v1.relocInfoBuf[24 * i]), isTwlApp ) )
			{
	OS_TPrintf("RebootSystem failed: ROM Load Region error\n");
	            FS_CloseFile(file);
				return FALSE;
			}
		}

        for (i = region_header; i < region_max; ++i)
        {
            u32 len = length[i];
            
            if ( !isTwlApp && i >= region_arm9_twl ) continue;// nitroでは読み込まない領域

            bSuccess = FS_SeekFile(file, (s32)source[i], FS_SEEK_SET);

            if( ! bSuccess )
            {
OS_TPrintf("RebootSystem failed: cant seek file(%d)\n", source[i]);
                FS_CloseFile(file);
                return FALSE;
            }

            readLen = ReadFile(file, (void *)destaddr[i], (s32)len);

            if( readLen != (s32)len )
            {
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
                FS_CloseFile(file);
                return FALSE;
            }
        }

        (void)FS_CloseFile(file);
    }

	// ROMヘッダバッファをコピー
	MI_CpuCopy32( (void *)HW_TWL_ROM_HEADER_BUF, (void *)EPLA_TWL_HEADER_LOAD_MMEM, EPLA_TWL_HEADER_LOAD_MMEM_END - EPLA_TWL_HEADER_LOAD_MMEM );
	
	SYSMi_GetWork()->isLoadSucceeded = TRUE;
    return TRUE;
}

static void MenuScene(void)
{
	BOOL tp_select = FALSE;
	LauncherBootFlags tempflag = {TRUE, 0, TRUE, FALSE, TRUE, FALSE, 0};
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
		if( ++s_csr == MLEP_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=MLEP_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_menuParam );
	
   	DrawMenuScene();
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
		if( s_menuPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					//アプリ起動
					if(LoadTitle(0x0003000452434b30))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b30, &tempflag ); // RCK0
					break;
				case 1:
					//アプリ起動
					if(LoadTitle(0x0003000452434b31))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b31, &tempflag ); // RCK1
					break;
				case 2:
					//アプリ起動
					if(LoadTitle(0x0003000452434b32))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b32, &tempflag ); // RCK2
					break;
				case 3:
					//アプリ起動
					if(LoadTitle(0x0003000452434b33))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b33, &tempflag ); // RCK3
					break;
				case 4:
					//アプリ起動
					if(LoadTitle(0x0003000452434b34))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b34, &tempflag ); // RCK4
					break;
				case 5:
					//アプリ起動
					if(LoadTitle(0x0003000452434b35))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b35, &tempflag ); // RCK5
					break;
				case 6:
					OS_SetLauncherParamAndResetHardware( 0, NULL, &tempflag );
					//再起動
					break;
			}
		}
	}
}

// 初期化
void ExecPreLoadedAppInit( void )
{
	ChangeUserColor( TSD_GetUserColor() );
	MenuInit();
}

// メインループ
void ExecPreLoadedAppMain(void)
{
	s_pNowProcess();
}
