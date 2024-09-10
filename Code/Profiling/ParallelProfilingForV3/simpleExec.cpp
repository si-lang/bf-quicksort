#include <string>
#include <vector>
#include <algorithm> // Needed for median

#include "../TimeAndProfile/profile_helper.cpp"
#include "../TimeAndProfile/profile.hpp"

#include "../EdelkampWeiss/median.h"

#include "../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
// Don't use for quicksort testing
#include "../../QuickSort/ParallelQuicksort/v3/parallelPartitionDyn.cpp"
//#include "../../QuickSort/ParallelQuicksort/v3/parallelPartitionMaxTest.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelQuicksortDyn.cpp"


template<typename T>
void executeExperiment(std::string typeName, uint16_t numThreads, uint64_t seed) {
	typedef typename std::vector<T>::iterator iter;
	uint64_t size = std::pow(2, 30);
	uint64_t repetitions = 1;
	
	std::vector<T> v, worV, checkV;
	generateRandomInputs(v, size, seed);
	worV = v;
	iter begin = worV.begin();
	iter end = worV.end();
	
	/*iter pivotIter = median::median_of_5(
		begin,
	 	begin+1,
	 	begin+(end-begin)/2,
	 	end-2,
	 	end-1,
	 	[](T v1, T v2){
	 		return v1<v2;
	 	}
	);
	
	uint64_t pivotOff = pivotIter-begin;*/
	std::string str="";
	for (uint64_t i=0; i<repetitions; i++) {
		PerfEvents e;
		worV = v;
		begin = worV.begin();
		end = worV.end();
		//pivotIter = begin+pivotOff;
		e.timeAndProfile("My", size, [&](){
			//std::cout << "Press enter to start..." << std::endl;
			//std::getline(std::cin, str);

			//myPartition::partitionDyn(begin, end, pivotIter, [](T v1, T v2){return v1<v2;});
			//myParallelPartition::parallelPartition(begin, end, pivotIter, [](T v1, T v2){return v1<v2;}, numThreads);
			//myQuickSort::quickSortDyn(begin, end, [](T v1, T v2){return v1<v2;});
			myParallelQuickSort::parallelQuickSortDebug(begin, end, [](T v1, T v2){return v1<v2;}, numThreads);

			//std::cout << "Press enter to end..." << std::endl;
			//std::getline(std::cin, str);
		}, 1, {{"thr", std::to_string(numThreads)}, {"type", typeName}});

		/*PerfEvents f;
		worV = v;
		begin = worV.begin();
		end = worV.end();
		pivotIter = begin+pivotOff;
		T pivot = *pivotIter;
		f.timeAndProfile("Other", size, [&](){
			//std::cout << "Press enter to start..." << std::endl;
			//std::getline(std::cin, str);

			//std::partition(begin, end, [&](T elem){
			//	return pivot < elem;
			//});

			//myPartition::parallelPartition(begin, end, pivotIter, [](T v1, T v2){return v1<v2;}, numThreads);
			// 	return pivot < elem;
			// });
			//__gnu_parallel::partition(begin, end, [&](T elem){
			// 	return pivot < elem;
			// });

			std::sort(begin, end);
			//myQuickSort::quickSortDyn(begin, end, [](T v1, T v2){return v1<v2;});
			//__gnu_parallel::sort(begin, end);

			//std::cout << "Press enter to end..." << std::endl;
			//std::getline(std::cin, str);
		}, 1, {{"thr", std::to_string(numThreads)}, {"type", typeName}});*/
	}
}


int main(int argc, char** argv) {
	assert(argc>1);

	// executeExperiment<uint8_t>("uint8", atoi(argv[1]), 1);
	// executeExperiment<uint16_t>("uint16", atoi(argv[1]), 2);
	// executeExperiment<uint32_t>("uint32", atoi(argv[1]), 3);
	executeExperiment<uint64_t>("uint64", atoi(argv[1]), 4);

	// executeExperiment<Record<16>>("record16", atoi(argv[1]), 5);
	// executeExperiment<Record<32>>("record32", atoi(argv[1]), 6);
	// executeExperiment<Record<64>>("record64", atoi(argv[1]), 7);
	// executeExperiment<Record<128>>("record128", atoi(argv[1]), 8);
}
