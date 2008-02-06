/* $Id$ */
/*
 * Copyright (C) 1998-2003 RSA Security Inc. All rights reserved. 
 *
 * This work contains proprietary information of RSA Security. 
 * Distribution is limited to authorized licensees of RSA 
 * Security. Any unauthorized reproduction, distribution or 
 * modification of this work is strictly prohibited.
 */

#ifndef NO_AES

#include "aes.h"
#include "aes_sbox.h"

static const unsigned char Rcon[30]={
	0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,
	0x1b,0x36,0x6c,0xd8,0xab,0x4d,0x9a,0x2f,
	0x5e,0xbc,0x63,0xc6,0x97,0x35,0x6a,0xd4,
	0xb3,0x7d,0xfa,0xef,0xc5,0x91,
	};

int AES_set_key(AES_KEY *ctx, const unsigned char *key, int len)
	{
	int i,ii;
	AES_INT4 *W,tmp,tmp2;
	const unsigned char *ip;
	int words;

	if (len == 16)
		{
		i=10;
		words=4;
		}
	else if (len == 24)
		{
		i=12;
		words=6;
		}
	else if (len == 32)
		{
		i=14;
		words=8;
		}
	else
		return(1);

	ctx->rounds=i;
	ctx->key_size=words;
	W=(unsigned int *)ctx->ks;
	for (i=0; i<words; i+=2)
		{
		W[i+0]=	((AES_INT4)key[ 0]<<24)|
			((AES_INT4)key[ 1]<<16)|
			((AES_INT4)key[ 2]<< 8)|
			((AES_INT4)key[ 3]    );
		W[i+1]=	((AES_INT4)key[ 4]<<24)|
			((AES_INT4)key[ 5]<<16)|
			((AES_INT4)key[ 6]<< 8)|
			((AES_INT4)key[ 7]    );
		key+=8;
		}

	ip=Rcon;
	ii=4*(ctx->rounds+1);
	for (i=words; i<ii; i++)
		{
		tmp=W[i-1];
		if ((i % words) == 0)
			{
			tmp2 =(AES_INT4)aes_sbox[(tmp    )&0xff]<< 8;
			tmp2|=(AES_INT4)aes_sbox[(tmp>> 8)&0xff]<<16;
			tmp2|=(AES_INT4)aes_sbox[(tmp>>16)&0xff]<<24;
			tmp2|=(AES_INT4)aes_sbox[(tmp>>24)     ];
			tmp=tmp2^(((unsigned int)*ip)<<24);
			ip++;
			}
		if ((words == 8) && ((i % words) == 4))
			{
			tmp2 =(AES_INT4)aes_sbox[(tmp    )&0xff]    ;
			tmp2|=(AES_INT4)aes_sbox[(tmp>> 8)&0xff]<< 8;
			tmp2|=(AES_INT4)aes_sbox[(tmp>>16)&0xff]<<16;
			tmp2|=(AES_INT4)aes_sbox[(tmp>>24)     ]<<24;
			tmp=tmp2;
			}
		W[i]=W[i-words]^tmp;
		}
	return(0);
	}

#define rot1(x) (((x) << 24) | ((x) >> 8))
#define rot2(x) (((x) << 16) | ((x) >> 16))
#define rot3(x) (((x) <<  8) | ((x) >> 24))

/* This cute trick does 4 'mul by two' at once.  Stolen from
 * Dr B. R. Gladman <brg@gladman.uk.net> but I'm sure the u-(u>>7) is
 * a standard graphics trick
 * The key to this is that we need to xor with 0x1b if the top bit is set.
 * a 1xxx xxxx   0xxx 0xxx First we mask the 7bit,
 * b 1000 0000   0000 0000 then we shift right by 7 puting the 7bit in 0bit,
 * c 0000 0001   0000 0000 we then subtract (c) from (b)
 * d 0111 1111   0000 0000 and now we and with our mask
 * e 0001 1011   0000 0000
 */
#define mt  0x80808080
#define ml  0x7f7f7f7f
#define mh  0xfefefefe
#define mm  0x1b1b1b1b
#define mul2(x,t)	((t)=((x)&mt), \
			((((x)+(x))&mh)^(((t)-((t)>>7))&mm)))

#define mix_col(x,f2,f3) (\
			(f2)=mul2(x,f3), \
			(f3)=(x)^(f2), \
			(rot3(f3) ^ rot2(x) ^ rot1(x)^(f2)))

#define inv_mix_col(x,f2,f4,f8,f9) (\
			(f2)=mul2(x,f2), \
			(f4)=mul2(f2,f4), \
			(f8)=mul2(f4,f8), \
			(f9)=(x)^(f8), \
			(f8)=((f2)^(f4)^(f8)), \
			(f2)^=(f9), \
			(f4)^=(f9), \
			(f8)^=rot3(f2), \
			(f8)^=rot2(f4), \
			(f8)^rot1(f9))

void AES_convert_key(AES_KEY *ctx)
	{
	int i;
	AES_INT4 *k,w,t1,t2,t3,t4;

	k= ctx->ks;
	k+=4;
	for (i=ctx->rounds*4; i>4; i--)
		{
		w= *k;
		w = inv_mix_col(w,t1,t2,t3,t4);
		*k++ =w;
		}
	}
#endif /* NO_AES */
