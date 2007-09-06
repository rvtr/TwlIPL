
#ifndef __SDMC_H__
#define __SDMC_H__

//#include <firm.h>
//#include <rtfs.h>


#ifdef __cplusplus
extern "C" {
#endif



/*********************************************
 ポート番号
*********************************************/
typedef enum {
    SDMC_PORT_CARD    = 0x400,
    SDMC_PORT_NAND    = 0x401
}SDMC_PORT_NO;


/*********************************************
 DMA番号
*********************************************/
typedef enum {
    SDMC_USE_DMA_0    = 0,
    SDMC_USE_DMA_1    = 1,
    SDMC_USE_DMA_2    = 2,
    SDMC_USE_DMA_3    = 3,
    SDMC_NOUSE_DMA    = 0xFF
}SDMC_DMA_NO;


/*********************************************
 カードエラーコード（カードエラーステータス設定値）アプリケーション固有のSDCARD_ErrStatusに対して
*********************************************/
typedef enum {
    SDMC_NORMAL             =   0,      /* 正常終了 */
    SDMC_ERR_COMMAND        =   0x0001, /* CMDエラー */
    SDMC_ERR_CRC            =   0x0002, /* CRCエラー */
    SDMC_ERR_END            =   0x0004, /* 実行エラー */
    SDMC_ERR_TIMEOUT        =   0x0008, /* コマンドタイムアウト */
    SDMC_ERR_FIFO_OVF       =   0x0010, /* FIFO オーバーフローエラー(INFO2のIllegal write access to buffer) */
    SDMC_ERR_FIFO_UDF       =   0x0020, /* FIFO アンダーフローエラー(INFO2のIllegal read access to buffer) */
    SDMC_ERR_WP             =   0x0040, /* WriteProtectによる書き込みエラー */
    SDMC_ERR_FPGA_TIMEOUT   =   0x0100, /* FPGA アクセスタイムアウト */
    SDMC_ERR_PARAM          =   0x0200, /* コマンドパラメータエラー */
    SDMC_ERR_R1_STATUS      =   0x0800, /* Normal response command カードステータス エラー */
    SDMC_ERR_NUM_WR_SECTORS =   0x1000, /* 書き込み完了セクタ数 エラー */
    SDMC_ERR_RESET          =   0x2000, /* 初期化カードリセットコマンド時1.5秒タイムアウトエラー */
    SDMC_ERR_ILA            =   0x4000, /* イリーガルアクセスエラー */
    SDMC_ERR_INFO_DETECT    =   0x8000  /* カード排出時判別エラービット(IO3) */
}SDMC_ERR_CODE;


/*********************************************
 SDドライバ処理結果通知情報構造体
*********************************************/
typedef struct {
    u16    b_flags;                     /* 処理内容         */
    u16    result;                      /* 実行結果         */
    u32    resid;                       /* 読み(書き)サイズ */
} SdmcResultInfo;





/*********************************************
 SDポート状態保存用構造体
*********************************************/
typedef struct
{
    u16           SD_CID[8];    /* CID保存用 (Card IDentification register) : ID*/
    u16           SD_CSD[8];    /* CSD保存用 (Card Specific Data register) : spec*/
    u16           SD_OCR[2];    /* OCR保存用 (Operation Condition Register) : voltage and status*/
    u16           SD_SCR[4];    /* SCR保存用 (Sd card Configulation Register) : bus-width, card-ver, etc*/
    u16           SD_RCA;       /* RCA保存用 (Relative Card Address register) : address*/
    s16           MMCFlag;
    s16           SDHCFlag;
    s16           SDFlag;
    SDMC_ERR_CODE ErrStatus;    /* SDCARD_ErrStatus */
    u32           Status;       /* SDCARD_Status */
    u16           SD_CLK_CTRL_VALUE;
    u16           SD_OPTION_VALUE;

    s16           OutFlag;
    u16           port_no;
}
SDPortContext;





/*********************************************
 SDスペック構造体
*********************************************/
typedef struct {
    u32     csd_ver2_flag;              //CSDフォーマットバージョン(SDHCのときは1)
    u32     memory_capacity;            //data areaのサイズ(512Byte単位)
    u32     protected_capacity;         //protected areaのサイズ(512Byte単位)
    u32     card_capacity;              //カード全体のサイズ(512Byte単位)

    u32     adjusted_memory_capacity;   //memory_capacityをシリンダ(heads*secptrack)の倍数に調整したサイズ(cylinders*heads*secptrackになる)
      
    u16     heads;
    u16     secptrack;
    u16     cylinders;
    u16     SC;                         //sectors per cluster
    u16     BU;
    u16     RDE;                        //number of root dir entries(512 fix)
    u32     SS;                         //sector size(512 fix)
    u32     RSC;                        //reserved sector count(1 fix)
//    u32     TS;                         //total sectors
    u16     FATBITS;                    //16 or 32
    u16     SF;                         //sectors per FAT
    u32     SSA;                        //sectors in system area
    u32     NOM;                        //sectors in master boot record
} SdmcSpec;


/*********************************************
 RTFS用ドライバインタフェース
*********************************************/
#if 0
BOOL sdmcRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading);
int  sdmcRtfsCtrl( int driveno, int opcode, void* pargs);
BOOL sdmcRtfsAttach( int driveno);
#endif

BOOL sdmcCheckMedia( void);



/*********************************************
 基本API
*********************************************/
void             sdmcClearPortContext( SDPortContext* buf_adr);
SDMC_ERR_CODE    sdmcCheckPortContext( SDPortContext* buf_adr);


SDMC_ERR_CODE    sdmcInit( SDMC_DMA_NO dma_no, void (*func1)(void),void (*func2)(void));   /* カードドライバ初期化 */
SDMC_ERR_CODE    sdmcReset( void);                                    /* カードリセット */

SDMC_ERR_CODE    sdmcGetStatus(u16 *status);           /* カードドライバの現在の状態を取得する */
u32              sdmcGetCardSize(void);                /* カード全サイズの取得 */

/*SD I/FのFIFOを使ってリードする（高速）*/
SDMC_ERR_CODE    sdmcReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);/* テスト用カードリード */
/*リードする*/
SDMC_ERR_CODE    sdmcRead(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);    /* テスト用カードリード */

/*SD I/FのFIFOを使ってライトする（高速）*/
SDMC_ERR_CODE    sdmcWriteFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);/* テスト用カードライト */
/*ライトする*/
SDMC_ERR_CODE    sdmcWrite(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);    /* テスト用カードライト */

u16           sdmcSelectedNo(void);
SDMC_ERR_CODE sdmcSelect( u16 select);


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif /*__SDMC_H__*/
