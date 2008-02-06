#ifndef _ACSIGN_GCD_H_
#define _ACSIGN_GCD_H_

#include    "sha.h"
#include    "format_sign.h"
#include    "format_rom.h"
#include    "acsign.h"

#ifdef __cplusplus
extern "C" {
#endif

//
int     ACSign_Final(
                    GCDHeader*    header,     //  ヘッダへのポインタ
                    void*         buffer,     //  出力領域
                    const void*   key
                    );

//
int     ACSign_DigestHeader(
                    void*         buffer,     //  出力領域
                    GCDHeader*    header      //  データへのポインタ
                    );

#ifdef __cplusplus
}
#endif

#endif //_ACSIGN_GCD_H_
