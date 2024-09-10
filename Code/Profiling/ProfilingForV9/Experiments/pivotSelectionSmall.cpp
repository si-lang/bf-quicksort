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

	std::vector<std::function<iter(iter, iter)>> pivotSelectors = {
		[](iter begin, iter end) { return end; },
		[](iter begin, iter end) { return median::median_of_3(begin, end, [](T v1, T v2){ return v1 < v2; }); },
		[](iter begin, iter end) { return median::median_of_5(begin, end, [](T v1, T v2){ return v1 < v2; }); },
		[](iter begin, iter end) { return median::median_of_3_medians_of_3(begin, end, [](T v1, T v2){ return v1 < v2; }); },
		[](iter begin, iter end) { return median::median_of_k(begin, end, [](T v1, T v2){ return v1 < v2; }, sqrt(end-begin)); },
		[](iter begin, iter end) { return median::median_of_k(begin, end, [](T v1, T v2){ return v1 < v2; }, log10(end-begin)); }
	};
	std::vector<std::string> pivotSelectorsNames = {
		"rnd",
		"medo3",
		"medo5",
		"medo3o3",
		"medoSqrt",
		"medoLg10"
	};
	short base = 2; // 2
	short minElemFac = 5; // 6
  short maxElemFac = 16; // 16
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
		uint64_t repetitions = ceil(134217728/(sizeof(T)*size)) > 0 ? ceil(134217728/(sizeof(T)*size)) : 1; // 128 MB data
		double timer = 0, sumTimer = 0;

		//
		// Perform Work
		//
		for (int p=0; p<pivotSelectors.size(); p++) {
			name = "bfPar";
			timer = 0; sumTimer = 0;
			for (uint64_t r=0; r<repetitions; r++) {
				std::vector<T> worV = v;
				iter worV_begin = worV.begin();
				iter worV_end = worV.end();

				timer = gettime();
				myQuickSort::quickSortDynIterationPivSel(worV_begin, worV_end, [](T v1, T v2){
					return v1 < v2;
				}, pivotSelectors[p]);
				sumTimer += gettime()-timer;
			}
			std::cout << name << ", " << std::to_string(base) << "^" << std::to_string(s) << ", " << pivotSelectorsNames[p] << ", " << typeName << ", " << std::to_string(sumTimer*1e3) << std::endl;
		}
	}
}


int main(int argc, char** argv) {
	performWarmUp(false);

	std::cout << "     name, elements, selector,     type,   timems" << std::endl;
	executeExperiment<uint8_t>("uint8");
	executeExperiment<uint16_t>("uint16");
	executeExperiment<uint32_t>("uint32");
	executeExperiment<uint64_t>("uint64");
	executeExperiment<Record<16>>("record16");
	executeExperiment<Record<32>>("record32");
	executeExperiment<Record<64>>("record64");
	executeExperiment<Record<128>>("record128");
}
