/********************************************************************/
/*      myFunc.c                                                    */
/*          NITRO-IPL                                               */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	標準サブルーチン群


	$Log: myFunc.c,v $
	Revision 1.1.4.1  2006/02/01 06:29:37  yosiokat
	mf_strcpyの仕様変更。
	
	Revision 1.1  2005/02/08 04:41:25  yosiokat
	新規追加。
	
	Revision 1.2  2004/05/19 08:25:28  yosiokat
	更新ログの追加。
	

*/

#include "myFunc.h"

// define data----------------------------------
#define CHAR_INDEX		0x200	// キャラベース0x4000


// function's prototype-------------------------
void mf_copyBuffer(u8 *srcp,u8 *dstp,u16 length);

// extern data----------------------------------
//extern u16 bgBakS[32*24];											// BG  バックアップ
//extern u32 oamBakS[128*2];											// OAM バックアップ


// const data-----------------------------------
const u16	csr_charList1[]  ={0x008c, 0x008c, 0x008c, 0x008c};
const u8	str_time_period[]={"    .  ."};


// global variable------------------------------
MyTime		myTime;
//KeyWork	pad;

// static variable------------------------------
static u16	csr_animeCount, csr_animeCharNum;
static u16	key_rapid[4];
static u16	blinkCount;
static u16	csr_pos_x, csr_pos_y, csr_add_y;


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
	if((int)srcp & 0x00000001)	return (u16)(*(u16 *)((int)srcp ^ 0x00000001) >> 8);
	else						return (u16)(*(u16 *)(srcp) & 0x00ff);
}


// 初期化
void mf_init(void)
{
	mf_KEYPAD_initRapid();
	mf_CSR_init(0,0,0);
	mf_BLINK_initCounter();
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
			*dstp++	=(u16)( charCode | color | CHAR_INDEX );
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
		*dstp--=(u16)( charCode | color | CHAR_INDEX );
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
		*dstp++=(u16)( color | data16 | CHAR_INDEX );
	}
}


// 2byte文字列データ表示（0x0100以降の1文字に2byte必要な文字列を描画する
void mf_drawString2(u16 pos_x,u16 pos_y, u16 color, const u16 *strp)
{
	u16 *dstp=calcBgOffset(pos_x,pos_y);
	
	while(*strp) *dstp++ = (u16)( *strp++ | color | CHAR_INDEX );
}


// 矩形BGクリア
void mf_clearRect(u16 pos_x,u16 pos_y,u8 height,u8 width)
{
	u16 i,j;
	u16 *dstp=calcBgOffset(pos_x,pos_y);
	
	for(i=0;i<height;i++) {
		for(j=0;j<width;j++) *dstp++ = (u16)( 0x0000 );
		dstp+=0x20-width;
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


// Xframeウェイト
void mf_waitXframe(u16 frame)
{
	BOOL prelIRQ=OS_EnableIrq();
	while(frame--) SVC_WaitVBlankIntr();
    (void)OS_RestoreIrq(prelIRQ);
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
	mf_drawDecimal((u16)(pos_x+4-keta), pos_y, color, &myTime.hour,keta,2);
	mf_drawDecimal((u16)(pos_x+5),		 pos_y, color, &myTime.minute,2,1);
	mf_drawDecimal((u16)(pos_x+8),		 pos_y, color, &myTime.second,2,1);
}

// 文字列コピー
u8 *mf_strcpy(const u8 *str1p,u8 *str2p)
{
	while(*str1p) *str2p++=*str1p++;
	*str2p=0x00;
	return str2p;
}


// 文字列比較
u8 mf_strcmp(const u8 *str1p,const u8 *str2p)
{
	while(*str1p) {
		if(*str1p++!=*str2p++) return 1;
	}
	return 0;
}
