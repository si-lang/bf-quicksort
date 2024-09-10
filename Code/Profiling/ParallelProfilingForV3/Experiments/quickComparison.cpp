#include <random>
#include <math.h> // pow
#include <thread>

#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../TimeAndProfile/profile.hpp"

#include "../../../QuickSort/ParallelQuicksort/v3/parallelQuicksortDyn.cpp"
#include "../../../QuickSort/ParallelQuicksort/v3/parallelQuicksortDynNoTP.cpp"

#include <algorithm> // GNU parallel sort
#include "tbb/parallel_sort.h"
#include "../../ips4o/ips4o.hpp"
//#include "../../libstdc++/parallel/balanced_quicksort.h"
//#include "../../libstdc++/parallel/quicksort.h"


template<typename T>
void executeExperiment(std::string typeName, std::vector<short> elemFacs) {
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 2
	short step = 1; // 1
  short minElemFac = elemFacs[0]; // 18
	short maxElemFac = elemFacs[1]; // 32
  int numberOfIterations = 15; // 15
	uint64_t seed = 4264918403728822297; // Choose seed
	const uint16_t numThreads = std::thread::hardware_concurrency();

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

			//
			// Create Working Set
			//
			std::vector<T> worV;
			iter worV_begin, worV_end;

			//
			// Perform Work
			//
			// Warm-up run
			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
			ips4o::parallel::sort(worV_begin, worV_end, [](T v1, T v2){
      	return v1<v2;
      });
			// Warm-up run

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "tbbqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        tbb::parallel_sort(worV_begin, worV_end);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "ip4s";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        ips4o::parallel::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "gnuqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          __gnu_parallel::sort(worV_begin, worV_end);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			if (!((typeName=="record32" && size>=pow(2,23)) || (typeName=="record64" && size>=pow(2,22)) || (typeName=="record128" && size>=pow(2,21)))) {
			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "gnums";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          __gnu_parallel::stable_sort(worV_begin, worV_end);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});
			}

			/*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "gnuqsu";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          __gnu_parallel::__parallel_sort_qs(worV_begin, worV_end, [](T v1, T v2){ return v1<v2; }, 20);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "gnuqsb";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          __gnu_parallel::__parallel_sort_qsb(worV_begin, worV_end, [](T v1, T v2){ return v1<v2; }, 20);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "bfqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myParallelQuickSort::parallelQuickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			/*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "bfqsnotp";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myParallelQuickSortNoTP::parallelQuickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	std::vector<short> uintElemFacs = {20, 30};
	executeExperiment<uint8_t>("uint8", uintElemFacs);
	executeExperiment<uint16_t>("uint16", uintElemFacs);
	executeExperiment<uint32_t>("uint32", uintElemFacs);
	executeExperiment<uint64_t>("uint64", uintElemFacs);

	std::vector<short> recordElemFacs = {18, 28};
	executeExperiment<Record<16>>("record16", recordElemFacs);
	executeExperiment<Record<32>>("record32", recordElemFacs);
	executeExperiment<Record<64>>("record64", recordElemFacs);
	executeExperiment<Record<128>>("record128", recordElemFacs);
}
