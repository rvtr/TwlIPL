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

// ï∂éöÉRÅ[ÉhÇÃíËã` 

#define EOM_	   0x00			 // èIóπÉRÅ[Éh 

// ÉIÅ[ÉiÅ[èÓïÒï“èWópÉ{É^Éì----------------------
#define CODE_BUTTON_TOP_	0x200
#define VAR_BUTTON1_		CODE_BUTTON_TOP_
#define VAR_BUTTON2_		(CODE_BUTTON_TOP_ + 1)
#define DEL_BUTTON_			(CODE_BUTTON_TOP_ + 2)
#define CANCEL_BUTTON_		(CODE_BUTTON_TOP_ + 3)
#define OK_BUTTON_			(CODE_BUTTON_TOP_ + 4)
#define CODE_BUTTON_BOTTOM_	(CODE_BUTTON_TOP_ + 5)

/*
// Ç–ÇÁÇ™Ç»----------------------------------------

#define a_         0x0f9         // "Ç†" 
#define i_         0x0fa         // "Ç¢" 
#define u_         0x0fb         // "Ç§" 
#define e_         0x0fc         // "Ç¶" 
#define o_         0x0fd         // "Ç®" 

#define ka_        0x0fe         // "Ç©" 
#define ki_        0x0ff         // "Ç´" 
#define ku_        0x100         // "Ç≠" 
#define ke_        0x101         // "ÇØ" 
#define ko_        0x102         // "Ç±" 

#define sa_        0x103         // "Ç≥" 
#define si_        0x104         // "Çµ" 
#define su_        0x105         // "Ç∑" 
#define se_        0x106         // "Çπ" 
#define so_        0x107         // "Çª" 

#define ta_        0x108         // "ÇΩ" 
#define ti_        0x109         // "Çø" 
#define tu_        0x10a         // "Ç¬" 
#define te_        0x10b         // "Çƒ" 
#define to_        0x10c         // "Ç∆" 

#define na_        0x10d         // "Ç»" 
#define ni_        0x10e         // "Ç…" 
#define nu_        0x10f         // "Ç " 
#define ne_        0x110         // "ÇÀ" 
#define no_        0x111         // "ÇÃ" 

#define ha_        0x112         // "ÇÕ" 
#define hi_        0x113         // "Ç–" 
#define hu_        0x114         // "Ç”" 
#define he_        0x115         // "Ç÷" 
#define ho_        0x116         // "ÇŸ" 

#define ma_        0x117         // "Ç‹" 
#define mi_        0x118         // "Ç›" 
#define mu_        0x119         // "Çﬁ" 
#define me_        0x11a         // "Çﬂ" 
#define mo_        0x11b         // "Ç‡" 

#define ya_        0x11c         // "Ç‚" 
#define yu_        0x11d         // "Ç‰" 
#define yo_        0x11e         // "ÇÊ" 

#define ra_        0x11f         // "ÇÁ" 
#define ri_        0x120         // "ÇË" 
#define ru_        0x121         // "ÇÈ" 
#define re_        0x122         // "ÇÍ" 
#define ro_        0x123         // "ÇÎ" 

#define wa_        0x124         // "ÇÌ" 
#define wo_        0x0f8         // "Ç" 
#define n_         0x125         // "ÇÒ" 


#define ga_        0x126         // "Ç™" 
#define gi_        0x127         // "Ç¨" 
#define gu_        0x128         // "ÇÆ" 
#define ge_        0x129         // "Ç∞" 
#define go_        0x12a         // "Ç≤" 
#define za_        0x12b         // "Ç¥" 
#define zi_        0x12c         // "Ç∂" 
#define zu_        0x12d         // "Ç∏" 
#define ze_        0x12e         // "Ç∫" 
#define zo_        0x12f         // "Çº" 
#define da_        0x130         // "Çæ" 
#define di_        0x131         // "Ç¿" 
#define du_        0x132         // "Ç√" 
#define de_        0x133         // "Ç≈" 
#define do_        0x134         // "Ç«" 
#define ba_        0x135         // "ÇŒ" 
#define bi_        0x136         // "Ç—" 
#define bu_        0x137         // "Ç‘" 
#define be_        0x138         // "Ç◊" 
#define bo_        0x139         // "Ç⁄" 
#define pa_        0x13a         // "Çœ" 
#define pi_        0x13b         // "Ç“" 
#define pu_        0x13c         // "Ç’" 
#define pe_        0x13d         // "Çÿ" 
#define po_        0x13e         // "Ç€" 

#define aa_        0x08d        // "Çü" 
#define ii_        0x08e        // "Ç°" 
#define uu_        0x08f        // "Ç£" 
#define ee_        0x090        // "Ç•" 
#define oo_        0x091        // "Çß" 
#define yya_       0x092        // "Ç·" 
#define yyu_       0x093        // "Ç„" 
#define yyo_       0x094        // "ÇÂ" 
#define ttu_       0x095        // "Ç¡" 


// ÉJÉ^ÉJÉi----------------------------------------
#define A_         0x0b1         // "ÉA" 
#define I_         0x0b2         // "ÉC" 
#define U_         0x0b3         // "ÉE" 
#define E_         0x0b4         // "ÉG" 
#define O_         0x0b5         // "ÉI" 
#define KA_        0x0b6         // "ÉJ" 
#define KI_        0x0b7         // "ÉL" 
#define KU_        0x0b8         // "ÉN" 
#define KE_        0x0b9         // "ÉP" 
#define KO_        0x0ba         // "ÉR" 
#define SA_        0x0bb         // "ÉT" 
#define SI_        0x0bc         // "ÉV" 
#define SU_        0x0bd         // "ÉX" 
#define SE_        0x0be         // "ÉZ" 
#define SO_        0x0bf         // "É\" 
#define TA_        0x0c0         // "É^" 
#define TI_        0x0c1         // "É`" 
#define TU_        0x0c2         // "Éc" 
#define TE_        0x0c3         // "Ée" 
#define TO_        0x0c4         // "Ég" 
#define NA_        0x0c5         // "Éi" 
#define NI_        0x0c6         // "Éj" 
#define NU_        0x0c7         // "Ék" 
#define NE_        0x0c8         // "Él" 
#define NO_        0x0c9         // "Ém" 
#define HA_        0x0ca         // "Én" 
#define HI_        0x0cb         // "Éq" 
#define HU_        0x0cc         // "Ét" 
#define HE_        0x0cd         // "Éw" 
#define HO_        0x0ce         // "Éz" 
#define MA_        0x0cf         // "É}" 
#define MI_        0x0d0         // "É~" 
#define MU_        0x0d1         // "ÉÄ" 
#define ME_        0x0d2         // "ÉÅ" 
#define MO_        0x0d3         // "ÉÇ" 
#define YA_        0x0d4         // "ÉÑ" 
#define YU_        0x0d5         // "ÉÜ" 
#define YO_        0x0d6         // "Éà" 
#define RA_        0x0d7         // "Éâ" 
#define RI_        0x0d8         // "Éä" 
#define RU_        0x0d9         // "Éã" 
#define RE_        0x0da         // "Éå" 
#define RO_        0x0db         // "Éç" 
#define WA_        0x0dc         // "Éè" 
#define WO_        0x0a6         // "Éí" 
#define N_         0x0dd         // "Éì" 

#define GA_        0x0de         // "ÉK" 
#define GI_        0x0df         // "ÉM" 
#define GU_        0x0e0         // "ÉO" 
#define GE_        0x0e1         // "ÉQ" 
#define GO_        0x0e2         // "ÉS" 
#define ZA_        0x0e3         // "ÉU" 
#define ZI_        0x0e4         // "ÉW" 
#define ZU_        0x0e5         // "ÉY" 
#define ZE_        0x0e6         // "É[" 
#define ZO_        0x0e7         // "É]" 
#define DA_        0x0e8         // "É_" 
#define DI_        0x0e9         // "Éa" 
#define DU_        0x0ea         // "Éd" 
#define DE_        0x0eb         // "Éf" 
#define DO_        0x0ec         // "Éh" 
#define BA_        0x0ed         // "Éo" 
#define BI_        0x0ee         // "Ér" 
#define BU_        0x0ef         // "Éu" 
#define BE_        0x0f0         // "Éx" 
#define BO_        0x0f1         // "É{" 
#define VU_        0x0f2         // "Éî" 
#define PA_        0x0f3         // "Ép" 
#define PI_        0x0f4         // "És" 
#define PU_        0x0f5         // "Év" 
#define PE_        0x0f6         // "Éy" 
#define PO_        0x0f7         // "É|" 

#define AA_        0x0a7         // "É@" 
#define II_        0x0a8         // "ÉB" 
#define UU_        0x0a9         // "ÉD" 
#define EE_        0x0aa         // "ÉF" 
#define OO_        0x0ab         // "ÉH" 
#define YYA_       0x0ac         // "ÉÉ" 
#define YYU_       0x0ad         // "ÉÖ" 
#define YYO_       0x0ae         // "Éá" 
#define TTU_       0x0af         // "Éb" 


// ÉAÉãÉtÉ@ÉxÉbÉgëÂï∂éö----------------------------
#define A__        0x041         // "Ç`" 
#define B__        0x042         // "Ça" 
#define C__        0x043         // "Çb" 
#define D__        0x044         // "Çc" 
#define E__        0x045         // "Çd" 
#define F__        0x046         // "Çe" 
#define G__        0x047         // "Çf" 
#define H__        0x048         // "Çg" 
#define I__        0x049         // "Çh" 
#define J__        0x04a         // "Çi" 
#define K__        0x04b         // "Çj" 
#define L__        0x04c         // "Çk" 
#define M__        0x04d         // "Çl" 
#define N__        0x04e         // "Çm" 
#define O__        0x04f         // "Çn" 
#define P__        0x050         // "Ço" 
#define Q__        0x051         // "Çp" 
#define R__        0x052         // "Çq" 
#define S__        0x053         // "Çr" 
#define T__        0x054         // "Çs" 
#define U__        0x055         // "Çt" 
#define V__        0x056         // "Çu" 
#define W__        0x057         // "Çv" 
#define X__        0x058         // "Çw" 
#define Y__        0x059         // "Çx" 
#define Z__        0x05a         // "Çy" 


// ÉAÉãÉtÉ@ÉxÉbÉgè¨ï∂éö----------------------------
#define a__        0x061         // "ÇÅ" 
#define b__        0x062         // "ÇÇ" 
#define c__        0x063         // "ÇÉ" 
#define d__        0x064         // "ÇÑ" 
#define e__        0x065         // "ÇÖ" 
#define f__        0x066         // "ÇÜ" 
#define g__        0x067         // "Çá" 
#define h__        0x068         // "Çà" 
#define i__        0x069         // "Çâ" 
#define j__        0x06a         // "Çä" 
#define k__        0x06b         // "Çã" 
#define l__        0x06c         // "Çå" 
#define m__        0x06d         // "Çç" 
#define n__        0x06e         // "Çé" 
#define o__        0x06f         // "Çè" 
#define p__        0x070         // "Çê" 
#define q__        0x071         // "Çë" 
#define r__        0x072         // "Çí" 
#define s__        0x073         // "Çì" 
#define t__        0x074         // "Çî" 
#define u__        0x075         // "Çï" 
#define v__        0x076         // "Çñ" 
#define w__        0x077         // "Çó" 
#define x__        0x078         // "Çò" 
#define y__        0x079         // "Çô" 
#define z__        0x07a         // "Çö" 


// êîéö--------------------------------------------
#define n0_        0x030         // "ÇO" 
#define n1_        0x031         // "ÇP" 
#define n2_        0x032         // "ÇQ" 
#define n3_        0x033         // "ÇR" 
#define n4_        0x034         // "ÇS" 
#define n5_        0x035         // "ÇT" 
#define n6_        0x036         // "ÇU" 
#define n7_        0x037         // "ÇV" 
#define n8_        0x038         // "ÇW" 
#define n9_        0x039         // "ÇX" 

// ãLçÜ--------------------------------------------
#define spc_       0x020         // "Å@" 
#define bicri_     0x021         // "ÅI" 
#define cyoncyon_  0x022         // "Åh" 
#define sharp_     0x023         // "Åî" 
#define dollar_    0x024         // "Åî" 
#define percent_   0x025         // "Åì" 
#define and_       0x026         // "Åï" 
#define cyon_      0x027         // "Åf" 
#define kakko_     0x028         // "Åi" 
#define kakkot_    0x029         // "Åj" 
#define kome_      0x02a         // "Åñ" 
#define tasu_      0x02b         // "Å{" 
#define comma_     0x02c         // "ÅC" 
#define hiku_      0x02d         // "Å|" 
#define period_    0x02e         // "ÅD" 
#define sura_      0x02f         // "Å^" 

#define colon_     0x03a         // "ÅF" 
#define semicolon_ 0x03b         // "ÅG" 
#define dainari_   0x03c         // "ÅÉ" 
#define equal_     0x03d         // "ÅÅ" 
#define syounari_  0x03e         // "ÅÑ" 
#define hate_      0x03f         // "ÅH" 

#define atomark_   0x040         // "Åó" 
#define dkakko_    0x05b         // "Åm" 
#define bsura_     0x05c         // "Å^" 
#define dkakkot_   0x05d         // "Ån" 
#define yama_      0x05e         // "ÅO" 
#define uscore_    0x05f         // "ÅQ" 

#define bcyon_     0x060         // "Åe" 
#define ckakko_    0x07b         // "Åo" 
#define mataha_    0x07c         // "Åb" 
#define ckakkot_   0x07d         // "Åp" 
#define kara_      0x07e         // "Å`" 

#define kten_      0x0a1         // "ÅB" 
#define k_kakko_   0x0a2         // "Åu" 
#define k_kakkot_  0x0a3         // "Åv" 
#define tten_      0x0a4         // "ÅA" 
#define nakat_     0x0a5         // "ÅE" 
#define bou_       0x0b0         // "Å[" 

#define yazi_      0x08c         // "Å•" 
*/

#endif	/* __MY_FONT_H_ */
