/*
 * Previous experiments where executed with 2^33 elements for all input types.
 */

#include <random>
#include <math.h> // pow

#include "../../../QuickSort/ParallelQuicksort/v3/parallelPartitionDyn.cpp"
#include "../../../QuickSort/ParallelQuicksort/v3/parallelQuicksortDyn.cpp"
#include "../../TimeAndProfile/profile.hpp"
#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../EdelkampWeiss/median.h"


template<typename T>
void executeExperiment(std::string typeName, std::vector<short> elemFacs) {
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 2
  int numberOfIterations = 15; // 15
	uint16_t minNumThreads = 1;
	uint16_t maxNumThreads = 20;
	uint64_t seed = 4264918403728822297; // Choose seed

	//
	// Execution Loop
	//
	PerfEvents e;
	std::string name = "";
	for (auto elemFac : elemFacs)
	for (uint32_t i=0; i<numberOfIterations; i++) {
		//
		// Generate Inputs
		//
		uint64_t size = pow(base, elemFac);
		int divider = size;
		std::vector<T> v;
		generateRandomInputs(v, size, seed+i);

		//
		// Pivot selection
		//
		// Generate Exact/Skewed Pivot
		int skew = 2; // 2 == exact
		iter pivotIter;
		{
			std::vector<T> worV2 = v;
			myParallelQuickSort::parallelQuickSort(worV2.begin(), worV2.end(), [](T v1, T v2){
				return v1 < v2;
			});
			T temp = worV2[size/skew];
			pivotIter = std::find(v.begin(), v.end(), temp);
		}
		uint64_t pivotInd = pivotIter-v.begin();

		//
		// Perform Work
		//
		std::vector<T> worV = v;
		iter worV_begin = worV.begin();
		iter worV_end = worV.end();
		iter worP = worV_begin+pivotInd;
		name = "bfSeqPar";
		e.timeAndProfile(name, divider, [&](){
			myPartition::partitionDyn(worV_begin, worV_end, worP, [](T val1, T val2) {
				return val1<val2;
			});
		}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(elemFac)}, {"numThreads", "1"}, {"type", typeName}});

		name = "bfParPar";
		for (uint16_t numThreads=minNumThreads; numThreads<=maxNumThreads; numThreads++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;

			e.timeAndProfile(name, divider, [&](){
				myParallelPartition::parallelPartition(worV_begin, worV_end, worP, [](T val1, T val2) {
					return val1<val2;
				}, numThreads);
			}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(elemFac)}, {"numThreads", std::to_string(numThreads)}, {"type", typeName}});
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	executeExperiment<uint8_t>("uint8", {32});
	executeExperiment<uint16_t>("uint16", {32});
	executeExperiment<uint32_t>("uint32", {32});
	/*executeExperiment<uint64_t>("uint64");

	executeExperiment<Record<16>>("record16");
	executeExperiment<Record<32>>("record32");
	executeExperiment<Record<64>>("record64");
	executeExperiment<Record<128>>("record128");*/
}
