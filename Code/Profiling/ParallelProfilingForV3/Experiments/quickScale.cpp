#include <random>
#include <math.h> // pow

#include "../../../QuickSort/ParallelQuicksort/v3/parallelQuicksortDyn.cpp"
#include "../../TimeAndProfile/profile.hpp"
#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../EdelkampWeiss/median.h"


template<typename T>
void executeExperiment(std::string typeName) {
	typedef typename std::vector<T>::iterator iter;

	std::vector<short> elemFacs = {30}; // 30
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
		// Perform Work
		//
		std::vector<T> worV = v;
		iter worV_begin = worV.begin();
		iter worV_end = worV.end();

		name = "bfSeqQs";
		e.timeAndProfile(name, divider, [&](){
			myQuickSort::quickSortDyn(worV_begin, worV_end, [](T val1, T val2) {
				return val1<val2;
			});
		}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(elemFac)}, {"numThreads", "1"}, {"type", typeName}});

		name = "bfParQs";
		for (uint16_t numThreads=minNumThreads; numThreads<=maxNumThreads; numThreads++) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();

			e.timeAndProfile(name, divider, [&](){
				myParallelQuickSort::parallelQuickSortDebug(worV_begin, worV_end, [](T val1, T val2) {
					return val1<val2;
				}, numThreads);
			}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(elemFac)}, {"numThreads", std::to_string(numThreads)}, {"type", typeName}});
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	/*executeExperiment<uint8_t>("uint8");
	executeExperiment<uint16_t>("uint16");*/
	executeExperiment<uint32_t>("uint32");
	executeExperiment<uint64_t>("uint64");

	/*executeExperiment<Record<16>>("record16");
	executeExperiment<Record<32>>("record32");
	executeExperiment<Record<64>>("record64");
	executeExperiment<Record<128>>("record128");*/
}
