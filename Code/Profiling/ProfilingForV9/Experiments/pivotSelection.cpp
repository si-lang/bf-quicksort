#include <random>
#include <math.h> // pow, sqrt
#include "tbb/parallel_sort.h" // Exact pivot
#include <functional>

#include "../../../QuickSort/SingleThreadedQuicksort/v9/quickSort.cpp"
#include "../../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"
#include "../../TimeAndProfile/profile.hpp"
#include "../../TimeAndProfile/profile_helper.cpp"
#include "../../EdelkampWeiss/partition.h"
#include "../../EdelkampWeiss/median.h"


template<typename T>
void executeExperiment(std::string typeName) {
	typedef typename std::vector<T>::iterator iter;

	// Median selection implementations by Edelkamp & Weiss
	// Thresholds adapted and enhanced
	std::vector<std::function<iter(iter, iter)>> pivotSelectors = {
		// Random
		[](iter begin, iter end) {
			return begin;
		},

		// Median of 3
		[](iter begin, iter end) {
			return median::median_of_3(begin, end, [](T v1, T v2){
				return v1 < v2;
			});
		},

		// Median of 5
		[](iter begin, iter end) {
			if (end-begin > 1000)
				return median::median_of_5(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
			else
				return median::median_of_3(begin, end, [](T v1, T v2){
				return v1 < v2;
			});
		},

		// Median of 3 medians of 3
		[](iter begin, iter end) {
			if (end-begin > 2000)
				return median::median_of_3_medians_of_3(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
			else
				return median::median_of_3(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
		},

		// Median of 5 medians of 5
		[](iter begin, iter end) {
			if (end-begin > 4000)
				return median::median_of_5_medians_of_5(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
			else if (end-begin > 2000)
				return median::median_of_3_medians_of_3(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
			else
				return median::median_of_3(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
		},

		// Median of sqrt
		[](iter begin, iter end) {
			if (end-begin > 160000) {
				unsigned int pivot_sample_size = sqrt(end-begin);
				pivot_sample_size += (1 - (pivot_sample_size % 2));//make it an odd number
				return median::median_of_k(begin, end, [](T v1, T v2){
					return v1 < v2;
				}, pivot_sample_size); //choose pivot as median of sqrt(n)
			} else {
				if (end-begin > 4000)
					return median::median_of_5_medians_of_5(begin, end, [](T v1, T v2){
						return v1 < v2;
					});
				else if (end-begin > 2000)
					return median::median_of_3_medians_of_3(begin, end, [](T v1, T v2){
						return v1 < v2;
					});
				else
					return median::median_of_3(begin, end, [](T v1, T v2){
						return v1 < v2;
					});
			}
		},

		// Median of log_2
		[](iter begin, iter end) {
			if (end-begin > 2000) {
				unsigned int pivot_sample_size = log2(end-begin);
				pivot_sample_size += (1-(pivot_sample_size % 2));//make it an odd number
				return median::median_of_k(begin, end, [](T v1, T v2){
					return v1 < v2;
				}, pivot_sample_size); //choose pivot as median of log2(n)
			} else {
				return median::median_of_3(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
			}
		},

		// Median of ln
		[](iter begin, iter end) {
			if (end-begin > 2000) {
				unsigned int pivot_sample_size = log(end-begin);
				pivot_sample_size += (1-(pivot_sample_size % 2));//make it an odd number
				return median::median_of_k(begin, end, [](T v1, T v2){
					return v1 < v2;
				}, pivot_sample_size); //choose pivot as median of ln(n)
			} else {
				return median::median_of_3(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
			}
		},

		// Median of log_10
		[](iter begin, iter end) {
			if (end-begin > 10000) {
				unsigned int pivot_sample_size = log10(end-begin);
				pivot_sample_size += (1-(pivot_sample_size % 2));//make it an odd number
				return median::median_of_k(begin, end, [](T v1, T v2){
					return v1 < v2;
				}, pivot_sample_size); //choose pivot as median of log10(n)
			} else {
				return median::median_of_3(begin, end, [](T v1, T v2){
					return v1 < v2;
				});
			}
		}
	};
	std::vector<std::string> pivotSelectorsNames = {
		"rnd",
		"medo3",
		"medo5",
		"medo3o3",
		"medo5o5",
		"medoSqrt",
		"medoLog2",
		"medoLogE",
		"medoLog10"
	};

	//
	// Configuration
	//
	short base = 2; // 2
	short minElemFac = 18; // 18
  short maxElemFac = 24; // 24
	short step = 1; // 1
  int numberOfIterations = 15; // 15
	uint64_t seed = 4264918403728822297; // Choose seed

	//
	// Execution Loop
	//
	PerfEvents e;
	std::string name = "bfPar";
	for (int s=minElemFac; s<=maxElemFac; s+=step)
	for (uint32_t i=0; i<numberOfIterations; i++) {
		//
		// Generate Inputs
		//
		uint64_t size = pow(base, s);
		int divider = size;
		std::vector<T> v;
		generateRandomInputs(v, size, seed+i);

		//
		// Perform Work
		//
		for (int p=0; p<pivotSelectors.size(); p++) {
			std::vector<T> worV = v;
			iter worV_begin = worV.begin();
			iter worV_end = worV.end();

			e.timeAndProfile(name, divider, [&](){
				myQuickSort::quickSortDynPivSel(worV_begin, worV_end, [](T v1, T v2){
					return v1 < v2;
				}, pivotSelectors[p]);
			}, 1, {{"elements", std::to_string(base)+"^"+std::to_string(s)}, {"pivotSelector", pivotSelectorsNames[p]}, {"type", typeName}});
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	executeExperiment<uint8_t>("uint8");
	executeExperiment<uint16_t>("uint16");
	executeExperiment<uint32_t>("uint32");
	executeExperiment<uint64_t>("uint64");
	executeExperiment<Record<16>>("record16");
	executeExperiment<Record<32>>("record32");
	executeExperiment<Record<64>>("record64");
	executeExperiment<Record<128>>("record128");
}
