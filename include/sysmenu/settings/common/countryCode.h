/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     countryCode.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-06#$
  $Rev: 104 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/


#ifndef	COUNTRY_CODE_H_
#define	COUNTRY_CODE_H_
#if		defined(SDK_CW)							// NTRConfigDataにビットフィールドを使っているので、コンパイラ依存で不具合が発生する可能性がある。
												// よって、CW以外のコンパイラの場合は、このヘッダを無効にしてエラーを出させるようにして再確認する。

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------


// 言語設定コード
typedef enum TWLCountryCode{
	TWL_COUNTRY_UNDEFINED	= 0,		// 未設定

	// JPNリージョン
	TWL_COUNTRY_JAPAN		= 1,		// 日本

	// USAリージョン
	TWL_COUNTRY_Anguilla	= 8,        // アンギラ
	TWL_COUNTRY_ANTIGUA_AND_BARBUDA,    // アンティグア・バーブーダ
	TWL_COUNTRY_ARGENTINA   = 10,		// アルゼンチン
	TWL_COUNTRY_ARUBA,                  // アルバ
	TWL_COUNTRY_BAHAMAS,                // バハマ
	TWL_COUNTRY_BARBADOS,               // バルバドス
	TWL_COUNTRY_BELIZE,                 // ベリーズ
	TWL_COUNTRY_BOLIVIA,                // ボリビア
	TWL_COUNTRY_BRAZIL,                 // ブラジル
	TWL_COUNTRY_BRITISH_VIRGIN_ISLANDS, // 英領ヴァージン諸島
	TWL_COUNTRY_CANADA,                 // カナダ
	TWL_COUNTRY_CAYMAN_ISLANDS,         // ケイマン諸島
	TWL_COUNTRY_CHILE       = 20,       // チリ
	TWL_COUNTRY_COLOMBIA,               // コロンビア
	TWL_COUNTRY_COSTA_RICA,             // コスタリカ
	TWL_COUNTRY_DOMINICA,               // ドミニカ国
	TWL_COUNTRY_DOMINICAN_REPUBLIC,     // ドミニカ共和国
	TWL_COUNTRY_ECUADOR,                // エクアドル
	TWL_COUNTRY_EL_SALVADOR,            // エルサルバドル
	TWL_COUNTRY_FRENCH_GUIANA,          // フランス領ギアナ
	TWL_COUNTRY_GRENADA,                // グレナダ
	TWL_COUNTRY_GUADELOUPE,             // グアドループ
	TWL_COUNTRY_GUATEMALA   = 30,       // グアテマラ
	TWL_COUNTRY_GUYANA,                 // ガイアナ
	TWL_COUNTRY_HAITI,                  // ハイチ
	TWL_COUNTRY_HONDURAS,               // ホンジュラス
	TWL_COUNTRY_JAMAICA,                // ジャマイカ
	TWL_COUNTRY_MARTINIQUE,             // マルティニーク
	TWL_COUNTRY_MEXICO,                 // メキシコ
	TWL_COUNTRY_MONTSERRAT,             // モントセラト
	TWL_COUNTRY_NETHERLANDS_ANTILLES,   // オランダ領アンティル
	TWL_COUNTRY_NICARAGUA,              // ニカラグア
	TWL_COUNTRY_PANAMA      = 40,       // パナマ
	TWL_COUNTRY_PARAGUAY,               // パラグアイ
	TWL_COUNTRY_PERU,                   // ペルー
	TWL_COUNTRY_ST_KITTS_AND_NEVIS,     // セントキッツ・ネイビス
	TWL_COUNTRY_ST_LUCIA,               // セントルシア
	TWL_COUNTRY_ST_VINCENT_AND_THE_GRENADINES,  // セントビンセント・グレナディーン
	TWL_COUNTRY_SURINAME,               // スリナム
	TWL_COUNTRY_TRINIDAD_AND_TOBAGO,    // トリニダード・トバゴ
	TWL_COUNTRY_TURKS_AND_CAICOS_ISLANDS,   // タークス・カイコス諸島
	TWL_COUNTRY_UNITED_STATES,          // アメリカ
	TWL_COUNTRY_URUGUAY     = 50,       // ウルグアイ
	TWL_COUNTRY_US_VIRGIN_ISLANDS,      // 米領バージン諸島
	TWL_COUNTRY_VENEZUELA,              // ベネズエラ

    // EUR, NAL リージョン
	TWL_COUNTRY_ALBANIA     = 64,       // アルバニア
	TWL_COUNTRY_AUSTRALIA,              // オーストラリア
	TWL_COUNTRY_AUSTRIA,                // オーストリア
	TWL_COUNTRY_BELGIUM,                // ベルギー
	TWL_COUNTRY_BOSNIA_AND_HERZEGOVINA, // ボスニア・ヘルツェゴビナ
	TWL_COUNTRY_BOTSWANA,               // ボツワナ
	TWL_COUNTRY_BULGARIA    = 70,       // ブルガリア
	TWL_COUNTRY_CROATIA,                // クロアチア
	TWL_COUNTRY_CYPRUS,                 // キプロス
	TWL_COUNTRY_CZECH_REPUBLIC,         // チェコ
	TWL_COUNTRY_DENMARK,                // デンマーク
	TWL_COUNTRY_ESTONIA,                // エストニア
	TWL_COUNTRY_FINLAND,                // フィンランド
	TWL_COUNTRY_FRANCE,                 // フランス
	TWL_COUNTRY_GERMANY,                // ドイツ
	TWL_COUNTRY_GREECE,                 // ギリシャ
	TWL_COUNTRY_HUNGARY     = 80,       // ハンガリー
	TWL_COUNTRY_ICELAND,                // アイスランド
	TWL_COUNTRY_IRELAND,                // アイルランド
	TWL_COUNTRY_ITALY,                  // イタリア
	TWL_COUNTRY_LATVIA,                 // ラトビア
	TWL_COUNTRY_LESOTHO,                // レソト
	TWL_COUNTRY_LIECHTENSTEIN,          // リヒテンシュタイン
	TWL_COUNTRY_LITHUANIA,              // リトアニア
	TWL_COUNTRY_LUXEMBOURG,             // ルクセンブルク
	TWL_COUNTRY_MACEDONIA,              // マケドニア
	TWL_COUNTRY_MALTA       = 90,       // マルタ
	TWL_COUNTRY_MONTENEGRO,             // モンテネグロ
	TWL_COUNTRY_MOZAMBIQUE,             // モザンビーク
	TWL_COUNTRY_NAMIBIA,                // ナミビア
	TWL_COUNTRY_NETHERLANDS,            // オランダ
	TWL_COUNTRY_NEW_ZEALAND,            // ニュージーランド
	TWL_COUNTRY_NORWAY,                 // ノルウェー
	TWL_COUNTRY_POLAND,                 // ポーランド
	TWL_COUNTRY_PORTUGAL,               // ポルトガル
	TWL_COUNTRY_ROMANIA,                // ルーマニア
	TWL_COUNTRY_RUSSIA      = 100,      // ロシア
	TWL_COUNTRY_SERBIA,                 // セルビア
	TWL_COUNTRY_SLOVAKIA,               // スロバキア
	TWL_COUNTRY_SLOVENIA,               // スロベニア
	TWL_COUNTRY_SOUTH_AFRICA,           // 南アフリカ
	TWL_COUNTRY_SPAIN,                  // スペイン
	TWL_COUNTRY_SWAZILAND,              // スワジランド
	TWL_COUNTRY_SWEDEN,                 // スウェーデン
	TWL_COUNTRY_SWITZERLAND,            // スイス
	TWL_COUNTRY_TURKEY,                 // トルコ
	TWL_COUNTRY_UNITED_KINGDOM = 110,   // イギリス
	TWL_COUNTRY_ZAMBIA,                 // ザンビア
	TWL_COUNTRY_ZIMBABWE,               // ジンバブエ

    // TWNリージョン
    TWL_COUNTRY_TAIWAN      = 128,      // 台湾
    
    // KORリージョン
    TWL_COUNTRY_SOUTH_KOREA = 136,      // 韓国
    
    // HKGリージョン（Wiiの国リストに存在）
    TWL_COUNTRY_HONG_KONG   = 144,      // ホンコン
    TWL_COUNTRY_MACAU,                  // マカオ
    
    // ASIリージョン（Wiiの国リストに存在）
    TWL_COUNTRY_INDONESIA   = 152,      // インドネシア
    
    // USAリージョン
    TWL_COUNTRY_SINGAPORE   = 153,      // シンガポール
    
    // ASIリージョン（再び）
    TWL_COUNTRY_THAILAND    = 154,      // タイ
    TWL_COUNTRY_PHILIPPINES,            // フィリピン
    TWL_COUNTRY_MALAYSIA,               // マレーシア
    
    // 未定義リージョン（IQueリージョン？）
    TWL_COUNTRY_CHINA       = 160,      // 中国
    
    // USAリージョン
    TWL_COUNTRY_UAE         = 168,      // アラブ首長国連邦
    
    // 未定義リージョン
    TWL_COUNTRY_INDIA       = 169,      // インド
    TWL_COUNTRY_EGYPT       = 170,      // エジプト
    TWL_COUNTRY_OMAN,                   // オマーン
    TWL_COUNTRY_QATAR,                  // カタール
    TWL_COUNTRY_KUWAIT,                 // クウェート
    TWL_COUNTRY_SAUDI_ARABIA,           // サウジアラビア
    TWL_COUNTRY_SYRIA,                  // シリア
    TWL_COUNTRY_BAHRAIN,                // バーレーン
    TWL_COUNTRY_JORDAN,                 // ヨルダン

    TWL_COUNTRY_OTHERS      = 254,
    TWL_COUNTRY_UNKNOWN     = 255
    
}TWLCountryCode;



#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// COUNTRY_CODE_H_
