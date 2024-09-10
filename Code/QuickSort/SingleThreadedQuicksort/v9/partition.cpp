#include <stdint.h>
#include <algorithm> // Used for swap()
#include "../../../Profiling/EdelkampWeiss/median.h"
#include <iostream>

#ifndef PARTITION_CPP
#define PARTITION_CPP

#ifndef CACHE_SIZE
#define CACHE_SIZE 32768
#endif

#ifndef MYBLOCKSIZE
#define MYBLOCKSIZE 512
#endif

#ifndef MYUNROLLTHRESH
#define MYUNROLLTHRESH MYBLOCKSIZE-15
#endif

#ifndef MYIMPROVEDBLOCKSIZE
#define MYIMPROVEDBLOCKSIZE 512
#endif

#ifndef MYIMPROVEDUNROLLTHRESH
#define MYIMPROVEDUNROLLTHRESH MYIMPROVEDBLOCKSIZE-15
#endif

#ifndef checkAndAdvanceToLeft
#define checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc) {\
	ri--;\
	rResult = pred(*ri, pivot);\
	toLeft[lc] = ri;\
	lc += rResult;\
}
#endif

#ifndef checkAndAdvanceToRight
#define checkAndAdvanceToRight(li, lResult, pivot, toRight, rc) {\
	lResult = !pred(*li, pivot);\
	toRight[rc] = li;\
	rc += lResult;\
	li++;\
}
#endif

#ifndef checkAndAdvanceToLeftBRANCHES
#define checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc) {\
	ri--;\
	if (pred(*ri, pivot)) {\
		toLeft[lc] = ri;\
		lc++;\
	}\
}
#endif

#ifndef checkAndAdvanceToRightBRANCHES
#define checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc) {\
	if (!pred(*li, pivot)) {\
		toRight[rc] = li;\
		rc++;\
	}\
	li++;\
}
#endif

#ifndef performSwap
#define performSwap(toLeft, toRight, off) {\
	*toLeft[off-1] = *toRight[off];\
	*toRight[off] = *toLeft[off];\
	off++;\
}
#endif

namespace myPartition {

	template<typename iter, typename BinaryPredicate>
  uint64_t improvedQSBlockPartition(const iter beginIter, const iter endIter,
      iter pivotIter, BinaryPredicate pred) {
		
		typedef typename std::iterator_traits<iter>::value_type T;
 		T pivot = *pivotIter; // Pivot might get swapped
		T temp;
 		iter toLeft[MYIMPROVEDBLOCKSIZE];
 		iter toRight[MYIMPROVEDBLOCKSIZE];
 		uint64_t lc;
 		uint64_t rc;

 		bool rResult = 0;
 		bool lResult = 0;
 		iter ri = endIter;
 		iter li = beginIter;
 		long long int lCnt = 0; //TODO Type
 		uint64_t off = 0;

 		while(li<ri) {
 			lc = 0;
 			rc = 0;

 			while((lc<MYIMPROVEDUNROLLTHRESH)&&(li<(ri-15))) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			while((lc<MYIMPROVEDBLOCKSIZE)&&(li<ri)) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			lCnt += lc-(li-beginIter);

 			while((rc<MYIMPROVEDUNROLLTHRESH)&&((li+15)<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			while((rc<MYIMPROVEDBLOCKSIZE)&&(li<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			lCnt += (li-beginIter)-rc;

 			uint64_t minc = std::min(lc, rc);
			off = 0;
			if (minc > 0) {
				temp = *toRight[off];
				*toRight[off] = *toLeft[off];
				off++;
				while (off<minc) {
					*toLeft[off-1] = *toRight[off];
					*toRight[off] = *toLeft[off];
					off++;
				}
				*toLeft[off-1] = temp;
			}
 		}

 		if (lc-off > 0) {
 			uint64_t i = off;
 			const iter lMidIter = beginIter+lCnt-1;
 			iter swpIter = lMidIter;
 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
 				if (!pred(*(swpIter), pivot)) {
 					std::iter_swap(swpIter, toLeft[i]);
 					i++;
 				}
 				swpIter--;
 			}
 		} else {
 			uint64_t i = off;
 			const iter rMidIter = beginIter+lCnt;
 			iter swpIter = rMidIter;
 			while ((i<rc)&&(toRight[i]<rMidIter)) {
 				if (pred(*(swpIter), pivot)) {
 					std::iter_swap(toRight[i], swpIter);
 					i++;
 				}
 				swpIter++;
 			}
 		}

 		return lCnt;
	}


	// /*
	//  * Specific BF partition implementation to be called from Quicksort.
	//  * Requires < as input, because of duplicate check
	//  */
  // template<typename iter, typename BinaryPredicate>
  // uint64_t qsPartitionDC(const iter beginIter, const iter endIter,
  //     iter pivotIter, BinaryPredicate pred, uint64_t& pivotLength) {
	// 	typedef typename std::iterator_traits<iter>::value_type T;
	// 	T pivot = *pivotIter; // Pivot might get swapped
	// 	long long int lCnt = 0; //TODO Type

	// 	if (endIter-beginIter>=2*MYBLOCKSIZE) {
	// 		T temp;
	// 		iter toLeft[MYBLOCKSIZE];
	// 		iter toRight[MYBLOCKSIZE];
	// 		uint64_t lc;
	// 		uint64_t rc;

	// 		bool rResult = 0;
	// 		bool lResult = 0;
	// 		iter ri = endIter;
	// 		iter li = beginIter;
	// 		uint64_t off = 0;

	// 		while(li<ri) {
	// 			lc = 0;
	// 			rc = 0;

	// 			while((lc<MYUNROLLTHRESH)&&(li<(ri-15))) {
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 			}
	// 			while((lc<MYBLOCKSIZE)&&(li<ri)) {
	// 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
	// 			}
	// 			lCnt += lc-(li-beginIter);

	// 			while((rc<MYUNROLLTHRESH)&&((li+15)<ri)) {
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 			}
	// 			while((rc<MYBLOCKSIZE)&&(li<ri)) {
	// 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
	// 			}
	// 			lCnt += (li-beginIter)-rc;

	// 			uint64_t minc = std::min(lc, rc);
	// 			off = 0;
	// 			if (minc > 0) {
	// 				temp = *toRight[off];
	// 				*toRight[off] = *toLeft[off];
	// 				off++;
	// 				while (off<minc) {
	// 					*toLeft[off-1] = *toRight[off];
	// 					*toRight[off] = *toLeft[off];
	// 					off++;
	// 				}
	// 				*toLeft[off-1] = temp;
	// 			}
	// 		}

	// 		if (lc-off > 0) {
	// 			uint64_t i = off;
	// 			const iter lMidIter = beginIter+lCnt-1;
	// 			iter swpIter = lMidIter;
	// 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
	// 				if (!pred(*(swpIter), pivot)) {
	// 					std::iter_swap(swpIter, toLeft[i]);
	// 					i++;
	// 				}
	// 				swpIter--;
	// 			}
	// 		} else {
	// 			uint64_t i = off;
	// 			const iter rMidIter = beginIter+lCnt;
	// 			iter swpIter = rMidIter;
	// 			while ((i<rc)&&(toRight[i]<rMidIter)) {
	// 				if (pred(*(swpIter), pivot)) {
	// 					std::iter_swap(toRight[i], swpIter);
	// 					i++;
	// 				}
	// 				swpIter++;
	// 			}
	// 		}
	// 	} else {
	// 		iter midIter = std::partition(beginIter, endIter, [&pivot, &pred](T v1){ return pred(v1, pivot); });
	// 		lCnt = midIter-beginIter;
	// 	}
  //
	// 	// Our version of the double pivot check proposed by Edelkamp & Weiss
	// 	// Branch-free Lomuoto Partition based on Elmasry, Katajainen and Stenmark
	// 	// At the end we have begin+lCnt = pos for out swapped pivot and begin+lCnt+pivotLength = first non-pivot element of right side
	// 	uint64_t swpCnt = 1; // Count of all swapped duplicates
	// 	uint64_t stepCnt = 0; // Count of all iterations to abort if there are not enough duplicates
	// 	uint64_t swpOff = 0; // Swap offset
	// 	bool res = 0; // Comparison result

	// 	if (lCnt<(endIter-beginIter)-3) {
	// 		iter swp = beginIter+lCnt;
	// 		iter i = beginIter+lCnt+1;

	// 		while ((swpCnt<<2)>stepCnt && i<endIter) {
	// 			// Compare
	// 			res = pivot==*i;
	// 			swp += res;
	// 			swpCnt += res;

	// 			// Swap
	// 			swpOff = res*(swp-i); // Negative!
	// 			T buff = std::move(*i);
	// 			iter swpR = swp-swpOff; // Equal to i if res==1; swp otw.
	// 			iter swpL = i+swpOff; // Equal to swp if res==1; i otw.
	// 			*swpR = std::move(*swp);
	// 			*swpL = std::move(buff);

	// 			// Proceed
	// 			i++;
	// 			stepCnt++;
	// 		}
	// 	}
	// 	pivotLength = swpCnt-1; // We have at least one pivot

	// 	return lCnt;
  // }



  /*
   * BF partition implementation
   */
  template<typename iter, typename BinaryPredicate>
 	uint64_t partition(const iter beginIter, const iter endIter,
 			iter pivotIter, BinaryPredicate pred) {
		typedef typename std::iterator_traits<iter>::value_type T;
 		T pivot = *pivotIter; // Pivot might get swapped
		T temp;
 		iter toLeft[MYBLOCKSIZE];
 		iter toRight[MYBLOCKSIZE];
 		uint64_t lc;
 		uint64_t rc;

 		bool rResult = 0;
 		bool lResult = 0;
 		iter ri = endIter;
 		iter li = beginIter;
 		long long int lCnt = 0; //TODO Type
 		uint64_t off = 0;

 		while(li<ri) {
 			lc = 0;
 			rc = 0;

 			while((lc<MYUNROLLTHRESH)&&(li<(ri-15))) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			while((lc<MYBLOCKSIZE)&&(li<ri)) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			lCnt += lc-(li-beginIter);

 			while((rc<MYUNROLLTHRESH)&&((li+15)<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			while((rc<MYBLOCKSIZE)&&(li<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			lCnt += (li-beginIter)-rc;

 			uint64_t minc = std::min(lc, rc);
			off = 0;
			if (minc > 0) {
				temp = *toRight[off];
				*toRight[off] = *toLeft[off];
				off++;
				while (off<minc) {
					*toLeft[off-1] = *toRight[off];
					*toRight[off] = *toLeft[off];
					off++;
				}
				*toLeft[off-1] = temp;
			}
 		}

 		if (lc-off > 0) {
 			uint64_t i = off;
 			const iter lMidIter = beginIter+lCnt-1;
 			iter swpIter = lMidIter;
 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
 				if (!pred(*(swpIter), pivot)) {
 					std::iter_swap(swpIter, toLeft[i]);
 					i++;
 				}
 				swpIter--;
 			}
 		} else {
 			uint64_t i = off;
 			const iter rMidIter = beginIter+lCnt;
 			iter swpIter = rMidIter;
 			while ((i<rc)&&(toRight[i]<rMidIter)) {
 				if (pred(*(swpIter), pivot)) {
 					std::iter_swap(toRight[i], swpIter);
 					i++;
 				}
 				swpIter++;
 			}
 		}

 		return lCnt;
 	}

	/*
   * BF partition implementation with dynamic block size
   */
  template<typename iter, typename BinaryPredicate>
 	uint64_t partitionDyn(const iter beginIter, const iter endIter,
 			iter pivotIter, BinaryPredicate pred) {
		typedef typename std::iterator_traits<iter>::value_type T;
 		T pivot = *pivotIter; // Pivot might get swapped
		T temp;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		const uint16_t unrollThresh = blockSize-15 > 1 ? blockSize-15 : 1;
 		iter toLeft[blockSize];
 		iter toRight[blockSize];
 		uint64_t lc;
 		uint64_t rc;

 		bool rResult = 0;
 		bool lResult = 0;
 		iter ri = endIter;
 		iter li = beginIter;
 		long long int lCnt = 0; //TODO Type
 		uint64_t off = 0;

 		while(li<ri) {
 			lc = 0;
 			rc = 0;

 			while((lc<unrollThresh)&&(li<(ri-15))) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			while((lc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			lCnt += lc-(li-beginIter);

 			while((rc<unrollThresh)&&((li+15)<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			while((rc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			lCnt += (li-beginIter)-rc;

 			uint64_t minc = std::min(lc, rc);
			off = 0;
			if (minc > 0) {
				temp = *toRight[off];
				*toRight[off] = *toLeft[off];
				off++;
				while (off<minc) {
					*toLeft[off-1] = *toRight[off];
					*toRight[off] = *toLeft[off];
					off++;
				}
				*toLeft[off-1] = temp;
			}
 		}

 		if (lc-off > 0) {
 			uint64_t i = off;
 			const iter lMidIter = beginIter+lCnt-1;
 			iter swpIter = lMidIter;
 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
 				if (!pred(*(swpIter), pivot)) {
 					std::iter_swap(swpIter, toLeft[i]);
 					i++;
 				}
 				swpIter--;
 			}
 		} else {
 			uint64_t i = off;
 			const iter rMidIter = beginIter+lCnt;
 			iter swpIter = rMidIter;
 			while ((i<rc)&&(toRight[i]<rMidIter)) {
 				if (pred(*(swpIter), pivot)) {
 					std::iter_swap(toRight[i], swpIter);
 					i++;
 				}
 				swpIter++;
 			}
 		}

 		return lCnt;
 	}

	/*
   * BF partition implementation for block sizes experiment and dynamic Quicksort
   */
  template<typename iter, typename BinaryPredicate>
 	uint64_t partitionDynQs(const iter beginIter, const iter endIter,
 			iter pivotIter, BinaryPredicate pred, const uint16_t blockSize) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t thresh = blockSize-15;

 		T pivot = *pivotIter; // Pivot might get swapped
		T temp;
 		iter toLeft[blockSize];
 		iter toRight[blockSize];
 		uint64_t lc;
 		uint64_t rc;

 		bool rResult = 0;
 		bool lResult = 0;
 		iter ri = endIter;
 		iter li = beginIter;
 		long long int lCnt = 0; //TODO Type
 		uint64_t off = 0;

 		while(li<ri) {
 			lc = 0;
 			rc = 0;

 			while((lc<thresh)&&(li<(ri-15))) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			while((lc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			lCnt += lc-(li-beginIter);

 			while((rc<thresh)&&((li+15)<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			while((rc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			lCnt += (li-beginIter)-rc;

 			uint64_t minc = std::min(lc, rc);
			off = 0;
			if (minc > 0) {
				temp = *toRight[off];
				*toRight[off] = *toLeft[off];
				off++;
				while (off<minc) {
					*toLeft[off-1] = *toRight[off];
					*toRight[off] = *toLeft[off];
					off++;
				}
				*toLeft[off-1] = temp;
			}
 		}

 		if (lc-off > 0) {
 			uint64_t i = off;
 			const iter lMidIter = beginIter+lCnt-1;
 			iter swpIter = lMidIter;
 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
 				if (!pred(*(swpIter), pivot)) {
 					std::iter_swap(swpIter, toLeft[i]);
 					i++;
 				}
 				swpIter--;
 			}
 		} else {
 			uint64_t i = off;
 			const iter rMidIter = beginIter+lCnt;
 			iter swpIter = rMidIter;
 			while ((i<rc)&&(toRight[i]<rMidIter)) {
 				if (pred(*(swpIter), pivot)) {
 					std::iter_swap(toRight[i], swpIter);
 					i++;
 				}
 				swpIter++;
 			}
 		}

 		return lCnt;
 	}

	/*
   * BF partition implementation for block sizes experiment and dynamic Quicksort
   */
  template<typename iter, typename BinaryPredicate>
 	uint64_t partitionDynQsDC(const iter beginIter, const iter endIter,
 			iter pivotIter, BinaryPredicate pred, const uint16_t blockSize, uint64_t& pivotLength) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t thresh = blockSize-15;

 		T pivot = *pivotIter; // Pivot might get swapped
		T temp;
 		iter toLeft[blockSize];
 		iter toRight[blockSize];
 		uint64_t lc;
 		uint64_t rc;

 		bool rResult = 0;
 		bool lResult = 0;
 		iter ri = endIter;
 		iter li = beginIter;
 		long long int lCnt = 0; //TODO Type
 		uint64_t off = 0;

 		while(li<ri) {
 			lc = 0;
 			rc = 0;

 			while((lc<thresh)&&(li<(ri-15))) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			while((lc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			lCnt += lc-(li-beginIter);

 			while((rc<thresh)&&((li+15)<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			while((rc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			lCnt += (li-beginIter)-rc;

 			uint64_t minc = std::min(lc, rc);
			off = 0;
			if (minc > 0) {
				temp = *toRight[off];
				*toRight[off] = *toLeft[off];
				off++;
				while (off<minc) {
					*toLeft[off-1] = *toRight[off];
					*toRight[off] = *toLeft[off];
					off++;
				}
				*toLeft[off-1] = temp;
			}
 		}

 		if (lc-off > 0) {
 			uint64_t i = off;
 			const iter lMidIter = beginIter+lCnt-1;
 			iter swpIter = lMidIter;
 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
 				if (!pred(*(swpIter), pivot)) {
 					std::iter_swap(swpIter, toLeft[i]);
 					i++;
 				}
 				swpIter--;
 			}
 		} else {
 			uint64_t i = off;
 			const iter rMidIter = beginIter+lCnt;
 			iter swpIter = rMidIter;
 			while ((i<rc)&&(toRight[i]<rMidIter)) {
 				if (pred(*(swpIter), pivot)) {
 					std::iter_swap(toRight[i], swpIter);
 					i++;
 				}
 				swpIter++;
 			}
 		}

		// Our version of the double pivot check proposed by Edelkamp & Weiss
		// Branch-free Lomuoto Partition based on Elmasry, Katajainen and Stenmark
		uint64_t swpCnt = 1; // Count of all swapped duplicates
		uint64_t stepCnt = 0; // Count of all iterations to abort if there are not enough duplicates
		uint64_t swpOff = 0; // Swap offset
		bool res = 0; // Comparison result

		if (lCnt<(endIter-beginIter)-3) {
			iter swp = beginIter+lCnt;
			iter i = beginIter+lCnt+1;

			while ((swpCnt<<2)>stepCnt && i<endIter) {
				// Compare
				res = pivot==*i;
				swp += res;
				swpCnt += res;

				// SwapS
				swpOff = res*(swp-i);
				T buff = std::move(*i);
				iter swpR = swp-swpOff; // Equal to i if res==1; swp otw.
				iter swpL = i+swpOff; // Equal to swp if res==1; i otw.
				*swpR = std::move(*swp);
				*swpL = std::move(buff);

				// Proceed
				i++;
				stepCnt++;
			}
		}
		pivotLength = swpCnt-1;

 		return lCnt;
 	}

	/*
   * BF partition implementations WITH branches!
   */

	/*
	 * To use standalone
	 */
		/*
   * BF partition implementation with dynamic block size
   */
  template<typename iter, typename BinaryPredicate>
 	uint64_t partitionDynBRANCHES(const iter beginIter, const iter endIter,
 			iter pivotIter, BinaryPredicate pred) {
		typedef typename std::iterator_traits<iter>::value_type T;
 		T pivot = *pivotIter; // Pivot might get swapped
		T temp;
		const uint16_t blockSize = CACHE_SIZE/(8*sizeof(T))<512 ? CACHE_SIZE/(8*sizeof(T)) : 512;
		const uint16_t unrollThresh = blockSize-15 > 1 ? blockSize-15 : 1;
 		iter toLeft[blockSize];
 		iter toRight[blockSize];
 		uint64_t lc;
 		uint64_t rc;

 		bool rResult = 0;
 		bool lResult = 0;
 		iter ri = endIter;
 		iter li = beginIter;
 		long long int lCnt = 0; //TODO Type
 		uint64_t off = 0;

 		while(li<ri) {
 			lc = 0;
 			rc = 0;

 			while((lc<unrollThresh)&&(li<(ri-15))) {
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 			}
 			while((lc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 			}
 			lCnt += lc-(li-beginIter);

 			while((rc<unrollThresh)&&((li+15)<ri)) {
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 			}
 			while((rc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 			}
 			lCnt += (li-beginIter)-rc;

 			uint64_t minc = std::min(lc, rc);
			off = 0;
			if (minc > 0) {
				temp = *toRight[off];
				*toRight[off] = *toLeft[off];
				off++;
				while (off<minc) {
					*toLeft[off-1] = *toRight[off];
					*toRight[off] = *toLeft[off];
					off++;
				}
				*toLeft[off-1] = temp;
			}
 		}

 		if (lc-off > 0) {
 			uint64_t i = off;
 			const iter lMidIter = beginIter+lCnt-1;
 			iter swpIter = lMidIter;
 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
 				if (!pred(*(swpIter), pivot)) {
 					std::iter_swap(swpIter, toLeft[i]);
 					i++;
 				}
 				swpIter--;
 			}
 		} else {
 			uint64_t i = off;
 			const iter rMidIter = beginIter+lCnt;
 			iter swpIter = rMidIter;
 			while ((i<rc)&&(toRight[i]<rMidIter)) {
 				if (pred(*(swpIter), pivot)) {
 					std::iter_swap(toRight[i], swpIter);
 					i++;
 				}
 				swpIter++;
 			}
 		}

 		return lCnt;
 	}

	/*
	 * To use with Quicsort
	 */
  template<typename iter, typename BinaryPredicate>
 	uint64_t partitionDynQsBRANCHES(const iter beginIter, const iter endIter,
 			iter pivotIter, BinaryPredicate pred, const uint16_t blockSize) {
		typedef typename std::iterator_traits<iter>::value_type T;
		const uint16_t thresh = blockSize-15;

 		T pivot = *pivotIter; // Pivot might get swapped
		T temp;
 		iter toLeft[blockSize];
 		iter toRight[blockSize];
 		uint64_t lc;
 		uint64_t rc;

 		bool rResult = 0;
 		bool lResult = 0;
 		iter ri = endIter;
 		iter li = beginIter;
 		long long int lCnt = 0; //TODO Type
 		uint64_t off = 0;

 		while(li<ri) {
 			lc = 0;
 			rc = 0;

 			while((lc<thresh)&&(li<(ri-15))) {
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 			}
 			while((lc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToLeftBRANCHES(ri, rResult, pivot, toLeft, lc)
 			}
 			lCnt += lc-(li-beginIter);

 			while((rc<thresh)&&((li+15)<ri)) {
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 			}
 			while((rc<blockSize)&&(li<ri)) {
 				checkAndAdvanceToRightBRANCHES(li, lResult, pivot, toRight, rc)
 			}
 			lCnt += (li-beginIter)-rc;

 			uint64_t minc = std::min(lc, rc);
			off = 0;
			if (minc > 0) {
				temp = *toRight[off];
				*toRight[off] = *toLeft[off];
				off++;
				while (off<minc) {
					*toLeft[off-1] = *toRight[off];
					*toRight[off] = *toLeft[off];
					off++;
				}
				*toLeft[off-1] = temp;
			}
 		}

 		if (lc-off > 0) {
 			uint64_t i = off;
 			const iter lMidIter = beginIter+lCnt-1;
 			iter swpIter = lMidIter;
 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
 				if (!pred(*(swpIter), pivot)) {
 					std::iter_swap(swpIter, toLeft[i]);
 					i++;
 				}
 				swpIter--;
 			}
 		} else {
 			uint64_t i = off;
 			const iter rMidIter = beginIter+lCnt;
 			iter swpIter = rMidIter;
 			while ((i<rc)&&(toRight[i]<rMidIter)) {
 				if (pred(*(swpIter), pivot)) {
 					std::iter_swap(toRight[i], swpIter);
 					i++;
 				}
 				swpIter++;
 			}
 		}

 		return lCnt;
 	}

	/*
   * Conforming to our Tuned Quicksort implementation.
	 * Implements duplicate checks.
   */
  template<typename iter, typename BinaryPredicate>
	iter tunedPartitionDC(const iter beginIter, const iter endIter,
			iter pivotIter, BinaryPredicate pred, uint64_t& pivotLength) {

		typedef typename std::iterator_traits<iter>::value_type T;
 		T pivot = *pivotIter; // Pivot might get swapped
		T temp;
 		iter toLeft[MYBLOCKSIZE];
 		iter toRight[MYBLOCKSIZE];
 		uint64_t lc;
 		uint64_t rc;

 		bool rResult = 0;
 		bool lResult = 0;
 		iter ri = endIter-1; //!
 		iter li = beginIter;
 		long long int lCnt = 0; //TODO Type
 		uint64_t off = 0;

		std::iter_swap(pivotIter, endIter-1); //!

 		while(li<ri) {
 			lc = 0;
 			rc = 0;

 			while((lc<MYUNROLLTHRESH)&&(li<(ri-15))) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			while((lc<MYBLOCKSIZE)&&(li<ri)) {
 				checkAndAdvanceToLeft(ri, rResult, pivot, toLeft, lc)
 			}
 			lCnt += lc-(li-beginIter);

 			while((rc<MYUNROLLTHRESH)&&((li+15)<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			while((rc<MYBLOCKSIZE)&&(li<ri)) {
 				checkAndAdvanceToRight(li, lResult, pivot, toRight, rc)
 			}
 			lCnt += (li-beginIter)-rc;

 			uint64_t minc = std::min(lc, rc);
			off = 0;
			if (minc > 0) {
				temp = *toRight[off];
				*toRight[off] = *toLeft[off];
				off++;
				while (off<minc) {
					*toLeft[off-1] = *toRight[off];
					*toRight[off] = *toLeft[off];
					off++;
				}
				*toLeft[off-1] = temp;
			}
 		}

 		if (lc-off > 0) {
 			uint64_t i = off;
 			const iter lMidIter = beginIter+lCnt-1;
 			iter swpIter = lMidIter;
 			while ((i<lc)&&(toLeft[i]>lMidIter)) {
 				if (!pred(*(swpIter), pivot)) {
 					std::iter_swap(swpIter, toLeft[i]);
 					i++;
 				}
 				swpIter--;
 			}
 		} else {
 			uint64_t i = off;
 			const iter rMidIter = beginIter+lCnt;
 			iter swpIter = rMidIter;
 			while ((i<rc)&&(toRight[i]<rMidIter)) {
 				if (pred(*(swpIter), pivot)) {
 					std::iter_swap(toRight[i], swpIter);
 					i++;
 				}
 				swpIter++;
 			}
 		}


		// Our version of the double pivot check proposed by Edelkamp & Weiss
		// Branch-free Lomuoto Partition based on Elmasry, Katajainen and Stenmark
		uint64_t swpCnt = 1;
		uint64_t stepCnt = 0;
		uint64_t swpOff = 0;
		bool res = 0;

		if (lCnt<(endIter-beginIter)-3) {
			iter swp = beginIter+lCnt;
			iter i = beginIter+lCnt-1;

			while ((swpCnt<<2)>stepCnt && i>=beginIter) {
				// Compare
				res = pivot==*i;
				swp -= res;
				swpCnt += res;

				// Swap
				swpOff = res*(swp-i);
				T buff = std::move(*i);
				iter swpR = swp-swpOff; // Equal to i if res==1; swp otw.
				iter swpL = i+swpOff; // Equal to swp if res==1; i otw.
				*swpR = std::move(*swp);
				*swpL = std::move(buff);

				// Proceed
				i--;
				stepCnt++;
			}
		}
		pivotLength = swpCnt-1;

		iter midIter = beginIter+lCnt;
		std::iter_swap(midIter, endIter-1);
		return midIter;
	}

	template<typename iter, typename Compare>
	struct Tuned_block_partitionDC {
		static inline iter partition(iter begin, iter end, Compare less, uint64_t& pivotLength) {
			//choose pivot
			iter mid = median::median_of_3(begin, end, less);
			//partition
			return tunedPartitionDC(begin, end, mid, less, pivotLength);
			//return hoare_block_partition_unroll_loop(begin + 1, end - 1, mid, less);
		}
	};


	//
	// Branch-free Hoare partitioner proposed and implemented by Edelkamp & Weiss
	// Code from //TODO
	//
	template< typename iter, typename Compare>
	inline iter hoare_block_partition_unroll_loop(iter begin, iter end, iter pivot_pos, Compare less ) {
		using t = typename std::iterator_traits<iter>::value_type;
		iter last = end - 1;
		int indexL[128], indexR[128];

		t pivot = std::move(*pivot_pos);
		*pivot_pos = std::move(*last);
		iter hole = last;
		t temp;
		last--;

		int num_left = 0;
		int num_right = 0;
		int start_left = 0;
		int start_right = 0;

		int j;
		int num;
		//main loop
		while (last - begin + 1 > 2 * 128)
		{
			//Compare and store in buffers
			if (num_left == 0) {
				start_left = 0;
				for (j = 0; j < 128; ) {
					indexL[num_left] = j;
					num_left += (!(less(begin[j], pivot)));
					j++;
					indexL[num_left] = j;
					num_left += (!(less(begin[j], pivot)));
					j++;
					indexL[num_left] = j;
					num_left += (!(less(begin[j], pivot)));
					j++;
					indexL[num_left] = j;
					num_left += (!(less(begin[j], pivot)));
					j++;
				}
			}
			if (num_right == 0) {
				start_right = 0;
				for (j = 0; j < 128; ) {
					indexR[num_right] = j;
					num_right += !(less(pivot, *(last - j)));
					j++;
					indexR[num_right] = j;
					num_right += !(less(pivot, *(last - j)));
					j++;
					indexR[num_right] = j;
					num_right += !(less(pivot, *(last - j)));
					j++;
					indexR[num_right] = j;
					num_right += !(less(pivot, *(last - j)));
					j++;
				}
			}
			//rearrange elements
			num = std::min(num_left, num_right);
			if (num != 0)
			{
				*hole = std::move(*(begin + indexL[start_left]));
				*(begin + indexL[start_left]) = std::move(*(last - indexR[start_right]));
				for (j = 1; j < num; j++)
				{
					*(last - indexR[start_right + j - 1]) = std::move(*(begin + indexL[start_left + j]));
					*(begin + indexL[start_left + j]) = std::move(*(last - indexR[start_right + j]));
				}
				hole = (last - indexR[start_right + num - 1]);
			}
			num_left -= num;
			num_right -= num;
			start_left += num;
			start_right += num;
			begin += (num_left == 0) ? 128 : 0;
			last -= (num_right == 0) ? 128 : 0;
		}//end main loop

		 //Compare and store in buffers final iteration
		int shiftR = 0, shiftL = 0;
		if (num_right == 0 && num_left == 0) {	//for small arrays or in the unlikely case that both buffers are empty
			shiftL = (int)((last - begin) + 1) / 2;
			shiftR = (int)(last - begin) + 1 - shiftL;
			assert(shiftL >= 0); assert(shiftL <= 128);
			assert(shiftR >= 0); assert(shiftR <= 128);
			start_left = 0; start_right = 0;
			for (j = 0; j < shiftL; j++) {
				indexL[num_left] = j;
				num_left += (!less(begin[j], pivot));
				indexR[num_right] = j;
				num_right += !less(pivot, *(last - j));
			}
			if (shiftL < shiftR)
			{
				assert(shiftL + 1 == shiftR);
				indexR[num_right] = j;
				num_right += !less(pivot, *(last - j));
			}
		}
		else if ((last - begin) + 1 - 128 <= 2 * 128 - (start_right + num_right + start_left + num_left) && (num_right + num_left) < 128 / 3) {
			int upper_right = start_right + num_right;
			int upper_left = start_left + num_left;
			assert((last - begin) - 128 + 1 > 0);
			shiftL = (int)(((last - begin) + 1 - 128) / 2); // +2*(num_right + num_left)  //- num_left
			shiftR = (int)(last - begin) - 128 + 1 - shiftL;
			if (shiftL > 128 - upper_left)
			{
				shiftR += shiftL - (128 - upper_left);
				shiftL = 128 - upper_left;
			}
			else if (shiftL < 0)
			{
				shiftR -= shiftL;
				shiftL = 0;
			}
			if (shiftR > 128 - upper_right)
			{
				shiftL += shiftR - (128 - upper_right);
				shiftR = 128 - upper_right;
			}
			else if (shiftR < 0)
			{
				shiftL -= shiftR;
				shiftR = 0;
			}

			assert(shiftL + shiftR + 128 == (last - begin) + 1);
			assert(shiftL >= 0); assert(shiftL <= 128 - upper_left);
			assert(shiftR >= 0); assert(shiftR <= 128 - upper_right);

			int j_L = 0;
			int j_R = 0;
			if (num_left != 0) {
				shiftL += 128;
				j_L = 128;
			}
			if (num_right != 0) {
				shiftR += 128;
				j_R = 128;
			}

			for (; j_L < shiftL; j_L++) {
				indexL[upper_left] = j_L;
				upper_left += (!less(begin[j_L], pivot));
			}
			num_left = upper_left - start_left;

			for (; j_R < shiftR; j_R++) {
				indexR[upper_right] = j_R;
				upper_right += !(less(pivot, *(last - j_R)));
			}
			num_right = upper_right - start_right;
		}
		else if (num_right != 0) {
			shiftL = (int)(last - begin) - 128 + 1;
			shiftR = 128;
			assert(shiftL >= 0); assert(shiftL <= 128); assert(num_left == 0);
			start_left = 0;
			for (j = 0; j < shiftL; j++) {
				indexL[num_left] = j;
				num_left += (!less(begin[j], pivot));
			}
		}
		else {
			shiftL = 128;
			shiftR = (int)(last - begin) - 128 + 1;
			assert(shiftR >= 0); assert(shiftR <= 128); assert(num_right == 0);
			start_right = 0;
			for (j = 0; j < shiftR; j++) {
				indexR[num_right] = j;
				num_right += !(less(pivot, *(last - j)));
			}
		}

		//rearrange final iteration
		num = std::min(num_left, num_right);
		if (num != 0)
		{
			*hole = std::move(*(begin + indexL[start_left]));
			*(begin + indexL[start_left]) = std::move(*(last - indexR[start_right]));
			for (j = 1; j < num; j++)
			{
				*(last - indexR[start_right + j - 1]) = std::move(*(begin + indexL[start_left + j]));
				*(begin + indexL[start_left + j]) = std::move(*(last - indexR[start_right + j]));
			}
			hole = (last - indexR[start_right + num - 1]);
		}
		num_left -= num;
		num_right -= num;
		start_left += num;
		start_right += num;

		if (num_left == 0)
			begin += shiftL;
		if (num_right == 0)
			last -= shiftR;

		//rearrange remaining elements
		if (num_left != 0)
		{
			assert(num_right == 0);
			int lowerI = start_left + num_left - 1;
			int upper = (int)(last - begin);
			while (lowerI >= start_left && indexL[lowerI] == upper)
			{
				upper--; lowerI--;
			}
			temp = std::move(*(begin + upper));
			while (lowerI >= start_left)
			{
				*(begin + upper) = std::move(*(begin + indexL[lowerI]));
				*(begin + indexL[lowerI]) = std::move(*(begin + (--upper)));
				lowerI--;
			}
			*(begin + upper) = std::move(temp);
			*hole = std::move(*(begin + upper + 1));

			*(begin + upper + 1) = std::move(pivot); // fetch the pivot
			return begin + upper + 1;

		}
		else if (num_right != 0) {
			assert(num_left == 0);
			int lowerI = start_right + num_right - 1;
			int upper = (int)(last - begin);
			while (lowerI >= start_right && indexR[lowerI] == upper)
			{
				upper--; lowerI--;
			}
			*hole = std::move(*(last - upper));
			while (lowerI >= start_right)
			{
				*(last - upper) = std::move(*(last - indexR[lowerI]));
				*(last - indexR[lowerI--]) = std::move(*(last - (--upper)));
			}

			*(last - upper) = std::move(pivot); // fetch the pivot
			return last - upper;

		}
		else { //no remaining elements
			assert(last + 1 == begin);
			*hole = std::move(*begin);
			*begin = std::move(pivot); // fetch the pivot
			return begin;
		}
	}

	// with check for elements equal to pivot -- requires that *(begin - 1) <= *pivot_pos <= *end   (in particular these positions must exist)
	template< typename iter, typename Compare>
	inline iter hoare_block_partition_unroll_loop(iter begin, iter end, iter pivot_pos, Compare less, uint64_t& pivot_length) {
		using t = typename std::iterator_traits<iter>::value_type;
		using index = typename std::iterator_traits<iter>::difference_type;
		iter last = end - 1;
		iter temp_begin = begin;
		int indexL[128], indexR[128];

		bool double_pivot_check = ((!less(*pivot_pos, *end)) || (!(less(*(begin - 1), *pivot_pos))));
		pivot_length = 1;

		t pivot = std::move(*pivot_pos);
		*pivot_pos = std::move(*last);
		iter hole = last;
		t temp;
		last--;

		int num_left = 0;
		int num_right = 0;
		int start_left = 0;
		int start_right = 0;
		int j;
		int num;

		bool small_array = (last - begin + 1 <= 2 * 128) && ((last - begin) > 48);
		//main loop
		while (last - begin + 1 > 2 * 128)
		{
			//Compare and store in buffers
			if (num_left == 0) {
				start_left = 0;
				for (j = 0; j < 128; ) {
					indexL[num_left] = j;
					num_left += (!(less(begin[j], pivot)));
					j++;
					indexL[num_left] = j;
					num_left += (!(less(begin[j], pivot)));
					j++;
					indexL[num_left] = j;
					num_left += (!(less(begin[j], pivot)));
					j++;
					indexL[num_left] = j;
					num_left += (!(less(begin[j], pivot)));
					j++;
				}
			}
			if (num_right == 0) {
				start_right = 0;
				for (j = 0; j < 128; ) {
					indexR[num_right] = j;
					num_right += !(less(pivot, *(last - j)));
					j++;
					indexR[num_right] = j;
					num_right += !(less(pivot, *(last - j)));
					j++;
					indexR[num_right] = j;
					num_right += !(less(pivot, *(last - j)));
					j++;
					indexR[num_right] = j;
					num_right += !(less(pivot, *(last - j)));
					j++;
				}
			}
			//rearrange elements
			num = std::min(num_left, num_right);
			if (num != 0)
			{
				*hole = std::move(*(begin + indexL[start_left]));
				*(begin + indexL[start_left]) = std::move(*(last - indexR[start_right]));
				for (j = 1; j < num; j++)
				{
					*(last - indexR[start_right + j - 1]) = std::move(*(begin + indexL[start_left + j]));
					*(begin + indexL[start_left + j]) = std::move(*(last - indexR[start_right + j]));
				}
				hole = (last - indexR[start_right + num - 1]);
			}
			num_left -= num;
			num_right -= num;
			start_left += num;
			start_right += num;
			begin += (num_left == 0) ? 128 : 0;
			last -= (num_right == 0) ? 128 : 0;
		}//end main loop

		if (num_left == 0) start_left = 0;
		if (num_right == 0) start_right = 0;

		 //Compare and store in buffers final iteration
		int shiftR = 0, shiftL = 0;
		if (num_right == 0 && num_left == 0) {	//for small arrays or in the unlikely case that both buffers are empty
			shiftL = (int)((last - begin) + 1) / 2;
			shiftR = (int)(last - begin) + 1 - shiftL;
			assert(shiftL >= 0); assert(shiftL <= 128);
			assert(shiftR >= 0); assert(shiftR <= 128);
			start_left = 0; start_right = 0;
			for (j = 0; j < shiftL; j++) {
				indexL[num_left] = j;
				num_left += (!less(begin[j], pivot));
				indexR[num_right] = j;
				num_right += !less(pivot, *(last - j));
			}
			if (shiftL < shiftR)
			{
				assert(shiftL + 1 == shiftR);
				indexR[num_right] = j;
				num_right += !less(pivot, *(last - j));
			}
		}
		else if (num_right != 0) {
			shiftL = (int)(last - begin) - 128 + 1;
			shiftR = 128;
			assert(shiftL >= 0); assert(shiftL <= 128); assert(num_left == 0);
			start_left = 0;
			for (j = 0; j < shiftL; j++) {
				indexL[num_left] = j;
				num_left += (!less(begin[j], pivot));
			}
		}
		else {
			shiftL = 128;
			shiftR = (int)(last - begin) - 128 + 1;
			assert(shiftR >= 0); assert(shiftR <= 128); assert(num_right == 0);
			start_right = 0;
			for (j = 0; j < shiftR; j++) {
				indexR[num_right] = j;
				num_right += !(less(pivot, *(last - j)));
			}
		}

		//rearrange final iteration
		num = std::min(num_left, num_right);
		if (num != 0)
		{
			*hole = std::move(*(begin + indexL[start_left]));
			*(begin + indexL[start_left]) = std::move(*(last - indexR[start_right]));
			for (j = 1; j < num; j++)
			{
				*(last - indexR[start_right + j - 1]) = std::move(*(begin + indexL[start_left + j]));
				*(begin + indexL[start_left + j]) = std::move(*(last - indexR[start_right + j]));
			}
			hole = (last - indexR[start_right + num - 1]);
		}
		num_left -= num;
		num_right -= num;
		start_left += num;
		start_right += num;

		if (num_left == 0)
			begin += shiftL;
		if (num_right == 0)
			last -= shiftR;

		//rearrange remaining elements
		if (num_left != 0)
		{
			assert(num_right == 0);
			int lowerI = start_left + num_left - 1;
			int upper = (int)(last - begin);
			while (lowerI >= start_left && indexL[lowerI] == upper)
			{
				upper--; lowerI--;
			}
			temp = std::move(*(begin + upper));
			while (lowerI >= start_left)
			{
				*(begin + upper) = std::move(*(begin + indexL[lowerI]));
				*(begin + indexL[lowerI]) = std::move(*(begin + (--upper)));
				lowerI--;
			}
			*(begin + upper) = std::move(temp);
			*hole = std::move(*(begin + upper + 1));

			//check for double elements if the pivot sample has repetitions or a small array is partitioned very unequal
			if (double_pivot_check || (small_array && num_left >= (15 * shiftL) / 16)) {
				iter begin_lomuto = begin + upper + 1;
				iter q = begin_lomuto + 1;

				//check at least 4 elements whether they are equal to the pivot using Elmasry, Katajainen and Stenmark's Lomuto partitioner
				unsigned int count_swaps = 1;
				unsigned int count_steps = 0;
				while (q < end && (count_swaps << 2) > count_steps) { //continue as long as there are many elements equal to pivot
					typename std::iterator_traits<iter>::value_type x = std::move(*q);
					bool smaller = !less(pivot, x);
					begin_lomuto += smaller; // smaller = 1 ? begin++ : begin
					count_swaps += smaller;
					index delta = smaller * (q - begin_lomuto);
					iter s = begin_lomuto + delta; // smaller = 1 => s = q : s = begin
					iter y = q - delta; // smaller = 1 => y = begin : y = q
					*s = std::move(*begin_lomuto);
					*y = std::move(x);
					++q;
					count_steps++;
				}

				pivot_length = begin_lomuto + 1 - (begin + upper + 1);

			//	std::cout << "check for double elements left" << pivot_length << " of " << num_left << " array size " << end - temp_begin << std::endl;
			}
			*(begin + upper + 1) = std::move(pivot); // fetch the pivot
			return begin + upper + 1;

		}
		else if (num_right != 0) {
			assert(num_left == 0);
			int lowerI = start_right + num_right - 1;
			int upper = (int)(last - begin);
			while (lowerI >= start_right && indexR[lowerI] == upper)
			{
				upper--; lowerI--;
			}
			*hole = std::move(*(last - upper));
			while (lowerI >= start_right)
			{
				*(last - upper) = std::move(*(last - indexR[lowerI]));
				*(last - indexR[lowerI--]) = std::move(*(last - (--upper)));
			}

			//check for double elements if the pivot sample has repetitions or a small array is partitioned very unequal
			if (double_pivot_check || (small_array && num_right >= (15 * shiftR) / 16)) {
				iter begin_lomuto = last - upper;
				iter q = begin_lomuto - 1;

				//check at least 4 elements whether they are equal to the pivot using Elmasry, Katajainen and Stenmark's Lomuto partitioner
				unsigned int count_swaps = 1;
				unsigned int count_steps = 0;
				while (q > temp_begin && (count_swaps << 2) > count_steps) { //continue as long as there are many elements equal to pivot
					typename std::iterator_traits<iter>::value_type x = std::move(*q);
					bool smaller = !less(x, pivot);
					begin_lomuto -= smaller; // smaller = 1 ? begin++ : begin
					count_swaps += smaller;
					index delta = smaller * (q - begin_lomuto);
					iter s = begin_lomuto + delta; // smaller = 1 => s = q : s = begin
					iter y = q - delta; // smaller = 1 => y = begin : y = q
					*s = std::move(*begin_lomuto);
					*y = std::move(x);
					--q;
					count_steps++;
				}

				pivot_length = (last - upper) + 1 - begin_lomuto;
				*(last - upper) = std::move(pivot); // fetch the pivot
				return begin_lomuto;
			}
			else
			{
				*(last - upper) = std::move(pivot); // fetch the pivot
				return last - upper;
			}


		}
		else { //no remaining elements
			assert(last + 1 == begin);
			*hole = std::move(*begin);
			*begin = std::move(pivot); // fetch the pivot
			return begin;
		}
	}


	// Lomuto Partition implemented by Elmasry, Katajainen, and Stenmark
	// Code from http://www.diku.dk/~jyrki/Myris/Kat2014S.html
	template<typename iter, typename Comp>
	inline uint64_t lomuto_partition(iter begin, iter end, iter q, Comp less) {
		typedef typename std::iterator_traits<iter>::difference_type index;
		typedef typename std::iterator_traits<iter>::value_type t;
		t v = std::move(*q);
		iter first = begin;
		*q = std::move(*first);
		q = first + 1;
		while (q < end) {
			t x = std::move(*q);
			bool smaller = less(x, v);
			begin += smaller; // smaller = 1 ? begin++ : begin
			index delta = smaller * (q - begin);
			iter s = begin + delta; // smaller = 1 => s = q : s = begin
			iter y = q - delta; // smaller = 1 => y = begin : y = q
			*s = std::move(*begin);
			*y = std::move(x);
			++q;
		}
		*first = std::move(*begin);
		*begin = std::move(v);
		return (begin-first);
	}
}
#endif
