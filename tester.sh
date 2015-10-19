#!/bin/bash

NP=10

for j in `seq 100 100 1000`
do
    echo $j$

    param="'-DN="${j}"'"

    make CFLAGS=$param

    rm time${j}.dat 2> /dev/null

    for i in $(seq 1 ${NP})
    do
        echo -n .
        echo -n "$i " >> time${j}.dat
        ./gen.py 1 ${j} 3 > in_tmp && ./main < in_tmp 2> /dev/null >> time${j}.dat
    done

    echo 
done
