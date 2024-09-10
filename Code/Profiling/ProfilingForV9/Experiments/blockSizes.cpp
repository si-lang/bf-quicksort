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

	std::vector<uint16_t> blockSizes = {32, 64, 128, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 3072, 4096};
	short base = 2; // 2
  short elemFac = 26; // 26
  int numberOfIterations = 15; // 15
	uint64_t seed = 4264918403728822297; // Choose seed

	//
	// Execution Loop
	//
	PerfEvents e;
	std::string name = "bfPar";
	for (uint32_t i=0; i<numberOfIterations; i++) {
		//
		// Generate Inputs
		//
		uint64_t size = pow(base, elemFac);
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
		for (auto blockSize : blockSizes) {
			std::vector<T> worV = v;
			iter worV_begin = worV.begin();
			iter worV_end = worV.end();
			iter worP = worV_begin+pivotInd;

			e.timeAndProfile(name, divider, [&](){
				myPartition::partitionBlEx(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				}, blockSize);
			}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(elemFac)}, {"blockSize", std::to_string(blockSize)}, {"type", typeName}});
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
