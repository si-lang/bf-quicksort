#include "../profile.cpp"
#include "../TimeAndProfile/profile_helper.cpp"

#include "../../QuickSort/SingleThreadedQuicksort/v9/quickSort.cpp"

#include "../EdelkampWeiss/blocked.h++"
#include "../pdqsort/pdqsort.h"
#include "../Yaroslavskiy/Yaroslavskiy.h++"
#include "../ips4o/ips4o.hpp"


int main(int argc, char** argv) {
	//typedef Record<64> T;
  typedef uint64_t T;

  short minElemFac = 3;
  short maxElemFac = 7;
  short numberOfIterations = 50;
  bool dispMem = false;
  bool dispIntermediate = false;

	std::random_device rd; uint64_t seed = rd(); // Random
	//uint64_t seed = 759271836937529; // Choose seed


  performWarmUp(false);

  std::cout << "--------------------------------------------" << '\n';
  std::cout << "---------- Starting Profilingsuite ---------" << '\n';
  std::cout << "--------------------------------------------" << '\n';

  std::cout << '\n';

  for (int i=minElemFac; i<=maxElemFac; i++) {
    if (dispIntermediate)
      std::cout << "---------- Test for 10^" << i <<" elements ----------";
    double stdRes[5] = {0};
    double pdRes[5] = {0};
    double yaRes[5] = {0};
    double edweRes[5] = {0};
    double ips4oRes[5] = {0};
    double myBlRes[5] = {0};
    double myImprBlRes[5] = {0};
    double myTunRes[5] = {0};

    uint64_t size = pow(10, i);
    int divider = /*1;*/ size;

    for(int j=0; j<numberOfIterations; j++) {
      if (dispIntermediate) {
        std::cout << '\n';
        std::cout << "10^" << i << " iteration " << j+1 << ":" << '\n';
      }

      double elemPerSec = 0;
      double misses = 0;
      double l1misses = 0;
      double instructions = 0;
      double bmisses = 0;

      std::vector<T> v;
      generateRandomInputs(v, size, seed+j);
      //generateAscendingInputs(v, size);
      //generateDescendingInputs(v, size);
      //generatePipeOrganInputs(v, size);
      //generateAllEqualInputs(v, size);
      std::vector<T> worV = v;
      std::vector<T>::iterator worV_begin = worV.begin();
      std::vector<T>::iterator worV_end = worV.end();
      std::string name = "";


      /* ------------ Profiling Iteration ------------ */
      /*worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "StandardImplementation";
      timeAndProfileOut(name, divider, [worV_begin, worV_end](){
          std::sort(worV_begin, worV_end, [](T v1, T v2){
            return v1<v2;
          });
      }, elemPerSec, misses, l1misses, instructions, bmisses, dispMem, dispIntermediate);
      stdRes[0] += elemPerSec;
      stdRes[1] += misses;
      stdRes[2] += l1misses;
      stdRes[3] += instructions;
      stdRes[4] += bmisses;
      elemPerSec = 0;
      misses = 0;
      l1misses = 0;
      instructions = 0;
      bmisses = 0;

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "Yaroslavskiy";
      timeAndProfileOut(name, divider, [worV_begin, worV_end](){
        Yaroslavskiy::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, elemPerSec, misses, l1misses, instructions, bmisses, dispMem, dispIntermediate);
      yaRes[0] += elemPerSec;
      yaRes[1] += misses;
      yaRes[2] += l1misses;
      yaRes[3] += instructions;
      yaRes[4] += bmisses;
      elemPerSec = 0;
      misses = 0;
      l1misses = 0;
      instructions = 0;
      bmisses = 0;

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "EdelkampWeissQS";
      timeAndProfileOut(name, divider, [worV_begin, worV_end](){
        blocked::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, elemPerSec, misses, l1misses, instructions, bmisses, dispMem, dispIntermediate);
      edweRes[0] += elemPerSec;
      edweRes[1] += misses;
      edweRes[2] += l1misses;
      edweRes[3] += instructions;
      edweRes[4] += bmisses;
      elemPerSec = 0;
      misses = 0;
      l1misses = 0;
      instructions = 0;
      bmisses = 0;

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "SuperScalarSampleS";
      timeAndProfileOut(name, divider, [worV_begin, worV_end](){
        ips4o::sort(worV_begin, worV_end, [](T v1, T v2){
          return v1<v2;
        });
      }, elemPerSec, misses, l1misses, instructions, bmisses, dispMem, dispIntermediate);
      ips4oRes[0] += elemPerSec;
      ips4oRes[1] += misses;
      ips4oRes[2] += l1misses;
      ips4oRes[3] += instructions;
      ips4oRes[4] += bmisses;
      elemPerSec = 0;
      misses = 0;
      l1misses = 0;
      instructions = 0;
      bmisses = 0;

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "PatternDefeatingQS";
      timeAndProfileOut(name, divider, [worV_begin, worV_end](){
          pdqsort_branchless(worV_begin, worV_end, [](T v1, T v2){
            return v1<v2;
          });
      }, elemPerSec, misses, l1misses, instructions, bmisses, dispMem, dispIntermediate);
      pdRes[0] += elemPerSec;
      pdRes[1] += misses;
      pdRes[2] += l1misses;
      pdRes[3] += instructions;
      pdRes[4] += bmisses;
      elemPerSec = 0;
      misses = 0;
      l1misses = 0;
      instructions = 0;
      bmisses = 0;*/

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "MyQuickSort";
      timeAndProfileOut(name, divider, [worV_begin, worV_end](){
          myQuickSort::quickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, elemPerSec, misses, l1misses, instructions, bmisses, dispMem, dispIntermediate);
      myBlRes[0] += elemPerSec;
      myBlRes[1] += misses;
      myBlRes[2] += l1misses;
      myBlRes[3] += instructions;
      myBlRes[4] += bmisses;
      elemPerSec = 0;
      misses = 0;
      l1misses = 0;
      instructions = 0;
      bmisses = 0;

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "MyImprovedImplementation";
      timeAndProfileOut(name, divider, [worV_begin, worV_end](){
          myQuickSort::improvedQuickSort(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, elemPerSec, misses, l1misses, instructions, bmisses, dispMem, dispIntermediate);
      myImprBlRes[0] += elemPerSec;
      myImprBlRes[1] += misses;
      myImprBlRes[2] += l1misses;
      myImprBlRes[3] += instructions;
      myImprBlRes[4] += bmisses;
      elemPerSec = 0;
      misses = 0;
      l1misses = 0;
      instructions = 0;
      bmisses = 0;

      worV = v;
      worV_begin = worV.begin();
      worV_end = worV.end();
      name = "MyTunedQuickSort";
      timeAndProfileOut(name, divider, [worV_begin, worV_end](){
          myQuickSort::tunedQuickSortDC(worV_begin, worV_end, [](T v1, T v2){
            return v1 < v2;
          });
      }, elemPerSec, misses, l1misses, instructions, bmisses, dispMem, dispIntermediate);
      myTunRes[0] += elemPerSec;
      myTunRes[1] += misses;
      myTunRes[2] += l1misses;
      myTunRes[3] += instructions;
      myTunRes[4] += bmisses;
      elemPerSec = 0;
      misses = 0;
      l1misses = 0;
      instructions = 0;
      bmisses = 0;
      /* ------------ Profiling Iteration End ------------ */

      if (dispIntermediate)
        std::cout << '\n';
    }

    std::cout << "------ Average of " << numberOfIterations << " for 10^" << i
        <<" elements ------" << '\n';
    std::cout << "StandardImplementation" << " "
        << stdRes[0]/numberOfIterations << "M/s "
        << stdRes[1]/numberOfIterations << " misses "
        << stdRes[2]/numberOfIterations << " L1misses "
        << stdRes[3]/numberOfIterations << " instructions "
        << stdRes[4]/numberOfIterations << " bmisses " << '\n';

    std::cout << "YaroslavskiyQS" << " "
        << yaRes[0]/numberOfIterations << "M/s "
        << yaRes[1]/numberOfIterations << " misses "
        << yaRes[2]/numberOfIterations << " L1misses "
        << yaRes[3]/numberOfIterations << " instructions "
        << yaRes[4]/numberOfIterations << " bmisses " << '\n';

    std::cout << "EdelkampWeissQS" << " "
        << edweRes[0]/numberOfIterations << "M/s "
        << edweRes[1]/numberOfIterations << " misses "
        << edweRes[2]/numberOfIterations << " L1misses "
        << edweRes[3]/numberOfIterations << " instructions "
        << edweRes[4]/numberOfIterations << " bmisses " << '\n';

    std::cout << "SuperScalarSampleS" << " "
        << ips4oRes[0]/numberOfIterations << "M/s "
        << ips4oRes[1]/numberOfIterations << " misses "
        << ips4oRes[2]/numberOfIterations << " L1misses "
        << ips4oRes[3]/numberOfIterations << " instructions "
        << ips4oRes[4]/numberOfIterations << " bmisses " << '\n';

    std::cout << "PatternDefeatingQS" << " "
        << pdRes[0]/numberOfIterations << "M/s "
        << pdRes[1]/numberOfIterations << " misses "
        << pdRes[2]/numberOfIterations << " L1misses "
        << pdRes[3]/numberOfIterations << " instructions "
        << pdRes[4]/numberOfIterations << " bmisses " << '\n';

    std::cout << "MyImplementation" << " "
        << myBlRes[0]/numberOfIterations << "M/s "
        << myBlRes[1]/numberOfIterations << " misses "
        << myBlRes[2]/numberOfIterations << " L1misses "
        << myBlRes[3]/numberOfIterations << " instructions "
        << myBlRes[4]/numberOfIterations << " bmisses " << '\n';

    std::cout << "MyImprovedImplementation" << " "
        << myImprBlRes[0]/numberOfIterations << "M/s "
        << myImprBlRes[1]/numberOfIterations << " misses "
        << myImprBlRes[2]/numberOfIterations << " L1misses "
        << myImprBlRes[3]/numberOfIterations << " instructions "
        << myImprBlRes[4]/numberOfIterations << " bmisses " << '\n';

    std::cout << "MyTunedImplementation" << " "
        << myTunRes[0]/numberOfIterations << "M/s "
        << myTunRes[1]/numberOfIterations << " misses "
        << myTunRes[2]/numberOfIterations << " L1misses "
        << myTunRes[3]/numberOfIterations << " instructions "
        << myTunRes[4]/numberOfIterations << " bmisses " << '\n';

    std::cout << '\n';
    std::cout << '\n';
  }
}
