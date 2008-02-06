#ifndef _ACSIGN_NOR_H_
#define _ACSIGN_NOR_H_

#include    "sha.h"
#include    "format_sign.h"
#include    "format_rom.h"
#include    "acsign.h"

#ifdef __cplusplus
extern "C" {
#endif

//
int     ACSign_Final(
                    NORHeader*    header,     //  �w�b�_�ւ̃|�C���^
                    void*         buffer,     //  �o�͗̈�
                    const void*   key
                    );

//
int     ACSign_DigestHeader(
                    void*         buffer,     //  �o�͗̈�
                    NORHeader*    header      //  �f�[�^�ւ̃|�C���^
                    );

#ifdef __cplusplus
}
#endif

#endif //_ACSIGN_NOR_H_
