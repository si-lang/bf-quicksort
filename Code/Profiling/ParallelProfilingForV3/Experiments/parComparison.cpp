#include <random>
#include <math.h> // pow
#include <thread> // std::thread::hardware_concurrency();

#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../TimeAndProfile/profile.hpp"

#include "../../EdelkampWeiss/median.h"

#include "../../../QuickSort/ParallelQuicksort/v3/parallelPartitionDyn.cpp"

#include <algorithm> // GNU parallel sort
#include "../../FriasPetitParPar/parallel_partition_def.h"


template<typename T>
void executeExperiment(std::string typeName, std::vector<short> elemFacs) {
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 2
	short step = 1; // 1
  short minElemFac = elemFacs[0]; // 26
	short maxElemFac = elemFacs[1]; // 32
  int numberOfIterations = 15; // 15
	uint64_t seed = 4264918403728822297; // Choose seed
	uint16_t maxThreads = std::thread::hardware_concurrency();

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
			iter pivotIter = median::median_of_5(
				v.begin(),
				v.begin()+1,
				v.begin()+(v.end()-v.begin())/2,
				v.end()-2,
				v.end()-1,
				[](T v1, T v2){
					return v1<v2;
				}
			);

			//
			// Create Working Set
			//
			std::vector<T> worV;
			iter worV_begin, worV_end, worP;
			uint64_t pivotOff = pivotIter-v.begin();
			T pivot;

			//
			// Perform Work
			//
			// Warm-up run
			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
			worP = worV_begin+pivotOff;
			myParallelPartition::parallelPartition(worV_begin, worV_end, worP, [](T v1, T v2){
				return v1 < v2;
			}, maxThreads);
			// Warm-up run

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotOff;
			name = "gnuPar";
			pivot = *pivotIter;
			e.timeAndProfile(name, divider, [&](){
				__gnu_parallel::partition(worV_begin, worV_end, [pivot](T v1){
					return v1<pivot;
				});
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotOff;
			name = "frPar";
			pivot = *pivotIter;
			iter pivotCut;
			e.timeAndProfile(name, divider, [&](){
				unguarded_parallel_partition_2(worV_begin, worV_end, pivotCut, [pivot](T v1){
					return v1<pivot;
				}, maxThreads);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotOff;
			name = "bfPar";
			e.timeAndProfile(name, divider, [&](){
				myParallelPartition::parallelPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				}, maxThreads);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	std::vector<short> elemFacsInt = {24, 30};
	executeExperiment<uint8_t>("uint8", elemFacsInt);
	executeExperiment<uint16_t>("uint16", elemFacsInt);
	executeExperiment<uint32_t>("uint32", elemFacsInt);
	executeExperiment<uint64_t>("uint64", elemFacsInt);

	std::vector<short> elemFacsRecord = {22, 28};
	executeExperiment<Record<16>>("record16", elemFacsRecord);
	executeExperiment<Record<32>>("record32", elemFacsRecord);
	executeExperiment<Record<64>>("record64", elemFacsRecord);
	executeExperiment<Record<128>>("record128", elemFacsRecord);
}
