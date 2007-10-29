/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     acsign.h

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

#ifndef _ACSIGN_H_
#define _ACSIGN_H_

#ifdef __cplusplus
extern "C" {
#endif

#if     defined( RSA_ENC_DEC )
#define     ACSIGN_BUFFER  ((160/8)*4)  // 出力バッファ量
#else
#define     ACSIGN_BUFFER  ((160/8)*1)  // 出力バッファ量
#endif

//
int     ACSign_Decrypto(
                    void*   buffer,     //  出力領域
                    void*   sgn_ptr,    //  データへのポインタ
                    void*   key_ptr     //  キーへのポインタ
                    );

// 
int      ACSign_Digest(
                    void*   buffer,     //  出力領域
                    void*   romh_ptr,   //  データへのポインタ
                    void*   mbin_ptr,   //  データへのポインタ
                    int     mbin_len,   //  データの長さ
                    void*   sbin_ptr,   //  データへのポインタ
                    int     sbin_len,   //  データの長さ
                    u32     serial_num  //  シリアルナンバー
                    );

//
int     ACSign_Compare(
                    void* dercypto,     //  ACSign_Decryptoの出力
                    void* digest        //  ACSign_Digestの出力
                      );

#ifdef __cplusplus
}
#endif

#endif //_ACSIGN_H_
