/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - CARD - hotswDebug
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/os/common/format_rom.h>
#include "DEMO.h"
#include "hotswTypes.h"
#include "font.h"

#define SDK_MAKERCODE  						'10'
#define DIGEST_HASH_BLOCK_SIZE_SHA1         (512/8)

#define MY_TEMP_BUFFER_SIZE					0x80000
#define GAME_FIELD_START_ADDRESS			0x8000

#define READY_MASK							0x00800000
#define CARD_COMMAND_MASK           		0x07000000

#define MY_ROMHEADER_TWL					((ROM_Header_Short *)HW_TWL_CARD_ROM_HEADER_BUF)
#define MY_ROMHEADER_NTR					((ROM_Header_Short *)HW_CARD_ROM_HEADER)

// ---------------------------------------------------------------
// HMACSHA1の鍵
static u8 s_digestDefaultKey[ DIGEST_HASH_BLOCK_SIZE_SHA1 ] = {
    0x21, 0x06, 0xc0, 0xde,
    0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d,
    0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08,
    0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf,
    0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c,
    0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23,
    0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe,
    0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37,
    0x87, 0x46, 0x58, 0x24
};

static u32 checkBuf[MY_TEMP_BUFFER_SIZE/sizeof(u32)] ATTRIBUTE_ALIGN(16);

static ROM_Header_Short *rh;
static BOOL isTwlApplication;
static BOOL isTwlCard;
static BOOL flxhash, ltdhash, mirrorimage;
static u16  card_lock_id;

static u16  screen[32 * 32] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);

// ---------------------------------------------------------------
static BOOL CheckHashValue(void* buf, u32 size, void* digest);
static void MY_LoadCard_arm7Static(void);
static void MY_LoadCard_arm7LtdStatic(void);
static void debugMessage(void);
static void ShowResult(void);

static void SetCommand(Cmd64 *cndLE);
static void ReadIDGame(void);
static void ReadPageGame(u32 start_addr, void* buf, u32 size);
static void ReadCardData(void* dest, s32 offset, s32 length);
static void isGameModeForNTR(void);

static void VBlankIntr      (void);
static void PrintString     (s16 x, s16 y, u8 palette, char *text, ...);
static void VolumeSwitchCallback(SNDEXResult result, void* arg);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  メイン エントリポイント.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    CARDRomHeader *card_header = (void*)CARD_GetRomHeader();

    OS_Init();
    OS_InitTick();
    CARD_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();
    
    if ( card_header->maker_code == SDK_MAKERCODE )
    {
        CARD_Enable(TRUE);
    }

    /* 表示設定初期化 */
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void*)HW_OAM, GX_LCD_SIZE_Y, HW_OAM_SIZE);
    MI_CpuClearFast((void*)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void*)HW_DB_OAM, GX_LCD_SIZE_Y, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void*)HW_DB_PLTT, HW_DB_PLTT_SIZE);
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
            GX_BG_COLORMODE_16,
            GX_BG_SCRBASE_0xf800,      // SCR ベースブロック 31
            GX_BG_CHARBASE_0x00000,    // CHR ベースブロック 0
            GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    MI_CpuClearFast((void*)screen, sizeof(screen));
    DC_FlushRange(screen, sizeof(screen));
    GX_LoadBG0Scr(screen, 0, sizeof(screen));

    /* 割込み設定 */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    /* LCD 表示開始 */
    GX_DispOn();
    GXS_DispOn();
    
    PrintString(0, 0, 0xa, "***** Card Mode Check *****");
    
    // カードアプリタイプを判別。使うRom Header Buffer を切替える
    if(MY_ROMHEADER_NTR->platform_code & PLATFORM_CODE_FLAG_TWL){
        PrintString(0, 2, 0xf, "- TWL Application -");
        isTwlApplication = TRUE;
		rh = MY_ROMHEADER_TWL;
    }
    else{
        PrintString(0, 2, 0xf, "- NTR Application -");
        isTwlApplication = FALSE;
		rh = MY_ROMHEADER_NTR;
    }

    OS_Printf("Buf Adr : 0x%08x\n", checkBuf);
    
    // カードタイプの判別
    isTwlCard = (*(vu32 *)(HW_BOOT_CHECK_INFO_BUF) & HOTSW_ROMID_TWLROM_MASK) ? TRUE : FALSE;

    // 注：rhの初期化が終わってから呼び出す
	debugMessage();

    // フラグの初期化
	flxhash 	= FALSE;
    ltdhash 	= FALSE;
    mirrorimage = FALSE;
    
    // ロックIDを取得しておく
    card_lock_id = (u16)OS_GetLockID();

    if(isTwlApplication){
    	MY_LoadCard_arm7Static();
    	MY_LoadCard_arm7LtdStatic();
    }
    else{
		isGameModeForNTR();
    }
    
	ShowResult();
    
    while(1){
    	DEMO_DrawFlip();
    	OS_WaitVBlankIntr();
    }
}


/*---------------------------------------------------------------------------*
  Name:         ShowResult
  
  Description:
 *---------------------------------------------------------------------------*/
static void ShowResult(void)
{
    PrintString(0, 13, 0xf, "-- Result --");
    
    if(flxhash && ltdhash){
        if(isTwlCard){
            PrintString(0, 15, 0x6, "Game2 Mode");
        }
        else{
            PrintString(0, 15, 0x5, "Game Mode");
        }
    }
    else if(mirrorimage){
        PrintString(0, 15, 0x5, "Game Mode");
    }
    else{
        PrintString(0, 15, 0x1, "Unknown Mode");
    }
}


/*---------------------------------------------------------------------------*
  Name:         debugMessage
  
  Description:
 *---------------------------------------------------------------------------*/
static void debugMessage(void)
{
    PrintString(0, 4, 0xf, "amr7 NmlRomOfs : 0x%08x", rh->sub_rom_offset);
    PrintString(0, 5, 0xf, "amr7 LtdRomOfs : 0x%08x", rh->sub_ltd_rom_offset);

    OS_TPrintf("amr7 NmlRomOfs : 0x%08x\n", rh->sub_rom_offset);
    OS_TPrintf("amr7 NmlSize   : 0x%08x\n", rh->sub_size);
    OS_TPrintf("amr7 LtdRomOfs : 0x%08x\n", rh->sub_ltd_rom_offset);
    OS_TPrintf("amr7 LtdSize   : 0x%08x\n", rh->sub_ltd_size);
}


/*---------------------------------------------------------------------------*
  Name:         CheckHashValue
  
  Description:  モジュールハッシュチェック
 *---------------------------------------------------------------------------*/
static BOOL CheckHashValue(void* buf, u32 size, void* digest)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    BOOL    retval = TRUE;

    // クリア
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // ARM7常駐モジュールのHash値照合
    SVC_CalcHMACSHA1( sha1data,
                      buf,
                      size,
                      s_digestDefaultKey,
                      sizeof(s_digestDefaultKey) );

    return SVC_CompareSHA1( sha1data, digest );
}


/*---------------------------------------------------------------------------*
  Name:         MY_LoadCard_arm7Static
  
  Description:  ARM7常駐モジュール 読み出し・ハッシュチェック
 *---------------------------------------------------------------------------*/
static void MY_LoadCard_arm7Static(void)
{
    BOOL result;
	u32* pBuf = checkBuf;

    CARD_LockRom(card_lock_id);
    
	ReadCardData(pBuf, (s32)rh->sub_rom_offset, (s32)rh->sub_size);

	CARD_UnlockRom(card_lock_id);
    
    result = CheckHashValue(pBuf, rh->sub_size, (void *)rh->sub_static_digest);
    
    if(result){
        flxhash = TRUE;
        PrintString(0, 7, 0x8, "Flx Area     : Accessible");
    }
    else{
        flxhash = FALSE;
        PrintString(0, 7, 0x1, "Flx Area     : Not Accessible");
    }
}


/*---------------------------------------------------------------------------*
  Name:         MY_LoadCard_arm7LtdStatic
  
  Description:  ARM7拡張常駐モジュール 読み出し・ハッシュチェック
 *---------------------------------------------------------------------------*/
static void MY_LoadCard_arm7LtdStatic(void)
{
    BOOL result;
    u32* pBuf = checkBuf;

    CARD_LockRom(card_lock_id);
    
	ReadCardData(pBuf, (s32)rh->sub_ltd_rom_offset, (s32)rh->sub_ltd_size);

	CARD_UnlockRom(card_lock_id);
    
    result = CheckHashValue(pBuf, rh->sub_ltd_size, (void *)rh->sub_ltd_static_digest);
    
    if(result){
        ltdhash = TRUE;
        PrintString(0, 9, 0x8, "Ltd Area     : Accessible");
    }
    else{
        ltdhash = FALSE;
        PrintString(0, 9, 0x1, "Ltd Area     : Not Accessible");
    }
}


/*---------------------------------------------------------------------------*
  Name:         isGameModeForNTR
  
  Description:  Gameモードかどうか判定する
 *---------------------------------------------------------------------------*/
static void	isGameModeForNTR(void)
{
	u32 buf1[PAGE_SIZE/sizeof(u32)];
	u32 buf2[PAGE_SIZE/sizeof(u32)];
	int result;

    /* Rom Header領域を読んで、Game領域のイメージが出てたらGameモード */
    ReadCardData(buf1, (s32)0, 						  (s32)PAGE_SIZE);
    ReadCardData(buf2, (s32)GAME_FIELD_START_ADDRESS, (s32)PAGE_SIZE);

    result = MI_CpuComp8( buf1, buf2, PAGE_SIZE );

    if(result){
       	mirrorimage = FALSE;
        PrintString(0, 7, 0x1, "Flx Area     : Not Accessible");
    }
    else{
        mirrorimage = TRUE;
        PrintString(0, 7, 0x8, "Flx Area     : Accessible");
    }
}


/*---------------------------------------------------------------------------*
  Name:			HOTSWi_SetCommand

  Description:  引数で与えられたコマンドのエンディアンを変えてレジスタにセットする
 *---------------------------------------------------------------------------*/
static void SetCommand(Cmd64 *cndLE)
{
	Cmd64 cndBE;

    // ビッグエンディアンに直す
	cndBE.b[7] = cndLE->b[0];
	cndBE.b[6] = cndLE->b[1];
    cndBE.b[5] = cndLE->b[2];
    cndBE.b[4] = cndLE->b[3];
    cndBE.b[3] = cndLE->b[4];
    cndBE.b[2] = cndLE->b[5];
    cndBE.b[1] = cndLE->b[6];
    cndBE.b[0] = cndLE->b[7];

    // MCCMD レジスタ設定
	reg_MI_MCCMD0_A = *(u32*)cndBE.b;
	reg_MI_MCCMD1_A = *(u32*)&cndBE.b[4];
}


/*---------------------------------------------------------------------------*
  Name:         ReadIDGame
  
  Description:  ゲームモードでIDを読み込む
 *---------------------------------------------------------------------------*/
static void ReadIDGame(void)
{
    u32 	id;
	Cmd64 	cndLE;
	u32     rom_ctrl = *(vu32 *)(HW_CARD_ROM_HEADER + 0x60);

	rom_ctrl = rom_ctrl & ~CARD_COMMAND_MASK;
    
   	// リトルエンディアンで作って
	cndLE.dw  = HSWOP_G_OP_RD_ID;

	// MCCMD レジスタ設定
	SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
	reg_MI_MCCNT0_A = (u16)((reg_MI_MCCNT0_A & 0x00ff) | REG_MI_MCCNT0_E_MASK);

   	// MCCNT1 レジスタ設定
	reg_MI_MCCNT1_A = rom_ctrl | START_MASK | PAGE_1;

	while(reg_MI_MCCNT1_A & START_MASK){
		while(!(reg_MI_MCCNT1_A & READY_MASK)){}
        id = reg_MI_MCD1_A;
	}

	OS_Printf("Card ID : 0x%08x\n", id);
    
    OS_SpinWait( 100 );
}


/*---------------------------------------------------------------------------*
  Name:         ReadPageGame
  
  Description:  ゲームモードで、指定されたページを指定バッファに指定サイズ分を読み込む
 
  CT=150ns  Pagecount=1page  Latency=RomHeaderで指定の値
 *---------------------------------------------------------------------------*/
static void ReadPageGame(u32 start_addr, void* buf, u32 size)
{
    u32 		loop, counter=0;
	u64			i, page;
	Cmd64		cndLE;
	u32     	rom_ctrl = *(vu32 *)(HW_CARD_ROM_HEADER + 0x60);

    rom_ctrl = rom_ctrl & ~CARD_COMMAND_MASK;
    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

    for(i=0; i<loop; i++){
        // コマンド作成
		cndLE.dw  = HSWOP_G_OP_RD_PAGE;
		cndLE.dw |= (page + i) << HSWOP_G_RD_PAGE_ADDR_SHIFT;

		// MCCMD レジスタ設定
		SetCommand(&cndLE);

		// MCCNT0 レジスタ設定
		reg_MI_MCCNT0_A = (u16)((reg_MI_MCCNT0_A & 0x00ff) | REG_MI_MCCNT0_E_MASK);
        
   		// MCCNT1 レジスタ設定
		reg_MI_MCCNT1_A = rom_ctrl | START_MASK | PAGE_1;

		while(reg_MI_MCCNT1_A & START_MASK){
			while(!(reg_MI_MCCNT1_A & READY_MASK)){}
            *((u32 *)buf + counter++) = reg_MI_MCD1_A;
		}
    }

    OS_SpinWait( 100 );
}


/*---------------------------------------------------------------------------*
  Name:         ReadCardData

  Description:  カードから中途半端なサイズ(page途中)のデータを読み出す関数
  				エラーコードを返す
 *---------------------------------------------------------------------------*/
static void ReadCardData(void* dest, s32 offset, s32 length)
{
    static u8 page_buffer[512];
    u32 page_offset = (u32)(offset & -512);
    u32 buffer_offset = (u32)(offset % 512);
    u32 valid_length = 512 - buffer_offset;
    u32 remain_length;

    // 開始アドレスがページの途中
    if ( offset % 512 )
    {
        ReadPageGame(page_offset, page_buffer, 512);

        MI_CpuCopy8(page_buffer + buffer_offset, dest, (length < valid_length ? length : valid_length));

        dest = (u8*)dest + valid_length;
        offset += valid_length;
        length -= valid_length;
        if ( length < 0)
        {
            return;
        }
    }

    remain_length = (u32)(length % 512);
    ReadPageGame((u32)offset, dest, (u32)(length - remain_length));

    // ケツがページ途中
    if( remain_length ){
        dest   = (u8*)dest + (length - remain_length);
        offset += length - remain_length;

        ReadPageGame((u32)offset, page_buffer, 512);

        MI_CpuCopy8(page_buffer, dest, remain_length);
    }
}


/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みベクトル。
 *---------------------------------------------------------------------------*/
static void VBlankIntr (void)
{
    /* 仮想スクリーンを VRAM に反映 */
    DC_FlushRange(screen, sizeof(screen));
    GX_LoadBG0Scr(screen, 0, sizeof(screen));

    /* IRQ チェックフラグをセット */
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  仮想スクリーンに文字列を配置する。文字列は32文字まで。

  Arguments:    x       - 文字列の先頭を配置する x 座標( × 8 ドット )。
                y       - 文字列の先頭を配置する y 座標( × 8 ドット )。
                palette - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintString (s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *((u16*)(&temp[32]))    =   0x0000;
    for (i = 0; ; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        screen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}