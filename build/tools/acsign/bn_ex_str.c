/* $Id$ */
/*
 * Copyright (C) 1998-2002 RSA Security Inc. All rights reserved.
 *
 * This work contains proprietary information of RSA Security.
 * Distribution is limited to authorized licensees of RSA
 * Security. Any unauthorized reproduction, distribution or
 * modification of this work is strictly prohibited.
 *
 */
#include "bn_lcl.h"

const static unsigned char p2 []={0,8,1,1,1,1,1,0,0,0};
const static unsigned char p4 []={0,8,1,1,1,4,1,0,0,0};
const static unsigned char p16[]={0,8,1,1,1,16,1,0,0,0};

/* The following defines allow for redefinition of the window size
 * of the exponent string at compile time, this affects the size
 * of temporary data required in montgomery operations.
 * Larger window sizes have more memory and are slightly faster
 */
#ifndef MAX_WIN_SIZE
#define MAX_WIN_SIZE 5
#endif

#if (MAX_WIN_SIZE == 6)
#define MAX_NUM_SIZE 16
#endif

#if (MAX_WIN_SIZE == 5)
#define MAX_NUM_SIZE 16
#endif

#if (MAX_WIN_SIZE == 4)
#define MAX_NUM_SIZE 8
#endif

#if (MAX_WIN_SIZE == 3)
#define MAX_NUM_SIZE 4
#endif

#ifndef MAX_NUM_SIZE
#define MAX_NUM_SIZE 16
#endif

/* This table is used to calculate how far to shift a window to find
 * the next 1 bit within the window, for a given window size
 * Comment next to each value represents window size, value of the window
 * and the number of shifts to find the next 1 bit.
 * where the value of the window is 0, the shift is the size of the window
 * and thus may not necessary yield a 1 bit, but refreshes the window
 */
const static unsigned char shift[64]=
                {6,   /* window 6, bits 000000, shift 6 */
                 0,   /*        6, bits 000001, shift 0 */
                 1,   /*        6, bits 000010, shift 1 */
                 0,   /*        6, bits 000011, shift 0 */
                 2,   /*        6, bits 000100, shift 2 */
                 0,   /*        6, bits 000101, shift 0 */
                 1,   /*        6, bits 000110, shift 1 */
                 0,   /*        6, bits 000111, shift 0 */
                 3,   /*        6, bits 001000, shift 3 */
                 0,   /*        6, bits 001001, shift 0 */
                 1,   /*        6, bits 001010, shift 1 */
                 0,   /*        6, bits 001011, shift 0 */
                 2,   /*        6, bits 001100, shift 2 */
                 0,   /*        6, bits 001101, shift 0 */
                 1,   /*        6, bits 001110, shift 1 */
                 0,   /*        6, bits 001111, shift 0 */
                 4,   /*        6, bits 010000, shift 4 */
                 0,   /*        6, bits 010001, shift 0 */
                 1,   /*        6, bits 010010, shift 1 */
                 0,   /*        6, bits 010011, shift 0 */
                 2,   /*        6, bits 010100, shift 2 */
                 0,   /*        6, bits 010101, shift 0 */
                 1,   /*        6, bits 010110, shift 1 */
                 0,   /*        6, bits 010111, shift 0 */
                 3,   /*        6, bits 011000, shift 3 */
                 0,   /*        6, bits 011001, shift 0 */
                 1,   /*        6, bits 011010, shift 1 */
                 0,   /*        6, bits 011011, shift 0 */
                 2,   /*        6, bits 011100, shift 2 */
                 0,   /*        6, bits 011101, shift 0 */
                 1,   /*        6, bits 011110, shift 1 */
                 0,   /*        6, bits 011111, shift 0 */
                 5,   /*        6, bits 100000, shift 5 */
                 /* also window 5, bits 00000, shift 5  */
                 0,  /*         5 ,bits 00001, shift 0  */ 
                 1,  /*         5 ,bits 00010, shift 1  */
                 0,  /*         5 ,bits 00011, shift 0  */
                 2,  /*         5 ,bits 00100, shift 2  */
                 0,  /*         5 ,bits 00101, shift 0  */
                 1,  /*         5 ,bits 00110, shift 1  */
                 0,  /*         5 ,bits 00111, shift 0  */
                 3,  /*         5 ,bits 01000, shift 3  */
                 0,  /*         5 ,bits 01001, shift 0  */
                 1,  /*         5 ,bits 01010, shift 1  */
                 0,  /*         5 ,bits 01011, shift 0  */
                 2,  /*         5 ,bits 01100, shift 2  */
                 0,  /*         5 ,bits 01101, shift 0  */
                 1,  /*         5 ,bits 01110, shift 1  */
                 0,  /*         5 ,bits 01111, shift 0  */
                 4,  /*         5 ,bits 10000, shift 1  */
             /* also         window 4, bits 0000, shift 4   */
                 0,  /*         4 ,bits 0001, shift 0   */
                 1,  /*         4 ,bits 0010, shift 1   */
                 0,  /*         4 ,bits 0011, shift 0   */
                 2,  /*         4 ,bits 0100, shift 2   */
                 0,  /*         4 ,bits 0101, shift 0   */
                 1,  /*         4 ,bits 0110, shift 1   */
                 0,  /*         4 ,bits 0111, shift 0   */
                 3,  /*         4 ,bits 1000, shift 3   */
            /* also          window 3, bits 000, shift 3    */
                 0,  /*         3 ,bits 001, shift 0    */
                 1,  /*         3 ,bits 010, shift 1    */
                 0,  /*         3 ,bits 011, shift 0    */
                 2,  /*         3 ,bits 100, shift 2    */
                 0,  /*         3 ,bits 101, shift 0    */
                 1,  /*         3 ,bits 110, shift 1    */
                 0   /*         3 ,bits 111, shift 0    */
                };

/* This table defines the starting point in the shift table for
 * a particular window size
 */
const static unsigned char *shift_val[7]=
    {
    &(shift[63]), /* window 0 - unused */
    &(shift[62]), /* window 1 */
    &(shift[60]), /* window 2 - unused */
    &(shift[56]), /* window 3 */
    &(shift[48]), /* window 4 */
    &(shift[32]), /* window 5 */
    &(shift[ 0]), /* window 6 - unused */
    };

/**
 * Calculates a Montgomery exponent string.
 *
 * For a supplied exponent p, generate an exponent string strp, which
 * defines in pairs the number of multiplies and square operations
 * required by a particular bit pattern, commonly used exponents
 * 3, 11 and F4 have predefined constant string values, the rest
 * are calculated into a cast unsigned char * array via the data
 * pointer of a BIGNUM taken from the BN_CTX stack of BIGNUMs 
 * 
 * @param    p       [In]  Exponent
 * @param    strp    [Out] Exponent string result
 * @param    flags   [In]  Unused
 * @param    ctx     [In]  Temporary data storage
 *
 * @pre      p, and ctx are initialised and valid
 * @post     strp points to required exponent string
 *
 * @notes    String length value in strp[2] is invalid for strings
 *           greater than length 255
 *           string terminates with pattern, 0, 0 this should be
 *           used in accurately determining the length of a returned
 *           string strp.
 *
 * @note     strings with value sqr = 255, mul = 0, sqr value should be
 *           treated as value 256, and added to the next sqr value, this
 *           is used by exponent strings where more then 256 contiguous
 *           zero bits are in the exponent bit representation.
 *
 * @note     code contains conditional compilation of code dependent on
 *           the OS int/long sizes
 */
int BN_gen_exp_bits(p,strp,flags,ctx)
BIGNUM *p;
unsigned char **strp;
int flags;
BN_CTX *ctx;
    {
    int bits,i,j,window,num;
    unsigned char *str=NULL;
    BIGNUM *tmp;

    flags=flags;
    bits=p->top*BN_BITS2;
    tmp=&(ctx->bn[ctx->tos]);
    if (p->top == 0)
        return(0);

#if (BN_BITS2 > 17)
    if (p->top == 1)
#else
    if (bits <= 32)
#endif
        {
#if (BN_BITS2 > 17)
        if ((p->top == 1) && (p->d[0] == 0x10001))
            str=(unsigned char *)p16;
#endif
#if (16 >= BN_BITS2) && (BN_BITS > 8)
        if (    (p->top == 2) && 
            (p->d[0] == 0x0001) &&
            (p->d[1] == 0x0001))
            str=(unsigned char *)p16;
#endif
#if (8 >= BN_BITS)
        if (    (p->top == 3) &&
            (p->d[0] == 0x01) &&
            (p->d[1] == 0x00) &&
            (p->d[2] == 0x01))
            str=(unsigned char *)p16;
#endif
        else if ((p->d[0] == 0x11) && (p->top == 1))
            str=(unsigned char *)p4;
        else if ((p->d[0] == 0x3) && (p->top == 1))
            str=(unsigned char *)p2;
        window=1;
        num=1;
        i=BN_BITS2;
        }
    else if (bits >= 256)
        {
        window=MAX_WIN_SIZE;    /* max size of window */
        num=MAX_NUM_SIZE;
        i=(BN_BITS2+(MAX_WIN_SIZE -1))/MAX_WIN_SIZE;
        }
    else if (bits >= 128)
        {
        window=4;
        num=8;
        i=(BN_BITS2+3)/4;
        }
    else /* 128 to 33 */
        {
        window=3;
        num=4;
        i=(BN_BITS2+2)/3;
        }

    /* Number of tmp words */
    j=(p->top*i*2+BN_BYTES-1+4)/BN_BYTES;

    if (str == NULL)
        {
        if (!bn_wexpand(tmp,j))
            return(0);
        str=(unsigned char *)tmp->d;
        i=BN_gen_exp_string(&(str[4]),p,window);
        i+=2;
        str[0]=(unsigned char)((i>>8)&0xff);
        str[1]=(unsigned char)((i   )&0xff);
        str[2]=(unsigned char)window;
        str[3]=(unsigned char)num;
        }
    else
        {
        i=8;
        }
    *strp=str;
    return(i+2);
    }


/**
 * Generates the Montgomery exponent string.
 *
 * This function is used to generate an 'exponent string'
 * which is an array of bytes that encode how to perform the steps in
 * the a^p%m operation.
 * 
 * @param str   [Out] Containing the generated string
 * @param p     [In]  Exponent to generate the string for
 * @param bits  [In]  Size of the window for shifting the values of the BIGNUM
 *
 * @pre         p is initialised and value BIGNUM, bits is not 0
 * @post        str points to generated exponent string
 *
 * @note        str is cast assigned the data of a BIGNUM allocated and 
 *              expanded from the BN_CTX of the calling function BN_gen_exp_bits
 *              it does not need to be de-allocated
 *
 * @note        string consisted of unsigned char pairs and 4 byte init
 *              pairs are sqr count and multiply, where strings are greater
 *              than 256 bit, length in position str[2] is invalid
 *
 * @note        strings with value sqr = 255, mul = 0, sqr value should be
 *              treated as value 256, and added to the next sqr value, this
 *              is used by exponent strings where more then 256 contiguous
 *              zero bits are in the exponent bit representation.
 *
 */
int BN_gen_exp_string(str,p,bits)
unsigned char *str;
BIGNUM *p;
int bits;
    {
    unsigned char *sp;
    unsigned int mask;
    const unsigned char *shift;
    BN_ULONG w,wh,wl,*d;
    unsigned int i,mul,sqr,t,s,ss;
    int top;

    if (bits > 6) bits=6;
    shift= shift_val[bits];

    /* This is the mask for the bits we wish to operate on */
    mask=(1<<bits)-1;

    ss=0;
    sp= &(str[((p->top*BN_BITS2+bits-1)/bits)*2+2]);
    *sp-- = 0;
    *sp-- = 0;
    top=p->top;         /* Total words we will shift in */
    d=p->d;
    w=wl= *d++;
    if (top <= 1)
        wh=0;
    else
        wh= *d++;
    sqr=0;
    i=0;

    for (;;)
        {
        /* t will contain how far we need to shift to set a 1
         * in the bottom bit. */
        for (;;)
            {
            t=w&mask;                     /* retrieve our window */
            s=shift[t];    /* get the shift value for the window */
            if (s == 0) break;    /* no shift write out the vals */
            sqr+=s;         /* add the shifted zero count to sqr */
            ss+=s;                   /* ss is total shift for wl */
            if (ss >= BN_BITS2)    /* we have shifted > word len */
                {
                if (top <= 1) break;        /* no more to do */
                top--;                  /* dec the count */

                wl=wh;                 /* copy the next word */
                      /* load the word after or 0 if no more */
                wh=(top <= 1)?0:(*d++); 
                          /* adjust our shift by len of word */
                ss-=BN_BITS2;        
                }
            /* reset our window word w */
            if (ss == 0)
                w=wl;
            else
                w=(wl>>ss)|(wh<<(BN_BITS2-ss));
            }

        /* At this point we have the 0th bit set */
        mul=t;
        if (t == 0) break;           /* we have reached the end of p */
                         /* write out sqr/mul pair */
        *sp-- = (unsigned char)(sqr & 0xff);                   
        *sp-- = (unsigned char)(mul & 0xff);
        if(sqr >= 256)        /* check whether sqr exceeds max uchar */
            {
             /* output the expanded list of to allow for this
              * and numbers will require to be added together
              * at interpret time
              */ 
            while(sqr >= 256)
                {
                *sp-- = 255; 
                *sp-- = 0;     /* mul is never zero normally */
                sqr-= 256;
                }
            }
        sqr=bits;                   /* set sqr to be the window size */

        ss+=bits;                        /* ss is total shift for wl */
        if (ss >= BN_BITS2)    /* adjust window words w,wl, wh again */
            {
            if (top <= 1) break;
            top--;

            wl=wh;
            wh=(top <= 1)?0:(*(d++));
            ss-=BN_BITS2;
            }

        if (ss == 0)
            w=wl;
        else
            w=(wl>>ss)|(wh<<(BN_BITS2-ss));
        }
    sp++;
    i=2;
    /* reverse the string from the top of the exponent string
         * and copy to the bottom, allocated exponent string is 2 * max length
         * expected for exponent string 
         */
    while (sp[0] != 0 || sp[1] != 0)
        {
        str[0]=sp[0];
        str[1]=sp[1];
        str+=2;
        sp+=2;
        i+=2;
        }
    str[0]=0;
    str[1]=0;

    return(i);
    }

#ifdef MAIN
main()
    {
    BIGNUM p;
    unsigned char buf[512],*pp;
    int i;

    BN_init(&p);
    BN_rand(&p,33,1,0);
#ifndef NO_FP_API
    BN_print_fp(stdout,&p); fprintf(stdout,"\n");
#endif

    BN_rand(&p,512,1,1);
#ifndef NO_FP_API
    BN_print_fp(stdout,&p); fprintf(stdout,"\n");
#endif
    for (i=0; i<10000; i++)
        BN_gen_exp_string(buf,&p,5);

#if 0
    BN_gen_exp_string(buf,&p,3);

    pp=buf;
    for (;;)
        {
        printf("mul %d sqr %d\n",pp[0],pp[1]);
        if (pp[1] == 0) break;
        pp+=2;
        }
#endif
    }
#endif
