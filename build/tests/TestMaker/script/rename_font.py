#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os, sys, yaml, codecs

#yamlpath = sys.argv[1]
yamlpath = '../romparam.yaml'
romparam = yaml.load(codecs.open(yamlpath,'r','utf16'))

#----- 定数
rsfkey    = 'RSF'
bnrkey    = 'Banner'
eulakey   = 'EULA'
ratingkey = 'Rating'

os.chdir('../roms')

for num in romparam.keys():
    srcname  = ''.join(['twl_',romparam[num][rsfkey]['GameCode'],'.tad'])
    #destname = ''.join([('No.%03d_' % num),
    #                    romparam[num][bnrkey]['JP'][0].split('_')[1],
    #                    '.tad'])
    destname = ''.join([('No.%03d_' % num),
                        romparam[num][bnrkey]['JP'][1].split('_')[0],
                        '_L2.tad'])

    try:
        os.rename(srcname,destname)
    except:
        print srcname, destname