#!/usr/bin/python3

for i in range(1,11):
    name = "time" + str(i*100) + ".dat"
    x = open(name)
    xl = x.readlines()
    xll = [ w.splitlines() for w in xl]
    ss = 0.0
    for j in xll:
        w = j[0].split(' ')
        ss += float(w[2])

    ss = ss / len(xll)

    print(i*100, ss)
