import glob
import os
import re
import gzip
import numpy as np

results = glob.glob("./runs/LIRS/*.stats.gz")
print results

methods = ['LRU', 'random', 'FIFO', 'LIRS']

cycles = []
names = []

for i in range(len(results)):
    name = results[i][12:-9]
    names.append(name)
    print name

    row = []
    for method in methods:
        cycle = None
        print "runs/" + method + "/" + name + ".stats.gz"

        with gzip.open("runs/" + method + "/" + name + ".stats.gz", "r") as f:
            flag = False
            while True:
                line = f.readline()
                if line == None or len(line) == 0:
                    break
                line = line.strip().split(" ")
                if len(line) > 6 and line[-5] == "Cycles:" and line[-6] == "Global":
                    cycle = int(line[-4])
                    break
                    #print line[-4]

                """
                if line.strip() == "Per Thread Demand Reference Statistics:":
                    flag = True
                    continue
                line = line.strip().split(" ")
                if flag:
                    miss_rate = float(line[-1])
                    break
                """
        print method, ": ", cycle
        row.append(cycle)
    cycles.append(row)

cycles = np.asarray(cycles)
print cycles

cut = 5
begin = 0
end = begin + cut
while True:
    print "\\hline"
    row = "methods";
    for i in range(begin, end):
        row += " & " + names[i]
    print row + "\\\\"
    print "\\hline"

    for i in range(len(methods)):
        row = methods[i] + " & "
        for j in range(begin, end):
            flag = True
            for k in range(len(methods)):
                if cycles[j][k] < cycles[j][i] - 1e-20:
                    flag = False
                    break 
            if flag:
                row = row + "\\textbf{" + str(cycles[j][i]) + "}"
            else:
                row = row + str(cycles[j][i]) 
            if j == end - 1:
                row = row + "\\\\"
            else:
                row = row + " & "
        print row
        print "\\hline"
    begin = end
    if begin >= len(cycles):
        break
    end = min(len(cycles), begin + cut)





