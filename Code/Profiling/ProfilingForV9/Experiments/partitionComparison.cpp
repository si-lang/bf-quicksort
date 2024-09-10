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
void executeBlockExperiment(std::string typeName) {
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 2
	short step = 2; // 2
  short minElemFac = 18; // 18
	short maxElemFac = 28; // 28 // Otw. we exceed RAM for 128 byte inputs
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

			//
			// Perform Work
			//
			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "stdPar";
      e.timeAndProfile(name, divider, [worV_begin, worV_end, pivot](){
        std::partition(worV_begin, worV_end, [pivot](T v1){
          return v1<pivot;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      worP = worV_begin+pivotInd;
      name = "lomPar";
			e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
        myPartition::lomuto_partition(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      worP = worV_begin+pivotInd;
      name = "hoaPar";
			e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
        partition::hoare_block_partition_unroll_loop(worV_begin, worV_end,
            worP, [](T v1, T v2){
          return v1 < v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			/*worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			name = "obfPar";
			e.timeAndProfile(name, divider, [&](){
        myPartition::partition(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			name = "bfPar";
			e.timeAndProfile(name, divider, [&](){
				myPartition::partitionDyn(worV_begin, worV_end, worP, [](T v1, T v2){
						return v1 < v2;
					});
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			name = "nbfPar";
			e.timeAndProfile(name, divider, [&](){
				myPartition::partitionDynBRANCHES(worV_begin, worV_end, worP, [](T v1, T v2){
						return v1 < v2;
					});
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			/*worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			name = "bfParPar";
			e.timeAndProfile(name, divider, [&](){
        myPartition::parallelPartition(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        }, 1);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	executeBlockExperiment<uint8_t>("uint8");
	executeBlockExperiment<uint16_t>("uint16");
	executeBlockExperiment<uint32_t>("uint32");
	executeBlockExperiment<uint64_t>("uint64");
	executeBlockExperiment<Record<16>>("record16");
	executeBlockExperiment<Record<32>>("record32");
	executeBlockExperiment<Record<64>>("record64");
	executeBlockExperiment<Record<128>>("record128");
}
