#include "../EdelkampWeiss/median.h"
#include "../profile.cpp"
#include "../../QuickSort/ParallelQuicksort/v2/parallelPartitionStatic.cpp"


int main(int argc, char** argv) {
  typedef uint64_t T;

  short minElemFac = 6;
  short maxElemFac = 6;
  int numberOfIterations = 10;


  for (int i=minElemFac; i<=maxElemFac; i++) {
    uint64_t size = pow(10, i);
    int divider = /*1;*/ size;

    std::vector<T> v;
    generateRandomInputs(v, size, i);
    //generateAscendingInputs(v, size);
    //generateDescendingInputs(v, size);
    //generatePipeOrganInputs(v, size);
    //generateAllEqualInputs(v, size);

    typedef typename std::vector<T>::iterator iter;
    iter begin = v.begin();
    iter end = v.end();

    for(int j=0; j<numberOfIterations; j++) {
      iter pivotIter =
          median::median_of_5(begin, begin+1,
              begin+(end-begin)/2, end-2,
              end-1, [](T v1, T v2){
                return v1<v2;
              });

      myPartition::parallelPartition(begin, end, pivotIter,
          [](T v1, T v2){
        return v1 < v2;
      });
    }
  }

  exit(0);
}
