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
    destname = ''.join([str(romparam[num][bnrkey]['JP'][0]).replace('/','_'),'.tad'])
    os.rename(srcname,destname)

    srcname  = ''.join(['twl_',romparam[num][rsfkey]['GameCode'],'.srl'])
    destname = ''.join([str(romparam[num][bnrkey]['JP'][0]).replace('/','_'),'.srl'])
    os.rename(srcname,destname)