#!/bin/ruby
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

ROM_DATA = {
    "#{HEADER}A" => ["Launcher", "00030017"],
    "#{HEADER}B" => ["本体設定", "00030015"],
    "#{HEADER}C" => ["無線ファーム", "0003000f"],
    "#{HEADER}D" => ["DSダウンロードプレイ", "00030005"],
    "#{HEADER}E" => ["ピクトチャット", "00030005"],
    "#{HEADER}F" => ["ショップ", "00030015"],
    "#{HEADER}G" => ["フルブラウザ", "00030004"],
    "#{HEADER}H" => ["DSハッシュリスト", "0003000f"],
    "#{HEADER}I" => ["写真帳", "00030005"],
    "#{HEADER}J" => ["ホットスポット", "00030005"],
    "#{HEADER}K" => ["DSサウンド", "00030005"],
    "#{HEADER}L" => ["バージョンデータ", "0003000f"],
}

REGION_DIRS = {
    "AUS" => ["aus", "U"],
    "EUR" => ["euro", "P"],
    "JPN" => ["jp", "J"],
    "USA" => ["usa", "E"],
    "VERUP" => ["verup", "UPJE"],
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
    #p files
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

def add_rom_list(rom_list, code, region, version, size)
    rom_list << [code, region, version, size]
end

def make_rom_list(target_dir, region)
    region_dir = REGION_DIRS[region][0]
    region_codes = (REGION_DIRS[region][1] + "A").unpack("A" * (REGION_DIRS[region][1].size + 1))
    #p region_codes
    # 出力用リスト
    rom_list = []
    
    (ALL_REGION + FIX_REGION).each{|app_code|
        region_codes.each{|region_code|
            search_path = File.join(target_dir, region_dir, "#{HEADER}#{app_code}#{region_code}*.tad")
            files = Dir.glob(search_path)
            files.each{|tad|
                add_rom_list(rom_list, "#{HEADER}#{app_code}", region_code, get_rom_version(tad), File.stat(tad).size)
            }
        }
    }
    write_rom_list(rom_list, "rom_list_#{region}.txt")
end

def write_rom_list(rom_list, file_name)
    region_name = {
        "A" => "ALL", "E" => "USA", "U" => "AUS", "J" => "JPN", "P" => "EUR"
    }
    out = ""
    sp = [8, 20, 10, 17, 10, 14]
    line = ""
    sp.each{|s|
        line += "+#{"-" * s}"
    }
    line += "+\n"
    out += line
    out += sprintf("|%#{sp[0]}s|%-#{sp[1]}s|%#{sp[2]}s|%-#{sp[3]}s|%#{sp[4]}s|%#{sp[5]}s|\n", 
                        "コード", "名前", "リージョン", "タイトルID", "バージョン", "ファイルサイズ") 
    out += line
    rom_list.each{|rom|
        rom_data = ROM_DATA[rom[0]]
        size = (rom[3].to_s).reverse.scan(/.{1,3}/).join(",").reverse
        out += sprintf("|%#{sp[0]}s|%-#{sp[1]}s|%-#{sp[2]}s|%8s %8x|%#{sp[4]}s|%#{sp[5]}s|\n", 
            rom[0] + rom[1], rom_data[0],region_name[rom[1]], rom_data[1], (rom[0] + rom[1]).unpack("N")[0], rom[2], size);
    }
    out += line
    File.open(file_name, "w") {|file|
        file.puts out
    }
end

def get_rom_version(path)
    files = Dir.glob(path)
    files[0].slice(/v[0-9]+/).delete("v")
end

def check(target_dir, region)
    ret = true
    if REGION_DIRS[region] == nil
        puts "Invalid region, [JPN, USA, EUR, AUS, VERUP]"
        return false
    end
    region_dir = REGION_DIRS[region][0]
    region_code = REGION_DIRS[region][1]
    
    # 出力用リスト
    rom_list = []
    
    ALL_REGION.each{|app_code|
        # オールリージョンのロムが含まれているかのチェック
        search_path = File.join(target_dir, region_dir, "#{HEADER}#{app_code}A*.tad")
        #if ret &= check_pickuped(search_path, false)
            #add_rom_list(rom_list, "#{HEADER}#{app_code}", "A", get_rom_version(search_path))
        #end
        check_pickuped(search_path, false)
    }
    FIX_REGION.each{|app_code|
        # 各リージョン固有のロムが含まれているかのチェック
        search_path = File.join(target_dir, region_dir, "#{HEADER}#{app_code}#{region_code}*.tad")
        #if ret &= check_pickuped(search_path, false)
            #add_rom_list(rom_list, "#{HEADER}#{app_code}", region_code, get_rom_version(search_path))
        #end
        ret &= check_pickuped(search_path, false)
    }
    (ALL_REGION + FIX_REGION).each{|app_code|
        # HNxy が 各リージョンごと1つだけ存在するかのチェック
        # HNxA と HNxU など、aとuが混在していたりするとここでひっかかる
        search_path = File.join(target_dir, region_dir, "#{HEADER}#{app_code}*.tad")
        ret &= check_pickuped(search_path, false)
    }
    
    # nand firm, font のチェック
    search_path = File.join(target_dir, region_dir, NAND_FIRM)
    ret &= check_pickuped(search_path, false)
    search_path = File.join(target_dir, region_dir, FONT_DATA)
    ret &= check_pickuped(search_path, false)
    $stdout.printf("\n")

    # CSUに含まれているロムの一覧を書き出す
#    write_rom_list(rom_list, "rom_list_#{region}.txt")

#    make_rom_list(target_dir, region)

    ret
end

# こっからメイン

if ARGV.size < 2
    p "Usage: #{__FILE__} targetdir region"
    exit -1
end

# カレントディレクトリの変更
Dir.chdir(File.dirname(File.expand_path(__FILE__)))


target = ARGV[0]
region = ARGV[1]
make_rom_list(target, region)
if region == "VERUP"
    exit 0
else
    exit check(target,region) ? 0 : -1
end

