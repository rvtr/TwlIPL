#!/usr/bin/env python
# -*- coding:utf-8 -*-

import sys, codecs
import yaml, pyExcelerator

target     = sys.argv[1]
yamlpath   = sys.argv[2]
reverseflg = sys.argv[3]
#target   = '../doc/_20080617_DS_fontcodeList_NOE_FIANL.xls'
#yamlpath = '../romparam.yaml'
sheets = pyExcelerator.parse_xls(target)
for (sheetName, sheet) in sheets:
    #--- '全世界向け本体_内蔵フォントコード一覧' というタイトル名決め打ち
    if sheetName == u'全世界向け本体_内蔵フォントコード一覧':
        keys = sorted(sheet.keys())
        fontcodes = [sheet[key] for key in keys if key[0] > 2 and key[1] == 1][:-29]
        fonts     = [sheet[key][1] for key in keys if key[0] > 2 and key[1] == 2][:-29]

        #--- ROM の作成順序の変更
        if reverseflg:
            fontcodes.reverse()
            fonts.reverse()

        #--- romparam.yaml の生成
        try:
            yamlfile = codecs.open(yamlpath, 'w', 'utf16')
        except:
            print 'genparam.py : can not open %s file' % yamlpath
            continue
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
            for index, font in enumerate(fonts):
                #--- gamecode, bnrtitle の決定
                gamecode = ''.join(['4',codes[index/26],codes[(index%26)],'A'])
                #--- 0x0060 だけ特別対応　Excel データが存在しないため（img 貼付）
                if fontcodes[index] == '0x0060':
                    bnrline1 = repr(''.join(['\x60_',fontcodes[index]]))[2:-1]
                else:
                    bnrline1 = repr(''.join([font,'_',fontcodes[index]]))[2:-1]
                #--- index
                yamlfile.write('#-- ROM No.%d ------------------------\n' % index)
                yamlfile.write('%d:\n' % index)
                #--- rsf & properties parameter
                yamlfile.write('  RSF :\n')
                yamlfile.write('    GameCode  : %s\n' % gamecode  )
                yamlfile.write('    Version   : %s\n' % version   )
                #yamlfile.write('    AppType   : %s\n' % apptype   )
                #yamlfile.write('    Launch    : %s\n' % launch    )
                yamlfile.write('    Media     : %s\n' % media     )
                #yamlfile.write('    DataOnly  : %s\n' % dataonly  )
                #yamlfile.write('    Secure    : %s\n' % secure    )
                yamlfile.write('    TitleName : %s\n' % titlename )
                yamlfile.write('    TitleType : %s\n' % titletype )
                yamlfile.write('    eTicket    : %s\n' % etikect   )
                #--- bsf parameter
                yamlfile.write('  Banner :\n')
                yamlfile.write('    File      : %s\n' % filename  )
                yamlfile.write('    Version   : %s\n' % bnrver    )
                for language in ('JP','EN','FR','GE','IT','SP','CN','HN'):
                    bnrline2 = ''.join(['No.',str(index),' Lang:',language])
                    if font == '"':
                        yamlfile.write("    %s        : ['%s', '%s']\n" % (language, bnrline1, bnrline2))
                    else:
                        yamlfile.write('    %s        : ["%s", "%s"]\n' % (language, bnrline1, bnrline2))
                yamlfile.write('\n\n')
        finally:
            yamlfile.close()