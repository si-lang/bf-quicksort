#!/bin/bash

typ=$1
prefix="#define MYBLOCKSIZE"


# Invariant: Assumes blocksize 512 is selected at the begining
old=512
new=32
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


old=$new
new=64
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


old=$new
new=128
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


old=$new
new=256
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


old=$new
new=512
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


old=$new
new=768
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


old=$new
new=1024
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


old=$new
new=1280
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


old=$new
new=2048
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native PartitionProfile.cpp -o partitionProfile -L./../TimeAndProfile -ljevents -ltbb
./partitionProfile | tee "partition_block""$new""_unroll16_elem4-7_rep50n10_""$typ"".csv"


# Restore invariant
old=$new
new=512
sed -i -e "s/$prefix $old/$prefix $new/g" ../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp

