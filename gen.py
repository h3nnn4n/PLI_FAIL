#!/usr/bin/python3
import random
import sys

M = int(sys.argv[1])
N = int(sys.argv[2])
X = int(sys.argv[3])

a = [[ random.randint( 1,25  )   for i in range(0, N) ] for j in range(0, M)]
b = [  random.randint( 1,25*X)   for i in range(0, M) ]
c = [  random.randint( 1,25  )   for i in range(0, N) ]

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


