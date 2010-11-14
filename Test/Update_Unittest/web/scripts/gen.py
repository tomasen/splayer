#!/usr/bin/python 
#create patch files for ShooterPlayer
#
#
#

import glob
import optparse
import os
import re
import shutil
import sys 
import hashlib
import subprocess 
import gzip
import shutil

def createGzipFile(inname, outname):
    f_in = open(inname, 'rb')
    f_out = gzip.open(outname, 'wb')
    f_out.writelines(f_in)
    f_out.close()
    f_in.close()

def filemd5(fileName):
    m = hashlib.md5()
    try:
        fd = open(fileName,"rb")
    except IOError:
        print "Unable to open the file in readmode:", filename
        return
    content = fd.read()
    fd.close()
    m.update(content)
    return m.hexdigest()
    
def main(options, args): 
    latestMD5 = filemd5(options.latest)
    gzfile = options.outputdir + "\\out.gz"
    if not os.path.exists(options.outputdir):
        os.mkdir(options.outputdir)
#    createGzipFile(options.latest, gzfile)
    createGzipFile(options.latest, "out.gz")
    shutil.move("out.gz", gzfile)
    gzmd5 = filemd5(gzfile)
    latestszie = os.path.getsize(options.latest)
    gzszie = os.path.getsize(gzfile)
    descfile = open(options.outputdir + "\\desc.txt", "wa")
    ##PATH;         MD5 of uncompressed;             ID; TEMP PATH for unzipped file;    MD5 of compressed;               METHOD; LENGTH; GZ LENGTH
    descfile.write(options.latest)
    descfile.write(";")
    descfile.write(latestMD5)
    descfile.write(";1;")
    #TEMP PATH for unzipped file
    m = hashlib.md5()
    m.update(options.latest)
    descfile.write(m.hexdigest())
    descfile.write(";")
    descfile.write(gzmd5)
    descfile.write(";default;")
    descfile.write(str(latestszie))
    descfile.write(";")
    descfile.write(str(gzszie))
    descfile.write(";")    
    descfile.write("\n")
    descfile.close();
    return 0

if '__main__' == __name__:
    option_parser = optparse.OptionParser()
    option_parser.add_option('', '--latest', default='splayer.exe',
        help='path to latest release')
    option_parser.add_option('', '--outputdir', default='.\\output',
                             help='output path')
    options, args = option_parser.parse_args()
    sys.exit(main(options, args)) 