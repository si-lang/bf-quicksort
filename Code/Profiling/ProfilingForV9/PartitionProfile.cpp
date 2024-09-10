#include <random>
#include <math.h> // pow
#include "tbb/parallel_sort.h" // Exact pivot

#include "../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
#include "../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"
#include "../TimeAndProfile/profile.hpp"
#include "../TimeAndProfile/profile_helper.cpp"
#include "../EdelkampWeiss/partition.h"
#include "../EdelkampWeiss/median.h"


template<typename iter, typename Comp>
inline void lomuto_partition(iter begin, iter end, Comp comp) {
  partition::Lomuto_partition<iter, Comp>::partition(begin, end, comp);
}

int main(int argc, char** argv) {
	//typedef uint32_t T;
	typedef uint64_t T; // !
	//typedef Record<16> T;
	//typedef Record<32> T;
	//typedef Record<64> T;
	//typedef Record<84> T; // 84 byte record like Edelkamp&Weiss
	//typedef Record<128> T;
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 10
	short step = 1; // 1
  short minElemFac = 4; // 4
  short maxElemFac = 12; // 9
  int numberOfIterations = 20; // 20
	int exchangeInputAfterIteration = 20; // 5

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
			/*iter pivotIter = median::median_of_3(v.begin(), v.end(), [](T v1, T v2){
        return v1 < v2;
      });*/
			
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
      name = "stdPar";
      e.timeAndProfile(name, divider, [worV_begin, worV_end, pivot](){
        std::partition(worV_begin, worV_end, [pivot](T v1){
          return v1<pivot;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}}); // Standard
			//}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"skew", std::to_string(skew)}}); // Skew

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      worP = worV_begin+pivotInd;
      name = "LomutoPartition";
			e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
        myPartition::lomuto_partition(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      worP = worV_begin+pivotInd;
      name = "HoarePartitionUnrolled";
			e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
        partition::hoare_block_partition_unroll_loop(worV_begin, worV_end,
            worP, [](T v1, T v2){
          return v1 < v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}});*/

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      worP = worV_begin+pivotInd;
      name = "MyBlockImplementation";
			e.timeAndProfile(name, divider, [worV_begin, worV_end, worP](){
        myPartition::partition(worV_begin, worV_end, worP, [](T v1, T v2){
          return v1 < v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}}); // Standard
			//}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"skew", std::to_string(skew)}}); // Skew

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
}
