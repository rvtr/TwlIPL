/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - pm
  File:     pm_reg_ex.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef FIRM_PM_PM_REG_EX_H_
#define FIRM_PM_PM_REG_EX_H_

#define SDK_PMIC_REV    100

#ifdef __cplusplus
extern "C" {
#endif


//================================================================
//    PMIC extention register parameter
//================================================================
//---------------- address
#define REG_PMIC_CTL2_ADDR       0x10     // R/W
#if SDK_PMIC_REV!=0
#define REG_PMIC_OUT_CTL_ADDR    0x11     // R/W
#define REG_PMIC_TIME_CTL_ADDR   0x12     // R/W
#define REG_PMIC_PM_INFO_ADDR    0x14     // R
#define REG_PMIC_BT_INFO_ADDR    0x15     // R
#define REG_PMIC_SW_FLAGS_ADDR   0x16     // R/W
#define REG_PMIC_BT_CRCT_ADDR    0x17     // R/W
#define REG_PMIC_BT_THL_ADDR     0x18     // R/W
#define REG_PMIC_BT_THH_ADDR     0x19     // R/W
#define REG_PMIC_LED_CTL_ADDR    0x1a     // R/W
#define REG_PMIC_LED12_BLK_ADDR  0x1a     // R/W  mirror
#define REG_PMIC_LED12_B4_ADDR   0x1b     // R/W
#define REG_PMIC_LED12_B3_ADDR   0x1c     // R/W
#define REG_PMIC_LED12_B2_ADDR   0x1d     // R/W
#define REG_PMIC_LED12_B1_ADDR   0x1e     // R/W
#define REG_PMIC_BL1_BRT_ADDR    0x1f     // R/W
#define REG_PMIC_BL2_BRT_ADDR    0x20     // R/W
#else // SDK_PMIC_REV==0
#define REG_PMIC_BT_INFO_ADDR    0x11     // R
#define REG_PMIC_SW_FLAGS_ADDR   0x12     // R/W
#define REG_PMIC_TIME_CTL_ADDR   0x15     // R/W
#define REG_PMIC_OUT_CTL_ADDR    0x16     // R/W
#define REG_PMIC_BT_CRCT_ADDR    0x18     // R/W
#define REG_PMIC_BT_THL_ADDR     0x19     // R/W
#define REG_PMIC_BT_THH_ADDR     0x1a     // R/W
#define REG_PMIC_LED_CTL_ADDR    0x1c     // R/W
#define REG_PMIC_LED12_B4_ADDR   0x1d     // R/W
#define REG_PMIC_LED12_B3_ADDR   0x1e     // R/W
#define REG_PMIC_LED12_B2_ADDR   0x1f     // R/W
#define REG_PMIC_LED12_B1_ADDR   0x20     // R/W
#define REG_PMIC_LED12_BLK_ADDR  0x22     // R/W
#define REG_PMIC_BL1_BRT_ADDR    0x24     // R/W
#define REG_PMIC_BL2_BRT_ADDR    0x24     // R/W
#endif // SDK_PMIC_REV==0
#define REG_PMIC_GPIO_CTL_ADDR   0x13     // R/W


//---------------- each register spec
//---- PMIC_CTL2
#define PMIC_CTL2_RST           (1<< 0)
#define PMIC_CTL2_VCNT5_BLO     (1<< 1)
#define PMIC_CTL2_BKLT1         (1<< 2)
#define PMIC_CTL2_BKLT2         (1<< 3)
#define PMIC_CTL2_VLT42         (1<< 4)  // ?
#define PMIC_CTL2_LCD_PWR       (1<< 5)
#define PMIC_CTL2_PWR_OFF       (1<< 6)

//---- PMIC_BT_INFO
#define PMIC_BT_INFO_VLTLOW     (1<< 0)
#define PMIC_BT_INFO_VLT_SHIFT  1
#define PMIC_BT_INFO_VLT_MASK   (7<< PMIC_BT_INFO_VLT_SHIFT)
#define PMIC_BT_INFO_MKR_SHIFT  5
#define PMIC_BT_INFO_MKR_MASK   (7<< PMIC_BT_INFO_MKR_SHIFT)

//---- PMIC_SW_FLAGS
#define PMIC_SW_FLAGS_WARMBOOT  (1 << 7)

//---- PMIC_GPIO_CTL
#if SDK_PMIC_REV!=0
#define PMIC_GPIO_CTL_O1        (1<< 0)
#define PMIC_GPIO_CTL_O2        (1<< 1)
#define PMIC_GPIO_CTL_O3        (1<< 2)
#else // SDK_PMIC_REV==0
#define PMIC_GPIO_CTL_O1        (1<< 1)
#define PMIC_GPIO_CTL_O2        (1<< 2)
#define PMIC_GPIO_CTL_O3        (1<< 3)
#define PMIC_GPIO_CTL_AO_SHIFT  6
#define PMIC_GPIO_CTL_AO_MASK   (3<< PMIC_GPIO_CTL_SHIFT)
#endif // SDK_PMIC_REV==0

//---- PMIC_TIME_CTL
#if SDK_PMIC_REV!=0
#define PMIC_TIME_CTL_OFF_SHIFT 0
#define PMIC_TIME_CTL_OFF_MASK  (0x7<< PMIC_TIME_CTL_OFF_SHIFT)
#define PMIC_TIME_CTL_ACQ_SHIFT 4
#define PMIC_TIME_CTL_ACQ_MASK  (0x3<< PMIC_TIME_CTL_ACQ_SHIFT)
#else // SDK_PMIC_REV==0
#define PMIC_TIME_CTL_OFF_SHIFT 0
#define PMIC_TIME_CTL_OFF_MASK  (0xf<< PMIC_TIME_CTL_OFF_SHIFT)
#endif // SDK_PMIC_REV==0

//---- PMIC_BT_CRCT
#define PMIC_BT_CRCT_TEMP_ON    (1<< 0)
#define PMIC_BT_CRCT_AMPR_ON    (1<< 1)
#define PMIC_BT_CRCT_AK_SHIFT   4
#define PMIC_BT_CRCT_AK_MASK    (0x3<< PMIC_BT_CRCT_AK_SHIFT)
#define PMIC_BT_CRCT_TK_SHIFT   6
#define PMIC_BT_CRCT_TK_MASK    (0x3<< PMIC_BT_CRCT_TK_SHIFT)

//---- PMIC_BT_THL
#define PMIC_BT_THL_TH1_SHIFT   0
#define PMIC_BT_THL_TH1_MASK    (7<< PMIC_BT_THL_TH1_SHIFT)
#define PMIC_BT_THL_TH2_SHIFT   4
#define PMIC_BT_THL_TH2_MASK    (7<< PMIC_BT_THL_TH2_SHIFT)

//---- PMIC_BT_THH
#define PMIC_BT_THH_TH3_SHIFT   0
#define PMIC_BT_THH_TH3_MASK    (7<< PMIC_BT_THH_TH3_SHIFT)
#define PMIC_BT_THH_TH4_SHIFT   4
#define PMIC_BT_THH_TH4_MASK    (7<< PMIC_BT_THH_TH4_SHIFT)

//---- PMIC_OUT_CTL
#if SDK_PMIC_REV!=0
#define PMIC_OUT_CTL_V12_SHIFT  0
#define PMIC_OUT_CTL_V12_MASK   (0x1<< PMIC_OUT_CTL_V12_SHIFT)
#define PMIC_OUT_CTL_V18_SHIFT  1
#define PMIC_OUT_CTL_V18_MASK   (0x1<< PMIC_OUT_CTL_V18_SHIFT)
#define PMIC_OUT_CTL_V33_SHIFT  2
#define PMIC_OUT_CTL_V33_MASK   (0x1<< PMIC_OUT_CTL_V33_SHIFT)
#else // SDK_PMIC_REV==0
#define PMIC_OUT_CTL_V18_SHIFT  0
#define PMIC_OUT_CTL_V18_MASK   (0x3<< PMIC_OUT_CTL_V18_SHIFT)
#define PMIC_OUT_CTL_V12_SHIFT  2
#define PMIC_OUT_CTL_V12_MASK   (0x1<< PMIC_OUT_CTL_V12_SHIFT)
#define PMIC_OUT_CTL_V33_SHIFT  3
#define PMIC_OUT_CTL_V33_MASK   (0x1<< PMIC_OUT_CTL_V33_SHIFT)
#endif // SDK_PMIC_REV==0

//---- PMIC_LED_CTL
#define PMIC_LED_CTL_L12_B4_ONLY    (1<< 0)
#define PMIC_LED_CTL_L12_AT_BLK     (1<< 1)
#if SDK_PMIC_REV!=0
#define PMIC_LED_CTL_L12_BLK_BY_SLP (1<< 2)
#define PMIC_LED_CTL_L12_BLKF_SHIFT 4
#define PMIC_LED_CTL_L12_BLKF_MASK  (0x3<< PMIC_LED_CTL_L12_BLKF_SHIFT)
#define PMIC_LED_CTL_L12_BLKD_SHIFT 6
#define PMIC_LED_CTL_L12_BLKD_MASK  (0x3<< PMIC_LED_CTL_L12_BLKD_SHIFT)
#endif // SDK_PMIC_REV!=0

//---- PMIC_LED12_BLK (mirror)
#if SDK_PMIC_REV!=0
#define PMIC_LED12_BLK_FQ_SHIFT 4
#define PMIC_LED12_BLK_FQ_MASK  (0x3<< PMIC_LED12_BLK_FQ_SHIFT)
#define PMIC_LED12_BLK_DT_SHIFT 6
#define PMIC_LED12_BLK_DT_MASK  (0x3<< PMIC_LED12_BLK_DT_SHIFT)
#else // SDK_PMIC_REV==0
#define PMIC_LED12_BLK_FQ_SHIFT 0
#define PMIC_LED12_BLK_FQ_MASK  (0x7<< PMIC_LED12_BLK_FQ_SHIFT)
#define PMIC_LED12_BLK_DT_SHIFT 4
#define PMIC_LED12_BLK_DT_MASK  (0x3<< PMIC_LED12_BLK_DT_SHIFT)
#endif // SDK_PMIC_REV==0

//---- PMIC_LED12_B4
#define PMIC_LED12_B4_L1_SHIFT  0
#define PMIC_LED12_B4_L1_MASK   (0x7<< PMIC_LED12_B4_L1_SHIFT)
#define PMIC_LED12_B4_L2_SHIFT  4
#define PMIC_LED12_B4_L2_MASK   (0x7<< PMIC_LED12_B4_L2_SHIFT)

//---- PMIC_LED12_B3
#define PMIC_LED12_B3_L1_SHIFT  0
#define PMIC_LED12_B3_L1_MASK   (0x7<< PMIC_LED12_B3_L1_SHIFT)
#define PMIC_LED12_B3_L2_SHIFT  4
#define PMIC_LED12_B3_L2_MASK   (0x7<< PMIC_LED12_B3_L2_SHIFT)

//---- PMIC_LED12_B2
#define PMIC_LED12_B2_L1_SHIFT  0
#define PMIC_LED12_B2_L1_MASK   (0x7<< PMIC_LED12_B2_L1_SHIFT)
#define PMIC_LED12_B2_L2_SHIFT  4
#define PMIC_LED12_B2_L2_MASK   (0x7<< PMIC_LED12_B2_L2_SHIFT)

//---- PMIC_LED12_B1
#define PMIC_LED12_B1_L1_SHIFT  0
#define PMIC_LED12_B1_L1_MASK   (0x7<< PMIC_LED12_B1_L1_SHIFT)
#define PMIC_LED12_B1_L2_SHIFT  4
#define PMIC_LED12_B1_L2_MASK   (0x7<< PMIC_LED12_B1_L2_SHIFT)

//---- PMIC_BL1_BRT
#define PMIC_BL1_BRT_SHIFT      0
#define PMIC_BL1_BRT_MASK       (0x1f<< PMIC_BL1_BRT_SHIFT)

//---- PMIC_BL2_BRT
#define PMIC_BL2_BRT_SHIFT      0
#define PMIC_BL2_BRT_MASK       (0x1f<< PMIC_BL2_BRT_SHIFT)


//---- PMIC_BT_INFO
typedef enum
{
    PMIC_BT_INFO_VLT_L1 = (0 << PMIC_BT_INFO_VLT_SHIFT),
    PMIC_BT_INFO_VLT_L2 = (1 << PMIC_BT_INFO_VLT_SHIFT),
    PMIC_BT_INFO_VLT_L3 = (2 << PMIC_BT_INFO_VLT_SHIFT),
    PMIC_BT_INFO_VLT_L4 = (3 << PMIC_BT_INFO_VLT_SHIFT),
    PMIC_BT_INFO_VLT_L5 = (4 << PMIC_BT_INFO_VLT_SHIFT)
}
PMBatteryLevel;

//---- PMIC_TIME_CTL
typedef enum
{
#if SDK_PMIC_REV!=0
    PMIC_TIME_CTL_OFF_125MS  = (0x0 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_250MS  = (0x1 << PMIC_TIME_CTL_OFF_SHIFT),  // default
    PMIC_TIME_CTL_OFF_500MS  = (0x2 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_750MS  = (0x3 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_1S     = (0x4 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_2S     = (0x5 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_4S     = (0x6 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_8S     = (0x7 << PMIC_TIME_CTL_OFF_SHIFT)
#else // SDK_PMIC_REV==0
    PMIC_TIME_CTL_OFF_100MS  = (0x0 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_200MS  = (0x1 << PMIC_TIME_CTL_OFF_SHIFT),  // default
    PMIC_TIME_CTL_OFF_300MS  = (0x2 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_500MS  = (0x3 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_700MS  = (0x4 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_900MS  = (0x5 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_1S     = (0x6 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_1500MS = (0x7 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_2S     = (0x8 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_2500MS = (0x9 << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_3S     = (0xa << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_4S     = (0xb << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_5S     = (0xc << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_7S     = (0xd << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_9S     = (0xe << PMIC_TIME_CTL_OFF_SHIFT),
    PMIC_TIME_CTL_OFF_10S    = (0xf << PMIC_TIME_CTL_OFF_SHIFT)
#endif // SDK_PMIC_REV==0
}
PMOffTime;

//---- PMIC_BT_CRCT
typedef enum
{
    PMIC_BT_CRCT_AK_30_10 = (0 << PMIC_BT_CRCT_AK_SHIFT),
    PMIC_BT_CRCT_AK_50_10 = (1 << PMIC_BT_CRCT_AK_SHIFT),  // default
    PMIC_BT_CRCT_AK_70_10 = (2 << PMIC_BT_CRCT_AK_SHIFT),
    PMIC_BT_CRCT_AK_90_10 = (3 << PMIC_BT_CRCT_AK_SHIFT)
}
PMAmprCoeff;

typedef enum
{
    PMIC_BT_CRCT_TK_10 = (0 << PMIC_BT_CRCT_TK_SHIFT),
    PMIC_BT_CRCT_TK_15 = (1 << PMIC_BT_CRCT_TK_SHIFT),  // default
    PMIC_BT_CRCT_TK_20 = (2 << PMIC_BT_CRCT_TK_SHIFT),
    PMIC_BT_CRCT_TK_30 = (3 << PMIC_BT_CRCT_TK_SHIFT)
}
PMTempCoeff;

//---- PMIC_BT_THL / PMIC_BT_THH
typedef enum
{
    PMIC_BT_TH_D0   = 0,
    PMIC_BT_TH_D20  = 1,
    PMIC_BT_TH_D40  = 2,
    PMIC_BT_TH_D60  = 3,
    PMIC_BT_TH_D80  = 4,
    PMIC_BT_TH_D100 = 5,
    PMIC_BT_TH_D120 = 6,
    PMIC_BT_TH_D140 = 7
}
PMBatteryThresholdDownCommon;

typedef enum
{
    PMIC_BT_THL_TH1_D0   = (0 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D20  = (1 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D40  = (2 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D60  = (3 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D80  = (4 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D100 = (5 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D120 = (6 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D140 = (7 << PMIC_BT_THL_TH1_SHIFT)
}
PMBatteryThreshold1Down;

typedef enum
{
    PMIC_BT_THL_TH2_D0   = (0 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D20  = (1 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D40  = (2 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D60  = (3 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D80  = (4 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D100 = (5 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D120 = (6 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D140 = (7 << PMIC_BT_THL_TH2_SHIFT)
}
PMBatteryThreshold2Down;

typedef enum
{
    PMIC_BT_THH_TH3_D0   = (0 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D20  = (1 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D40  = (2 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D60  = (3 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D80  = (4 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D100 = (5 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D120 = (6 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D140 = (7 << PMIC_BT_THH_TH3_SHIFT)
}
PMBatteryThreshold3Down;

typedef enum
{
    PMIC_BT_THH_TH4_D0   = (0 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D20  = (1 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D40  = (2 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D60  = (3 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D80  = (4 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D100 = (5 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D120 = (6 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D140 = (7 << PMIC_BT_THH_TH4_SHIFT)
}
PMBatteryThreshold4Down;

//---- PMIC_OUT_CTL
typedef enum
{
    PMIC_OUT_CTL_V18_PWM  = (0 << PMIC_OUT_CTL_V18_SHIFT),  // default
    PMIC_OUT_CTL_V18_PFM  = (1 << PMIC_OUT_CTL_V18_SHIFT)
}
PMPfmPwmV18;

typedef enum
{
    PMIC_OUT_CTL_V12_PWM  = (0 << PMIC_OUT_CTL_V12_SHIFT),  // default
    PMIC_OUT_CTL_V12_PFM  = (1 << PMIC_OUT_CTL_V12_SHIFT)
}
PMPfmPwmV12;

typedef enum
{
    PMIC_OUT_CTL_V33_PWM  = (0 << PMIC_OUT_CTL_V33_SHIFT),  // default
    PMIC_OUT_CTL_V33_PFM  = (1 << PMIC_OUT_CTL_V33_SHIFT)
}
PMPfmPwmV33;

//---- PMIC_LED12_B4 / PMIC_LED12_B3 / PMIC_LED12_B2 / PMIC_LED12_B1
typedef enum
{
    PMIC_LED_BRT_OFF = 0,  // default
    PMIC_LED_BRT_14  = 1,
    PMIC_LED_BRT_28  = 2,
    PMIC_LED_BRT_43  = 3,
    PMIC_LED_BRT_57  = 4,
    PMIC_LED_BRT_71  = 5,
    PMIC_LED_BRT_85  = 6,
    PMIC_LED_BRT_100 = 7
}
PMLedBrightCommon;

typedef enum
{
    PMIC_LED12_B4_L1_OFF = (0 << PMIC_LED12_B4_L1_SHIFT),  // default
    PMIC_LED12_B4_L1_14  = (1 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_28  = (2 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_43  = (3 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_57  = (4 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_71  = (5 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_85  = (6 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_100 = (7 << PMIC_LED12_B4_L1_SHIFT)
}
PMLed1Bright4;

typedef enum
{
    PMIC_LED12_B4_L2_OFF = (0 << PMIC_LED12_B4_L2_SHIFT),  // default
    PMIC_LED12_B4_L2_14  = (1 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_28  = (2 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_43  = (3 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_57  = (4 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_71  = (5 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_85  = (6 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_100 = (7 << PMIC_LED12_B4_L2_SHIFT)
}
PMLed2Bright4;

typedef enum
{
    PMIC_LED12_B3_L1_OFF = (0 << PMIC_LED12_B3_L1_SHIFT),  // default
    PMIC_LED12_B3_L1_14  = (1 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_28  = (2 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_43  = (3 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_57  = (4 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_71  = (5 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_85  = (6 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_100 = (7 << PMIC_LED12_B3_L1_SHIFT)
}
PMLed1Bright3;

typedef enum
{
    PMIC_LED12_B3_L2_OFF = (0 << PMIC_LED12_B3_L2_SHIFT),  // default
    PMIC_LED12_B3_L2_14  = (1 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_28  = (2 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_43  = (3 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_57  = (4 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_71  = (5 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_85  = (6 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_100 = (7 << PMIC_LED12_B3_L2_SHIFT)
}
PMLed2Bright3;

typedef enum
{
    PMIC_LED12_B2_L1_OFF = (0 << PMIC_LED12_B2_L1_SHIFT),  // default
    PMIC_LED12_B2_L1_14  = (1 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_28  = (2 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_43  = (3 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_57  = (4 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_71  = (5 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_85  = (6 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_100 = (7 << PMIC_LED12_B2_L1_SHIFT)
}
PMLed1Bright2;

typedef enum
{
    PMIC_LED12_B2_L2_OFF = (0 << PMIC_LED12_B2_L2_SHIFT),  // default
    PMIC_LED12_B2_L2_14  = (1 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_28  = (2 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_43  = (3 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_57  = (4 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_71  = (5 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_85  = (6 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_100 = (7 << PMIC_LED12_B2_L2_SHIFT)
}
PMLed2Bright2;

typedef enum
{
    PMIC_LED12_B1_L1_OFF = (0 << PMIC_LED12_B1_L1_SHIFT),  // default
    PMIC_LED12_B1_L1_14  = (1 << PMIC_LED12_B1_L1_SHIFT),
    PMIC_LED12_B1_L1_28  = (2 << PMIC_LED12_B1_L1_SHIFT),
    PMIC_LED12_B1_L1_43  = (3 << PMIC_LED12_B1_L1_SHIFT),
    PMIC_LED12_B1_L1_57  = (4 << PMIC_LED12_B1_L1_SHIFT),
    PMIC_LED12_B1_L1_71  = (5 << PMIC_LED12_B1_L1_SHIFT),
    PMIC_LED12_B1_L1_85  = (6 << PMIC_LED12_B1_L1_SHIFT),
    PMIC_LED12_B1_L1_100 = (7 << PMIC_LED12_B1_L1_SHIFT)
}
PMLed1Bright5;

typedef enum
{
    PMIC_LED12_B1_L2_OFF = (0 << PMIC_LED12_B1_L2_SHIFT),  // default
    PMIC_LED12_B1_L2_14  = (1 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_28  = (2 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_43  = (3 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_57  = (4 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_71  = (5 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_85  = (6 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_100 = (7 << PMIC_LED12_B1_L2_SHIFT)
}
PMLed2Bright1;

//---- PMIC_LED_CTL ( PMIC_LED12_BLK )

typedef enum
{
    PMIC_LED_BLK_FREQ_033HZ = 0,  // default
    PMIC_LED_BLK_FREQ_050HZ = 1,
    PMIC_LED_BLK_FREQ_067HZ = 2,
    PMIC_LED_BLK_FREQ_1HZ   = 3,
    PMIC_LED_BLK_FREQ_2HZ   = 4,
    PMIC_LED_BLK_FREQ_4HZ   = 5
}
PMLedBlinkFreqCommon;

typedef enum
{
#if SDK_PMIC_REV!=0
    PMIC_LED12_BLK_FREQ_041HZ = (0 << PMIC_LED_CTL_L12_BLKF_SHIFT),
    PMIC_LED12_BLK_FREQ_082HZ = (1 << PMIC_LED_CTL_L12_BLKF_SHIFT),
    PMIC_LED12_BLK_FREQ_2Z    = (2 << PMIC_LED_CTL_L12_BLKF_SHIFT),  // default
    PMIC_LED12_BLK_FREQ_4HZ   = (3 << PMIC_LED_CTL_L12_BLKF_SHIFT)
#else // SDK_PMIC_REV==0
    PMIC_LED12_BLK_FREQ_033HZ = (0 << PMIC_LED12_BLK_FQ_SHIFT),  // default
    PMIC_LED12_BLK_FREQ_050HZ = (1 << PMIC_LED12_BLK_FQ_SHIFT),
    PMIC_LED12_BLK_FREQ_067HZ = (2 << PMIC_LED12_BLK_FQ_SHIFT),
    PMIC_LED12_BLK_FREQ_1HZ   = (3 << PMIC_LED12_BLK_FQ_SHIFT),
    PMIC_LED12_BLK_FREQ_2HZ   = (4 << PMIC_LED12_BLK_FQ_SHIFT),
    PMIC_LED12_BLK_FREQ_4HZ   = (5 << PMIC_LED12_BLK_FQ_SHIFT)
#endif // SDK_PMIC_REV==0
}
PMLed12BlinkFreq;

typedef enum
{
#if SDK_PMIC_REV!=0
    PMIC_LED12_BLK_DUTY_10 = (0 << PMIC_LED_CTL_L12_BLKD_SHIFT),
    PMIC_LED12_BLK_DUTY_25 = (1 << PMIC_LED_CTL_L12_BLKD_SHIFT),
    PMIC_LED12_BLK_DUTY_50 = (2 << PMIC_LED_CTL_L12_BLKD_SHIFT),  // default
    PMIC_LED12_BLK_DUTY_75 = (3 << PMIC_LED_CTL_L12_BLKD_SHIFT)
#else // SDK_PMIC_REV==0
    PMIC_LED12_BLK_DUTY_10 = (0 << PMIC_LED12_BLK_DT_SHIFT),  // default
    PMIC_LED12_BLK_DUTY_25 = (1 << PMIC_LED12_BLK_DT_SHIFT),
    PMIC_LED12_BLK_DUTY_50 = (2 << PMIC_LED12_BLK_DT_SHIFT),
    PMIC_LED12_BLK_DUTY_75 = (3 << PMIC_LED12_BLK_DT_SHIFT)
#endif // SDK_PMIC_REV==0
}
PMLed12BlinkDuty;

//---- PMIC_BL1_BRT / PMIC_BL2_BRT
typedef enum
{
    PMIC_BL_BRT_MIN = 0,  // default
    PMIC_BL_BRT_MAX = 0x1f,
    PMIC_BL_BRT_DEFAULT = 0x08
}
PMBackLightBrightness;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_PM_PM_REG_EX_H_ */
#endif
