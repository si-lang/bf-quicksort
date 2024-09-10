#include <random>
#include <math.h> // pow
#include "tbb/parallel_sort.h" // Exact pivot
#include <algorithm> // GNU parallel sort
#include <thread>

#include "../TimeAndProfile/profile_helper.cpp"
#include "../TimeAndProfile/profile.hpp"

#include "../EdelkampWeiss/partition.h"
#include "../EdelkampWeiss/median.h"

#include "../../QuickSort/SingleThreadedQuicksort/v9/quickSort.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelQuicksort.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelQuicksortDyn.cpp"
#include "../libstdc++/parallel/balanced_quicksort.h"
#include "../libstdc++/parallel/balanced_quicksort_mod.h"

#include "tbb/parallel_sort.h"
#include "../pdqsort/pdqsort.h"
#include "../Yaroslavskiy/Yaroslavskiy.h++"
#include "../ips4o/ips4o.hpp"


int main(int argc, char** argv) {
	typedef uint8_t T;
	//typedef uint32_t T;
	//typedef uint64_t T;
	//typedef Record<32> T;
	//typedef Record<64> T;
	//typedef Record<84> T; // 84 byte record like Edelkamp&Weiss
	//typedef Record<128> T;
	typedef typename std::vector<T>::iterator iter;

	short base = 2;
	short step = 2;
  short minElemFac = 22;
  short maxElemFac = 22;
  int numberOfIterations = 2;
	int exchangeInputAfterIteration = 5;
	const uint16_t numThreads = std::thread::hardware_concurrency();

	//std::random_device rd; uint64_t seed = rd(); // Random
	uint64_t seed = 4264918403728822297; // Choose seed

  performWarmUp(false);

  for (int i=minElemFac; i<=maxElemFac; i+=step) {
    uint64_t size = pow(base, i);
    int divider = size;
		std::vector<T> v;

		PerfEvents e;
    for(int j=0; j<numberOfIterations; j++) {
			//
			// Generate Inputs
			//
			if (j%exchangeInputAfterIteration==0) {
				v.clear();
				generateRandomInputs(v, size, seed+j);
				// generateAscendingInputs(v, size);
				// generateDescendingInputs(v, size);
				// generatePipeOrganInputs(v, size);
				// generateAllEqualInputs(v, size);
			}

			//
			// Create/Reset working set
			//
			std::vector<T> worV;
      iter worV_begin, worV_end;
      std::string name = "";

			//
			// Perform work
			//
			/*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "StandardImplementation";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          std::sort(worV_begin, worV_end, [](T v1, T v2){
            return v1<v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "PatternDefeatingQS";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          pdqsort_branchless(worV_begin, worV_end, [](T v1, T v2){
            return v1<v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "TBB";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        tbb::parallel_sort(worV_begin, worV_end, [](T v1, T v2){ //TODO
          return v1<v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "SuperScalarSampleSort";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        ips4o::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "Ipssss";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        ips4o::parallel::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "MyQick";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::quickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "GnuUS";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          __gnu_parallel::sort(worV_begin, worV_end);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "GnuBS";
      e.timeAndProfile(name, divider, [worV_begin, worV_end, numThreads](){
          __gnu_parallel::__parallel_sort_qsb(worV_begin, worV_end, [](T v1, T v2){ return v1<v2;}, numThreads);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "ModGnuBS";
      e.timeAndProfile(name, divider, [worV_begin, worV_end, numThreads](){
          __gnu_parallel_mod::__parallel_sort_qsb(worV_begin, worV_end, [](T v1, T v2){ return v1<v2;}, numThreads);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

			/*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "GnuSS";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          __gnu_parallel::stable_sort(worV_begin, worV_end);
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "MyParQ";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::parallelQuickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

			for (uint32_t i=1; i<worV.size(); i++) {
				assert(worV[i]>=worV[i-1]);
			}

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "MyDynParQ";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myParallelQuickSort::parallelQuickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

			for (uint32_t i=1; i<worV.size(); i++) {
				assert(worV[i]>=worV[i-1]);
			}*/

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "MyTunedQuickSort";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::tunedQuickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/
    }
  }
}
