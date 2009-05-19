
#include <twl.h>
#include <twl/os/common/ownerInfoEx_private.h>
#include "font.h"
#include "screen.h"
#include "keypad.h"

#define DEVKP_PATH "nand:/sys/dev.kp"

void VBlankIntr(void);
void myInit(void);
BOOL isDamyDevkp();
BOOL deleteDevkp();

s16 drawLine = 0;

void TwlMain( void )
{
	myInit();
	
	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);
		
	PrintString(0, drawLine++, CONSOLE_WHITE, "Press A key To delete dev.kp");

	while(1){
		kamiPadRead();
		
		if(kamiPadIsTrigger(PAD_BUTTON_A)){
			
			if (! isDamyDevkp() )
			{
				PrintString(0, drawLine++, CONSOLE_RED, "Dev.kp is not damy or existed.");
				break;
			}
			
			if ( !deleteDevkp() )
			{
				FSResult res = FS_GetArchiveResultCode(DEVKP_PATH);
				PrintString(0, drawLine++, CONSOLE_RED, "Delete Failed. Err:%03x", res);
				break;
			}
			
			PrintString(0, drawLine++, CONSOLE_WHITE, "Delete Succeeded.");	
		}
		

	}
		
	OS_WaitVBlankIntr();	
	OS_Terminate();
}

static BOOL deleteDevkp()
{
	return FS_DeleteFile(DEVKP_PATH);
}

static BOOL isDamyDevkp()
{
	FSFile file;
	u32 filesize;
	
	// �t�@�C���T�C�Y�����擾����
	
	if( ! FS_OpenFileEx(&file, DEVKP_PATH,  FS_FILEMODE_R))
	{
		FSResult res = FS_GetArchiveResultCode(DEVKP_PATH);
		PrintString(0, drawLine++, CONSOLE_RED, "File Open Failed. err:%03x",res);
		return FALSE;
	}
	
	filesize = FS_GetFileLength(&file);
	
	FS_CloseFile(&file);
	
	return 
		filesize == 16384 || filesize == 5 ?
		TRUE :
		FALSE ;
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
    /* DMA�����IO���W�X�^�փA�N�Z�X����̂ŃL���b�V���� Wait �͕s�v */
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

