#include <string.h>
#include "acsign_nor.h"

static void debug_dump(void* buf, int len, char* str, int line_elms)
{
    u8* bufp = (u8*)buf;
    int i,ii;

    if (str)
    {
        debug_printf("%s :\n", str);
    }
    for (i=0; i<=len/line_elms; i++)
    {
        if (i*line_elms >= len)
        {
            break;
        }
        for (ii=0; ii<line_elms; ii++)
        {
            if (i*line_elms+ii >= len)
            {
                break;
            }
            debug_printf("%02x ", bufp[i*line_elms+ii]);
        }
        debug_printf("\n");
    }
}

//
int     ACSign_DigestHeader(
                    void*         buffer,     //  出力領域
                    NORHeader*    header      //  ヘッダへのポインタ
                    )
{
    HASHContext     context;
    NORHeader* nh = header;
    unsigned char   *bufferp = buffer;

    HASHReset( &context );
    HASHUpdate( &context, (void*)&nh->d, sizeof(NORHeaderDS) );
    HASHUpdate( &context, (void*)&nh->l, sizeof(NORHeaderLow) );
    HASHUpdate( &context, (void*)&nh->h, sizeof(NORHeaderHigh) );
    HASHGetDigest( &context, bufferp );

    return TRUE;
}

//
int     ACSign_Final(
                    NORHeader*    header,     //  ヘッダへのポインタ
                    void*         buffer,     //  入力領域
                    const void*   key
                    )
{
    NORHeader* nh = header;
    FIRMSignedContext* sc = buffer;

    if (key)
    {
        unsigned char   decSign[ACS_ENCRYPTED_HASH_LEN];

        debug_dump(sc->hash, sizeof(sc->hash), "5 hashs of header, norfirms and total", 20);

        ACSign_Encrypto(&nh->sign, key, buffer, sizeof(FIRMSignedContext));
        ACSign_Decrypto(decSign, key, (void*)&nh->sign, ACS_ENCRYPTED_HASH_LEN);

        debug_dump(&nh->sign, ACS_ENCRYPTED_HASH_LEN, "encrypted sign", 16);
        debug_dump(decSign, ACS_ENCRYPTED_HASH_LEN, "decrypted sign", 16);
    }

    return TRUE;
}

