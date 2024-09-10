#include <algorithm> // Used for swap()
#include <iostream>
#include <thread> // Used für threads
#include <functional> // Used for std::ref
#include <mutex> // Used std::mutex
#include <atomic> // Used for std::atomic<std::uint64_t>
#include <cstring> // Used for memmove
#include <future>

#include "parallelPartition.cpp"
#include "../../SingleThreadedQuicksort/v9/quickSort.cpp"

#include "../../../Profiling/EdelkampWeiss/blocked.h++"
#include "../../../Profiling/EdelkampWeiss/median.h"
#include "../../../Profiling/EdelkampWeiss/insertionsort.h"
#include "../../../Profiling/EdelkampWeiss/partition.h"
#include "../ThreadPool/ThreadPool.h"


#ifndef PARALLEL_QUICKSORT_CPP
#define PARALLEL_QUICKSORT_CPP


namespace myQuickSort {
	template<typename iter, typename Comp>
  inline void sequentialQuickSort(iter begin, iter end, Comp comp, ThreadPool* pool);

  template<typename iter, typename Comp>
  inline void sequentialQuickSortIteration(iter begin, iter end, Comp comp, ThreadPool* pool) {
		typedef typename std::iterator_traits<iter>::value_type T;
    iter pivotIntermed = end-1;
    uint64_t lCnt = 0;

    // Caution: Iterator names get mixed here!
    //const iter pivotIter = median::median_of_3(begin, end, comp);//myMedian::medianOfThree(begin, lastElemIter);
		const iter pivotIter = median::median_of_5(begin, begin+1, begin+(end-begin)/2, end-2, end-1, comp);
    std::iter_swap(pivotIter, pivotIntermed);

    lCnt = myPartition::qsBlockPartition(begin, pivotIntermed, pivotIntermed, comp);

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
			pool->enqueue([=](){sequentialQuickSort(begin, midIter, comp, pool);});
      //sequentialQuickSort(begin, midIter, comp, pool);
    }
    if (beginIterR < end) {
      sequentialQuickSort(beginIterR, end, comp, pool);
    }
  }

  template<typename iter, typename Comp>
	inline void sequentialQuickSort(iter begin, iter end, Comp comp, ThreadPool* pool) {
    if ((end-begin) > 100) {
			sequentialQuickSortIteration(begin, end, comp, pool);
    } else {
      blocked::sort(begin, end, comp);
    }
	}


	template<typename iter, typename Comp>
	inline void parallelQuickSort(iter begin, iter end, Comp comp, uint16_t numThreads, ThreadPool* pool);

  template<typename iter, typename Comp>
  inline void parallelQuickSortIteration(iter begin, iter end, Comp comp, uint16_t numThreads, ThreadPool* pool) {
    typedef typename std::iterator_traits<iter>::value_type T;

    // Caution: Iterator names get mixed here!
    //const iter pivotIter = median::median_of_3(begin, end, comp);//myMedian::medianOfThree(begin, lastElemIter);
		const iter pivotIter = median::median_of_5(begin, begin+1, begin+(end-begin)/2, end-2, end-1, comp);
		iter midIter = myPartition::parallelPartition(begin, end, pivotIter, comp, numThreads, pool);

		float ratio = (end-begin)/(pivotIter-begin);
		uint16_t numThreadsLeft = numThreads/ratio<2 ? 1 : numThreads/ratio;
		uint16_t numThreadsRight = numThreads-numThreadsLeft<2 ? 1 : numThreads-numThreadsLeft;
		iter beginIterR = midIter+1;
		//std::future<void> left;
    if (begin < midIter) {
    	pool->enqueue([=](){parallelQuickSort(begin, midIter, comp, numThreadsLeft, pool);});
    }
    if (beginIterR < end) {
    	parallelQuickSort(beginIterR, end, comp, numThreadsRight, pool);
    }
		//left.get();
  }

  template<typename iter, typename Comp>
	inline void parallelQuickSort(iter begin, iter end, Comp comp, uint16_t numThreads, ThreadPool* pool) {
		uint64_t size = end-begin;
		if (numThreads > 1 && size < 1000) { // TODO: 1000
			parallelQuickSortIteration(begin, end, comp, numThreads, pool);
		} else {
			sequentialQuickSortIteration(begin, end, comp, pool);
		}
	}

	template<typename iter, typename Comp>
	inline void parallelQuickSort(iter begin, iter end, Comp comp) {
		uint16_t numThreads = std::thread::hardware_concurrency();
		ThreadPool* pool = new ThreadPool(numThreads);
		auto q = pool->enqueue([=](){parallelQuickSort(begin, end, comp, numThreads, pool);});
		//q.get();
		pool->waitUntilComplete();
		delete pool;
	}

	template<typename iter, typename Comp>
	inline void parallelQuickSortDebug(iter begin, iter end, Comp comp, uint32_t numThreads) {
		ThreadPool* pool = new ThreadPool(numThreads);
		auto q = pool->enqueue([&](){parallelQuickSort(begin, end, comp, numThreads, pool);});
		//q.get();
		pool->waitUntilComplete();
		delete pool;
	}
}

#endif
