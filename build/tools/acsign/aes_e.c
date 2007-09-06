/* $Id$ */
/*
 * Copyright (C) 1998-2003 RSA Security Inc. All rights reserved. 
 *
 * This work contains proprietary information of RSA Security. 
 * Distribution is limited to authorized licensees of RSA 
 * Security. Any unauthorized reproduction, distribution or 
 * modification of this work is strictly prohibited.
 */

/**
 * @file aes_e.c
 * \brief Large code size (thus fast) AES Encryption
 */

#ifndef NO_AES

#include "aes.h"

/* Include the Large aes_submix tables */
#include "aes_e.h"

/* AES ECB Encryption of 1 x 128 bit Block */
void AES_encrypt(AES_KEY *ctx, AES_INT4 *data)
{
	int r;
	int rounds; 
	AES_INT4 *k;
	AES_INT4 o0,o1,o2,o3;
	AES_INT4 tmp0,tmp1,tmp2,tmp3;

	rounds=ctx->rounds;
	k= (AES_INT4 *)ctx->ks;

	/* Encrypt one block. 
	 */
	tmp0=data[0];
	tmp1=data[1];
	tmp2=data[2];
	tmp3=data[3];
/*printf("K  %08X %08X %08X %08X\n",tmp0,tmp1,tmp2,tmp3);
printf("KS %08X %08X %08X %08X\n",k[0],k[1],k[2],k[3]); */
	tmp0^=k[0];
	tmp1^=k[1];
	tmp2^=k[2];
	tmp3^=k[3];
	k+=4;

	for(r = 1; r < rounds; r++)
		{
/*printf("%2d %08X %08X %08X %08X\n",r,tmp0,tmp1,tmp2,tmp3); */
		o0 =	aes_submix[3][(tmp3    )&0xff]^
			aes_submix[2][(tmp2>> 8)&0xff]^
			aes_submix[1][(tmp1>>16)&0xff]^
			aes_submix[0][(tmp0>>24)&0xff];

		o1 =	aes_submix[3][(tmp0    )&0xff]^
			aes_submix[2][(tmp3>> 8)&0xff]^
			aes_submix[1][(tmp2>>16)&0xff]^
			aes_submix[0][(tmp1>>24)&0xff];

		o2 =	
			aes_submix[3][(tmp1    )&0xff]^
			aes_submix[2][(tmp0>> 8)&0xff]^
			aes_submix[1][(tmp3>>16)&0xff]^
			aes_submix[0][(tmp2>>24)&0xff];

		o3 =
			aes_submix[3][(tmp2    )&0xff]^
			aes_submix[2][(tmp1>> 8)&0xff]^
			aes_submix[1][(tmp0>>16)&0xff]^
			aes_submix[0][(tmp3>>24)&0xff];

/*
printf("   %08X %08X %08X %08X\n",o0,o1,o2,o3); 
printf("KS %08X %08X %08X %08X\n",k[0],k[1],k[2],k[3]);  */
		tmp0=o0^k[0];
		tmp1=o1^k[1];
		tmp2=o2^k[2];
		tmp3=o3^k[3];
		k+=4;
		}
/*printf("O  %08X %08X %08X %08X\n",o0,o1,o2,o3); */
	o0 =	(aes_submix[0][(tmp3    )&0xff]>> 8)&0xFF;
	o1 =	(aes_submix[0][(tmp0    )&0xff]>> 8)&0xFF;
	o2 =	(aes_submix[0][(tmp1    )&0xff]>> 8)&0xFF;
	o3 =	(aes_submix[0][(tmp2    )&0xff]>> 8)&0xFF;

	o0|=	(aes_submix[0][(tmp2>> 8)&0xff]    )&0xFF00;
	o1|=	(aes_submix[0][(tmp3>> 8)&0xff]    )&0xFF00;
	o2|=	(aes_submix[0][(tmp0>> 8)&0xff]    )&0xFF00;
	o3|=	(aes_submix[0][(tmp1>> 8)&0xff]    )&0xFF00;

	o0|=	(aes_submix[0][(tmp1>>16)&0xff]<< 8)&0xFF0000;
	o1|=	(aes_submix[0][(tmp2>>16)&0xff]<< 8)&0xFF0000;
	o2|=	(aes_submix[0][(tmp3>>16)&0xff]<< 8)&0xFF0000;
	o3|=	(aes_submix[0][(tmp0>>16)&0xff]<< 8)&0xFF0000;

	o0|=	(aes_submix[0][(tmp0>>24)     ]<<16)&0xFF000000;
	o1|=	(aes_submix[0][(tmp1>>24)     ]<<16)&0xFF000000;
	o2|=	(aes_submix[0][(tmp2>>24)     ]<<16)&0xFF000000;
	o3|=	(aes_submix[0][(tmp3>>24)     ]<<16)&0xFF000000;
/*printf("P  %08X %08X %08X %08X\n",o0,o1,o2,o3); */
	o0^=k[0];
	o1^=k[1];
	o2^=k[2];
	o3^=k[3];
/*printf("Q  %08X %08X %08X %08X\n",o0,o1,o2,o3); */
	data[0]= o0;
	data[1]= o1;
	data[2]= o2;
	data[3]= o3;
	}

#endif /* NO_AES */
