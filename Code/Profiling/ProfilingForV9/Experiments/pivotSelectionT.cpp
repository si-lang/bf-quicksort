#include <random>
#include <math.h> // pow, sqrt
#include "tbb/parallel_sort.h" // Exact pivot
#include <functional>

#include "../../../QuickSort/SingleThreadedQuicksort/v9/quickSort.cpp"
#include "../../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"
#include "../../TimeAndProfile/profile.hpp"
#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../EdelkampWeiss/partition.h"
#include "../../EdelkampWeiss/median.h"

/*
 * Finding 1: Don't use median of 3 at all...
 * Finding 2: 
 * 
 * 
 */

template<typename T>
void executeExperiment(std::string typeName) {
	typedef typename std::vector<T>::iterator iter;

	//std::vector<uint32_t> thresholds = {16, 32, 64, 128, 256, 512, 1024, 2048}; // Stage 1
	std::vector<uint32_t> thresholds = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, uint32_t(pow(2, 14)), uint32_t(pow(2, 15)), uint32_t(pow(2, 16)), uint32_t(pow(2, 17)), uint32_t(pow(2, 18)), uint32_t(pow(2, 19)), uint32_t(pow(2, 20)), uint32_t(pow(2, 21)), uint32_t(pow(2, 22)), uint32_t(pow(2, 23)), uint32_t(pow(2, 24))};//, uint32_t(pow(2, 25), uint32_t(pow(2, 26)}; // Stage 2
	short base = 2; // 2
	short minElemFac = 24; // 24
  short maxElemFac = 24; // 24
	short step = 2; // 2
  int numberOfIterations = 15; // 15
	uint64_t seed = 4264918403728822297; // Choose seed

	//
	// Execution Loop
	//
	PerfEvents e;
	std::string name = "bfPar";
	for (int s=minElemFac; s<=maxElemFac; s+=step)
	for (uint32_t i=0; i<numberOfIterations; i++) {
		//
		// Generate Inputs
		//
		uint64_t size = pow(base, s);
		int divider = size;
		std::vector<T> v;
		generateRandomInputs(v, size, seed+i);

		//
		// Perform Work
		//
		for (auto thresh : thresholds) {
			std::vector<T> worV = v;
			iter worV_begin = worV.begin();
			iter worV_end = worV.end();

			e.timeAndProfile(name, divider, [&](){
				myQuickSort::quickSortDynIterationPivSelT(worV_begin, worV_end, [](T v1, T v2){
					return v1 < v2;
				}, thresh);
			}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(s)}, {"thresh", std::to_string(thresh)}, {"type", typeName}});
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	/*executeExperiment<uint8_t>("uint8");
	executeExperiment<uint16_t>("uint16");*/
	executeExperiment<uint32_t>("uint32");
	/*executeExperiment<uint64_t>("uint64");
	executeExperiment<Record<16>>("record16");
	executeExperiment<Record<32>>("record32");
	executeExperiment<Record<64>>("record64");
	executeExperiment<Record<128>>("record128");*/
}
