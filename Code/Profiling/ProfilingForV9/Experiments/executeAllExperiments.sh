#!/bin/bash

# Block Size:
g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native blockSizes.cpp -o blockSizes -L../../TimeAndProfile -ljevents -ltbb &&
./blockSizes | tee blockSizesData.csv

# Partition Comparison:
g++-7 -std=c++14 -pthread -lm -lc -lgcc_s -lgcc -O3 -march=native partitionComparison.cpp -o partitionComparison -L../../TimeAndProfile -ljevents -ltbb &&
./partitionComparison | tee partitionComparisonData.csv
