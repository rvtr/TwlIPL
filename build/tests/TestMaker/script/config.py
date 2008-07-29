#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os, sys
import yaml,codecs

romparampath = sys.argv[1]
codeslist    = sys.argv[2]
testtype     = sys.argv[3]
#romparampath = "../romparam.yaml"
#codeslist    = "../codeslist"
romparam = yaml.load(codecs.open(romparampath,'r','utf16'))

#----- 定数
rsfkey    = 'RSF'
bnrkey    = 'Banner'
eulakey   = 'EULA'
ratingkey = 'Rating'

#----- gamecodes ファイルの生成 ------------------------------------
try:
    gamecodes = open(codeslist,'w')
except:
    print 'config.py : can not open %s file' % gamecodes
else:
    gamecodes.write('TEST_TYPE = %s\n' % testtype)
    gamecodes.write('GAMECODES = ')
    for num in romparam.keys():
         gamecodes.write(romparam[num][rsfkey]['GameCode']+' ')
finally:
    gamecodes.close()

#----- gamecode 毎の各種設定ファイル生成 ---------------------------
for num in romparam.keys():
    gamecode = romparam[num][rsfkey]['GameCode']
    #----- gamecode dir の生成
    try:
        os.mkdir(gamecode)
    except:
        print 'config.py : can not create dir %s' % gamecode
        continue

    #----- gamecode.param の生成
    try:
        codepath  = ''.join(['./',gamecode,'/',gamecode,'.param'])
        codeparam = open(codepath,'w')
    except:
        print 'config.py : can not open %s file' % codepath
        continue
    else:
        #----- MAKETAD_OPTION, ROM_SPEC_TEMPLATE の指定
        if romparam[num][rsfkey].get('AppType') == 'SYSTEM':
            #----- MAKEROM の指定
            if romparam[num][rsfkey]['Secure']:
                codeparam.write('MAKEROM           := $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe\n')
            else:
                codeparam.write('MAKEROM           := $(TWL_TOOLSDIR)/bin/makerom.TWL.sys.exe\n')
            codeparam.write('MAKETAD_OPTION    += -s\n')
            codeparam.write('ROM_SPEC_TEMPLATE  = $(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf\n')
        else:
            # 暫定対処 include/twl/specfiles 以下をきちんと使うように変更する必要がある
            if testtype == 'PARENTAL':
                codeparam.write('ROM_SPEC_TEMPLATE  = ../config/ROM-TS_nand_forPARENTAL.rsf\n')
            else:
                codeparam.write('ROM_SPEC_TEMPLATE  = $(ROOT)/include/twl/specfiles/ROM-TS_nand.rsf\n')

        #----- ROM_SPEC_OPTIONS key の抽出
        if romparam[num][rsfkey].get('AppType') == 'SYSTEM':
            keys = [key for key in romparam[num][rsfkey].keys() if key != 'TitleType' and key != 'eTicket' ]
        else:
            keys = [key for key in romparam[num][rsfkey].keys() if key != 'TitleType' and key != 'eTicket'
                    and key != 'AppType' and key != 'Launch' and key != 'DataOnly' and key != 'Secure']

        #----- ROM_SPEC_OPTIONS 文字列の生成
        rsf_opt = []
        for key in keys:
            if key == 'Version':
                rsf_opt.append('RomVersion=%s '%(romparam[num][rsfkey][key]/256))
            elif key == 'GameCode':
                rsf_opt.append('%s=%s '%(key,str(romparam[num][rsfkey][key])))
            elif key == 'TitleName':
                if str(romparam[num][rsfkey][key]) == 'None':
                    rsf_opt.append('%s=%s '%(key,'TITLE_'+gamecode.upper()))
                else:
                    rsf_opt.append('%s=%s '%(key,romparam[num][rsfkey][key]))
            else:
                if isinstance(romparam[num][rsfkey][key],bool):
                    rsf_opt.append('%s=%s '%(key,str(romparam[num][rsfkey][key]).upper()))
                else:
                    rsf_opt.append('%s=%s '%(key,str(romparam[num][rsfkey][key])))

        #----- Banner 関連 ROM_SPEC_OPTIONS
        if romparam[num].has_key(bnrkey):
            rsf_opt.append('%s=%s '%('BannerFile',romparam[num][bnrkey]['File']))
        #----- EULA 関連 ROM_SPEC_OPTIONS
        if romparam[num].has_key(eulakey):
            rsf_opt.append('%s=%s '%('AgreeEULA',romparam[num][eulakey]['Agree']))
            rsf_opt.append('%s=%s '%('AgreeEULAVersion',str(romparam[num][eulakey]['Version'])))
        #----- Rating 関連 ROM_SPEC_OPTIONS
        if romparam[num].has_key(ratingkey):
            rsf_opt.append('%s=%s '%(romparam[num][ratingkey]['Ogn'],str(romparam[num][ratingkey]['Age'])))
        #----- ROM_SPEC_OPTIONS の指定
        rsf_opt_str = ''.join(rsf_opt)
        codeparam.write('ROM_SPEC_PARAM   = %s\n' % rsf_opt_str)
        #----- TITLE_VERSION_LO の指定
        codeparam.write('TITLE_VERSION_LO   = %d\n' % (romparam[num][rsfkey]['Version']%256))
    finally:
        codeparam.close()

    #----- banner.bsf の生成
    try:
        bsfpath = ''.join(['./',gamecode,'/banner.bsf'])
        bsffile = codecs.open(bsfpath,'w','utf_16')
    except:
        print 'config.py : can not open %s file' % bsfpath
        continue
    else:
        #----- バージョンの指定
        bsffile.write('%s: %s\r\n'%(key,str(romparam[num][bnrkey]['Version'])))

        #----- タイトル文字列の指定（言語別）
        for key in ('JP','EN','FR','GE','IT','SP','CN','HN'):
            if romparam[num][bnrkey].has_key(key):
                bsffile.write('%s:'% key)
                for line in romparam[num][bnrkey][key]:
                    bsffile.write(' "%s"\r\n'% line)
            else:
                titlestr  = ''.join(['"TITLE_',gamecode,'"\r\n',' "LANG_',key,'"\r\n'])
                bsffile.write('%s: %s'%(key,titlestr))
    finally:
        bsffile.close()