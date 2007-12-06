/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fatfs
  File:     fatfs_loader.c

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

#include <symbols.h>

#include <firm.h>
#include <twl/os/common/format_rom.h>
#include <twl/aes/ARM7/lo.h>
#include <rtfs.h>
#include <devices/sdif_reg.h>

/*
    PROFILE_ENABLE を定義するとある程度のパフォーマンスチェックができます。
    利用するためには、main.cかどこかに、u32 profile[256]; u32 pf_cnt; を
    定義する必要があります。
*/
//#define PROFILE_ENABLE

#define MODULE_ALIGNMENT    0x10    // 16バイト単位で読み込む
//#define MODULE_ALIGNMENT  0x200   // 512バイト単位で読み込む
#define RoundUpModuleSize(value)    (((value) + MODULE_ALIGNMENT - 1) & -MODULE_ALIGNMENT)

#ifdef SDK_FINALROM // FINALROMで無効化
#undef PROFILE_ENABLE
#endif

#ifdef PROFILE_ENABLE
#define PROFILE_PXI_SEND    0x10000000
#define PROFILE_PXI_RECV    0x20000000
extern u32 profile[];
extern u32 pf_cnt;
#endif

static ROM_Header* const rh= (ROM_Header*)HW_TWL_ROM_HEADER_BUF;
static int menu_fd = -1;

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenRecentMenu

  Description:  open recent menu file
                システムメニューのファイルを特定し、オープンし、ファイルIDを
                menu_fdにセットします。
                最終的には、固定のタイトルメタデータを読み込み、eTicketの処理
                をして、システムメニューのファイルを特定することになる予定。

  Arguments:    driveno     drive number ('A' is 0)

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenRecentMenu( int driveno )
{
    char *menufile = (char*)L"A:\\title\\00010001\\52434e4c\\content\\12123434.app";
    if (driveno < 0 || driveno >= 26)
    {
        return FALSE;
    }
    menufile[0] = (char)('A' + driveno);
    menu_fd = po_open((u8*)menufile, PO_BINARY, 0);
    if (menu_fd < 0)
    {
        return FALSE;
    }
    FATFS_SaveSrlFilename(FATFS_MEDIA_TYPE_NAND, menufile);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenSpecifiedSrl

  Description:  open specified menu file

                任意のファイルをオープンし、ファイルIDをmenu_fdにセットします。

  Arguments:    menufile    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenSpecifiedSrl( const char* menufile )
{
    menu_fd = po_open((u8*)menufile, PO_BINARY, 0);
    if (menu_fd < 0)
    {
        return FALSE;
    }
    FATFS_SaveSrlFilename(FATFS_MEDIA_TYPE_SD, menufile);
    return TRUE;
}

static void OverrideDefaultMountInfo( void )
{
    static const OSMountInfo    DefaultSettings[] =
    {
        { 'A', OS_MOUNT_DEVICE_SD,   OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
        { 'B', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, 0,                               0, 0, "nand",    "/" },   // ユーザーはこのアーカイブを使えない(RW不可)
        { 'C', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 1, OS_MOUNT_RSC_WRAM, 0,                               0, 0, "nand2",   "/" },   // ユーザーはこのアーカイブを使えない(RW不可)
        { 'D', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "shared2", "nand2:/shared2" },
        { 'E', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_DIR,  0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "photo",   "nand2:/photo" },
        { 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPrv", NULL },
        { 'G', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_FILE, 0, OS_MOUNT_RSC_MMEM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "dataPub", NULL },
        { 0, },
    };
    MI_CpuCopy8(DefaultSettings, (char*)HW_TWL_FS_MOUNT_INFO_BUF, sizeof(DefaultSettings));
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_SaveSrlFilename

  Description:  store filename to HW_TWL_FS_BOOT_SRL_PATH_BUF

                ファイル名をHW_TWL_FS_BOOT_SRL_PATH_BUFに書き込みます。

  Arguments:    media       media type
                filename    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_SaveSrlFilename( FATFSMediaType media, const char* filename )
{
    char* dest = (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF;
    const char nandStr[] = "nand:/";
    const char sdmcStr[] = "sdmc:/";

    if ( filename[2] == ':' )   // ドライブレターは削除
    {
        filename += 4;
    }
    if ( filename[0] == '\\' )  // 先頭のパス記号は削除
    {
        filename += 2;
    }
    switch( media )
    {
    case FATFS_MEDIA_TYPE_NAND:
        MI_CpuCopy8(nandStr, dest, sizeof(nandStr)-1);
        dest += sizeof(nandStr)-1;
        break;
    case FATFS_MEDIA_TYPE_SD:
        MI_CpuCopy8(sdmcStr, dest, sizeof(sdmcStr)-1);
        dest += sizeof(sdmcStr)-1;
        break;
    default:
        return FALSE;
    }
    // unicode詰め (ASCII only)
    while ( dest < (char*)HW_TWL_ROM_HEADER_BUF )
    {
        if ( *filename == '\\' )    // パス記号変換
        {
            *dest++ = '/';
            filename++;
        }
        else if ( *filename != 0 )  // 通常コピー
        {
            *dest++ = *filename++;
        }
        else if ( *(filename + 1) != 0 )    // \0が連続していないならunicodeの詰めるべき隙間
        {
            filename++; // omit
        }
        else    // \0が連続しているなら終端
        {
            MI_CpuClear8( dest, HW_TWL_ROM_HEADER_BUF - (u32)dest );    // 残りバッファのクリア
            OverrideDefaultMountInfo(); // デフォルトマウント情報を書いておく
            break;
        }
    }
    OS_TPrintf("Stored: %s\n", (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetSrlDescriptor

  Description:  retrieve current file descriptor

                menu_fdを返します。

  Arguments:    None

  Returns:      int
 *---------------------------------------------------------------------------*/
int FATFS_GetSrlDescriptor( void )
{
    return menu_fd;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetSrlDescriptor

  Description:  set current file descriptor that was opened outside

                オープン済みのファイルIDをmenu_fdにセットします。

  Arguments:    None

  Returns:      int
 *---------------------------------------------------------------------------*/
void FATFS_SetSrlDescriptor( int fd )
{
    menu_fd = fd;
}

#define HEADER_SIZE 0x1000
#define AUTH_SIZE   ROM_HEADER_SIGN_TARGET_SIZE

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadBuffer

  Description:  load data and pass to ARM9 via WRAM[B]

                LoadBufferメカニズムで、FAT中のファイルの内容をARM9に転送します。

                [LoadBufferメカニズム]
                WRAM[B]を利用して、ARM7,ARM9間のデータ転送を行います。
                WRAM[B]の各スロットをバケツリレー方式で渡します。
                1スロット分のデータまたは全データが格納できたとき、ARM9へ
                FIRM_PXI_ID_LOAD_PIRIODを送信します。
                データ残がある場合は次のスロットの処理に移ります。
                2回目以降の呼び出しでは、前回最後のスロットの続きから使用します。
                使用したいスロットがARM9側に割り当てられているときは、ARM7側に
                なるまでストールします。

                [使用条件]
                WRAM[B]をロックせず、初期状態としてARM7側に倒しておくこと。

                [注意点]
                offsetとsizeはARM9に通知されません。別の経路で同期を取ってください。
                SRLファイルを読み込む場合は、互いにROMヘッダを参照できれば十分です。
                (ROMヘッダ部分は元から知っているはず)

                補足:
                ここでは、あるライブラリ内でARM7/ARM9側で歩調を合わせられることを
                前提にしているが、汎用的にするには(独立ライブラリ化するなら)、
                送受信でスロットを半分ずつとし、それぞれに受信側のPXIコールバック
                ＆スレッドを用意し、送信側APIがデータをWRAMに格納した後、他方に
                destとsizeを通知するという形でOKではないか？
                (で完了したら返事を返す)

                補足２：
                一度に複数スロットを使えそうな場合は、一気にpo_readした方が速い。
                ARM9からデータ内容を触る予定がないなら、本APIを使わず、普通に
                直接メインメモリに転送した方が速い。

  Arguments:    offset      offset of the file to load (512 bytes alignment)
                size        size to load

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadBuffer(u32 offset, u32 size)
{
    u8* base = (u8*)HW_FIRM_LOAD_BUFFER_BASE;
    static int count = 0;

    // seek first
    //OS_TPrintf("po_lseek(offset=0x%x);\n", offset);
    if (po_lseek(menu_fd, (s32)offset, PSEEK_SET) < 0)
    {
        return FALSE;
    }
#ifdef PROFILE_ENABLE
    // x2: after Seek
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    // loading loop
    while (size > 0)
    {
        u8* dest = base + count * HW_FIRM_LOAD_BUFFER_UNIT_SIZE;    // target buffer address
        u32 unit = size < HW_FIRM_LOAD_BUFFER_UNIT_SIZE ? size : HW_FIRM_LOAD_BUFFER_UNIT_SIZE; // size
        while (MI_GetWramBankMaster_B(count) != MI_WRAM_ARM7)       // waiting to be master
        {
        }
#ifdef PROFILE_ENABLE
        // x3...: after to wait ARM9
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        //OS_TPrintf("po_read(dest=%p, unit=0x%x);\n", dest, unit);
#if 1   /* 0: 2KBバグパッチ */
        if (po_read(menu_fd, (u8*)dest, (int)unit) < 0)     // reading
        {
            return FALSE;
        }
#else
{
    u32 done = 0;
    while (done < unit)
    {
        u8* dest2 = dest + done;
        u32 unit2 = (unit - done) < 2048 ? (unit - done) : 2048;
//OS_TPrintf("    po_read(dest=%p, unit=0x%x) ... ", dest2, unit2);
        if (po_read(menu_fd, (u8*)dest2, (int)unit2) < 0)   // reading
        {
            return FALSE;
        }
        done += unit2;
//OS_TPrintf("done\n");
    }
}
#endif
#ifdef PROFILE_ENABLE
        // x4...: before PXI
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_PIRIOD;    // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_PIRIOD );
        count = (count + 1) % HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
        size -= unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadHeader

  Description:  load header

                SRLのROMヘッダ部分を読み込み、ARM9に渡します。
                送信前に、ARM9へ FIRM_PXI_ID_LOAD_HEADER を送信します。
                送信後、ARM9から FIRM_PXI_ID_AUTH_HEADER を受信します。
                この時点で、メインメモリの所定の位置にROMヘッダが格納されたと
                想定します。
                問題なければ、seedデータを16バイト受信します。
                受け取ったseedはSeedAとKeyCに設定されます。
                makerom.TWLまたはIPLの仕様に依存します。

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadHeader( void )
{
    // open the file in FATFS_InitFIRM()
    if (menu_fd < 0)
    {
        return FALSE;
    }

#ifdef PROFILE_ENABLE
    // 10: before PXI
    pf_cnt = 0x10;
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_HEADER;    // checkpoint
#endif
    // load header without AES
    PXI_NotifyID( FIRM_PXI_ID_LOAD_HEADER );
    FATFS_DisableAES();
    if (!FATFS_LoadBuffer(0, AUTH_SIZE) ||
#ifdef PROFILE_ENABLE
        // 12: after to load half
        ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
        !FATFS_LoadBuffer(AUTH_SIZE, HEADER_SIZE - AUTH_SIZE) ||
#ifdef PROFILE_ENABLE
        // 1x: after to load remain
        ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
        PXI_RecvID() != FIRM_PXI_ID_AUTH_HEADER )
    {
        return FALSE;
    }
#ifdef PROFILE_ENABLE
    // 1x: after PXI
    profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_HEADER;    // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    // set id depends on game_code and seed to use (or all?)
    AESi_InitKeysForApp((u8*)rh->s.game_code);
    AESi_RecvSeed();

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFSi_GetCounter

  Description:  get counter

                offsetに対応したAESのカウンタ値を計算します。
                makerom.TWL内のコードに依存します。

  Arguments:    offset  offset from head of ROM_Header

  Returns:      counter
 *---------------------------------------------------------------------------*/
static AESCounter* FATFSi_GetCounter( u32 offset )
{
    static AESCounter counter;

    MI_CpuCopy8( rh->s.main_static_digest, &counter, 16 );
    AESi_AddCounter( &counter, offset - rh->s.aes_target_rom_offset );
    return &counter;
}

/*---------------------------------------------------------------------------*
  Name:         FATFSi_SetupAES

  Description:  setup whiere to use AES

                AES暗号化されたデータを読み込むためのセットアップを行います。
                fatfs_sdmc.cのドライバを使用していることが条件となります。
                (TwlSDK標準で行う場合は、その仕様に合わせて修正が必要！)

                このAPIを呼び出す前に、メインメモリの所定の位置にROMヘッダが
                格納されている必要があります。

                鍵の選択も行っていますが、鍵の設定は別の場所で行っておく
                必要があります。

                転送範囲がAES領域をまたぐ場合は、境界までのサイズ (引数より
                小さなサイズ) を返します。
                makerom.TWLまたはIPLの使用に依存します。

  Arguments:    offset  offset of region from head of ROM_Header
                size    size of region

  Returns:      size to transfer once
 *---------------------------------------------------------------------------*/
static u32 FATFSi_SetupAES( u32 offset, u32 size )
{
    u32 aes_offset = rh->s.aes_target_rom_offset;
    u32 aes_end = aes_offset + RoundUpModuleSize(rh->s.aes_target_size);
    u32 end = offset + RoundUpModuleSize(size);
    if ( rh->s.enable_aes )
    {
        if ( offset >= aes_offset && offset < aes_end )
        {
            if ( end > aes_end )
            {
                size = aes_end - offset;
            }
            AESi_WaitKey();
            if (rh->s.developer_encrypt)
            {
                AESi_LoadKey( AES_KEY_SLOT_C );
            }
            else
            {
                AESi_LoadKey( AES_KEY_SLOT_A );
            }
            FATFS_EnableAES( FATFSi_GetCounter( offset ) );
        }
        else
        {
            if ( offset < aes_offset && offset + size > aes_offset )
            {
                size = aes_offset - offset;
            }
            FATFS_DisableAES();
        }
    }
    else
    {
        FATFS_DisableAES();
    }
    return size;
}

/*---------------------------------------------------------------------------*
  Name:         FATFSi_LoadModule

  Description:  transfer module to ARM9 via WRAM[B]

                FATFSi_LoadBufferの上位APIです。

                AES境界をまたぐときに2回に分けるだけです。

  Arguments:    offset      offset from head of ROM_Header
                size        size to load

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static /*inline*/ BOOL FATFSi_LoadModule(u32 offset, u32 size)
{
    size = RoundUpModuleSize( size );   // アラインメント調整
    while ( size > 0 )
    {
        u32 unit = FATFSi_SetupAES( offset, size ); // 一度の転送サイズ
        if ( !FATFS_LoadBuffer( offset, unit ) )
        {
            return FALSE;
        }
        offset += unit;
        size -= unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadStatic

  Description:  load static binary

                ARM9/ARM7のStaticおよびLTD Staticを読み込みます。
                送信前に、ARM9へFIRM_PXI_ID_LOAD_*_STATICを送信します。
                送信後は、ARM9からFIRM_PXI_ID_AUTH_*_STATICを受信します。
                サイズが0の場合は、そのパートのPXI通信すら行いません。

                このAPIを呼び出す前に、メインメモリの所定の位置にROMヘッダが
                格納されている必要があります。

                ARM9側と異なり、デバイス依存のアライメント修正を行っています。
                (サイズのみ)

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadStatic( void )
{
#ifdef PROFILE_ENABLE
    // 30: LoadStatic
    pf_cnt = 0x30;
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_STATIC;    // checkpoint
#endif
    PXI_NotifyID( FIRM_PXI_ID_LOAD_STATIC );

    // load ARM9 static region without AES
    if ( rh->s.main_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 31: before PXI
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !FATFSi_LoadModule( rh->s.main_rom_offset, rh->s.main_size ) )
        {
            return FALSE;
        }
    }
    // load ARM7 static region without AES
    if ( rh->s.sub_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 50: before PXI
        pf_cnt = 0x50;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !FATFSi_LoadModule( rh->s.sub_rom_offset, rh->s.sub_size ) )
        {
            return FALSE;
        }
    }
    // load ARM9 extended static region with AES
    if ( rh->s.main_ltd_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 70: before PXI
        pf_cnt = 0x70;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !FATFSi_LoadModule( rh->s.main_ltd_rom_offset, rh->s.main_ltd_size ) )
        {
            return FALSE;
        }
    }
    // load ARM7 extended static region with AES
    if ( rh->s.sub_ltd_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 90: before PXI
        pf_cnt = 0x90;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !FATFSi_LoadModule( rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size ) )
        {
            return FALSE;
        }
    }

    // waiting result
    if ( PXI_RecvID() != FIRM_PXI_ID_AUTH_STATIC )
    {
        return FALSE;
    }
#ifdef PROFILE_ENABLE
    // 9x: after PXI
    profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_STATIC;    // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_Boot

  Description:  boot

                ROMヘッダの情報を引数に、OSi_Bootを呼び出すだけです。

                このAPIを呼び出す前に、メインメモリの所定の位置にROMヘッダが
                格納されている必要があります。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_Boot( void )
{
    OS_BootWithRomHeaderFromFIRM( rh );
}
