from setup import *
import os, stat

#changes the rights for file (for scripts)
def chmod(f):
    os.chmod(f, stat.S_IRWXU | stat.S_IRGRP | stat.S_IROTH)

#create a script that:
# - sets the gcc and root environment
# - runs orbit offset searching or merging
# - copies the output files from pool to output directory     
def createJobFile(totem, cms, output):
    output = output.rstrip("root")
    output = output.rstrip('.')
    stdOutName = output + "_out"
    fileName = output + "_job.sh"
    jobFile = open(fileName, "w")
    #jobFile.write("source /afs/cern.ch/sw/lcg/external/gcc/4.4/x86_64-slc5-gcc44-opt/setup.sh\n")
    #jobFile.write("source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.28.00b/x86_64-slc5-gcc44-opt/root/bin/thisroot.sh\n")
    jobFile.write("source /afs/cern.ch/sw/lcg/external/gcc/4.4.3/x86_64-slc5-gcc43-opt/setup.sh\n")
    #jobFile.write("source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.28.00b/x86_64-slc5-gcc43-opt/root/bin/thisroot.sh\n")
    jobFile.write("source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.26.00a/x86_64-slc5-gcc43-opt/root/bin/thisroot.sh\n")
    chmod(fileName)
    totemFilePath = totemDirectory + totem
    cmsFilePath = cmsDirectory + cms
    output = output + ".root"
    runJob = None
    if compiledProgramPath.split('/')[-1] == 'mergeNtuples':
	runJob = compiledProgramPath + " " + totemFilePath + " " + cmsFilePath + " " + output + " " + str(cmsRunNumber) + " " + str(orbitOffset) + " > " + stdOutName + "\n"
    else:
	runJob = compiledProgramPath + " " + totemFilePath + " " + cmsFilePath + " " + output + " > " + stdOutName + "\n"
    jobFile.write(runJob)
    cp = ""
    if outputCastor == True:
        cp = "rfcp"
    else:
        cp = "cp"
    copyRootFile = cp + " " + output + " " + outputDirectory + "\n"
    jobFile.write(copyRootFile)
    copyStdOutFile = cp + " " + stdOutName + " " + outputDirectory + "\n"
    jobFile.write(copyStdOutFile)
    jobFile.close()
    return fileName

#create directory for files
directory = "submit"
if not os.path.exists(directory):
    os.makedirs(directory)
os.chdir(directory)
submitFile = open("submit.sh", "w")  
chmod("submit.sh")

#create scripts for every pair of ntuples and then create one script that will submit the jobs on lxbatch
for output, inputFiles in files.iteritems():
    totem = inputFiles[0]
    cms = inputFiles[1]
    jobFileName = createJobFile(totem, cms, output)
    jobName = jobFileName.rstrip("sh")
    jobName = jobName.rstrip('.')
    submitJobFile = "bsub -J " + jobName + " -R \"swp>2000&&pool>40000\" -q " + queueName + " " + jobFileName + "\n"
    submitFile.write(submitJobFile)
    
submitFile.close()
