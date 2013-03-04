/*---------------------------------------------------------------------------*
  Project:  NinTendo Compress tool
  File:     nitroCompLib.h

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

// nitroCompLib.h : nitroCompLib.DLL のメイン ヘッダー ファイル
//

#ifndef __NITROCOMPLIB_H__
#define __NITROCOMPLIB_H__

//===========================================================================================
// インクルード
//===========================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"

#ifdef WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

//===========================================================================================
// プロトタイプ宣言
//===========================================================================================
// C++用
#ifdef __cplusplus
  extern "C"
  {
#endif

//BOOL WINAPI DllMain( HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved);
u8*  STDCALL    nitroCompMalloc( u32 size );
void STDCALL    nitroCompFree( u8 *p );
u32  STDCALL    nitroCompress  ( const u8 *srcp, u32 srcSize, u8 *dstp, char *compList, u8 rawHeader );
s32  STDCALL    nitroDecompress( const u8 *srcp, u32 srcSize, u8 *dstp, s8 depth );
void STDCALL    debugMemPrint   ( FILE * fp, u8 *str, u32 size );
void STDCALL    debugMemBitPrint( FILE * fp, u8 *str, u32 size );
int  STDCALL    matchingCheck( u8 *srcp, u32 srcSize, u8 *dstp, u32 dstSize );
u32  STDCALL    nitroGetDecompFileSize( const void* srcp );

#ifdef __cplusplus
  }
#endif

//===========================================================================================
// 関数の使用法
//===========================================================================================
//----------------------------------------------------------------------------------
//  圧縮後のデータを置くためのメモリ領域を確保
//    圧縮前のデータの2倍の領域を確保する
//    引数
//      u32  size   圧縮前のデータサイズ
//                     u32 は符号無し整数型でたいていは、
//                     unsigned int (処理系依存)
//    返り値
//      u8    *        圧縮後のデータ領域を指すポインタ
//                     (汎用ポインタなので、好きな型にキャスト)
//                                              ではない
//----------------------------------------------------------------------------------
//u8 *nitroCompMalloc(u32 size);

//----------------------------------------------------------------------------------
//  圧縮後のデータを置いていたメモリ領域を解放
//    引数
//      u8    *p   圧縮後のデータ領域を指すポインタ
//----------------------------------------------------------------------------------
//void nitroCompFree(u8 *p);

//----------------------------------------------------------------------------------
//  データを圧縮する
//  圧縮方式、圧縮順序は引数compListを用いて指示する
//    引数
//              u8       *srcp                  圧縮対象データを指すポインタ
//              u32       size                  圧縮対象データのサイズ(単位はバイト)
//              u8       *dstp                  圧縮後のデータを保持するデータ領域を指すポインタ
//                                                      十分な領域を確保しておく必要あり
//                                                      nitroCompMallocの返り値でよい
//              char *compList          圧縮方式、圧縮順序を格納したリスト (C言語のヌル文字列)
//                                                      d8      : 8ビット差分フィルタ
//                                                      d16     : 16ビット差分フィルタ
//                                                      r       : ランレングス符号化
//                                                      lx      : LZ77．xには同一データ検索の開始点を示すオフセット
//                                                              : 2以上でなければならない．上限は255.
//                                                      h4      : 4ビット・ハフマン圧縮
//                                                      h8      : 8ビット・ハフマン圧縮
//              u8        rawHeaderFlag 圧縮前のデータであることを示すヘッダ情報を付加するかどうかを
//                                                      指示するフラグ．0であれば付加せず、1であれば、展開しきった
//                                                      データにもヘッダが付加される．
//        返り値
//          圧縮後のデータサイズ
//----------------------------------------------------------------------------------
//u32   nitroCompress(u8 *srcp, u32 size, u8 *dstp, char *compList, u8 rawHeader);

//----------------------------------------------------------------------------------
//  データを展開する
//  展開方式、展開順序はデータのヘッダを見て判断する
//    引数
//              u8       *srcp                  展開対象データを指すポインタ
//              u32       size                  展開対象データのサイズ(単位はバイト)
//              u8       *dstp                  展開後のデータを保持するデータ領域を指すポインタ
//                                                      十分な領域を確保しておく必要あり
//                                                      nitroCompMallocの返り値でよい
//              u8        depth                 展開する深さ(回数)
//                          0未満の場合は、圧縮前データを示すヘッダ情報が得られるまで展開し続ける
//        返り値
//          展開後のデータサイズ
//----------------------------------------------------------------------------------
//u32   nitroDecompress(u8 *srcp, u32 size, u8 *dstp, s8 depth);

#endif // __NITROCOMPLIB_H__
