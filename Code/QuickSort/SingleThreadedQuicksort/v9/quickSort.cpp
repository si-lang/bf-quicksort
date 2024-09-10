#include <algorithm> // Used for swap()
#include <iostream>

#include "partition.cpp"

#include "../../../Profiling/EdelkampWeiss/blocked.h++"
#include "../../../Profiling/EdelkampWeiss/median.h"
#include "../../../Profiling/EdelkampWeiss/insertionsort.h"
#include "../../../Profiling/EdelkampWeiss/partition.h"


#ifndef QUICKSORT_CPP
#define QUICKSORT_CPP

#ifndef THRESH
#define THRESH 20
#endif

// Mal Assembler für g++ und clang ansehen
// Assembler in MT zeigen + CPU-Statistik
// Aufschlüsseln innere Schleife und Swapping
// Zeiten aufschlüsseln
// Swap mal raus nehmen
// Erklären, warum AllEqual Inputs schlecht waren (lCnt ist dann immer size-1)


namespace myQuickSort {

	template<typename iter, typename Comp>
  inline void improvedQuickSort(iter begin, iter end, Comp comp);

  template<typename iter, typename Comp>
  inline void improvedQuickSortIteration(iter begin, iter end, Comp comp, const uint16_t blockSize) {
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

    const iter endIterL = midIter;
    const iter beginIterR = midIter+1;

		if (begin < endIterL) {
			if ((endIterL-begin) > THRESH) {
				improvedQuickSortIteration(begin, endIterL, comp, blockSize);
			} else {
				insertionsort::insertion_sort(begin, endIterL, comp);
			}
		}
		if (beginIterR < end) {
			if ((end-beginIterR) > THRESH) {
				improvedQuickSortIteration(beginIterR, end, comp, blockSize);
			} else {
				insertionsort::insertion_sort(beginIterR, end, comp);
			}
		}
  }

  template<typename iter, typename Comp>
	inline void improvedQuickSort(iter begin, iter end, Comp comp) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		improvedQuickSortIteration(begin, end, comp, blockSize);
	}




  /*
   * My quick sort implementation
   */
  template<typename iter, typename Comp>
  inline void quickSort(iter begin, iter end, Comp comp);


  template<typename iter, typename Comp>
  inline void quickSortIteration(iter begin, iter end, Comp comp) {
    typedef typename std::iterator_traits<iter>::value_type T;
    iter pivotIntermed = end-1;

		iter midIter;
		if ((end-begin) > 1024) {
			// Caution: Iterator names get mixed here!
    	//const iter pivotIter = median::median_of_3(begin, end, comp);//myMedian::medianOfThree(begin, lastElemIter);
			const iter pivotIter = median::median_of_5(begin, begin+1, begin+(end-begin)/2, end-2, end-1, comp);
    	std::iter_swap(pivotIter, pivotIntermed);
    	midIter = begin+myPartition::partition(begin, pivotIntermed, pivotIntermed, comp);
			std::iter_swap(midIter, pivotIntermed);
			// Iterators naming consistent again
		} else {
			midIter = partition::Hoare_block_partition<iter, Comp>::partition(begin, end, comp);
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

    const iter endIterL = midIter;
    const iter beginIterR = midIter+1;

		// Process small partition first
		//if ((endIterL-begin)<(beginIterR-begin)) {
		if (begin < endIterL) {
			quickSort(begin, endIterL, comp);
		}
		if (beginIterR < end) {
			quickSort(beginIterR, end, comp);
		}
		/*} else {
			if (beginIterR < end) {
				quickSort(beginIterR, end, comp);
			}
			if (begin < endIterL) {
				quickSort(begin, endIterL, comp);
			}
		}*/
  }

  template<typename iter, typename Comp>
	inline void quickSort(iter begin, iter end, Comp comp) {
    /*if ((end-begin) > 1000) {
      quickSortIteration(begin, end, comp);
    } else {
      blocked::sort(begin, end, comp);
    }*/

		if ((end-begin) > THRESH) {
      quickSortIteration(begin, end, comp);
    } else {
      insertionsort::insertion_sort(begin, end, comp);
    }
	}


	  
	/*
   * My dynamic quick sort implementation
   */
  template<typename iter, typename Comp>
  inline void quickSortDyn(iter begin, iter end, Comp comp);


  template<typename iter, typename Comp>
  inline void quickSortDynIteration(iter begin, iter end, Comp comp, const uint16_t blockSize) {
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

    const iter endIterL = midIter;
    const iter beginIterR = midIter+1;

		// Process small partition first
		//if ((endIterL-begin)<(beginIterR-begin)) {
		if (begin < endIterL) {
			if ((endIterL-begin) > THRESH) {
				quickSortDynIteration(begin, endIterL, comp, blockSize);
			} else {
				insertionsort::insertion_sort(begin, endIterL, comp);
			}
		}
		if (beginIterR < end) {
			if ((end-beginIterR) > THRESH) {
				quickSortDynIteration(beginIterR, end, comp, blockSize);
			} else {
				insertionsort::insertion_sort(beginIterR, end, comp);
			}
		}
		/*} else {
			if (beginIterR < end) {
				if ((end-beginIterR) > THRESH) {
					quickSortDynIteration(beginIterR, end, comp, blockSize);
				} else {
					insertionsort::insertion_sort(beginIterR, end, comp);
				}
			}
			if (begin < endIterL) {
				if ((endIterL-begin) > THRESH) {
					quickSortDynIteration(begin, endIterL, comp, blockSize);
				} else {
					insertionsort::insertion_sort(begin, endIterL, comp);
				}
			}
		}*/
  }

  template<typename iter, typename Comp>
	inline void quickSortDyn(iter begin, iter end, Comp comp) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		quickSortDynIteration(begin, end, comp, blockSize);
	}


	/*
   * My dynamic quick sort implementation with duplicate check
   */
  template<typename iter, typename Comp>
  inline void quickSortDynDC(iter begin, iter end, Comp comp);


  template<typename iter, typename Comp>
  inline void quickSortDynDCIteration(iter begin, iter end, Comp comp, const uint16_t blockSize) {
    iter pivotIntermed = end-1;
		iter midIter;
		uint64_t pivotLength = 0;

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
    	//const iter pivotIter = median::median_of_3(begin, end, comp);//myMedian::medianOfThree(begin, lastElemIter);
			std::iter_swap(pivotIter, pivotIntermed);
    	midIter = begin+myPartition::partitionDynQsDC(begin, pivotIntermed, pivotIntermed, comp, blockSize, pivotLength);
			std::iter_swap(midIter, pivotIntermed);
			// Iterators naming consistent again
			pivotLength+=1; // Because we swaped a pivot out and back in
		} else {
			midIter = myPartition::hoare_block_partition_unroll_loop(begin+1, end-1, pivotIter, comp, pivotLength);
		}

		/*// Catch all equal case
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
    }*/

    const iter endIterL = midIter;
    const iter beginIterR = midIter+pivotLength;

		// Process small partition first
		//if ((endIterL-begin)<(beginIterR-begin)) {
		if (begin < endIterL) {
			if ((endIterL-begin) > THRESH) {
				quickSortDynDCIteration(begin, endIterL, comp, blockSize);
			} else {
				insertionsort::insertion_sort(begin, endIterL, comp);
			}
		}
		if (beginIterR < end) {
			if ((end-beginIterR) > THRESH) {
				quickSortDynDCIteration(beginIterR, end, comp, blockSize);
			} else {
				insertionsort::insertion_sort(beginIterR, end, comp);
			}
		}
		/*} else {
			if (beginIterR < end) {
				if ((end-beginIterR) > THRESH) {
					quickSortDynDCIteration(beginIterR, end, comp, blockSize);
				} else {
					insertionsort::insertion_sort(beginIterR, end, comp);
				}
			}
			if (begin < endIterL) {
				if ((endIterL-begin) > THRESH) {
					quickSortDynDCIteration(begin, endIterL, comp, blockSize);
				} else {
					insertionsort::insertion_sort(begin, endIterL, comp);
				}
			}
		}*/
  }

  template<typename iter, typename Comp>
	inline void quickSortDynDC(iter begin, iter end, Comp comp) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		quickSortDynDCIteration(begin, end, comp, blockSize);
	}


	/*
   * My dynamic quick sort implementation without pattern detector
   */
  template<typename iter, typename Comp>
  inline void quickSortDynNoPD(iter begin, iter end, Comp comp);


  template<typename iter, typename Comp>
  inline void quickSortDynNoPDIteration(iter begin, iter end, Comp comp, const uint16_t blockSize) {
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
		/*if (midIter == pivotIntermed || begin == midIter) { // Need this really bad e.g. for uint8_t
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
    }*/

    const iter endIterL = midIter;
    const iter beginIterR = midIter+1;

		// Process small partition first
		//if ((endIterL-begin)<(beginIterR-begin)) {
		if (begin < endIterL) {
			if ((endIterL-begin) > THRESH) {
				quickSortDynNoPDIteration(begin, endIterL, comp, blockSize);
			} else {
				insertionsort::insertion_sort(begin, endIterL, comp);
			}
		}
		if (beginIterR < end) {
			if ((end-beginIterR) > THRESH) {
				quickSortDynNoPDIteration(beginIterR, end, comp, blockSize);
			} else {
				insertionsort::insertion_sort(beginIterR, end, comp);
			}
		}
  }

  template<typename iter, typename Comp>
	inline void quickSortDynNoPD(iter begin, iter end, Comp comp) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		quickSortDynNoPDIteration(begin, end, comp, blockSize);
	}


	/*
   * My dynamic quick sort implementation for pivot selection experiment
   */
  template<typename iter, typename Comp, typename Piv>
  inline void quickSortDynPivSel(iter begin, iter end, Comp comp, Piv piv);


  template<typename iter, typename Comp, typename Piv>
  inline void quickSortDynIterationPivSel(iter begin, iter end, Comp comp, const uint16_t blockSize, Piv piv) {
    iter pivotIntermed = end-1;
		iter midIter;

		const iter pivotIter = piv(begin, end);
		if ((end-begin) > 1024) {
			// Caution: Iterator names get mixed up here!
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

    const iter endIterL = midIter;
    const iter beginIterR = midIter+1;

		// Process small partition first
		//if ((endIterL-begin)<(beginIterR-begin)) {
		if (begin < endIterL) {
			if ((endIterL-begin) > THRESH) {
				quickSortDynIterationPivSel(begin, endIterL, comp, blockSize, piv);
			} else {
				insertionsort::insertion_sort(begin, endIterL, comp);
			}
		}
		if (beginIterR < end) {
			if ((end-beginIterR) > THRESH) {
				quickSortDynIterationPivSel(beginIterR, end, comp, blockSize, piv);
			} else {
				insertionsort::insertion_sort(beginIterR, end, comp);
			}
		}
		/*} else {
			if (beginIterR < end) {
				if ((end-beginIterR) > THRESH) {
					quickSortDynIterationPivSel(beginIterR, end, comp, blockSize, piv);
				} else {
					insertionsort::insertion_sort(beginIterR, end, comp);
				}
			}
			if (begin < endIterL) {
				if ((endIterL-begin) > THRESH) {
					quickSortDynIterationPivSel(begin, endIterL, comp, blockSize, piv);
				} else {
					insertionsort::insertion_sort(begin, endIterL, comp);
				}
			}
		}*/
  }

  template<typename iter, typename Comp, typename Piv>
	inline void quickSortDynPivSel(iter begin, iter end, Comp comp, Piv piv) {
  	typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
    quickSortDynIterationPivSel(begin, end, comp, blockSize, piv);
	}


	/*
   * My dynamic quick sort implementation WITH branches
   */
  template<typename iter, typename Comp>
  inline void quickSortDynBRANCHES(iter begin, iter end, Comp comp);


  template<typename iter, typename Comp>
  inline void quickSortDynBRANCHESIteration(iter begin, iter end, Comp comp, const uint16_t blockSize) {
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

		// Caution: Iterator names get mixed here!
		std::iter_swap(pivotIter, pivotIntermed);
		if ((end-begin) > 1024) {
    	midIter = begin+myPartition::partitionDynQsBRANCHES(begin, pivotIntermed, pivotIntermed, comp, blockSize);
		} else {
			midIter = std::partition(begin, pivotIntermed, [&comp, &pivotIntermed](T v1){ return comp(v1, *pivotIntermed); });
		}
		std::iter_swap(midIter, pivotIntermed);
		// Iterators naming consistent again

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

    const iter endIterL = midIter;
    const iter beginIterR = midIter+1;

		// Process small partition first
		//if ((endIterL-begin)<(beginIterR-begin)) {
		if (begin < endIterL) {
			if ((endIterL-begin) > THRESH) {
				quickSortDynBRANCHESIteration(begin, endIterL, comp, blockSize);
			} else {
				insertionsort::insertion_sort(begin, endIterL, comp);
			}
		}
		if (beginIterR < end) {
			if ((end-beginIterR) > THRESH) {
				quickSortDynBRANCHESIteration(beginIterR, end, comp, blockSize);
			} else {
				insertionsort::insertion_sort(beginIterR, end, comp);
			}
		}
		/*} else {
			if (beginIterR < end) {
				if ((end-beginIterR) > THRESH) {
					quickSortDynBRANCHESIteration(beginIterR, end, comp, blockSize);
				} else {
					insertionsort::insertion_sort(beginIterR, end, comp);
				}
			}
			if (begin < endIterL) {
				if ((endIterL-begin) > THRESH) {
					quickSortDynBRANCHESIteration(begin, endIterL, comp, blockSize);
				} else {
					insertionsort::insertion_sort(begin, endIterL, comp);
				}
			}
		}*/
  }

  template<typename iter, typename Comp>
	inline void quickSortDynBRANCHES(iter begin, iter end, Comp comp) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		quickSortDynBRANCHESIteration(begin, end, comp, blockSize);
	}


	// /*
  //  * My dynamic quick sort implementation for pivot selection experiment 2
  //  */
  // template<typename iter, typename Comp>
  // inline void quickSortDynPivSelT(iter begin, iter end, Comp comp, uint32_t thresh);


  // template<typename iter, typename Comp>
  // inline void quickSortDynIterationPivSelT(iter begin, iter end, Comp comp, uint32_t thresh) {
  //   typedef typename std::iterator_traits<iter>::value_type T;
  //   iter pivotIntermed = end-1;
  //   uint64_t lCnt = 0;
	// 	iter midIter;

  //   // Caution: Iterator names get mixed here!
  //   //const iter pivotIter = median::median_of_3(begin, end, comp);//myMedian::medianOfThree(begin, lastElemIter);
	// 	iter pivotIter;

	// 	// Stage 1
	// 	if ((end-begin)<thresh) {
	// 		pivotIter = median::median_of_3(begin, end, comp);
	// 	} else {
	// 		pivotIter = median::median_of_5(begin, begin+1, begin+(end-begin)/2, end-2, end-1, comp);
	// 	}

	// 	// Stage 2
	// 	// if ((end-begin)<thresh) {
	// 	// 	pivotIter = median::median_of_5(begin, begin+1, begin+(end-begin)/2, end-2, end-1, comp);
	// 	// } else {
	// 	// 	pivotIter = median::median_of_3_medians_of_3(begin, end, comp);
	// 	// }

	// 	// Stage 3
	// 	if ((end-begin)<thresh) {
	// 		pivotIter = median::median_of_5(begin, begin+1, begin+(end-begin)/2, end-2, end-1, comp);
	// 	} else {
	// 		unsigned int pivot_sample_size = sqrt(end-begin);
	// 		pivot_sample_size += (1-(pivot_sample_size%2));//make it an odd number
	// 		pivotIter = median::median_of_k(begin, end, comp, pivot_sample_size); //choose pivot as median of sqrt(n)
	// 	}

  //   std::iter_swap(pivotIter, pivotIntermed);

	// 	if ((end-begin)<1024) {
  //   	lCnt = myPartition::partitionDyn(begin, pivotIntermed, pivotIntermed, comp);
	// 		midIter = begin+lCnt;
	// 	} else {
	// 		midIter = std::partition(begin, pivotIntermed, [&comp, &pivotIntermed](T v){ return comp(v, *pivotIntermed); });
	// 	}

	// 	// Catch all equal case
	// 	if (pivotIntermed == midIter || begin == midIter) {
  //   	bool allEqual = true;
	// 		iter beg = begin;
  //   	while(beg != pivotIntermed) {
  //   		if (*beg!=*pivotIntermed) {
  //   			allEqual = false;
  //   			break;
  //   		}
  //   		beg++;
  //   	}
  //   	if (allEqual) {
  //   		return;
	// 		}
  //   }

  //   std::iter_swap(midIter, pivotIntermed);
  //   // Iterators naming consistent again

  //   const iter endIterL = midIter;
  //   const iter beginIterR = midIter+1;

  //   if (begin < endIterL) {
  //     quickSortDynPivSelT(begin, endIterL, comp, thresh);
  //   }
  //   if (beginIterR < end) {
  //     quickSortDynPivSelT(beginIterR, end, comp, thresh);
  //   }
  // }

  // template<typename iter, typename Comp>
	// inline void quickSortDynPivSelT(iter begin, iter end, Comp comp, uint32_t thresh) {
  //   if ((end-begin) > 16) {
  //     quickSortDynIterationPivSelT(begin, end, comp, thresh);
  //   } else {
  //     insertionsort::insertion_sort(begin, end, comp);
  //   }
	// }




	/*
   * Conforming to Tuned Quick Sort of Elmasry, Katajainen and Stenmark.
	 * Available at http://www.diku.dk/~jyrki/Myris/Kat2014S.html
	 * Implementation adapted from Edelkamp&Weiss.
   */
	 template<typename iter, typename Compare>
	 inline void tunedQuickSort(iter begin, iter end, Compare less) {
		typedef typename std::iterator_traits<iter>::value_type T; //
		const uint16_t block_size = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512; //
 		const int depth_limit = 2 * ilogb((double)(end - begin)) + 3;
 		iter stack[80];
 		iter* s = stack;
 		int depth_stack[40];
 		int depth = 0;
 		int* d_s_top = depth_stack;
		long long int diff = 0; //
 		*s = begin;
 		*(s + 1) = end;
 		s += 2;
 		*d_s_top = 0;
 		++d_s_top;
 		do {
			diff = end-begin; //
 			if (depth < depth_limit && diff > THRESH) {
 				iter pivot;

				// Pivot selection
				iter pivotIter;
				if (end-begin > 160000) {
					unsigned int pivot_sample_size = sqrt(end-begin);
					pivot_sample_size += (1 - (pivot_sample_size % 2));// Make it an odd number
					pivotIter = median::median_of_k(begin, end, less, pivot_sample_size); // Choose pivot as median of sqrt(n)
				} else {
					if (end-begin > 4000)
						pivotIter = median::median_of_5_medians_of_5(begin, end, less);
					else if (end-begin > 2000)
						pivotIter = median::median_of_3_medians_of_3(begin, end, less);
					else
						pivotIter = median::median_of_3(begin, end, less);
				}

				if (diff > 1024) { // Current best with 2024
					std::iter_swap(pivotIter, end-1);
					pivot = (begin+myPartition::partitionDynQs(begin, end-1, end-1, less, block_size));
					std::iter_swap(pivot, end-1);
				} else {
					/*std::iter_swap(pivotIter, end-1);
					T piv = *(end-1);
					pivot = std::partition(begin, end-1, [&less, &piv](T v){ return piv>v; });
					std::iter_swap(pivot, end-1);*/
 					pivot = myPartition::hoare_block_partition_unroll_loop(begin+1, end-1, pivotIter, less);
				}

 				if (pivot - begin > end - pivot) {
 					*s = begin;
 					*(s + 1) = pivot;
 					begin = pivot + 1;
 				} else {
 					*s = pivot + 1;
 					*(s + 1) = end;
 					end = pivot;
 				}
 				s += 2;
 				depth++;
 				*d_s_top = depth;
 				++d_s_top;
 			} else {
 				if (end - begin > THRESH) { // if recursion depth limit exceeded
 					std::partial_sort(begin, end, end);
 				} else {
 					insertionsort::insertion_sort(begin, end, less); // copy of std::__insertion_sort (GCC 4.7.2)
				}
 				//pop new subarray from stack
 				s -= 2;
 				begin = *s;
 				end = *(s + 1);
 				--d_s_top;
 				depth = *d_s_top;
 			}
 		} while (s != stack);
 	}
	
	template<typename iter, typename Compare>
	inline void tunedQuickSortDC(iter begin, iter end, Compare less) {
		typedef typename std::iterator_traits<iter>::value_type T; //
		const uint16_t block_size = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512; //
		const int depth_limit = 2 * ilogb((double)(end - begin)) + 3;
		iter stack[80];
		iter* s = stack;
		int depth_stack[40];
		int depth = 0;
		int* d_s_top = depth_stack;
		long long int diff = 0; //
		*s = begin;
		*(s + 1) = end;
		s += 2;
		*d_s_top = 0;
		++d_s_top;
		do {
			diff = end-begin; //
			if (depth < depth_limit && diff > THRESH) {
 				iter pivot;
				uint64_t pivot_length = 1;

				// Pivot selection
				iter pivotIter;
				if (end-begin > 160000) {
					unsigned int pivot_sample_size = sqrt(end-begin);
					pivot_sample_size += (1 - (pivot_sample_size % 2));// Make it an odd number
					pivotIter = median::median_of_k(begin, end, less, pivot_sample_size); // Choose pivot as median of sqrt(n)
				} else {
					if (end-begin > 4000)
						pivotIter = median::median_of_5_medians_of_5(begin, end, less);
					else if (end-begin > 2000)
						pivotIter = median::median_of_3_medians_of_3(begin, end, less);
					else
						pivotIter = median::median_of_3(begin, end, less);
				}

				if (diff > 1024) {
					std::iter_swap(pivotIter, end-1);
					pivot = (begin+myPartition::partitionDynQsDC(begin, end-1, end-1, less, block_size, pivot_length));
					std::iter_swap(pivot, end-1);
				} else {
 					pivot = myPartition::hoare_block_partition_unroll_loop(begin+1, end-1, pivotIter, less, pivot_length);
				}
				
				if (pivot - begin > end - pivot) {
					*s = begin;
					*(s + 1) = pivot;
					begin = pivot + pivot_length;
				} else {
					*s = pivot + pivot_length;
					*(s + 1) = end;
					end = pivot;
				}
				s += 2;
				depth++;
				*d_s_top = depth;
				++d_s_top;
			} else {
				if (end - begin > THRESH) {
					std::partial_sort(begin, end, end);
				} else {
					insertionsort::insertion_sort(begin, end, less); // copy of std::__insertion_sort (GCC 4.7.2)
				}
				s -= 2;
				begin = *s;
				end = *(s + 1);
				--d_s_top;
				depth = *d_s_top;
			}
		} while (s != stack);
	}
}
#endif
