#include <algorithm> // Used for swap()
#include <iostream>
#include <thread> // Used für threads
#include <functional> // Used for std::ref
#include <mutex> // Used std::mutex
#include <atomic> // Used for std::atomic<std::uint64_t>
#include <cstring> // Used for memmove
#include <future>

#include "../ThreadPool/ThreadPool.h"
#include "parallelPartitionDyn.cpp"
#include "../../SingleThreadedQuicksort/v9/quickSort.cpp"

#include "../../../Profiling/EdelkampWeiss/median.h"
#include "../../../Profiling/EdelkampWeiss/insertionsort.h"
#include "../../../Profiling/EdelkampWeiss/partition.h"


#ifndef PARALLEL_QUICKSORT_DYN_CPP
#define PARALLEL_QUICKSORT_DYN_CPP


namespace myParallelQuickSort {
  template<typename iter, typename Comp>
  inline void sequentialQuickSort(iter begin, iter end, Comp comp, ThreadPool* pool, const uint16_t blockSize) {
		iter pivotIntermed = end-1;
		iter midIter;

		// Pivot selection
		iter pivotIter;
		if (end-begin > 160000) {
			unsigned int pivot_sample_size = sqrt(end-begin);
			pivot_sample_size += (1 - (pivot_sample_size % 2));// Make it an odd number
			pivotIter = median::median_of_k(begin, end, comp, pivot_sample_size); // Choose pivot as median of sqrt(n)
		} else {
			if (end-begin > 4000)
				pivotIter = median::median_of_5_medians_of_5(begin, end, comp);
			else if (end-begin > 2000)
				pivotIter = median::median_of_3_medians_of_3(begin, end, comp);
			else
				pivotIter = median::median_of_3(begin, end, comp);
		}

		if ((end-begin) > 1024) {
			// Caution: Iterator names get mixed here!
			std::iter_swap(pivotIter, pivotIntermed);
    	midIter = begin+myPartition::partitionDynQs(begin, pivotIntermed, pivotIntermed, comp, blockSize);
			std::iter_swap(midIter, pivotIntermed);
					// Iterators naming consistent again
		} else {
			midIter = myPartition::hoare_block_partition_unroll_loop(begin+1, end-1, pivotIter, comp);
		}

		// Catch all equal case
		if (midIter == pivotIntermed || begin == midIter) { // Need this really bad e.g. for uint8_t
    	bool allEqual = true;
			iter beg = begin;
    	while(beg != pivotIntermed) {
    		if (*beg!=*midIter) {
    			allEqual = false;
    			break;
    		}
    		beg++;
    	}
    	if (allEqual) {
    		return;
			}
    }

    const iter beginIterR = midIter+1;
		if (begin < midIter) {
			if ((midIter-begin) > THRESH) {
				pool->enqueue([=](){sequentialQuickSort(begin, midIter, comp, pool, blockSize);});
			} else {
				pool->enqueue([=](){insertionsort::insertion_sort(begin, midIter, comp);});
			}
		}
		if (beginIterR < end) {
			if ((end-beginIterR) > THRESH) {
				sequentialQuickSort(beginIterR, end, comp, pool, blockSize);
			} else {
				insertionsort::insertion_sort(beginIterR, end, comp);
			}
		}
  }


	template<typename iter, typename Comp>
	inline void parallelQuickSort(iter begin, iter end, Comp comp, uint16_t numThreads, ThreadPool* pool);

  template<typename iter, typename Comp>
  inline void parallelQuickSortIteration(iter begin, iter end, Comp comp, uint16_t numThreads, ThreadPool* pool, const uint16_t blockSize) {
		if (end-begin<1000) sequentialQuickSort(begin, end, comp, pool, blockSize); // Critical! 10000?

    typedef typename std::iterator_traits<iter>::value_type T;
		iter pivotIntermed = end-1;
		iter midIter;

		// Pivot selection
		iter pivotIter;
		if (end-begin > 160000) {
			unsigned int pivot_sample_size = sqrt(end-begin);
			pivot_sample_size += (1 - (pivot_sample_size % 2));// Make it an odd number
			pivotIter = median::median_of_k(begin, end, comp, pivot_sample_size); // Choose pivot as median of sqrt(n)
		} else {
			if (end-begin > 4000)
				pivotIter = median::median_of_5_medians_of_5(begin, end, comp);
			else if (end-begin > 2000)
				pivotIter = median::median_of_3_medians_of_3(begin, end, comp);
			else
				pivotIter = median::median_of_3(begin, end, comp);
		}

		midIter = myParallelPartition::parallelPartition(begin, end, pivotIter, comp, numThreads, pool, blockSize);
		// TODO: Fix midIter bug => Always 2?

		float ratio = (end-begin)/(pivotIter-begin);
		uint16_t numThreadsLeft = numThreads/ratio<2 ? 1 : numThreads/ratio;
		uint16_t numThreadsRight = numThreads-numThreadsLeft<2 ? 1 : numThreads-numThreadsLeft;
		iter beginIterR = midIter+1;
    if (begin < midIter) {
			if (numThreadsLeft > 1)
    		pool->enqueue([=](){parallelQuickSortIteration(begin, midIter, comp, numThreadsLeft, pool, blockSize);});
			else
				pool->enqueue([=](){sequentialQuickSort(begin, end, comp, pool, blockSize);});
    }
    if (beginIterR < end) {
			if (numThreads > 1)
    		parallelQuickSortIteration(beginIterR, end, comp, numThreadsRight, pool, blockSize);
			else
				sequentialQuickSort(beginIterR, end, comp, pool, blockSize);
    }
  }

	template<typename iter, typename Comp>
	inline void parallelQuickSort(iter begin, iter end, Comp comp) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		uint16_t numThreads = std::thread::hardware_concurrency();
		ThreadPool* pool = new ThreadPool(numThreads);
		pool->enqueue([=](){parallelQuickSortIteration(begin, end, comp, numThreads, pool, blockSize);});
		pool->waitUntilComplete();
		delete pool;
	}

	template<typename iter, typename Comp>
	inline void parallelQuickSortDebug(iter begin, iter end, Comp comp, uint32_t numThreads) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		ThreadPool* pool = new ThreadPool(numThreads);
		pool->enqueue([=](){parallelQuickSortIteration(begin, end, comp, numThreads, pool, blockSize);}); // Changed & to = !
		pool->waitUntilComplete();
		delete pool;
	}
}

#endif
