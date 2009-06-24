#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os, sys, shutil
import yaml,codecs
import binascii

romparampath = sys.argv[1]
romparam = yaml.load(codecs.open(romparampath,'r','utf16'))

#----- 定数
rsfkey    = 'RSF'
bnrkey    = 'Banner'
eulakey   = 'EULA'
ratingkey = 'Rating'

#----- forCLS ディレクトリの生成 ------------------------------------
forclsdir = './forCLS'
try:
    os.mkdir(forclsdir)
except:
    print 'clsdir.py : can not create dir %s' % forclsdir

#----- cls 操作用ディレクトリの生成 ---------------------------------
for num in romparam.keys():
    gamecode = romparam[num][rsfkey]['GameCode']
    tadpath  = ''.join(['./roms/','twl_',gamecode,'.tad'])

    #----- gamecode dir の生成.
    if os.path.exists(tadpath):
        try:
            clsappdir = ''.join([forclsdir,'/',gamecode])
            os.mkdir(clsappdir)
        except:
            print 'clsdir.py : can not create dir %s' % clsappdir
    else:
        print 'clsdir.py : can not create dir %s' % clsappdir
        continue

    #----- version ディレクトリの生成
    verdir = ''.join([clsappdir,'/v',str(romparam[num][rsfkey]['Version'])])
    try:
        os.mkdir(verdir)
    except:
        print 'clsdir.py : can not create dir %s' % verdir
        continue

    #----- tadfile のコピー
    try:
        shutil.copy(tadpath,verdir)
    except (IOError, os.error), why:
        print "Can't copy %s to %s: %s" % (`tadpath`, `verdir`, str(why))
        continue

    #----- TitleID 取得
    tmpId = 0x00030000
    keys = ('AppType','Launch','Media','DataOnly','Secure')
    for shiftbit, key in enumerate(keys):
        if key == 'Launch':
            if not romparam[num][rsfkey][key]:
                tmpId = tmpId + (1<<shiftbit)
        else:
            if romparam[num][rsfkey][key] == True     or \
               romparam[num][rsfkey][key] == 'SYSTEM' or \
               romparam[num][rsfkey][key] == 'NAND' :
                tmpId = tmpId + (1<<shiftbit)
    else:
        TitleId_H = '%08x' % tmpId
        TitleId_L = binascii.hexlify(gamecode)

    #----- properties の生成
    try:
        proppath   = ''.join([clsappdir,'/properties'])
        properties = open(proppath,'w')
    except:
        print 'clsdir.py : can not open %s file' % proppath
        continue
    else:
        properties.write('TITLE_ID\t\t\t%s%s\n' % (TitleId_H, TitleId_L))
        properties.write('TITLE_NAME\t\t\t%s\n' % romparam[num][rsfkey]['TitleName'])
        properties.write('TITLE_TYPE\t\t\t%s\n' % romparam[num][rsfkey]['TitleType'])
        properties.write('PASSWORD\t\t\t%s\n'   % romparam[num][rsfkey]['TitleName'])
        properties.write('ALLOW_COMMON_TICKET\t%s\n' % ('1' if romparam[num][rsfkey]['eTicket'] == 'common' else '0'))
    finally:
        properties.close()
