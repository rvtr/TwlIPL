/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     Card.h
 *---------------------------------------------------------------------------*/
#ifndef __MY_CARD_H__
#define __MY_CARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include 	<twl.h>
#include 	<twl/os/common/format_rom.h>

// Define -------------------------------------------------------------------
#define KEY_BUF_SIZE						3			// Blowfishキーのバッファサイズ
#define DMA_NO						   		2			// 
#define BOOT_SEGMENT_SIZE 					0x1000		// Boot Segment領域のサイズ
#define SECURE_SEGMENT_SIZE					0x4000		// Secure領域のサイズ

#define VAE_VALUE							0xaaaaaa	// VAE (コマンド認証値(期待値))
#define VBI_VALUE							0x00000		// VBI (コマンドカウンタ 初期値)
#define VD_VALUE							0xdddddd	// VD  (PNジェネレータ   初期値)

#define PNA_BASE_VALUE						0x60e8		// 
#define PNB_L_VALUE							0x879b9b05	// 
#define PNB_H_VALUE							0x5c		// 

// コントロールレジスタ１ bit関連
#define START_FLG_MASK						0x80000000
#define READY_FLG_MASK						0x00800000

#define LATENCY1_SHIFT						0
#define LATENCY1_MASK						0x00001fff

#define DS_SHIFT							13
#define DS_MASK								0x00002000

#define SE_SHIFT							14
#define SE_MASK								0x00004000

#define SCR_SHIFT							15
#define SCR_MASK							0x00008000

#define LATENCY2_SHIFT						16
#define LATENCY2_MASK						0x003f0000

#define CS_SHIFT							22
#define CS_MASK								0x00400000

#define RDY_SHIFT							23
#define RDY_MASK							0x00800000

#define PC_SHIFT							24
#define PC_MASK								0x07000000

#define CT_SHIFT							27
#define CT_MASK								0x08000000

#define TRM_SHIFT							28
#define TRM_MASK							0x10000000

#define RESB_SHIFT							29
#define RESB_MASK							0x20000000

#define WR_SHIFT							30
#define WR_MASK								0x40000000

#define START_SHIFT							31
#define START_MASK							0x80000000

#define CNT1_FLD( start, wr, resb, rtm, ct, pc, rdy, cs, l2, scr, se, ds, l1 ) \
    (u32)( \
    	((u32)(start)<< START_SHIFT) 	| \
    	((u32)(wr) 	 << WR_SHIFT) 		| \
    	((u32)(resb) << RESB_SHIFT) 	| \
    	((u32)(rtm)	 << TRM_SHIFT) 		| \
    	((u32)(ct) 	 << CT_SHIFT) 		| \
    	((u32)(pc) 	 << PC_SHIFT) 		| \
    	((u32)(rdy)  << RDY_SHIFT) 		| \
        ((u32)(cs) 	 << CS_SHIFT) 		| \
    	((u32)(l2) 	 << LATENCY2_SHIFT) | \
    	((u32)(scr)  << SCR_SHIFT) 		| \
    	((u32)(se) 	 << SE_SHIFT) 		| \
    	((u32)(ds) 	 << DS_SHIFT) 		| \
    	((u32)(l1) 	 << LATENCY1_SHIFT) \
    )

#define CNT1_MSK( start, wr, resb, rtm, ct, pc, rdy, cs, l2, scr, se, ds, l1 ) \
    (u32)( ((start) ? START_MASK : 0) 	| \
           ((wr) 	? WR_MASK : 0) 	 	| \
           ((resb) 	? RESB_MASK : 0)  	| \
           ((rtm) 	? TRM_MASK : 0)   	| \
           ((ct) 	? CT_MASK : 0)    	| \
           ((pc) 	? PC_MASK : 0) 	  	| \
           ((rdy) 	? RDY_MASK : 0)   	| \
           ((cs) 	? CS_MASK : 0) 	  	| \
           ((l2) 	? LATENCY2_MASK : 0)| \
           ((scr) 	? SCR_MASK : 0) 	| \
           ((se) 	? SE_MASK : 0) 	  	| \
           ((ds) 	? DS_MASK : 0) 	  	| \
           ((l1) 	? LATENCY1_MASK : 0))



//#define USE_SLOT_A

#ifdef USE_SLOT_A
// Slot A
#define		SLOT_STATUS_MODE_SELECT_MSK			0x0c
#define		SLOT_STATUS_MODE_00					0x00
#define 	SLOT_STATUS_MODE_01					0x04
#define 	SLOT_STATUS_MODE_10					0x08
#define 	SLOT_STATUS_MODE_11					0x0c

#define 	reg_MCCNT1							reg_MI_MCCNT1_A

#define		reg_MCCMD0							reg_MI_MCCMD0_A
#define		reg_MCCMD1							reg_MI_MCCMD1_A

#define		reg_MCCNT0							reg_MI_MCCNT0_A
#define		reg_MCCNT1							reg_MI_MCCNT1_A

#define		reg_MCD1							reg_MI_MCD1_A

#define		reg_MCSCR0							reg_MI_MCSCR0_A
#define		reg_MCSCR1							reg_MI_MCSCR1_A
#define		reg_MCSCR2							reg_MI_MCSCR2_A

#else
// Slot B
#define		SLOT_STATUS_MODE_SELECT_MSK			0xc0
#define		SLOT_STATUS_MODE_00					0x00
#define 	SLOT_STATUS_MODE_01					0x40
#define 	SLOT_STATUS_MODE_10					0x80
#define 	SLOT_STATUS_MODE_11					0xc0

#define 	reg_MCCNT1							reg_MI_MCCNT1_B

#define		reg_MCCMD0							reg_MI_MCCMD0_B
#define		reg_MCCMD1							reg_MI_MCCMD1_B

#define		reg_MCCNT0							reg_MI_MCCNT0_B
#define		reg_MCCNT1							reg_MI_MCCNT1_B

#define		reg_MCD1							reg_MI_MCD1_B

#define		reg_MCSCR0							reg_MI_MCSCR0_B
#define		reg_MCSCR1							reg_MI_MCSCR1_B
#define		reg_MCSCR2							reg_MI_MCSCR2_B

#endif


// Enum ---------------------------------------------------------------------
typedef enum CardTypeEx{
	DS_CARD_TYPE_1 = 0,
    DS_CARD_TYPE_2,
    TWL_CARD
}CardTypeEx;

typedef enum NormalCommandType{
	RD_ID = 0,
    RD_BSEG,
    CHG_MODE
}NormalCommandType;

typedef enum SecureCommandType{
	S_RD_ID = 0,
    S_RD_SEG,
    S_PNG_ON,
    S_PNG_OFF,
    S_CHG_MODE
}SecureCommandType;

typedef enum GameCommandType{
	G_RD_ID = 0,
    G_RD_PAGE
}GameCommandType;

typedef enum CardType{
	CARD_DS_TYPE1 = 0,
    CARD_DS_TYPE2,
    CARD_TWL
}CardType;

// union ---------------------------------------------------------------------
typedef union
{
    u64 			dw;
    u8  			b[8];
}
GCDCmd64;

// ブートセグメントデータ
typedef union BootSegmentData
{
	ROM_Header			rh;
    u32					word[BOOT_SEGMENT_SIZE / sizeof(u32)];
}
BootSegmentData;

// struct -------------------------------------------------------------------
typedef struct BLOWFISH_CTX{
    u32 		P[16 + 2];
    u32 		S[4][256];
} BLOWFISH_CTX;

// カードブート時に必要な変数一式をまとめた構造体
typedef struct CardBootData{
	u16 				lockID;					
    
	u32					vae;					
    u32 				vbi;					
    u32					vd;						

    u32					id_nml;					
    u32					id_scr;					
    u32					id_gam;					

	u32					arm9StaticSize;			
	u32					arm7StaticSize;			
	u32					arm9LtdSize;			
    u32					arm7LtdSize;			
    
	BOOL			   	twlFlg;					
    
    u32					keyBuf[KEY_BUF_SIZE];	

	u64					secureSegNum;			

	CardTypeEx			cardType;
    
    BootSegmentData 	*pBootSegBuf;			
    u32 				*pSecureSegBuf;			

    BLOWFISH_CTX		keyTable;				
}
CardBootData;

// カード起動用関数
typedef struct CardBootFunction {
//	void (*ReadID_N)(CardBootData *cbd);	// ノーマルモードのID読みは全タイプで共通
	void (*ReadBootSegment_N)(CardBootData *cbd);
	void (*ChangeMode_N)(CardBootData *cbd);

	void (*ReadID_S)(CardBootData *cbd);
	void (*ReadSegment_S)(CardBootData *cbd);
	void (*SetPNG_S)(CardBootData *cbd);
	void (*ChangeMode_S)(CardBootData *cbd);

	void (*ReadID_G)(CardBootData *cbd);
    void (*ReadPage_G)(u32 addr, void* buf, u32 size);
}
CardBootFunction;


// Function prototype -------------------------------------------------------
// 活栓挿抜処理の初期化
void Cardm_Init(void);

// カード起動。Normalモード→Secureモード→Gameモードを行う
BOOL Card_Boot(void);

// ARM7,9の常駐モジュールを展開する関数
void Card_LoadStaticModule(void);

// Boot Segment バッファの指定
void Card_SetBootSegmentBuffer(void* buf, u32 size);

// Secure Segment バッファの指定
void Card_SetSecureSegmentBuffer(void* buf, u32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __MY_CARD_H__