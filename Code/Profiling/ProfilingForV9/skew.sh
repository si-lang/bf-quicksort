#!/bin/bash

pre="partition_"
suf="_unroll16_elem7-8_rep50n10_uint32.csv"
compile="g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb"

# Invariant: Assumes skew = 2, minElemFac = 4, maxElemFac = 9, and uint64_t are selected from the beginning!
old="typedef uint64_t"
new="typedef uint32_t"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
sed -i -e "s#minElemFac = 4;#minElemFac = 7;#g" PartitionProfile.cpp
sed -i -e "s#maxElemFac = 9;#maxElemFac = 8;#g" PartitionProfile.cpp

old="skew = 2;"
new="skew = 2;"
name="skew2"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="skew = 4;"
name="skew4"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="skew = 6;"
name="skew6"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="skew = 8;"
name="skew8"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="skew = 10;"
name="skew10"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="skew = 12;"
name="skew12"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="skew = 14;"
name="skew14"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"

old=$new
new="skew = 16;"
name="skew16"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp
$compile && ./partitionProfile | tee "$pre""$name""$suf"


# Reset to invariant
old=$new
new="skew = 2;"
sed -i -e "s#$old#$new#g" PartitionProfile.cpp

sed -i -e "s#maxElemFac = 8;#maxElemFac = 9;#g" PartitionProfile.cpp
sed -i -e "s#minElemFac = 7;#minElemFac = 4;#g" PartitionProfile.cpp
old="typedef uint32_t"
new="typedef uint64_t"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
