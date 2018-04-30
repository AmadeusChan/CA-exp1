import glob
import os

rep_option = 0 

allfiles = glob.glob("traces/*.out.trace.gz")

for i in range(len(allfiles)):
    print "\n\n"
    print "***********************************************************************"
    print "***** testing ", i, "/", len(allfiles), ": ", allfiles[i], "******"
    print "***********************************************************************"

    """
    rep_option = 0
    stats = "runs/" + str(rep_option) + "/" + allfiles[i][7:-13] + ".stats"
    command = "./CRC/bin/CMPsim.usetrace.64 -threads 1 -t " + allfiles[i] + " -o " + stats + " -cache UL3:512:64:8 -LLCrepl " + str(rep_option)
    print command
    os.system(command)

    rep_option = 1
    stats = "runs/" + str(rep_option) + "/" + allfiles[i][7:-13] + ".stats"
    command = "./CRC/bin/CMPsim.usetrace.64 -threads 1 -t " + allfiles[i] + " -o " + stats + " -cache UL3:512:64:8 -LLCrepl " + str(rep_option)
    print command
    os.system(command)
    """

    rep_option = 2
    stats = "runs/" + str(rep_option) + "/" + allfiles[i][7:-13] + ".stats"
    command = "./CRC/bin/CMPsim.usetrace.64 -threads 1 -t " + allfiles[i] + " -o " + stats + " -cache UL3:512:64:8 -LLCrepl " + str(rep_option)
    print command
    os.system(command)

