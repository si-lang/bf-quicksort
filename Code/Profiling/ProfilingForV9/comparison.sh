#!/bin/bash

# Idea:
# Run each algorithm against each other and alter the input type

pre="partition_compare_unroll16_elem10-30_rep50n10_medo3_"
suf=".csv"
compile="g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb"

# Invariant: Assumes minElemFac = 4, maxElemFac = 9, and uint64_t are selected from the beginning!
sed -i -e "s#base = 10;#base = 2;#g" PartitionProfile.cpp
sed -i -e "s#step = 1;#step = 2;#g" PartitionProfile.cpp
sed -i -e "s#minElemFac = 4;#minElemFac = 10;#g" PartitionProfile.cpp
sed -i -e "s#maxElemFac = 9;#maxElemFac = 30;#g" PartitionProfile.cpp


old="typedef uint64_t"
new="typedef uint32_t"
name="uint32"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="typedef uint64_t"
name="uint64"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="typedef Record<32>"
name="record32"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="typedef Record<64>"
name="record64"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="typedef Record<84>"
name="record84"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="typedef Record<128>"
name="record128"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"


# Reset to invariant
old=$new
new="typedef uint64_t"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp

sed -i -e "s#maxElemFac = 30;#maxElemFac = 9;#g" PartitionProfile.cpp
sed -i -e "s#minElemFac = 10;#minElemFac = 4;#g" PartitionProfile.cpp
sed -i -e "s#step = 2;#step = 1;#g" PartitionProfile.cpp
sed -i -e "s#base = 2;#base = 10;#g" PartitionProfile.cpp
