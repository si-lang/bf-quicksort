#include <random>
#include <math.h> // pow
#include "tbb/parallel_sort.h" // Exact pivot

#include "../../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
#include "../../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"
#include "../../TimeAndProfile/profile.hpp"
#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../EdelkampWeiss/partition.h"
#include "../../EdelkampWeiss/median.h"


template<typename T>
void executeBlockExperiment(std::string typeName) {
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 2
	short minElemFac = 10;
  short maxElemFac = 26; // 26
  int numberOfIterations = 15; // 15
	uint64_t seed = 4264918403728822297; // Choose seed

	//
	// Execution Loop
	//
	PerfEvents e;
	std::string name = "bfPar";
	for (short s=minElemFac; s<=maxElemFac; s++)
	for (uint32_t i=0; i<numberOfIterations; i++) {
		//
		// Generate Inputs
		//
		uint64_t size = pow(base, s);
		int divider = size;
		std::vector<T> v;
		generateRandomInputs(v, size, seed+i);

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
		uint64_t pivotInd = pivotIter-v.begin();

		//
		// Perform Work
		//
		std::vector<T> worV = v;
		iter worV_begin = worV.begin();
		iter worV_end = worV.end();
		iter worP = worV_begin+pivotInd;

		e.timeAndProfile("bfPar", divider, [&](){
			myPartition::partition(worV_begin, worV_end, worP, [](T v1, T v2){
				return v1 < v2;
			});
		}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(s)}, {"blockSize", "512"}, {"type", typeName}});

		worV = v;
		worV_begin = worV.begin();
		worV_end = worV.end();
		worP = worV_begin+pivotInd;

		e.timeAndProfile("dynBfPar", divider, [&](){
			myPartition::partitionDyn(worV_begin, worV_end, worP, [](T v1, T v2){
				return v1 < v2;
			});
		}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(s)}, {"blockSize", std::to_string(32768/(8*sizeof(T)))}, {"type", typeName}});
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
