#----------------------------------------------------------------------------
# Project:  TwlSDK
# File:     romchecker.rb
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date::            $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

ALL_REGION = [
    "C", "D", "E", "H",
]

FIX_REGION = [
    "A", "B", "F", "G", "I", "J", "K", "L",
]

HEADER = "HN"

REGION_DIRS = {
    "aus" => "U",
    "euro" => "P",
    "jp" => "J",
    "usa" => "E",
}

REG_PATH = /[a-zA-Z0-9\.\-_]+\/[a-zA-Z0-9\.\-_*]+\z/

NAND_FIRM = "*.nand"
FONT_DATA = "*.dat"

# ファイルを検索し、ROM内に含まれるべきかどうかを判定する
# 条件に一致したファイル名の配列の要素数で判定
# 0 個　ファイルがない
# 1 個　正常っぽい
# 2 個以上　バージョン違いや、リージョン違いが混ざっている
def check_pickuped(path, print_success)
    ret = true
    files = Dir.glob(path)
    p_path = path.slice(REG_PATH)
    case 
    when files.size == 0
        $stdout.printf( "NG:#{p_path} -- #{p_path} is not included ***********\n")
        ret = false
    when files.size == 1 && print_success
        $stdout.printf( "OK:#{files[0].slice(REG_PATH)}\n")
    when files.size >= 2
        $stdout.printf("NG:")
        files.each{|file|
            $stdout.printf( "#{file.slice(REG_PATH)} ")
        }
        $stdout.printf("-- Too many files are included ***********\n")
        ret = false
    end
    ret
end


def check(target_dir)
    ret = true
    REGION_DIRS.each{|region_dir, region_code|
        ALL_REGION.each{|app_code|
            # オールリージョンのロムが含まれているかのチェック
            search_path = "#{target_dir}/#{region_dir}/#{HEADER}#{app_code}a*.tad"
            ret &= check_pickuped(search_path, false)
        }
        FIX_REGION.each{|app_code|
            # 各リージョン固有のロムが含まれているかのチェック
            search_path = "#{target_dir}/#{region_dir}/#{HEADER}#{app_code}#{region_code}*.tad"
            ret &= check_pickuped(search_path, false)
        }
        (ALL_REGION + FIX_REGION).each{|app_code|
            # HNxy が 各リージョンごと1つだけ存在するかのチェック
            # HNxA と HNxU など、aとuが混在していたりするとここでひっかかる
            search_path = "#{target_dir}/#{region_dir}/#{HEADER}#{app_code}*.tad"
            ret &= check_pickuped(search_path, false)
        }
        
        # nand firm, font のチェック
        search_path = "#{target_dir}/#{region_dir}/#{NAND_FIRM}"
        ret &= check_pickuped(search_path, false)
        search_path = "#{target_dir}/#{region_dir}/#{FONT_DATA}"
        ret &= check_pickuped(search_path, false)

        $stdout.printf("\n")
    }
    ret
end

# こっからメイン

if ARGV.size < 1
    p "Usage: #{__FILE__} targetdir"
    exit -1
end

target = ARGV[0]

exit (check target) ? 0 : -1

