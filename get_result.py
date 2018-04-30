import glob
import os
import re
import gzip
import numpy as np

results = glob.glob("./runs/2/*.stats.gz")
print results

methods = ['0', '1', '2']

miss_rates = []
names = []

for i in range(len(results)):
    name = results[i][9:-9]
    names.append(name)
    print name

    row = []
    for method in methods:
        miss_rate = None
        #print "runs/" + method + "/" + name + ".stats.gz"
        with gzip.open("runs/" + method + "/" + name + ".stats.gz", "r") as f:
            flag = False
            while True:
                line = f.readline()
                if line == None or len(line) == 0:
                    break
                if line.strip() == "Per Thread Demand Reference Statistics:":
                    flag = True
                    continue
                line = line.strip().split(" ")
                if flag:
                    miss_rate = float(line[-1])
                    break
        print method, ": ", miss_rate
        row.append(miss_rate)
    miss_rates.append(row)

miss_rates = np.asarray(miss_rates)
print miss_rates

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
                if miss_rates[j][k] < miss_rates[j][i] - 1e-20:
                    flag = False
                    break 
            if flag:
                row = row + "\\textbf{" + str(miss_rates[j][i]) + "}"
            else:
                row = row + str(miss_rates[j][i]) 
            if j == end - 1:
                row = row + "\\\\"
            else:
                row = row + " & "
        print row
        print "\\hline"
    begin = end
    if begin >= len(miss_rates):
        break
    end = min(len(miss_rates), begin + cut)





