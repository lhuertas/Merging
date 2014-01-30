#! /usr/bin/env python

import os, sys
import optparse
from subprocess import Popen,PIPE
###################################
def listFilesInCastor(castor_dir,type):
    p1 = Popen(['nsls',castor_dir],stdout=PIPE)
    p2 = Popen(['grep',type],stdin=p1.stdout,stdout=PIPE)
# totemfiles = [castor_dir + "/" + item[:-1] for item in p2.stdout]
    totemfiles = [item[:-1] for item in p2.stdout]
    p2.stdout.close()
    return totemfiles


def listFiles(local_dir,type):

    cmsfiles = os.listdir(local_dir)
   # cmsfiles = [ "%s%s" % (local_dir,item) for item in cmsfiles if item.find(type) != -1 ]
    cmsfiles = [ "%s" % (item) for item in cmsfiles if item.find(type) != -1 ]
    return cmsfiles



def TOTEM_CMS_Combine_Files(totem_dir,cms_dir,type):
    from subprocess import call
    f={}
    #mergeName=[]
    totemfiles = listFilesInCastor(totem_dir,type)
    #totemfiles = listFiles(totem_dir,type)
    cmsfiles = listFiles(cms_dir,type)
    totemfiles = [item for item in totemfiles]
    print "##################################"
    print "# Combine TOTEM and CMS tuples"
    print "N of TOTEM Files: ",len(totemfiles)
    print "N of CMS Files: ", len(cmsfiles)
    print "##################################"
    cmsfiles = [item for item in cmsfiles]
   
# print "####################################"
    for indexTOTEM in range(len(totemfiles)):
        for indexCMS in range(len(cmsfiles)):
          
             mergeName = "%s_%s" %(totemfiles[indexTOTEM].rstrip(".root"),cmsfiles[indexCMS])
             f[mergeName] =(totemfiles[indexTOTEM],cmsfiles[indexCMS])
    
    print "N of Merge Files:", len(f)
    print "####################################"
    return f
    

if __name__ == '__main__':
    parser = optparse.OptionParser(usage="usage: %prog [options]")
    parser.add_option("-c","--castorPath", dest="castorPath", metavar="CASTORPATH", help="Castor Path")
    parser.add_option("-l","--localPath", dest="localPath", metavar="LOCALPATH", help="local path")
    parser.add_option("-t","--type", dest="type", default="root", metavar="TYPE", help="select only totemfiles with substring TYPE (Default: 'root')")

    (input, args) = parser.parse_args()

    if not input.castorPath: parser.error('must set directory option')
    if not input.localPath: parser.error('must set directory option')

    f = TOTEM_CMS_Combine_Files(castor_dir = input.castorPath,
                                      local_dir = input.localPath,
                                      type = input.type)

    print "N of Merge Files:", len(f)
    print "####################################"
    #for i in f: print f
 
    sys.exit(f)
    #sys.exit()




