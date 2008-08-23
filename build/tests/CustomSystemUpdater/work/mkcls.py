#!/usr/bin/env python
# -*- coding:utf-8 -*-

import sys, os, struct
import string, random
import re, shutil, binascii

def roundup(val, align):
    return int((val+align-1)/align)*align

def randstr(n):
    words = ''.join([string.digits,string.letters,'.','+'])
    return ''.join(random.choice(words) for i in xrange(n))

def main():
    try:
        tadfile = sys.argv[1]
    except:
        print '%s : please specfy the tad file' % (sys.argv[0])
        exit(-1)

    #--- parse *.tad
    fin = open(tadfile, 'rb')
    try:
        tadData = fin.read()
    except:
        fin.close()
    start, end = 0, struct.calcsize('>l2sh6l')
    (hdrSize, tadType, tadVersion, certSize, crlSize, ticketSize, tmdSize,
     contentSize, metaSize) = struct.unpack('>l2sh6l', tadData[start:end])
    certOffset    = roundup(hdrSize, 64)
    crlOffset     = roundup(certOffset    + certSize,    64)
    ticketOffset  = roundup(crlOffset     + crlSize,     64)
    tmdOffset     = roundup(ticketOffset  + ticketSize,  64)
    contentOffset = roundup(tmdOffset     + tmdSize,     64)
    metaOffset    = roundup(contentOffset + contentSize, 64)
    fileSize      = roundup(metaOffset    + metaSize,    64)

    #print  '-'*20+'\n'
    #print  ' hdrSize        %s\n' % hdrSize,
    #print  ' tadType        %s\n' % tadType,
    #print  ' tadVersion,    %s\n' % tadVersion,
    #print  ' certSize,      %s\n' % certSize,
    #print  ' crlSize,       %s\n' % crlSize,
    #print  ' ticketSize,    %s\n' % ticketSize,
    #print  ' tmdSize,       %s\n' % tmdSize,
    #print  ' contentSize,   %s\n' % contentSize,
    #print  ' metaSize       %s'
    #print  '\n'+'-'*20
    #open("cert.bin",    'wb').write(tadData[certOffset:certOffset+certSize])
    #open("crl.bin",     'wb').write(tadData[crlOffset:crlOffset+crlSize])
    #open("ticket.bin",  'wb').write(tadData[ticketOffset:ticketOffset+ticketSize])
    #open("tmd.bin",     'wb').write(tadData[tmdOffset:tmdOffset+tmdSize])
    #open("content.bin", 'wb').write(tadData[contentOffset:contentOffset+contentSize])
    #open("meta.bin",    'wb').write(tadData[metaOffset:metaOffset+metaSize])

    #--- parse TMD
    tmd = tadData[tmdOffset:tmdOffset+tmdSize]

    #--- RSA2048 signature
    (sigB, sigE) = (0, struct.calcsize('>i256s60s64s'))
    (IOSCCertSigType, IOSCRsaSig2048, IOSCSigDummy, IOSCName)\
     = struct.unpack('>i256s60s64s', tmd[sigB:sigE])

    #print "=== RSA2048 signature ==============="
    #print "IOSCCertSigType : %x" % IOSCCertSigType
    #print "IOSCRsaSig2048  : %s" % IOSCRsaSig2048
    #print "IOSCSigDummy    : %s" % IOSCSigDummy
    #print "IOSCName        : %s" % IOSCName

    #--- ESTitleMetaHeader
    (tmhB, tmhE) = (sigE, sigE+struct.calcsize('>4B5iH62si3H'))
    (version,
     caCrlVersion,
     signerCrlVersion,
     dummy,
     sysVersion_H,
     sysVersion_L,
     titleId_H,
     titleId_L,
     titleType,
     groupID,
     reserved,
     accessRights,
     titleVersion,
     numContents,
     bootIndex) = struct.unpack('>4B5iH62si3H',tmd[tmhB:tmhE])

    print "=== ESTitleMetaHeader ==============="
    #print ('version          : %d ') % version
    #print ('caCrlVersion     : %d ') % caCrlVersion
    #print ('signerCrlVersion : %d ') % signerCrlVersion
    #print ('dummy            : %s ') % dummy
    #print ('sysVersion_H     : %x ') % sysVersion_H
    #print ('sysVersion_L     : %x ') % sysVersion_L
    print ('titleId_H        : %08x ') % titleId_H
    print ('titleId_L        : %08x ') % titleId_L
    #print ('titleType        : %x ') % titleType
    print ('groupID          : %x ') % groupID
    #print ('reserved[62]     : %s ') % reserved
    #print ('accessRights     : %d ') % accessRights
    print ('titleVersion     : %d ') % titleVersion
    #print ('numContents      : %d ') % numContents
    #print ('bootIndex        : %d ') % bootIndex

    #--- make directory for cls
    tadpath  = os.path.dirname(tadfile)
    gamecode = binascii.unhexlify('%x'%titleId_L)
    #clsdir  = ''.join([tadpath,'/',gamecode])
    clsdir  = ''.join(['./',gamecode])
    try:
        os.mkdir(clsdir)
    except:
        print '%s : can not create dir %s' % (sys.argv[0],clsdir)

    #--- write properties
    properties = open(''.join([clsdir,'/properties']),'w')
    properties.write('TITLE_ID\t\t\t\t%08x%08x\n'  % (titleId_H, titleId_L))
    properties.write('TITLE_NAME\t\t\t\t%s\n'      % gamecode)
    properties.write('TITLE_TYPE\t\t\t\t%s\n'      % 'TWL_SYS')
    properties.write('PASSWORD\t\t\t\t%s\n'        % randstr(64))
    properties.write('ALLOW_COMMON_TICKET\t\t%s\n' % '1')

    #--- make version dir
    verdir = ''.join([clsdir,'/v',str(titleVersion)])
    try:
        os.mkdir(verdir)
    except:
        print '%s : can not create dir %s' % (sys.argv[0],verdir)
    else:
        shutil.copy(tadfile,verdir)

if __name__ == '__main__':
    sys.exit(main() or 0)