// Constraints:
//	- Minimum elem_per_block>=PP_BLOCKSIZE elements (Because of last partitioning phase, since a whole buffer might be swapped into one block)

#include <algorithm> // Used for swap()
#include <iostream>
#include <thread> // Used for threads
#include <functional> // Used for std::ref
#include <cstring> // Used for memmove
#include <future>

#include "../../SingleThreadedQuicksort/v9/partition.cpp"
#include "../ThreadPool/ThreadPool.h"

#include "./parallelPartitionBaseDyn.cpp"


#ifndef PARALLELPARTITIONDYN_CPP
#define PARALLELPARTITIONDYN_CPP

#ifndef CACHE_SIZE
#define CACHE_SIZE 32768
#endif

#ifndef L2CACHESIZE
#define L2CACHESIZE 1048576
#endif

namespace myParallelPartition {

	template<typename iter, typename comp>
	inline void remInsertionSort(iter* remArr, uint32_t remArrSize, comp cmp) {
		uint32_t j;
		iter temp;
		for (uint32_t i=0; i<remArrSize; i+=2) {
			j = i;

			while (j>0 && cmp(remArr[j], remArr[j-2])) {
				temp = remArr[j];
				remArr[j] = remArr[j-2];
				remArr[j-2] = temp;
				temp = remArr[j+1];
				remArr[j+1] = remArr[j-1];
				remArr[j-1] = temp;
				j-=2;
			}
		}
	}

	// Invariant: Pivot swapped to end-1
	template<typename iter, typename Comp>
	inline iter sequentialPartitionRemainder(const iter& begin, const iter& end, const iter& pivot_iter, const Comp comp, const uint16_t numThreads,
			const uint64_t elemPerSide, const iter pivot_intermed, Parallel_Partitioner<iter, Comp>& partitioner, const uint16_t blockSize) {
		// Catch all equal case and stop recursion if applicable
		/*if (pivot_intermed == mid_iter || partitioner.l_cnt == 0) {
    	bool all_equal = true;
    	iter beg = begin;
    	while(beg != pivot_intermed) {
    		if (*beg!=*pivot_intermed) {
    			all_equal = false;
    			break;
    		}
    		beg++;
    	}
    	if (all_equal) {
    		return mid_iter; // TODO: Is this correct?
			}
    }*/
		// End catch all equal case

    // Sort: Close to partition mid (not mid_iter) first
    // Necessary: Swap elements close to mid block first (so they substitute their selves, just in case)
    remInsertionSort(partitioner.left_rem, numThreads*2, [](iter i1, iter i2){return i1>i2;}); // sorting: (big index) << (small index)
    remInsertionSort(partitioner.right_rem, numThreads*2, [](iter i1, iter i2){return i1<i2;}); // sorting: (small index) << (big index)

		// Neutralize remainder as far as possible
		/*if (numThreads > 1) {
			partitioner.neutralizeRemainder(pivot_intermed, comp, blockSize);
		}*/

    // Find middle block
    volatile int64_t midOffL = (partitioner.l_next-begin); // Swap from (here - block size) including
    volatile int64_t midOffR = (partitioner.r_next-begin)+1; // Swap from here including (+1: Leave last elem of mid)

    // Swap remaining blocks to middle block
    typedef typename std::iterator_traits<iter>::value_type T;
		void* swpBuff = malloc(elemPerSide*sizeof(T));
		uint64_t leftRemCnt = 0;
		uint64_t rightRemCnt = 0;
		for (uint32_t i=0; i<numThreads*2; i+=2) {
			leftRemCnt = partitioner.left_rem[i+1]-partitioner.left_rem[i];
			if (leftRemCnt>0) {
				midOffL -= leftRemCnt; // leftRemCnt == elemPerSide
				int64_t overlap = ((partitioner.left_rem[i+1])-begin)-midOffL;
				if (overlap>0) {
					leftRemCnt-=overlap;
					midOffL+=overlap;
				}
				memcpy(swpBuff, &(*partitioner.left_rem[i]), leftRemCnt*sizeof(T));
				memcpy(&(*partitioner.left_rem[i]), &(*(begin+midOffL)), leftRemCnt*sizeof(T));
				memcpy(&(*(begin+midOffL)), swpBuff, leftRemCnt*sizeof(T));
				if (overlap>0) {
					midOffL-=overlap;
				}
			}

			rightRemCnt = partitioner.right_rem[i+1]-partitioner.right_rem[i];
			if (rightRemCnt>0) {
				int64_t overlap = (midOffR+rightRemCnt)-((partitioner.right_rem[i])-begin);
				if (overlap>0) {
					rightRemCnt-=overlap;
					partitioner.right_rem[i]+=overlap;
				}
				memcpy(swpBuff, &(*partitioner.right_rem[i]), rightRemCnt*sizeof(T));
				memcpy(&(*partitioner.right_rem[i]), &(*(begin+midOffR)), rightRemCnt*sizeof(T));
				memcpy(&(*(begin+midOffR)), swpBuff, rightRemCnt*sizeof(T));
				if (overlap>0) {
					midOffR += rightRemCnt+overlap;
				} else {
					midOffR += rightRemCnt; // rightRemCnt == elemPerSide
				}
			}
		}

		//std::cout << "begin: " << midOffL << " end: " << midOffR << std::endl;
    iter mid_iter = begin+(midOffL+myPartition::partitionDynQs(begin+midOffL, begin+midOffR, pivot_intermed, comp, blockSize));

    delete[] (T*)swpBuff;


		// Reinsert removed pivot
    std::iter_swap(mid_iter, pivot_intermed);

    /*std::cout << "Mid is " << "[" << (mid_iter-begin) << "]: " << *mid_iter
    		<< "." << "\n\n";*/

		return mid_iter;
	}	

	// To be called as part of Quicksort
  template<typename iter, typename Comp>
  inline iter parallelPartition(const iter begin, const iter end, const iter pivot_iter, const Comp comp, const uint16_t numThreads, ThreadPool* pool, const uint16_t blockSize) {
		typedef typename std::iterator_traits<iter>::value_type T;
		iter pivot_intermed = end-1;
		const uint64_t minSize = (((end-begin)/numThreads)/2);
		uint64_t elemPerSide = std::min(((L2CACHESIZE/sizeof(T))/2)/numThreads, minSize/32);
    Parallel_Partitioner<iter, Comp> partitioner(numThreads, elemPerSide, begin+numThreads*elemPerSide, (pivot_intermed-1)-numThreads*elemPerSide);

    std::iter_swap(pivot_iter, pivot_intermed);

		std::future<void>* fut = new std::future<void>[numThreads-1];
    // Partition in parallel
		uint32_t l=0;
    for(; l<numThreads-1; l++) {
			fut[l] = pool->enqueue([&, l](){
				partitioner.parallel_partition(
						begin+l*elemPerSide,
						begin+(l+1)*elemPerSide,
						(pivot_intermed-1)-l*elemPerSide,
						(pivot_intermed-1)-(l+1)*elemPerSide,
						pivot_intermed,
						comp,
						l,
						blockSize);
				}
			);
    }

		// Execute last iteration on this thread
		partitioner.parallel_partition(
			begin+l*elemPerSide,
			begin+(l+1)*elemPerSide,
			(pivot_intermed-1)-l*elemPerSide,
			(pivot_intermed-1)-(l+1)*elemPerSide,
			pivot_intermed,
			comp,
			l,
			blockSize);

    for(int i=0; i<numThreads-1; i++) {
			fut[i].get(); // Wait for other threads
    }
		delete[] fut;
    // End partition in parallel

    return sequentialPartitionRemainder(begin, end, pivot_iter, comp, numThreads, elemPerSide, pivot_intermed, partitioner, blockSize);
  }

	// To be called as parallel partition
	template<typename iter, typename Comp>
  inline iter parallelPartition(const iter begin, const iter end, const iter pivot_iter, const Comp comp, const uint16_t numThreads) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		std::thread* threads = new std::thread[numThreads-1];
    iter pivot_intermed = end-1;
		const uint64_t minSize = (((end-begin)/numThreads)/2);
    uint64_t elemPerSide = std::min(((L2CACHESIZE/sizeof(T))/2)/numThreads, minSize/32);
    Parallel_Partitioner<iter, Comp> partitioner(numThreads, elemPerSide, begin+numThreads*elemPerSide, (pivot_intermed-1)-numThreads*elemPerSide);

    std::iter_swap(pivot_iter, pivot_intermed);

    // Partition in parallel
    for(int i=0; i<numThreads-1; i++) {
      threads[i] = std::thread(&Parallel_Partitioner<iter, Comp>::parallel_partition,
					&partitioner,
          begin+i*elemPerSide,
					begin+(i+1)*elemPerSide,
					(pivot_intermed-1)-i*elemPerSide,
					(pivot_intermed-1)-(i+1)*elemPerSide,
					pivot_intermed,
					comp,
					i,
					blockSize
				);
		};

		partitioner.parallel_partition(
				begin+(numThreads-1)*elemPerSide,
				begin+((numThreads-1)+1)*elemPerSide,
		 		(pivot_intermed-1)-(numThreads-1)*elemPerSide,
				(pivot_intermed-1)-((numThreads-1)+1)*elemPerSide,
				pivot_intermed,
				comp,
				(numThreads-1),
				blockSize);

    for(int i=0; i<numThreads-1; i++) {
    	threads[i].join();
    }
    // End partition in parallel

    return sequentialPartitionRemainder(begin, end, pivot_iter, comp, numThreads, elemPerSide, pivot_intermed, partitioner, blockSize);
  }

	/*// To be called as parallel partition
	template<typename iter, typename Comp>
  inline iter parallelPartitionFlex(const iter begin, const iter end, const iter pivot_iter, const Comp comp, const uint16_t numThreads, const uint16_t flex) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		std::thread* threads = new std::thread[numThreads-1];
    iter pivot_intermed = end-1;
		const uint64_t minSize = (((end-begin)/numThreads)/2);
    const uint64_t elemPerSide = std::min(((L2CACHESIZE/sizeof(T))/2)/numThreads, minSize/flex);
    Parallel_Partitioner<iter, Comp> partitioner(numThreads, elemPerSide, begin+numThreads*elemPerSide, (pivot_intermed-1)-numThreads*elemPerSide);

    std::iter_swap(pivot_iter, pivot_intermed);

    // Partition in parallel
    for(int i=0; i<numThreads-1; i++) {
      threads[i] = std::thread(&Parallel_Partitioner<iter, Comp>::parallel_partition,
					&partitioner,
          begin+i*elemPerSide,
					begin+(i+1)*elemPerSide,
					(pivot_intermed-1)-i*elemPerSide,
					(pivot_intermed-1)-(i+1)*elemPerSide,
					pivot_intermed,
					comp,
					i,
					blockSize
				);
		};

		partitioner.parallel_partition(
				begin+(numThreads-1)*elemPerSide,
				begin+((numThreads-1)+1)*elemPerSide,
		 		(pivot_intermed-1)-(numThreads-1)*elemPerSide,
				(pivot_intermed-1)-((numThreads-1)+1)*elemPerSide,
				pivot_intermed,
				comp,
				(numThreads-1),
				blockSize);

    for(int i=0; i<numThreads-1; i++) {
    	threads[i].join();
    }
    // End partition in parallel

    return sequentialPartitionRemainder(begin, end, pivot_iter, comp, numThreads, elemPerSide, pivot_intermed, partitioner, blockSize);
  }*/

	// To be called as parallel partition for our chunk size experiment
	template<typename iter, typename Comp>
	inline iter parallelPartition(const iter begin, const iter end, const iter pivot_iter, const Comp comp, const uint16_t numThreads, const uint64_t elemPerSide) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		std::thread* threads = new std::thread[numThreads-1];
		iter pivot_intermed = end-1;
		Parallel_Partitioner<iter, Comp> partitioner(numThreads, elemPerSide, begin+numThreads*elemPerSide, (pivot_intermed-1)-numThreads*elemPerSide);

		std::iter_swap(pivot_iter, pivot_intermed);

		// Partition in parallel
		for(int i=0; i<numThreads-1; i++) {
			threads[i] = std::thread(&Parallel_Partitioner<iter, Comp>::parallel_partition,
					&partitioner,
					begin+i*elemPerSide,
					begin+(i+1)*elemPerSide,
					(pivot_intermed-1)-i*elemPerSide,
					(pivot_intermed-1)-(i+1)*elemPerSide,
					pivot_intermed,
					comp,
					i,
					blockSize
				);
		};

		partitioner.parallel_partition(
				begin+(numThreads-1)*elemPerSide,
				begin+((numThreads-1)+1)*elemPerSide,
				(pivot_intermed-1)-(numThreads-1)*elemPerSide,
				(pivot_intermed-1)-((numThreads-1)+1)*elemPerSide,
				pivot_intermed,
				comp,
				(numThreads-1),
				blockSize);

		for(int i=0; i<numThreads-1; i++) {
			threads[i].join();
		}
		// End partition in parallel

		return sequentialPartitionRemainder(begin, end, pivot_iter, comp, numThreads, elemPerSide, pivot_intermed, partitioner, blockSize);
	}
}

#endif
