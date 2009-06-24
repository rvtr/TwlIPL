
#include <twl.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include "font.h"
#include "screen.h"
#include "keypad.h"

#define DAMMY_TITLEID_HI 0x00030005
#define DAMMY_TITLEID_LO 0x484e4a00
#define JUMP_GAMECODE 0x00030015484e4241ULL
#define JUMP_PARAMETER 50

void VBlankIntr(void);
void myInit(void);

void TwlMain( void )
{
	unsigned char regioncode;
	myInit();
	
	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);
	
		
	PrintString(0, 0, CONSOLE_WHITE, "Press A key To AppJump");
	
	switch(OS_GetRegion()){
		case OS_TWL_REGION_JAPAN:
			regioncode = 'J';
			break;
		case OS_TWL_REGION_AMERICA:
			regioncode = 'E';
			break;
		case OS_TWL_REGION_EUROPE:
			regioncode = 'P';
			break;
		case OS_TWL_REGION_AUSTRALIA:
			regioncode = 'U';
			break;
		case OS_TWL_REGION_KOREA:
			regioncode = 'K';
			break;
		case OS_TWL_REGION_CHINA:
			regioncode = 'C';
			break;
		default:
			PrintString(0, 1, CONSOLE_RED, "Illegal Region setting!");
			OS_Terminate();
			goto ERROR;
	}
	
	
	while(1){
		kamiPadRead();
		
		if(kamiPadIsTrigger(PAD_BUTTON_A)){
			OSDeliverArgInfo info;
			u32 dammycode_lo = (u32)(DAMMY_TITLEID_LO | (u32)regioncode);
			
			// romheaderのタイトルIDを書き換えて騙す
			PrintString(0,3, CONSOLE_WHITE, "%x", dammycode_lo);
			OS_WaitVBlankIntr();
			MI_StoreLE32((void*)(HW_TWL_ROM_HEADER_BUF + 0x230), dammycode_lo);
			MI_StoreLE32((void*)(HW_TWL_ROM_HEADER_BUF + 0x234), DAMMY_TITLEID_HI);
			
			OS_InitDeliverArgInfo(&info, 0);
			OS_SetSysParamToDeliverArg(JUMP_PARAMETER);
			OS_EncodeDeliverArg();
			OS_DoApplicationJump(JUMP_GAMECODE, OS_APP_JUMP_NORMAL);
			
			PrintString(0, 1, CONSOLE_RED, "Jump Failed");
		}
		
		OS_WaitVBlankIntr();
	}
	
ERROR:
		
	OS_WaitVBlankIntr();
	OS_Terminate();
}


static void 
VBlankIntr(void)
{
	    //---- upload pseudo screen to VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));


    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}


void myInit(void)
{
    //---- init
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    //---- init displaying
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    //---- setting 2D for top screen
    GX_SetBankForBG(GX_VRAM_BG_128_A);

    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                     GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);

    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));



    //---- setting 2D for bottom screen
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);

    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                      GX_BG_COLORMODE_16,
                      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_BG0Mosaic(FALSE);
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);

    GXS_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));


    //---- screen
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

   //---- init interrupt
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

	//---- FileSytem init
	FS_Init(FS_DMA_NOT_USE);

    //---- start displaying
    GX_DispOn();
    GXS_DispOn();
}

