#!/usr/bin/python3
import random
import sys

M = int(sys.argv[1])
N = int(sys.argv[2])
S = int(sys.argv[3])

a = [[ random.randint(            1,         25 ) for i in range(0, N) ] for j in range(0, M)]
b = [  random.randint( int(N*0.8*S),int(N*1.4*S)) for i in range(0, M) ]
c = [  random.randint(            1,         25 ) for i in range(0, N) ]



for i in b:
    print(i, end=" ")

print("")



for i in c:
    print(i, end=" ")

print("")



for i in a:
    for j in i:
        print(j, end=" ")

    print("")


