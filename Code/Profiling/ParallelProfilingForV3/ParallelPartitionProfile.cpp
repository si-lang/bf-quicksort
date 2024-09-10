#include <random>
#include <math.h> // pow
#include "tbb/parallel_sort.h" // Exact pivot
//#include <parallel/algorithm>

#include "../TimeAndProfile/profile_helper.cpp"
#include "../TimeAndProfile/profile.hpp"

#include "../EdelkampWeiss/partition.h"
#include "../EdelkampWeiss/median.h"

#include "../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
#include "../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelPartition.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelPartitionDyn.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelPartitionMaxTest.cpp"


template<typename iter, typename Comp>
inline void lomuto_partition(iter begin, iter end, Comp comp) {
  partition::Lomuto_partition<iter, Comp>::partition(begin, end, comp);
}

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
	short step = 1;
  short minElemFac = 30;
  short maxElemFac = 30;
  int numberOfIterations = 4;
	int exchangeInputAfterIteration = 10;
	const uint16_t numThreads = 10;//std::thread::hardware_concurrency();

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

			// Generate Pivot
			// iter pivotIter = median::median_of_3(v.begin(), v.end(), [](T v1, T v2){
      //   return v1 < v2;
      // });
			
			// iter pivotIter = median::median_of_5(v.begin(), v.end(), [](T v1, T v2){
      //   return v1 < v2;
      // });

			// iter pivotIter = median::median_of_3_medians_of_5(v.begin(), v.end(), [](T v1, T v2){
      //   return v1 < v2;
      // });

			// iter pivotIter = median::median_of_k(v.begin(), v.end(), [](T v1, T v2){
      //   return v1 < v2;
      // }, std::sqrt(size));

			// Exact/Skewed pivot
			int skew = 2; // 2 == exact
			std::vector<T> worV2 = v;
			tbb::parallel_sort(worV2.begin(), worV2.end(), [](T v1, T v2){
        return v1 < v2;
      });
			iter pivotIter = std::find(v.begin(), v.end(), worV2[size/skew]);

			//
			// Create/Reset working set
			//
			std::vector<T> worV;
      iter worV_begin, worV_end;
      std::string name = "";
      T pivot = *pivotIter;
      uint64_t pivotInd = pivotIter-v.begin();
      iter worP;

			//
			// Perform work
			//
			/*worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("Std", divider, [&](){
				std::partition(worV_begin, worV_end, [pivot](T v1) {
					return v1<pivot;
				});
			}, 1, {{"threads", "1"}, {"elements", std::to_string(base)+"^"+std::to_string(i)}});

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("StdMax", divider, [&]() {
				myPartition::stdMaxPartition(worV_begin, worV_end, [pivot](T v1){
					return v1<pivot;
				}, numThreads);
			}, 1, {{"threads", std::to_string(numThreads)}, {"elements", std::to_string(base)+"^"+std::to_string(i)}});

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myPar", divider, [&]() {
				myPartition::partition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1<v2;
				});
			}, 1, {{"threads", "1"}, {"elements", std::to_string(base)+"^"+std::to_string(i)}});

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myParMax", divider, [&](){
				myPartition::parallelMaxPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1<v2;
				}, numThreads);
			}, 1, {{"threads", std::to_string(numThreads)}, {"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

			/*worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myPar", divider, [&](){
				myPartition::parallelPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1<v2;
				}, 1);
			}, 1, {{"threads", "1"}, {"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

			/*worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("gnuPar", divider, [&](){
				__gnu_parallel::partition(worV_begin, worV_end, [pivot](T v1){
					return v1<pivot;
				});
			}, 1, {{"threads", std::to_string(numThreads)}, {"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myPar", divider, [&](){
				myParallelPartition::parallelPartition(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				}, numThreads);
			}, 1, {{"threads", std::to_string(numThreads)}, {"elements", std::to_string(base)+"^"+std::to_string(i)}});

			for (uint16_t flex=2; flex<=64; flex+=2) {
			worV = v;
			worV_begin = worV.begin();
			worV_end = worV.end();
			worP = worV_begin+pivotInd;
			e.timeAndProfile("myParO", divider, [&](){
				myParallelPartition::parallelPartitionFlex(worV_begin, worV_end, worP, [](T v1, T v2){
					return v1 < v2;
				}, numThreads, flex);
			}, 1, {{"threads", std::to_string(numThreads)}, {"elements", std::to_string(base)+"^"+std::to_string(i)}});
    	}
		}
  }
}
