#include <random>
#include <math.h> // pow
#include "tbb/parallel_sort.h" // Exact pivot

#include "../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
#include "../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"
#include "../TimeAndProfile/profile.1.hpp"
#include "../TimeAndProfile/profile_helper.cpp"
#include "../EdelkampWeiss/partition.h"
#include "../EdelkampWeiss/median.h"


template<typename iter, typename Comp>
inline void lomuto_partition(iter begin, iter end, Comp comp) {
  partition::Lomuto_partition<iter, Comp>::partition(begin, end, comp);
}

template<typename vec, typename iter>
void reset(vec& v, uint64_t size, uint64_t& pivotInd, vec& worV, iter& worV_begin, iter& worV_end, iter& worP, bool newInput, uint64_t seedOff=0) {
	typedef typename std::iterator_traits<iter>::value_type T;
	//std::random_device rd; uint64_t seed = rd(); // Random
	uint64_t seed = 4264918403728822297; // Choose seed

	// New Input
	if (newInput) {
		// Generate Input
		v.clear();
		generateRandomInputs(v, size, seed+seedOff);
		// generateAscendingInputs(v, size);
		// generateDescendingInputs(v, size);
		// generatePipeOrganInputs(v, size);
		// generateAllEqualInputs(v, size);

		// Generate Pivot
		// iter pivotIter = median::median_of_3(v.begin(), v.end(), [](T v1, T v2){
		// 	return v1 < v2;
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
		iter pivotIter;
		{
			std::vector<T> worV2 = v;
			tbb::parallel_sort(worV2.begin(), worV2.end(), [](T v1, T v2){
				return v1 < v2;
			});
			T temp = worV2[size/skew];
			pivotIter = std::find(v.begin(), v.end(), temp);
		}

		pivotInd = pivotIter-v.begin();
	}

	// Reset working set
	worV = v;
	worV_begin = worV.begin();
	worV_end = worV.end();
	worP = worV_begin+pivotInd;
}


int main(int argc, char** argv) {
	//typedef uint32_t T;
	typedef uint64_t T;
	//typedef Record<16> T;
	//typedef Record<32> T;
	//typedef Record<64> T;
	//typedef Record<128> T;
	typedef typename std::vector<T>::iterator iter;

	short base = 2;
	short step = 2;
  short minElemFac = 4;
  short maxElemFac = 16;
  int numberOfIterations = 1000;
	int exchangeInputAfterIteration = 10;

  performWarmUp(false);

  for (int i=minElemFac; i<=maxElemFac; i+=step) {
    uint64_t size = pow(base, i);
    int divider = 1;
		std::vector<T> v;

		// Create working set
		std::vector<T> worV;
		iter worV_begin, worV_end;
		std::string name = "";
		uint64_t pivotInd=0;
		iter worP;
		reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, true);
		T pivot = *(v.begin()+pivotInd);

		PerfEvents e;
		//
		// Perform work
		//
		reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, false);
		name = "stdPar";
		e.timeAndProfile(name, divider, [worV_begin, worV_end, pivot](){
			std::partition(worV_begin, worV_end, [pivot](T v1){
				return pivot<v1;
			});
		}, [&](size_t index){
			reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, false, index);
		}, numberOfIterations, {{"elements", std::to_string(base)+"^"+std::to_string(i)}}); // Standard
		//}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"skew", std::to_string(skew)}}); // Skew

		/*worV = v;
		worV_begin = worV.begin();
		worV_end = worV.end();
		worP = worV_begin+pivotInd;
		name = "LomutoPartition";
		e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
			myPartition::lomuto_partition(worV_begin, worV_end, worP, [](T v1, T v2){
				return v1 < v2;
			});
		}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

		reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, false);
		name = "HoarePartitionUnrolled";
		e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
			partition::hoare_block_partition_unroll_loop(worV_begin, worV_end,
					worP, [](T v1, T v2){
				return v1 < v2;
			});
		}, [&](size_t index){
			reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, false, 0);
		}, numberOfIterations, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

		reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, false);
		name = "MyPartition";
		e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
			myPartition::partition(worV_begin, worV_end, worP, [](T v1, T v2){
				return v1 < v2;
			});
		}, [&](size_t index){
			reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, false, 0);
		}, numberOfIterations, {{"elements", std::to_string(base)+"^"+std::to_string(i)}}); // Standard
		//}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"skew", std::to_string(skew)}}); // Skew

		/*reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, true);
		name = "MyPartitionDC";
		e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
			uint64_t pivotLength = 0;
			myPartition::qsPartitionDC(worV_begin, worV_end, worP, [](T v1, T v2){
				return v1 < v2;
			}, pivotLength);
		}, [&](size_t index){
			reset(v, size, pivotInd, worV, worV_begin, worV_end, worP, index>1&&exchangeInputAfterIteration%index==0, index);
		}, numberOfIterations, {{"elements", std::to_string(base)+"^"+std::to_string(i)}}); // Standard*/
		//}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"skew", std::to_string(skew)}}); // Skew*/

		/*worV = v;
		worV_begin = worV.begin();
		worV_end = worV.end();
		worP = worV_begin+pivotInd;
		name = "MyImprovedBlockImplementation";
		e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
			myPartition::improvedQSBlockPartition(worV_begin, worV_end, worP,
					[](T v1, T v2){
				return v1 < v2;
			});
		}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

		/*//
		// Alternative: To save time, perform work directly on v
		// Caution: Only use with exchangeInputAfterIteration = 1
		//
		exchangeInputAfterIteration = 1;

		e.timeAndProfile("MyBlockImplementation", divider, [&v, &pivotIter](){
			myPartition::qsBlockPartition(v.begin(), v.end(), pivotIter, [](T v1, T v2){
				return v1 < v2;
			});
		}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/
  }
}
