#include <random>
#include <math.h> // pow

#include "../../TimeAndProfile/profile.hpp"
#include "../../TimeAndProfile/profile_helper.cpp"

#include "../../../QuickSort/SingleThreadedQuicksort/v9/quickSort.cpp"
#include "../../EdelkampWeiss/blocked.h++"
#include "../../pdqsort/pdqsort.h"
#include "../../Yaroslavskiy/Yaroslavskiy.h++"
#include "../../ips4o/ips4o.hpp"


template<typename T>
void executeExperiment(std::string typeName) {
	typedef typename std::vector<T>::iterator iter;

	short base = 2; // 2
	short step = 1; // 1
  short minElemFac = 18; // 18
	short maxElemFac = 26; // 26
  int numberOfIterations = 15; // 15
	uint64_t seed = 4264918403728822297; // Choose seed

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
			iter worP;

			//
			// Perform Work
			//
      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "stdqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          std::sort(worV_begin, worV_end, [](T v1, T v2){
            return v1<v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "yaqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        Yaroslavskiy::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "blqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        blocked::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "blqs(dc,sq)";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        blocked::sortDC(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "ip4s";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
        ips4o::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "pdqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          pdqsort_branchless(worV_begin, worV_end, [](T v1, T v2){
            return v1<v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "obfqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::quickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/

			/*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "ibfqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::improvedQuickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "bfqs(aec)";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::quickSortDyn(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "bfqs(dc)";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::quickSortDynDC(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

			if (!(typeName=="uint8" || typeName=="uint16")) {
			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "bfqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::quickSortDynNoPD(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});
			}

			worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "nbfqs(aec)";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::quickSortDynBRANCHES(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "tbfqs";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::tunedQuickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/

      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "tbfdc";
      e.timeAndProfile(name, divider, [worV_begin, worV_end](){
          myQuickSort::tunedQuickSortDC(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, 1, {{"elements", std::to_string(base)+"^"+std::to_string(i)}, {"type", typeName}});*/
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
