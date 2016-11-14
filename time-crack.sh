#!/bin/bash

make

# # Small tests
# data_files="pass-files/encrypted-5-2-from-10.txt dict-files/english-10.txt dict-files/english-10.txt"

# # Small tests
# data_files="pass-files/encrypted-5-1-from-1K.txt dict-files/english-1K.txt"

# Medium tests
data_files="pass-files/encrypted-5-2-from-100.txt dict-files/english-100.txt dict-files/english-100.txt"


# # Larger timing tests
# data_files="pass-files/encrypted-5-2-from-1K.txt dict-files/english-1K.txt dict-files/english-1K.txt"

num_threads="2 4 6 8"

echo Running with data files $data_files

# serial
echo Serial code
time -p passcrack $data_files > crack.serial.out
echo

# omp parallel versions
for nt in $num_threads; do
    export PASSCRACK_NUMTHREADS=$nt
    echo OMP nthreads $nt
    time -p omp_passcrack $data_files > crack.out
    if [[ `diff -q crack.out crack.serial.out` ]]; then diff -q crack.out crack.serial.out; diff -y crack.out crack.serial.out; fi
    echo
done


#pthreads parallel versions
for nt in $num_threads; do
   export PASSCRACK_NUMTHREADS=$nt
    echo PTHREADS nthreads $nt
    time -p pthread_passcrack $data_files > crack.out
    if [[ `diff -q crack.out crack.serial.out` ]]; then diff -q crack.out crack.serial.out; diff -y crack.out crack.serial.out; fi
    echo
done

