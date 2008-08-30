#----------------------------------------------------------------------------
# Project:  TwlSDK - tools
# File:     toaqua.sh
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
require "find"
require "fileutils"

REGION_DIR = {
    "U" => ["aus"],
    "E" => ["usa"],
    "J" => ["jp"],
    "P" => ["euro"],
    "A" => ["aus", "usa", "jp", "euro"],
}

VERUP_DIR = "verup"
VERUP_CLS = "#{VERUP_DIR}/cls"
VERUP_TAD = "#{VERUP_DIR}/tad"

def proc(target_dir, output_dir)
    Find.find(target_dir){ |path|
        next unless File.file?(path) && (/[a-z0-9A-Z]{4}-.*v[0-9]+\.tad\.out/ =~ path) != nil
#        p path
        src = path

        # ディレクトリ内のtadファイルの.tadより前を取り出す
        dest = ""
        version = ""
        Find.find(File.split(src)[0]){|tad|
            # .tad が見つかった
            if (/.*\.tad\z/ =~ tad) != nil 
                dest = File.split(tad)[1]
            end
            if (/.*\.cls\z/ =~ tad) != nil
                p tad
                version = tad.slice(/v[0-9]+/)
            end
        }
        dest = /\.tad/.match(dest).pre_match + "-" + version + ".tad"
        # リージョンを判別してコピー
        regions = REGION_DIR[File.split(src)[1][3..3]]
        for region in regions do
            if version != "v0"
                p version
                p src
                system("cp #{src} #{[output_dir, "#{VERUP_TAD}", dest].join("/")}")                
                system("cp #{src.sub(/\.tad\.out/, ".cls")} #{[output_dir, "#{VERUP_CLS}", dest.sub(/\.tad/, ".cls")].join("/")}")
            else
                system("cp #{src} #{[output_dir, region, dest].join("/")}")
            end
        end
    }
    
end

# こっからメイン

if ARGV.size < 1
    $stdout.printf("Usage:#{__FILE__} target_clsed_dir output_dir")
    exit -1
end
src = ARGV[0]
out = "#{src}_foraqua"
system("mkdir #{out}")
system("mkdir #{out}/aus")
system("mkdir #{out}/usa")
system("mkdir #{out}/euro")
system("mkdir #{out}/jp")
system("mkdir #{out}/#{VERUP_DIR}")
system("mkdir #{out}/#{VERUP_CLS}")
system("mkdir #{out}/#{VERUP_TAD}")

system("cp -rf #{src}/aus #{out}")
system("cp -rf #{src}/usa #{out}")
system("cp -rf #{src}/euro #{out}")
system("cp -rf #{src}/jp #{out}")

proc(src, out)

