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
#include <firm/format/format_rom.h>
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


#define PXI_FIFO_TAG_DATA   PXI_FIFO_TAG_USER_0

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
    char *menufile = (char*)L"A:\\ipl\\menu.srl";
    if (driveno < 0 || driveno >= 26)
    {
        return FALSE;
    }
    menufile[0] += (char)driveno;
    menu_fd = po_open((u8*)menufile, PO_BINARY, 0);
    if (menu_fd < 0)
    {
        return FALSE;
    }
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
    return TRUE;
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

  Arguments:    offset      offset of the file to load (512 bytes alignment)
                size        size to load

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL FATFS_LoadBuffer(u32 offset, u32 size)
{
    u8* base = (u8*)HW_FIRM_LOAD_BUFFER_BASE;
    static int count = 0;

    // seek first
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
        //OS_TPrintf("%s: dest=%X, unit=%X\n", __func__, dest, unit);
        while (MI_GetWramBankMaster_B(count) != MI_WRAM_ARM7)       // waiting to be master
        {
        }
#ifdef PROFILE_ENABLE
        // x3...: after to wait ARM9
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if (po_read(menu_fd, (u8*)dest, (int)unit) < 0)            // reading
        {
            return FALSE;
        }
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
    {
        AESKeySeed seed;
        AESi_InitGameKeys((u8*)rh->s.game_code);
        PXI_RecvDataByFifo( PXI_FIFO_TAG_DATA, &seed, AES_BLOCK_SIZE );
        AESi_WaitKey();
        AESi_SetKeySeedA(&seed);    // APP
        //AESi_WaitKey();
        //AESi_SetKeySeedB(&seed);    // APP & HARD
        //AESi_WaitKey();
        //AESi_SetKeySeedC(&seed);    //
        //AESi_WaitKey();
        //AESi_SetKeySeedD(&seed);    // HARD
        AESi_WaitKey();
        AESi_SetKeyC(&seed);        // Direct
    }

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
                makerom.TWLまたはIPLの使用に依存します。

  Arguments:    offset  offset of region from head of ROM_Header
                size    size of region (for check only)

  Returns:      counter
 *---------------------------------------------------------------------------*/
static void FATFSi_SetupAES( u32 offset, u32 size )
{
    u32 aes_end = rh->s.aes_target_rom_offset + RoundUpModuleSize(rh->s.aes_target_size);
    u32 arg_end = offset + RoundUpModuleSize(size);
    if ( rh->s.enable_aes && offset >= rh->s.aes_target_rom_offset && arg_end <= aes_end )
    {
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
        FATFS_DisableAES();
    }
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
    // load ARM9 static region without AES
    if ( rh->s.main_size > 0 )
    {
        u32 aligned = RoundUpModuleSize(rh->s.main_size);
#ifdef PROFILE_ENABLE
        // 30: before PXI
        pf_cnt = 0x30;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_ARM9_STATIC;   // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM9_STATIC );
        FATFSi_SetupAES( rh->s.main_rom_offset, aligned );
        if ( !FATFS_LoadBuffer( rh->s.main_rom_offset, aligned ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM9_STATIC )
        {
            return FALSE;
        }
#ifdef PROFILE_ENABLE
        // 3x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_ARM9_STATIC;   // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    }
    // load ARM7 static region without AES
    if ( rh->s.sub_size > 0 )
    {
        u32 aligned = RoundUpModuleSize(rh->s.sub_size);
#ifdef PROFILE_ENABLE
        // 50: before PXI
        pf_cnt = 0x50;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_ARM7_STATIC;   // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM7_STATIC );
        FATFSi_SetupAES( rh->s.sub_rom_offset, aligned );
        if ( !FATFS_LoadBuffer( rh->s.sub_rom_offset, aligned ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM7_STATIC )
        {
            return FALSE;
        }
#ifdef PROFILE_ENABLE
        // 5x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_ARM7_STATIC;   // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    }
    // load ARM9 extended static region with AES
    if ( rh->s.main_ltd_size > 0 )
    {
        u32 aligned = RoundUpModuleSize(rh->s.main_ltd_size);
#ifdef PROFILE_ENABLE
        // 70: before PXI
        pf_cnt = 0x70;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_ARM9_LTD_STATIC;    // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM9_LTD_STATIC );
        FATFSi_SetupAES( rh->s.main_ltd_rom_offset, aligned );
        if ( !FATFS_LoadBuffer( rh->s.main_ltd_rom_offset, aligned ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM9_LTD_STATIC )
        {
            return FALSE;
        }
#ifdef PROFILE_ENABLE
        // 7x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_ARM9_LTD_STATIC;    // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    }
    // load ARM7 extended static region with AES
    if ( rh->s.sub_ltd_size > 0 )
    {
        u32 aligned = RoundUpModuleSize(rh->s.sub_ltd_size);
#ifdef PROFILE_ENABLE
        // 90: before PXI
        pf_cnt = 0x90;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_ARM7_LTD_STATIC;    // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_ARM7_LTD_STATIC );
        FATFSi_SetupAES( rh->s.sub_ltd_rom_offset, aligned );
        if ( !FATFS_LoadBuffer( rh->s.sub_ltd_rom_offset, aligned ) ||
             PXI_RecvID() != FIRM_PXI_ID_AUTH_ARM7_LTD_STATIC )
        {
            return FALSE;
        }
#ifdef PROFILE_ENABLE
        // 9x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_ARM7_LTD_STATIC;    // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    }
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
    OSi_Boot( rh->s.sub_entry_address, (MIHeader_WramRegs*)rh->s.main_wram_config_data );
}
