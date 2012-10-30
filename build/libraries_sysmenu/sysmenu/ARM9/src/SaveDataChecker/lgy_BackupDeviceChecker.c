/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     lgy_SaveDataChecker.cpp

  Copyright (C)2009-2012 Nintendo Co., Ltd.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Rev$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "lgy_SaveDataChecker.h"
#include "lgy_BackupDeviceChecker.h"

static BOOL ReadBackupStatus( u8* status);
static BOOL ReadBackupID( u32* devid);
static BOOL checkDevice( void);


typedef struct lgyGameCodeList
{
    char** code_list;
    u16   num;
    u16   reserved;
} lgyGameCodeList;

// 以下、3文字目を揃えたリスト
// なお、バックアップデバイスの有無は最初の 3文字だけで識別可能
// （ROMバージョンの違いや仕向地によってバックアップデバイスの
// 有り無しは変わらない）であることは確認済み。
static char *list_2[] = {
    "AG2", "AB2", "AC2", "AD2", "AE2", "AF2", "AH2", "AI2",
    "AK2", "AL2", "AM2", "AN2", "AO2", "AP2", "AQ2", "AR2",
    "AS2", "AU2", "AV2", "AW2", "AX2", "AZ2", "B22", "B32",
    "B42", "B52", "B62", "B72", "B82", "BA2", "BB2", "BD2",
    "BE2", "BF2", "BG2", "BH2", "BJ2", "BL2", "BM2", "BN2",
    "BO2", "BP2", "BQ2", "BR2", "BS2", "BT2", "BU2", "BV2",
    "BW2", "BX2", "BY2", "BZ2", "C22", "C32", "C42", "C52",
    "C62", "C72", "CA2", "CB2", "CC2", "CD2", "CE2", "CF2",
    "CG2", "CH2", "CI2", "CJ2", "CK2", "CL2", "CM2", "CN2",
    "CO2", "CP2", "CQ2", "CR2", "CS2", "CT2", "CU2", "CW2",
    "CX2", "CY2", "CZ2", "TK2", "TP2", "YA2", "YB2", "YC2",
    "YD2", "YE2", "YF2", "YG2", "YH2", "YI2", "YJ2", "YK2",
    "YL2", "YM2", "YN2", "YO2", "YP2", "YQ2", "YR2", "YS2",
    "YT2", "YU2", "YV2", "YW2", "YX2", "YY2", "YZ2"
};

static char *list_3[] = {
    "AL3", "AB3",
    "AC3", "AD3", "AE3", "AF3", "AG3", "AH3", "AI3", "AK3",
    "AO3", "AQ3", "AR3", "AS3", "AU3", "AV3", "AW3",
    "AX3", "AY3", "AZ3", "B23", "B33", "B43", "B53", "B63",
    "B83", "BA3", "BB3", "BC3", "BD3", "BE3", "BF3", "BG3",
    "BH3", "BJ3", "BK3", "BL3", "BM3", "BN3", "BP3", "BQ3",
    "BR3", "BS3", "BT3", "BV3", "BW3", "BX3", "BY3", "BZ3",
    "C23", "C33", "C53", "C63", "C73", "CB3", "CC3", "CD3",
    "CE3", "CF3", "CG3", "CH3", "CI3", "CJ3", "CL3", "CM3",
    "CN3", "CO3", "CP3", "CQ3", "CR3", "CS3", "CT3", "CU3",
    "CV3", "CW3", "CX3", "CY3", "CZ3", "TH3", "TK3", "TM3",
    "TN3", "YA3", "YB3", "YC3", "YE3", "YF3", "YG3", "YH3",
    "YI3", "YJ3", "YK3", "YL3", "YM3", "YN3", "YO3", "YQ3",
    "YR3", "YS3", "YT3", "YU3", "YV3", "YW3", "YY3", "YZ3"
};

static char *list_4[] = {
    "AB4", "AC4", "AE4", "AH4", "AI4", "AK4", "AL4", "AN4",
    "AO4", "AQ4", "AR4", "AS4", "AU4", "AV4", "AW4", "AX4",
    "AY4", "AZ4", "B24", "B34", "B44", "B54", "B64", "B74",
    "B84", "BA4", "BB4", "BC4", "BF4", "BG4", "BH4", "BJ4",
    "BK4", "BL4", "BM4", "BN4", "BO4", "BP4", "BQ4", "BR4",
    "BS4", "BU4", "BV4", "BW4", "BX4", "BY4", "BZ4", "C24",
    "C34", "C44", "C54", "C64", "C74", "CB4", "CC4", "CD4",
    "CE4", "CF4", "CG4", "CH4", "CI4", "CJ4", "CK4", "CL4",
    "CM4", "CO4", "CP4", "CR4", "CS4", "CT4", "CU4", "CV4",
    "CW4", "CX4", "CY4", "CZ4", "TB4", "TC4", "TD4", "TF4",
    "TG4", "TM4", "TP4", "YB4", "YC4", "YE4", "YF4", "YG4",
    "YH4", "YJ4", "YK4", "YL4", "YM4", "YN4", "YO4", "YP4",
    "YQ4", "YR4", "YS4", "YT4", "YU4", "YV4", "YW4", "YY4",
    "YZ4"
};

static char *list_5[] = {
    "AB5", "AC5", "AD5", "AE5", "AG5", "AH5", "AI5",
    "AK5", "AL5", "AM5", "AN5", "AO5", "AQ5", "AR5", "AS5",
    "AU5", "AV5", "AW5", "AX5", "AY5", "B35", "B55", "B65",
    "B75", "B85", "BA5", "BB5", "BC5", "BD5", "BE5", "BF5",
    "BG5", "BJ5", "BL5", "BM5", "BN5", "BO5", "BP5", "BQ5",
    "BR5", "BS5", "BT5", "BU5", "BX5", "BY5", "BZ5", "C25",
    "C45", "C55", "C65", "C75", "CA5", "CB5", "CD5", "CF5",
    "CG5", "CH5", "CI5", "CJ5", "CK5", "CL5", "CM5", "CN5",
    "CO5", "CP5", "CR5", "CS5", "CT5", "CU5", "CV5", "CW5",
    "CX5", "CY5", "CZ5", "TC5", "TJ5", "TK5", "YB5", "YD5",
    "YF5", "YG5", "YH5", "YI5", "YJ5", "YL5", "YN5", "YO5",
    "YP5", "YQ5", "YR5", "YS5", "YT5", "YU5", "YV5", "YY5",
    "YZ5"
};

static char *list_6[] = {
    "AB6", "AC6", "AE6", "AF6", "AG6", "AH6", "AK6",
    "AL6", "AM6", "AN6", "AO6", "AQ6", "AR6", "AS6", "AU6",
    "AV6", "AW6", "AX6", "AY6", "AZ6", "B26", "B36", "B46",
    "B56", "B66", "B76", "B86", "BA6", "BC6", "BE6", "BF6",
    "BG6", "BH6", "BK6", "BL6", "BN6", "BO6", "BP6", "BQ6",
    "BR6", "BS6", "BU6", "BV6", "BW6", "BX6", "BY6", "BZ6",
    "C26", "C36", "C46", "C56", "C66", "C76", "CA6", "CB6",
    "CD6", "CE6", "CF6", "CG6", "CH6", "CI6", "CJ6", "CK6",
    "CL6", "CM6", "CN6", "CO6", "CP6", "CR6", "CS6", "CT6",
    "CV6", "CW6", "CX6", "CY6", "CZ6", "YB6", "YC6", "YE6",
    "YF6", "YG6", "YH6", "YI6", "YL6", "YM6", "YN6",
    "YO6", "YP6", "YR6", "YS6", "YT6", "YU6", "YV6", "YW6",
    "YZ6"
}; //"YK6", はバックアップなしリージョンが存在するので対象から外す

static char *list_7[] = {
    "YF7", "AB7", "AC7", "AD7", "AE7", "AF7", "AG7", "AH7",
    "AK7", "AL7", "AM7", "AN7", "AO7", "AQ7", "AR7", "AS7",
    "AU7", "AV7", "AW7", "AX7", "AY7", "AZ7", "B27", "B47",
    "B57", "B67", "B77", "BA7", "BB7", "BC7", "BD7", "BE7",
    "BF7", "BG7", "BH7", "BJ7", "BL7", "BM7", "BN7", "BO7",
    "BP7", "BQ7", "BR7", "BS7", "BU7", "BV7", "BW7", "BX7",
    "BY7", "BZ7", "C27", "C37", "C47", "C57", "C67", "C77",
    "CA7", "CB7", "CD7", "CE7", "CF7", "CG7", "CH7", "CI7",
    "CJ7", "CL7", "CN7", "CP7", "CQ7", "CR7", "CS7", "CT7",
    "CV7", "CY7", "TJ7", "TN7", "YA7", "YB7", "YC7", "YD7",
    "YE7", "YG7", "YH7", "YI7", "YJ7", "YK7", "YL7",
    "YM7", "YN7", "YO7", "YR7", "YS7", "YT7", "YV7", "YW7",
    "YX7", "YY7", "YZ7"
};

static char *list_8[] = {
    "YS8", "AB8", "AD8", "AE8", "AF8", "AG8",
    "AH8", "AI8", "AL8", "AM8", "AN8", "AO8", "AP8", "AQ8",
    "AR8", "AS8", "AU8", "AV8", "AW8", "AX8", "AY8", "AZ8",
    "B28", "B38", "B48", "B58", "B68", "B78", "BA8", "BB8",
    "BC8", "BD8", "BE8", "BF8", "BG8", "BH8", "BJ8", "BM8",
    "BN8", "BO8", "BP8", "BQ8", "BR8", "BS8", "BT8", "BU8",
    "BV8", "BW8", "BY8", "BZ8", "C38", "C48", "C58", "C68",
    "CB8", "CC8", "CD8", "CE8", "CF8", "CG8", "CH8", "CI8",
    "CJ8", "CK8", "CL8", "CM8", "CN8", "CO8", "CP8", "CQ8",
    "CR8", "CS8", "CT8", "CU8", "CV8", "CW8", "CX8", "CY8",
    "TJ8", "YA8", "YB8", "YC8", "YD8", "YF8", "YG8",
    "YI8", "YJ8", "YK8", "YL8", "YM8", "YN8", "YO8", "YP8",
    "YQ8", "YR8", "YT8", "YV8", "YW8", "YX8", "YY8",
    "YZ8"
}; //"IA8", は IRC基板でバックアップデバイスの触り方が異なるため対象から外す

static char *list_9[] = {
    "AB9", "AC9", "AD9", "AE9", "AF9", "AG9", "AH9",
    "AI9", "AK9", "AL9", "AM9", "AN9", "AO9", "AP9", "AQ9",
    "AR9", "AS9", "AU9", "AW9", "AX9", "AY9", "AZ9", "B39",
    "B49", "B59", "B69", "B79", "B89", "BA9", "BB9", "BC9",
    "BD9", "BE9", "BF9", "BG9", "BH9", "BJ9", "BK9", "BL9",
    "BM9", "BO9", "BP9", "BQ9", "BR9", "BS9", "BT9", "BU9",
    "BV9", "BW9", "BY9", "BZ9", "C29", "C39", "C49", "C59",
    "C69", "CB9", "CC9", "CD9", "CE9", "CF9", "CG9", "CH9",
    "CI9", "CK9", "CM9", "CP9", "CQ9", "CR9", "CS9", "CV9",
    "CW9", "CX9", "CY9", "CZ9", "TA9", "TJ9", "TK9", "YA9",
    "YB9", "YE9", "YF9", "YH9", "YJ9", "YK9", "YL9", "YM9",
    "YN9", "YO9", "YP9", "YR9", "YS9", "YT9", "YU9", "YV9",
    "YW9", "YX9", "YY9", "YZ9"
};

static char *list_A[] = {
    "ARA", "AEA", "A2A", "A3A", "A5A", "A6A",
    "A8A", "ABA", "ACA", "ADA", "AFA", "AIA", "AJA",
    "AKA", "ALA", "AMA", "ANA", "APA", "AQA", "ATA",
    "AUA", "AVA", "AWA", "AXA", "AYA", "AZA", "B2A", "B3A",
    "B4A", "B5A", "B6A", "B7A", "B8A", "BBA", "BCA", "BDA",
    "BFA", "BJA", "BKA", "BLA", "BMA", "BNA", "BOA", "BPA",
    "BQA", "BRA", "BSA", "BTA", "BVA", "BWA", "BYA", "BZA",
    "C2A", "C3A", "C4A", "C5A", "C6A", "CAA", "CBA", "CDA",
    "CEA", "CFA", "CHA", "CIA", "CJA", "CKA", "CMA", "CNA",
    "COA", "CPA", "CQA", "CRA", "CSA", "CTA", "CUA", "CVA",
    "CWA", "CXA", "CYA", "CZA", "TJA", "TLA", "TMA", "TQA",
    "YAA", "YBA", "YDA", "YEA", "YFA", "YGA", "YHA", "YIA",
    "YJA", "YLA", "YMA", "YNA", "YOA", "YQA", "YRA", "YSA",
    "YTA", "YUA", "YXA", "YYA", "YZA"
};

static char *list_B[] = {
    "A2B", "A3B", "A5B",
    "A6B", "A8B", "ABB", "ACB", "ADB", "AFB", "AHB", "AIB",
    "AJB", "AKB", "ALB", "AMB", "ANB", "APB", "AQB", "ARB",
    "ASB", "ATB", "AUB", "AVB", "AWB", "AXB", "AYB", "AZB",
    "B2B", "B3B", "B4B", "B5B", "B6B", "B8B", "BAB", "BBB",
    "BDB", "BEB", "BFB", "BGB", "BHB", "BIB", "BJB", "BKB",
    "BLB", "BMB", "BNB", "BOB", "BPB", "BQB", "BSB", "BTB",
    "BUB", "BVB", "BWB", "BYB", "BZB", "C2B", "C3B", "C4B",
    "C5B", "C6B", "C7B", "CAB", "CBB", "CDB", "CEB", "CGB",
    "CHB", "CIB", "CJB", "CKB", "CLB", "CMB", "CNB", "COB",
    "CPB", "CQB", "CRB", "CSB", "CTB", "CUB", "CVB", "CWB",
    "CXB", "CYB", "TBB", "TDB", "TGB", "TQB", "YAB", "YBB",
    "YCB", "YEB", "YFB", "YGB", "YHB", "YIB", "YJB", "YKB",
    "YLB", "YMB", "YNB", "YPB", "YQB", "YRB", "YSB", "YTB",
    "YUB", "YVB", "YWB", "YXB", "YYB", "YZB"
};

static char *list_C[] = {
    "A4C", "A5C",
    "A6C", "A8C", "ABC", "ACC", "ADC", "AEC", "AFC", "AGC",
    "AIC", "AJC", "AMC", "ANC", "AOC", "APC", "AQC", "ARC",
    "ASC", "ATC", "AUC", "AVC", "AWC", "AXC", "AYC", "AZC",
    "B3C", "B4C", "B5C", "B6C", "B7C", "BCC", "BDC", "BEC",
    "BFC", "BGC", "BHC", "BJC", "BKC", "BLC", "BMC", "BNC",
    "BOC", "BRC", "BSC", "BTC", "BUC", "BVC", "BWC", "BXC",
    "BYC", "BZC", "C2C", "C3C", "C4C", "C5C", "C6C", "CAC",
    "CCC", "CDC", "CEC", "CFC", "CGC", "CHC", "CIC", "CJC",
    "CKC", "CLC", "CMC", "CNC", "COC", "CPC", "CQC", "CRC",
    "CSC", "CVC", "CWC", "CXC", "CYC", "CZC", "TBC", "TCC",
    "TEC", "TGC", "TQC", "YAC", "YBC", "YCC", "YDC",
    "YEC", "YFC", "YGC", "YHC", "YIC", "YJC", "YKC", "YMC",
    "YOC", "YQC", "YRC", "YSC", "YTC", "YUC", "YVC", "YWC",
    "YXC", "YYC", "YZC"
}; // "UZC", は BT基板でバックアップデバイスの触り方が異なる可能性があるため対象から外す

static char *list_D[] = {
    "A2D", "A3D", "A4D", "A5D", "A6D",
    "A8D", "ABD", "ADD", "AED", "AFD", "AGD", "AHD", "AID",
    "AJD", "AKD", "ALD", "AMD", "AOD", "APD", "AQD",
    "ARD", "ASD", "ATD", "AUD", "AVD", "AWD", "AXD", "AYD",
    "B2D", "B3D", "B4D", "B5D", "BBD", "BCD", "BDD", "BFD",
    "BHD", "BID", "BJD", "BKD", "BLD", "BMD", "BND", "BPD",
    "BQD", "BSD", "BTD", "BUD", "BVD", "BWD", "BXD", "BZD",
    "C2D", "C3D", "C4D", "C5D", "C6D", "C7D", "CAD", "CBD",
    "CCD", "CDD", "CED", "CFD", "CGD", "CHD", "CID", "CJD",
    "CKD", "CLD", "CMD", "CND", "CPD", "CQD", "CRD", "CSD",
    "CTD", "CVD", "CWD", "CXD", "CZD", "TAD", "TED", "TGD",
    "TKD", "TQD", "YBD", "YCD", "YDD", "YED", "YGD", "YHD",
    "YID", "YJD", "YKD", "YLD", "YMD", "YND", "YOD", "YPD",
    "YQD", "YRD", "YSD", "YTD", "YUD", "YVD", "YWD", "YXD",
    "YZD"
}; // "AND", はバックアップなしリージョンが存在するので対象から外す

static char *list_E[] = {
    "YCE", "A3E", "A5E", "A6E", "A8E", "ABE", "ACE", "ADE",
    "AEE", "AFE", "AGE", "AHE", "AJE", "AKE", "ALE", "AME",
    "ANE", "AOE", "APE", "AQE", "ARE", "ASE", "ATE", "AUE",
    "AVE", "AWE", "AXE", "AYE", "AZE", "B2E", "B4E", "B5E",
    "B6E", "B7E", "BBE", "BCE", "BDE", "BEE", "BFE", "BGE",
    "BHE", "BJE", "BKE", "BLE", "BME", "BNE", "BOE", "BQE",
    "BSE", "BTE", "BUE", "BVE", "BWE", "BXE", "BYE", "BZE",
    "C2E", "C3E", "C4E", "C5E", "C6E", "CAE", "CBE", "CCE",
    "CDE", "CEE", "CFE", "CGE", "CHE", "CIE", "CJE", "CKE",
    "CLE", "CME", "CNE", "COE", "CPE", "CQE", "CRE", "CSE",
    "CTE", "CUE", "CVE", "CWE", "CXE", "CYE", "THE", "TQE",
    "YAE", "YBE", "YEE", "YFE", "YGE", "YHE", "YIE",
    "YJE", "YKE", "YLE", "YME", "YNE", "YOE", "YPE", "YQE",
    "YRE", "YTE", "YWE", "YXE", "YYE", "YZE"
};

static char *list_F[] = {
    "A2F", "A3F",
    "A5F", "A6F", "ABF", "ACF", "ADF", "AEF", "AFF", "AGF",
    "AHF", "AIF", "AKF", "AMF", "ANF", "AOF", "APF", "AQF",
    "ARF", "ASF", "ATF", "AUF", "AVF", "AXF", "B2F", "B3F",
    "B4F", "B5F", "B6F", "B7F", "BBF", "BCF", "BDF", "BEF",
    "BFF", "BHF", "BJF", "BKF", "BLF", "BMF", "BNF", "BPF",
    "BQF", "BRF", "BSF", "BTF", "BUF", "BWF", "BXF", "BZF",
    "C2F", "C3F", "C4F", "C5F", "C6F", "C7F", "CAF", "CBF",
    "CCF", "CDF", "CEF", "CFF", "CGF", "CHF", "CIF", "CJF",
    "CKF", "CLF", "CMF", "CPF", "CRF", "CSF", "CTF", "CUF",
    "CVF", "CWF", "CXF", "CYF", "TBF", "TKF", "TQF", "YAF",
    "YBF", "YDF", "YEF", "YFF", "YGF", "YHF", "YIF", "YJF",
    "YKF", "YLF", "YMF", "YOF", "YPF", "YQF", "YRF", "YSF",
    "YTF", "YUF", "YVF", "YWF", "YXF", "YYF", "YZF"
};

static char *list_G[] = {
    "A2G",
    "A3G", "A4G", "A5G", "A6G", "A8G", "ACG", "ADG", "AEG",
    "AFG", "AGG", "AHG", "AIG", "AJG", "AKG", "ALG", "AMG",
    "ANG", "AOG", "APG", "AQG", "ARG", "ASG", "ATG", "AUG",
    "AVG", "AWG", "AYG", "AZG", "B2G", "B3G", "B4G", "B5G",
    "B6G", "B7G", "BAG", "BCG", "BDG", "BEG", "BFG", "BGG",
    "BHG", "BIG", "BKG", "BLG", "BMG", "BNG", "BQG", "BRG",
    "BTG", "BUG", "BVG", "BWG", "BXG", "BYG", "BZG", "C2G",
    "C3G", "C4G", "C5G", "C6G", "C7G", "CAG", "CBG", "CCG",
    "CDG", "CEG", "CFG", "CGG", "CHG", "CIG", "CJG", "CLG",
    "CMG", "CNG", "CPG", "CQG", "CRG", "CSG", "CTG", "CUG",
    "CWG", "CXG", "CYG", "TCG", "TGG", "THG", "TKG",
    "TLG", "TPG", "YAG", "YBG", "YCG", "YEG", "YFG", "YHG",
    "YIG", "YJG", "YKG", "YLG", "YNG", "YOG", "YPG", "YQG",
    "YRG", "YTG", "YUG", "YVG", "YWG", "YXG", "YYG", "YZG"
}; //"IPG", は IRC01orIRC02基板でバックアップデバイスの触り方が異なるため対象から外す

static char *list_H[] = {
    "A2H", "A3H", "A4H", "A5H", "A6H", "A8H", "ABH", "ACH",
    "ADH", "AEH", "AFH", "AHH", "AIH", "AJH", "AKH", "ALH",
    "AMH", "ANH", "AOH", "APH", "AQH", "ASH", "ATH", "AUH",
    "AVH", "AWH", "AXH", "AYH", "AZH", "B3H", "B4H", "B5H",
    "B6H", "B7H", "B8H", "BAH", "BCH", "BDH", "BEH", "BGH",
    "BHH", "BIH", "BJH", "BKH", "BLH", "BMH", "BNH", "BOH",
    "BPH", "BQH", "BRH", "BSH", "BTH", "BVH", "BYH", "BZH",
    "C2H", "C3H", "C4H", "C5H", "C6H", "CAH", "CDH", "CEH",
    "CFH", "CGH", "CHH", "CIH", "CJH", "CKH", "CLH", "CNH",
    "COH", "CPH", "CQH", "CRH", "CSH", "CTH", "CUH", "CVH",
    "CWH", "CXH", "CYH", "TCH", "TJH", "TPH", "TQH", "YAH",
    "YBH", "YCH", "YDH", "YEH", "YFH", "YGH", "YHH", "YIH",
    "YJH", "YKH", "YLH", "YMH", "YNH", "YOH", "YPH", "YQH",
    "YRH", "YSH", "YTH", "YUH", "YVH", "YWH", "YXH", "YYH",
    "YZH"
};

static char *list_I[] = {
    "A2I", "A3I", "A4I", "A5I", "A6I", "A8I", "ABI",
    "ACI", "ADI", "AFI", "AHI", "AII", "AMI", "ANI", "AOI",
    "API", "AQI", "ARI", "ASI", "AUI", "AVI", "AWI", "AXI",
    "AYI", "AZI", "B2I", "B3I", "B4I", "B6I", "B7I", "B8I",
    "BCI", "BDI", "BEI", "BGI", "BHI", "BII", "BKI", "BLI",
    "BNI", "BOI", "BPI", "BQI", "BRI", "BSI", "BTI", "BUI",
    "BVI", "BWI", "BZI", "C4I", "C5I", "C6I", "CAI", "CBI",
    "CDI", "CFI", "CHI", "CJI", "CKI", "CLI", "CMI", "CNI",
    "COI", "CPI", "CQI", "CRI", "CSI", "CTI", "CUI", "CVI",
    "CWI", "CXI", "CYI", "YAI", "YBI", "YCI", "YDI",
    "YEI", "YFI", "YGI", "YHI", "YII", "YKI", "YLI", "YNI",
    "YPI", "YQI", "YRI", "YTI", "YUI", "YVI", "YWI", "YXI",
    "YYI", "YZI"
}; //"UEI", は C22基板でバックアップデバイスの触り方が異なるため対象から外す

static char *list_J[] = {
    "ABJ", "A4J", "A2J", "A6J", "ACJ", "ADJ",
    "AEJ", "AHJ", "AIJ", "AJJ", "AKJ", "ALJ", "AMJ", "ANJ",
    "AOJ", "APJ", "AQJ", "ARJ", "ASJ", "ATJ", "AUJ", "AVJ",
    "AXJ", "AYJ", "B2J", "B3J", "B4J", "B6J", "B7J",
    "B8J", "BAJ", "BBJ", "BCJ", "BDJ", "BEJ", "BFJ", "BGJ",
    "BHJ", "BJJ", "BKJ", "BLJ", "BMJ", "BNJ", "BOJ", "BPJ",
    "BQJ", "BRJ", "BSJ", "BTJ", "BUJ", "BVJ", "BXJ", "BYJ",
    "BZJ", "C3J", "C4J", "C5J", "C6J", "C7J", "CAJ", "CBJ",
    "CCJ", "CEJ", "CFJ", "CGJ", "CHJ", "CIJ", "CJJ", "CKJ",
    "CLJ", "CMJ", "CNJ", "COJ", "CPJ", "CQJ", "CRJ", "CSJ",
    "CTJ", "CUJ", "CVJ", "CWJ", "CYJ", "CZJ", "TQJ", "YAJ",
    "YBJ", "YCJ", "YDJ", "YEJ", "YFJ", "YGJ", "YHJ", "YIJ",
    "YJJ", "YKJ", "YLJ", "YMJ", "YNJ", "YOJ", "YPJ", "YQJ",
    "YRJ", "YSJ", "YTJ", "YUJ", "YVJ", "YWJ", "YXJ", "YYJ",
    "YZJ"
}; //"B5J", はバックアップなしリージョンが存在するので対象から外す

static char *list_K[] = {
    "A2K", "A3K", "A4K", "A5K", "A6K", "A8K", "ABK",
    "ACK", "ADK", "AEK", "AFK", "AHK", "AIK", "AJK", "AKK",
    "ALK", "AMK", "ANK", "APK", "AQK", "ARK", "ASK", "ATK",
    "AUK", "AVK", "AWK", "AXK", "AYK", "AZK", "B2K", "B3K",
    "B4K", "B6K", "B7K", "B8K", "BBK", "BCK", "BDK",
    "BEK", "BFK", "BGK", "BHK", "BIK", "BJK", "BKK", "BLK",
    "BMK", "BNK", "BOK", "BQK", "BRK", "BSK", "BUK", "BVK",
    "BXK", "BYK", "BZK", "C2K", "C3K", "C4K", "C5K", "C6K",
    "C7K", "CAK", "CBK", "CCK", "CDK", "CEK", "CGK", "CHK",
    "CIK", "CJK", "CKK", "CLK", "CNK", "COK", "CQK", "CRK",
    "CSK", "CTK", "CUK", "CWK", "CXK", "CYK", "CZK", 
    "TAK", "TGK", "TMK", "TQK", "YAK", "YDK", "YEK", "YFK",
    "YGK", "YHK", "YIK", "YJK", "YKK", "YLK", "YMK", "YNK",
    "YOK", "YPK", "YQK", "YSK", "YTK", "YUK", "YVK", "YWK",
    "YXK", "YYK", "YZK"
}; //"B5K", はバックアップなしリージョンが存在するので対象から外す
   //"IPK", は IRC01orIRC02基板でバックアップデバイスの触り方が異なるため対象から外す

static char *list_L[] = {
    "ADL", "A2L", "A3L", "A4L", "A5L", "A6L",
    "ABL", "ACL", "AEL", "AFL", "AGL", "AHL", "AIL",
    "AJL", "AKL", "ALL", "AML", "ANL", "AOL", "APL", "AQL",
    "ARL", "ASL", "ATL", "AUL", "AVL", "AWL", "AXL", "AYL",
    "AZL", "B2L", "B3L", "B5L", "B6L", "B7L", "BAL", "BBL",
    "BCL", "BDL", "BEL", "BFL", "BGL", "BHL", "BIL", "BJL",
    "BKL", "BLL", "BML", "BNL", "BOL", "BPL", "BQL", "BRL",
    "BSL", "BTL", "BUL", "BVL", "BWL", "BXL", "BYL", "BZL",
    "C2L", "C5L", "C6L", "C7L", "CAL", "CBL", "CCL", "CDL",
    "CFL", "CHL", "CIL", "CJL", "CKL", "CLL", "CML", "CNL",
    "COL", "CPL", "CQL", "CRL", "CSL", "CTL", "CUL", "CVL",
    "CXL", "CYL", "CZL", "TCL", "TFL", "THL", "TML", "TPL",
    "TQL", "YAL", "YBL", "YCL", "YDL", "YEL", "YFL", "YGL",
    "YHL", "YIL", "YJL", "YLL", "YML", "YNL", "YOL", "YPL",
    "YQL", "YRL", "YSL", "YTL", "YUL", "YVL", "YWL", "YXL",
    "YYL", "YZL"
};

static char *list_M[] = {
    "A2M", "A3M", "A5M", "ABM", "ACM", "ADM",
    "AEM", "AFM", "AHM", "AIM", "AJM", "AKM", "AMM", "ANM",
    "AOM", "APM", "AQM", "ARM", "ASM", "ATM", "AUM", "AVM",
    "AWM", "AYM", "AZM", "B2M", "B3M", "B4M", "B5M", "B7M",
    "B8M", "BAM", "BBM", "BCM", "BDM", "BFM", "BGM", "BHM",
    "BIM", "BJM", "BKM", "BLM", "BMM", "BNM", "BPM", "BQM",
    "BRM", "BSM", "BTM", "BVM", "BWM", "BXM", "BYM", "BZM",
    "C2M", "C4M", "C5M", "C6M", "C7M", "CAM", "CBM", "CCM",
    "CDM", "CFM", "CGM", "CHM", "CIM", "CKM", "CLM", "CMM",
    "CNM", "COM", "CPM", "CQM", "CRM", "CSM", "CTM", "CUM",
    "CVM", "CWM", "CXM", "CYM", "CZM", "TAM", "TBM", "TCM",
    "THM", "TKM", "TQM", "YAM", "YBM", "YCM", "YDM", "YEM",
    "YFM", "YGM", "YHM", "YIM", "YJM", "YKM", "YLM", "YMM",
    "YNM", "YOM", "YQM", "YRM", "YSM", "YTM", "YUM", "YXM",
    "YYM", "YZM"
 };

static char *list_N[] = {
    "YQN", "ARN", "AFN", "A4N", "A6N", "A8N", "ABN", "ACN", "ADN",
    "AEN", "AGN", "AHN", "AIN", "AJN", "AKN", "ALN",
    "AMN", "ANN", "AON", "APN", "AQN", "ASN", "AUN",
    "AVN", "AWN", "AXN", "AYN", "AZN", "B2N", "B3N", "B4N",
    "B5N", "B7N", "B8N", "BAN", "BBN", "BCN", "BDN", "BEN",
    "BFN", "BHN", "BIN", "BJN", "BMN", "BNN", "BON", "BPN",
    "BQN", "BSN", "BTN", "BVN", "BWN", "BXN", "BYN", "BZN",
    "C2N", "C3N", "C4N", "C5N", "C6N", "C7N", "CAN", "CBN",
    "CCN", "CDN", "CEN", "CFN", "CGN", "CHN", "CIN", "CJN",
    "CKN", "CLN", "CON", "CQN", "CRN", "CSN", "CTN", "CUN",
    "CVN", "CWN", "CXN", "CYN", "CZN", "TEN", "TKN", "TQN",
    "YBN", "YCN", "YDN", "YEN", "YGN", "YHN", "YIN", "YJN",
    "YKN", "YLN", "YMN", "YNN", "YON", "YPN", "YSN",
    "YTN", "YUN", "YVN", "YWN", "YXN", "YYN", "YZN"
};

static char *list_O[] = {
    "AVO", "A2O",
    "A3O", "A4O", "A5O", "A6O", "ABO", "ACO", "AEO", "AFO",
    "AGO", "AHO", "AIO", "AKO", "AMO", "ANO", "AOO", "APO",
    "AQO", "ARO", "ATO", "AUO", "AWO", "AXO", "AYO",
    "B2O", "B4O", "B5O", "B6O", "B8O", "BAO", "BBO", "BCO",
    "BDO", "BFO", "BGO", "BHO", "BIO", "BJO", "BKO", "BLO",
    "BMO", "BNO", "BOO", "BPO", "BRO", "BSO", "BTO", "BUO",
    "BWO", "BXO", "BYO", "BZO", "C2O", "C3O", "C5O", "C6O",
    "C7O", "CAO", "CBO", "CDO", "CEO", "CFO", "CGO", "CHO",
    "CJO", "CLO", "CMO", "CNO", "COO", "CPO", "CQO", "CRO",
    "CSO", "CTO", "CWO", "CXO", "CZO", "YBO", "YCO", "YDO",
    "YEO", "YGO", "YHO", "YIO", "YKO", "YMO", "YNO", "YOO",
    "YPO", "YQO", "YRO", "YSO", "YTO", "YUO", "YVO", "YWO",
    "YXO", "YZO"
};

static char *list_P[] = {
    "CHP", "ANP", "A3P", "A4P", "A5P", "A6P", "A8P", "ACP",
    "ADP", "AEP", "AFP", "AGP", "AHP", "AJP", "AKP", "ALP",
    "AMP", "AOP", "APP", "AQP", "ARP", "ASP", "ATP",
    "AUP", "AVP", "AWP", "AXP", "AZP", "B2P", "B3P", "B5P",
    "B6P", "B7P", "B8P", "BAP", "BBP", "BCP", "BDP", "BEP",
    "BFP", "BGP", "BIP", "BJP", "BKP", "BLP", "BMP", "BNP",
    "BOP", "BPP", "BQP", "BRP", "BSP", "BTP", "BUP", "BVP",
    "BWP", "BXP", "BYP", "BZP", "C2P", "C3P", "C4P", "C5P",
    "C6P", "C7P", "CAP", "CBP", "CDP", "CEP", "CFP", "CGP",
    "CIP", "CJP", "CKP", "CLP", "CMP", "CNP", "COP",
    "CPP", "CQP", "CRP", "CSP", "CTP", "CUP", "CVP", "CWP",
    "CXP", "CYP", "TFP", "TMP", "TPP", "YAP", "YBP",
    "YCP", "YDP", "YEP", "YFP", "YGP", "YHP", "YIP", "YJP",
    "YKP", "YLP", "YMP", "YNP", "YOP", "YPP", "YQP", "YSP",
    "YTP", "YUP", "YVP", "YWP", "YXP", "YZP"
}; //"UZP", は BT基板でバックアップデバイスの触り方が異なる可能性があるため対象から外す

static char *list_Q[] = {
    "ASQ", "A3Q", "A4Q",
    "A5Q", "A6Q", "A8Q", "ACQ", "ADQ", "AEQ", "AFQ", "AGQ",
    "AHQ", "AIQ", "AJQ", "AKQ", "ALQ", "AMQ", "ANQ", "AOQ",
    "AQQ", "ARQ", "AUQ", "AVQ", "AWQ", "AYQ", "AZQ",
    "B2Q", "B3Q", "B5Q", "B6Q", "B7Q", "B8Q", "BAQ", "BBQ",
    "BCQ", "BEQ", "BFQ", "BGQ", "BHQ", "BKQ", "BLQ", "BMQ",
    "BNQ", "BOQ", "BPQ", "BQQ", "BRQ", "BTQ", "BUQ", "BVQ",
    "BWQ", "BXQ", "BYQ", "BZQ", "C2Q", "C3Q", "C4Q", "C5Q",
    "C6Q", "C7Q", "CAQ", "CBQ", "CCQ", "CDQ", "CEQ", "CFQ",
    "CGQ", "CIQ", "CJQ", "CKQ", "CLQ", "CMQ", "CNQ", "COQ",
    "CPQ", "CQQ", "CRQ", "CSQ", "CTQ", "CUQ", "CVQ", "CWQ",
    "CYQ", "CZQ", "TAQ", "TBQ", "TPQ", "YAQ", "YBQ", "YCQ",
    "YDQ", "YEQ", "YFQ", "YGQ", "YHQ", "YIQ", "YJQ", "YKQ",
    "YLQ", "YMQ", "YNQ", "YOQ", "YPQ", "YRQ", "YSQ", "YTQ",
    "YUQ", "YWQ", "YXQ", "YYQ", "YZQ"
};

static char *list_R[] = {
    "AVR", "A2R", "A3R", "A4R",
    "A5R", "A6R", "A8R", "ABR", "ACR", "ADR", "AER", "AFR",
    "AGR", "AHR", "AIR", "AJR", "AKR", "ALR", "AMR", "ANR",
    "APR", "AQR", "ARR", "ASR", "ATR", "AUR", "AWR",
    "AXR", "AYR", "AZR", "B3R", "B4R", "B5R", "B6R", "B8R",
    "BAR", "BDR", "BER", "BFR", "BGR", "BHR", "BIR", "BJR",
    "BKR", "BLR", "BMR", "BNR", "BOR", "BPR", "BQR", "BRR",
    "BSR", "BTR", "BUR", "BVR", "BWR", "BYR", "BZR", "C3R",
    "C4R", "C5R", "C6R", "C7R", "CAR", "CBR", "CCR", "CDR",
    "CER", "CFR", "CGR", "CIR", "CJR", "CKR", "CLR", "CMR",
    "CNR", "CPR", "CQR", "CRR", "CSR", "CTR", "CUR", "CVR",
    "CWR", "CXR", "CYR", "CZR", "TBR", "TCR", "THR", "TNR",
    "TRR", "UBR", "YAR", "YBR", "YCR", "YDR", "YER", "YFR",
    "YGR", "YHR", "YIR", "YJR", "YKR", "YLR", "YMR", "YNR",
    "YPR", "YQR", "YRR", "YSR", "YTR", "YUR", "YVR", "YWR",
    "YYR", "YZR"
};

static char *list_S[] = {
    "A2S", "A3S", "A6S", "A8S", "ABS", "AFS",
    "AHS", "AIS", "AJS", "AKS", "ALS", "AMS", "ANS", "AOS",
    "APS", "AQS", "ARS", "ASS", "AUS", "AVS", "AWS", "AXS",
    "AYS", "AZS", "B2S", "B3S", "B6S", "B8S", "BBS", "BCS",
    "BES", "BFS", "BJS", "BKS", "BLS", "BNS", "BOS", "BPS",
    "BQS", "BSS", "BUS", "BVS", "BWS", "BXS", "BYS", "BZS",
    "C2S", "C3S", "C5S", "CAS", "CBS", "CCS", "CDS", "CFS",
    "CGS", "CHS", "CIS", "CJS", "CLS", "CMS", "CNS", "COS",
    "CPS", "CQS", "CRS", "CSS", "CTS", "CUS", "CVS", "CWS",
    "CXS", "CYS", "CZS", "TAS", "TBS", "TFS", "TGS", "TKS",
    "TMS", "TPS", "YAS", "YBS", "YCS", "YDS", "YES",
    "YFS", "YGS", "YHS", "YIS", "YJS", "YLS", "YMS", "YOS",
    "YPS", "YQS", "YSS", "YTS", "YUS", "YVS", "YWS", "YXS",
    "YYS", "YZS"
}; // "UNS", は OSD用基板でバックアップデバイスの触り方が異なる可能性があるため対象から外す

static char *list_T[] = {
    "AOT", "A3T", "A2T", "A5T", "A6T", "A8T", "ABT",
    "ACT", "AET", "AFT", "AGT", "AHT", "AIT", "AJT", "AKT",
    "ALT", "AMT", "ANT", "APT", "AQT", "ART", "AST",
    "ATT", "AUT", "AVT", "AWT", "AXT", "AYT", "AZT", "B2T",
    "B3T", "B4T", "B5T", "B6T", "B8T", "BAT", "BDT", "BET",
    "BFT", "BGT", "BIT", "BKT", "BLT", "BMT", "BOT", "BQT",
    "BTT", "BUT", "BVT", "BYT", "C2T", "C3T", "C4T", "C5T",
    "C6T", "C7T", "CAT", "CBT", "CCT", "CDT", "CET", "CFT",
    "CHT", "CIT", "CKT", "CLT", "CMT", "CNT", "COT", "CPT",
    "CQT", "CST", "CTT", "CUT", "CVT", "CWT", "CXT", "CYT",
    "CZT", "TBT", "TCT", "TFT", "TGT", "THT", "TKT", "TLT",
    "TMT", "YAT", "YBT", "YCT", "YDT", "YET", "YFT", "YGT",
    "YHT", "YIT", "YJT", "YKT", "YLT", "YMT", "YNT", "YOT",
    "YPT", "YQT", "YRT", "YST", "YTT", "YUT", "YVT", "YWT",
    "YXT", "YYT", "YZT"
};

static char *list_U[] = {
    "AOU", "AWU", "YOU", "A2U", "A3U", "A4U", "A5U", "A6U",
    "A8U", "ABU", "ACU", "ADU", "AEU", "AFU", "AGU", "AJU",
    "AKU", "ALU", "AMU", "ANU", "APU", "AQU", "ARU",
    "ASU", "ATU", "AUU", "AVU", "AXU", "AYU", "AZU",
    "B2U", "B3U", "B5U", "B6U", "B8U", "BBU", "BCU", "BDU",
    "BEU", "BFU", "BGU", "BHU", "BIU", "BJU", "BKU", "BLU",
    "BMU", "BNU", "BOU", "BPU", "BQU", "BRU", "BSU", "BTU",
    "BUU", "BWU", "BXU", "BYU", "BZU", "C2U", "C3U", "C4U",
    "C5U", "C7U", "CBU", "CCU", "CDU", "CEU", "CFU", "CGU",
    "CHU", "CIU", "CJU", "CKU", "CLU", "CMU", "CNU", "COU",
    "CPU", "CQU", "CSU", "CTU", "CUU", "CVU", "CWU", "CXU",
    "CYU", "TMU", "YAU", "YBU", "YCU", "YDU", "YEU", "YFU",
    "YGU", "YIU", "YJU", "YKU", "YLU", "YMU", "YNU",
    "YQU", "YRU", "YSU", "YTU", "YUU", "YVU", "YWU",
    "YXU", "YYU", "YZU"
}; //"YHU", はバックアップなしリージョンが存在するので対象から外す

static char *list_V[] = {
    "A2V", "A3V", "A4V", "A8V", "ABV",
    "ACV", "ADV", "AEV", "AFV", "AGV", "AHV", "AIV", "AJV",
    "AKV", "ALV", "AMV", "AOV", "APV", "AQV", "ARV", "ASV",
    "ATV", "AUV", "AVV", "AWV", "AXV", "AZV", "B2V", "B3V",
    "B4V", "B5V", "B6V", "B7V", "B8V", "BAV", "BBV", "BDV",
    "BEV", "BFV", "BGV", "BHV", "BIV", "BKV", "BLV", "BMV",
    "BNV", "BPV", "BQV", "BRV", "BSV", "BTV", "BUV", "BVV",
    "BWV", "BXV", "BYV", "BZV", "C2V", "C3V", "C4V", "C5V",
    "C6V", "C7V", "CAV", "CBV", "CCV", "CDV", "CFV", "CHV",
    "CIV", "CKV", "CLV", "CMV", "CNV", "COV", "CPV", "CQV",
    "CRV", "CSV", "CTV", "CVV", "CWV", "CXV", "CYV", "CZV",
    "TDV", "TMV", "YAV", "YBV", "YCV", "YDV", "YEV", "YGV",
    "YHV", "YIV", "YJV", "YKV", "YLV", "YMV", "YNV", "YOV",
    "YPV", "YQV", "YRV", "YSV", "YTV", "YUV", "YVV", "YWV",
    "YXV", "YZV"
};

static char *list_W[] = {
    "A6W", "AVW", "A2W", "A3W", "A8W", "ACW", "ADW",
    "AEW", "AFW", "AGW", "AIW", "AJW", "AKW", "ALW", "AMW",
    "ANW", "AOW", "APW", "AQW", "ARW", "ASW", "ATW", "AUW",
    "AWW", "AXW", "AYW", "AZW", "B2W", "B3W", "B5W",
    "B6W", "B7W", "BAW", "BBW", "BCW", "BDW", "BEW", "BFW",
    "BGW", "BHW", "BIW", "BKW", "BLW", "BMW", "BNW", "BOW",
    "BPW", "BQW", "BRW", "BSW", "BTW", "BUW", "BWW", "BXW",
    "BYW", "C2W", "C3W", "C4W", "C5W", "C6W", "C7W", "CAW",
    "CBW", "CCW", "CDW", "CEW", "CFW", "CHW", "CIW", "CJW",
    "CKW", "CLW", "CMW", "COW", "CPW", "CRW", "CSW", "CTW",
    "CUW", "CVW", "CXW", "CYW", "CZW", "TBW", "TPW",
    "Y7W", "YAW", "YBW", "YCW", "YDW", "YEW", "YFW", "YGW",
    "YHW", "YIW", "YJW", "YKW", "YLW", "YMW", "YNW", "YOW",
    "YQW", "YRW", "YSW", "YTW", "YUW", "YVW", "YWW", "YXW",
    "YYW", "YZW"
}; //"IMW", は IRC基板でバックアップデバイスの触り方が異なるため対象から外す

static char *list_X[] = {
    "YUX", "ALX", "A2X", "A3X", "A6X", "A8X", "ABX", "ACX",
    "ADX", "AEX", "AFX", "AGX", "AHX", "AIX", "AKX", 
    "AMX", "ANX", "AOX", "APX", "AQX", "ARX", "ASX", "ATX",
    "AUX", "AVX", "AWX", "AXX", "AYX", "AZX", "B2X", "B3X",
    "B4X", "B5X", "B6X", "B7X", "BAX", "BBX", "BCX", "BDX",
    "BEX", "BFX", "BGX", "BHX", "BKX", "BMX", "BNX", "BOX",
    "BPX", "BRX", "BSX", "BTX", "BUX", "BVX", "BWX", "BXX",
    "BYX", "BZX", "C2X", "C3X", "C5X", "C6X", "C7X", "CAX",
    "CBX", "CCX", "CDX", "CEX", "CFX", "CGX", "CHX", "CIX",
    "CJX", "CKX", "CLX", "CMX", "CPX", "CQX", "CSX", "CTX",
    "CUX", "CVX", "CWX", "CXX", "CYX", "CZX", "TAX", "TMX",
    "YAX", "YBX", "YCX", "YDX", "YEX", "YFX", "YGX", "YHX",
    "YIX", "YJX", "YKX", "YLX", "YMX", "YNX", "YOX", "YPX",
    "YQX", "YRX", "YSX", "YTX", "YVX", "YWX", "YXX",
    "YYX", "YZX"
};

static char *list_Y[] = {
    "A2Y", "A3Y", "A4Y", "A6Y", "A8Y", "ACY",
    "AEY", "AFY", "AGY", "AIY", "AJY", "AKY", "ALY", "AMY",
    "ANY", "AOY", "APY", "AQY", "ARY", "ASY", "AUY", "AVY",
    "AWY", "AXY", "AYY", "AZY", "B2Y", "B3Y", "B4Y", "B5Y",
    "B6Y", "B7Y", "B8Y", "BBY", "BCY", "BDY", "BEY", "BFY",
    "BGY", "BHY", "BIY", "BJY", "BKY", "BMY", "BNY", "BOY",
    "BPY", "BQY", "BRY", "BSY", "BTY", "BUY", "BVY", "BWY",
    "BXY", "BYY", "BZY", "C2Y", "C3Y", "C4Y", "C5Y", "C6Y",
    "C7Y", "CAY", "CBY", "CCY", "CDY", "CEY", "CFY", "CGY",
    "CHY", "CIY", "CJY", "CKY", "CLY", "CMY", "CNY", "COY",
    "CQY", "CRY", "CSY", "CTY", "CUY", "CVY", "CWY", "CXY",
    "CYY", "CZY", "TMY", "YAY", "YBY", "YCY", "YDY", "YEY",
    "YFY", "YGY", "YHY", "YIY", "YJY", "YKY", "YLY", "YMY",
    "YNY", "YOY", "YQY", "YRY", "YSY", "YUY", "YVY", "YWY",
    "YXY", "YYY", "YZY"
};

static char *list_Z[] = {
    "A2Z", "A3Z", "A4Z", "A5Z", "A6Z",
    "A8Z", "ABZ", "ACZ", "ADZ", "AEZ", "AFZ", "AGZ", "AHZ",
    "AIZ", "AJZ", "AKZ", "ALZ", "AMZ", "ANZ", "AOZ", "APZ",
    "AQZ", "ARZ", "ASZ", "AUZ", "AWZ", "AXZ", "AYZ", "AZZ",
    "B2Z", "B3Z", "B4Z", "B5Z", "B6Z", "B7Z", "B8Z", "BAZ",
    "BBZ", "BCZ", "BDZ", "BEZ", "BFZ", "BGZ", "BHZ", "BIZ",
    "BLZ", "BMZ", "BNZ", "BOZ", "BPZ", "BQZ", "BRZ", "BTZ",
    "BUZ", "BVZ", "BWZ", "BYZ", "BZZ", "C2Z", "C4Z", "C5Z",
    "C6Z", "C7Z", "CAZ", "CBZ", "CCZ", "CDZ", "CEZ", "CGZ",
    "CHZ", "CIZ", "CKZ", "CLZ", "CMZ", "CNZ", "COZ", "CPZ",
    "CQZ", "CRZ", "CSZ", "CTZ", "CUZ", "CVZ", "CXZ", "CZZ",
    "THZ", "TKZ", "TRZ", "YAZ", "YBZ", "YCZ", "YDZ", "YEZ",
    "YGZ", "YHZ", "YIZ", "YJZ", "YKZ", "YLZ", "YMZ", "YNZ",
    "YOZ", "YPZ", "YQZ", "YRZ", "YSZ", "YTZ", "YVZ", "YWZ",
    "YYZ", "YZZ"
};

static lgyGameCodeList game_code_list[43] = {
    { NULL,    0}, // '0'
    { NULL,    0}, // '1'
    { list_2, sizeof(list_2)/sizeof(char*)}, // '2'
    { list_3, sizeof(list_3)/sizeof(char*)}, // '3'
    { list_4, sizeof(list_4)/sizeof(char*)}, // '4'
    { list_5, sizeof(list_5)/sizeof(char*)}, // '5'
    { list_6, sizeof(list_6)/sizeof(char*)}, // '6'
    { list_7, sizeof(list_7)/sizeof(char*)}, // '7'
    { list_8, sizeof(list_8)/sizeof(char*)}, // '8'
    { list_9, sizeof(list_9)/sizeof(char*)}, // '9'
    { NULL,    0}, // ':'
    { NULL,    0}, // ';'
    { NULL,    0}, // '<'
    { NULL,    0}, // '='
    { NULL,    0}, // '>'
    { NULL,    0}, // '?'
    { NULL,    0}, // '@'
    { list_A, sizeof(list_A)/sizeof(char*)}, // 'A'
    { list_B, sizeof(list_B)/sizeof(char*)}, // 'B'
    { list_C, sizeof(list_C)/sizeof(char*)}, // 'C'
    { list_D, sizeof(list_D)/sizeof(char*)}, // 'D'
    { list_E, sizeof(list_E)/sizeof(char*)}, // 'E'
    { list_F, sizeof(list_F)/sizeof(char*)}, // 'F'
    { list_G, sizeof(list_G)/sizeof(char*)}, // 'G'
    { list_H, sizeof(list_H)/sizeof(char*)}, // 'H'
    { list_I, sizeof(list_I)/sizeof(char*)}, // 'I'
    { list_J, sizeof(list_J)/sizeof(char*)}, // 'J'
    { list_K, sizeof(list_K)/sizeof(char*)}, // 'K'
    { list_L, sizeof(list_L)/sizeof(char*)}, // 'L'
    { list_M, sizeof(list_M)/sizeof(char*)}, // 'M'
    { list_N, sizeof(list_N)/sizeof(char*)}, // 'N'
    { list_O, sizeof(list_O)/sizeof(char*)}, // 'O'
    { list_P, sizeof(list_P)/sizeof(char*)}, // 'P'
    { list_Q, sizeof(list_Q)/sizeof(char*)}, // 'Q'
    { list_R, sizeof(list_R)/sizeof(char*)}, // 'R'
    { list_S, sizeof(list_S)/sizeof(char*)}, // 'S'
    { list_T, sizeof(list_T)/sizeof(char*)}, // 'T'
    { list_U, sizeof(list_U)/sizeof(char*)}, // 'U'
    { list_V, sizeof(list_V)/sizeof(char*)}, // 'V'
    { list_W, sizeof(list_W)/sizeof(char*)}, // 'W'
    { list_X, sizeof(list_X)/sizeof(char*)}, // 'X'
    { list_Y, sizeof(list_Y)/sizeof(char*)}, // 'Y'
    { list_Z, sizeof(list_Z)/sizeof(char*)}  // 'Z'
};



BOOL CheckBackupDevice( TitleProperty *rhs)
{
    u16 list_num;
    u8 second_character_value;
    char test[5];

    {
        char* n = (char*)&rhs->titleID;
        test[0] = n[3];
        test[1] = n[2];
        test[2] = n[1];
        test[3] = n[0];
        test[4] = '\0';
    }

/*
    {   // テスト用
        int total = 0;
        for( int i=0; i<43; i++)
        {
            total += game_code_list[i].num;
            if( !game_code_list[i].code_list)
            {
                OS_TPrintf( "[%d] : NULL, %d\n", i, game_code_list[i].num);
            }
            else
            {
                OS_TPrintf( "[%d] : 0x%x, %d\n", i, game_code_list[i].code_list, game_code_list[i].num);
            }
        }
        OS_TPrintf( "total : %d\n", total);
    }
*/

    second_character_value = (u8)(test[2]); // 3文字目
    if( (second_character_value < '0')||(second_character_value > 'Z'))
    {
        OS_TPrintf(" This game has no backup device...\n");
        return TRUE;
    }

    {
        u8     second_character_index = second_character_value - '0';
        u16    max                    = game_code_list[second_character_index].num;
        char** small_list             = game_code_list[second_character_index].code_list;
        for( list_num = 0; list_num < max; list_num++)
        {
            OS_TPrintf("small list:%s\n", small_list[list_num]);
            if( 0 == STD_CompareNString( &test[0], small_list[list_num], 2))
            {
                OS_TPrintf("%s has backup device.\n", small_list[list_num]);
                return checkDevice();
            }
        }
    }
    OS_TPrintf(" This game has no backup device.\n");
    return TRUE;
}

/*
  今のところは WriteEnable & WriteDisable による StatusRegister の変化をチェック
  将来的にはデバイス毎にライト＆ベリファイチェックまで行う可能性あり。
 */
static BOOL checkDevice( void)
{
    BOOL result1, result2;
    u8  status1, status2;

    InitializeBackup();
    result1 = ReadBackupStatus(&status1);
    result2 = ReadBackupStatus(&status2);
    FinalizeBackup();

    OS_TPrintf("status1 : 0x%x\n", status1);
    OS_TPrintf("status2 : 0x%x\n", status2);
    
    if( (result1 != FALSE)&&(!(status1 & 0x01))&&
        (result2 != FALSE)&&(!(status2 & 0x01)))
    {
        OS_TPrintf(" backup device check ok.\n");
        return TRUE;
    }
    OS_TPrintf(" backup device check ng.\n");
    return FALSE;
}

/*
    ステータスレジスタのチェック
 */
static BOOL ReadBackupStatus( u8* status)
{
    return CARDi_RequestStreamCommand((u32)0, (u32)status, 1, NULL, 0, FALSE,
                                      CARD_REQ_READ_STATUS, 0, CARD_REQUEST_MODE_RECV);
}

static BOOL ReadBackupID( u32* devid)
{
    return CARDi_RequestStreamCommand((u32)0, (u32)devid, 4, NULL, 0, FALSE,
                                      CARD_REQ_READ_ID, 0, CARD_REQUEST_MODE_RECV);
}


/*
    InitializeTwlBackup();
    // 全部CPUなのでキャッシュケアは必要なし
    OS_TPrintf("buf : 0x%x\n", buf);

    memset( buf, 0x5A, 512);
    if( WriteTwlEEPROM( 0, buf, 512).IsFailure())
    {
        OS_TPrintf("launch NG. (write EEPROM error!)\n");
    }
    memset( buf, 0xFF, 512);
    if( ReadNtrEEPROM( 0, (u32*)buf, 512).IsFailure())
    {
        OS_TPrintf("launch NG. (read error!)\n");
        FinalizeTwlBackup();
        return nn::fs::ResultInvalidCciFormat();
    }
    FinalizeTwlBackup();
        __breakpoint(0);
*/
