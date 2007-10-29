/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     myFunc.c

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

#include "misc.h"

// define data----------------------------------

// function's prototype-------------------------

// extern data----------------------------------
//extern u16 bgBakS[32*24];											// BG  バックアップ
//extern GXOamAttr oamBakS[128];										// OAM バックアップ


// const data-----------------------------------
//const u16	csr_charList1[]  ={0x008c, 0x008c, 0x008c, 0x008c};
//const u8	str_time_period[]={"    .  ."};


// global variable------------------------------
int 		(*nowProcess)(void);
//MyTime		myTime;
//KeyWork		pad;

// static variable------------------------------
static u16	key_rapid[4];
//static u16	csr_animeCount, csr_animeCharNum;
//static u16	csr_pos_x, csr_pos_y, csr_add_y;
//static u16	blinkCount;


// ============================================================================
// function's description
// ============================================================================

// BgBakオフセット値算出
__inline static u16 *calcBgOffset(u16 pos_x,u16 pos_y)
{
	return bgBakS+pos_x+(pos_y<<5);
}


// メインメモリからでも大丈夫な形式での1byteのデータ読み出し。
__inline static u16 ReadByteHWBus(const void *srcp)
{
	if( (u32)srcp & 0x00000001 ) return (u16)(*(u16 *)( (u32)srcp ^ 0x00000001 ) >> 8 );
	else						 return (u16)(*(u16 *)( srcp ) & 0x00ff );
}


// 初期化
void mf_init(void)
{
	mf_KEYPAD_initRapid();
//	mf_BLINK_initCounter();
}


/*
// キー入力読み出し
void mf_KEYPAD_read(void)
{
	u16 ReadData= PAD_Read();
	pad.trg	= (u16)(ReadData & (ReadData ^ pad.cont));		// トリガ 入力
	pad.cont= ReadData;											//   ベタ 入力
}
*/


// キー連射入力処理　初期化（十字キーのみ）
void mf_KEYPAD_initRapid(void)
{
	u16 *krp=key_rapid;
	
	*krp++=0;
	*krp++=0;
	*krp++=0;
	*krp++=0;
}


// キー連射入力（十字キーのみ）
void mf_KEYPAD_rapid(void)
{
	u16 mask,i;
	
	for(i=0;i<4;i++){
		mask=(u16)(0x0001<<(i+4));
		if(pad.cont & mask){
			key_rapid[i]++;
			if(key_rapid[i]==30) {
				pad.trg|=mask;
				key_rapid[i]=20;
			}
		}else{
			key_rapid[i]=0;
		}
	}
}


// Xframeウェイト
void mf_waitXframe(u16 frame)
{
	BOOL prelIRQ=OS_EnableIrq();
	while(frame--) OS_WaitIrq(1, OS_IE_V_BLANK);
    (void)OS_RestoreIrq(prelIRQ);
}



// 矩形BGクリア
void mf_clearRect(u16 pos_x,u16 pos_y,u8 height,u8 width)
{
	u16 i,j;
	u16 *dstp=calcBgOffset(pos_x,pos_y);
	
	for(i=0;i<height;i++) {
		for(j=0;j<width;j++) *dstp++=0x0020;
		dstp+=0x20-width;
	}
}


/*
// 10進データ表示
void mf_drawDecimal(u16 pos_x,u16 pos_y,u16 color,const void *valuep,u8 drawLength,u8 size)
{
	u16 count,charCode;
	u16 *dstp;
	u32 mask,divisor,target;
	
	mask=0xff;
	while(--size>0)	mask=(mask<<8)|0xff;
	target=(*(u32 *)valuep)&mask;
	count=10;
	divisor=1000000000;
	dstp=calcBgOffset(pos_x,pos_y);
	while(count) {
		CP_SetDiv32_32(target, divisor);
		if (count<=drawLength) {
			charCode=(u16)(CP_GetDivResult32()+0x0030);
			*dstp++	=(u16)(charCode | color);
		}
		target=(u32)CP_GetDivRemainder32();
		CP_SetDiv32_32(divisor, 10);
		divisor=(u32)CP_GetDivResult32();
		count--;
	}
}


// 16進データ表示
void mf_drawHex(u16 pos_x,u16 pos_y,u16 color,const void *valuep,u8 drawLength)
{
	u16	count,charCode;
	u16	*dstp=calcBgOffset(pos_x,pos_y)+drawLength-1;
	
	for(count=0;count<drawLength;count++){
		charCode=ReadByteHWBus(valuep);
		if (count & 0x01){
			charCode=(u16)((charCode>>4) & 0x000f);
			((u8 *)valuep)++;
		}else{
			charCode=(u16)( charCode     & 0x000f);
		}
		if (charCode<0x000a)	charCode+=0x0030;
		else					charCode+=0x0041-0x000a;
		*dstp--=(u16)(charCode | color);
	}
}


// 1byte文字列データ表示（0x00～0xffまでの1文字が1byteで良い文字列を描画する
void mf_drawString(u16 pos_x,u16 pos_y, u16 color, const u8 *strp)
{
	u16 data16;
	u16 *dstp=calcBgOffset(pos_x,pos_y);
	
	while(1) {
		data16=ReadByteHWBus(strp++);
		if(data16==0) break;
		*dstp++=(u16)(color | data16);
	}
}


// 2byte文字列データ表示（0x0100以降の1文字に2byte必要な文字列を描画する
void mf_drawString2(u16 pos_x,u16 pos_y, u16 color, const u16 *strp)
{
	u16 *dstp=calcBgOffset(pos_x,pos_y);
	
	while(*strp) *dstp++ = (u16)(*strp++ | color);
}

// 矩形フレーム描画
void mf_drawRectFrame(u16 pos_x,u16 pos_y,u16 color,u8 height,u8 width)
{
	u16 i,j,code;
	u16 *dstp;
	
	for(i=0;i<height;i++) {
		dstp=calcBgOffset(pos_x,pos_y);
		dstp+=(i<<5);
		if(i==0){
			code=0x0132;
		}else if(i==height-1){
			code=0x0136;
		}else{
			code=0x0135;
		}
		*dstp++=(u16)(color | code);
		if((i==0)||(i==height-1)){
			for(j=1;j<width-1;j++) *dstp++=(u16)(color | 0x0133);
		}else{
			dstp+=width-2;
		}
		*dstp++=(u16)(color | code | 0x0400);		// BG_SC_H_FLIP
	}
}


// カーソル初期化
void mf_CSR_init(u16 pos_x,u16 pos_y,u16 add_y)
{
	csr_pos_x=pos_x;
	csr_pos_y=pos_y;
	csr_add_y=add_y;
}


// カーソル移動＆アニメーション
void mf_CSR_moveAndAnime(int nowNum)
{
	u16 *oamp=(u16 *)&oamBakS[0];
	
	// move
	oamp[0]=(u16)(csr_pos_y + nowNum*csr_add_y);
	oamp[1]=csr_pos_x;
	// anime
	mf_CSR_anime(csr_charList1);
}


// カーソルアニメーション
void mf_CSR_anime(const u16 *csr_charListp)
{
	u16 *oamp=(u16 *)&oamBakS[0];
	
	// anime
	if((++csr_animeCount & 0x0f)==0) {
		csr_animeCharNum=(u16)((csr_animeCharNum+1) & 0x03);
	}
	oamp[2]=(u16)(RED | csr_charListp[csr_animeCharNum]);
}


// メッセージ点滅カウンター　初期化
void mf_BLINK_initCounter(void)
{
	blinkCount=0;
	mf_clearRect(0,20,1,0x0020);
}


// 文字列点滅表示
void mf_BLINK_drawString(u16 pos_x, u16 pos_y, u16 color, const u8 *strp)
{
	u16 len=0;
	
	blinkCount++;
	if((blinkCount & 0x60)==0x60) {
		while(*strp++) len++;
		mf_clearRect(pos_x,pos_y,1,(u8)len);
	}else {
		mf_drawString(pos_x,pos_y,color,strp);
	}
}


// 時間表示
void mf_TIME_init(void)
{
	BOOL prelIRQ=OS_EnableIrq();
	myTime.frame=0;
	myTime.hour=0;
	myTime.minute=0;
	myTime.second=0;
	myTime.enable=0;
	(void)OS_RestoreIrq(prelIRQ);
}


void mf_TIME_start(int init_flag)
{
	if(init_flag) mf_TIME_init();
	myTime.enable=1;
}


void mf_TIME_stop(void)
{
	myTime.enable=2;
}


void mf_TIME_count(void)
{
	if(myTime.enable!=1) return;
	
	myTime.frame++;
	if(myTime.frame==60) {
		myTime.frame=0;
		myTime.second++;
		if(myTime.second==60) {
			myTime.second=0;
			myTime.minute++;
			if(myTime.minute==60) {
				myTime.minute=0;
				myTime.hour++;
			}
		}
	}
}


void mf_TIME_draw(u16 pos_x, u16 pos_y, u16 color)
{
	u8 keta;
	
	if(myTime.enable==0) {
		return;
	}else if(myTime.enable==2){
		color=RED;
	}
	
	mf_drawString(pos_x,pos_y,color,str_time_period);
	
	if(myTime.hour<10){
		keta=1;
	}else if(myTime.hour<100){
		keta=2;
	}else if(myTime.hour<1000){
		keta=3;
	}else{
		keta=4;
	}
	mf_drawUInt((u16)(pos_x+4-keta), pos_y, color, &myTime.hour,keta,2);
	mf_drawUInt((u16)(pos_x+5),		 pos_y, color, &myTime.minute,2,1);
	mf_drawUInt((u16)(pos_x+8),		 pos_y, color, &myTime.second,2,1);
}


// 文字列コピー
void mf_strcpy(const u8 *str1p,u8 *str2p)
{
	while(*str1p) *str2p++=*str1p++;
	*str2p=0x00;
}


// 文字列比較
u8 mf_strcmp(const u8 *str1p,const u8 *str2p)
{
	while(*str1p) {
		if(*str1p++!=*str2p++) return 1;
	}
	return 0;
}
*/
