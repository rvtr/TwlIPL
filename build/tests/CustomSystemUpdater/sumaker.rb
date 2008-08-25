#----------------------------------------------------------------------------
# Project:  TwlSDK
# File:     sumaker.rb
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

require "yaml"
require "fileutils"
require "tempfile"

DEFAULT_CONFIG = "default.yaml"
CUSTOM_CONFIG = "custom.yaml"

FILE_MATCH = /[a-zA-Z0-9\-_\.]+\z/

REGION = {
    "JPN" => "OS_TWL_REGION_JAPAN",
    "EUR" => "OS_TWL_REGION_EUROPE",
    "AUS" => "OS_TWL_REGION_AUSTRALIA",
    "USA" => "OS_TWL_REGION_AMERICA",
}

REGION_PATH = {
    "JPN" => "jp",
    "EUR" => "euro",
    "AUS" => "aus",
    "USA" => "usa",
}

# �f�t�H���g�ݒ�̍쐬
def make_default_config
    config = {
        :TadFiles => [
        "HNAA.tad",
        "HNBA.tad",
        "HNCA.tad",
        "HNLA.tad",
        "HNHA.tad",
        ],
        :FontFile => "TWLFontTable.dat",
        :NandFirm => "menu_launcher.nand",
        :TargetPath => ".",
        :TemplatePath => "template",
        :RSF => "ARM9.TWL/main.rsf",
        :SRC => "ARM9.TWL/src/main.c",
        :DataPath => "data",
        :Region => "JPN",
    }
    write_config(DEFAULT_CONFIG, config)
    
end

# �R���t�B�O�t�@�C����ǂ�œW�J
def read_config(filename)
    YAML.load(File.read(filename))
end

# �R���t�B�O�̃��C�g
def write_config(filename, data)
    File.open(filename, "w") {|file|
        file.write data.to_yaml
    }
end

# �f�[�^���̕������u��������
def replace_data(src, mark, data)
    while src.index(/%#{mark}%/) != nil do
        src = src.sub(/%#{mark}%/, data)
    end
    src
end


# �t�@�C������������
def write_data(filename, data)
    temp = Tempfile.new("temp")
    temp.puts data
    temp.close
    FileUtils.cp(temp.path, filename)
end

# main.rsf�̏�������
def make_rsf(config)
    tadlist = config[:TadFiles].dup
    tadlist << config[:FontFile] << config[:NandFirm]
    tadlist = tadlist.join(" ")
    src = File.read(config[:TemplatePath] + "/" + config[:RSF])
    src = replace_data(src, "datapath", config[:DataPath])
    src = replace_data(src, "regionname", config[:Region])
    write_data(config[:TargetPath] + "/" + config[:RSF], replace_data(src, "filelist", tadlist))
end

# main.c �̏�������
def make_main(config)
    filelist = config[:TadFiles]
    data = []
    for t in filelist do 
        data.push("\t\"rom:/data/" + t + "\", \n")
    end
    data = data.join
    src = File.read(config[:TemplatePath] + "/" + config[:SRC])
    mod = replace_data(src, "tadlist", data)
    mod = replace_data(mod, "nandfirm", config[:NandFirm])
    mod = replace_data(mod, "fontfile", config[:FontFile])
    
    # region
	region_name = (config[:Region] != nil) ? config[:Region] : "JPN" 
    region = (REGION[region_name] != nil) ? REGION[region_name] : REGION["JPN"] 
    mod = replace_data(mod, "region", region)
    mod = replace_data(mod, "regionname", region_name)
    write_data(config[:TargetPath] + "/" + config[:SRC], mod)    
end

# SystemUpdater���쐬����
def make_updater(filename)
    config = read_config(filename)  
    make_rsf(config)
    make_main(config)
end

# �f�B���N�g������tad������
def pickup_files(target_dir, name)
    Dir.glob(target_dir + name)
end


# �������烁�C��

#sumaker mode [filename]

if ARGV.size == 0
    p "Usage: sumaker mode [filename]"
    exit
end

mode = ARGV[0]

case mode 
when "default" then
    # �f�t�H���g�ݒ�ō쐬
    make_default_config
    make_updater(DEFAULT_CONFIG)
when "custom" then
    if ARGV.size < 3 
        p "Usage: sumaker custom target_dir region"
        exit
    end
    # �J�X�^���ݒ�ō쐬
    config = "custom_base.yaml"
    region = ARGV[2]

    # �x�[�X�R���t�B�O�ɁAdata�f�B���N�g������tad��nand��ǉ�
    config = read_config(config)
    if config[:TadFiles] == nil
        config[:TadFiles] = []
    end
    
    config[:DataPath] += "/#{ARGV[1]}/#{REGION_PATH[region]}"
    config[:Region] = region
    tads = pickup_files(config[:DataPath], "/*.tad")
    nand = pickup_files(config[:DataPath], "/*.nand")
    font = pickup_files(config[:DataPath], "/*.dat")
    if nand.size > 0 
        config[:NandFirm] = nand[0].slice(FILE_MATCH)
    end
    if font.size > 0 
        config[:FontFile] = font[0].slice(FILE_MATCH)
    end
    for tad in tads do
        tad = tad.slice(FILE_MATCH)
        config[:TadFiles] << tad
    end
    config[:TadFiles] = config[:TadFiles].uniq
    write_config(CUSTOM_CONFIG, config)
    make_updater(CUSTOM_CONFIG)
end

