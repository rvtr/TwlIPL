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
optkey    = 'Option'

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
        romspectemplate = ''
        #----- MAKETAD_OPTION, ROM_SPEC_TEMPLATE の指定
        debugmakerom = ''
        if romparam[num].has_key(optkey) and romparam[num][optkey].has_key('SysmenuDebug') and romparam[num][optkey]['SysmenuDebug']:
            debugmakerom = '.DEBUG'
        if romparam[num][rsfkey].get('AppType') == 'SYSTEM':
            #----- MAKEROM の指定
            if romparam[num][rsfkey]['Secure']:
                codeparam.write(''.join(['override MAKEROM           = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure', debugmakerom, '.exe\n']))
            else:
                codeparam.write(''.join(['override MAKEROM           = $(TWL_TOOLSDIR)/bin/makerom.TWL.sys', debugmakerom, '.exe\n']))
            codeparam.write('MAKETAD_OPTION    += -s\n')
            romspectemplate = '$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf'
        else:
            if debugmakerom == '.DEBUG':
                codeparam.write('override MAKEROM           = $(TWL_TOOLSDIR)/bin/makerom.TWL.DEBUG.exe\n')
            # 暫定対処 include/twl/specfiles 以下をきちんと使うように変更する必要がある
            if testtype == 'PARENTAL':
                romspectemplate = '../config/ROM-TS_nand_forPARENTAL.rsf'
            else:
                romspectemplate = '$(ROOT)/include/twl/specfiles/ROM-TS_nand.rsf'
        if romparam[num].has_key(optkey) and romparam[num][optkey].has_key('RomSpecTemplate'):
            romspectemplate = romparam[num][optkey].get('RomSpecTemplate')
        codeparam.write(''.join(['ROM_SPEC_TEMPLATE  = ',romspectemplate,'\n']))

        #----- ROM_HEADER_TEMPLATEおよびLIBSYSCALLの指定
        if romparam[num].has_key(optkey) and romparam[num][optkey].has_key('UseFinalHeader') and romparam[num][optkey]['UseFinalHeader']:
            codeparam.write(''.join(['ROM_HEADER_TEMPLATE = $(SYSMENU_ROM_HEADER_DIR)/',gamecode,'/rom_header_', gamecode.lower(), '.template.sbin\n']))
            codeparam.write(''.join(['LIBSYSCALL = $(SYSMENU_ROM_HEADER_DIR)/',gamecode,'/libsyscall.a\n']))
            if debugmakerom == '.DEBUG':
                codeparam.write('MAKEROM_FLAGS += -DSYSCALL_C=\'$(call empath,$(LIBSYSCALL:.a=_c.bin))\'\n')
        #----- モジュールサイズ拡張
        if romparam[num].has_key(optkey) and debugmakerom == '.DEBUG':
            if romparam[num][optkey].has_key('ARM9FLXExpand'):
                codeparam.write(''.join(['MAKEROM_FLAGS += -DARM9FLX_EXPAND=', hex(romparam[num][optkey]['ARM9FLXExpand']), '\n']))
            if romparam[num][optkey].has_key('ARM7FLXExpand'):
                codeparam.write(''.join(['MAKEROM_FLAGS += -DARM7FLX_EXPAND=', hex(romparam[num][optkey]['ARM7FLXExpand']), '\n']))
            if romparam[num][optkey].has_key('ARM9LTDExpand'):
                codeparam.write(''.join(['MAKEROM_FLAGS += -DARM9LTD_EXPAND=', hex(romparam[num][optkey]['ARM9LTDExpand']), '\n']))
            if romparam[num][optkey].has_key('ARM7LTDExpand'):
                codeparam.write(''.join(['MAKEROM_FLAGS += -DARM7LTD_EXPAND=', hex(romparam[num][optkey]['ARM7LTDExpand']), '\n']))
        #----- モジュールオフセット
        if romparam[num].has_key(optkey) and romparam[num][optkey].has_key('ARM9FLXAddr'):
            codeparam.write(''.join(['ADDRESS_STATIC	= ', hex(romparam[num][optkey]['ARM9FLXAddr']), '\n']))
        #----- ARCHGEN
        if romparam[num].has_key(optkey) and romparam[num][optkey].has_key('ArchGen'):
            codeparam.write(''.join(['override TWL_ARCHGEN     = ', romparam[num][optkey].get('ArchGen'), '\n']))
        #----- ARM7コンポーネント指定
        if romparam[num].has_key(optkey) and romparam[num][optkey].has_key('ARM7Base'):
            codeparam.write(''.join(['MAKEROM_ARM7_BASE	= ', romparam[num][optkey].get('ARM7Base'), '\n']))
            codeparam.write('MAKEROM_ARM7		= $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)\n')
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
        #----- RomHeaderTemplateの指定
        if romparam[num].has_key(optkey) and romparam[num][optkey].has_key('UseFinalHeader') and romparam[num][optkey]['UseFinalHeader']:
            rsf_opt.append('RomHeaderTemplate=$(call empath,$(ROM_HEADER_TEMPLATE)) ')
        #----- ROM_SPEC_OPTIONS の指定
        rsf_opt_str = ''.join(rsf_opt)
        codeparam.write('ROM_SPEC_PARAM   = %s\n' % rsf_opt_str)
        #----- TITLE_VERSION_LO の指定
        codeparam.write('TITLE_VERSION_LO   = %d\n' % (romparam[num][rsfkey]['Version']%256))
        #----- TARGET_SUFFIX の指定
        codeparam.write('TARGET_SUFFIX   = %s\n' % ('tad' if romparam[num][rsfkey]['Media'] == 'NAND' else 'srl'))
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