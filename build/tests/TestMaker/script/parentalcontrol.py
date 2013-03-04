#!/usr/bin/env python
# -*- coding:utf-8 -*-

import sys, yaml, codecs


target   = sys.argv[1]
yamlpath = sys.argv[2]
#target   = '../config/twl_parentalcontrol_spec.yaml'
#yamlpath = '../romparam.yaml'
ognparam = yaml.load(codecs.open(target,'r','utf16'))

#--- romparam.yaml の生成
try:
    yamlfile = codecs.open(yamlpath, 'w', 'utf16')
except:
    print '%s : can not open %s file' % ('hoge', yamlpath)
else:
    #--- 定数値
    version   = '0x0000'
    apptype   = 'USER'
    launch    = 'TRUE'
    media     = 'NAND'
    dataonly  = 'FALSE'
    secure    = 'FALSE'
    etikect    = 'personalized'
    titlename = ''
    titletype = 'TWL_APP'
    filename  = 'banner.bnr'
    bnrver    = '3'
    codes = map(chr,range(ord('a'),ord('z')+1))

    index = 0
    for ogn in sorted(ognparam.keys()):
        #--- CardRegion の決定
        region = ognparam[ogn][0]
        for age in ognparam[ogn][1]:
            #--- gamecode, bnrtitle の決定
            gamecode = ''.join(['4',codes[index/26],codes[(index%26)],'A'])
            bnrline1 = ''.join([ogn,'/',str(age)])
            #--- index
            yamlfile.write('#-- ROM No.%d ------------------------\n' % index)
            yamlfile.write('%d:\n' % index)
            #--- rsf & properties parameter
            yamlfile.write('  RSF :\n')
            yamlfile.write('    GameCode   : %s\n' % gamecode  )
            yamlfile.write('    Version    : %s\n' % version   )
            yamlfile.write('    AppType    : %s\n' % apptype   )
            yamlfile.write('    Launch     : %s\n' % launch    )
            yamlfile.write('    Media      : %s\n' % media     )
            yamlfile.write('    DataOnly   : %s\n' % dataonly  )
            yamlfile.write('    Secure     : %s\n' % secure    )
            yamlfile.write('    CardRegion : %s\n' % region    )
            yamlfile.write('    TitleName  : %s\n' % titlename )
            yamlfile.write('    TitleType  : %s\n' % titletype )
            yamlfile.write('    eTicket    : %s\n' % etikect   )
            #--- bsf parameter
            yamlfile.write('  Banner :\n')
            yamlfile.write('    File       : %s\n' % filename  )
            yamlfile.write('    Version    : %s\n' % bnrver    )
            for language in ('JP','EN','FR','GE','IT','SP','CN','HN'):
                bnrline2 = ''.join(['No.',str(index),' Lang:',language])
                yamlfile.write("    %s         : %s" % (language,str(yaml.dump([bnrline1,bnrline2]))))
            #--- Rating
            yamlfile.write('  Rating :\n')
            yamlfile.write('    Ogn        : %s\n' % ogn       )
            yamlfile.write('    Age        : %s\n' % age       )
            yamlfile.write('\n\n')
            index = index + 1
finally:
    yamlfile.close()