/*---------------------------------------------------------------------------*
  Project:  TwlFirm - ELF
  File:     elf.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef ELF_H_
#define ELF_H_

#include "misc.h"

/*---------------------------------------------------------
 型定義
 --------------------------------------------------------*/
typedef u32 Elf32_Addr;       /* size:4, align:4    Unsigned program address  */
typedef u16 Elf32_Half;       /* size:2, align:2    Unsigned medium int */
typedef u32 Elf32_Off;        /* size:4, align:4    Unsigned file offset */
typedef s32 Elf32_Sword;      /* size:4, align:4    Signed large int */
typedef u32 Elf32_Word;       /* size:4, align:4    Unsigned large int */

/*---------------------------------------------------------
 ELF Header
 --------------------------------------------------------*/
/* e_identのインデックス */
#define EI_MAG0       0        /* File identification */
#define EI_MAG1       1        /* File identification */
#define EI_MAG2       2        /* File identification */
#define EI_MAG3       3        /* File identification */
#define EI_CLASS      4        /* File class 0=invalid, 1=32bit, 2=64bit */
#define EI_DATA       5        /* Data encoding 0=invalid, 1=LSB, 2=MSB */
#define EI_VERSION    6        /* File version 現在は1 */
#define EI_PAD        7        /* Start of padding bytes */
#define EI_NIDENT    16        /* Size of e_ident[] */

typedef struct {
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half    e_type;      /* ELFの形式(再配置可能, 実行可能など) */
  Elf32_Half    e_machine;   /* ファイルで要求されるアーキテクチャ */
  Elf32_Word    e_version;   /* ELFフォーマットのバージョン（現在は1） */
  Elf32_Addr    e_entry;     /* プログラムのエントリポイント。指定無しなら0。 */
  Elf32_Off     e_phoff;     /* プログラムヘッダテーブルのファイル先頭からのオフセット */
  Elf32_Off     e_shoff;     /* セクションヘッダテーブルのファイル先頭からのオフセット */
  Elf32_Word    e_flags;     /* プロセッサ固有のフラグ */
  Elf32_Half    e_ehsize;    /* ELFヘッダのサイズ */
  Elf32_Half    e_phentsize; /* 1プログラムヘッダのサイズ */
  Elf32_Half    e_phnum;     /* プログラムヘッダの数 */
  Elf32_Half    e_shentsize; /* 1セクションヘッダのサイズ */
  Elf32_Half    e_shnum;     /* セクションヘッダの数 */
  Elf32_Half    e_shstrndx;  /* セクション名文字列テーブルセクションへのインデックス */
} Elf32_Ehdr;

/* e_ident[EI_*]の中身定義 */
#define ELFMAG0         0x7f
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'
#define ELFCLASSNONE    0        /* invalid */
#define ELFCLASS32      1        /* ARM and Thumb processors use 32-bit ELF. */
#define ELFCLASS64      2
#define ELFDATANONE     0        /* invalid */
#define ELFDATA2LSB     1        /* little-endian */
#define ELFDATA2MSB     2        /* big-endian */


/* [e_type] */
#define ET_NONE      0           /* No file type */
#define ET_REL       1           /* Re-locatable file */
#define ET_EXEC      2           /* Executable file */
#define ET_DYN       3           /* Shared object file */
#define ET_CORE      4           /* Core file */
#define ET_LOPROC    0xff00      /* Processor-specific */
#define ET_HIPROC    0xffff      /* Processor-specific */

/* [e_machine] */
#define EM_NONE         0        /* No machine */
#define EM_M32          1
#define EM_SPARC        2
#define EM_386          3
#define EM_68K          4
#define EM_88K          5
#define EM_860          7
#define EM_MIPS         8
#define EM_MIPS_RS4_BE 10
#define EM_ARM         40        /* ARM/Thumb Architecture */


/* [e_version] This member identifies the object file version.*/
#define EV_NONE       0          /* Invalid version */
#define EV_CURRENT    1          /* Current version */


/*
  ARM-specific e_flags
  e_flags Field           Value   Meaning
  EF_ARM_HASENTRY         (0x02)  e_entry contains a program-loader entry point
                                  (see section 4.1.1, Entry points, below).
  EF_ARM_SYMSARESORTED    (0x04)  Each subsection of the symbol table is sorted by symbol value
                                  (see section 4.4.8, Symbol table order, below)
  EF_ARM_DYNSYMSUSESEGIDX (0x8)   Symbols in dynamic symbol tables that are defined in sections 
                                  included in program segment n have st_shndx = n + 1. 
                                  (see section 4.4.9, Dynamic symbol table entries, below).
  EF_ARM_MAPSYMSFIRST     (0x10)  Mapping symbols precede other local symbols in the symbol table
                                  (see section 4.4.8, Symbol table order, below).

  EF_ARM_EABIMASK         (0xFF000000)(current version is 0x02000000)
                                  This masks an 8-bit version number, the version of the ARM
                                  EABI to which this ELF file conforms. This EABI is version 2. A
                                  value of 0 denotes unknown conformance.
*/
#define EF_ARM_HASENTRY         0x02
#define EF_ARM_SYMSARESORTED    0x04
#define EF_ARM_DYNSYMSUSESEGIDX 0x8
#define EF_ARM_MAPSYMSFIRST     0x10
#define EF_ARM_EABIMASK         0xFF000000


/*---------------------------------------------------------
 Program headers
 --------------------------------------------------------*/
typedef struct {
  Elf32_Word p_type;
  Elf32_Off  p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} Elf32_Phdr;

/* [p_type] */
#define PT_NULL    0 /* 使われないエントリで、他のメンバの値の意味は未定義 */
#define PT_LOAD    1 /* 実行時にロードされるセグメント */
#define PT_DYNAMIC 2 /* 動的構造体配列を保持するセグメント */
#define PT_INTERP  3 /* ファイルの解釈に使われるインタプリタのパスを保持するセグメント */
#define PT_NOTE    4 /* ファイルの解釈には使われない情報を保持するセグメント */
#define PT_SHLIB   5 /* 予約 */
#define PT_PHDR    6 /* プログラムヘッダテーブル（プログラムのメモリイメージの一部である場合のみ存在） */
//#define PT_TLS   ? /* スレッド局所記憶領域のテンプレート */

#define PT_LOOS    0x60000000  /* OS固有に予約された領域 */
#define PT_HIOS    0x6fffffff

#define PT_LOPROC  0x70000000  /* プロセッサ固有に予約された領域 */
#define PT_HIPROC  0x7fffffff

/* [p_flags]*/
#define PF_X         1          /*実行可能*/
#define PF_W         2          /*書き込み可能*/
#define PF_R         4          /*読み出し可能*/
#define PF_ARM_SB    0x10000000 /*The segment contains the location addressed by the static base*/
#define PF_ARM_PI    0x20000000 /*The segment is position-independent*/
#define PF_ARM_ENTRY 0x80000000 /*The segment contains the entry point*/
#define PF_MASKPROC  0xf0000000


/*---------------------------------------------------------
 Section headers
 --------------------------------------------------------*/
typedef struct {
  Elf32_Word    sh_name;        /*セクションヘッダ文字列テーブルセクションのインデックス*/
  Elf32_Word    sh_type;        /* タイプ（下記定義参照） */
  Elf32_Word    sh_flags;
  Elf32_Addr    sh_addr;        /*  */
  Elf32_Off     sh_offset;      /* ファイルの先頭からのオフセット */
  Elf32_Word    sh_size;        /* バイト単位のサイズ */
  Elf32_Word    sh_link;        /* sh_typeによって値の意味が変わる */
  Elf32_Word    sh_info;        /* sh_typeによって値の意味が変わる */
  Elf32_Word    sh_addralign;   /* アラインメント制限(0or1で制限なし,4で4ByteAlign) */
  Elf32_Word    sh_entsize;     /* 固定サイズのエントリテーブルがある場合、1要素のサイズ */
} Elf32_Shdr;

/* sh_addr mod sh_addralign = 0 でなければならない */

/* Section Types, [sh_type] */
#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB   10
#define SHT_DYNSYM  11
#define SHT_LOPROC  0x70000000
#define SHT_HIPROC  0x7fffffff
#define SHT_LOUSER  0x80000000
#define SHT_HIUSER  0xffffffff


/* [sh_flags] */
#define SHF_WRITE     0x1
#define SHF_ALLOC     0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC  0xf0000000
/* ARM-EABI-specific */
#define SHF_ENTRYSECT      0x10000000   /* The section contains an entry point.  */
#define SHF_COMDEF         0x80000000   /* The section may be multiply defined in the input to a link step.  */
/* others */
#define SHF_LINK_ORDER     0x80

/*セクションインデックス*/
//Sym->st_shndxなど
#define SHN_UNDEF          0
#define SHN_LORESERVE      0xff00
#define SHN_LOPROC         0xff00
#define SHN_HIPROC         0xff1f
#define SHN_ABS            0xfff1
#define SHN_COMMON         0xfff2
#define SHN_HIRESERVE      0xffff


//ここからはヘッダでなく実体データ構造

/*---------------------------------------------------------
 Symbol Table Entry
 --------------------------------------------------------*/
typedef struct {
  Elf32_Word    st_name;    /* シンボル文字列テーブルのインデックス */
  Elf32_Addr    st_value;   /* おそらく関連するセクション内でのオフセット値 */
  Elf32_Word    st_size;    /* サイズがないか、不明な場合は 0 */
  unsigned char st_info;    /* バインド と タイプ */
  unsigned char st_other;   /* 現在は 0 が入る */
  Elf32_Half    st_shndx;   /* 関連するセクションヘッダテーブルのインデックス */
} Elf32_Sym;


/* st_info */
#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

/* st_info の BIND */
#define STB_LOCAL   0
#define STB_GLOBAL  1
#define STB_WEAK    2
#define STB_LOPROC 13
#define STB_HIPROC 15

/* st_info の TYPE */
#define STT_NOTYPE  0        /*未定義*/
#define STT_OBJECT  1        /*データオブジェクト*/
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_LOPROC 13
#define STT_HIPROC 15


/*---------------------------------------------------------
 Relocation Entry
 --------------------------------------------------------*/
typedef struct {
  Elf32_Addr r_offset;
  Elf32_Word r_info;
} Elf32_Rel;

typedef struct {
  Elf32_Addr r_offset;
  Elf32_Word r_info;
  Elf32_Sword r_addend;
} Elf32_Rela;

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))


/* r_info の TYPE */
#define R_ARM_NONE        0 /* Any No relocation. Encodes dependencies between sections. */
#define R_ARM_PC24        1 /* ARM B/BL S . P + A */
#define R_ARM_ABS32       2 /* 32-bit word S + A */
#define R_ARM_REL32       3 /* 32-bit word S . P + A */
#define R_ARM_PC13        4 /* ARM LDR r, [pc,…] S . P + A */
#define R_ARM_ABS16       5 /* 16-bit half-word S + A */
#define R_ARM_ABS12       6 /* ARM LDR/STR S + A */
#define R_ARM_THM_ABS5    7 /* Thumb LDR/STR S + A */
#define R_ARM_ABS8        8 /* 8-bit byte S + A */
#define R_ARM_SBREL32     9 /* 32-bit word S . B + A */
#define R_ARM_THM_PC22   10 /* Thumb BL pair S . P+ A */
#define R_ARM_THM_PC8    11 /* Thumb LDR r, [pc,…] S . P + A */
#define R_ARM_AMP_VCALL9 12 /* AMP VCALL Obsolete.SA-1500 only. */
#define R_ARM_SWI24      13 /* ARM SWI S + A */
#define R_ARM_THM_SWI8   14 /* Thumb SWI S + A */
#define R_ARM_XPC25      15 /* ARM BLX S . P+ A */
#define R_ARM_THM_XPC22  16 /* Thumb BLX pair S . P+ A */

/* 17-31, reserved to ARM Linux */
//17-19 Reserved to ARM LINUX
#define R_ARM_COPY      20 /* 32 bit word Copy symbol at dynamic link time. */
#define R_ARM_GLOB_DAT  21 /* 32 bit word Create GOT entry. */
#define R_ARM_JUMP_SLOT 22 /* 32 bit word Create PLT entry. */
#define R_ARM_RELATIVE  23 /* 32 bit word Adjust by program base. */
#define R_ARM_GOTOFF    24 /* 32 bit word Offset relative to start of GOT. */
#define R_ARM_GOTPC     25 /* 32 bit word Insert address of GOT. */
#define R_ARM_GOT32     26 /* 32 bit word Entry in GOT. */
#define R_ARM_PLT32     27 /* ARM BL Entry in PLT. */

/* 28-31 Reserved to ARM LINUX */
#define R_ARM_ALU_PCREL_7_0   32 /* ARM ADD/SUB (S . P + A) & 0x000000FF */
#define R_ARM_ALU_PCREL_15_8  33 /* ARM ADD/SUB (S . P + A) & 0x0000FF00 */
#define R_ARM_ALU_PCREL_23_15 34 /* ARM ADD/SUB (S . P + A) & 0x00FF0000 */
#define R_ARM_LDR_SBREL_11_0  35 /* ARM LDR/STR (S . B + A) & 0x00000FFF */
#define R_ARM_ALU_SBREL_19_12 36 /* ARM ADD/SUB (S . B + A) & 0x000FF000 */
#define R_ARM_ALU_SBREL_27_20 37 /* ARM ADD/SUB (S . B + A) & 0x0FF00000 */

#define R_ARM_TARGET1     38
#define R_ARM_ROSEGREL32  39
#define R_ARM_V4BX        40
#define R_ARM_TARGET2     41
#define R_ARM_PREL31      42

/* 96-111, reserved to ARM g++ */
#define R_ARM_GNU_VTENTRY   100 /* 32 bit word Record C++ vtable entry. */
#define R_ARM_GNU_VTINHERIT 101 /* 32 bit word Record C++ member usage. */
#define R_ARM_THM_PC11      102 /* Thumb B S . P + A */
#define R_ARM_THM_PC9       103 /* Thumb B<cond> S . P + A */

/* 112-127, reserved for private experiments */

/* 128-248, reserved to ARM */
#define R_ARM_RXPC25    249 /* ARM BLX (ΔS . ΔP) + A #define For calls between program segments. */
#define R_ARM_RSBREL32  250 /* Word (ΔS . ΔSB) + A For an offset from SB, the static base. */
#define R_ARM_THM_RPC22 251 /* Thumb BL/BLX pair (ΔS . ΔP) + A For calls between program segments. */
#define R_ARM_RREL32    252 /* Word (ΔS . ΔP) + A For on offset between two segments. */
#define R_ARM_RABS32    253 /* Word ΔS + A For the address of a location in the target segment. */
#define R_ARM_RPC24     254 /* ARM B/BL (ΔS . ΔP) + A For calls between program segments. */
#define R_ARM_RBASE     255 /* None None.Identifies the segment being relocated by the following 
                   relocation directives. The ARM EABI poses two problems for relocating 
                   executables and shared objects encoded in */


// shirait
#define R_ARM_LDR_PC_G0        4    //LDR

#define R_ARM_ABS12            6    //LDR, STR

#define R_ARM_THM_CALL        10    //R_ARM_THM_PC22と同じ

#define R_ARM_CALL            28    //BL/BLX
#define R_ARM_JUMP24          29    //B/BL<cond>
#define R_ARM_THM_JUMP24      30

#define R_ARM_MOVW_ABS_NC     43    //MOVW
#define R_ARM_MOVT_ABS        44    //MOVT
#define R_ARM_MOVW_PREL_NC    45    //MOVW
#define R_ARM_MOVT_PREL       46    //MOVT

#define R_ARM_ALU_PC_G0_NC    57    //ADD, SUB
#define R_ARM_ALU_PC_G0       58    //ADD, SUB
#define R_ARM_ALU_PC_G1_NC    59    //ADD, SUB
#define R_ARM_ALU_PC_G1       60    //ADD, SUB
#define R_ARM_ALU_PC_G2       61    //ADD, SUB
#define R_ARM_LDR_PC_G1       62    //LDR, STR, LDRB, STRB
#define R_ARM_LDR_PC_G2       63    //LDR, STR, LDRB, STRB
#define R_ARM_LDRS_PC_G0      64    //LDRD, STRD, LDRH, STRH, LDRSH, LDRSB
#define R_ARM_LDRS_PC_G1      65    //LDRD, STRD, LDRH, STRH, LDRSH, LDRSB
#define R_ARM_LDRS_PC_G2      66    //LDRD, STRD, LDRH, STRH, LDRSH, LDRSB
#define R_ARM_LDC_PC_G0       67    //LDC, STC
#define R_ARM_LDC_PC_G1       68    //LDC, STC
#define R_ARM_LDC_PC_G2       69    //LDC, STC
#define R_ARM_ALU_SB_G0_NC    70    //ADD, SUB
#define R_ARM_ALU_SB_G0       71    //ADD, SUB
#define R_ARM_ALU_SB_G1_NC    72    //ADD, SUB
#define R_ARM_ALU_SB_G1       73    //ADD, SUB
#define R_ARM_ALU_SB_G2       74    //ADD, SUB
#define R_ARM_LDR_SB_G0       75    //LDR, STR, LDRB, STRB
#define R_ARM_LDR_SB_G1       76    //LDR, STR, LDRB, STRB
#define R_ARM_LDR_SB_G2       77    //LDR, STR, LDRB, STRB
#define R_ARM_LDRS_SB_G0      78    //LDRD, STRD, LDRH, STRH, LDRSH, LDRSB
#define R_ARM_LDRS_SB_G1      79    //LDRD, STRD, LDRH, STRH, LDRSH, LDRSB
#define R_ARM_LDRS_SB_G2      80    //LDRD, STRD, LDRH, STRH, LDRSH, LDRSB
#define R_ARM_LDC_SB_G0       81    //LDC, STC
#define R_ARM_LDC_SB_G1       82    //LDC, STC
#define R_ARM_LDC_SB_G2       83    //LDC, STC
#define R_ARM_MOVW_BREL_NC    84    //MOVW
#define R_ARM_MOVT_BREL       85    //MOVT
#define R_ARM_MOVW_BREL       86    //MOVW

#define R_ARM_GOT_BREL12      97    //LDR
#define R_ARM_GOTOFF12        98    //LDR, STR

#define R_ARM_TLS_LDO12      109    //LDR, STR
#define R_ARM_TLS_LE12       110    //LDR, STR
#define R_ARM_TLS_TE12GP     111    //LDR



/*---------------------------------------------------------
 Dynamic Section elf_v1.2
 --------------------------------------------------------*/
typedef struct {
  Elf32_Sword d_tag;
  union {
    Elf32_Word d_val;
    Elf32_Addr d_ptr;
  } d_un;
} Elf32_Dyn;


/* Additional symbol types for Thumb.  */
#define STT_ARM_TFUNC      STT_LOPROC   /* A Thumb function.  */
#define STT_ARM_16BIT      STT_HIPROC   /* A Thumb label.  */









/*---------------------------------------------------------
 ELFヘッダを読み出す
 --------------------------------------------------------*/
void *ELF_LoadELFHeader(const void *buf, Elf32_Ehdr *ehdr);



#endif /* ELF_H_ */

