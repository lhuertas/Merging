import sys
import optparse
from subprocess import Popen,PIPE
#import TOTEM_CMS_Combine_Files
from TOTEM_CMS_Combine_Files import *

## Parameters
#totem_dir = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/TotemNtuples/HighBeta/8371/"
#totem_dir = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/TotemNtuples/HighBeta/8369/"
#totem_dir = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/TotemNtuples/HighBeta/8372/"
#totem_dir = "/castor/cern.ch/totem/offline/CMSTOTEM/TotemNtuples/HighBeta/8369/"
#totem_dir = "/castor/cern.ch/totem/offline/CMSTOTEM/TotemNtuples/HighBeta/8371/"
totem_dir = "/castor/cern.ch/totem/offline/CMSTOTEM/TotemNtuples/HighBeta/8372/"
#cms_dir = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/CMSNtuples/HighBeta/LP_ZeroBias_Run2012C-PromptReco-v1-HighBetaJuly2012-Run198902/uaBaseTree-v1-V00-02-00/uaBaseTree-v1-V00-02-00/"
cms_dir = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/CMSNtuples/HighBeta/LP_ZeroBias_Run2012C-PromptReco-v1-HighBetaJuly2012-Run198903/uaBaseTree-v1-V00-02-00/"
type ="root"

#cmsRunNumber = 198902
cmsRunNumber = 198903

orbitOffset = 0
#orbitOffset = 1905

##################################################################
def combine(totem_dir,cms_dir,type):
     from subprocess import call
     files_ = TOTEM_CMS_Combine_Files(totem_dir,cms_dir,type)
     return files_
##########################################################################

#directory where totem ntuples are stored, please put "rfio:directoryName" if it is on CASTOR
#totemDirectory = "rfio:/castor/cern.ch/totem/offline/Analysis/2012/Physics/RP_only,sr+hsx/"
#totemDirectory = "file:" + totem_dir 
totemDirectory = "rfio:" + totem_dir 

#directory where cms ntuples are stored, "rfio:directoryName" if it's on CASTOR
cmsDirectory = "file:" + cms_dir

#the map contains the runs that are going to be merged
#key - merged file name
#value - [totemNtuple, cmsNtuple]
#files = {"8368_ntuple_merged_all.root" : ["8368_ntuple.root", "UABaseTree_CMS-TOTEM_LP_MinBias1_Run2012C-PromptReco-v1-HighBetaJuly2012-Run198901_uaBaseTree-v1_mergedTTree.root"]}
files = combine(totem_dir,cms_dir,type)

#if the output directory is on CASTOR please set outputCastor = True
outputCastor = False
#a place where merged #files will be stored
#outputDirectory = "/castor/cern.ch/user/l/lhuertas/mergeNtuples_8369_198902"
#outputDirectory = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/CMSTotemJobSplitter/test_findOrbitOffset"
outputDirectory = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/CMSTotemJobSplitter/mergeNtuples_8372_198903"

#the path to the compiled program
#compiledProgramPath = "/afs/cern.ch/work/j/jsmajek/CMSTotem_original/Merge/combine"
#compiledProgramPath = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/CMSTotem/Merge/findOrbitOffset"
compiledProgramPath = "/afs/cern.ch/user/l/lhuertas/work/data/CMSTOTEM/CMSTotem/Merge/mergeNtuples"

#the queue which will be used on lxbatch
queueName = "1nw"

print "========================="
print "----> Combine completed."
print "========================="
