#include <random>
#include <math.h> // pow

#include "../../../QuickSort/ParallelQuicksort/v3/parallelPartitionDyn.cpp"
#include "../../TimeAndProfile/profile.hpp"
#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../EdelkampWeiss/median.h"


#ifndef L2CACHESIZE
#define L2CACHESIZE 1048576
#endif


template<typename T>
void executeBlockExperiment(std::string typeName, std::vector<short> elemFacs) {
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 2
  int numberOfIterations = 15; // 15
	uint16_t minNumThreads = 1;
	uint16_t maxNumThreads = 20;
	uint16_t step = 1;
	uint64_t seed = 4264918403728822297; // Choose seed

	//
	// Execution Loop
	//
	PerfEvents e;
	std::string name = "bfParPar";
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
		// Pivot selection (like in Quicksort)
		//
		iter pivotIter;
		if (v.end()-v.begin() > 160000) {
			unsigned int pivot_sample_size = sqrt(v.end()-v.begin());
			pivot_sample_size += (1 - (pivot_sample_size % 2));// Make it an odd number
			pivotIter = median::median_of_k(v.begin(), v.end(), [](T v1, T v2){
				return v1<v2;
			}, pivot_sample_size); // Choose pivot as median of sqrt(n)
		} else {
			if (v.end()-v.begin() > 4000)
				pivotIter = median::median_of_5_medians_of_5(v.begin(), v.end(), [](T v1, T v2){
					return v1<v2;
				});
			else if (v.end()-v.begin() > 2000)
				pivotIter = median::median_of_3_medians_of_3(v.begin(), v.end(), [](T v1, T v2){
					return v1<v2;
				});
			else
				pivotIter = median::median_of_3(v.begin(), v.end(), [](T v1, T v2){
					return v1<v2;
				});
		}
		uint64_t pivotInd = pivotIter-v.begin();

		//
		// Perform Work
		//
		//for (uint16_t numThreads=minNumThreads; numThreads<=maxNumThreads; numThreads+=step) {
		for (uint16_t numThreads : {1, 2, 10, 20}) {
		uint64_t targetSize = ((L2CACHESIZE/sizeof(T))/2)/numThreads;
		uint64_t quater=0.25*targetSize, half=0.5*targetSize, tquater=0.75*targetSize, doubl=2*targetSize, four=4*targetSize;
		std::vector<uint64_t> chunkSizes = {quater, half, tquater, targetSize, doubl, four, four*2};//{512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, extraSize};

		for (auto chunkSize : chunkSizes) {
			//std::cout << "Elements: " << std::to_string(base)+"^"+std::to_string(elemFac) << "; NumThreads: " << numThreads << "; ChunkSize: " << chunkSize << ";" << std::endl;
			std::vector<T> worV = v;
			iter worV_begin = worV.begin();
			iter worV_end = worV.end();
			iter worP = worV_begin+pivotInd;

			e.timeAndProfile(name, divider, [&](){
				myParallelPartition::parallelPartition(worV_begin, worV_end, worP, [](T val1, T val2) {
      		return val1<val2;
    		}, numThreads, chunkSize);
			}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(elemFac)}, {"numThreads", std::to_string(numThreads)}, {"chunkSize", std::to_string(chunkSize)}, {"type", typeName}});
		}
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	//std::vector<short> elemFacs = {24, 26, 28}; // 24, 26, 28
	// 1GiB
	executeBlockExperiment<uint8_t>("uint8", {/*28, 29, */30});
	executeBlockExperiment<uint16_t>("uint16", {/*27, 28, */29});
	executeBlockExperiment<uint32_t>("uint32", {/*26, 27, */28});
	executeBlockExperiment<uint64_t>("uint64", {/*25, 26, */27});

	//elemFacs = {22, 24, 26}; // 22, 24, 26 => Caution 20 does not work, because 2^20 < 20*65536!!!!!!
	executeBlockExperiment<Record<16>>("record16", {/*24, 25, */26});
	executeBlockExperiment<Record<32>>("record32", {/*23, 24, */25});
	executeBlockExperiment<Record<64>>("record64", {/*22, 23, */24});
	executeBlockExperiment<Record<128>>("record128", {/*21, 22, */23});

	/*
	// 8GiB
	executeBlockExperiment<uint8_t>("uint8", {33});
	executeBlockExperiment<uint16_t>("uint16", {32});
	executeBlockExperiment<uint32_t>("uint32", {31});
	executeBlockExperiment<uint64_t>("uint64", {30});

	executeBlockExperiment<Record<16>>("record16", {29});
	executeBlockExperiment<Record<32>>("record32", {28});
	executeBlockExperiment<Record<64>>("record64", {27});
	executeBlockExperiment<Record<128>>("record128", {26});
	*/
}
