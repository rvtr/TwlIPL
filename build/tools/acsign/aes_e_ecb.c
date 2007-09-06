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


void AES_ecb_encrypt_com(AES_KEY *key, unsigned char *out,
     const unsigned char *in, AES_ENC_DEC_FN enc_fn)
	{
	AES_INT4 data[4],w;

	data[0]=((in[ 0]<<24)|(in[ 1]<<16)|(in[ 2]<<8)|(in[ 3]));
	data[1]=((in[ 4]<<24)|(in[ 5]<<16)|(in[ 6]<<8)|(in[ 7]));
	data[2]=((in[ 8]<<24)|(in[ 9]<<16)|(in[10]<<8)|(in[11]));
	data[3]=((in[12]<<24)|(in[13]<<16)|(in[14]<<8)|(in[15]));
	enc_fn(key,&(data[0]));
	w=data[0];
	out[ 0]=(w>>24)&0xff;
	out[ 1]=(w>>16)&0xff;
	out[ 2]=(w>> 8)&0xff;
	out[ 3]=(w    )&0xff;
	w=data[1];
	out[ 4]=(w>>24)&0xff;
	out[ 5]=(w>>16)&0xff;
	out[ 6]=(w>> 8)&0xff;
	out[ 7]=(w    )&0xff;
	w=data[2];
	out[ 8]=(w>>24)&0xff;
	out[ 9]=(w>>16)&0xff;
	out[10]=(w>> 8)&0xff;
	out[11]=(w    )&0xff;
	w=data[3];
	out[12]=(w>>24)&0xff;
	out[13]=(w>>16)&0xff;
	out[14]=(w>> 8)&0xff;
	out[15]=(w    )&0xff;
	}

#endif /* NO_AES */
