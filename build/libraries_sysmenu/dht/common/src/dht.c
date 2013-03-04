/*---------------------------------------------------------------------------*
  Project:  TwlIPL - DHT
  File:     dht.c

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

//�z���C�g�`�F�b�N���ɓǂ񂾃I�[�o���C���r�c�ɓf��
#define MGCN_TEST_OVL_OUT 1

//#define MGCN_TEST_OVL_OUT_0_REPEAT 1


//OVT��1�y�[�W�Âǂ�
//#define MGCN_TEST_OVT_1PAGE 1

#include <twl.h>
#include <sysmenu/dht/dht.h>
#include <nitro/os/ARM9/cache.h>


/*
    ��`����Ə������Ԃ�\������
*/
//#define PRINT_PROFILE

#ifdef PRINT_PROFILE
static int      count;
static OSTick   profile[0x10];
#define PROFILE_INIT()  (count = 0)
#define PROFILE_COUNT() (profile[count++] = OS_GetTick())
#else
#define PROFILE_INIT()  ((void)0)
#define PROFILE_COUNT() ((void)0)
#endif

static const u8 g_pubkey_DER[ 0xa2 ] = {
    0x30, 0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
    0x05, 0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xc7, 0xf4, 0x1d,
    0x27, 0x3f, 0xe8, 0xae, 0x7f, 0x7c, 0xbc, 0x9a, 0xae, 0x09, 0x8d, 0x19, 0x26, 0x2e, 0x90, 0x04,
    0x03, 0x13, 0x93, 0xbc, 0xb2, 0xe0, 0x8b, 0x1f, 0x85, 0x48, 0xf5, 0xf6, 0x94, 0x69, 0x3e, 0x05,
    0x1b, 0x97, 0x85, 0x44, 0x6d, 0xa3, 0xcd, 0xa8, 0x01, 0xfe, 0xdc, 0x77, 0x5d, 0xd1, 0xb1, 0x36,
    0x21, 0xfc, 0x80, 0xe8, 0xa6, 0x0e, 0xde, 0x59, 0x76, 0xca, 0x96, 0xcc, 0x87, 0x4c, 0xc3, 0x90,
    0xc6, 0x3b, 0xc8, 0x17, 0x9d, 0x2d, 0xac, 0x45, 0xbc, 0xa7, 0x15, 0xb2, 0xe3, 0xd7, 0x76, 0xfa,
    0x09, 0x8c, 0x55, 0x09, 0x22, 0x95, 0x4b, 0xe7, 0xde, 0xc0, 0x82, 0xf2, 0x02, 0x1a, 0x8a, 0x42,
    0x38, 0x7f, 0xbb, 0x31, 0xd6, 0xa8, 0x36, 0xdc, 0x8d, 0x2c, 0x42, 0x56, 0x51, 0xc1, 0xa3, 0x30,
    0x21, 0x30, 0xef, 0x06, 0x72, 0x0c, 0xa6, 0x55, 0xb7, 0x4f, 0x30, 0x35, 0x1b, 0x02, 0x03, 0x01,
    0x00, 0x01
};

static const u8 hmac_key[] = DHT_HMAC_KEY;

static DHTReadFunc  imageReadFunc;
static void*        imageBuffer;
static u8*          fatCache;
static int          fatPage;


#ifdef MGCN_TEST_OVL_OUT
static s32 mgcn_ovl_len;
extern u32 mgcn_cfg; //����f�[�^: title.c/SYSMi_LoadTitleThreadFunc��SD����ǂ܂��
u8* pmgcn_cfg = (u8*)&mgcn_cfg;
//+0
//phase2 OVL out enable
#define mgcn_cfg_flg_out 0x80
//Overlay size read/write enable
#define mgcn_cfg_flg_rf 1
#define mgcn_cfg_flg_wf 2
//+1
//�I�[�o���C�T�C�Y�œǂޏ��
#define mgcn_cfg_out_max 0xff
#endif


/*
    ���Ɛ�bsearch
*/

static void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
    u32 left = 0;
    u32 right = nmemb;

    if ( compar((u8*)base + size * (right - 1), key) < 0 )
    {
        return NULL;
    }
    if ( compar((u8*)base + size * left, key) > 0 )
    {
        return NULL;
    }

    while (left <= right)
    {
        u32 mid = (left + right) >> 1;
        const void* data = (u8*)base + size * mid;
        int result = compar(data, key);
//OS_TPrintf("left = %d, mid = %d, right = %d\n", left, mid, right);
        if ( !result )
        {
            return (void*)data;
        }
        if ( result < 0 )
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return NULL;
}
static int CompareGameCodeAndVersion(const void* a, const void* b)
{
    return MI_CpuComp8(a, b, 5);
}

/*
�f�[�^�x�[�X��ǂݍ��� (�O����)
*/
u32 DHT_GetDatabaseLength(const DHTFile* pDHT)
{
    if ( pDHT->header.magic_code != DHT_MAGIC_CODE )    // magic code�`�F�b�N
    {
        OS_TPrintf("Invalid magic code (magic=0x%08X).\n", pDHT->header.magic_code);
        return 0;
    }
    return sizeof(DHTHeader) + pDHT->header.nums * sizeof(DHTDatabase);
}
static BOOL DHT_CheckDatabase(const DHTFile* pDHT)
{
    SVCSignHeapContext pool;
    static u8 heap[4*1024]; // avoid stack overflow
    u8 md1[20];
    u8 md2[20];
    s32 result;
    // �t�@�C���������o��
    SVC_InitSignHeap(&pool, heap, sizeof(heap));
    SVC_DecryptSign(&pool, md1, pDHT->header.sign, &g_pubkey_DER[29]);
    // �n�b�V���v�Z
    SVC_CalcSHA1(md2, DHT_GET_SIGN_TARGET_ADDR(&pDHT->header), DHT_GET_SIGN_TARGET_SIZE(&pDHT->header));
    // ����
    result = SVC_CompareSHA1(md1, md2);
    if ( !result )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("SIGN = % 20B\n", md1);
        OS_TPrintfEx("HASH = % 20B\n", md2);
        OS_TPrintf("Signature is not valid.\n");
        return FALSE;
    }
    return TRUE;
}

BOOL DHT_PrepareDatabase(DHTFile* pDHT, FSFile* fp)
{
    s32 result;
    s32 length;
    PROFILE_INIT();

    if ( fp )
    {
        // �w�b�_�ǂݍ���
        PROFILE_COUNT();
        result = FS_ReadFile(fp, &pDHT->header, sizeof(DHTHeader));
        if ( result != sizeof(DHTHeader) )
        {
            OS_TPrintf("Cannot read the DHT header (result=%d).\n", result);
            return FALSE;
        }
        // �f�[�^�x�[�X�ǂݍ���
        PROFILE_COUNT();
        length = (s32)DHT_GetDatabaseLength(pDHT) - (s32)sizeof(DHTHeader); // �w�b�_������
        if ( length < 0 )
        {
            OS_TPrintf("Invalid DHT header.\n");
            return FALSE;
        }
        result = FS_ReadFile(fp, pDHT->database, length);
        if ( result != length )
        {
            OS_TPrintf("Cannot read the DHT database (result=%d).\n", result);
            return FALSE;
        }
    }
    else
    {
        PROFILE_COUNT();
        PROFILE_COUNT();
    }

    // �f�[�^�x�[�X�̌���
    PROFILE_COUNT();
    result = DHT_CheckDatabase(pDHT);

    // ���ʕ�
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to prepare the database.\n");
    OS_TPrintf("%10d msec for reading header.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for reading database.\n", (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("\nTotal: %10d msec.\n",             (int)OS_TicksToMilliSeconds(profile[3]-profile[0]));
#endif
    return result;
}

/*
ROM�w�b�_�ɑΉ�����f�[�^�x�[�X����ɓ����
*/
const DHTDatabase* DHT_GetDatabase(const DHTFile* pDHT, const ROM_Header_Short* pROMHeader)
{
    u8 data[5];
    DHTDatabase* db;
    PROFILE_INIT();

    // ����
    PROFILE_COUNT();
    MI_CpuCopy8( pROMHeader->game_code, data, 4 );
    data[4] = pROMHeader->rom_version;
    db = (DHTDatabase*)bsearch(data, pDHT->database, pDHT->header.nums, sizeof(DHTDatabase), CompareGameCodeAndVersion);
    if ( !db )
    {
        OS_TPrintf("Cannot find the database.\n");
    }
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("%10d msec for searching database.\n", (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
    return db;
}
/*
�n�b�V���v�Z (1)
�ǂݍ��ݍς݃f�[�^���`�F�b�N����
*/
void DHT_CheckHashPhase1Init(SVCHMACSHA1Context* ctx, const ROM_Header_Short* pROMHeader)
{
    PROFILE_INIT();
    PROFILE_COUNT();
    // ����
    SVC_HMACSHA1Init(ctx, hmac_key, sizeof(hmac_key));
    // �w�b�_
    SVC_HMACSHA1Update(ctx, pROMHeader, DHT_DS_HEADER_SIZE);
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\n%10d msec for scanning header.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
}
void DHT_CheckHashPhase1Update(SVCHMACSHA1Context* ctx, const void* ptr, s32 length)
{
    PROFILE_INIT();
    PROFILE_COUNT();
    // ARM9 or ARM7 static
    if ( length > 0 )
    {
        SVC_HMACSHA1Update(ctx, ptr, (u32)length);
    }
    // ���ʕ�
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("%10d msec for scanning %d bytes.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]), length);
#endif
}
BOOL DHT_CheckHashPhase1Final(SVCHMACSHA1Context* ctx, const u8 *hash)
{
    u8 md[20];
    BOOL result;
    PROFILE_INIT();
    PROFILE_COUNT();
    SVC_HMACSHA1GetHash(ctx, md);
    result = SVC_CompareSHA1(hash, md);
    if ( !result )
    {
        OS_TPrintf("\n");
        OS_TPrintfEx("DB   = % 20B\n", hash);
        OS_TPrintfEx("HASH = % 20B\n", md);
        OS_TPrintf("%s: hash[0] is not valid.\n", __func__);
    }
    // ���ʕ�
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
    return result;
}
BOOL DHT_CheckHashPhase1(const u8* hash, const ROM_Header_Short* pROMHeader, const void* pARM9, const void* pARM7)
{
    SVCHMACSHA1Context ctx;
    BOOL result;
    PROFILE_INIT();

    // �������w�b�_
    PROFILE_COUNT();
    DHT_CheckHashPhase1Init(&ctx, pROMHeader);

    // ARM9 Static
    DHT_CheckHashPhase1Update(&ctx, pARM9, (s32)pROMHeader->main_size);
    // ARM7 Static
    DHT_CheckHashPhase1Update(&ctx, pARM7, (s32)pROMHeader->sub_size);
    // ����
    result = DHT_CheckHashPhase1Final(&ctx, hash);
    // ���ʕ�
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to check the hash (phase 1).\n");
    OS_TPrintf("\nTotal: %10d msec.\n",     (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
#endif
    return result;
}

/*
�n�b�V���v�Z (2)
�Ώۗ̈�̓ǂݍ��݂ƃ`�F�b�N���s��
*/
static BOOL ImageHMACSHA1Update(SVCHMACSHA1Context* ctx, s32 offset, s32 length, void* arg)
{
    if ( !imageBuffer || !imageReadFunc )
    {
        return FALSE;
    }
#ifdef MGCN_TEST_OVL_OUT
    {
    s32 len;
    if ((pmgcn_cfg[0] & mgcn_cfg_flg_rf) && mgcn_ovl_len)len = mgcn_ovl_len;//�I�[�o���C�T�C�Y: 512K�܂�
	else len = length;
    if (len > 512*1024) len = 512*1024;//Hash�v�Z���̃��[�N�m���̂ŁA�M���܂Ŏg���ƕs��������
    //DC_InvalidateRange( (void*)imageBuffer,length );// �}�W�R������
    if ( !imageReadFunc(imageBuffer, offset, len, arg) )
    {
           return FALSE;
    }
    }
#else
    if ( !imageReadFunc(imageBuffer, offset, length, arg) )
    {
        return FALSE;
    }
#endif
    SVC_HMACSHA1Update(ctx, imageBuffer, (u32)length);
    return TRUE;
}

static BOOL GetOverlayInfo(int no, int fat_offset, int* pOffset, int* pLength, DHTReadFunc func, void* arg)
{
    ROM_FAT *fat;
    int page = (fat_offset + no * (s32)sizeof(ROM_FAT)) / DHT_FAT_PAGE_SIZE;
    if ( !fatCache )
    {
        return FALSE;
    }
    if ( fatPage != page )
    {
#ifdef MGCN_TEST_OVT_1PAGE
        //����P�y�[�W�ǂ� .. �}�W�R������
        if ( !func(fatCache, page * DHT_FAT_PAGE_SIZE, DHT_FAT_PAGE_SIZE, arg))
        {
            return FALSE;
        }
#else
        if ( fatPage + 1 == page )    // 1�y�[�W�̓L���b�V���ς�
        {
            MI_CpuCopy8( &fatCache[DHT_FAT_PAGE_SIZE], &fatCache[0], DHT_FAT_PAGE_SIZE );
            if ( !func(&fatCache[DHT_FAT_PAGE_SIZE], (page+1) * DHT_FAT_PAGE_SIZE, DHT_FAT_PAGE_SIZE, arg) )
            {
                return FALSE;
            }
        }
        else    // �ʏ��2�y�[�W�ǂ�
        {
            if ( !func(fatCache, page * DHT_FAT_PAGE_SIZE, DHT_FAT_CACHE_SIZE, arg) )
            {
                return FALSE;
            }
        }
#endif
        fatPage = page;
    }
    fat = (ROM_FAT*)(fatCache + fat_offset + no * sizeof(ROM_FAT) - page * DHT_FAT_PAGE_SIZE);
    if ( pOffset )
    {
        *pOffset = (s32)fat->top.offset;
    }
    if ( pLength )
    {
        *pLength = (s32)(fat->bottom.offset - fat->top.offset);
    }
    return TRUE;
}

BOOL DHT_CheckHashPhase2(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2Work* work, DHTReadFunc func, void* arg)
{
    imageBuffer = work->buffer;
    imageReadFunc = func;
    return DHT_CheckHashPhase2Ex(hash, pROMHeader, (DHTPhase2ExWork*)work->fatCache, func, ImageHMACSHA1Update, arg);
}

BOOL DHT_CheckHashPhase2Ex(const u8* hash, const ROM_Header_Short* pROMHeader, DHTPhase2ExWork* work, DHTReadFunc func, DHTReadFuncEx funcEx, void* arg)
{
    int overlay_nums = (int)(pROMHeader->main_ovt_size / sizeof(ROM_OVT));
    u8 md[20];
#ifdef MGCN_TEST_OVL_OUT
    int ovl_len;
#endif
    PROFILE_INIT();


    if ( overlay_nums )
    {
        SVCHMACSHA1Context ctx;
        int total_sectors;
        int i;

        if ( !func || !funcEx || !work )
        {
            return FALSE;
        }
        fatCache    = work->fatCache;
        fatPage     = -2;   // default value = out of range

        // ����
        PROFILE_COUNT();
        SVC_HMACSHA1Init(&ctx, hmac_key, sizeof(hmac_key));

#ifdef MGCN_TEST_OVL_OUT
        //funcEx(=ImageHMACSHA1Update)�� �I�[�o���C�ȊO�͈����T�C�Y�Ń��[�h
        mgcn_ovl_len = 0;
#endif

        // OVT
        PROFILE_COUNT();
        if ( !funcEx(&ctx, (s32)pROMHeader->main_ovt_offset, (s32)pROMHeader->main_ovt_size, arg) )
        {
            OS_TPrintf("Cannot calc HMAC-SHA1 for OVT.\n");
            return FALSE;
        }
        

        // FAT
        PROFILE_COUNT();
        if ( !funcEx(&ctx, (s32)pROMHeader->fat_offset, overlay_nums * (s32)sizeof(ROM_FAT), arg) )
        {
            OS_TPrintf("Cannot calc HMAC-SHA1 for %d of FAT.\n", overlay_nums);
            return FALSE;
        }
        
/*
#ifdef MGCN_TEST_OVL_OUT
        if (pmgcn_cfg[0] & mgcn_cfg_flg_out)
        {    
	        FSFile dest;
	        char filename[64];
		    STD_TSNPrintf( filename, 64, "sdmc:/OVLFAT_%d_%8x.dat",overlay_nums,(s32)pROMHeader->fat_offset);
	        FS_InitFile( &dest );
	        (void)FS_CreateFile(filename, FS_PERMIT_W | FS_PERMIT_R);
	        FS_OpenFileEx( &dest, filename, FS_FILEMODE_W );
	        FS_WriteFile( &dest, imageBuffer, overlay_nums * (s32)sizeof(ROM_FAT));
	        FS_CloseFile( &dest );
        }
#endif
*/
        // �e�I�[�o�[���C
        PROFILE_COUNT();
        total_sectors = 0;
        for (i = 0; i < overlay_nums; i++)
        {
            int max_sectors = (DHT_OVERLAY_MAX/512 - total_sectors) / (overlay_nums - i);
            int offset;
            int length;
            if ( !GetOverlayInfo(i, (s32)pROMHeader->fat_offset, &offset, &length, func, arg) )
            {
                OS_TPrintf("Cannot get %d of overlay info.\n", i);
                return FALSE;
            }

#ifdef MGCN_TEST_OVL_OUT_0_REPEAT
            if(i ==0)
            {//�J��Ԃ����[�h
	            FSFile dest;
                SVCHMACSHA1Context ctx2;
	            char filename[64];
                int j,len = (length + 511) / 512;  // bytes -> sectors
                if ( len > max_sectors ) len = max_sectors;
                for ( j =0;j<2;j++){
		            STD_TSNPrintf( filename, 64, "sdmc:/OVL0_%d.dat",j);
	                FS_InitFile( &dest );
	                (void)FS_CreateFile(filename, FS_PERMIT_W | FS_PERMIT_R);
	                FS_OpenFileEx( &dest, filename, FS_FILEMODE_W );
			        DC_InvalidateRange( (void*)imageBuffer,length );
                    imageReadFunc(imageBuffer, offset,length,arg);
	                FS_WriteFile( &dest, imageBuffer,length );
	                FS_CloseFile( &dest );
                
                    //Hash��̏��
                    SVC_HMACSHA1Update(&ctx2, imageBuffer, (u32)len);
		            STD_TSNPrintf( filename, 64, "sdmc:/OVL0_%d_hs.dat",j);
	                FS_InitFile( &dest );
	                (void)FS_CreateFile(filename, FS_PERMIT_W | FS_PERMIT_R);
	                FS_OpenFileEx( &dest, filename, FS_FILEMODE_W );
	                FS_WriteFile( &dest, imageBuffer,length );
	                FS_CloseFile( &dest );
                }
            }
#endif

#ifdef MGCN_TEST_OVL_OUT
            //�I�[�o���C�T�C�Y��ۑ�
            ovl_len = length;
#endif

            length = (length + 511) / 512;  // bytes -> sectors
#ifdef MGCN_TEST_OVL_OUT
            // Hash�v�Z���Ƀ��[�h���`�F�b�N�T�C�Y�ȉ��ɂȂ�Ȃ��悤����
            mgcn_ovl_len = length * 512;
#endif
            if ( length > max_sectors )
            {
                length = max_sectors;
            }

#ifdef MGCN_TEST_OVL_OUT
            //  �w��ȏ�̓`�F�b�N�T�C�Y�ɍ��킹��
            if (i > pmgcn_cfg[1]) mgcn_ovl_len = length * 512;
#endif

            if ( length < 0 || offset < sizeof(ROM_Header) )
            {
                return FALSE;
            }

            if ( !funcEx(&ctx, offset, length * 512, arg) )
            {
                OS_TPrintf("Cannot calc HMAC-SHA1 for %d of overlay.\n", i);
                return FALSE;
            }


#ifdef MGCN_TEST_OVL_OUT
            //----------- �}�W�R������
            //�I�[�o���C��f��
            if (pmgcn_cfg[0] & mgcn_cfg_flg_out)
            {
	            FSFile dest;
	            char filename[64];
            /*
	            int sz,len = length;
		      
		        STD_TSNPrintf( filename, 64, "sdmc:/OVL%d_%8x_%8x.dat",i,offset,length );
	            FS_InitFile( &dest );
	            (void)FS_CreateFile(filename, FS_PERMIT_W | FS_PERMIT_R);
	            FS_OpenFileEx( &dest, filename, FS_FILEMODE_W );
	            while(len){
                    if (remain < DHT_OVERLAY_MAX )sz = len;
                    else sz = DHT_OVERLAY_MAX;
					DC_InvalidateRange( (void*)imageBuffer,sz );
                    imageReadFunc(imageBuffer, offset, sz, arg);
	                FS_WriteFile( &dest, imageBuffer,sz );
	                len-=sz;
	            }
	            FS_CloseFile( &dest );
           */
           
                if((pmgcn_cfg[0] & mgcn_cfg_flg_wf ) && (i <= pmgcn_cfg[1]))
                {
                    STD_TSNPrintf( filename, 64, "sdmc:/OVL%d_%8x_%8x.dat",i,offset,ovl_len );
                }else{
                    ovl_len = length*512;
		            STD_TSNPrintf( filename, 64, "sdmc:/CHK%d_%8x_%8x.dat",i,offset,ovl_len);
	            }
                FS_InitFile( &dest );
                (void)FS_CreateFile(filename, FS_PERMIT_W | FS_PERMIT_R);
                FS_OpenFileEx( &dest, filename, FS_FILEMODE_W );
                FS_WriteFile( &dest,imageBuffer,ovl_len );
	            FS_CloseFile( &dest );
            }
#endif

/* -- origin
            length = (length + 511) / 512;  // bytes -> sectors
            if ( length > max_sectors )
            {
                length = max_sectors;
            }

            if ( length < 0 || offset < sizeof(ROM_Header) )
            {
                OS_TPrintf("Broken FAT for %d of overlay.\n", i);
                return FALSE;
            }
            if ( !funcEx(&ctx, offset, length * 512, arg) )
            {
                OS_TPrintf("Cannot calc HMAC-SHA1 for %d of overlay.\n", i);
                return FALSE;
            }
*/


            total_sectors += length;
        }
        // ����
        PROFILE_COUNT();
        SVC_HMACSHA1GetHash(&ctx, md);
    }
    else
    {
        PROFILE_COUNT();
        PROFILE_COUNT();
        PROFILE_COUNT();
        PROFILE_COUNT();
        PROFILE_COUNT();
        MI_CpuClear8(md, sizeof(md));
    }
    if ( !SVC_CompareSHA1(md, hash) )
    {
#ifdef MGCN_TEST_OVL_OUT
        if (pmgcn_cfg[0] & mgcn_cfg_flg_out)
        {
	        FSFile dest;
	        char filename[64];
            
		    STD_TSNPrintf( filename, 64, "sdmc:/hasherr");
	        FS_InitFile( &dest );
	        (void)FS_CreateFile(filename, FS_PERMIT_W | FS_PERMIT_R);
	        FS_OpenFileEx( &dest, filename, FS_FILEMODE_W );
	        FS_WriteFile( &dest,&hash,20 );
	        FS_WriteFile( &dest,&md,20 );
	        FS_CloseFile( &dest );
        }
#endif

        OS_TPrintf("\n");
        OS_TPrintfEx("DB   = % 20B\n", hash);
        OS_TPrintfEx("HASH = % 20B\n", md);
        OS_TPrintf("%s: hash[1] is not valid.\n", __func__);
        return FALSE;
    }
    // ���ʕ�
#ifdef PRINT_PROFILE
    PROFILE_COUNT();
    OS_TPrintf("\nDone to check the hash (phase 2).\n");
    OS_TPrintf("%10d msec for preparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[1]-profile[0]));
    OS_TPrintf("%10d msec for scanning OVT.\n",     (int)OS_TicksToMilliSeconds(profile[2]-profile[1]));
    OS_TPrintf("%10d msec for scanning FAT.\n",     (int)OS_TicksToMilliSeconds(profile[3]-profile[2]));
    OS_TPrintf("%10d msec for scanning every overlays.\n",  (int)OS_TicksToMilliSeconds(profile[4]-profile[3]));
    OS_TPrintf("%10d msec for comparing hash.\n",   (int)OS_TicksToMilliSeconds(profile[5]-profile[4]));
    OS_TPrintf("\nTotal: %10d msec.\n",     (int)OS_TicksToMilliSeconds(profile[5]-profile[0]));
#endif

#ifdef MGCN_TEST_OVL_OUT
    if (pmgcn_cfg[0] & mgcn_cfg_flg_out)
    {
	    FSFile dest;
	    char filename[64];
            
		STD_TSNPrintf( filename, 64, "sdmc:/complate");
	    FS_InitFile( &dest );
	    (void)FS_CreateFile(filename, FS_PERMIT_W | FS_PERMIT_R);
	    FS_OpenFileEx( &dest, filename, FS_FILEMODE_W );
	    FS_CloseFile( &dest );
    }
#endif

    return TRUE;
}

void DHT_CheckHashPhase2ExUpdate(SVCHMACSHA1Context* ctx, const void* ptr, s32 length)
{
    // ARM9 or ARM7 static
    if ( length > 0 )
    {
        SVC_HMACSHA1Update(ctx, ptr, (u32)length);
    }
}
