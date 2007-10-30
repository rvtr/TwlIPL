/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     myFontequ.h

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

#ifndef	__MY_FONT_H__
#define	__MY_FONT_H__

// �����R�[�h�̒�` 

#define EOM_	   0x00			 // �I���R�[�h 

// �I�[�i�[���ҏW�p�{�^��----------------------
#define CODE_BUTTON_TOP_	0x200
#define VAR_BUTTON1_		CODE_BUTTON_TOP_
#define VAR_BUTTON2_		(CODE_BUTTON_TOP_ + 1)
#define DEL_BUTTON_			(CODE_BUTTON_TOP_ + 2)
#define CANCEL_BUTTON_		(CODE_BUTTON_TOP_ + 3)
#define OK_BUTTON_			(CODE_BUTTON_TOP_ + 4)
#define CODE_BUTTON_BOTTOM_	(CODE_BUTTON_TOP_ + 5)

/*
// �Ђ炪��----------------------------------------

#define a_         0x0f9         // "��" 
#define i_         0x0fa         // "��" 
#define u_         0x0fb         // "��" 
#define e_         0x0fc         // "��" 
#define o_         0x0fd         // "��" 

#define ka_        0x0fe         // "��" 
#define ki_        0x0ff         // "��" 
#define ku_        0x100         // "��" 
#define ke_        0x101         // "��" 
#define ko_        0x102         // "��" 

#define sa_        0x103         // "��" 
#define si_        0x104         // "��" 
#define su_        0x105         // "��" 
#define se_        0x106         // "��" 
#define so_        0x107         // "��" 

#define ta_        0x108         // "��" 
#define ti_        0x109         // "��" 
#define tu_        0x10a         // "��" 
#define te_        0x10b         // "��" 
#define to_        0x10c         // "��" 

#define na_        0x10d         // "��" 
#define ni_        0x10e         // "��" 
#define nu_        0x10f         // "��" 
#define ne_        0x110         // "��" 
#define no_        0x111         // "��" 

#define ha_        0x112         // "��" 
#define hi_        0x113         // "��" 
#define hu_        0x114         // "��" 
#define he_        0x115         // "��" 
#define ho_        0x116         // "��" 

#define ma_        0x117         // "��" 
#define mi_        0x118         // "��" 
#define mu_        0x119         // "��" 
#define me_        0x11a         // "��" 
#define mo_        0x11b         // "��" 

#define ya_        0x11c         // "��" 
#define yu_        0x11d         // "��" 
#define yo_        0x11e         // "��" 

#define ra_        0x11f         // "��" 
#define ri_        0x120         // "��" 
#define ru_        0x121         // "��" 
#define re_        0x122         // "��" 
#define ro_        0x123         // "��" 

#define wa_        0x124         // "��" 
#define wo_        0x0f8         // "��" 
#define n_         0x125         // "��" 


#define ga_        0x126         // "��" 
#define gi_        0x127         // "��" 
#define gu_        0x128         // "��" 
#define ge_        0x129         // "��" 
#define go_        0x12a         // "��" 
#define za_        0x12b         // "��" 
#define zi_        0x12c         // "��" 
#define zu_        0x12d         // "��" 
#define ze_        0x12e         // "��" 
#define zo_        0x12f         // "��" 
#define da_        0x130         // "��" 
#define di_        0x131         // "��" 
#define du_        0x132         // "��" 
#define de_        0x133         // "��" 
#define do_        0x134         // "��" 
#define ba_        0x135         // "��" 
#define bi_        0x136         // "��" 
#define bu_        0x137         // "��" 
#define be_        0x138         // "��" 
#define bo_        0x139         // "��" 
#define pa_        0x13a         // "��" 
#define pi_        0x13b         // "��" 
#define pu_        0x13c         // "��" 
#define pe_        0x13d         // "��" 
#define po_        0x13e         // "��" 

#define aa_        0x08d        // "��" 
#define ii_        0x08e        // "��" 
#define uu_        0x08f        // "��" 
#define ee_        0x090        // "��" 
#define oo_        0x091        // "��" 
#define yya_       0x092        // "��" 
#define yyu_       0x093        // "��" 
#define yyo_       0x094        // "��" 
#define ttu_       0x095        // "��" 


// �J�^�J�i----------------------------------------
#define A_         0x0b1         // "�A" 
#define I_         0x0b2         // "�C" 
#define U_         0x0b3         // "�E" 
#define E_         0x0b4         // "�G" 
#define O_         0x0b5         // "�I" 
#define KA_        0x0b6         // "�J" 
#define KI_        0x0b7         // "�L" 
#define KU_        0x0b8         // "�N" 
#define KE_        0x0b9         // "�P" 
#define KO_        0x0ba         // "�R" 
#define SA_        0x0bb         // "�T" 
#define SI_        0x0bc         // "�V" 
#define SU_        0x0bd         // "�X" 
#define SE_        0x0be         // "�Z" 
#define SO_        0x0bf         // "�\" 
#define TA_        0x0c0         // "�^" 
#define TI_        0x0c1         // "�`" 
#define TU_        0x0c2         // "�c" 
#define TE_        0x0c3         // "�e" 
#define TO_        0x0c4         // "�g" 
#define NA_        0x0c5         // "�i" 
#define NI_        0x0c6         // "�j" 
#define NU_        0x0c7         // "�k" 
#define NE_        0x0c8         // "�l" 
#define NO_        0x0c9         // "�m" 
#define HA_        0x0ca         // "�n" 
#define HI_        0x0cb         // "�q" 
#define HU_        0x0cc         // "�t" 
#define HE_        0x0cd         // "�w" 
#define HO_        0x0ce         // "�z" 
#define MA_        0x0cf         // "�}" 
#define MI_        0x0d0         // "�~" 
#define MU_        0x0d1         // "��" 
#define ME_        0x0d2         // "��" 
#define MO_        0x0d3         // "��" 
#define YA_        0x0d4         // "��" 
#define YU_        0x0d5         // "��" 
#define YO_        0x0d6         // "��" 
#define RA_        0x0d7         // "��" 
#define RI_        0x0d8         // "��" 
#define RU_        0x0d9         // "��" 
#define RE_        0x0da         // "��" 
#define RO_        0x0db         // "��" 
#define WA_        0x0dc         // "��" 
#define WO_        0x0a6         // "��" 
#define N_         0x0dd         // "��" 

#define GA_        0x0de         // "�K" 
#define GI_        0x0df         // "�M" 
#define GU_        0x0e0         // "�O" 
#define GE_        0x0e1         // "�Q" 
#define GO_        0x0e2         // "�S" 
#define ZA_        0x0e3         // "�U" 
#define ZI_        0x0e4         // "�W" 
#define ZU_        0x0e5         // "�Y" 
#define ZE_        0x0e6         // "�[" 
#define ZO_        0x0e7         // "�]" 
#define DA_        0x0e8         // "�_" 
#define DI_        0x0e9         // "�a" 
#define DU_        0x0ea         // "�d" 
#define DE_        0x0eb         // "�f" 
#define DO_        0x0ec         // "�h" 
#define BA_        0x0ed         // "�o" 
#define BI_        0x0ee         // "�r" 
#define BU_        0x0ef         // "�u" 
#define BE_        0x0f0         // "�x" 
#define BO_        0x0f1         // "�{" 
#define VU_        0x0f2         // "��" 
#define PA_        0x0f3         // "�p" 
#define PI_        0x0f4         // "�s" 
#define PU_        0x0f5         // "�v" 
#define PE_        0x0f6         // "�y" 
#define PO_        0x0f7         // "�|" 

#define AA_        0x0a7         // "�@" 
#define II_        0x0a8         // "�B" 
#define UU_        0x0a9         // "�D" 
#define EE_        0x0aa         // "�F" 
#define OO_        0x0ab         // "�H" 
#define YYA_       0x0ac         // "��" 
#define YYU_       0x0ad         // "��" 
#define YYO_       0x0ae         // "��" 
#define TTU_       0x0af         // "�b" 


// �A���t�@�x�b�g�啶��----------------------------
#define A__        0x041         // "�`" 
#define B__        0x042         // "�a" 
#define C__        0x043         // "�b" 
#define D__        0x044         // "�c" 
#define E__        0x045         // "�d" 
#define F__        0x046         // "�e" 
#define G__        0x047         // "�f" 
#define H__        0x048         // "�g" 
#define I__        0x049         // "�h" 
#define J__        0x04a         // "�i" 
#define K__        0x04b         // "�j" 
#define L__        0x04c         // "�k" 
#define M__        0x04d         // "�l" 
#define N__        0x04e         // "�m" 
#define O__        0x04f         // "�n" 
#define P__        0x050         // "�o" 
#define Q__        0x051         // "�p" 
#define R__        0x052         // "�q" 
#define S__        0x053         // "�r" 
#define T__        0x054         // "�s" 
#define U__        0x055         // "�t" 
#define V__        0x056         // "�u" 
#define W__        0x057         // "�v" 
#define X__        0x058         // "�w" 
#define Y__        0x059         // "�x" 
#define Z__        0x05a         // "�y" 


// �A���t�@�x�b�g������----------------------------
#define a__        0x061         // "��" 
#define b__        0x062         // "��" 
#define c__        0x063         // "��" 
#define d__        0x064         // "��" 
#define e__        0x065         // "��" 
#define f__        0x066         // "��" 
#define g__        0x067         // "��" 
#define h__        0x068         // "��" 
#define i__        0x069         // "��" 
#define j__        0x06a         // "��" 
#define k__        0x06b         // "��" 
#define l__        0x06c         // "��" 
#define m__        0x06d         // "��" 
#define n__        0x06e         // "��" 
#define o__        0x06f         // "��" 
#define p__        0x070         // "��" 
#define q__        0x071         // "��" 
#define r__        0x072         // "��" 
#define s__        0x073         // "��" 
#define t__        0x074         // "��" 
#define u__        0x075         // "��" 
#define v__        0x076         // "��" 
#define w__        0x077         // "��" 
#define x__        0x078         // "��" 
#define y__        0x079         // "��" 
#define z__        0x07a         // "��" 


// ����--------------------------------------------
#define n0_        0x030         // "�O" 
#define n1_        0x031         // "�P" 
#define n2_        0x032         // "�Q" 
#define n3_        0x033         // "�R" 
#define n4_        0x034         // "�S" 
#define n5_        0x035         // "�T" 
#define n6_        0x036         // "�U" 
#define n7_        0x037         // "�V" 
#define n8_        0x038         // "�W" 
#define n9_        0x039         // "�X" 

// �L��--------------------------------------------
#define spc_       0x020         // "�@" 
#define bicri_     0x021         // "�I" 
#define cyoncyon_  0x022         // "�h" 
#define sharp_     0x023         // "��" 
#define dollar_    0x024         // "��" 
#define percent_   0x025         // "��" 
#define and_       0x026         // "��" 
#define cyon_      0x027         // "�f" 
#define kakko_     0x028         // "�i" 
#define kakkot_    0x029         // "�j" 
#define kome_      0x02a         // "��" 
#define tasu_      0x02b         // "�{" 
#define comma_     0x02c         // "�C" 
#define hiku_      0x02d         // "�|" 
#define period_    0x02e         // "�D" 
#define sura_      0x02f         // "�^" 

#define colon_     0x03a         // "�F" 
#define semicolon_ 0x03b         // "�G" 
#define dainari_   0x03c         // "��" 
#define equal_     0x03d         // "��" 
#define syounari_  0x03e         // "��" 
#define hate_      0x03f         // "�H" 

#define atomark_   0x040         // "��" 
#define dkakko_    0x05b         // "�m" 
#define bsura_     0x05c         // "�^" 
#define dkakkot_   0x05d         // "�n" 
#define yama_      0x05e         // "�O" 
#define uscore_    0x05f         // "�Q" 

#define bcyon_     0x060         // "�e" 
#define ckakko_    0x07b         // "�o" 
#define mataha_    0x07c         // "�b" 
#define ckakkot_   0x07d         // "�p" 
#define kara_      0x07e         // "�`" 

#define kten_      0x0a1         // "�B" 
#define k_kakko_   0x0a2         // "�u" 
#define k_kakkot_  0x0a3         // "�v" 
#define tten_      0x0a4         // "�A" 
#define nakat_     0x0a5         // "�E" 
#define bou_       0x0b0         // "�[" 

#define yazi_      0x08c         // "��" 
*/

#endif	/* __MY_FONT_H_ */
