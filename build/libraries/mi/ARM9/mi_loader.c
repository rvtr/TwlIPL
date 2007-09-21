/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - mi
  File:     mi_loader.c

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

#include <twl.h>
#include <firm.h>
#include <firm/format/format_rom.h>

#define PXI_FIFO_TAG_DATA   PXI_FIFO_TAG_USER_0

static ROM_Header* const    rh = (ROM_Header*)(HW_MAIN_MEM_SYSTEM_END - 0x2000);

#define HEADER_SIZE 0x1000
#define AUTH_SIZE   0xe00
#define RSA_BLOCK_SIZE  128

#define SLOT_SIZE   0x8000

#define HASH_UNIT   0x800   // TODO: optimizing to maximize cache efficiency

typedef struct SHA1_CTX // 実際には、サイズが同じなら中身は何でも良い
{
    u32 h0,h1,h2,h3,h4;
    u32 Nl,Nh;
    u32 data[16];
    int num;
    void (*sha_block)(struct SHA1_CTX *c, const u8 *W, int num);
}
SHA1_CTX;

static inline void SHA1_Init(SHA1_CTX *ctx)
{
    if (ctx == NULL)
        return;

    MI_CpuClear8(ctx, sizeof(SHA1_CTX));
    SVC_SHA1Init(ctx);
}

static inline void SHA1_Update(SHA1_CTX *ctx, const void* data, u32 len)
{
    if (ctx == NULL)
        return;
    if (len > 0 && data == NULL)
        return;
    SVC_SHA1Update(ctx, data, len);
}

static inline void SHA1_GetHash(SHA1_CTX *ctx, u8* md)
{
    if (ctx == NULL)
        return;
    if (md == NULL)
        return;
    SVC_SHA1GetHash(md, ctx);
}

static inline void SHA1_Calc(u8* md, const void* data, u32 len)
{
    SVC_CalcSHA1(md, data, len);
}


static BOOL MI_LoadBuffer(u8* dest, u32 size, SHA1_CTX *ctx)
{
    u8*   base = (void*)MI_GetWramMapStart_B();
    static int count = 0;

    while (size > 0)
    {
        u8* src = base + count * SLOT_SIZE;
        u32 unit = size < SLOT_SIZE ? size : SLOT_SIZE;
        if ( PXI_RecvID() != FIRM_PXI_ID_LOAD_PIRIOD )
        {
            return FALSE;
        }
        MIi_SetWramBankMaster_B(count, MI_WRAM_ARM9);
        if (ctx)
        {
            int done;
            for (done = 0; done < unit; done += HASH_UNIT)
            {
                u8* s = src + done;
                u8* d = dest + done;
                u32 u = unit < done + HASH_UNIT ? unit - done : HASH_UNIT;
                SHA1_Update( ctx, s, u );
                MI_CpuCopyFast( s, d, u );
            }
        }
        else
        {
            MI_CpuCopyFast( src, dest, unit );
        }
        MI_CpuClearFast( src, unit );
        DC_FlushRange( src, unit );
        MIi_SetWramBankMaster_B(count, MI_WRAM_ARM7);
        count = (count + 1) & 0x07;
        size -= unit;
        dest += unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MI_LoadHeader

  Description:  load header

  Arguments:    pool        pointer to the pool info for SVC_DecryptoSign
                rsa_key     key address

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadHeader( int* pool, const void* rsa_key )
{
    SHA1_CTX ctx;
    u8 md[DIGEST_SIZE_SHA1];
    SHA1_Init(&ctx);

    // load header (hash target)
    if ( PXI_RecvID() != FIRM_PXI_ID_LOAD_HEADER ||
         !MI_LoadBuffer( (u8*)rh, AUTH_SIZE, &ctx ) )
    {
        return FALSE;
    }
    SHA1_GetHash(&ctx, md);
    // load header (remain)
    if ( !MI_LoadBuffer( (u8*)rh + AUTH_SIZE, HEADER_SIZE - AUTH_SIZE, NULL ) )
    {
        return FALSE;
    }

    // RSA
#if 0
    makerom.TWLのset_signature.cのCheckRomCertificateを元に書き換える
#endif
    {
        SignatureData sd;
#if 0
        int i;
        BOOL result = TRUE;
        SVC_DecryptoSign(pool, &sd, rh->signature, rsa_key);
        for (i = 0; i < DIGEST_SIZE_SHA1; i++)
        {
            if ( md[i] != sd.digest[i] )
            {
                result = FALSE;
            }
        }
#endif
#if 0
        if ( !result )
        {
            return FALSE;
        }
#endif
        PXI_NotifyID( FIRM_PXI_ID_AUTH_HEADER );
//        PXI_SendDataByFifo( PXI_FIFO_TAG_DATA, sd.aes_key_seed, AES_BLOCK_SIZE );
        MI_CpuClear8(&sd, sizeof(sd));
        MI_CpuClear8(&md, sizeof(md));
    }

    return TRUE;
}

static inline BOOL MIi_LoadModule(void* dest, u32 size, const u8 digest[DIGEST_SIZE_SHA1])
{
    SHA1_CTX ctx;
    u8 md[DIGEST_SIZE_SHA1];
    int i;
    BOOL result = TRUE;

    SHA1_Init(&ctx);
    if ( !MI_LoadBuffer( dest, size, &ctx ) )
    {
        return FALSE;
    }
#if 0
    SHA1_GetHash(&ctx, md);
    for ( i = 0; i < DIGEST_SIZE_SHA1; i++ )
    {
        if ( md[i] != digest[i] )
        {
            result = FALSE;
        }
    }
#endif
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MI_LoadMenu

  Description:  load menu program

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadMenu( void )
{
    // load ARM9 static region
    if ( rh->s.main_size > 0 )
    {
        if ( PXI_RecvID() !=  FIRM_PXI_ID_LOAD_ARM9_STATIC ||
             !MIi_LoadModule( rh->s.main_ram_address, rh->s.main_size, rh->s.main_static_digest ) )
        {
            return FALSE;
        }
        PXI_NotifyID( FIRM_PXI_ID_AUTH_ARM9_STATIC );
    }

    // load ARM7 static region
    if ( rh->s.sub_size > 0 )
    {
        if ( PXI_RecvID() !=  FIRM_PXI_ID_LOAD_ARM7_STATIC ||
             !MIi_LoadModule( rh->s.sub_ram_address, rh->s.sub_size, rh->s.sub_static_digest ) )
        {
            return FALSE;
        }
        PXI_NotifyID( FIRM_PXI_ID_AUTH_ARM7_STATIC );
    }

    // load ARM9 extended static region
    if ( rh->s.main_ex_size > 0 )
    {
        if ( PXI_RecvID() !=  FIRM_PXI_ID_LOAD_ARM9_STATIC_EX ||
             !MIi_LoadModule( rh->s.main_ex_ram_address, rh->s.main_ex_size, rh->s.main_static_ex_digest ) )
        {
            return FALSE;
        }
        PXI_NotifyID( FIRM_PXI_ID_AUTH_ARM9_STATIC_EX );
    }
    // load ARM7 extended static region
    if ( rh->s.sub_ex_size > 0 )
    {
        if ( PXI_RecvID() !=  FIRM_PXI_ID_LOAD_ARM7_STATIC_EX ||
             !MIi_LoadModule( rh->s.sub_ex_ram_address, rh->s.sub_ex_size, rh->s.sub_static_ex_digest ) )
        {
            return FALSE;
        }
        PXI_NotifyID( FIRM_PXI_ID_AUTH_ARM7_STATIC_EX );
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MI_BootMenu

  Description:  boot menu

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_BootMenu( void )
{
    OSi_Boot( rh->s.main_entry_address, (MIHeader_WramRegs*)rh->s.main_wram_config_data );
}
