#!/bin/bash

sed -i -e "s#maxElemFac = 9;#maxElemFac = 7;#g" PartitionProfile.cpp
# Invariant: Assumes uint64_t is uncommented from the beginning!
old="typedef uint64_t"
new="typedef uint32_t"
name="uint32"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
bash -x loopForBlockSizes.sh "$name"

old=$new
new="typedef uint64_t"
name="uint64"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
bash -x loopForBlockSizes.sh "$name"

sed -i -e "s#maxElemFac = 7;#maxElemFac = 5;#g" PartitionProfile.cpp
old=$new
new="typedef Record<16>"
name="record16"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
bash -x loopForBlockSizes.sh "$name"

old=$new
new="typedef Record<32>"
name="record32"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
bash -x loopForBlockSizes.sh "$name"

old=$new
new="typedef Record<64>"
name="record64"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
bash -x loopForBlockSizes.sh "$name"

if [ 1 -eq 0 ]; then
old=$new
new="typedef Record<84>"
name="record84"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
bash -x loopForBlockSizes.sh "$name"
fi

old=$new
new="typedef Record<128>"
name="record128"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
bash -x loopForBlockSizes.sh "$name"


# Reset to invariant
sed -i -e "s#maxElemFac = 7;#maxElemFac = 9;#g" PartitionProfile.cpp
old=$new
new="typedef uint64_t"
sed -i -e "s#$old#//$old#g" PartitionProfile.cpp
sed -i -e "s#//$new#$new#g" PartitionProfile.cpp
