#include <random>
#include <math.h> // pow
#include "tbb/parallel_sort.h" // Exact pivot

#include "../../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
#include "../../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"
#include "../../../QuickSort/ParallelQuicksort/v3/parallelPartition.cpp"
#include "../../TimeAndProfile/profile.hpp"
#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../EdelkampWeiss/partition.h"
#include "../../EdelkampWeiss/median.h"


template<typename T>
void executeExperiment(std::string typeName) {
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 2
	short step = 2; // 2
  short minElemFac = 4; // 4
	short maxElemFac = 16; // 16
  int numberOfIterations = 15; // 15
	uint64_t seed = 4264918403728822297; // Choose seed

	//
	// Execution Loop
	//
	PerfEvents e;
	std::string name = "";
	for (uint32_t i=minElemFac; i<=maxElemFac; i+=step) {
		for (uint32_t j=0; j<numberOfIterations; j++) {
			//
			// Generate Inputs
			//
			uint64_t size = pow(base, i);
			int divider = size;
			std::vector<T> v;
			generateRandomInputs(v, size, seed+j);

			// Generate Exact/Skewed Pivot
			int skew = 2; // 2 == exact
			iter pivotIter;
			{
				std::vector<T> worV2 = v;
				tbb::parallel_sort(worV2.begin(), worV2.end(), [](T v1, T v2){
					return v1 < v2;
				});
				T temp = worV2[size/skew];
				pivotIter = std::find(v.begin(), v.end(), temp);
			}
			T pivot = *pivotIter;

			//
			// Create Working Set
			//
			std::vector<T> worV;
			iter worV_begin, worV_end;
			uint64_t pivotInd = pivotIter-v.begin();
			iter worP;
			uint64_t repetitions = 0;
			double timer = 0, sumTimer = 0;
			repetitions = (134217728/(sizeof(T)*size)) > 0 ? (134217728/(sizeof(T)*size)) : 1;

			//
			// Perform Work
			//
			name = "stdPar";
			timer = 0; sumTimer = 0;
			for (uint64_t r=0; r<repetitions; r++) {
				worV = v;
				worV_begin = worV.begin();
				worV_end = worV.end();

				timer = gettime();
				std::partition(worV_begin, worV_end, [pivot](T v1){
					return v1<pivot;
				});
				sumTimer += gettime()-timer;
			}
			std::cout << name << ", " << std::to_string(base) << "^" << std::to_string(i) << ", " << typeName << ", " << std::to_string(sumTimer*1e3) << std::endl;

			name = "lomPar";
			timer = 0; sumTimer = 0;
			for (uint64_t r=0; r<repetitions; r++) {
				worV = v;
				worV_begin = worV.begin();
				worV_end = worV.end();
				worP = worV_begin+pivotInd;

				timer = gettime();
        myPartition::lomuto_partition(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        });
				sumTimer += gettime()-timer;
			}
			std::cout << name << ", " << std::to_string(base) << "^" << std::to_string(i) << ", " << typeName << ", " << std::to_string(sumTimer*1e3) << std::endl;

			name = "hoaPar";
			timer = 0; sumTimer = 0;
			for (uint64_t r=0; r<repetitions; r++) {
				worV = v;
				worV_begin = worV.begin();
				worV_end = worV.end();
				worP = worV_begin+pivotInd;

				timer = gettime();
        partition::hoare_block_partition_unroll_loop(worV_begin, worV_end,
            worP, [](T v1, T v2){
          return v1 < v2;
        });
				sumTimer += gettime()-timer;
			}
			std::cout << name << ", " << std::to_string(base) << "^" << std::to_string(i) << ", " << typeName << ", " << std::to_string(sumTimer*1e3) << std::endl;

			/*name = "bfPar";
			timer = 0; sumTimer = 0;
			for (uint64_t r=0; r<repetitions; r++) {
				worV = v;
				worV_begin = worV.begin();
				worV_end = worV.end();
				worP = worV_begin+pivotInd;

				timer = gettime();
        myPartition::partition(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        });
				sumTimer += gettime()-timer;
			}
			std::cout << name << ", " << std::to_string(base) << "^" << std::to_string(i) << ", " << typeName << ", " << std::to_string(sumTimer*1e3) << std::endl;*/

			name = "bfPar";
			timer = 0; sumTimer = 0;
			for (uint64_t r=0; r<repetitions; r++) {
				worV = v;
				worV_begin = worV.begin();
				worV_end = worV.end();
				worP = worV_begin+pivotInd;

				timer = gettime();
        myPartition::partitionDyn(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        });
				sumTimer += gettime()-timer;
			}
			std::cout << name << ", " << std::to_string(base) << "^" << std::to_string(i) << ", " << typeName << ", " << std::to_string(sumTimer*1e3) << std::endl;

			name = "nbfPar";
			timer = 0; sumTimer = 0;
			for (uint64_t r=0; r<repetitions; r++) {
				worV = v;
				worV_begin = worV.begin();
				worV_end = worV.end();
				worP = worV_begin+pivotInd;

				timer = gettime();
        myPartition::partitionDynBRANCHES(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        });
				sumTimer += gettime()-timer;
			}
			std::cout << name << ", " << std::to_string(base) << "^" << std::to_string(i) << ", " << typeName << ", " << std::to_string(sumTimer*1e3) << std::endl;
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	std::cout << "     name, elements,     type,   timems" << std::endl;
	executeExperiment<uint8_t>("uint8");
	executeExperiment<uint16_t>("uint16");
	executeExperiment<uint32_t>("uint32");
	executeExperiment<uint64_t>("uint64");
	executeExperiment<Record<16>>("record16");
	executeExperiment<Record<32>>("record32");
	executeExperiment<Record<64>>("record64");
	executeExperiment<Record<128>>("record128");
}
