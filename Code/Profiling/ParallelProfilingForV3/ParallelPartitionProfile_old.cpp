#include <random>

#include "../TimeAndProfile/profile_helper.cpp"
#include "../TimeAndProfile/profile.hpp"

#include "../EdelkampWeiss/median.h"

#include "../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelPartition.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelPartition.1.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelPartitionMaxTest.cpp"


int main(int argc, char** argv) {
  typedef uint8_t T;

  short minElemFac = 4;
  short maxElemFac = 10;
  int numberOfIterations = 10;
	const uint16_t numThreads = 20;

  performWarmUp(false);

  for (int i=minElemFac; i<=maxElemFac; i++) {
    uint64_t size = pow(10, i);
    int divider = size;

		std::vector<T> v;
		//std::random_device rd; uint64_t seed = rd(); // Random
		uint64_t seed = 759271836937529; // Choose seed

		generateRandomInputs(v, size, seed);
		//generateAscendingInputs(v, size);
		//generateDescendingInputs(v, size);
		//generatePipeOrganInputs(v, size);
		//generateAllEqualInputs(v, size);

		typedef typename std::vector<T>::iterator iter;
		std::vector<T> worV = v;
		iter worV_begin = worV.begin();
		iter worV_end = worV.end();

		iter pivotIter =
				median::median_of_5(worV_begin, worV_begin+1,
						worV_begin+(worV_end-worV_begin)/2, worV_end-2,
						worV_end-1, [](T v1, T v2){
							return v1<v2;
						});
			
		T pivot = *pivotIter;
		uint64_t pivotInd = pivotIter-worV_begin;
		iter worP = worV_begin+pivotInd;
					
		PerfEvents e;
		/*for (uint64_t j=0; j<numberOfIterations; j++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("Std", divider, [&](){
				std::partition(worV_begin, worV_end, [pivot](T v1) {
					return pivot<v1;
				});
			}, 1, {{"mode", "single"}, {"elements", std::to_string(10)+"^"+std::to_string(i)}});
		}

		for (uint64_t j=0; j<numberOfIterations; j++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("Std", divider, [&]() {
				myPartition::stdMaxPartition(worV_begin, worV_end, [pivot](T v1){
					return pivot<v1;
				}, numThreads);
			}, 1, {{"mode", "maxMulti"}, {"elements", std::to_string(10)+"^"+std::to_string(i)}});
		}

		for (uint64_t j=0; j<numberOfIterations; j++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myPar", divider, [&]() {
				myPartition::qsBlockPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				});
			}, 1, {{"mode", "single"}, {"elements", std::to_string(10)+"^"+std::to_string(i)}});
		}

		for (uint64_t j=0; j<numberOfIterations; j++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myPar", divider, [&](){
				myPartition::parallelMaxPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				}, numThreads);
			}, 1, {{"mode", "maxMulti"}, {"elements", std::to_string(10)+"^"+std::to_string(i)}});
		}*/

		/*for (uint64_t j=0; j<numberOfIterations; j++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myPar", divider, [&](){
				myPartition::parallelPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				}, 1);
			}, 1, {{"threads", std::to_string(1)}, {"elements", std::to_string(10)+"^"+std::to_string(i)}});
		}*/

		for (uint64_t j=0; j<numberOfIterations; j++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myPar", divider, [&](){
				myPartition::parallelPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				}, numThreads);
			}, 1, {{"threads", std::to_string(numThreads)}, {"elements", std::to_string(10)+"^"+std::to_string(i)}});
		}

		/*for (uint64_t j=0; j<numberOfIterations; j++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myParO", divider, [&](){
				myPartitionA::parallelPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				}, numThreads);
			}, 1, {{"threads", std::to_string(numThreads)}, {"elements", std::to_string(10)+"^"+std::to_string(i)}});
		}*/
	}
}
