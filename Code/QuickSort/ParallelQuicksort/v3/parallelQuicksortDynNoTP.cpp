#include <algorithm> // Used for swap()
#include <iostream>
#include <thread> // Used für threads
#include <functional> // Used for std::ref
#include <mutex> // Used std::mutex
#include <atomic> // Used for std::atomic<std::uint64_t>
#include <cstring> // Used for memmove
#include <future>
#include <thread>

#include "parallelPartitionDyn.cpp"
#include "../../SingleThreadedQuicksort/v9/quickSort.cpp"

#include "../../../Profiling/EdelkampWeiss/blocked.h++"
#include "../../../Profiling/EdelkampWeiss/median.h"
#include "../../../Profiling/EdelkampWeiss/insertionsort.h"
#include "../../../Profiling/EdelkampWeiss/partition.h"


#ifndef PARALLEL_QUICKSORT_DYN_No_TP_CPP
#define PARALLEL_QUICKSORT_DYN_No_TP_CPP

namespace myParallelQuickSortNoTP {
	/*// Sequential shared Quicksort
	template<typename iter, typename Comp>
  inline void sequentialQuickSort(iter begin, iter end, Comp comp, const uint16_t blockSize);

  template<typename iter, typename Comp>
  inline void sequentialQuickSortIteration(iter begin, iter end, Comp comp, const uint16_t blockSize) {
		typedef typename std::iterator_traits<iter>::value_type T;
    iter pivotIntermed = end-1;
    uint64_t lCnt = 0;

    // Caution: Iterator names get mixed here!
    //const iter pivotIter = median::median_of_3(begin, end, comp);//myMedian::medianOfThree(begin, lastElemIter);
		const iter pivotIter = median::median_of_5(begin, begin+1, begin+(end-begin)/2, end-2, end-1, comp);
    std::iter_swap(pivotIter, pivotIntermed);

    lCnt = myPartition::partitionDynQs(begin, pivotIntermed, pivotIntermed, comp, blockSize);
    iter midIter = begin+lCnt;

		// Catch all equal case
		if (pivotIntermed == midIter || lCnt == 0) {
    	bool allEqual = true;
			iter beg = begin;
    	while(beg != pivotIntermed) {
    		if (*beg!=*pivotIntermed) {
    			allEqual = false;
    			break;
    		}
    		beg++;
    	}
    	if (allEqual) {
    		return;
			}
    }

    std::iter_swap(midIter, pivotIntermed);
    // Iterators naming consistent again
    const iter beginIterR = midIter+1;

    if (begin < midIter) {
			sequentialQuickSort(begin, midIter, comp, blockSize);
    }
    if (beginIterR < end) {
      sequentialQuickSort(beginIterR, end, comp, blockSize);
    }
  }

  template<typename iter, typename Comp>
	inline void sequentialQuickSort(iter begin, iter end, Comp comp, const uint16_t blockSize) {
    if ((end-begin) > 100) { // Avoid overhead from thread pool
			sequentialQuickSortIteration(begin, end, comp, pool, blockSize);
    } else if ((end-begin) > THRESH) {
      myQuickSort::quickSortDynIteration(begin, end, comp, blockSize);
    } else {
			insertionsort::insertion_sort(begin, end, comp);
		}
	}*/


	template<typename iter, typename Comp>
	inline void parallelQuickSort(iter begin, iter end, Comp comp, uint16_t numThreads, const uint16_t blockSize);

  template<typename iter, typename Comp>
  inline void parallelQuickSortIteration(iter begin, iter end, Comp comp, uint16_t numThreads, const uint16_t blockSize) {
    typedef typename std::iterator_traits<iter>::value_type T;

    // Caution: Iterator names get mixed here!
		const iter pivotIter = median::median_of_5(begin, begin+1, begin+(end-begin)/2, end-2, end-1, comp);
		iter midIter = myParallelPartition::parallelPartition(begin, end, pivotIter, comp, numThreads, blockSize);

		uint64_t size = end-begin, leftSide = midIter-begin;
		float ratio = leftSide > 0 ? size/leftSide : numThreads;
		uint16_t numThreadsLeft = numThreads/ratio<2 ? 1 : std::min(uint16_t(numThreads/ratio), uint16_t(numThreads-1)); // numThreads >= 2
		uint16_t numThreadsRight = numThreads-numThreadsLeft<2 ? 1 : numThreads-numThreadsLeft;
		iter beginIterR = midIter+1;
		std::thread left;
    if (begin < midIter) {
    	left = std::thread(&parallelQuickSort<iter, Comp>, begin, midIter, comp, numThreadsLeft, blockSize);
    }
    if (beginIterR < end) {
    	parallelQuickSort(beginIterR, end, comp, numThreadsRight, blockSize);
    }
		if (begin < midIter) left.join();
  }

  template<typename iter, typename Comp>
	inline void parallelQuickSort(iter begin, iter end, Comp comp, uint16_t numThreads, const uint16_t blockSize) {
		uint64_t size = end-begin;
		if (numThreads > 1 && size > 1000) { // Thresh to sequential execution
			parallelQuickSortIteration(begin, end, comp, numThreads, blockSize);
		} else {
			myQuickSort::quickSortDynIteration(begin, end, comp, blockSize);
		}
	}

	template<typename iter, typename Comp>
	inline void parallelQuickSort(iter begin, iter end, Comp comp) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		uint16_t numThreads = std::thread::hardware_concurrency();
		parallelQuickSort(begin, end, comp, numThreads, blockSize);
	}

	template<typename iter, typename Comp>
	inline void parallelQuickSortDebug(iter begin, iter end, Comp comp, uint32_t numThreads) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		parallelQuickSort(begin, end, comp, numThreads, blockSize);
	}
}

#endif
