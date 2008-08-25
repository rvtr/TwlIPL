
#include <twl.h>
#include "getInformation.h"
#include "viewSystemInfo.h"
#include "strResource.h"
#include "myIoreg_SCFG.h"
//#include "misc.h"

void getSCFGARM9Info( void );
void getSCFGARM7InfoReg( void );
void getSCFGARM7InfoShared( void );
void setSCFGAccessFlag( BOOL flag );
void setPsramBoundaryFlag( int idx );
void setDSPResetFlag( BOOL flag );
BOOL scfgTotalCheck( void );
void getTotalCheckResult( void );
u64 getArm7CheckData( BOOL isSecure, BOOL isNandAccessable, BOOL isVisibleSCFG, BOOL isDebugger );
u32 getArm9CheckData( void );

void getSCFGInfo( void )
{
	getSCFGARM9Info();
	getSCFGARM7InfoReg();
	getSCFGARM7InfoShared();
	getTotalCheckResult();
}	

void getSCFGARM9Info( void )
// ARM9側で取得できるSCFG情報を取得する
// ARM9SCFGAPIはレジスタを直接参照しているので、APIを使ってもレジスタを直接見ても同じ値
{
	int value;

	// ROM制御レジスタ	
	
	// IsSecureRomAccessibleの返り値はレジスタビットが反転
	value = !SCFG_IsSecureRomAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_SEC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_SEC].str.sjis = s_strJoint[ value ];
	
	value = SCFG_GetSystemRomType() == SCFG_SYSTEM_ROM_FOR_NITRO;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_STATE].iValue = 	value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_STATE].str.sjis = s_strRomMode[ value ];
	
	// クロック制御レジスタ
	value = SCFG_GetCpuSpeed();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].str.sjis = s_strCpuSpeed[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].changeFunc.cBool = SCFG_SetCpuSpeed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].kindNameList = s_strCpuSpeed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].numKindName = 2;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].changable = TRUE;

	value = SCFG_IsClockSuppliedToDSP();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].str.sjis = s_strSupply[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].changeFunc.cBool = SCFG_SupplyClockToDSP;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].kindNameList = s_strSupply;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].numKindName = 2;

	value = SCFG_IsClockSuppliedToCamera();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].str.sjis = s_strSupply[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].changeFunc.cBool = SCFG_SupplyClockToCamera;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].kindNameList = s_strSupply;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].numKindName = 2;

	value = SCFG_IsClockSuppliedToWram();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_WRAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_WRAM].str.sjis = s_strSupply[ value ];	

	value = SCFG_IsCameraCKIClockEnable();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].changeFunc.cBool = SCFG_SetCameraCKIClock;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].numKindName = 2;

	// 新規ブロック制御レジスタ
	value = ! SCFG_IsDSPReset();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].str.sjis = s_strBool[ !value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].kindNameList = s_strBool;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].changeFunc.cBool = setDSPResetFlag;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].numKindName = 2;

	// 拡張機能制御レジスタ

	value =  SCFG_IsDmacFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].changeFunc.cBool = SCFG_SetDmacFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].numKindName = 2;

	value =  SCFG_IsGeometryFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].changeFunc.cBool = SCFG_SetGeometryFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].numKindName = 2;

	value =  SCFG_IsRendererFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].changeFunc.cBool = SCFG_SetRendererFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].numKindName = 2;

	value =  SCFG_Is2DEngineFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].changeFunc.cBool = SCFG_Set2DEngineFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].numKindName = 2;

	value =  SCFG_IsDividerFixed();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].changeFunc.cBool = SCFG_SetDividerFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].numKindName = 2;
	
	value =  SCFG_IsCardFixed() ;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].changeFunc.cBool = SCFG_SetCardFixed;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].numKindName = 2;

	value = SCFG_IsIntcExpanded();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].changeFunc.cBool = SCFG_SetIntcExpanded;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].numKindName = 2;
	
	value = SCFG_IsLCDCExpanded();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].changeFunc.cBool = SCFG_SetLCDCExpanded;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].numKindName = 2;
	
	value = SCFG_IsVRAMExpanded();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].str.sjis = s_strEnable[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].changeFunc.cBool = SCFG_SetVRAMExpanded;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].kindNameList = s_strEnable;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].numKindName = 2;

	{
		u8 idx;
		value = SCFG_GetPsramdBoundary();
		
		if( value  == SCFG_PSRAM_BOUNDARY_4MB )
		{
			idx = 0;
		}
		else if ( value == SCFG_PSRAM_BOUNDARY_16MB )
		{
			idx = 1;
		}
		else if ( value == SCFG_PSRAM_BOUNDARY_32MB )
		{
			idx = 2;
		}
		
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].iValue = idx;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].str.sjis = s_strPSRAM[ idx ];
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].changable = TRUE;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].argType = ARG_INT;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].kindNameList = s_strPSRAM;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].numKindName = 3;
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].changeFunc.cInt = setPsramBoundaryFlag;
	}
	
	value = SCFG_IsNDmaAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].str.sjis = s_strAccess[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].changeFunc.cBool = SCFG_SetNDmaAccessible;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].kindNameList = s_strAccess;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].numKindName = 2;
	
	value = SCFG_IsCameraAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].str.sjis = s_strAccess[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].changeFunc.cBool = SCFG_SetCameraAccessible;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].kindNameList = s_strAccess;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].numKindName = 2;
	
	value = SCFG_IsDSPAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].str.sjis = s_strAccess[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].changeFunc.cBool = SCFG_SetDSPAccessible;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].kindNameList = s_strAccess;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].numKindName = 2;
	
	value = (reg_SCFG_EXT & REG_SCFG_EXT_MC_B_MASK) || 0;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MCB].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MCB].str.sjis = s_strAccess[ value ];
	
	value = SCFG_IsWRAMAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_WRAM].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_WRAM].str.sjis = s_strAccess[ value ];

	value = SCFG_IsConfigBlockAccessible();
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].iValue = value;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].str.sjis = s_strAccess[ value ];
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].changable = TRUE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].argType = ARG_BOOL;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].kindNameList = s_strAccess;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].changeFunc.cBool = setSCFGAccessFlag;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].numKindName = 2;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].isAligned = FALSE;
	gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].numLines = 2;
	
}

void getSCFGARM7InfoReg( void )
{
	// レジスタに直接格納されているほうのSCFGデータを取得
	

	int value;
	
	// ROM制御レジスタ(L)、(H)
	{
		// SECフラグはTRUE = 切り離し(アクセス不可),  FALSE = 接続(アクセス可)
		value = ( gArm7SCFGReg[DISP_REG_A9ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A9ROM_SEC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_SEC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_SEC].str.sjis = s_strJoint[ value ];
			
		value = ( gArm7SCFGReg[DISP_REG_A9ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A9ROM_RSEL_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_RSEL].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_RSEL].str.sjis = s_strRomMode[ value ];

		// SECフラグはTRUE = 切り離し(アクセス不可),  FALSE = 接続(アクセス可)
		value = ( gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_SEC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_SEC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_SEC].str.sjis = s_strJoint[ value ];
			
		value = ( gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_RSEL_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_RSEL].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_RSEL].str.sjis = s_strRomMode[ value ];
			
		// FuseROMフラグはTRUE = 切り離し(アクセス不可),  FALSE = 接続(アクセス可)
		value = ( gArm7SCFGReg[DISP_REG_A7ROM_OFFSET - 0x4000] & DISP_REG_SCFG_A7ROM_FUSE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_FUSE].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_FUSE].str.sjis = s_strJoint[ value ];
			
		value = ( gArm7SCFGReg[DISP_REG_ROMWE_OFFSET - 0x4000] & DISP_REG_SCFG_ROMWE_WE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_WE].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_WE].str.sjis = s_strEnable[ value ];
	}
		
	// 新規ブロッククロック制御レジスタ
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_CLK_OFFSET - 0x4000] );
		
		value = ( flag & DISP_REG_SCFG_CLK_SD1HCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD1].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD1].str.sjis = s_strSupply[ value ];
			
		value = ( flag & DISP_REG_SCFG_CLK_SD2HCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD2].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD2].str.sjis = s_strSupply[ value ];
		
		value = ( flag & DISP_REG_SCFG_CLK_AESHCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_AES].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_AES].str.sjis = s_strSupply[ value ];
			
		value = ( flag & DISP_REG_SCFG_CLK_WRAMHCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_WRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_WRAM].str.sjis = s_strSupply[ value ];
			
		value = ( flag & DISP_REG_SCFG_CLK_SNDMCLK_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SND].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SND].str.sjis = s_strSupply[ value ];
	}
	
	// JTAG制御レジスタ
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_JTAG_OFFSET - 0x4000] );
		
		value = ( flag & DISP_REG_SCFG_JTAG_ARM7SEL_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].numLines = 2;

		value = ( flag & DISP_REG_SCFG_JTAG_CPUJE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_CPU].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_CPU].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_JTAG_DSPJE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_DSP].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_DSP].str.sjis = s_strEnable[ value ];
	}	
	
	// 拡張機能制御レジスタ
	{
		u32 flag = MI_LoadLE32( &gArm7SCFGReg[DISP_REG_EXT_OFFSET - 0x4000] );
		
		value = ( flag & DISP_REG_SCFG_EXT_DMA_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMA].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMA].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_SDMA_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SDMA].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SDMA].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_SND_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SND].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SND].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_MC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].numLines = 2;

		value = ( flag & DISP_REG_SCFG_EXT_INTC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].numLines = 2;

		value = ( flag & DISP_REG_SCFG_EXT_SPI_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SPI].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SPI].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_DSEL_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].numLines = 2;

		value = ( flag & DISP_REG_SCFG_EXT_SIO_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SIO].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SIO].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_LCDC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_LCDC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_LCDC].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_VRAM_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_VRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_VRAM].str.sjis = s_strEnable[ value ];

		{
			u8 idx = 0;
			value = (int) ( (flag & DISP_REG_SCFG_EXT_PSRAM_MASK) >> DISP_REG_SCFG_EXT_PSRAM_SHIFT );

			
			if( value <= 1 )
			{
				idx = 0;
			}
			else if ( value == 2 )
			{
				idx = 1;
			}
			else if ( value == 3 )
			{
				idx = 2;
			}

			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PS].iValue = value;
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PS].str.sjis = s_strPSRAM[idx];
		}
		
		value = ( flag & DISP_REG_SCFG_EXT_DMAC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMAC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMAC].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_AES_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_AES].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_AES].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_SD1_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD1].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD1].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_SD2_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD2].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD2].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_MIC_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MIC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MIC].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_I2S_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2S].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2S].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_I2C_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2C].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2C].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_GPIO_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_GPIO].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_GPIO].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_MC_B_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MCB].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MCB].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_WRAM_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_WRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_WRAM].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_PUENABLE_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PU].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PU].str.sjis = s_strEnable[ value ];

		value = ( flag & DISP_REG_SCFG_EXT_CFG_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_CFG].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_CFG].str.sjis = s_strEnable[ value ];
	
	}
	
	// メモリカード I/F 制御レジスタ
	{
		u16 flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MC_OFFSET - 0x4000] );
		
		value = ( flag & DISP_REG_MI_MC_SL1_CDET_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_CDET].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_CDET].str.sjis = s_strBool[ value ];

		value = ( flag & DISP_REG_MI_MC_SL2_CDET_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_CDET].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_CDET].str.sjis = s_strBool[ value ];
			
		value = ( flag & DISP_REG_MI_MC_SWP_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SWP].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SWP].str.sjis = s_strBool[ value ];

		value = (flag & DISP_REG_MI_MC_SL1_MODE_MASK) >> DISP_REG_MI_MC_SL1_MODE_SHIFT ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_MODE].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_MODE].str.sjis = s_strMCMode[value];

		value = (flag & DISP_REG_MI_MC_SL2_MODE_MASK) >> DISP_REG_MI_MC_SL2_MODE_SHIFT;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_MODE].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_MODE].str.sjis = s_strMCMode[value];
		
		flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MCCHAT_OFFSET - 0x4000] );
		value = (flag & DISP_REG_MI_MCCHAT_CC_MASK) >> DISP_REG_MI_MCCHAT_CC_SHIFT;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CC].iValue = value;
		snprintf( gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CC].str.sjis , DISPINFO_BUFSIZE-1, "%04x", value);
		
		flag = MI_LoadLE16( &gArm7SCFGReg[DISP_REG_MC2_OFFSET - 0x4000] );
		value = (flag & DISP_REG_MI_MC2_CA_MASK ) >> DISP_REG_MI_MC2_CA_SHIFT;
		;gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CA].iValue = value;
		snprintf( gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CA].str.sjis , DISPINFO_BUFSIZE-1, "%04x", value );
	}
	
	// 旧無線送受信制御レジスタ
	{
		u8 flag =  gArm7SCFGReg[DISP_REG_WL_OFFSET - 0x4000];

		value = ( flag & DISP_REG_SCFG_WL_OFFB_MASK ) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_WL_OFFB].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_WL_OFFB].str.sjis = s_strEnable[ value ];
	}
	
	// オプション端子読み出しレジスタ
	{
		
		u8 flag = gArm7SCFGReg[DISP_REG_OP_OFFSET - 0x4000];
		value =  (flag & DISP_REG_SCFG_OP_OPT_MASK) >> DISP_REG_SCFG_OP_OPT_SHIFT;
				
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_FORM].iValue = (value & 0x2) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_FORM].str.sjis = value == 3 ? s_strRomForm[1] : s_strRomForm[0];
		
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_APP].iValue = (value & 0x1) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_OP_APP].str.sjis = s_strRomApp[ value ];
	}
}

void getSCFGARM7InfoShared( void )
{
	// 共有領域に退避されたほうのSCFGデータを取得する
	int value;
	
	// 拡張機能制御レジスタ (4byte)
	{
		u32 extData = MI_LoadLE32( &gArm7SCFGShared[HWi_WSYS04_OFFSET] );
		
		value = (extData & DISP_REG_SCFG_EXT_DMA_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DMA].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DMA].str.sjis = s_strEnable[ value ];
		
		value = (extData & DISP_REG_SCFG_EXT_SDMA_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SDMA].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SDMA].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_SND_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SND].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SND].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_MC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MC].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MC].numLines = 2;

		value = (extData & DISP_REG_SCFG_EXT_INTC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_INTC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_INTC].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_INTC].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_INTC].numLines = 2;

		value = (extData & DISP_REG_SCFG_EXT_SPI_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SPI].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SPI].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_DSEL_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DSEL].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DSEL].str.sjis = s_strEnable[ value ];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DSEL].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DSEL].numLines = 2;

		value = (extData & DISP_REG_SCFG_EXT_LCDC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_LCDC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_LCDC].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_VRAM_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_VRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_VRAM].str.sjis = s_strEnable[ value ];

		{
			u8 idx = 0;
			value = (int) ( (extData & HWi_WSYS04_EXT_PSRAM_MASK ) >> HWi_WSYS04_EXT_PSRAM_SHIFT );
			
			if( value <= 1 )
			{
				idx = 0;
			}
			else if ( value == 2 )
			{
				idx = 1;
			}
			else if ( value == 3 )
			{
				idx = 2;
			}

			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_PS].iValue = value;
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_PS].str.sjis = s_strPSRAM[idx];
		}

		value = (extData & DISP_REG_SCFG_EXT_DMAC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DMAC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_DMAC].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_AES_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_AES].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_AES].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_SD1_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SD1].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SD1].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_SD2_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SD2].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_SD2].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_MIC_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MIC].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MIC].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_I2S_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_I2S].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_I2S].str.sjis = s_strEnable[ value ];
				
		value = (extData & DISP_REG_SCFG_EXT_I2C_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_I2C].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_I2C].str.sjis = s_strEnable[ value ];
		
		value = (extData & DISP_REG_SCFG_EXT_GPIO_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_GPIO].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_GPIO].str.sjis = s_strEnable[ value ];
		
		value = (extData & DISP_REG_SCFG_EXT_MC_B_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MCB].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_MCB].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_WRAM_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_WRAM].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_WRAM].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_PUENABLE_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_PU].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_PU].str.sjis = s_strEnable[ value ];

		value = (extData & DISP_REG_SCFG_EXT_CFG_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_CFG].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_EXT_CFG].str.sjis = s_strEnable[ value ];
	}
	
	// ボンディング、rom状態、旧無線レジスタ ( 1byte )
	{
		u8 regData = gArm7SCFGShared[HWi_WSYS08_OFFSET];
		
		value = (regData & HWi_WSYS08_OP_OPT_MASK) >> HWi_WSYS08_OP_OPT_SHIFT ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_OP_FORM].iValue = (value & 0x2) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_OP_FORM].str.sjis = value == 3 ? s_strRomForm[1] : s_strRomForm[0];
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_OP_APP].iValue = (value & 0x1) || 0 ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_OP_APP].str.sjis = s_strRomApp[ value ];
		
		// rom制御
		value = ( regData & HWi_WSYS08_ROM_ARM9SEC_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM9_SEC ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM9_SEC ].str.sjis = s_strJoint[ value ] ;
		
		value = ( regData & HWi_WSYS08_ROM_ARM9RSEL_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM9_RSEL ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM9_RSEL ].str.sjis = s_strRomMode[value] ;
		
		value = ( regData & HWi_WSYS08_ROM_ARM7RSEL_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM7_RSEL ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM7_RSEL ].str.sjis = s_strRomMode[value] ;
		
		value = ( regData & HWi_WSYS08_ROM_ARM7FUSE_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM7_FUSE ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_ROM_ARM7_FUSE ].str.sjis = s_strJoint[ value ] ;
		
		// 
		value = ( regData & HWi_WSYS08_WL_OFFB_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_WL_OFFB ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_WL_OFFB ].str.sjis = s_strEnable[value] ;
	
	}
	
	// jtag、clkレジスタ ( 1byte )
	{
		u8 regData = gArm7SCFGShared[HWi_WSYS09_OFFSET];
		
		value = ( regData & HWi_WSYS09_JTAG_ARM7SEL_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_A7 ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_A7 ].str.sjis = s_strEnable[value] ;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_A7 ].isAligned = FALSE;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_A7 ].numLines = 2;
		
		value = ( regData & HWi_WSYS09_JTAG_CPUJE_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_CPU ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_CPU ].str.sjis = s_strEnable[value] ;
		
		value = ( regData & HWi_WSYS09_JTAG_DSPJE_MASK  ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_DSP ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_JTAG_DSP ].str.sjis = s_strEnable[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_SD1HCLK_MASK) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SD1 ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SD1 ].str.sjis = s_strSupply[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_SD2HCLK_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SD2 ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SD2 ].str.sjis = s_strSupply[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_AESHCLK_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_AES ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_AES ].str.sjis = s_strSupply[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_WRAMHCLK_MASK ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_WRAM ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_WRAM].str.sjis = s_strSupply[value] ;
		
		value = ( regData & HWi_WSYS09_CLK_SNDMCLK_MASK  ) || 0;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SND ].iValue = value;
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_SHARED_OFFSET + SCFG_ARM7_CLK_SND ].str.sjis = s_strSupply[value] ;
	}
}

void setDSPResetFlag( BOOL flag )
{
	if( flag )
	{
		SCFG_ResetDSP();
	}
	else
	{
		SCFG_ReleaseResetDSP();
	}
}

void setPsramBoundaryFlag( int idx )
{
	SCFGPsramBoundary value;
	
	if( idx < 0 || 2 < idx )
	{
		return;
	}

	OS_TPrintf("call setPsramBoundary( %d )\n",  idx );

	switch( idx )
	{	
		case 0:
			value = SCFG_PSRAM_BOUNDARY_4MB;
			break;
		
		case 1:
			value = SCFG_PSRAM_BOUNDARY_16MB;
			break;
		
		case 2:
			value = SCFG_PSRAM_BOUNDARY_32MB;
			break;
	}
	
	SCFG_SetPsramBoundary( value );
}

void setSCFGAccessFlag( BOOL flag )
{
	// Inaccessible = falseなのでフラグ反転
	if( !flag )
	{
		SCFG_SetConfigBlockInaccessible();
	}	
}

void getTotalCheckResult( void )
{
	BOOL isSecure;
	BOOL isNandAccessable;
	BOOL isVisibleSCFG;
	BOOL isDebugger;
	u64 arm7res;
	u32 arm9res;

	isDebugger = ( OS_GetRunningConsoleType() & OS_CONSOLE_TWLDEBUGGER ) || 0;

	switch( DISPINFO_BIN_IDX )
	{
		case 0:
		case 4:
			isSecure = FALSE;
			isNandAccessable = TRUE;
			isVisibleSCFG = FALSE;
			break;
			
		case 1:
		case 5:
			isSecure = FALSE;
			isNandAccessable = TRUE;
			isVisibleSCFG = TRUE;
			break;
		case 2:
			isSecure = FALSE;
			isNandAccessable = FALSE;
			isVisibleSCFG = FALSE;
			break;
		case 3:
			isSecure = FALSE;
			isNandAccessable = FALSE;
			isVisibleSCFG = TRUE;
			break;
		case 6:
		case 8:
		case 14:
		case 16:
			isSecure = TRUE;
			isNandAccessable = TRUE;
			isVisibleSCFG = FALSE;
			break;

		case 7:
		case 9:
		case 15:
		case 17:
			isSecure = TRUE;
			isNandAccessable = TRUE;
			isVisibleSCFG = TRUE;
			break;

		case 10:
		case 12:
			isSecure = TRUE;
			isNandAccessable = FALSE;
			isVisibleSCFG = FALSE;
			break;

		case 11:
		case 13:
			isSecure = TRUE;
			isNandAccessable = FALSE;
			isVisibleSCFG = TRUE;
			break;
			
		default:
			// mast not come here
			return;
	}
	
	// SCFGデータの検証
	arm7res = getArm7CheckData( isSecure, isNandAccessable, isVisibleSCFG, isDebugger );
	arm9res = getArm9CheckData(  );
	
	
	// 両方とも0が返ってくれば正解
	if( !arm7res && !arm9res )
	{
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_TOTAL_CHECK].str.sjis = s_strCorrect[TRUE];
		STD_StrLCpy( gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CHECK_ERROR7].str.sjis, s_strNA, DISPINFO_BUFSIZE );
		STD_StrLCpy( gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CHECK_ERROR9].str.sjis, s_strNA, DISPINFO_BUFSIZE );
	}
	else
	{
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_TOTAL_CHECK].str.sjis = s_strCorrect[FALSE];
		STD_TSNPrintf( gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CHECK_ERROR7].str.sjis, 
						DISPINFO_BUFSIZE, "%012llx", arm7res );
		STD_TSNPrintf( gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CHECK_ERROR9].str.sjis, 
						DISPINFO_BUFSIZE, "%08x", arm9res );

	}
}
/*
BOOL scfgTotalCheck( BOOL isSecure, BOOL isNandAccessable, BOOL isVisibleSCFG, BOOL isDebugger  )
{
	// ARM7 check

	if( isVisibleSCFG &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_SEC].iValue &&
		! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_RSEL].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_SEC].iValue &&
		! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_RSEL].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_FUSE].iValue != isSecure &&
		! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_WE].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD1].iValue == isNandAccessable &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD2].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_AES].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_WRAM].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SND].iValue &&
		! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMA].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SDMA].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SND].iValue &&
		! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SPI].iValue &&
		! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SIO].iValue && 
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_LCDC].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_VRAM].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PS].iValue == 3 &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMAC].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_AES].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD1].iValue == isNandAccessable &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD2].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MIC].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2S].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2C].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_GPIO].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MCB].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_WRAM].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PU].iValue &&
		gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_CFG].iValue )
	{
		arm7res = TRUE;
	}
	else if( !isVisibleSCFG &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_SEC].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_RSEL].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_SEC].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_RSEL].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_FUSE].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_WE].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD1].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD2].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_AES].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_WRAM].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SND].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMA].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SDMA].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SND].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SPI].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SIO].iValue && 
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_LCDC].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_VRAM].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PS].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMAC].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_AES].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD1].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD2].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MIC].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2S].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2C].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_GPIO].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MCB].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_WRAM].iValue &&
			! gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PU].iValue &&
			!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_CFG].iValue )
	{
		arm7res = TRUE;
	}
	else
	{
		arm7res = FALSE;
	}
	
	
	// arm9 check
	// こっちはSCFGLockでも見える
	if( gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_SEC].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_ROM_STATE].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CPU].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_DSP].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_WRAM].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_CLK_CAM_CKI].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_RST_DSP].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMA].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_GEO].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_REN].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_2DE].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DIV].iValue &&
		! gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_MC].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_INTC].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_LCDC].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_VRAM].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_PS].iValue == 2 &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DMAC].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CAM].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_DSP].iValue &&
		gAllInfo[MENU_SCFG_ARM9][SCFG_ARM9_EXT_CFG].iValue )
	{
		arm9res = TRUE;
	}
	else
	{
		arm9res = FALSE;
	}
	
	return arm9res && arm7res;
}*/

u64 getArm7CheckData( BOOL isSecure, BOOL isNandAccessable, BOOL isVisibleSCFG, BOOL isDebugger  )
{
	// 規定値に全て一致していればゼロが帰る
	// 一致していなければ、一致していない項目に対応したビットが立ったデータが帰る
	
	u64 result = 0;
	
	int correctValue[SCFG_ARM7_MENU_SIZE] = {
		// ROM status
		TRUE, FALSE, TRUE, FALSE, 0, FALSE,
		// CLK
		0, TRUE, TRUE, TRUE, TRUE,
		// JTAG
		0, 0, 0,
		// EXP
		FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE,
		TRUE, TRUE, TRUE, 3, TRUE, TRUE, 0, TRUE,
		TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, 0,
		// MI
		FALSE, 2, 0, FALSE, FALSE, 0x1998, 0x264c,
		// old wireless
		TRUE
		// Bonding opは不定
	};

	correctValue[SCFG_ARM7_ROM_ARM7_FUSE] = !isSecure;
	correctValue[SCFG_ARM7_CLK_SD1] = isNandAccessable;
	correctValue[SCFG_ARM7_JTAG_CPU] = isDebugger;
	correctValue[SCFG_ARM7_JTAG_DSP] = isDebugger;
	correctValue[SCFG_ARM7_JTAG_A7] = isDebugger;
	correctValue[SCFG_ARM7_EXT_SD1] = isNandAccessable;
	correctValue[SCFG_ARM7_EXT_CFG] = isVisibleSCFG;
	correctValue[SCFG_ARM7_MI_SC2_CDET] = !isDebugger;

	if( isVisibleSCFG )
	{
/*		result = 
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_SEC].iValue 	<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_ROM_ARM9_SEC) |
			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM9_RSEL].iValue) << (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_ROM_ARM9_RSEL ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_SEC].iValue 	<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_ROM_ARM7_SEC ) |
			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_RSEL].iValue)	<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_ROM_ARM7_RSEL ) |
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_ARM7_FUSE].iValue != isSecure)
																		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_ROM_ARM7_FUSE ) |
			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_ROM_WE].iValue)		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_ROM_WE ) |
			
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD1].iValue == isNandAccessable)
																		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_CLK_SD1 ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SD2].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_CLK_SD2 ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_AES].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_CLK_AES ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_WRAM].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_CLK_WRAM ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_CLK_SND].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_CLK_SND ) |
			
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_CPU].iValue == isDebugger)
																		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_JTAG_CPU ) |
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_DSP].iValue == isDebugger )
																		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_JTAG_DSP ) |
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_JTAG_A7].iValue == isDebugger )
																		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_JTAG_A7 ) |

			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMA].iValue)		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_DMA ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SDMA].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_SDMA ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SND].iValue 			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_SND ) |
			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MC].iValue)		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_MC ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_INTC].iValue 		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_INTC ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SPI].iValue 			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_SPI ) |
			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DSEL].iValue)		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_DSEL ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SIO].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_SIO ) | 
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_LCDC].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_LCDC ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_VRAM].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_VRAM ) |
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PS].iValue == 3)	<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_PS ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_DMAC].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_DMAC ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_AES].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_AES ) |
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD1].iValue == isNandAccessable)
																		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_SD1 ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_SD2].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_SD2 ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MIC].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_MIC ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2S].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_I2S ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_I2C].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_I2C ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_GPIO].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_GPIO ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_MCB].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_MCB ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_WRAM].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_WRAM ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_PU].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_PU ) |
			gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_EXT_CFG].iValue			<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_EXT_CFG ) |
			
			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_CDET].iValue)	<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_MI_SC1_CDET) |
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC1_MODE].iValue	== 2)
																		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_MI_SC1_MODE) |
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_CDET].iValue != isDebugger )
																		<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_MI_SC2_CDET) |
			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SC2_MODE].iValue)	<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_MI_SC2_MODE) |
			(!gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_SWP].iValue)	<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_MI_SWP) |
			(gAllInfo[MENU_SCFG_ARM7][SCFG_ARM7_MI_CC].iValue) == 0x1988)
																	<< (SCFG_ARM7_MENU_SIZE - SCFG_ARM7_MI_SWP) |
			;*/
		int index;
		
		for( index = 0; index < SCFG_ARM7_MENU_SIZE ; index++ )
		{
			if(SCFG_ARM7_OP_FORM <= index)
			{
				// この辺の値は開発機か量産機かで変わるので目視で判断
				//result |= 0 << index;
				continue;
			}
			
			OS_TPrintf("%d:%d:%d ", index, gAllInfo[MENU_SCFG_ARM7][index].iValue ,correctValue[index] );
			result |= ( gAllInfo[MENU_SCFG_ARM7][index].iValue != correctValue[index] ) << index;
		}
		
		return result;
	}
	else
	{
		int index;
		result = 0;
		
		for( index = 0; index < SCFG_ARM7_MENU_SIZE ; index++ )
		{
			result |= ( gAllInfo[MENU_SCFG_ARM7][index].iValue ) << index;
		}
		
		return result;
	}

}

u32 getArm9CheckData( void  )
{
	u32 result = 0;
	int index; 
	
	int correctValue[SCFG_ARM9_MENU_SIZE] = {
		// ROM status
		TRUE, FALSE,
		// CLK
		TRUE, FALSE, TRUE, TRUE, FALSE,
		// RST
		FALSE,
		// EXT
		FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
		TRUE, TRUE, TRUE, 2, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE
	};
		
	for( index = 0; index < SCFG_ARM9_MENU_SIZE ; index++ )
	{
		if( SCFG_ARM9_TOTAL_CHECK <= index)
		{
			//result |= 0 << index;
			continue;
		}
	
		OS_TPrintf("%d:%d  ",gAllInfo[MENU_SCFG_ARM9][index].iValue ,correctValue[index]  );
		result |= ( gAllInfo[MENU_SCFG_ARM9][index].iValue != correctValue[index] ) << index;
	}
		
	OS_TPrintf("\nresult: %d\n", result);
	return result;
	
}

	