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
    
def createPatch(folder, options):
    patchtool = ".\\courgette.exe"
    cmd = patchtool + " -gen " 
    cmd +=options.OldBuildDir + "\\" + folder + "\\splayer.exe "
    cmd +=options.latest + " " + options.outputdir + "\\" + folder + ".patch"
    print cmd
    retcode = subprocess.call(cmd)
    return retcode

def main(options, args): 
    latestMD5 = filemd5(options.latest)
    print latestMD5
    revisions = []
    folders = [f for f in os.listdir(options.OldBuildDir) if os.path.isdir(os.path.join(options.OldBuildDir, f))]
    for x in folders: 
        revisions.append(int(x))
    revisions.sort()
    revisions.reverse()
    foldercount = options.Count
    if (len(revisions)<foldercount):
        foldercount = len(revisions)
    if not os.path.exists(options.outputdir):
        os.mkdir(options.outputdir)
    descfile = open(options.outputdir + "\\desc.txt", "wa")
    for index in range(foldercount):
        createPatch(str(revisions[index]), options)
        folder = str(revisions[index])
        md51 = filemd5(options.OldBuildDir + "\\" + folder + "\\splayer.exe")
        try:
            descfile.write(md51)
            descfile.write(", ")
            descfile.write(folder + ".patch")
            descfile.write("\n")
        except Exception,e:
            print e   
    descfile.close();
    return 0

if '__main__' == __name__:
    option_parser = optparse.OptionParser()

    option_parser.add_option('', '--latest', default='.\\splayer.exe',
        help='path to latest release')
    option_parser.add_option('', '--OldBuildDir', default='.\\archieve',
                             help='path to the old builds directory')
    option_parser.add_option('', '--Count', default=10,
                             help='How many patches to be created')
    option_parser.add_option('', '--outputdir', default='.\\output',
                             help='output path')

    options, args = option_parser.parse_args()
    sys.exit(main(options, args)) 